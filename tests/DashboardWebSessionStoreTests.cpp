#include <QtTest>

#include "DashboardWebSessionStore.h"

class DashboardWebSessionStoreTests : public QObject
{
    Q_OBJECT

private slots:
    void createsFindsAndRemovesOpaqueSessions();
    void clearInvalidatesEverySession();
    void listsAndRevokesSessionsWithoutExposingTokens();
    void reportsBoundedOperationalContextAndTtls();
};

void DashboardWebSessionStoreTests::createsFindsAndRemovesOpaqueSessions()
{
    DashboardWebSessionStore store;
    QSignalSpy countSpy(&store, &DashboardWebSessionStore::countChanged);
    const QVariantMap user{{QStringLiteral("username"), QStringLiteral("viewer")},
                           {QStringLiteral("role"), QStringLiteral("operator")},
                           {QStringLiteral("permissions"), 3},
                           {QStringLiteral("cameraScopes"),
                            QStringList{QStringLiteral("front-gate")}}};
    const QByteArray first = store.create(user);
    const QByteArray second = store.create(user);

    QCOMPARE(first.size(), 43);
    QVERIFY(first != second);
    QCOMPARE(store.count(), 2);
    const auto session = store.find(first);
    QVERIFY(session.isValid());
    QCOMPARE(session.username, QStringLiteral("viewer"));
    QCOMPARE(session.permissions, 3);
    QCOMPARE(session.cameraScopes, QStringList{QStringLiteral("front-gate")});
    QCOMPARE(session.toVariantMap().value(QStringLiteral("cameraScopes")).toStringList(),
             QStringList{QStringLiteral("front-gate")});
    QVERIFY(store.remove(first));
    QVERIFY(!store.find(first).isValid());
    QCOMPARE(store.count(), 1);
    QVERIFY(countSpy.count() >= 3);
}

void DashboardWebSessionStoreTests::clearInvalidatesEverySession()
{
    DashboardWebSessionStore store;
    const QByteArray token = store.create({
        {QStringLiteral("username"), QStringLiteral("admin")},
        {QStringLiteral("role"), QStringLiteral("admin")},
        {QStringLiteral("permissions"), 255}
    });
    store.clear();
    QCOMPARE(store.count(), 0);
    QVERIFY(!store.find(token).isValid());
}

void DashboardWebSessionStoreTests::listsAndRevokesSessionsWithoutExposingTokens()
{
    DashboardWebSessionStore store;
    const QVariantMap viewer{{QStringLiteral("username"), QStringLiteral("viewer")},
                             {QStringLiteral("role"), QStringLiteral("operator")},
                             {QStringLiteral("permissions"), 1}};
    const QByteArray current = store.create(viewer);
    store.create(viewer);
    store.create({{QStringLiteral("username"), QStringLiteral("admin")},
                  {QStringLiteral("role"), QStringLiteral("admin")},
                  {QStringLiteral("permissions"), 255}});

    const QVariantList sessions = store.sessions(current);
    QCOMPARE(sessions.size(), 3);
    int currentCount = 0;
    QString revokeId;
    for (const QVariant &value : sessions) {
        const QVariantMap item = value.toMap();
        QVERIFY(!item.contains(QStringLiteral("token")));
        QCOMPARE(item.value(QStringLiteral("id")).toString().size(), 64);
        if (item.value(QStringLiteral("current")).toBool()) ++currentCount;
        else if (item.value(QStringLiteral("username")).toString() == QStringLiteral("viewer"))
            revokeId = item.value(QStringLiteral("id")).toString();
    }
    QCOMPARE(currentCount, 1);
    QVERIFY(store.removeById(revokeId));
    QCOMPARE(store.removeForUser(QStringLiteral("viewer")), 1);
    QCOMPARE(store.count(), 1);
}

void DashboardWebSessionStoreTests::reportsBoundedOperationalContextAndTtls()
{
    DashboardWebSessionStore store;
    store.setTimeoutMinutes(15);
    const QByteArray token = store.create({
        {QStringLiteral("username"), QStringLiteral("operator")},
        {QStringLiteral("role"), QStringLiteral("operator")},
        {QStringLiteral("permissions"), 1}
    }, {
        {QStringLiteral("peerAddress"), QStringLiteral("127.0.0.1")},
        {QStringLiteral("origin"), QStringLiteral("https://dashboard.example")},
        {QStringLiteral("userAgent"), QString(700, QLatin1Char('x'))}
    });
    const QVariantMap session = store.find(token, false).toVariantMap();
    QCOMPARE(session.value(QStringLiteral("peerAddress")).toString(), QStringLiteral("127.0.0.1"));
    QCOMPARE(session.value(QStringLiteral("origin")).toString(),
             QStringLiteral("https://dashboard.example"));
    QCOMPARE(session.value(QStringLiteral("userAgent")).toString().size(), 512);
    QVERIFY(session.value(QStringLiteral("idleTtlSeconds")).toLongLong() <= 15 * 60);
    QVERIFY(session.value(QStringLiteral("idleTtlSeconds")).toLongLong() > 14 * 60);
    QVERIFY(session.value(QStringLiteral("absoluteTtlSeconds")).toLongLong() > 23 * 60 * 60);
}

QTEST_MAIN(DashboardWebSessionStoreTests)
#include "DashboardWebSessionStoreTests.moc"
