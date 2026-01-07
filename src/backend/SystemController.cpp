#include "SystemController.h"
#include <QUuid>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QTimer>
#include <algorithm>
#include <QDate>
#include <QDirIterator>
#include <QCoreApplication>
#include <QImage>
#include <QThread>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QUrlQuery>
#include <QDesktopServices>
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#undef min
#undef max
#endif

SystemController::SystemController(QObject *parent)
    : QObject(parent)
    , m_serviceStatus("Stopped")
    , m_process(new QProcess(this))
    , m_cameraModel(new CameraModel(this))
    , m_discoveryModel(new CameraModel(this))
    , m_gridModel(new CameraModel(this))
    , m_analyticsEngine(new AnalyticsEngine(this))
    , m_userManager(new UserManager(this))
    , m_logModel(new LogModel(this))
    , m_ptzController(new PtzController(this))
    , m_dahuaDiscovery(new DiscoveryController(this))
    , m_archiveController(new ArchiveController(this))
    , m_udpSocket(new QUdpSocket(this))
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::authenticationRequired, this, &SystemController::onAuthenticationRequired);
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &SystemController::onUdpReadyRead);
    
    // Connect Dahua discovery to main discovery model
    connect(m_dahuaDiscovery, &DiscoveryController::deviceFound, this, [this](const DiscoveredDevice& dev){
        // Check for duplicates in m_discoveryModel
        for (int i = 0; i < m_discoveryModel->rowCount(); ++i) {
            QModelIndex idx = m_discoveryModel->index(i, 0);
            QString ip = m_discoveryModel->data(idx, CameraModel::IpRole).toString();
            if (ip == dev.ip) return;
        }

        Camera cam;
        cam.ip = dev.ip;
        cam.port = 554; // RTSP port assumption
        cam.onvifPort = 80;
        cam.name = dev.type.isEmpty() ? "Dahua Camera" : dev.type;
        cam.login = "admin";
        cam.password = "admin";
        cam.status = "Discovered";
        cam.serialNumber = dev.serial;
        cam.manufacturer = dev.manufacturer;
        
        m_discoveryModel->addCamera(cam);
    });

    m_analyticsEngine->initialize();
    connect(m_analyticsEngine, &AnalyticsEngine::settingsChanged, this, &SystemController::saveState);
    
    // Bind to the multicast port or any port to receive unicast responses
    m_udpSocket->bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress);

    // Default settings
    m_appSettings["language"] = "ru";
    m_appSettings["recordingsPath"] = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    m_appSettings["screenshotsPath"] = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    m_appSettings["hwAccel"] = "auto";
    m_appSettings["notificationsEnabled"] = true;
    m_appSettings["preferredStream"] = "auto";
    // 0 = fit (letterbox), -1 = crop; default to fit for correct aspect
    m_appSettings["playerFillMode"] = 0.0;
    m_appSettings["showStatsOverlay"] = true;
    m_appSettings["defaultAutoplay"] = true;
    // Disable analytics by default to avoid crashes from heavy ONNX inference on low GPUs
    m_appSettings["analyticsEnabled"] = false;
    // Sane grid defaults; avoid spawning hundreds of cells when state.json is absent
    m_appSettings["gridRows"] = 2;
    m_appSettings["gridCols"] = 2;
    
    // Player settings
    m_appSettings["playerBufferMode"] = 1; // Balanced
    m_appSettings["playerRtspTransport"] = "tcp";

    m_gridRows = 2;
    m_gridCols = 2; 

    loadState();
    // If no saved state, ensure default 2x2 grid placeholders
    if (m_gridModel->rowCount() == 0) {
        for (int i = 0; i < 4; ++i) {
            m_gridModel->addCamera(Camera());
        }
    }
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
            for (const QNetworkAddressEntry &entry : entries) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ip = entry.ip().toString();
                    break;
                }
            }
            
            if (!ip.isEmpty()) {
                QVariantMap map;
                map["name"] = iface.humanReadableName(); // Display name
                map["id"] = iface.name(); // Internal name (e.g., eth0, {UUID})
                map["ip"] = ip;
                list.append(map);
            }
        }
    }
    return list;
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

