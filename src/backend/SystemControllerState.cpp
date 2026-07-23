#include "SystemController.h"
#include "AppPaths.h"
#include "PathUtils.h"
#include "StateStore.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrl>
#include <algorithm>
#include <keychain.h>

namespace {

constexpr int kRecordingSegmentMinMinutes = 5;
constexpr int kRecordingSegmentMaxMinutes = 60;
constexpr int kRecordingSegmentStepMinutes = 5;
constexpr int kRecordingSegmentDefaultMinutes = 15;

QString normalizedLocalPathForState(const QString &pathOrUrl)
{
    return PathUtils::localPathFromUserInput(pathOrUrl);
}

int normalizedRecordingSegmentDurationForState(const QVariant &value)
{
    bool ok = false;
    int minutes = value.toInt(&ok);
    if (!ok) {
        minutes = kRecordingSegmentDefaultMinutes;
    }

    minutes = std::clamp(minutes, kRecordingSegmentMinMinutes, kRecordingSegmentMaxMinutes);
    minutes = ((minutes + kRecordingSegmentStepMinutes / 2) / kRecordingSegmentStepMinutes)
        * kRecordingSegmentStepMinutes;
    return std::clamp(minutes, kRecordingSegmentMinMinutes, kRecordingSegmentMaxMinutes);
}

int normalizedPlayerFillModeForState(const QVariant &value)
{
    bool ok = false;
    const int mode = value.toInt(&ok);
    if (!ok) {
        return 0;
    }
    if (mode < 0) {
        return -1;
    }
    if (mode > 0) {
        return 1;
    }
    return 0;
}

void normalizeAppSettingsForState(QVariantMap &settings)
{
    if (!settings.contains(QStringLiteral("webDeploymentProfile"))) {
        settings[QStringLiteral("webDeploymentProfile")] =
            settings.value(QStringLiteral("webServerAllowRemote"), false).toBool()
                ? QStringLiteral("lan") : QStringLiteral("localhost");
    }
    const QStringList pathKeys{
        QStringLiteral("recordingsPath"),
        QStringLiteral("screenshotsPath")
    };

    for (const QString &key : pathKeys) {
        const QString value = settings.value(key).toString();
        if (!value.trimmed().isEmpty()) {
            settings[key] = normalizedLocalPathForState(value);
        }
    }

    if (settings.contains(QStringLiteral("recordingSegmentDuration"))) {
        settings[QStringLiteral("recordingSegmentDuration")] =
            normalizedRecordingSegmentDurationForState(settings.value(QStringLiteral("recordingSegmentDuration")));
    }

    if (settings.contains(QStringLiteral("playerFillMode"))) {
        settings[QStringLiteral("playerFillMode")] =
            normalizedPlayerFillModeForState(settings.value(QStringLiteral("playerFillMode")));
    }

    const QString profile = settings.value(QStringLiteral("webDeploymentProfile"))
                                .toString().trimmed().toLower();
    if (profile == QStringLiteral("localhost")) {
        settings[QStringLiteral("webServerAllowRemote")] = false;
    } else if (profile == QStringLiteral("lan") || profile == QStringLiteral("vpn")) {
        settings[QStringLiteral("webServerAllowRemote")] = true;
    }
}

void removeLegacyPasswords(QJsonObject &state)
{
    QJsonArray cameras = state.value(QStringLiteral("cameras")).toArray();
    for (int i = 0; i < cameras.size(); ++i) {
        QJsonObject camera = cameras.at(i).toObject();
        camera.remove(QStringLiteral("password"));
        cameras[i] = camera;
    }
    state[QStringLiteral("cameras")] = cameras;

    QJsonArray grid = state.value(QStringLiteral("grid")).toArray();
    for (int i = 0; i < grid.size(); ++i) {
        QJsonObject slot = grid.at(i).toObject();
        QJsonObject camera = slot.value(QStringLiteral("camera")).toObject();
        camera.remove(QStringLiteral("password"));
        slot[QStringLiteral("camera")] = camera;
        grid[i] = slot;
    }
    state[QStringLiteral("grid")] = grid;
}

} // namespace
QString SystemController::stateFilePath() const
{
    const QString baseDir = AppPaths::dataDirectory();
    QDir().mkpath(baseDir);
    return baseDir + "/state.json";
}

