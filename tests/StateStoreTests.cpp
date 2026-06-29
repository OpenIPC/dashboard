#include <QtTest>

#include "StateStore.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>

class StateStoreTests : public QObject
{
    Q_OBJECT

private slots:
    void savesAndUpdatesStateAtomically();
    void createsVersionedSchema();
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

QTEST_GUILESS_MAIN(StateStoreTests)

#include "StateStoreTests.moc"
