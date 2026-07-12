#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class CameraHealthPolicy
{
public:
    static QString normalizeProfile(const QString &profileId);
    static QVariantList profiles();
    static QVariantList probePlan(const QString &profileId, bool hasSubStream);
    static QString overallStatus(const QVariantList &probes);
    static QString recommendation(const QVariantMap &cameraResult);
    static QString reportText(const QVariantMap &run);

private:
    static QVariantMap probe(const QString &id, const QString &label,
                             const QString &kind, bool required);
};
