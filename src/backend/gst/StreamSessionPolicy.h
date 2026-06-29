#ifndef STREAMSESSIONPOLICY_H
#define STREAMSESSIONPOLICY_H

#include <QString>

class StreamSessionPolicy
{
public:
    static constexpr int DefaultMaxPreviewStreams = 16;

    static int previewPriorityScore(int gridIndex,
                                    int spanRows,
                                    int spanCols,
                                    bool selected,
                                    bool recordingActive,
                                    bool analyticsActive,
                                    bool online);

    static bool shouldRunPreview(bool smartBudgetEnabled,
                                 int maxPreviewStreams,
                                 int previewBudgetRank,
                                 bool hasCamera,
                                 bool canLive,
                                 bool fullscreenActive,
                                 bool archiveOpen,
                                 bool recordingActive,
                                 bool analyticsActive);

    static QString previewPauseReasonCode(bool smartBudgetEnabled,
                                          int maxPreviewStreams,
                                          int previewBudgetRank,
                                          bool hasCamera,
                                          bool canLive,
                                          bool fullscreenActive,
                                          bool archiveOpen,
                                          bool recordingActive,
                                          bool analyticsActive);
};

#endif // STREAMSESSIONPOLICY_H
