#include "AnalyticsEngine.h"
#include "YoloDetector.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QSslSocket>
#include <QtConcurrent>

AnalyticsEngine::AnalyticsEngine(QObject *parent)
    : QObject(parent)
{
    fprintf(stderr, "AnalyticsEngine::AnalyticsEngine constructor called\n");
    m_networkManager = new QNetworkAccessManager(this);

    qInfo() << "SSL Support:" << QSslSocket::supportsSsl();
    qInfo() << "SSL Library Build:" << QSslSocket::sslLibraryBuildVersionString();
    qInfo() << "SSL Library Run:" << QSslSocket::sslLibraryVersionString();

    // Use AppData location for modules to ensure write permissions
    m_modulesDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/modules";
    qInfo() << "Analytics modules directory:" << m_modulesDir;
    
    QDir dir(m_modulesDir);
    if (!dir.exists()) {
        if (dir.mkpath(".")) {
            qInfo() << "Created modules directory";
        } else {
            qWarning() << "Failed to create modules directory:" << m_modulesDir;
        }
    }
}

AnalyticsEngine::~AnalyticsEngine()
{
    // Cancel any active downloads
    for (auto reply : m_currentDownloads) {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    }
    
    // Close and delete any open files
    for (auto file : m_downloadFiles) {
        if (file) {
            file->close();
            delete file;
        }
    }
}

void AnalyticsEngine::initialize()
{
    setupModules();
}

void AnalyticsEngine::setupModules()
{
    auto setupModule = [&](ModuleType type, const QString &name, const QString &desc, const QString &ver, 
                          const QString &url, const QString &filename, const YoloDetector::Options &opts) {
        ModuleContext ctx;
        ctx.name = name;
        ctx.description = desc;
        ctx.version = ver;
        ctx.modelUrl = url;
        ctx.modelFileName = filename;
        ctx.backend = std::make_shared<YoloDetector>(opts);
        
        // Check if already installed
        if (ctx.backend->load(m_modulesDir)) {
            ctx.status = "ready";
            ctx.progress = 1.0f;
        } else {
            ctx.status = "not_installed";
            ctx.progress = 0.0f;
        }
        
        m_modules[type] = ctx;
        emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
    };

    // 1. Face Detector
    {
        YoloDetector::Options opts;
        opts.modelFile = "yolov11n-face.onnx";
        opts.classLabels = {"Face"};
        opts.colorPalette = {"#ff7f50"};
        setupModule(FaceDetector, "Face Detector", "Detects faces in video stream", "1.0.0",
                   "https://raw.githubusercontent.com/Rinibr25/Face-Detector-Module-for-Dashboard-/main/yolov11n-face.onnx",
                   "yolov11n-face.onnx", opts);
    }

    // 2. Object Counter
    {
        YoloDetector::Options opts;
        opts.modelFile = "yolo11s.onnx";
        opts.classLabels = {"person", "bicycle", "car", "motorcycle", "bus", "truck"}; 
        opts.colorPalette = {"#2563eb", "#10b981", "#f59e0b", "#ef4444", "#8b5cf6", "#ec4899"};
        setupModule(ObjectCounter, "Object Counter", "Counts people and vehicles", "1.0.0",
                   "https://github.com/Rinibr25/Object-Counter-for-Dashboard/releases/download/v0.1.0/yolo11s.onnx",
                   "yolo11s.onnx", opts);
    }

    // 3. License Plate
    {
        YoloDetector::Options opts;
        opts.modelFile = "anpr_yolov8.onnx";
        opts.classLabels = {"License Plate"};
        opts.colorPalette = {"#ffff00"};
        setupModule(LicensePlate, "License Plate", "Recognizes license plates", "1.0.0",
                   "https://github.com/Rinibr25/License-Plate-Detector-for-Dashboard/releases/download/v0.1.0/anpr_yolov8.onnx",
                   "anpr_yolov8.onnx", opts);
    }
}

bool AnalyticsEngine::isModuleAvailable(ModuleType type) const
{
    if (!m_modules.contains(type)) return false;
    return m_modules[type].status == "ready";
}

