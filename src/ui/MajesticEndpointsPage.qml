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
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 16

            Text {
                Layout.fillWidth: true
                text: I18n.t("Полезные точки доступа Majestic для этой камеры")
                color: Theme.textPrimary
                font.pixelSize: 16
                font.bold: true
            }

            MajesticButton {
                text: I18n.t("Обновить")
                enabled: root.controller && !root.controller.loading
                onClicked: root.controller.refresh()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            spacing: 10

            MajesticEndpointProbeCard {
                controller: root.controller
                title: I18n.t("HD RTSP")
                state: root.controller ? root.controller.rtspMainProbeState : "idle"
                message: root.controller ? root.controller.rtspMainProbeMessage : ""
                elapsedMs: root.controller ? root.controller.rtspMainProbeElapsedMs : 0
                buttonText: I18n.t("Проверить HD")
                onRun: root.controller.startEndpointProbe("main")
            }

            MajesticEndpointProbeCard {
                controller: root.controller
                title: I18n.t("SD RTSP")
                state: root.controller ? root.controller.rtspSubProbeState : "idle"
                message: root.controller ? root.controller.rtspSubProbeMessage : ""
                elapsedMs: root.controller ? root.controller.rtspSubProbeElapsedMs : 0
                buttonText: I18n.t("Проверить SD")
                onRun: root.controller.startEndpointProbe("sub")
            }

            MajesticEndpointProbeCard {
                controller: root.controller
                title: I18n.t("Majestic API")
                state: root.controller ? root.controller.majesticApiProbeState : "idle"
                message: root.controller ? root.controller.majesticApiProbeMessage : ""
                elapsedMs: root.controller ? root.controller.majesticApiProbeElapsedMs : 0
                buttonText: I18n.t("Проверить API")
                onRun: root.controller.startEndpointProbe("api")
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            columns: width > 900 ? 4 : 2
            rowSpacing: 10
            columnSpacing: 10

            Repeater {
                model: root.controller ? root.controller.endpointSummaryRows() : []

                delegate: MajesticStatusCard {
                    required property var modelData

                    title: modelData.title
                    value: modelData.value
                    subtitle: modelData.subtitle
                    percent: 100
                    accent: Theme.accent
                }
            }
        }

        Repeater {
            model: root.controller ? root.controller.endpointRows() : []

            delegate: Rectangle {
                id: endpointDelegate

                required property var modelData

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.preferredHeight: 82
                color: Theme.cardBackground
                border.color: Theme.cardBorder
                radius: Theme.radiusLg

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 92
                        Layout.preferredHeight: 26
                        radius: 13
                        color: "#172554"
                        border.color: Theme.accent

                        Text {
                            anchors.centerIn: parent
                            text: endpointDelegate.modelData.group
                            color: Theme.accentHover
                            font.pixelSize: 11
                            font.bold: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: endpointDelegate.modelData.name
                            color: Theme.textPrimary
                            font.bold: true
                            font.pixelSize: 12
                        }

                        Text {
                            Layout.fillWidth: true
                            text: endpointDelegate.modelData.value
                            color: Theme.textSecondary
                            font.family: "Consolas"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: endpointDelegate.modelData.hint
                            color: Theme.textMuted
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }

                    MajesticButton {
                        text: I18n.t("Копировать")
                        subtle: true
                        onClicked: root.controller.copyEndpoint(endpointDelegate.modelData)
                    }

                    MajesticButton {
                        text: I18n.t("Открыть")
                        enabled: endpointDelegate.modelData.openable === true
                        onClicked: root.controller.openEndpoint(endpointDelegate.modelData)
                    }
                }
            }
        }
    }
}

