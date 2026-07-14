#include "AnalyticsUploadRetryPolicy.h"

#include <QtGlobal>

namespace AnalyticsUploadRetryPolicy {

bool shouldRetry(int completedAttempts, bool retryable, int maximumAttempts)
{
    return retryable && completedAttempts > 0 && completedAttempts < qMax(1, maximumAttempts);
}

int retryDelayMs(int completedAttempts, int baseDelayMs, int maximumDelayMs)
{
    const int safeBase = qMax(1, baseDelayMs);
    const int safeMaximum = qMax(safeBase, maximumDelayMs);
    const int exponent = qBound(0, completedAttempts - 1, 10);
    const qint64 delay = static_cast<qint64>(safeBase) << exponent;
    return static_cast<int>(qMin<qint64>(delay, safeMaximum));
}

} // namespace AnalyticsUploadRetryPolicy
