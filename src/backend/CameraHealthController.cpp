#include "CameraHealthController.h"

#include "CameraHealthPolicy.h"
#include "CameraModel.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQueue>
#include <QRegularExpression>
#include <QSet>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>
#include <QUuid>
#include <QUrlQuery>

namespace {

constexpr int maximumHistoryRuns = 30;
constexpr int maximumConcurrentProbes = 4;

QString isoNow()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}

QString displayDateTime(const QString &isoDate)
{
    const QDateTime date = QDateTime::fromString(isoDate, Qt::ISODateWithMs);
    return date.isValid() ? date.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                          : isoDate;
}

QString cameraHost(const Camera &camera)
{
    QUrl url(camera.ip);
    if (url.isValid() && !url.host().isEmpty()) return url.host();
    return camera.ip.trimmed();
}

QUrl streamUrl(const Camera &camera, bool subStream)
{
    const QString configured = subStream
        ? camera.sdStreamUrl
        : (!camera.hdStreamUrl.trimmed().isEmpty() ? camera.hdStreamUrl : camera.streamUrl);
    QUrl url(configured);
    if (url.isValid() && !url.host().isEmpty()) return url;

    url.setScheme(QStringLiteral("rtsp"));
    url.setHost(cameraHost(camera));
    url.setPort(camera.port > 0 ? camera.port : 554);
    url.setPath(subStream ? QStringLiteral("/stream=1") : QStringLiteral("/stream=0"));
    return url;
}

QString safeUrlString(QUrl url)
{
    url.setUserInfo(QString());
    return url.toString(QUrl::FullyEncoded);
}

QString profileLabel(const QString &profileId)
{
    for (const QVariant &value : CameraHealthPolicy::profiles()) {
        const QVariantMap profile = value.toMap();
        if (profile.value(QStringLiteral("id")).toString() == profileId) {
            return profile.value(QStringLiteral("label")).toString();
        }
    }
    return profileId;
}

QString responseVersion(const QByteArray &body)
{
    const QString text = QString::fromUtf8(body.left(64 * 1024));
    const QList<QRegularExpression> expressions{
        QRegularExpression(QStringLiteral(
            "\"(?:version|firmware|build)\"\\s*:\\s*\"([^\"]+)\""),
            QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QStringLiteral(
            R"((?:Firmware|Version|Build)\s*</?[^>]*>\s*[:=]?\s*([A-Za-z0-9._+-]+))"),
            QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QStringLiteral(
            R"((?:Firmware|Version|Build)\s*[:=]\s*([A-Za-z0-9._+-]+))"),
            QRegularExpression::CaseInsensitiveOption)
    };
    for (const QRegularExpression &expression : expressions) {
        const QRegularExpressionMatch match = expression.match(text);
        if (match.hasMatch()) return match.captured(1).trimmed().left(120);
    }
    return {};
}

QString cleanText(const QByteArray &body, int maximumLength)
{
    QString text = QString::fromUtf8(body.left(maximumLength * 2));
    text.remove(QRegularExpression(QStringLiteral("<script[^>]*>.*?</script>"),
                                   QRegularExpression::CaseInsensitiveOption
                                       | QRegularExpression::DotMatchesEverythingOption));
    text.replace(QRegularExpression(QStringLiteral("<[^>]+>")), QStringLiteral(" "));
    text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    text.replace(QRegularExpression(QStringLiteral("[\\t\\r ]+")), QStringLiteral(" "));
    text.replace(QRegularExpression(QStringLiteral("\\n{3,}")), QStringLiteral("\n\n"));
    return text.trimmed().left(maximumLength);
}

QString metricsSummary(const QByteArray &body)
{
    QStringList result;
    const QStringList lines = QString::fromUtf8(body.left(64 * 1024)).split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) continue;
        result.append(trimmed.left(240));
        if (result.size() >= 20) break;
    }
    return result.join(QLatin1Char('\n'));
}

