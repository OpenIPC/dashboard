#pragma once
#include <QObject>
#include <QAbstractListModel>
#include <QVector>
#include <QMutex>

// Forward declaration to avoid including heavy SDK headers in header file if possible, 
// but we need DEVICE_NET_INFO_EX for the callback signature if we make it static member.
// Or we can include it here.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "dhnetsdk.h"

// struct DEVICE_NET_INFO_EX;

// #ifndef CALLBACK
// #if defined(_WIN32)
// #define CALLBACK __stdcall
// #else
// #define CALLBACK
// #endif
// #endif

// #ifndef LLONG
// typedef long long LLONG;
// #endif

struct DiscoveredDevice {
    QString ip;
    int port;
    QString mac;
    QString type;
    QString serial;
    QString version;
    QString manufacturer;
};

class DiscoveryModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IpRole = Qt::UserRole + 1,
        PortRole,
        MacRole,
        TypeRole,
        SerialRole,
        VersionRole,
        ManufacturerRole
    };

    explicit DiscoveryModel(QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addDevice(const DiscoveredDevice& device);
    void clear();

private:
    QVector<DiscoveredDevice> m_devices;
    mutable QMutex m_mutex;
};

class DiscoveryController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isSearching READ isSearching NOTIFY isSearchingChanged)
    Q_PROPERTY(DiscoveryModel* model READ model CONSTANT)

public:
    explicit DiscoveryController(QObject *parent = nullptr);
    ~DiscoveryController();

    Q_INVOKABLE void startSearch();
    Q_INVOKABLE void stopSearch();

    bool isSearching() const { return m_isSearching; }
    DiscoveryModel* model() const { return m_model; }

signals:
    void isSearchingChanged();
    void deviceFound(const DiscoveredDevice& device);

private:
    static void CALLBACK SearchDevicesCallback(DEVICE_NET_INFO_EX *pDevNetInfo, void* pUserData);
    void handleDeviceFound(const DiscoveredDevice& device);

    LLONG m_searchHandle = 0;
    bool m_isSearching = false;
    DiscoveryModel* m_model;
    bool m_sdkInit = false;
};
