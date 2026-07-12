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
    radius: Theme.metroTileRadius
    color: Theme.metroSurface
    border.color: Theme.metroStroke
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 7
        anchors.bottomMargin: 7
        spacing: 2

        Text {
            text: root.value
            color: root.accentColor
            font.family: Theme.metroFontFamily
            font.pixelSize: 18
            font.bold: true
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Text {
            text: root.title
            color: Theme.textMuted
            font.family: Theme.metroFontFamily
            font.pixelSize: 10
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
}
