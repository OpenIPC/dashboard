#include "CameraStatusPolicy.h"

namespace {
bool isOfflineStatus(const QString &status)
{
    return status.compare(QStringLiteral("Offline"), Qt::CaseInsensitive) == 0;
}
} // namespace

QString CameraStatusPolicy::effectiveStatus(const QStringList &gridStatuses,
                                            const QString &modelStatus,
                                            const QString &fallbackStatus,
                                            bool streamOfflineIsAuthoritative)
{
    if (streamOfflineIsAuthoritative) {
        return QStringLiteral("Offline");
    }

    QString selectedGridStatus;
    for (const QString &rawStatus : gridStatuses) {
        const QString status = rawStatus.trimmed();
        if (status.isEmpty()) {
            continue;
        }

        if (isOfflineStatus(status)) {
            return status;
        }

        if (selectedGridStatus.isEmpty()) {
            selectedGridStatus = status;
        }
    }

    if (!selectedGridStatus.isEmpty()) {
        return selectedGridStatus;
    }

    const QString model = modelStatus.trimmed();
    if (!model.isEmpty()) {
        return model;
    }

    return fallbackStatus.trimmed();
}

bool CameraStatusPolicy::isOnline(const QString &status)
{
    return status.trimmed().compare(QStringLiteral("Online"), Qt::CaseInsensitive) == 0;
}

QString CameraStatusPolicy::attentionReason(const QString &effectiveStatus, const QString &statusDetail)
{
    const QString detail = statusDetail.simplified();
    if (!detail.isEmpty()) {
        return detail;
    }

    const QString status = effectiveStatus.trimmed();
    if (status.isEmpty()) {
        return QStringLiteral("Unknown");
    }

    if (!isOnline(status)) {
        return status;
    }

    return {};
}

QString CameraStatusPolicy::searchText(const QString &effectiveStatus, const QString &statusDetail)
{
    const QString status = effectiveStatus.trimmed();
    const QString reason = attentionReason(status, statusDetail);

    QStringList parts;
    if (!status.isEmpty()) {
        parts.append(status);
    }
    if (!reason.isEmpty() && reason.compare(status, Qt::CaseInsensitive) != 0) {
        parts.append(reason);
    }

    return parts.join(QLatin1Char(' '));
}

bool CameraStatusPolicy::needsAttention(const QString &effectiveStatus, const QString &statusDetail)
{
    return !attentionReason(effectiveStatus, statusDetail).isEmpty();
}
