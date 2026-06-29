#pragma once

#include <QElapsedTimer>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QSet>
#include <QVariantList>

class QNetworkReply;
class QTcpSocket;
class QTimer;
class QUdpSocket;

struct NetworkDiscoveryCandidate
{
    QString ip;
    QString name;
    QString manufacturer;
    QString model;
    QString serial;
    QString method;
    QString evidence;
    int httpPort = 80;
    int rtspPort = 554;
    int onvifPort = 80;
    int confidence = 0;
    bool openIpc = false;
    bool majestic = false;
    bool onvif = false;
    bool rtsp = false;
};

Q_DECLARE_METATYPE(NetworkDiscoveryCandidate)

// Multi-protocol LAN camera discovery.
//
// OpenIPC is primarily identified by its official mDNS TXT marker
// (vendor=OpenIPC) or a Majestic API response. WS-Discovery finds ONVIF
// devices, and a bounded HTTP/RTSP sweep covers legacy/disabled discovery.
class NetworkDiscoveryService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString phase READ phase NOTIFY phaseChanged)
    Q_PROPERTY(int foundCount READ foundCount NOTIFY foundCountChanged)

public:
    explicit NetworkDiscoveryService(QObject *parent = nullptr);
    ~NetworkDiscoveryService() override;

    bool running() const { return m_running; }
    int progress() const { return m_progress; }
    QString phase() const { return m_phase; }
    int foundCount() const { return m_foundIps.size(); }

    Q_INVOKABLE void start(const QString &interfaceName = QString(), bool deepScan = false);
    Q_INVOKABLE void stop();

    static QByteArray buildMdnsQueryForTest();
    static QVariantList parseMdnsPacketForTest(const QByteArray &packet,
                                               const QString &senderAddress);
    static QStringList subnetHostsForTest(const QString &address, int prefixLength,
                                          bool deepScan);

signals:
    void candidateFound(const NetworkDiscoveryCandidate &candidate);
    void runningChanged();
    void progressChanged();
    void phaseChanged();
    void foundCountChanged();
    void finished(bool cancelled);

private:
    struct InterfaceTarget {
        QNetworkInterface networkInterface;
        QNetworkAddressEntry address;
    };
    struct HttpJob {
        QString ip;
        int port = 80;
        QString path;
    };

    QList<InterfaceTarget> selectInterfaces(const QString &interfaceName) const;
    void startMulticastDiscovery(const QList<InterfaceTarget> &targets);
    void sendWsProbe(QUdpSocket *socket, bool typedProbe);
    void handleWsDatagram(const QByteArray &data, const QHostAddress &sender);
    void handleMdnsDatagram(const QByteArray &data, const QHostAddress &sender);
    void enqueueSubnetSweep(const QList<InterfaceTarget> &targets, bool deepScan);
    void pumpHttpJobs();
    void handleHttpReply(QNetworkReply *reply);
    void pumpRtspJobs();
    void finishRtspSocket(QTcpSocket *socket, const QString &ip, bool inspectResponse);
    void publish(const NetworkDiscoveryCandidate &candidate);
    void updateProgress();
    void setPhase(const QString &phase);
    void complete(bool cancelled);

    static QByteArray buildWsProbe(bool typedProbe);
    static QVariantList parseMdnsPacket(const QByteArray &packet,
                                        const QHostAddress &senderAddress);
    static QStringList subnetHosts(const QHostAddress &address, int prefixLength,
                                   bool deepScan);

    QNetworkAccessManager m_http;
    QList<QPointer<QUdpSocket>> m_udpSockets;
    QSet<QNetworkReply *> m_httpReplies;
    QSet<QTcpSocket *> m_rtspSockets;
    QQueue<HttpJob> m_httpQueue;
    QQueue<QString> m_rtspQueue;
    QSet<QString> m_foundIps;
    QSet<QString> m_publishedEvidence;
    QElapsedTimer m_elapsed;
    QTimer *m_deadlineTimer = nullptr;
    QTimer *m_progressTimer = nullptr;
    int m_activeHttp = 0;
    int m_activeRtsp = 0;
    int m_totalJobs = 0;
    int m_completedJobs = 0;
    int m_deadlineMs = 0;
    int m_progress = 0;
    quint64 m_generation = 0;
    bool m_running = false;
    QString m_phase;
};
