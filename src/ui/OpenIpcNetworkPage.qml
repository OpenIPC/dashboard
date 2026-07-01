pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ScrollView {
    id: root

    property var controller: null
    property alias hostname: networkHostname.text
    property alias interfaceIndex: networkInterface.currentIndex
    property string interfaceName: networkInterface.currentText
    property alias dhcpEnabled: networkDhcp.checked
    property alias address: networkAddress.text
    property alias netmask: networkNetmask.text
    property alias gateway: networkGateway.text
    property alias nameserver: networkDns.text
    property alias wlanSsid: networkWlanSsid.text
    property alias wlanPassword: networkWlanPassword.text

    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: parent.width
        spacing: 14

        OpenIpcPageHeader {
            title: I18n.t("Network")
            description: I18n.t("Сводка сетевых сервисов камеры. Изменение IP/DHCP будет добавлено только с dry-run и явным подтверждением, чтобы не потерять камеру.")

            MajesticButton {
                text: I18n.t("Загрузить")
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.loadFirmwareNetwork()
            }

            MajesticButton {
                text: I18n.t("Сохранить")
                primary: true
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.openFirmwareNetworkConfirm()
            }

            MajesticButton {
                text: I18n.t("Открыть Network WebUI")
                enabled: root.controller !== null
                onClicked: root.controller.openWebUiPath("/cgi-bin/fw-network.cgi")
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.preferredHeight: 250
            color: Theme.cardBackground
            border.color: Theme.cardBorder
            radius: Theme.radiusLg

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        Layout.fillWidth: true
                        text: I18n.t("Настройки сети OpenIPC")
                        color: Theme.textPrimary
                        font.bold: true
                        font.pixelSize: 17
                    }

                    MajesticButton {
                        text: I18n.t("Wi‑Fi scan")
                        enabled: root.controller && !root.controller.firmwareBusy
                        onClicked: root.controller.scanFirmwareWifi()
                    }

                    MajesticButton {
                        text: I18n.t("Reset network")
                        danger: true
                        enabled: root.controller && !root.controller.firmwareBusy
                        onClicked: root.controller.openFirmwareNetworkResetConfirm()
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: width > 900 ? 4 : 2
                    rowSpacing: 8
                    columnSpacing: 10

                    Text { text: I18n.t("Hostname"); color: Theme.textSecondary; font.pixelSize: 11 }
                    MajesticTextField { id: networkHostname; Layout.fillWidth: true; placeholderText: "openipc-camera" }
                    Text { text: I18n.t("Interface"); color: Theme.textSecondary; font.pixelSize: 11 }
                    MajesticComboBox { id: networkInterface; Layout.fillWidth: true; model: ["eth0", "wlan0"] }
                    MajesticCheckBox { id: networkDhcp; text: "DHCP"; Layout.columnSpan: 2 }
                    Text { text: I18n.t("IP address"); color: Theme.textSecondary; font.pixelSize: 11; visible: !networkDhcp.checked }
                    MajesticTextField { id: networkAddress; Layout.fillWidth: true; placeholderText: "192.168.0.219"; visible: !networkDhcp.checked }
                    Text { text: I18n.t("Netmask"); color: Theme.textSecondary; font.pixelSize: 11; visible: !networkDhcp.checked }
                    MajesticTextField { id: networkNetmask; Layout.fillWidth: true; placeholderText: "255.255.255.0"; visible: !networkDhcp.checked }
                    Text { text: I18n.t("Gateway"); color: Theme.textSecondary; font.pixelSize: 11; visible: !networkDhcp.checked }
                    MajesticTextField { id: networkGateway; Layout.fillWidth: true; placeholderText: "192.168.0.1"; visible: !networkDhcp.checked }
                    Text { text: "DNS"; color: Theme.textSecondary; font.pixelSize: 11; visible: !networkDhcp.checked }
                    MajesticTextField { id: networkDns; Layout.fillWidth: true; placeholderText: "1.1.1.1"; visible: !networkDhcp.checked }
                    Text { text: "Wi‑Fi SSID"; color: Theme.textSecondary; font.pixelSize: 11; visible: networkInterface.currentText === "wlan0" }
                    MajesticTextField { id: networkWlanSsid; Layout.fillWidth: true; visible: networkInterface.currentText === "wlan0" }
                    Text { text: I18n.t("Wi‑Fi password"); color: Theme.textSecondary; font.pixelSize: 11; visible: networkInterface.currentText === "wlan0" }
                    MajesticTextField { id: networkWlanPassword; Layout.fillWidth: true; echoMode: TextInput.Password; visible: networkInterface.currentText === "wlan0" }
                }

                Text {
                    Layout.fillWidth: true
                    text: root.controller && root.controller.firmwareWifiNetworks.length
                          ? I18n.t("Найдено Wi‑Fi сетей: %1", [root.controller.firmwareWifiNetworks.length])
                          : I18n.t("Сохранение сети может изменить IP камеры. Подтвердите действие перед отправкой.")
                    color: Theme.warning
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            columns: width > 900 ? 2 : 1
            rowSpacing: 12
            columnSpacing: 12

            Repeater {
                model: root.controller ? root.controller.networkServiceRows() : []

                delegate: Rectangle {
                    id: serviceRow

                    required property var modelData

                    Layout.fillWidth: true
                    Layout.preferredHeight: 92
                    color: Theme.cardBackground
                    border.color: Theme.cardBorder
                    radius: Theme.radiusLg

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 82
                            Layout.preferredHeight: 28
                            radius: 14
                            color: "#172554"
                            border.color: Theme.accent

                            Text {
                                anchors.centerIn: parent
                                text: serviceRow.modelData.label
                                color: Theme.accentHover
                                font.bold: true
                                font.pixelSize: 11
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                Layout.fillWidth: true
                                text: serviceRow.modelData.value
                                color: Theme.textPrimary
                                font.family: "Consolas"
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                text: serviceRow.modelData.hint
                                color: Theme.textMuted
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                        }

                        MajesticButton {
                            text: I18n.t("Копировать")
                            subtle: true
                            onClicked: root.controller.copyControlCenterValue(serviceRow.modelData.label, serviceRow.modelData.value)
                        }
                    }
                }
            }
        }
    }
}

