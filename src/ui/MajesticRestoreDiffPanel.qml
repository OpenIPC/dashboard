pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var rows: []
    property var changes: []
    property string summary: ""

    Layout.fillWidth: true
    Layout.minimumHeight: 172
    Layout.preferredHeight: Math.max(Layout.minimumHeight, content.implicitHeight + 28)
    color: Theme.metroSurfaceAlt
    border.color: Theme.metroStroke
    radius: Theme.metroTileRadius

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: I18n.t("Restore diff")
            color: Theme.textPrimary
            font.bold: true
            font.pixelSize: 14
        }

        Text {
            Layout.fillWidth: true
            text: root.summary
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width > 680 ? 4 : 2
            rowSpacing: 6
            columnSpacing: 6

            Repeater {
                model: root.rows

                delegate: Rectangle {
                    id: riskPill

                    required property var modelData

                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    color: riskPill.modelData.state === "block" ? Theme.dangerSurface
                          : riskPill.modelData.state === "warn" ? Theme.warningSurface
                          : Theme.cardBackground
                    border.color: riskPill.modelData.state === "block" ? Theme.metroRed
                                  : riskPill.modelData.state === "warn" ? Theme.metroAmber
                                  : Theme.metroStroke
                    radius: Theme.metroTileRadius

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8

                        Text {
                            Layout.fillWidth: true
                            text: riskPill.modelData.label
                            color: Theme.textSecondary
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }

                        Text {
                            text: riskPill.modelData.value
                            color: riskPill.modelData.state === "block" ? Theme.danger
                                   : riskPill.modelData.state === "warn" ? Theme.warning
                                   : Theme.textPrimary
                            font.bold: true
                            font.pixelSize: 13
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            Repeater {
                model: root.changes.slice(0, 6)

                delegate: RowLayout {
                    id: changeRow

                    required property var modelData

                    Layout.fillWidth: true

                    Text {
                        Layout.preferredWidth: 142
                        text: changeRow.modelData.path
                        color: Theme.textPrimary
                        font.family: "Consolas"
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: String(changeRow.modelData.before) + " -> " + String(changeRow.modelData.after)
                        color: Theme.textMuted
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
