#include <QtTest>
#include <QCoreApplication>

#include "../src/backend/analytics/AnalyticsEvidenceImageProcessor.h"

class AnalyticsEvidenceImageProcessorTests : public QObject
{
    Q_OBJECT

private slots:
    void normalizedDetectionProducesClampedPortraitBounds();
    void invalidDetectionProducesEmptyBounds();
    void smallCropIsUpscaledAndEnhanced();
};

void AnalyticsEvidenceImageProcessorTests::normalizedDetectionProducesClampedPortraitBounds()
{
    const QImage image(1280, 720, QImage::Format_RGB32);
    const QVariantMap detection{{"x", 0.45}, {"y", 0.3}, {"w", 0.1}, {"h", 0.2}};

    const QRect bounds = AnalyticsEvidenceImageProcessor::detectionBounds(image, detection, 0.55, 0.70);

    QVERIFY(bounds.isValid());
    QVERIFY(QRect(0, 0, image.width(), image.height()).contains(bounds));
    QVERIFY(qAbs((static_cast<double>(bounds.width()) / bounds.height()) - 0.8) < 0.02);
}

void AnalyticsEvidenceImageProcessorTests::invalidDetectionProducesEmptyBounds()
{
    const QImage image(640, 480, QImage::Format_RGB32);
    const QVariantMap detection{{"x", 20}, {"y", 20}, {"w", 0}, {"h", 100}};

    QVERIFY(AnalyticsEvidenceImageProcessor::detectionBounds(image, detection, 0.5, 0.5).isEmpty());
}

void AnalyticsEvidenceImageProcessorTests::smallCropIsUpscaledAndEnhanced()
{
    QImage crop(120, 180, QImage::Format_RGB32);
    crop.fill(QColor(24, 28, 32));

    const QImage prepared = AnalyticsEvidenceImageProcessor::prepareCrop(crop);

    QVERIFY(!prepared.isNull());
    QCOMPARE(prepared.width(), 360);
    QCOMPARE(prepared.height(), 540);
    QVERIFY(qGray(prepared.pixel(prepared.width() / 2, prepared.height() / 2)) > 24);
}

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    AnalyticsEvidenceImageProcessorTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "AnalyticsEvidenceImageProcessorTests.moc"
