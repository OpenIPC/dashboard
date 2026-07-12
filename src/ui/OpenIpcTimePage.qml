pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ScrollView {
    id: root

    property var controller: null
    property alias zoneName: timeZoneName.text
    property alias zoneData: timeZoneData.text
    property alias server0: timeServer0.text
    property alias server1: timeServer1.text
    property alias server2: timeServer2.text
    property alias server3: timeServer3.text

    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: parent.width
        spacing: 14

        OpenIpcPageHeader {
            title: I18n.t("Time")
            description: I18n.t("Сравнение времени камеры и Dashboard. Корректное время важно для OSD, архива, событий и TLS.")

            MajesticButton {
                text: I18n.t("Загрузить")
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.loadFirmwareTime()
            }

            MajesticButton {
                text: I18n.t("Сохранить")
                primary: true
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.openFirmwareTimeConfirm()
            }

            MajesticButton {
                text: I18n.t("Открыть Time WebUI")
                enabled: root.controller !== null
                onClicked: root.controller.openWebUiPath("/cgi-bin/fw-time.cgi")
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.minimumHeight: 230
            Layout.preferredHeight: Math.max(Layout.minimumHeight, timeCardContent.implicitHeight + 28)
            color: Theme.cardBackground
            border.color: Theme.cardBorder
            radius: Theme.radiusLg

            ColumnLayout {
                id: timeCardContent

                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Время и NTP OpenIPC")
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 17
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    MajesticButton {
                        text: I18n.t("NTP sync")
                        enabled: root.controller && !root.controller.firmwareBusy
                        onClicked: root.controller.syncFirmwareTime(false)
                    }

                    MajesticButton {
                        text: I18n.t("Set from PC")
                        enabled: root.controller && !root.controller.firmwareBusy
                        onClicked: root.controller.syncFirmwareTime(true)
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: width > 900 ? 4 : 2
                    rowSpacing: 8
                    columnSpacing: 10

                    Text { text: I18n.t("Zone name"); color: Theme.textSecondary; font.pixelSize: 11 }
                    MajesticTextField { id: timeZoneName; Layout.fillWidth: true; placeholderText: "Asia/Vladivostok" }
                    Text { text: I18n.t("POSIX string"); color: Theme.textSecondary; font.pixelSize: 11 }
                    MajesticTextField { id: timeZoneData; Layout.fillWidth: true; placeholderText: "VLAT-10" }
                    Text { text: "NTP 1"; color: Theme.textSecondary; font.pixelSize: 11 }
                    MajesticTextField { id: timeServer0; Layout.fillWidth: true; placeholderText: "pool.ntp.org" }
                    Text { text: "NTP 2"; color: Theme.textSecondary; font.pixelSize: 11 }
                    MajesticTextField { id: timeServer1; Layout.fillWidth: true; placeholderText: "time.cloudflare.com" }
                    Text { text: "NTP 3"; color: Theme.textSecondary; font.pixelSize: 11 }
                    MajesticTextField { id: timeServer2; Layout.fillWidth: true }
                    Text { text: "NTP 4"; color: Theme.textSecondary; font.pixelSize: 11 }
                    MajesticTextField { id: timeServer3; Layout.fillWidth: true }
                }

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Изменение timezone пишет /etc/TZ и /etc/timezone; камера может запросить reboot для полного применения.")
                    color: Theme.warning
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            spacing: 10

            Repeater {
                model: root.controller ? root.controller.timeRows() : []

                delegate: Rectangle {
                    id: timeRow

                    required property var modelData

                    Layout.fillWidth: true
                    Layout.minimumHeight: 82
                    Layout.preferredHeight: Math.max(Layout.minimumHeight, timeRowContent.implicitHeight + 28)
                    color: Theme.cardBackground
                    border.color: Theme.cardBorder
                    radius: Theme.radiusLg

                    RowLayout {
                        id: timeRowContent

                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 14

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: timeRow.modelData.label
                                color: Theme.textPrimary
                                font.bold: true
                                font.pixelSize: 14
                            }

                            Text {
                                Layout.fillWidth: true
                                text: timeRow.modelData.hint
                                color: Theme.textMuted
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                        }

                        Text {
                            Layout.preferredWidth: 260
                            text: timeRow.modelData.value
                            color: Theme.accentHover
                            font.family: "Consolas"
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}

