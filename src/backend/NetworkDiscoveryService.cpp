#include "NetworkDiscoveryService.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkDatagram>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QUrl>
#include <QUuid>
#include <QXmlStreamReader>

#include <algorithm>

namespace {

// A /20 contains 4094 usable addresses. These bounded limits and LAN-oriented
// timeouts allow the opt-in deep scan to actually cover that range before the
// global deadline, while still avoiding unbounded socket creation.
constexpr int kMaximumParallelHttp = 192;
constexpr int kMaximumParallelRtsp = 256;
constexpr int kHttpProbeTimeoutMs = 650;
constexpr int kRtspProbeTimeoutMs = 650;
constexpr int kQuickDeadlineMs = 12000;
constexpr int kDeepDeadlineMs = 35000;
constexpr int kMinimumMulticastWindowMs = 1200;

quint16 readU16(const QByteArray &data, qsizetype offset, bool *ok = nullptr)
{
    const bool valid = offset >= 0 && offset + 2 <= data.size();
    if (ok) *ok = valid;
    if (!valid) return 0;
    return (static_cast<quint16>(static_cast<quint8>(data.at(offset))) << 8)
        | static_cast<quint8>(data.at(offset + 1));
}

quint32 readU32(const QByteArray &data, qsizetype offset, bool *ok = nullptr)
{
    const bool valid = offset >= 0 && offset + 4 <= data.size();
    if (ok) *ok = valid;
    if (!valid) return 0;
    return (static_cast<quint32>(static_cast<quint8>(data.at(offset))) << 24)
        | (static_cast<quint32>(static_cast<quint8>(data.at(offset + 1))) << 16)
        | (static_cast<quint32>(static_cast<quint8>(data.at(offset + 2))) << 8)
        | static_cast<quint8>(data.at(offset + 3));
}

void appendU16(QByteArray *data, quint16 value)
{
    data->append(static_cast<char>((value >> 8) & 0xff));
    data->append(static_cast<char>(value & 0xff));
}

void appendDnsName(QByteArray *data, const QString &name)
{
    const QStringList labels = name.split(QLatin1Char('.'), Qt::SkipEmptyParts);
    for (const QString &label : labels) {
        const QByteArray encoded = label.toUtf8();
        data->append(static_cast<char>(std::min<qsizetype>(encoded.size(), 63)));
        data->append(encoded.left(63));
    }
    data->append('\0');
}

QString readDnsName(const QByteArray &data, qsizetype *offset, int depth = 0)
{
    if (!offset || *offset < 0 || *offset >= data.size() || depth > 16) return {};
    qsizetype cursor = *offset;
    bool jumped = false;
    QStringList labels;
    int labelsRead = 0;
    while (cursor < data.size() && labelsRead++ < 128) {
        const quint8 length = static_cast<quint8>(data.at(cursor));
        if (length == 0) {
            cursor++;
            if (!jumped) *offset = cursor;
            return labels.join(QLatin1Char('.'));
        }
        if ((length & 0xc0) == 0xc0) {
            bool ok = false;
            const quint16 pointer = readU16(data, cursor, &ok);
            if (!ok) return {};
            if (!jumped) *offset = cursor + 2;
            jumped = true;
            qsizetype pointedOffset = pointer & 0x3fff;
            const QString suffix = readDnsName(data, &pointedOffset, depth + 1);
            if (suffix.isEmpty() && labels.isEmpty()) return {};
            if (!suffix.isEmpty()) labels.append(suffix);
            return labels.join(QLatin1Char('.'));
        }
        if ((length & 0xc0) != 0 || cursor + 1 + length > data.size()) return {};
        labels.append(QString::fromUtf8(data.mid(cursor + 1, length)));
        cursor += 1 + length;
        if (!jumped) *offset = cursor;
    }
    return {};
}

QString headerValue(const QList<QByteArray> &headers, const QByteArray &name)
{
    const QByteArray lowerName = name.toLower() + ':';
    for (const QByteArray &line : headers) {
        if (line.toLower().startsWith(lowerName)) {
            return QString::fromUtf8(line.mid(line.indexOf(':') + 1).trimmed());
        }
    }
    return {};
}

QString scopeValue(const QStringList &scopes, const QString &prefix)
{
    for (const QString &scope : scopes) {
        if (scope.startsWith(prefix, Qt::CaseInsensitive)) {
            return QUrl::fromPercentEncoding(scope.mid(prefix.size()).toUtf8());
        }
    }
    return {};
}

bool containsOpenIpcMarker(const QString &value)
{
    return value.contains(QStringLiteral("openipc"), Qt::CaseInsensitive)
        || value.contains(QStringLiteral("majestic"), Qt::CaseInsensitive);
}

} // namespace

