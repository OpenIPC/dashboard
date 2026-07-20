#include <QtTest>

#include "CameraModel.h"
#include "presentation/DashboardPresentation.h"

class DashboardPresentationTests : public QObject
{
    Q_OBJECT

private slots:
    void normalizesCameraStatus();
    void formatsValuesDeterministically();
    void cameraViewDoesNotExposeCredentials();
    void capabilitiesFollowPermissions();
    void settingsContractRejectsUnknownAndReadOnlyValues();
};

void DashboardPresentationTests::normalizesCameraStatus()
{
    DashboardPresentation presentation;
    QCOMPARE(presentation.cameraStatusCode(QStringLiteral("Online")), QStringLiteral("online"));
    QCOMPARE(presentation.cameraStatusCode(QStringLiteral("Онлайн")), QStringLiteral("online"));
    QCOMPARE(presentation.cameraStatusCode(QStringLiteral("Auth Required")),
             QStringLiteral("auth-required"));
    QCOMPARE(presentation.cameraStatusCode(QStringLiteral("Offline")), QStringLiteral("offline"));
}

void DashboardPresentationTests::formatsValuesDeterministically()
{
    DashboardPresentation presentation;
    QCOMPARE(presentation.formatBytes(0), QStringLiteral("0 B"));
    QCOMPARE(presentation.formatBytes(1536), QStringLiteral("1.5 KiB"));
    QCOMPARE(presentation.formatBitrate(2500000), QStringLiteral("2.5 Mbps"));
    QCOMPARE(presentation.formatDuration(3723000), QStringLiteral("1:02:03"));
}

void DashboardPresentationTests::cameraViewDoesNotExposeCredentials()
{
    Camera camera;
    camera.id = QStringLiteral("camera-id");
    camera.name = QStringLiteral("Front gate");
    camera.ip = QStringLiteral("192.0.2.10");
    camera.login = QStringLiteral("root");
    camera.password = QStringLiteral("secret");
    camera.streamUrl = QStringLiteral("rtsp://root:secret@192.0.2.10/stream=0");
    DashboardPresentation presentation;
    const QVariantMap view = presentation.cameraView(camera, 0);
    QVERIFY(!view.contains(QStringLiteral("login")));
    QVERIFY(!view.contains(QStringLiteral("password")));
    QVERIFY(!view.contains(QStringLiteral("streamUrl")));
    QCOMPARE(view.value(QStringLiteral("statusCode")).toString(), QStringLiteral("unknown"));
}

void DashboardPresentationTests::capabilitiesFollowPermissions()
{
    DashboardPresentation presentation;
    const QVariantMap view = presentation.capabilityManifest(0x01, true, true);
    const QVariantMap monitor = view.value(QStringLiteral("monitor")).toMap();
    const QVariantMap administration = view.value(QStringLiteral("administration")).toMap();
    QVERIFY(monitor.value(QStringLiteral("live")).toBool());
    QVERIFY(monitor.value(QStringLiteral("audio")).toBool());
    QVERIFY(!monitor.value(QStringLiteral("ptz")).toBool());
    QVERIFY(!administration.value(QStringLiteral("settings")).toBool());
}

void DashboardPresentationTests::settingsContractRejectsUnknownAndReadOnlyValues()
{
    DashboardPresentation presentation;
    QVariantMap normalized;
    QString error;
    QVERIFY(presentation.normalizeSettingsPatch(
        {{QStringLiteral("preferredStream"), QStringLiteral("hd")},
         {QStringLiteral("recordingSegmentDuration"), 20}}, &normalized, &error));
    QCOMPARE(normalized.value(QStringLiteral("preferredStream")).toString(), QStringLiteral("hd"));
    QCOMPARE(normalized.value(QStringLiteral("recordingSegmentDuration")).toInt(), 20);
    QVERIFY(!presentation.normalizeSettingsPatch(
        {{QStringLiteral("recordingsPath"), QStringLiteral("C:/secret")}}, &normalized, &error));
    QVERIFY(!presentation.normalizeSettingsPatch(
        {{QStringLiteral("webServerPort"), 9000}}, &normalized, &error));
}

QTEST_MAIN(DashboardPresentationTests)
#include "DashboardPresentationTests.moc"
