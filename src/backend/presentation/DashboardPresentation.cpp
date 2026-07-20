#include "DashboardPresentation.h"

#include "CameraModel.h"
#include "UserManager.h"

#include <QLocale>
#include <climits>

namespace {

bool hasPermission(int permissions, int permission)
{
    return (permissions & UserManager::Perm_All) == UserManager::Perm_All
        || (permissions & permission) == permission;
}

QString compactNumber(double value, int precision = 1)
{
    QString result = QLocale::c().toString(value, 'f', precision);
    if (result.contains(QLatin1Char('.'))) {
        while (result.endsWith(QLatin1Char('0'))) result.chop(1);
        if (result.endsWith(QLatin1Char('.'))) result.chop(1);
    }
    return result;
}

} // namespace

DashboardPresentation::DashboardPresentation(QObject *parent)
    : QObject(parent)
{
}

QVariantMap DashboardPresentation::designTokens() const
{
    return {
        {QStringLiteral("version"), 1},
        {QStringLiteral("colors"), QVariantMap{
             {QStringLiteral("background"), QStringLiteral("#10141c")},
             {QStringLiteral("sidebar"), QStringLiteral("#191919")},
             {QStringLiteral("surface"), QStringLiteral("#202632")},
             {QStringLiteral("surfaceAlt"), QStringLiteral("#171d28")},
             {QStringLiteral("tile"), QStringLiteral("#202a38")},
             {QStringLiteral("tileHover"), QStringLiteral("#263447")},
             {QStringLiteral("tilePressed"), QStringLiteral("#182236")},
             {QStringLiteral("stroke"), QStringLiteral("#28344a")},
             {QStringLiteral("strokeStrong"), QStringLiteral("#3b82f6")},
             {QStringLiteral("accent"), QStringLiteral("#2563eb")},
             {QStringLiteral("accentHover"), QStringLiteral("#1d4ed8")},
             {QStringLiteral("success"), QStringLiteral("#16a34a")},
             {QStringLiteral("danger"), QStringLiteral("#dc2626")},
             {QStringLiteral("warning"), QStringLiteral("#f59e0b")},
             {QStringLiteral("textPrimary"), QStringLiteral("#ffffff")},
             {QStringLiteral("textSecondary"), QStringLiteral("#cbd5e1")},
             {QStringLiteral("textMuted"), QStringLiteral("#94a3b8")},
             {QStringLiteral("textFaint"), QStringLiteral("#64748b")}}},
        {QStringLiteral("metrics"), QVariantMap{
             {QStringLiteral("radiusSmall"), 3},
             {QStringLiteral("radiusMedium"), 6},
             {QStringLiteral("controlHeight"), 38},
             {QStringLiteral("topBarHeight"), 80},
             {QStringLiteral("statusBarHeight"), 30},
             {QStringLiteral("sidebarWidth"), 400},
             {QStringLiteral("spaceSmall"), 6},
             {QStringLiteral("spaceMedium"), 10},
             {QStringLiteral("spaceLarge"), 16}}},
        {QStringLiteral("typography"), QVariantMap{
             {QStringLiteral("family"), QStringLiteral("Segoe UI")},
             {QStringLiteral("body"), 13},
             {QStringLiteral("caption"), 12},
             {QStringLiteral("title"), 20}}}
    };
}

QString DashboardPresentation::cameraStatusCode(const QString &status) const
{
    const QString normalized = status.trimmed().toLower();
    if (normalized == QStringLiteral("online") || normalized == QStringLiteral("онлайн")) {
        return QStringLiteral("online");
    }
    if (normalized.contains(QStringLiteral("auth"))
        || normalized.contains(QStringLiteral("авториза"))) {
        return QStringLiteral("auth-required");
    }
    if (normalized.contains(QStringLiteral("connect"))
        || normalized.contains(QStringLiteral("подключ"))) {
        return QStringLiteral("connecting");
    }
    if (normalized.isEmpty() || normalized == QStringLiteral("unknown")
        || normalized == QStringLiteral("неизвестно")) {
        return QStringLiteral("unknown");
    }
    return QStringLiteral("offline");
}

