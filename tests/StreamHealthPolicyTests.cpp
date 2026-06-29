#include "StreamHealthPolicy.h"

#include <QTest>

class StreamHealthPolicyTests : public QObject
{
    Q_OBJECT

private slots:
    void inactiveWhenPlayerIsStopped();
    void allowsStartupGraceBeforeFirstFrame();
    void stallsWhenStartupGraceExpiresWithoutFrames();
    void staysHealthyAfterRecentFrame();
    void stallsWhenFramesStopArriving();
    void ignoresInvalidCustomWindows();
};

void StreamHealthPolicyTests::inactiveWhenPlayerIsStopped()
{
    QCOMPARE(StreamHealthPolicy::evaluate(false, false, 30000, 1000, 0),
             StreamHealthPolicy::State::Inactive);
    QVERIFY(!StreamHealthPolicy::isFrameStalled(false, false, 30000, 1000, 0));
}

void StreamHealthPolicyTests::allowsStartupGraceBeforeFirstFrame()
{
    QCOMPARE(StreamHealthPolicy::evaluate(true, false, 11999, 0, 0),
             StreamHealthPolicy::State::Starting);
    QVERIFY(!StreamHealthPolicy::isFrameStalled(true, false, 11999, 0, 0));
}

void StreamHealthPolicyTests::stallsWhenStartupGraceExpiresWithoutFrames()
{
    QCOMPARE(StreamHealthPolicy::evaluate(true, false, 13000, 1000, 0),
             StreamHealthPolicy::State::Stalled);
    QVERIFY(StreamHealthPolicy::isFrameStalled(true, false, 13000, 1000, 0));
}

void StreamHealthPolicyTests::staysHealthyAfterRecentFrame()
{
    QCOMPARE(StreamHealthPolicy::evaluate(true, true, 10000, 1000, 3000),
             StreamHealthPolicy::State::Healthy);
    QVERIFY(!StreamHealthPolicy::isFrameStalled(true, true, 10000, 1000, 3000));
}

void StreamHealthPolicyTests::stallsWhenFramesStopArriving()
{
    QCOMPARE(StreamHealthPolicy::evaluate(true, true, 11001, 1000, 3000),
             StreamHealthPolicy::State::Stalled);
    QVERIFY(StreamHealthPolicy::isFrameStalled(true, true, 11001, 1000, 3000));
}

void StreamHealthPolicyTests::ignoresInvalidCustomWindows()
{
    QCOMPARE(StreamHealthPolicy::evaluate(true, false, 11999, 0, 0, -1, 0),
             StreamHealthPolicy::State::Starting);
    QCOMPARE(StreamHealthPolicy::evaluate(true, true, 11001, 1000, 3000, 0, -1),
             StreamHealthPolicy::State::Stalled);
}

QTEST_APPLESS_MAIN(StreamHealthPolicyTests)
#include "StreamHealthPolicyTests.moc"
