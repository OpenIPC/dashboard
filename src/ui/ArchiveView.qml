import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Dialog {
    id: root
    title: I18n.t("Архив")
    modal: true
    parent: Overlay.overlay
    width: isFullScreen ? parent.width : 1280
    height: isFullScreen ? parent.height : 800
    x: isFullScreen ? 0 : (parent.width - width) / 2
    y: isFullScreen ? 0 : (parent.height - height) / 2
    
    // Settings
    property string defaultDownloadPath: ""
    property var currentFile: null
    property bool isFullScreen: false
    property string currentCameraIp: ""

    function archivePlaybackUrl(file) {
        if (!file) return ""
        if (file.fileUrl !== undefined && String(file.fileUrl).length > 0) {
            return String(file.fileUrl)
        }
        if (file.filePath !== undefined && String(file.filePath).length > 0) {
            var localPath = SystemController.normalizeLocalPath(String(file.filePath)).replace(/\\/g, "/")
            if (localPath.length === 0) return ""
            if (Qt.platform.os === "windows") {
                return localPath.startsWith("/") ? "file://" + localPath : "file:///" + localPath
            }
            return localPath.startsWith("/") ? "file://" + localPath : "file:///" + localPath
        }
        return ""
    }

    function playArchiveFile(file) {
        var url = archivePlaybackUrl(file)
        if (url.length === 0) return
        currentFile = file
        exportMode = false
        exportStartMs = 0
        exportEndMs = 0
        player.url = url
        player.running = true
    }

    function selectedArchiveFile() {
        if (archiveSidebar.selectedFile) return archiveSidebar.selectedFile
        return currentFile
    }

    function openExportDialog() {
        if (exportEndMs <= exportStartMs) return
        var file = selectedArchiveFile()
        if (!file || !file.filePath) return
        exportDialog.openForFile(file, file.filePath, exportStartMs, exportEndMs)
    }

    onIsFullScreenChanged: {
        if (Window.window) {
            Window.window.visibility = isFullScreen ? Window.FullScreen : Window.Windowed
        }
    }

    onClosed: {
        player.running = false
        player.url = ""
        SystemController.isArchiveOpen = false
    }

    onOpened: {
        if (!SystemController.userManager.hasPermission(SystemController.userManager.Perm_Playback)) {
            root.close()
            return
        }
        SystemController.isArchiveOpen = true
    }

    onVisibleChanged: {
        if (!visible) {
            player.running = false
            player.url = ""
            SystemController.isArchiveOpen = false
        }
    }

    Connections {
        target: SystemController.archiveController
        function onLoginFailed(error) {
            console.error("Login failed: " + error)
            errorDialog.text = I18n.t("Ошибка входа: ") + error
            errorDialog.open()
        }
        function onSearchFinished(count) {
            console.log("Search finished. Found " + count + " files.")
        }
    }

    MessageDialog {
        id: errorDialog
        title: I18n.t("Ошибка")
        buttons: MessageDialog.Ok
    }

    Component.onCompleted: {
        var settings = SystemController.getAppSettings()
        if (settings && settings.recordingsPath) {
            defaultDownloadPath = settings.recordingsPath
        }
    }

    background: Rectangle {
        color: Theme.metroBackground
        border.color: Theme.metroTile
        radius: 5
    }

    header: Rectangle {
        color: Theme.metroTile
        height: isFullScreen ? 0 : 40
        visible: !isFullScreen
        width: parent.width
        radius: 5
        
        // Fix bottom corners
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 5
            color: Theme.metroTile
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            
            Text {
                text: I18n.t("Архив")
                color: Theme.textSecondary
                font.bold: true
                font.pixelSize: 14
            }
            
            Item { Layout.fillWidth: true }
            
            MetroWindowButton {
                kind: "close"
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                onClicked: root.close()
            }
        }
    }

    contentItem: RowLayout {
        anchors.fill: parent
        spacing: 0

        ArchiveSearchSidebar {
            id: archiveSidebar
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            visible: !isFullScreen
            currentCameraIp: root.currentCameraIp
            defaultDownloadPath: root.defaultDownloadPath
            onFileSelected: (file, index) => playArchiveFile(file)
            onFolderRequested: (file) => SystemController.openFolder(file.filePath)
        }
        // RIGHT CONTENT (Video & Controls)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#000"
            
            // Video Player Area
            Rectangle {
                id: videoArea
                anchors.fill: parent
                color: "#000"
                
                // Container to enforce 16:9 aspect ratio
                Item {
                    id: playerContainer
                    anchors.centerIn: parent
                    
                    // Calculate size to fit in parent while keeping 16:9
                    readonly property real targetRatio: 16/9
                    readonly property real parentRatio: videoArea.width / videoArea.height
                    
                    width: parentRatio > targetRatio ? videoArea.height * targetRatio : videoArea.width
                    height: parentRatio > targetRatio ? videoArea.height : videoArea.width / targetRatio

                    VideoPlayer {
                        id: player
                        anchors.fill: parent
                        url: "" // Set when playing
                        // orientation: 0 
                        // fillMode: 0 // IgnoreAspectRatio (Stretch to container)
                        transform: Scale { origin.x: player.width / 2; origin.y: player.height / 2; xScale: 1 }
                        
                        // Settings bindings
                        hwDecoding: (SystemController.appSettings.playerHwDecoding !== undefined) ? SystemController.appSettings.playerHwDecoding : "auto"
                        brightness: (SystemController.appSettings.playerBrightness !== undefined) ? SystemController.appSettings.playerBrightness : 1.0
                        contrast: (SystemController.appSettings.playerContrast !== undefined) ? SystemController.appSettings.playerContrast : 1.0
                        hue: (SystemController.appSettings.playerHue !== undefined) ? SystemController.appSettings.playerHue : 0
                        saturation: (SystemController.appSettings.playerSaturation !== undefined) ? SystemController.appSettings.playerSaturation : 1.0
                        gamma: (SystemController.appSettings.playerGamma !== undefined) ? SystemController.appSettings.playerGamma : 1.0
                    }
                    
                    Text {
                        anchors.centerIn: parent
                        text: currentFile ? currentFile.fileName : I18n.t("Выберите файл")
                        color: Theme.textFaint
                        visible: !player.running && !player.url
                        font.pixelSize: 20
                    }
                }
            }

            ArchivePlaybackControls {
                id: playbackControls
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                player: player
                currentFile: root.currentFile
                isFullScreen: root.isFullScreen
                exportMode: root.exportMode
                exportStartMs: root.exportStartMs
                exportEndMs: root.exportEndMs

                onResumeRequested: {
                    if (root.currentFile) root.playArchiveFile(root.currentFile)
                }
                onStopRequested: {
                    player.running = false
                    player.url = ""
                }
                onExportModeRequested: {
                    if (!root.currentFile || playbackControls.playerDuration <= 0) return
                    root.exportMode = true
                    root.exportStartMs = 0
                    root.exportEndMs = playbackControls.playerDuration
                }
                onExportSaveRequested: root.openExportDialog()
                onExportCancelRequested: root.exportMode = false
                onExportStartRequested: (ms) => root.exportStartMs = ms
                onExportEndRequested: (ms) => root.exportEndMs = ms
                onFullscreenToggled: root.isFullScreen = !root.isFullScreen
            }
        }
    }

    property bool exportMode: false
    property real exportStartMs: 0
    property real exportEndMs: 0

    ArchiveExportDialog {
        id: exportDialog
        defaultDownloadPath: root.defaultDownloadPath

        onExportAccepted: (sourcePath, outputPath, startMs, endMs) => {
            SystemController.archiveController.exportVideo(sourcePath, outputPath, startMs, endMs)
            root.exportMode = false
        }
    }

}
