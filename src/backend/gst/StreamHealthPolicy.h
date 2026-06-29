#ifndef STREAMHEALTHPOLICY_H
#define STREAMHEALTHPOLICY_H

#include <QtGlobal>

class StreamHealthPolicy
{
public:
    enum class State {
        Inactive,
        Starting,
        Healthy,
        Stalled
    };

    static constexpr int DefaultStartupGraceMs = 12000;
    static constexpr int DefaultFrameStallMs = 8000;

    static State evaluate(bool running,
                          bool hasFrame,
                          qint64 nowMs,
                          qint64 startedMs,
                          qint64 lastFrameMs,
                          int startupGraceMs = DefaultStartupGraceMs,
                          int frameStallMs = DefaultFrameStallMs);

    static bool isFrameStalled(bool running,
                               bool hasFrame,
                               qint64 nowMs,
                               qint64 startedMs,
                               qint64 lastFrameMs,
                               int startupGraceMs = DefaultStartupGraceMs,
                               int frameStallMs = DefaultFrameStallMs);
};

#endif // STREAMHEALTHPOLICY_H
