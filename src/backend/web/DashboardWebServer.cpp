#include "DashboardWebServer.h"

#include "CameraHealthController.h"
#include "CameraModel.h"
#include "DashboardWebPreviewManager.h"
#include "DashboardWebRecordingManager.h"
#include "DashboardWebRtcManager.h"
#include "SystemController.h"
#include "UserManager.h"
#include "analytics/AnalyticsEngine.h"

#include <QDesktopServices>
#include <QCoreApplication>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QSslSocket>
#include <QTcpSocket>
#include <QUrl>

#ifdef OPENIPC_HAS_QT_WEBSOCKETS
#include <QWebSocket>
#endif

namespace {

constexpr int kMaxHttpConnections = 64;
constexpr qsizetype kMaxHttpRequestBytes = 32 * 1024 + 1024 * 1024 + 4;

QString redactDeviceText(QString text)
{
    static const QRegularExpression bearer(
        QStringLiteral("(?i)\\bBearer\\s+[A-Za-z0-9._~+/-]{8,}"));
    static const QRegularExpression assignment(
        QStringLiteral("(?i)\\b(password|passwd|token|secret|authorization)\\s*[:=]\\s*[^\\s,;]+"));
    text.replace(bearer, QStringLiteral("Bearer [REDACTED]"));
    text.replace(assignment, QStringLiteral("\\1=[REDACTED]"));
    return text;
}

} // namespace