void SystemController::addLog(QtMsgType type, const QString &msg)
{
    if (m_logModel) {
        // Ensure we are on the main thread for UI updates
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

void SystemController::scanNetwork(const QString &interfaceName)
{
    qDebug() << "Starting network scan. Interface:" << (interfaceName.isEmpty() ? "All" : interfaceName);
    m_discoveryModel->clear();
    
    // Start Dahua SDK Search
    if (m_dahuaDiscovery) {
        m_dahuaDiscovery->startSearch();
    }

    if (interfaceName.isEmpty()) {
        // Scan all capable interfaces
        const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &iface : interfaces) {
            if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
                !iface.flags().testFlag(QNetworkInterface::IsLoopBack) &&
                iface.flags().testFlag(QNetworkInterface::CanMulticast)) {
                
                // Check if it has an IPv4 address
                bool hasIPv4 = false;
                for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                        hasIPv4 = true;
                        break;
                    }
                }

                if (hasIPv4) {
                    qDebug() << "Sending probe on interface:" << iface.humanReadableName();
                    m_udpSocket->setMulticastInterface(iface);
                    sendDiscoveryProbe();
                }
            }
        }
    } else {
        QNetworkInterface iface = QNetworkInterface::interfaceFromName(interfaceName);
        if (!iface.isValid()) {
             // Try finding by human readable name if internal name fails
             const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
             for (const QNetworkInterface &i : interfaces) {
                 if (i.humanReadableName() == interfaceName) {
                     iface = i;
                     break;
                 }
             }
        }
        
        if (iface.isValid()) {
            m_udpSocket->setMulticastInterface(iface);
            sendDiscoveryProbe();
        } else {
            qWarning() << "Interface not found:" << interfaceName;
        }
    }
}

void SystemController::addDevice(int index)
{
    Camera cam = m_discoveryModel->getCamera(index);
    if (!cam.id.isEmpty()) {
        if (!m_cameraModel->contains(cam.ip)) {
             m_cameraModel->addCamera(cam);
             saveState();
        }
    }
}

void SystemController::addManualCamera(const QString &name, const QString &ip, const QString &url, int port, int onvifPort, const QString &login, const QString &password)
{
    Camera cam;
    cam.id = QUuid::createUuid().toString();
    cam.name = name;
    cam.ip = ip;
    cam.streamUrl = url;
    // Heuristic: if user passes stream=0 treat it as HD and derive SD from =1; if stream=1, do the opposite.
    if (url.contains("stream=0")) {
        cam.hdStreamUrl = url;
        cam.sdStreamUrl = QString(url).replace("stream=0", "stream=1");
    } else if (url.contains("stream=1")) {
        cam.sdStreamUrl = url;
        cam.hdStreamUrl = QString(url).replace("stream=1", "stream=0");
    } else {
        cam.sdStreamUrl = url;
        cam.hdStreamUrl = url;
    }
    cam.status = "Online";
    cam.port = port;
    cam.onvifPort = onvifPort;
    cam.login = login;
    cam.password = password;
    
    qDebug() << "Adding camera:" << name << "IP:" << ip << "URL:" << url;

    if (!m_cameraModel->contains(cam.ip)) {
            m_cameraModel->addCamera(cam);
            saveState();
    }
}

