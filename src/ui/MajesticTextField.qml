import QtQuick
import QtQuick.Controls
import OpenIPC

TextField {
    id: input
    implicitHeight: 36
    leftPadding: 10
    rightPadding: 10
    color: Theme.textPrimary
    placeholderTextColor: Theme.textFaint
    selectionColor: Theme.metroBlue
    selectedTextColor: Theme.textPrimary
    font.family: Theme.metroFontFamily
    font.pixelSize: 12
    background: Rectangle {
        radius: Theme.metroTileRadius
        color: input.enabled ? Theme.controlBackground : Theme.metroTileDisabled
        border.color: input.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
        border.width: input.activeFocus ? 2 : 1
    }
}