QVariant metricNumber(const QByteArray &body, const QString &metricName)
{
    const QStringList lines = QString::fromUtf8(body.left(64 * 1024)).split(QLatin1Char('\n'));
    for (const QString &lineValue : lines) {
        const QString line = lineValue.trimmed();
        if (!line.startsWith(metricName)) continue;
        if (line.size() > metricName.size()) {
            const QChar suffix = line.at(metricName.size());
            if (!suffix.isSpace() && suffix != QLatin1Char('{')) continue;
        }
        const int separator = line.lastIndexOf(QRegularExpression(QStringLiteral("\\s+")));
        if (separator < 0) continue;
        bool ok = false;
        const double value = line.mid(separator).trimmed().toDouble(&ok);
        if (ok) return value;
    }
    return {};
}

QVariantMap taskForProbe(const Camera &camera, const QVariantMap &probeSpec,
                         const QString &runId)
{
    QVariantMap task = probeSpec;
    task.insert(QStringLiteral("runId"), runId);
    task.insert(QStringLiteral("cameraIp"), camera.ip);
    task.insert(QStringLiteral("username"), camera.login);
    task.insert(QStringLiteral("password"), camera.password);

    const QString id = probeSpec.value(QStringLiteral("id")).toString();
    if (id == QStringLiteral("rtsp-main") || id == QStringLiteral("rtsp-sub")) {
        const QUrl url = streamUrl(camera, id == QStringLiteral("rtsp-sub"));
        task.insert(QStringLiteral("url"), url);
        task.insert(QStringLiteral("host"), url.host());
        task.insert(QStringLiteral("port"), url.port(554));
        return task;
    }

    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(cameraHost(camera));
    url.setPort(camera.onvifPort > 0 ? camera.onvifPort : 80);
    if (id == QStringLiteral("majestic-config")) {
        url.setPath(QStringLiteral("/api/v1/config.json"));
    } else if (id == QStringLiteral("majestic-schema")) {
        url.setPath(QStringLiteral("/api/v1/config.schema.json"));
    } else if (id == QStringLiteral("firmware-status")) {
        url.setPath(QStringLiteral("/cgi-bin/status.cgi"));
    } else if (id == QStringLiteral("metrics")) {
        url.setPath(QStringLiteral("/metrics"));
    } else if (id == QStringLiteral("logs-readiness")) {
        url.setPath(QStringLiteral("/ws/logs"));
    } else if (id == QStringLiteral("logs-sample")) {
        url.setPath(QStringLiteral("/cgi-bin/j/run.cgi"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("web"),
                           QString::fromUtf8(QByteArrayLiteral("logread -n 40").toBase64()));
        url.setQuery(query);
    } else if (id == QStringLiteral("snapshot")) {
        url.setPath(QStringLiteral("/image.jpg"));
    }
    task.insert(QStringLiteral("url"), url);
    task.insert(QStringLiteral("host"), url.host());
    task.insert(QStringLiteral("port"), url.port(80));
    return task;
}

} // namespace

class CameraHealthControllerPrivate
{
public:
    CameraModel *cameraModel = nullptr;
    CameraModel *gridModel = nullptr;
    QNetworkAccessManager *network = nullptr;
    QVariantList history;
    QHash<QString, QVariantMap> cameraResults;
    QHash<QString, QVariantMap> telemetry;
    QHash<QString, qint64> telemetryRequestedAt;
    QSet<QString> telemetryPending;
    QStringList cameraOrder;
    QQueue<QVariantMap> pendingTasks;
    QVariantMap activeRun;
    QString activeProfile;
    QString activeRunId;
    int activeProbeCount = 0;
    int completedProbes = 0;
    int totalProbes = 0;
    bool running = false;
};

CameraHealthController::CameraHealthController(CameraModel *cameraModel, CameraModel *gridModel,
                                               QObject *parent)
    : QObject(parent)
    , d(std::make_unique<CameraHealthControllerPrivate>())
{
    d->cameraModel = cameraModel;
    d->gridModel = gridModel;
    d->network = new QNetworkAccessManager(this);
}

CameraHealthController::~CameraHealthController() = default;

