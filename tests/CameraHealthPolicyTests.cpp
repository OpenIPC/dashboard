#include <QtTest>

#include "CameraHealthPolicy.h"

class CameraHealthPolicyTests : public QObject
{
    Q_OBJECT

private slots:
    void exposesFourDiagnosticProfiles();
    void buildsProfileSpecificProbePlans();
    void distinguishesRequiredAndOptionalFailures();
    void recommendsActionForCommonFailureCombinations();
    void reportContainsProbeEvidence();
};

void CameraHealthPolicyTests::exposesFourDiagnosticProfiles()
{
    const QVariantList profiles = CameraHealthPolicy::profiles();
    QCOMPARE(profiles.size(), 4);
    QCOMPARE(CameraHealthPolicy::normalizeProfile(QStringLiteral("DEEP")),
             QStringLiteral("deep"));
    QCOMPARE(CameraHealthPolicy::normalizeProfile(QStringLiteral("invalid")),
             QStringLiteral("quick"));
}

void CameraHealthPolicyTests::buildsProfileSpecificProbePlans()
{
    const QVariantList quick = CameraHealthPolicy::probePlan(QStringLiteral("quick"), true);
    QCOMPARE(quick.size(), 2);
    QCOMPARE(quick.constFirst().toMap().value(QStringLiteral("id")).toString(),
             QStringLiteral("rtsp-main"));

    const QVariantList rtspWithoutSub =
        CameraHealthPolicy::probePlan(QStringLiteral("rtsp"), false);
    QCOMPARE(rtspWithoutSub.size(), 2);

    const QVariantList deep =
        CameraHealthPolicy::probePlan(QStringLiteral("deep"), true);
    QVERIFY(deep.size() >= 9);
    QStringList ids;
    for (const QVariant &value : deep) {
        ids.append(value.toMap().value(QStringLiteral("id")).toString());
    }
    QVERIFY(ids.contains(QStringLiteral("rtsp-sub")));
    QVERIFY(ids.contains(QStringLiteral("majestic-schema")));
    QVERIFY(ids.contains(QStringLiteral("firmware-status")));
    QVERIFY(ids.contains(QStringLiteral("metrics")));
    QVERIFY(ids.contains(QStringLiteral("logs-readiness")));
    QVERIFY(ids.contains(QStringLiteral("snapshot")));
}

void CameraHealthPolicyTests::distinguishesRequiredAndOptionalFailures()
{
    const QVariantList warningOnly{
        QVariantMap{
            {QStringLiteral("status"), QStringLiteral("ok")},
            {QStringLiteral("required"), true}
        },
        QVariantMap{
            {QStringLiteral("status"), QStringLiteral("warning")},
            {QStringLiteral("required"), false}
        }
    };
    QCOMPARE(CameraHealthPolicy::overallStatus(warningOnly), QStringLiteral("warning"));

    QVariantList requiredFailure = warningOnly;
    requiredFailure.append(QVariantMap{
        {QStringLiteral("status"), QStringLiteral("error")},
        {QStringLiteral("required"), true}
    });
    QCOMPARE(CameraHealthPolicy::overallStatus(requiredFailure), QStringLiteral("error"));
}

void CameraHealthPolicyTests::recommendsActionForCommonFailureCombinations()
{
    const QVariantMap authResult{
        {QStringLiteral("probes"), QVariantList{
            QVariantMap{
                {QStringLiteral("id"), QStringLiteral("rtsp-main")},
                {QStringLiteral("status"), QStringLiteral("error")},
                {QStringLiteral("message"), QStringLiteral("RTSP authentication failed (401)")}
            }
        }}
    };
    QVERIFY(CameraHealthPolicy::recommendation(authResult).contains(
        QStringLiteral("username and password")));

    const QVariantMap splitResult{
        {QStringLiteral("probes"), QVariantList{
            QVariantMap{
                {QStringLiteral("id"), QStringLiteral("rtsp-main")},
                {QStringLiteral("status"), QStringLiteral("error")}
            },
            QVariantMap{
                {QStringLiteral("id"), QStringLiteral("firmware-status")},
                {QStringLiteral("status"), QStringLiteral("ok")}
            }
        }}
    };
    QVERIFY(CameraHealthPolicy::recommendation(splitResult).contains(
        QStringLiteral("WebUI responds")));
}

void CameraHealthPolicyTests::reportContainsProbeEvidence()
{
    const QVariantMap run{
        {QStringLiteral("id"), QStringLiteral("run-1")},
        {QStringLiteral("profileLabel"), QStringLiteral("Deep")},
        {QStringLiteral("startedAt"), QStringLiteral("2026-07-09T01:00:00Z")},
        {QStringLiteral("completedAt"), QStringLiteral("2026-07-09T01:00:02Z")},
        {QStringLiteral("summary"), QStringLiteral("1 camera")},
        {QStringLiteral("cameras"), QVariantList{
            QVariantMap{
                {QStringLiteral("name"), QStringLiteral("Front gate")},
                {QStringLiteral("ip"), QStringLiteral("192.168.1.10")},
                {QStringLiteral("status"), QStringLiteral("warning")},
                {QStringLiteral("inGrid"), true},
                {QStringLiteral("mainStreamUrl"), QStringLiteral("rtsp://192.168.1.10/stream=0")},
                {QStringLiteral("httpPort"), 80},
                {QStringLiteral("firmwareVersion"), QStringLiteral("2.4.1")},
                {QStringLiteral("majesticVersion"), QStringLiteral("2026.06")},
                {QStringLiteral("recommendation"), QStringLiteral("Review optional probes.")},
                {QStringLiteral("lastLogs"), QStringLiteral("majestic started")},
                {QStringLiteral("probes"), QVariantList{
                    QVariantMap{
                        {QStringLiteral("status"), QStringLiteral("ok")},
                        {QStringLiteral("label"), QStringLiteral("RTSP main")},
                        {QStringLiteral("elapsedMs"), 12},
                        {QStringLiteral("message"), QStringLiteral("RTSP endpoint responded")}
                    }
                }}
            }
        }}
    };

    const QString report = CameraHealthPolicy::reportText(run);
    QVERIFY(report.contains(QStringLiteral("Front gate")));
    QVERIFY(report.contains(QStringLiteral("Firmware: 2.4.1")));
    QVERIFY(report.contains(QStringLiteral("RTSP endpoint responded")));
    QVERIFY(report.contains(QStringLiteral("majestic started")));
}

QTEST_GUILESS_MAIN(CameraHealthPolicyTests)

#include "CameraHealthPolicyTests.moc"
