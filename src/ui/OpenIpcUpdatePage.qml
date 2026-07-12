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
            title: I18n.t("Firmware Update")
            description: I18n.t("Прошивка — самый опасный раздел. Native update доступен через safety-gate: проверяются WebSocket updater, SoC/flash, источник прошивки и выбранные опции.")

            MajesticButton {
                text: I18n.t("Загрузить")
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.refreshFirmwareUpdateInfo()
            }

            MajesticButton {
                text: I18n.t("Upload archive")
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.openFirmwareUploadDialog()
            }

            MajesticButton {
                text: I18n.t("Install uploaded")
                danger: true
                enabled: root.controller && !root.controller.firmwareBusy && root.controller.canStartFirmwareUpdate("uploaded")
                onClicked: root.controller.openUploadedFirmwareUpdateConfirm()
            }

            MajesticButton {
                text: I18n.t("GitHub update")
                danger: true
                enabled: root.controller && !root.controller.firmwareBusy && root.controller.canStartFirmwareUpdate("github")
                onClicked: root.controller.openGithubFirmwareUpdateConfirm()
            }

            MajesticButton {
                text: I18n.t("Открыть Update WebUI")
                primary: true
                enabled: root.controller !== null
                onClicked: root.controller.openWebUiPath("/cgi-bin/fw-update.cgi")
            }
        }

        OpenIpcUpdateWarningCard {
            webSocketsAvailable: root.controller && root.controller.firmwareWebSocketsAvailable
        }

        OpenIpcUpdateOptionsCard {
            updateKernel: root.controller ? root.controller.firmwareUpdateKernel : true
            updateRootfs: root.controller ? root.controller.firmwareUpdateRootfs : true
            updateReset: root.controller ? root.controller.firmwareUpdateReset : false
            updateForce: root.controller ? root.controller.firmwareUpdateForce : false
            powerSafetyConfirmed: root.controller ? root.controller.firmwarePowerSafetyConfirmed : false
            dangerOptionsConfirmed: root.controller ? root.controller.firmwareDangerOptionsConfirmed : false
            optionsSummary: root.controller ? root.controller.firmwareUpdateOptionsSummary() : ""
            onKernelToggled: checked => root.controller.firmwareUpdateKernel = checked
            onRootfsToggled: checked => root.controller.firmwareUpdateRootfs = checked
            onResetToggled: checked => {
                root.controller.firmwareUpdateReset = checked
                if (!root.controller.firmwareDangerousOptionsActive())
                    root.controller.firmwareDangerOptionsConfirmed = false
            }
            onForceToggled: checked => {
                root.controller.firmwareUpdateForce = checked
                if (!root.controller.firmwareDangerousOptionsActive())
                    root.controller.firmwareDangerOptionsConfirmed = false
            }
            onPowerSafetyToggled: checked => root.controller.firmwarePowerSafetyConfirmed = checked
            onDangerOptionsToggled: checked => root.controller.firmwareDangerOptionsConfirmed = checked
        }

        OpenIpcFirmwareManifestPanel {
            rows: root.controller ? root.controller.firmwareArchiveManifestRows() : []
            summary: root.controller ? root.controller.firmwareArchiveManifestSummary() : ""
            state: root.controller ? root.controller.firmwareArchiveManifestState() : "warn"
        }

        OpenIpcUpgradeProgressPanel {
            progressText: root.controller ? root.controller.firmwareUpgradeText : ""
            upgradeRebooting: root.controller && root.controller.firmwareUpgradeRebooting
            returnPolling: root.controller && root.controller.firmwareReturnPolling
            returnPollTries: root.controller ? root.controller.firmwareReturnPollTries : 0
            returnPollMaxTries: root.controller ? root.controller.firmwareReturnPollMaxTries : 0
            returnPhase: root.controller ? root.controller.firmwareReturnPhase : "idle"
            returnHealthText: root.controller ? root.controller.firmwareReturnHealthText : ""
            firmwareBusy: root.controller && root.controller.firmwareBusy
            onClearRequested: root.controller.firmwareUpgradeText = ""
        }

        OpenIpcFirmwareStatusGrid {
            updateInfo: root.controller ? root.controller.firmwareUpdateInfo : ({})
            socText: root.controller ? root.controller.firmwareSocText() : ""
            flashText: root.controller ? root.controller.firmwareFlashText() : ""
        }

        OpenIpcUpdateChecklistGrid {
            rows: root.controller ? root.controller.updateChecklistRows() : []
        }
    }
}

