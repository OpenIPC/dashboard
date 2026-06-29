#ifndef CAMERASTATUSPOLICY_H
#define CAMERASTATUSPOLICY_H

#include <QString>
#include <QStringList>

class CameraStatusPolicy
{
public:
    static QString effectiveStatus(const QStringList &gridStatuses,
                                   const QString &modelStatus,
                                   const QString &fallbackStatus,
                                   bool streamOfflineIsAuthoritative);
    static bool isOnline(const QString &status);
    static QString attentionReason(const QString &effectiveStatus, const QString &statusDetail);
    static QString searchText(const QString &effectiveStatus, const QString &statusDetail);
    static bool needsAttention(const QString &effectiveStatus, const QString &statusDetail);
};

#endif // CAMERASTATUSPOLICY_H