void SystemController::updateCamera(int index, const QString &name, const QString &ip, const QString &url, int port, int onvifPort, const QString &login, const QString &password)
{
    Camera cam = m_cameraModel->getCamera(index);
    QString oldIp = cam.ip; // Save old IP for matching if ID fails

    cam.name = name;
    cam.ip = ip;
    cam.streamUrl = url;
    // Heuristic: if user passes stream=0 treat it as HD and derive SD from =1; if stream=1, do the opposite.
    if (url.contains("stream=0")) {
        cam.hdStreamUrl = url;
        cam.sdStreamUrl = QString(url).replace("stream=0", "stream=1");
    } else if (url.contains("stream=1")) {
        cam.sdStreamUrl = url;
        cam.hdStreamUrl = QString(url).replace("stream=1", "stream=0");
    } else {
        cam.sdStreamUrl = url;
        cam.hdStreamUrl = url;
    }
    cam.port = port;
    cam.onvifPort = onvifPort;
    cam.login = login;
    cam.password = password;
    
    m_cameraModel->setCamera(index, cam);

    // Update grid model if this camera is present
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
        }
    }

    saveState();
}

void SystemController::removeDevice(int index)
{
    // Also remove from grid if present
    Camera cam = m_cameraModel->getCamera(index);
    if (!cam.ip.isEmpty()) {
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
            // If all slots filled, overwrite the first slot as a fallback
            preserveSpan(0, cam);
            m_gridModel->setCamera(0, cam);
            saveState();
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

void SystemController::sendDiscoveryProbe()
{
    // WS-Discovery Probe Message
    QByteArray probeData = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<e:Envelope xmlns:e=\"http://www.w3.org/2003/05/soap-envelope\" "
        "xmlns:w=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
        "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
        "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">"
        "<e:Header>"
        "<w:MessageID>uuid:" + QUuid::createUuid().toString().toUtf8() + "</w:MessageID>"
        "<w:To e:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</w:To>"
        "<w:Action a:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</w:Action>"
        "</e:Header>"
        "<e:Body>"
        "<d:Probe>"
        "<d:Types>dn:NetworkVideoTransmitter</d:Types>"
        "</d:Probe>"
        "</e:Body>"
        "</e:Envelope>";

    // Send to multicast address
    m_udpSocket->writeDatagram(probeData, QHostAddress("239.255.255.250"), 3702);
}

QString SystemController::stateFilePath() const
{
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(baseDir);
    return baseDir + "/state.json";
}

QJsonObject SystemController::cameraToJson(const Camera &cam)
{
    QJsonObject obj;
    obj["id"] = cam.id;
    obj["name"] = cam.name;
    obj["ip"] = cam.ip;
    obj["streamUrl"] = cam.streamUrl;
    obj["sdStreamUrl"] = cam.sdStreamUrl;
    obj["hdStreamUrl"] = cam.hdStreamUrl;
    obj["status"] = cam.status;
    obj["port"] = cam.port;
    obj["onvifPort"] = cam.onvifPort;
    obj["login"] = cam.login;
    obj["password"] = cam.password;
    obj["group"] = cam.group;
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
    cam.password = obj.value("password").toString();
    cam.group = obj.value("group").toString();
    return cam;
}

void SystemController::saveState() const
{
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
    root["analytics"] = QJsonObject::fromVariantMap(m_analyticsEngine->getSettings());
    root["appSettings"] = QJsonObject::fromVariantMap(m_appSettings);
    root["cameraGroups"] = groups;
    root["layoutTemplates"] = QJsonArray::fromVariantList(m_layoutTemplates);

    const QString path = stateFilePath();
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
        f.close();
    } else {
        qWarning() << "Failed to save state to" << path << f.errorString();
    }
}

void SystemController::loadState()
{
    const QString path = stateFilePath();
    QFile f(path);
    if (!f.exists()) {
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
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open state file" << path << f.errorString();
        return;
    }
    const QByteArray data = f.readAll();
    f.close();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "State file is not an object" << path;
        return;
    }
    const QJsonObject root = doc.object();

    if (root.contains("analytics")) {
        m_analyticsEngine->setSettings(root.value("analytics").toObject().toVariantMap());
    }

    if (root.contains("appSettings")) {
        QVariantMap savedSettings = root.value("appSettings").toObject().toVariantMap();
        // Merge with defaults
        for (auto it = savedSettings.begin(); it != savedSettings.end(); ++it) {
            m_appSettings[it.key()] = it.value();
        }
        m_gridRows = m_appSettings.value("gridRows", 2).toInt();
        m_gridCols = m_appSettings.value("gridCols", 2).toInt();
        
        // Sanity check to prevent crash from invalid/legacy settings
        if (m_gridRows > 64) m_gridRows = 2;
        if (m_gridCols > 64) m_gridCols = 2;
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
    m_gridModel->clear();

    const QJsonArray cameras = root.value("cameras").toArray();
    for (const auto &v : cameras) {
        m_cameraModel->addCamera(cameraFromJson(v.toObject()));
        const QString groupName = v.toObject().value("group").toString();
        if (!groupName.isEmpty() && !m_cameraGroups.contains(groupName, Qt::CaseInsensitive)) {
            m_cameraGroups.append(groupName);
        }
    }

    QJsonArray grid = root.value("grid").toArray();
    const int slotCount = grid.size() > 0 ? grid.size() : 4;
    for (int i = 0; i < slotCount; ++i) {
        Camera cam;
        if (i < grid.size()) {
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
        }
        m_gridModel->addCamera(cam);
    }

    emit cameraGroupsChanged();
}

static quint64 fileTimeToUInt64(const FILETIME &ft)
{
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return li.QuadPart;
}

double SystemController::processCpuPercent()
{
#ifndef Q_OS_WIN
    return 0.0;
#else
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

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int cpuCount = sysInfo.dwNumberOfProcessors > 0 ? sysInfo.dwNumberOfProcessors : 1;

    double cpu = (static_cast<double>(procDelta) / static_cast<double>(sysDelta)) * 100.0;
    // Clamp to reasonable range
    if (cpu < 0.0) cpu = 0.0;
    if (cpu > 100.0) cpu = 100.0;
    return cpu;
#endif
}

double SystemController::processMemoryMB()
{
#ifndef Q_OS_WIN
    return 0.0;
#else
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }
    return 0.0;
#endif
}