QVariantList CameraHealthController::profiles() const
{
    return CameraHealthPolicy::profiles();
}

QVariantList CameraHealthController::history() const
{
    return d->history;
}

QVariantList CameraHealthController::currentResults() const
{
    QVariantList results;
    results.reserve(d->cameraOrder.size());
    for (const QString &cameraIp : d->cameraOrder) {
        results.append(d->cameraResults.value(cameraIp));
    }
    return results;
}

QVariantMap CameraHealthController::latestRun() const
{
    return d->history.isEmpty() ? QVariantMap{} : d->history.constFirst().toMap();
}

bool CameraHealthController::running() const
{
    return d->running;
}

int CameraHealthController::completedProbes() const
{
    return d->completedProbes;
}

int CameraHealthController::totalProbes() const
{
    return d->totalProbes;
}

QString CameraHealthController::activeProfile() const
{
    return d->activeProfile;
}

bool CameraHealthController::runAll(const QString &profileId)
{
    QList<int> indexes;
    if (d->cameraModel) {
        indexes.reserve(d->cameraModel->rowCount());
        for (int index = 0; index < d->cameraModel->rowCount(); ++index) indexes.append(index);
    }
    return startRun(indexes, profileId);
}

bool CameraHealthController::runCamera(const QString &cameraIp, const QString &profileId)
{
    if (!d->cameraModel) return false;
    const int index = d->cameraModel->findIndexByIp(cameraIp.trimmed());
    return index >= 0 && startRun({index}, profileId);
}

bool CameraHealthController::refreshCameraTelemetry(const QString &cameraIp)
{
    if (!d->cameraModel
        || qEnvironmentVariable("OPENIPC_SMOKE_QML") == QStringLiteral("1")) {
        return false;
    }

    const QString ip = cameraIp.trimmed();
    const int index = d->cameraModel->findIndexByIp(ip);
    if (index < 0 || d->telemetryPending.contains(ip)) return false;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (d->telemetry.contains(ip)
        && now - d->telemetryRequestedAt.value(ip) < 30000) {
        return false;
    }

    const Camera camera = d->cameraModel->getCamera(index);
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(cameraHost(camera));
    url.setPort(camera.onvifPort > 0 ? camera.onvifPort : 80);
    url.setPath(QStringLiteral("/metrics"));

    QNetworkRequest request(url);
    request.setTransferTimeout(5000);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setRawHeader("User-Agent", "OpenIPC-Dashboard/0.3 Sidebar");
    request.setRawHeader("Accept", "text/plain, */*");
    if (!camera.login.isEmpty()) {
        const QByteArray credentials = (camera.login + QLatin1Char(':')
            + camera.password).toUtf8().toBase64();
        request.setRawHeader("Authorization", QByteArrayLiteral("Basic ") + credentials);
    }

    d->telemetryPending.insert(ip);
    d->telemetryRequestedAt.insert(ip, now);
    QNetworkReply *reply = d->network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, ip]() {
        const QByteArray body = reply->readAll();
        d->telemetryPending.remove(ip);
        if (reply->error() == QNetworkReply::NoError) {
            const QVariant temperature = metricNumber(
                body, QStringLiteral("node_hwmon_temp_celsius"));
            if (temperature.isValid()) {
                QVariantMap values = d->telemetry.value(ip);
                values.insert(QStringLiteral("temperatureC"), temperature);
                values.insert(QStringLiteral("updatedAt"), isoNow());
                d->telemetry.insert(ip, values);
                emit currentResultsChanged();
                emit telemetryUpdated(ip);
            }
        }
        reply->deleteLater();
    });
    return true;
}

void CameraHealthController::refreshAllTelemetry()
{
    if (!d->cameraModel) return;
    const int limit = qMin(d->cameraModel->rowCount(), 32);
    for (int index = 0; index < limit; ++index) {
        const Camera camera = d->cameraModel->getCamera(index);
        if (!camera.ip.trimmed().isEmpty()) refreshCameraTelemetry(camera.ip);
    }
}

