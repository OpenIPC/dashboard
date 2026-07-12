pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: content

    property var updateChecker: null
    property string iconFontFamily: "Material Icons"

    readonly property string latestVersion: updateChecker ? updateChecker.latestVersion : ""
    readonly property string latestName: updateChecker ? updateChecker.latestName : ""
    readonly property string releaseNotes: updateChecker ? updateChecker.releaseNotes : ""
    readonly property bool latestPrerelease: updateChecker ? updateChecker.latestPrerelease : false
    readonly property bool downloadAvailable: updateChecker ? updateChecker.downloadAvailable : false
    readonly property bool downloading: updateChecker ? updateChecker.downloading : false
    readonly property bool installing: updateChecker ? updateChecker.installing : false
    readonly property int downloadProgress: updateChecker ? updateChecker.downloadProgress : 0
    readonly property real downloadReceivedBytes: updateChecker ? updateChecker.downloadReceivedBytes : 0
    readonly property real downloadTotalBytes: updateChecker ? updateChecker.downloadTotalBytes : 0
    readonly property string downloadedFilePath: updateChecker ? updateChecker.downloadedFilePath : ""
    readonly property string assetName: updateChecker ? updateChecker.assetName : ""
    readonly property string errorString: updateChecker ? updateChecker.errorString : ""
    readonly property bool downloaded: downloadedFilePath.length > 0

    signal closeRequested()
    signal skipRequested()
    signal remindLaterRequested()
    signal cancelDownloadRequested()
    signal openReleaseRequested()
    signal primaryActionRequested()

    spacing: 0

    function formatBytes(bytes) {
        if (bytes <= 0) return "—"
        if (bytes < 1024) return bytes + " B"
        var kib = bytes / 1024
        if (kib < 1024) return kib.toFixed(1) + " KiB"
        var mib = kib / 1024
        if (mib < 1024) return mib.toFixed(1) + " MiB"
        return (mib / 1024).toFixed(2) + " GiB"
    }

    function primaryText() {
        if (installing) return I18n.t("Запуск установки...")
        if (downloading) return I18n.t("Скачивание...")
        if (downloaded) return I18n.t("Установить и перезапустить")
        if (downloadAvailable) return I18n.t("Скачать и установить")
        return I18n.t("Открыть релиз")
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 86
        color: Theme.metroSidebarBackground
        radius: Theme.metroTileRadius

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: Theme.radiusLg
            color: Theme.metroSidebarBackground
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 22
            anchors.rightMargin: 12
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 50
                Layout.preferredHeight: 50
                radius: Theme.metroTileRadius
                color: Theme.metroBlue

                Text {
                    anchors.centerIn: parent
                    text: "system_update"
                    font.family: content.iconFontFamily
                    font.pixelSize: 26
                    color: Theme.textPrimary
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    Layout.fillWidth: true
                    text: content.latestName.length > 0 ? content.latestName : I18n.t("Доступна новая версия")
                    color: Theme.textPrimary
                    font.pixelSize: 21
                    font.bold: true
                    elide: Text.ElideRight
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: I18n.t("Текущая версия: %1", [content.updateChecker ? content.updateChecker.currentVersion : "—"])
                        color: Theme.textSecondary
                        font.pixelSize: 13
                    }

                    Text {
                        text: "→"
                        color: Theme.textMuted
                        font.pixelSize: 13
                    }

                    Text {
                        text: I18n.t("Новая версия: %1", [content.latestVersion.length > 0 ? content.latestVersion : "—"])
                        color: Theme.metroGreen
                        font.pixelSize: 13
                        font.bold: true
                    }

                    Rectangle {
                        visible: content.latestPrerelease
                        Layout.preferredWidth: prereleaseLabel.implicitWidth + 16
                        Layout.preferredHeight: 22
                        radius: 11
                        color: Theme.metroAmber
                        opacity: 0.95

                        Text {
                            id: prereleaseLabel
                            anchors.centerIn: parent
                            text: I18n.t("Предварительный релиз")
                            color: Theme.metroBackground
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }
                }
            }

            MetroWindowButton {
                kind: "close"
                Layout.preferredWidth: 42
                Layout.preferredHeight: 42
                onClicked: content.closeRequested()
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 18
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: content.downloading || content.downloaded || content.installing || content.errorString.length > 0 ? 112 : 70
            radius: Theme.metroTileRadius
            color: Theme.metroSurface
            border.color: Theme.metroStroke
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        Layout.fillWidth: true
                        text: content.assetName.length > 0
                              ? I18n.t("Файл обновления: %1", [content.assetName])
                              : I18n.t("Для этой платформы нет подходящего файла обновления.")
                        color: content.downloadAvailable ? Theme.textPrimary : Theme.metroAmber
                        font.pixelSize: 13
                        elide: Text.ElideRight
                    }

                    Text {
                        text: content.downloadTotalBytes > 0 ? content.formatBytes(content.downloadTotalBytes) : ""
                        color: Theme.textMuted
                        font.pixelSize: 12
                    }
                }

                ProgressBar {
                    visible: content.downloading || content.downloaded || content.installing
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: content.downloadProgress
                }

                Text {
                    visible: content.downloading || content.downloaded || content.installing || content.errorString.length > 0
                    Layout.fillWidth: true
                    text: content.errorString.length > 0
                          ? content.errorString
                          : content.installing
                            ? I18n.t("Запускаем установщик. Приложение сейчас закроется.")
                            : content.downloading
                              ? I18n.t("Загружено %1 из %2", [content.formatBytes(content.downloadReceivedBytes), content.formatBytes(content.downloadTotalBytes)])
                              : I18n.t("Файл загружен. Готово к установке.")
                    color: content.errorString.length > 0 ? Theme.warning : Theme.textSecondary
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("Описание релиза")
            color: Theme.textPrimary
            font.pixelSize: 16
            font.bold: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Theme.metroTileRadius
            color: Theme.metroSurfaceAlt
            border.color: Theme.metroStroke
            border.width: 1

            ScrollView {
                id: releaseNotesScroll

                anchors.fill: parent
                anchors.margins: 12
                clip: true

                TextArea {
                    width: releaseNotesScroll.availableWidth
                    implicitWidth: releaseNotesScroll.availableWidth
                    text: content.releaseNotes.length > 0
                          ? content.releaseNotes
                          : I18n.t("GitHub Release не содержит описания.")
                    readOnly: true
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    color: Theme.textSecondary
                    selectedTextColor: Theme.textPrimary
                    selectionColor: Theme.accent
                    font.pixelSize: 13
                    background: Item {}
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("После загрузки приложение запустит установщик, затем временный файл обновления будет удалён.")
            color: Theme.textMuted
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            DashboardDialogButton {
                text: I18n.t("Пропустить эту версию")
                Layout.preferredWidth: 170
                Layout.preferredHeight: 38
                enabled: !content.downloading && !content.installing
                buttonColor: Theme.metroTile
                buttonHoverColor: Theme.metroTileHover
                buttonBorderColor: Theme.metroStroke
                buttonTextColor: Theme.textSecondary
                onClicked: content.skipRequested()
            }

            DashboardDialogButton {
                text: I18n.t("Напомнить позже")
                Layout.preferredWidth: 145
                Layout.preferredHeight: 38
                enabled: !content.downloading && !content.installing
                buttonColor: Theme.metroTile
                buttonHoverColor: Theme.metroTileHover
                buttonBorderColor: Theme.metroStroke
                buttonTextColor: Theme.textPrimary
                onClicked: content.remindLaterRequested()
            }

            Item { Layout.fillWidth: true }

            DashboardDialogButton {
                visible: content.downloading
                text: I18n.t("Отмена")
                Layout.preferredWidth: 110
                Layout.preferredHeight: 38
                buttonColor: Theme.metroTile
                buttonHoverColor: Theme.metroTileHover
                buttonBorderColor: Theme.metroStroke
                buttonTextColor: Theme.textPrimary
                onClicked: content.cancelDownloadRequested()
            }

            DashboardDialogButton {
                text: I18n.t("Открыть релиз")
                Layout.preferredWidth: 135
                Layout.preferredHeight: 38
                enabled: !content.downloading && !content.installing
                buttonColor: Theme.metroTile
                buttonHoverColor: Theme.metroTileHover
                buttonBorderColor: Theme.metroStroke
                buttonTextColor: Theme.textPrimary
                onClicked: content.openReleaseRequested()
            }

            DashboardDialogButton {
                text: content.primaryText()
                Layout.preferredWidth: 180
                Layout.preferredHeight: 38
                enabled: !content.downloading && !content.installing && (content.downloadAvailable || content.downloaded)
                buttonColor: Theme.metroBlue
                buttonHoverColor: Theme.metroBlueHover
                buttonBorderColor: Theme.metroBlue
                buttonTextColor: Theme.textPrimary
                onClicked: content.primaryActionRequested()
            }
        }
    }
}
