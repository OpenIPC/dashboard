#include "AnalyticsEngine.h"
#include "YoloDetector.h"
#include "../PathUtils.h"
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
#include <QElapsedTimer>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTcpSocket>
#include <keychain.h>
#include <gst/app/gstappsrc.h>
#include <algorithm>

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
    m_eventStorePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/analytics_events.sqlite";
    m_eventStoreConnectionName = QStringLiteral("analytics_events_%1").arg(reinterpret_cast<quintptr>(this));
    initEventStore();
    m_analyticsEvents = loadRecentAnalyticsEvents(m_maxAnalyticsEvents);
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

    if (!m_eventStoreConnectionName.isEmpty() && QSqlDatabase::contains(m_eventStoreConnectionName)) {
        {
            QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(m_eventStoreConnectionName);
    }
}

QVariantList AnalyticsEngine::analyticsEvents() const
{
    return m_analyticsEvents;
}

QVariantList AnalyticsEngine::queryAnalyticsEvents(int type, const QString &cameraId, const QString &text, int limit) const
{
    const QString trimmedCameraId = cameraId.trimmed();
    const QString trimmedText = text.trimmed();
    const int safeLimit = qBound(1, limit, 5000);

    {
        QMutexLocker locker(&m_eventStoreMutex);
        if (m_eventStoreReady && QSqlDatabase::contains(m_eventStoreConnectionName)) {
            QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
            if (db.isOpen()) {
                QSqlQuery query(db);
                query.prepare(
                    "SELECT payload_json FROM analytics_events "
                    "WHERE (? < 0 OR module_type = ?) "
                    "AND (? = '' OR camera_id = ?) "
                    "AND (? = '' OR label LIKE ? OR message LIKE ? OR rule_name LIKE ? OR event_type LIKE ?) "
                    "ORDER BY timestamp_ms DESC, rowid DESC LIMIT ?"
                );

                const QString likeText = "%" + trimmedText + "%";
                query.addBindValue(type);
                query.addBindValue(type);
                query.addBindValue(trimmedCameraId);
                query.addBindValue(trimmedCameraId);
                query.addBindValue(trimmedText);
                query.addBindValue(likeText);
                query.addBindValue(likeText);
                query.addBindValue(likeText);
                query.addBindValue(likeText);
                query.addBindValue(safeLimit);

                QVariantList result;
                if (!query.exec()) {
                    qWarning() << "Failed to query analytics events:" << query.lastError().text();
                    return result;
                }

                while (query.next()) {
                    const QJsonDocument doc = QJsonDocument::fromJson(query.value(0).toString().toUtf8());
                    if (doc.isObject()) {
                        result.append(doc.object().toVariantMap());
                    }
                }
                return result;
            }
        }
    }

    QVariantList filtered;
    for (const QVariant &eventVar : m_analyticsEvents) {
        const QVariantMap event = eventVar.toMap();
        if (type >= 0 && event.value("moduleType").toInt() != type) {
            continue;
        }
        if (!trimmedCameraId.isEmpty() && event.value("cameraId").toString() != trimmedCameraId) {
            continue;
        }
        if (!trimmedText.isEmpty()) {
            const QString haystack = QStringList{
                event.value("label").toString(),
                event.value("message").toString(),
                event.value("ruleName").toString(),
                event.value("eventType").toString()
            }.join(" ");
            if (!haystack.contains(trimmedText, Qt::CaseInsensitive)) {
                continue;
            }
        }
        filtered.append(event);
        if (filtered.size() >= safeLimit) {
            break;
        }
    }
    return filtered;
}

void AnalyticsEngine::clearAnalyticsEvents(int type, const QString &cameraId)
{
    const QString trimmedCameraId = cameraId.trimmed();
    if (type < 0 && trimmedCameraId.isEmpty()) {
        if (m_analyticsEvents.isEmpty()) {
            deleteStoredAnalyticsEvents(type, trimmedCameraId);
            return;
        }
        m_analyticsEvents.clear();
        deleteStoredAnalyticsEvents(type, trimmedCameraId);
        emit analyticsEventsChanged();
        return;
    }

    QVariantList filtered;
    bool changed = false;

    for (const QVariant &eventVar : m_analyticsEvents) {
        const QVariantMap event = eventVar.toMap();
        bool matches = true;

        if (type >= 0) {
            matches = matches && (event.value("moduleType").toInt() == type);
        }

        if (!trimmedCameraId.isEmpty()) {
            matches = matches && (event.value("cameraId").toString() == trimmedCameraId);
        }

        if (matches) {
            changed = true;
            continue;
        }

        filtered.append(event);
    }

    if (!changed) {
        return;
    }

    m_analyticsEvents = filtered;
    deleteStoredAnalyticsEvents(type, trimmedCameraId);
    emit analyticsEventsChanged();
}

QVariantMap AnalyticsEngine::telemetryStateToVariant(const TelemetryState &state) const
{
    QVariantMap result;
    result["processedFrames"] = static_cast<qlonglong>(state.processedFrames);
    result["skippedFrames"] = static_cast<qlonglong>(state.skippedFrames);
    result["detections"] = static_cast<qlonglong>(state.detections);
    result["events"] = static_cast<qlonglong>(state.events);
    result["lastInferenceMs"] = state.lastInferenceMs;
    result["averageInferenceMs"] = state.processedFrames > 0
        ? (state.totalInferenceMs / static_cast<double>(state.processedFrames))
        : 0.0;
    result["lastProcessedMs"] = static_cast<qlonglong>(state.lastProcessedMs);
    result["lastSkippedMs"] = static_cast<qlonglong>(state.lastSkippedMs);
    result["lastDetectionMs"] = static_cast<qlonglong>(state.lastDetectionMs);
    result["lastEventMs"] = static_cast<qlonglong>(state.lastEventMs);
    return result;
}

QString AnalyticsEngine::countsToText(const QMap<QString, int> &counts) const
{
    QStringList parts;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        parts.append(QString("%1: %2").arg(it.key(), QString::number(it.value())));
    }
    return parts.join(", ");
}

int AnalyticsEngine::analyticsFrameIntervalMs() const
{
    const int targetFps = qBound(1, m_analyticsTargetFps, 30);
    return qMax(1, qRound(1000.0 / static_cast<double>(targetFps)));
}

