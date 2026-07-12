import QtQuick
import QtQuick.Controls
import OpenIPC

Button {
    id: root

    property color buttonColor: Theme.metroTile
    property color buttonHoverColor: Theme.metroTileHover
    property color buttonTextColor: Theme.textPrimary

    implicitHeight: 36
    leftPadding: 14
    rightPadding: 14

    background: Rectangle {
        color: root.enabled ? (root.hovered ? root.buttonHoverColor : root.buttonColor) : Theme.controlBackgroundAlt
        radius: Theme.metroTileRadius
        border.color: root.enabled && (root.hovered || root.visualFocus) ? Theme.metroStrokeStrong : Theme.metroStroke
        border.width: root.hovered || root.visualFocus ? 2 : 1
    }

    contentItem: Text {
        text: root.text
        color: root.enabled ? root.buttonTextColor : Theme.textMuted
        font.family: Theme.metroFontFamily
        font.pixelSize: 12
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
