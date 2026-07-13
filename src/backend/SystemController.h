#ifndef SYSTEMCONTROLLER_H
#define SYSTEMCONTROLLER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QDebug>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QHash>
#include <QSet>
#include "CameraModel.h"
#include "analytics/AnalyticsEngine.h"
#include "UserManager.h"
#include "LogModel.h"
#include "PtzController.h"
#include "DiscoveryController.h"
#include "ArchiveController.h"
#include "CamexController.h"
#include "MajesticClient.h"
#include "NetworkDiscoveryService.h"
#include "OpenIpcFirmwareClient.h"
#include "AppUpdateChecker.h"
#include "CameraHealthController.h"

class StatusChecker;

class SystemController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serviceStatus READ serviceStatus NOTIFY serviceStatusChanged)
    Q_PROPERTY(CameraModel* cameraModel READ cameraModel CONSTANT)
    Q_PROPERTY(CameraModel* discoveryModel READ discoveryModel CONSTANT)
    Q_PROPERTY(DiscoveryController* dahuaDiscovery READ dahuaDiscovery CONSTANT)
    Q_PROPERTY(ArchiveController* archiveController READ archiveController CONSTANT)
    Q_PROPERTY(CameraModel* gridModel READ gridModel CONSTANT)
    Q_PROPERTY(QStringList cameraGroups READ cameraGroups NOTIFY cameraGroupsChanged)
    Q_PROPERTY(AnalyticsEngine* analyticsEngine READ analyticsEngine CONSTANT)
    Q_PROPERTY(UserManager* userManager READ userManager CONSTANT)
    Q_PROPERTY(LogModel* logModel READ logModel CONSTANT)
    Q_PROPERTY(PtzController* ptzController READ ptzController CONSTANT)
    Q_PROPERTY(CamexController* camexController READ camexController CONSTANT)
    Q_PROPERTY(MajesticClient* majesticClient READ majesticClient CONSTANT)
    Q_PROPERTY(OpenIpcFirmwareClient* firmwareClient READ firmwareClient CONSTANT)
    Q_PROPERTY(NetworkDiscoveryService* networkDiscovery READ networkDiscovery CONSTANT)
    Q_PROPERTY(AppUpdateChecker* appUpdateChecker READ appUpdateChecker CONSTANT)
    Q_PROPERTY(CameraHealthController* cameraHealthController READ cameraHealthController CONSTANT)
    Q_PROPERTY(QString discoveryLastUpdated READ discoveryLastUpdated NOTIFY discoverySessionChanged)
    Q_PROPERTY(QVariantMap appSettings READ getAppSettings WRITE saveAppSettings NOTIFY appSettingsChanged)
    Q_PROPERTY(int gridRows READ gridRows WRITE setGridRows NOTIFY gridLayoutChanged)
    Q_PROPERTY(int gridCols READ gridCols WRITE setGridCols NOTIFY gridLayoutChanged)
    Q_PROPERTY(QVariantList layoutTemplates READ layoutTemplates WRITE setLayoutTemplates NOTIFY layoutTemplatesChanged)
    Q_PROPERTY(bool isArchiveOpen READ isArchiveOpen WRITE setIsArchiveOpen NOTIFY isArchiveOpenChanged)

public:
    explicit SystemController(QObject *parent = nullptr);
    Q_INVOKABLE double processCpuPercent();
    Q_INVOKABLE double processMemoryMB();
    Q_INVOKABLE void openFolder(const QString &path);
    Q_INVOKABLE QString normalizeLocalPath(const QString &pathOrUrl) const;
    
    QString serviceStatus() const;
    CameraModel* cameraModel() const;
    CameraModel* discoveryModel() const;
    DiscoveryController* dahuaDiscovery() const;
    ArchiveController* archiveController() const;
    CameraModel* gridModel() const;
    int gridRows() const { return m_gridRows; }
    int gridCols() const { return m_gridCols; }
    bool isArchiveOpen() const { return m_isArchiveOpen; }
    void setIsArchiveOpen(bool open);

    QVariantList layoutTemplates() const { return m_layoutTemplates; }
    void setLayoutTemplates(const QVariantList &templates);
    QStringList cameraGroups() const { return m_cameraGroups; }
    AnalyticsEngine* analyticsEngine() const;
    UserManager* userManager() const;
    LogModel* logModel() const;
    PtzController* ptzController() const;
    CamexController* camexController() const;
    MajesticClient* majesticClient() const { return m_majesticClient; }
    OpenIpcFirmwareClient* firmwareClient() const { return m_firmwareClient; }
    NetworkDiscoveryService* networkDiscovery() const { return m_networkDiscovery; }
    AppUpdateChecker* appUpdateChecker() const { return m_appUpdateChecker; }
    CameraHealthController* cameraHealthController() const { return m_cameraHealthController; }
    QString discoveryLastUpdated() const { return m_discoveryLastUpdated; }
    Q_INVOKABLE QVariantMap parseCameraQrPayload(const QString &payload) const;
    Q_INVOKABLE QString xmSofiaPasswordHash(const QString &password) const;
    Q_INVOKABLE QString probeCameraEndpoint(const QString &kind,
                                            const QString &host,
                                            int port,
                                            const QString &path = QString(),
                                            const QString &username = QString(),
                                            const QString &password = QString());

    void addLog(QtMsgType type, const QString &msg);