DashboardWebServer::DashboardWebServer(SystemController *systemController, QObject *parent)
    : QObject(parent)
    , m_systemController(systemController)
    , m_sessions(this)
{
    m_previewManager = new DashboardWebPreviewManager(systemController, this);
    m_recordingManager = new DashboardWebRecordingManager(systemController, this);
    m_webRtcManager = new DashboardWebRtcManager(systemController, this);
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    connect(m_webRtcManager, &DashboardWebRtcManager::messageReady, this,
            [this](const QString &peerId, const QVariantMap &message) {
        QWebSocket *socket = m_webRtcPeerSockets.value(peerId);
        if (!socket || !m_webSockets.contains(socket)) {
            m_webRtcPeerSockets.remove(peerId);
            m_webRtcManager->stopPeer(peerId);
            return;
        }
        socket->sendTextMessage(QString::fromUtf8(QJsonDocument(
            QJsonObject::fromVariantMap(message)).toJson(QJsonDocument::Compact)));
    });
    connect(m_webRtcManager, &DashboardWebRtcManager::peerStopped, this,
            [this](const QString &peerId) { m_webRtcPeerSockets.remove(peerId); });
#endif
    connect(&m_httpServer, &QTcpServer::newConnection,
            this, &DashboardWebServer::acceptHttpConnections);
    connect(&m_sessions, &DashboardWebSessionStore::countChanged,
            this, &DashboardWebServer::activeSessionsChanged);

    m_sessionCleanupTimer.setInterval(60 * 1000);
    connect(&m_sessionCleanupTimer, &QTimer::timeout,
            &m_sessions, &DashboardWebSessionStore::cleanupExpired);
    m_sessionCleanupTimer.start();

    m_broadcastTimer.setSingleShot(true);
    m_broadcastTimer.setInterval(150);
    connect(&m_broadcastTimer, &QTimer::timeout, this, &DashboardWebServer::broadcastState);

    if (!m_systemController) return;
    CameraModel *cameras = m_systemController->cameraModel();
    connect(cameras, &QAbstractItemModel::rowsInserted, this, &DashboardWebServer::scheduleBroadcast);
    connect(cameras, &QAbstractItemModel::rowsRemoved, this, &DashboardWebServer::scheduleBroadcast);
    connect(cameras, &QAbstractItemModel::dataChanged, this, &DashboardWebServer::scheduleBroadcast);
    connect(cameras, &QAbstractItemModel::modelReset, this, &DashboardWebServer::scheduleBroadcast);
    connect(m_systemController->cameraHealthController(), &CameraHealthController::currentResultsChanged,
            this, &DashboardWebServer::scheduleBroadcast);
    connect(m_systemController->analyticsEngine(), &AnalyticsEngine::analyticsEventsChanged,
            this, &DashboardWebServer::scheduleBroadcast);
    connect(m_systemController->majesticClient(), &MajesticClient::configurationLoaded, this,
            [this](const QString &id, const QVariantMap &config, const QVariantMap &schema,
                   const QVariantList &fields, const QVariantMap &capabilities) {
        m_devicePrivateData.insert(id, QVariantMap{
            {QStringLiteral("config"), config},
            {QStringLiteral("schema"), schema}
        });
        while (m_devicePrivateData.size() > 20) {
            m_devicePrivateData.erase(m_devicePrivateData.begin());
        }
        completeDeviceOperation(id, QVariantMap{
            {QStringLiteral("config"), scrubSensitive(config)},
            {QStringLiteral("schema"), scrubSensitive(schema)},
            {QStringLiteral("fields"), scrubSensitive(fields)},
            {QStringLiteral("capabilities"), scrubSensitive(capabilities)}
        });
    });
    connect(m_systemController->majesticClient(), &MajesticClient::metricsLoaded, this,
            [this](const QString &id, const QVariantMap &metrics, const QString &rawText) {
        completeDeviceOperation(id, QVariantMap{
            {QStringLiteral("metrics"), scrubSensitive(metrics)},
            {QStringLiteral("raw"), redactDeviceText(rawText).left(128 * 1024)}
        });
    });
    connect(m_systemController->majesticClient(), &MajesticClient::operationSucceeded, this,
            [this](const QString &id, const QString &operation, const QString &result) {
        if (m_deviceOperations.value(id).value(QStringLiteral("status")).toString()
            == QStringLiteral("pending")) {
            completeDeviceOperation(id, QVariantMap{{QStringLiteral("operation"), operation},
                                                     {QStringLiteral("result"), redactDeviceText(result)}});
        }
    });
    connect(m_systemController->majesticClient(), &MajesticClient::operationFailed, this,
            [this](const QString &id, const QString &, const QString &message, int httpStatus) {
        completeDeviceOperation(id, {}, redactDeviceText(message), httpStatus);
    });
    connect(m_systemController->firmwareClient(), &OpenIpcFirmwareClient::statusLoaded, this,
            [this](const QString &id, const QVariantMap &data) {
        completeDeviceOperation(id, scrubSensitive(data));
    });
    connect(m_systemController->firmwareClient(), &OpenIpcFirmwareClient::networkLoaded, this,
            [this](const QString &id, const QVariantMap &data) {
        completeDeviceOperation(id, scrubSensitive(data));
    });
    connect(m_systemController->firmwareClient(), &OpenIpcFirmwareClient::timeLoaded, this,
            [this](const QString &id, const QVariantMap &data) {
        completeDeviceOperation(id, scrubSensitive(data));
    });
    connect(m_systemController->firmwareClient(), &OpenIpcFirmwareClient::timeSynced, this,
            [this](const QString &id, const QVariantMap &data) {
        completeDeviceOperation(id, scrubSensitive(data));
    });
    connect(m_systemController->firmwareClient(), &OpenIpcFirmwareClient::logsLoaded, this,
            [this](const QString &id, const QString &source, const QString &logText) {
        completeDeviceOperation(id, QVariantMap{
            {QStringLiteral("source"), source},
            {QStringLiteral("text"), redactDeviceText(logText).left(256 * 1024)}
        });
    });
    connect(m_systemController->firmwareClient(), &OpenIpcFirmwareClient::updateInfoLoaded, this,
            [this](const QString &id, const QVariantMap &data) {
        completeDeviceOperation(id, scrubSensitive(data));
    });
    connect(m_systemController->firmwareClient(), &OpenIpcFirmwareClient::rebootStarted, this,
            [this](const QString &id) {
        completeDeviceOperation(id, QVariantMap{
            {QStringLiteral("accepted"), true},
            {QStringLiteral("message"), QStringLiteral("Device reboot started")}
        });
    });
    connect(m_systemController->firmwareClient(), &OpenIpcFirmwareClient::operationFailed, this,
            [this](const QString &id, const QString &, const QString &message, int httpStatus) {
        completeDeviceOperation(id, {}, redactDeviceText(message), httpStatus);
    });
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    connect(m_systemController->logModel(), &QAbstractItemModel::rowsInserted, this,
            [this](const QModelIndex &, int first, int last) { broadcastLogTail(first, last); });
#endif
    connect(m_systemController->userManager(), &UserManager::userSecurityChanged,
            this, &DashboardWebServer::invalidateUserSessions);
}

DashboardWebServer::~DashboardWebServer()
{
    stop();
}

QString DashboardWebServer::url() const
{
    return DashboardWebDeploymentPolicy::publicHttpUrl(
        m_deployment, m_effectiveBindAddress, port());
}

QString DashboardWebServer::localUrl() const
{
    return DashboardWebDeploymentPolicy::localHttpUrl(m_effectiveBindAddress, port());
}

