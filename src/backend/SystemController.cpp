#include "SystemController.h"
#include "PathUtils.h"
#include "StateStore.h"
#include "CameraOnboardingParser.h"
#include "CameraStatusPolicy.h"
#include "StatusChecker.h"
#include "gst/StreamHealthPolicy.h"
#include "gst/StreamQualityPolicy.h"
#include "gst/StreamSessionPolicy.h"
#include "web/DashboardWebServer.h"
#include <QUuid>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QTimer>
#include <algorithm>
#include <QDate>
#include <QDateTime>
#include <QDirIterator>
#include <QCoreApplication>
#include <QImage>
#include <QImageReader>
#include <QClipboard>
#include <QCryptographicHash>
#include <QGuiApplication>
#include <QThread>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QUrlQuery>
#include <QDesktopServices>
#include <QEventLoop>
#include <QTcpSocket>
#include <QSharedPointer>
#include <keychain.h>
#include <QTextStream>
#include <QRegularExpression>
#include <QAudioSource>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QIODevice>
#include <QtEndian>
#include <cmath>
#include <cstring>
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#undef min
#undef max
#elif defined(Q_OS_LINUX)
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace {

constexpr int kRecordingSegmentMinMinutes = 5;
constexpr int kRecordingSegmentMaxMinutes = 60;
constexpr int kRecordingSegmentStepMinutes = 5;
constexpr int kRecordingSegmentDefaultMinutes = 15;

QString normalizedLocalPath(const QString &pathOrUrl)
{
    return PathUtils::localPathFromUserInput(pathOrUrl);
}

void normalizeAppSettingPaths(QVariantMap &settings)
{
    const QStringList pathKeys{
        QStringLiteral("recordingsPath"),
        QStringLiteral("screenshotsPath")
    };

    for (const QString &key : pathKeys) {
        const QString value = settings.value(key).toString();
        if (!value.trimmed().isEmpty()) {
            settings[key] = normalizedLocalPath(value);
        }
    }
}

int normalizedImportedPlayerFillMode(const QVariant &value)
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

int normalizedImportedRecordingSegmentDuration(const QVariant &value)
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

void normalizeImportedAppSettings(QVariantMap &settings)
{
    normalizeAppSettingPaths(settings);
    if (settings.contains(QStringLiteral("playerFillMode"))) {
        settings[QStringLiteral("playerFillMode")] =
            normalizedImportedPlayerFillMode(settings.value(QStringLiteral("playerFillMode")));
    }
    if (settings.contains(QStringLiteral("recordingSegmentDuration"))) {
        settings[QStringLiteral("recordingSegmentDuration")] =
            normalizedImportedRecordingSegmentDuration(settings.value(QStringLiteral("recordingSegmentDuration")));
    }
}

QStringList splitDiscoveryList(const QString &value)
{
    return value.split(QStringLiteral(", "), Qt::SkipEmptyParts);
}

void appendUniqueDiscoveryValue(QString *target, const QString &value)
{
    if (!target) return;
    const QString trimmed = value.trimmed();
    if (trimmed.isEmpty()) return;
    QStringList parts = splitDiscoveryList(*target);
    for (const QString &part : std::as_const(parts)) {
        if (part.compare(trimmed, Qt::CaseInsensitive) == 0) return;
    }
    parts.append(trimmed);
    *target = parts.join(QStringLiteral(", "));
}

QString normalizedOnboardingProfile(const QString &profile)
{
    const QString normalized = profile.trimmed().toLower();
    if (normalized == QStringLiteral("onvif")) return QStringLiteral("onvif");
    if (normalized == QStringLiteral("rtsp") || normalized == QStringLiteral("rtsp-manual"))
        return QStringLiteral("rtsp");
    return QStringLiteral("openipc");
}

#ifdef Q_OS_LINUX
bool readLinuxSystemCpuTotal(quint64 &total)
{
    QFile file(QStringLiteral("/proc/stat"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const QList<QByteArray> parts = file.readLine().simplified().split(' ');
    if (parts.size() < 5 || parts.at(0) != QByteArrayLiteral("cpu")) {
        return false;
    }

    quint64 parsedTotal = 0;
    for (int i = 1; i < parts.size(); ++i) {
        bool ok = false;
        const quint64 value = parts.at(i).toULongLong(&ok);
        if (!ok) {
            return false;
        }
        parsedTotal += value;
    }

    total = parsedTotal;
    return total > 0;
}

bool readLinuxProcessCpuTotal(quint64 &total)
{
    QFile file(QStringLiteral("/proc/self/stat"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const QByteArray data = file.readAll();
    const int commEnd = data.lastIndexOf(')');
    if (commEnd < 0 || commEnd + 2 >= data.size()) {
        return false;
    }

    // Fields after ") " start with stat field 3. utime/stime are fields 14/15.
    const QList<QByteArray> parts = data.mid(commEnd + 2).simplified().split(' ');
    if (parts.size() <= 12) {
        return false;
    }

    bool okUser = false;
    bool okSystem = false;
    const quint64 userTime = parts.at(11).toULongLong(&okUser);
    const quint64 systemTime = parts.at(12).toULongLong(&okSystem);
    if (!okUser || !okSystem) {
        return false;
    }

    total = userTime + systemTime;
    return true;
}
#endif

} // namespace

SystemController::SystemController(QObject *parent)
    : QObject(parent)
    , m_serviceStatus("Stopped")
    , m_process(new QProcess(this))
    , m_cameraModel(new CameraModel(this))
    , m_discoveryModel(new CameraModel(this))
    , m_saveTimer(new QTimer(this))
    , m_gridModel(new CameraModel(this))
    , m_analyticsEngine(new AnalyticsEngine(this))
    , m_userManager(new UserManager(this))
    , m_logModel(new LogModel(this))
    , m_ptzController(new PtzController(this))
    , m_camexController(new CamexController(this))
    , m_dahuaDiscovery(new DiscoveryController(this))
    , m_archiveController(new ArchiveController(this))
    , m_majesticClient(new MajesticClient(this))
    , m_firmwareClient(new OpenIpcFirmwareClient(this))
    , m_networkDiscovery(new NetworkDiscoveryService(this))
    , m_appUpdateChecker(new AppUpdateChecker(this))
    , m_cameraHealthController(new CameraHealthController(m_cameraModel, m_gridModel, this))
    , m_fleetManager(new FleetManager(m_cameraModel, m_cameraHealthController,
                                      m_firmwareClient, m_majesticClient,
                                      m_userManager, m_logModel, this))
    , m_incidentManager(new IncidentManager(m_cameraModel, this))
    , m_presentation(new DashboardPresentation(this))
    , m_statusChecker(new StatusChecker(m_cameraModel, this))
    , m_webServer(new DashboardWebServer(this, this))
    , m_networkManager(new QNetworkAccessManager(this))
{
    m_incidentManager->setLocationResolver(
        [this](const QString &cameraId, const QString &cameraIp) {
            QString resolvedId = cameraId.trimmed();
            if (resolvedId.isEmpty() && !cameraIp.trimmed().isEmpty()) {
                const int index = m_cameraModel->findIndexByIp(cameraIp.trimmed());
                if (index >= 0) {
                    const Camera camera = m_cameraModel->getCamera(index);
                    resolvedId = camera.id.trimmed().isEmpty() ? camera.ip : camera.id;
                }
            }
            return m_fleetManager->cameraAssignment(resolvedId);
        });
    m_userManager->setCameraScopeResolver(
        [this](const QString &cameraId, const QString &cameraIp, int cameraIndex) {
            return m_fleetManager->scopeAliases(cameraId, cameraIp, cameraIndex);
        });
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(1000); // 1 second debounce
    connect(m_saveTimer, &QTimer::timeout, this, &SystemController::performSave);
    
    connect(m_networkManager, &QNetworkAccessManager::authenticationRequired, this, &SystemController::onAuthenticationRequired);
    connect(m_userManager, &UserManager::permissionsVersionChanged, this, [this]() {
        if (m_pushToTalkActive && !m_userManager->canTalk()) {
            stopPushToTalk();
        }
    });
    connect(this, &SystemController::cameraEndpointProbeFinished, this,
            [this](const QString &requestId, const QString &, const QString &, int,
                   bool success, const QString &message, int httpStatus, int elapsedMs) {
        handleDiscoveryValidationProbe(requestId, success, message, httpStatus, elapsedMs);
    });
    connect(m_statusChecker, &StatusChecker::cameraStatusResolved,
            this, &SystemController::updateCameraStatus);
    connect(m_statusChecker, &StatusChecker::cameraStatusDetailResolved,
            this, &SystemController::updateCameraStatusDetail);
    
    // Connect Dahua discovery to main discovery model
    connect(m_dahuaDiscovery, &DiscoveryController::deviceFound, this, [this](const DiscoveredDevice& dev){
        Camera cam;
        cam.id = QUuid::createUuid().toString();
        cam.ip = dev.ip;
        cam.port = 554; // RTSP port assumption
        cam.onvifPort = 80;
        cam.name = dev.type.isEmpty() ? "Dahua Camera" : dev.type;
        cam.login = "admin";
        cam.status = "Discovered";
        cam.serialNumber = dev.serial;
        cam.manufacturer = dev.manufacturer;
        cam.discoveryMethods = QStringLiteral("Dahua SDK");
        cam.discoveryEvidence = QStringLiteral("Dahua device search response");
        cam.discoveryConfidence = 90;
        cam.onboardingProfile = QStringLiteral("rtsp");
        mergeDiscoveryCamera(cam);
    });

    connect(m_networkDiscovery, &NetworkDiscoveryService::candidateFound, this,
            [this](const NetworkDiscoveryCandidate &candidate) {
        Camera cam;
        cam.id = QUuid::createUuid().toString();
        cam.ip = candidate.ip;
        cam.port = candidate.rtspPort > 0 ? candidate.rtspPort : 554;
        cam.onvifPort = candidate.onvifPort > 0 ? candidate.onvifPort
                                                : (candidate.httpPort > 0 ? candidate.httpPort : 80);
        cam.hdStreamUrl = QStringLiteral("rtsp://%1:%2/stream=0").arg(cam.ip).arg(cam.port);
        cam.sdStreamUrl = QStringLiteral("rtsp://%1:%2/stream=1").arg(cam.ip).arg(cam.port);
        cam.streamUrl = cam.hdStreamUrl;
        cam.status = QStringLiteral("Discovered");
        cam.discoveryConfidence = candidate.confidence;
        cam.isOpenIpc = candidate.openIpc;
        if (candidate.openIpc) {
            cam.manufacturer = QStringLiteral("OpenIPC");
            cam.login = QStringLiteral("root");
            cam.name = candidate.name.isEmpty() ? QStringLiteral("OpenIPC Camera") : candidate.name;
            cam.onboardingProfile = QStringLiteral("openipc");
        } else {
            cam.manufacturer = candidate.manufacturer;
            cam.name = candidate.name.isEmpty() ? QStringLiteral("Network Camera") : candidate.name;
            cam.onboardingProfile = candidate.onvif ? QStringLiteral("onvif") : QStringLiteral("rtsp");
        }
        cam.serialNumber = candidate.serial;
        cam.discoveryMethods = candidate.method;
        cam.discoveryEvidence = candidate.evidence;
        mergeDiscoveryCamera(cam);
    });

    connect(m_networkDiscovery, &NetworkDiscoveryService::finished, this,
            [this](bool cancelled) {
        if (!cancelled) {
            m_discoveryLastUpdated = QDateTime::currentDateTime().toString(Qt::ISODate);
            emit discoverySessionChanged();
            saveState();
        }
    });

    connect(m_archiveController, &ArchiveController::exportStarted, this,
            [this](const QString &outputFile) {
        addLog(QtInfoMsg, QStringLiteral("Archive export started: %1").arg(outputFile));
        m_incidentManager->ingestEvent({
            {QStringLiteral("source"), QStringLiteral("archive")},
            {QStringLiteral("category"), QStringLiteral("archive")},
            {QStringLiteral("type"), QStringLiteral("export-started")},
            {QStringLiteral("severity"), QStringLiteral("info")},
            {QStringLiteral("title"), QStringLiteral("Archive export started")},
            {QStringLiteral("message"), outputFile},
            {QStringLiteral("attributes"), QVariantMap{{QStringLiteral("outputFile"), outputFile}}}
        });
    });
    connect(m_archiveController, &ArchiveController::exportFinished, this,
            [this]() {
        addLog(QtInfoMsg,
               QStringLiteral("Archive export finished: %1").arg(m_archiveController->exportOutputFile()));
        const QString outputFile = m_archiveController->exportOutputFile();
        m_incidentManager->ingestEvent({
            {QStringLiteral("source"), QStringLiteral("archive")},
            {QStringLiteral("category"), QStringLiteral("archive")},
            {QStringLiteral("type"), QStringLiteral("export-finished")},
            {QStringLiteral("severity"), QStringLiteral("info")},
            {QStringLiteral("title"), QStringLiteral("Archive export finished")},
            {QStringLiteral("message"), outputFile},
            {QStringLiteral("evidence"), QVariantList{QVariantMap{
                 {QStringLiteral("kind"), QStringLiteral("recording-export")},
                 {QStringLiteral("path"), outputFile}}}}
        });
    });
    connect(m_archiveController, &ArchiveController::exportError, this,
            [this](const QString &error) {
        addLog(QtWarningMsg, QStringLiteral("Archive export failed: %1").arg(error.left(500)));
        m_incidentManager->ingestEvent({
            {QStringLiteral("source"), QStringLiteral("archive")},
            {QStringLiteral("category"), QStringLiteral("archive")},
            {QStringLiteral("type"), QStringLiteral("export-failed")},
            {QStringLiteral("severity"), QStringLiteral("error")},
            {QStringLiteral("title"), QStringLiteral("Archive export failed")},
            {QStringLiteral("message"), error}
        });
    });
    connect(m_archiveController, &ArchiveController::cleanupFinished, this,
            [this](const QVariantMap &result) {
        const bool dryRun = result.value(QStringLiteral("dryRun")).toBool();
        const int count = dryRun
            ? result.value(QStringLiteral("wouldDeleteCount")).toInt()
            : result.value(QStringLiteral("deletedCount")).toInt();
        const QString sizeText = dryRun
            ? result.value(QStringLiteral("wouldDeleteSizeText")).toString()
            : result.value(QStringLiteral("deletedSizeText")).toString();
        const QString action = dryRun ? QStringLiteral("Archive cleanup preview")
                                      : QStringLiteral("Archive cleanup completed");
        addLog(QtInfoMsg, QStringLiteral("%1: %2 files, %3, %4")
                            .arg(action,
                                 QString::number(count),
                                 sizeText,
                                 result.value(QStringLiteral("rootPath")).toString()));
    });

    m_analyticsEngine->initialize();
    connect(m_analyticsEngine, &AnalyticsEngine::settingsChanged, this, &SystemController::saveState);
    connect(m_analyticsEngine, &AnalyticsEngine::analyticsEventsChanged, this, [this]() {
        const QVariantList events = m_analyticsEngine->analyticsEvents();
        for (const QVariant &value : events) {
            m_incidentManager->ingestAnalyticsEvent(value.toMap());
        }
    });
    connect(m_cameraHealthController, &CameraHealthController::runCompleted,
            this, [this](const QString &runId) {
        m_incidentManager->ingestHealthRun(m_cameraHealthController->runById(runId));
    });
    connect(m_fleetManager, &FleetManager::auditEvent,
            this, [this](const QVariantMap &event) {
        m_incidentManager->ingestAuditEvent(event);
    });
    
    // Default settings
    m_appSettings["language"] = "ru";
    m_appSettings["recordingsPath"] = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    m_appSettings["screenshotsPath"] = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    m_appSettings["hwAccel"] = "auto";
    m_appSettings["notificationsEnabled"] = true;
    m_appSettings["preferredStream"] = "auto";
    // 0 = fit (letterbox), -1 = crop; default to fit for correct aspect
    m_appSettings["playerFillMode"] = 0;
    m_appSettings["showStatsOverlay"] = true;
    m_appSettings["defaultAutoplay"] = true;
    m_appSettings["sidebarVisible"] = true;
    m_appSettings["sidebarToolsExpanded"] = true;
    // Disable analytics by default to avoid crashes from heavy ONNX inference on low GPUs
    m_appSettings["analyticsEnabled"] = false;
    // Sane grid defaults; avoid spawning hundreds of cells when state.json is absent
    m_appSettings["gridRows"] = 2;
    m_appSettings["gridCols"] = 2;
    
    // Player settings
    m_appSettings["playerBufferMode"] = 1; // Balanced
    m_appSettings["playerRtspTransport"] = "tcp";
    m_appSettings["recordingSegmentDuration"] = 15;
    // Web access is deliberately localhost-only and disabled until explicitly enabled.
    m_appSettings["webServerEnabled"] = false;
    m_appSettings["webServerAllowRemote"] = false;
    m_appSettings["webDeploymentProfile"] = "localhost";
    m_appSettings["webServerBindAddress"] = "127.0.0.1";
    m_appSettings["webServerPort"] = 8080;
    m_appSettings["webSocketPort"] = 8081;
    m_appSettings["webSessionTimeoutMinutes"] = 60;
    m_appSettings["webSecureCookies"] = false;
    m_appSettings["webExternalBaseUrl"] = "";
    m_appSettings["webExternalWebSocketUrl"] = "";
    m_appSettings["webTrustedProxyAddresses"] = "";

    m_gridRows = 2;
    m_gridCols = 2; 

    loadState();
    for (const QVariant &value : m_analyticsEngine->analyticsEvents()) {
        m_incidentManager->ingestAnalyticsEvent(value.toMap());
    }
    
    // QML smoke tests instantiate the real controller to verify bindings, but
    // they should not probe saved cameras or touch the LAN.
    if (qEnvironmentVariable("OPENIPC_SMOKE_QML") == QStringLiteral("1")) {
        qInfo() << "QML smoke mode: camera status monitoring disabled";
    } else {
        m_webServer->applySettings(m_appSettings);
        m_statusChecker->start();
    }

    // If no saved state, ensure default 2x2 grid placeholders
    if (m_gridModel->rowCount() == 0) {
        for (int i = 0; i < 4; ++i) {
            m_gridModel->addCamera(Camera());
        }
    }

    // Auto-save on significant changes
    connect(m_cameraModel, &QAbstractListModel::rowsInserted, this, &SystemController::saveState);
    connect(m_cameraModel, &QAbstractListModel::rowsRemoved, this, &SystemController::saveState);
    // Use lambda to swallow arguments for dataChanged
    connect(m_cameraModel, &QAbstractListModel::dataChanged, this, [this](){ saveState(); });
    connect(m_cameraModel, &QAbstractListModel::rowsInserted, this, [this]() { markDiscoveryAddedFlags(); });
    connect(m_cameraModel, &QAbstractListModel::rowsRemoved, this, [this]() { markDiscoveryAddedFlags(); });
    connect(m_cameraModel, &QAbstractListModel::dataChanged, this, [this]() { markDiscoveryAddedFlags(); });

    connect(m_discoveryModel, &QAbstractListModel::rowsInserted, this, &SystemController::saveState);
    connect(m_discoveryModel, &QAbstractListModel::rowsRemoved, this, &SystemController::saveState);
    connect(m_discoveryModel, &QAbstractListModel::dataChanged, this, [this](){ saveState(); });

    connect(m_gridModel, &QAbstractListModel::rowsInserted, this, &SystemController::saveState);
    connect(m_gridModel, &QAbstractListModel::rowsRemoved, this, &SystemController::saveState);
    connect(m_gridModel, &QAbstractListModel::dataChanged, this, [this](){ saveState(); });
    connect(m_cameraHealthController, &CameraHealthController::historyChanged,
            this, &SystemController::saveState);
    connect(m_fleetManager, &FleetManager::stateChanged,
            this, &SystemController::saveState);
    connect(m_incidentManager, &IncidentManager::stateChanged,
            this, &SystemController::saveState);
}

void SystemController::setIsArchiveOpen(bool open)
{
    if (m_isArchiveOpen == open) return;
    qInfo() << "SystemController::setIsArchiveOpen" << open;
    m_isArchiveOpen = open;
    emit isArchiveOpenChanged();
}

QVariantList SystemController::getNetworkInterfaces()
{
    QVariantList list;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        // Filter for active, non-loopback interfaces that have an IPv4 address
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            
            QList<QNetworkAddressEntry> entries = iface.addressEntries();
            QString ip;
            int prefixLength = -1;
            for (const QNetworkAddressEntry &entry : entries) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ip = entry.ip().toString();
                    prefixLength = entry.prefixLength();
                    break;
                }
            }
            
            if (!ip.isEmpty()) {
                QVariantMap map;
                map["name"] = iface.humanReadableName(); // Display name
                map["id"] = iface.name(); // Internal name (e.g., eth0, {UUID})
                map["ip"] = ip;
                map["prefixLength"] = prefixLength;
                list.append(map);
            }
        }
    }
    return list;
}