QJsonObject SystemController::cameraToJson(const Camera &cam)
{
    auto sanitizedUrl = [](const QString &value) {
        QUrl url(value);
        if (!url.isValid()) return value;
        url.setUserName(QString());
        url.setPassword(QString());
        return url.toString(QUrl::FullyEncoded);
    };

    QJsonObject obj;
    obj["id"] = cam.id;
    obj["name"] = cam.name;
    obj["ip"] = cam.ip;
    obj["streamUrl"] = sanitizedUrl(cam.streamUrl);
    obj["sdStreamUrl"] = sanitizedUrl(cam.sdStreamUrl);
    obj["hdStreamUrl"] = sanitizedUrl(cam.hdStreamUrl);
    obj["status"] = cam.status;
    obj["port"] = cam.port;
    obj["onvifPort"] = cam.onvifPort;
    obj["login"] = cam.login;
    // Do not save password in plain text JSON anymore
    // obj["password"] = cam.password;
    obj["group"] = cam.group;
    obj["serialNumber"] = cam.serialNumber;
    obj["manufacturer"] = cam.manufacturer;
    obj["discoveryMethods"] = cam.discoveryMethods;
    obj["discoveryEvidence"] = cam.discoveryEvidence;
    obj["discoveryConfidence"] = cam.discoveryConfidence;
    obj["isOpenIpc"] = cam.isOpenIpc;
    obj["onboardingProfile"] = cam.onboardingProfile;
    obj["validationStatus"] = cam.validationStatus;
    obj["validationMessage"] = cam.validationMessage;
    obj["alreadyAdded"] = cam.alreadyAdded;
    return obj;
}

Camera SystemController::cameraFromJson(const QJsonObject &obj)
{
    Camera cam;
    cam.id = obj.value("id").toString();
    cam.name = obj.value("name").toString();
    cam.ip = obj.value("ip").toString();
    cam.streamUrl = obj.value("streamUrl").toString();
    cam.sdStreamUrl = obj.value("sdStreamUrl").toString();
    cam.hdStreamUrl = obj.value("hdStreamUrl").toString();
    cam.status = obj.value("status").toString();
    cam.port = obj.value("port").toInt(80);
    cam.onvifPort = obj.value("onvifPort").toInt(80);
    cam.login = obj.value("login").toString();
    cam.group = obj.value("group").toString();
    cam.serialNumber = obj.value("serialNumber").toString();
    cam.manufacturer = obj.value("manufacturer").toString();
    cam.discoveryMethods = obj.value("discoveryMethods").toString();
    cam.discoveryEvidence = obj.value("discoveryEvidence").toString();
    cam.discoveryConfidence = obj.value("discoveryConfidence").toInt(0);
    cam.isOpenIpc = obj.value("isOpenIpc").toBool(false);
    cam.onboardingProfile = obj.value("onboardingProfile").toString();
    cam.validationStatus = obj.value("validationStatus").toString();
    cam.validationMessage = obj.value("validationMessage").toString();
    cam.alreadyAdded = obj.value("alreadyAdded").toBool(false);

    QString passwordFromLegacyUrl;
    auto stripLegacyCredentials = [&cam, &passwordFromLegacyUrl](QString &value) {
        QUrl url(value);
        if (!url.isValid() || url.userName().isEmpty()) return;
        if (cam.login.isEmpty()) cam.login = url.userName(QUrl::FullyDecoded);
        if (passwordFromLegacyUrl.isEmpty()) {
            passwordFromLegacyUrl = url.password(QUrl::FullyDecoded);
        }
        url.setUserName(QString());
        url.setPassword(QString());
        value = url.toString(QUrl::FullyEncoded);
    };
    stripLegacyCredentials(cam.streamUrl);
    stripLegacyCredentials(cam.sdStreamUrl);
    stripLegacyCredentials(cam.hdStreamUrl);

    // Migration & Loading logic for passwords
    if (obj.contains("password")) {
        // Legacy plain text password found. Migrate to keychain.
        cam.password = obj.value("password").toString();
        if (!cam.password.isEmpty() && !cam.ip.isEmpty()) {
            auto job = new QKeychain::WritePasswordJob("OpenIPC");
            job->setAutoDelete(true);
            job->setKey(cam.ip);
            job->setTextData(cam.password);
            job->start();
        }
    } else if (!passwordFromLegacyUrl.isEmpty()) {
        cam.password = passwordFromLegacyUrl;
        auto job = new QKeychain::WritePasswordJob("OpenIPC");
        job->setAutoDelete(true);
        job->setKey(cam.ip);
        job->setTextData(cam.password);
        job->start();
    } else if (!cam.ip.isEmpty()) {
        // Load from keychain asynchronously
        auto job = new QKeychain::ReadPasswordJob("OpenIPC");
        job->setAutoDelete(true);
        job->setKey(cam.ip);
        QString ip = cam.ip;
        // We need a way to update the model when the job finishes.
        // Since cameraFromJson returns a copy, we can't update it directly here.
        // We will handle the async update in loadState or via a signal.
    }

    return cam;
}

