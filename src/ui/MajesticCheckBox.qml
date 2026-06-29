import QtQuick
import QtQuick.Controls
import OpenIPC

CheckBox {
    id: check
    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: check.leftPadding
        y: check.height / 2 - height / 2
        radius: 4
        color: check.checked ? Theme.accent : Theme.controlBackground
        border.color: check.checked ? Theme.accentHover : (check.hovered ? Theme.accent : Theme.controlBorder)
        Text {
            anchors.centerIn: parent
            text: "✓"
            visible: check.checked
            color: Theme.textPrimary
            font.pixelSize: 11
            font.bold: true
        }
    }
    contentItem: Text {
        text: check.text
        color: check.enabled ? Theme.textSecondary : Theme.textFaint
        font.pixelSize: 12
        verticalAlignment: Text.AlignVCenter
        leftPadding: check.indicator.width + check.spacing
    }
}