NetworkDiscoveryService::NetworkDiscoveryService(QObject *parent)
    : QObject(parent)
    , m_deadlineTimer(new QTimer(this))
    , m_progressTimer(new QTimer(this))
{
    qRegisterMetaType<NetworkDiscoveryCandidate>();
    m_http.setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    m_deadlineTimer->setSingleShot(true);
    connect(m_deadlineTimer, &QTimer::timeout, this, [this]() { complete(false); });
    m_progressTimer->setInterval(160);
    connect(m_progressTimer, &QTimer::timeout, this, [this]() { updateProgress(); });
}

NetworkDiscoveryService::~NetworkDiscoveryService()
{
    stop();
}

QList<NetworkDiscoveryService::InterfaceTarget>
NetworkDiscoveryService::selectInterfaces(const QString &interfaceName) const
{
    QList<InterfaceTarget> targets;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        const auto flags = iface.flags();
        if (!flags.testFlag(QNetworkInterface::IsUp)
            || flags.testFlag(QNetworkInterface::IsLoopBack)) continue;
        if (!interfaceName.isEmpty() && iface.name() != interfaceName
            && iface.humanReadableName() != interfaceName) continue;
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol
                || entry.ip().isNull()) continue;
            targets.append({iface, entry});
        }
    }
    return targets;
}

void NetworkDiscoveryService::start(const QString &interfaceName, bool deepScan)
{
    stop();
    ++m_generation;
    m_running = true;
    m_progress = 0;
    m_totalJobs = 0;
    m_completedJobs = 0;
    m_deadlineMs = deepScan ? kDeepDeadlineMs : kQuickDeadlineMs;
    m_foundIps.clear();
    m_publishedEvidence.clear();
    m_elapsed.restart();
    emit runningChanged();
    emit progressChanged();
    emit foundCountChanged();

    const QList<InterfaceTarget> targets = selectInterfaces(interfaceName);
    if (targets.isEmpty()) {
        setPhase(QStringLiteral("No active IPv4 interface"));
        complete(false);
        return;
    }

    setPhase(QStringLiteral("mDNS + WS-Discovery"));
    m_progress = 3;
    emit progressChanged();
    startMulticastDiscovery(targets);
    enqueueSubnetSweep(targets, deepScan);
    m_progressTimer->start();
    m_deadlineTimer->start(m_deadlineMs);
}

void NetworkDiscoveryService::stop()
{
    if (!m_running && m_httpReplies.isEmpty() && m_rtspSockets.isEmpty()
        && m_udpSockets.isEmpty()) return;
    ++m_generation;
    m_deadlineTimer->stop();
    m_progressTimer->stop();
    for (QNetworkReply *reply : std::as_const(m_httpReplies)) {
        if (reply) reply->abort();
    }
    m_httpReplies.clear();
    for (QTcpSocket *socket : std::as_const(m_rtspSockets)) {
        if (socket) {
            socket->setProperty("discoveryDone", true);
            socket->abort();
            socket->deleteLater();
        }
    }
    m_rtspSockets.clear();
    for (const QPointer<QUdpSocket> &socket : std::as_const(m_udpSockets)) {
        if (socket) {
            socket->close();
            socket->deleteLater();
        }
    }
    m_udpSockets.clear();
    m_httpQueue.clear();
    m_rtspQueue.clear();
    m_activeHttp = 0;
    m_activeRtsp = 0;
    m_deadlineMs = 0;
    if (m_running) complete(true);
}