bool CameraHealthController::startRun(const QList<int> &cameraIndexes, const QString &profileId)
{
    if (d->running || !d->cameraModel || cameraIndexes.isEmpty()) return false;

    d->activeProfile = CameraHealthPolicy::normalizeProfile(profileId);
    d->activeRunId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    d->cameraResults.clear();
    d->cameraOrder.clear();
    d->pendingTasks.clear();
    d->activeProbeCount = 0;
    d->completedProbes = 0;
    d->totalProbes = 0;
    d->running = true;

    d->activeRun = {
        {QStringLiteral("schemaVersion"), 2},
        {QStringLiteral("id"), d->activeRunId},
        {QStringLiteral("profile"), d->activeProfile},
        {QStringLiteral("profileLabel"), profileLabel(d->activeProfile)},
        {QStringLiteral("startedAt"), isoNow()},
        {QStringLiteral("completedAt"), QString()},
        {QStringLiteral("summary"), QStringLiteral("Running")}
    };

    for (int cameraIndex : cameraIndexes) {
        if (cameraIndex < 0 || cameraIndex >= d->cameraModel->rowCount()) continue;
        const Camera camera = d->cameraModel->getCamera(cameraIndex);
        if (camera.ip.trimmed().isEmpty()) continue;

        bool inGrid = false;
        if (d->gridModel) {
            for (int gridIndex = 0; gridIndex < d->gridModel->rowCount(); ++gridIndex) {
                if (d->gridModel->getCamera(gridIndex).ip == camera.ip) {
                    inGrid = true;
                    break;
                }
            }
        }

        const bool hasSubStream = !camera.sdStreamUrl.trimmed().isEmpty();
        const QVariantList plan = CameraHealthPolicy::probePlan(d->activeProfile, hasSubStream);
        QVariantList pendingProbes;
        for (const QVariant &value : plan) {
            QVariantMap item = value.toMap();
            item.insert(QStringLiteral("status"), QStringLiteral("pending"));
            item.insert(QStringLiteral("message"), QStringLiteral("Waiting"));
            item.insert(QStringLiteral("httpStatus"), 0);
            item.insert(QStringLiteral("elapsedMs"), 0);
            pendingProbes.append(item);
            d->pendingTasks.enqueue(taskForProbe(camera, value.toMap(), d->activeRunId));
        }

        const QUrl mainUrl = streamUrl(camera, false);
        QVariantMap result{
            {QStringLiteral("name"), camera.name.trimmed().isEmpty() ? camera.ip : camera.name},
            {QStringLiteral("ip"), camera.ip},
            {QStringLiteral("profile"), d->activeProfile},
            {QStringLiteral("status"), QStringLiteral("running")},
            {QStringLiteral("recommendation"), QStringLiteral("Diagnostics are running.")},
            {QStringLiteral("inGrid"), inGrid},
            {QStringLiteral("group"), camera.group},
            {QStringLiteral("rtspPort"), mainUrl.port(camera.port > 0 ? camera.port : 554)},
            {QStringLiteral("httpPort"), camera.onvifPort > 0 ? camera.onvifPort : 80},
            {QStringLiteral("mainStreamUrl"), safeUrlString(mainUrl)},
            {QStringLiteral("subStreamUrl"), camera.sdStreamUrl.trimmed().isEmpty()
                 ? QString() : safeUrlString(QUrl(camera.sdStreamUrl))},
            {QStringLiteral("firmwareVersion"), QString()},
            {QStringLiteral("majesticVersion"), QString()},
            {QStringLiteral("metrics"), QString()},
            {QStringLiteral("temperatureC"), QVariant()},
            {QStringLiteral("lastLogs"), QString()},
            {QStringLiteral("probes"), pendingProbes}
        };
        d->cameraOrder.append(camera.ip);
        d->cameraResults.insert(camera.ip, result);
        d->totalProbes += plan.size();
    }

    emit runningChanged();
    emit progressChanged();
    emit currentResultsChanged();

    if (d->totalProbes == 0) {
        finalizeRun();
    } else {
        startPendingProbes();
    }
    return true;
}

