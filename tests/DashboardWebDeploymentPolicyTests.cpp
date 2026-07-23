#include <QtTest>

#include "DashboardWebDeploymentPolicy.h"

class DashboardWebDeploymentPolicyTests : public QObject
{
    Q_OBJECT

private slots:
    void migratesLegacyProfilesWithoutBroadeningAccess();
    void requiresExplicitSecureReverseProxyConfiguration();
    void acceptsExternalOriginOnlyFromTrustedProxy();
    void derivesPublicUrlsWithoutExposingProxyPeers();
};

void DashboardWebDeploymentPolicyTests::migratesLegacyProfilesWithoutBroadeningAccess()
{
    auto local = DashboardWebDeploymentPolicy::fromSettings({});
    QCOMPARE(local.profile, QStringLiteral("localhost"));
    QVERIFY(!local.allowRemote);

    auto lan = DashboardWebDeploymentPolicy::fromSettings({
        {QStringLiteral("webServerAllowRemote"), true},
        {QStringLiteral("webServerBindAddress"), QStringLiteral("0.0.0.0")}
    });
    QCOMPARE(lan.profile, QStringLiteral("lan"));
    QVERIFY(lan.allowRemote);
}

void DashboardWebDeploymentPolicyTests::requiresExplicitSecureReverseProxyConfiguration()
{
    auto missing = DashboardWebDeploymentPolicy::fromSettings({
        {QStringLiteral("webDeploymentProfile"), QStringLiteral("reverse_proxy")},
        {QStringLiteral("webExternalBaseUrl"), QStringLiteral("http://dashboard.example")}
    });
    QVERIFY(!missing.valid);
    QVERIFY(missing.externalBaseUrl.isEmpty());

    auto valid = DashboardWebDeploymentPolicy::fromSettings({
        {QStringLiteral("webDeploymentProfile"), QStringLiteral("reverse_proxy")},
        {QStringLiteral("webExternalBaseUrl"), QStringLiteral("https://dashboard.example")},
        {QStringLiteral("webTrustedProxyAddresses"), QStringLiteral("127.0.0.1, ::1")}
    });
    QVERIFY(valid.valid);
    QVERIFY(valid.secureCookies);
    QCOMPARE(valid.trustedProxyAddresses.size(), 2);
}

void DashboardWebDeploymentPolicyTests::acceptsExternalOriginOnlyFromTrustedProxy()
{
    const auto config = DashboardWebDeploymentPolicy::fromSettings({
        {QStringLiteral("webDeploymentProfile"), QStringLiteral("reverse_proxy")},
        {QStringLiteral("webExternalBaseUrl"), QStringLiteral("https://dashboard.example")},
        {QStringLiteral("webTrustedProxyAddresses"), QStringLiteral("127.0.0.1")}
    });
    QVERIFY(DashboardWebDeploymentPolicy::originAllowed(
        config, "https://dashboard.example", "127.0.0.1:8080", QStringLiteral("127.0.0.1")));
    QVERIFY(!DashboardWebDeploymentPolicy::originAllowed(
        config, "https://dashboard.example", "127.0.0.1:8080", QStringLiteral("192.168.1.20")));
    QVERIFY(!DashboardWebDeploymentPolicy::originAllowed(
        config, "https://evil.example", "127.0.0.1:8080", QStringLiteral("127.0.0.1")));
    QVERIFY(DashboardWebDeploymentPolicy::originAllowed(
        config, "http://127.0.0.1:8080", "127.0.0.1:8080", QStringLiteral("192.168.1.20")));
}

void DashboardWebDeploymentPolicyTests::derivesPublicUrlsWithoutExposingProxyPeers()
{
    const auto config = DashboardWebDeploymentPolicy::fromSettings({
        {QStringLiteral("webDeploymentProfile"), QStringLiteral("reverse_proxy")},
        {QStringLiteral("webExternalBaseUrl"), QStringLiteral("https://dashboard.example")},
        {QStringLiteral("webTrustedProxyAddresses"), QStringLiteral("10.0.0.2")}
    });
    QCOMPARE(DashboardWebDeploymentPolicy::publicHttpUrl(config, QStringLiteral("127.0.0.1"), 8080),
             QStringLiteral("https://dashboard.example/"));
    QCOMPARE(DashboardWebDeploymentPolicy::publicWebSocketUrl(
                 config, QStringLiteral("127.0.0.1"), 8081),
             QStringLiteral("wss://dashboard.example/ws"));
    const QVariantMap status = DashboardWebDeploymentPolicy::publicStatus(config);
    QVERIFY(!status.contains(QStringLiteral("trustedProxyAddresses")));
    QCOMPARE(status.value(QStringLiteral("trustedProxyCount")).toInt(), 1);
}

QTEST_GUILESS_MAIN(DashboardWebDeploymentPolicyTests)

#include "DashboardWebDeploymentPolicyTests.moc"
