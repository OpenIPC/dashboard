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
    color: Theme.metroSurfaceAlt
    radius: Theme.metroTileRadius
    border.color: Theme.metroStroke
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
            font.family: Theme.metroFontFamily
            font.pixelSize: 12
            font.bold: true
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.preferredWidth: Math.max(30, groupCountText.implicitWidth + 16)
            Layout.preferredHeight: 20
            radius: Theme.metroTileRadius
            color: Theme.metroSurface
            border.color: Theme.metroStroke

            Text {
                id: groupCountText
                anchors.centerIn: parent
                text: root.count
                color: Theme.textSecondary
                font.family: Theme.metroFontFamily
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