QString SystemController::discoverySessionSummary() const
{
    if (m_discoveryModel->rowCount() == 0) {
        return QStringLiteral("Результатов поиска пока нет");
    }
    QString summary = QStringLiteral("Сохранено результатов: %1").arg(m_discoveryModel->rowCount());
    if (!m_discoveryLastUpdated.isEmpty()) {
        summary += QStringLiteral(" · последний поиск: %1").arg(m_discoveryLastUpdated);
    }
    if (!m_discoveryLastInterface.isEmpty()) {
        summary += QStringLiteral(" · интерфейс: %1").arg(m_discoveryLastInterface);
    } else {
        summary += QStringLiteral(" · все интерфейсы");
    }
    if (m_discoveryLastDeepScan) {
        summary += QStringLiteral(" · глубокий режим");
    }
    return summary;
}

void SystemController::clearDiscoveryResults()
{
    m_discoveryValidationProbes.clear();
    m_discoveryValidationRemaining.clear();
    m_discoveryValidationFailed.clear();
    m_discoveryValidationMessages.clear();
    m_discoveryValidationCompleted = 0;
    m_discoveryValidationTotal = 0;
    m_discoveryLastUpdated.clear();
    m_discoveryLastInterface.clear();
    m_discoveryLastDeepScan = false;
    m_discoveryModel->clear();
    emit discoverySessionChanged();
    saveState();
}

void SystemController::refreshDiscoveryAddedFlags()
{
    markDiscoveryAddedFlags();
}

int SystemController::findDiscoveryMergeIndex(const Camera &candidate) const
{
    const QString ip = candidate.ip.trimmed();
    if (!ip.isEmpty()) {
        const int ipIndex = m_discoveryModel->findIndexByIp(ip);
        if (ipIndex >= 0) return ipIndex;
    }

    const QString serial = candidate.serialNumber.trimmed();
    if (!serial.isEmpty()) {
        for (int i = 0; i < m_discoveryModel->rowCount(); ++i) {
            const Camera existing = m_discoveryModel->getCamera(i);
            if (!existing.serialNumber.trimmed().isEmpty()
                && existing.serialNumber.compare(serial, Qt::CaseInsensitive) == 0) {
                return i;
            }
        }
    }

    return -1;
}

void SystemController::mergeDiscoveryCamera(const Camera &incoming)
{
    if (incoming.ip.trimmed().isEmpty()) return;

    const int existingIndex = findDiscoveryMergeIndex(incoming);
    Camera cam = existingIndex >= 0 ? m_discoveryModel->getCamera(existingIndex) : Camera{};
    if (cam.id.isEmpty()) cam.id = incoming.id.isEmpty() ? QUuid::createUuid().toString() : incoming.id;

    cam.ip = incoming.ip.trimmed();
    if (incoming.port > 0) cam.port = incoming.port;
    if (incoming.onvifPort > 0) cam.onvifPort = incoming.onvifPort;
    if (!incoming.hdStreamUrl.isEmpty()) cam.hdStreamUrl = incoming.hdStreamUrl;
    if (!incoming.sdStreamUrl.isEmpty()) cam.sdStreamUrl = incoming.sdStreamUrl;
    if (cam.streamUrl.isEmpty()) cam.streamUrl = cam.hdStreamUrl;
    if (cam.streamUrl.isEmpty() && !cam.ip.isEmpty()) {
        cam.streamUrl = QStringLiteral("rtsp://%1:%2/stream=0").arg(cam.ip).arg(cam.port > 0 ? cam.port : 554);
    }

    const bool incomingIsBetterName = cam.name.isEmpty()
        || cam.name == QStringLiteral("Network Camera")
        || cam.name == QStringLiteral("RTSP Camera")
        || cam.name == QStringLiteral("ONVIF Camera")
        || incoming.isOpenIpc;
    if (!incoming.name.trimmed().isEmpty() && incomingIsBetterName) cam.name = incoming.name.trimmed();
    if (cam.name.isEmpty()) cam.name = QStringLiteral("Network Camera");

    if (!incoming.manufacturer.trimmed().isEmpty()
        && (cam.manufacturer.isEmpty() || incoming.isOpenIpc)) {
        cam.manufacturer = incoming.manufacturer.trimmed();
    }
    if (!incoming.serialNumber.trimmed().isEmpty()) cam.serialNumber = incoming.serialNumber.trimmed();
    if (!incoming.login.trimmed().isEmpty() && cam.login.isEmpty()) cam.login = incoming.login.trimmed();

    cam.isOpenIpc = cam.isOpenIpc || incoming.isOpenIpc;
    cam.discoveryConfidence = std::max(cam.discoveryConfidence, incoming.discoveryConfidence);
    appendUniqueDiscoveryValue(&cam.discoveryMethods, incoming.discoveryMethods);
    appendUniqueDiscoveryValue(&cam.discoveryEvidence, incoming.discoveryEvidence);
    if (cam.discoveryEvidence.isEmpty()) {
        cam.discoveryEvidence = QStringLiteral("Discovered on local network");
    }

    const QString incomingProfile = normalizedOnboardingProfile(incoming.onboardingProfile);
    if (cam.onboardingProfile.isEmpty()
        || (incomingProfile == QStringLiteral("openipc") && cam.onboardingProfile != QStringLiteral("openipc"))) {
        cam.onboardingProfile = incomingProfile;
    }

    cam.alreadyAdded = m_cameraModel->contains(cam.ip);
    cam.status = cam.alreadyAdded ? QStringLiteral("Already added") : QStringLiteral("Discovered");
    if (cam.alreadyAdded && cam.validationStatus != QStringLiteral("running")) {
        cam.validationStatus = QStringLiteral("added");
        cam.validationMessage = QStringLiteral("Камера уже есть в списке устройств");
    } else if (cam.validationStatus.isEmpty()) {
        cam.validationStatus = QStringLiteral("idle");
        cam.validationMessage = QStringLiteral("Готово к проверке перед добавлением");
    }

    if (existingIndex >= 0) m_discoveryModel->setCamera(existingIndex, cam);
    else m_discoveryModel->addCamera(cam);

    m_discoveryLastUpdated = QDateTime::currentDateTime().toString(Qt::ISODate);
    emit discoverySessionChanged();
}

