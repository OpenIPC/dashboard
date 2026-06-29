import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property string title: ""
    property string value: "0"
    property color accentColor: Theme.accent

    Layout.fillWidth: true
    implicitHeight: 54
    radius: Theme.radiusMd
    color: Theme.panelSoftBackground
    border.color: Theme.controlBorder
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 2

        Text {
            text: root.value
            color: root.accentColor
            font.pixelSize: 16
            font.bold: true
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Text {
            text: root.title
            color: Theme.textMuted
            font.pixelSize: 10
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
}