QString DashboardWebServer::webSocketUrl() const
{
    return DashboardWebDeploymentPolicy::publicWebSocketUrl(
        m_deployment, m_effectiveBindAddress, webSocketPort());
}

int DashboardWebServer::connectedClients() const
{
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    return m_webSockets.size();
#else
    return 0;
#endif
}

bool DashboardWebServer::webSocketsAvailable() const
{
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    return true;
#else
    return false;
#endif
}

QVariantList DashboardWebServer::accessUrls() const
{
    QVariantList urls;
    if (!running()) return urls;
    urls.append(QVariantMap{{QStringLiteral("label"), QStringLiteral("Local")},
                            {QStringLiteral("url"), url()}});
    if (!m_allowRemote) return urls;
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() != QAbstractSocket::IPv4Protocol || address.isLoopback()) continue;
        urls.append(QVariantMap{{QStringLiteral("label"), address.toString()},
                                {QStringLiteral("url"), QStringLiteral("http://%1:%2")
                                     .arg(address.toString()).arg(port())}});
    }
    return urls;
}

void DashboardWebServer::applySettings(const QVariantMap &settings)
{
    QVariantMap effectiveSettings = settings;
    const QString environmentProfile = qEnvironmentVariable("OPENIPC_WEB_DEPLOYMENT_PROFILE").trimmed();
    const QString environmentBind = qEnvironmentVariable("OPENIPC_WEB_BIND_ADDRESS").trimmed();
    bool environmentPortOk = false;
    bool environmentWebSocketPortOk = false;
    const int environmentPort = qEnvironmentVariableIntValue("OPENIPC_WEB_PORT", &environmentPortOk);
    const int environmentWebSocketPort = qEnvironmentVariableIntValue(
        "OPENIPC_WEBSOCKET_PORT", &environmentWebSocketPortOk);
    if (!environmentProfile.isEmpty()) {
        effectiveSettings[QStringLiteral("webDeploymentProfile")] = environmentProfile;
    }
    if (!environmentBind.isEmpty()) {
        effectiveSettings[QStringLiteral("webServerBindAddress")] = environmentBind;
    }
    if (environmentPortOk) effectiveSettings[QStringLiteral("webServerPort")] = environmentPort;
    if (environmentWebSocketPortOk) {
        effectiveSettings[QStringLiteral("webSocketPort")] = environmentWebSocketPort;
    }

    const bool wasRunning = running();
    const DashboardWebDeploymentPolicy::Config previousDeployment = m_deployment;
    const QString previousAddress = m_configuredBindAddress;
    const int previousPort = m_port;
    const int previousWebSocketPort = m_webSocketPort;
    const bool previousAllowRemote = m_allowRemote;

    m_enabled = effectiveSettings.value(QStringLiteral("webServerEnabled"), false).toBool();
    m_deployment = DashboardWebDeploymentPolicy::fromSettings(effectiveSettings);
    m_allowRemote = m_deployment.allowRemote;
    m_configuredBindAddress = m_deployment.configuredBindAddress;
    m_port = qBound(1024, effectiveSettings.value(QStringLiteral("webServerPort"), 8080).toInt(), 65535);
    m_webSocketPort = qBound(1024, effectiveSettings.value(QStringLiteral("webSocketPort"), 8081).toInt(), 65535);
    if (m_webSocketPort == m_port) m_webSocketPort = m_port < 65535 ? m_port + 1 : m_port - 1;
    m_sessionTimeoutMinutes = qBound(5, effectiveSettings.value(QStringLiteral("webSessionTimeoutMinutes"), 60).toInt(), 1440);
    m_secureCookies = m_deployment.secureCookies;
    m_sessions.setTimeoutMinutes(m_sessionTimeoutMinutes);

    const bool endpointChanged = previousAddress != m_configuredBindAddress
        || previousPort != m_port || previousWebSocketPort != m_webSocketPort
        || previousAllowRemote != m_allowRemote
        || previousDeployment.profile != m_deployment.profile
        || previousDeployment.externalBaseUrl != m_deployment.externalBaseUrl
        || previousDeployment.externalWebSocketUrl != m_deployment.externalWebSocketUrl
        || previousDeployment.trustedProxyAddresses != m_deployment.trustedProxyAddresses;
    if (!m_enabled) {
        stop();
    } else if (!wasRunning || endpointChanged) {
        restart();
    }
    emit stateChanged();
}