void SystemController::markDiscoveryAddedFlags()
{
    for (int i = 0; i < m_discoveryModel->rowCount(); ++i) {
        Camera cam = m_discoveryModel->getCamera(i);
        const bool added = !cam.ip.isEmpty() && m_cameraModel->contains(cam.ip);
        if (cam.alreadyAdded == added
            && ((added && cam.validationStatus == QStringLiteral("added"))
                || (!added && cam.validationStatus != QStringLiteral("added")))) {
            continue;
        }
        cam.alreadyAdded = added;
        cam.status = added ? QStringLiteral("Already added") : QStringLiteral("Discovered");
        if (added) {
            cam.validationStatus = QStringLiteral("added");
            cam.validationMessage = QStringLiteral("Камера уже есть в списке устройств");
        } else if (cam.validationStatus == QStringLiteral("added")) {
            cam.validationStatus = QStringLiteral("idle");
            cam.validationMessage = QStringLiteral("Готово к проверке перед добавлением");
        }
        m_discoveryModel->setCamera(i, cam);
    }
}

void SystemController::setDiscoveryValidationState(int index, const QString &status,
                                                   const QString &message)
{
    if (index < 0 || index >= m_discoveryModel->rowCount()) return;
    Camera cam = m_discoveryModel->getCamera(index);
    cam.validationStatus = status;
    cam.validationMessage = message;
    m_discoveryModel->setCamera(index, cam);
}

QString SystemController::discoveryProfileForCamera(const Camera &camera,
                                                    const QString &requestedProfile) const
{
    const QString profile = normalizedOnboardingProfile(requestedProfile);
    if (!requestedProfile.trimmed().isEmpty()) return profile;
    if (!camera.onboardingProfile.trimmed().isEmpty()) {
        return normalizedOnboardingProfile(camera.onboardingProfile);
    }
    if (camera.isOpenIpc) return QStringLiteral("openipc");
    if (camera.discoveryMethods.contains(QStringLiteral("ONVIF"), Qt::CaseInsensitive)) {
        return QStringLiteral("onvif");
    }
    return QStringLiteral("rtsp");
}

QString SystemController::rtspPathForProfile(const Camera &camera, const QString &profile,
                                             bool subStream) const
{
    const QString url = subStream ? camera.sdStreamUrl : camera.hdStreamUrl;
    const QUrl parsed(url);
    if (parsed.isValid() && parsed.scheme().startsWith(QStringLiteral("rtsp"))
        && !parsed.path().isEmpty()) {
        QString path = parsed.path();
        if (parsed.hasQuery()) path += QLatin1Char('?') + parsed.query();
        return path;
    }

    const QString normalized = normalizedOnboardingProfile(profile);
    if (normalized == QStringLiteral("openipc")) {
        return subStream ? QStringLiteral("/stream=1") : QStringLiteral("/stream=0");
    }
    return subStream ? QStringLiteral("/stream=1") : QStringLiteral("/stream=0");
}

QString SystemController::buildSanitizedRtspUrl(const Camera &camera, const QString &profile,
                                                bool subStream) const
{
    const int port = camera.port > 0 ? camera.port : 554;
    QString path = rtspPathForProfile(camera, profile, subStream);
    if (!path.startsWith(QLatin1Char('/'))) path.prepend(QLatin1Char('/'));
    return QStringLiteral("rtsp://%1:%2%3").arg(camera.ip).arg(port).arg(path);
}

Camera SystemController::cameraFromDiscoveryForAdd(const Camera &source,
                                                   const QString &profile,
                                                   const QString &login,
                                                   const QString &password) const
{
    Camera cam = source;
    cam.id = QUuid::createUuid().toString();
    cam.name = source.name.trimmed().isEmpty() ? source.ip : source.name.trimmed();
    cam.ip = source.ip.trimmed();
    cam.port = source.port > 0 ? source.port : 554;
    cam.onvifPort = source.onvifPort > 0 ? source.onvifPort : 80;
    cam.login = login.trimmed();
    cam.password = password;
    cam.onboardingProfile = discoveryProfileForCamera(source, profile);
    cam.hdStreamUrl = buildSanitizedRtspUrl(source, cam.onboardingProfile, false);
    cam.sdStreamUrl = buildSanitizedRtspUrl(source, cam.onboardingProfile, true);
    cam.streamUrl = cam.hdStreamUrl;
    cam.status = QStringLiteral("Online");
    cam.alreadyAdded = false;
    cam.validationStatus.clear();
    cam.validationMessage.clear();
    return cam;
}

void SystemController::validateDiscoverySelection(const QVariantList &indexes,
                                                  const QString &login,
                                                  const QString &password,
                                                  const QString &profile)
{
    m_discoveryValidationProbes.clear();
    m_discoveryValidationRemaining.clear();
    m_discoveryValidationFailed.clear();
    m_discoveryValidationMessages.clear();
    m_discoveryValidationCompleted = 0;
    m_discoveryValidationTotal = 0;

    QSet<int> uniqueIndexes;
    for (const QVariant &value : indexes) {
        bool ok = false;
        const int index = value.toInt(&ok);
        if (ok && index >= 0 && index < m_discoveryModel->rowCount()) uniqueIndexes.insert(index);
    }

    int immediateOk = 0;
    int immediateFail = 0;
    for (int index : std::as_const(uniqueIndexes)) {
        Camera cam = m_discoveryModel->getCamera(index);
        cam.onboardingProfile = discoveryProfileForCamera(cam, profile);
        m_discoveryModel->setCamera(index, cam);

        if (cam.alreadyAdded || m_cameraModel->contains(cam.ip)) {
            setDiscoveryValidationState(index, QStringLiteral("added"),
                                        QStringLiteral("Камера уже есть в списке устройств"));
            ++immediateOk;
            continue;
        }

        setDiscoveryValidationState(index, QStringLiteral("running"),
                                    QStringLiteral("Проверка endpoints и credentials…"));

        struct ProbeSpec {
            QString kind;
            int port = 0;
            QString path;
            QString label;
        };

        QList<ProbeSpec> probes;
        const QString selectedProfile = discoveryProfileForCamera(cam, profile);
        if (selectedProfile == QStringLiteral("openipc")) {
            probes.append({QStringLiteral("majestic"), cam.onvifPort > 0 ? cam.onvifPort : 80,
                           QStringLiteral("/api/v1/config.json"), QStringLiteral("Majestic")});
            probes.append({QStringLiteral("rtsp"), cam.port > 0 ? cam.port : 554,
                           rtspPathForProfile(cam, selectedProfile, false), QStringLiteral("RTSP")});
        } else if (selectedProfile == QStringLiteral("onvif")) {
            probes.append({QStringLiteral("http"), cam.onvifPort > 0 ? cam.onvifPort : 80,
                           QStringLiteral("/onvif/device_service"), QStringLiteral("ONVIF")});
            probes.append({QStringLiteral("rtsp"), cam.port > 0 ? cam.port : 554,
                           rtspPathForProfile(cam, selectedProfile, false), QStringLiteral("RTSP")});
        } else {
            probes.append({QStringLiteral("rtsp"), cam.port > 0 ? cam.port : 554,
                           rtspPathForProfile(cam, selectedProfile, false), QStringLiteral("RTSP")});
        }

        m_discoveryValidationRemaining.insert(index, probes.size());
        m_discoveryValidationFailed.insert(index, false);
        m_discoveryValidationMessages.insert(index, QStringList());
        m_discoveryValidationTotal += probes.size();

        for (const ProbeSpec &probe : std::as_const(probes)) {
            const QString requestId = probeCameraEndpoint(probe.kind, cam.ip, probe.port,
                                                          probe.path, login.trimmed(), password);
            m_discoveryValidationProbes.insert(requestId, {index, probe.label});
        }
    }

    if (m_discoveryValidationTotal == 0) {
        emit discoveryValidationProgress(0, 0);
        emit discoveryValidationFinished(immediateOk, immediateFail);
    } else {
        emit discoveryValidationProgress(0, m_discoveryValidationTotal);
    }
}

void SystemController::handleDiscoveryValidationProbe(const QString &requestId, bool success,
                                                      const QString &message, int httpStatus,
                                                      int elapsedMs)
{
    if (!m_discoveryValidationProbes.contains(requestId)) return;

    const DiscoveryValidationProbe probe = m_discoveryValidationProbes.take(requestId);
    if (probe.index < 0 || probe.index >= m_discoveryModel->rowCount()) return;

    m_discoveryValidationCompleted += 1;
    QStringList messages = m_discoveryValidationMessages.value(probe.index);
    QString line = QStringLiteral("%1: %2").arg(probe.label, success ? QStringLiteral("OK") : message);
    if (httpStatus == 401 || httpStatus == 403) {
        line = QStringLiteral("%1: credentials не приняты").arg(probe.label);
    }
    if (elapsedMs > 0) line += QStringLiteral(" (%1 ms)").arg(elapsedMs);
    messages.append(line);
    m_discoveryValidationMessages.insert(probe.index, messages);
    if (!success) m_discoveryValidationFailed.insert(probe.index, true);

    int remaining = m_discoveryValidationRemaining.value(probe.index, 0);
    remaining = std::max(0, remaining - 1);
    m_discoveryValidationRemaining.insert(probe.index, remaining);

    if (remaining == 0) {
        const bool failed = m_discoveryValidationFailed.value(probe.index, false);
        setDiscoveryValidationState(probe.index,
                                    failed ? QStringLiteral("fail") : QStringLiteral("ok"),
                                    messages.join(QStringLiteral(" · ")));
    } else {
        setDiscoveryValidationState(probe.index, QStringLiteral("running"),
                                    messages.join(QStringLiteral(" · ")));
    }

    emit discoveryValidationProgress(m_discoveryValidationCompleted, m_discoveryValidationTotal);

    if (m_discoveryValidationCompleted >= m_discoveryValidationTotal) {
        int okCount = 0;
        int failCount = 0;
        for (auto it = m_discoveryValidationRemaining.constBegin();
             it != m_discoveryValidationRemaining.constEnd(); ++it) {
            if (m_discoveryValidationFailed.value(it.key(), false)) ++failCount;
            else ++okCount;
        }
        m_discoveryValidationRemaining.clear();
        m_discoveryValidationFailed.clear();
        m_discoveryValidationMessages.clear();
        m_discoveryValidationTotal = 0;
        m_discoveryValidationCompleted = 0;
        emit discoveryValidationFinished(okCount, failCount);
    }
}

int SystemController::addDiscoveredCameras(const QVariantList &indexes,
                                           const QString &login,
                                           const QString &password,
                                           const QString &profile)
{
    QSet<int> uniqueIndexes;
    for (const QVariant &value : indexes) {
        bool ok = false;
        const int index = value.toInt(&ok);
        if (ok && index >= 0 && index < m_discoveryModel->rowCount()) uniqueIndexes.insert(index);
    }

    int added = 0;
    int skipped = 0;
    QList<int> sorted = uniqueIndexes.values();
    std::sort(sorted.begin(), sorted.end());

    for (int index : std::as_const(sorted)) {
        Camera source = m_discoveryModel->getCamera(index);
        if (source.ip.trimmed().isEmpty() || m_cameraModel->contains(source.ip)) {
            setDiscoveryValidationState(index, QStringLiteral("added"),
                                        QStringLiteral("Камера уже есть в списке устройств"));
            ++skipped;
            continue;
        }

        Camera cam = cameraFromDiscoveryForAdd(source, profile, login, password);
        if (cam.ip.isEmpty()) {
            setDiscoveryValidationState(index, QStringLiteral("fail"),
                                        QStringLiteral("Не удалось подготовить параметры камеры"));
            ++skipped;
            continue;
        }

        m_cameraModel->addCamera(cam);
        if (!cam.password.isEmpty()) {
            auto job = new QKeychain::WritePasswordJob("OpenIPC");
            job->setAutoDelete(true);
            job->setKey(cam.ip);
            job->setTextData(cam.password);
            job->start();
        }

        source.alreadyAdded = true;
        source.status = QStringLiteral("Already added");
        source.validationStatus = QStringLiteral("added");
        source.validationMessage = QStringLiteral("Добавлена в список устройств");
        source.onboardingProfile = cam.onboardingProfile;
        m_discoveryModel->setCamera(index, source);
        ++added;
    }

    saveState();
    emit discoveryBatchAddFinished(added, skipped);
    return added;
}

