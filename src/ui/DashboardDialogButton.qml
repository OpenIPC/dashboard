import QtQuick
import QtQuick.Controls
import OpenIPC

Button {
    id: root

    property color buttonColor: Theme.cardBackground
    property color buttonHoverColor: Theme.cardHover
    property color buttonDisabledColor: Theme.controlBackgroundAlt
    property color buttonBorderColor: Theme.controlBorder
    property color buttonDisabledBorderColor: Theme.controlBorder
    property color buttonTextColor: Theme.textPrimary
    property color buttonDisabledTextColor: Theme.textFaint
    property bool buttonTextBold: false

    hoverEnabled: true
    implicitHeight: 32

    background: Rectangle {
        color: root.enabled
               ? (root.down ? root.buttonHoverColor : (root.hovered ? root.buttonHoverColor : root.buttonColor))
               : root.buttonDisabledColor
        radius: Theme.radiusMd
        border.color: root.enabled ? root.buttonBorderColor : root.buttonDisabledBorderColor
        border.width: 1
    }

    contentItem: Text {
        text: root.text
        color: root.enabled ? root.buttonTextColor : root.buttonDisabledTextColor
        font.pixelSize: 13
        font.bold: root.buttonTextBold && root.enabled
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
