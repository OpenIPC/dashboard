import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property string title: ""
    property var rows: []

    Layout.fillWidth: true
    Layout.minimumHeight: 252
    Layout.preferredHeight: Math.max(Layout.minimumHeight, infoRowsContent.implicitHeight + 32)
    color: Theme.metroSurface
    border.color: Theme.metroStroke
    radius: Theme.metroTileRadius

    ColumnLayout {
        id: infoRowsContent

        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
            text: root.title
            color: Theme.textPrimary
            font.family: Theme.metroFontFamily
            font.bold: true
            font.pixelSize: 18
        }

        Repeater {
            model: root.rows

            delegate: RowLayout {
                id: rowDelegate

                required property var modelData

                Layout.fillWidth: true
                spacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    Text {
                        text: rowDelegate.modelData.label
                        color: Theme.textPrimary
                        font.family: Theme.metroFontFamily
                        font.bold: true
                        font.pixelSize: 12
                    }

                    Text {
                        Layout.fillWidth: true
                        text: rowDelegate.modelData.hint
                        color: Theme.textMuted
                        font.family: Theme.metroFontFamily
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                }

                Text {
                    Layout.preferredWidth: 210
                    text: rowDelegate.modelData.value
                    color: Theme.metroBlue
                    font.family: "Consolas"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignRight
                    elide: Text.ElideRight
                }
            }
        }
    }
}
