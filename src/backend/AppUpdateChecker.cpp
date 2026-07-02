#include "AppUpdateChecker.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
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

int majorVersionOf(const QString &value)
{
    bool ok = false;
    const int major = value.section(QLatin1Char('-'), 0, 0)
        .section(QLatin1Char('+'), 0, 0)
        .section(QLatin1Char('.'), 0, 0)
        .toInt(&ok);
    return ok ? major : 0;
}

#if defined(Q_OS_WIN)
QString powerShellSingleQuoted(QString value)
{
    value.replace(QLatin1Char('\''), QStringLiteral("''"));
    return QStringLiteral("'") + value + QStringLiteral("'");
}
#endif

#if defined(Q_OS_LINUX)
QString shellSingleQuoted(QString value)
{
    value.replace(QLatin1Char('\''), QStringLiteral("'\"'\"'"));
    return QStringLiteral("'") + value + QStringLiteral("'");
}
#endif

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
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

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

void AppUpdateChecker::downloadUpdate()
{
    if (m_downloading || m_installing) {
        return;
    }
    if (m_assetDownloadUrl.trimmed().isEmpty()) {
        setError(QStringLiteral("No compatible update asset for this platform (%1)").arg(platformAssetHint()));
        return;
    }

    QString error;
    if (!prepareDownloadFile(&error)) {
        setError(error);
        return;
    }

    setError(QString());
    m_downloadProgress = 0;
    m_downloadReceivedBytes = 0;
    m_downloadTotalBytes = m_assetSize;
    emit downloadProgressChanged();
    setDownloading(true);

    QNetworkRequest request{QUrl(m_assetDownloadUrl)};
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("OpenIPC-Dashboard/%1").arg(m_currentVersion));
    request.setRawHeader("Accept", "application/octet-stream");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    m_downloadReply = m_network->get(request);
    connect(m_downloadReply, &QNetworkReply::readyRead, this, [this]() {
        if (!m_downloadReply || !m_downloadFile) {
            return;
        }
        m_downloadFile->write(m_downloadReply->readAll());
    });
    connect(m_downloadReply, &QNetworkReply::downloadProgress,
            this, [this](qint64 received, qint64 total) {
        m_downloadReceivedBytes = received;
        m_downloadTotalBytes = total > 0 ? total : m_assetSize;
        if (m_downloadTotalBytes > 0) {
            m_downloadProgress = qBound(0, static_cast<int>((received * 100) / m_downloadTotalBytes), 100);
        } else {
            m_downloadProgress = 0;
        }
        emit downloadProgressChanged();
    });
    connect(m_downloadReply, &QNetworkReply::finished, this, [this]() {
        handleDownloadFinished(m_downloadReply);
    });
}

void AppUpdateChecker::cancelDownload()
{
    if (!m_downloadReply) {
        return;
    }
    m_downloadReply->abort();
}