void CameraHealthController::startPendingProbes()
{
    while (d->running && d->activeProbeCount < maximumConcurrentProbes
           && !d->pendingTasks.isEmpty()) {
        const QVariantMap task = d->pendingTasks.dequeue();
        ++d->activeProbeCount;
        setProbeRunning(task);
        startProbe(task);
    }
}

void CameraHealthController::setProbeRunning(const QVariantMap &task)
{
    const QString cameraIp = task.value(QStringLiteral("cameraIp")).toString();
    QVariantMap camera = d->cameraResults.value(cameraIp);
    QVariantList probes = camera.value(QStringLiteral("probes")).toList();
    for (QVariant &value : probes) {
        QVariantMap item = value.toMap();
        if (item.value(QStringLiteral("id")) == task.value(QStringLiteral("id"))) {
            item.insert(QStringLiteral("status"), QStringLiteral("running"));
            item.insert(QStringLiteral("message"), QStringLiteral("Checking"));
            value = item;
            break;
        }
    }
    camera.insert(QStringLiteral("probes"), probes);
    d->cameraResults.insert(cameraIp, camera);
    emit currentResultsChanged();
    emit cameraResultUpdated(cameraIp);
}

void CameraHealthController::startProbe(const QVariantMap &task)
{
    if (task.value(QStringLiteral("kind")).toString() == QStringLiteral("rtsp")) {
        startRtspProbe(task);
    } else {
        startHttpProbe(task);
    }
}

void CameraHealthController::startRtspProbe(const QVariantMap &task)
{
    const QUrl url = task.value(QStringLiteral("url")).toUrl();
    if (url.host().isEmpty()) {
        completeProbe(task, false, QStringLiteral("RTSP host is empty"), 0, 0);
        return;
    }

    auto *socket = new QTcpSocket(this);
    auto *timeout = new QTimer(socket);
    timeout->setSingleShot(true);
    timeout->setInterval(5000);
    const qint64 startedAt = QDateTime::currentMSecsSinceEpoch();
    const auto finished = QSharedPointer<bool>::create(false);

    const auto finish = [this, task, socket, startedAt, finished](
                            bool success, const QString &message, int statusCode) {
        if (*finished) return;
        *finished = true;
        const int elapsed = static_cast<int>(QDateTime::currentMSecsSinceEpoch() - startedAt);
        completeProbe(task, success, message, statusCode, elapsed);
        socket->abort();
        socket->deleteLater();
    };

    connect(socket, &QTcpSocket::connected, socket, [socket, url, task]() {
        QByteArray request = QByteArrayLiteral("OPTIONS ") + url.toEncoded()
            + QByteArrayLiteral(" RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: OpenIPC-Dashboard\r\n");
        const QString username = task.value(QStringLiteral("username")).toString();
        if (!username.isEmpty()) {
            const QByteArray credentials = (username + QLatin1Char(':')
                + task.value(QStringLiteral("password")).toString()).toUtf8().toBase64();
            request += QByteArrayLiteral("Authorization: Basic ") + credentials + QByteArrayLiteral("\r\n");
        }
        request += QByteArrayLiteral("\r\n");
        socket->write(request);
        socket->flush();
    });
    connect(socket, &QTcpSocket::readyRead, this, [socket, finish]() {
        const QByteArray response = socket->readAll();
        const QRegularExpression expression(QStringLiteral(R"(RTSP/\d(?:\.\d)?\s+(\d{3}))"));
        const QRegularExpressionMatch match = expression.match(QString::fromLatin1(response.left(256)));
        if (!match.hasMatch()) {
            finish(false, QStringLiteral("The TCP port responded, but the response is not RTSP"), 0);
            return;
        }
        const int status = match.captured(1).toInt();
        if (status >= 200 && status < 400) {
            finish(true, QStringLiteral("RTSP endpoint responded"), status);
        } else if (status == 401 || status == 403) {
            finish(false, QStringLiteral("RTSP authentication failed (%1)").arg(status), status);
        } else {
            finish(false, QStringLiteral("RTSP endpoint returned %1").arg(status), status);
        }
    });
    connect(socket, &QTcpSocket::errorOccurred, this,
            [socket, finish](QAbstractSocket::SocketError) {
        finish(false, socket->errorString(), 0);
    });
    connect(timeout, &QTimer::timeout, this, [finish]() {
        finish(false, QStringLiteral("RTSP connection timed out"), 0);
    });

    timeout->start();
    socket->connectToHost(url.host(), static_cast<quint16>(url.port(554)));
}

