#include "StreamHealthPolicy.h"

#include <QtGlobal>

namespace {

int saneWindow(int value, int fallback)
{
    return value > 0 ? value : fallback;
}

qint64 elapsedSince(qint64 nowMs, qint64 thenMs)
{
    if (nowMs <= 0 || thenMs <= 0) {
        return 0;
    }
    return qMax<qint64>(0, nowMs - thenMs);
}

} // namespace

StreamHealthPolicy::State StreamHealthPolicy::evaluate(bool running,
                                                       bool hasFrame,
                                                       qint64 nowMs,
                                                       qint64 startedMs,
                                                       qint64 lastFrameMs,
                                                       int startupGraceMs,
                                                       int frameStallMs)
{
    if (!running) {
        return State::Inactive;
    }

    const int startupGrace = saneWindow(startupGraceMs, DefaultStartupGraceMs);
    const int frameStall = saneWindow(frameStallMs, DefaultFrameStallMs);
    const qint64 effectiveStartedMs = startedMs > 0 ? startedMs : nowMs;

    if (!hasFrame || lastFrameMs <= 0) {
        return elapsedSince(nowMs, effectiveStartedMs) >= startupGrace
            ? State::Stalled
            : State::Starting;
    }

    return elapsedSince(nowMs, lastFrameMs) >= frameStall
        ? State::Stalled
        : State::Healthy;
}

bool StreamHealthPolicy::isFrameStalled(bool running,
                                        bool hasFrame,
                                        qint64 nowMs,
                                        qint64 startedMs,
                                        qint64 lastFrameMs,
                                        int startupGraceMs,
                                        int frameStallMs)
{
    return evaluate(running, hasFrame, nowMs, startedMs, lastFrameMs,
                    startupGraceMs, frameStallMs) == State::Stalled;
}
