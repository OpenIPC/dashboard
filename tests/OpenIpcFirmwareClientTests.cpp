#include "OpenIpcFirmwareClient.h"

#include <QTest>

class OpenIpcFirmwareClientTests : public QObject
{
    Q_OBJECT

private slots:
    void parsesNetworkPage();
    void parsesTimePage();
    void parsesUpdatePage();
    void parsesPulseAndMetrics();
};

void OpenIpcFirmwareClientTests::parsesNetworkPage()
{
    const QString html = QStringLiteral(R"(
        <form>
          <input type="text" id="network_hostname" name="network_hostname" value="openipc-hi3516ev200">
          <select id="network_interface" name="network_interface">
            <option value="eth0" selected>eth0</option><option value="wlan0">wlan0</option>
          </select>
          <input type="hidden" id="network_dhcp-false" name="network_dhcp" value="false">
          <input type="checkbox" id="network_dhcp" name="network_dhcp" value="true" checked>
          <input type="text" id="network_address" name="network_address" value="192.168.0.219">
          <input type="text" id="network_netmask" name="network_netmask" value="255.255.255.0">
          <input type="text" id="network_gateway" name="network_gateway" value="192.168.0.1">
          <input type="text" id="network_nameserver" name="network_nameserver" value="1.1.1.1">
          <input type="password" id="network_wlan_password" name="network_wlan_password" value="secret">
          <input type="text" id="mac_address" name="mac_address" value="00:12:31:62:8d:a5">
        </form>
        <dl>
          <dt>Hostname</dt><dd>openipc-hi3516ev200</dd>
          <dt>Interface</dt><dd>eth0</dd>
          <dt>Mode</dt><dd>DHCP</dd>
          <dt>MAC</dt><dd class="text-break">00:12:31:62:8d:a5</dd>
        </dl>
    )");

    const QVariantMap network = OpenIpcFirmwareClient::parseNetworkPageForTest(html);
    QCOMPARE(network.value(QStringLiteral("hostname")).toString(), QStringLiteral("openipc-hi3516ev200"));
    QCOMPARE(network.value(QStringLiteral("interface")).toString(), QStringLiteral("eth0"));
    QCOMPARE(network.value(QStringLiteral("address")).toString(), QStringLiteral("192.168.0.219"));
    QCOMPARE(network.value(QStringLiteral("netmask")).toString(), QStringLiteral("255.255.255.0"));
    QCOMPARE(network.value(QStringLiteral("dhcp")).toBool(), true);
    QCOMPARE(network.value(QStringLiteral("macAddress")).toString(), QStringLiteral("00:12:31:62:8d:a5"));
}

void OpenIpcFirmwareClientTests::parsesTimePage()
{
    const QString html = QStringLiteral(R"(
        <input type="text" id="tz_name" name="tz_name" value="Asia/Vladivostok">
        <input type="text" id="tz_data" name="tz_data" value="VLAT-10">
        <input type="text" id="server_0" name="server_0" value="pool.ntp.org">
        <input type="text" id="server_1" name="server_1" value="time.cloudflare.com">
        <dl>
          <dt>Device time</dt><dd>2026-06-29 10:00:00</dd>
          <dt>Zone name</dt><dd>Asia/Vladivostok</dd>
          <dt>POSIX string</dt><dd>VLAT-10</dd>
          <dt>NTP servers</dt><dd>pool.ntp.org, time.cloudflare.com</dd>
        </dl>
    )");

    const QVariantMap time = OpenIpcFirmwareClient::parseTimePageForTest(html);
    QCOMPARE(time.value(QStringLiteral("zoneName")).toString(), QStringLiteral("Asia/Vladivostok"));
    QCOMPARE(time.value(QStringLiteral("zoneData")).toString(), QStringLiteral("VLAT-10"));
    QCOMPARE(time.value(QStringLiteral("servers")).toList().at(0).toString(), QStringLiteral("pool.ntp.org"));
    QCOMPARE(time.value(QStringLiteral("ntpSummary")).toString(),
             QStringLiteral("pool.ntp.org, time.cloudflare.com"));
}

void OpenIpcFirmwareClientTests::parsesUpdatePage()
{
    const QString html = QStringLiteral(R"(
        <dl>
          <dt>Installed</dt><dd>2.6.06.23-lite</dd>
          <dt>Latest on GitHub</dt><dd><span id="firmware-master-ver">26.06.29</span></dd>
          <dt>SoC</dt><dd>hi3516ev200 <span>(hi3516ev200)</span></dd>
          <dt>Flash</dt><dd>nor</dd>
        </dl>
        <button id="fw-install-github" type="button" class="btn btn-primary">Install</button>
    )");

    const QVariantMap update = OpenIpcFirmwareClient::parseUpdatePageForTest(html);
    QCOMPARE(update.value(QStringLiteral("installed")).toString(), QStringLiteral("2.6.06.23-lite"));
    QCOMPARE(update.value(QStringLiteral("latest")).toString(), QStringLiteral("26.06.29"));
    QCOMPARE(update.value(QStringLiteral("flash")).toString(), QStringLiteral("nor"));
    QCOMPARE(update.value(QStringLiteral("githubAvailable")).toBool(), true);
}

void OpenIpcFirmwareClientTests::parsesPulseAndMetrics()
{
    const QVariantMap pulse = OpenIpcFirmwareClient::parsePulseForTest(
        QByteArrayLiteral(R"({"soc_temp":"59°C","time_now":"1782700000","timezone":"Asia/Vladivostok","mem_used":"42","overlay_used":"17","uptime":"0d 1h 2m 3s","mj_uptime":"1h 1m"})"));
    QCOMPARE(pulse.value(QStringLiteral("soc_temp")).toString(), QStringLiteral("59°C"));
    QCOMPARE(pulse.value(QStringLiteral("timezone")).toString(), QStringLiteral("Asia/Vladivostok"));

    const QVariantMap metrics = OpenIpcFirmwareClient::metricsFromTextForTest(QStringLiteral(
        "node_memory_MemTotal_bytes 268435456\n"
        "node_hwmon_temp_celsius 58.5\n"
        "# comment\n"));
    QCOMPARE(metrics.value(QStringLiteral("node_memory_MemTotal_bytes")).toDouble(), 268435456.0);
    QCOMPARE(metrics.value(QStringLiteral("node_hwmon_temp_celsius")).toDouble(), 58.5);
}

QTEST_MAIN(OpenIpcFirmwareClientTests)
#include "OpenIpcFirmwareClientTests.moc"
