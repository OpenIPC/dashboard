pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: panel

    property var settings: null
    property var updateChecker: null
    readonly property bool compactLayout: width < 620

    Layout.fillWidth: true
    spacing: 12

    Text {
        text: I18n.t("Обновления")
        color: Theme.textPrimary
        font.pixelSize: 16
        font.bold: true
    }

    GridLayout {
        id: updatesGrid

        columns: panel.compactLayout ? 1 : 2
        columnSpacing: 14
        rowSpacing: 10
        Layout.fillWidth: true

        property int labelWidth: panel.compactLayout ? 0 : 160

        Text { text: I18n.t("Статус"); color: Theme.textMuted; Layout.preferredWidth: updatesGrid.labelWidth }
        Text {
            text: panel.settings ? panel.settings.updateStatusText() : ""
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Text { text: I18n.t("Текущая версия"); color: Theme.textMuted; Layout.preferredWidth: updatesGrid.labelWidth }
        Text {
            text: panel.updateChecker.currentVersion
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Text {
            visible: panel.updateChecker.hasUpdate
            text: I18n.t("Новая версия")
            color: Theme.textMuted
            Layout.preferredWidth: updatesGrid.labelWidth
            Layout.preferredHeight: visible ? implicitHeight : 0
        }
        RowLayout {
            visible: panel.updateChecker.hasUpdate
            spacing: 8
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? implicitHeight : 0

            Text {
                text: panel.updateChecker.latestVersion
                color: Theme.success
                font.bold: true
                Layout.fillWidth: true
            }

            Rectangle {
                visible: panel.updateChecker.latestPrerelease
                Layout.preferredWidth: prereleaseSettingsLabel.implicitWidth + 16
                Layout.preferredHeight: 22
                radius: 11
                color: Theme.warning

                Text {
                    id: prereleaseSettingsLabel
                    anchors.centerIn: parent
                    text: I18n.t("Предварительный релиз")
                    color: "black"
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        Text {
            visible: panel.updateChecker.hasUpdate
            text: I18n.t("Файл обновления")
            color: Theme.textMuted
            Layout.preferredWidth: updatesGrid.labelWidth
            Layout.preferredHeight: visible ? implicitHeight : 0
        }
        Text {
            visible: panel.updateChecker.hasUpdate
            text: panel.updateChecker.assetName !== ""
                  ? panel.updateChecker.assetName
                  : I18n.t("Для этой платформы нет подходящего файла обновления.")
            color: panel.updateChecker.downloadAvailable ? Theme.textSecondary : Theme.warning
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? implicitHeight : 0
        }

        Text {
            visible: panel.updateChecker.downloading || panel.updateChecker.downloadedFilePath !== "" || panel.updateChecker.installing
            text: I18n.t("Прогресс")
            color: Theme.textMuted
            Layout.preferredWidth: updatesGrid.labelWidth
            Layout.preferredHeight: visible ? implicitHeight : 0
        }
        ProgressBar {
            visible: panel.updateChecker.downloading || panel.updateChecker.downloadedFilePath !== "" || panel.updateChecker.installing
            from: 0
            to: 100
            value: panel.updateChecker.downloadProgress
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? 20 : 0
        }

        Text { text: I18n.t("Действие"); color: Theme.textMuted; Layout.preferredWidth: updatesGrid.labelWidth }
        Flow {
            spacing: 8
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            Button {
                id: checkUpdatesButton
                text: panel.updateChecker.checking
                      ? I18n.t("Проверка...")
                      : I18n.t("Проверить обновления")
                enabled: !panel.updateChecker.checking
                         && !panel.updateChecker.downloading
                         && !panel.updateChecker.installing
                width: Math.min(190, parent.width)
                height: 34
                focusPolicy: Qt.StrongFocus
                background: Rectangle {
                    color: checkUpdatesButton.enabled ? Theme.metroBlue : Theme.metroTileDisabled
                    border.color: checkUpdatesButton.visualFocus ? Theme.textPrimary : Theme.metroStroke
                    border.width: checkUpdatesButton.visualFocus ? 2 : 1
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: checkUpdatesButton.text
                    color: checkUpdatesButton.enabled ? Theme.textPrimary : Theme.textFaint
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: if (panel.settings) panel.settings.startUpdateCheck()
            }

            Button {
                id: openReleaseButton
                visible: panel.updateChecker.hasUpdate
                text: I18n.t("Открыть релиз")
                width: Math.min(140, parent.width)
                height: visible ? 34 : 0
                focusPolicy: Qt.StrongFocus
                enabled: !panel.updateChecker.downloading
                         && !panel.updateChecker.installing
                background: Rectangle {
                    color: openReleaseButton.enabled ? Theme.success : Theme.metroTileDisabled
                    border.color: openReleaseButton.visualFocus ? Theme.textPrimary : Theme.metroStroke
                    border.width: openReleaseButton.visualFocus ? 2 : 1
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: openReleaseButton.text
                    color: openReleaseButton.enabled ? Theme.textPrimary : Theme.textFaint
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: panel.updateChecker.openReleasePage()
            }

            Button {
                id: downloadUpdateButton
                visible: panel.updateChecker.hasUpdate
                text: panel.updateChecker.installing
                      ? I18n.t("Запуск установки...")
                      : panel.updateChecker.downloadedFilePath !== ""
                        ? I18n.t("Установить и перезапустить")
                        : panel.updateChecker.downloading
                          ? I18n.t("Отмена")
                          : I18n.t("Скачать и установить")
                width: Math.min(210, parent.width)
                height: visible ? 34 : 0
                focusPolicy: Qt.StrongFocus
                enabled: !panel.updateChecker.installing
                         && (panel.updateChecker.downloadAvailable
                             || panel.updateChecker.downloadedFilePath !== ""
                             || panel.updateChecker.downloading)
                background: Rectangle {
                    color: !downloadUpdateButton.enabled || panel.updateChecker.downloading
                           ? Theme.metroTileDisabled
                           : Theme.accent
                    border.color: downloadUpdateButton.visualFocus ? Theme.textPrimary : Theme.metroStroke
                    border.width: downloadUpdateButton.visualFocus ? 2 : 1
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: downloadUpdateButton.text
                    color: downloadUpdateButton.enabled ? Theme.textPrimary : Theme.textFaint
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: {
                    if (panel.updateChecker.downloading) {
                        if (panel.settings) panel.settings.updateInstallAfterDownload = false
                        panel.updateChecker.cancelDownload()
                    } else if (panel.updateChecker.downloadedFilePath !== "") {
                        panel.updateChecker.installDownloadedUpdate()
                    } else {
                        if (panel.settings) panel.settings.updateInstallAfterDownload = true
                        panel.updateChecker.downloadUpdate()
                    }
                }
            }
        }

        Text {
            text: panel.settings && panel.settings.updateStatus === "error" ? I18n.t("Ошибка") : ""
            color: panel.settings && panel.settings.updateStatus === "error" ? Theme.metroRed : "transparent"
            Layout.preferredWidth: updatesGrid.labelWidth
        }
        Text {
            text: panel.settings && panel.settings.updateStatus === "error" ? panel.settings.updateError : ""
            color: Theme.metroRed
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
