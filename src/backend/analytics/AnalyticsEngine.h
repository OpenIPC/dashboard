#pragma once

#include <QObject>
#include <QMap>
#include <QImage>
#include <memory>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QMutex>
#include <QSet>
#include <QQueue>
#include <QTcpServer>
#include <QUrl>
#include <QMap>
#include "InferenceBackend.h"
#include "YoloDetector.h"

class AnalyticsEngine : public QObject {
    Q_OBJECT
public:
    enum ModuleType {
        FaceDetector,
        ObjectCounter,
        LicensePlate
    };
    Q_ENUM(ModuleType)

    explicit AnalyticsEngine(QObject *parent = nullptr);
    ~AnalyticsEngine();

    Q_INVOKABLE void initialize();
    Q_INVOKABLE bool isModuleAvailable(ModuleType type) const;
    Q_INVOKABLE bool isModuleEnabled(ModuleType type) const;
    Q_INVOKABLE void setModuleEnabled(ModuleType type, bool enabled);
    Q_INVOKABLE float getModuleProgress(ModuleType type) const;
    Q_INVOKABLE QString getModuleStatus(ModuleType type) const; // "not_installed", "downloading", "ready", "error"
    Q_INVOKABLE QString getModuleError(ModuleType type) const;
    Q_INVOKABLE void processFrame(const QImage &frame, const QString &cameraId);
    
    Q_INVOKABLE QVariantMap getSettings() const;
    Q_INVOKABLE void setSettings(const QVariantMap &settings);
    Q_INVOKABLE QVariantMap getEvidenceSettings() const;
    Q_INVOKABLE void setEvidenceSettings(const QVariantMap &settings);
    Q_INVOKABLE void startOAuth(const QString &provider, const QString &clientId, const QString &clientSecret = QString());
    Q_INVOKABLE void cancelOAuth();

    Q_INVOKABLE void setCameraModuleEnabled(const QString &cameraId, ModuleType type, bool enabled);
    Q_INVOKABLE bool isCameraModuleEnabled(const QString &cameraId, ModuleType type) const;

    Q_INVOKABLE void setModuleConfig(int type, const QVariantMap &config);
    Q_INVOKABLE QVariantMap getModuleConfig(int type) const;

    Q_INVOKABLE bool isBusy(const QString &cameraId) const;
    Q_INVOKABLE bool hasActiveModules(const QString &cameraId) const;

signals:
    void detectionOccurred(const QString &moduleId, const QString &cameraId, const QVariantMap &detection);
    void frameProcessed(const QString &cameraId, const QVariantList &detections);
    void moduleStatusChanged(int type, QString status, float progress, QString error);
    void moduleConfigChanged(int type);
    void settingsChanged();
    void oauthUrlReady(const QString &provider, const QString &url);
    void oauthCompleted(const QString &provider, const QString &accessToken, const QString &refreshToken, int expiresIn);
    void oauthError(const QString &provider, const QString &message);
    void clipRequested(const QString &cameraId, const QString &path, int durationMs);
    void clipStopRequested(const QString &cameraId, const QString &path);

private:
    struct ModuleContext {
        std::shared_ptr<InferenceBackend> backend;
        YoloDetector::Options options;
        bool enabled = false;
        QString status = "not_installed"; // not_installed, downloading, ready, error
        float progress = 0.0f;
        QString error;
        QString name;
        QString description;
        QString version;
        QString modelUrl;
        QString modelFileName;
        
        // Configuration
        QString snapshotsDir;
        QString faceSnapshotsMode = "standard"; // disabled, standard, anonymized, encrypted
        QString faceSnapshotKeyHex;
        bool faceSnapshotKeyConfigured = false;
    };

    QMap<ModuleType, ModuleContext> m_modules;
    QMap<QString, QMap<ModuleType, bool>> m_cameraModules; // cameraId -> (type -> enabled)
    QString m_modulesDir;
    
    QNetworkAccessManager *m_networkManager;
    QMap<ModuleType, QNetworkReply*> m_currentDownloads;
    QMap<ModuleType, QFile*> m_downloadFiles;

    mutable QMutex m_processingMutex;
    QSet<QString> m_processingCameras;
    
    // Throttling for snapshots: Key = "cameraId_moduleType", Value = timestamp ms
    QMap<QString, qint64> m_lastSnapshotTimes;
    QMutex m_snapshotMutex;

    // Evidence (snapshots/clips) settings
    bool m_evidenceEnabled = false;
    bool m_evidenceSnapshotsEnabled = true;
    bool m_evidenceClipsEnabled = true;
    QString m_evidenceSnapshotsDir;
    QString m_evidenceClipsDir;
    int m_evidencePreSeconds = 5;
    int m_evidencePostSeconds = 5;
    int m_evidenceMaxBufferSeconds = 20;
    int m_evidenceCooldownMs = 2000;
    float m_evidenceMinConfidence = 0.6f;
    int m_evidenceClipFps = 10;

    // Upload settings
    bool m_uploadEnabled = false;
    QString m_uploadProvider = "local"; // local, ftp, dropbox, gdrive, onedrive
    QString m_uploadTarget;
    QString m_uploadClientId;
    QString m_uploadClientSecret;
    QString m_uploadAccessToken;
    QString m_uploadRefreshToken;
    qint64 m_uploadExpiresAt = 0;

    // OAuth state
    QTcpServer *m_oauthServer = nullptr;
    QString m_oauthProvider;
    QString m_oauthClientId;
    QString m_oauthClientSecret;
    QString m_oauthRedirectUri;
    QString m_oauthCodeVerifier;

    struct BufferedFrame {
        QImage frame;
        qint64 timestampMs = 0;
    };
    struct PendingEvent {
        qint64 startMs = 0;
        qint64 endMs = 0;
        QString snapshotPath;
        QString clipPath;
        QVariantMap detection;
        bool streamRequested = false;
    };

    QMap<QString, QVector<BufferedFrame>> m_frameBuffers;
    QMutex m_bufferMutex;

    QMap<QString, PendingEvent> m_pendingEvents;
    QMap<QString, qint64> m_lastEventTimes;
    QMutex m_eventMutex;

    struct UploadTask {
        QString filePath;
        QString provider;
        QString target;
    };
    QQueue<UploadTask> m_uploadQueue;
    QMutex m_uploadMutex;
    bool m_uploadActive = false;

    void setupModules();
    void startDownload(ModuleType type);
    void checkRuntimeAndDownload(ModuleType type);
    void downloadFile(const QString &url, const QString &filePath, ModuleType type, bool isRuntime = false);

    void appendFrameToBuffer(const QString &cameraId, const QImage &frame, qint64 ts);
    QVector<BufferedFrame> collectFrames(const QString &cameraId, qint64 startMs, qint64 endMs);
    QString ensureDir(const QString &path);
    QString buildEvidenceFileName(const QString &cameraId, const QString &suffix) const;
    QString saveSnapshotImage(const QImage &frame, const QString &cameraId, const QVariantMap &det, const QString &moduleId);
    void scheduleClipIfReady(const QString &cameraId, qint64 nowMs);
    void saveClipAsync(const QString &cameraId, const QVector<BufferedFrame> &frames, const QString &path);
    bool writeClipMp4(const QVector<BufferedFrame> &frames, const QString &path, int fps);

    void enqueueUpload(const QString &filePath);
    void processNextUpload();
};