QString SystemController::serviceStatus() const
{
    return m_serviceStatus;
}

CameraModel* SystemController::cameraModel() const
{
    return m_cameraModel;
}

CameraModel* SystemController::discoveryModel() const
{
    return m_discoveryModel;
}

DiscoveryController* SystemController::dahuaDiscovery() const
{
    return m_dahuaDiscovery;
}

ArchiveController* SystemController::archiveController() const
{
    return m_archiveController;
}

CameraModel* SystemController::gridModel() const
{
    return m_gridModel;
}

AnalyticsEngine* SystemController::analyticsEngine() const
{
    return m_analyticsEngine;
}

UserManager* SystemController::userManager() const
{
    return m_userManager;
}

LogModel* SystemController::logModel() const
{
    return m_logModel;
}

PtzController* SystemController::ptzController() const
{
    return m_ptzController;
}

QVariantMap SystemController::parseCameraQrPayload(const QString &payload) const
{
    return CameraOnboardingParser::parse(payload);
}

QString SystemController::xmSofiaPasswordHash(const QString &password) const
{
    static constexpr char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const QByteArray digest = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);

    QString result;
    result.reserve(8);
    for (int i = 0; i < 8 && (i * 2 + 1) < digest.size(); ++i) {
        const int first = static_cast<unsigned char>(digest.at(i * 2));
        const int second = static_cast<unsigned char>(digest.at(i * 2 + 1));
        result.append(QLatin1Char(alphabet[(first + second) % 62]));
    }
    return result;
}

QString SystemController::probeCameraEndpoint(const QString &kind,
                                              const QString &host,
                                              int port,
                                              const QString &path,
                                              const QString &username,
                                              const QString &password)
{
    const QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString normalizedKind = kind.trimmed().toLower();
    QString trimmedHost = host.trimmed();

    if (trimmedHost.isEmpty()) {
        QMetaObject::invokeMethod(this, [this, requestId, normalizedKind]() {
            emit cameraEndpointProbeFinished(requestId, normalizedKind, QString(), 0, false,
                                             QStringLiteral("Host is empty"), 0, 0);
        }, Qt::QueuedConnection);
        return requestId;
    }

    if (normalizedKind == QStringLiteral("rtsp")) {
        if (port <= 0) port = 554;

        auto *socket = new QTcpSocket(this);
        auto *timeout = new QTimer(socket);
        timeout->setSingleShot(true);
        timeout->setInterval(4500);

        const qint64 startedAt = QDateTime::currentMSecsSinceEpoch();
        const auto finished = QSharedPointer<bool>::create(false);
        const QString requestPath = path.trimmed().isEmpty()
            ? QStringLiteral("/stream=0")
            : (path.trimmed().startsWith(QLatin1Char('/')) ? path.trimmed()
                                                           : QLatin1Char('/') + path.trimmed());
        const QString rtspTarget = QStringLiteral("rtsp://%1:%2%3")
                                       .arg(trimmedHost)
                                       .arg(port)
                                       .arg(requestPath);

        const auto finish = [this, requestId, normalizedKind, trimmedHost, port, socket,
                             startedAt, finished](bool success, const QString &message) {
            if (*finished) {
                return;
            }
            *finished = true;
            const int elapsedMs = static_cast<int>(QDateTime::currentMSecsSinceEpoch() - startedAt);
            emit cameraEndpointProbeFinished(requestId, normalizedKind, trimmedHost, port, success,
                                             message, 0, elapsedMs);
            socket->abort();
            socket->deleteLater();
        };

        connect(socket, &QTcpSocket::connected, socket, [socket, rtspTarget]() {
            const QByteArray request = QByteArrayLiteral("OPTIONS ") + rtspTarget.toUtf8()
                + QByteArrayLiteral(" RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: OpenIPC-Dashboard\r\n\r\n");
            socket->write(request);
            socket->flush();
        });

        connect(socket, &QTcpSocket::readyRead, this, [socket, finish]() {
            const QByteArray response = socket->readAll();
            if (response.isEmpty()) {
                return;
            }

            const bool looksRtsp = response.startsWith("RTSP/")
                || response.contains("RTSP/1.0")
                || response.contains("RTSP/2.0");
            if (looksRtsp) {
                finish(true, QStringLiteral("RTSP endpoint responded"));
            } else {
                finish(true, QStringLiteral("RTSP TCP port is reachable, but response is not RTSP"));
            }
        });

        connect(socket, &QTcpSocket::errorOccurred, this,
                [socket, finish](QAbstractSocket::SocketError) {
            finish(false, socket->errorString());
        });

        connect(timeout, &QTimer::timeout, this, [socket, finish]() {
            if (socket->state() == QAbstractSocket::ConnectedState) {
                finish(true, QStringLiteral("RTSP TCP port is reachable, no OPTIONS response before timeout"));
            } else {
                finish(false, QStringLiteral("RTSP connection timed out"));
            }
        });

        timeout->start();
        socket->connectToHost(trimmedHost, static_cast<quint16>(port));
        return requestId;
    }

    if (normalizedKind == QStringLiteral("majestic") || normalizedKind == QStringLiteral("http")) {
        QUrl url(trimmedHost.contains(QStringLiteral("://"))
                     ? trimmedHost
                     : QStringLiteral("http://") + trimmedHost);
        if (url.scheme().isEmpty()) {
            url.setScheme(QStringLiteral("http"));
        }
        if (port > 0) {
            url.setPort(port);
        } else if (url.port() < 0) {
            port = url.scheme() == QStringLiteral("https") ? 443 : 80;
            url.setPort(port);
        } else {
            port = url.port();
        }

        QString requestPath = path.trimmed();
        if (requestPath.isEmpty()) {
            requestPath = normalizedKind == QStringLiteral("majestic")
                ? QStringLiteral("/api/v1/config.json")
                : QStringLiteral("/");
        }
        if (!requestPath.startsWith(QLatin1Char('/'))) {
            requestPath.prepend(QLatin1Char('/'));
        }
        url.setPath(requestPath);

        QNetworkRequest request(url);
        request.setTransferTimeout(6000);
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
        request.setRawHeader("User-Agent", "OpenIPC-Dashboard/0.2");
        request.setRawHeader("Accept", "application/json, text/plain, */*");
        if (!username.isEmpty()) {
            request.setRawHeader("Authorization",
                                 "Basic "
                                     + (username + QLatin1Char(':') + password).toUtf8().toBase64());
        }

        const qint64 startedAt = QDateTime::currentMSecsSinceEpoch();
        QNetworkReply *reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this,
                [this, reply, requestId, normalizedKind, url, startedAt]() {
            const QByteArray body = reply->readAll();
            const int elapsedMs = static_cast<int>(QDateTime::currentMSecsSinceEpoch() - startedAt);
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString message;
            bool success = false;

            if (reply->error() == QNetworkReply::NoError && status >= 200 && status < 300) {
                success = true;
                message = normalizedKind == QStringLiteral("majestic")
                    ? QStringLiteral("Majestic API responded")
                    : QStringLiteral("HTTP endpoint responded");
            } else if (status == 401 || status == 403) {
                message = QStringLiteral("HTTP authentication is required or credentials are invalid");
            } else if (status > 0) {
                message = QStringLiteral("HTTP endpoint returned an error");
            } else {
                message = reply->errorString();
            }

            emit cameraEndpointProbeFinished(requestId, normalizedKind, url.host(), url.port(), success,
                                             message, status, elapsedMs);
            reply->deleteLater();
        });
        return requestId;
    }

    QMetaObject::invokeMethod(this, [this, requestId, normalizedKind, trimmedHost, port]() {
        emit cameraEndpointProbeFinished(requestId, normalizedKind, trimmedHost, port, false,
                                         QStringLiteral("Unsupported endpoint probe type"), 0, 0);
    }, Qt::QueuedConnection);
    return requestId;
}

CamexController* SystemController::camexController() const
{
    return m_camexController;
}

void SystemController::addLog(QtMsgType type, const QString &msg)
{
    if (m_logModel) {
        if (m_logModel->thread() == QThread::currentThread()) {
            m_logModel->addLog(type, msg);
            return;
        }

        QMetaObject::invokeMethod(m_logModel, [this, type, msg]() {
            m_logModel->addLog(type, msg);
        }, Qt::QueuedConnection);
    }
}

void SystemController::startService()
{
    if (m_process->state() != QProcess::NotRunning) {
        qInfo() << "Service already running";
        return;
    }

    QString program = "go2rtc";
#ifdef Q_OS_WIN
    program = "go2rtc.exe";
#endif
    
    // Check various locations
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath(),
        QCoreApplication::applicationDirPath() + "/bin",
        QCoreApplication::applicationDirPath() + "/../bin"
    };
    
    QString executablePath;
    for (const QString &path : searchPaths) {
        QString candidate = QDir(path).filePath(program);
        if (QFile::exists(candidate)) {
            executablePath = candidate;
            break;
        }
    }

    if (executablePath.isEmpty()) {
        qWarning() << "go2rtc binary not found";
        m_serviceStatus = "Missing go2rtc";
        emit serviceStatusChanged();
        executablePath = program; // Fallback to PATH
    } else {
        qInfo() << "Found go2rtc at" << executablePath;
    }

    m_process->disconnect(this);

    connect(m_process, &QProcess::started, this, [this](){
        m_serviceStatus = "Running";
        emit serviceStatusChanged();
    });
    
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            this, [this](int code, QProcess::ExitStatus status){
        m_serviceStatus = (status == QProcess::CrashExit) ? "Crashed" : "Stopped";
        emit serviceStatusChanged();
        qInfo() << "go2rtc finished with code" << code;
    });

    m_process->setProgram(executablePath);
    m_process->start();
}

void SystemController::stopService()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
        }
    }
    m_serviceStatus = "Stopped";
    emit serviceStatusChanged();
}

void SystemController::scanNetwork(const QString &interfaceName, bool deepScan)
{
    qDebug() << "Starting network scan. Interface:" << (interfaceName.isEmpty() ? "All" : interfaceName);
    m_discoveryLastInterface = interfaceName;
    m_discoveryLastDeepScan = deepScan;
    markDiscoveryAddedFlags();
    emit discoverySessionChanged();
    
    // Start Dahua SDK Search
    if (m_dahuaDiscovery) {
        m_dahuaDiscovery->startSearch();
    }

    m_networkDiscovery->start(interfaceName, deepScan);
}

void SystemController::stopNetworkScan()
{
    if (m_networkDiscovery) m_networkDiscovery->stop();
    if (m_dahuaDiscovery) m_dahuaDiscovery->stopSearch();
}

void SystemController::addDevice(int index)
{
    Camera cam = m_discoveryModel->getCamera(index);
    if (!cam.id.isEmpty()) {
        if (!m_cameraModel->contains(cam.ip)) {
            Camera prepared = cameraFromDiscoveryForAdd(cam, cam.onboardingProfile, cam.login, QString());
            m_cameraModel->addCamera(prepared);
            markDiscoveryAddedFlags();
            saveState();
        }
    }
}

void SystemController::addManualCamera(const QString &name, const QString &ip, const QString &url, int port, int onvifPort, const QString &login, const QString &password, const QString &sdUrl)
{
    Camera cam;
    cam.id = QUuid::createUuid().toString();
    cam.name = name;
    cam.ip = ip;
    cam.streamUrl = url;
    
    // Use explicit SD URL if provided, otherwise fallback to heuristics
    cam.hdStreamUrl = url;
    if (!sdUrl.isEmpty()) {
        cam.sdStreamUrl = sdUrl;
    } else {
        // Legacy heuristics for OpenIPC/Generic
        if (url.contains("stream=0")) {
            cam.sdStreamUrl = QString(url).replace("stream=0", "stream=1");
        } else if (url.contains("stream=1")) {
            cam.sdStreamUrl = url;
            cam.hdStreamUrl = QString(url).replace("stream=1", "stream=0");
        } else {
             cam.sdStreamUrl = url;
        }
    }

    cam.status = "Online";
    cam.port = port;
    cam.onvifPort = onvifPort;
    cam.login = login;
    cam.password = password;
    
    qDebug() << "Adding camera:" << name << "IP:" << ip << "URL:" << url;

    if (!m_cameraModel->contains(cam.ip)) {
            m_cameraModel->addCamera(cam);
            
            if (!cam.password.isEmpty() && !cam.ip.isEmpty()) {
                auto job = new QKeychain::WritePasswordJob("OpenIPC");
                job->setAutoDelete(true);
                job->setKey(cam.ip);
                job->setTextData(cam.password);
                job->start();
            }
            
            markDiscoveryAddedFlags();
            saveState();
    }
}

