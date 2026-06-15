import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtMultimedia
import OpenIPC

Item {
    id: root
    
    property string cameraName: ""
    property string cameraIp: ""
    property int cameraPort: 554
    property int cameraOnvifPort: 80
    property string cameraLogin: "admin"
    property string streamUrl: ""
    property string sdStreamUrl: ""
    property string hdStreamUrl: ""
    property string status: ""
    property bool isSelected: false
    property bool isRecording: recorder.recordingPath !== ""
    property bool isMuted: true
    property bool audioNormalization: false
    property real volume: 1.0
    property bool useHdPreview: false
    property bool useHdFullscreen: true
    property string manufacturer: ""
    property int recordingSegmentDuration: (SystemController.appSettings.recordingSegmentDuration !== undefined) ? SystemController.appSettings.recordingSegmentDuration : 15
    property bool analyticsActive: false
    property bool eventRecordingActive: eventRecorder.recordingPath !== ""
    property bool canLive: true
    property bool canPlayback: true
    property bool canPtz: true
    property bool canExport: true
    property bool canSettings: true
    property bool effectiveCanLive: canLive || SystemController.userManager.isAdmin() || (SystemController.userManager.currentUser && SystemController.userManager.currentUser.username === "admin")
    readonly property bool hasCamera: root.cameraIp !== "" || root.cameraName !== "" || root.streamUrl !== ""
    readonly property bool statusOnline: String(root.status || "").toLowerCase() === "online"
    property double lastFrameSeenMs: 0
    property double lastStatusPushMs: 0
    
    // UI Scaling for small cells
    // Start scaling down when width is below 600px to prevent overlap of stats and name
    property real uiScale: Math.min(1.0, Math.max(0.4, root.width / 600))
    
    // Grid resizing props
    property var gridParent: null
    property int totalRows: 1
    property int totalCols: 1
    property int spanRows: 1
    property int spanCols: 1
    property int gridIndex: -1
    property real unitWidth: 10
    property real unitHeight: 10
    
    // Explicitly ignore implicit size of children
    implicitWidth: 0
    implicitHeight: 0

    // Force size based on span and unit size
    Layout.preferredWidth: spanCols * unitWidth
    Layout.preferredHeight: spanRows * unitHeight

    function getHwDecoders(mode) {
        // Prioritize D3D11/MFT on Windows for stability, even for Nvidia
        if (mode === "nvidia") return ["MFT:d3d=11", "D3D11", "CUDA", "NVDEC", "FFmpeg"]
        if (mode === "intel") return ["MFT:d3d=11", "D3D11", "FFmpeg"]
        if (mode === "dxva") return ["D3D11", "DXVA", "FFmpeg"]
        if (mode === "none") return ["FFmpeg"]
        // auto
        return ["MFT:d3d=11", "D3D11", "CUDA", "FFmpeg"]
    }

    Component.onCompleted: {
        console.info("GridCell completed", index, cameraIp, streamUrl)
        refreshAnalyticsActive()
    }

    function refreshAnalyticsActive() {
        var active = false
        if (SystemController.analyticsEngine.isModuleEnabled(0) && SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 0)) active = true
        if (SystemController.analyticsEngine.isModuleEnabled(1) && SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 1)) active = true
        if (SystemController.analyticsEngine.isModuleEnabled(2) && SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 2)) active = true
        analyticsActive = active
        if (!analyticsActive) {
            detectionModel.clear()
        }
    }

    function pushCameraStatus(newStatus) {
        if (!root.hasCamera || root.cameraIp === "" || !SystemController.updateCameraStatus)
            return

        var now = Date.now()
        if (newStatus === "Online") {
            root.lastFrameSeenMs = now
            if (root.statusOnline && (now - root.lastStatusPushMs) < 10000)
                return
        } else if ((now - root.lastFrameSeenMs) < 8000) {
            return
        }

        root.lastStatusPushMs = now
        SystemController.updateCameraStatus(root.cameraIp, newStatus)
    }

    function startEventClip(path, durationMs) {
        if (!path || path === "") return
        if (eventRecorder.recordingPath !== "") {
            return
        }
        eventRecorder.recordingPath = path
        eventClipTimer.stop()
    }

    function stopEventClip() {
        if (eventRecorder.recordingPath !== "") {
            eventRecorder.recordingPath = ""
        }
        eventClipTimer.stop()
    }

    function statusCaption() {
        if (!hasCamera) return I18n.t("Свободно")
        if (root.status && root.status !== "") return I18n.t(root.status)
        return root.streamUrl === "" ? I18n.t("Нет потока") : I18n.t("Подключение")
    }

    onStreamUrlChanged: {
        console.log("GridCell streamUrl changed:", streamUrl)
        if (streamUrl === "") {
            hdPlayer.running = false
            hdWindow.visible = false
            useHdPreview = false
            useHdFullscreen = true
        }
    }

    onSdStreamUrlChanged: {
        console.log("GridCell sdStreamUrl changed:", sdStreamUrl)
        // Ensure player picks up the change if we are in SD mode
        if (!root.useHdPreview && root.sdStreamUrl !== "") {
             // Force re-evaluation might be needed if player is running
             var currentUrl = player.url
             var newUrl = root.sdStreamUrl
             if (currentUrl !== newUrl) {
                 console.log("GridCell switching to new SD URL:", newUrl)
                 // This binding update should be automatic, but logging helps confirm it.
             }
        }
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    readonly property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"
    
    signal closeClicked()
    signal editRequested()
    signal deleteRequested()
    signal archiveRequested()
    signal snapshotClicked()
    signal recordClicked()
    signal audioClicked()
    signal permissionDenied()
    
    // Shared detection model for both preview and fullscreen
    ListModel { id: detectionModel }

    Connections {
        target: SystemController.analyticsEngine
        function onFrameProcessed(cameraId, detections) {
            if (cameraId === root.cameraIp) {
                if (!analyticsActive) {
                    detectionModel.clear()
                    return
                }
                detectionModel.clear()
                for (var i = 0; i < detections.length; i++) {
                    var detection = detections[i]
                    var moduleId = detection.moduleId
                    
                    var color = "red"
                    if (moduleId === "Face Detector") color = "#ff7f50"
                    else if (moduleId === "Object Counter") color = "#2563eb"
                    else if (moduleId === "License Plate") color = "#ffff00"
                    
                    detection.color = color
                    detectionModel.append(detection)
                }
            }
        }
        function onModuleStatusChanged() { refreshAnalyticsActive() }
        function onClipRequested(cameraId, path, durationMs) {
            if (cameraId === root.cameraIp) {
                startEventClip(path, durationMs)
            }
        }
        function onClipStopRequested(cameraId, path) {
            if (cameraId === root.cameraIp) {
                stopEventClip()
            }
        }
    }

    // Background
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        border.color: root.isRecording ? Theme.danger : (root.isSelected ? Theme.accent : Theme.textFaint)
        border.width: root.isRecording ? 2 : (root.isSelected ? 2 : 1)
        radius: Theme.radiusSm

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: (mouse) => {
                if (root.cameraIp !== "") {
                    gridContextMenu.cameraIp = root.cameraIp
                    gridContextMenu.cameraName = root.cameraName
                    gridContextMenu.popup()
                }
            }
        }
    }

    CameraContextMenu {
        id: gridContextMenu
        isGridContext: true
        canLive: root.canLive
        canPlayback: root.canPlayback
        canSettings: root.canSettings
        canExport: root.canExport
        onDeleteRequested: {
            root.deleteRequested()
        }
        onEditRequested: {
             root.editRequested()
        }
        onArchiveRequested: {
            root.archiveRequested()
        }
    }

    // Video Player fill parent, clip to cell (no fixed aspect)
    Item {
        id: videoHolder
        anchors.fill: parent
        clip: true
        visible: root.streamUrl !== "" && root.effectiveCanLive

        VideoPlayer {
            id: player
            anchors.fill: parent
            visible: root.streamUrl !== "" && !hdWindow.visible

            // Respect app setting (Crop/Fit/Stretch)
            fillMode: (SystemController.appSettings.playerFillMode !== undefined) ? SystemController.appSettings.playerFillMode : 1.0
            
            // Orientation
            orientation: (SystemController.appSettings.playerOrientation !== undefined) ? SystemController.appSettings.playerOrientation : 0
            mirror: (SystemController.appSettings.playerMirror !== undefined) ? SystemController.appSettings.playerMirror : false
            
            property bool fallbackToSd: false
            url: (root.useHdPreview && !fallbackToSd) && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
            
            // Start only if visible and URL is valid
            running: visible && root.streamUrl !== "" && root.effectiveCanLive
            
            muted: {
                if (SystemController.isArchiveOpen) return true
                return root.isMuted
            }
            volume: root.volume
            audioNormalization: root.audioNormalization
            hwDecoders: root.getHwDecoders(SystemController.appSettings.hwAccel || "auto") // Keep legacy for now
            hwDecoding: (SystemController.appSettings.playerHwDecoding !== undefined) ? SystemController.appSettings.playerHwDecoding : "auto"
            
            // Adjustments
            brightness: (SystemController.appSettings.playerBrightness !== undefined) ? SystemController.appSettings.playerBrightness : 1.0
            contrast: (SystemController.appSettings.playerContrast !== undefined) ? SystemController.appSettings.playerContrast : 1.0
            hue: (SystemController.appSettings.playerHue !== undefined) ? SystemController.appSettings.playerHue : 0
            saturation: (SystemController.appSettings.playerSaturation !== undefined) ? SystemController.appSettings.playerSaturation : 1.0
            gamma: (SystemController.appSettings.playerGamma !== undefined) ? SystemController.appSettings.playerGamma : 1.0
            
            onBrightnessChanged: console.log("GridCell brightness updated:", brightness)
            
            bufferMode: (SystemController.appSettings.playerBufferMode !== undefined) ? SystemController.appSettings.playerBufferMode : 1
            rtspTransport: (SystemController.appSettings.playerRtspTransport !== undefined) ? SystemController.appSettings.playerRtspTransport : "tcp"
            
            analyticsUrl: ""
            analyticsEngine: null
            cameraId: root.cameraIp
            
            property string lastError: ""
            onFrameReady: root.pushCameraStatus("Online")
            onErrorOccurred: function(msg) {
                lastError = msg;
                errorTimer.restart();
                root.pushCameraStatus("Offline")
                
                // Auto-fallback to SD if HD fails
                if (root.useHdPreview && !fallbackToSd && root.sdStreamUrl !== "") {
                    console.warn("HD stream failed, falling back to SD stream for camera:", root.cameraIp);
                    fallbackToSd = true;
                }
            }
            
            Timer {
                id: errorTimer
                interval: 5000
                onTriggered: player.lastError = ""
            }
            
            Rectangle {
                anchors.centerIn: parent
                width: errorText.width + 20
                height: errorText.height + 10
                color: "#80000000"
                radius: 4
                visible: player.lastError !== ""
                
                Text {
                    id: errorText
                    anchors.centerIn: parent
                    text: player.lastError
                    color: "#ff5555"
                    font.pixelSize: 12 * root.uiScale
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // Hidden analytics player (HD preferred) to ensure snapshots/clips use HD stream
        VideoPlayer {
            id: analyticsPlayer
            visible: false
            backgroundMode: true
            url: root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
            running: analyticsActive && url !== ""
            muted: true
            rtspTransport: player.rtspTransport
            bufferMode: player.bufferMode
            analyticsEngine: SystemController.analyticsEngine
            cameraId: root.cameraIp
        }

        // Background Recorder (HD preferred)
        VideoPlayer {
            id: recorder
            visible: false 
            backgroundMode: true
            
            // Always prefer HD for recording
            url: root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
            
            // Run only when recording path is set
            running: recordingPath !== ""
            
            // Ensure audio is captured
            muted: false
            volume: 1.0
            
            // Match transport settings
            rtspTransport: player.rtspTransport
            bufferMode: player.bufferMode
        }

        // Event Recorder (HD preferred, includes audio)
        VideoPlayer {
            id: eventRecorder
            visible: false
            backgroundMode: true
            url: root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
            running: recordingPath !== ""
            muted: false
            volume: 1.0
            rtspTransport: player.rtspTransport
            bufferMode: player.bufferMode
        }
        
        Timer {
            id: reconnectTimer
            interval: 5000
            repeat: false
            onTriggered: {
                if (player.running) {
                    console.info("Reconnecting stream...", root.cameraIp)
                    var currentUrl = player.url
                    player.url = ""
                    player.url = currentUrl
                }
            }
        }

        // Detection Overlay
        Item {
            id: detectionOverlay
            anchors.fill: parent
            visible: analyticsActive
            
            Repeater {
                model: detectionModel
                delegate: Rectangle {
                    x: model.x * detectionOverlay.width
                    y: model.y * detectionOverlay.height
                    width: model.w * detectionOverlay.width
                    height: model.h * detectionOverlay.height
                    color: "transparent"
                    border.color: model.color || "red"
                    border.width: 2
                    
                    Rectangle {
                        anchors.bottom: parent.top
                        anchors.left: parent.left
                        color: parent.border.color
                        height: 16
                        width: labelText.width + 8
                        
                        Text {
                            id: labelText
                            anchors.centerIn: parent
                            text: model.label
                                  + ((model.trackId !== undefined && model.trackId !== "") ? (" #" + model.trackId) : "")
                                  + " " + Math.round(model.confidence * 100) + "%"
                            color: "white"
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }
                }
            }
        }
    }

    // No-permission overlay for live view
    Rectangle {
        anchors.fill: parent
        color: "#88000000"
        visible: !root.effectiveCanLive && root.streamUrl !== ""
        z: 20
        Text {
            anchors.centerIn: parent
            text: I18n.t("Нет доступа")
            color: "white"
            font.bold: true
            font.pixelSize: 16
        }
    }

    // Placeholder / Loading / Error
    Item {
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        visible: !player.running // Simplified logic
        
        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 24, 260)
            spacing: 8

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                width: 42
                height: 42
                radius: 10
                color: root.hasCamera ? "#151b26" : "#101827"
                border.color: root.hasCamera ? "#303848" : Theme.controlBorder

                Text {
                    anchors.centerIn: parent
                    text: root.hasCamera ? "videocam" : "add"
                    color: root.hasCamera ? Theme.textMuted : Theme.accentHover
                    font.family: root.iconFontFamily
                    font.pixelSize: 22
                }
            }
            
            Text {
                Layout.fillWidth: true
                text: root.hasCamera ? root.statusCaption() : I18n.t("Свободная ячейка")
                color: root.hasCamera ? Theme.textMuted : Theme.textSecondary
                font.pixelSize: 14
                font.bold: !root.hasCamera
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                visible: !root.hasCamera
                text: I18n.t("Перетащите камеру из списка устройств")
                color: Theme.textFaint
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                visible: root.hasCamera && player.lastError !== ""
                text: player.lastError
                color: Theme.danger
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 8
        scale: root.uiScale
        transformOrigin: Item.TopLeft
        visible: root.hasCamera
        height: 24
        width: Math.min(statusLabel.implicitWidth + 30, (root.width * 0.62) / root.uiScale)
        radius: Theme.radiusMd
        color: "#d0000000"
        border.color: Theme.overlayBorder
        border.width: 1
        clip: true
        z: 6

        Row {
            id: statusOverlayRow
            anchors.centerIn: parent
            width: parent.width - 16
            spacing: 6

            Rectangle {
                width: 8
                height: 8
                radius: 4
                anchors.verticalCenter: parent.verticalCenter
                color: root.statusOnline ? Theme.success : Theme.danger
            }

            Text {
                id: statusLabel
                text: root.statusCaption()
                color: Theme.textPrimary
                font.pixelSize: 11
                font.bold: true
                width: parent.width - 14
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 38 * root.uiScale
        anchors.leftMargin: 8 * root.uiScale
        scale: root.uiScale
        transformOrigin: Item.TopLeft
        visible: root.hasCamera && (root.analyticsActive || root.isRecording || root.eventRecordingActive)
        height: 22
        width: Math.min(flagsRow.implicitWidth + 14, (root.width * 0.62) / root.uiScale)
        radius: Theme.radiusMd
        color: "#d0000000"
        border.color: Theme.overlayBorder
        border.width: 1
        clip: true
        z: 6

        Row {
            id: flagsRow
            anchors.centerIn: parent
            width: parent.width - 14
            spacing: 8

            Text {
                visible: root.analyticsActive
                text: "AI"
                color: Theme.accentHover
                font.pixelSize: 10
                font.bold: true
            }

            Text {
                visible: root.isRecording
                text: "REC"
                color: Theme.danger
                font.pixelSize: 10
                font.bold: true
            }

            Text {
                visible: root.eventRecordingActive
                text: "EVT"
                color: Theme.warning
                font.pixelSize: 10
                font.bold: true
            }
        }
    }

    // Hover Area for Controls & Drag
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onDoubleClicked: {
            if (hdWindow.visible) {
                hdWindow.close()
            } else {
                hdPlayer.url = root.useHdFullscreen && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
                hdPlayer.running = true
                hdWindow.showFullScreen()
            }
        }
        
        drag.target: dragItem
        
        onPressed: {
            root.isSelected = true
            // Prepare drag item
            dragItem.parent = root.parent.parent // Move to higher z-index layer if possible, or just use Drag.active
            dragItem.anchors.fill = undefined
            dragItem.width = root.width
            dragItem.height = root.height
            dragItem.x = root.x
            dragItem.y = root.y
        }
        
        onReleased: {
            dragItem.Drag.drop()
            // Reset drag item
            dragItem.parent = root
            dragItem.anchors.fill = parent
            dragItem.x = 0
            dragItem.y = 0
        }
    }
    
    // Drag Visual Proxy (Invisible usually, visible when dragging)
    Rectangle {
        id: dragItem
        anchors.fill: parent
        color: "#33ffffff"
        visible: hoverArea.drag.active
        
        Drag.active: hoverArea.drag.active
        Drag.keys: ["grid-cell"]
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Drag.mimeData: { "grid-index": index } // Pass index
        
        // Visual feedback content
        Text {
            anchors.centerIn: parent
            text: root.cameraIp
            color: "white"
        }
    }
    
    // Drop Area to accept drops
    DropArea {
        anchors.fill: parent
        keys: ["grid-cell", "camera"]

        onEntered: (drag) => {
            // Visual feedback for drop target?
            root.border.color = Theme.success
            drag.accept(Qt.MoveAction)
        }
        onExited: {
            root.border.color = root.isSelected ? Theme.accent : Theme.textFaint
        }
        onDropped: (drop) => {
            // Handle grid cell swap
            if (drop.keys.indexOf("grid-cell") >= 0) {
                if (drop.source && drop.source.Drag && drop.source.Drag.mimeData) {
                    var fromIndex = drop.source.Drag.mimeData["grid-index"]
                    var toIndex = index
                    if (fromIndex !== undefined && fromIndex !== toIndex) {
                        SystemController.gridModel.swapCameras(fromIndex, toIndex)
                    }
                }
            }
            // Handle new camera from sidebar
            else if (drop.keys.indexOf("camera") >= 0) {
                var idx = drop.mimeData.getData("application/camera-index")
                if (idx !== undefined && idx !== null) {
                    SystemController.addCameraToGrid(parseInt(idx), index)
                }
            }

            root.border.color = root.isSelected ? Theme.accent : Theme.textFaint
            drop.accept()
        }
    }

    // Fullscreen HD window on double-click
    Window {
        id: hdWindow
        modality: Qt.NonModal
        visible: false
        color: "black"
        flags: Qt.Window | Qt.WindowFullscreenButtonHint | Qt.FramelessWindowHint

        onClosing: {
            hdPlayer.running = false
            hdPlayer.url = ""
        }

        Item {
            anchors.fill: parent
            clip: true

            VideoPlayer {
                id: hdPlayer
                anchors.fill: parent
                // Respect app setting (Crop/Fit/Stretch)
                fillMode: (SystemController.appSettings.playerFillMode !== undefined) ? SystemController.appSettings.playerFillMode : 1.0
                
                // Use app settings for orientation
                orientation: (SystemController.appSettings.playerOrientation !== undefined) ? SystemController.appSettings.playerOrientation : 0
                mirror: (SystemController.appSettings.playerMirror !== undefined) ? SystemController.appSettings.playerMirror : false

                url: root.useHdFullscreen && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
                
                // Auto run when window actsally visible
                running: hdWindow.visible && url !== ""
                
                muted: root.isMuted
                volume: root.volume
                hwDecoders: root.getHwDecoders(SystemController.appSettings.hwAccel || "auto")
                bufferMode: (SystemController.appSettings.playerBufferMode !== undefined) ? SystemController.appSettings.playerBufferMode : 1
                rtspTransport: (SystemController.appSettings.playerRtspTransport !== undefined) ? SystemController.appSettings.playerRtspTransport : "tcp"
                
                // Disable analytics for fullscreen player to prevent race conditions/double processing
                // The preview player continues to run in the background and provides detections
                analyticsEngine: null
                cameraId: root.cameraIp
            }

            // Fullscreen Detection Overlay
            Item {
                id: hdDetectionOverlay
                anchors.fill: parent
                
                Repeater {
                    model: detectionModel
                    delegate: Rectangle {
                        x: model.x * hdDetectionOverlay.width
                        y: model.y * hdDetectionOverlay.height
                        width: model.w * hdDetectionOverlay.width
                        height: model.h * hdDetectionOverlay.height
                        color: "transparent"
                        border.color: model.color || "red"
                        border.width: 2
                        
                        Rectangle {
                            anchors.bottom: parent.top
                            anchors.left: parent.left
                            color: parent.border.color
                            height: 16
                            width: hdLabelText.width + 8
                            
                            Text {
                                id: hdLabelText
                                anchors.centerIn: parent
                                text: model.label
                                      + ((model.trackId !== undefined && model.trackId !== "") ? (" #" + model.trackId) : "")
                                      + " " + Math.round(model.confidence * 100) + "%"
                                color: "white"
                                font.pixelSize: 12
                                font.bold: true
                            }
                        }
                    }
                }
            }
            /*
            Item {
                id: hdPlayer
                anchors.fill: parent
                property string url: root.useHdFullscreen && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
                property bool running: false
                property int fillMode: -1
                property bool muted: root.isMuted
                property real volume: root.volume
                Text { anchors.centerIn: parent; text: "Streaming Disabled (HD)"; color: "red" }
            }
            */

            // PTZ Overlay for Fullscreen
            Item {
                id: hdPtzOverlay
                anchors.fill: parent
                property bool ptzVisible: false
                visible: root.canPtz && ptzVisible
                z: 10

                PtzControlPanel {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.bottomMargin: 50
                    
                    cameraIp: root.cameraIp
                    cameraPort: root.cameraOnvifPort
                    cameraLogin: root.cameraLogin
                    iconFontFamily: root.iconFontFamily
                    compact: false
                }
            }

            // Allow leaving fullscreen with a double click anywhere on the video
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onDoubleClicked: hdWindow.close()
            }
        }


            // Top right fullscreen controls to toggle quality and exit
            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 12
                height: 40
                width: hdControlsRow.implicitWidth + 16
                color: "#cc000000"
                radius: 6
                border.color: "#55ffffff"
                border.width: 1

                Row {
                    id: hdControlsRow
                    anchors.centerIn: parent
                    spacing: 8

                    // PTZ Toggle
                    Button {
                        width: 42
                        height: 28
                        background: Rectangle { color: hdPtzOverlay.visible ? "#44ffffff" : "transparent"; radius: 4 }
                        contentItem: Text {
                            text: "control_camera"
                            font.family: root.iconFontFamily
                            font.pixelSize: 18
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (!root.canPtz) { root.permissionDenied(); return }
                            hdPtzOverlay.ptzVisible = !hdPtzOverlay.ptzVisible
                        }
                    }

                    Button {
                        width: 42
                        height: 28
                        background: Rectangle { color: "transparent"; radius: 4 }
                        contentItem: Text {
                            text: root.useHdFullscreen && root.hdStreamUrl !== "" ? "hd" : "sd"
                            font.family: root.iconFontFamily
                            font.pixelSize: 18
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            root.useHdFullscreen = !root.useHdFullscreen
                            hdPlayer.url = root.useHdFullscreen && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
                            hdPlayer.running = true
                        }
                    }

                    Button {
                        width: 42
                        height: 28
                        background: Rectangle { color: "transparent"; radius: 4 }
                        contentItem: Text {
                            text: "fullscreen_exit"
                            font.family: root.iconFontFamily
                            font.pixelSize: 18
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: hdWindow.close()
                    }
                }
            }

            // Live stats for fullscreen stream
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.margins: 12
                color: "#e0000000"
                radius: 6
                visible: (SystemController.appSettings.showStatsOverlay === undefined || SystemController.appSettings.showStatsOverlay) && hdStatsText() !== ""
                height: 26
                width: Math.max(120, hdStatsLabel.implicitWidth + 16)
                border.color: "#44ffffff"
                border.width: 1

                Text {
                    id: hdStatsLabel
                    anchors.centerIn: parent
                    text: hdStatsText()
                    color: "white"
                    font.pixelSize: 12
                    font.family: "monospace"
                    font.bold: true
                }
            }

            // Preview vs fullscreen quality indicator so user sees which stream is where
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 12
                color: "#e0000000"
                radius: 6
                height: 26
                width: qualityLabel.implicitWidth + 20
                border.color: "#44ffffff"
                border.width: 1

                Text {
                    id: qualityLabel
                    anchors.centerIn: parent
                    text: "Preview: " + (root.useHdPreview && root.hdStreamUrl !== "" && root.hdStreamUrl !== root.sdStreamUrl ? "HD" : "SD") + "  |  Fullscreen: " + (root.useHdFullscreen && root.hdStreamUrl !== "" && root.hdStreamUrl !== root.sdStreamUrl ? "HD" : "SD")
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        // Keys.onEscapePressed: hdWindow.close()
    }

    // Recording Indicator
    Rectangle {
        width: 12
        height: 12
        radius: 6
        color: "red"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 8
        visible: root.isRecording
        
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            running: root.isRecording
            PropertyAnimation { to: 0.2; duration: 800 }
            PropertyAnimation { to: 1.0; duration: 800 }
        }
    }

    // Stream stats (codec / resolution / bitrate / fps)
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 8
        scale: root.uiScale
        transformOrigin: Item.BottomLeft
        color: "#e0000000" // darker for readability
        radius: Theme.radiusMd
        visible: (SystemController.appSettings.showStatsOverlay === undefined || SystemController.appSettings.showStatsOverlay) && statsText() !== ""
        height: 30
        // Limit width to 45% of cell width (accounting for scale) to prevent overlap
        width: Math.min(Math.max(110, statsLabel.implicitWidth + 18), (root.width * 0.45) / root.uiScale)
        border.color: Theme.overlayBorder
        border.width: 1
        clip: true

        Text {
            id: statsLabel
            anchors.centerIn: parent
            text: statsText()
            color: "white"
            font.pixelSize: 10
            font.family: "monospace"
            font.bold: true
            elide: Text.ElideRight
            width: parent.width - 18
            horizontalAlignment: Text.AlignHCenter
        }
    }

    // Helper functions for stats
    function statsText() {
        if (!player || !player.running) return ""
        // Use properties from GstPlayer if available
        var codec = player.videoCodec || "H264"
        var w = player.videoWidth || 0
        var h = player.videoHeight || 0
        var fps = player.videoFps || 0
        var bitrate = player.videoBitrate || 0
        
        if (w === 0 || h === 0) return ""
        
        var txt = codec + " " + w + "x" + h
        if (fps > 0) txt += " " + fps + "fps"
        if (bitrate > 0) txt += " " + bitrate + "kbps"
        return txt
    }
    
    function hdStatsText() {
        if (!hdPlayer || !hdPlayer.running) return ""
        var codec = hdPlayer.videoCodec || "H264"
        var w = hdPlayer.videoWidth || 0
        var h = hdPlayer.videoHeight || 0
        var fps = hdPlayer.videoFps || 0
        var bitrate = hdPlayer.videoBitrate || 0
        
        if (w === 0 || h === 0) return ""
        return codec + " " + w + "x" + h + " " + fps + "fps " + bitrate + "kbps"
    }

    // Periodic stats refresh to keep bitrate/fps live
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            if (typeof player.updateMediaInfo === 'function') {
                player.updateMediaInfo()
            }
        }
    }
    
    // Segmented Recording Timer
    Timer {
        id: segmentTimer
        interval: root.recordingSegmentDuration * 60 * 1000
        repeat: true
        running: false
        onTriggered: {
            if (recorder.recordingPath !== "") {
                console.info("Segment limit reached. Splitting recording...")
                // Stop (Internal flush)
                recorder.recordingPath = ""
                // Start new segment
                var path = SystemController.generateRecordingPath(root.cameraIp)
                recorder.recordingPath = path
                console.info("New segment started to", path)
            } else {
                running = false
            }
        }
    }

    // Event clip stop timer
    Timer {
        id: eventClipTimer
        interval: 5000
        repeat: false
        onTriggered: {
            eventRecorder.recordingPath = ""
        }
    }

    // Keep fullscreen stats fresh while window is visible
    Timer {
        interval: 1000
        running: hdWindow.visible
        repeat: true
        onTriggered: {
            if (typeof hdPlayer.updateMediaInfo === 'function') {
                hdPlayer.updateMediaInfo()
            }
        }
    }

    // PTZ Overlay
    Item {
        id: ptzOverlay
        anchors.fill: parent
        property bool ptzVisible: false
        visible: root.canPtz && ptzVisible
        z: 10

        PtzControlPanel {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 8
            
            cameraIp: root.cameraIp
            cameraPort: root.cameraOnvifPort
            cameraLogin: root.cameraLogin
            iconFontFamily: root.iconFontFamily
            compact: true
        }
    }

    // Top Right Controls (Visible on Hover, mirrors previous app layout)
    Rectangle {
        id: controlsPanel
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        height: 40
        width: controlsRow.implicitWidth + 12
        color: "#cc000000" // Semi-transparent black
        radius: Theme.radiusMd
        // Keep visible while hovering the panel itself to avoid flicker
        visible: root.effectiveCanLive && (hoverArea.containsMouse || controlsHover.hovered || volumeGroup.sliderShowing)
        border.color: Theme.overlayBorder
        border.width: 1

        HoverHandler {
            id: controlsHover
        }
        
        Row {
            id: controlsRow
            anchors.right: parent.right
            anchors.rightMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6
            property bool volumeHovered: false
            
            // PTZ Toggle
            Button {
                width: 32
                height: 26
                background: Rectangle { color: ptzOverlay.visible ? "#44ffffff" : "transparent"; radius: 3 }
                contentItem: Text {
                    text: "control_camera"
                    font.family: root.iconFontFamily
                    font.pixelSize: 18
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (!root.canPtz) { root.permissionDenied(); return }
                    ptzOverlay.ptzVisible = !ptzOverlay.ptzVisible
                }
            }

            // Quality toggle SD/HD for preview
            Button {
                width: 32
                height: 26
                background: Rectangle { color: "transparent"; radius: 3 }
                contentItem: Text {
                            text: root.useHdPreview ? "hd" : "sd"
                            font.family: root.iconFontFamily
                            font.pixelSize: 18
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    root.useHdPreview = !root.useHdPreview
                    player.url = root.useHdPreview && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
                }
            }

            // Audio toggle + hover-preserved slider area
            Item {
                id: volumeGroup
                
                // Use HoverHandler to capture hover state even if buttons are stealing mouse events
                HoverHandler {
                    id: volHover
                }
                
                // Show controls if mouse is over the group or slider is being manipulated
                property bool sliderShowing: volHover.hovered || volumeSlider.pressed || volumeSlider.hovered || normalizeBtn.hovered || audioButton.hovered
                
                // Base width is audio button (26). 
                // Expanded adds: Spacing(6) + N_Button(26) + Spacing(6) + Slider(110) = 148
                // We expand by growing width. Content aligns to the LEFT (AudioButton)
                width: 26 + (sliderShowing ? 148 : 0)
                height: 26

                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: volumeGroup.sliderShowing ? 6 : 0

                    Slider {
                        id: volumeSlider
                        visible: volumeGroup.sliderShowing
                        width: 110
                        height: 20
                        from: 0.0
                        to: 2.0 // Allow up to 200% amplification
                        stepSize: 0.05
                        value: root.volume
                        onValueChanged: root.volume = value
                        
                        // Custom slider style
                        background: Rectangle {
                            x: parent.leftPadding
                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                            implicitWidth: 200
                            implicitHeight: 4
                            width: parent.availableWidth
                            height: implicitHeight
                            radius: 2
                            color: "#444"
                            
                            Rectangle {
                                width: parent.visualPosition * parent.width
                                height: parent.height
                                color: parent.visualPosition > 0.5 ? "#ff9800" : "#2196f3" 
                                radius: 2
                            }
                            
                            // 100% Marker
                            Rectangle {
                                x: parent.width * 0.5
                                width: 2
                                height: 8
                                anchors.verticalCenter: parent.verticalCenter
                                color: "#888"
                            }
                        }
                        
                        handle: Rectangle {
                            x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: 8
                            color: parent.pressed ? "#f0f0f0" : "#f6f6f6"
                            border.color: "#bdbebf"
                        }
                    }

                    // Normalization Toggle (N)
                    Button {
                        id: normalizeBtn
                        visible: volumeGroup.sliderShowing
                        width: 26
                        height: 26
                        background: Rectangle { 
                            color: root.audioNormalization ? "#2563eb" : "transparent" 
                            radius: 3 
                            border.color: "#999"
                            border.width: 1
                        }
                        contentItem: Text {
                            text: "N"
                            font.bold: true
                            font.pixelSize: 12
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        // Simple tooltip using standard QtQuick Controls ToolTip
                        ToolTip.visible: normalizeBtn.hovered
                        ToolTip.text: I18n.t("Нормализация")
                        
                        onClicked: {
                            root.audioNormalization = !root.audioNormalization
                        }
                    }

                    Button {
                        id: audioButton
                        width: 26
                        height: 26
                        background: Rectangle { color: "transparent"; radius: 3 }
                        contentItem: Text {
                            text: root.isMuted ? "volume_off" : "volume_up"
                            font.family: root.iconFontFamily
                            font.pixelSize: 18
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            root.isMuted = !root.isMuted
                            root.audioClicked()
                        }
                    }
                }
            }

            // Record
            Button {
                id: recordBtn
                width: 26
                height: 26
                background: Rectangle { color: "transparent"; radius: 3 }
                contentItem: Text {
                    id: recordIcon
                    text: "fiber_manual_record"
                    font.family: root.iconFontFamily
                    font.pixelSize: 18
                    color: recorder.recordingPath !== "" ? "#f44336" : "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    
                    SequentialAnimation {
                        running: recorder.recordingPath !== ""
                        loops: Animation.Infinite
                        NumberAnimation { target: recordIcon; property: "opacity"; from: 1.0; to: 0.3; duration: 800; easing.type: Easing.InOutQuad }
                        NumberAnimation { target: recordIcon; property: "opacity"; from: 0.3; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                    }
                }
                onClicked: {
                    if (recorder.recordingPath !== "") {
                        // Stop recording
                        recorder.recordingPath = ""
                        segmentTimer.stop()
                        console.log("Recording stopped")
                    } else {
                        // Start recording (HD, background, no display switch)
                        var path = SystemController.generateRecordingPath(root.cameraIp)
                        recorder.recordingPath = path
                        
                        // Restart segment timer
                        if (root.recordingSegmentDuration > 0) {
                            segmentTimer.restart()
                        }
                        
                        console.log("Recording started to", path)
                    }
                    root.recordClicked()
                }
            }

            // Snapshot
            Button {
                width: 26
                height: 26
                background: Rectangle { color: "transparent"; radius: 3 }
                contentItem: Text {
                            text: "photo_camera"
                            font.family: root.iconFontFamily
                            font.pixelSize: 18
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (root.manufacturer === "Dahua") {
                        SystemController.takeDahuaSnapshot(root.cameraIp, root.cameraPort, root.cameraLogin, SystemController.getCameraPassword(root.cameraIp))
                    } else {
                        var fileName = "snapshot_" + root.cameraIp.replace(/\./g, "_") + "_" + Qt.formatDateTime(new Date(), "yyyyMMdd_HHmmss_zzz") + ".png"
                        var fullPath = SystemController.getSnapshotPath(fileName)
                        
                        // Try native player snapshot first (full resolution)
                        // Note: player.saveSnapshot is synchronous and uses the current frame buffer
                        if (player.saveSnapshot(fullPath)) {
                            SystemController.notifySnapshotSaved(fullPath)
                        } else {
                            // Fallback to grabToImage (screen resolution)
                            videoHolder.grabToImage(function(result) {
                                if (result.saveToFile(fullPath)) {
                                    console.log("Snapshot saved to " + fullPath)
                                    SystemController.notifySnapshotSaved(fullPath)
                                } else {
                                    console.warn("Failed to save QML snapshot")
                                }
                            })
                        }
                    }
                    root.snapshotClicked()
                }
            }

            // Close / Remove (hide in fullscreen)
            Button {
                width: 26
                height: 26
                visible: !hdWindow.visible
                background: Rectangle { color: "transparent"; radius: 3 }
                contentItem: Text {
                            text: "close"
                            font.family: root.iconFontFamily
                            font.pixelSize: 18
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.closeClicked()
            }

            // Separator
            Rectangle {
                width: 1
                height: 20
                color: "#55ffffff"
                anchors.verticalCenter: parent.verticalCenter
                visible: analyticsRow.visible
            }

            // Analytics Controls (Merged into main row)
            Row {
                id: analyticsRow
                spacing: 6
                visible: SystemController.analyticsEngine.isModuleEnabled(0) || SystemController.analyticsEngine.isModuleEnabled(1) || SystemController.analyticsEngine.isModuleEnabled(2)
                
                // Face Detector
                Button {
                    width: 26
                    height: 26
                    visible: SystemController.analyticsEngine.isModuleEnabled(0) // FaceDetector
                    background: Rectangle { 
                        color: SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 0) ? "#4299e1" : "transparent"
                        radius: 3 
                        border.color: "#55ffffff"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: "face"
                        font.family: root.iconFontFamily
                        font.pixelSize: 18
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        var current = SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 0)
                        SystemController.analyticsEngine.setCameraModuleEnabled(root.cameraIp, 0, !current)
                        // Force update binding
                        background.color = !current ? "#4299e1" : "transparent"
                        refreshAnalyticsActive()
                    }
                }

                // Object Counter
                Button {
                    width: 26
                    height: 26
                    visible: SystemController.analyticsEngine.isModuleEnabled(1) // ObjectCounter
                    background: Rectangle { 
                        color: SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 1) ? "#4299e1" : "transparent"
                        radius: 3 
                        border.color: "#55ffffff"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: "category"
                        font.family: root.iconFontFamily
                        font.pixelSize: 18
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        var current = SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 1)
                        SystemController.analyticsEngine.setCameraModuleEnabled(root.cameraIp, 1, !current)
                        background.color = !current ? "#4299e1" : "transparent"
                        refreshAnalyticsActive()
                    }
                }

                // License Plate
                Button {
                    width: 26
                    height: 26
                    visible: SystemController.analyticsEngine.isModuleEnabled(2) // LicensePlate
                    background: Rectangle { 
                        color: SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 2) ? "#4299e1" : "transparent"
                        radius: 3 
                        border.color: "#55ffffff"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: "directions_car"
                        font.family: root.iconFontFamily
                        font.pixelSize: 18
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        var current = SystemController.analyticsEngine.isCameraModuleEnabled(root.cameraIp, 2)
                        SystemController.analyticsEngine.setCameraModuleEnabled(root.cameraIp, 2, !current)
                        background.color = !current ? "#4299e1" : "transparent"
                        refreshAnalyticsActive()
                    }
                }
            }
        }
    }

    // Bottom Info Overlay (camera name / IP) pinned to bottom-right to avoid stats
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 8
        scale: root.uiScale
        transformOrigin: Item.BottomRight
        color: "#e0000000"
        radius: Theme.radiusMd
        visible: root.cameraName !== "" || root.cameraIp !== ""
        height: 24
        // Limit width to 45% of cell width (accounting for scale) to prevent overlap
        width: Math.min(Math.max(100, infoRow.implicitWidth + 16), (root.width * 0.45) / root.uiScale)
        border.color: Theme.overlayBorder
        border.width: 1
        clip: true

        Row {
            id: infoRow
            anchors.centerIn: parent
            spacing: 6
            width: parent.width - 16
            
            Text {
                text: (root.cameraName && root.cameraName.trim() !== "")
                      ? root.cameraName
                      : (root.cameraIp && root.cameraIp !== "" ? root.cameraIp : I18n.t("Камера"))
                color: "white"
                font.pixelSize: 11
                font.bold: true
                elide: Text.ElideRight
                // Allow name to take up available space, leaving room for IP if possible
                width: Math.min(implicitWidth, parent.width - (ipText.visible ? ipText.width + 6 : 0))
            }
            Text {
                id: ipText
                text: root.cameraIp
                color: "#dddddd"
                font.pixelSize: 11
                visible: parent.width > 150 // Hide IP if very tight
            }
        }
    }

