#include <QtTest>

#include "ReconnectPolicy.h"

class ReconnectPolicyTests : public QObject
{
    Q_OBJECT

private slots:
    void usesBoundedExponentialBackoff();
    void recognizesAuthenticationFailures();
    void keepsNetworkFailuresRetryable();
};

void ReconnectPolicyTests::usesBoundedExponentialBackoff()
{
    QCOMPARE(ReconnectPolicy::delayMs(-1), 1000);
    QCOMPARE(ReconnectPolicy::delayMs(0), 1000);
    QCOMPARE(ReconnectPolicy::delayMs(1), 2000);
    QCOMPARE(ReconnectPolicy::delayMs(2), 4000);
    QCOMPARE(ReconnectPolicy::delayMs(3), 8000);
    QCOMPARE(ReconnectPolicy::delayMs(4), 16000);
    QCOMPARE(ReconnectPolicy::delayMs(5), 30000);
    QCOMPARE(ReconnectPolicy::delayMs(50), 30000);
}

void ReconnectPolicyTests::recognizesAuthenticationFailures()
{
    QVERIFY(ReconnectPolicy::isAuthenticationError(QStringLiteral("Unauthorized (401)")));
    QVERIFY(ReconnectPolicy::isAuthenticationError(
        QStringLiteral("Could not open resource"), QStringLiteral("Authentication failed")));
}

void ReconnectPolicyTests::keepsNetworkFailuresRetryable()
{
    QVERIFY(!ReconnectPolicy::isAuthenticationError(QStringLiteral("Connection timed out")));
    QVERIFY(!ReconnectPolicy::isAuthenticationError(QStringLiteral("Host is unreachable")));
}

QTEST_APPLESS_MAIN(ReconnectPolicyTests)

#include "ReconnectPolicyTests.moc"