void SystemController::updateCamera(int index, const QString &name, const QString &ip, const QString &url, int port, int onvifPort, const QString &login, const QString &password, const QString &sdUrl)
{
    Camera cam = m_cameraModel->getCamera(index);
    QString oldIp = cam.ip; // Save old IP for matching if ID fails

    cam.name = name;
    cam.ip = ip;
    cam.streamUrl = url;
    
    // Explicit SD URL support
    cam.hdStreamUrl = url;
    if (!sdUrl.isEmpty()) {
        cam.sdStreamUrl = sdUrl;
        qInfo() << "SystemController: Saved explicit SD URL for camera" << index << ":" << sdUrl;
    } else {
        // Fallback Heuristics
        qInfo() << "SystemController: No explicit SD URL provided for camera" << index << ", using fallback heuristics.";
        if (url.contains("stream=0")) {
            cam.sdStreamUrl = QString(url).replace("stream=0", "stream=1");
        } else if (url.contains("stream=1")) {
            cam.sdStreamUrl = url;
            cam.hdStreamUrl = QString(url).replace("stream=1", "stream=0");
        } else {
            cam.sdStreamUrl = url;
        }
    }

    cam.port = port;
    cam.onvifPort = onvifPort;
    cam.login = login;
    cam.password = password;
    
    m_cameraModel->setCamera(index, cam);

    if (!oldIp.isEmpty() && oldIp != cam.ip) {
        auto oldJob = new QKeychain::DeletePasswordJob("OpenIPC");
        oldJob->setAutoDelete(true);
        oldJob->setKey(oldIp);
        oldJob->start();
    }

    if (!cam.ip.isEmpty()) {
        if (cam.password.isEmpty()) {
            auto job = new QKeychain::DeletePasswordJob("OpenIPC");
            job->setAutoDelete(true);
            job->setKey(cam.ip);
            job->start();
        } else {
            auto job = new QKeychain::WritePasswordJob("OpenIPC");
            job->setAutoDelete(true);
            job->setKey(cam.ip);
            job->setTextData(cam.password);
            job->start();
        }
    }

    // Update grid model if this camera is present
    bool gridUpdated = false;
    for (int i = 0; i < m_gridModel->rowCount(); ++i) {
        Camera gridCam = m_gridModel->getCamera(i);
        
        bool match = false;
        // Try to match by ID first
        if (!cam.id.isEmpty() && !gridCam.id.isEmpty()) {
            if (gridCam.id == cam.id) match = true;
        } else {
            // Fallback to IP matching using old IP (in case IP was changed)
            if (gridCam.ip == oldIp) match = true;
        }

        if (match) {
            // Preserve grid-specific state (span)
            cam.spanRows = gridCam.spanRows;
            cam.spanCols = gridCam.spanCols;
            m_gridModel->setCamera(i, cam);
            gridUpdated = true;
            qInfo() << "SystemController: Updated camera in grid slot" << i;
        }
    }
    
    if (!gridUpdated) {
        qWarning() << "SystemController: Camera updated but not found in Grid to update live view. ID:" << cam.id << "IP:" << oldIp;
    }

    markDiscoveryAddedFlags();
    saveState();
}

void SystemController::updateCameraStatus(const QString &cameraIp, const QString &status)
{
    const QString ip = cameraIp.trimmed();
    QString normalizedStatus = status.trimmed();
    if (ip.isEmpty() || normalizedStatus.isEmpty()) {
        return;
    }

    if (normalizedStatus.compare(QStringLiteral("Online"), Qt::CaseInsensitive) == 0) {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const qint64 streamOfflineUntil = m_streamOfflineUntilMs.value(ip, 0);
        if (streamOfflineUntil > now) {
            normalizedStatus = QStringLiteral("Offline");
        } else {
            m_streamOfflineUntilMs.remove(ip);
            normalizedStatus = QStringLiteral("Online");
        }
    } else if (normalizedStatus.compare(QStringLiteral("Offline"), Qt::CaseInsensitive) == 0) {
        normalizedStatus = QStringLiteral("Offline");
    }

    const int listIndex = m_cameraModel->findIndexByIp(ip);
    if (listIndex >= 0) {
        m_cameraModel->setStatus(listIndex, normalizedStatus);
    }

    for (int i = 0; i < m_gridModel->rowCount(); ++i) {
        const Camera gridCamera = m_gridModel->getCamera(i);
        if (gridCamera.ip == ip) {
            m_gridModel->setStatus(i, normalizedStatus);
        }
    }
}

void SystemController::updateCameraStreamStatus(const QString &cameraIp, const QString &status)
{
    const QString ip = cameraIp.trimmed();
    const QString normalizedStatus = status.trimmed();
    if (ip.isEmpty() || normalizedStatus.isEmpty()) {
        return;
    }

    if (normalizedStatus.compare(QStringLiteral("Offline"), Qt::CaseInsensitive) == 0) {
        // A live cell is the strongest signal for video availability: if no frames
        // arrive, keep the device visually Offline long enough to bridge the
        // background TCP checker interval. A real frame clears this immediately.
        m_streamOfflineUntilMs.insert(ip, QDateTime::currentMSecsSinceEpoch() + 15000);
    } else if (normalizedStatus.compare(QStringLiteral("Online"), Qt::CaseInsensitive) == 0) {
        m_streamOfflineUntilMs.remove(ip);
    }

    updateCameraStatus(ip, normalizedStatus);
}

void SystemController::updateCameraStatusDetail(const QString &cameraIp, const QString &detail)
{
    const QString ip = cameraIp.trimmed();
    if (ip.isEmpty()) {
        return;
    }

    const QString normalizedDetail = detail.simplified().left(180);
    const QString previousDetail = m_cameraStatusDetails.value(ip);
    const qint64 streamOfflineUntil = m_streamOfflineUntilMs.value(ip, 0);
    const bool streamOfflineIsAuthoritative = streamOfflineUntil > QDateTime::currentMSecsSinceEpoch();
    if (normalizedDetail.isEmpty()) {
        if (streamOfflineIsAuthoritative && !previousDetail.isEmpty()) {
            return;
        }
        if (!m_cameraStatusDetails.contains(ip)) {
            return;
        }
        m_cameraStatusDetails.remove(ip);
        emit cameraStatusDetailsChanged();
        return;
    }

    if (previousDetail == normalizedDetail) {
        return;
    }

    if (streamOfflineIsAuthoritative && !previousDetail.isEmpty()) {
        return;
    }

    m_cameraStatusDetails.insert(ip, normalizedDetail);
    emit cameraStatusDetailsChanged();
}

QString SystemController::cameraStatusDetail(const QString &cameraIp) const
{
    return m_cameraStatusDetails.value(cameraIp.trimmed());
}

QString SystemController::effectiveCameraStatus(const QString &cameraIp, const QString &fallbackStatus) const
{
    const QString ip = cameraIp.trimmed();
    QStringList gridStatuses;
    QString modelStatus;
    bool streamOfflineIsAuthoritative = false;

    if (!ip.isEmpty()) {
        for (int i = 0; i < m_gridModel->rowCount(); ++i) {
            const Camera slot = m_gridModel->getCamera(i);
            if (slot.ip == ip) {
                gridStatuses.append(slot.status);
            }
        }

        const int listIndex = m_cameraModel->findIndexByIp(ip);
        if (listIndex >= 0) {
            modelStatus = m_cameraModel->getCamera(listIndex).status;
        }

        streamOfflineIsAuthoritative = m_streamOfflineUntilMs.value(ip, 0) > QDateTime::currentMSecsSinceEpoch();
    }

    return CameraStatusPolicy::effectiveStatus(gridStatuses,
                                               modelStatus,
                                               fallbackStatus,
                                               streamOfflineIsAuthoritative);
}

bool SystemController::isCameraOnline(const QString &cameraIp, const QString &fallbackStatus) const
{
    return CameraStatusPolicy::isOnline(effectiveCameraStatus(cameraIp, fallbackStatus));
}

QString SystemController::cameraAttentionReason(const QString &cameraIp, const QString &fallbackStatus) const
{
    return CameraStatusPolicy::attentionReason(effectiveCameraStatus(cameraIp, fallbackStatus),
                                               cameraStatusDetail(cameraIp));
}

QString SystemController::cameraStatusSearchText(const QString &cameraIp, const QString &fallbackStatus) const
{
    return CameraStatusPolicy::searchText(effectiveCameraStatus(cameraIp, fallbackStatus),
                                          cameraStatusDetail(cameraIp));
}

bool SystemController::cameraNeedsAttention(const QString &cameraIp, const QString &fallbackStatus) const
{
    return CameraStatusPolicy::needsAttention(effectiveCameraStatus(cameraIp, fallbackStatus),
                                              cameraStatusDetail(cameraIp));
}

bool SystemController::isCameraInGrid(const QString &cameraIp) const
{
    const QString ip = cameraIp.trimmed();
    if (ip.isEmpty()) {
        return false;
    }

    for (int i = 0; i < m_gridModel->rowCount(); ++i) {
        const Camera slot = m_gridModel->getCamera(i);
        if (slot.ip == ip) {
            return true;
        }
    }

    return false;
}

int SystemController::onlineCameraCount() const
{
    int count = 0;
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        const Camera cam = m_cameraModel->getCamera(i);
        if (isCameraOnline(cam.ip, cam.status)) {
            ++count;
        }
    }
    return count;
}

int SystemController::camerasNeedingAttentionCount() const
{
    int count = 0;
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        const Camera cam = m_cameraModel->getCamera(i);
        if (cameraNeedsAttention(cam.ip, cam.status)) {
            ++count;
        }
    }
    return count;
}

void SystemController::refreshCameraHealth(const QString &cameraIp)
{
    const QString ip = cameraIp.trimmed();
    if (ip.isEmpty() || !m_statusChecker) {
        return;
    }

    updateCameraStatusDetail(ip, QStringLiteral("Проверка RTSP…"));
    m_statusChecker->checkOne(ip);
}

void SystemController::removeDevice(int index)
{
    // Also remove from grid if present
    Camera cam = m_cameraModel->getCamera(index);
    if (!cam.ip.isEmpty()) {
        updateCameraStatusDetail(cam.ip, QString());
        m_streamOfflineUntilMs.remove(cam.ip);

        // Delete password from keychain
        auto job = new QKeychain::DeletePasswordJob("OpenIPC");
        job->setAutoDelete(true);
        job->setKey(cam.ip);
        job->start();
        
        // Find in grid model and remove
        // This is a bit inefficient but works for small lists
        for (int i = 0; i < m_gridModel->rowCount(); ++i) {
            if (m_gridModel->getCamera(i).ip == cam.ip) {
                // Don't remove the row, just clear the camera data to preserve grid layout
                removeCameraFromGrid(i);
                break;
            }
        }
    }
    m_cameraModel->removeCamera(index);
    markDiscoveryAddedFlags();
    saveState();
}

void SystemController::setGridCellSpan(int index, int rows, int cols)
{
    if (m_gridModel) {
        m_gridModel->setSpan(index, rows, cols);
        saveState();
    }
}

void SystemController::addCameraToGrid(int index, int slot)
{
    Camera cam = m_cameraModel->getCamera(index);
    if (!cam.id.isEmpty()) {
        bool exists = m_gridModel->contains(cam.ip);
        
        // Helper to apply existing span to new camera object
        auto preserveSpan = [&](int targetSlot, Camera &c) {
            Camera existing = m_gridModel->getCamera(targetSlot);
            c.spanRows = existing.spanRows;
            c.spanCols = existing.spanCols;
        };

        if (slot >= 0 && slot < m_gridModel->rowCount()) {
            preserveSpan(slot, cam);
            m_gridModel->setCamera(slot, cam);
            saveState();
            return;
        }
        if (!exists) {
            // Respect fixed grid size (4 slots preallocated); find first empty slot
            for (int i = 0; i < m_gridModel->rowCount(); ++i) {
                Camera slotCam = m_gridModel->getCamera(i);
                if (slotCam.ip.isEmpty()) {
                    preserveSpan(i, cam);
                    m_gridModel->setCamera(i, cam);
                    saveState();
                    return;
                }
            }
            // A full page must never destroy an existing assignment. Grow by one
            // page; DashboardView will navigate to the newly allocated page.
            const int previousCapacity = m_gridModel->rowCount();
            const int fallbackPageSize = std::max(1, m_gridRows * m_gridCols);
            if (ensureGridPageCapacity(fallbackPageSize, true) > previousCapacity) {
                preserveSpan(previousCapacity, cam);
                m_gridModel->setCamera(previousCapacity, cam);
                saveState();
            } else {
                qWarning() << "Grid capacity limit reached; camera was not assigned:" << cam.ip;
            }
        }
    }
}

