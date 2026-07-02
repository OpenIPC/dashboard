pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ScrollView {
    id: root

    property var controller: null

    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: parent.width
        spacing: 14

        OpenIpcPageHeader {
            title: I18n.t("Tools")
            description: I18n.t("Быстрые инструменты OpenIPC: штатные страницы камеры, диагностические входы и полезные справочники.")
        }

        OpenIpcFirmwareBackupPanel {
            controller: root.controller
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            columns: width > 900 ? 2 : 1
            rowSpacing: 12
            columnSpacing: 12

            Repeater {
                model: root.controller ? root.controller.toolRows() : []

                delegate: Rectangle {
                    id: toolRow

                    required property var modelData

                    Layout.fillWidth: true
                    Layout.preferredHeight: 132
                    color: Theme.cardBackground
                    border.color: Theme.cardBorder
                    radius: Theme.radiusLg

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8

                        Text {
                            text: toolRow.modelData.title
                            color: Theme.textPrimary
                            font.bold: true
                            font.pixelSize: 16
                        }

                        Text {
                            Layout.fillWidth: true
                            text: toolRow.modelData.text
                            color: Theme.textMuted
                            wrapMode: Text.WordWrap
                            font.pixelSize: 12
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Item { Layout.fillWidth: true }

                            MajesticButton {
                                text: I18n.t("Открыть")
                                primary: true
                                onClicked: toolRow.modelData.external
                                           ? Qt.openUrlExternally(toolRow.modelData.path)
                                           : root.controller.openWebUiPath(toolRow.modelData.path)
                            }
                        }
                    }
                }
            }
        }
    }
}

