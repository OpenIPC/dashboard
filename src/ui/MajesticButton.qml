import QtQuick
import QtQuick.Controls
import OpenIPC

Button {
    id: btn
    property bool primary: false
    property bool danger: false
    property bool subtle: false
    implicitHeight: 34
    leftPadding: 14
    rightPadding: 14
    contentItem: Text {
        text: btn.text
        color: btn.enabled ? Theme.textPrimary : Theme.textFaint
        font.pixelSize: 12
        font.bold: btn.primary
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: Theme.radiusMd
        color: !btn.enabled ? Theme.controlBackgroundAlt
              : btn.primary ? (btn.pressed ? "#1d4ed8" : (btn.hovered ? Theme.accentHover : Theme.accent))
              : btn.danger ? (btn.pressed ? "#991b1b" : (btn.hovered ? "#ef4444" : "#7f1d1d"))
              : btn.subtle ? (btn.hovered ? Theme.cardHover : "transparent")
              : (btn.hovered ? Theme.cardHover : Theme.controlBackground)
        border.color: btn.primary ? Theme.accentHover
                     : btn.danger ? Theme.danger
                     : (btn.hovered ? Theme.accent : Theme.controlBorder)
    }
}
