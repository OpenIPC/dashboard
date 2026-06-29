import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import OpenIPC

RowLayout {
    id: root

    spacing: 4

    Rectangle {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        radius: Theme.radiusMd
        color: minMouse.containsMouse ? "#3e4654" : "#2d3442"
        border.color: "#3c4353"

        Text {
            anchors.centerIn: parent
            text: "—"
            color: Theme.textPrimary
            font.pixelSize: 14
            font.bold: true
        }

        MouseArea {
            id: minMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: Window.window.showMinimized()
        }
    }

    Rectangle {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        radius: Theme.radiusMd
        color: maxMouse.containsMouse ? "#3e4654" : "#2d3442"
        border.color: "#3c4353"

        Text {
            anchors.centerIn: parent
            text: Window.window.visibility === Window.Maximized ? "❐" : "☐"
            color: Theme.textPrimary
            font.pixelSize: 14
        }

        MouseArea {
            id: maxMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (Window.window.visibility === Window.Maximized)
                    Window.window.showNormal()
                else
                    Window.window.showMaximized()
            }
        }
    }

    Rectangle {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        radius: Theme.radiusMd
        color: closeMouse.containsMouse ? "#c42b1c" : "#2d3442"
        border.color: closeMouse.containsMouse ? "#c42b1c" : "#3c4353"

        Text {
            anchors.centerIn: parent
            text: "✕"
            color: Theme.textPrimary
            font.pixelSize: 14
        }

        MouseArea {
            id: closeMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: Window.window.close()
        }
    }
}
