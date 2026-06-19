#ifndef SYSTEMCONTROLLER_H
#define SYSTEMCONTROLLER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QUdpSocket>
#include <QDebug>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include "CameraModel.h"
#include "analytics/AnalyticsEngine.h"
#include "UserManager.h"
#include "LogModel.h"
#include "PtzController.h"
#include "DiscoveryController.h"
#include "ArchiveController.h"
#include "CamexController.h"

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

    void addLog(QtMsgType type, const QString &msg);

public slots:
    void startService();
    void stopService();
    void scanNetwork(const QString &interfaceName = "");
    void addDevice(int index); // Adds from discovery to device list
    Q_INVOKABLE void addManualCamera(const QString &name, const QString &ip, const QString &url, int port, int onvifPort, const QString &login = "", const QString &password = "", const QString &sdUrl = "");
    Q_INVOKABLE void updateCamera(int index, const QString &name, const QString &ip, const QString &url, int port, int onvifPort, const QString &login = "", const QString &password = "", const QString &sdUrl = "");
    Q_INVOKABLE void updateCameraStatus(const QString &cameraIp, const QString &status);
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
    Q_INVOKABLE void applyLayoutPreset(int rows, int cols);
    Q_INVOKABLE void applyLayoutTemplate(const QVariantMap &layout);
    Q_INVOKABLE QString getCameraPassword(const QString &cameraIp) const;
    
    // Dahua SDK
    Q_INVOKABLE void takeDahuaSnapshot(const QString &ip, int port, const QString &login, const QString &password);
    Q_INVOKABLE void notifySnapshotSaved(const QString &path);
    Q_INVOKABLE QString getSnapshotPath(const QString &filename);
    Q_INVOKABLE bool deleteLocalFile(const QString &fileUrl);
    Q_INVOKABLE bool localFileExists(const QString &fileUrl) const;
    Q_INVOKABLE QVariantMap getFileInfo(const QString &fileUrl) const;
    Q_INVOKABLE bool copyImageToClipboard(const QString &fileUrl);
    Q_INVOKABLE void copyTextToClipboard(const QString &text);
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

private slots:
    void onUdpReadyRead();
    void performSave();

private:
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
    StatusChecker *m_statusChecker;
    QUdpSocket *m_udpSocket;
    
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

    void sendDiscoveryProbe();
    void saveState();
    void loadState();
    QString stateFilePath() const;
    static QJsonObject cameraToJson(const Camera &cam);
    static Camera cameraFromJson(const QJsonObject &obj);
};

#endif // SYSTEMCONTROLLER_H
