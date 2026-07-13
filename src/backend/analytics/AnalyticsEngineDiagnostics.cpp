#include "AnalyticsEngine.h"
#include "ModelArtifactVerifier.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QStandardPaths>

#include <algorithm>

QVariantList AnalyticsEngine::moduleInventory() const
{
    QVariantList result;

    for (int typeValue = static_cast<int>(FaceDetector);
         typeValue <= static_cast<int>(LicensePlate);
         ++typeValue) {
        const ModuleType type = static_cast<ModuleType>(typeValue);
        if (!m_modules.contains(type)) {
            continue;
        }

        const ModuleContext &ctx = m_modules[type];
        const QString modelPath = QDir(m_modulesDir).filePath(ctx.modelFileName);
        const QFileInfo modelInfo(modelPath);
        const QFileInfo partialInfo(modelPath + QStringLiteral(".part"));
        const QFileInfo previousInfo(modelPath + QStringLiteral(".previous"));
        const bool installed = modelInfo.exists() && modelInfo.isFile();
        const bool sizeMatches = installed && (ctx.modelSizeBytes <= 0 || modelInfo.size() == ctx.modelSizeBytes);

        int assignedCameras = 0;
        for (auto cameraIt = m_cameraModules.begin(); cameraIt != m_cameraModules.end(); ++cameraIt) {
            if (cameraIt.value().value(type, false)) {
                assignedCameras += 1;
            }
        }

        QVariantMap entry = getModuleDiagnostics(typeValue);
        entry["type"] = typeValue;
        entry["enabled"] = ctx.enabled;
        entry["installed"] = installed;
        entry["sizeMatches"] = sizeMatches;
        entry["verificationState"] = !installed
            ? QStringLiteral("missing")
            : (!sizeMatches
                ? QStringLiteral("size_mismatch")
                : (ctx.status == QStringLiteral("ready") ? QStringLiteral("trusted") : QStringLiteral("size_ok")));
        entry["partialPath"] = partialInfo.absoluteFilePath();
        entry["partialExists"] = partialInfo.exists() && partialInfo.isFile();
        entry["partialBytes"] = partialInfo.exists() ? static_cast<qlonglong>(partialInfo.size()) : 0;
        entry["previousPath"] = previousInfo.absoluteFilePath();
        entry["previousExists"] = previousInfo.exists() && previousInfo.isFile();
        entry["previousBytes"] = previousInfo.exists() ? static_cast<qlonglong>(previousInfo.size()) : 0;
        entry["storageBytes"] = static_cast<qlonglong>(
            (modelInfo.exists() ? modelInfo.size() : 0)
            + (partialInfo.exists() ? partialInfo.size() : 0)
            + (previousInfo.exists() ? previousInfo.size() : 0));
        entry["assignedCameras"] = assignedCameras;
        entry["telemetry"] = getModuleTelemetry(typeValue);
        result.append(entry);
    }

    return result;
}

QVariantMap AnalyticsEngine::verifyModuleArtifact(int type) const
{
    const ModuleType moduleType = static_cast<ModuleType>(type);
    QVariantMap result;
    result["type"] = type;
    result["ok"] = false;
    result["verified"] = false;

    if (!m_modules.contains(moduleType)) {
        result["status"] = QStringLiteral("error");
        result["message"] = QStringLiteral("Unknown module");
        return result;
    }

    const ModuleContext &ctx = m_modules[moduleType];
    const QString modelPath = QDir(m_modulesDir).filePath(ctx.modelFileName);
    const QFileInfo modelInfo(modelPath);
    result["path"] = modelPath;
    result["fileName"] = ctx.modelFileName;
    result["expectedSizeBytes"] = static_cast<qlonglong>(ctx.modelSizeBytes);
    result["actualSizeBytes"] = modelInfo.exists() ? static_cast<qlonglong>(modelInfo.size()) : 0;
    result["expectedSha256"] = ctx.modelSha256;

    if (!modelInfo.exists() || !modelInfo.isFile()) {
        result["status"] = QStringLiteral("missing");
        result["message"] = QStringLiteral("Model file is missing");
        return result;
    }

    QString verificationError;
    const bool verified = ModelArtifactVerifier::verify(modelPath, ctx.modelSha256,
                                                        ctx.modelSizeBytes, &verificationError);
    result["ok"] = verified;
    result["verified"] = verified;
    result["status"] = verified ? QStringLiteral("verified") : QStringLiteral("failed");
    result["message"] = verified
        ? QStringLiteral("Model artifact is verified")
        : verificationError;
    return result;
}