void SystemController::openFolder(const QString &path)
{
    if (path.isEmpty()) return;
    
    QString targetPath = path;
    QFileInfo fi(path);
    if (fi.exists() && fi.isFile()) {
        targetPath = fi.absolutePath();
    }
    
    QDesktopServices::openUrl(QUrl::fromLocalFile(targetPath));
}

void SystemController::saveAppSettings(const QVariantMap &settings)
{
    m_appSettings = settings;
    emit appSettingsChanged();
    saveState();
}

QVariantMap SystemController::getAppSettings() const
{
    return m_appSettings;
}

void SystemController::addCameraGroup(const QString &groupName)
{
    const QString trimmed = groupName.trimmed();
    if (trimmed.isEmpty()) return;
    if (m_cameraGroups.contains(trimmed, Qt::CaseInsensitive)) return;
    m_cameraGroups.append(trimmed);
    emit cameraGroupsChanged();
    saveState();
}

void SystemController::setCameraGroup(int cameraIndex, const QString &groupName)
{
    if (cameraIndex < 0 || cameraIndex >= m_cameraModel->rowCount()) return;
    Camera cam = m_cameraModel->getCamera(cameraIndex);
    const QString trimmed = groupName.trimmed();
    if (cam.group == trimmed) return;
    cam.group = trimmed;
    m_cameraModel->setCamera(cameraIndex, cam);

    if (!trimmed.isEmpty() && !m_cameraGroups.contains(trimmed, Qt::CaseInsensitive)) {
        m_cameraGroups.append(trimmed);
        emit cameraGroupsChanged();
    }
    saveState();
}

void SystemController::removeCameraGroup(const QString &groupName)
{
    const QString trimmed = groupName.trimmed();
    if (trimmed.isEmpty()) return; // do not remove default empty group
    int idx = -1;
    for (int i = 0; i < m_cameraGroups.size(); ++i) {
        if (m_cameraGroups.at(i).compare(trimmed, Qt::CaseInsensitive) == 0) {
            idx = i; break;
        }
    }
    if (idx < 0) return;

    // Clear group on cameras
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        Camera cam = m_cameraModel->getCamera(i);
        if (cam.group.compare(trimmed, Qt::CaseInsensitive) == 0) {
            cam.group.clear();
            m_cameraModel->setCamera(i, cam);
        }
    }

    m_cameraGroups.removeAt(idx);
    emit cameraGroupsChanged();
    saveState();
}