public slots:
    void startService();
    void stopService();
    void scanNetwork(const QString &interfaceName = "", bool deepScan = false);
    Q_INVOKABLE void stopNetworkScan();
    Q_INVOKABLE void clearDiscoveryResults();
    Q_INVOKABLE void refreshDiscoveryAddedFlags();
    Q_INVOKABLE QString discoverySessionSummary() const;
    Q_INVOKABLE void validateDiscoverySelection(const QVariantList &indexes,
                                                const QString &login,
                                                const QString &password,
                                                const QString &profile);
    Q_INVOKABLE int addDiscoveredCameras(const QVariantList &indexes,
                                         const QString &login,
                                         const QString &password,
                                         const QString &profile);
    void addDevice(int index); // Adds from discovery to device list
    Q_INVOKABLE void addManualCamera(const QString &name, const QString &ip, const QString &url, int port, int onvifPort, const QString &login = "", const QString &password = "", const QString &sdUrl = "");
    Q_INVOKABLE void updateCamera(int index, const QString &name, const QString &ip, const QString &url, int port, int onvifPort, const QString &login = "", const QString &password = "", const QString &sdUrl = "");
    Q_INVOKABLE void updateCameraStatus(const QString &cameraIp, const QString &status);
    Q_INVOKABLE void updateCameraStreamStatus(const QString &cameraIp, const QString &status);
    Q_INVOKABLE void updateCameraStatusDetail(const QString &cameraIp, const QString &detail);
    Q_INVOKABLE QString cameraStatusDetail(const QString &cameraIp) const;
    Q_INVOKABLE QString effectiveCameraStatus(const QString &cameraIp, const QString &fallbackStatus = QString()) const;
    Q_INVOKABLE bool isCameraOnline(const QString &cameraIp, const QString &fallbackStatus = QString()) const;
    Q_INVOKABLE QString cameraAttentionReason(const QString &cameraIp, const QString &fallbackStatus = QString()) const;
    Q_INVOKABLE QString cameraStatusSearchText(const QString &cameraIp, const QString &fallbackStatus = QString()) const;
    Q_INVOKABLE bool cameraNeedsAttention(const QString &cameraIp, const QString &fallbackStatus = QString()) const;
    Q_INVOKABLE bool isCameraInGrid(const QString &cameraIp) const;
    Q_INVOKABLE int onlineCameraCount() const;
    Q_INVOKABLE int camerasNeedingAttentionCount() const;
    Q_INVOKABLE void refreshCameraHealth(const QString &cameraIp);
    void removeDevice(int index); // Removes from device list
    void addCameraToGrid(int index, int slot = -1); // Adds from device list to grid
    void removeCameraFromGrid(int index); // Clears a grid slot but keeps grid size
    Q_INVOKABLE void setGridCellSpan(int index, int rows, int cols);
    QVariantList getNetworkInterfaces();

    // Ensure grid model has exactly 'size' slots
    Q_INVOKABLE void updateGridSize(int size);
    Q_INVOKABLE int gridCapacity() const;

    Q_INVOKABLE void saveAppSettings(const QVariantMap &settings);
    Q_INVOKABLE QVariantMap getAppSettings() const;

    Q_INVOKABLE bool exportConfiguration(const QString &path);
    Q_INVOKABLE bool importConfiguration(const QString &path);

    // Camera groups
    Q_INVOKABLE void addCameraGroup(const QString &groupName);
    Q_INVOKABLE void setCameraGroup(int cameraIndex, const QString &groupName);
    Q_INVOKABLE void removeCameraGroup(const QString &groupName);
    Q_INVOKABLE void renameCameraGroup(const QString &oldName, const QString &newName);

    // Archive
    Q_INVOKABLE QVariantList getRecordings(const QString &cameraIp, const QDate &date);
    Q_INVOKABLE QList<int> getRecordingDates(const QString &cameraIp, int year, int month);
    Q_INVOKABLE void exportRecording(const QString &inputFile, const QString &outputFile, int startMs, int endMs);
    
    Q_INVOKABLE QString generateRecordingPath(const QString &ip);

    // Recording
    Q_INVOKABLE void toggleRecording(int gridIndex);
    Q_INVOKABLE void notifyRecordingStarted(const QString &cameraIp, const QString &path, const QString &source);
    Q_INVOKABLE void notifyRecordingStopped(const QString &cameraIp, const QString &path, const QString &source);
    Q_INVOKABLE void notifyRecordingSegment(const QString &cameraIp, const QString &oldPath, const QString &newPath);
    Q_INVOKABLE void notifyRecordingError(const QString &cameraIp, const QString &path, const QString &message);
    Q_INVOKABLE void applyLayoutPreset(int rows, int cols);
    Q_INVOKABLE void applyLayoutTemplate(const QVariantMap &layout);
    Q_INVOKABLE QString getCameraPassword(const QString &cameraIp) const;
    Q_INVOKABLE QString authenticatedStreamUrl(const QString &url, const QString &cameraIp) const;
    Q_INVOKABLE QString preferredPreviewStreamUrl(const QString &streamUrl,
                                                  const QString &sdStreamUrl,
                                                  const QString &hdStreamUrl,
                                                  const QString &preferredStream,
                                                  int gridRows,
                                                  int gridCols,
                                                  int spanRows,
                                                  int spanCols,
                                                  bool forceMain) const;
    Q_INVOKABLE QString preferredPreviewStreamQuality(const QString &preferredStream,
                                                      int gridRows,
                                                      int gridCols,
                                                      int spanRows,
                                                      int spanCols,
                                                      bool forceMain) const;
    Q_INVOKABLE QString manualStreamUrl(const QString &streamUrl,
                                        const QString &sdStreamUrl,
                                        const QString &hdStreamUrl,
                                        bool preferMain) const;
    Q_INVOKABLE bool isStreamFrameStalled(bool running,
                                          bool hasFrame,
                                          double nowMs,
                                          double startedMs,
                                          double lastFrameMs,
                                          int startupGraceMs,
                                          int frameStallMs) const;
    Q_INVOKABLE int streamPreviewPriorityScore(int gridIndex,
                                               int spanRows,
                                               int spanCols,
                                               bool selected,
                                               bool recordingActive,
                                               bool analyticsActive,
                                               bool online) const;
    Q_INVOKABLE bool shouldRunPreviewStream(bool smartBudgetEnabled,
                                            int maxPreviewStreams,
                                            int previewBudgetRank,
                                            bool hasCamera,
                                            bool canLive,
                                            bool fullscreenActive,
                                            bool archiveOpen,
                                            bool recordingActive,
                                            bool analyticsActive) const;
    Q_INVOKABLE QString previewPauseReasonCode(bool smartBudgetEnabled,
                                               int maxPreviewStreams,
                                               int previewBudgetRank,
                                               bool hasCamera,
                                               bool canLive,
                                               bool fullscreenActive,
                                               bool archiveOpen,
                                               bool recordingActive,
                                               bool analyticsActive) const;
    
    // Dahua SDK
    Q_INVOKABLE void takeDahuaSnapshot(const QString &ip, int port, const QString &login, const QString &password);
    Q_INVOKABLE void notifySnapshotSaved(const QString &path);
    Q_INVOKABLE QString getSnapshotPath(const QString &filename);
    Q_INVOKABLE bool deleteLocalFile(const QString &fileUrl);
    Q_INVOKABLE bool localFileExists(const QString &fileUrl) const;
    Q_INVOKABLE QVariantMap getFileInfo(const QString &fileUrl) const;
    Q_INVOKABLE QVariantMap inspectFirmwareArchive(const QString &fileUrl,
                                                   const QString &expectedSha256 = QString()) const;
    Q_INVOKABLE bool copyImageToClipboard(const QString &fileUrl);
    Q_INVOKABLE void copyTextToClipboard(const QString &text);
    Q_INVOKABLE bool saveTextFile(const QString &pathOrUrl, const QString &content) const;
    Q_INVOKABLE bool openWithDialog(const QString &fileUrl);
    Q_INVOKABLE bool printImage(const QString &fileUrl);