void SystemController::removeCameraFromGrid(int index)
{
    // Keep placeholder instead of shrinking model to preserve grid geometry
    if (index >= 0 && index < m_gridModel->rowCount()) {
        Camera existing = m_gridModel->getCamera(index);
        Camera empty;
        // Preserve the span of the cell
        empty.spanRows = existing.spanRows;
        empty.spanCols = existing.spanCols;
        
        // Fallback if existing span is invalid/too small for current grid
        // This prevents cells from collapsing to 1px if they somehow lost their span info
        if (empty.spanRows <= 1 && m_gridRows > 0) empty.spanRows = std::max(1, 1200 / m_gridRows);
        if (empty.spanCols <= 1 && m_gridCols > 0) empty.spanCols = std::max(1, 1200 / m_gridCols);

        m_gridModel->setCamera(index, empty);
        saveState();
    }
}

void SystemController::updateGridSize(int size)
{
    if (size < 0) {
        return;
    }
    // Safety limit to prevent OOM/Crash
    if (size > 256) {
        qWarning() << "Requested grid size too large:" << size << ". Clamping to 256.";
        size = 256;
    }

    const int current = m_gridModel->rowCount();
    if (current == size) {
        return;
    }
    
    if (current < size) {
        for (int i = current; i < size; ++i) {
            m_gridModel->addCamera(Camera());
        }
    } else {
        // Shrink: remove from the end
        while (m_gridModel->rowCount() > size) {
            m_gridModel->removeCamera(m_gridModel->rowCount() - 1);
        }
    }
    saveState();
}

int SystemController::gridCapacity() const
{
    return m_gridModel->rowCount();
}

int SystemController::ensureGridPageCapacity(int pageSize, bool appendPage)
{
    pageSize = std::clamp(pageSize, 1, 256);
    const int current = m_gridModel->rowCount();
    const int currentPages = std::max(1, (current + pageSize - 1) / pageSize);
    const int requestedPages = currentPages + (appendPage ? 1 : 0);
    const int maxPages = std::max(1, 256 / pageSize);
    const int target = pageSize * std::min(requestedPages, maxPages);

    QVector<QPair<int, int>> spanPattern;
    spanPattern.reserve(pageSize);
    const int defaultRows = std::max(1, 1200 / std::max(1, m_gridRows));
    const int defaultCols = std::max(1, 1200 / std::max(1, m_gridCols));
    for (int index = 0; index < pageSize; ++index) {
        const Camera existing = m_gridModel->getCamera(index);
        spanPattern.append({existing.spanRows > 1 ? existing.spanRows : defaultRows,
                            existing.spanCols > 1 ? existing.spanCols : defaultCols});
    }

    if (target > current) {
        updateGridSize(target);
        for (int index = current; index < target; ++index) {
            const auto span = spanPattern.at(index % pageSize);
            m_gridModel->setSpan(index, span.first, span.second);
        }
        saveState();
    }
    return m_gridModel->rowCount();
}

int SystemController::compactGridPages(int pageSize)
{
    pageSize = std::clamp(pageSize, 1, 256);
    int lastAssigned = -1;
    for (int index = m_gridModel->rowCount() - 1; index >= 0; --index) {
        const Camera camera = m_gridModel->getCamera(index);
        if (!camera.id.isEmpty() || !camera.ip.isEmpty() || !camera.name.isEmpty()
            || !camera.streamUrl.isEmpty() || !camera.sdStreamUrl.isEmpty()
            || !camera.hdStreamUrl.isEmpty()) {
            lastAssigned = index;
            break;
        }
    }
    const int pages = std::max(1, (lastAssigned + 1 + pageSize - 1) / pageSize);
    const int target = std::min(256, pages * pageSize);
    if (m_gridModel->rowCount() > target) {
        updateGridSize(target);
    }
    return m_gridModel->rowCount();
}

bool SystemController::startPushToTalk(int gridIndex)
{
    stopPushToTalk();
    m_pushToTalkError.clear();

    const Camera camera = m_gridModel->getCamera(gridIndex);
    const int cameraIndex = m_cameraModel->findIndexByIp(camera.ip);
    if (!m_userManager->canTalk()) {
        m_pushToTalkError = tr("Push-to-talk permission is required");
    } else if (gridIndex < 0 || camera.ip.isEmpty() || cameraIndex < 0) {
        m_pushToTalkError = tr("Camera is not available for push-to-talk");
    } else if (!m_userManager->canAccessCamera(camera.id, camera.ip, cameraIndex)) {
        m_pushToTalkError = tr("Camera access is not allowed");
    }

    const QAudioDevice input = QMediaDevices::defaultAudioInput();
    if (m_pushToTalkError.isEmpty() && input.isNull()) {
        m_pushToTalkError = tr("No microphone is available");
    }
    if (!m_pushToTalkError.isEmpty()) {
        emit pushToTalkStateChanged();
        return false;
    }

    m_pushToTalkFormat = input.preferredFormat();
    const QAudioFormat::SampleFormat sampleFormat = m_pushToTalkFormat.sampleFormat();
    const bool supportedSampleFormat = sampleFormat == QAudioFormat::UInt8
        || sampleFormat == QAudioFormat::Int16
        || sampleFormat == QAudioFormat::Int32
        || sampleFormat == QAudioFormat::Float;
    if (!supportedSampleFormat || m_pushToTalkFormat.sampleRate() < 8000
        || m_pushToTalkFormat.channelCount() < 1) {
        m_pushToTalkError = tr("The microphone format is not supported");
        emit pushToTalkStateChanged();
        return false;
    }

    m_pushToTalkInputBuffer.clear();
    m_pushToTalkOutputBuffer.clear();
    m_pushToTalkResampleAccumulator = 0;
    m_pushToTalkGridIndex = gridIndex;
    m_pushToTalkSource = new QAudioSource(input, m_pushToTalkFormat, this);
    m_pushToTalkSource->setBufferSize(std::max(
        16384, static_cast<int>(m_pushToTalkFormat.bytesForDuration(500000))));
    connect(m_pushToTalkSource, &QAudioSource::stateChanged, this,
            [this](QAudio::State state) {
        if (state != QAudio::StoppedState || !m_pushToTalkActive
            || !m_pushToTalkSource || m_pushToTalkSource->error() == QAudio::NoError) {
            return;
        }
        m_pushToTalkError = tr("Microphone capture stopped unexpectedly");
        QTimer::singleShot(0, this, &SystemController::stopPushToTalk);
        emit pushToTalkStateChanged();
    });
    m_pushToTalkDevice = m_pushToTalkSource->start();
    if (!m_pushToTalkDevice || m_pushToTalkSource->error() != QAudio::NoError) {
        m_pushToTalkError = tr("Microphone capture could not be started");
        m_pushToTalkSource->deleteLater();
        m_pushToTalkSource = nullptr;
        m_pushToTalkGridIndex = -1;
        emit pushToTalkStateChanged();
        return false;
    }

    m_pushToTalkActive = true;
    connect(m_pushToTalkDevice, &QIODevice::readyRead,
            this, &SystemController::readPushToTalkAudio);
    emit pushToTalkStateChanged();
    return true;
}

void SystemController::stopPushToTalk()
{
    if (m_pushToTalkDevice && m_pushToTalkActive) {
        readPushToTalkAudio();
    }
    if (m_pushToTalkSource) {
        m_pushToTalkSource->stop();
    }
    sendPushToTalkChunk(true);
    if (m_pushToTalkSource) {
        m_pushToTalkSource->deleteLater();
    }
    m_pushToTalkSource = nullptr;
    m_pushToTalkDevice = nullptr;
    m_pushToTalkInputBuffer.clear();
    m_pushToTalkOutputBuffer.clear();
    m_pushToTalkResampleAccumulator = 0;
    const bool changed = m_pushToTalkActive || m_pushToTalkGridIndex >= 0;
    m_pushToTalkActive = false;
    m_pushToTalkGridIndex = -1;
    if (changed) emit pushToTalkStateChanged();
}

void SystemController::readPushToTalkAudio()
{
    if (!m_pushToTalkActive || !m_pushToTalkDevice) return;
    const QByteArray captured = m_pushToTalkDevice->readAll();
    if (captured.isEmpty()) return;
    m_pushToTalkOutputBuffer.append(convertPushToTalkAudio(captured));
    sendPushToTalkChunk(false);
}

QByteArray SystemController::convertPushToTalkAudio(const QByteArray &data)
{
    m_pushToTalkInputBuffer.append(data);
    const int channels = m_pushToTalkFormat.channelCount();
    const int bytesPerSample = m_pushToTalkFormat.bytesPerSample();
    const int frameBytes = channels * bytesPerSample;
    const int inputRate = m_pushToTalkFormat.sampleRate();
    if (frameBytes <= 0 || inputRate <= 0) return {};

    const int frameCount = m_pushToTalkInputBuffer.size() / frameBytes;
    QByteArray result;
    result.reserve((frameCount * 8000 / inputRate + 2) * 2);
    const char *bytes = m_pushToTalkInputBuffer.constData();
    for (int frame = 0; frame < frameCount; ++frame) {
        double mono = 0.0;
        for (int channel = 0; channel < channels; ++channel) {
            const char *sample = bytes + frame * frameBytes + channel * bytesPerSample;
            switch (m_pushToTalkFormat.sampleFormat()) {
            case QAudioFormat::UInt8:
                mono += (static_cast<unsigned char>(*sample) - 128.0) / 128.0;
                break;
            case QAudioFormat::Int16: {
                qint16 value = 0;
                std::memcpy(&value, sample, sizeof(value));
                mono += static_cast<double>(value) / 32768.0;
                break;
            }
            case QAudioFormat::Int32: {
                qint32 value = 0;
                std::memcpy(&value, sample, sizeof(value));
                mono += static_cast<double>(value) / 2147483648.0;
                break;
            }
            case QAudioFormat::Float: {
                float value = 0.0f;
                std::memcpy(&value, sample, sizeof(value));
                mono += value;
                break;
            }
            default:
                break;
            }
        }
        mono = std::clamp(mono / channels, -1.0, 1.0);
        m_pushToTalkResampleAccumulator += 8000;
        if (m_pushToTalkResampleAccumulator >= inputRate) {
            m_pushToTalkResampleAccumulator -= inputRate;
            const qint16 pcm = static_cast<qint16>(std::lround(
                mono < 0.0 ? mono * 32768.0 : mono * 32767.0));
            const qint16 littleEndian = qToLittleEndian(pcm);
            result.append(reinterpret_cast<const char *>(&littleEndian), sizeof(littleEndian));
        }
    }
    m_pushToTalkInputBuffer.remove(0, frameCount * frameBytes);
    return result;
}

void SystemController::sendPushToTalkChunk(bool flush)
{
    constexpr int chunkBytes = 8000;
    while (m_pushToTalkOutputBuffer.size() >= chunkBytes
           || (flush && !m_pushToTalkOutputBuffer.isEmpty())) {
        int size = std::min(chunkBytes, static_cast<int>(m_pushToTalkOutputBuffer.size()));
        if ((size % 2) != 0) --size;
        if (size <= 0) break;
        const QByteArray pcm = m_pushToTalkOutputBuffer.left(size);
        m_pushToTalkOutputBuffer.remove(0, size);
        const Camera camera = m_gridModel->getCamera(m_pushToTalkGridIndex);
        if (!camera.ip.isEmpty()) {
            m_majesticClient->playPcmData(camera.ip, camera.onvifPort,
                                          camera.login, camera.password, pcm);
        }
        if (!flush) break;
    }
}

#ifdef Q_OS_WIN
static quint64 fileTimeToUInt64(const FILETIME &ft)
{
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return li.QuadPart;
}
#endif

double SystemController::processCpuPercent()
{
#ifdef Q_OS_WIN
    FILETIME idleTime, kernelTime, userTime;
    FILETIME createTime, exitTime, procKernel, procUser;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 0.0;
    }
    if (!GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &procKernel, &procUser)) {
        return 0.0;
    }

    quint64 sysKernel = fileTimeToUInt64(kernelTime);
    quint64 sysUser = fileTimeToUInt64(userTime);
    quint64 pKernel = fileTimeToUInt64(procKernel);
    quint64 pUser = fileTimeToUInt64(procUser);

    if (!m_cpuInit) {
        m_prevSysKernel = sysKernel;
        m_prevSysUser = sysUser;
        m_prevProcKernel = pKernel;
        m_prevProcUser = pUser;
        m_cpuInit = true;
        m_cpuTimer.start();
        return 0.0;
    }

    quint64 sysDelta = (sysKernel - m_prevSysKernel) + (sysUser - m_prevSysUser);
    quint64 procDelta = (pKernel - m_prevProcKernel) + (pUser - m_prevProcUser);

    m_prevSysKernel = sysKernel;
    m_prevSysUser = sysUser;
    m_prevProcKernel = pKernel;
    m_prevProcUser = pUser;

    if (sysDelta == 0) {
        return 0.0;
    }

    double cpu = (static_cast<double>(procDelta) / static_cast<double>(sysDelta)) * 100.0;
    // Clamp to reasonable range
    if (cpu < 0.0) cpu = 0.0;
    if (cpu > 100.0) cpu = 100.0;
    return cpu;