void AppUpdateChecker::installDownloadedUpdate()
{
    if (m_downloading || m_installing) {
        return;
    }
    if (m_downloadedFilePath.trimmed().isEmpty() || !QFileInfo::exists(m_downloadedFilePath)) {
        setError(QStringLiteral("Update file is not downloaded yet"));
        return;
    }

    QString error;
    if (!startInstallerHelper(&error)) {
        setError(error);
        return;
    }

    setError(QString());
    setInstalling(true);
    emit installStarted(m_downloadedFilePath);
    QTimer::singleShot(250, qApp, &QCoreApplication::quit);
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

void AppUpdateChecker::setDownloading(bool downloading)
{
    if (m_downloading == downloading) {
        return;
    }
    m_downloading = downloading;
    emit downloadStateChanged();
}

void AppUpdateChecker::setInstalling(bool installing)
{
    if (m_installing == installing) {
        return;
    }
    m_installing = installing;
    emit installStateChanged();
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
        || !m_releaseUrl.isEmpty() || !m_releaseNotes.isEmpty() || m_latestPrerelease
        || !m_assetName.isEmpty() || !m_assetDownloadUrl.isEmpty() || m_assetSize > 0;
    m_hasUpdate = false;
    m_latestVersion.clear();
    m_latestName.clear();
    m_releaseUrl.clear();
    m_releaseNotes.clear();
    m_latestPrerelease = false;
    m_assetName.clear();
    m_assetDownloadUrl.clear();
    m_assetSize = 0;
    resetDownloadState(true);
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
        || m_assetName != release.assetName
        || m_assetDownloadUrl != release.assetDownloadUrl
        || m_assetSize != release.assetSize
        || m_hasUpdate != !ignored;

    m_latestVersion = release.version;
    m_latestName = release.name;
    m_releaseUrl = release.url;
    m_releaseNotes = release.notes;
    m_latestPrerelease = release.prerelease;
    if (m_assetName != release.assetName || m_assetDownloadUrl != release.assetDownloadUrl) {
        resetDownloadState(true);
    }
    m_assetName = release.assetName;
    m_assetDownloadUrl = release.assetDownloadUrl;
    m_assetSize = release.assetSize;
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
        const int currentMajor = majorVersionOf(normalizeVersion(m_currentVersion));
        const int candidateMajor = majorVersionOf(version);
        if (candidateMajor > currentMajor + 1) {
            continue;
        }

        QString assetDownloadUrl;
        qint64 assetSize = 0;
        const QString assetName = compatibleAssetName(obj.value(QStringLiteral("assets")).toArray(),
                                                      &assetDownloadUrl, &assetSize);
        if (assetName.isEmpty() || assetDownloadUrl.isEmpty()) {
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
        candidate.assetName = assetName;
        candidate.assetDownloadUrl = assetDownloadUrl;
        candidate.assetSize = assetSize;

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

void AppUpdateChecker::handleDownloadFinished(QNetworkReply *reply)
{
    if (!reply || reply != m_downloadReply) {
        return;
    }

    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorString = reply->errorString();
    if (m_downloadFile) {
        m_downloadFile->write(reply->readAll());
        m_downloadFile->close();
    }

    const QString filePath = m_downloadFile ? m_downloadFile->fileName() : QString();
    if (m_downloadFile) {
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
    }
    m_downloadReply = nullptr;
    reply->deleteLater();
    setDownloading(false);

    if (networkError != QNetworkReply::NoError) {
        if (!filePath.isEmpty()) {
            QFile::remove(filePath);
        }
        m_downloadedFilePath.clear();
        m_downloadProgress = 0;
        m_downloadReceivedBytes = 0;
        emit downloadProgressChanged();
        setError(networkError == QNetworkReply::OperationCanceledError ? QString() : networkErrorString);
        emit downloadFinished(false, QString());
        return;
    }

    m_downloadProgress = 100;
    if (m_downloadTotalBytes <= 0) {
        m_downloadTotalBytes = QFileInfo(filePath).size();
    }
    m_downloadReceivedBytes = m_downloadTotalBytes;
    m_downloadedFilePath = filePath;
#if defined(Q_OS_UNIX)
    QFile::setPermissions(m_downloadedFilePath,
                          QFile::permissions(m_downloadedFilePath)
                          | QFileDevice::ExeOwner
                          | QFileDevice::ExeUser
                          | QFileDevice::ExeGroup
                          | QFileDevice::ExeOther);
#endif
    emit downloadProgressChanged();
    emit downloadStateChanged();
    emit downloadFinished(true, m_downloadedFilePath);
}

bool AppUpdateChecker::prepareDownloadFile(QString *error)
{
    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempRoot.isEmpty()) {
        if (error) *error = QStringLiteral("Temporary directory is not available");
        return false;
    }

    QDir dir(tempRoot);
    if (!dir.mkpath(QStringLiteral("OpenIPC-Dashboard-Updates"))
        || !dir.cd(QStringLiteral("OpenIPC-Dashboard-Updates"))) {
        if (error) *error = QStringLiteral("Could not create update download directory");
        return false;
    }

    const QString filePath = dir.filePath(safeDownloadFileName(m_assetName));
    QFile::remove(filePath);
    if (m_downloadFile) {
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
    }

    m_downloadFile = new QFile(filePath, this);
    if (!m_downloadFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) *error = QStringLiteral("Could not write update file: %1").arg(m_downloadFile->errorString());
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
        return false;
    }

    m_downloadedFilePath.clear();
    emit downloadStateChanged();
    return true;
}

bool AppUpdateChecker::startInstallerHelper(QString *error)
{
#if defined(Q_OS_WIN)
    const QString scriptPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
        .filePath(QStringLiteral("OpenIPC-Dashboard-Update-Install.ps1"));
    QSaveFile script(scriptPath);
    if (!script.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) *error = QStringLiteral("Could not create installer helper");
        return false;
    }

    const QString body = QStringLiteral(
        "$ErrorActionPreference = 'SilentlyContinue'\n"
        "$installer = %1\n"
        "$p = Start-Process -FilePath $installer -PassThru\n"
        "if ($p) { $p.WaitForExit() }\n"
        "Start-Sleep -Seconds 2\n"
        "Remove-Item -LiteralPath $installer -Force\n"
        "Remove-Item -LiteralPath $MyInvocation.MyCommand.Path -Force\n")
        .arg(powerShellSingleQuoted(QDir::toNativeSeparators(m_downloadedFilePath)));
    script.write(body.toUtf8());
    if (!script.commit()) {
        if (error) *error = QStringLiteral("Could not save installer helper");
        return false;
    }

    const QStringList args{
        QStringLiteral("-NoProfile"),
        QStringLiteral("-ExecutionPolicy"),
        QStringLiteral("Bypass"),
        QStringLiteral("-File"),
        scriptPath
    };
    if (!QProcess::startDetached(QStringLiteral("powershell.exe"), args)) {
        if (error) *error = QStringLiteral("Could not start installer");
        return false;
    }
    return true;
#elif defined(Q_OS_LINUX)
    const QString currentAppImage = QString::fromLocal8Bit(qgetenv("APPIMAGE")).trimmed();
    if (currentAppImage.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Automatic Linux installation is available only when running from AppImage");
        }
        return false;
    }

    const QFileInfo targetInfo(currentAppImage);
    if (!targetInfo.exists() || !targetInfo.isWritable()) {
        if (error) {
            *error = QStringLiteral("Current AppImage is not writable: %1").arg(currentAppImage);
        }
        return false;
    }

    const QString scriptPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
        .filePath(QStringLiteral("openipc-dashboard-update-install.sh"));
    QSaveFile script(scriptPath);
    if (!script.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) *error = QStringLiteral("Could not create AppImage installer helper");
        return false;
    }

    const QString body = QStringLiteral(
        "#!/bin/sh\n"
        "NEW=%1\n"
        "TARGET=%2\n"
        "OLDPID=%3\n"
        "while kill -0 \"$OLDPID\" 2>/dev/null; do sleep 1; done\n"
        "cp \"$NEW\" \"$TARGET\"\n"
        "chmod +x \"$TARGET\"\n"
        "rm -f \"$NEW\"\n"
        "rm -f \"$0\"\n"
        "exec \"$TARGET\" >/dev/null 2>&1 &\n")
        .arg(shellSingleQuoted(m_downloadedFilePath),
             shellSingleQuoted(currentAppImage),
             QString::number(QCoreApplication::applicationPid()));
    script.write(body.toUtf8());
    if (!script.commit()) {
        if (error) *error = QStringLiteral("Could not save AppImage installer helper");
        return false;
    }
    QFile::setPermissions(scriptPath,
                          QFile::permissions(scriptPath)
                          | QFileDevice::ExeOwner
                          | QFileDevice::ExeUser
                          | QFileDevice::ExeGroup
                          | QFileDevice::ExeOther);

    if (!QProcess::startDetached(QStringLiteral("/bin/sh"), QStringList{scriptPath})) {
        if (error) *error = QStringLiteral("Could not start AppImage installer helper");
        return false;
    }
    return true;
