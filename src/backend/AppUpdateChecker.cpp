#include "AppUpdateChecker.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <algorithm>

namespace {

constexpr int kInitialCheckDelayMs = 15 * 1000;
constexpr int kPeriodicCheckMs = 6 * 60 * 60 * 1000;

QStringList versionParts(const QString &value)
{
    return value.split(QLatin1Char('.'), Qt::SkipEmptyParts);
}

bool isNumericIdentifier(const QString &value)
{
    static const QRegularExpression numeric(QStringLiteral("^[0-9]+$"));
    return numeric.match(value).hasMatch();
}

} // namespace

AppUpdateChecker::AppUpdateChecker(QObject *parent)
    : QObject(parent)
    , m_currentVersion(QCoreApplication::applicationVersion().trimmed())
    , m_network(new QNetworkAccessManager(this))
    , m_periodicTimer(new QTimer(this))
{
    if (m_currentVersion.isEmpty()) {
        m_currentVersion = QStringLiteral("0.0.0");
    }

    QSettings settings;
    m_ignoredVersion = settings.value(QStringLiteral("updates/ignoredVersion")).toString();

    m_periodicTimer->setInterval(kPeriodicCheckMs);
    connect(m_periodicTimer, &QTimer::timeout, this, &AppUpdateChecker::checkNow);
    m_periodicTimer->start();

    QTimer::singleShot(kInitialCheckDelayMs, this, &AppUpdateChecker::checkNow);
}

void AppUpdateChecker::checkNow()
{
    if (m_checking) {
        return;
    }

    setChecking(true);
    setError(QString());

    QNetworkRequest request(QUrl(QStringLiteral("https://api.github.com/repos/OpenIPC/dashboard/releases?per_page=20")));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("OpenIPC-Dashboard/%1").arg(m_currentVersion));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");

    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void AppUpdateChecker::openReleasePage() const
{
    if (!m_releaseUrl.trimmed().isEmpty()) {
        QDesktopServices::openUrl(QUrl(m_releaseUrl));
    } else {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/OpenIPC/dashboard/releases")));
    }
}

void AppUpdateChecker::dismissCurrentUpdate()
{
    if (m_latestVersion.isEmpty()) {
        return;
    }

    m_ignoredVersion = m_latestVersion;
    QSettings settings;
    settings.setValue(QStringLiteral("updates/ignoredVersion"), m_ignoredVersion);
    if (m_hasUpdate) {
        m_hasUpdate = false;
        emit updateStateChanged();
    }
}

void AppUpdateChecker::remindLater()
{
    if (m_hasUpdate) {
        m_hasUpdate = false;
        emit updateStateChanged();
    }
}

bool AppUpdateChecker::isNewerThanCurrent(const QString &version) const
{
    return isVersionNewer(version, m_currentVersion);
}

int AppUpdateChecker::compareVersions(const QString &left, const QString &right)
{
    const QString normalizedLeft = normalizeVersion(left);
    const QString normalizedRight = normalizeVersion(right);

    const QString leftCore = normalizedLeft.section(QLatin1Char('-'), 0, 0).section(QLatin1Char('+'), 0, 0);
    const QString rightCore = normalizedRight.section(QLatin1Char('-'), 0, 0).section(QLatin1Char('+'), 0, 0);
    const QString leftPre = normalizedLeft.contains(QLatin1Char('-'))
        ? normalizedLeft.section(QLatin1Char('-'), 1).section(QLatin1Char('+'), 0, 0)
        : QString();
    const QString rightPre = normalizedRight.contains(QLatin1Char('-'))
        ? normalizedRight.section(QLatin1Char('-'), 1).section(QLatin1Char('+'), 0, 0)
        : QString();

    const QStringList leftNumbers = versionParts(leftCore);
    const QStringList rightNumbers = versionParts(rightCore);
    const int numericCount = std::max(leftNumbers.size(), rightNumbers.size());
    for (int i = 0; i < numericCount; ++i) {
        bool leftOk = false;
        bool rightOk = false;
        const int leftValue = i < leftNumbers.size() ? leftNumbers.at(i).toInt(&leftOk) : 0;
        const int rightValue = i < rightNumbers.size() ? rightNumbers.at(i).toInt(&rightOk) : 0;
        const int safeLeft = leftOk ? leftValue : 0;
        const int safeRight = rightOk ? rightValue : 0;
        if (safeLeft != safeRight) {
            return safeLeft < safeRight ? -1 : 1;
        }
    }

    if (leftPre.isEmpty() && rightPre.isEmpty()) {
        return 0;
    }
    if (leftPre.isEmpty()) {
        return 1;
    }
    if (rightPre.isEmpty()) {
        return -1;
    }

    const QStringList leftPreParts = versionParts(leftPre);
    const QStringList rightPreParts = versionParts(rightPre);
    const int preCount = std::max(leftPreParts.size(), rightPreParts.size());
    for (int i = 0; i < preCount; ++i) {
        if (i >= leftPreParts.size()) {
            return -1;
        }
        if (i >= rightPreParts.size()) {
            return 1;
        }

        const QString leftId = leftPreParts.at(i);
        const QString rightId = rightPreParts.at(i);
        if (leftId == rightId) {
            continue;
        }

        const bool leftNumeric = isNumericIdentifier(leftId);
        const bool rightNumeric = isNumericIdentifier(rightId);
        if (leftNumeric && rightNumeric) {
            const int leftValue = leftId.toInt();
            const int rightValue = rightId.toInt();
            if (leftValue != rightValue) {
                return leftValue < rightValue ? -1 : 1;
            }
        } else if (leftNumeric != rightNumeric) {
            return leftNumeric ? -1 : 1;
        } else {
            const int cmp = QString::compare(leftId, rightId, Qt::CaseInsensitive);
            if (cmp != 0) {
                return cmp < 0 ? -1 : 1;
            }
        }
    }

    return 0;
}

