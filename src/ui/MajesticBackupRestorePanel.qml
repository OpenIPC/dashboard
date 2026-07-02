pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var controller: null
    property alias snapshotWidth: snapshotWidth.value
    property alias snapshotHeight: snapshotHeight.value
    property alias snapshotQuality: snapshotQuality.value
    property alias snapshotGray: snapshotGray.checked

    Layout.fillWidth: true
    Layout.preferredHeight: 218
    color: Theme.cardBackground
    border.color: Theme.cardBorder
    radius: Theme.radiusLg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 8

        Text {
            text: I18n.t("Majestic snapshot / config backup / audio")
            color: Theme.textPrimary
            font.bold: true
            font.pixelSize: 15
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("Это backup только конфигурации Majestic. Полный OpenIPC firmware/overlay backup находится во вкладке Tools.")
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }

        RowLayout {
            Label {
                text: I18n.t("Ширина")
                color: Theme.textMuted
            }
            MajesticSpinBox {
                id: snapshotWidth
                from: 0
                to: 8192
                editable: true
            }
            Label {
                text: I18n.t("Высота")
                color: Theme.textMuted
            }
            MajesticSpinBox {
                id: snapshotHeight
                from: 0
                to: 8192
                editable: true
            }
        }

        RowLayout {
            Label {
                text: I18n.t("Качество")
                color: Theme.textMuted
            }
            MajesticSpinBox {
                id: snapshotQuality
                from: 1
                to: 100
                value: 85
                editable: true
            }
            MajesticCheckBox {
                id: snapshotGray
                text: I18n.t("Ч/Б")
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            MajesticButton {
                text: I18n.t("Сохранить JPEG…")
                primary: true
                enabled: root.controller !== null
                onClicked: root.controller.openSnapshotDialog()
            }

            MajesticButton {
                text: I18n.t("Создать backup…")
                enabled: root.controller && root.controller.fields.length > 0
                onClicked: root.controller.openSaveBackupDialog()
            }

            MajesticButton {
                text: I18n.t("Открыть backup…")
                enabled: root.controller !== null
                onClicked: root.controller.openBackupRestoreDialog()
            }

            MajesticButton {
                text: I18n.t("Копировать без секретов")
                enabled: root.controller && root.controller.fields.length > 0
                onClicked: root.controller.copyRedactedJson(root.controller.originalConfig, I18n.t("Backup Majestic"))
            }

            MajesticButton {
                text: I18n.t("Передать PCM…")
                enabled: root.controller !== null
                onClicked: root.controller.openPcmDialog()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.panelBorder
        }

        Text {
            Layout.fillWidth: true
            text: root.controller ? root.controller.backupRestoreSummary() : ""
            color: root.controller && root.controller.backupRestorePath.length ? Theme.accentHover : Theme.textMuted
            elide: Text.ElideRight
            font.pixelSize: 11
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            MajesticButton {
                text: I18n.t("Preview restore")
                subtle: true
                enabled: root.controller && root.controller.backupRestorePath.length > 0
                onClicked: root.controller.previewBackupRestore()
            }

            MajesticButton {
                text: I18n.t("Применить backup diff")
                primary: true
                enabled: root.controller
                         && root.controller.backupRestorePath.length > 0
                         && root.controller.backupRestoreChanges.length > 0
                         && root.controller.capabilities.configWrite === true
                         && !root.controller.loading
                onClicked: root.controller.applyBackupRestore()
            }

            MajesticButton {
                text: I18n.t("Сбросить restore")
                subtle: true
                enabled: root.controller && root.controller.backupRestorePath.length > 0
                onClicked: root.controller.clearBackupRestore()
            }
        }
    }
}

