pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: root

    property var controller: null

    spacing: 10

    RowLayout {
        Layout.fillWidth: true
        Layout.margins: 12

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                text: I18n.t("Prometheus-метрики Majestic")
                color: Theme.textPrimary
                font.pixelSize: 18
                font.bold: true
            }

            Text {
                Layout.fillWidth: true
                text: I18n.t("Ключевые показатели encoder, sensor, streaming и runtime")
                color: Theme.textMuted
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }

        MajesticButton {
            text: root.controller && root.controller.activeMetricsId.length ? I18n.t("Обновление…") : I18n.t("Обновить метрики")
            primary: true
            enabled: root.controller && !root.controller.activeMetricsId.length
            onClicked: root.controller.refreshMetrics()
        }
    }

    GridLayout {
        Layout.fillWidth: true
        Layout.leftMargin: 12
        Layout.rightMargin: 12
        columns: width > 900 ? 4 : 2
        rowSpacing: 10
        columnSpacing: 10

        Repeater {
            model: root.controller ? root.controller.metricsOverviewRows() : []

            delegate: MajesticStatusCard {
                required property var modelData

                title: modelData.title
                value: modelData.value
                subtitle: modelData.subtitle
                percent: modelData.percent
                accent: modelData.accent
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: 12
        Layout.rightMargin: 12
        spacing: 8

        Repeater {
            model: root.controller ? root.controller.metricsHealthRows() : []

            delegate: Rectangle {
                id: healthRow

                required property var modelData

                Layout.fillWidth: true
                Layout.preferredHeight: 58
                color: Theme.cardBackground
                border.color: healthRow.modelData.color
                radius: Theme.radiusMd

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: 10
                        radius: 5
                            color: healthRow.modelData.color
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            Layout.fillWidth: true
                            text: healthRow.modelData.title
                            color: Theme.textPrimary
                            font.bold: true
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: healthRow.modelData.text
                            color: Theme.textMuted
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.leftMargin: 12
        Layout.rightMargin: 12
        spacing: 10

        TextField {
            Layout.fillWidth: true
            implicitHeight: 34
            text: root.controller ? root.controller.metricsFilterText : ""
            placeholderText: I18n.t("Фильтр raw-метрик…")
            color: Theme.textPrimary
            placeholderTextColor: Theme.textMuted
            selectionColor: Theme.accent
            selectedTextColor: Theme.textPrimary
            leftPadding: 12
            rightPadding: 12
            onTextChanged: if (root.controller) root.controller.metricsFilterText = text

            background: Rectangle {
                color: Theme.controlBackground
                radius: Theme.radiusMd
                border.color: parent.activeFocus ? Theme.accent : Theme.controlBorder
            }
        }

        Text {
            text: I18n.t("Raw Prometheus")
            color: Theme.textMuted
            font.pixelSize: 12
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 12
        Layout.topMargin: 0

        TextArea {
            text: root.controller ? root.controller.filteredMetricsText() : ""
            readOnly: true
            color: Theme.textSecondary
            font.family: "Consolas"
            font.pixelSize: 11
            wrapMode: TextEdit.NoWrap

            background: Rectangle {
                color: Theme.controlBackground
                border.color: Theme.controlBorder
                radius: Theme.radiusMd
            }
        }
    }
}

