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
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QStandardPaths>
#include <QSslSocket>
#include <QtConcurrent>
#include <QFileInfo>
#include <QEventLoop>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <gst/app/gstappsrc.h>

static void ensureGstInitOnce() {
    static bool initialized = false;
    if (!initialized) {
        gst_init(nullptr, nullptr);
        initialized = true;
    }
}

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

    // Default evidence directories
    m_evidenceSnapshotsDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/OpenIPC/Evidence";
    m_evidenceClipsDir = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/OpenIPC/Evidence";
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
        ctx.options = opts;
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
            if (!ctx.backend || !ctx.backend->isLoaded()) {
                ctx.backend = std::make_shared<YoloDetector>(ctx.options);
                if (!ctx.backend->load(m_modulesDir)) {
                    ctx.status = "error";
                    ctx.error = ctx.backend ? ctx.backend->getError() : "Failed to load model";
                    emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
                    if (changed) emit settingsChanged();
                    return;
                }
            }
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
        ctx.backend.reset();
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

        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

        const bool evidenceActive = (m_evidenceEnabled || m_evidenceSnapshotsEnabled || m_evidenceClipsEnabled);

        // Buffer frames for evidence (pre/post clips)
        if (evidenceActive) {
            appendFrameToBuffer(cameraId, frameCopy, nowMs);
        }

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

                // Evidence capture (generic snapshot + event scheduling)
                if (evidenceActive && box.confidence >= m_evidenceMinConfidence) {
                    bool shouldTrigger = false;
                    {
                        QMutexLocker eventLocker(&m_eventMutex);
                        qint64 last = m_lastEventTimes.value(cameraId, 0);
                        if (nowMs - last >= m_evidenceCooldownMs) {
                            shouldTrigger = true;
                            m_lastEventTimes[cameraId] = nowMs;
                        }
                    }

                    if (shouldTrigger) {
                        // Create or extend pending event
                        {
                            QMutexLocker eventLocker(&m_eventMutex);
                            PendingEvent &evt = m_pendingEvents[cameraId];
                            if (evt.endMs < nowMs) {
                                evt.startMs = nowMs - (m_evidencePreSeconds * 1000);
                                evt.endMs = nowMs + (m_evidencePostSeconds * 1000);
                                evt.detection = detection;
                                evt.snapshotPath.clear();
                                evt.clipPath.clear();
                                evt.streamRequested = false;
                            } else {
                                evt.endMs = nowMs + (m_evidencePostSeconds * 1000);
                            }
                        }

                        if (m_evidenceClipsEnabled && receivers(SIGNAL(clipRequested(QString,QString,int))) > 0) {
                            QString dir = ensureDir(m_evidenceClipsDir);
                            if (!dir.isEmpty()) {
                                QString name = buildEvidenceFileName(cameraId, "clip");
                                QString path = QDir(dir).filePath(name + ".mp4");
                                emit clipRequested(cameraId, path, 0);
                                QMutexLocker eventLocker(&m_eventMutex);
                                m_pendingEvents[cameraId].streamRequested = true;
                                m_pendingEvents[cameraId].clipPath = path;
                            }
                        }

                        if (m_evidenceSnapshotsEnabled) {
                            QString snapshotPath = saveSnapshotImage(frameCopy, cameraId, detection, task.moduleId);
                            if (!snapshotPath.isEmpty()) {
                                QMutexLocker eventLocker(&m_eventMutex);
                                m_pendingEvents[cameraId].snapshotPath = snapshotPath;
                                if (m_uploadEnabled) {
                                    enqueueUpload(snapshotPath);
                                }
                            }
                        }
                    }
                }

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
                        } else if (task.moduleType == LicensePlate) {
                            int x = (int)(box.bounds.x() * frameCopy.width());
                            int y = (int)(box.bounds.y() * frameCopy.height());
                            int w = (int)(box.bounds.width() * frameCopy.width());
                            int h = (int)(box.bounds.height() * frameCopy.height());

                            int padW = w / 3;
                            int padH = h / 3;

                            x -= padW / 2;
                            y -= padH / 2;
                            w += padW;
                            h += padH;

                            x = std::max(0, x);
                            y = std::max(0, y);
                            w = std::min(frameCopy.width() - x, w);
                            h = std::min(frameCopy.height() - y, h);

                            QRect plateRect(x, y, w, h);
                            if (w > 10 && h > 10) {
                                QImage plateImg = frameCopy.copy(plateRect);
                                QString filename = QString("%1/plate_%2_%3.jpg")
                                    .arg(task.snapshotsDir)
                                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz"))
                                    .arg((int)(box.confidence * 100));
                                plateImg.save(filename, "JPG", 95);
                            }
                        }
                    }
                }
                // ---------------------
            }
        }

        // Check if any pending events are ready to finalize clips
        if (evidenceActive && m_evidenceClipsEnabled) {
            scheduleClipIfReady(cameraId, nowMs);
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

    if (enabled) {
        if (m_modules.contains(type)) {
            ModuleContext &ctx = m_modules[type];
            if (ctx.enabled && (!ctx.backend || !ctx.backend->isLoaded())) {
                ctx.backend = std::make_shared<YoloDetector>(ctx.options);
                if (!ctx.backend->load(m_modulesDir)) {
                    ctx.status = "error";
                    ctx.error = ctx.backend ? ctx.backend->getError() : "Failed to load model";
                    emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
                }
            }
        }
    } else {
        bool anyCameraUsesModule = false;
        for (auto it = m_cameraModules.begin(); it != m_cameraModules.end(); ++it) {
            if (it.value().value(type, false)) {
                anyCameraUsesModule = true;
                break;
            }
        }
        if (!anyCameraUsesModule && m_modules.contains(type)) {
            m_modules[type].backend.reset();
        }
    }

    if (!enabled && !hasActiveModules(cameraId)) {
        {
            QMutexLocker bufferLocker(&m_bufferMutex);
            m_frameBuffers.remove(cameraId);
        }
        {
            QMutexLocker eventLocker(&m_eventMutex);
            m_pendingEvents.remove(cameraId);
            m_lastEventTimes.remove(cameraId);
        }
        {
            QMutexLocker snapshotLocker(&m_snapshotMutex);
            const QString prefix = cameraId + "_";
            auto it = m_lastSnapshotTimes.begin();
            while (it != m_lastSnapshotTimes.end()) {
                if (it.key().startsWith(prefix)) {
                    it = m_lastSnapshotTimes.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
}

bool AnalyticsEngine::isCameraModuleEnabled(const QString &cameraId, ModuleType type) const
{
    return m_cameraModules.value(cameraId).value(type, false);
}

QVariantMap AnalyticsEngine::getSettings() const
{
    QVariantMap settings;
    QVariantMap modules;
    QVariantMap moduleConfigs;
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        QVariantMap moduleSettings;
        moduleSettings["enabled"] = it.value().enabled;
        modules[QString::number(it.key())] = moduleSettings;

        QVariantMap cfg;
        if (!it.value().snapshotsDir.isEmpty()) cfg["snapshotsDir"] = it.value().snapshotsDir;
        if (!it.value().faceSnapshotsMode.isEmpty()) cfg["faceSnapshotsMode"] = it.value().faceSnapshotsMode;
        if (!it.value().faceSnapshotKeyHex.isEmpty()) cfg["faceSnapshotKeyHex"] = it.value().faceSnapshotKeyHex;
        moduleConfigs[QString::number(it.key())] = cfg;
    }
    settings["modules"] = modules;
    settings["moduleConfigs"] = moduleConfigs;
    settings["evidence"] = getEvidenceSettings();
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

    if (settings.contains("moduleConfigs")) {
        QVariantMap configs = settings.value("moduleConfigs").toMap();
        for (auto it = configs.begin(); it != configs.end(); ++it) {
            int type = it.key().toInt();
            QVariantMap cfg = it.value().toMap();
            if (m_modules.contains((ModuleType)type)) {
                setModuleConfig(type, cfg);
            }
        }
    }

    if (settings.contains("evidence")) {
        setEvidenceSettings(settings.value("evidence").toMap());
    }
}

QVariantMap AnalyticsEngine::getEvidenceSettings() const
{
    QVariantMap s;
    s["enabled"] = m_evidenceEnabled;
    s["snapshotsEnabled"] = m_evidenceSnapshotsEnabled;
    s["clipsEnabled"] = m_evidenceClipsEnabled;
    s["snapshotsDir"] = m_evidenceSnapshotsDir;
    s["clipsDir"] = m_evidenceClipsDir;
    s["preSeconds"] = m_evidencePreSeconds;
    s["postSeconds"] = m_evidencePostSeconds;
    s["maxBufferSeconds"] = m_evidenceMaxBufferSeconds;
    s["cooldownMs"] = m_evidenceCooldownMs;
    s["minConfidence"] = m_evidenceMinConfidence;
    s["clipFps"] = m_evidenceClipFps;
    s["uploadEnabled"] = m_uploadEnabled;
    s["uploadProvider"] = m_uploadProvider;
    s["uploadTarget"] = m_uploadTarget;
    s["uploadClientId"] = m_uploadClientId;
    s["uploadClientSecret"] = m_uploadClientSecret;
    s["uploadAccessToken"] = m_uploadAccessToken;
    s["uploadRefreshToken"] = m_uploadRefreshToken;
    s["uploadExpiresAt"] = (qint64)m_uploadExpiresAt;
    return s;
}

void AnalyticsEngine::setEvidenceSettings(const QVariantMap &settings)
{
    bool changed = false;

    auto setIfChanged = [&](auto &field, const auto &value) {
        if (field != value) { field = value; changed = true; }
    };

    if (settings.contains("enabled")) setIfChanged(m_evidenceEnabled, settings.value("enabled").toBool());
    if (settings.contains("snapshotsEnabled")) setIfChanged(m_evidenceSnapshotsEnabled, settings.value("snapshotsEnabled").toBool());
    if (settings.contains("clipsEnabled")) setIfChanged(m_evidenceClipsEnabled, settings.value("clipsEnabled").toBool());
    if (settings.contains("snapshotsDir")) {
        QString dir = settings.value("snapshotsDir").toString();
        if (!dir.trimmed().isEmpty()) {
            setIfChanged(m_evidenceSnapshotsDir, dir);
        }
    }
    if (settings.contains("clipsDir")) {
        QString dir = settings.value("clipsDir").toString();
        if (!dir.trimmed().isEmpty()) {
            setIfChanged(m_evidenceClipsDir, dir);
        }
    }
    if (settings.contains("preSeconds")) setIfChanged(m_evidencePreSeconds, settings.value("preSeconds").toInt());
    if (settings.contains("postSeconds")) setIfChanged(m_evidencePostSeconds, settings.value("postSeconds").toInt());
    if (settings.contains("maxBufferSeconds")) setIfChanged(m_evidenceMaxBufferSeconds, settings.value("maxBufferSeconds").toInt());
    if (settings.contains("cooldownMs")) setIfChanged(m_evidenceCooldownMs, settings.value("cooldownMs").toInt());
    if (settings.contains("minConfidence")) setIfChanged(m_evidenceMinConfidence, settings.value("minConfidence").toFloat());
    if (settings.contains("clipFps")) setIfChanged(m_evidenceClipFps, settings.value("clipFps").toInt());
    if (settings.contains("uploadEnabled")) setIfChanged(m_uploadEnabled, settings.value("uploadEnabled").toBool());
    if (settings.contains("uploadProvider")) setIfChanged(m_uploadProvider, settings.value("uploadProvider").toString());
    if (settings.contains("uploadTarget")) setIfChanged(m_uploadTarget, settings.value("uploadTarget").toString());
    if (settings.contains("uploadClientId")) setIfChanged(m_uploadClientId, settings.value("uploadClientId").toString());
    if (settings.contains("uploadClientSecret")) setIfChanged(m_uploadClientSecret, settings.value("uploadClientSecret").toString());
    if (settings.contains("uploadAccessToken")) setIfChanged(m_uploadAccessToken, settings.value("uploadAccessToken").toString());
    if (settings.contains("uploadRefreshToken")) setIfChanged(m_uploadRefreshToken, settings.value("uploadRefreshToken").toString());
    if (settings.contains("uploadExpiresAt")) {
        qint64 v = settings.value("uploadExpiresAt").toLongLong();
        if (m_uploadExpiresAt != v) m_uploadExpiresAt = v;
    }

    // Keep buffer large enough to cover pre/post
    int minBuffer = qMax(10, m_evidencePreSeconds + m_evidencePostSeconds + 5);
    if (m_evidenceMaxBufferSeconds < minBuffer) {
        m_evidenceMaxBufferSeconds = minBuffer;
    }

    if (changed) {
        emit settingsChanged();
    }
}

static QString base64Url(const QByteArray &data)
{
    QByteArray b = data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    return QString::fromLatin1(b);
}

static QString makeCodeVerifier()
{
    QByteArray bytes(32, 0);
    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] = (char)QRandomGenerator::global()->bounded(0, 256);
    }
    return base64Url(bytes);
}

void AnalyticsEngine::startOAuth(const QString &provider, const QString &clientId, const QString &clientSecret)
{
    if (clientId.trimmed().isEmpty()) {
        emit oauthError(provider, "client_id is empty");
        return;
    }

    cancelOAuth();

    m_oauthProvider = provider;
    m_oauthClientId = clientId;
    m_oauthClientSecret = clientSecret;
    m_oauthCodeVerifier = makeCodeVerifier();

    m_oauthServer = new QTcpServer(this);
    int preferredPort = (provider == "dropbox") ? 53682 : 0;
    if (preferredPort > 0) {
        if (!m_oauthServer->listen(QHostAddress::LocalHost, preferredPort)) {
            emit oauthError(provider, "Failed to start local callback server on port 53682");
            cancelOAuth();
            return;
        }
    } else {
        if (!m_oauthServer->listen(QHostAddress::LocalHost, 0)) {
            emit oauthError(provider, "Failed to start local callback server");
            cancelOAuth();
            return;
        }
    }

    m_oauthRedirectUri = QString("http://localhost:%1/").arg(m_oauthServer->serverPort());

    connect(m_oauthServer, &QTcpServer::newConnection, this, [this]() {
        QTcpSocket *socket = m_oauthServer->nextPendingConnection();
        if (!socket) return;
        socket->waitForReadyRead(3000);
        QByteArray request = socket->readAll();
        QString reqLine = QString::fromUtf8(request).split("\r\n").value(0);
        QString path = reqLine.section(' ', 1, 1);
        QUrl url("http://localhost" + path);
        QUrlQuery q(url);
        QString code = q.queryItemValue("code");
        QString error = q.queryItemValue("error");

        QByteArray body = "<html><body><h3>Authorization completed.</h3>You can close this window.</body></html>";
        QByteArray resp = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body;
        socket->write(resp);
        socket->flush();
        socket->disconnectFromHost();

        if (!error.isEmpty()) {
            emit oauthError(m_oauthProvider, error);
            cancelOAuth();
            return;
        }

        if (code.isEmpty()) {
            emit oauthError(m_oauthProvider, "No authorization code in callback");
            cancelOAuth();
            return;
        }

        // Exchange code for token
        QUrl tokenUrl;
        QUrlQuery bodyQuery;
        if (m_oauthProvider == "gdrive") {
            tokenUrl = QUrl("https://oauth2.googleapis.com/token");
            bodyQuery.addQueryItem("grant_type", "authorization_code");
            bodyQuery.addQueryItem("code", code);
            bodyQuery.addQueryItem("client_id", m_oauthClientId);
            bodyQuery.addQueryItem("redirect_uri", m_oauthRedirectUri);
            bodyQuery.addQueryItem("code_verifier", m_oauthCodeVerifier);
        } else if (m_oauthProvider == "onedrive") {
            tokenUrl = QUrl("https://login.microsoftonline.com/common/oauth2/v2.0/token");
            bodyQuery.addQueryItem("grant_type", "authorization_code");
            bodyQuery.addQueryItem("code", code);
            bodyQuery.addQueryItem("client_id", m_oauthClientId);
            bodyQuery.addQueryItem("redirect_uri", m_oauthRedirectUri);
            bodyQuery.addQueryItem("code_verifier", m_oauthCodeVerifier);
        } else if (m_oauthProvider == "dropbox") {
            tokenUrl = QUrl("https://api.dropbox.com/oauth2/token");
            bodyQuery.addQueryItem("grant_type", "authorization_code");
            bodyQuery.addQueryItem("code", code);
            bodyQuery.addQueryItem("client_id", m_oauthClientId);
            bodyQuery.addQueryItem("redirect_uri", m_oauthRedirectUri);
            bodyQuery.addQueryItem("code_verifier", m_oauthCodeVerifier);
        } else if (m_oauthProvider == "yadisk") {
            tokenUrl = QUrl("https://oauth.yandex.com/token");
            bodyQuery.addQueryItem("grant_type", "authorization_code");
            bodyQuery.addQueryItem("code", code);
            bodyQuery.addQueryItem("client_id", m_oauthClientId);
            bodyQuery.addQueryItem("client_secret", m_oauthClientSecret);
        } else {
            emit oauthError(m_oauthProvider, "Unsupported provider");
            cancelOAuth();
            return;
        }

        QNetworkRequest req(tokenUrl);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply *reply = m_networkManager->post(req, bodyQuery.query(QUrl::FullyEncoded).toUtf8());
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                emit oauthError(m_oauthProvider, reply->errorString());
                reply->deleteLater();
                cancelOAuth();
                return;
            }
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            QString accessToken = obj.value("access_token").toString();
            QString refreshToken = obj.value("refresh_token").toString();
            int expiresIn = obj.value("expires_in").toInt();
            if (accessToken.isEmpty()) {
                emit oauthError(m_oauthProvider, "No access_token in response");
            } else {
                emit oauthCompleted(m_oauthProvider, accessToken, refreshToken, expiresIn);
            }
            reply->deleteLater();
            cancelOAuth();
        });
    });

    QString scope;
    QUrl authUrl;
    QUrlQuery q;
    QString codeChallenge = base64Url(QCryptographicHash::hash(m_oauthCodeVerifier.toUtf8(), QCryptographicHash::Sha256));

    if (provider == "gdrive") {
        authUrl = QUrl("https://accounts.google.com/o/oauth2/v2/auth");
        scope = "https://www.googleapis.com/auth/drive.file";
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", m_oauthRedirectUri);
        q.addQueryItem("scope", scope);
        q.addQueryItem("access_type", "offline");
        q.addQueryItem("prompt", "consent");
        q.addQueryItem("code_challenge", codeChallenge);
        q.addQueryItem("code_challenge_method", "S256");
    } else if (provider == "onedrive") {
        authUrl = QUrl("https://login.microsoftonline.com/common/oauth2/v2.0/authorize");
        scope = "offline_access Files.ReadWrite";
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", m_oauthRedirectUri);
        q.addQueryItem("scope", scope);
        q.addQueryItem("response_mode", "query");
        q.addQueryItem("code_challenge", codeChallenge);
        q.addQueryItem("code_challenge_method", "S256");
    } else if (provider == "dropbox") {
        authUrl = QUrl("https://www.dropbox.com/oauth2/authorize");
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", m_oauthRedirectUri);
        q.addQueryItem("token_access_type", "offline");
        q.addQueryItem("code_challenge", codeChallenge);
        q.addQueryItem("code_challenge_method", "S256");
    } else if (provider == "yadisk") {
        authUrl = QUrl("https://oauth.yandex.com/authorize");
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", m_oauthRedirectUri);
        q.addQueryItem("force_confirm", "yes");
        q.addQueryItem("code_challenge", codeChallenge);
        q.addQueryItem("code_challenge_method", "S256");
    }

    if (!authUrl.isEmpty()) {
        authUrl.setQuery(q);
        emit oauthUrlReady(provider, authUrl.toString());
    } else {
        emit oauthError(provider, "Unsupported provider");
    }
}

