import QtQuick
import QtQuick.Controls
import OpenIPC

Button {
    id: btn
    property bool primary: false
    property bool danger: false
    property bool subtle: false
    implicitHeight: 36
    leftPadding: 16
    rightPadding: 16
    hoverEnabled: true

    contentItem: Text {
        text: btn.text
        color: btn.enabled
               ? (btn.subtle && !btn.hovered ? Theme.textSecondary : Theme.textPrimary)
               : Theme.textFaint
        font.family: Theme.metroFontFamily
        font.pixelSize: 12
        font.bold: btn.primary
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: Theme.metroTileRadius
        color: !btn.enabled ? Theme.controlBackgroundAlt
              : btn.primary ? (btn.pressed ? Theme.metroBlueHover : (btn.hovered ? Theme.metroBlueHover : Theme.metroBlue))
              : btn.danger ? (btn.pressed ? Theme.dangerSurfacePressed : (btn.hovered ? Theme.metroRed : Theme.dangerSurface))
              : btn.subtle ? (btn.hovered ? Theme.metroTileHover : "transparent")
              : (btn.pressed ? Theme.metroTilePressed : (btn.hovered ? Theme.metroTileHover : Theme.metroTile))
        border.color: !btn.enabled ? Theme.metroStroke
                     : btn.primary ? Theme.metroBlue
                     : btn.danger ? Theme.metroRed
                     : (btn.hovered || btn.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke)
        border.width: btn.hovered || btn.visualFocus ? 2 : 1
    }
}
