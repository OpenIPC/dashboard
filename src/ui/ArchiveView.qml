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

    onCurrentCameraIpChanged: {
        if (currentCameraIp === "") return
        for (var i = 0; i < cameraSelector.count; i++) {
            var cam = SystemController.cameraModel.getCamera(i)
            if (cam && cam.cameraIp === currentCameraIp) {
                cameraSelector.currentIndex = i
                break
            }
        }
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
            if (count === 0) {
                infoDialog.text = I18n.t("Записи не найдены за выбранный период.")
                infoDialog.open()
            }
        }
    }

    MessageDialog {
        id: errorDialog
        title: I18n.t("Ошибка")
        buttons: MessageDialog.Ok
    }

    MessageDialog {
        id: infoDialog
        title: I18n.t("Информация")
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

        // LEFT SIDEBAR
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: Theme.metroTile
            visible: !isFullScreen
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                anchors.topMargin: 40 // Increased top margin for better visibility
                spacing: 15

                // Camera Selection
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Label { text: I18n.t("Камера"); color: Theme.textSecondary; font.bold: true }
                    StyledComboBox {
                        id: cameraSelector
                        Layout.fillWidth: true
                        textRole: "cameraName"
                        model: SystemController.cameraModel
                    }
                }

                // Date Selection
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Label { text: I18n.t("Начало"); color: Theme.textSecondary; font.bold: true }
                    RowLayout {
                        TextField {
                            id: startTimeField
                            Layout.fillWidth: true
                            text: new Date(new Date().setHours(0,0,0,0)).toLocaleString(Qt.locale(), "yyyy-MM-dd HH:mm:ss")
                            color: Theme.textSecondary
                            background: Rectangle { color: Theme.metroBackground; border.color: Theme.metroStroke; radius: 2 }
                        }
                        Button {
                            text: "📅"
                            Layout.preferredWidth: 30
                            background: Rectangle { color: Theme.metroTile; radius: 2 }
                            contentItem: Text { text: parent.text; color: Theme.textSecondary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: { calendarPopup.targetField = startTimeField; calendarPopup.open() }
                        }
                    }

                    Label { text: I18n.t("Конец"); color: Theme.textSecondary; font.bold: true }
                    RowLayout {
                        TextField {
                            id: endTimeField
                            Layout.fillWidth: true
                            text: new Date(new Date().setHours(23,59,59,999)).toLocaleString(Qt.locale(), "yyyy-MM-dd HH:mm:ss")
                            color: Theme.textSecondary
                            background: Rectangle { color: Theme.metroBackground; border.color: Theme.metroStroke; radius: 2 }
                        }
                        Button {
                            text: "📅"
                            Layout.preferredWidth: 30
                            background: Rectangle { color: Theme.metroTile; radius: 2 }
                            contentItem: Text { text: parent.text; color: Theme.textSecondary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: { calendarPopup.targetField = endTimeField; calendarPopup.open() }
                        }
                    }
                }

                // Search Button
                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    text: SystemController.archiveController.isSearching ? I18n.t("Поиск") + "..." : I18n.t("Найти")
                    enabled: !SystemController.archiveController.isSearching
                    
                    background: Rectangle {
                        color: parent.down ? Theme.metroBlueHover : Theme.metroBlue
                        radius: Theme.metroTileRadius
                        opacity: parent.enabled ? 1 : 0.5
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.textPrimary
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        var camIndex = cameraSelector.currentIndex
                        if (camIndex >= 0) {
                            var cam = SystemController.cameraModel.getCamera(camIndex)
                            SystemController.archiveController.login(cam.cameraIp, cam.cameraPort, cam.cameraLogin, "")
                            
                            var start = Date.fromLocaleString(Qt.locale(), startTimeField.text, "yyyy-MM-dd HH:mm:ss")
                            var end = Date.fromLocaleString(Qt.locale(), endTimeField.text, "yyyy-MM-dd HH:mm:ss")
                            
                            var settings = SystemController.getAppSettings()
                            var recPath = (settings && settings.recordingsPath) ? settings.recordingsPath : ""
                            
                            SystemController.archiveController.search(start, end, cam.cameraIp, recPath)
                        }
                    }
                }

                // Results List
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: Theme.metroBackground
                    border.color: Theme.metroTile
                    radius: 2
                    
                    ListView {
                        id: resultsList
                        anchors.fill: parent
                        anchors.margins: 1
                        clip: true
                        model: SystemController.archiveController.searchResults
                        
                        delegate: ItemDelegate {
                            width: ListView.view.width
                            height: 50
                            highlighted: ListView.isCurrentItem
                            
                            background: Rectangle {
                                color: highlighted ? Theme.metroBlue : (index % 2 == 0 ? Theme.metroSidebarBackground : Theme.metroTile)
                            }
                            
                            contentItem: RowLayout {
                                spacing: 10
                                Image { 
                                    source: "qrc:/OpenIPC/src/ui/SidebarIcon.qml" // Placeholder icon
                                    sourceSize.width: 24; sourceSize.height: 24
                                    visible: false
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text { 
                                        text: modelData.fileName
                                        color: Theme.textPrimary
                                        font.pixelSize: 12
                                        elide: Text.ElideMiddle
                                        Layout.fillWidth: true
                                    }
                                    Text { 
                                        text: modelData.startTime.toLocaleString(Qt.locale(), "HH:mm:ss") + " - " + modelData.endTime.toLocaleString(Qt.locale(), "HH:mm:ss")
                                        color: Theme.textMuted
                                        font.pixelSize: 10
                                    }
                                }
                                Button {
                                    Layout.preferredWidth: 30
                                    Layout.preferredHeight: 30
                                    hoverEnabled: false
                                    background: Rectangle { color: "transparent" }
                                    contentItem: SidebarIcon {
                                        anchors.centerIn: parent
                                        width: 20
                                        height: 20
                                        color: Theme.textSecondary
                                        name: "folder_open"
                                        path: "M10 4H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2h-8l-2-2z"
                                    }
                                    onClicked: {
                                        SystemController.openFolder(modelData.filePath)
                                    }
                                }
                            }
                            onClicked: {
                                resultsList.currentIndex = index
                                currentFile = modelData
                                // Auto-play
                                var path = defaultDownloadPath + "/" + modelData.fileName
                                player.url = "file:///" + path
                                player.running = true
                            }
                        }
                        
                        ScrollBar.vertical: ScrollBar { }
                    }
                }
            }
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
                                            var path = defaultDownloadPath + "/" + currentFile.fileName
                                            player.url = "file:///" + path
                                            player.running = true
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
            
            var index = resultsList.currentIndex
            var sourcePath = SystemController.archiveController.searchResults[index].filePath
            
            SystemController.archiveController.exportVideo(sourcePath, path, exportStartMs, exportEndMs)
            exportMode = false
        }
    }

    Popup {
        id: calendarPopup
        width: 300
        height: 320
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        
        property var targetField: null
        property date selectedDate: new Date()

        background: Rectangle {
            color: Theme.metroSidebarBackground
            border.color: Theme.metroBlue
            radius: 4
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            
            RowLayout {
                Layout.fillWidth: true
                Button {
                    text: "<"
                    Layout.preferredWidth: 30
                    onClicked: {
                        var d = new Date(calendarPopup.selectedDate)
                        d.setMonth(d.getMonth() - 1)
                        calendarPopup.selectedDate = d
                    }
                    background: Rectangle { color: Theme.metroTile; radius: 2 }
                    contentItem: Text { text: parent.text; color: Theme.textSecondary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
                Label {
                    text: calendarPopup.selectedDate.toLocaleString(Qt.locale(), "MMMM yyyy")
                    color: "white"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.bold: true
                }
                Button {
                    text: ">"
                    Layout.preferredWidth: 30
                    onClicked: {
                        var d = new Date(calendarPopup.selectedDate)
                        d.setMonth(d.getMonth() + 1)
                        calendarPopup.selectedDate = d
                    }
                    background: Rectangle { color: Theme.metroTile; radius: 2 }
                    contentItem: Text { text: parent.text; color: Theme.textSecondary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
            }

            DayOfWeekRow {
                Layout.fillWidth: true
                delegate: Text {
                    text: model.shortName
                    color: Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 12
                }
            }

            MonthGrid {
                Layout.fillWidth: true
                Layout.fillHeight: true
                month: calendarPopup.selectedDate.getMonth()
                year: calendarPopup.selectedDate.getFullYear()
                
                delegate: ItemDelegate {
                    text: model.day
                    
                    highlighted: {
                        var d = model.date
                        return d.getDate() === calendarPopup.selectedDate.getDate() && 
                               d.getMonth() === calendarPopup.selectedDate.getMonth() &&
                               d.getFullYear() === calendarPopup.selectedDate.getFullYear()
                    }
                    onClicked: {
                        calendarPopup.selectedDate = model.date
                        
                        var currentText = calendarPopup.targetField.text
                        var timePart = "00:00:00"
                        if (currentText.includes(" ")) {
                            timePart = currentText.split(" ")[1]
                        }
                        
                        var newDateStr = model.date.toLocaleString(Qt.locale(), "yyyy-MM-dd")
                        calendarPopup.targetField.text = newDateStr + " " + timePart
                        calendarPopup.close()
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: parent.highlighted ? "white" : Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.highlighted ? Theme.metroBlue : "transparent"
                        radius: 2
                    }
                }
            }
        }
    }
}

