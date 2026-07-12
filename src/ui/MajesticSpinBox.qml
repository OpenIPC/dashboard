import QtQuick
import QtQuick.Controls
import OpenIPC

SpinBox {
    id: spin
    implicitHeight: 36
    contentItem: TextInput {
        z: 2
        text: String(spin.value)
        color: Theme.textPrimary
        selectionColor: Theme.metroBlue
        selectedTextColor: Theme.textPrimary
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !spin.editable
        validator: spin.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
        font.family: Theme.metroFontFamily
        font.pixelSize: 12
    }
    up.indicator: Rectangle {
        x: spin.mirrored ? 0 : parent.width - width
        y: 1
        width: 24
        height: parent.height / 2 - 1
        radius: Theme.metroTileRadius
        color: spin.up.pressed ? Theme.metroTileHover : "transparent"
        Text { anchors.centerIn: parent; text: "+"; color: Theme.textMuted; font.pixelSize: 11 }
    }
    down.indicator: Rectangle {
        x: spin.mirrored ? 0 : parent.width - width
        y: parent.height / 2
        width: 24
        height: parent.height / 2 - 1
        radius: Theme.metroTileRadius
        color: spin.down.pressed ? Theme.metroTileHover : "transparent"
        Text { anchors.centerIn: parent; text: "-"; color: Theme.textMuted; font.pixelSize: 11 }
    }
    background: Rectangle {
        radius: Theme.metroTileRadius
        color: spin.enabled ? Theme.controlBackground : Theme.metroTileDisabled
        border.color: spin.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
        border.width: spin.activeFocus ? 2 : 1
    }
}
