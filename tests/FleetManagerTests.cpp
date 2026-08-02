#include <QtTest>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "CameraModel.h"
#include "FleetManager.h"
#include "LogModel.h"
#include "UserManager.h"

class FleetManagerTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void topologyScopesAndPersistence();
    void configurationDriftIsRedacted();
    void mutatingPreflightEnforcesSafetyGates();
    void siteExportNeverContainsCredentials();
    void siteScopedOperatorSeesOnlyAssignedFleet();

private:
    void addCamera(CameraModel &model) const;
    void loginAdmin(UserManager &users) const;

    QTemporaryDir m_runtimeRoot;
};

void FleetManagerTests::initTestCase()
{
    QVERIFY(m_runtimeRoot.isValid());
    QCoreApplication::setOrganizationName(QStringLiteral("OpenIPC"));
    QCoreApplication::setApplicationName(QStringLiteral("FleetManagerTests"));
    QStandardPaths::setTestModeEnabled(true);
    qputenv("OPENIPC_DATA_ROOT", m_runtimeRoot.path().toUtf8());
    qputenv("OPENIPC_TEST_SECRET_STORE", "settings");
}

void FleetManagerTests::init()
{
    QSettings settings;
    settings.clear();
    settings.sync();
    QDir(m_runtimeRoot.path() + QStringLiteral("/config")).removeRecursively();
    QDir(m_runtimeRoot.path() + QStringLiteral("/data")).removeRecursively();
}

void FleetManagerTests::addCamera(CameraModel &model) const
{
    Camera camera;
    camera.id = QStringLiteral("camera-front-gate");
    camera.name = QStringLiteral("Front gate");
    camera.ip = QStringLiteral("192.168.10.20");
    camera.status = QStringLiteral("Online");
    camera.manufacturer = QStringLiteral("OpenIPC");
    camera.isOpenIpc = true;
    camera.onboardingProfile = QStringLiteral("openipc");
    camera.login = QStringLiteral("root");
    camera.password = QStringLiteral("camera-secret");
    model.addCamera(camera);
}

void FleetManagerTests::loginAdmin(UserManager &users) const
{
    QVERIFY(users.setupInitialAdmin(QStringLiteral("admin"),
                                    QStringLiteral("AdminPass123!"), false));
    QVERIFY(users.isAdmin());
}

void FleetManagerTests::topologyScopesAndPersistence()
{
    CameraModel cameras;
    addCamera(cameras);
    UserManager users;
    loginAdmin(users);
    LogModel logs;
    FleetManager fleet(&cameras, nullptr, nullptr, nullptr, &users, &logs);

    const QString siteId = fleet.createSite(QStringLiteral("Vladivostok"));
    QVERIFY(!siteId.isEmpty());
    const QString areaId = fleet.createArea(siteId, QStringLiteral("Entrance"));
    QVERIFY(!areaId.isEmpty());
    QVERIFY(fleet.assignCamera(QStringLiteral("camera-front-gate"), siteId, areaId,
                               {QStringLiteral("perimeter"), QStringLiteral("critical")}));

    const QStringList aliases = fleet.scopeAliases(QStringLiteral("camera-front-gate"));
    QVERIFY(aliases.contains(QStringLiteral("site:") + siteId));
    QVERIFY(aliases.contains(QStringLiteral("area:") + areaId));
    QVERIFY(aliases.contains(QStringLiteral("tag:critical")));

    FleetManager restored(&cameras, nullptr, nullptr, nullptr, &users, &logs);
    restored.restoreJson(fleet.toJson());
    QCOMPARE(restored.sites().size(), 1);
    QCOMPARE(restored.areas().size(), 1);
    QCOMPARE(restored.cameraAssignment(QStringLiteral("camera-front-gate"))
                 .value(QStringLiteral("areaId")).toString(), areaId);
}

