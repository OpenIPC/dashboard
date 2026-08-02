#include <QtTest>

#include "CameraModel.h"
#include "IncidentManager.h"

#include <QJsonObject>
#include <QSignalSpy>

class IncidentManagerTests : public QObject
{
    Q_OBJECT

private slots:
    void normalizesAndEnrichesEvents();
    void redactsSecretsAndDeduplicatesSourceEvents();
    void correlatesByCameraTypeAndTimeWindow();
    void adaptsHealthAnalyticsRecordingAndAudit();
    void persistsFiltersAndRestoresSafely();
};

void IncidentManagerTests::normalizesAndEnrichesEvents()
{
    CameraModel cameras;
    Camera camera;
    camera.id = QStringLiteral("camera-1");
    camera.ip = QStringLiteral("192.0.2.10");
    cameras.addCamera(camera);

    IncidentManager manager(&cameras);
    manager.setLocationResolver([](const QString &cameraId, const QString &) {
        return cameraId == QStringLiteral("camera-1")
            ? QVariantMap{{QStringLiteral("siteId"), QStringLiteral("site-a")},
                          {QStringLiteral("areaId"), QStringLiteral("area-1")}}
            : QVariantMap{};
    });

    QSignalSpy ingested(&manager, &IncidentManager::eventIngested);
    const QVariantMap event = manager.ingestEvent({
        {QStringLiteral("source"), QStringLiteral("Health")},
        {QStringLiteral("sourceEventId"), QStringLiteral("probe-1")},
        {QStringLiteral("type"), QStringLiteral("RTSP Failure")},
        {QStringLiteral("severity"), QStringLiteral("HIGH")},
        {QStringLiteral("occurredAtMs"), 1'700'000'000'000LL},
        {QStringLiteral("cameraIp"), camera.ip},
        {QStringLiteral("title"), QStringLiteral("Camera unavailable")},
        {QStringLiteral("message"), QStringLiteral("RTSP probe timed out")}
    });

    QCOMPARE(event.value(QStringLiteral("schemaVersion")).toInt(), 1);
    QCOMPARE(event.value(QStringLiteral("source")).toString(), QStringLiteral("health"));
    QCOMPARE(event.value(QStringLiteral("type")).toString(), QStringLiteral("rtsp-failure"));
    QCOMPARE(event.value(QStringLiteral("severity")).toString(), QStringLiteral("error"));
    QCOMPARE(event.value(QStringLiteral("cameraId")).toString(), camera.id);
    QCOMPARE(event.value(QStringLiteral("siteId")).toString(), QStringLiteral("site-a"));
    QCOMPARE(event.value(QStringLiteral("areaId")).toString(), QStringLiteral("area-1"));
    QVERIFY(!event.value(QStringLiteral("id")).toString().isEmpty());
    QVERIFY(!event.value(QStringLiteral("fingerprint")).toString().isEmpty());
    QCOMPARE(ingested.count(), 1);
}

void IncidentManagerTests::redactsSecretsAndDeduplicatesSourceEvents()
{
    IncidentManager manager(nullptr);
    const QVariantMap source{
        {QStringLiteral("source"), QStringLiteral("audit")},
        {QStringLiteral("sourceEventId"), QStringLiteral("audit-1")},
        {QStringLiteral("type"), QStringLiteral("configuration")},
        {QStringLiteral("attributes"), QVariantMap{
             {QStringLiteral("username"), QStringLiteral("operator")},
             {QStringLiteral("password"), QStringLiteral("must-not-leak")},
             {QStringLiteral("credentialHint"), QStringLiteral("must-also-not-leak")},
             {QStringLiteral("nested"), QVariantMap{
                  {QStringLiteral("api_token"), QStringLiteral("also-secret")},
                  {QStringLiteral("port"), 554}}}}}
    };

    const QVariantMap first = manager.ingestEvent(source);
    const QVariantMap duplicate = manager.ingestEvent(source);
    QCOMPARE(manager.events().size(), 1);
    QCOMPARE(duplicate.value(QStringLiteral("id")), first.value(QStringLiteral("id")));
    const QVariantMap attributes = first.value(QStringLiteral("attributes")).toMap();
    QCOMPARE(attributes.value(QStringLiteral("password")).toString(), QStringLiteral("[redacted]"));
    QCOMPARE(attributes.value(QStringLiteral("credentialHint")).toString(), QStringLiteral("[redacted]"));
    QCOMPARE(attributes.value(QStringLiteral("nested")).toMap()
                 .value(QStringLiteral("api_token")).toString(), QStringLiteral("[redacted]"));
    QCOMPARE(attributes.value(QStringLiteral("nested")).toMap()
                 .value(QStringLiteral("port")).toInt(), 554);
}