bool DashboardWebServer::start()
{
    if (running()) return true;
    m_lastError.clear();
    m_timeToReadyMs = -1;
    m_uptimeTimer.start();
    if (!m_deployment.valid) {
        m_lastError = m_deployment.validationError;
        emit stateChanged();
        return false;
    }
    m_effectiveBindAddress = m_allowRemote ? m_configuredBindAddress : QStringLiteral("127.0.0.1");
    QHostAddress address;
    if (!address.setAddress(m_effectiveBindAddress)) {
        if (m_effectiveBindAddress.compare(QStringLiteral("localhost"), Qt::CaseInsensitive) == 0) {
            address = QHostAddress::LocalHost;
            m_effectiveBindAddress = QStringLiteral("127.0.0.1");
        } else {
            m_lastError = tr("Invalid bind address: %1").arg(m_effectiveBindAddress);
            emit stateChanged();
            return false;
        }
    }
    if (!m_httpServer.listen(address, static_cast<quint16>(m_port))) {
        m_lastError = tr("Could not listen on %1:%2: %3. Another Dashboard instance or service may already use this port.")
                          .arg(m_effectiveBindAddress).arg(m_port).arg(m_httpServer.errorString());
        emit stateChanged();
        return false;
    }
    m_httpServer.setMaxPendingConnections(kMaxHttpConnections);
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    startWebSocketServer(address);
#endif
    m_timeToReadyMs = m_uptimeTimer.elapsed();
    qInfo().noquote() << "Dashboard web server ready"
                      << "profile=" << m_deployment.profile
                      << "local=" << localUrl()
                      << "public=" << url()
                      << "websocket=" << webSocketUrl()
                      << "tlsRuntime=" << QSslSocket::supportsSsl()
                      << "readyMs=" << m_timeToReadyMs;
    emit stateChanged();
    return true;
}

void DashboardWebServer::stop()
{
    if (m_previewManager) m_previewManager->stop();
    if (m_recordingManager) m_recordingManager->stopAll();
    for (QTcpSocket *socket : std::as_const(m_previewSockets)) {
        socket->disconnectFromHost();
    }
    m_previewSockets.clear();
    for (QTcpSocket *socket : m_httpBuffers.keys()) {
        socket->disconnectFromHost();
    }
    m_httpBuffers.clear();
    m_httpServer.close();
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    m_webRtcManager->stopAll();
    m_webRtcPeerSockets.clear();
    m_webSocketSessions.clear();
    for (QWebSocket *socket : std::as_const(m_webSockets)) {
        socket->close(QWebSocketProtocol::CloseCodeGoingAway, QStringLiteral("Server stopped"));
    }
    m_webSockets.clear();
    if (m_webSocketServer) {
        m_webSocketServer->close();
        m_webSocketServer->deleteLater();
        m_webSocketServer = nullptr;
    }
#endif
    m_sessions.clear();
    emit connectedClientsChanged();
    emit stateChanged();
}

bool DashboardWebServer::restart()
{
    stop();
    return start();
}

void DashboardWebServer::openInBrowser()
{
    if (!running() && !start()) return;
    QDesktopServices::openUrl(QUrl(url()));
}

QVariantMap DashboardWebServer::status() const
{
    return {
        {QStringLiteral("running"), running()},
        {QStringLiteral("ready"), ready()},
        {QStringLiteral("url"), url()},
        {QStringLiteral("localUrl"), localUrl()},
        {QStringLiteral("webSocketUrl"), webSocketUrl()},
        {QStringLiteral("bindAddress"), bindAddress()},
        {QStringLiteral("port"), port()},
        {QStringLiteral("webSocketPort"), webSocketPort()},
        {QStringLiteral("webSocketsAvailable"), webSocketsAvailable()},
        {QStringLiteral("webRtcAvailable"), m_webRtcManager && m_webRtcManager->available()},
        {QStringLiteral("webRtcError"), m_webRtcManager
             ? m_webRtcManager->availabilityError() : QString()},
        {QStringLiteral("activeWebRtcPeers"), m_webRtcManager
             ? m_webRtcManager->activePeers() : 0},
        {QStringLiteral("activeWebRecordings"), m_recordingManager
             ? m_recordingManager->activeCount() : 0},
        {QStringLiteral("activeSessions"), activeSessions()},
        {QStringLiteral("connectedClients"), connectedClients()},
        {QStringLiteral("allowRemote"), m_allowRemote},
        {QStringLiteral("secureCookies"), m_secureCookies},
        {QStringLiteral("deployment"), DashboardWebDeploymentPolicy::publicStatus(m_deployment)},
        {QStringLiteral("tlsRuntimeAvailable"), QSslSocket::supportsSsl()},
        {QStringLiteral("uptimeMs"), m_uptimeTimer.isValid() ? m_uptimeTimer.elapsed() : 0},
        {QStringLiteral("timeToReadyMs"), m_timeToReadyMs},
        {QStringLiteral("lastError"), m_lastError},
        {QStringLiteral("accessUrls"), accessUrls()}
    };
}

