#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class QNetworkReply;

// HTTP integration for the Majestic media server shipped with OpenIPC.
//
// Majestic exposes its effective configuration and a JSON schema at runtime.
// Keeping the client schema-driven is important: available settings vary by
// SoC, sensor and firmware build, and a hard-coded form silently loses fields.
class MajesticClient : public QObject
{
    Q_OBJECT

public:
    explicit MajesticClient(QObject *parent = nullptr);

    Q_INVOKABLE QString loadConfiguration(const QString &host, int port,
                                          const QString &username, const QString &password);
    Q_INVOKABLE QString applyConfiguration(const QString &host, int port,
                                           const QString &username, const QString &password,
                                           const QVariantMap &patch);
    Q_INVOKABLE QString resetConfigurationFields(const QString &host, int port,
                                                 const QString &username, const QString &password,
                                                 const QStringList &fieldPaths);
    Q_INVOKABLE QString applyLiveImage(const QString &host, int port,
                                      const QString &username, const QString &password,
                                      const QVariantMap &values);
    Q_INVOKABLE QString reloadPipeline(const QString &host, int port,
                                       const QString &username, const QString &password);
    Q_INVOKABLE QString loadMetrics(const QString &host, int port,
                                    const QString &username, const QString &password);

    Q_INVOKABLE QString takeSnapshot(const QString &host, int port, const QString &username,
                                     const QString &password, const QString &destinationPath,
                                     int width = 0, int height = 0, int quality = 0,
                                     bool grayscale = false);
    Q_INVOKABLE QString setNightMode(const QString &host, int port, const QString &username,
                                     const QString &password, const QString &mode);
    Q_INVOKABLE QString playPcmFile(const QString &host, int port, const QString &username,
                                    const QString &password, const QString &filePath);
    QString playPcmData(const QString &host, int port, const QString &username,
                        const QString &password, const QByteArray &pcmData);

    Q_INVOKABLE QString saveConfigurationBackup(const QVariantMap &config,
                                                const QVariantMap &schema,
                                                const QString &destinationPath);
    Q_INVOKABLE QString loadConfigurationBackup(const QString &sourcePath);

    Q_INVOKABLE QVariantMap buildPatch(const QVariantMap &original,
                                       const QVariantMap &edited) const;
    Q_INVOKABLE QVariantList describeChanges(const QVariantMap &original,
                                             const QVariantMap &edited) const;
    Q_INVOKABLE QVariantList flattenFields(const QVariantMap &schema,
                                           const QVariantMap &config) const;
    Q_INVOKABLE QVariantMap parseJsonObject(const QString &json) const;

    static QVariantMap buildPatchForTest(const QVariantMap &original,
                                         const QVariantMap &edited);
    static QVariantList describeChangesForTest(const QVariantMap &original,
                                               const QVariantMap &edited);
    static QVariantList flattenFieldsForTest(const QVariantMap &schema,
                                             const QVariantMap &config);

signals:
    void configurationLoaded(const QString &requestId, const QVariantMap &config,
                             const QVariantMap &schema, const QVariantList &fields,
                             const QVariantMap &capabilities);
    void configurationApplied(const QString &requestId);
    void configurationFieldsReset(const QString &requestId, const QStringList &fieldPaths);
    void metricsLoaded(const QString &requestId, const QVariantMap &metrics, const QString &rawText);
    void backupSaved(const QString &requestId, const QString &path);
    void backupLoaded(const QString &requestId, const QVariantMap &config,
                      const QVariantMap &schema, const QString &path);

    void operationSucceeded(const QString &requestId, const QString &operation,
                            const QString &result);
    void operationFailed(const QString &requestId, const QString &operation,
                         const QString &message, int httpStatus);

    // Compatibility signals used by the first Majestic integration.
    void requestSucceeded(const QString &operation, const QString &result);
    void requestFailed(const QString &operation, const QString &message);
    void snapshotSaved(const QString &path);

private:
    QNetworkRequest makeRequest(const QString &host, int port, const QString &path,
                                const QString &username, const QString &password) const;
    QNetworkRequest makeRequest(const QString &host, int port, const QUrl &relativeUrl,
                                const QString &username, const QString &password) const;
    void handleSimpleReply(QNetworkReply *reply, const QString &requestId,
                           const QString &operation,
                           const std::function<void(const QByteArray &)> &onSuccess = {});
    void emitFailure(const QString &requestId, const QString &operation,
                     QNetworkReply *reply, const QByteArray &body);
    void emitFailureLater(const QString &requestId, const QString &operation,
                          const QString &message, int httpStatus = 0);

    static QString newRequestId();
    static bool parseJsonMap(const QByteArray &data, QVariantMap *result, QString *error);
    static QVariantMap capabilities(const QVariantMap &schema, const QVariantMap &config);
    static QVariantMap metricsFromText(const QString &text);
    static bool validatePatch(const QVariantMap &patch, const QVariantMap &schema,
                              QString *error);

    QNetworkAccessManager m_networkManager;
    QHash<QString, QVariantMap> m_schemaByEndpoint;
};
