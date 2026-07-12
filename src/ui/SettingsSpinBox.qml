import QtQuick
import QtQuick.Controls
import OpenIPC

SpinBox {
    id: control

    editable: true
    implicitHeight: 32
    implicitWidth: 120
    focusPolicy: Qt.StrongFocus
    leftPadding: 30
    rightPadding: 30

    contentItem: TextInput {
        text: control.displayText
        font: control.font
        color: Theme.textSecondary
        selectionColor: Theme.accent
        selectedTextColor: Theme.textPrimary
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhDigitsOnly
    }

    background: Rectangle {
        color: control.enabled ? Theme.panelSoftBackground : Theme.metroTileDisabled
        border.color: control.visualFocus ? Theme.controlBorderStrong : Theme.controlBorder
        border.width: control.visualFocus ? 2 : 1
        radius: Theme.radiusSm
    }

    up.indicator: Rectangle {
        x: control.width - width
        height: control.height
        width: 30
        color: control.up.pressed ? Theme.cardHover : "transparent"

        Text {
            anchors.centerIn: parent
            text: "+"
            color: control.enabled ? Theme.textMuted : Theme.textFaint
            font.pixelSize: 16
        }
    }

    down.indicator: Rectangle {
        x: 0
        height: control.height
        width: 30
        color: control.down.pressed ? Theme.cardHover : "transparent"

        Text {
            anchors.centerIn: parent
            text: "-"
            color: control.enabled ? Theme.textMuted : Theme.textFaint
            font.pixelSize: 16
        }
    }
}