void SystemController::saveState()
{
    // Debounce: restart timer if already running, or start if not.
    // This effectively waits for 1 second of "silence" before writing to disk.
    m_saveTimer->start();
}

void SystemController::performSave()
{
    QJsonObject root;
    QJsonArray cameras;
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        cameras.append(cameraToJson(m_cameraModel->getCamera(i)));
    }
    QJsonArray discoveryCameras;
    for (int i = 0; i < m_discoveryModel->rowCount(); ++i) {
        Camera cam = m_discoveryModel->getCamera(i);
        if (cam.validationStatus == QStringLiteral("running")) {
            cam.validationStatus = QStringLiteral("idle");
            cam.validationMessage = QStringLiteral("Проверка была прервана");
        }
        discoveryCameras.append(cameraToJson(cam));
    }
    QJsonArray groups;
    for (const auto &g : m_cameraGroups) {
        groups.append(g);
    }
    QJsonArray grid;
    for (int i = 0; i < m_gridModel->rowCount(); ++i) {
        QJsonObject slot;
        const Camera cam = m_gridModel->getCamera(i);
        slot["ip"] = cam.ip;
        slot["camera"] = cameraToJson(cam);
        slot["spanRows"] = cam.spanRows;
        slot["spanCols"] = cam.spanCols;
        grid.append(slot);
    }
    root["cameras"] = cameras;
    QJsonObject discovery;
    discovery["updatedAt"] = m_discoveryLastUpdated;
    discovery["interface"] = m_discoveryLastInterface;
    discovery["deepScan"] = m_discoveryLastDeepScan;
    discovery["cameras"] = discoveryCameras;
    root["lastDiscovery"] = discovery;
    root["grid"] = grid;
    root["analytics"] = QJsonObject::fromVariantMap(m_analyticsEngine->getPersistedSettings());
    root["appSettings"] = QJsonObject::fromVariantMap(m_appSettings);
    root["cameraGroups"] = groups;
    root["layoutTemplates"] = QJsonArray::fromVariantList(m_layoutTemplates);
    root["cameraHealthHistory"] = m_cameraHealthController->historyJson();

    QString errorMessage;
    StateStore store(stateDatabasePath());
    if (!store.save(root, &errorMessage)) {
        qCritical() << "Failed to save application state:" << errorMessage;
    } else {
        qInfo() << "Application state saved transactionally.";
    }
}