bool AnalyticsEngine::isModuleEnabled(ModuleType type) const
{
    if (!m_modules.contains(type)) return false;
    return m_modules[type].enabled;
}

void AnalyticsEngine::setModuleEnabled(ModuleType type, bool enabled)
{
    qInfo() << "setModuleEnabled" << type << enabled;
    // Also print to stderr directly to be absolutely sure
    fprintf(stderr, "AnalyticsEngine::setModuleEnabled type=%d enabled=%d\n", (int)type, enabled);
    
    if (!m_modules.contains(type)) {
        qWarning() << "Module type not found:" << type;
        return;
    }
    
    ModuleContext &ctx = m_modules[type];
    bool changed = false;
    
    if (enabled) {
        if (ctx.status == "ready") {
            if (!ctx.enabled) changed = true;
            ctx.enabled = true;
            emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
        } else if (ctx.status == "not_installed" || ctx.status == "error") {
            // Trigger download
            qInfo() << "Module not installed or error, starting download for type" << type;
            startDownload(type);
        }
    } else {
        if (ctx.enabled) changed = true;
        ctx.enabled = false;
        emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
    }

    if (changed) {
        emit settingsChanged();
    }
}

float AnalyticsEngine::getModuleProgress(ModuleType type) const
{
    if (!m_modules.contains(type)) return 0.0f;
    return m_modules[type].progress;
}

QString AnalyticsEngine::getModuleStatus(ModuleType type) const
{
    if (!m_modules.contains(type)) return "error";
    return m_modules[type].status;
}

QString AnalyticsEngine::getModuleError(ModuleType type) const
{
    if (!m_modules.contains(type)) return "";
    return m_modules[type].error;
}

void AnalyticsEngine::startDownload(ModuleType type)
{
    qInfo() << "startDownload" << type;
    fprintf(stderr, "AnalyticsEngine::startDownload type=%d\n", (int)type);
    
    if (!m_modules.contains(type)) return;
    
    ModuleContext &ctx = m_modules[type];
    ctx.status = "downloading";
    ctx.progress = 0.0f;
    ctx.error.clear();
    emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
    
    // First check if we need to download the runtime
    checkRuntimeAndDownload(type);
}

void AnalyticsEngine::checkRuntimeAndDownload(ModuleType type)
{
    // Runtime is now linked statically/shipped with the app, so we only need to download the model.
    qInfo() << "Downloading model...";
    ModuleContext &ctx = m_modules[type];
    QString modelPath = QDir(m_modulesDir).filePath(ctx.modelFileName);
    downloadFile(ctx.modelUrl, modelPath, type, false);
}

