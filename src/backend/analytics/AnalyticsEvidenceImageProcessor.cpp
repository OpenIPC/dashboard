#include "AnalyticsEvidenceImageProcessor.h"

#include <QtMath>

namespace {

int channelLuma(QRgb pixel)
{
    return (qRed(pixel) * 54 + qGreen(pixel) * 183 + qBlue(pixel) * 19) >> 8;
}

QImage upscaleCrop(const QImage &image)
{
    if (image.isNull()) {
        return image;
    }

    constexpr int minShortSide = 360;
    constexpr int maxLongSide = 900;
    const int shortSide = qMin(image.width(), image.height());
    const int longSide = qMax(image.width(), image.height());
    if (shortSide <= 0 || shortSide >= minShortSide) {
        return image;
    }

    double scale = static_cast<double>(minShortSide) / static_cast<double>(shortSide);
    scale = qMin(scale, static_cast<double>(maxLongSide) / static_cast<double>(longSide));
    scale = qBound(1.0, scale, 3.0);
    if (scale <= 1.01) {
        return image;
    }

    return image.scaled(qRound(image.width() * scale),
                        qRound(image.height() * scale),
                        Qt::KeepAspectRatio,
                        Qt::SmoothTransformation);
}

QImage enhanceCrop(const QImage &source)
{
    if (source.isNull()) {
        return source;
    }

    QImage image = source.convertToFormat(QImage::Format_RGB32);
    const int sampleStep = qMax(1, qMin(image.width(), image.height()) / 180);
    qint64 lumaSum = 0;
    int sampleCount = 0;

    for (int y = 0; y < image.height(); y += sampleStep) {
        const auto *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); x += sampleStep) {
            lumaSum += channelLuma(line[x]);
            ++sampleCount;
        }
    }

    const double averageLuma = sampleCount > 0
        ? static_cast<double>(lumaSum) / static_cast<double>(sampleCount)
        : 128.0;
    const bool lowLight = averageLuma < 105.0;
    const double contrast = lowLight ? 1.18 : 1.08;
    const int lift = lowLight ? 12 : 2;

    const auto enhanceChannel = [contrast, lift, lowLight](int value) {
        double adjusted = (static_cast<double>(value) - 128.0) * contrast + 128.0 + lift;
        adjusted = qBound(0.0, adjusted, 255.0);
        const double normalized = adjusted / 255.0;
        const double shadowLift = (lowLight ? 0.14 : 0.05) * (1.0 - normalized);
        adjusted += (255.0 - adjusted) * shadowLift;
        return qBound(0, qRound(adjusted), 255);
    };

    for (int y = 0; y < image.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const QRgb pixel = line[x];
            line[x] = qRgb(enhanceChannel(qRed(pixel)),
                           enhanceChannel(qGreen(pixel)),
                           enhanceChannel(qBlue(pixel)));
        }
    }

    if (image.width() < 5 || image.height() < 5) {
        return image;
    }

    QImage sharpened = image.copy();
    constexpr double sharpenAmount = 0.32;
    for (int y = 1; y < image.height() - 1; ++y) {
        const auto *previous = reinterpret_cast<const QRgb *>(image.constScanLine(y - 1));
        const auto *current = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        const auto *next = reinterpret_cast<const QRgb *>(image.constScanLine(y + 1));
        auto *destination = reinterpret_cast<QRgb *>(sharpened.scanLine(y));

        for (int x = 1; x < image.width() - 1; ++x) {
            const QRgb center = current[x];
            const auto sharpenChannel = [](int centerValue, int n1, int n2, int n3, int n4) {
                const double neighborAverage = (n1 + n2 + n3 + n4) / 4.0;
                return qBound(0,
                              qRound(centerValue + ((centerValue - neighborAverage) * sharpenAmount)),
                              255);
            };

            destination[x] = qRgb(
                sharpenChannel(qRed(center), qRed(current[x - 1]), qRed(current[x + 1]), qRed(previous[x]), qRed(next[x])),
                sharpenChannel(qGreen(center), qGreen(current[x - 1]), qGreen(current[x + 1]), qGreen(previous[x]), qGreen(next[x])),
                sharpenChannel(qBlue(center), qBlue(current[x - 1]), qBlue(current[x + 1]), qBlue(previous[x]), qBlue(next[x])));
        }
    }

    return sharpened;
}

} // namespace

namespace AnalyticsEvidenceImageProcessor {

QRect detectionBounds(const QImage &image,
                      const QVariantMap &detection,
                      double padXFactor,
                      double padYFactor)
{
    if (image.isNull()) {
        return {};
    }

    double x = detection.value("x").toDouble();
    double y = detection.value("y").toDouble();
    double width = detection.value("w").toDouble();
    double height = detection.value("h").toDouble();
    if (width <= 0.0 || height <= 0.0) {
        return {};
    }

    const bool normalized = x <= 1.0 && y <= 1.0 && width <= 1.0 && height <= 1.0;
    if (normalized) {
        x *= image.width();
        y *= image.height();
        width *= image.width();
        height *= image.height();
    }

    QRect bounds(qRound(x), qRound(y), qRound(width), qRound(height));
    if (!bounds.isValid()) {
        return {};
    }

    const int padX = qRound(bounds.width() * padXFactor);
    const int padY = qRound(bounds.height() * padYFactor);
    bounds.adjust(-padX, -padY, padX, padY);

    constexpr double targetAspect = 4.0 / 5.0;
    if (bounds.width() > 0 && bounds.height() > 0) {
        const double currentAspect = static_cast<double>(bounds.width()) / bounds.height();
        if (currentAspect > targetAspect) {
            const int desiredHeight = qRound(bounds.width() / targetAspect);
            const int delta = desiredHeight - bounds.height();
            bounds.adjust(0, -delta / 2, 0, delta - delta / 2);
        } else {
            const int desiredWidth = qRound(bounds.height() * targetAspect);
            const int delta = desiredWidth - bounds.width();
            bounds.adjust(-delta / 2, 0, delta - delta / 2, 0);
        }
    }

    return bounds.intersected(QRect(0, 0, image.width(), image.height()));
}

QImage prepareCrop(const QImage &image)
{
    return enhanceCrop(upscaleCrop(image));
}

} // namespace AnalyticsEvidenceImageProcessor