QVariantMap DashboardWebServer::healthStatus(bool readiness) const
{
    const bool healthy = readiness ? ready() : true;
    return {
        {QStringLiteral("status"), healthy ? QStringLiteral("ok") : QStringLiteral("not-ready")},
        {QStringLiteral("ready"), ready()},
        {QStringLiteral("running"), running()},
        {QStringLiteral("version"), QCoreApplication::applicationVersion()},
        {QStringLiteral("profile"), m_deployment.profile},
        {QStringLiteral("uptimeMs"), m_uptimeTimer.isValid() ? m_uptimeTimer.elapsed() : 0},
        {QStringLiteral("timeToReadyMs"), m_timeToReadyMs},
        {QStringLiteral("tlsRuntimeAvailable"), QSslSocket::supportsSsl()},
        {QStringLiteral("webRtcAvailable"), m_webRtcManager && m_webRtcManager->available()},
        {QStringLiteral("webSocketsAvailable"), webSocketsAvailable()},
        {QStringLiteral("bootstrapRequired"), m_systemController
             && !m_systemController->userManager()->hasUsers()}
    };
}

QString DashboardWebServer::beginDeviceOperation(int cameraIndex, const QString &operation,
                                                 QString *error)
{
    if (!m_systemController || cameraIndex < 0
        || cameraIndex >= m_systemController->cameraModel()->rowCount()) {
        if (error) *error = tr("Camera not found");
        return {};
    }
    const Camera camera = m_systemController->cameraModel()->getCamera(cameraIndex);
    const int port = camera.onvifPort > 0 ? camera.onvifPort : 80;
    QString requestId;
    if (operation == QStringLiteral("status")) {
        requestId = m_systemController->firmwareClient()->loadStatus(
            camera.ip, port, camera.login, camera.password);
    } else if (operation == QStringLiteral("majestic")) {
        requestId = m_systemController->majesticClient()->loadConfiguration(
            camera.ip, port, camera.login, camera.password);
    } else if (operation == QStringLiteral("metrics")) {
        requestId = m_systemController->majesticClient()->loadMetrics(
            camera.ip, port, camera.login, camera.password);
    } else if (operation == QStringLiteral("network")) {
        requestId = m_systemController->firmwareClient()->loadNetwork(
            camera.ip, port, camera.login, camera.password);
    } else if (operation == QStringLiteral("time")) {
        requestId = m_systemController->firmwareClient()->loadTime(
            camera.ip, port, camera.login, camera.password);
    } else if (operation == QStringLiteral("logs")) {
        requestId = m_systemController->firmwareClient()->loadLogs(
            camera.ip, port, camera.login, camera.password, QString(), 250);
    } else if (operation == QStringLiteral("update-info")) {
        requestId = m_systemController->firmwareClient()->loadUpdateInfo(
            camera.ip, port, camera.login, camera.password);
    } else if (operation == QStringLiteral("sync-time")) {
        requestId = m_systemController->firmwareClient()->syncTime(
            camera.ip, port, camera.login, camera.password, true);
    } else if (operation == QStringLiteral("reboot")) {
        requestId = m_systemController->firmwareClient()->reboot(
            camera.ip, port, camera.login, camera.password);
    } else if (operation == QStringLiteral("github-update")) {
        requestId = m_systemController->firmwareClient()->startGithubUpdate(
            camera.ip, port, camera.login, camera.password, true, true, false, false);
    } else {
        if (error) *error = tr("Unsupported device operation");
        return {};
    }
    if (requestId.isEmpty()) {
        if (error) *error = tr("Could not start the device operation");
        return {};
    }
    while (m_deviceOperations.size() >= 100) {
        auto oldest = m_deviceOperations.begin();
        for (auto it = m_deviceOperations.begin(); it != m_deviceOperations.end(); ++it) {
            if (it.value().value(QStringLiteral("startedAt")).toString()
                < oldest.value().value(QStringLiteral("startedAt")).toString()) oldest = it;
        }
        m_deviceOperations.erase(oldest);
    }
    m_deviceOperations.insert(requestId, QVariantMap{
        {QStringLiteral("id"), requestId},
        {QStringLiteral("cameraIndex"), cameraIndex},
        {QStringLiteral("cameraId"), camera.id},
        {QStringLiteral("operation"), operation},
        {QStringLiteral("status"), QStringLiteral("pending")},
        {QStringLiteral("startedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}
    });
    return requestId;
}

QVariantMap DashboardWebServer::deviceOperation(const QString &requestId) const
{
    return m_deviceOperations.value(requestId);
}

void DashboardWebServer::completeDeviceOperation(const QString &requestId, const QVariant &data,
                                                 const QString &error, int httpStatus)
{
    auto it = m_deviceOperations.find(requestId);
    if (it == m_deviceOperations.end()) return;
    it->insert(QStringLiteral("status"), error.isEmpty()
        ? QStringLiteral("succeeded") : QStringLiteral("failed"));
    it->insert(QStringLiteral("completedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    if (error.isEmpty()) it->insert(QStringLiteral("data"), scrubSensitive(data));
    else {
        it->insert(QStringLiteral("error"), error);
        it->insert(QStringLiteral("httpStatus"), httpStatus);
    }
}

void DashboardWebServer::acceptHttpConnections()
{
    while (QTcpSocket *socket = m_httpServer.nextPendingConnection()) {
        if (m_httpBuffers.size() >= kMaxHttpConnections) {
            socket->abort();
            socket->deleteLater();
            continue;
        }
        m_httpBuffers.insert(socket, {});
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() { readHttpRequest(socket); });
        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            m_httpBuffers.remove(socket);
            m_previewSockets.remove(socket);
            socket->deleteLater();
        });
        QTimer::singleShot(15000, socket, [this, socket]() {
            if (m_httpBuffers.contains(socket)
                && socket->state() != QAbstractSocket::UnconnectedState) {
                socket->disconnectFromHost();
            }
        });
    }
}

bool DashboardWebServer::sendPreviewStream(
    QTcpSocket *socket, int cameraIndex, const QString &quality,
    DashboardHttpProtocol::Response *errorResponse)
{
    if (!socket || !m_previewManager) {
        if (errorResponse) *errorResponse = jsonResponse(503, {}, tr("Preview service is unavailable"));
        return false;
    }

    const auto initial = m_previewManager->frame(cameraIndex, quality);
    if (initial.status == DashboardWebPreviewManager::FrameStatus::InvalidCamera) {
        if (errorResponse) *errorResponse = jsonResponse(404, {}, tr("Camera not found"));
        return false;
    }
    if (initial.status == DashboardWebPreviewManager::FrameStatus::MissingStream) {
        if (errorResponse) *errorResponse = jsonResponse(409, {}, tr("Camera has no configured stream"));
        return false;
    }

    static constexpr auto boundary = "openipcframe";
    QByteArray header = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
    header += "Content-Type: multipart/x-mixed-replace; boundary=";
    header += boundary;
    header += "\r\nCache-Control: no-store, max-age=0\r\n";
    header += "Pragma: no-cache\r\nX-Content-Type-Options: nosniff\r\n";
    header += "Cross-Origin-Resource-Policy: same-origin\r\n\r\n";
    socket->write(header);
    m_previewSockets.insert(socket);

    auto *timer = new QTimer(socket);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(quality.compare(QStringLiteral("hd"), Qt::CaseInsensitive) == 0 ? 100 : 80);
    connect(timer, &QTimer::timeout, socket, [this, socket, timer, cameraIndex, quality]() {
        if (!m_previewSockets.contains(socket)
            || socket->state() == QAbstractSocket::UnconnectedState) {
            timer->stop();
            return;
        }
        if (socket->bytesToWrite() > 768 * 1024) return;

        const auto frame = m_previewManager->frame(cameraIndex, quality);
        if (frame.status != DashboardWebPreviewManager::FrameStatus::Ready
            || frame.jpeg.isEmpty()) {
            return;
        }

        QByteArray part = "--openipcframe\r\nContent-Type: image/jpeg\r\nContent-Length: ";
        part += QByteArray::number(frame.jpeg.size());
        part += "\r\n\r\n";
        part += frame.jpeg;
        part += "\r\n";
        socket->write(part);
    });
    timer->start();
    return true;
}

void DashboardWebServer::readHttpRequest(QTcpSocket *socket)
{
    if (!socket || !m_httpBuffers.contains(socket)) return;
    QByteArray &buffer = m_httpBuffers[socket];
    buffer += socket->readAll();
    if (buffer.size() > kMaxHttpRequestBytes) {
        m_httpBuffers.remove(socket);
        sendResponse(socket, jsonResponse(413, {}, tr("Request is too large")));
        return;
    }
    const auto parsed = DashboardHttpProtocol::parseRequest(buffer);
    if (!parsed.complete) return;
    if (!parsed.valid) {
        DashboardHttpProtocol::Response response = jsonResponse(parsed.status, {}, parsed.error);
        sendResponse(socket, response);
        return;
    }
    m_httpBuffers.remove(socket);
    handleHttpRequest(socket, parsed.request);
}

void DashboardWebServer::handleHttpRequest(QTcpSocket *socket,
                                           const DashboardHttpProtocol::Request &request)
{
    bool streamingResponse = false;
    DashboardHttpProtocol::Response response;
    if (request.path.startsWith(QStringLiteral("/api/"))) {
        response = routeApi(request, socket->peerAddress().toString(), &streamingResponse, socket);
    } else {
        response = routeStatic(request);
    }
    if (!streamingResponse) sendResponse(socket, response, request.method == "HEAD");
}

void DashboardWebServer::sendResponse(QTcpSocket *socket,
                                      const DashboardHttpProtocol::Response &response,
                                      bool headOnly)
{
    if (!socket) return;
    socket->write(response.serialize(!headOnly));
    socket->disconnectFromHost();
}

void DashboardWebServer::scheduleBroadcast()
{
    if (!m_broadcastTimer.isActive()) m_broadcastTimer.start();
}

void DashboardWebServer::invalidateSessions()
{
    m_sessions.clear();
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    const QList<QWebSocket *> sockets = m_webSockets.values();
    for (QWebSocket *socket : sockets) {
        socket->close(QWebSocketProtocol::CloseCodePolicyViolated,
                      QStringLiteral("Session invalidated"));
    }
#endif
}

void DashboardWebServer::invalidateUserSessions(const QString &username)
{
    m_sessions.removeForUser(username);
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    const QList<QWebSocket *> sockets = m_webSockets.values();
    for (QWebSocket *socket : sockets) {
        if (m_webSocketSessions.value(socket).username == username) {
            socket->close(QWebSocketProtocol::CloseCodePolicyViolated,
                          QStringLiteral("Session invalidated"));
        }
    }
#endif
}

#ifdef OPENIPC_HAS_QT_WEBSOCKETS
void DashboardWebServer::startWebSocketServer(const QHostAddress &address)
{
    m_webSocketServer = new QWebSocketServer(QStringLiteral("OpenIPC Dashboard"),
                                             QWebSocketServer::NonSecureMode, this);
    if (!m_webSocketServer->listen(address, static_cast<quint16>(m_webSocketPort))) {
        qWarning() << "Dashboard WebSocket server disabled:" << m_webSocketServer->errorString();
        m_webSocketServer->deleteLater();
        m_webSocketServer = nullptr;
        return;
    }
    connect(m_webSocketServer, &QWebSocketServer::newConnection,
            this, &DashboardWebServer::acceptWebSocketConnections);
}

void DashboardWebServer::acceptWebSocketConnections()
{
    while (m_webSocketServer && m_webSocketServer->hasPendingConnections()) {
        QWebSocket *socket = m_webSocketServer->nextPendingConnection();
        const QByteArray cookieHeader = socket->request().rawHeader("Cookie");
        const QByteArray token = DashboardHttpProtocol::parseCookies(cookieHeader).value("openipc_session");
        const auto session = m_sessions.find(token);
        if (!session.isValid() || (session.permissions & UserManager::Perm_LiveView) == 0) {
            socket->close(QWebSocketProtocol::CloseCodePolicyViolated, QStringLiteral("Authentication required"));
            socket->deleteLater();
            continue;
        }
        m_webSockets.insert(socket);
        m_webSocketSessions.insert(socket, session);
        connect(socket, &QWebSocket::disconnected, this, [this, socket]() {
            const QStringList peerIds = m_webRtcPeerSockets.keys(socket);
            for (const QString &peerId : peerIds) {
                m_webRtcPeerSockets.remove(peerId);
                m_webRtcManager->stopPeer(peerId);
            }
            m_webSockets.remove(socket);
            m_webSocketSessions.remove(socket);
            socket->deleteLater();
            emit connectedClientsChanged();
        });
        connect(socket, &QWebSocket::textMessageReceived, this,
                [this, socket](const QString &message) {
            handleWebSocketMessage(socket, message);
        });
        emit connectedClientsChanged();
        const QJsonObject message{{QStringLiteral("type"), QStringLiteral("dashboard")},
                                  {QStringLiteral("data"), QJsonValue::fromVariant(dashboardData())}};
        socket->sendTextMessage(QString::fromUtf8(QJsonDocument(message).toJson(QJsonDocument::Compact)));
    }
}

void DashboardWebServer::handleWebSocketMessage(QWebSocket *socket,
                                                const QString &message)
{
    if (!socket || !m_webSockets.contains(socket)) return;
    if (message == QStringLiteral("ping")) {
        socket->sendTextMessage(QStringLiteral("pong"));
        return;
    }
    if (message.size() > 64 * 1024) return;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) return;
    const QJsonObject object = document.object();
    const QString type = object.value(QStringLiteral("type")).toString();
    const QString peerId = object.value(QStringLiteral("peerId")).toString();
    static const QRegularExpression peerIdPattern(QStringLiteral("^[A-Za-z0-9_-]{8,80}$"));

    const auto sendError = [socket, &peerId](const QString &error) {
        const QJsonObject payload{
            {QStringLiteral("type"), QStringLiteral("webrtc-error")},
            {QStringLiteral("peerId"), peerId},
            {QStringLiteral("error"), error}
        };
        socket->sendTextMessage(QString::fromUtf8(
            QJsonDocument(payload).toJson(QJsonDocument::Compact)));
    };
    if (!peerIdPattern.match(peerId).hasMatch()) {
        sendError(tr("Invalid WebRTC peer identifier"));
        return;
    }

    if (type == QStringLiteral("webrtc-start")) {
        if (!m_webRtcManager || !m_webRtcManager->available()) {
            sendError(m_webRtcManager ? m_webRtcManager->availabilityError()
                                      : tr("WebRTC is unavailable"));
            return;
        }
        if (m_webRtcPeerSockets.keys(socket).size() >= 9) {
            sendError(tr("Too many WebRTC streams"));
            return;
        }
        const int cameraIndex = object.value(QStringLiteral("cameraIndex")).toInt(-1);
        const QString quality = object.value(QStringLiteral("quality")).toString();
        m_webRtcPeerSockets.insert(peerId, socket);
        QString error;
        if (!m_webRtcManager->startPeer(peerId, cameraIndex, quality, &error)) {
            m_webRtcPeerSockets.remove(peerId);
            sendError(error);
        }
        return;
    }

    if (m_webRtcPeerSockets.value(peerId) != socket) {
        sendError(tr("WebRTC peer not found"));
        return;
    }
    if (type == QStringLiteral("webrtc-answer")) {
        QString error;
        if (!m_webRtcManager->setAnswer(
                peerId, object.value(QStringLiteral("sdp")).toString(), &error)) {
            sendError(error);
        }
    } else if (type == QStringLiteral("webrtc-ice")) {
        QString error;
        if (!m_webRtcManager->addIceCandidate(
                peerId, object.value(QStringLiteral("mlineIndex")).toInt(-1),
                object.value(QStringLiteral("candidate")).toString(), &error)) {
            sendError(error);
        }
    } else if (type == QStringLiteral("webrtc-stop")) {
        m_webRtcPeerSockets.remove(peerId);
        m_webRtcManager->stopPeer(peerId);
    }
}

