#include <QtTest>

#include "DashboardWebSessionStore.h"

class DashboardWebSessionStoreTests : public QObject
{
    Q_OBJECT

private slots:
    void createsFindsAndRemovesOpaqueSessions();
    void clearInvalidatesEverySession();
    void listsAndRevokesSessionsWithoutExposingTokens();
};

void DashboardWebSessionStoreTests::createsFindsAndRemovesOpaqueSessions()
{
    DashboardWebSessionStore store;
    QSignalSpy countSpy(&store, &DashboardWebSessionStore::countChanged);
    const QVariantMap user{{QStringLiteral("username"), QStringLiteral("viewer")},
                           {QStringLiteral("role"), QStringLiteral("operator")},
                           {QStringLiteral("permissions"), 3}};
    const QByteArray first = store.create(user);
    const QByteArray second = store.create(user);

    QCOMPARE(first.size(), 43);
    QVERIFY(first != second);
    QCOMPARE(store.count(), 2);
    const auto session = store.find(first);
    QVERIFY(session.isValid());
    QCOMPARE(session.username, QStringLiteral("viewer"));
    QCOMPARE(session.permissions, 3);
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

QTEST_MAIN(DashboardWebSessionStoreTests)
#include "DashboardWebSessionStoreTests.moc"
