#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class QNetworkReply;
class QNetworkRequest;
#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
class QWebSocket;
#endif

// HTTP integration for the OpenIPC firmware WebUI shipped together with Majestic.
//
// The camera-side WebUI is mostly haserl CGI plus a few JSON helpers.  This
// client mirrors those contracts so QML can use structured read/write
// operations without scraping pages in the UI layer.
class OpenIpcFirmwareClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool webSocketsAvailable READ webSocketsAvailable CONSTANT)

public:
    explicit OpenIpcFirmwareClient(QObject *parent = nullptr);
    ~OpenIpcFirmwareClient() override;

    bool webSocketsAvailable() const;

    Q_INVOKABLE QString loadStatus(const QString &host, int port,
                                   const QString &username, const QString &password);

    Q_INVOKABLE QString loadNetwork(const QString &host, int port,
                                    const QString &username, const QString &password);
    Q_INVOKABLE QString saveNetwork(const QString &host, int port,
                                    const QString &username, const QString &password,
                                    const QVariantMap &settings);
    Q_INVOKABLE QString resetNetwork(const QString &host, int port,
                                     const QString &username, const QString &password);
    Q_INVOKABLE QString changeMacAddress(const QString &host, int port,
                                         const QString &username, const QString &password,
                                         const QString &macAddress);
    Q_INVOKABLE QString scanWifi(const QString &host, int port,
                                 const QString &username, const QString &password);

    Q_INVOKABLE QString loadTime(const QString &host, int port,
                                 const QString &username, const QString &password);
    Q_INVOKABLE QString saveTime(const QString &host, int port,
                                 const QString &username, const QString &password,
                                 const QVariantMap &settings);
    Q_INVOKABLE QString syncTime(const QString &host, int port,
                                 const QString &username, const QString &password,
                                 bool setFromComputer = false);

    Q_INVOKABLE QString loadLogs(const QString &host, int port,
                                 const QString &username, const QString &password,
                                 const QString &source = QString(),
                                 int lines = 250);
    Q_INVOKABLE QString setLogBufferSize(const QString &host, int port,
                                         const QString &username, const QString &password,
                                         int sizeKiB);

    Q_INVOKABLE QString saveFirmwareBackup(const QString &host, int port,
                                           const QString &username, const QString &password,
                                           const QString &destinationPath);

    Q_INVOKABLE QString reboot(const QString &host, int port,
                               const QString &username, const QString &password);

    Q_INVOKABLE QString loadUpdateInfo(const QString &host, int port,
                                       const QString &username, const QString &password);
    Q_INVOKABLE QString uploadFirmwareArchive(const QString &host, int port,
                                              const QString &username, const QString &password,
                                              const QString &archivePath);
    Q_INVOKABLE QString startGithubUpdate(const QString &host, int port,
                                          const QString &username, const QString &password,
                                          bool kernel = true, bool rootfs = true,
                                          bool reset = false, bool force = false);
    Q_INVOKABLE QString startFirmwareUpgrade(const QString &host, int port,
                                             const QString &username, const QString &password,
                                             const QString &source,
                                             bool kernel = true, bool rootfs = true,
                                             bool reset = false, bool force = false);
    Q_INVOKABLE QString startLiveLogs(const QString &host, int port,
                                      const QString &username, const QString &password);
    Q_INVOKABLE void stopLiveLogs();

    static QVariantMap parseNetworkPageForTest(const QString &html);
    static QVariantMap parseTimePageForTest(const QString &html);
    static QVariantMap parseUpdatePageForTest(const QString &html);
    static QVariantMap parseStatusPageForTest(const QString &html);
    static QVariantMap parsePulseForTest(const QByteArray &json);
    static QVariantMap metricsFromTextForTest(const QString &text);

signals:
    void statusLoaded(const QString &requestId, const QVariantMap &status);
    void networkLoaded(const QString &requestId, const QVariantMap &network);
    void networkSaved(const QString &requestId, const QVariantMap &network);
    void networkReset(const QString &requestId);
    void macAddressChanged(const QString &requestId, const QString &macAddress);
    void wifiScanned(const QString &requestId, const QVariantList &networks, const QString &error);
    void timeLoaded(const QString &requestId, const QVariantMap &time);
    void timeSaved(const QString &requestId, const QVariantMap &time);
    void timeSynced(const QString &requestId, const QVariantMap &result);
    void logsLoaded(const QString &requestId, const QString &source, const QString &text);
    void logBufferSizeChanged(const QString &requestId, int sizeKiB);
    void backupSaved(const QString &requestId, const QString &path);
    void rebootStarted(const QString &requestId);
    void updateInfoLoaded(const QString &requestId, const QVariantMap &info);
    void firmwareUploaded(const QString &requestId, const QString &remotePath);
    void updateStarted(const QString &requestId, const QString &mode);
    void firmwareUpgradeOutput(const QString &requestId, const QString &text);
    void firmwareUpgradeRebooting(const QString &requestId);
    void liveLogsStarted(const QString &requestId);
    void liveLogChunk(const QString &requestId, const QString &text);
    void liveLogsStopped(const QString &requestId, const QString &reason);

    void operationSucceeded(const QString &requestId, const QString &operation,
                            const QString &result);
    void operationFailed(const QString &requestId, const QString &operation,
                         const QString &message, int httpStatus);

#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
private slots:
    void onUpgradeSocketError();
    void onLiveLogsSocketError();
#endif

private:
    QNetworkRequest makeRequest(const QString &host, int port, const QString &path,
                                const QString &username, const QString &password) const;
    QNetworkRequest makeRequest(const QString &host, int port, const QUrl &relativeUrl,
                                const QString &username, const QString &password) const;
    QNetworkRequest makeWebSocketRequest(const QString &host, int port, const QString &path,
                                         const QString &username, const QString &password) const;

    void getText(const QString &requestId, const QString &operation,
                 const QString &host, int port, const QString &username, const QString &password,
                 const QString &path,
                 const std::function<void(const QByteArray &, QNetworkReply *)> &onSuccess);
    void postForm(const QString &requestId, const QString &operation,
                  const QString &host, int port, const QString &username, const QString &password,
                  const QString &path, const QVariantMap &fields,
                  const std::function<void(const QByteArray &, QNetworkReply *)> &onSuccess = {});
    void handleSimpleReply(QNetworkReply *reply, const QString &requestId,
                           const QString &operation,
                           const std::function<void(const QByteArray &, QNetworkReply *)> &onSuccess = {});
    void emitFailure(const QString &requestId, const QString &operation,
                     QNetworkReply *reply, const QByteArray &body);
    void emitFailureLater(const QString &requestId, const QString &operation,
                          const QString &message, int httpStatus = 0);

    static QString newRequestId();
    static QVariantMap parseJsonObject(const QByteArray &json, QString *error = nullptr);
    static QVariantList parseJsonArray(const QByteArray &json, QString *error = nullptr);

    QNetworkAccessManager m_networkManager;
#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
    QWebSocket *m_upgradeSocket = nullptr;
    QWebSocket *m_liveLogsSocket = nullptr;
    QString m_upgradeRequestId;
    QString m_liveLogsRequestId;
    bool m_upgradeSocketOpened = false;
    bool m_liveLogsSocketOpened = false;
#endif
};
