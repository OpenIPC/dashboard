import QtQuick
import QtQuick.Controls

ScrollBar {
    policy: ScrollBar.AsNeeded
    active: true
    size: 0.3 // Default hint
    orientation: Qt.Vertical

    contentItem: Rectangle {
        implicitWidth: 12
        radius: 6
        color: parent.pressed ? "#4caf50" : (parent.hovered ? "#cccccc" : "#a0a0a0")
    }
    background: Rectangle {
        width: parent.width
        color: "#2d2d2d" // Lighter track color
    }
}
