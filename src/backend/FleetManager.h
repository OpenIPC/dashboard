#pragma once

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class CameraHealthController;
class CameraModel;
class LogModel;
class MajesticClient;
class OpenIpcFirmwareClient;
class UserManager;

class FleetManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList sites READ sites NOTIFY topologyChanged)
    Q_PROPERTY(QVariantList areas READ areas NOTIFY topologyChanged)
    Q_PROPERTY(QVariantList savedViews READ savedViews NOTIFY savedViewsChanged)
    Q_PROPERTY(QVariantList baselines READ baselines NOTIFY baselinesChanged)
    Q_PROPERTY(QVariantList inventory READ inventory NOTIFY inventoryChanged)
    Q_PROPERTY(QVariantMap batchState READ batchState NOTIFY batchStateChanged)
    Q_PROPERTY(QVariantList batchResults READ batchResults NOTIFY batchResultsChanged)
    Q_PROPERTY(QVariantList batchHistory READ batchHistory NOTIFY batchHistoryChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit FleetManager(CameraModel *cameraModel,
                          CameraHealthController *healthController,
                          OpenIpcFirmwareClient *firmwareClient,
                          MajesticClient *majesticClient,
                          UserManager *userManager,
                          LogModel *logModel,
                          QObject *parent = nullptr);

    QVariantList sites() const;
    QVariantList areas() const;
    QVariantList savedViews() const;
    QVariantList baselines() const;
    QVariantList inventory() const;
    QVariantMap batchState() const { return m_batchState; }
    QVariantList batchResults() const { return m_batchResults; }
    QVariantList batchHistory() const { return m_batchHistory; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE QString createSite(const QString &name,
                                   const QString &description = QString(),
                                   const QStringList &tags = {});
    Q_INVOKABLE bool updateSite(const QString &siteId, const QVariantMap &changes);
    Q_INVOKABLE bool removeSite(const QString &siteId);
    Q_INVOKABLE QString createArea(const QString &siteId, const QString &name,
                                   const QStringList &tags = {});
    Q_INVOKABLE bool updateArea(const QString &areaId, const QVariantMap &changes);
    Q_INVOKABLE bool removeArea(const QString &areaId);
    Q_INVOKABLE bool setSiteMaintenanceWindow(const QString &siteId,
                                              const QVariantMap &window);
    Q_INVOKABLE bool setSiteVersionBaseline(const QString &siteId,
                                            const QString &firmwareVersion,
                                            const QString &majesticVersion);

    Q_INVOKABLE bool assignCamera(const QString &cameraId,
                                  const QString &siteId,
                                  const QString &areaId,
                                  const QStringList &tags = {},
                                  const QString &maintenanceState = QStringLiteral("active"));
    Q_INVOKABLE QVariantMap cameraAssignment(const QString &cameraId) const;
    Q_INVOKABLE QStringList scopeAliases(const QString &cameraId,
                                         const QString &cameraIp = QString(),
                                         int cameraIndex = -1) const;

    Q_INVOKABLE QVariantList filterInventory(const QVariantMap &filters = {}) const;
    Q_INVOKABLE void refreshInventory();
    Q_INVOKABLE QString createSavedView(const QString &name, const QVariantMap &filters);
    Q_INVOKABLE bool updateSavedView(const QString &viewId, const QString &name,
                                     const QVariantMap &filters);
    Q_INVOKABLE bool removeSavedView(const QString &viewId);
    Q_INVOKABLE bool exportInventory(const QString &pathOrUrl,
                                     const QVariantMap &filters = {});
    Q_INVOKABLE bool exportDiagnostics(const QString &pathOrUrl,
                                       const QVariantList &cameraIds = {});

    Q_INVOKABLE QString createBaseline(const QString &name,
                                       const QString &siteId,
                                       const QString &areaId,
                                       const QVariantMap &configuration);
    Q_INVOKABLE QString captureBaselineFromCamera(const QString &cameraId,
                                                  const QString &name);
    Q_INVOKABLE bool removeBaseline(const QString &baselineId);
    Q_INVOKABLE QVariantMap driftPreview(const QString &baselineId,
                                         const QVariantList &cameraIds = {}) const;
    Q_INVOKABLE void setDeviceConfigurationSnapshot(const QString &cameraId,
                                                    const QVariantMap &configuration,
                                                    const QVariantMap &schema = {});

    Q_INVOKABLE QVariantMap preflightBatch(const QString &operation,
                                           const QVariantList &cameraIds,
                                           const QVariantMap &options = {}) const;
    Q_INVOKABLE bool startBatch(const QString &operation,
                                const QVariantList &cameraIds,
                                const QVariantMap &options = {});
    Q_INVOKABLE void cancelBatch();

    Q_INVOKABLE QVariantMap previewSiteImport(const QString &pathOrUrl) const;
    Q_INVOKABLE bool importSiteDefinitions(const QString &pathOrUrl, bool merge = false);
    Q_INVOKABLE bool exportSiteDefinitions(const QString &pathOrUrl) const;

    QJsonObject toJson() const;
    void restoreJson(const QJsonObject &object);

signals:
    void topologyChanged();
    void assignmentsChanged();
    void savedViewsChanged();
    void baselinesChanged();
    void inventoryChanged();
    void batchStateChanged();
    void batchResultsChanged();
    void batchHistoryChanged();
    void lastErrorChanged();
    void stateChanged();
    void operationMessage(const QString &message);
    void auditEvent(const QVariantMap &event);

private:
    struct PendingRequest {
        QString cameraId;
        QString stage;
        QString name;
        QVariantMap configuration;
        QVariantMap schema;
        QVariantMap patch;
        QString backupPath;
    };

    bool canManageTopology() const;
    bool canConfigureCamera(const QString &cameraId) const;
    int cameraIndexForKey(const QString &cameraId) const;
    QString cameraKeyAt(int index) const;
    QVariantMap siteById(const QString &siteId) const;
    QVariantMap areaById(const QString &areaId) const;
    QVariantMap baselineById(const QString &baselineId) const;
    bool isWithinMaintenanceWindow(const QString &siteId) const;
    void syncLastSeen();
    void setLastError(const QString &message) const;
    void touchState(bool topology = false);
    void audit(const QString &action, const QString &target,
               const QString &outcome, const QString &detail = QString()) const;

    static QString normalizedId(const QString &value);
    static QString newId();
    static QString localPath(const QString &pathOrUrl);
    static QStringList normalizedTags(const QStringList &tags);
    static QVariantMap sanitizedConfiguration(const QVariantMap &configuration);
    static QVariantMap configurationPatch(const QVariantMap &current,
                                          const QVariantMap &expected);
    static QVariantList configurationDifferences(const QVariantMap &current,
                                                 const QVariantMap &expected);
    static QString safeFilePart(const QString &value);
    static QString firstVersionValue(const QVariantMap &map);

    QStringList normalizedCameraIds(const QVariantList &cameraIds) const;
    QVariantMap devicePreflight(const QString &operation, const QString &cameraId,
                                const QVariantMap &options) const;
    void dispatchBatch();
    void completeBatchDevice(const QString &cameraId, const QString &status,
                             const QString &message,
                             const QVariantMap &details = {});
    void finishBatch(const QString &outcome);
    void handleConfigurationLoaded(const QString &requestId,
                                   const QVariantMap &configuration,
                                   const QVariantMap &schema,
                                   const QVariantMap &capabilities);
    void handleRequestFailure(const QString &requestId, const QString &message,
                              int httpStatus);

    CameraModel *m_cameraModel = nullptr;
    CameraHealthController *m_healthController = nullptr;
    OpenIpcFirmwareClient *m_firmwareClient = nullptr;
    MajesticClient *m_majesticClient = nullptr;
    UserManager *m_userManager = nullptr;
    LogModel *m_logModel = nullptr;

    QVariantList m_sites;
    QVariantList m_areas;
    QVariantList m_savedViews;
    QVariantList m_baselines;
    QHash<QString, QVariantMap> m_assignments;
    QHash<QString, QVariantMap> m_deviceMetadata;
    QHash<QString, QVariantMap> m_deviceConfigurations;
    QHash<QString, QVariantMap> m_deviceSchemas;

    QVariantMap m_batchState;
    QVariantList m_batchResults;
    QVariantList m_batchHistory;
    QStringList m_batchQueue;
    QSet<QString> m_batchInFlight;
    QHash<QString, PendingRequest> m_pendingRequests;
    QString m_activeHealthCameraId;
    mutable QString m_lastError;
};
