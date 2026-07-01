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
            title: I18n.t("OpenIPC Firmware")
            description: I18n.t("Единый центр управления камерой: системное состояние, WebUI, Majestic и безопасные firmware-действия.")

            MajesticButton {
                text: I18n.t("Открыть WebUI")
                primary: true
                enabled: root.controller !== null
                onClicked: root.controller.openWebUiPath("")
            }

            MajesticButton {
                text: I18n.t("Обновить")
                enabled: root.controller && !root.controller.loading
                onClicked: {
                    root.controller.refresh()
                    root.controller.refreshMetrics()
                }
            }
        }

        OpenIpcFirmwareStatusCardsGrid {
            rows: root.controller ? root.controller.firmwareSystemRows() : []
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            columns: width > 900 ? 2 : 1
            rowSpacing: 12
            columnSpacing: 12

            OpenIpcInfoRowsCard {
                title: I18n.t("Идентификация камеры")
                rows: root.controller ? root.controller.firmwareIdentityRows() : []
            }

            OpenIpcFirmwareQuickActionsCard {
                onOpenWebUiRequested: path => root.controller.openWebUiPath(path)
                onBackupRequested: root.controller.openFirmwareBackupDialog()
                onRebootRequested: root.controller.openFirmwareRebootConfirm()
                onCopyWebUiRequested: root.controller.copyControlCenterValue(I18n.t("Web UI камеры"), root.controller.webUiUrl(""))
            }
        }
    }
}