void AnalyticsEngine::downloadFile(const QString &url, const QString &filePath, ModuleType type, bool isRuntime)
{
    qInfo() << "downloadFile" << url << "to" << filePath;
    fprintf(stderr, "AnalyticsEngine::downloadFile url=%s path=%s\n", qPrintable(url), qPrintable(filePath));
    
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    QNetworkReply *reply = m_networkManager->get(request);
    m_currentDownloads[type] = reply;
    
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        m_modules[type].status = "error";
        m_modules[type].error = "Failed to open file for writing: " + filePath;
        emit moduleStatusChanged(type, m_modules[type].status, 0.0f, m_modules[type].error);
        delete file;
        reply->abort();
        reply->deleteLater();
        return;
    }
    m_downloadFiles[type] = file;

    connect(reply, &QNetworkReply::readyRead, this, [this, type, reply]() {
        if (m_downloadFiles.contains(type)) {
            m_downloadFiles[type]->write(reply->readAll());
        }
    });

    connect(reply, &QNetworkReply::downloadProgress, this, [this, type, isRuntime](qint64 bytesReceived, qint64 bytesTotal) {
        // qInfo() << "Progress" << type << bytesReceived << "/" << bytesTotal;
        if (m_modules.contains(type)) {
            float progress = 0.0f;
            if (bytesTotal > 0) {
                progress = (float)bytesReceived / (float)bytesTotal;
            } else {
                // Indeterminate progress: cycle 0.1 -> 0.9
                // Just use a fake increment based on received bytes if total is unknown
                // or just set to 0.5 to show "something"
                progress = 0.5f; 
            }
            
            ModuleContext &ctx = m_modules[type];
            if (isRuntime) {
                ctx.progress = progress * 0.5f;
            } else {
                ctx.progress = progress;
            }
            
            // Force status to downloading if it somehow changed
            if (ctx.status != "downloading") {
                ctx.status = "downloading";
            }
            
            emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
        }
    });
    
    connect(reply, &QNetworkReply::sslErrors, this, [this, reply](const QList<QSslError> &errors) {
        qWarning() << "SSL Errors:";
        for (const auto &error : errors) {
            qWarning() << error.errorString();
        }
        reply->ignoreSslErrors(); // TEMPORARY: Ignore SSL errors to test if that's the blocker
    });

    connect(reply, &QNetworkReply::finished, this, [this, type, isRuntime, reply]() {
        qInfo() << "Download finished" << type << "Error:" << reply->error();
        fprintf(stderr, "AnalyticsEngine::downloadFile finished type=%d error=%d\n", (int)type, reply->error());
        
        if (m_downloadFiles.contains(type)) {
            m_downloadFiles[type]->close();
            delete m_downloadFiles[type];
            m_downloadFiles.remove(type);
        }
        
        m_currentDownloads.remove(type);
        
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Network error:" << reply->errorString();
            if (m_modules.contains(type)) {
                m_modules[type].status = "error";
                m_modules[type].error = reply->errorString();
                emit moduleStatusChanged(type, m_modules[type].status, 0.0f, m_modules[type].error);
            }
        } else {
            // Check for redirect manually if needed (though RedirectPolicy should handle it)
            QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
            if (redirect.isValid()) {
                QUrl newUrl = reply->url().resolved(redirect.toUrl());
                qInfo() << "Redirecting to:" << newUrl;
                // If auto-redirect didn't work (e.g. different scheme), we might need to handle it here
                // But UserVerifiedRedirectPolicy usually stops and asks. 
                // Let's assume NoLessSafeRedirectPolicy was better, but maybe we need to handle the redirect manually if it fails.
                // Actually, let's just recurse.
                downloadFile(newUrl.toString(), QDir(m_modulesDir).filePath(isRuntime ? "onnxruntime.dll" : m_modules[type].modelFileName), type, isRuntime);
                reply->deleteLater();
                return;
            }

            if (isRuntime) {
                // Runtime downloaded, now download model
                qInfo() << "Runtime downloaded, starting model download";
                ModuleContext &ctx = m_modules[type];
                QString modelPath = QDir(m_modulesDir).filePath(ctx.modelFileName);
                downloadFile(ctx.modelUrl, modelPath, type, false);
            } else {
                // Model downloaded, load it
                qInfo() << "Model downloaded, loading backend";
                ModuleContext &ctx = m_modules[type];
                if (ctx.backend->load(m_modulesDir)) {
                    ctx.status = "ready";
                    ctx.progress = 1.0f;
                    ctx.enabled = true;
                    qInfo() << "Module ready";
                    emit settingsChanged();
                } else {
                    ctx.status = "error";
                    ctx.error = "Failed to load model after download";
                    qWarning() << "Failed to load model";
                }
                emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
            }
        }
        
        reply->deleteLater();
    });
}

