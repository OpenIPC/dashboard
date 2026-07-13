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
        player.url = url
        player.running = true
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

            // Controls Overlay
            Rectangle {
                id: controlsOverlay
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 120
                color: isFullScreen ? "#80000000" : Theme.metroTile
                
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    
                    // Timeline
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        color: "transparent"
                        visible: player.duration > 0
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10
                            
                            Text { 
                                text: formatTime(player.position)
                                color: Theme.textSecondary
                            }
                            
                            MetroSlider {
                                Layout.fillWidth: true
                                from: 0
                                to: player.duration
                                value: player.position
                                onMoved: player.position = value
                                
                                background: Rectangle {
                                    x: parent.leftPadding
                                    y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                    implicitWidth: 200
                                    implicitHeight: 4
                                    width: parent.availableWidth
                                    height: implicitHeight
                                    radius: 2
                                    color: Theme.metroStroke
                                    Rectangle {
                                        width: parent.visualPosition * parent.width
                                        height: parent.height
                                        color: Theme.metroBlue
                                        radius: 2
                                    }
                                    // Highlight selected range
                                    Rectangle {
                                        visible: exportMode
                                        x: (exportStartMs / player.duration) * parent.width
                                        width: ((exportEndMs - exportStartMs) / player.duration) * parent.width
                                        height: parent.height
                                        color: Theme.textPrimary
                                        opacity: 0.3
                                    }

                                    // Start Marker (Green)
                                    Rectangle {
                                        id: startMarker
                                        visible: exportMode
                                        width: 20; height: 24
                                        color: "transparent"
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        property bool dragging: startMouseArea.drag.active
                                        x: dragging ? x : (exportStartMs / player.duration) * parent.width - width/2
                                        
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 2; height: 24
                                            color: "green"
                                        }
                                        Rectangle {
                                            anchors.bottom: parent.bottom
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            width: 10; height: 10
                                            radius: 5
                                            color: "green"
                                        }

                                        MouseArea {
                                            id: startMouseArea
                                            anchors.fill: parent
                                            drag.target: parent
                                            drag.axis: Drag.XAxis
                                            drag.minimumX: -parent.width/2
                                            drag.maximumX: parent.parent.width - parent.width/2
                                            
                                            onPositionChanged: {
                                                if (drag.active) {
                                                    var pos = parent.x + parent.width/2
                                                    var ms = (pos / parent.parent.width) * player.duration
                                                    if (ms < 0) ms = 0
                                                    if (ms > exportEndMs) ms = exportEndMs
                                                    exportStartMs = ms
                                                }
                                            }
                                        }
                                    }

                                    // End Marker (Red)
                                    Rectangle {
                                        id: endMarker
                                        visible: exportMode
                                        width: 20; height: 24
                                        color: "transparent"
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        property bool dragging: endMouseArea.drag.active
                                        x: dragging ? x : (exportEndMs / player.duration) * parent.width - width/2
                                        
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 2; height: 24
                                            color: "red"
                                        }
                                        Rectangle {
                                            anchors.top: parent.top
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            width: 10; height: 10
                                            radius: 5
                                            color: "red"
                                        }

                                        MouseArea {
                                            id: endMouseArea
                                            anchors.fill: parent
                                            drag.target: parent
                                            drag.axis: Drag.XAxis
                                            drag.minimumX: -parent.width/2
                                            drag.maximumX: parent.parent.width - parent.width/2
                                            
                                            onPositionChanged: {
                                                if (drag.active) {
                                                    var pos = parent.x + parent.width/2
                                                    var ms = (pos / parent.parent.width) * player.duration
                                                    if (ms < exportStartMs) ms = exportStartMs
                                                    if (ms > player.duration) ms = player.duration
                                                    exportEndMs = ms
                                                }
                                            }
                                        }
                                    }
                                }
                                handle: Rectangle {
                                    x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                                    y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    radius: 8
                                    color: parent.pressed ? Theme.metroBlueHover : Theme.metroBlue
                                    border.color: parent.hovered ? Theme.textPrimary : Theme.metroStrokeStrong
                                }
                            }
                            
                            Text { 
                                text: formatTime(player.duration)
                                color: Theme.textSecondary
                            }
                        }
                    }

                    // Controls Bar
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 80
                        color: "transparent"
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 15
                            
                            // Rewind -10s
                            Button {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                text: "-10s"
                                hoverEnabled: false
                                background: Rectangle { color: "transparent"; border.color: Theme.textFaint; radius: 20 }
                                contentItem: Text { text: parent.text; color: Theme.textSecondary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 10 }
                                onClicked: player.position = Math.max(0, player.position - 10000)
                            }

                            // Play/Pause
                            Button {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                text: player.running ? "⏸" : "▶"
                                hoverEnabled: false
                                
                                background: Rectangle {
                                    color: parent.down ? Theme.metroBlueHover : Theme.metroBlue
                                    radius: 20
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.textPrimary
                                    font.pixelSize: 20
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    if (player.running) {
                                        player.running = false
                                    } else {
                                        if (currentFile) {
                                            playArchiveFile(currentFile)
                                        }
                                    }
                                }
                            }
                            
                            // Forward +10s
                            Button {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                text: "+10s"
                                hoverEnabled: false
                                background: Rectangle { color: "transparent"; border.color: Theme.textFaint; radius: 20 }
                                contentItem: Text { text: parent.text; color: Theme.textSecondary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 10 }
                                onClicked: player.position = Math.min(player.duration, player.position + 10000)
                            }
                            
                            // Stop
                            Button {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                text: "■"
                                hoverEnabled: false
                                background: Rectangle { color: "transparent"; border.color: Theme.textFaint; radius: 20 }
                                contentItem: Text { text: parent.text; color: Theme.textSecondary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                onClicked: {
                                    player.running = false
                                    player.url = ""
                                }
                            }
                            
                            // Volume
                            RowLayout {
                                spacing: 10
                                Text { text: "🔊"; color: Theme.textSecondary }
                                
                                // Normalization Toggle
                                Button {
                                    width: 26
                                    height: 26
                                    background: Rectangle { 
                                        color: player.audioNormalization ? Theme.metroBlue : "transparent"
                                        radius: 3 
                                        border.color: Theme.textFaint
                                        border.width: 1
                                    }
                                    contentItem: Text {
                                        text: "N"
                                        font.bold: true
                                        font.pixelSize: 12
                                        color: Theme.textPrimary
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    
                                    // Custom ToolTip
                                    ToolTip.visible: hovered
                                    ToolTip.text: I18n.t("Нормализация (усиление тихих звуков)")
                                    
                                    onClicked: {
                                        player.audioNormalization = !player.audioNormalization
                                    }
                                }

                                MetroSlider {
                                    from: 0
                                    to: 2.0 // Boost up to 200%
                                    value: player.volume
                                    onMoved: player.volume = value
                                    Layout.preferredWidth: 100
                                    
                                    background: Rectangle {
                                        x: parent.leftPadding
                                        y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                        implicitWidth: 200
                                        implicitHeight: 4
                                        width: parent.availableWidth
                                        height: implicitHeight
                                        radius: 2
                                        color: Theme.metroStroke
                                        Rectangle {
                                            width: parent.visualPosition * parent.width
                                            height: parent.height
                                            color: parent.visualPosition > 0.5 ? Theme.metroAmber : Theme.metroBlue
                                            radius: 2
                                        }
                                        // 100% Mark
                                        Rectangle {
                                            x: parent.width * 0.5
                                            width: 1
                                            height: 8
                                            anchors.verticalCenter: parent.verticalCenter
                                            color: Theme.textMuted
                                        }
                                    }

                                    handle: Rectangle {
                                        x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                                        y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                        implicitWidth: 16
                                        implicitHeight: 16
                                        radius: 8
                                        color: parent.pressed ? Theme.metroBlueHover : Theme.metroBlue
                                        border.color: parent.hovered ? Theme.textPrimary : Theme.metroStrokeStrong
                                    }
                                }
                            }

                            // Speed Control
                            StyledComboBox {
                                Layout.preferredWidth: 80
                                Layout.preferredHeight: 30
                                model: ["0.5x", "1.0x", "2.0x", "4.0x", "8.0x"]
                                currentIndex: 1
                                onCurrentTextChanged: {
                                    var rate = parseFloat(currentText.replace("x", ""))
                                    player.playbackRate = rate
                                }
                            }
                            
                            Item { Layout.fillWidth: true }
                            
                            // Export Controls (Visible when active)
                            RowLayout {
                                visible: exportMode
                                spacing: 10
                                
                                Button {
                                    text: I18n.t("Сохранить")
                                    hoverEnabled: false
                                    onClicked: {
                                        if (exportEndMs > exportStartMs) {
                                            // Pre-fill filename
                                            if (currentFile) {
                                                var suggestedName = "cut_" + currentFile.fileName
                                                var folder = defaultDownloadPath
                                                if (Qt.platform.os === "windows") folder = folder.replace(/\\/g, "/")
                                                if (!folder.startsWith("file:///")) folder = "file:///" + folder
                                                if (!folder.endsWith("/")) folder += "/"
                                                fileDialog.currentFile = folder + suggestedName
                                            }
                                            fileDialog.open()
                                        }
                                    }
                                    background: Rectangle { color: Theme.metroBlue; radius: Theme.metroTileRadius }
                                    contentItem: Text { text: parent.text; color: "white" }
                                }
                                Button {
                                    text: I18n.t("Отмена")
                                    hoverEnabled: false
                                    onClicked: exportMode = false
                                    background: Rectangle { color: "transparent"; radius: 4 }
                                    contentItem: Text { text: parent.text; color: Theme.textSecondary }
                                }
                            }

                            // Export Button (Toggle Mode)
                            Button {
                                visible: !exportMode
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                hoverEnabled: false
                                background: Rectangle { 
                                    color: "transparent"
                                    radius: 4 
                                }
                                contentItem: Text { 
                                    text: "✂"
                                    color: Theme.textSecondary
                                    font.pixelSize: 24
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    if (currentFile) {
                                        exportMode = true
                                        exportStartMs = 0
                                        exportEndMs = player.duration
                                    }
                                }
                            }

                            // Fullscreen Toggle
                            Button {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                hoverEnabled: false
                                background: Rectangle { 
                                    color: "transparent"
                                    radius: 4 
                                }
                                contentItem: SidebarIcon {
                                    anchors.centerIn: parent
                                    width: 24
                                    height: 24
                                    color: Theme.textSecondary
                                    name: isFullScreen ? "fullscreen_exit" : "fullscreen"
                                    path: isFullScreen 
                                        ? "M5 16h3v3h2v-5H5v2zm3-8H5v2h5V5H8v3zm6 11h2v-3h3v-2h-5v5zm2-11V5h-2v5h5V8h-3z" 
                                        : "M7 14H5v5h5v-2H7v-3zm-2-4h2V7h3V5H5v5zm12 7h-3v2h5v-5h-2v3zM14 5v2h3v3h2V5h-5z"
                                }
                                onClicked: isFullScreen = !isFullScreen
                            }
                        }
                    }
                }
            }
        }
    }

    property bool exportMode: false
    property real exportStartMs: 0
    property real exportEndMs: 0

    function formatTime(ms) {
        var totalSeconds = Math.floor(ms / 1000)
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        return (minutes < 10 ? "0" : "") + minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }

    FileDialog {
        id: fileDialog
        title: I18n.t("Сохранить видео как...")
        fileMode: FileDialog.SaveFile
        nameFilters: [I18n.t("Видео файлы (*.mp4)"), I18n.t("Все файлы (*)")]
        
        onAccepted: {
            var path = SystemController.normalizeLocalPath(selectedFile)
            
            var index = archiveSidebar.currentIndex
            if (index < 0) return
            var sourcePath = SystemController.archiveController.searchResults[index].filePath
            
            SystemController.archiveController.exportVideo(sourcePath, path, exportStartMs, exportEndMs)
            exportMode = false
        }
    }

}