void AnalyticsEngine::cancelOAuth()
{
    if (m_oauthServer) {
        m_oauthServer->close();
        m_oauthServer->deleteLater();
        m_oauthServer = nullptr;
    }
    m_oauthProvider.clear();
    m_oauthClientId.clear();
    m_oauthClientSecret.clear();
    m_oauthRedirectUri.clear();
    m_oauthCodeVerifier.clear();
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

void AnalyticsEngine::appendFrameToBuffer(const QString &cameraId, const QImage &frame, qint64 ts)
{
    QMutexLocker locker(&m_bufferMutex);
    QVector<BufferedFrame> &buffer = m_frameBuffers[cameraId];
    buffer.append({frame, ts});

    const qint64 cutoff = ts - (m_evidenceMaxBufferSeconds * 1000);
    while (!buffer.isEmpty() && buffer.first().timestampMs < cutoff) {
        buffer.removeFirst();
    }
}

QVector<AnalyticsEngine::BufferedFrame> AnalyticsEngine::collectFrames(const QString &cameraId, qint64 startMs, qint64 endMs)
{
    QVector<BufferedFrame> result;
    QMutexLocker locker(&m_bufferMutex);
    const auto &buffer = m_frameBuffers.value(cameraId);
    for (const auto &f : buffer) {
        if (f.timestampMs >= startMs && f.timestampMs <= endMs) {
            result.append(f);
        }
    }
    return result;
}

QString AnalyticsEngine::ensureDir(const QString &path)
{
    if (path.isEmpty()) return QString();
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.exists() ? path : QString();
}

QString AnalyticsEngine::buildEvidenceFileName(const QString &cameraId, const QString &suffix) const
{
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz");
    return QString("%1_%2_%3").arg(cameraId.isEmpty() ? "camera" : cameraId, ts, suffix);
}

QString AnalyticsEngine::saveSnapshotImage(const QImage &frame, const QString &cameraId, const QVariantMap &det, const QString &moduleId)
{
    QString safeModule = moduleId;
    if (safeModule.isEmpty()) safeModule = "Unknown";
    safeModule.replace(" ", "_");
    QString dir = ensureDir(QDir(m_evidenceSnapshotsDir).filePath(safeModule));
    if (dir.isEmpty()) return QString();

    QString label = det.value("label").toString();
    QString name = buildEvidenceFileName(cameraId, label.isEmpty() ? "snapshot" : label);
    QImage img = frame;

    const bool isFaceModule = (moduleId.compare("Face Detector", Qt::CaseInsensitive) == 0);
    if (isFaceModule && !img.isNull()) {
        double x = det.value("x").toDouble();
        double y = det.value("y").toDouble();
        double w = det.value("w").toDouble();
        double h = det.value("h").toDouble();
        if (w > 0.0 && h > 0.0) {
            bool normalized = (x <= 1.0 && y <= 1.0 && w <= 1.0 && h <= 1.0);
            if (normalized) {
                x *= img.width();
                y *= img.height();
                w *= img.width();
                h *= img.height();
            }
            QRect rect(qRound(x), qRound(y), qRound(w), qRound(h));
            int pad = qRound(qMin(rect.width(), rect.height()) * 0.15);
            rect.adjust(-pad, -pad, pad, pad);
            rect = rect.intersected(QRect(0, 0, img.width(), img.height()));
            if (rect.isValid()) {
                img = img.copy(rect);
            }
        }
    }

    if (!img.isNull()) {
        if (isFaceModule) {
            QString path = QDir(dir).filePath(name + ".png");
            img.save(path, "PNG");
            return path;
        } else {
            QString path = QDir(dir).filePath(name + ".jpg");
            img.save(path, "JPG", 95);
            return path;
        }
    }
    return QString();
}

void AnalyticsEngine::scheduleClipIfReady(const QString &cameraId, qint64 nowMs)
{
    PendingEvent evt;
    bool hasEvent = false;
    {
        QMutexLocker locker(&m_eventMutex);
        if (m_pendingEvents.contains(cameraId)) {
            evt = m_pendingEvents.value(cameraId);
            hasEvent = true;
            if (nowMs < evt.endMs) {
                return;
            }
            m_pendingEvents.remove(cameraId);
        }
    }

    if (!hasEvent) return;

    if (evt.streamRequested) {
        emit clipStopRequested(cameraId, evt.clipPath);
        return;
    }

    QString dir = ensureDir(m_evidenceClipsDir);
    if (dir.isEmpty()) return;

    QString name = buildEvidenceFileName(cameraId, "clip");
    QString path = QDir(dir).filePath(name + ".mp4");

    QVector<BufferedFrame> frames = collectFrames(cameraId, evt.startMs, evt.endMs);
    if (frames.isEmpty()) return;

    saveClipAsync(cameraId, frames, path);
}

void AnalyticsEngine::saveClipAsync(const QString &cameraId, const QVector<BufferedFrame> &frames, const QString &path)
{
    Q_UNUSED(cameraId)
    (void)QtConcurrent::run([this, frames, path]() {
        if (writeClipMp4(frames, path, m_evidenceClipFps)) {
            if (m_uploadEnabled) {
                enqueueUpload(path);
            }
        } else {
            qWarning() << "Failed to write clip:" << path;
        }
    });
}

bool AnalyticsEngine::writeClipMp4(const QVector<BufferedFrame> &frames, const QString &path, int fps)
{
    if (frames.isEmpty()) return false;
    ensureGstInitOnce();

    if (!gst_element_factory_find("mp4mux")) {
        qWarning() << "mp4mux not available in GStreamer install";
        return false;
    }

    QString encoderPipeline;
    if (gst_element_factory_find("x264enc")) {
        encoderPipeline = QString("x264enc tune=zerolatency speed-preset=ultrafast key-int-max=%1").arg(qMax(1, fps));
    } else if (gst_element_factory_find("openh264enc")) {
        encoderPipeline = "openh264enc";
    } else if (gst_element_factory_find("avenc_h264")) {
        encoderPipeline = "avenc_h264";
    } else {
        qWarning() << "No H.264 encoder available (x264enc/openh264enc/avenc_h264)";
        return false;
    }

    GError *error = nullptr;
    QString safePath = path;
    safePath.replace("\\", "/");
    QString pipelineDesc = QString("appsrc name=src is-live=false format=time ! videoconvert ! video/x-raw,format=I420 ! %1 ! h264parse ! mp4mux ! filesink location=\"%2\"")
        .arg(encoderPipeline)
        .arg(safePath);

    GstElement *pipeline = gst_parse_launch(pipelineDesc.toUtf8().constData(), &error);
    if (error) {
        qWarning() << "Clip pipeline error:" << error->message;
        g_error_free(error);
        return false;
    }

    GstElement *src = gst_bin_get_by_name(GST_BIN(pipeline), "src");
    if (!src) {
        gst_object_unref(pipeline);
        return false;
    }

    const QImage &first = frames.first().frame;
    int width = first.width();
    int height = first.height();
    qint64 durationMs = frames.last().timestampMs - frames.first().timestampMs;
    int estimatedFps = fps;
    if (durationMs > 0 && frames.size() > 1) {
        double f = (frames.size() - 1) * 1000.0 / (double)durationMs;
        estimatedFps = qBound(1, (int)qRound(f), 60);
    }
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "format", G_TYPE_STRING, "RGBA",
                                        "width", G_TYPE_INT, width,
                                        "height", G_TYPE_INT, height,
                                        "framerate", GST_TYPE_FRACTION, estimatedFps, 1,
                                        NULL);
    gst_app_src_set_caps(GST_APP_SRC(src), caps);
    gst_caps_unref(caps);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    GstClockTime baseTs = (frames.first().timestampMs > 0) ? (GstClockTime)frames.first().timestampMs * GST_MSECOND : 0;
    GstClockTime frameDuration = gst_util_uint64_scale_int(1, GST_SECOND, qMax(1, estimatedFps));

    for (int i = 0; i < frames.size(); ++i) {
        const auto &f = frames[i];
        QImage img = f.frame;
        if (img.isNull()) continue;
        if (img.format() != QImage::Format_RGBA8888) {
            img = img.convertToFormat(QImage::Format_RGBA8888);
        }

        GstBuffer *buffer = gst_buffer_new_allocate(nullptr, img.sizeInBytes(), nullptr);
        GstMapInfo map;
        gst_buffer_map(buffer, &map, GST_MAP_WRITE);
        memcpy(map.data, img.bits(), img.sizeInBytes());
        gst_buffer_unmap(buffer, &map);

        if (f.timestampMs > 0 && baseTs > 0) {
            GST_BUFFER_PTS(buffer) = ((GstClockTime)f.timestampMs * GST_MSECOND) - baseTs;
        } else {
            GST_BUFFER_PTS(buffer) = (GstClockTime)i * frameDuration;
        }

        if (i + 1 < frames.size() && frames[i + 1].timestampMs > 0 && f.timestampMs > 0) {
            qint64 deltaMs = frames[i + 1].timestampMs - f.timestampMs;
            GST_BUFFER_DURATION(buffer) = (GstClockTime)qMax<qint64>(1, deltaMs) * GST_MSECOND;
        } else {
            GST_BUFFER_DURATION(buffer) = frameDuration;
        }

        GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(src), buffer);
        if (ret != GST_FLOW_OK) {
            qWarning() << "appsrc push failed" << ret;
            break;
        }
    }

    gst_app_src_end_of_stream(GST_APP_SRC(src));

    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, 10 * GST_SECOND, (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
    if (msg) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err = nullptr;
            gchar *debug = nullptr;
            gst_message_parse_error(msg, &err, &debug);
            qWarning() << "Clip pipeline runtime error:" << (err ? err->message : "unknown")
                       << "debug:" << (debug ? debug : "");
            if (err) g_error_free(err);
            if (debug) g_free(debug);
        }
        gst_message_unref(msg);
    } else {
        qWarning() << "Clip pipeline timeout waiting for EOS";
    }
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(src);
    gst_object_unref(pipeline);

    QFileInfo fi(path);
    return fi.exists() && fi.size() > 0;
}

