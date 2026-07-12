pragma ComponentBehavior: Bound

import QtQuick
import OpenIPC

Window {
    id: root

    width: 740
    height: 660
    minimumWidth: 600
    minimumHeight: 480
    title: I18n.t("Доступно обновление")
    color: Theme.metroBackground
    flags: Qt.Window | Qt.FramelessWindowHint
    modality: Qt.ApplicationModal

    property var updateChecker: null
    property bool installAfterDownload: false

    readonly property bool downloadAvailable: updateChecker ? updateChecker.downloadAvailable : false
    readonly property bool installing: updateChecker ? updateChecker.installing : false
    readonly property string downloadedFilePath: updateChecker ? updateChecker.downloadedFilePath : ""
    readonly property bool downloaded: downloadedFilePath.length > 0
    readonly property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"

    function openDialog() {
        installAfterDownload = false
        show()
        requestActivate()
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
        color: Theme.metroBackground
        radius: Theme.metroTileRadius
        border.color: Theme.metroStroke
        border.width: 1
    }

    AppUpdateContent {
        anchors.fill: parent
        updateChecker: root.updateChecker
        iconFontFamily: root.iconFontFamily

        onCloseRequested: root.hide()
        onSkipRequested: {
            if (root.updateChecker) root.updateChecker.dismissCurrentUpdate()
            root.hide()
        }
        onRemindLaterRequested: {
            if (root.updateChecker) root.updateChecker.remindLater()
            root.hide()
        }
        onCancelDownloadRequested: {
            root.installAfterDownload = false
            if (root.updateChecker) root.updateChecker.cancelDownload()
        }
        onOpenReleaseRequested: {
            if (root.updateChecker) root.updateChecker.openReleasePage()
        }
        onPrimaryActionRequested: root.runPrimaryAction()
    }
}
