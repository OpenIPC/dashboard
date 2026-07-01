pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property string title: ""
    property int count: 0
    property bool interactive: false

    signal contextRequested()

    Layout.fillWidth: true
    Layout.preferredHeight: 34
    color: Theme.controlBackgroundAlt
    radius: Theme.radiusMd
    border.color: Theme.panelBorder
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 8
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: root.title
            color: Theme.textSecondary
            font.pixelSize: 12
            font.bold: true
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.preferredWidth: Math.max(30, groupCountText.implicitWidth + 16)
            Layout.preferredHeight: 20
            radius: 10
            color: Theme.panelSoftBackground
            border.color: Theme.controlBorder

            Text {
                id: groupCountText
                anchors.centerIn: parent
                text: root.count
                color: Theme.textSecondary
                font.pixelSize: 11
                font.bold: true
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        enabled: root.interactive
        cursorShape: root.interactive ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton)
                root.contextRequested()
        }
    }
}
