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

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            columns: width > 900 ? 3 : 1
            rowSpacing: 10
            columnSpacing: 10

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

        OpenIpcSafeActionsPanel {
            rows: root.controller ? root.controller.safeActionRows() : []
            title: I18n.t("Action gates")
            description: I18n.t("Эти gates объясняют, почему часть endpoints только справочная, а часть доступна для выполнения из Control Center.")
        }

        Repeater {
            model: root.controller ? root.controller.endpointRows() : []

            delegate: Rectangle {
                id: endpointDelegate

                required property var modelData

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.minimumHeight: 96
                Layout.preferredHeight: Math.max(Layout.minimumHeight, endpointContent.implicitHeight + 24)
                color: Theme.cardBackground
                border.color: endpointDelegate.modelData.state === "block" ? Theme.metroRed
                              : endpointDelegate.modelData.state === "warn" ? Theme.metroAmber
                              : endpointDelegate.modelData.state === "ok" ? Theme.metroGreen
                              : Theme.cardBorder
                radius: Theme.radiusLg

                RowLayout {
                    id: endpointContent

                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 92
                        Layout.preferredHeight: 26
                        radius: 13
                        color: Theme.metroDeepBlue
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
                        spacing: 4

                        Text {
                            text: endpointDelegate.modelData.name
                            color: Theme.textPrimary
                            font.bold: true
                            font.pixelSize: 12
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 6

                            Rectangle {
                                implicitWidth: statusText.implicitWidth + 16
                                implicitHeight: 22
                                radius: Theme.metroTileRadius
                                color: endpointDelegate.modelData.state === "block" ? Theme.dangerSurface
                                      : endpointDelegate.modelData.state === "warn" ? Theme.warningSurface
                                      : endpointDelegate.modelData.state === "ok" ? Theme.successSurface
                                      : Theme.metroSurfaceAlt
                                border.color: endpointDelegate.modelData.state === "block" ? Theme.metroRed
                                              : endpointDelegate.modelData.state === "warn" ? Theme.metroAmber
                                              : endpointDelegate.modelData.state === "ok" ? Theme.metroGreen
                                              : Theme.metroStroke

                                Text {
                                    id: statusText
                                    anchors.centerIn: parent
                                    text: endpointDelegate.modelData.statusText
                                    color: endpointDelegate.modelData.state === "block" ? Theme.danger
                                           : endpointDelegate.modelData.state === "warn" ? Theme.warning
                                           : endpointDelegate.modelData.state === "ok" ? Theme.success
                                           : Theme.textSecondary
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }

                            Rectangle {
                                implicitWidth: riskText.implicitWidth + 16
                                implicitHeight: 22
                                radius: Theme.metroTileRadius
                                color: endpointDelegate.modelData.risk === "danger" ? Theme.dangerSurface
                                      : endpointDelegate.modelData.risk === "warn" ? Theme.warningSurface
                                      : Theme.metroSurfaceAlt
                                border.color: endpointDelegate.modelData.risk === "danger" ? Theme.metroRed
                                              : endpointDelegate.modelData.risk === "warn" ? Theme.metroAmber
                                              : Theme.metroStroke

                                Text {
                                    id: riskText
                                    anchors.centerIn: parent
                                    text: endpointDelegate.modelData.risk === "danger" ? I18n.t("danger")
                                          : endpointDelegate.modelData.risk === "warn" ? I18n.t("write/action")
                                          : I18n.t("read-only")
                                    color: endpointDelegate.modelData.risk === "danger" ? Theme.danger
                                           : endpointDelegate.modelData.risk === "warn" ? Theme.warning
                                           : Theme.textSecondary
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }
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