void FleetManagerTests::configurationDriftIsRedacted()
{
    CameraModel cameras;
    addCamera(cameras);
    UserManager users;
    loginAdmin(users);
    LogModel logs;
    FleetManager fleet(&cameras, nullptr, nullptr, nullptr, &users, &logs);

    const QString siteId = fleet.createSite(QStringLiteral("HQ"));
    QVERIFY(fleet.assignCamera(QStringLiteral("camera-front-gate"), siteId, QString()));
    fleet.setDeviceConfigurationSnapshot(QStringLiteral("camera-front-gate"), {
        {QStringLiteral("video"), QVariantMap{{QStringLiteral("bitrate"), 1024}}},
        {QStringLiteral("password"), QStringLiteral("current-secret")}
    });
    const QString baselineId = fleet.createBaseline(QStringLiteral("Outdoor"), siteId,
                                                     QString(), {
        {QStringLiteral("video"), QVariantMap{{QStringLiteral("bitrate"), 2048}}},
        {QStringLiteral("api_token"), QStringLiteral("baseline-secret")}
    });
    QVERIFY(!baselineId.isEmpty());

    const QVariantMap preview = fleet.driftPreview(baselineId,
                                                   {QStringLiteral("camera-front-gate")});
    QCOMPARE(preview.value(QStringLiteral("driftedCount")).toInt(), 1);
    QVERIFY(preview.value(QStringLiteral("redacted")).toBool());
    const QByteArray json = QJsonDocument::fromVariant(preview).toJson(QJsonDocument::Compact);
    QVERIFY(!json.contains("current-secret"));
    QVERIFY(!json.contains("baseline-secret"));
    QVERIFY(!json.contains("password"));
    QVERIFY(!json.contains("api_token"));
}

void FleetManagerTests::mutatingPreflightEnforcesSafetyGates()
{
    CameraModel cameras;
    addCamera(cameras);
    UserManager users;
    loginAdmin(users);
    LogModel logs;
    FleetManager fleet(&cameras, nullptr, nullptr, nullptr, &users, &logs);

    const QString siteId = fleet.createSite(QStringLiteral("Remote"));
    QVERIFY(fleet.assignCamera(QStringLiteral("camera-front-gate"), siteId, QString()));
    const QString baselineId = fleet.createBaseline(QStringLiteral("Safe"), siteId,
                                                     QString(), {{QStringLiteral("video.enabled"), true}});
    QVERIFY(!baselineId.isEmpty());

    const QVariantMap blocked = fleet.preflightBatch(QStringLiteral("apply-baseline"),
        {QStringLiteral("camera-front-gate")},
        {{QStringLiteral("dryRun"), false},
         {QStringLiteral("baselineId"), baselineId}});
    QVERIFY(!blocked.value(QStringLiteral("allowed")).toBool());
    QVERIFY(blocked.value(QStringLiteral("blockerCount")).toInt() >= 2);

    const QVariantMap dryRun = fleet.preflightBatch(QStringLiteral("apply-baseline"),
        {QStringLiteral("camera-front-gate")},
        {{QStringLiteral("dryRun"), true},
         {QStringLiteral("baselineId"), baselineId}});
    QVERIFY(dryRun.value(QStringLiteral("allowed")).toBool());
    QVERIFY(dryRun.value(QStringLiteral("warningCount")).toInt() >= 1);
}

void FleetManagerTests::siteExportNeverContainsCredentials()
{
    CameraModel cameras;
    addCamera(cameras);
    UserManager users;
    loginAdmin(users);
    LogModel logs;
    FleetManager fleet(&cameras, nullptr, nullptr, nullptr, &users, &logs);

    const QString siteId = fleet.createSite(QStringLiteral("Exportable"));
    QVERIFY(fleet.assignCamera(QStringLiteral("camera-front-gate"), siteId, QString()));
    QVERIFY(!fleet.createBaseline(QStringLiteral("Safe"), siteId, QString(), {
        {QStringLiteral("video"), QVariantMap{{QStringLiteral("bitrate"), 1500}}},
        {QStringLiteral("password"), QStringLiteral("must-not-leak")},
        {QStringLiteral("secret"), QStringLiteral("must-not-leak-either")}
    }).isEmpty());

    const QString path = m_runtimeRoot.path() + QStringLiteral("/sites.json");
    QVERIFY(fleet.exportSiteDefinitions(path));
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray data = file.readAll();
    QVERIFY(!data.contains("camera-secret"));
    QVERIFY(!data.contains("must-not-leak"));
    QVERIFY(!data.contains("\"password\""));
    QVERIFY(!data.contains("\"secret\""));
    QVERIFY(data.contains("\"credentialsIncluded\": false"));

    const QString unsafePath = m_runtimeRoot.path() + QStringLiteral("/unsafe-sites.json");
    QFile unsafeFile(unsafePath);
    QVERIFY(unsafeFile.open(QIODevice::WriteOnly));
    unsafeFile.write(QJsonDocument::fromVariant(QVariantMap{
        {QStringLiteral("format"), QStringLiteral("openipc-dashboard-sites")},
        {QStringLiteral("sites"), QVariantList{QVariantMap{
             {QStringLiteral("id"), QStringLiteral("unsafe")},
             {QStringLiteral("name"), QStringLiteral("Unsafe")},
             {QStringLiteral("apiToken"), QStringLiteral("should-be-rejected")}
         }}}
    }).toJson());
    unsafeFile.close();
    QVERIFY(fleet.previewSiteImport(unsafePath).value(QStringLiteral("containsCredentials")).toBool());
    QVERIFY(!fleet.importSiteDefinitions(unsafePath, true));
}