void AnalyticsEngine::processFrame(const QImage &frame, const QString &cameraId)
{
    // Check if we are already processing a frame for this camera
    {
        QMutexLocker locker(&m_processingMutex);
        if (m_processingCameras.contains(cameraId)) {
            return; // Skip frame to maintain real-time performance
        }
        
        // Throttling for frame *ingestion* to avoid queue buildup is good,
        // but we also need snapshot throttling.
        m_processingCameras.insert(cameraId);
    }

    // Prepare data for background thread
    QImage frameCopy = frame;
    
    // Collect active backends to avoid accessing m_modules (which is not thread-safe) in the worker thread
    struct TaskContext {
        QString moduleId;
        ModuleType moduleType;
        std::shared_ptr<InferenceBackend> backend;
        QString snapshotsDir;
        QString faceSnapshotsMode;
    };
    QList<TaskContext> tasks;

    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        ModuleType type = it.key();
        bool globallyEnabled = it.value().enabled && it.value().status == "ready";
        bool cameraEnabled = m_cameraModules.value(cameraId).value(type, false);

        if (globallyEnabled && cameraEnabled) {
            tasks.append({
                it.value().name, 
                type,
                it.value().backend, 
                it.value().snapshotsDir, 
                it.value().faceSnapshotsMode
            });
        }
    }

    if (tasks.isEmpty()) {
        QMutexLocker locker(&m_processingMutex);
        m_processingCameras.remove(cameraId);
        return;
    }

    // Run in background thread
    (void)QtConcurrent::run([this, frameCopy, cameraId, tasks]() {
        QVariantList allDetections;

        for (const auto &task : tasks) {
            if (!task.backend) continue;

            QVector<DetectionBox> results = task.backend->detect(frameCopy);
            
            for (const auto &box : results) {
                QVariantMap detection;
                detection["label"] = box.label;
                detection["confidence"] = box.confidence;
                detection["x"] = box.bounds.x();
                detection["y"] = box.bounds.y();
                detection["w"] = box.bounds.width();
                detection["h"] = box.bounds.height();
                detection["moduleId"] = task.moduleId;
                
                allDetections.append(detection);

                // --- Snapshot Logic ---
                // Only save if directory is configured and exists
                if (!task.snapshotsDir.isEmpty() && QDir(task.snapshotsDir).exists()) {
                    
                    // Throttling: Check if we saved recently for this camera + module
                    // We need a thread-safe check. 
                    // Using invokeMethod to check/update state on main thread is too slow for blocking the worker.
                    // Accessing m_lastSnapshotTimes with mutex here.
                    bool canSnapshot = false;
                    QString key = cameraId + "_" + QString::number(task.moduleType);
                    qint64 now = QDateTime::currentMSecsSinceEpoch();
                    
                    {
                        QMutexLocker snapshotLocker(&m_snapshotMutex);
                        qint64 last = m_lastSnapshotTimes.value(key, 0);
                        if (now - last > 1000) { // Limit: 1 snapshot per second per module per camera
                            m_lastSnapshotTimes[key] = now;
                            canSnapshot = true;
                        }
                    }

                    if (canSnapshot && box.confidence > 0.6) { // Ensure quality
                        if (task.moduleType == FaceDetector && task.faceSnapshotsMode != "disabled") {
                            // Calculate rect with padding (50% larger)
                            int x = (int)(box.bounds.x() * frameCopy.width());
                            int y = (int)(box.bounds.y() * frameCopy.height());
                            int w = (int)(box.bounds.width() * frameCopy.width());
                            int h = (int)(box.bounds.height() * frameCopy.height());

                            // Apply padding
                            int padW = w / 2;
                            int padH = h / 2;
                            
                            x -= padW / 2;
                            y -= padH / 2;
                            w += padW;
                            h += padH;

                            // Clamp values
                            x = std::max(0, x);
                            y = std::max(0, y);
                            w = std::min(frameCopy.width() - x, w);
                            h = std::min(frameCopy.height() - y, h);
                            
                            QRect faceRect(x, y, w, h);
                            if (w > 10 && h > 10) {
                                QImage faceImg = frameCopy.copy(faceRect);
                                
                                // TODO: Implement "anonymized" blur if needed
                                if (task.faceSnapshotsMode == "anonymized") {
                                    // Scale down and up to pixelate
                                    faceImg = faceImg.scaled(w/10, h/10).scaled(w, h);
                                }
                                
                                QString filename = QString("%1/face_%2_%3.jpg")
                                    .arg(task.snapshotsDir)
                                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz"))
                                    .arg((int)(box.confidence * 100));
                                    
                                faceImg.save(filename, "JPG", 100);
                            }
                        }
                        // Add other modules here
                    }
                }
                // ---------------------
            }
        }

        // Report back to main thread
        QMetaObject::invokeMethod(this, [this, cameraId, allDetections]() {
            {
                QMutexLocker locker(&m_processingMutex);
                m_processingCameras.remove(cameraId);
            }
            
            // Emit individual detections for specific listeners
            for (const auto &detVar : allDetections) {
                QVariantMap det = detVar.toMap();
                emit detectionOccurred(det["moduleId"].toString(), cameraId, det);
            }

            // Emit batch for UI overlay
            emit frameProcessed(cameraId, allDetections);
        });
    });
}

