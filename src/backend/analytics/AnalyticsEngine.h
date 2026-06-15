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
#include <QPointF>
#include <QTcpServer>
#include <QUrl>
#include <QMap>
#include <QVariantMap>
#include <QVariantList>
#include "InferenceBackend.h"
#include "YoloDetector.h"

class AnalyticsEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList analyticsEvents READ analyticsEvents NOTIFY analyticsEventsChanged)
    Q_PROPERTY(QVariantMap analyticsDiagnostics READ analyticsDiagnostics NOTIFY analyticsTelemetryChanged)
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
    Q_INVOKABLE QVariantList analyticsEvents() const;
    Q_INVOKABLE QVariantMap analyticsDiagnostics() const;
    Q_INVOKABLE QVariantMap getModuleTelemetry(int type) const;
    Q_INVOKABLE QVariantList getObjectCounterSummary() const;
    Q_INVOKABLE QVariantList queryAnalyticsEvents(int type = -1,
                                                   const QString &cameraId = QString(),
                                                   const QString &text = QString(),
                                                   int limit = 500) const;
    Q_INVOKABLE void clearAnalyticsEvents(int type = -1, const QString &cameraId = QString());

    Q_INVOKABLE bool isBusy(const QString &cameraId) const;
    Q_INVOKABLE bool hasActiveModules(const QString &cameraId) const;
    QVariantMap getPersistedSettings() const;

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
    void analyticsEventsChanged();
    void analyticsTelemetryChanged();
    void analyticsNotificationRaised(const QVariantMap &event);

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
        QString faceSnapshotsMode = "standard"; // disabled, standard, anonymized
        QVariantMap extraConfig;
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

    struct TelemetryState {
        quint64 processedFrames = 0;
        quint64 skippedFrames = 0;
        quint64 detections = 0;
        quint64 events = 0;
        double totalInferenceMs = 0.0;
        double lastInferenceMs = 0.0;
    };

    struct TrackState {
        QString id;
        QString label;
        QRectF bounds;
        QPointF center;
        qint64 lastSeenMs = 0;
        int seenFrames = 0;
        float confidence = 0.0f;
        bool counted = false;
    };

    struct CounterState {
        qint64 nextTrackNumber = 1;
        QMap<QString, TrackState> tracks;
        QMap<QString, int> totalCountByLabel;
    };

    QMap<QString, QVector<BufferedFrame>> m_frameBuffers;
    QMutex m_bufferMutex;

    QMap<QString, PendingEvent> m_pendingEvents;
    QMap<QString, qint64> m_lastEventTimes;
    QMutex m_eventMutex;

    QMap<QString, CounterState> m_counterStates;
    mutable QMutex m_counterMutex;

    QMap<QString, TelemetryState> m_cameraTelemetry;
    QMap<ModuleType, TelemetryState> m_moduleTelemetry;
    mutable QMutex m_telemetryMutex;

    QMap<QString, qint64> m_ruleLastTriggeredMs;
    mutable QMutex m_ruleMutex;

    QVariantList m_analyticsEvents;
    int m_maxAnalyticsEvents = 250;
    QString m_eventStorePath;
    QString m_eventStoreConnectionName;
    bool m_eventStoreReady = false;
    mutable QMutex m_eventStoreMutex;

    struct UploadTask {
        QString filePath;
        QString provider;
        QString target;
    };
    QQueue<UploadTask> m_uploadQueue;
    mutable QMutex m_uploadMutex;
    bool m_uploadActive = false;

    void setupModules();
    QVariantMap buildEvidenceSettings(bool includeSensitiveData) const;
    QVariantMap telemetryStateToVariant(const TelemetryState &state) const;
    QVariantMap detectionToVariant(const DetectionBox &box, const QString &moduleId, ModuleType moduleType) const;
    bool zoneMatches(const QVariantMap &detection, const QString &zonePreset) const;
    QString countsToText(const QMap<QString, int> &counts) const;
    QString analyticsSecretKey(const QString &name) const;
    QString readSecretFromKeychain(const QString &name) const;
    void writeSecretToKeychain(const QString &name, const QString &value) const;
    void deleteSecretFromKeychain(const QString &name) const;
    void startDownload(ModuleType type);
    void checkRuntimeAndDownload(ModuleType type);
    void downloadFile(const QString &url, const QString &filePath, ModuleType type, bool isRuntime = false);
    void recordSkippedFrame(const QString &cameraId);
    bool moduleHasClipRules(const QVariantMap &extraConfig) const;
    QString ensurePendingClip(const QString &cameraId, const QVariantMap &detection, qint64 nowMs);
    QVariantList updateObjectCounterTracking(const QString &cameraId, QVector<DetectionBox> &results, qint64 nowMs);
    QVariantList evaluateRulesForDetection(const QString &cameraId,
                                           ModuleType moduleType,
                                           const QString &moduleId,
                                           const QVariantMap &detection,
                                           const QVariantList &rules,
                                           const QImage &frame,
                                           qint64 nowMs,
                                           const QString &existingSnapshotPath = QString(),
                                           const QString &existingClipPath = QString());
    void appendAnalyticsEvents(const QVariantList &events);
    void initEventStore();
    QVariantList loadRecentAnalyticsEvents(int limit) const;
    void persistAnalyticsEvents(const QVariantList &events);
    void deleteStoredAnalyticsEvents(int type = -1, const QString &cameraId = QString());

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
