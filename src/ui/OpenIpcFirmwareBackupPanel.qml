pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var controller: null

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    Layout.minimumHeight: 166
    Layout.preferredHeight: Math.max(Layout.minimumHeight, backupContent.implicitHeight + 28)
    color: Theme.metroSurface
    border.color: Theme.metroStroke
    radius: Theme.metroTileRadius

    ColumnLayout {
        id: backupContent

        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                Text {
                    text: I18n.t("OpenIPC backup / restore")
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 17
                }

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Полный backup прошивки/overlay отличается от Majestic JSON backup. Его используем перед update и только для аварийного восстановления.")
                    color: Theme.textMuted
                    wrapMode: Text.WordWrap
                    font.pixelSize: 12
                }
            }

            Rectangle {
                implicitWidth: backupStateText.implicitWidth + 18
                implicitHeight: 24
                radius: Theme.metroTileRadius
                color: root.controller && root.controller.firmwareBackupSaved ? Theme.successSurface : Theme.warningSurface
                border.color: root.controller && root.controller.firmwareBackupSaved ? Theme.metroGreen : Theme.metroAmber

                Text {
                    id: backupStateText
                    anchors.centerIn: parent
                    text: root.controller && root.controller.firmwareBackupSaved ? I18n.t("backup сохранён") : I18n.t("backup нужен")
                    color: root.controller && root.controller.firmwareBackupSaved ? Theme.success : Theme.warning
                    font.bold: true
                    font.pixelSize: 10
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width > 720 ? 3 : 1
            rowSpacing: 8
            columnSpacing: 8

            MajesticButton {
                text: I18n.t("Создать OpenIPC backup…")
                primary: true
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.openFirmwareBackupDialog()
            }

            MajesticButton {
                text: I18n.t("Restore через WebUI…")
                danger: true
                enabled: root.controller && !root.controller.firmwareBusy
                onClicked: root.controller.openFirmwareRestoreWebUiConfirm()
            }

            MajesticButton {
                text: I18n.t("Majestic backup")
                subtle: true
                enabled: root.controller !== null
                onClicked: if (root.controller) root.controller.openSaveBackupDialog()
            }
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("Restore полного backup может менять overlay, сеть и пароли. Поэтому восстановление вынесено в штатный WebUI камеры и требует отдельного подтверждения.")
            color: Theme.metroAmber
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }
    }
}