bool AnalyticsEngine::isBusy(const QString &cameraId) const
{
    QMutexLocker locker(&m_processingMutex);
    return m_processingCameras.contains(cameraId);
}

bool AnalyticsEngine::hasActiveModules(const QString &cameraId) const
{
    if (!m_cameraModules.contains(cameraId)) return false;
    const auto &modules = m_cameraModules[cameraId];
    for (auto it = modules.begin(); it != modules.end(); ++it) {
        if (it.value()) {
            // Check if module is globally enabled and ready
            ModuleType type = it.key();
            if (m_modules.contains(type) && m_modules[type].enabled && m_modules[type].status == "ready") {
                return true;
            }
        }
    }
    return false;
}

void AnalyticsEngine::setCameraModuleEnabled(const QString &cameraId, ModuleType type, bool enabled)
{
    m_cameraModules[cameraId][type] = enabled;
}

bool AnalyticsEngine::isCameraModuleEnabled(const QString &cameraId, ModuleType type) const
{
    return m_cameraModules.value(cameraId).value(type, false);
}

QVariantMap AnalyticsEngine::getSettings() const
{
    QVariantMap settings;
    QVariantMap modules;
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        QVariantMap moduleSettings;
        moduleSettings["enabled"] = it.value().enabled;
        modules[QString::number(it.key())] = moduleSettings;
    }
    settings["modules"] = modules;
    return settings;
}

void AnalyticsEngine::setSettings(const QVariantMap &settings)
{
    QVariantMap modules = settings.value("modules").toMap();
    for (auto it = modules.begin(); it != modules.end(); ++it) {
        int type = it.key().toInt();
        QVariantMap moduleSettings = it.value().toMap();
        bool enabled = moduleSettings.value("enabled").toBool();
        
        if (m_modules.contains((ModuleType)type)) {
             if (enabled) {
                 setModuleEnabled((ModuleType)type, true);
             }
             setModuleConfig(type, moduleSettings);
        }
    }
}

void AnalyticsEngine::setModuleConfig(int type, const QVariantMap &config)
{
    if (!m_modules.contains((ModuleType)type)) return;
    
    ModuleContext &ctx = m_modules[(ModuleType)type];
    bool changed = false;
    
    if (config.contains("snapshotsDir")) {
        QString newDir = config["snapshotsDir"].toString();
        if (ctx.snapshotsDir != newDir) {
            ctx.snapshotsDir = newDir;
            changed = true;
        }
    }
    
    if (config.contains("faceSnapshotsMode")) {
        QString newMode = config["faceSnapshotsMode"].toString();
        if (ctx.faceSnapshotsMode != newMode) {
            ctx.faceSnapshotsMode = newMode;
            changed = true;
        }
    }
    
    if (config.contains("faceSnapshotKeyHex")) {
        QString newKey = config["faceSnapshotKeyHex"].toString();
        if (ctx.faceSnapshotKeyHex != newKey) {
            ctx.faceSnapshotKeyHex = newKey;
            ctx.faceSnapshotKeyConfigured = !newKey.isEmpty();
            changed = true;
        }
    }
    
    if (config.contains("resetFaceSnapshotKey") && config["resetFaceSnapshotKey"].toBool()) {
        ctx.faceSnapshotKeyHex = "";
        ctx.faceSnapshotKeyConfigured = false;
        changed = true;
    }
    
    if (changed) {
        emit moduleConfigChanged(type);
        emit settingsChanged();
    }
}

QVariantMap AnalyticsEngine::getModuleConfig(int type) const
{
    if (!m_modules.contains((ModuleType)type)) return QVariantMap();
    
    const ModuleContext &ctx = m_modules[(ModuleType)type];
    QVariantMap config;
    config["snapshotsDir"] = ctx.snapshotsDir;
    config["faceSnapshotsMode"] = ctx.faceSnapshotsMode;
    config["faceSnapshotKeyConfigured"] = ctx.faceSnapshotKeyConfigured;
    // Do not return the actual key for security, just whether it's configured
    
    return config;
}