void SystemController::renameCameraGroup(const QString &oldName, const QString &newName)
{
    const QString from = oldName.trimmed();
    const QString to = newName.trimmed();
    if (from.isEmpty() || to.isEmpty()) return; // default group cannot be renamed and empty target not allowed
    if (from.compare(to, Qt::CaseInsensitive) == 0) return;

    int idx = -1;
    for (int i = 0; i < m_cameraGroups.size(); ++i) {
        if (m_cameraGroups.at(i).compare(from, Qt::CaseInsensitive) == 0) {
            idx = i; break;
        }
    }
    if (idx < 0) return;

    // Update list, avoid duplicates
    if (m_cameraGroups.contains(to, Qt::CaseInsensitive)) {
        m_cameraGroups.removeAt(idx);
    } else {
        m_cameraGroups[idx] = to;
    }

    // Update cameras that belong to the old group
    for (int i = 0; i < m_cameraModel->rowCount(); ++i) {
        Camera cam = m_cameraModel->getCamera(i);
        if (cam.group.compare(from, Qt::CaseInsensitive) == 0) {
            cam.group = to;
            m_cameraModel->setCamera(i, cam);
        }
    }

    emit cameraGroupsChanged();
    saveState();
}

QVariantList SystemController::getRecordings(const QString &cameraIp, const QDate &date)
{
    QVariantList results;
    QString path = m_appSettings.value("recordingsPath").toString();
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/OpenIPC";
    }
    
    QDir dir(path);
    if (!dir.exists()) return results;
    
    QString dateStr1 = date.toString("yyyy-MM-dd");
    QString dateStr2 = date.toString("yyyyMMdd");
    
    // Recursive search for mp4 files
    QDirIterator it(path, QStringList() << "*.mp4" << "*.mkv" << "*.avi", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QString fileName = it.fileName();
        
        // Filter by date
        if (!fileName.contains(dateStr1) && !fileName.contains(dateStr2)) {
            continue;
        }
        
        // Filter by camera IP (if provided and not empty)
        // Note: IP in filename might be sanitized (e.g. 192.168.1.10 -> 192_168_1_10)
        if (!cameraIp.isEmpty()) {
            QString sanitizedIp = cameraIp;
            sanitizedIp.replace(".", "_");
            if (!fileName.contains(cameraIp) && !fileName.contains(sanitizedIp)) {
                // Try checking parent folder name too
                QString parent = QFileInfo(filePath).dir().dirName();
                if (!parent.contains(cameraIp) && !parent.contains(sanitizedIp)) {
                    continue;
                }
            }
        }
        
        QFileInfo fi(filePath);
        QVariantMap rec;
        rec["fileName"] = fileName;
        rec["filePath"] = "file:///" + filePath; // URL for QML
        rec["size"] = fi.size();
        rec["created"] = fi.birthTime();
        rec["modified"] = fi.lastModified();
        
        // Try to extract time from filename
        // Look for HH-mm-ss or HHmmss pattern
        // Simple heuristic: find 6 digits or XX-XX-XX
        // For now, just use modification time as a fallback for sorting
        
        results.append(rec);
    }
    
    // Sort by time (modification time)
    std::sort(results.begin(), results.end(), [](const QVariant &a, const QVariant &b) {
        return a.toMap()["modified"].toDateTime() < b.toMap()["modified"].toDateTime();
    });
    
    return results;
}

