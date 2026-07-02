pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var controller: null

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    Layout.preferredHeight: 360
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
                text: I18n.t("Export")
                subtle: true
                enabled: root.controller && root.controller.firmwareLogsText.length > 0
                onClicked: firmwareLogsExportDialog.open()
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

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: I18n.t("Источник")
                color: Theme.textMuted
                font.pixelSize: 11
            }

            MajesticButton {
                text: I18n.t("All")
                primary: root.controller && root.controller.firmwareLogsSource === "all"
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.loadFirmwareLogs("all")
            }

            MajesticButton {
                text: "majestic"
                primary: root.controller && root.controller.firmwareLogsSource === "majestic"
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.loadFirmwareLogs("majestic")
            }

            MajesticButton {
                text: "kernel"
                primary: root.controller && (root.controller.firmwareLogsSource === "kernel"
                                             || root.controller.firmwareLogsSource === "dmesg")
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.loadFirmwareLogs("kernel")
            }

            Item { Layout.fillWidth: true }

            Text {
                text: I18n.t("Ring buffer")
                color: Theme.textMuted
                font.pixelSize: 11
            }

            MajesticSpinBox {
                id: logBufferSpin
                from: 16
                to: 4096
                value: 256
                stepSize: 16
                editable: true
                Layout.preferredWidth: 110
            }

            Text {
                text: "KiB"
                color: Theme.textMuted
                font.pixelSize: 11
            }

            MajesticButton {
                text: I18n.t("Apply")
                subtle: true
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.setFirmwareLogBufferSize(logBufferSpin.value)
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Rectangle {
                implicitWidth: logText.implicitWidth + 24
                implicitHeight: logText.implicitHeight + 24
                color: Theme.controlBackground
                border.color: Theme.controlBorder
                radius: Theme.radiusMd

                Text {
                    id: logText
                    x: 12
                    y: 12
                    textFormat: Text.RichText
                    text: root.controller ? root.controller.filteredFirmwareLogsHtml() : ""
                    color: Theme.textSecondary
                    font.family: "Consolas"
                    font.pixelSize: 11
                }
            }
        }
    }

    FileDialog {
        id: firmwareLogsExportDialog
        title: I18n.t("Экспортировать логи OpenIPC")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "log"
        nameFilters: [I18n.t("Log files (*.log *.txt)"), I18n.t("Все файлы (*)")]
        onAccepted: if (root.controller) root.controller.exportFirmwareLogs(String(selectedFile))
    }
}