void CameraHealthController::startHttpProbe(const QVariantMap &task)
{
    const QUrl url = task.value(QStringLiteral("url")).toUrl();
    if (!url.isValid() || url.host().isEmpty()) {
        completeProbe(task, false, QStringLiteral("HTTP host is empty"), 0, 0);
        return;
    }

    QNetworkRequest request(url);
    request.setTransferTimeout(6000);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setRawHeader("User-Agent", "OpenIPC-Dashboard/0.3 Health");
    request.setRawHeader("Accept", "*/*");
    const QString username = task.value(QStringLiteral("username")).toString();
    if (!username.isEmpty()) {
        const QByteArray credentials = (username + QLatin1Char(':')
            + task.value(QStringLiteral("password")).toString()).toUtf8().toBase64();
        request.setRawHeader("Authorization", QByteArrayLiteral("Basic ") + credentials);
    }

    const QString id = task.value(QStringLiteral("id")).toString();
    if (id == QStringLiteral("snapshot")) {
        request.setRawHeader("Range", "bytes=0-4095");
    } else if (id == QStringLiteral("logs-readiness")) {
        request.setRawHeader("Connection", "Upgrade");
        request.setRawHeader("Upgrade", "websocket");
        request.setRawHeader("Sec-WebSocket-Version", "13");
        request.setRawHeader("Sec-WebSocket-Key", "T3BlbklQQ0hlYWx0aA==");
    }

    const qint64 startedAt = QDateTime::currentMSecsSinceEpoch();
    QNetworkReply *reply = d->network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, task, id, startedAt]() {
        const QByteArray body = reply->readAll();
        const int elapsed = static_cast<int>(QDateTime::currentMSecsSinceEpoch() - startedAt);
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        bool success = reply->error() == QNetworkReply::NoError && status >= 200 && status < 300;
        if (id == QStringLiteral("logs-readiness")
            && (status == 101 || status == 400 || status == 426)) {
            success = true;
        }

        QString message;
        if (success) {
            message = QStringLiteral("Endpoint responded");
        } else if (status == 401 || status == 403) {
            message = QStringLiteral("HTTP authentication failed (%1)").arg(status);
        } else if (status > 0) {
            message = QStringLiteral("HTTP endpoint returned %1").arg(status);
        } else {
            message = reply->errorString();
        }

        QVariantMap details;
        const QString version = responseVersion(body);
        if (!version.isEmpty()) details.insert(QStringLiteral("version"), version);
        if (id == QStringLiteral("metrics") && success) {
            details.insert(QStringLiteral("metrics"), metricsSummary(body));
            const QVariant temperature = metricNumber(
                body, QStringLiteral("node_hwmon_temp_celsius"));
            if (temperature.isValid()) {
                details.insert(QStringLiteral("temperatureC"), temperature);
            }
        } else if (id == QStringLiteral("logs-sample") && success) {
            details.insert(QStringLiteral("logs"), cleanText(body, 6000));
        } else if (id == QStringLiteral("snapshot") && success) {
            details.insert(QStringLiteral("contentType"),
                           QString::fromLatin1(reply->rawHeader("Content-Type")));
        }

        completeProbe(task, success, message, status, elapsed, details);
        reply->deleteLater();
    });
}