QVariantMap AnalyticsEngine::cleanupModuleArtifacts(int type)
{
    const ModuleType moduleType = static_cast<ModuleType>(type);
    QVariantMap result;
    result["type"] = type;
    result["ok"] = false;
    result["removedCount"] = 0;
    result["freedBytes"] = 0;

    if (!m_modules.contains(moduleType)) {
        result["message"] = QStringLiteral("Unknown module");
        return result;
    }

    if (m_currentDownloads.contains(moduleType)) {
        result["message"] = QStringLiteral("Download is active; cleanup skipped");
        return result;
    }

    const ModuleContext &ctx = m_modules[moduleType];
    const QString modelPath = QDir(m_modulesDir).filePath(ctx.modelFileName);
    const QStringList candidates{
        modelPath + QStringLiteral(".part"),
        modelPath + QStringLiteral(".tmp"),
        modelPath + QStringLiteral(".download"),
        modelPath + QStringLiteral(".previous")
    };

    QVariantList removed;
    qint64 freedBytes = 0;
    for (const QString &candidate : candidates) {
        QFileInfo info(candidate);
        if (!info.exists() || !info.isFile()) {
            continue;
        }

        const qint64 fileSize = info.size();
        if (QFile::remove(candidate)) {
            QVariantMap item;
            item["path"] = candidate;
            item["bytes"] = static_cast<qlonglong>(fileSize);
            removed.append(item);
            freedBytes += fileSize;
        }
    }

    result["ok"] = true;
    result["removedCount"] = removed.size();
    result["removed"] = removed;
    result["freedBytes"] = static_cast<qlonglong>(freedBytes);
    result["message"] = removed.isEmpty()
        ? QStringLiteral("No temporary artifacts found")
        : QStringLiteral("Temporary artifacts removed");

    if (!removed.isEmpty()) {
        emit analyticsTelemetryChanged();
    }
    return result;
}

QVariantMap AnalyticsEngine::analyticsEvidenceSummary() const
{
    auto scanDirectory = [](const QString &path) -> QVariantMap {
        QVariantMap entry;
        const QString cleanPath = QDir::cleanPath(path);
        QFileInfo rootInfo(cleanPath);
        entry["path"] = cleanPath;
        entry["exists"] = rootInfo.exists() && rootInfo.isDir();
        entry["files"] = 0;
        entry["imageFiles"] = 0;
        entry["videoFiles"] = 0;
        entry["bytes"] = 0;

        if (!rootInfo.exists() || !rootInfo.isDir()) {
            return entry;
        }

        static const QSet<QString> imageSuffixes{
            QStringLiteral("jpg"), QStringLiteral("jpeg"), QStringLiteral("png"), QStringLiteral("webp")
        };
        static const QSet<QString> videoSuffixes{
            QStringLiteral("mp4"), QStringLiteral("mkv"), QStringLiteral("avi"), QStringLiteral("mov")
        };

        int files = 0;
        int imageFiles = 0;
        int videoFiles = 0;
        qint64 bytes = 0;
        constexpr int kMaxScannedFiles = 20000;

        QDirIterator iterator(cleanPath, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext() && files < kMaxScannedFiles) {
            iterator.next();
            const QFileInfo info = iterator.fileInfo();
            const QString suffix = info.suffix().toLower();
            files += 1;
            bytes += info.size();
            if (imageSuffixes.contains(suffix)) {
                imageFiles += 1;
            } else if (videoSuffixes.contains(suffix)) {
                videoFiles += 1;
            }
        }

        entry["files"] = files;
        entry["imageFiles"] = imageFiles;
        entry["videoFiles"] = videoFiles;
        entry["bytes"] = static_cast<qlonglong>(bytes);
        entry["truncated"] = files >= kMaxScannedFiles;
        return entry;
    };

    QVariantMap result;
    result["enabled"] = m_evidenceEnabled;
    result["snapshotsEnabled"] = m_evidenceSnapshotsEnabled;
    result["clipsEnabled"] = m_evidenceClipsEnabled;
    result["snapshotsDir"] = m_evidenceSnapshotsDir;
    result["clipsDir"] = m_evidenceClipsDir;

    QVariantList directories;
    QSet<QString> seenPaths;
    auto addDirectory = [&](const QString &path, const QString &kind, const QString &moduleName = QString()) {
        if (path.trimmed().isEmpty()) {
            return;
        }
        const QString cleanPath = QDir::cleanPath(path);
        if (seenPaths.contains(cleanPath)) {
            return;
        }
        seenPaths.insert(cleanPath);
        QVariantMap entry = scanDirectory(cleanPath);
        entry["kind"] = kind;
        entry["moduleName"] = moduleName;
        directories.append(entry);
    };

    addDirectory(m_evidenceSnapshotsDir, QStringLiteral("snapshots"));
    addDirectory(m_evidenceClipsDir, QStringLiteral("clips"));
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        addDirectory(it.value().snapshotsDir, QStringLiteral("module_snapshots"), it.value().name);
    }

    int totalFiles = 0;
    int totalImages = 0;
    int totalVideos = 0;
    qint64 totalBytes = 0;
    for (const QVariant &dirVar : directories) {
        const QVariantMap dir = dirVar.toMap();
        totalFiles += dir.value("files").toInt();
        totalImages += dir.value("imageFiles").toInt();
        totalVideos += dir.value("videoFiles").toInt();
        totalBytes += dir.value("bytes").toLongLong();
    }

    result["directories"] = directories;
    result["directoryCount"] = directories.size();
    result["totalFiles"] = totalFiles;
    result["totalImageFiles"] = totalImages;
    result["totalVideoFiles"] = totalVideos;
    result["totalBytes"] = static_cast<qlonglong>(totalBytes);
    return result;
}

