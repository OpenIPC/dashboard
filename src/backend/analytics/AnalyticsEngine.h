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
#include "InferenceBackend.h"

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

private:
    struct ModuleContext {
        std::shared_ptr<InferenceBackend> backend;
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

    void setupModules();
    void startDownload(ModuleType type);
    void checkRuntimeAndDownload(ModuleType type);
    void downloadFile(const QString &url, const QString &filePath, ModuleType type, bool isRuntime = false);
};
