import QtQuick
import QtQuick.Controls
import OpenIPC

Slider {
    id: control

    property color activeColor: Theme.metroBlue
    property color inactiveColor: Theme.metroStroke
    property color markerColor: Theme.textFaint
    property bool showCenterMarker: false

    implicitHeight: 28
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: 180
        implicitHeight: 4
        width: control.availableWidth
        height: implicitHeight
        radius: Theme.metroRadius
        color: control.enabled ? control.inactiveColor : Theme.metroTileDisabled

        Rectangle {
            width: parent.width * control.visualPosition
            height: parent.height
            radius: parent.radius
            color: control.enabled ? control.activeColor : Theme.textFaint
        }

        Rectangle {
            visible: control.showCenterMarker
            x: Math.round(parent.width / 2)
            width: 1
            height: 10
            anchors.verticalCenter: parent.verticalCenter
            color: control.markerColor
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: control.pressed || control.hovered ? 16 : 14
        implicitHeight: control.pressed || control.hovered ? 18 : 16
        radius: Theme.metroTileRadius
        color: control.enabled
               ? (control.pressed ? Theme.metroBlueHover : Theme.metroBlue)
               : Theme.metroTileDisabled
        border.color: control.hovered || control.visualFocus ? Theme.textPrimary : Theme.metroStrokeStrong
        border.width: control.hovered || control.visualFocus ? 1 : 0
    }
}
