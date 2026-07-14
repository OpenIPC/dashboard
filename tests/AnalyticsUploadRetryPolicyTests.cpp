#include <QtTest>

#include "AnalyticsUploadRetryPolicy.h"

class AnalyticsUploadRetryPolicyTests : public QObject
{
    Q_OBJECT

private slots:
    void retriesOnlyRetryableFailuresWithinLimit();
    void delayUsesBoundedExponentialBackoff();
};

void AnalyticsUploadRetryPolicyTests::retriesOnlyRetryableFailuresWithinLimit()
{
    QVERIFY(AnalyticsUploadRetryPolicy::shouldRetry(1, true, 3));
    QVERIFY(AnalyticsUploadRetryPolicy::shouldRetry(2, true, 3));
    QVERIFY(!AnalyticsUploadRetryPolicy::shouldRetry(3, true, 3));
    QVERIFY(!AnalyticsUploadRetryPolicy::shouldRetry(1, false, 3));
}

void AnalyticsUploadRetryPolicyTests::delayUsesBoundedExponentialBackoff()
{
    QCOMPARE(AnalyticsUploadRetryPolicy::retryDelayMs(1), 1000);
    QCOMPARE(AnalyticsUploadRetryPolicy::retryDelayMs(2), 2000);
    QCOMPARE(AnalyticsUploadRetryPolicy::retryDelayMs(3), 4000);
    QCOMPARE(AnalyticsUploadRetryPolicy::retryDelayMs(20), 30000);
}

QTEST_GUILESS_MAIN(AnalyticsUploadRetryPolicyTests)

#include "AnalyticsUploadRetryPolicyTests.moc"
