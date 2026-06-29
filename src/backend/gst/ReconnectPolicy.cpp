#include "ReconnectPolicy.h"

#include <algorithm>

int ReconnectPolicy::delayMs(int attempt)
{
    constexpr int baseDelayMs = 1000;
    constexpr int maximumDelayMs = 30000;
    const int safeAttempt = std::clamp(attempt, 0, 30);
    const qint64 multiplier = qint64{1} << std::min(safeAttempt, 5);
    return static_cast<int>(std::min<qint64>(baseDelayMs * multiplier, maximumDelayMs));
}

bool ReconnectPolicy::isAuthenticationError(const QString &message, const QString &debug)
{
    const QString details = message + QLatin1Char(' ') + debug;
    return details.contains(QStringLiteral("401"), Qt::CaseInsensitive)
        || details.contains(QStringLiteral("unauthorized"), Qt::CaseInsensitive)
        || details.contains(QStringLiteral("authentication failed"), Qt::CaseInsensitive)
        || details.contains(QStringLiteral("not authorized"), Qt::CaseInsensitive);
}
