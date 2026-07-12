pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ScrollView {
    id: root

    property var controller: null
    property alias snapshotWidth: backupRestorePanel.snapshotWidth
    property alias snapshotHeight: backupRestorePanel.snapshotHeight
    property alias snapshotQuality: backupRestorePanel.snapshotQuality
    property alias snapshotGray: backupRestorePanel.snapshotGray

    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: parent.width
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 16
            spacing: 10

            Text {
                Layout.fillWidth: true
                text: I18n.t("Device Status")
                color: Theme.textPrimary
                font.family: Theme.metroFontFamily
                font.pixelSize: 28
                font.bold: true
            }

            Rectangle {
                Layout.preferredWidth: 128
                Layout.preferredHeight: 26
                radius: Theme.metroTileRadius
                color: root.controller && root.controller.statusError ? Theme.dangerSurface : Theme.successSurface

                Text {
                    anchors.centerIn: parent
                    text: root.controller && root.controller.statusError ? I18n.t("Требует внимания") : I18n.t("All systems OK")
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.bold: true
                    font.pixelSize: 11
                }
            }

            MajesticButton {
                text: I18n.t("Обновить")
                enabled: root.controller && !root.controller.loading
                onClicked: {
                    root.controller.refresh()
                    root.controller.refreshOverviewMetrics()
                }
            }
        }

        OpenIpcLogsPanel {
            controller: root.controller
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            columns: width > 900 ? 4 : 2
            rowSpacing: 12
            columnSpacing: 12

            MajesticStatusCard {
                title: "LOAD"
                value: root.controller ? String(root.controller.metric("node_load1", "—")) : "—"
                subtitle: I18n.t("CPU load average")
                percent: root.controller ? Math.min(100, Number(root.controller.metric("node_load1", 0)) * 35) : 0
                accent: Theme.metroBlue
            }

            MajesticStatusCard {
                title: "MEMORY"
                value: root.controller ? root.controller.ramPercent() + "%" : "—"
                subtitle: root.controller ? root.controller.ramText() : ""
                percent: root.controller ? root.controller.ramPercent() : 0
                accent: Theme.metroBlue
            }

            MajesticStatusCard {
                title: "TEMPERATURE"
                value: root.controller ? root.controller.tempText() : "—"
                subtitle: "SoC"
                percent: root.controller ? Math.min(100, Number(root.controller.metric("node_hwmon_temp_celsius", 0)) / 90 * 100) : 0
                accent: Theme.metroOrange
            }

            MajesticStatusCard {
                title: "UPTIME"
                value: root.controller ? root.controller.uptimeText() : "—"
                subtitle: "Majestic / node metrics"
                percent: 100
                accent: Theme.metroGreen
            }
        }

        OpenIpcSafeActionsPanel {
            rows: root.controller ? root.controller.safeActionRows() : []
            title: I18n.t("Safe capabilities / actions")
            description: I18n.t("Карта доступных действий Control Center с текущими safety-gates и причинами блокировки.")
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            columns: width > 900 ? 2 : 1
            rowSpacing: 12
            columnSpacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 232
                Layout.preferredHeight: Math.max(Layout.minimumHeight, streamsContent.implicitHeight + 32)
                color: Theme.metroSurface
                border.color: Theme.metroStroke
                radius: Theme.metroTileRadius

                ColumnLayout {
                    id: streamsContent

                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Text {
                        text: I18n.t("Streams")
                        color: Theme.textPrimary
                        font.family: Theme.metroFontFamily
                        font.bold: true
                        font.pixelSize: 18
                    }

                    Repeater {
                        model: root.controller ? root.controller.streamRows() : []

                        delegate: ColumnLayout {
                            id: streamRow

                            required property var modelData

                            Layout.fillWidth: true

                            RowLayout {
                                Rectangle {
                                    Layout.preferredWidth: 56
                                    Layout.preferredHeight: 24
                                    radius: Theme.metroTileRadius
                                    color: Theme.metroBlue

                                    Text {
                                        anchors.centerIn: parent
                                        text: streamRow.modelData.name
                                        color: Theme.textPrimary
                                        font.bold: true
                                        font.pixelSize: 11
                                    }
                                }

                                Text {
                                    text: streamRow.modelData.size
                                    color: Theme.textPrimary
                                    font.bold: true
                                    font.pixelSize: 14
                                }

                                Rectangle {
                                    visible: streamRow.modelData.codec.length > 0
                                    Layout.preferredWidth: 56
                                    Layout.preferredHeight: 22
                                    radius: Theme.metroTileRadius
                                    color: "#f8fafc"

                                    Text {
                                        anchors.centerIn: parent
                                        text: streamRow.modelData.codec
                                        color: "#0f172a"
                                        font.bold: true
                                        font.pixelSize: 10
                                    }
                                }
                            }

                            Text {
                                text: (streamRow.modelData.fps ? streamRow.modelData.fps + " fps" : "")
                                      + (streamRow.modelData.bitrate ? " · " + streamRow.modelData.bitrate + " kbit/s" : "")
                                color: Theme.textMuted
                                font.pixelSize: 11
                            }
                        }
                    }

                    Text {
                        visible: !(root.controller && root.controller.streamRows().length > 0)
                        text: I18n.t("Нет включённых потоков")
                        color: Theme.textMuted
                        font.pixelSize: 12
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: Theme.metroStroke
                    }

                    Text {
                        text: root.controller
                              ? ((root.controller.metric("night_enabled", 0) ? "🌙 " + I18n.t("Ночь") : "☀ " + I18n.t("День"))
                                 + " · IR-cut " + (root.controller.metric("ircut_enabled", 0) ? "on" : "off")
                                 + " · HLS " + root.controller.metric("hls_clients_total", 0))
                              : ""
                        color: Theme.textSecondary
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 232
                Layout.preferredHeight: Math.max(Layout.minimumHeight, capabilitiesContent.implicitHeight + 32)
                color: Theme.metroSurface
                border.color: Theme.metroStroke
                radius: Theme.metroTileRadius

                ColumnLayout {
                    id: capabilitiesContent

                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Text {
                        text: I18n.t("Возможности этой камеры")
                        color: Theme.textPrimary
                        font.family: Theme.metroFontFamily
                        font.bold: true
                        font.pixelSize: 18
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        rowSpacing: 8
                        columnSpacing: 18

                        Repeater {
                            model: root.controller ? root.controller.capabilityRows() : []

                            delegate: RowLayout {
                                id: capabilityRow

                                required property var modelData

                                Layout.fillWidth: true

                                Rectangle {
                                    Layout.preferredWidth: 9
                                    Layout.preferredHeight: 9
                                    radius: 5
                                    color: capabilityRow.modelData.value ? Theme.metroGreen : Theme.textFaint
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: capabilityRow.modelData.label
                                    color: Theme.textSecondary
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }

                    Text {
                        text: I18n.t("Доступно параметров: %1", [root.controller ? root.controller.fields.length : 0])
                        color: Theme.textMuted
                        font.pixelSize: 11
                    }

                    Text {
                        text: root.controller ? root.controller.cameraHost + ":" + root.controller.cameraPort : ""
                        color: Theme.metroBlue
                        font.family: "Consolas"
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 244
                Layout.preferredHeight: Math.max(Layout.minimumHeight, mechanicsContent.implicitHeight + 28)
                color: Theme.metroSurface
                border.color: Theme.metroStroke
                radius: Theme.metroTileRadius

                ColumnLayout {
                    id: mechanicsContent

                    anchors.fill: parent
                    anchors.margins: 14

                    Text {
                        text: I18n.t("День / ночь и механика")
                        color: Theme.textPrimary
                        font.family: Theme.metroFontFamily
                        font.bold: true
                        font.pixelSize: 15
                    }

                    Text {
                        text: I18n.t("Команды выполняются сразу и не меняют majestic.yaml")
                        color: Theme.textMuted
                        font.pixelSize: 11
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        MajesticButton { text: I18n.t("День"); enabled: root.controller !== null; onClicked: root.controller.setNightMode("off") }
                        MajesticButton { text: I18n.t("Ночь"); enabled: root.controller !== null; onClicked: root.controller.setNightMode("on") }
                        MajesticButton { text: I18n.t("Переключить"); primary: true; enabled: root.controller !== null; onClicked: root.controller.setNightMode("toggle") }
                        MajesticButton { text: I18n.t("IR-cut"); enabled: root.controller !== null; onClicked: root.controller.setNightMode("ircut") }
                        MajesticButton { text: I18n.t("ИК-подсветка"); enabled: root.controller !== null; onClicked: root.controller.setNightMode("light") }
                    }
                }
            }

            MajesticBackupRestorePanel {
                id: backupRestorePanel
                controller: root.controller
            }
        }
    }
}

