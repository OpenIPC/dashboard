import QtQuick
import QtQuick.Controls
import OpenIPC

ScrollBar {
    id: scrollBar

    policy: ScrollBar.AsNeeded
    active: true
    size: 0.3 // Default hint
    orientation: Qt.Vertical

    contentItem: Rectangle {
        implicitWidth: 12
        radius: 6
        color: scrollBar.pressed ? Theme.success : (scrollBar.hovered ? Theme.textSecondary : Theme.textMuted)
    }
    background: Rectangle {
        width: scrollBar.width
        color: Theme.panelAltBackground
    }
}