#elif defined(Q_OS_LINUX)
    quint64 systemTotal = 0;
    quint64 processTotal = 0;
    if (!readLinuxSystemCpuTotal(systemTotal) || !readLinuxProcessCpuTotal(processTotal)) {
        return 0.0;
    }

    if (!m_cpuInit) {
        m_prevLinuxSystemCpu = systemTotal;
        m_prevLinuxProcessCpu = processTotal;
        m_cpuInit = true;
        m_cpuTimer.start();
        return 0.0;
    }

    const quint64 systemDelta = systemTotal - m_prevLinuxSystemCpu;
    const quint64 processDelta = processTotal - m_prevLinuxProcessCpu;

    m_prevLinuxSystemCpu = systemTotal;
    m_prevLinuxProcessCpu = processTotal;

    if (systemDelta == 0) {
        return 0.0;
    }

    double cpu = (static_cast<double>(processDelta) / static_cast<double>(systemDelta)) * 100.0;
    if (cpu < 0.0) cpu = 0.0;
    if (cpu > 100.0) cpu = 100.0;
    return cpu;
#else
    return 0.0;
#endif
}

double SystemController::processMemoryMB()
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }
    return 0.0;
#elif defined(Q_OS_LINUX)
    // Prefer the current resident set size from /proc/self/status. Some sandboxed
    // Linux environments may expose a partial /proc, so keep fallbacks below.
    QFile file("/proc/self/status");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.startsWith(QStringLiteral("VmRSS:"))) {
                const QStringList parts = line.split(QRegularExpression(QStringLiteral("\\s+")),
                                                     Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    // Value is in kB
                    bool ok = false;
                    const double valueKb = parts.at(1).toDouble(&ok);
                    if (ok && valueKb > 0.0) {
                        return valueKb / 1024.0;
                    }
                }
            }
        }
    }

    QFile statmFile(QStringLiteral("/proc/self/statm"));
    if (statmFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QStringList parts = QString::fromUtf8(statmFile.readAll())
                                      .split(QRegularExpression(QStringLiteral("\\s+")),
                                             Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            bool ok = false;
            const double residentPages = parts.at(1).toDouble(&ok);
            const long pageSize = sysconf(_SC_PAGESIZE);
            if (ok && residentPages > 0.0 && pageSize > 0) {
                return (residentPages * static_cast<double>(pageSize)) / (1024.0 * 1024.0);
            }
        }
    }

    struct rusage usage {};
    if (getrusage(RUSAGE_SELF, &usage) == 0 && usage.ru_maxrss > 0) {
        // Linux reports ru_maxrss in kilobytes.
        return static_cast<double>(usage.ru_maxrss) / 1024.0;
    }

    return 0.0;
#else
    return 0.0;
#endif
}

void SystemController::setGridRows(int rows)
{
    if (m_gridRows != rows) {
        m_gridRows = rows;
        m_appSettings["gridRows"] = rows;
        saveState();
        emit gridLayoutChanged();
    }
}

void SystemController::setGridCols(int cols)
{
    if (m_gridCols != cols) {
        m_gridCols = cols;
        m_appSettings["gridCols"] = cols;
        saveState();
        emit gridLayoutChanged();
    }
}

void SystemController::applyLayoutPreset(int rows, int cols)
{
    rows = std::max(1, rows);
    cols = std::max(1, cols);
    const int pageSize = rows * cols;
    // A layout describes one page. Existing camera assignments remain intact
    // and capacity is padded to a whole number of pages.
    ensureGridPageCapacity(pageSize);

    // Base grid is 1200x1200 for smooth resizing
    int spanRows = std::max(1, 1200 / rows);
    int spanCols = std::max(1, 1200 / cols);
    
    // Update all cameras in grid
    for (int i = 0; i < m_gridModel->rowCount(); ++i) {
        Camera cam = m_gridModel->getCamera(i);
        cam.spanRows = spanRows;
        cam.spanCols = spanCols;
        m_gridModel->setCamera(i, cam);
    }
    
    // Update stored grid dimensions for reference
    setGridRows(rows);
    setGridCols(cols);
}

void SystemController::applyLayoutTemplate(const QVariantMap &layout)
{
    int rows = layout.value("rows", 1).toInt();
    int cols = layout.value("cols", 1).toInt();

    // If no specific cells defined, use uniform preset
    if (!layout.contains("cells") || layout.value("cells").toList().isEmpty()) {
        applyLayoutPreset(rows, cols);
        return;
    }
    
    QVariantList cells = layout.value("cells").toList();
    
    // Base grid is 1200x1200
    // We scale the logical rows/cols to 1200
    double rowScale = 1200.0 / (double)rows;
    double colScale = 1200.0 / (double)cols;
    
    const int pageSize = std::max(1, static_cast<int>(cells.size()));
    ensureGridPageCapacity(pageSize);
    
    // Apply spans
    for (int i = 0; i < m_gridModel->rowCount(); ++i) {
        QVariantMap cellDef = cells[i % pageSize].toMap();
        int rSpan = cellDef.value("rowSpan", 1).toInt();
        int cSpan = cellDef.value("colSpan", 1).toInt();
        
        Camera cam = m_gridModel->getCamera(i);
        cam.spanRows = std::max(1, (int)(rSpan * rowScale));
        cam.spanCols = std::max(1, (int)(cSpan * colScale));
        m_gridModel->setCamera(i, cam);
    }
    
    setGridRows(rows);
    setGridCols(cols);
}

QString SystemController::getCameraPassword(const QString &cameraIp) const
{
    if (cameraIp.isEmpty()) {
        return QString();
    }

    Camera camera = m_cameraModel->findByIp(cameraIp);
    if (camera.password.isEmpty()) {
        camera = m_gridModel->findByIp(cameraIp);
    }
    if (!camera.password.isEmpty()) {
        return camera.password;
    }

    QKeychain::ReadPasswordJob job(QStringLiteral("OpenIPC"));
    job.setKey(cameraIp);

    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    return job.textData();
}

void SystemController::takeDahuaSnapshot(const QString &ip, int port, const QString &login, const QString &password)
{
    // Use HTTP API instead of SDK for better compatibility and resolution control
    // URL: http://<ip>/cgi-bin/snapshot.cgi?channel=1&subtype=0
    // subtype=0 means Main Stream (HD), subtype=1 means Sub Stream (SD)
    
    // Assume HTTP port is 80. If 'port' passed is 80 or 8080, use it, otherwise default to 80.
    // Dahua SDK port is usually 37777, so we ignore it if it looks like that.
    int httpPort = 80;
    if (port > 0 && port != 37777 && port != 37778) {
        httpPort = port;
    }

    QUrl url;
    url.setScheme("http");
    url.setHost(ip);
    url.setPort(httpPort);
    url.setPath("/cgi-bin/snapshot.cgi");
    QUrlQuery query;
    query.addQueryItem("channel", "1");
    query.addQueryItem("subtype", "0"); // Force Main Stream
    url.setQuery(query);

    qInfo() << "Requesting snapshot via HTTP:" << url.toString();

    // Store credentials for authentication
    m_pendingCredentials.insert(ip, qMakePair(login, password));

    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, ip]() {
        reply->deleteLater();
        m_pendingCredentials.remove(ip); // Cleanup credentials

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "HTTP Snapshot failed:" << reply->errorString();
            addLog(QtWarningMsg, "Snapshot failed for " + ip + ": " + reply->errorString());
            return;
        }

        QByteArray data = reply->readAll();
        if (data.isEmpty()) {
            qWarning() << "HTTP Snapshot returned empty data";
            return;
        }

        // Prepare filename
        QString savePath = normalizedLocalPath(m_appSettings.value("screenshotsPath").toString());
        if (savePath.isEmpty()) savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        QDir().mkpath(savePath);
        
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
        QString safeIp = ip;
        safeIp.replace(".", "_");
        QString filename = QString("%1/snapshot_%2_%3.jpg").arg(savePath).arg(safeIp).arg(timestamp);

        QImage img;
        if (img.loadFromData(data)) {
            qInfo() << "Snapshot downloaded. Resolution:" << img.width() << "x" << img.height();

            
            // Explicitly rotate 180 degrees using QTransform
            QTransform transform;
            transform.rotate(180);
            img = img.transformed(transform);
            
            if (img.save(filename, "JPG", 95)) {
                qInfo() << "Snapshot saved to" << filename;
                addLog(QtInfoMsg, "Snapshot saved: " + filename);
                emit snapshotSaved(filename);
            } else {
                qWarning() << "Failed to save snapshot to file:" << filename;
            }
        } else {
            qWarning() << "Failed to load image from data";
        }
    });
}

void SystemController::onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    QString host = reply->url().host();
    if (m_pendingCredentials.contains(host)) {
        QPair<QString, QString> creds = m_pendingCredentials.value(host);
        authenticator->setUser(creds.first);
        authenticator->setPassword(creds.second);
    }
}

void SystemController::notifySnapshotSaved(const QString &path)
{
    emit snapshotSaved(path);
}

QString SystemController::getSnapshotPath(const QString &filename)
{
    QString savePath = normalizedLocalPath(m_appSettings.value("screenshotsPath").toString());
    if (savePath.isEmpty()) savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDir().mkpath(savePath);
    return QDir(savePath).filePath(filename);
}

bool SystemController::deleteLocalFile(const QString &fileUrl)
{
    if (fileUrl.isEmpty()) return false;
    QString targetPath = normalizedLocalPath(fileUrl);
    QFileInfo fi(targetPath);
    if (!fi.exists() || !fi.isFile()) return false;
    return QFile::remove(targetPath);
}

QString SystemController::authenticatedStreamUrl(const QString &value, const QString &cameraIp) const
{
    QUrl url(value);
    if (!url.isValid() || url.host().isEmpty()) return value;

    Camera camera = m_cameraModel->findByIp(cameraIp);
    if (camera.ip.isEmpty()) camera = m_gridModel->findByIp(cameraIp);
    const QString password = getCameraPassword(cameraIp);
    if (!camera.login.isEmpty()) url.setUserName(camera.login);
    if (!password.isEmpty()) url.setPassword(password);
    return url.toString(QUrl::FullyEncoded);
}

QString SystemController::preferredPreviewStreamUrl(const QString &streamUrl,
                                                    const QString &sdStreamUrl,
                                                    const QString &hdStreamUrl,
                                                    const QString &preferredStream,
                                                    int gridRows,
                                                    int gridCols,
                                                    int spanRows,
                                                    int spanCols,
                                                    bool forceMain) const
{
    return StreamQualityPolicy::selectPreviewUrl(streamUrl, sdStreamUrl, hdStreamUrl,
                                                 preferredStream, gridRows, gridCols,
                                                 spanRows, spanCols, forceMain);
}

QString SystemController::preferredPreviewStreamQuality(const QString &preferredStream,
                                                        int gridRows,
                                                        int gridCols,
                                                        int spanRows,
                                                        int spanCols,
                                                        bool forceMain) const
{
    return StreamQualityPolicy::qualityLabel(StreamQualityPolicy::resolvePreviewQuality(
        preferredStream, gridRows, gridCols, spanRows, spanCols, forceMain));
}

QString SystemController::manualStreamUrl(const QString &streamUrl,
                                          const QString &sdStreamUrl,
                                          const QString &hdStreamUrl,
                                          bool preferMain) const
{
    return StreamQualityPolicy::selectManualUrl(streamUrl, sdStreamUrl, hdStreamUrl, preferMain);
}

bool SystemController::isStreamFrameStalled(bool running,
                                            bool hasFrame,
                                            double nowMs,
                                            double startedMs,
                                            double lastFrameMs,
                                            int startupGraceMs,
                                            int frameStallMs) const
{
    return StreamHealthPolicy::isFrameStalled(running,
                                              hasFrame,
                                              static_cast<qint64>(nowMs),
                                              static_cast<qint64>(startedMs),
                                              static_cast<qint64>(lastFrameMs),
                                              startupGraceMs,
                                              frameStallMs);
}

int SystemController::streamPreviewPriorityScore(int gridIndex,
                                                 int spanRows,
                                                 int spanCols,
                                                 bool selected,
                                                 bool recordingActive,
                                                 bool analyticsActive,
                                                 bool online) const
{
    return StreamSessionPolicy::previewPriorityScore(gridIndex,
                                                     spanRows,
                                                     spanCols,
                                                     selected,
                                                     recordingActive,
                                                     analyticsActive,
                                                     online);
}

bool SystemController::shouldRunPreviewStream(bool smartBudgetEnabled,
                                              int maxPreviewStreams,
                                              int previewBudgetRank,
                                              bool hasCamera,
                                              bool canLive,
                                              bool fullscreenActive,
                                              bool archiveOpen,
                                              bool recordingActive,
                                              bool analyticsActive) const
{
    return StreamSessionPolicy::shouldRunPreview(smartBudgetEnabled,
                                                 maxPreviewStreams,
                                                 previewBudgetRank,
                                                 hasCamera,
                                                 canLive,
                                                 fullscreenActive,
                                                 archiveOpen,
                                                 recordingActive,
                                                 analyticsActive);
}