private slots:
    void onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

public:
    void setGridRows(int rows);
    void setGridCols(int cols);

signals:
    void serviceStatusChanged();
    void cameraGroupsChanged();
    void appSettingsChanged();
    void gridLayoutChanged();
    void layoutTemplatesChanged();
    void snapshotSaved(const QString &path);
    void isArchiveOpenChanged();
    void cameraStatusDetailsChanged();
    void cameraEndpointProbeFinished(const QString &requestId,
                                     const QString &kind,
                                     const QString &host,
                                     int port,
                                     bool success,
                                     const QString &message,
                                     int httpStatus,
                                     int elapsedMs);
    void discoverySessionChanged();
    void discoveryValidationProgress(int completed, int total);
    void discoveryValidationFinished(int okCount, int failCount);
    void discoveryBatchAddFinished(int addedCount, int skippedCount);

private slots:
    void performSave();

private:
    struct DiscoveryValidationProbe {
        int index = -1;
        QString label;
    };

    QString m_serviceStatus;
    QProcess *m_process;
    CameraModel *m_cameraModel;     // All added cameras (Device List)
    CameraModel *m_discoveryModel;  // Discovered cameras
    // CPU sampling state
    bool m_cpuInit = false;
    bool m_isArchiveOpen = false;
    quint64 m_prevSysKernel = 0;
    quint64 m_prevSysUser = 0;
    quint64 m_prevProcKernel = 0;
    quint64 m_prevProcUser = 0;
    quint64 m_prevLinuxSystemCpu = 0;
    quint64 m_prevLinuxProcessCpu = 0;
    QElapsedTimer m_cpuTimer;
    QTimer *m_saveTimer;            // Debounce timer for saving state
    CameraModel *m_gridModel;       // Cameras in the grid
    QStringList m_cameraGroups;
    AnalyticsEngine *m_analyticsEngine;
    UserManager *m_userManager;
    LogModel *m_logModel;
    PtzController *m_ptzController;
    CamexController *m_camexController;
    DiscoveryController *m_dahuaDiscovery;
    ArchiveController *m_archiveController;
    MajesticClient *m_majesticClient;
    OpenIpcFirmwareClient *m_firmwareClient;
    NetworkDiscoveryService *m_networkDiscovery;
    AppUpdateChecker *m_appUpdateChecker;
    CameraHealthController *m_cameraHealthController;
    StatusChecker *m_statusChecker;
    QHash<QString, qint64> m_streamOfflineUntilMs;
    QHash<QString, QString> m_cameraStatusDetails;
    QString m_discoveryLastUpdated;
    QString m_discoveryLastInterface;
    bool m_discoveryLastDeepScan = false;
    QHash<QString, DiscoveryValidationProbe> m_discoveryValidationProbes;
    QHash<int, int> m_discoveryValidationRemaining;
    QHash<int, bool> m_discoveryValidationFailed;
    QHash<int, QStringList> m_discoveryValidationMessages;
    int m_discoveryValidationCompleted = 0;
    int m_discoveryValidationTotal = 0;
    
    // App Settings
    QVariantMap m_appSettings;
    int m_gridRows = 2;
    QVariantList m_layoutTemplates;
    int m_gridCols = 2;
    
    // Active recordings: map gridIndex -> QProcess*
    QMap<int, QProcess*> m_activeRecordings;
    
    QNetworkAccessManager *m_networkManager;
    // Map to store credentials for pending requests: url host -> pair(login, password)
    QMap<QString, QPair<QString, QString>> m_pendingCredentials;

    void saveState();
    void loadState();
    QString stateFilePath() const;
    QString stateDatabasePath() const;
    static QJsonObject cameraToJson(const Camera &cam);
    static Camera cameraFromJson(const QJsonObject &obj);
    int findDiscoveryMergeIndex(const Camera &candidate) const;
    void mergeDiscoveryCamera(const Camera &incoming);
    void markDiscoveryAddedFlags();
    void setDiscoveryValidationState(int index, const QString &status, const QString &message);
    Camera cameraFromDiscoveryForAdd(const Camera &source,
                                     const QString &profile,
                                     const QString &login,
                                     const QString &password) const;
    QString discoveryProfileForCamera(const Camera &camera, const QString &requestedProfile) const;
    QString rtspPathForProfile(const Camera &camera, const QString &profile, bool subStream) const;
    QString buildSanitizedRtspUrl(const Camera &camera, const QString &profile, bool subStream) const;
    void handleDiscoveryValidationProbe(const QString &requestId, bool success,
                                        const QString &message, int httpStatus, int elapsedMs);
};

#endif // SYSTEMCONTROLLER_H