QVariantList AnalyticsEngine::analyticsRecommendations() const
{
    QVariantList result;
    auto add = [&](const QString &level, const QString &title, const QString &message, const QString &action = QString()) {
        QVariantMap item;
        item["level"] = level;
        item["title"] = title;
        item["message"] = message;
        item["action"] = action;
        result.append(item);
    };

    if (!m_eventStoreReady) {
        add(QStringLiteral("danger"),
            QStringLiteral("Event store is unavailable"),
            QStringLiteral("Analytics events will remain only in memory until SQLite storage is available."),
            m_eventStorePath);
    }

    if (!m_evidenceEnabled) {
        add(QStringLiteral("warning"),
            QStringLiteral("Evidence capture is disabled"),
            QStringLiteral("Enable analytics evidence if you need snapshots, clips and persistent incident context."));
    } else {
        if (m_evidenceSnapshotsEnabled && !QDir(m_evidenceSnapshotsDir).exists()) {
            add(QStringLiteral("warning"),
                QStringLiteral("Snapshot directory is not created yet"),
                QStringLiteral("The directory will be created on first event, but checking the path now helps avoid permission issues."),
                m_evidenceSnapshotsDir);
        }
        if (m_evidenceClipsEnabled && !QDir(m_evidenceClipsDir).exists()) {
            add(QStringLiteral("info"),
                QStringLiteral("Clip directory is not created yet"),
                QStringLiteral("Buffered clips will be written here when a rule requests video evidence."),
                m_evidenceClipsDir);
        }
    }

    int enabledModules = 0;
    int enabledReadyModules = 0;
    int assignedModules = 0;
    int configuredRules = 0;
    QSet<QString> camerasWithAnalytics;

    for (int typeValue = static_cast<int>(FaceDetector);
         typeValue <= static_cast<int>(LicensePlate);
         ++typeValue) {
        const ModuleType type = static_cast<ModuleType>(typeValue);
        if (!m_modules.contains(type)) {
            continue;
        }

        const ModuleContext &ctx = m_modules[type];
        const QString modelPath = QDir(m_modulesDir).filePath(ctx.modelFileName);
        const QFileInfo modelInfo(modelPath);
        if (ctx.enabled) {
            enabledModules += 1;
            if (ctx.status == QStringLiteral("ready")) {
                enabledReadyModules += 1;
            } else {
                add(QStringLiteral("warning"),
                    QStringLiteral("Enabled module is not ready"),
                    QStringLiteral("%1: %2").arg(ctx.name, ctx.error.isEmpty() ? ctx.status : ctx.error),
                    modelPath);
            }
        }

        if (modelInfo.exists() && ctx.modelSizeBytes > 0 && modelInfo.size() != ctx.modelSizeBytes) {
            add(QStringLiteral("danger"),
                QStringLiteral("Model artifact size mismatch"),
                QStringLiteral("%1 has an unexpected size. Run artifact verification or reload the module.").arg(ctx.name),
                modelPath);
        }

        const QVariantList rules = ctx.extraConfig.value("rules").toList();
        for (const QVariant &ruleVar : rules) {
            if (ruleVar.toMap().value("enabled", true).toBool()) {
                configuredRules += 1;
            }
        }
    }

    for (auto cameraIt = m_cameraModules.begin(); cameraIt != m_cameraModules.end(); ++cameraIt) {
        bool hasEnabledAssignment = false;
        for (auto moduleIt = cameraIt.value().begin(); moduleIt != cameraIt.value().end(); ++moduleIt) {
            if (moduleIt.value()) {
                assignedModules += 1;
                hasEnabledAssignment = true;
            }
        }
        if (hasEnabledAssignment) {
            camerasWithAnalytics.insert(cameraIt.key());
        }
    }

    if (enabledModules == 0) {
        add(QStringLiteral("warning"),
            QStringLiteral("No AI module is enabled"),
            QStringLiteral("Enable at least one module before assigning analytics to cameras."));
    } else if (enabledReadyModules < enabledModules) {
        add(QStringLiteral("info"),
            QStringLiteral("Some modules are still preparing"),
            QStringLiteral("Analytics will start for assigned cameras as soon as enabled modules become ready."));
    }

    if (assignedModules == 0) {
        add(QStringLiteral("warning"),
            QStringLiteral("No camera has analytics assigned"),
            QStringLiteral("Assign enabled modules to cameras on the Cameras tab."));
    }

    if (configuredRules == 0) {
        add(QStringLiteral("info"),
            QStringLiteral("No analytics rules configured"),
            QStringLiteral("The event feed can show generic detections; add rules for precise incident workflows."));
    }

    if (result.isEmpty()) {
        add(QStringLiteral("success"),
            QStringLiteral("Analytics pipeline looks healthy"),
            QStringLiteral("Modules, event storage and camera assignments are ready."),
            QStringLiteral("%1 camera(s) with AI").arg(camerasWithAnalytics.size()));
    }

    return result;
}