QVariantMap AnalyticsEngine::analyticsDiagnostics() const
{
    QVariantMap result;
    QVariantList cameraStats;
    QVariantList moduleStats;

    quint64 processedFrames = 0;
    quint64 skippedFrames = 0;
    quint64 detections = 0;
    quint64 events = 0;
    double totalInferenceMs = 0.0;
    quint64 inferenceSamples = 0;
    QSet<QString> cameraIds;

    {
        QMutexLocker locker(&m_telemetryMutex);

        for (auto it = m_cameraTelemetry.begin(); it != m_cameraTelemetry.end(); ++it) {
            cameraIds.insert(it.key());
            processedFrames += it.value().processedFrames;
            skippedFrames += it.value().skippedFrames;
            detections += it.value().detections;
            events += it.value().events;
            totalInferenceMs += it.value().totalInferenceMs;
            inferenceSamples += it.value().processedFrames;
        }

        for (auto it = m_moduleTelemetry.begin(); it != m_moduleTelemetry.end(); ++it) {
            QVariantMap entry = telemetryStateToVariant(it.value());
            entry["moduleType"] = static_cast<int>(it.key());
            entry["moduleName"] = m_modules.contains(it.key()) ? m_modules[it.key()].name : QString::number(static_cast<int>(it.key()));
            moduleStats.append(entry);
        }
    }

    for (auto cameraIt = m_cameraModules.begin(); cameraIt != m_cameraModules.end(); ++cameraIt) {
        cameraIds.insert(cameraIt.key());
    }

    QSet<QString> processingCameras;
    QMap<QString, qint64> lastAcceptedFrames;
    {
        QMutexLocker locker(&m_processingMutex);
        processingCameras = m_processingCameras;
        lastAcceptedFrames = m_lastAcceptedFrameMs;
    }

    QStringList sortedCameraIds = cameraIds.values();
    std::sort(sortedCameraIds.begin(), sortedCameraIds.end());
    for (const QString &cameraId : sortedCameraIds) {
        QVariantMap entry;
        {
            QMutexLocker locker(&m_telemetryMutex);
            entry = telemetryStateToVariant(m_cameraTelemetry.value(cameraId));
        }

        QVariantList moduleAssignments;
        int assignedModules = 0;
        int readyModules = 0;
        int activeModules = 0;
        int configuredRules = 0;
        const QMap<ModuleType, bool> assignments = m_cameraModules.value(cameraId);

        for (int moduleTypeValue = static_cast<int>(FaceDetector);
             moduleTypeValue <= static_cast<int>(LicensePlate);
             ++moduleTypeValue) {
            const ModuleType moduleType = static_cast<ModuleType>(moduleTypeValue);
            if (!assignments.value(moduleType, false)) {
                continue;
            }

            assignedModules += 1;
            QVariantMap moduleState;
            moduleState["moduleType"] = moduleTypeValue;

            if (m_modules.contains(moduleType)) {
                const ModuleContext &ctx = m_modules[moduleType];
                const bool ready = ctx.status == "ready";
                const bool active = ctx.enabled && ready;
                int enabledRules = 0;
                const QVariantList rules = ctx.extraConfig.value("rules").toList();
                for (const QVariant &ruleVar : rules) {
                    const QVariantMap rule = ruleVar.toMap();
                    if (rule.value("enabled", true).toBool()) {
                        enabledRules += 1;
                    }
                }

                if (ready) {
                    readyModules += 1;
                }
                if (active) {
                    activeModules += 1;
                }
                configuredRules += enabledRules;

                moduleState["moduleName"] = ctx.name;
                moduleState["enabled"] = ctx.enabled;
                moduleState["ready"] = ready;
                moduleState["active"] = active;
                moduleState["status"] = ctx.status;
                moduleState["rules"] = enabledRules;
            } else {
                moduleState["moduleName"] = QString::number(moduleTypeValue);
                moduleState["enabled"] = false;
                moduleState["ready"] = false;
                moduleState["active"] = false;
                moduleState["status"] = QStringLiteral("not_installed");
                moduleState["rules"] = 0;
            }

            moduleAssignments.append(moduleState);
        }

        const bool isProcessing = processingCameras.contains(cameraId);
        const quint64 processed = entry.value("processedFrames").toULongLong();
        const quint64 skipped = entry.value("skippedFrames").toULongLong();
        QString pipelineState = QStringLiteral("unassigned");
        if (assignedModules > 0 && activeModules == 0) {
            pipelineState = QStringLiteral("module_not_ready");
        } else if (isProcessing) {
            pipelineState = QStringLiteral("processing");
        } else if (processed > 0) {
            pipelineState = QStringLiteral("receiving");
        } else if (skipped > 0) {
            pipelineState = QStringLiteral("throttled");
        } else if (activeModules > 0) {
            pipelineState = QStringLiteral("waiting");
        }

        entry["cameraId"] = cameraId;
        entry["assignedModules"] = assignedModules;
        entry["readyModules"] = readyModules;
        entry["activeModules"] = activeModules;
        entry["configuredRules"] = configuredRules;
        entry["moduleAssignments"] = moduleAssignments;
        entry["isProcessing"] = isProcessing;
        entry["lastAcceptedFrameMs"] = static_cast<qlonglong>(lastAcceptedFrames.value(cameraId, 0));
        entry["pipelineState"] = pipelineState;
        cameraStats.append(entry);
    }

    result["processedFrames"] = static_cast<qlonglong>(processedFrames);
    result["skippedFrames"] = static_cast<qlonglong>(skippedFrames);
    result["detections"] = static_cast<qlonglong>(detections);
    result["events"] = static_cast<qlonglong>(events);
    result["averageInferenceMs"] = inferenceSamples > 0
        ? (totalInferenceMs / static_cast<double>(inferenceSamples))
        : 0.0;
    result["cameraStats"] = cameraStats;
    result["moduleStats"] = moduleStats;
    result["eventBufferSize"] = m_analyticsEvents.size();
    result["eventStoreReady"] = m_eventStoreReady;
    result["eventStorePath"] = m_eventStorePath;
    result["objectCounterSummary"] = getObjectCounterSummary();
    result["analyticsPerformancePreset"] = m_analyticsPerformancePreset;
    result["analyticsTargetFps"] = m_analyticsTargetFps;
    result["analyticsFrameIntervalMs"] = analyticsFrameIntervalMs();
    result["analyticsMaxParallelJobs"] = m_analyticsMaxParallelJobs;

    {
        QMutexLocker locker(&m_counterMutex);
        int activeTracks = 0;
        for (auto it = m_counterStates.begin(); it != m_counterStates.end(); ++it) {
            activeTracks += it.value().tracks.size();
        }
        result["activeTracks"] = activeTracks;
    }

    {
        QMutexLocker locker(&m_uploadMutex);
        result["uploadQueueDepth"] = m_uploadQueue.size() + (m_uploadActive ? 1 : 0);
    }

    result["analyticsActiveJobs"] = processingCameras.size();

    return result;
}

QVariantMap AnalyticsEngine::getModuleTelemetry(int type) const
{
    const ModuleType moduleType = static_cast<ModuleType>(type);
    QVariantMap result;

    {
        QMutexLocker locker(&m_telemetryMutex);
        result = telemetryStateToVariant(m_moduleTelemetry.value(moduleType));
    }

    result["moduleType"] = type;
    result["moduleName"] = m_modules.contains(moduleType) ? m_modules[moduleType].name : QString::number(type);

    if (moduleType == ObjectCounter) {
        QVariantMap countsMap;
        QVariantList cameras;
        QMap<QString, int> aggregateCounts;
        int activeTracks = 0;
        int totalUniqueObjects = 0;

        QMutexLocker locker(&m_counterMutex);
        for (auto it = m_counterStates.begin(); it != m_counterStates.end(); ++it) {
            QVariantMap cameraEntry;
            cameraEntry["cameraId"] = it.key();
            cameraEntry["activeTracks"] = it.value().tracks.size();
            cameraEntry["countsText"] = countsToText(it.value().totalCountByLabel);

            QVariantMap perCameraCounts;
            int perCameraTotal = 0;
            for (auto countIt = it.value().totalCountByLabel.begin(); countIt != it.value().totalCountByLabel.end(); ++countIt) {
                perCameraCounts.insert(countIt.key(), countIt.value());
                aggregateCounts[countIt.key()] += countIt.value();
                perCameraTotal += countIt.value();
            }

            cameraEntry["counts"] = perCameraCounts;
            cameraEntry["totalUniqueObjects"] = perCameraTotal;
            cameras.append(cameraEntry);

            activeTracks += it.value().tracks.size();
            totalUniqueObjects += perCameraTotal;
        }

        for (auto it = aggregateCounts.begin(); it != aggregateCounts.end(); ++it) {
            countsMap.insert(it.key(), it.value());
        }

        result["activeTracks"] = activeTracks;
        result["counts"] = countsMap;
        result["countsText"] = countsToText(aggregateCounts);
        result["totalUniqueObjects"] = totalUniqueObjects;
        result["cameras"] = cameras;
    } else {
        result["activeTracks"] = 0;
        result["counts"] = QVariantMap();
        result["countsText"] = QString();
        result["totalUniqueObjects"] = 0;
        result["cameras"] = QVariantList();
    }

    return result;
}