QList<int> SystemController::getRecordingDates(const QString &cameraIp, int year, int month)
{
    QList<int> days;
    QString path = m_appSettings.value("recordingsPath").toString();
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/OpenIPC";
    }
    
    QDir dir(path);
    if (!dir.exists()) return days;
    
    QString sanitizedIp = cameraIp;
    sanitizedIp.replace(".", "_");
    
    // Filter: IP_YYYY-MM-*
    QString pattern = QString("%1_%2-%3-*").arg(sanitizedIp).arg(year).arg(month, 2, 10, QChar('0'));
    QStringList filters;
    filters << pattern;
    
    QDirIterator it(path, filters, QDir::Files);
    while (it.hasNext()) {
        it.next();
        QString filename = it.fileName();
        // Extract day from filename: IP_YYYY-MM-DD_...
        // Split by underscore
        QStringList parts = filename.split("_");
        // Find the part that looks like a date YYYY-MM-DD
        for (const QString &part : parts) {
            if (part.count('-') == 2 && part.length() == 10) {
                QDate d = QDate::fromString(part, "yyyy-MM-dd");
                if (d.isValid() && d.year() == year && d.month() == month) {
                    if (!days.contains(d.day())) {
                        days.append(d.day());
                    }
                }
            }
        }
    }
    
    return days;
}

void SystemController::toggleRecording(int gridIndex)
{
    if (m_activeRecordings.contains(gridIndex)) {
        // Stop recording
        QProcess *proc = m_activeRecordings.take(gridIndex);
        if (proc) {
            qDebug() << "Stopping recording for grid index" << gridIndex;
            
            // Update model immediately to reflect UI state
            Camera cam = m_gridModel->getCamera(gridIndex);
            cam.isRecording = false;
            m_gridModel->setCamera(gridIndex, cam);

            if (proc->state() == QProcess::Running) {
                // Send 'q' to quit gracefully
                proc->write("q");
                proc->closeWriteChannel();
                
                // Set up a timer to force kill if it doesn't exit gracefully
                QTimer *timer = new QTimer(proc);
                timer->setSingleShot(true);
                timer->setInterval(3000); // 3 seconds timeout
                
                connect(timer, &QTimer::timeout, proc, [proc]() {
                    if (proc->state() == QProcess::Running) {
                        qDebug() << "FFmpeg did not stop gracefully, terminating...";
                        proc->terminate();
                        
                        // Give it another 2 seconds then kill
                        QTimer::singleShot(2000, proc, [proc]() {
                            if (proc->state() == QProcess::Running) {
                                qDebug() << "FFmpeg did not stop with terminate, killing...";
                                proc->kill();
                            }
                        });
                    }
                });
                
                connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), proc, [proc, timer]() {
                     timer->stop();
                     proc->deleteLater();
                });
                
                timer->start();
            } else {
                proc->deleteLater();
            }
        }
    } else {
        // Start recording
        Camera cam = m_gridModel->getCamera(gridIndex);
        if (cam.ip.isEmpty()) return;
        
        QString url = cam.hdStreamUrl;
        if (url.isEmpty()) url = cam.streamUrl;
        if (url.isEmpty()) return;

        // Inject credentials if present and not already in URL
        if (!cam.login.isEmpty() && !cam.password.isEmpty() && !url.contains("@")) {
            QUrl qUrl(url);
            if (qUrl.isValid()) {
                qUrl.setUserName(cam.login);
                qUrl.setPassword(cam.password);
                url = qUrl.toString();
            }
        }
        
        QString path = m_appSettings.value("recordingsPath").toString();
        if (path.isEmpty()) {
            path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/OpenIPC";
        }
        QDir().mkpath(path);
        
        // Filename format: IP_Date_Time.mp4
        // e.g. 192_168_1_10_2025-12-25_12-00-00.mp4
        QString sanitizedIp = cam.ip;
        sanitizedIp.replace(".", "_");
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
        QString filename = QString("%1/%2_%3.mp4").arg(path, sanitizedIp, timestamp);
        
        qDebug() << "Starting recording for" << cam.ip << "to" << filename;
        
        QProcess *proc = new QProcess(this);
        
        connect(proc, &QProcess::readyReadStandardOutput, this, [proc]() {
            qDebug() << "[FFmpeg stdout] " << proc->readAllStandardOutput();
        });
        
        connect(proc, &QProcess::readyReadStandardError, this, [proc]() {
            qWarning() << "[FFmpeg stderr] " << proc->readAllStandardError();
        });

        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, proc, gridIndex](int exitCode, QProcess::ExitStatus exitStatus) {
            qDebug() << "[FFmpeg finished] Code:" << exitCode << "Status:" << exitStatus;
            
            // If process finished unexpectedly (crashed or error), update UI
            if (m_activeRecordings.contains(gridIndex) && m_activeRecordings[gridIndex] == proc) {
                m_activeRecordings.remove(gridIndex);
                Camera cam = m_gridModel->getCamera(gridIndex);
                cam.isRecording = false;
                m_gridModel->setCamera(gridIndex, cam);
            }
            proc->deleteLater();
        });

        // FFmpeg command: ffmpeg -i URL -c:v copy -c:a aac -y filename
        // -c:v copy ensures no video transcoding (low CPU usage)
        // -c:a aac ensures audio compatibility with MP4 container (pcm_alaw is not supported in MP4)
        // -rtsp_transport tcp ensures reliability
        QStringList args;
        args << "-rtsp_transport" << "tcp" 
             << "-i" << url 
             << "-c:v" << "copy" 
             << "-c:a" << "aac" 
             << "-y" << filename;
             
        // Check if ffmpeg is in path or local bin
        QString program = "ffmpeg";
