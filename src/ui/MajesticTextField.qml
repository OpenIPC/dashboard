import QtQuick
import QtQuick.Controls
import OpenIPC

TextField {
    id: input
    color: Theme.textPrimary
    placeholderTextColor: Theme.textFaint
    selectionColor: Theme.accent
    selectedTextColor: Theme.textPrimary
    font.pixelSize: 12
    background: Rectangle {
        radius: Theme.radiusMd
        color: Theme.controlBackground
        border.color: input.activeFocus ? Theme.accent : Theme.controlBorder
    }
}