QVariantList AnalyticsEngine::getObjectCounterSummary() const
{
    QVariantList result;
    QMutexLocker locker(&m_counterMutex);

    for (auto it = m_counterStates.begin(); it != m_counterStates.end(); ++it) {
        QVariantMap countsMap;
        int totalUniqueObjects = 0;

        for (auto countIt = it.value().totalCountByLabel.begin(); countIt != it.value().totalCountByLabel.end(); ++countIt) {
            countsMap.insert(countIt.key(), countIt.value());
            totalUniqueObjects += countIt.value();
        }

        QVariantMap entry;
        entry["cameraId"] = it.key();
        entry["activeTracks"] = it.value().tracks.size();
        entry["counts"] = countsMap;
        entry["countsText"] = countsToText(it.value().totalCountByLabel);
        entry["totalUniqueObjects"] = totalUniqueObjects;
        result.append(entry);
    }

    return result;
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

QVariantMap AnalyticsEngine::getModuleDiagnostics(int type) const
{
    const ModuleType moduleType = static_cast<ModuleType>(type);
    QVariantMap result;
    result["moduleType"] = type;

    if (!m_modules.contains(moduleType)) {
        result["status"] = QStringLiteral("error");
        result["error"] = QStringLiteral("Unknown module");
        result["installed"] = false;
        result["loaded"] = false;
        return result;
    }

    const ModuleContext &ctx = m_modules[moduleType];
    const QString modelPath = QDir(m_modulesDir).filePath(ctx.modelFileName);
    const QFileInfo modelInfo(modelPath);

    result["moduleName"] = ctx.name;
    result["description"] = ctx.description;
    result["version"] = ctx.version;
    result["enabled"] = ctx.enabled;
    result["status"] = ctx.status;
    result["progress"] = ctx.progress;
    result["error"] = ctx.error;
    result["modelFileName"] = ctx.modelFileName;
    result["modelPath"] = modelPath;
    result["modulesDir"] = m_modulesDir;
    result["modelUrl"] = ctx.modelUrl;
    result["installed"] = modelInfo.exists() && modelInfo.isFile();
    result["modelSizeBytes"] = modelInfo.exists() ? static_cast<qlonglong>(modelInfo.size()) : 0;
    result["lastModifiedMs"] = modelInfo.exists() ? modelInfo.lastModified().toMSecsSinceEpoch() : 0;
    result["loaded"] = ctx.backend && ctx.backend->isLoaded();
    result["confidenceThreshold"] = ctx.options.confidenceThreshold;
    result["nmsThreshold"] = ctx.options.nmsThreshold;
    result["classCount"] = ctx.options.classLabels.size();
    result["classes"] = ctx.options.classLabels;

    return result;
}

void AnalyticsEngine::cancelModuleDownload(ModuleType type)
{
    if (m_currentDownloads.contains(type)) {
        QNetworkReply *reply = m_currentDownloads.take(type);
        if (reply) {
            reply->disconnect(this);
            if (reply->isRunning()) {
                reply->abort();
            }
            reply->deleteLater();
        }
    }

    if (m_downloadFiles.contains(type)) {
        QFile *file = m_downloadFiles.take(type);
        const QString partialPath = file ? file->fileName() : QString();
        if (file) {
            file->close();
            delete file;
        }
        if (!partialPath.isEmpty()) {
            QFile::remove(partialPath);
        }
    }
}

void AnalyticsEngine::reloadModule(int type)
{
    const ModuleType moduleType = static_cast<ModuleType>(type);
    if (!m_modules.contains(moduleType)) {
        return;
    }

    cancelModuleDownload(moduleType);

    ModuleContext &ctx = m_modules[moduleType];
    const QString modelPath = QDir(m_modulesDir).filePath(ctx.modelFileName);
    ctx.enabled = false;
    ctx.backend.reset();
    ctx.error.clear();
    ctx.status = "downloading";
    ctx.progress = 0.0f;

    if (QFileInfo::exists(modelPath) && !QFile::remove(modelPath)) {
        ctx.status = "error";
        ctx.progress = 0.0f;
        ctx.error = QStringLiteral("Failed to remove model file: %1").arg(modelPath);
        emit moduleStatusChanged(moduleType, ctx.status, ctx.progress, ctx.error);
        emit analyticsTelemetryChanged();
        return;
    }

    ctx.backend = std::make_shared<YoloDetector>(ctx.options);
    emit moduleStatusChanged(moduleType, ctx.status, ctx.progress, ctx.error);
    emit analyticsTelemetryChanged();
    startDownload(moduleType);
}

void AnalyticsEngine::startDownload(ModuleType type)
{
    qInfo() << "startDownload" << type;
    fprintf(stderr, "AnalyticsEngine::startDownload type=%d\n", (int)type);
    
    if (!m_modules.contains(type)) return;

    cancelModuleDownload(type);
    
    ModuleContext &ctx = m_modules[type];
    ctx.backend = std::make_shared<YoloDetector>(ctx.options);
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

void AnalyticsEngine::downloadFile(const QString &url, const QString &filePath, ModuleType type,
                                   bool isRuntime, int retryCount)
{
    qInfo() << "downloadFile" << url << "to" << filePath;
    fprintf(stderr, "AnalyticsEngine::downloadFile url=%s path=%s\n", qPrintable(url), qPrintable(filePath));

    constexpr int kTransferTimeoutMs = 20000;
    constexpr int kMaxDownloadRetries = 2;

    QNetworkRequest request{QUrl(url)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    // Qt 6.4 can occasionally leave HTTP/2 GitHub downloads waiting forever on Windows.
    // HTTP/1.1 is more reliable here because each module is downloaded as one large file.
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(kTransferTimeoutMs);
    request.setRawHeader("User-Agent", "OpenIPC-Dashboard/0.2");
    request.setRawHeader("Accept", "application/octet-stream");

    QNetworkReply *reply = m_networkManager->get(request);
    m_currentDownloads[type] = reply;
    
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        m_modules[type].status = "error";
        m_modules[type].error = "Failed to open file for writing: " + filePath;
        emit moduleStatusChanged(type, m_modules[type].status, 0.0f, m_modules[type].error);
        if (m_currentDownloads.value(type) == reply) {
            m_currentDownloads.remove(type);
        }
        delete file;
        reply->abort();
        reply->deleteLater();
        return;
    }
    m_downloadFiles[type] = file;

    connect(reply, &QNetworkReply::readyRead, this, [this, type, reply]() {
        if (m_currentDownloads.value(type) != reply) {
            return;
        }
        if (m_downloadFiles.contains(type)) {
            m_downloadFiles[type]->write(reply->readAll());
        }
    });

    connect(reply, &QNetworkReply::metaDataChanged, this, [type, reply]() {
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const qint64 contentLength = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        qInfo() << "Model download response" << type
                << "HTTP" << statusCode
                << "contentLength" << contentLength
                << "url" << reply->url();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [type, reply](QNetworkReply::NetworkError error) {
        qWarning() << "Model download network error" << type << error << reply->errorString();
    });

    connect(reply, &QNetworkReply::downloadProgress, this, [this, type, isRuntime, reply](qint64 bytesReceived, qint64 bytesTotal) {
        if (m_currentDownloads.value(type) != reply) {
            return;
        }
        // qInfo() << "Progress" << type << bytesReceived << "/" << bytesTotal;
        if (m_modules.contains(type)) {
            float progress = 0.35f;
            if (bytesTotal > 0) {
                progress = static_cast<float>(bytesReceived) / static_cast<float>(bytesTotal);
                progress = std::clamp(progress, 0.0f, 0.99f);
            }

            ModuleContext &ctx = m_modules[type];
            if (isRuntime) {
                ctx.progress = std::min(progress * 0.5f, 0.49f);
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
        QStringList errorMessages;
        qWarning() << "SSL Errors:";
        for (const auto &error : errors) {
            qWarning() << error.errorString();
            errorMessages.append(error.errorString());
        }
        reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this,
            [this, type, isRuntime, retryCount, url, reply, filePath]() {
        if (m_currentDownloads.value(type) != reply) {
            reply->deleteLater();
            return;
        }

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
            QFile::remove(filePath);

            if (retryCount < kMaxDownloadRetries &&
                reply->error() != QNetworkReply::OperationCanceledError) {
                if (m_modules.contains(type)) {
                    ModuleContext &ctx = m_modules[type];
                    ctx.status = "downloading";
                    ctx.progress = 0.0f;
                    ctx.error.clear();
                    emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
                }

                const int nextRetry = retryCount + 1;
                const int retryDelayMs = 1000 * nextRetry;
                qInfo() << "Retrying model download" << type
                        << "attempt" << nextRetry
                        << "in" << retryDelayMs << "ms";
                QTimer::singleShot(retryDelayMs, this,
                    [this, type, isRuntime, nextRetry, url, filePath]() {
                        if (!m_currentDownloads.contains(type) && m_modules.contains(type)) {
                            downloadFile(url, filePath, type, isRuntime, nextRetry);
                        }
                    });
                reply->deleteLater();
                return;
            }

            if (m_modules.contains(type)) {
                ModuleContext &ctx = m_modules[type];
                ctx.status = "error";
                ctx.progress = 0.0f;
                ctx.error = reply->errorString();
                emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
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
                downloadFile(newUrl.toString(),
                             QDir(m_modulesDir).filePath(isRuntime ? "onnxruntime.dll" : m_modules[type].modelFileName),
                             type, isRuntime, retryCount);
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
                    ctx.progress = 0.0f;
                    ctx.error = "Failed to load model after download";
                    qWarning() << "Failed to load model";
                }
                emit moduleStatusChanged(type, ctx.status, ctx.progress, ctx.error);
            }
        }
        
        reply->deleteLater();
    });
}

QVariantMap AnalyticsEngine::detectionToVariant(const DetectionBox &box, const QString &moduleId, ModuleType moduleType) const
{
    QVariantMap detection;
    detection["label"] = box.label;
    detection["confidence"] = box.confidence;
    detection["x"] = box.bounds.x();
    detection["y"] = box.bounds.y();
    detection["w"] = box.bounds.width();
    detection["h"] = box.bounds.height();
    detection["moduleId"] = moduleId;
    detection["moduleType"] = static_cast<int>(moduleType);
    if (!box.trackId.isEmpty()) {
        detection["trackId"] = box.trackId;
    }
    return detection;
}

bool AnalyticsEngine::zoneMatches(const QVariantMap &detection, const QString &zonePreset) const
{
    const QString normalizedZone = zonePreset.trimmed().toLower();
    if (normalizedZone.isEmpty() || normalizedZone == "full") {
        return true;
    }

    const double x = detection.value("x").toDouble();
    const double y = detection.value("y").toDouble();
    const double w = detection.value("w").toDouble();
    const double h = detection.value("h").toDouble();
    const double cx = x + (w / 2.0);
    const double cy = y + (h / 2.0);

    if (normalizedZone == "center") {
        return cx >= 0.25 && cx <= 0.75 && cy >= 0.25 && cy <= 0.75;
    }
    if (normalizedZone == "left") {
        return cx <= 0.40;
    }
    if (normalizedZone == "right") {
        return cx >= 0.60;
    }
    if (normalizedZone == "top") {
        return cy <= 0.40;
    }
    if (normalizedZone == "bottom") {
        return cy >= 0.60;
    }

    return true;
}

void AnalyticsEngine::recordSkippedFrame(const QString &cameraId)
{
    QMutexLocker locker(&m_telemetryMutex);
    TelemetryState &state = m_cameraTelemetry[cameraId];
    state.skippedFrames += 1;
    state.lastSkippedMs = QDateTime::currentMSecsSinceEpoch();
}

bool AnalyticsEngine::moduleHasClipRules(const QVariantMap &extraConfig) const
{
    const QVariantList rules = extraConfig.value("rules").toList();
    for (const QVariant &ruleVar : rules) {
        const QVariantMap rule = ruleVar.toMap();
        if (!rule.value("enabled", true).toBool()) {
            continue;
        }
        if (rule.value("actionClip", false).toBool()) {
            return true;
        }
    }
    return false;
}

QString AnalyticsEngine::ensurePendingClip(const QString &cameraId, const QVariantMap &detection, qint64 nowMs)
{
    QString clipPath;
    bool requestStream = false;

    {
        QMutexLocker locker(&m_eventMutex);
        PendingEvent &evt = m_pendingEvents[cameraId];

        if (evt.endMs < nowMs) {
            evt.startMs = nowMs - (m_evidencePreSeconds * 1000);
            evt.endMs = nowMs + (m_evidencePostSeconds * 1000);
            evt.detection = detection;
            evt.snapshotPath.clear();
            evt.streamRequested = false;
            evt.clipPath.clear();
        } else {
            evt.endMs = nowMs + (m_evidencePostSeconds * 1000);
            if (evt.detection.isEmpty()) {
                evt.detection = detection;
            }
        }

        if (evt.clipPath.isEmpty()) {
            const QString dir = ensureDir(m_evidenceClipsDir);
            if (!dir.isEmpty()) {
                evt.clipPath = QDir(dir).filePath(buildEvidenceFileName(cameraId, "clip") + ".mp4");
            }
        }

        clipPath = evt.clipPath;
        if (!evt.streamRequested && !clipPath.isEmpty() && receivers(SIGNAL(clipRequested(QString,QString,int))) > 0) {
            evt.streamRequested = true;
            requestStream = true;
        }
    }

    if (requestStream) {
        emit clipRequested(cameraId, clipPath, 0);
    }

    return clipPath;
}

QVariantList AnalyticsEngine::updateObjectCounterTracking(const QString &cameraId, QVector<DetectionBox> &results, qint64 nowMs)
{
    QVariantList generatedEvents;
    if (results.isEmpty()) {
        QMutexLocker locker(&m_counterMutex);
        auto it = m_counterStates.find(cameraId);
        if (it == m_counterStates.end()) {
            return generatedEvents;
        }

        auto trackIt = it->tracks.begin();
        while (trackIt != it->tracks.end()) {
            if (nowMs - trackIt->lastSeenMs > 2500) {
                trackIt = it->tracks.erase(trackIt);
            } else {
                ++trackIt;
            }
        }
        return generatedEvents;
    }

    auto centerForBounds = [](const QRectF &bounds) {
        return QPointF(bounds.x() + (bounds.width() / 2.0), bounds.y() + (bounds.height() / 2.0));
    };

    struct MatchCandidate {
        QString trackId;
        int detectionIndex = -1;
        double distanceSquared = 0.0;
    };

    constexpr double maxTrackingDistanceSquared = 0.14 * 0.14;

    QMutexLocker locker(&m_counterMutex);
    CounterState &state = m_counterStates[cameraId];

    auto trackIt = state.tracks.begin();
    while (trackIt != state.tracks.end()) {
        if (nowMs - trackIt->lastSeenMs > 2500) {
            trackIt = state.tracks.erase(trackIt);
        } else {
            ++trackIt;
        }
    }

    QVector<MatchCandidate> candidates;
    for (auto it = state.tracks.begin(); it != state.tracks.end(); ++it) {
        for (int i = 0; i < results.size(); ++i) {
            if (results[i].label.compare(it.value().label, Qt::CaseInsensitive) != 0) {
                continue;
            }

            const QPointF detectionCenter = centerForBounds(results[i].bounds);
            const double dx = detectionCenter.x() - it.value().center.x();
            const double dy = detectionCenter.y() - it.value().center.y();
            const double distanceSquared = (dx * dx) + (dy * dy);

            if (distanceSquared <= maxTrackingDistanceSquared) {
                candidates.append({it.key(), i, distanceSquared});
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const MatchCandidate &a, const MatchCandidate &b) {
        return a.distanceSquared < b.distanceSquared;
    });

    QSet<QString> matchedTracks;
    QSet<int> matchedDetections;
    QStringList touchedTrackIds;

    for (const MatchCandidate &candidate : candidates) {
        if (matchedTracks.contains(candidate.trackId) || matchedDetections.contains(candidate.detectionIndex)) {
            continue;
        }

        auto existingTrack = state.tracks.find(candidate.trackId);
        if (existingTrack == state.tracks.end()) {
            continue;
        }

        matchedTracks.insert(candidate.trackId);
        matchedDetections.insert(candidate.detectionIndex);
        touchedTrackIds.append(candidate.trackId);

        existingTrack->bounds = results[candidate.detectionIndex].bounds;
        existingTrack->center = centerForBounds(results[candidate.detectionIndex].bounds);
        existingTrack->lastSeenMs = nowMs;
        existingTrack->seenFrames += 1;
        existingTrack->confidence = results[candidate.detectionIndex].confidence;
        results[candidate.detectionIndex].trackId = existingTrack->id;
    }

    for (int i = 0; i < results.size(); ++i) {
        if (matchedDetections.contains(i)) {
            continue;
        }

        TrackState track;
        track.id = QString::number(state.nextTrackNumber++);
        track.label = results[i].label;
        track.bounds = results[i].bounds;
        track.center = centerForBounds(results[i].bounds);
        track.lastSeenMs = nowMs;
        track.seenFrames = 1;
        track.confidence = results[i].confidence;
        track.counted = false;

        state.tracks.insert(track.id, track);
        touchedTrackIds.append(track.id);
        results[i].trackId = track.id;
    }

    touchedTrackIds.removeDuplicates();

    for (const QString &trackId : touchedTrackIds) {
        auto existingTrack = state.tracks.find(trackId);
        if (existingTrack == state.tracks.end()) {
            continue;
        }

        if (existingTrack->counted || existingTrack->seenFrames < 2) {
            continue;
        }

        existingTrack->counted = true;
        state.totalCountByLabel[existingTrack->label] += 1;

        QVariantMap event;
        event["id"] = QString("counter:%1:%2:%3").arg(cameraId, existingTrack->id, QString::number(nowMs));
        event["eventType"] = "counter";
        event["timestampMs"] = nowMs;
        event["timestampText"] = QDateTime::fromMSecsSinceEpoch(nowMs).toString("yyyy-MM-dd HH:mm:ss.zzz");
        event["cameraId"] = cameraId;
        event["moduleId"] = QStringLiteral("Object Counter");
        event["moduleType"] = static_cast<int>(ObjectCounter);
        event["label"] = existingTrack->label;
        event["confidence"] = existingTrack->confidence;
        event["trackId"] = existingTrack->id;
        event["x"] = existingTrack->bounds.x();
        event["y"] = existingTrack->bounds.y();
        event["w"] = existingTrack->bounds.width();
        event["h"] = existingTrack->bounds.height();
        event["countTotal"] = state.totalCountByLabel.value(existingTrack->label);
        event["activeTracks"] = state.tracks.size();
        event["countsText"] = countsToText(state.totalCountByLabel);
        event["message"] = QString("%1 counted on %2").arg(existingTrack->label, cameraId);
        generatedEvents.append(event);
    }

    return generatedEvents;
}

QVariantList AnalyticsEngine::evaluateRulesForDetection(const QString &cameraId,
                                                        ModuleType moduleType,
                                                        const QString &moduleId,
                                                        const QVariantMap &detection,
                                                        const QVariantList &rules,
                                                        const QImage &frame,
                                                        qint64 nowMs,
                                                        const QString &existingSnapshotPath,
                                                        const QString &existingClipPath)
{
    QVariantList generatedEvents;

    for (const QVariant &ruleVar : rules) {
        const QVariantMap rule = ruleVar.toMap();
        if (!rule.value("enabled", true).toBool()) {
            continue;
        }

        const QString ruleLabel = rule.value("label").toString().trimmed();
        const QString detectionLabel = detection.value("label").toString();
        if (!ruleLabel.isEmpty() && ruleLabel.compare("any", Qt::CaseInsensitive) != 0 && ruleLabel.compare(detectionLabel, Qt::CaseInsensitive) != 0) {
            continue;
        }

        if (detection.value("confidence").toDouble() < rule.value("minConfidence", 0.6).toDouble()) {
            continue;
        }

        const QString zonePreset = rule.value("zonePreset", "full").toString();
        if (!zoneMatches(detection, zonePreset)) {
            continue;
        }

        QString ruleId = rule.value("id").toString().trimmed();
        if (ruleId.isEmpty()) {
            ruleId = rule.value("name").toString().trimmed();
        }
        if (ruleId.isEmpty()) {
            ruleId = QString("%1:%2").arg(detectionLabel, zonePreset);
        }

        const int cooldownMs = qMax(1000, rule.value("cooldownMs", 5000).toInt());
        bool allowed = false;
        const QString cooldownKey = QString("%1:%2:%3").arg(cameraId, QString::number(static_cast<int>(moduleType)), ruleId);
        {
            QMutexLocker locker(&m_ruleMutex);
            const qint64 lastTriggered = m_ruleLastTriggeredMs.value(cooldownKey, 0);
            if ((nowMs - lastTriggered) >= cooldownMs) {
                m_ruleLastTriggeredMs[cooldownKey] = nowMs;
                allowed = true;
            }
        }

        if (!allowed) {
            continue;
        }

        const bool actionSnapshot = rule.value("actionSnapshot", true).toBool();
        const bool actionClip = rule.value("actionClip", true).toBool();
        const bool actionNotify = rule.value("actionNotify", false).toBool();
        const QString ruleName = rule.value("name").toString().trimmed();

        QVariantMap event = detection;
        event["id"] = QString("rule:%1:%2:%3:%4").arg(cameraId,
                                                           QString::number(static_cast<int>(moduleType)),
                                                           ruleId,
                                                           QString::number(nowMs));
        event["eventType"] = "rule";
        event["timestampMs"] = nowMs;
        event["timestampText"] = QDateTime::fromMSecsSinceEpoch(nowMs).toString("yyyy-MM-dd HH:mm:ss.zzz");
        event["cameraId"] = cameraId;
        event["moduleId"] = moduleId;
        event["moduleType"] = static_cast<int>(moduleType);
        event["ruleId"] = ruleId;
        event["ruleName"] = ruleName;
        event["zonePreset"] = zonePreset;
        event["actionSnapshot"] = actionSnapshot;
        event["actionClip"] = actionClip;
        event["actionNotify"] = actionNotify;
        event["message"] = ruleName.isEmpty()
            ? QString("%1 matched on %2").arg(detectionLabel, cameraId)
            : QString("%1 matched on %2").arg(ruleName, cameraId);

        if (actionSnapshot) {
            QString snapshotPath = existingSnapshotPath;
            if (snapshotPath.isEmpty()) {
                snapshotPath = saveSnapshotImage(frame, cameraId, detection, moduleId);
            }
            if (!snapshotPath.isEmpty()) {
                event["snapshotPath"] = snapshotPath;
                event["snapshotUrl"] = QUrl::fromLocalFile(snapshotPath).toString();
                if (m_uploadEnabled) {
                    enqueueUpload(snapshotPath);
                }
            }
        }

        if (actionClip) {
            QString clipPath = existingClipPath;
            if (clipPath.isEmpty()) {
                clipPath = ensurePendingClip(cameraId, detection, nowMs);
            }
            if (!clipPath.isEmpty()) {
                event["clipPath"] = clipPath;
                event["clipUrl"] = QUrl::fromLocalFile(clipPath).toString();
            }
        }

        generatedEvents.append(event);
    }

    return generatedEvents;
}

void AnalyticsEngine::appendAnalyticsEvents(const QVariantList &events)
{
    if (events.isEmpty()) {
        return;
    }

    persistAnalyticsEvents(events);

    for (int i = events.size() - 1; i >= 0; --i) {
        m_analyticsEvents.prepend(events.at(i));
    }

    while (m_analyticsEvents.size() > m_maxAnalyticsEvents) {
        m_analyticsEvents.removeLast();
    }

    emit analyticsEventsChanged();
}

void AnalyticsEngine::initEventStore()
{
    QMutexLocker locker(&m_eventStoreMutex);
    if (m_eventStorePath.isEmpty() || m_eventStoreConnectionName.isEmpty()) {
        return;
    }

    QDir dir(QFileInfo(m_eventStorePath).absolutePath());
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "Failed to create analytics event store directory:" << dir.absolutePath();
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_eventStoreConnectionName);
    db.setDatabaseName(m_eventStorePath);
    if (!db.open()) {
        qWarning() << "Failed to open analytics event store:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.exec("PRAGMA journal_mode=WAL");
    query.exec("PRAGMA synchronous=NORMAL");

    const bool tableOk = query.exec(
        "CREATE TABLE IF NOT EXISTS analytics_events ("
        "id TEXT PRIMARY KEY,"
        "timestamp_ms INTEGER NOT NULL,"
        "camera_id TEXT,"
        "module_type INTEGER,"
        "module_id TEXT,"
        "event_type TEXT,"
        "label TEXT,"
        "confidence REAL,"
        "rule_name TEXT,"
        "message TEXT,"
        "snapshot_path TEXT,"
        "clip_path TEXT,"
        "payload_json TEXT NOT NULL"
        ")"
    );

    if (!tableOk) {
        qWarning() << "Failed to initialize analytics_events table:" << query.lastError().text();
        db.close();
        return;
    }

    query.exec("CREATE INDEX IF NOT EXISTS idx_analytics_events_time ON analytics_events(timestamp_ms DESC)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_analytics_events_camera ON analytics_events(camera_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_analytics_events_module ON analytics_events(module_type)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_analytics_events_type ON analytics_events(event_type)");

    m_eventStoreReady = true;
}

QVariantList AnalyticsEngine::loadRecentAnalyticsEvents(int limit) const
{
    QVariantList events;
    QMutexLocker locker(&m_eventStoreMutex);
    if (!m_eventStoreReady || !QSqlDatabase::contains(m_eventStoreConnectionName)) {
        return events;
    }

    QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
    if (!db.isOpen()) {
        return events;
    }

    QSqlQuery query(db);
    query.prepare("SELECT payload_json FROM analytics_events ORDER BY timestamp_ms DESC, rowid DESC LIMIT ?");
    query.addBindValue(qMax(1, limit));
    if (!query.exec()) {
        qWarning() << "Failed to load analytics events:" << query.lastError().text();
        return events;
    }

    while (query.next()) {
        const QByteArray payload = query.value(0).toString().toUtf8();
        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        if (doc.isObject()) {
            events.append(doc.object().toVariantMap());
        }
    }

    return events;
}

void AnalyticsEngine::persistAnalyticsEvents(const QVariantList &events)
{
    QMutexLocker locker(&m_eventStoreMutex);
    if (!m_eventStoreReady || !QSqlDatabase::contains(m_eventStoreConnectionName)) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
    if (!db.isOpen()) {
        return;
    }

    if (!db.transaction()) {
        qWarning() << "Failed to start analytics event transaction:" << db.lastError().text();
    }

    QSqlQuery query(db);
    query.prepare(
        "INSERT OR REPLACE INTO analytics_events ("
        "id, timestamp_ms, camera_id, module_type, module_id, event_type, label, confidence, "
        "rule_name, message, snapshot_path, clip_path, payload_json"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    bool ok = true;
    for (const QVariant &eventVar : events) {
        const QVariantMap event = eventVar.toMap();
        QString id = event.value("id").toString();
        const qint64 timestampMs = event.value("timestampMs").toLongLong();
        if (id.isEmpty()) {
            id = QStringLiteral("event:%1:%2:%3")
                .arg(event.value("cameraId").toString(),
                     event.value("moduleType").toString(),
                     QString::number(timestampMs));
        }

        query.bindValue(0, id);
        query.bindValue(1, timestampMs > 0 ? timestampMs : QDateTime::currentMSecsSinceEpoch());
        query.bindValue(2, event.value("cameraId").toString());
        query.bindValue(3, event.value("moduleType").toInt());
        query.bindValue(4, event.value("moduleId").toString());
        query.bindValue(5, event.value("eventType").toString());
        query.bindValue(6, event.value("label").toString());
        query.bindValue(7, event.value("confidence").toDouble());
        query.bindValue(8, event.value("ruleName").toString());
        query.bindValue(9, event.value("message").toString());
        query.bindValue(10, event.value("snapshotPath").toString());
        query.bindValue(11, event.value("clipPath").toString());
        query.bindValue(12, QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(event)).toJson(QJsonDocument::Compact)));

        if (!query.exec()) {
            ok = false;
            qWarning() << "Failed to persist analytics event:" << query.lastError().text();
        }
    }

    if (ok) {
        db.commit();
    } else {
        db.rollback();
    }
}

void AnalyticsEngine::deleteStoredAnalyticsEvents(int type, const QString &cameraId)
{
    QMutexLocker locker(&m_eventStoreMutex);
    if (!m_eventStoreReady || !QSqlDatabase::contains(m_eventStoreConnectionName)) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(m_eventStoreConnectionName, false);
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    if (type < 0 && cameraId.trimmed().isEmpty()) {
        if (!query.exec("DELETE FROM analytics_events")) {
            qWarning() << "Failed to clear analytics events:" << query.lastError().text();
        }
        return;
    }

    query.prepare(
        "DELETE FROM analytics_events "
        "WHERE (? < 0 OR module_type = ?) "
        "AND (? = '' OR camera_id = ?)"
    );
    query.addBindValue(type);
    query.addBindValue(type);
    query.addBindValue(cameraId.trimmed());
    query.addBindValue(cameraId.trimmed());
    if (!query.exec()) {
        qWarning() << "Failed to delete analytics events:" << query.lastError().text();
    }
}

void AnalyticsEngine::processFrame(const QImage &frame, const QString &cameraId)
{
    const qint64 acceptedAtMs = QDateTime::currentMSecsSinceEpoch();
    bool shouldSkip = false;

    {
        QMutexLocker locker(&m_processingMutex);
        if (m_processingCameras.contains(cameraId) ||
            (m_analyticsMaxParallelJobs > 0 && m_processingCameras.size() >= m_analyticsMaxParallelJobs)) {
            shouldSkip = true;
        } else {
            const int frameIntervalMs = analyticsFrameIntervalMs();
            const qint64 lastAcceptedMs = m_lastAcceptedFrameMs.value(cameraId, 0);
            if (lastAcceptedMs > 0 && acceptedAtMs - lastAcceptedMs < frameIntervalMs) {
                shouldSkip = true;
            } else {
                m_processingCameras.insert(cameraId);
                m_lastAcceptedFrameMs[cameraId] = acceptedAtMs;
            }
        }
    }

    if (shouldSkip) {
        recordSkippedFrame(cameraId);
        return;
    }

    QImage frameCopy = frame;

    struct TaskContext {
        QString moduleId;
        ModuleType moduleType;
        std::shared_ptr<InferenceBackend> backend;
        QString snapshotsDir;
        QString faceSnapshotsMode;
        QVariantMap extraConfig;
    };

    QList<TaskContext> tasks;
    const bool evidenceEnabled = m_evidenceEnabled;
    bool frameBufferRequired = evidenceEnabled && m_evidenceClipsEnabled;

    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        const ModuleType type = it.key();
        const bool globallyEnabled = it.value().enabled && it.value().status == "ready";
        const bool cameraEnabled = m_cameraModules.value(cameraId).value(type, false);

        if (!globallyEnabled || !cameraEnabled) {
            continue;
        }

        if (evidenceEnabled && moduleHasClipRules(it.value().extraConfig)) {
            frameBufferRequired = true;
        }

        tasks.append({
            it.value().name,
            type,
            it.value().backend,
            it.value().snapshotsDir,
            it.value().faceSnapshotsMode,
            it.value().extraConfig
        });
    }

    if (tasks.isEmpty()) {
        QMutexLocker locker(&m_processingMutex);
        m_processingCameras.remove(cameraId);
        return;
    }

    (void)QtConcurrent::run([this, frameCopy, cameraId, tasks, evidenceEnabled, frameBufferRequired]() {
        QVariantList allDetections;
        QVariantList generatedEvents;
        QVariantList moduleStats;

        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        QElapsedTimer frameTimer;
        frameTimer.start();

        if (frameBufferRequired) {
            appendFrameToBuffer(cameraId, frameCopy, nowMs);
        }

        for (const auto &task : tasks) {
            if (!task.backend) {
                continue;
            }

            QElapsedTimer moduleTimer;
            moduleTimer.start();
            QVector<DetectionBox> results = task.backend->detect(frameCopy);
            const double moduleInferenceMs = static_cast<double>(moduleTimer.nsecsElapsed()) / 1000000.0;

            QVariantList taskEvents;
            const QVariantList rules = task.extraConfig.value("rules").toList();
            const bool hasConfiguredRules = !rules.isEmpty();

            if (task.moduleType == ObjectCounter) {
                const QVariantList counterEvents = updateObjectCounterTracking(cameraId, results, nowMs);
                if (evidenceEnabled) {
                    for (const QVariant &counterEventVar : counterEvents) {
                        taskEvents.append(counterEventVar);
                    }
                }
            }

            for (const DetectionBox &box : results) {
                const QVariantMap detection = detectionToVariant(box, task.moduleId, task.moduleType);
                allDetections.append(detection);

                QString existingSnapshotPath;
                QString existingClipPath;

                if (evidenceEnabled && box.confidence >= m_evidenceMinConfidence) {
                    bool shouldTriggerEvidence = false;
                    const QString eventKey = cameraId + ":" + QString::number(static_cast<int>(task.moduleType));

                    {
                        QMutexLocker eventLocker(&m_eventMutex);
                        const qint64 lastTriggered = m_lastEventTimes.value(eventKey, 0);
                        if ((nowMs - lastTriggered) >= m_evidenceCooldownMs) {
                            shouldTriggerEvidence = true;
                            m_lastEventTimes[eventKey] = nowMs;
                        }
                    }

                    if (shouldTriggerEvidence) {
                        if (m_evidenceClipsEnabled) {
                            existingClipPath = ensurePendingClip(cameraId, detection, nowMs);
                        }

                        if (m_evidenceSnapshotsEnabled) {
                            existingSnapshotPath = saveSnapshotImage(frameCopy, cameraId, detection, task.moduleId);
                            if (!existingSnapshotPath.isEmpty() && m_uploadEnabled) {
                                enqueueUpload(existingSnapshotPath);
                            }
                        }

                        if (!hasConfiguredRules) {
                            QVariantMap genericEvent = detection;
                            genericEvent["id"] = QString("detection:%1:%2:%3:%4")
                                .arg(cameraId,
                                     QString::number(static_cast<int>(task.moduleType)),
                                     box.label,
                                     QString::number(nowMs));
                            genericEvent["eventType"] = "detection";
                            genericEvent["timestampMs"] = nowMs;
                            genericEvent["timestampText"] = QDateTime::fromMSecsSinceEpoch(nowMs).toString("yyyy-MM-dd HH:mm:ss.zzz");
                            genericEvent["cameraId"] = cameraId;
                            genericEvent["moduleId"] = task.moduleId;
                            genericEvent["moduleType"] = static_cast<int>(task.moduleType);
                            genericEvent["message"] = QString("%1 detected on %2").arg(box.label, cameraId);

                            if (!existingSnapshotPath.isEmpty()) {
                                genericEvent["snapshotPath"] = existingSnapshotPath;
                                genericEvent["snapshotUrl"] = QUrl::fromLocalFile(existingSnapshotPath).toString();
                            }

                            if (!existingClipPath.isEmpty()) {
                                genericEvent["clipPath"] = existingClipPath;
                                genericEvent["clipUrl"] = QUrl::fromLocalFile(existingClipPath).toString();
                            }

                            taskEvents.append(genericEvent);
                        }
                    }
                }

                if (evidenceEnabled) {
                    const QVariantList ruleEvents = evaluateRulesForDetection(cameraId,
                                                                              task.moduleType,
                                                                              task.moduleId,
                                                                              detection,
                                                                              rules,
                                                                              frameCopy,
                                                                              nowMs,
                                                                              existingSnapshotPath,
                                                                              existingClipPath);
                    for (const QVariant &ruleEventVar : ruleEvents) {
                        taskEvents.append(ruleEventVar);
                    }
                }

                if (evidenceEnabled && !task.snapshotsDir.isEmpty() && QDir(task.snapshotsDir).exists()) {
                    bool canSnapshot = false;
                    const QString key = cameraId + "_" + QString::number(task.moduleType);
                    const qint64 snapshotNow = QDateTime::currentMSecsSinceEpoch();

                    {
                        QMutexLocker snapshotLocker(&m_snapshotMutex);
                        const qint64 last = m_lastSnapshotTimes.value(key, 0);
                        if (snapshotNow - last > 1000) {
                            m_lastSnapshotTimes[key] = snapshotNow;
                            canSnapshot = true;
                        }
                    }

                    if (canSnapshot && box.confidence > 0.6f) {
                        if (task.moduleType == FaceDetector && task.faceSnapshotsMode != "disabled") {
                            int x = static_cast<int>(box.bounds.x() * frameCopy.width());
                            int y = static_cast<int>(box.bounds.y() * frameCopy.height());
                            int w = static_cast<int>(box.bounds.width() * frameCopy.width());
                            int h = static_cast<int>(box.bounds.height() * frameCopy.height());

                            const int padW = w / 2;
                            const int padH = h / 2;

                            x -= padW / 2;
                            y -= padH / 2;
                            w += padW;
                            h += padH;

                            x = std::max(0, x);
                            y = std::max(0, y);
                            w = std::min(frameCopy.width() - x, w);
                            h = std::min(frameCopy.height() - y, h);

                            const QRect faceRect(x, y, w, h);
                            if (w > 10 && h > 10) {
                                QImage faceImg = frameCopy.copy(faceRect);
                                if (task.faceSnapshotsMode == "anonymized") {
                                    faceImg = faceImg.scaled(qMax(1, w / 10), qMax(1, h / 10)).scaled(w, h);
                                }

                                const QString filename = QString("%1/face_%2_%3.jpg")
                                    .arg(task.snapshotsDir)
                                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz"))
                                    .arg(static_cast<int>(box.confidence * 100));
                                faceImg.save(filename, "JPG", 100);
                            }
                        } else if (task.moduleType == LicensePlate) {
                            int x = static_cast<int>(box.bounds.x() * frameCopy.width());
                            int y = static_cast<int>(box.bounds.y() * frameCopy.height());
                            int w = static_cast<int>(box.bounds.width() * frameCopy.width());
                            int h = static_cast<int>(box.bounds.height() * frameCopy.height());

                            const int padW = w / 3;
                            const int padH = h / 3;

                            x -= padW / 2;
                            y -= padH / 2;
                            w += padW;
                            h += padH;

                            x = std::max(0, x);
                            y = std::max(0, y);
                            w = std::min(frameCopy.width() - x, w);
                            h = std::min(frameCopy.height() - y, h);

                            const QRect plateRect(x, y, w, h);
                            if (w > 10 && h > 10) {
                                QImage plateImg = frameCopy.copy(plateRect);
                                const QString filename = QString("%1/plate_%2_%3.jpg")
                                    .arg(task.snapshotsDir)
                                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz"))
                                    .arg(static_cast<int>(box.confidence * 100));
                                plateImg.save(filename, "JPG", 95);
                            }
                        }
                    }
                }
            }

            QVariantMap moduleTelemetry;
            moduleTelemetry["moduleType"] = static_cast<int>(task.moduleType);
            moduleTelemetry["detections"] = results.size();
            moduleTelemetry["events"] = taskEvents.size();
            moduleTelemetry["inferenceMs"] = moduleInferenceMs;
            moduleStats.append(moduleTelemetry);

            for (const QVariant &eventVar : taskEvents) {
                generatedEvents.append(eventVar);
            }
        }

        if (evidenceEnabled) {
            scheduleClipIfReady(cameraId, nowMs);
        }
        const double frameInferenceMs = static_cast<double>(frameTimer.nsecsElapsed()) / 1000000.0;

        QMetaObject::invokeMethod(this, [this, cameraId, allDetections, generatedEvents, moduleStats, frameInferenceMs]() {
            const qint64 completedAtMs = QDateTime::currentMSecsSinceEpoch();

            {
                QMutexLocker locker(&m_processingMutex);
                m_processingCameras.remove(cameraId);
            }

            {
                QMutexLocker locker(&m_telemetryMutex);
                TelemetryState &cameraTelemetry = m_cameraTelemetry[cameraId];
                cameraTelemetry.processedFrames += 1;
                cameraTelemetry.detections += static_cast<quint64>(allDetections.size());
                cameraTelemetry.events += static_cast<quint64>(generatedEvents.size());
                cameraTelemetry.lastInferenceMs = frameInferenceMs;
                cameraTelemetry.totalInferenceMs += frameInferenceMs;
                cameraTelemetry.lastProcessedMs = completedAtMs;
                if (!allDetections.isEmpty()) {
                    cameraTelemetry.lastDetectionMs = completedAtMs;
                }
                if (!generatedEvents.isEmpty()) {
                    cameraTelemetry.lastEventMs = completedAtMs;
                }

                for (const QVariant &moduleStatVar : moduleStats) {
                    const QVariantMap moduleStat = moduleStatVar.toMap();
                    const ModuleType moduleType = static_cast<ModuleType>(moduleStat.value("moduleType").toInt());
                    TelemetryState &moduleTelemetry = m_moduleTelemetry[moduleType];
                    moduleTelemetry.processedFrames += 1;
                    moduleTelemetry.detections += static_cast<quint64>(moduleStat.value("detections").toInt());
                    moduleTelemetry.events += static_cast<quint64>(moduleStat.value("events").toInt());
                    moduleTelemetry.lastInferenceMs = moduleStat.value("inferenceMs").toDouble();
                    moduleTelemetry.totalInferenceMs += moduleTelemetry.lastInferenceMs;
                    moduleTelemetry.lastProcessedMs = completedAtMs;
                    if (moduleStat.value("detections").toInt() > 0) {
                        moduleTelemetry.lastDetectionMs = completedAtMs;
                    }
                    if (moduleStat.value("events").toInt() > 0) {
                        moduleTelemetry.lastEventMs = completedAtMs;
                    }
                }
            }

            if (!generatedEvents.isEmpty()) {
                appendAnalyticsEvents(generatedEvents);
                for (const QVariant &eventVar : generatedEvents) {
                    const QVariantMap event = eventVar.toMap();
                    if (event.value("actionNotify").toBool()) {
                        emit analyticsNotificationRaised(event);
                    }
                }
            }

            emit analyticsTelemetryChanged();

            for (const QVariant &detVar : allDetections) {
                const QVariantMap det = detVar.toMap();
                emit detectionOccurred(det.value("moduleId").toString(), cameraId, det);
            }

            emit frameProcessed(cameraId, allDetections);
        });
    });
}

bool AnalyticsEngine::isBusy(const QString &cameraId) const
{
    QMutexLocker locker(&m_processingMutex);
    return m_processingCameras.contains(cameraId);
}

bool AnalyticsEngine::canAcceptFrame(const QString &cameraId) const
{
    if (cameraId.trimmed().isEmpty() || !hasActiveModules(cameraId)) {
        return false;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    QMutexLocker locker(&m_processingMutex);
    if (m_processingCameras.contains(cameraId)) {
        return false;
    }
    if (m_analyticsMaxParallelJobs > 0 && m_processingCameras.size() >= m_analyticsMaxParallelJobs) {
        return false;
    }

    const qint64 lastAcceptedMs = m_lastAcceptedFrameMs.value(cameraId, 0);
    return lastAcceptedMs <= 0 || nowMs - lastAcceptedMs >= analyticsFrameIntervalMs();
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
    const bool previous = m_cameraModules.value(cameraId).value(type, false);
    if (previous == enabled) {
        return;
    }

    if (enabled) {
        m_cameraModules[cameraId][type] = true;
    } else if (m_cameraModules.contains(cameraId)) {
        m_cameraModules[cameraId].remove(type);
        if (m_cameraModules[cameraId].isEmpty()) {
            m_cameraModules.remove(cameraId);
        }
    }

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
            QMutexLocker processingLocker(&m_processingMutex);
            m_lastAcceptedFrameMs.remove(cameraId);
        }
        {
            QMutexLocker bufferLocker(&m_bufferMutex);
            m_frameBuffers.remove(cameraId);
        }
        {
            QMutexLocker eventLocker(&m_eventMutex);
            m_pendingEvents.remove(cameraId);
            auto lastEventIt = m_lastEventTimes.begin();
            const QString prefix = cameraId + ":";
            while (lastEventIt != m_lastEventTimes.end()) {
                if (lastEventIt.key() == cameraId || lastEventIt.key().startsWith(prefix)) {
                    lastEventIt = m_lastEventTimes.erase(lastEventIt);
                } else {
                    ++lastEventIt;
                }
            }
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
        {
            QMutexLocker counterLocker(&m_counterMutex);
            m_counterStates.remove(cameraId);
        }
        {
            QMutexLocker ruleLocker(&m_ruleMutex);
            auto it = m_ruleLastTriggeredMs.begin();
            const QString prefix = cameraId + ":";
            while (it != m_ruleLastTriggeredMs.end()) {
                if (it.key().startsWith(prefix)) {
                    it = m_ruleLastTriggeredMs.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    emit settingsChanged();
}

bool AnalyticsEngine::isCameraModuleEnabled(const QString &cameraId, ModuleType type) const
{
    return m_cameraModules.value(cameraId).value(type, false);
}

QVariantMap AnalyticsEngine::getPerformanceSettings() const
{
    QVariantMap settings;
    settings["preset"] = m_analyticsPerformancePreset;
    settings["targetFps"] = m_analyticsTargetFps;
    settings["maxParallelJobs"] = m_analyticsMaxParallelJobs;
    settings["frameIntervalMs"] = analyticsFrameIntervalMs();
    return settings;
}

void AnalyticsEngine::setPerformanceSettings(const QVariantMap &settings)
{
    QString preset = settings.value("preset", m_analyticsPerformancePreset).toString().trimmed();
    if (preset.isEmpty()) {
        preset = QStringLiteral("custom");
    }
    if (preset != "eco" && preset != "balanced" && preset != "max" && preset != "custom") {
        preset = QStringLiteral("custom");
    }

    const int targetFps = qBound(1, settings.value("targetFps", m_analyticsTargetFps).toInt(), 30);
    const int maxParallelJobs = qBound(1, settings.value("maxParallelJobs", m_analyticsMaxParallelJobs).toInt(), 16);

    if (preset == m_analyticsPerformancePreset &&
        targetFps == m_analyticsTargetFps &&
        maxParallelJobs == m_analyticsMaxParallelJobs) {
        return;
    }

    m_analyticsPerformancePreset = preset;
    m_analyticsTargetFps = targetFps;
    m_analyticsMaxParallelJobs = maxParallelJobs;

    {
        QMutexLocker locker(&m_processingMutex);
        m_lastAcceptedFrameMs.clear();
    }

    emit settingsChanged();
    emit analyticsTelemetryChanged();
}

QVariantMap AnalyticsEngine::getSettings() const
{
    QVariantMap settings;
    QVariantMap modules;
    QVariantMap moduleConfigs;
    QVariantMap cameraModules;
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        QVariantMap moduleSettings;
        moduleSettings["enabled"] = it.value().enabled;
        modules[QString::number(it.key())] = moduleSettings;

        QVariantMap cfg = it.value().extraConfig;
        if (!it.value().snapshotsDir.isEmpty()) cfg["snapshotsDir"] = it.value().snapshotsDir;
        if (!it.value().faceSnapshotsMode.isEmpty()) cfg["faceSnapshotsMode"] = it.value().faceSnapshotsMode;
        moduleConfigs[QString::number(it.key())] = cfg;
    }
    for (auto cameraIt = m_cameraModules.begin(); cameraIt != m_cameraModules.end(); ++cameraIt) {
        QVariantMap assignments;
        for (auto moduleIt = cameraIt.value().begin(); moduleIt != cameraIt.value().end(); ++moduleIt) {
            assignments[QString::number(static_cast<int>(moduleIt.key()))] = moduleIt.value();
        }
        if (!assignments.isEmpty()) {
            cameraModules[cameraIt.key()] = assignments;
        }
    }
    settings["modules"] = modules;
    settings["moduleConfigs"] = moduleConfigs;
    settings["cameraModules"] = cameraModules;
    settings["evidence"] = buildEvidenceSettings(true);
    settings["performance"] = getPerformanceSettings();
    return settings;
}

QVariantMap AnalyticsEngine::getPersistedSettings() const
{
    QVariantMap settings;
    QVariantMap modules;
    QVariantMap moduleConfigs;
    QVariantMap cameraModules;
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        QVariantMap moduleSettings;
        moduleSettings["enabled"] = it.value().enabled;
        modules[QString::number(it.key())] = moduleSettings;

        QVariantMap cfg = it.value().extraConfig;
        if (!it.value().snapshotsDir.isEmpty()) cfg["snapshotsDir"] = it.value().snapshotsDir;
        if (!it.value().faceSnapshotsMode.isEmpty()) cfg["faceSnapshotsMode"] = it.value().faceSnapshotsMode;
        moduleConfigs[QString::number(it.key())] = cfg;
    }
    for (auto cameraIt = m_cameraModules.begin(); cameraIt != m_cameraModules.end(); ++cameraIt) {
        QVariantMap assignments;
        for (auto moduleIt = cameraIt.value().begin(); moduleIt != cameraIt.value().end(); ++moduleIt) {
            assignments[QString::number(static_cast<int>(moduleIt.key()))] = moduleIt.value();
        }
        if (!assignments.isEmpty()) {
            cameraModules[cameraIt.key()] = assignments;
        }
    }
    settings["modules"] = modules;
    settings["moduleConfigs"] = moduleConfigs;
    settings["cameraModules"] = cameraModules;
    settings["evidence"] = buildEvidenceSettings(false);
    settings["performance"] = getPerformanceSettings();
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

    if (settings.contains("performance")) {
        setPerformanceSettings(settings.value("performance").toMap());
    }

    if (settings.contains("cameraModules")) {
        QMap<QString, QMap<ModuleType, bool>> restoredCameraModules;
        QVariantMap cameras = settings.value("cameraModules").toMap();
        for (auto cameraIt = cameras.begin(); cameraIt != cameras.end(); ++cameraIt) {
            const QString cameraId = cameraIt.key();
            QVariantMap assignments = cameraIt.value().toMap();
            for (auto moduleIt = assignments.begin(); moduleIt != assignments.end(); ++moduleIt) {
                const int type = moduleIt.key().toInt();
                if (m_modules.contains(static_cast<ModuleType>(type)) && moduleIt.value().toBool()) {
                    restoredCameraModules[cameraId][static_cast<ModuleType>(type)] = true;
                }
            }
        }
        if (restoredCameraModules != m_cameraModules) {
            m_cameraModules = restoredCameraModules;
            emit settingsChanged();
        }
    }
}

QVariantMap AnalyticsEngine::getEvidenceSettings() const
{
    return buildEvidenceSettings(true);
}

QVariantMap AnalyticsEngine::buildEvidenceSettings(bool includeSensitiveData) const
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
    s["uploadExpiresAt"] = (qint64)m_uploadExpiresAt;

    if (includeSensitiveData) {
        s["uploadClientSecret"] = m_uploadClientSecret;
        s["uploadAccessToken"] = m_uploadAccessToken;
        s["uploadRefreshToken"] = m_uploadRefreshToken;
    }

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
        QString dir = PathUtils::localPathFromUserInput(settings.value("snapshotsDir").toString());
        setIfChanged(m_evidenceSnapshotsDir, dir);
    }
    if (settings.contains("clipsDir")) {
        QString dir = PathUtils::localPathFromUserInput(settings.value("clipsDir").toString());
        setIfChanged(m_evidenceClipsDir, dir);
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
    if (settings.contains("uploadExpiresAt")) {
        qint64 v = settings.value("uploadExpiresAt").toLongLong();
        if (m_uploadExpiresAt != v) {
            m_uploadExpiresAt = v;
            changed = true;
        }
    }

    auto syncSecret = [&](const QString &settingName, QString &field) {
        if (settings.contains(settingName)) {
            const QString value = settings.value(settingName).toString();
            if (field != value) {
                field = value;
                changed = true;
            }

            if (value.isEmpty()) {
                deleteSecretFromKeychain(settingName);
            } else {
                writeSecretToKeychain(settingName, value);
            }
            return;
        }

        const QString storedValue = readSecretFromKeychain(settingName);
        if (field != storedValue) {
            field = storedValue;
            changed = true;
        }
    };

    syncSecret("uploadClientSecret", m_uploadClientSecret);
    syncSecret("uploadAccessToken", m_uploadAccessToken);
    syncSecret("uploadRefreshToken", m_uploadRefreshToken);

    if (settings.contains("uploadClientSecret") || settings.contains("uploadAccessToken") || settings.contains("uploadRefreshToken")) {
        qInfo() << "AnalyticsEngine: upload secrets synchronized to keychain";
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

QString AnalyticsEngine::analyticsSecretKey(const QString &name) const
{
    return QStringLiteral("analytics/upload/%1").arg(name);
}

QString AnalyticsEngine::readSecretFromKeychain(const QString &name) const
{
    QKeychain::ReadPasswordJob job(QStringLiteral("OpenIPC"));
    job.setKey(analyticsSecretKey(name));

    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    return job.textData();
}

void AnalyticsEngine::writeSecretToKeychain(const QString &name, const QString &value) const
{
    QKeychain::WritePasswordJob job(QStringLiteral("OpenIPC"));
    job.setKey(analyticsSecretKey(name));
    job.setTextData(value);

    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
}

void AnalyticsEngine::deleteSecretFromKeychain(const QString &name) const
{
    QKeychain::DeletePasswordJob job(QStringLiteral("OpenIPC"));
    job.setKey(analyticsSecretKey(name));

    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
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
        QString newDir = PathUtils::localPathFromUserInput(config["snapshotsDir"].toString());
        if (ctx.snapshotsDir != newDir) {
            ctx.snapshotsDir = newDir;
            changed = true;
        }
    }
    
    if (config.contains("faceSnapshotsMode")) {
        QString newMode = config["faceSnapshotsMode"].toString();
        if (newMode == "encrypted") {
            qWarning() << "Encrypted face snapshots are not available; using anonymized mode";
            newMode = "anonymized";
        }
        if (newMode != "disabled" && newMode != "standard" && newMode != "anonymized") {
            newMode = "standard";
        }
        if (ctx.faceSnapshotsMode != newMode) {
            ctx.faceSnapshotsMode = newMode;
            changed = true;
        }
    }
    
    for (auto it = config.begin(); it != config.end(); ++it) {
        const QString key = it.key();
        if (key == "snapshotsDir" ||
            key == "faceSnapshotsMode" ||
            key == "faceSnapshotKeyHex" ||
            key == "resetFaceSnapshotKey" ||
            key == "faceSnapshotKeyConfigured" ||
            key == "enabled") {
            continue;
        }

        const QVariant value = it.value();
        if (!value.isValid() || value.isNull()) {
            if (ctx.extraConfig.contains(key)) {
                ctx.extraConfig.remove(key);
                changed = true;
            }
            continue;
        }

        if (!ctx.extraConfig.contains(key) || ctx.extraConfig.value(key) != value) {
            ctx.extraConfig.insert(key, value);
            changed = true;
        }
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
    QVariantMap config = ctx.extraConfig;
    config["snapshotsDir"] = ctx.snapshotsDir;
    config["faceSnapshotsMode"] = ctx.faceSnapshotsMode;
    
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
