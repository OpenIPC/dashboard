#include "NetworkDiscoveryService.h"

#include <QTest>

namespace {

void appendU16(QByteArray *data, quint16 value)
{
    data->append(static_cast<char>((value >> 8) & 0xff));
    data->append(static_cast<char>(value & 0xff));
}

void appendU32(QByteArray *data, quint32 value)
{
    data->append(static_cast<char>((value >> 24) & 0xff));
    data->append(static_cast<char>((value >> 16) & 0xff));
    data->append(static_cast<char>((value >> 8) & 0xff));
    data->append(static_cast<char>(value & 0xff));
}

void appendName(QByteArray *data, const QByteArray &name)
{
    const QList<QByteArray> labels = name.split('.');
    for (const QByteArray &label : labels) {
        data->append(static_cast<char>(label.size()));
        data->append(label);
    }
    data->append('\0');
}

void appendRecord(QByteArray *packet, const QByteArray &name, quint16 type,
                  const QByteArray &recordData)
{
    appendName(packet, name);
    appendU16(packet, type);
    appendU16(packet, 1);
    appendU32(packet, 120);
    appendU16(packet, static_cast<quint16>(recordData.size()));
    packet->append(recordData);
}

QByteArray openIpcMdnsResponse()
{
    const QByteArray service = "_rtsp._tcp.local";
    const QByteArray instance = "camera._rtsp._tcp.local";
    const QByteArray host = "openipc.local";
    QByteArray packet(12, '\0');
    packet[2] = static_cast<char>(0x84);
    packet[7] = 4;

    QByteArray ptr;
    appendName(&ptr, instance);
    appendRecord(&packet, service, 12, ptr);

    QByteArray srv(4, '\0');
    appendU16(&srv, 554);
    appendName(&srv, host);
    appendRecord(&packet, instance, 33, srv);

    QByteArray txt;
    const QList<QByteArray> entries{"vendor=OpenIPC", "model=general", "path=/stream=0"};
    for (const QByteArray &entry : entries) {
        txt.append(static_cast<char>(entry.size()));
        txt.append(entry);
    }
    appendRecord(&packet, instance, 16, txt);

    QByteArray address;
    address.append(static_cast<char>(192));
    address.append(static_cast<char>(168));
    address.append(static_cast<char>(1));
    address.append(static_cast<char>(10));
    appendRecord(&packet, host, 1, address);
    return packet;
}

} // namespace

class NetworkDiscoveryServiceTests : public QObject
{
    Q_OBJECT

private slots:
    void buildsQueriesForOfficialOpenIpcServices();
    void recognizesOfficialOpenIpcMdnsMarker();
    void ignoresMdnsWithoutOpenIpcVendorMarker();
    void boundsQuickAndDeepSubnetSweeps();
};

void NetworkDiscoveryServiceTests::buildsQueriesForOfficialOpenIpcServices()
{
    const QByteArray query = NetworkDiscoveryService::buildMdnsQueryForTest();
    QVERIFY(query.size() > 12);
    QCOMPARE(static_cast<quint8>(query.at(5)), quint8(3));
    QVERIFY(query.contains("_http"));
    QVERIFY(query.contains("_rtsp"));
    QVERIFY(query.contains("_ssh"));
}

void NetworkDiscoveryServiceTests::recognizesOfficialOpenIpcMdnsMarker()
{
    const QVariantList results = NetworkDiscoveryService::parseMdnsPacketForTest(
        openIpcMdnsResponse(), QStringLiteral("192.168.1.10"));
    QCOMPARE(results.size(), 1);
    const QVariantMap result = results.constFirst().toMap();
    QCOMPARE(result.value(QStringLiteral("ip")).toString(), QStringLiteral("192.168.1.10"));
    QCOMPARE(result.value(QStringLiteral("service")).toString(),
             QStringLiteral("_rtsp._tcp.local"));
    QCOMPARE(result.value(QStringLiteral("port")).toInt(), 554);
    QCOMPARE(result.value(QStringLiteral("txt")).toMap()
                 .value(QStringLiteral("vendor")).toString(), QStringLiteral("OpenIPC"));
}

void NetworkDiscoveryServiceTests::ignoresMdnsWithoutOpenIpcVendorMarker()
{
    QByteArray packet = openIpcMdnsResponse();
    packet.replace("vendor=OpenIPC", "vendor=Generic");
    QVERIFY(NetworkDiscoveryService::parseMdnsPacketForTest(
                packet, QStringLiteral("192.168.1.10")).isEmpty());
}

void NetworkDiscoveryServiceTests::boundsQuickAndDeepSubnetSweeps()
{
    const QStringList quick = NetworkDiscoveryService::subnetHostsForTest(
        QStringLiteral("192.168.10.20"), 16, false);
    QCOMPARE(quick.size(), 253);
    QVERIFY(quick.contains(QStringLiteral("192.168.10.1")));
    QVERIFY(quick.contains(QStringLiteral("192.168.10.254")));
    QVERIFY(!quick.contains(QStringLiteral("192.168.10.20")));
    QVERIFY(!quick.contains(QStringLiteral("192.168.9.1")));

    const QStringList deep = NetworkDiscoveryService::subnetHostsForTest(
        QStringLiteral("192.168.10.20"), 16, true);
    QCOMPARE(deep.size(), 4093);
    QVERIFY(deep.contains(QStringLiteral("192.168.0.1")));
    QVERIFY(deep.contains(QStringLiteral("192.168.15.254")));
    QVERIFY(!deep.contains(QStringLiteral("192.168.16.1")));
}

QTEST_APPLESS_MAIN(NetworkDiscoveryServiceTests)

#include "NetworkDiscoveryServiceTests.moc"
