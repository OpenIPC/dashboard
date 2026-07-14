#include <QtTest>

#include "AnalyticsRuleZoneMatcher.h"

class AnalyticsRuleZoneMatcherTests : public QObject
{
    Q_OBJECT

private slots:
    void presetZonesUseDetectionCenter();
    void customPolygonIncludesEdges();
    void invalidCustomPolygonDoesNotMatch();
    void polygonCoordinatesAreClamped();
};

namespace {

QVariantMap detection(double x, double y, double w = 0.1, double h = 0.1)
{
    return {
        {QStringLiteral("x"), x},
        {QStringLiteral("y"), y},
        {QStringLiteral("w"), w},
        {QStringLiteral("h"), h}
    };
}

QVariantList square()
{
    return {
        QVariantMap{{QStringLiteral("x"), 0.2}, {QStringLiteral("y"), 0.2}},
        QVariantMap{{QStringLiteral("x"), 0.8}, {QStringLiteral("y"), 0.2}},
        QVariantMap{{QStringLiteral("x"), 0.8}, {QStringLiteral("y"), 0.8}},
        QVariantMap{{QStringLiteral("x"), 0.2}, {QStringLiteral("y"), 0.8}}
    };
}

} // namespace

void AnalyticsRuleZoneMatcherTests::presetZonesUseDetectionCenter()
{
    QVERIFY(AnalyticsRuleZoneMatcher::matches(detection(0.45, 0.45), QStringLiteral("center")));
    QVERIFY(!AnalyticsRuleZoneMatcher::matches(detection(0.05, 0.05), QStringLiteral("center")));
    QVERIFY(AnalyticsRuleZoneMatcher::matches(detection(0.05, 0.45), QStringLiteral("left")));
    QVERIFY(AnalyticsRuleZoneMatcher::matches(detection(0.85, 0.45), QStringLiteral("right")));
    QVERIFY(AnalyticsRuleZoneMatcher::matches(detection(0.45, 0.05), QStringLiteral("top")));
    QVERIFY(AnalyticsRuleZoneMatcher::matches(detection(0.45, 0.85), QStringLiteral("bottom")));
}

void AnalyticsRuleZoneMatcherTests::customPolygonIncludesEdges()
{
    QVERIFY(AnalyticsRuleZoneMatcher::matches(detection(0.45, 0.45), QStringLiteral("custom"), square()));
    QVERIFY(!AnalyticsRuleZoneMatcher::matches(detection(0.85, 0.85), QStringLiteral("custom"), square()));
    QVERIFY(AnalyticsRuleZoneMatcher::matches(detection(0.15, 0.45), QStringLiteral("custom"), square()));
}

void AnalyticsRuleZoneMatcherTests::invalidCustomPolygonDoesNotMatch()
{
    QVERIFY(!AnalyticsRuleZoneMatcher::matches(detection(0.45, 0.45), QStringLiteral("custom"), {}));
    QVERIFY(!AnalyticsRuleZoneMatcher::matches(
        detection(0.45, 0.45),
        QStringLiteral("custom"),
        QVariantList{QVariantMap{{QStringLiteral("x"), 0.2}}}));
}

void AnalyticsRuleZoneMatcherTests::polygonCoordinatesAreClamped()
{
    const QVariantList normalized = AnalyticsRuleZoneMatcher::normalizePolygon({
        QVariantList{-0.5, 0.2},
        QVariantList{1.5, 0.2},
        QVariantList{0.5, 1.2}
    });

    QCOMPARE(normalized.size(), 3);
    QCOMPARE(normalized.at(0).toMap().value(QStringLiteral("x")).toDouble(), 0.0);
    QCOMPARE(normalized.at(1).toMap().value(QStringLiteral("x")).toDouble(), 1.0);
    QCOMPARE(normalized.at(2).toMap().value(QStringLiteral("y")).toDouble(), 1.0);
}

QTEST_GUILESS_MAIN(AnalyticsRuleZoneMatcherTests)

#include "AnalyticsRuleZoneMatcherTests.moc"