QVariantMap DashboardPresentation::cameraView(const Camera &camera, int index,
                                              const QVariantMap &health) const
{
    const int httpPort = camera.onvifPort > 0 ? camera.onvifPort : 80;
    const QString host = camera.ip.contains(QLatin1Char(':'))
        ? QStringLiteral("[%1]").arg(camera.ip) : camera.ip;
    QVariantMap view{
        {QStringLiteral("index"), index},
        {QStringLiteral("id"), camera.id},
        {QStringLiteral("name"), camera.name},
        {QStringLiteral("ip"), camera.ip},
        {QStringLiteral("status"), camera.status},
        {QStringLiteral("statusCode"), cameraStatusCode(camera.status)},
        {QStringLiteral("group"), camera.group},
        {QStringLiteral("recording"), camera.isRecording},
        {QStringLiteral("openIpc"), camera.isOpenIpc},
        {QStringLiteral("manufacturer"), camera.manufacturer},
        {QStringLiteral("rtspPort"), camera.port},
        {QStringLiteral("httpPort"), httpPort},
        {QStringLiteral("webUiUrl"), QStringLiteral("http://%1:%2/").arg(host).arg(httpPort)},
        {QStringLiteral("previewUrl"), QStringLiteral("/api/v1/cameras/%1/preview.jpg").arg(index)},
        {QStringLiteral("previewStreamUrl"), QStringLiteral("/api/v1/cameras/%1/preview.mjpeg").arg(index)},
        {QStringLiteral("snapshotUrl"), QStringLiteral("/api/v1/cameras/%1/snapshot.jpg").arg(index)}
    };
    if (!health.isEmpty()) view.insert(QStringLiteral("health"), health);
    return view;
}

QVariantMap DashboardPresentation::capabilityManifest(int permissions,
                                                      bool webRtcAvailable,
                                                      bool audioAvailable) const
{
    const bool live = hasPermission(permissions, UserManager::Perm_LiveView);
    const bool playback = hasPermission(permissions, UserManager::Perm_Playback);
    const bool ptz = hasPermission(permissions, UserManager::Perm_PTZ);
    const bool exportAllowed = hasPermission(permissions, UserManager::Perm_Export);
    const bool settings = hasPermission(permissions, UserManager::Perm_Settings);
    const bool users = hasPermission(permissions, UserManager::Perm_UserManage);
    const bool analytics = hasPermission(permissions, UserManager::Perm_Analytics);
    return {
        {QStringLiteral("version"), 1},
        {QStringLiteral("monitor"), QVariantMap{
             {QStringLiteral("live"), live},
             {QStringLiteral("layouts"), QVariantList{1, 4, 9}},
             {QStringLiteral("webRtc"), live && webRtcAvailable},
             {QStringLiteral("mjpegFallback"), live},
             {QStringLiteral("recording"), live},
             {QStringLiteral("snapshot"), live},
             {QStringLiteral("audio"), live && webRtcAvailable && audioAvailable},
             {QStringLiteral("fullscreen"), live},
             {QStringLiteral("ptz"), ptz}}},
        {QStringLiteral("archive"), QVariantMap{
             {QStringLiteral("playback"), playback},
             {QStringLiteral("download"), playback && exportAllowed}}},
        {QStringLiteral("administration"), QVariantMap{
             {QStringLiteral("settings"), settings},
             {QStringLiteral("users"), users},
             {QStringLiteral("logs"), settings},
             {QStringLiteral("devices"), settings},
             {QStringLiteral("camex"), settings}}},
        {QStringLiteral("analytics"), analytics},
        {QStringLiteral("adaptations"), QVariantMap{
             {QStringLiteral("fileDialogs"), QStringLiteral("upload-download")},
             {QStringLiteral("keychain"), QStringLiteral("backend-only")},
             {QStringLiteral("ssh"), QStringLiteral("backend-gateway")},
             {QStringLiteral("windowChrome"), QStringLiteral("web-na")}}}
    };
}

QVariantList DashboardPresentation::permissionCatalog() const
{
    return {
        QVariantMap{{QStringLiteral("id"), QStringLiteral("liveView")},
                    {QStringLiteral("value"), UserManager::Perm_LiveView}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("playback")},
                    {QStringLiteral("value"), UserManager::Perm_Playback}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("ptz")},
                    {QStringLiteral("value"), UserManager::Perm_PTZ}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("export")},
                    {QStringLiteral("value"), UserManager::Perm_Export}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("settings")},
                    {QStringLiteral("value"), UserManager::Perm_Settings}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("userManage")},
                    {QStringLiteral("value"), UserManager::Perm_UserManage}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("analytics")},
                    {QStringLiteral("value"), UserManager::Perm_Analytics}}
    };
}

