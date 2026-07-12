pragma Singleton
import QtQuick

QtObject {
    readonly property color appBackground: metroBackground
    readonly property color topBarBackground: metroSidebarBackground
    readonly property color statusBarBackground: "#151922"

    readonly property string metroFontFamily: "Segoe UI"
    readonly property color metroBackground: "#10141c"
    readonly property color metroSidebarBackground: "#191919"
    readonly property color metroSurface: "#202632"
    readonly property color metroSurfaceAlt: "#171d28"
    readonly property color metroTile: "#202a38"
    readonly property color metroTileHover: "#263447"
    readonly property color metroTilePressed: "#182236"
    readonly property color metroTileDisabled: "#151923"
    readonly property color metroStroke: "#28344a"
    readonly property color metroStrokeStrong: "#3b82f6"
    readonly property color metroBlue: "#2563eb"
    readonly property color metroBlueHover: "#1d4ed8"
    readonly property color metroGreen: "#16a34a"
    readonly property color metroRed: "#dc2626"
    readonly property color metroAmber: "#f59e0b"
    readonly property color metroOrange: "#f97316"
    readonly property color metroDeepBlue: "#172554"

    readonly property color successSurface: "#166534"
    readonly property color dangerSurface: "#7f1d1d"
    readonly property color dangerSurfacePressed: "#991b1b"
    readonly property color warningSurface: "#422006"
    readonly property color warningSurfaceSoft: "#2a1a08"
    readonly property color warningText: "#fde68a"
    readonly property color infoSurface: "#164e63"
    readonly property color infoText: "#67e8f9"
    readonly property color changedSurface: "#78350f"

    readonly property color panelBackground: metroBackground
    readonly property color panelAltBackground: metroSurfaceAlt
    readonly property color panelSoftBackground: metroSurface
    readonly property color panelBorder: metroStroke
    readonly property color panelBorderStrong: metroStrokeStrong

    readonly property color cardBackground: metroSurface
    readonly property color cardBorder: metroStroke
    readonly property color cardHover: metroTileHover

    readonly property color controlBackground: metroSurfaceAlt
    readonly property color controlBackgroundAlt: metroTile
    readonly property color controlBorder: metroStroke
    readonly property color controlBorderStrong: metroStrokeStrong

    readonly property color textPrimary: "#ffffff"
    readonly property color textSecondary: "#cbd5e1"
    readonly property color textMuted: "#94a3b8"
    readonly property color textFaint: "#64748b"

    readonly property color accent: metroBlue
    readonly property color accentHover: metroBlueHover
    readonly property color success: metroGreen
    readonly property color warning: metroAmber
    readonly property color danger: metroRed

    readonly property color overlayDark: "#e0000000"
    readonly property color overlayBorder: "#44ffffff"

    readonly property int radiusXs: metroRadius
    readonly property int radiusSm: metroRadius
    readonly property int radiusMd: metroTileRadius
    readonly property int radiusLg: metroTileRadius
    readonly property int radiusXl: metroTileRadius
    readonly property int metroRadius: 1
    readonly property int metroTileRadius: 2
}