QString SystemController::previewPauseReasonCode(bool smartBudgetEnabled,
                                                 int maxPreviewStreams,
                                                 int previewBudgetRank,
                                                 bool hasCamera,
                                                 bool canLive,
                                                 bool fullscreenActive,
                                                 bool archiveOpen,
                                                 bool recordingActive,
                                                 bool analyticsActive) const
{
    return StreamSessionPolicy::previewPauseReasonCode(smartBudgetEnabled,
                                                       maxPreviewStreams,
                                                       previewBudgetRank,
                                                       hasCamera,
                                                       canLive,
                                                       fullscreenActive,
                                                       archiveOpen,
                                                       recordingActive,
                                                       analyticsActive);
}

bool SystemController::localFileExists(const QString &fileUrl) const
{
    if (fileUrl.isEmpty()) return false;
    QString targetPath = normalizedLocalPath(fileUrl);
    QFileInfo fi(targetPath);
    return fi.exists() && fi.isFile();
}

QVariantMap SystemController::getFileInfo(const QString &fileUrl) const
{
    QVariantMap info;
    if (fileUrl.isEmpty()) return info;

    QString targetPath = normalizedLocalPath(fileUrl);

    QFileInfo fi(targetPath);
    info["exists"] = fi.exists();
    info["filePath"] = fi.absoluteFilePath();
    info["fileName"] = fi.fileName();
    info["size"] = fi.exists() ? fi.size() : 0;
    info["suffix"] = fi.suffix();
    if (fi.exists()) {
        info["created"] = fi.birthTime();
        info["modified"] = fi.lastModified();
        info["createdText"] = fi.birthTime().toString("yyyy-MM-dd HH:mm:ss");
        info["modifiedText"] = fi.lastModified().toString("yyyy-MM-dd HH:mm:ss");
    }

    QImageReader reader(targetPath);
    QSize size = reader.size();
    if (size.isValid()) {
        info["width"] = size.width();
        info["height"] = size.height();
    }

    return info;
}

QVariantMap SystemController::inspectFirmwareArchive(const QString &fileUrl,
                                                     const QString &expectedSha256) const
{
    QVariantMap result = getFileInfo(fileUrl);
    QStringList issues;
    QStringList sidecarCandidates;
    QStringList signatureCandidates;

    const QString targetPath = result.value(QStringLiteral("filePath")).toString();
    QFileInfo fi(targetPath);
    result.insert(QStringLiteral("sha256"), QString());
    result.insert(QStringLiteral("sidecarSha256"), QString());
    result.insert(QStringLiteral("sidecarPath"), QString());
    result.insert(QStringLiteral("signaturePath"), QString());
    result.insert(QStringLiteral("signatureSize"), 0);
    result.insert(QStringLiteral("checksumStatus"), QStringLiteral("warn"));

    if (!fi.exists() || !fi.isFile()) {
        result.insert(QStringLiteral("checksumStatus"), QStringLiteral("block"));
        issues << QStringLiteral("файл не найден");
        result.insert(QStringLiteral("issues"), issues);
        return result;
    }

    const QString path = fi.absoluteFilePath();
    sidecarCandidates << path + QStringLiteral(".sha256")
                      << path + QStringLiteral(".sha256sum")
                      << path + QStringLiteral(".sha256.txt")
                      << fi.absolutePath() + QDir::separator() + fi.completeBaseName() + QStringLiteral(".sha256")
                      << fi.absolutePath() + QDir::separator() + fi.completeBaseName() + QStringLiteral(".sha256sum");
    signatureCandidates << path + QStringLiteral(".sig")
                        << path + QStringLiteral(".asc")
                        << path + QStringLiteral(".minisig");

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        result.insert(QStringLiteral("checksumStatus"), QStringLiteral("block"));
        issues << file.errorString();
        result.insert(QStringLiteral("issues"), issues);
        return result;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer;
    buffer.resize(1024 * 1024);
    while (!file.atEnd()) {
        const qint64 bytes = file.read(buffer.data(), buffer.size());
        if (bytes < 0) {
            result.insert(QStringLiteral("checksumStatus"), QStringLiteral("block"));
            issues << file.errorString();
            result.insert(QStringLiteral("issues"), issues);
            return result;
        }
        hash.addData(QByteArrayView(buffer.constData(), bytes));
    }
    const QString sha256 = QString::fromLatin1(hash.result().toHex());
    result.insert(QStringLiteral("sha256"), sha256);
    result.insert(QStringLiteral("sha256Short"), sha256.left(12));

    QRegularExpression shaRegex(QStringLiteral("\\b([A-Fa-f0-9]{64})\\b"));
    QString sidecarSha;
    QString sidecarPath;
    for (const QString &candidate : std::as_const(sidecarCandidates)) {
        QFile sidecar(candidate);
        if (!sidecar.exists() || !sidecar.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        const QString text = QString::fromUtf8(sidecar.read(16 * 1024));
        const QRegularExpressionMatch match = shaRegex.match(text);
        if (match.hasMatch()) {
            sidecarSha = match.captured(1).toLower();
            sidecarPath = QFileInfo(candidate).absoluteFilePath();
            break;
        }
    }
    if (!sidecarSha.isEmpty()) {
        result.insert(QStringLiteral("sidecarSha256"), sidecarSha);
        result.insert(QStringLiteral("sidecarPath"), sidecarPath);
        result.insert(QStringLiteral("sidecarMatches"), sidecarSha == sha256);
        if (sidecarSha != sha256) issues << QStringLiteral("sidecar checksum не совпадает с SHA-256 файла");
    }

    for (const QString &candidate : std::as_const(signatureCandidates)) {
        QFileInfo sig(candidate);
        if (sig.exists() && sig.isFile()) {
            result.insert(QStringLiteral("signaturePath"), sig.absoluteFilePath());
            result.insert(QStringLiteral("signatureSize"), sig.size());
            break;
        }
    }

    QString expected = expectedSha256.trimmed().toLower();
    const QRegularExpressionMatch expectedMatch = shaRegex.match(expected);
    if (expectedMatch.hasMatch()) {
        expected = expectedMatch.captured(1).toLower();
    }
    if (!expected.isEmpty() && shaRegex.match(expected).hasMatch()) {
        result.insert(QStringLiteral("expectedSha256"), expected);
        result.insert(QStringLiteral("expectedMatches"), expected == sha256);
        if (expected != sha256) issues << QStringLiteral("ожидаемый checksum update page не совпадает с SHA-256 файла");
    } else if (!expectedSha256.trimmed().isEmpty()) {
        result.insert(QStringLiteral("expectedSha256"), expectedSha256.trimmed());
        result.insert(QStringLiteral("expectedMatches"), QVariant());
    }

    QString status = QStringLiteral("warn");
    if (!issues.isEmpty()) {
        status = QStringLiteral("block");
    } else if (!expected.isEmpty() || !sidecarSha.isEmpty()) {
        status = QStringLiteral("ok");
    }
    result.insert(QStringLiteral("checksumStatus"), status);
    result.insert(QStringLiteral("issues"), issues);
    return result;
}

bool SystemController::copyImageToClipboard(const QString &fileUrl)
{
    if (fileUrl.isEmpty()) return false;
    QString targetPath = normalizedLocalPath(fileUrl);

    QImage img(targetPath);
    if (img.isNull()) return false;
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) return false;
    clipboard->setImage(img);
    return true;
}

void SystemController::copyTextToClipboard(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) return;
    clipboard->setText(text);
}

bool SystemController::saveTextFile(const QString &pathOrUrl, const QString &content) const
{
    const QString path = PathUtils::localPathFromUserInput(pathOrUrl);
    if (path.isEmpty()) {
        return false;
    }

    QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    if (file.write(content.toUtf8()) < 0) {
        return false;
    }

    return file.commit();
}

bool SystemController::openWithDialog(const QString &fileUrl)
{
    if (fileUrl.isEmpty()) return false;
    QString targetPath = normalizedLocalPath(fileUrl);

#ifdef Q_OS_WIN
    QString nativePath = QDir::toNativeSeparators(targetPath);
    return QProcess::startDetached("rundll32.exe", QStringList() << "shell32.dll,OpenAs_RunDLL" << nativePath);
#else
    return QDesktopServices::openUrl(QUrl::fromLocalFile(targetPath));
#endif
}

bool SystemController::printImage(const QString &fileUrl)
{
    if (fileUrl.isEmpty()) return false;
    QString targetPath = normalizedLocalPath(fileUrl);

#ifdef Q_OS_WIN
    const wchar_t *operation = L"print";
    HINSTANCE result = ShellExecuteW(nullptr, operation, reinterpret_cast<LPCWSTR>(targetPath.utf16()), nullptr, nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<intptr_t>(result) > 32;
#else
    return QDesktopServices::openUrl(QUrl::fromLocalFile(targetPath));
#endif
}

void SystemController::setLayoutTemplates(const QVariantList &templates)
{
    if (m_layoutTemplates != templates) {
        m_layoutTemplates = templates;
        saveState();
        emit layoutTemplatesChanged();
    }
}

bool SystemController::exportConfiguration(const QString &path)
{
    if (path.isEmpty()) return false;
    const QString targetPath = normalizedLocalPath(path);

    QJsonObject root;
    QJsonArray cameras;
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        cameras.append(cameraToJson(m_cameraModel->getCamera(i)));
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
    root["grid"] = grid;
    root["analytics"] = QJsonObject::fromVariantMap(m_analyticsEngine->getPersistedSettings());
    root["appSettings"] = QJsonObject::fromVariantMap(m_appSettings);
    root["cameraGroups"] = groups;
    root["layoutTemplates"] = QJsonArray::fromVariantList(m_layoutTemplates);
    
    QFile f(targetPath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        f.flush();
        f.close();
        qInfo() << "Configuration exported to" << targetPath;
        return true;
    }
    
    qWarning() << "Failed to export configuration to" << targetPath;
    return false;
}

bool SystemController::importConfiguration(const QString &path)
{
    if (path.isEmpty()) return false;
    const QString targetPath = normalizedLocalPath(path);
    
    QFile f(targetPath);
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file for import:" << targetPath;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &error);
    f.close();
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid JSON in config import:" << error.errorString();
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // 1. App Settings
    if (root.contains("appSettings")) {
        QVariantMap savedSettings = root.value("appSettings").toObject().toVariantMap();
        normalizeImportedAppSettings(savedSettings);
        for (auto it = savedSettings.begin(); it != savedSettings.end(); ++it) {
            m_appSettings[it.key()] = it.value();
        }

        m_gridRows = m_appSettings.value("gridRows", 2).toInt();
        m_gridCols = m_appSettings.value("gridCols", 2).toInt();
        emit appSettingsChanged();
    }

    // 2. Analytics
    if (root.contains("analytics")) {
        m_analyticsEngine->setSettings(root.value("analytics").toObject().toVariantMap());
    }

    // 3. Layout Templates
    if (root.contains("layoutTemplates")) {
        m_layoutTemplates = root.value("layoutTemplates").toArray().toVariantList();
        emit layoutTemplatesChanged();
    }
    
    // 4. Camera Groups
    // Clear and rebuild
    m_cameraGroups.clear();
    if (root.contains("cameraGroups")) {
        const QJsonArray groups = root.value("cameraGroups").toArray();
        for (const auto &g : groups) {
             const QString name = g.toString();
             if (!name.isEmpty()) m_cameraGroups.append(name);
        }
    }
    emit cameraGroupsChanged();

    // 5. Cameras
    m_cameraModel->clear();
    const QJsonArray cameras = root.value("cameras").toArray();
    for (const auto &v : cameras) {
        Camera cam = cameraFromJson(v.toObject());
        m_cameraModel->addCamera(cam);
        
        // Ensure group exists in list if referenced
        if (!cam.group.isEmpty() && !m_cameraGroups.contains(cam.group, Qt::CaseInsensitive)) {
            m_cameraGroups.append(cam.group);
            emit cameraGroupsChanged();
        }
    }
    
    // 6. Grid
    m_gridModel->clear();
    QJsonArray grid = root.value("grid").toArray();
    
    if (grid.isEmpty()) {
        // Fallback or empty
    } else {
        for (int i = 0; i < grid.size(); ++i) {
            Camera cam;
            QJsonObject slotObj = grid.at(i).toObject();
            const QString ip = slotObj.value("ip").toString();
            
            // Try to link with known camera details
            if (!ip.isEmpty()) {
                cam = m_cameraModel->findByIp(ip);
                if (cam.ip.isEmpty()) {
                    // Not found in cameras list, use embedded data
                    cam = cameraFromJson(slotObj.value("camera").toObject());
                }
            }
            
            cam.spanRows = slotObj.value("spanRows").toInt(1);
            cam.spanCols = slotObj.value("spanCols").toInt(1);
            
            m_gridModel->addCamera(cam);
        }
    }
    
    emit gridLayoutChanged();
    
    // Persist immediately
    saveState();
    
    qInfo() << "Configuration imported successfully from" << targetPath;
    return true;
}