QVariantMap AnalyticsEngine::getCameraAnalyticsDiagnostics(const QString &cameraId) const
{
    QVariantMap result;
    const QString trimmedCameraId = cameraId.trimmed();
    result["cameraId"] = trimmedCameraId;
    result["found"] = false;

    if (trimmedCameraId.isEmpty()) {
        result["pipelineState"] = QStringLiteral("unknown");
        result["message"] = QStringLiteral("Camera id is empty");
        return result;
    }

    const QVariantMap diagnostics = analyticsDiagnostics();
    const QVariantList cameras = diagnostics.value("cameraStats").toList();
    for (const QVariant &cameraVar : cameras) {
        const QVariantMap camera = cameraVar.toMap();
        if (camera.value("cameraId").toString() == trimmedCameraId) {
            result = camera;
            result["found"] = true;
            break;
        }
    }

    if (!result.value("found").toBool()) {
        result["pipelineState"] = hasActiveModules(trimmedCameraId)
            ? QStringLiteral("waiting")
            : QStringLiteral("unassigned");
        result["assignedModules"] = m_cameraModules.value(trimmedCameraId).size();
        result["activeModules"] = 0;
        result["readyModules"] = 0;
    }

    QVariantList recommendations;
    auto add = [&](const QString &level, const QString &title, const QString &message) {
        QVariantMap item;
        item["level"] = level;
        item["title"] = title;
        item["message"] = message;
        recommendations.append(item);
    };

    const int assigned = result.value("assignedModules").toInt();
    const int ready = result.value("readyModules").toInt();
    const QString pipelineState = result.value("pipelineState").toString();
    if (assigned == 0) {
        add(QStringLiteral("warning"),
            QStringLiteral("No modules assigned"),
            QStringLiteral("Enable analytics modules for this camera in the Cameras tab."));
    } else if (ready == 0) {
        add(QStringLiteral("warning"),
            QStringLiteral("Assigned modules are not ready"),
            QStringLiteral("Check module downloads, verification status and global module toggles."));
    } else if (pipelineState == QStringLiteral("throttled")) {
        add(QStringLiteral("info"),
            QStringLiteral("Frames are throttled"),
            QStringLiteral("Lower AI FPS or increase max parallel jobs only if CPU headroom allows it."));
    } else if (pipelineState == QStringLiteral("waiting")) {
        add(QStringLiteral("info"),
            QStringLiteral("Waiting for frames"),
            QStringLiteral("The camera has active analytics, but no frame has reached the engine yet."));
    } else {
        add(QStringLiteral("success"),
            QStringLiteral("Camera analytics is ready"),
            QStringLiteral("The camera has active modules and can feed the analytics pipeline."));
    }

    result["recommendations"] = recommendations;
    return result;
}
