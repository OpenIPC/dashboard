pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var rows: []
    property string summary: ""
    property string state: "warn"

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    Layout.minimumHeight: 154
    Layout.preferredHeight: Math.max(Layout.minimumHeight, content.implicitHeight + 28)
    color: root.state === "block" ? Theme.dangerSurface
          : root.state === "ok" ? Theme.successSurface
          : Theme.warningSurfaceSoft
    border.color: root.state === "block" ? Theme.metroRed
                  : root.state === "ok" ? Theme.metroGreen
                  : Theme.metroAmber
    radius: Theme.metroTileRadius

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                text: I18n.t("Firmware manifest / checksum")
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 17
            }

            Rectangle {
                implicitWidth: statusText.implicitWidth + 18
                implicitHeight: 24
                radius: Theme.metroTileRadius
                color: root.state === "ok" ? Theme.successSurface : (root.state === "block" ? Theme.dangerSurface : Theme.warningSurface)
                border.color: root.state === "ok" ? Theme.metroGreen : (root.state === "block" ? Theme.metroRed : Theme.metroAmber)

                Text {
                    id: statusText
                    anchors.centerIn: parent
                    text: root.state === "ok" ? "OK" : (root.state === "block" ? I18n.t("блок") : I18n.t("проверить"))
                    color: root.state === "ok" ? Theme.success : (root.state === "block" ? Theme.danger : Theme.warning)
                    font.bold: true
                    font.pixelSize: 10
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: root.summary
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width > 860 ? 2 : 1
            rowSpacing: 6
            columnSpacing: 8

            Repeater {
                model: root.rows

                delegate: RowLayout {
                    id: manifestRow

                    required property var modelData

                    Layout.fillWidth: true

                    Text {
                        Layout.preferredWidth: 132
                        text: manifestRow.modelData.label
                        color: Theme.textMuted
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: manifestRow.modelData.value
                        color: manifestRow.modelData.state === "block" ? Theme.danger
                               : manifestRow.modelData.state === "ok" ? Theme.success
                               : Theme.textPrimary
                        font.family: manifestRow.modelData.mono === true ? "Consolas" : Theme.metroFontFamily
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
