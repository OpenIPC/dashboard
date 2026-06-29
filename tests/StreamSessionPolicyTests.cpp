#include "StreamSessionPolicy.h"

#include <QTest>

class StreamSessionPolicyTests : public QObject
{
    Q_OBJECT

private slots:
    void blocksEmptyOrForbiddenTiles();
    void canBeDisabledForLegacyUnlimitedPreview();
    void pausesBehindFullscreenAndArchive();
    void keepsCriticalPreviewSessionsAlive();
    void scoresLargeSelectedAndCriticalTilesHigher();
    void enforcesPreviewBudgetByRank();
    void reportsPauseReasons();
    void usesSafeDefaultWhenLimitIsInvalid();
};

void StreamSessionPolicyTests::blocksEmptyOrForbiddenTiles()
{
    QVERIFY(!StreamSessionPolicy::shouldRunPreview(true, 4, 0, false, true, false, false, false, false));
    QVERIFY(!StreamSessionPolicy::shouldRunPreview(true, 4, 0, true, false, false, false, false, false));
}

void StreamSessionPolicyTests::canBeDisabledForLegacyUnlimitedPreview()
{
    QVERIFY(StreamSessionPolicy::shouldRunPreview(false, 1, 99, true, true, false, false, false, false));
}

void StreamSessionPolicyTests::pausesBehindFullscreenAndArchive()
{
    QVERIFY(!StreamSessionPolicy::shouldRunPreview(true, 4, 0, true, true, true, false, false, false));
    QVERIFY(!StreamSessionPolicy::shouldRunPreview(true, 4, 0, true, true, false, true, false, false));
}

void StreamSessionPolicyTests::keepsCriticalPreviewSessionsAlive()
{
    QVERIFY(StreamSessionPolicy::shouldRunPreview(true, 1, 20, true, true, false, false, true, false));
    QVERIFY(StreamSessionPolicy::shouldRunPreview(true, 1, 20, true, true, false, false, false, true));
    QVERIFY(StreamSessionPolicy::shouldRunPreview(true, 1, 20, true, true, true, true, true, false));
    QVERIFY(StreamSessionPolicy::shouldRunPreview(true, 1, 20, true, true, true, true, false, true));
}

void StreamSessionPolicyTests::scoresLargeSelectedAndCriticalTilesHigher()
{
    const int small = StreamSessionPolicy::previewPriorityScore(5, 300, 300, false, false, false, true);
    const int large = StreamSessionPolicy::previewPriorityScore(6, 800, 800, false, false, false, true);
    const int selected = StreamSessionPolicy::previewPriorityScore(7, 300, 300, true, false, false, true);
    const int recording = StreamSessionPolicy::previewPriorityScore(8, 300, 300, false, true, false, true);
    const int analytics = StreamSessionPolicy::previewPriorityScore(9, 300, 300, false, false, true, true);

    QVERIFY(large > small);
    QVERIFY(selected > large);
    QVERIFY(recording > small);
    QVERIFY(analytics > small);
}

void StreamSessionPolicyTests::enforcesPreviewBudgetByRank()
{
    QVERIFY(StreamSessionPolicy::shouldRunPreview(true, 2, 0, true, true, false, false, false, false));
    QVERIFY(StreamSessionPolicy::shouldRunPreview(true, 2, 1, true, true, false, false, false, false));
    QVERIFY(!StreamSessionPolicy::shouldRunPreview(true, 2, 2, true, true, false, false, false, false));
}

void StreamSessionPolicyTests::reportsPauseReasons()
{
    QCOMPARE(StreamSessionPolicy::previewPauseReasonCode(true, 2, 0, false, true, false, false, false, false),
             QStringLiteral("empty"));
    QCOMPARE(StreamSessionPolicy::previewPauseReasonCode(true, 2, 0, true, false, false, false, false, false),
             QStringLiteral("permission"));
    QCOMPARE(StreamSessionPolicy::previewPauseReasonCode(true, 2, 0, true, true, true, false, false, false),
             QStringLiteral("fullscreen"));
    QCOMPARE(StreamSessionPolicy::previewPauseReasonCode(true, 2, 0, true, true, false, true, false, false),
             QStringLiteral("archive"));
    QCOMPARE(StreamSessionPolicy::previewPauseReasonCode(true, 2, 2, true, true, false, false, false, false),
             QStringLiteral("budget"));
    QVERIFY(StreamSessionPolicy::previewPauseReasonCode(true, 2, 99, true, true, true, true, true, false).isEmpty());
    QVERIFY(StreamSessionPolicy::previewPauseReasonCode(false, 2, 99, true, true, false, false, false, false).isEmpty());
}

void StreamSessionPolicyTests::usesSafeDefaultWhenLimitIsInvalid()
{
    QVERIFY(StreamSessionPolicy::shouldRunPreview(true, 0, 15, true, true, false, false, false, false));
    QVERIFY(!StreamSessionPolicy::shouldRunPreview(true, 0, 16, true, true, false, false, false, false));
    QVERIFY(StreamSessionPolicy::shouldRunPreview(true, -1, -1, true, true, false, false, false, false));
}

QTEST_APPLESS_MAIN(StreamSessionPolicyTests)
#include "StreamSessionPolicyTests.moc"
