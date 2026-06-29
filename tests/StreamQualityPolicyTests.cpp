#include "StreamQualityPolicy.h"

#include <QTest>

class StreamQualityPolicyTests : public QObject
{
    Q_OBJECT

private slots:
    void autoUsesSubstreamForUniformGrid();
    void autoUsesMainstreamForSingleView();
    void autoUsesMainstreamForLargeCustomTiles();
    void respectsManualPreference();
    void fallsBackWhenRequestedStreamIsMissing();
};

void StreamQualityPolicyTests::autoUsesSubstreamForUniformGrid()
{
    QCOMPARE(StreamQualityPolicy::resolvePreviewQuality(QStringLiteral("auto"),
                                                        2, 2, 600, 600, false),
             StreamQualityPolicy::Quality::Sub);
    QCOMPARE(StreamQualityPolicy::selectPreviewUrl(QStringLiteral("rtsp://cam/0"),
                                                   QStringLiteral("rtsp://cam/1"),
                                                   QStringLiteral("rtsp://cam/0"),
                                                   QStringLiteral("auto"),
                                                   2, 2, 600, 600, false),
             QStringLiteral("rtsp://cam/1"));
}

void StreamQualityPolicyTests::autoUsesMainstreamForSingleView()
{
    QCOMPARE(StreamQualityPolicy::resolvePreviewQuality(QStringLiteral("auto"),
                                                        1, 1, 1200, 1200, false),
             StreamQualityPolicy::Quality::Main);
}

void StreamQualityPolicyTests::autoUsesMainstreamForLargeCustomTiles()
{
    // 1+5 layout: 3x3 logical grid, first tile spans 2x2.
    QCOMPARE(StreamQualityPolicy::resolvePreviewQuality(QStringLiteral("auto"),
                                                        3, 3, 800, 800, false),
             StreamQualityPolicy::Quality::Main);

    // 2+8 layout: 4x4 logical grid, first two tiles span 2x2.
    QCOMPARE(StreamQualityPolicy::resolvePreviewQuality(QStringLiteral("auto"),
                                                        4, 4, 600, 600, false),
             StreamQualityPolicy::Quality::Main);
}

void StreamQualityPolicyTests::respectsManualPreference()
{
    QCOMPARE(StreamQualityPolicy::resolvePreviewQuality(QStringLiteral("hd"),
                                                        4, 4, 300, 300, false),
             StreamQualityPolicy::Quality::Main);
    QCOMPARE(StreamQualityPolicy::resolvePreviewQuality(QStringLiteral("sd"),
                                                        1, 1, 1200, 1200, false),
             StreamQualityPolicy::Quality::Sub);
    QCOMPARE(StreamQualityPolicy::resolvePreviewQuality(QStringLiteral("auto"),
                                                        4, 4, 300, 300, true),
             StreamQualityPolicy::Quality::Main);
}

void StreamQualityPolicyTests::fallsBackWhenRequestedStreamIsMissing()
{
    QCOMPARE(StreamQualityPolicy::selectPreviewUrl(QStringLiteral("rtsp://cam/0"),
                                                   QString(),
                                                   QStringLiteral("rtsp://cam/0"),
                                                   QStringLiteral("sd"),
                                                   2, 2, 600, 600, false),
             QStringLiteral("rtsp://cam/0"));
    QCOMPARE(StreamQualityPolicy::selectManualUrl(QStringLiteral("rtsp://cam/0"),
                                                  QStringLiteral("rtsp://cam/1"),
                                                  QString(),
                                                  true),
             QStringLiteral("rtsp://cam/0"));
}

QTEST_APPLESS_MAIN(StreamQualityPolicyTests)
#include "StreamQualityPolicyTests.moc"
