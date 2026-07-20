#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

struct Camera;

class DashboardPresentation final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap designTokens READ designTokens CONSTANT)

public:
    explicit DashboardPresentation(QObject *parent = nullptr);

    QVariantMap designTokens() const;
    QVariantMap cameraView(const Camera &camera, int index,
                           const QVariantMap &health = {}) const;
    QVariantMap capabilityManifest(int permissions, bool webRtcAvailable,
                                   bool audioAvailable) const;
    QVariantList permissionCatalog() const;
    QVariantList settingsSchema() const;
    QVariantMap settingsView(const QVariantMap &settings) const;
    bool normalizeSettingsPatch(const QVariantMap &input, QVariantMap *normalized,
                                QString *error) const;

    Q_INVOKABLE QString cameraStatusCode(const QString &status) const;
    Q_INVOKABLE QString formatBytes(qint64 bytes) const;
    Q_INVOKABLE QString formatBitrate(qint64 bitsPerSecond) const;
    Q_INVOKABLE QString formatDuration(qint64 milliseconds) const;
};