void CameraHealthController::completeProbe(const QVariantMap &task, bool success,
                                           const QString &message, int httpStatus,
                                           int elapsedMs, const QVariantMap &details)
{
    if (!d->running || task.value(QStringLiteral("runId")).toString() != d->activeRunId) return;

    const QString cameraIp = task.value(QStringLiteral("cameraIp")).toString();
    const QString probeId = task.value(QStringLiteral("id")).toString();
    QVariantMap camera = d->cameraResults.value(cameraIp);
    QVariantList probes = camera.value(QStringLiteral("probes")).toList();
    for (QVariant &value : probes) {
        QVariantMap item = value.toMap();
        if (item.value(QStringLiteral("id")).toString() != probeId) continue;
        const bool required = item.value(QStringLiteral("required"), true).toBool();
        item.insert(QStringLiteral("status"), success ? QStringLiteral("ok")
                                                       : (required ? QStringLiteral("error")
                                                                   : QStringLiteral("warning")));
        item.insert(QStringLiteral("message"), message.left(300));
        item.insert(QStringLiteral("httpStatus"), httpStatus);
        item.insert(QStringLiteral("elapsedMs"), elapsedMs);
        item.insert(QStringLiteral("endpoint"),
                    safeUrlString(task.value(QStringLiteral("url")).toUrl()));
        value = item;
        break;
    }
    camera.insert(QStringLiteral("probes"), probes);
    if (probeId == QStringLiteral("firmware-status")) {
        camera.insert(QStringLiteral("firmwareVersion"), details.value(QStringLiteral("version")));
    } else if (probeId == QStringLiteral("majestic-config")) {
        camera.insert(QStringLiteral("majesticVersion"), details.value(QStringLiteral("version")));
    } else if (probeId == QStringLiteral("metrics")) {
        camera.insert(QStringLiteral("metrics"), details.value(QStringLiteral("metrics")));
        if (details.contains(QStringLiteral("temperatureC"))) {
            camera.insert(QStringLiteral("temperatureC"),
                          details.value(QStringLiteral("temperatureC")));
            QVariantMap telemetry = d->telemetry.value(cameraIp);
            telemetry.insert(QStringLiteral("temperatureC"),
                             details.value(QStringLiteral("temperatureC")));
            telemetry.insert(QStringLiteral("updatedAt"), isoNow());
            d->telemetry.insert(cameraIp, telemetry);
            emit telemetryUpdated(cameraIp);
        }
    } else if (probeId == QStringLiteral("logs-sample")) {
        camera.insert(QStringLiteral("lastLogs"), details.value(QStringLiteral("logs")));
    }
    camera.insert(QStringLiteral("status"), CameraHealthPolicy::overallStatus(probes));
    camera.insert(QStringLiteral("recommendation"), CameraHealthPolicy::recommendation(camera));
    d->cameraResults.insert(cameraIp, camera);

    d->activeProbeCount = qMax(0, d->activeProbeCount - 1);
    ++d->completedProbes;
    emit progressChanged();
    emit currentResultsChanged();
    emit cameraResultUpdated(cameraIp);

    if (d->completedProbes >= d->totalProbes) {
        finalizeRun();
    } else {
        startPendingProbes();
    }
}