void SystemController::loadState()
{
    const QString path = stateFilePath();
    const QString backupPath = path + ".bak";
    StateStore store(stateDatabasePath());
    QString databaseError;
    const std::optional<QJsonObject> storedState = store.load(&databaseError);
    QJsonObject root = storedState.value_or(QJsonObject{});
    bool loadedLegacyState = false;

    if (!databaseError.isEmpty()) {
        qWarning() << "Failed to load state database:" << databaseError;
    }

    // Helper to read and validate JSON
    auto readJson = [](const QString &p) -> QJsonObject {
        QFile f(p);
        if (!f.exists() || !f.open(QIODevice::ReadOnly)) return QJsonObject();

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &error);
        f.close();

        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "Invalid JSON in" << p << ":" << error.errorString();
            return QJsonObject();
        }
        return doc.object();
    };

    if (root.isEmpty()) {
        root = readJson(path);
        loadedLegacyState = !root.isEmpty();
    }
    if (root.isEmpty()) {
        root = readJson(backupPath);
        loadedLegacyState = !root.isEmpty();
    }

    if (loadedLegacyState) {
        QJsonObject sanitizedState = root;
        removeLegacyPasswords(sanitizedState);
        QString migrationError;
        if (store.save(sanitizedState, &migrationError)) {
            QFile::remove(path);
            QFile::remove(backupPath);
            qInfo() << "Migrated legacy state.json to versioned SQLite storage.";
        } else {
            qWarning() << "Could not migrate legacy state.json:" << migrationError;
        }
    }

    // Still empty? Use defaults
    if (root.isEmpty()) {
        // No persisted state: seed defaults so QML sees a 2x2 grid and a starter template
        m_gridRows = 2;
        m_gridCols = 2;

        // Default templates
        QVariantList defaults;
        {
            QVariantMap t; t["name"] = "Раскладка"; t["rows"] = 2; t["cols"] = 2; t["isDefault"] = true; defaults.append(t);
        }
        m_layoutTemplates = defaults;
        emit layoutTemplatesChanged();

        // Preallocate 2x2 placeholders
        m_gridModel->clear();
        for (int i = 0; i < 4; ++i) {
            m_gridModel->addCamera(Camera());
        }
        // Normalize spans to the 1200-grid so QML immediately gets a 2x2 layout
        applyLayoutPreset(2, 2);
        return;
    }

    if (root.contains("analytics")) {
        m_analyticsEngine->setSettings(root.value("analytics").toObject().toVariantMap());
    }
    if (root.contains("cameraHealthHistory")) {
        m_cameraHealthController->restoreHistory(root.value("cameraHealthHistory").toArray());
    }

    if (root.contains("appSettings")) {
        QVariantMap savedSettings = root.value("appSettings").toObject().toVariantMap();
        normalizeAppSettingsForState(savedSettings);
        // Merge with defaults
        for (auto it = savedSettings.begin(); it != savedSettings.end(); ++it) {
            m_appSettings[it.key()] = it.value();
        }
        m_gridRows = m_appSettings.value("gridRows", 2).toInt();
        m_gridCols = m_appSettings.value("gridCols", 2).toInt();

        // Sanity check to prevent crash from invalid/legacy settings
        if (m_gridRows < 1 || m_gridRows > 64) m_gridRows = 2;
        if (m_gridCols < 1 || m_gridCols > 64) m_gridCols = 2;
    } else {
        // Fallback if appSettings missing but file exists
        m_gridRows = 2;
        m_gridCols = 2;
    }

    if (root.contains("layoutTemplates")) {
        m_layoutTemplates = root.value("layoutTemplates").toArray().toVariantList();
    } else {
        // Default templates
        QVariantList defaults;

        // 1x1
        { QVariantMap t; t["name"] = "1x1"; t["rows"] = 1; t["cols"] = 1; t["isDefault"] = true; defaults.append(t); }
        // 2x2
        { QVariantMap t; t["name"] = "2x2"; t["rows"] = 2; t["cols"] = 2; t["isDefault"] = true; defaults.append(t); }
        // 3x3
        { QVariantMap t; t["name"] = "3x3"; t["rows"] = 3; t["cols"] = 3; t["isDefault"] = true; defaults.append(t); }
        // 4x4
        { QVariantMap t; t["name"] = "4x4"; t["rows"] = 4; t["cols"] = 4; t["isDefault"] = true; defaults.append(t); }

        // 1+5 (3x3 grid)
        {
            QVariantMap t; t["name"] = "1 + 5"; t["rows"] = 3; t["cols"] = 3; t["isDefault"] = true;
            QVariantList cells;
            // Big cell (2x2)
            { QVariantMap c; c["rowSpan"] = 2; c["colSpan"] = 2; cells.append(c); }
            // Right column (2 cells)
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            // Bottom row (3 cells)
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            t["cells"] = cells;
            defaults.append(t);
        }

        // 1+7 (4x4 grid)
        {
            QVariantMap t; t["name"] = "1 + 7"; t["rows"] = 4; t["cols"] = 4; t["isDefault"] = true;
            QVariantList cells;
            // Big cell (3x3)
            { QVariantMap c; c["rowSpan"] = 3; c["colSpan"] = 3; cells.append(c); }
            // Right column (3 cells)
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            // Bottom row (4 cells)
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c); }
            t["cells"] = cells;
            defaults.append(t);
        }

        // 2+8 (4x4 grid)
        {
            QVariantMap t; t["name"] = "2 + 8"; t["rows"] = 4; t["cols"] = 4; t["isDefault"] = true;
            QVariantList cells;
            // Top 2 big cells (2x2 each)
            { QVariantMap c; c["rowSpan"] = 2; c["colSpan"] = 2; cells.append(c); }
            { QVariantMap c; c["rowSpan"] = 2; c["colSpan"] = 2; cells.append(c); }
            // Bottom 8 cells (2 rows of 4)
            for(int i=0; i<8; ++i) {
                 QVariantMap c; c["rowSpan"] = 1; c["colSpan"] = 1; cells.append(c);
            }
            t["cells"] = cells;
            defaults.append(t);
        }

        m_layoutTemplates = defaults;
    }
    emit layoutTemplatesChanged();

    m_cameraGroups.clear();
    const QJsonArray groups = root.value("cameraGroups").toArray();
    for (const auto &g : groups) {
        const QString name = g.toString();
        if (!name.isEmpty() && !m_cameraGroups.contains(name, Qt::CaseInsensitive)) {
            m_cameraGroups.append(name);
        }
    }

    m_cameraModel->clear();
    m_discoveryModel->clear();
    m_gridModel->clear();

    const QJsonArray cameras = root.value("cameras").toArray();
    for (const auto &v : cameras) {
        QJsonObject obj = v.toObject();
        Camera cam = cameraFromJson(obj);
        m_cameraModel->addCamera(cam);

        // If password wasn't in JSON, load it from keychain
        if (!obj.contains("password") && !cam.ip.isEmpty()) {
            auto job = new QKeychain::ReadPasswordJob("OpenIPC");
            job->setAutoDelete(true);
            job->setKey(cam.ip);
            connect(job, &QKeychain::Job::finished, this, [this, ip = cam.ip, job]() {
                if (!job->error()) {
                    int idx = m_cameraModel->findIndexByIp(ip);
                    if (idx >= 0) {
                        Camera c = m_cameraModel->getCamera(idx);
                        c.password = job->textData();
                        m_cameraModel->setCamera(idx, c);
                    }
                    // Also update grid model if present
                    for (int i = 0; i < m_gridModel->rowCount(); ++i) {
                        Camera gc = m_gridModel->getCamera(i);
                        if (gc.ip == ip) {
                            gc.password = job->textData();
                            m_gridModel->setCamera(i, gc);
                        }
                    }
                }
            });
            job->start();
        }

        const QString groupName = obj.value("group").toString();
        if (!groupName.isEmpty() && !m_cameraGroups.contains(groupName, Qt::CaseInsensitive)) {
            m_cameraGroups.append(groupName);
        }
    }

    const QJsonObject discovery = root.value(QStringLiteral("lastDiscovery")).toObject();
    if (!discovery.isEmpty()) {
        const QString restoredUpdatedAt = discovery.value(QStringLiteral("updatedAt")).toString();
        const QString restoredInterface = discovery.value(QStringLiteral("interface")).toString();
        const bool restoredDeepScan = discovery.value(QStringLiteral("deepScan")).toBool(false);
        m_discoveryLastUpdated = restoredUpdatedAt;
        m_discoveryLastInterface = restoredInterface;
        m_discoveryLastDeepScan = restoredDeepScan;
        const QJsonArray discoveryCameras = discovery.value(QStringLiteral("cameras")).toArray();
        for (const QJsonValue &value : discoveryCameras) {
            Camera cam = cameraFromJson(value.toObject());
            cam.password.clear();
            if (cam.ip.isEmpty()) continue;
            if (cam.onboardingProfile.isEmpty()) {
                cam.onboardingProfile = discoveryProfileForCamera(cam, QString());
            }
            if (cam.validationStatus == QStringLiteral("running")) {
                cam.validationStatus = QStringLiteral("idle");
                cam.validationMessage = QStringLiteral("Проверка была прервана");
            }
            mergeDiscoveryCamera(cam);
        }
        m_discoveryLastUpdated = restoredUpdatedAt;
        m_discoveryLastInterface = restoredInterface;
        m_discoveryLastDeepScan = restoredDeepScan;
        markDiscoveryAddedFlags();
        emit discoverySessionChanged();
    }

    QJsonArray grid = root.value("grid").toArray();

    if (grid.isEmpty()) {
        qInfo() << "Empty grid state detected. Initializing default 2x2 layout.";
        m_gridRows = 2;
        m_gridCols = 2;
        applyLayoutPreset(2, 2);
    } else {
        for (int i = 0; i < grid.size(); ++i) {
            Camera cam;
            QJsonObject slotObj = grid.at(i).toObject();
            const QString ip = slotObj.value("ip").toString();
            if (!ip.isEmpty()) {
                cam = m_cameraModel->findByIp(ip);
                if (cam.ip.isEmpty()) {
                    cam = cameraFromJson(slotObj.value("camera").toObject());
                }
            }
            cam.spanRows = slotObj.value("spanRows").toInt(1);
            cam.spanCols = slotObj.value("spanCols").toInt(1);

            // Migration: If span is small (<= 8) and we are moving to 1200-col grid, scale it up
            // Only apply this for legacy grids where the logical dimensions were small (<= 8)
            // This prevents corrupting high-density grids (e.g. 64x64) where spans are naturally small (~18)
            // and prevents double-scaling modern grids where spans are already 1200-based.
            int oldCols = m_gridCols > 0 ? m_gridCols : 2;
            int oldRows = m_gridRows > 0 ? m_gridRows : 2;

            if (oldCols <= 8 && cam.spanCols <= 8) {
                cam.spanCols = std::max(1, (1200 / oldCols) * cam.spanCols);
            }
            if (oldRows <= 8 && cam.spanRows <= 8) {
                cam.spanRows = std::max(1, (1200 / oldRows) * cam.spanRows);
            }

            m_gridModel->addCamera(cam);
        }
    }

    // Safety fallback: if grid is empty or dimensions look uninitialized, reset to 2x2
    if (m_gridModel->rowCount() == 0 || m_gridRows < 1 || m_gridCols < 1) {
        qWarning() << "Invalid grid state detected after load, resetting to 2x2 default";
        m_gridRows = 2;
        m_gridCols = 2;
        applyLayoutPreset(2, 2);
    }

    emit cameraGroupsChanged();
}

QString SystemController::stateDatabasePath() const
{
    const QString baseDir = AppPaths::dataDirectory();
    QDir().mkpath(baseDir);
    return baseDir + QStringLiteral("/state.sqlite3");
}