QVariantList DashboardPresentation::settingsSchema() const
{
    return {
        QVariantMap{{QStringLiteral("key"), QStringLiteral("language")},
                    {QStringLiteral("group"), QStringLiteral("general")},
                    {QStringLiteral("type"), QStringLiteral("select")},
                    {QStringLiteral("options"), QVariantList{QStringLiteral("ru"), QStringLiteral("en")}}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("notificationsEnabled")},
                    {QStringLiteral("group"), QStringLiteral("general")},
                    {QStringLiteral("type"), QStringLiteral("boolean")}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("preferredStream")},
                    {QStringLiteral("group"), QStringLiteral("streaming")},
                    {QStringLiteral("type"), QStringLiteral("select")},
                    {QStringLiteral("options"), QVariantList{QStringLiteral("auto"), QStringLiteral("hd"), QStringLiteral("sd")}}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("playerFillMode")},
                    {QStringLiteral("group"), QStringLiteral("streaming")},
                    {QStringLiteral("type"), QStringLiteral("select")},
                    {QStringLiteral("options"), QVariantList{-1, 0, 1}}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("showStatsOverlay")},
                    {QStringLiteral("group"), QStringLiteral("streaming")},
                    {QStringLiteral("type"), QStringLiteral("boolean")}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("defaultAutoplay")},
                    {QStringLiteral("group"), QStringLiteral("streaming")},
                    {QStringLiteral("type"), QStringLiteral("boolean")}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("playerBufferMode")},
                    {QStringLiteral("group"), QStringLiteral("streaming")},
                    {QStringLiteral("type"), QStringLiteral("select")},
                    {QStringLiteral("options"), QVariantList{0, 1, 2}}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("playerRtspTransport")},
                    {QStringLiteral("group"), QStringLiteral("streaming")},
                    {QStringLiteral("type"), QStringLiteral("select")},
                    {QStringLiteral("options"), QVariantList{QStringLiteral("tcp"), QStringLiteral("udp"), QStringLiteral("udp_mcast"), QStringLiteral("http")}}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("recordingSegmentDuration")},
                    {QStringLiteral("group"), QStringLiteral("recording")},
                    {QStringLiteral("type"), QStringLiteral("integer")},
                    {QStringLiteral("minimum"), 5}, {QStringLiteral("maximum"), 60},
                    {QStringLiteral("step"), 5}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("analyticsEnabled")},
                    {QStringLiteral("group"), QStringLiteral("analytics")},
                    {QStringLiteral("type"), QStringLiteral("boolean")}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("sidebarVisible")},
                    {QStringLiteral("group"), QStringLiteral("appearance")},
                    {QStringLiteral("type"), QStringLiteral("boolean")}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("sidebarToolsExpanded")},
                    {QStringLiteral("group"), QStringLiteral("appearance")},
                    {QStringLiteral("type"), QStringLiteral("boolean")}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("webSessionTimeoutMinutes")},
                    {QStringLiteral("group"), QStringLiteral("web")},
                    {QStringLiteral("type"), QStringLiteral("integer")},
                    {QStringLiteral("minimum"), 5}, {QStringLiteral("maximum"), 1440}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("webSecureCookies")},
                    {QStringLiteral("group"), QStringLiteral("web")},
                    {QStringLiteral("type"), QStringLiteral("boolean")}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("webServerEnabled")},
                    {QStringLiteral("group"), QStringLiteral("web")},
                    {QStringLiteral("type"), QStringLiteral("boolean")},
                    {QStringLiteral("readOnly"), true}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("webServerAllowRemote")},
                    {QStringLiteral("group"), QStringLiteral("web")},
                    {QStringLiteral("type"), QStringLiteral("boolean")},
                    {QStringLiteral("readOnly"), true}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("webServerBindAddress")},
                    {QStringLiteral("group"), QStringLiteral("web")},
                    {QStringLiteral("type"), QStringLiteral("string")},
                    {QStringLiteral("readOnly"), true}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("webServerPort")},
                    {QStringLiteral("group"), QStringLiteral("web")},
                    {QStringLiteral("type"), QStringLiteral("integer")},
                    {QStringLiteral("readOnly"), true}},
        QVariantMap{{QStringLiteral("key"), QStringLiteral("webSocketPort")},
                    {QStringLiteral("group"), QStringLiteral("web")},
                    {QStringLiteral("type"), QStringLiteral("integer")},
                    {QStringLiteral("readOnly"), true}}
    };
}