void IncidentManagerTests::correlatesByCameraTypeAndTimeWindow()
{
    IncidentManager manager(nullptr);
    const qint64 base = 1'700'000'100'000LL;
    auto event = [base](qint64 offset, const QString &severity) {
        return QVariantMap{
            {QStringLiteral("source"), QStringLiteral("health")},
            {QStringLiteral("sourceEventId"), QString::number(offset)},
            {QStringLiteral("category"), QStringLiteral("health")},
            {QStringLiteral("type"), QStringLiteral("offline")},
            {QStringLiteral("severity"), severity},
            {QStringLiteral("occurredAtMs"), base + offset},
            {QStringLiteral("cameraId"), QStringLiteral("camera-a")}
        };
    };

    manager.ingestEvent(event(0, QStringLiteral("warning")));
    manager.ingestEvent(event(30'000, QStringLiteral("error")));
    QCOMPARE(manager.correlations().size(), 1);
    QCOMPARE(manager.correlations().first().toMap().value(QStringLiteral("count")).toInt(), 2);
    QCOMPARE(manager.correlations().first().toMap().value(QStringLiteral("severity")).toString(),
             QStringLiteral("error"));

    manager.ingestEvent(event(360'000, QStringLiteral("warning")));
    QCOMPARE(manager.correlations().size(), 2);
}

void IncidentManagerTests::adaptsHealthAnalyticsRecordingAndAudit()
{
    IncidentManager manager(nullptr);
    const QVariantMap analytics = manager.ingestAnalyticsEvent({
        {QStringLiteral("id"), QStringLiteral("rule-1")},
        {QStringLiteral("eventType"), QStringLiteral("rule")},
        {QStringLiteral("ruleName"), QStringLiteral("Perimeter")},
        {QStringLiteral("cameraId"), QStringLiteral("camera-a")},
        {QStringLiteral("timestampMs"), 1'700'000'000'000LL},
        {QStringLiteral("snapshotPath"), QStringLiteral("C:/evidence/frame.jpg")},
        {QStringLiteral("actionNotify"), true}
    });
    QCOMPARE(analytics.value(QStringLiteral("source")).toString(), QStringLiteral("analytics"));
    QCOMPARE(analytics.value(QStringLiteral("severity")).toString(), QStringLiteral("warning"));
    QCOMPARE(analytics.value(QStringLiteral("evidence")).toList().size(), 1);

    const QVariantList health = manager.ingestHealthRun({
        {QStringLiteral("id"), QStringLiteral("health-1")},
        {QStringLiteral("profile"), QStringLiteral("quick")},
        {QStringLiteral("completedAt"), QStringLiteral("2026-08-02T06:00:00.000Z")},
        {QStringLiteral("cameras"), QVariantList{QVariantMap{
             {QStringLiteral("name"), QStringLiteral("Front gate")},
             {QStringLiteral("ip"), QStringLiteral("192.0.2.20")},
             {QStringLiteral("status"), QStringLiteral("error")},
             {QStringLiteral("recommendation"), QStringLiteral("Check network")}}}}
    });
    QCOMPARE(health.size(), 1);
    QCOMPARE(health.first().toMap().value(QStringLiteral("severity")).toString(), QStringLiteral("error"));

    const QVariantMap recording = manager.ingestRecordingEvent(
        QStringLiteral("error"), QStringLiteral("192.0.2.20"),
        QStringLiteral("C:/recordings/cam.mkv"), QStringLiteral("Disk full"));
    QCOMPARE(recording.value(QStringLiteral("category")).toString(), QStringLiteral("recording"));
    QCOMPARE(recording.value(QStringLiteral("severity")).toString(), QStringLiteral("error"));

    const QVariantMap audit = manager.ingestAuditEvent({
        {QStringLiteral("id"), QStringLiteral("audit-2")},
        {QStringLiteral("action"), QStringLiteral("fleet.batch.device")},
        {QStringLiteral("outcome"), QStringLiteral("failed")},
        {QStringLiteral("cameraId"), QStringLiteral("camera-a")},
        {QStringLiteral("actor"), QStringLiteral("admin")}
    });
    QCOMPARE(audit.value(QStringLiteral("source")).toString(), QStringLiteral("audit"));
    QCOMPARE(audit.value(QStringLiteral("severity")).toString(), QStringLiteral("error"));
}

void IncidentManagerTests::persistsFiltersAndRestoresSafely()
{
    IncidentManager manager(nullptr);
    manager.ingestEvent({
        {QStringLiteral("source"), QStringLiteral("health")},
        {QStringLiteral("sourceEventId"), QStringLiteral("health-a")},
        {QStringLiteral("type"), QStringLiteral("offline")},
        {QStringLiteral("severity"), QStringLiteral("warning")},
        {QStringLiteral("cameraId"), QStringLiteral("camera-a")},
        {QStringLiteral("title"), QStringLiteral("North entrance")}
    });
    manager.ingestEvent({
        {QStringLiteral("source"), QStringLiteral("recording")},
        {QStringLiteral("sourceEventId"), QStringLiteral("recording-b")},
        {QStringLiteral("type"), QStringLiteral("error")},
        {QStringLiteral("severity"), QStringLiteral("error")},
        {QStringLiteral("cameraId"), QStringLiteral("camera-b")},
        {QStringLiteral("title"), QStringLiteral("Storage failure")}
    });

    QCOMPARE(manager.filterEvents({{QStringLiteral("cameraId"), QStringLiteral("camera-a")}}).size(), 1);
    QCOMPARE(manager.filterEvents({{QStringLiteral("severity"), QStringLiteral("error")}}).size(), 1);
    QCOMPARE(manager.filterEvents({{QStringLiteral("search"), QStringLiteral("entrance")}}).size(), 1);

    const QVariantList originalEvents = manager.events();
    IncidentManager restored(nullptr);
    restored.restoreJson(manager.toJson());
    QCOMPARE(restored.events().size(), 2);
    QCOMPARE(restored.summary().value(QStringLiteral("eventCount")).toInt(), 2);
    QCOMPARE(restored.events().first().toMap().value(QStringLiteral("receivedAt")),
             originalEvents.first().toMap().value(QStringLiteral("receivedAt")));
    QVERIFY(!restored.eventById(restored.events().first().toMap()
                                    .value(QStringLiteral("id")).toString()).isEmpty());
}

QTEST_GUILESS_MAIN(IncidentManagerTests)

#include "IncidentManagerTests.moc"
