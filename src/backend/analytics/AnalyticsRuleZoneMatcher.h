#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace AnalyticsRuleZoneMatcher {

bool matches(const QVariantMap &detection,
             const QString &zonePreset,
             const QVariantList &polygon = {});

QVariantList normalizePolygon(const QVariantList &polygon);

} // namespace AnalyticsRuleZoneMatcher