void DashboardWebServer::broadcastLogTail(int first, int last)
{
    Q_UNUSED(first)
    if (m_webSockets.isEmpty() || last < 0) return;
    const QVariantMap data = logsData(0, qBound(1, last - first + 1, 50), {}, {});
    const QJsonObject message{{QStringLiteral("type"), QStringLiteral("logs")},
                              {QStringLiteral("data"), QJsonValue::fromVariant(data)}};
    const QString payload = QString::fromUtf8(QJsonDocument(message).toJson(QJsonDocument::Compact));
    for (QWebSocket *socket : std::as_const(m_webSockets)) {
        const auto session = m_webSocketSessions.value(socket);
        if ((session.permissions & UserManager::Perm_All) == UserManager::Perm_All
            || (session.permissions & UserManager::Perm_Settings) != 0) {
            socket->sendTextMessage(payload);
        }
    }
}
#endif

void DashboardWebServer::broadcastState()
{
#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    if (m_webSockets.isEmpty()) return;
    const QJsonObject message{{QStringLiteral("type"), QStringLiteral("dashboard")},
                              {QStringLiteral("data"), QJsonValue::fromVariant(dashboardData())}};
    const QString payload = QString::fromUtf8(QJsonDocument(message).toJson(QJsonDocument::Compact));
    for (QWebSocket *socket : std::as_const(m_webSockets)) socket->sendTextMessage(payload);
#endif
}
