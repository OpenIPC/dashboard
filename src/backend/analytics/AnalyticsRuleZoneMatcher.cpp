#include "AnalyticsRuleZoneMatcher.h"

#include <QLineF>
#include <QPointF>
#include <QVector>
#include <QtGlobal>

#include <cmath>

namespace {

QPointF pointFromVariant(const QVariant &value, bool *ok)
{
    const QVariantMap map = value.toMap();
    if (map.contains(QStringLiteral("x")) && map.contains(QStringLiteral("y"))) {
        bool xOk = false;
        bool yOk = false;
        const double x = map.value(QStringLiteral("x")).toDouble(&xOk);
        const double y = map.value(QStringLiteral("y")).toDouble(&yOk);
        *ok = xOk && yOk && std::isfinite(x) && std::isfinite(y);
        return QPointF(x, y);
    }

    const QVariantList pair = value.toList();
    if (pair.size() >= 2) {
        bool xOk = false;
        bool yOk = false;
        const double x = pair.at(0).toDouble(&xOk);
        const double y = pair.at(1).toDouble(&yOk);
        *ok = xOk && yOk && std::isfinite(x) && std::isfinite(y);
        return QPointF(x, y);
    }

    *ok = false;
    return {};
}

QVector<QPointF> polygonPoints(const QVariantList &polygon)
{
    QVector<QPointF> points;
    points.reserve(polygon.size());
    for (const QVariant &value : polygon) {
        bool ok = false;
        const QPointF point = pointFromVariant(value, &ok);
        if (!ok) {
            return {};
        }
        points.append(QPointF(qBound(0.0, point.x(), 1.0),
                              qBound(0.0, point.y(), 1.0)));
    }
    return points.size() >= 3 ? points : QVector<QPointF>{};
}

bool pointOnSegment(const QPointF &point, const QPointF &a, const QPointF &b)
{
    constexpr double epsilon = 1e-8;
    const double cross = (point.y() - a.y()) * (b.x() - a.x())
        - (point.x() - a.x()) * (b.y() - a.y());
    if (std::abs(cross) > epsilon) {
        return false;
    }

    const double dot = (point.x() - a.x()) * (b.x() - a.x())
        + (point.y() - a.y()) * (b.y() - a.y());
    if (dot < -epsilon) {
        return false;
    }

    return dot <= QLineF(a, b).length() * QLineF(a, b).length() + epsilon;
}

bool pointInPolygon(const QPointF &point, const QVector<QPointF> &polygon)
{
    bool inside = false;
    for (qsizetype i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const QPointF &a = polygon.at(j);
        const QPointF &b = polygon.at(i);
        if (pointOnSegment(point, a, b)) {
            return true;
        }

        const bool crosses = ((a.y() > point.y()) != (b.y() > point.y()))
            && (point.x() < (b.x() - a.x()) * (point.y() - a.y())
                    / (b.y() - a.y()) + a.x());
        if (crosses) {
            inside = !inside;
        }
    }
    return inside;
}

} // namespace

namespace AnalyticsRuleZoneMatcher {

QVariantList normalizePolygon(const QVariantList &polygon)
{
    const QVector<QPointF> points = polygonPoints(polygon);
    QVariantList result;
    result.reserve(points.size());
    for (const QPointF &point : points) {
        result.append(QVariantMap{
            {QStringLiteral("x"), point.x()},
            {QStringLiteral("y"), point.y()}
        });
    }
    return result;
}

bool matches(const QVariantMap &detection,
             const QString &zonePreset,
             const QVariantList &polygon)
{
    const double x = detection.value(QStringLiteral("x")).toDouble();
    const double y = detection.value(QStringLiteral("y")).toDouble();
    const double w = detection.value(QStringLiteral("w")).toDouble();
    const double h = detection.value(QStringLiteral("h")).toDouble();
    const QPointF center(qBound(0.0, x + (w / 2.0), 1.0),
                         qBound(0.0, y + (h / 2.0), 1.0));

    const QString normalizedZone = zonePreset.trimmed().toLower();
    if (normalizedZone.isEmpty() || normalizedZone == QStringLiteral("full")) {
        return true;
    }
    if (normalizedZone == QStringLiteral("center")) {
        return center.x() >= 0.25 && center.x() <= 0.75
            && center.y() >= 0.25 && center.y() <= 0.75;
    }
    if (normalizedZone == QStringLiteral("left")) {
        return center.x() <= 0.40;
    }
    if (normalizedZone == QStringLiteral("right")) {
        return center.x() >= 0.60;
    }
    if (normalizedZone == QStringLiteral("top")) {
        return center.y() <= 0.40;
    }
    if (normalizedZone == QStringLiteral("bottom")) {
        return center.y() >= 0.60;
    }
    if (normalizedZone == QStringLiteral("custom")) {
        const QVector<QPointF> points = polygonPoints(polygon);
        return !points.isEmpty() && pointInPolygon(center, points);
    }

    // Unknown legacy values keep the previous full-frame behavior.
    return true;
}

} // namespace AnalyticsRuleZoneMatcher
