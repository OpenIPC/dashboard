import QtQuick
import QtQuick.Layouts
import OpenIPC

RowLayout {
    id: root

    property string title: ""
    property string description: ""
    default property alias actions: actionsRow.data

    Layout.fillWidth: true
    Layout.margins: 16
    spacing: 12

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 4

        Text {
            text: root.title
            color: Theme.accentHover
            font.pixelSize: 28
            font.bold: true
        }

        Text {
            Layout.fillWidth: true
            text: root.description
            color: Theme.textMuted
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    RowLayout {
        id: actionsRow
        spacing: 8
    }
}
