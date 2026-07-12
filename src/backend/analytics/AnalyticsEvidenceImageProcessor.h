#pragma once

#include <QImage>
#include <QRect>
#include <QVariantMap>

namespace AnalyticsEvidenceImageProcessor {

QRect detectionBounds(const QImage &image,
                      const QVariantMap &detection,
                      double padXFactor,
                      double padYFactor);

QImage prepareCrop(const QImage &image);

} // namespace AnalyticsEvidenceImageProcessor
