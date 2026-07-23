#include <QtTest>

#include "StateStore.h"

#include <QJsonArray>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QTemporaryDir>

class StateStoreTests : public QObject
{
    Q_OBJECT

private slots:
    void savesAndUpdatesStateAtomically();
    void createsVersionedSchema();
    void opensPreviousReleaseState_data();
    void opensPreviousReleaseState();
};

void StateStoreTests::savesAndUpdatesStateAtomically()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    StateStore store(directory.filePath(QStringLiteral("state.sqlite3")));

    QString error;
    QJsonObject first{{QStringLiteral("gridRows"), 2}};
    QVERIFY2(store.save(first, &error), qPrintable(error));
    QCOMPARE(store.load(&error), std::optional<QJsonObject>(first));

    QJsonObject second{{QStringLiteral("gridRows"), 4}, {QStringLiteral("language"), QStringLiteral("ru")}};
    QVERIFY2(store.save(second, &error), qPrintable(error));
    QCOMPARE(store.load(&error), std::optional<QJsonObject>(second));
}

void StateStoreTests::createsVersionedSchema()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("state.sqlite3"));
    StateStore store(path);
    QString error;
    QVERIFY2(store.save(QJsonObject{}, &error), qPrintable(error));

    const QString connection = QStringLiteral("state-store-test-inspection");
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        database.setDatabaseName(path);
        QVERIFY(database.open());
        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral("SELECT MAX(version) FROM schema_migrations")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), StateStore::currentSchemaVersion());
        database.close();
    }
    QSqlDatabase::removeDatabase(connection);
}

void StateStoreTests::opensPreviousReleaseState_data()
{
    QTest::addColumn<QString>("sourceVersion");
    QTest::addColumn<QJsonObject>("payload");

    QTest::newRow("v0.2.6.1") << QStringLiteral("0.2.6.1") << QJsonObject{
        {QStringLiteral("appSettings"), QJsonObject{
             {QStringLiteral("language"), QStringLiteral("ru")},
             {QStringLiteral("recordingDirectory"), QStringLiteral("D:/OpenIPC/recordings")},
             {QStringLiteral("webServerAllowRemote"), false},
             {QStringLiteral("webServerPort"), 8080}}},
        {QStringLiteral("cameraGroups"), QJsonArray{QStringLiteral("Entrance")}},
        {QStringLiteral("cameras"), QJsonArray{QJsonObject{
             {QStringLiteral("id"), QStringLiteral("camera-1")},
             {QStringLiteral("ip"), QStringLiteral("192.0.2.10")},
             {QStringLiteral("credentialReference"), QStringLiteral("camera/192.0.2.10")}}}}
    };
    QTest::newRow("v0.2.7") << QStringLiteral("0.2.7") << QJsonObject{
        {QStringLiteral("appSettings"), QJsonObject{
             {QStringLiteral("language"), QStringLiteral("en")},
             {QStringLiteral("recordingDirectory"), QStringLiteral("/srv/openipc/archive")},
             {QStringLiteral("webServerAllowRemote"), true},
             {QStringLiteral("webServerBindAddress"), QStringLiteral("0.0.0.0")},
             {QStringLiteral("webServerPort"), 9080},
             {QStringLiteral("webSocketPort"), 9081}}},
        {QStringLiteral("layoutTemplates"), QJsonArray{QJsonObject{
             {QStringLiteral("name"), QStringLiteral("2x2")},
             {QStringLiteral("rows"), 2},
             {QStringLiteral("cols"), 2}}}}
    };
}

void StateStoreTests::opensPreviousReleaseState()
{
    QFETCH(QString, sourceVersion);
    QFETCH(QJsonObject, payload);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("state.sqlite3"));
    const QString connection = QStringLiteral("state-upgrade-%1").arg(sourceVersion);
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        database.setDatabaseName(path);
        QVERIFY(database.open());
        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral(
            "CREATE TABLE schema_migrations (version INTEGER PRIMARY KEY, applied_at TEXT NOT NULL)")));
        QVERIFY(query.exec(QStringLiteral(
            "INSERT INTO schema_migrations(version, applied_at) VALUES(1, '2026-01-01T00:00:00Z')")));
        QVERIFY(query.exec(QStringLiteral(
            "CREATE TABLE app_state (id INTEGER PRIMARY KEY CHECK(id = 1), schema_version INTEGER NOT NULL, "
            "payload_json TEXT NOT NULL, updated_at TEXT NOT NULL)")));
        query.prepare(QStringLiteral(
            "INSERT INTO app_state(id, schema_version, payload_json, updated_at) VALUES(1, 1, ?, ?)"));
        query.addBindValue(QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact)));
        query.addBindValue(QStringLiteral("2026-01-01T00:00:00Z"));
        QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
        database.close();
    }
    QSqlDatabase::removeDatabase(connection);

    StateStore store(path);
    QString error;
    QCOMPARE(store.load(&error), std::optional<QJsonObject>(payload));
    QVERIFY2(error.isEmpty(), qPrintable(error));

    payload.insert(QStringLiteral("upgradedFrom"), sourceVersion);
    QVERIFY2(store.save(payload, &error), qPrintable(error));
    QCOMPARE(store.load(&error), std::optional<QJsonObject>(payload));
}

QTEST_GUILESS_MAIN(StateStoreTests)

#include "StateStoreTests.moc"