void CameraHealthController::finalizeRun()
{
    int healthy = 0;
    int warnings = 0;
    int errors = 0;
    QVariantList cameras;
    for (const QString &cameraIp : d->cameraOrder) {
        QVariantMap camera = d->cameraResults.value(cameraIp);
        const QString status = CameraHealthPolicy::overallStatus(
            camera.value(QStringLiteral("probes")).toList());
        camera.insert(QStringLiteral("status"), status);
        camera.insert(QStringLiteral("recommendation"), CameraHealthPolicy::recommendation(camera));
        d->cameraResults.insert(cameraIp, camera);
        cameras.append(camera);
        if (status == QStringLiteral("ok")) ++healthy;
        else if (status == QStringLiteral("error")) ++errors;
        else ++warnings;
    }

    d->activeRun.insert(QStringLiteral("completedAt"), isoNow());
    d->activeRun.insert(QStringLiteral("cameras"), cameras);
    d->activeRun.insert(QStringLiteral("healthyCount"), healthy);
    d->activeRun.insert(QStringLiteral("warningCount"), warnings);
    d->activeRun.insert(QStringLiteral("errorCount"), errors);
    d->activeRun.insert(
        QStringLiteral("summary"),
        QStringLiteral("%1 camera(s): %2 healthy, %3 warning(s), %4 error(s)")
            .arg(cameras.size()).arg(healthy).arg(warnings).arg(errors));
    d->activeRun.insert(QStringLiteral("startedAtLabel"),
                        displayDateTime(d->activeRun.value(QStringLiteral("startedAt")).toString()));
    d->activeRun.insert(QStringLiteral("completedAtLabel"),
                        displayDateTime(d->activeRun.value(QStringLiteral("completedAt")).toString()));

    d->history.prepend(d->activeRun);
    while (d->history.size() > maximumHistoryRuns) d->history.removeLast();
    d->running = false;
    d->activeProbeCount = 0;
    d->pendingTasks.clear();

    emit currentResultsChanged();
    emit progressChanged();
    emit runningChanged();
    emit historyChanged();
    emit runCompleted(d->activeRunId);
}

void CameraHealthController::clearHistory()
{
    if (d->history.isEmpty()) return;
    d->history.clear();
    emit historyChanged();
}

QVariantMap CameraHealthController::runById(const QString &runId) const
{
    const QString id = runId.trimmed();
    if (id.isEmpty()) return latestRun();
    for (const QVariant &value : d->history) {
        const QVariantMap run = value.toMap();
        if (run.value(QStringLiteral("id")).toString() == id) return run;
    }
    if (d->activeRun.value(QStringLiteral("id")).toString() == id) return d->activeRun;
    return {};
}

QVariantMap CameraHealthController::resultForCamera(const QString &cameraIp) const
{
    const QString ip = cameraIp.trimmed();
    QVariantMap result = d->cameraResults.value(ip);
    for (const QVariant &runValue : d->history) {
        const QVariantList cameras = runValue.toMap().value(QStringLiteral("cameras")).toList();
        for (const QVariant &cameraValue : cameras) {
            const QVariantMap camera = cameraValue.toMap();
            if (camera.value(QStringLiteral("ip")).toString() != ip) continue;
            if (result.isEmpty()) {
                result = camera;
            } else {
                const QStringList telemetryKeys{
                    QStringLiteral("temperatureC"),
                    QStringLiteral("firmwareVersion"),
                    QStringLiteral("majesticVersion")
                };
                for (const QString &key : telemetryKeys) {
                    if ((!result.value(key).isValid()
                         || result.value(key).toString().trimmed().isEmpty())
                        && camera.value(key).isValid()) {
                        result.insert(key, camera.value(key));
                    }
                }
            }
            break;
        }
    }
    const QVariantMap telemetry = d->telemetry.value(ip);
    for (auto it = telemetry.cbegin(); it != telemetry.cend(); ++it) {
        result.insert(it.key(), it.value());
    }
    return result;
}

QString CameraHealthController::reportText(const QString &runId) const
{
    QVariantMap run = runById(runId);
    if (run.isEmpty() && d->running) {
        run = d->activeRun;
        run.insert(QStringLiteral("cameras"), currentResults());
    }
    return CameraHealthPolicy::reportText(run);
}

QJsonArray CameraHealthController::historyJson() const
{
    return QJsonArray::fromVariantList(d->history);
}

void CameraHealthController::restoreHistory(const QJsonArray &history)
{
    QVariantList restored;
    const QVariantList values = history.toVariantList();
    for (const QVariant &value : values) {
        const QVariantMap run = value.toMap();
        if (run.value(QStringLiteral("id")).toString().trimmed().isEmpty()
            || !run.value(QStringLiteral("cameras")).canConvert<QVariantList>()) {
            continue;
        }
        restored.append(run);
        if (restored.size() >= maximumHistoryRuns) break;
    }
    d->history = restored;
    emit historyChanged();
}
