#include "StreamSessionPolicy.h"

#include <QtGlobal>

int StreamSessionPolicy::previewPriorityScore(int gridIndex,
                                              int spanRows,
                                              int spanCols,
                                              bool selected,
                                              bool recordingActive,
                                              bool analyticsActive,
                                              bool online)
{
    const int rows = qMax(1, spanRows);
    const int cols = qMax(1, spanCols);
    int score = qMin(rows, 1200) * qMin(cols, 1200);

    if (online) {
        score += 1000;
    }
    if (analyticsActive) {
        score += 4000000;
    }
    if (recordingActive) {
        score += 5000000;
    }
    if (selected) {
        score += 10000000;
    }

    // Stable, tiny tie-breaker preference for earlier slots without letting
    // ordering overpower tile size or user activity.
    if (gridIndex >= 0) {
        score += qMax(0, 999 - qMin(gridIndex, 999));
    }

    return score;
}

bool StreamSessionPolicy::shouldRunPreview(bool smartBudgetEnabled,
                                           int maxPreviewStreams,
                                           int previewBudgetRank,
                                           bool hasCamera,
                                           bool canLive,
                                           bool fullscreenActive,
                                           bool archiveOpen,
                                           bool recordingActive,
                                           bool analyticsActive)
{
    return previewPauseReasonCode(smartBudgetEnabled,
                                  maxPreviewStreams,
                                  previewBudgetRank,
                                  hasCamera,
                                  canLive,
                                  fullscreenActive,
                                  archiveOpen,
                                  recordingActive,
                                  analyticsActive).isEmpty();
}

QString StreamSessionPolicy::previewPauseReasonCode(bool smartBudgetEnabled,
                                                    int maxPreviewStreams,
                                                    int previewBudgetRank,
                                                    bool hasCamera,
                                                    bool canLive,
                                                    bool fullscreenActive,
                                                    bool archiveOpen,
                                                    bool recordingActive,
                                                    bool analyticsActive)
{
    if (!hasCamera) {
        return QStringLiteral("empty");
    }
    if (!canLive) {
        return QStringLiteral("permission");
    }
    if (!smartBudgetEnabled) {
        return QString();
    }
    if (recordingActive || analyticsActive) {
        return QString();
    }
    if (fullscreenActive) {
        return QStringLiteral("fullscreen");
    }
    if (archiveOpen) {
        return QStringLiteral("archive");
    }

    const int limit = maxPreviewStreams > 0 ? maxPreviewStreams : DefaultMaxPreviewStreams;
    if (limit <= 0 || previewBudgetRank < 0) {
        return QString();
    }

    return previewBudgetRank < limit ? QString() : QStringLiteral("budget");
}
