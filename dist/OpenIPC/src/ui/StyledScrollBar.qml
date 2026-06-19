import QtQuick
import QtQuick.Controls
import OpenIPC

ScrollBar {
    policy: ScrollBar.AsNeeded
    active: true
    size: 0.3 // Default hint
    orientation: Qt.Vertical

    contentItem: Rectangle {
        implicitWidth: 12
        radius: 6
        color: parent.pressed ? Theme.success : (parent.hovered ? Theme.textSecondary : Theme.textMuted)
    }
    background: Rectangle {
        width: parent.width
        color: Theme.panelAltBackground
    }
}
