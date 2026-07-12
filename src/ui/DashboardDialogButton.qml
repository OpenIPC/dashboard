import QtQuick
import QtQuick.Controls
import OpenIPC

Button {
    id: root

    property color buttonColor: Theme.metroTile
    property color buttonHoverColor: Theme.metroTileHover
    property color buttonDisabledColor: Theme.controlBackgroundAlt
    property color buttonBorderColor: Theme.metroStroke
    property color buttonDisabledBorderColor: Theme.metroStroke
    property color buttonTextColor: Theme.textPrimary
    property color buttonDisabledTextColor: Theme.textFaint
    property bool buttonTextBold: false

    hoverEnabled: true
    implicitHeight: 36

    background: Rectangle {
        color: root.enabled
               ? (root.down ? root.buttonHoverColor : (root.hovered ? root.buttonHoverColor : root.buttonColor))
               : root.buttonDisabledColor
        radius: Theme.metroTileRadius
        border.color: root.enabled && (root.hovered || root.visualFocus) ? Theme.metroStrokeStrong
                      : (root.enabled ? root.buttonBorderColor : root.buttonDisabledBorderColor)
        border.width: root.hovered || root.visualFocus ? 2 : 1
    }

    contentItem: Text {
        text: root.text
        color: root.enabled ? root.buttonTextColor : root.buttonDisabledTextColor
        font.family: Theme.metroFontFamily
        font.pixelSize: 12
        font.bold: root.buttonTextBold && root.enabled
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