/*
    // Resize Handle (Bottom-Right) - REMOVED
    /*
    Rectangle {
        width: 30
        height: 30
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "transparent"
        visible: !root.isRecording
        z: 999 // Max Z
        
        // Corner background for better visibility
        Image {
            anchors.fill: parent
            source: "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='white' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M21 15v6h-6'/><path d='M21 21l-9-9'/></svg>"
            sourceSize.width: 30
            sourceSize.height: 30
            opacity: 0.8
        }
        
        MouseArea {
            id: resizeMouseArea
            anchors.fill: parent
            cursorShape: Qt.SizeFDiagCursor
            preventStealing: true // Prevent parent/siblings from stealing the mouse
            property point startPos
            property int startSpanRows
            property int startSpanCols
            property point dragDiff: Qt.point(0,0)
            
            onPressed: (mouse) => {
                console.log("Resize Handle Pressed! GridIndex:", gridIndex, "GridParent:", gridParent)
                console.log("Grid Info: Total:", totalRows, "x", totalCols, "Current Span:", spanRows, "x", spanCols, "Cell Size:", root.width, "x", root.height)
                mouse.accepted = true
                if (gridParent) {
                    startPos = mapToItem(gridParent, mouse.x, mouse.y)
                    startSpanRows = root.spanRows
                    startSpanCols = root.spanCols
                    dragDiff = Qt.point(0,0)
                } else {
                    console.warn("Resize Handle: gridParent is null!")
                }
            }
            
            onPositionChanged: (mouse) => {
                if (!gridParent) return
                
                // Calculate delta in grid units
                var currentPos = mapToItem(gridParent, mouse.x, mouse.y)
                var dx = currentPos.x - startPos.x
                var dy = currentPos.y - startPos.y
                dragDiff = Qt.point(dx, dy)
                
                // LIVE UPDATE DISABLED to prevent jerkiness
                // We only update the visual ghost (resizeFeedback) via dragDiff
            }

            onReleased: {
                if (!gridParent) return
                
                // Commit changes on release
                var uW = Math.max(0.1, unitWidth)
                var uH = Math.max(0.1, unitHeight)
                
                var dCols = Math.round(dragDiff.x / uW)
                var dRows = Math.round(dragDiff.y / uH)
                
                var newCols = Math.max(1, Math.min(totalCols, startSpanCols + dCols))
                var newRows = Math.max(1, Math.min(totalRows, startSpanRows + dRows))
                
                if (newCols !== root.spanCols || newRows !== root.spanRows) {
                    console.log("Resize Committed:", gridIndex, "New Span:", newRows, "x", newCols)
                    SystemController.setGridCellSpan(gridIndex, newRows, newCols)
                }
                
                dragDiff = Qt.point(0,0)
            }
        }
    }

    // Feedback Rectangle (Ghost)
    Rectangle {
        id: resizeFeedback
        parent: root
        color: "transparent" 
        visible: resizeMouseArea.pressed
        x: 0
        y: 0
        // Calculate ghost size based on current dimensions + drag delta
        width: Math.max(50, root.width + resizeMouseArea.dragDiff.x)
        height: Math.max(50, root.height + resizeMouseArea.dragDiff.y)
        z: 9999
        
        border.color: "#3b82f6" // Blue highlight
        border.width: 3
        
        // Inner semi-transparent fill
        Rectangle {
            anchors.fill: parent
            color: "#3b82f6"
            opacity: 0.2
            anchors.margins: 3
        }
        
        // Size tooltip
        Rectangle {
            anchors.centerIn: parent
            width: sizeText.implicitWidth + 16
            height: sizeText.implicitHeight + 8
            color: "#11151f"
            radius: 4
            Text {
                id: sizeText
                anchors.centerIn: parent
                color: "white"
                text: {
                    var uW = Math.max(0.1, unitWidth)
                    var uH = Math.max(0.1, unitHeight)
                    var dCols = Math.round(resizeMouseArea.dragDiff.x / uW)
                    var dRows = Math.round(resizeMouseArea.dragDiff.y / uH)
                    var finalCols = Math.max(1, Math.min(totalCols, resizeMouseArea.startSpanCols + dCols))
                    var finalRows = Math.max(1, Math.min(totalRows, resizeMouseArea.startSpanRows + dRows))
                    return finalRows + " x " + finalCols
                }
            }
        }
    }
    */
}