void AnalyticsEngine::enqueueUpload(const QString &filePath)
{
    if (filePath.isEmpty()) return;
    if (!m_uploadEnabled || m_uploadTarget.isEmpty()) return;

    QMutexLocker locker(&m_uploadMutex);
    m_uploadQueue.enqueue({filePath, m_uploadProvider, m_uploadTarget});
    if (!m_uploadActive) {
        m_uploadActive = true;
        QMetaObject::invokeMethod(this, [this]() { processNextUpload(); }, Qt::QueuedConnection);
    }
}

void AnalyticsEngine::processNextUpload()
{
    UploadTask task;
    {
        QMutexLocker locker(&m_uploadMutex);
        if (m_uploadQueue.isEmpty()) {
            m_uploadActive = false;
            return;
        }
        task = m_uploadQueue.dequeue();
    }

    (void)QtConcurrent::run([this, task]() {
        auto parseParams = [](const QString &s) {
            QMap<QString, QString> params;
            const QStringList parts = s.split(";", Qt::SkipEmptyParts);
            for (const QString &part : parts) {
                const int idx = part.indexOf('=');
                if (idx > 0) {
                    QString key = part.left(idx).trimmed().toLower();
                    QString val = part.mid(idx + 1).trimmed();
                    params.insert(key, val);
                }
            }
            return params;
        };

        auto ftpUpload = [](const QString &filePath, const QString &target) {
            QUrl url(target);
            if (!url.isValid() || url.scheme().isEmpty()) {
                return false;
            }

            QString path = url.path();
            if (path.isEmpty() || path.endsWith("/")) {
                if (!path.endsWith("/")) path += "/";
                path += QFileInfo(filePath).fileName();
                url.setPath(path);
            }

            QStringList args;
            args << "-s" << "-S" << "--fail" << "--ftp-create-dirs" << "-T" << filePath << url.toString();

            QProcess proc;
            proc.start("curl", args);
            if (!proc.waitForFinished(120000)) {
                proc.kill();
                return false;
            }
            return proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0;
        };

        auto httpUpload = [](QNetworkReply *reply) {
            QEventLoop loop;
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
            bool ok = reply->error() == QNetworkReply::NoError;
            reply->deleteLater();
            return ok;
        };

        auto httpFetch = [](QNetworkReply *reply, QByteArray &out) {
            QEventLoop loop;
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
            bool ok = reply->error() == QNetworkReply::NoError;
            out = reply->readAll();
            reply->deleteLater();
            return ok;
        };

        bool ok = false;
        if (task.provider == "local") {
            QDir dir(task.target);
            if (!dir.exists()) dir.mkpath(".");
            QString dest = dir.filePath(QFileInfo(task.filePath).fileName());
            ok = QFile::copy(task.filePath, dest);
        } else if (task.provider == "ftp") {
            ok = ftpUpload(task.filePath, task.target);
        } else if (task.provider == "gdrive") {
            QMap<QString, QString> params = parseParams(task.target);
            QString token = params.value("token");
            if (token.isEmpty()) token = params.value("access_token");
            if (token.isEmpty()) token = m_uploadAccessToken;
            QString folderId = params.value("folder");
            if (!token.isEmpty()) {
                QNetworkAccessManager mgr;
                QNetworkRequest req(QUrl("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart"));
                req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

                QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);

                QHttpPart metaPart;
                metaPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
                QJsonObject meta;
                meta.insert("name", QFileInfo(task.filePath).fileName());
                if (!folderId.isEmpty()) {
                    QJsonArray parents;
                    parents.append(folderId);
                    meta.insert("parents", parents);
                }
                metaPart.setBody(QJsonDocument(meta).toJson(QJsonDocument::Compact));

                QHttpPart filePart;
                filePart.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
                QFile *file = new QFile(task.filePath);
                if (file->open(QIODevice::ReadOnly)) {
                    filePart.setBodyDevice(file);
                    file->setParent(multiPart);
                    multiPart->append(metaPart);
                    multiPart->append(filePart);

                    QNetworkReply *reply = mgr.post(req, multiPart);
                    multiPart->setParent(reply);
                    ok = httpUpload(reply);
                } else {
                    delete file;
                    delete multiPart;
                }
            }
        } else if (task.provider == "onedrive") {
            QMap<QString, QString> params = parseParams(task.target);
            QString token = params.value("token");
            if (token.isEmpty()) token = params.value("access_token");
            if (token.isEmpty()) token = m_uploadAccessToken;
            QString folderPath = params.value("path");
            if (folderPath.isEmpty()) folderPath = params.value("folder");
            if (!token.isEmpty()) {
                QString fileName = QFileInfo(task.filePath).fileName();
                QString path = folderPath;
                if (!path.isEmpty() && !path.startsWith("/")) path.prepend("/");
                if (!path.isEmpty() && path.endsWith("/")) path.chop(1);
                QString fullPath = path.isEmpty() ? ("/" + fileName) : (path + "/" + fileName);
                QUrl url("https://graph.microsoft.com/v1.0/me/drive/root:" + fullPath + ":/content");

                QNetworkAccessManager mgr;
                QNetworkRequest req(url);
                req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

                QFile *file = new QFile(task.filePath);
                if (file->open(QIODevice::ReadOnly)) {
                    QNetworkReply *reply = mgr.put(req, file);
                    file->setParent(reply);
                    ok = httpUpload(reply);
                } else {
                    delete file;
                }
            }
        } else if (task.provider == "dropbox") {
            QMap<QString, QString> params = parseParams(task.target);
            QString token = params.value("token");
            if (token.isEmpty()) token = m_uploadAccessToken;
            QString path = params.value("path");
            if (path.isEmpty()) path = "/OpenIPC";
            if (!path.startsWith("/")) path.prepend("/");
            QString fullPath = path;
            if (!fullPath.endsWith("/")) fullPath += "/";
            fullPath += QFileInfo(task.filePath).fileName();
            if (!token.isEmpty()) {
                QNetworkAccessManager mgr;
                QNetworkRequest req(QUrl("https://content.dropboxapi.com/2/files/upload"));
                req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
                QJsonObject arg;
                arg.insert("path", fullPath);
                arg.insert("mode", "add");
                arg.insert("autorename", true);
                arg.insert("mute", false);
                req.setRawHeader("Dropbox-API-Arg", QJsonDocument(arg).toJson(QJsonDocument::Compact));
                req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

                QFile *file = new QFile(task.filePath);
                if (file->open(QIODevice::ReadOnly)) {
                    QNetworkReply *reply = mgr.post(req, file);
                    file->setParent(reply);
                    ok = httpUpload(reply);
                } else {
                    delete file;
                }
            }
        } else if (task.provider == "yadisk") {
            QMap<QString, QString> params = parseParams(task.target);
            QString token = params.value("token");
            if (token.isEmpty()) token = m_uploadAccessToken;
            QString path = params.value("path");
            if (path.isEmpty()) path = "/OpenIPC";
            if (!path.startsWith("/")) path.prepend("/");
            QString fullPath = path;
            if (!fullPath.endsWith("/")) fullPath += "/";
            fullPath += QFileInfo(task.filePath).fileName();
            if (!token.isEmpty()) {
                QNetworkAccessManager mgr;
                QUrlQuery q;
                q.addQueryItem("path", fullPath);
                q.addQueryItem("overwrite", "true");
                QUrl reqUrl("https://cloud-api.yandex.net/v1/disk/resources/upload");
                reqUrl.setQuery(q);
                QNetworkRequest req(reqUrl);
                req.setRawHeader("Authorization", ("OAuth " + token).toUtf8());
                QNetworkReply *reply = mgr.get(req);
                QByteArray payload;
                if (httpFetch(reply, payload)) {
                    QJsonDocument doc = QJsonDocument::fromJson(payload);
                    QJsonObject obj = doc.object();
                    QUrl uploadUrl(obj.value("href").toString());
                    if (uploadUrl.isValid()) {
                        QNetworkRequest putReq(uploadUrl);
                        QFile *file = new QFile(task.filePath);
                        if (file->open(QIODevice::ReadOnly)) {
                            QNetworkReply *putReply = mgr.put(putReq, file);
                            file->setParent(putReply);
                            ok = httpUpload(putReply);
                        } else {
                            delete file;
                        }
                    }
                }
            }
        } else {
            qWarning() << "Upload provider not implemented:" << task.provider;
        }

        QMetaObject::invokeMethod(this, [this, ok]() {
            Q_UNUSED(ok)
            processNextUpload();
        }, Qt::QueuedConnection);
    });
}