#else
    Q_UNUSED(error)
    return false;
#endif
}

void AppUpdateChecker::resetDownloadState(bool removeFile)
{
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
    if (m_downloadFile) {
        const QString filePath = m_downloadFile->fileName();
        m_downloadFile->close();
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
        if (removeFile && !filePath.isEmpty()) {
            QFile::remove(filePath);
        }
    }
    if (removeFile && !m_downloadedFilePath.isEmpty()) {
        QFile::remove(m_downloadedFilePath);
    }
    m_downloadedFilePath.clear();
    m_downloading = false;
    m_installing = false;
    m_downloadProgress = 0;
    m_downloadReceivedBytes = 0;
    m_downloadTotalBytes = m_assetSize;
    emit downloadStateChanged();
    emit installStateChanged();
    emit downloadProgressChanged();
}

QString AppUpdateChecker::compatibleAssetName(const QJsonArray &assets, QString *downloadUrl, qint64 *size)
{
    for (const QJsonValue &value : assets) {
        const QJsonObject asset = value.toObject();
        const QString name = asset.value(QStringLiteral("name")).toString();
        if (!isCompatibleAssetName(name)) {
            continue;
        }

        if (downloadUrl) {
            *downloadUrl = asset.value(QStringLiteral("browser_download_url")).toString();
        }
        if (size) {
            *size = static_cast<qint64>(asset.value(QStringLiteral("size")).toDouble());
        }
        return name;
    }
    if (downloadUrl) downloadUrl->clear();
    if (size) *size = 0;
    return QString();
}

bool AppUpdateChecker::isCompatibleAssetName(const QString &name)
{
    const QString lower = name.toLower();
#if defined(Q_OS_WIN)
    return lower == QStringLiteral("openipc-dashboard-installer.exe")
        || (lower.contains(QStringLiteral("openipc-dashboard"))
            && lower.contains(QStringLiteral("installer"))
            && lower.endsWith(QStringLiteral(".exe")));
#elif defined(Q_OS_LINUX)
    return lower == QStringLiteral("openipc-dashboard-linux.appimage")
        || (lower.contains(QStringLiteral("openipc-dashboard"))
            && lower.endsWith(QStringLiteral(".appimage")));
#else
    Q_UNUSED(lower)
    return false;
#endif
}

QString AppUpdateChecker::platformAssetHint()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("OpenIPC-Dashboard-Installer.exe");
#elif defined(Q_OS_LINUX)
    return QStringLiteral("OpenIPC-Dashboard-Linux.AppImage");
#else
    return QStringLiteral("unsupported platform");
#endif
}

QString AppUpdateChecker::safeDownloadFileName(const QString &assetName)
{
    QString fileName = QFileInfo(assetName).fileName();
    if (fileName.isEmpty()) {
        fileName = platformAssetHint();
    }
    fileName.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
    return fileName;
}

QString AppUpdateChecker::normalizeVersion(QString version)
{
    version = version.trimmed();
    if (version.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        version.remove(0, 1);
    }
    return version;
}
