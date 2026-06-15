pragma Singleton
import QtQuick

QtObject {
    readonly property color appBackground: "#1e1e1e"
    readonly property color topBarBackground: "#252526"
    readonly property color statusBarBackground: "#151922"

    readonly property color panelBackground: "#161a24"
    readonly property color panelAltBackground: "#11151f"
    readonly property color panelSoftBackground: "#1f2733"
    readonly property color panelBorder: "#2a3347"
    readonly property color panelBorderStrong: "#364152"

    readonly property color cardBackground: "#1f2733"
    readonly property color cardBorder: "#334155"
    readonly property color cardHover: "#283244"

    readonly property color controlBackground: "#0f172a"
    readonly property color controlBackgroundAlt: "#121724"
    readonly property color controlBorder: "#334155"
    readonly property color controlBorderStrong: "#4a5568"

    readonly property color textPrimary: "#ffffff"
    readonly property color textSecondary: "#cbd5e1"
    readonly property color textMuted: "#94a3b8"
    readonly property color textFaint: "#666666"

    readonly property color accent: "#3b82f6"
    readonly property color accentHover: "#60a5fa"
    readonly property color success: "#16a34a"
    readonly property color warning: "#f59e0b"
    readonly property color danger: "#e53e3e"

    readonly property color overlayDark: "#e0000000"
    readonly property color overlayBorder: "#44ffffff"

    readonly property int radiusXs: 3
    readonly property int radiusSm: 4
    readonly property int radiusMd: 6
    readonly property int radiusLg: 8
    readonly property int radiusXl: 10
}