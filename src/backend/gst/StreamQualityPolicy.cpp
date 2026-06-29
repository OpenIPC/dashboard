#include "StreamQualityPolicy.h"

#include <QtGlobal>

namespace {

QString normalizedPreference(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("hd") || normalized == QStringLiteral("main")) {
        return QStringLiteral("hd");
    }
    if (normalized == QStringLiteral("sd") || normalized == QStringLiteral("sub")) {
        return QStringLiteral("sd");
    }
    return QStringLiteral("auto");
}

QString firstNonEmpty(const QString &primary, const QString &fallback)
{
    return primary.trimmed().isEmpty() ? fallback : primary;
}

bool isLargeTile(int gridRows, int gridCols, int spanRows, int spanCols)
{
    const int rows = qMax(1, gridRows);
    const int cols = qMax(1, gridCols);
    const int totalCells = rows * cols;
    if (totalCells <= 1) {
        return true;
    }

    const double normalRowSpan = 1200.0 / rows;
    const double normalColSpan = 1200.0 / cols;
    const bool widerThanNormal = spanCols > normalColSpan * 1.35;
    const bool tallerThanNormal = spanRows > normalRowSpan * 1.35;
    if (widerThanNormal || tallerThanNormal) {
        return true;
    }

    const double tileArea = qMax(1, spanRows) * qMax(1, spanCols);
    const double gridArea = 1200.0 * 1200.0;
    return tileArea / gridArea >= 0.40;
}

} // namespace

StreamQualityPolicy::Quality StreamQualityPolicy::resolvePreviewQuality(
    const QString &preferredStream,
    int gridRows,
    int gridCols,
    int spanRows,
    int spanCols,
    bool forceMain)
{
    if (forceMain) {
        return Quality::Main;
    }

    const QString preferred = normalizedPreference(preferredStream);
    if (preferred == QStringLiteral("hd")) {
        return Quality::Main;
    }
    if (preferred == QStringLiteral("sd")) {
        return Quality::Sub;
    }

    return isLargeTile(gridRows, gridCols, spanRows, spanCols) ? Quality::Main : Quality::Sub;
}

QString StreamQualityPolicy::selectPreviewUrl(const QString &streamUrl,
                                              const QString &sdStreamUrl,
                                              const QString &hdStreamUrl,
                                              const QString &preferredStream,
                                              int gridRows,
                                              int gridCols,
                                              int spanRows,
                                              int spanCols,
                                              bool forceMain)
{
    const QString mainUrl = firstNonEmpty(hdStreamUrl, streamUrl);
    const QString subUrl = firstNonEmpty(sdStreamUrl, streamUrl);
    if (mainUrl.isEmpty()) {
        return subUrl;
    }
    if (subUrl.isEmpty()) {
        return mainUrl;
    }

    return resolvePreviewQuality(preferredStream, gridRows, gridCols, spanRows, spanCols, forceMain) == Quality::Main
        ? mainUrl
        : subUrl;
}

QString StreamQualityPolicy::selectManualUrl(const QString &streamUrl,
                                             const QString &sdStreamUrl,
                                             const QString &hdStreamUrl,
                                             bool preferMain)
{
    const QString mainUrl = firstNonEmpty(hdStreamUrl, streamUrl);
    const QString subUrl = firstNonEmpty(sdStreamUrl, streamUrl);
    if (preferMain) {
        return firstNonEmpty(mainUrl, subUrl);
    }
    return firstNonEmpty(subUrl, mainUrl);
}

QString StreamQualityPolicy::qualityLabel(Quality quality)
{
    return quality == Quality::Main ? QStringLiteral("HD") : QStringLiteral("SD");
}
