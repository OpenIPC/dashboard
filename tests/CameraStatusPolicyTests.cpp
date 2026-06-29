#include <QtTest>

#include "CameraStatusPolicy.h"

class CameraStatusPolicyTests : public QObject
{
    Q_OBJECT

private slots:
    void streamOfflineOverridesEveryOtherStatus();
    void gridOfflineBeatsOptimisticOnline();
    void firstNonEmptyGridStatusBeatsModelFallback();
    void modelStatusIsUsedWhenGridIsSilent();
    void fallbackStatusIsUsedLast();
    void attentionIsRaisedForOfflineOrDetail();
    void attentionReasonPrefersDetailThenOfflineStatus();
    void searchTextCombinesStatusAndReasonWithoutDuplicates();
};

void CameraStatusPolicyTests::streamOfflineOverridesEveryOtherStatus()
{
    QCOMPARE(CameraStatusPolicy::effectiveStatus({QStringLiteral("Online")},
                                                 QStringLiteral("Online"),
                                                 QString(),
                                                 true),
             QStringLiteral("Offline"));
}

void CameraStatusPolicyTests::gridOfflineBeatsOptimisticOnline()
{
    QCOMPARE(CameraStatusPolicy::effectiveStatus({QStringLiteral("Online"), QStringLiteral("Offline")},
                                                 QStringLiteral("Online"),
                                                 QString(),
                                                 false),
             QStringLiteral("Offline"));
}

void CameraStatusPolicyTests::firstNonEmptyGridStatusBeatsModelFallback()
{
    QCOMPARE(CameraStatusPolicy::effectiveStatus({QString(), QStringLiteral("Online")},
                                                 QStringLiteral("Offline"),
                                                 QStringLiteral("Offline"),
                                                 false),
             QStringLiteral("Online"));
}

void CameraStatusPolicyTests::modelStatusIsUsedWhenGridIsSilent()
{
    QCOMPARE(CameraStatusPolicy::effectiveStatus({QString(), QStringLiteral("   ")},
                                                 QStringLiteral("Online"),
                                                 QStringLiteral("Offline"),
                                                 false),
             QStringLiteral("Online"));
}

void CameraStatusPolicyTests::fallbackStatusIsUsedLast()
{
    QCOMPARE(CameraStatusPolicy::effectiveStatus({},
                                                 QString(),
                                                 QStringLiteral("Offline"),
                                                 false),
             QStringLiteral("Offline"));
}

void CameraStatusPolicyTests::attentionIsRaisedForOfflineOrDetail()
{
    QVERIFY(!CameraStatusPolicy::needsAttention(QStringLiteral("Online"), QString()));
    QVERIFY(CameraStatusPolicy::needsAttention(QStringLiteral("Offline"), QString()));
    QVERIFY(CameraStatusPolicy::needsAttention(QStringLiteral("Online"), QStringLiteral("RTSP timeout")));
}

void CameraStatusPolicyTests::attentionReasonPrefersDetailThenOfflineStatus()
{
    QCOMPARE(CameraStatusPolicy::attentionReason(QStringLiteral("Online"), QString()), QString());
    QCOMPARE(CameraStatusPolicy::attentionReason(QStringLiteral("Offline"), QString()), QStringLiteral("Offline"));
    QCOMPARE(CameraStatusPolicy::attentionReason(QString(), QString()), QStringLiteral("Unknown"));
    QCOMPARE(CameraStatusPolicy::attentionReason(QStringLiteral("Offline"), QStringLiteral("  RTSP timeout  ")),
             QStringLiteral("RTSP timeout"));
}

void CameraStatusPolicyTests::searchTextCombinesStatusAndReasonWithoutDuplicates()
{
    QCOMPARE(CameraStatusPolicy::searchText(QStringLiteral("Online"), QString()), QStringLiteral("Online"));
    QCOMPARE(CameraStatusPolicy::searchText(QStringLiteral("Offline"), QString()), QStringLiteral("Offline"));
    QCOMPARE(CameraStatusPolicy::searchText(QStringLiteral("Online"), QStringLiteral("Auth error")),
             QStringLiteral("Online Auth error"));
}

QTEST_APPLESS_MAIN(CameraStatusPolicyTests)

#include "CameraStatusPolicyTests.moc"