void NetworkDiscoveryService::startMulticastDiscovery(const QList<InterfaceTarget> &targets)
{
    auto *mdns = new QUdpSocket(this);
    if (mdns->bind(QHostAddress::AnyIPv4, 5353,
                   QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        connect(mdns, &QUdpSocket::readyRead, this, [this, mdns]() {
            while (mdns->hasPendingDatagrams()) {
                const QNetworkDatagram datagram = mdns->receiveDatagram();
                handleMdnsDatagram(datagram.data(), datagram.senderAddress());
            }
        });
        const QByteArray query = buildMdnsQueryForTest();
        for (const InterfaceTarget &target : targets) {
            mdns->joinMulticastGroup(QHostAddress(QStringLiteral("224.0.0.251")), target.networkInterface);
            mdns->setMulticastInterface(target.networkInterface);
            mdns->writeDatagram(query, QHostAddress(QStringLiteral("224.0.0.251")), 5353);
        }
        m_udpSockets.append(mdns);
        const quint64 generation = m_generation;
        QTimer::singleShot(900, this, [this, mdns, targets, query, generation]() {
            if (!m_running || generation != m_generation || !mdns) return;
            for (const InterfaceTarget &target : targets) {
                mdns->setMulticastInterface(target.networkInterface);
                mdns->writeDatagram(query, QHostAddress(QStringLiteral("224.0.0.251")), 5353);
            }
        });
    } else {
        mdns->deleteLater();
    }

    for (const InterfaceTarget &target : targets) {
        auto *socket = new QUdpSocket(this);
        if (!socket->bind(target.address.ip(), 0, QUdpSocket::ShareAddress)) {
            socket->deleteLater();
            continue;
        }
        socket->setMulticastInterface(target.networkInterface);
        connect(socket, &QUdpSocket::readyRead, this, [this, socket]() {
            while (socket->hasPendingDatagrams()) {
                const QNetworkDatagram datagram = socket->receiveDatagram();
                handleWsDatagram(datagram.data(), datagram.senderAddress());
            }
        });
        sendWsProbe(socket, true);
        sendWsProbe(socket, false);
        m_udpSockets.append(socket);
        const quint64 generation = m_generation;
        QTimer::singleShot(800, this, [this, socket, generation]() {
            if (!m_running || generation != m_generation || !socket) return;
            sendWsProbe(socket, true);
        });
    }
}

QByteArray NetworkDiscoveryService::buildWsProbe(bool typedProbe)
{
    const QByteArray types = typedProbe
        ? QByteArrayLiteral("<d:Types>dn:NetworkVideoTransmitter</d:Types>") : QByteArray{};
    return QByteArrayLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
        + QByteArrayLiteral("<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" ")
        + QByteArrayLiteral("xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" ")
        + QByteArrayLiteral("xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" ")
        + QByteArrayLiteral("xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">")
        + QByteArrayLiteral("<s:Header><a:Action s:mustUnderstand=\"1\">")
        + QByteArrayLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe")
        + QByteArrayLiteral("</a:Action><a:MessageID>urn:uuid:")
        + QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8()
        + QByteArrayLiteral("</a:MessageID><a:ReplyTo><a:Address>")
        + QByteArrayLiteral("http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous")
        + QByteArrayLiteral("</a:Address></a:ReplyTo><a:To s:mustUnderstand=\"1\">")
        + QByteArrayLiteral("urn:schemas-xmlsoap-org:ws:2005:04:discovery")
        + QByteArrayLiteral("</a:To></s:Header><s:Body><d:Probe>") + types
        + QByteArrayLiteral("</d:Probe></s:Body></s:Envelope>");
}

void NetworkDiscoveryService::sendWsProbe(QUdpSocket *socket, bool typedProbe)
{
    if (!socket) return;
    socket->writeDatagram(buildWsProbe(typedProbe),
                          QHostAddress(QStringLiteral("239.255.255.250")), 3702);
}

void NetworkDiscoveryService::handleWsDatagram(const QByteArray &data,
                                                const QHostAddress &sender)
{
    if (!m_running || sender.protocol() != QAbstractSocket::IPv4Protocol) return;
    QXmlStreamReader xml(data);
    QString scopesText;
    QString xaddrsText;
    QString typesText;
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        const QString name = xml.name().toString();
        if (name == QStringLiteral("Scopes")) scopesText = xml.readElementText();
        else if (name == QStringLiteral("XAddrs")) xaddrsText = xml.readElementText();
        else if (name == QStringLiteral("Types")) typesText = xml.readElementText();
    }
    if (xml.hasError() || (scopesText.isEmpty() && xaddrsText.isEmpty()
                           && !data.contains("ProbeMatch"))) return;

    NetworkDiscoveryCandidate candidate;
    candidate.ip = sender.toString();
    candidate.method = QStringLiteral("WS-Discovery");
    candidate.evidence = QStringLiteral("ONVIF ProbeMatch");
    candidate.onvif = true;
    candidate.confidence = 80;
    const QStringList scopes = scopesText.split(QRegularExpression(QStringLiteral("\\s+")),
                                                Qt::SkipEmptyParts);
    candidate.name = scopeValue(scopes, QStringLiteral("onvif://www.onvif.org/name/"));
    candidate.model = scopeValue(scopes, QStringLiteral("onvif://www.onvif.org/hardware/"));
    candidate.manufacturer = scopeValue(scopes, QStringLiteral("onvif://www.onvif.org/manufacturer/"));
    const QString firstXAddr = xaddrsText.split(QRegularExpression(QStringLiteral("\\s+")),
                                                Qt::SkipEmptyParts).value(0);
    const QUrl xaddr(firstXAddr);
    if (xaddr.isValid() && xaddr.port() > 0) {
        candidate.onvifPort = xaddr.port();
        candidate.httpPort = xaddr.port();
    }
    const QString markers = scopesText + QLatin1Char(' ') + typesText + QLatin1Char(' ')
        + candidate.name + QLatin1Char(' ') + candidate.model + QLatin1Char(' ')
        + candidate.manufacturer;
    if (containsOpenIpcMarker(markers)) {
        candidate.openIpc = true;
        candidate.manufacturer = QStringLiteral("OpenIPC");
        candidate.confidence = 95;
    }
    if (candidate.name.isEmpty()) candidate.name = candidate.model;
    if (candidate.name.isEmpty()) candidate.name = QStringLiteral("ONVIF Camera");
    publish(candidate);
}

QByteArray NetworkDiscoveryService::buildMdnsQueryForTest()
{
    const QStringList services{QStringLiteral("_http._tcp.local"),
                               QStringLiteral("_rtsp._tcp.local"),
                               QStringLiteral("_ssh._tcp.local")};
    QByteArray query(12, '\0');
    query[5] = static_cast<char>(services.size());
    for (const QString &service : services) {
        appendDnsName(&query, service);
        appendU16(&query, 12); // PTR
        appendU16(&query, 1);  // IN
    }
    return query;
}

QVariantList NetworkDiscoveryService::parseMdnsPacket(const QByteArray &packet,
                                                       const QHostAddress &senderAddress)
{
    QVariantList results;
    if (packet.size() < 12) return results;
    bool ok = false;
    const quint16 questions = readU16(packet, 4, &ok);
    if (!ok) return results;
    const int recordCount = readU16(packet, 6) + readU16(packet, 8) + readU16(packet, 10);
    qsizetype offset = 12;
    for (int i = 0; i < questions; ++i) {
        if (readDnsName(packet, &offset).isEmpty() || offset + 4 > packet.size()) return {};
        offset += 4;
    }

    struct Srv { int port = 0; QString target; };
    QHash<QString, QString> serviceByInstance;
    QHash<QString, Srv> srvByInstance;
    QHash<QString, QVariantMap> txtByInstance;
    QHash<QString, QString> addressByHost;

    for (int i = 0; i < recordCount; ++i) {
        const QString name = readDnsName(packet, &offset).toLower();
        if (name.isEmpty() || offset + 10 > packet.size()) break;
        const quint16 type = readU16(packet, offset);
        const quint16 dataLength = readU16(packet, offset + 8);
        offset += 10;
        const qsizetype dataStart = offset;
        if (dataStart + dataLength > packet.size()) break;

        if (type == 12) { // PTR
            qsizetype pointerOffset = dataStart;
            const QString instance = readDnsName(packet, &pointerOffset).toLower();
            if (!instance.isEmpty()) serviceByInstance.insert(instance, name);
        } else if (type == 33 && dataLength >= 6) { // SRV
            qsizetype targetOffset = dataStart + 6;
            Srv srv;
            srv.port = readU16(packet, dataStart + 4);
            srv.target = readDnsName(packet, &targetOffset).toLower();
            srvByInstance.insert(name, srv);
        } else if (type == 16) { // TXT
            QVariantMap txt;
            qsizetype cursor = dataStart;
            while (cursor < dataStart + dataLength) {
                const quint8 length = static_cast<quint8>(packet.at(cursor++));
                if (cursor + length > dataStart + dataLength) break;
                const QString entry = QString::fromUtf8(packet.mid(cursor, length));
                cursor += length;
                const int equals = entry.indexOf(QLatin1Char('='));
                txt.insert(equals > 0 ? entry.left(equals).toLower() : entry.toLower(),
                           equals > 0 ? entry.mid(equals + 1) : QStringLiteral("true"));
            }
            txtByInstance.insert(name, txt);
        } else if (type == 1 && dataLength == 4) { // A
            addressByHost.insert(name, QHostAddress(readU32(packet, dataStart)).toString());
        }
        offset = dataStart + dataLength;
    }

    QSet<QString> instances;
    for (auto it = serviceByInstance.cbegin(); it != serviceByInstance.cend(); ++it) instances.insert(it.key());
    for (auto it = txtByInstance.cbegin(); it != txtByInstance.cend(); ++it) instances.insert(it.key());
    for (const QString &instance : std::as_const(instances)) {
        const QVariantMap txt = txtByInstance.value(instance);
        if (txt.value(QStringLiteral("vendor")).toString().compare(
                QStringLiteral("OpenIPC"), Qt::CaseInsensitive) != 0) continue;
        const QString service = serviceByInstance.value(instance);
        const Srv srv = srvByInstance.value(instance);
        QVariantMap result;
        QString ip = addressByHost.value(srv.target);
        if (ip.isEmpty() && senderAddress.protocol() == QAbstractSocket::IPv4Protocol)
            ip = senderAddress.toString();
        result.insert(QStringLiteral("ip"), ip);
        result.insert(QStringLiteral("instance"), instance);
        result.insert(QStringLiteral("service"), service);
        result.insert(QStringLiteral("target"), srv.target);
        result.insert(QStringLiteral("port"), srv.port);
        result.insert(QStringLiteral("txt"), txt);
        results.append(result);
    }
    return results;
}

void NetworkDiscoveryService::handleMdnsDatagram(const QByteArray &data,
                                                  const QHostAddress &sender)
{
    if (!m_running) return;
    const QVariantList announcements = parseMdnsPacket(data, sender);
    for (const QVariant &value : announcements) {
        const QVariantMap announcement = value.toMap();
        const QString ip = announcement.value(QStringLiteral("ip")).toString();
        if (ip.isEmpty()) continue;
        const QString service = announcement.value(QStringLiteral("service")).toString();
        const QVariantMap txt = announcement.value(QStringLiteral("txt")).toMap();
        NetworkDiscoveryCandidate candidate;
        candidate.ip = ip;
        candidate.name = announcement.value(QStringLiteral("target")).toString();
        if (candidate.name.endsWith(QStringLiteral(".local"), Qt::CaseInsensitive))
            candidate.name.chop(6);
        candidate.manufacturer = QStringLiteral("OpenIPC");
        candidate.model = txt.value(QStringLiteral("model")).toString();
        candidate.method = QStringLiteral("mDNS");
        candidate.evidence = service + QStringLiteral(" TXT vendor=OpenIPC");
        candidate.openIpc = true;
        candidate.confidence = 100;
        const int port = announcement.value(QStringLiteral("port")).toInt();
        if (service.startsWith(QStringLiteral("_rtsp"))) {
            candidate.rtsp = true;
            candidate.rtspPort = port > 0 ? port : 554;
        } else if (service.startsWith(QStringLiteral("_http"))) {
            candidate.httpPort = port > 0 ? port : 80;
            candidate.onvifPort = candidate.httpPort;
        }
        publish(candidate);
    }
}

QStringList NetworkDiscoveryService::subnetHosts(const QHostAddress &address, int prefixLength,
                                                  bool deepScan)
{
    QStringList hosts;
    bool ok = false;
    const quint32 local = address.toIPv4Address(&ok);
    if (!ok || prefixLength < 0 || prefixLength > 32) return hosts;
    const int effectivePrefix = deepScan ? std::max(prefixLength, 20)
                                         : std::max(prefixLength, 24);
    if (effectivePrefix >= 31) return hosts;
    const quint32 mask = effectivePrefix == 0 ? 0 : 0xffffffffu << (32 - effectivePrefix);
    const quint32 network = local & mask;
    const quint32 broadcast = network | ~mask;
    hosts.reserve(static_cast<int>(broadcast - network - 2));
    for (quint32 value = network + 1; value < broadcast; ++value) {
        if (value != local) hosts.append(QHostAddress(value).toString());
    }
    return hosts;
}

void NetworkDiscoveryService::enqueueSubnetSweep(const QList<InterfaceTarget> &targets,
                                                  bool deepScan)
{
    QSet<QString> hosts;
    for (const InterfaceTarget &target : targets) {
        const QStringList subnet = subnetHosts(target.address.ip(), target.address.prefixLength(),
                                               deepScan);
        for (const QString &host : subnet) hosts.insert(host);
    }
    QStringList sortedHosts = hosts.values();
    std::sort(sortedHosts.begin(), sortedHosts.end(), [](const QString &left, const QString &right) {
        return QHostAddress(left).toIPv4Address() < QHostAddress(right).toIPv4Address();
    });
    for (const QString &host : std::as_const(sortedHosts)) {
        m_httpQueue.enqueue({host, 80, QStringLiteral("/api/v1/config.json")});
        m_httpQueue.enqueue({host, 85, QStringLiteral("/")});
        m_rtspQueue.enqueue(host);
    }
    m_totalJobs = m_httpQueue.size() + m_rtspQueue.size();
    setPhase(QStringLiteral("Majestic + RTSP subnet probe"));
    updateProgress();
    pumpHttpJobs();
    pumpRtspJobs();
}

void NetworkDiscoveryService::pumpHttpJobs()
{
    while (m_running && m_activeHttp < kMaximumParallelHttp && !m_httpQueue.isEmpty()) {
        const HttpJob job = m_httpQueue.dequeue();
        QUrl url;
        url.setScheme(QStringLiteral("http"));
        url.setHost(job.ip);
        url.setPort(job.port);
        url.setPath(job.path);
        QNetworkRequest request(url);
        request.setTransferTimeout(kHttpProbeTimeoutMs);
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);
        request.setRawHeader("User-Agent", "OpenIPC-Dashboard-Discovery/0.2");
        QNetworkReply *reply = m_http.get(request);
        reply->setProperty("discoveryIp", job.ip);
        reply->setProperty("discoveryPort", job.port);
        reply->setProperty("discoveryPath", job.path);
        reply->setProperty("discoveryGeneration", QVariant::fromValue<qulonglong>(m_generation));
        m_httpReplies.insert(reply);
        ++m_activeHttp;
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            handleHttpReply(reply);
        });
    }
}

void NetworkDiscoveryService::handleHttpReply(QNetworkReply *reply)
{
    if (!reply || !m_httpReplies.remove(reply)) return;
    --m_activeHttp;
    ++m_completedJobs;
    const bool current = reply->property("discoveryGeneration").toULongLong() == m_generation
        && m_running;
    const QString ip = reply->property("discoveryIp").toString();
    const int port = reply->property("discoveryPort").toInt();
    const QString path = reply->property("discoveryPath").toString();
    const QByteArray body = reply->readAll();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (current && status > 0) {
        const auto headers = reply->rawHeaderPairs();
        QStringList headerLines;
        for (const auto &header : headers) {
            headerLines.append(QString::fromLatin1(header.first) + QStringLiteral(": ")
                               + QString::fromLatin1(header.second));
        }
        const QString markers = QString::fromUtf8(body.left(65536)) + QLatin1Char(' ')
            + headerLines.join(QLatin1Char(' '));
        bool majesticJson = false;
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        if (parseError.error == QJsonParseError::NoError && document.isObject()) {
            const QJsonObject object = document.object();
            majesticJson = object.contains(QStringLiteral("video0"))
                || object.contains(QStringLiteral("isp"))
                || (object.contains(QStringLiteral("system"))
                    && object.contains(QStringLiteral("rtsp")));
        }
        if (majesticJson || containsOpenIpcMarker(markers)) {
            NetworkDiscoveryCandidate candidate;
            candidate.ip = ip;
            candidate.name = QStringLiteral("OpenIPC Camera");
            candidate.manufacturer = QStringLiteral("OpenIPC");
            candidate.method = port == 80 ? QStringLiteral("Majestic HTTP")
                                          : QStringLiteral("OpenIPC WebUI");
            candidate.evidence = majesticJson ? path + QStringLiteral(" returned Majestic JSON")
                                              : QStringLiteral("OpenIPC/Majestic HTTP marker");
            candidate.httpPort = port == 85 ? 80 : port;
            candidate.onvifPort = candidate.httpPort;
            candidate.openIpc = true;
            candidate.majestic = majesticJson || markers.contains(QStringLiteral("majestic"),
                                                                   Qt::CaseInsensitive);
            candidate.confidence = majesticJson ? 100 : 95;
            publish(candidate);
        }
    }
    reply->deleteLater();
    updateProgress();
    pumpHttpJobs();
}

void NetworkDiscoveryService::pumpRtspJobs()
{
    while (m_running && m_activeRtsp < kMaximumParallelRtsp && !m_rtspQueue.isEmpty()) {
        const QString ip = m_rtspQueue.dequeue();
        auto *socket = new QTcpSocket(this);
        socket->setProperty("discoveryGeneration", QVariant::fromValue<qulonglong>(m_generation));
        socket->setProperty("discoveryDone", false);
        m_rtspSockets.insert(socket);
        ++m_activeRtsp;
        connect(socket, &QTcpSocket::connected, socket, [socket, ip]() {
            const QByteArray request = QByteArrayLiteral("OPTIONS rtsp://") + ip.toUtf8()
                + QByteArrayLiteral("/ RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: OpenIPC-Dashboard-Discovery/0.2\r\n\r\n");
            socket->write(request);
        });
        connect(socket, &QTcpSocket::readyRead, this, [this, socket, ip]() {
            socket->setProperty("rtspResponse", socket->property("rtspResponse").toByteArray()
                                + socket->readAll());
            if (socket->property("rtspResponse").toByteArray().contains("\r\n\r\n"))
                finishRtspSocket(socket, ip, true);
        });
        connect(socket, &QTcpSocket::errorOccurred, this, [this, socket, ip](QAbstractSocket::SocketError) {
            finishRtspSocket(socket, ip, false);
        });
        QTimer::singleShot(kRtspProbeTimeoutMs, socket, [this, socket, ip]() {
            finishRtspSocket(socket, ip, !socket->property("rtspResponse").toByteArray().isEmpty());
        });
        socket->connectToHost(ip, 554);
    }
}

void NetworkDiscoveryService::finishRtspSocket(QTcpSocket *socket, const QString &ip,
                                                bool inspectResponse)
{
    if (!socket || socket->property("discoveryDone").toBool()) return;
    socket->setProperty("discoveryDone", true);
    const bool current = socket->property("discoveryGeneration").toULongLong() == m_generation
        && m_running;
    const QByteArray response = socket->property("rtspResponse").toByteArray();
    if (current && inspectResponse && response.startsWith("RTSP/")) {
        const QList<QByteArray> lines = response.left(response.indexOf("\r\n\r\n")).split('\n');
        const QString server = headerValue(lines, QByteArrayLiteral("server"));
        const QString realm = headerValue(lines, QByteArrayLiteral("www-authenticate"));
        NetworkDiscoveryCandidate candidate;
        candidate.ip = ip;
        candidate.name = containsOpenIpcMarker(server + QLatin1Char(' ') + realm)
            ? QStringLiteral("OpenIPC Camera") : QStringLiteral("RTSP Camera");
        candidate.manufacturer = containsOpenIpcMarker(server + QLatin1Char(' ') + realm)
            ? QStringLiteral("OpenIPC") : server;
        candidate.method = QStringLiteral("RTSP probe");
        candidate.evidence = server.isEmpty() ? QStringLiteral("RTSP response")
                                              : QStringLiteral("RTSP Server: ") + server;
        candidate.rtsp = true;
        candidate.openIpc = containsOpenIpcMarker(server + QLatin1Char(' ') + realm);
        candidate.majestic = containsOpenIpcMarker(server);
        candidate.confidence = candidate.openIpc ? 95 : 55;
        publish(candidate);
    }
    if (m_rtspSockets.remove(socket)) {
        --m_activeRtsp;
        ++m_completedJobs;
    }
    socket->abort();
    socket->deleteLater();
    updateProgress();
    pumpRtspJobs();
}

void NetworkDiscoveryService::publish(const NetworkDiscoveryCandidate &candidate)
{
    if (!m_running || candidate.ip.isEmpty()) return;
    const QString evidenceKey = candidate.ip + QLatin1Char('|') + candidate.method
        + QLatin1Char('|') + candidate.evidence;
    if (m_publishedEvidence.contains(evidenceKey)) return;
    m_publishedEvidence.insert(evidenceKey);
    const bool newIp = !m_foundIps.contains(candidate.ip);
    m_foundIps.insert(candidate.ip);
    emit candidateFound(candidate);
    if (newIp) emit foundCountChanged();
}

void NetworkDiscoveryService::updateProgress()
{
    if (!m_running) return;

    int next = 3;
    if (m_totalJobs > 0) {
        const double completedRatio = std::clamp(
            static_cast<double>(m_completedJobs) / static_cast<double>(m_totalJobs),
            0.0, 1.0);
        const double elapsedRatio = m_deadlineMs > 0
            ? std::clamp(static_cast<double>(m_elapsed.elapsed()) / static_cast<double>(m_deadlineMs),
                         0.0, 1.0)
            : 0.0;

        // Network scans naturally arrive in bursts. Blend actual job completion
        // with the bounded scan deadline so the UI still advances smoothly while
        // slow sockets wait for their timeout, without ever claiming 100% early.
        const int completedProgress = 8 + static_cast<int>(completedRatio * 90.0);
        const int elapsedProgress = 3 + static_cast<int>(elapsedRatio * 92.0);
        next = std::clamp(std::max(completedProgress, elapsedProgress), 3, 99);
    }

    if (m_httpQueue.isEmpty() && m_rtspQueue.isEmpty()
        && m_activeHttp == 0 && m_activeRtsp == 0
        && m_elapsed.elapsed() >= kMinimumMulticastWindowMs) {
        complete(false);
        return;
    }

    if (next > m_progress) {
        m_progress = next;
        emit progressChanged();
    }
}

void NetworkDiscoveryService::setPhase(const QString &phase)
{
    if (m_phase == phase) return;
    m_phase = phase;
    emit phaseChanged();
}

void NetworkDiscoveryService::complete(bool cancelled)
{
    if (!m_running) return;
    m_running = false;
    m_deadlineTimer->stop();
    m_progressTimer->stop();
    if (!cancelled) {
        m_progress = 100;
        setPhase(QStringLiteral("Finished"));
        emit progressChanged();
    } else {
        setPhase(QStringLiteral("Cancelled"));
    }
    emit runningChanged();
    emit finished(cancelled);

    for (QNetworkReply *reply : std::as_const(m_httpReplies)) if (reply) reply->abort();
    for (QTcpSocket *socket : std::as_const(m_rtspSockets)) if (socket) socket->abort();
    for (const QPointer<QUdpSocket> &socket : std::as_const(m_udpSockets)) if (socket) socket->close();
}

QVariantList NetworkDiscoveryService::parseMdnsPacketForTest(const QByteArray &packet,
                                                              const QString &senderAddress)
{
    return parseMdnsPacket(packet, QHostAddress(senderAddress));
}

QStringList NetworkDiscoveryService::subnetHostsForTest(const QString &address, int prefixLength,
                                                        bool deepScan)
{
    return subnetHosts(QHostAddress(address), prefixLength, deepScan);
}
