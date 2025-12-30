#include "DiscoveryController.h"
#include <QDebug>
// #include <QTextCodec>
#include <QTimer>

DiscoveryModel::DiscoveryModel(QObject *parent) : QAbstractListModel(parent)
{
}

int DiscoveryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_devices.count();
}

QVariant DiscoveryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_devices.count())
        return QVariant();

    const auto &device = m_devices[index.row()];

    switch (role) {
    case IpRole: return device.ip;
    case PortRole: return device.port;
    case MacRole: return device.mac;
    case TypeRole: return device.type;
    case SerialRole: return device.serial;
    case VersionRole: return device.version;
    case ManufacturerRole: return device.manufacturer;
    }

    return QVariant();
}

QHash<int, QByteArray> DiscoveryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IpRole] = "ip";
    roles[PortRole] = "port";
    roles[MacRole] = "mac";
    roles[TypeRole] = "type";
    roles[SerialRole] = "serial";
    roles[VersionRole] = "version";
    roles[ManufacturerRole] = "manufacturer";
    return roles;
}

void DiscoveryModel::addDevice(const DiscoveredDevice &device)
{
    QMutexLocker locker(&m_mutex);
    // Check for duplicates
    for (const auto &d : m_devices) {
        if (d.mac == device.mac && d.ip == device.ip) {
            return;
        }
    }

    beginInsertRows(QModelIndex(), m_devices.count(), m_devices.count());
    m_devices.append(device);
    endInsertRows();
}

void DiscoveryModel::clear()
{
    QMutexLocker locker(&m_mutex);
    beginResetModel();
    m_devices.clear();
    endResetModel();
}

// --- DiscoveryController ---

DiscoveryController::DiscoveryController(QObject *parent) : QObject(parent)
{
    m_model = new DiscoveryModel(this);
    
    // Initialize SDK
    // Note: In a real app, SDK init should probably be global and happen once.
    // For now we do it here if not already done.
    // CLIENT_Init(nullptr, 0); // Assuming it might be called elsewhere or we call it here.
    // But CLIENT_Init needs a disconnect callback.
    
    // We'll assume SystemController or main might handle global init, 
    // but for this specific feature, we ensure it's initialized.
    // Actually, let's just call it. It returns FALSE if already initialized or fails.
    m_sdkInit = CLIENT_Init(nullptr, 0);
    if (m_sdkInit) {
        qInfo() << "Dahua SDK Initialized in DiscoveryController";
    } else {
        qWarning() << "Dahua SDK Init failed or already initialized";
    }
}

DiscoveryController::~DiscoveryController()
{
    stopSearch();
    if (m_sdkInit) {
        CLIENT_Cleanup();
    }
}

void DiscoveryController::startSearch()
{
    if (m_isSearching) stopSearch();

    m_model->clear();

    // CLIENT_StartSearchDevices
    // fSearchDevicesCB cbSearchDevices, void* pUserData, char* szLocalIp=NULL
    m_searchHandle = CLIENT_StartSearchDevices(SearchDevicesCallback, this, nullptr);
    
    if (m_searchHandle != 0) {
        m_isSearching = true;
        emit isSearchingChanged();
        qInfo() << "Started Dahua Device Search, handle:" << m_searchHandle;
        
        // Auto-stop after 15 seconds
        QTimer::singleShot(15000, this, &DiscoveryController::stopSearch);
    } else {
        qWarning() << "Failed to start Dahua Device Search";
    }
}

void DiscoveryController::stopSearch()
{
    if (!m_isSearching) return;

    if (m_searchHandle != 0) {
        CLIENT_StopSearchDevices(m_searchHandle);
        m_searchHandle = 0;
    }

    m_isSearching = false;
    emit isSearchingChanged();
    qInfo() << "Stopped Dahua Device Search";
}

void CALLBACK DiscoveryController::SearchDevicesCallback(DEVICE_NET_INFO_EX *pDevNetInfo, void *pUserData)
{
    if (!pDevNetInfo || !pUserData) return;

    DiscoveryController* controller = static_cast<DiscoveryController*>(pUserData);
    
    DiscoveredDevice device;
    device.ip = QString::fromLatin1(pDevNetInfo->szIP);
    device.port = pDevNetInfo->nPort;
    device.mac = QString::fromLatin1(pDevNetInfo->szMac);
    device.type = QString::fromLatin1(pDevNetInfo->szDeviceType);
    device.serial = QString::fromLatin1(pDevNetInfo->szSerialNo);
    device.version = QString::fromLatin1(pDevNetInfo->szDevSoftVersion);
    
    // Map manufacturer ID to string if possible, or just use ID
    // pDevNetInfo->byManuFactory
    // For now just generic
    device.manufacturer = "Dahua/Compatible";

    // Post to main thread to update model safely
    QMetaObject::invokeMethod(controller, [controller, device]() {
        controller->handleDeviceFound(device);
    });
}

void DiscoveryController::handleDeviceFound(const DiscoveredDevice &device)
{
    qInfo() << "Found device:" << device.ip << device.type << device.serial;
    m_model->addDevice(device);
    emit deviceFound(device);
}
