import QtQuick
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: root

    property string title: ""
    property string description: ""
    default property alias actions: actionsFlow.data

    Layout.fillWidth: true
    Layout.margins: 16
    spacing: 10

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 4

        Text {
            text: root.title
            color: Theme.textPrimary
            font.family: Theme.metroFontFamily
            font.pixelSize: 26
            font.bold: true
        }

        Text {
            Layout.fillWidth: true
            text: root.description
            color: Theme.textMuted
            font.family: Theme.metroFontFamily
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    Flow {
        id: actionsFlow
        Layout.fillWidth: true
        spacing: 8
    }
}
