import QtQuick

Rectangle {
    id: root

    property bool active: false
    property color activeColor: "red"

    width: 12
    height: 12
    radius: 6
    color: activeColor
    visible: active

    SequentialAnimation on opacity {
        loops: Animation.Infinite
        running: root.active

        PropertyAnimation { to: 0.2; duration: 800 }
        PropertyAnimation { to: 1.0; duration: 800 }
    }
}