#ifdef Q_OS_WIN
        if (QFile::exists(QCoreApplication::applicationDirPath() + "/ffmpeg.exe")) {
            program = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
        }
#endif
        
        connect(proc, &QProcess::started, this, [this, gridIndex, proc]() {
             m_activeRecordings.insert(gridIndex, proc);
             Camera cam = m_gridModel->getCamera(gridIndex);
             cam.isRecording = true;
             m_gridModel->setCamera(gridIndex, cam);
        });

        connect(proc, &QProcess::errorOccurred, this, [proc](QProcess::ProcessError error) {
             qWarning() << "FFmpeg process error:" << error << proc->errorString();
             proc->deleteLater();
        });
        
        proc->start(program, args);
    }
}

void SystemController::exportRecording(const QString &inputFile, const QString &outputFile, int startMs, int endMs)
{
    QProcess *proc = new QProcess(this);
    
    // Convert ms to HH:MM:SS.mmm format
    auto formatTime = [](int ms) {
        int h = ms / 3600000;
        ms %= 3600000;
        int m = ms / 60000;
        ms %= 60000;
        int s = ms / 1000;
        ms %= 1000;
        return QString("%1:%2:%3.%4")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'))
            .arg(ms, 3, 10, QChar('0'));
    };
    
    QString startTime = formatTime(startMs);
    QString duration = formatTime(endMs - startMs);
    
    QStringList args;
    args << "-ss" << startTime
         << "-i" << inputFile
         << "-t" << duration
         << "-c" << "copy"
         << "-y" << outputFile;
         
    QString program = "ffmpeg";
#ifdef Q_OS_WIN
    if (QFile::exists(QCoreApplication::applicationDirPath() + "/ffmpeg.exe")) {
        program = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
    }
#endif

    qDebug() << "Exporting:" << program << args.join(" ");

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [proc](int exitCode, QProcess::ExitStatus) {
        qDebug() << "Export finished with code" << exitCode;
        proc->deleteLater();
    });
    
    proc->start(program, args);
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
    // Ensure grid has correct number of slots FIRST
    // This prevents "porridge" where new cells (if growing) would be missed by the loop below
    // and also ensures we shrink if switching to a smaller grid.
    updateGridSize(rows * cols);

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
    if (!layout.contains("cells")) {
        applyLayoutPreset(rows, cols);
        return;
    }
    
    QVariantList cells = layout.value("cells").toList();
    
    // Base grid is 1200x1200
    // We scale the logical rows/cols to 1200
    double rowScale = 1200.0 / (double)rows;
    double colScale = 1200.0 / (double)cols;
    
    // Resize grid capacity to match template exactly
    // This ensures we remove extra cells if switching from a larger layout
    int needed = cells.size();
    updateGridSize(needed);
    
    // Apply spans
    for (int i = 0; i < cells.size(); ++i) {
        if (i >= m_gridModel->rowCount()) break;
        
        QVariantMap cellDef = cells[i].toMap();
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
        QString savePath = m_appSettings.value("screenshotsPath").toString();
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
    QString savePath = m_appSettings.value("screenshotsPath").toString();
    if (savePath.isEmpty()) savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDir().mkpath(savePath);
    return QDir(savePath).filePath(filename);
}