bool AppUpdateChecker::isVersionNewer(const QString &candidate, const QString &current)
{
    return compareVersions(candidate, current) > 0;
}

void AppUpdateChecker::setChecking(bool checking)
{
    if (m_checking == checking) {
        return;
    }
    m_checking = checking;
    emit checkingChanged();
}

void AppUpdateChecker::setError(const QString &error)
{
    if (m_errorString == error) {
        return;
    }
    m_errorString = error;
    emit updateStateChanged();
}

void AppUpdateChecker::clearUpdate()
{
    const bool changed = m_hasUpdate || !m_latestVersion.isEmpty() || !m_latestName.isEmpty()
        || !m_releaseUrl.isEmpty() || !m_releaseNotes.isEmpty() || m_latestPrerelease;
    m_hasUpdate = false;
    m_latestVersion.clear();
    m_latestName.clear();
    m_releaseUrl.clear();
    m_releaseNotes.clear();
    m_latestPrerelease = false;
    if (changed) {
        emit updateStateChanged();
    }
}

void AppUpdateChecker::applyRelease(const ReleaseInfo &release)
{
    const bool ignored = release.version == m_ignoredVersion;
    const bool changed = m_latestVersion != release.version
        || m_latestName != release.name
        || m_releaseUrl != release.url
        || m_releaseNotes != release.notes
        || m_latestPrerelease != release.prerelease
        || m_hasUpdate != !ignored;

    m_latestVersion = release.version;
    m_latestName = release.name;
    m_releaseUrl = release.url;
    m_releaseNotes = release.notes;
    m_latestPrerelease = release.prerelease;
    m_hasUpdate = !ignored;

    if (changed) {
        emit updateStateChanged();
    }
    if (m_hasUpdate) {
        emit updateAvailable(m_latestVersion, m_releaseUrl, m_latestPrerelease);
    }
}

void AppUpdateChecker::handleReply(QNetworkReply *reply)
{
    const auto cleanup = [this, reply]() {
        m_lastChecked = QDateTime::currentDateTime();
        setChecking(false);
        emit updateStateChanged();
        emit checkFinished(m_hasUpdate);
        reply->deleteLater();
    };

    if (reply->error() != QNetworkReply::NoError) {
        setError(reply->errorString());
        cleanup();
        return;
    }

    QJsonParseError parseError {};
    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
        setError(QStringLiteral("Invalid GitHub releases response"));
        cleanup();
        return;
    }

    ReleaseInfo best;
    const QJsonArray releases = doc.array();
    for (const QJsonValue &value : releases) {
        const QJsonObject obj = value.toObject();
        if (obj.value(QStringLiteral("draft")).toBool()) {
            continue;
        }

        const QString version = normalizeVersion(obj.value(QStringLiteral("tag_name")).toString());
        if (version.isEmpty() || !isVersionNewer(version, m_currentVersion)) {
            continue;
        }

        ReleaseInfo candidate;
        candidate.version = version;
        candidate.name = obj.value(QStringLiteral("name")).toString();
        if (candidate.name.trimmed().isEmpty()) {
            candidate.name = obj.value(QStringLiteral("tag_name")).toString();
        }
        candidate.url = obj.value(QStringLiteral("html_url")).toString();
        candidate.notes = obj.value(QStringLiteral("body")).toString();
        candidate.prerelease = obj.value(QStringLiteral("prerelease")).toBool();

        if (best.version.isEmpty() || compareVersions(candidate.version, best.version) > 0) {
            best = candidate;
        }
    }

    setError(QString());
    if (best.version.isEmpty()) {
        clearUpdate();
    } else {
        applyRelease(best);
    }

    cleanup();
}

QString AppUpdateChecker::normalizeVersion(QString version)
{
    version = version.trimmed();
    if (version.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        version.remove(0, 1);
    }
    return version;
}
