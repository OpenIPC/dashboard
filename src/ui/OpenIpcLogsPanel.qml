pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var controller: null

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    Layout.preferredHeight: 300
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
                text: I18n.t("Логи и обслуживание")
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 17
            }

            MajesticButton {
                text: root.controller && root.controller.firmwareLiveLogs ? I18n.t("Stop live") : I18n.t("Live logs")
                primary: !(root.controller && root.controller.firmwareLiveLogs)
                enabled: root.controller && (!root.controller.firmwareBusy || root.controller.firmwareLiveLogs)
                onClicked: root.controller.firmwareLiveLogs ? root.controller.stopFirmwareLiveLogs() : root.controller.startFirmwareLiveLogs()
            }

            MajesticButton {
                text: "syslog"
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.loadFirmwareLogs("syslog")
            }

            MajesticButton {
                text: "majestic"
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.loadFirmwareLogs("majestic")
            }

            MajesticButton {
                text: "dmesg"
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.loadFirmwareLogs("kernel")
            }

            MajesticButton {
                text: I18n.t("Backup")
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.openFirmwareBackupDialog()
            }

            MajesticButton {
                text: I18n.t("Reboot")
                danger: true
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.openFirmwareRebootConfirm()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            MajesticTextField {
                Layout.fillWidth: true
                placeholderText: I18n.t("Фильтр логов…")
                text: root.controller ? root.controller.firmwareLogFilter : ""
                onTextChanged: if (root.controller) root.controller.firmwareLogFilter = text
            }

            MajesticButton {
                text: root.controller && root.controller.firmwareLogsPaused ? I18n.t("Resume") : I18n.t("Pause")
                subtle: true
                enabled: root.controller !== null
                onClicked: root.controller.firmwareLogsPaused = !root.controller.firmwareLogsPaused
            }

            MajesticButton {
                text: I18n.t("Очистить")
                subtle: true
                enabled: root.controller !== null
                onClicked: root.controller.firmwareLogsText = ""
            }

            Text {
                text: root.controller && root.controller.firmwareWebSocketsAvailable ? "/ws/logs" : I18n.t("polling")
                color: root.controller && root.controller.firmwareLiveLogs ? Theme.success : Theme.textMuted
                font.family: "Consolas"
                font.pixelSize: 10
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            TextArea {
                readOnly: true
                wrapMode: TextEdit.NoWrap
                text: root.controller ? root.controller.filteredFirmwareLogsText() : ""
                color: Theme.textSecondary
                selectedTextColor: Theme.textPrimary
                selectionColor: Theme.accent
                font.family: "Consolas"
                font.pixelSize: 11
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: Theme.controlBackground
                    border.color: Theme.controlBorder
                }
            }
        }
    }
}