void FleetManagerTests::siteScopedOperatorSeesOnlyAssignedFleet()
{
    CameraModel cameras;
    addCamera(cameras);
    Camera west;
    west.id = QStringLiteral("camera-west");
    west.name = QStringLiteral("West office");
    west.ip = QStringLiteral("192.168.20.20");
    west.status = QStringLiteral("Online");
    west.manufacturer = QStringLiteral("OpenIPC");
    west.isOpenIpc = true;
    cameras.addCamera(west);

    UserManager users;
    loginAdmin(users);
    LogModel logs;
    FleetManager fleet(&cameras, nullptr, nullptr, nullptr, &users, &logs);
    users.setCameraScopeResolver(
        [&fleet](const QString &id, const QString &ip, int index) {
            return fleet.scopeAliases(id, ip, index);
        });

    const QString eastId = fleet.createSite(QStringLiteral("East"));
    const QString westId = fleet.createSite(QStringLiteral("West"));
    QVERIFY(fleet.assignCamera(QStringLiteral("camera-front-gate"), eastId, QString()));
    QVERIFY(fleet.assignCamera(QStringLiteral("camera-west"), westId, QString()));
    QVERIFY(!fleet.createBaseline(QStringLiteral("East baseline"), eastId, QString(),
                                  {{QStringLiteral("video.enabled"), true}}).isEmpty());
    QVERIFY(!fleet.createBaseline(QStringLiteral("West baseline"), westId, QString(),
                                  {{QStringLiteral("video.enabled"), true}}).isEmpty());
    QVERIFY(users.addUser(QStringLiteral("east-operator"),
                          QStringLiteral("OperatorPass123!"),
                          QStringLiteral("operator"),
                          int(UserManager::Perm_LiveView | UserManager::Perm_Settings
                              | UserManager::Perm_Export),
                          {QStringLiteral("site:") + eastId}));
    users.logout();
    QVERIFY(users.login(QStringLiteral("east-operator"),
                        QStringLiteral("OperatorPass123!"), false));

    QCOMPARE(fleet.inventory().size(), 1);
    QCOMPARE(fleet.inventory().first().toMap().value(QStringLiteral("cameraId")).toString(),
             QStringLiteral("camera-front-gate"));
    QCOMPARE(fleet.sites().size(), 1);
    QCOMPARE(fleet.sites().first().toMap().value(QStringLiteral("id")).toString(), eastId);
    QCOMPARE(fleet.baselines().size(), 1);
    QCOMPARE(fleet.baselines().first().toMap().value(QStringLiteral("siteId")).toString(), eastId);
    QVERIFY(fleet.driftPreview(fleet.toJson().value(QStringLiteral("baselines")).toArray()
                                  .at(0).toObject().value(QStringLiteral("id")).toString(), {}).isEmpty());
    QVERIFY(!fleet.exportSiteDefinitions(m_runtimeRoot.path() + QStringLiteral("/operator-sites.json")));
}

QTEST_GUILESS_MAIN(FleetManagerTests)

#include "FleetManagerTests.moc"
