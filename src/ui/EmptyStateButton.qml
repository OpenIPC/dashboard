import QtQuick
import QtQuick.Controls
import OpenIPC

Button {
    id: root

    property color buttonColor: Theme.controlBackground
    property color buttonHoverColor: Theme.cardHover
    property color buttonTextColor: Theme.textPrimary

    implicitHeight: 36
    leftPadding: 14
    rightPadding: 14

    background: Rectangle {
        color: root.enabled ? (root.hovered ? root.buttonHoverColor : root.buttonColor) : Theme.controlBackgroundAlt
        radius: Theme.radiusMd
        border.color: root.enabled ? Theme.controlBorderStrong : Theme.controlBorder
        border.width: 1
    }

    contentItem: Text {
        text: root.text
        color: root.enabled ? root.buttonTextColor : Theme.textMuted
        font.pixelSize: 13
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
