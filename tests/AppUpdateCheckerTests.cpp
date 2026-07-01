#include "AppUpdateChecker.h"

#include <QTest>

class AppUpdateCheckerTests : public QObject
{
    Q_OBJECT

private slots:
    void comparesSemanticVersions();
};

void AppUpdateCheckerTests::comparesSemanticVersions()
{
    QVERIFY(AppUpdateChecker::isVersionNewer(QStringLiteral("v0.2.5-pre.1"),
                                             QStringLiteral("0.2.4")));
    QVERIFY(!AppUpdateChecker::isVersionNewer(QStringLiteral("0.2.5-pre.1"),
                                              QStringLiteral("0.2.5")));
    QVERIFY(AppUpdateChecker::isVersionNewer(QStringLiteral("0.2.5"),
                                             QStringLiteral("0.2.5-rc.2")));
    QVERIFY(AppUpdateChecker::isVersionNewer(QStringLiteral("0.2.5-pre.2"),
                                             QStringLiteral("0.2.5-pre.1")));
    QVERIFY(AppUpdateChecker::isVersionNewer(QStringLiteral("0.3.0"),
                                             QStringLiteral("0.2.9")));
    QCOMPARE(AppUpdateChecker::compareVersions(QStringLiteral("v0.2.5+build.7"),
                                               QStringLiteral("0.2.5")), 0);
}

QTEST_MAIN(AppUpdateCheckerTests)
#include "AppUpdateCheckerTests.moc"