void SystemController::setLayoutTemplates(const QVariantList &templates)
{
    if (m_layoutTemplates != templates) {
        m_layoutTemplates = templates;
        saveState();
        emit layoutTemplatesChanged();
    }
}

void SystemController::onUdpReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        QByteArray data = datagram.data();
        
        // Very basic parsing to check if it's a ProbeMatch
        if (data.contains("ProbeMatches")) {
            QString senderIp = datagram.senderAddress().toString();
            // Handle IPv6 mapped IPv4 addresses
            if (senderIp.startsWith("::ffff:")) {
                senderIp = senderIp.mid(7);
            }

            if (!m_discoveryModel->contains(senderIp)) {
                qDebug() << "Found camera at:" << senderIp;
                
                QString xmlStr = QString::fromUtf8(data);
                QString extractedName = "OpenIPC Camera";
                
                // Try to find name in Scopes
                int nameIdx = xmlStr.indexOf("onvif://www.onvif.org/name/");
                if (nameIdx != -1) {
                    int start = nameIdx + 27; // length of "onvif://www.onvif.org/name/"
                    int endSpace = xmlStr.indexOf(" ", start);
                    int endTag = xmlStr.indexOf("<", start);
                    int end = -1;
                    
                    if (endSpace != -1 && endTag != -1) end = std::min(endSpace, endTag);
                    else if (endSpace != -1) end = endSpace;
                    else end = endTag;
                    
                    if (end != -1) {
                        extractedName = xmlStr.mid(start, end - start);
                        extractedName = QUrl::fromPercentEncoding(extractedName.toUtf8());
                    }
                } 
                
                // If name is still default or empty, try hardware
                if (extractedName == "OpenIPC Camera" || extractedName.isEmpty()) {
                    int hwIdx = xmlStr.indexOf("onvif://www.onvif.org/hardware/");
                    if (hwIdx != -1) {
                        int start = hwIdx + 31; // length of "onvif://www.onvif.org/hardware/"
                        int endSpace = xmlStr.indexOf(" ", start);
                        int endTag = xmlStr.indexOf("<", start);
                        int end = -1;
                        
                        if (endSpace != -1 && endTag != -1) end = std::min(endSpace, endTag);
                        else if (endSpace != -1) end = endSpace;
                        else end = endTag;
                        
                        if (end != -1) {
                            extractedName = xmlStr.mid(start, end - start);
                            extractedName = QUrl::fromPercentEncoding(extractedName.toUtf8());
                        }
                    }
                }
                
                // Fallback if empty
                if (extractedName.isEmpty()) extractedName = "OpenIPC Camera";

                Camera cam;
                cam.id = QUuid::createUuid().toString();
                cam.name = extractedName;
                cam.ip = senderIp;
                cam.hdStreamUrl = QString("rtsp://%1/stream=0").arg(senderIp); // main stream
                cam.sdStreamUrl = QString("rtsp://%1/stream=1").arg(senderIp); // sub stream
                cam.streamUrl = cam.hdStreamUrl; // legacy main
                cam.status = "Online";
                
                m_discoveryModel->addCamera(cam);
            }
        }
    }
}
