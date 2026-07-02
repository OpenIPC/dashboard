pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Window {
    id: root

    width: 740
    height: 660
    minimumWidth: 600
    minimumHeight: 480
    title: I18n.t("Доступно обновление")
    color: Theme.appBackground
    flags: Qt.Window | Qt.FramelessWindowHint
    modality: Qt.ApplicationModal

    property var updateChecker: null
    property bool installAfterDownload: false

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
    readonly property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"

    function openDialog() {
        installAfterDownload = false
        show()
        requestActivate()
    }

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

    function runPrimaryAction() {
        if (!updateChecker || installing) return
        if (downloaded) {
            updateChecker.installDownloadedUpdate()
            return
        }
        if (downloadAvailable) {
            installAfterDownload = true
            updateChecker.downloadUpdate()
            return
        }
        updateChecker.openReleasePage()
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    Connections {
        target: root.updateChecker
        function onDownloadFinished(success) {
            if (success && root.installAfterDownload && root.updateChecker) {
                root.updateChecker.installDownloadedUpdate()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.appBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorderStrong
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 86
            color: Theme.topBarBackground
            radius: Theme.radiusLg

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: Theme.radiusLg
                color: Theme.topBarBackground
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 22
                anchors.rightMargin: 12
                spacing: 14

                Rectangle {
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 50
                    radius: 16
                    color: Theme.accent

                    Text {
                        anchors.centerIn: parent
                        text: "system_update"
                        font.family: root.iconFontFamily
                        font.pixelSize: 26
                        color: "white"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        Layout.fillWidth: true
                        text: root.latestName.length > 0 ? root.latestName : I18n.t("Доступна новая версия")
                        color: Theme.textPrimary
                        font.pixelSize: 21
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: I18n.t("Текущая версия: %1", [root.updateChecker ? root.updateChecker.currentVersion : "—"])
                            color: Theme.textSecondary
                            font.pixelSize: 13
                        }

                        Text {
                            text: "→"
                            color: Theme.textMuted
                            font.pixelSize: 13
                        }

                        Text {
                            text: I18n.t("Новая версия: %1", [root.latestVersion.length > 0 ? root.latestVersion : "—"])
                            color: Theme.success
                            font.pixelSize: 13
                            font.bold: true
                        }

                        Rectangle {
                            visible: root.latestPrerelease
                            Layout.preferredWidth: prereleaseLabel.implicitWidth + 16
                            Layout.preferredHeight: 22
                            radius: 11
                            color: Theme.warning
                            opacity: 0.95

                            Text {
                                id: prereleaseLabel
                                anchors.centerIn: parent
                                text: I18n.t("Предварительный релиз")
                                color: "black"
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }
                    }
                }

                DashboardDialogButton {
                    text: "×"
                    Layout.preferredWidth: 42
                    Layout.preferredHeight: 42
                    buttonColor: Theme.controlBackground
                    buttonHoverColor: Theme.cardHover
                    buttonBorderColor: Theme.controlBorder
                    buttonTextColor: Theme.textPrimary
                    onClicked: root.hide()
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
                Layout.preferredHeight: root.downloading || root.downloaded || root.installing || root.errorString.length > 0 ? 112 : 70
                radius: Theme.radiusMd
                color: Theme.panelAltBackground
                border.color: Theme.controlBorder
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
                            text: root.assetName.length > 0
                                  ? I18n.t("Файл обновления: %1", [root.assetName])
                                  : I18n.t("Для этой платформы нет подходящего файла обновления.")
                            color: root.downloadAvailable ? Theme.textPrimary : Theme.warning
                            font.pixelSize: 13
                            elide: Text.ElideRight
                        }

                        Text {
                            text: root.downloadTotalBytes > 0 ? root.formatBytes(root.downloadTotalBytes) : ""
                            color: Theme.textMuted
                            font.pixelSize: 12
                        }
                    }

                    ProgressBar {
                        visible: root.downloading || root.downloaded || root.installing
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: root.downloadProgress
                    }

                    Text {
                        visible: root.downloading || root.downloaded || root.installing || root.errorString.length > 0
                        Layout.fillWidth: true
                        text: root.errorString.length > 0
                              ? root.errorString
                              : root.installing
                                ? I18n.t("Запускаем установщик. Приложение сейчас закроется.")
                                : root.downloading
                                  ? I18n.t("Загружено %1 из %2", [root.formatBytes(root.downloadReceivedBytes), root.formatBytes(root.downloadTotalBytes)])
                                  : I18n.t("Файл загружен. Готово к установке.")
                        color: root.errorString.length > 0 ? Theme.warning : Theme.textSecondary
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
                radius: Theme.radiusMd
                color: Theme.controlBackground
                border.color: Theme.controlBorder
                border.width: 1

                ScrollView {
                    id: releaseNotesScroll

                    anchors.fill: parent
                    anchors.margins: 12
                    clip: true

                    TextArea {
                        width: releaseNotesScroll.availableWidth
                        implicitWidth: releaseNotesScroll.availableWidth
                        text: root.releaseNotes.length > 0
                              ? root.releaseNotes
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
                    enabled: !root.downloading && !root.installing
                    buttonColor: Theme.controlBackground
                    buttonHoverColor: Theme.cardHover
                    buttonBorderColor: Theme.controlBorder
                    buttonTextColor: Theme.textSecondary
                    onClicked: {
                        if (root.updateChecker) root.updateChecker.dismissCurrentUpdate()
                        root.hide()
                    }
                }

                DashboardDialogButton {
                    text: I18n.t("Напомнить позже")
                    Layout.preferredWidth: 145
                    Layout.preferredHeight: 38
                    enabled: !root.downloading && !root.installing
                    buttonColor: Theme.controlBackground
                    buttonHoverColor: Theme.cardHover
                    buttonBorderColor: Theme.controlBorderStrong
                    buttonTextColor: Theme.textPrimary
                    onClicked: {
                        if (root.updateChecker) root.updateChecker.remindLater()
                        root.hide()
                    }
                }

                Item { Layout.fillWidth: true }

                DashboardDialogButton {
                    visible: root.downloading
                    text: I18n.t("Отмена")
                    Layout.preferredWidth: 110
                    Layout.preferredHeight: 38
                    buttonColor: Theme.controlBackground
                    buttonHoverColor: Theme.cardHover
                    buttonBorderColor: Theme.controlBorderStrong
                    buttonTextColor: Theme.textPrimary
                    onClicked: {
                        root.installAfterDownload = false
                        if (root.updateChecker) root.updateChecker.cancelDownload()
                    }
                }

                DashboardDialogButton {
                    text: I18n.t("Открыть релиз")
                    Layout.preferredWidth: 135
                    Layout.preferredHeight: 38
                    enabled: !root.downloading && !root.installing
                    buttonColor: Theme.controlBackground
                    buttonHoverColor: Theme.cardHover
                    buttonBorderColor: Theme.controlBorderStrong
                    buttonTextColor: Theme.textPrimary
                    onClicked: {
                        if (root.updateChecker) root.updateChecker.openReleasePage()
                    }
                }

                DashboardDialogButton {
                    text: root.primaryText()
                    Layout.preferredWidth: 180
                    Layout.preferredHeight: 38
                    enabled: !root.downloading && !root.installing && (root.downloadAvailable || root.downloaded)
                    buttonColor: Theme.accent
                    buttonHoverColor: Qt.lighter(Theme.accent, 1.15)
                    buttonBorderColor: Theme.accent
                    buttonTextColor: "white"
                    onClicked: root.runPrimaryAction()
                }
            }
        }
    }
}
