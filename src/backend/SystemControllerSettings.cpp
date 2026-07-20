#include "SystemController.h"
#include "PathUtils.h"
#include "web/DashboardWebServer.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>
#include <algorithm>

namespace {

constexpr int kRecordingSegmentMinMinutes = 5;
constexpr int kRecordingSegmentMaxMinutes = 60;
constexpr int kRecordingSegmentStepMinutes = 5;
constexpr int kRecordingSegmentDefaultMinutes = 15;

QString normalizedLocalPath(const QString &pathOrUrl)
{
    return PathUtils::localPathFromUserInput(pathOrUrl);
}

int normalizedRecordingSegmentDuration(const QVariant &value)
{
    bool ok = false;
    int minutes = value.toInt(&ok);
    if (!ok) {
        minutes = kRecordingSegmentDefaultMinutes;
    }

    minutes = std::clamp(minutes, kRecordingSegmentMinMinutes, kRecordingSegmentMaxMinutes);
    minutes = ((minutes + kRecordingSegmentStepMinutes / 2) / kRecordingSegmentStepMinutes)
        * kRecordingSegmentStepMinutes;
    return std::clamp(minutes, kRecordingSegmentMinMinutes, kRecordingSegmentMaxMinutes);
}

int normalizedPlayerFillMode(const QVariant &value)
{
    bool ok = false;
    const int mode = value.toInt(&ok);
    if (!ok) {
        return 0;
    }
    if (mode < 0) {
        return -1;
    }
    if (mode > 0) {
        return 1;
    }
    return 0;
}

void normalizeAppSettings(QVariantMap &settings)
{
    const QStringList pathKeys{
        QStringLiteral("recordingsPath"),
        QStringLiteral("screenshotsPath")
    };

    for (const QString &key : pathKeys) {
        const QString value = settings.value(key).toString();
        if (!value.trimmed().isEmpty()) {
            settings[key] = normalizedLocalPath(value);
        }
    }

    if (settings.contains(QStringLiteral("recordingSegmentDuration"))) {
        settings[QStringLiteral("recordingSegmentDuration")] =
            normalizedRecordingSegmentDuration(settings.value(QStringLiteral("recordingSegmentDuration")));
    }

    if (settings.contains(QStringLiteral("playerFillMode"))) {
        settings[QStringLiteral("playerFillMode")] =
            normalizedPlayerFillMode(settings.value(QStringLiteral("playerFillMode")));
    }
}

} // namespace

void SystemController::openFolder(const QString &path)
{
    if (path.isEmpty()) return;

    QString targetPath = normalizedLocalPath(path);
    QFileInfo fi(targetPath);
    if (fi.exists() && fi.isFile()) {
        targetPath = fi.absolutePath();
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(targetPath));
}

QString SystemController::normalizeLocalPath(const QString &pathOrUrl) const
{
    return normalizedLocalPath(pathOrUrl);
}

void SystemController::saveAppSettings(const QVariantMap &settings)
{
    QVariantMap normalizedSettings = settings;
    normalizeAppSettings(normalizedSettings);

    // Merge new settings with existing ones to prevent data loss (e.g. grid state, hidden flags)
    for (auto it = normalizedSettings.begin(); it != normalizedSettings.end(); ++it) {
        m_appSettings[it.key()] = it.value();
    }
    emit appSettingsChanged();
    saveState();
    if (m_webServer) {
        m_webServer->applySettings(m_appSettings);
    }
}

QVariantMap SystemController::getAppSettings() const
{
    return m_appSettings;
}
