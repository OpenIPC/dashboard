#pragma once

#include "DashboardHttpProtocol.h"
#include "DashboardWebSessionStore.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QTcpServer>
#include <QTimer>
#include <QVariantList>

#ifdef OPENIPC_HAS_QT_WEBSOCKETS
#include <QWebSocketServer>
class QWebSocket;
#endif

class QTcpSocket;
class DashboardWebPreviewManager;
class DashboardWebRecordingManager;
class DashboardWebRtcManager;
class SystemController;

class DashboardWebServer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY stateChanged)
    Q_PROPERTY(QString bindAddress READ bindAddress NOTIFY stateChanged)
    Q_PROPERTY(int port READ port NOTIFY stateChanged)
    Q_PROPERTY(int webSocketPort READ webSocketPort NOTIFY stateChanged)
    Q_PROPERTY(QString url READ url NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY stateChanged)
    Q_PROPERTY(int activeSessions READ activeSessions NOTIFY activeSessionsChanged)
    Q_PROPERTY(int connectedClients READ connectedClients NOTIFY connectedClientsChanged)
    Q_PROPERTY(bool webSocketsAvailable READ webSocketsAvailable CONSTANT)
    Q_PROPERTY(QVariantList accessUrls READ accessUrls NOTIFY stateChanged)

public:
    explicit DashboardWebServer(SystemController *systemController, QObject *parent = nullptr);
    ~DashboardWebServer() override;

    bool running() const { return m_httpServer.isListening(); }
    QString bindAddress() const { return m_effectiveBindAddress; }
    int port() const { return m_httpServer.isListening() ? m_httpServer.serverPort() : m_port; }
    int webSocketPort() const { return m_webSocketPort; }
    QString url() const;
    QString lastError() const { return m_lastError; }
    int activeSessions() const { return m_sessions.count(); }
    int connectedClients() const;
    bool webSocketsAvailable() const;
    QVariantList accessUrls() const;

    Q_INVOKABLE bool start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE bool restart();
    Q_INVOKABLE void openInBrowser();
    Q_INVOKABLE QVariantMap status() const;
    void applySettings(const QVariantMap &settings);

signals:
    void stateChanged();
    void activeSessionsChanged();
    void connectedClientsChanged();

private:
    struct LoginWindow {
        QList<qint64> attempts;
        qint64 blockedUntilMs = 0;
    };

    void acceptHttpConnections();
    void readHttpRequest(QTcpSocket *socket);
    void handleHttpRequest(QTcpSocket *socket, const DashboardHttpProtocol::Request &request);
    DashboardHttpProtocol::Response routeApi(const DashboardHttpProtocol::Request &request,
                                             const QString &peerAddress,
                                             bool *streamingResponse,
                                             QTcpSocket *socket);
    DashboardHttpProtocol::Response routeStatic(const DashboardHttpProtocol::Request &request) const;
    DashboardHttpProtocol::Response jsonResponse(int status, const QVariant &data,
                                                 const QString &error = QString()) const;
    DashboardHttpProtocol::Response emptyResponse(int status) const;
    DashboardWebSessionStore::Session authenticate(const DashboardHttpProtocol::Request &request,
                                                   QByteArray *rawToken = nullptr,
                                                   bool touch = true);
    bool requirePermission(const DashboardWebSessionStore::Session &session, int permission,
                           DashboardHttpProtocol::Response *response) const;
    bool validateMutationRequest(const DashboardHttpProtocol::Request &request,
                                 DashboardHttpProtocol::Response *response) const;
    bool mutationAllowed(const QByteArray &rawToken, const QString &peerAddress,
                         qint64 nowMs);
    void audit(const DashboardWebSessionStore::Session &session, const QString &action,
               const QString &target, const QString &outcome = QStringLiteral("succeeded"));
    bool loginAllowed(const QString &peerAddress, qint64 nowMs);
    void recordLoginFailure(const QString &peerAddress, qint64 nowMs);
    void addSecurityHeaders(DashboardHttpProtocol::Response *response, bool api) const;
    void sendResponse(QTcpSocket *socket, const DashboardHttpProtocol::Response &response,
                      bool headOnly = false);
    bool sendArchiveFile(QTcpSocket *socket, const DashboardHttpProtocol::Request &request,
                         const QString &fileId, DashboardHttpProtocol::Response *errorResponse);
    bool sendPreviewStream(QTcpSocket *socket, int cameraIndex, const QString &quality,
                           DashboardHttpProtocol::Response *errorResponse);
    QString archiveFileForId(const QString &fileId) const;
    QVariantList archiveItems(const QString &cameraId, int limit) const;
    QVariantMap dashboardData() const;
    QVariantList cameraData() const;
    QVariantMap discoveryData() const;
    QVariantMap healthData() const;
    QVariantMap analyticsData() const;
    QVariantMap logsData(int cursor, int limit, const QString &level,
                         const QString &search) const;
    QVariantMap diagnosticsData();
    QString beginDeviceOperation(int cameraIndex, const QString &operation,
                                 QString *error);
    QVariantMap deviceOperation(const QString &requestId) const;
    void completeDeviceOperation(const QString &requestId, const QVariant &data,
                                 const QString &error = QString(), int httpStatus = 0);
    QVariant scrubSensitive(const QVariant &value) const;
    void invalidateSessions();
    void invalidateUserSessions(const QString &username);
    void scheduleBroadcast();
    void broadcastState();

#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    void startWebSocketServer(const QHostAddress &address);
    void acceptWebSocketConnections();
    void handleWebSocketMessage(QWebSocket *socket, const QString &message);
    void broadcastLogTail(int first, int last);
#endif

    SystemController *m_systemController = nullptr;
    DashboardWebPreviewManager *m_previewManager = nullptr;
    DashboardWebRecordingManager *m_recordingManager = nullptr;
    DashboardWebRtcManager *m_webRtcManager = nullptr;
    QTcpServer m_httpServer;
    QHash<QTcpSocket *, QByteArray> m_httpBuffers;
    QSet<QTcpSocket *> m_previewSockets;
    DashboardWebSessionStore m_sessions;
    QHash<QString, LoginWindow> m_loginWindows;
    QHash<QByteArray, QList<qint64>> m_mutationWindows;
    QHash<QString, QVariantMap> m_deviceOperations;
    QHash<QString, QVariantMap> m_devicePrivateData;
    QHash<QString, QVariantMap> m_deviceChangePreviews;
    QHash<QString, QString> m_idempotentDeviceOperations;
    QTimer m_sessionCleanupTimer;
    QTimer m_broadcastTimer;
    QString m_configuredBindAddress = QStringLiteral("127.0.0.1");
    QString m_effectiveBindAddress = QStringLiteral("127.0.0.1");
    QString m_lastError;
    int m_port = 8080;
    int m_webSocketPort = 8081;
    int m_sessionTimeoutMinutes = 60;
    bool m_enabled = false;
    bool m_allowRemote = false;
    bool m_secureCookies = false;

#ifdef OPENIPC_HAS_QT_WEBSOCKETS
    QWebSocketServer *m_webSocketServer = nullptr;
    QSet<QWebSocket *> m_webSockets;
    QHash<QWebSocket *, DashboardWebSessionStore::Session> m_webSocketSessions;
    QHash<QString, QPointer<QWebSocket>> m_webRtcPeerSockets;
#endif
};