QVariantMap DashboardPresentation::settingsView(const QVariantMap &settings) const
{
    QVariantMap view;
    for (const QVariant &definition : settingsSchema()) {
        const QString key = definition.toMap().value(QStringLiteral("key")).toString();
        if (settings.contains(key)) view.insert(key, settings.value(key));
    }
    view.insert(QStringLiteral("recordingsPathConfigured"),
                !settings.value(QStringLiteral("recordingsPath")).toString().trimmed().isEmpty());
    view.insert(QStringLiteral("screenshotsPathConfigured"),
                !settings.value(QStringLiteral("screenshotsPath")).toString().trimmed().isEmpty());
    return view;
}

bool DashboardPresentation::normalizeSettingsPatch(const QVariantMap &input,
                                                   QVariantMap *normalized,
                                                   QString *error) const
{
    if (!normalized || !error || input.isEmpty() || input.size() > 32) {
        if (error) *error = QStringLiteral("Settings patch is empty or too large");
        return false;
    }
    QVariantMap definitions;
    for (const QVariant &definition : settingsSchema()) {
        const QVariantMap map = definition.toMap();
        definitions.insert(map.value(QStringLiteral("key")).toString(), map);
    }
    QVariantMap result;
    for (auto it = input.constBegin(); it != input.constEnd(); ++it) {
        const QVariantMap definition = definitions.value(it.key()).toMap();
        if (definition.isEmpty() || definition.value(QStringLiteral("readOnly")).toBool()) {
            *error = QStringLiteral("Setting is unknown or read-only: %1").arg(it.key());
            return false;
        }
        const QString type = definition.value(QStringLiteral("type")).toString();
        QVariant value = it.value();
        if (type == QStringLiteral("boolean")) {
            if (value.metaType().id() != QMetaType::Bool) {
                *error = QStringLiteral("Setting must be boolean: %1").arg(it.key());
                return false;
            }
        } else if (type == QStringLiteral("integer")) {
            bool ok = false;
            int number = value.toInt(&ok);
            const int minimum = definition.value(QStringLiteral("minimum"), INT_MIN).toInt();
            const int maximum = definition.value(QStringLiteral("maximum"), INT_MAX).toInt();
            if (!ok || number < minimum || number > maximum) {
                *error = QStringLiteral("Setting is outside the allowed range: %1").arg(it.key());
                return false;
            }
            const int step = definition.value(QStringLiteral("step"), 1).toInt();
            if (step > 1) number = minimum + ((number - minimum) / step) * step;
            value = number;
        } else if (type == QStringLiteral("select")) {
            bool found = false;
            for (const QVariant &option : definition.value(QStringLiteral("options")).toList()) {
                if (option == value) { found = true; break; }
            }
            if (!found) {
                *error = QStringLiteral("Setting has an unsupported value: %1").arg(it.key());
                return false;
            }
        }
        result.insert(it.key(), value);
    }
    *normalized = result;
    error->clear();
    return true;
}

QString DashboardPresentation::formatBytes(qint64 bytes) const
{
    const qint64 safeBytes = qMax<qint64>(0, bytes);
    static const QStringList units{QStringLiteral("B"), QStringLiteral("KiB"),
                                   QStringLiteral("MiB"), QStringLiteral("GiB"),
                                   QStringLiteral("TiB")};
    double value = static_cast<double>(safeBytes);
    int unit = 0;
    while (value >= 1024.0 && unit + 1 < units.size()) {
        value /= 1024.0;
        ++unit;
    }
    return QStringLiteral("%1 %2").arg(compactNumber(value, unit == 0 ? 0 : 1), units.at(unit));
}

QString DashboardPresentation::formatBitrate(qint64 bitsPerSecond) const
{
    const qint64 safeRate = qMax<qint64>(0, bitsPerSecond);
    if (safeRate >= 1000000) {
        return QStringLiteral("%1 Mbps").arg(compactNumber(safeRate / 1000000.0));
    }
    if (safeRate >= 1000) {
        return QStringLiteral("%1 Kbps").arg(compactNumber(safeRate / 1000.0));
    }
    return QStringLiteral("%1 bps").arg(safeRate);
}

QString DashboardPresentation::formatDuration(qint64 milliseconds) const
{
    qint64 seconds = qMax<qint64>(0, milliseconds) / 1000;
    const qint64 hours = seconds / 3600;
    seconds %= 3600;
    const qint64 minutes = seconds / 60;
    seconds %= 60;
    if (hours > 0) {
        return QStringLiteral("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }
    return QStringLiteral("%1:%2").arg(minutes).arg(seconds, 2, 10, QLatin1Char('0'));
}
