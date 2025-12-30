import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import OpenIPC

Item {
    id: root
    
    property string cameraName: ""
    property string cameraIp: ""
    property int cameraPort: 554
    property int cameraOnvifPort: 80
    property string cameraLogin: "admin"
    property string cameraPassword: ""
    property string streamUrl: ""
    property string sdStreamUrl: ""
    property string hdStreamUrl: ""
    property string status: ""
    property bool isSelected: false
    property bool isRecording: false
    property bool isMuted: true
    property real volume: 1.0
    property bool useHdPreview: false
    property bool useHdFullscreen: true
    property string manufacturer: ""
    
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
        if (mode === "none") return ["FFmpeg"]
        // auto
        return ["MFT:d3d=11", "D3D11", "CUDA", "FFmpeg"]
    }

    Component.onCompleted: {
        console.info("GridCell completed", index, cameraIp, streamUrl)
    }

    onStreamUrlChanged: {
        if (streamUrl === "") {
            hdPlayer.running = false
            hdWindow.visible = false
            useHdPreview = false
            useHdFullscreen = true
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
    
    // Shared detection model for both preview and fullscreen
    ListModel { id: detectionModel }

    Connections {
        target: SystemController.analyticsEngine
        function onFrameProcessed(cameraId, detections) {
            if (cameraId === root.cameraIp) {
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
    }

    // Background
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        border.color: root.isRecording ? "#ff0000" : (root.isSelected ? "#1976d2" : "#444444")
        border.width: root.isRecording ? 2 : (root.isSelected ? 2 : 1)
        radius: 4

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

    // Video Player (MDK) fill parent, clip to cell (no fixed aspect)
    Item {
        id: videoHolder
        anchors.fill: parent
        clip: true
        visible: root.streamUrl !== ""

        MdkPlayer {
            id: player
            anchors.fill: parent
            url: root.useHdPreview && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
            running: root.streamUrl !== "" && !hdWindow.visible
            fillMode: SystemController.appSettings.playerFillMode === undefined ? 0 : SystemController.appSettings.playerFillMode // 0 fit, -1 crop
            muted: root.isMuted
            volume: root.volume
            hwDecoders: root.getHwDecoders(SystemController.appSettings.hwAccel)

            // Direct C++ connection for analytics; disabled by default via appSettings.analyticsEnabled
            analyticsEngine: SystemController.appSettings.analyticsEnabled ? SystemController.analyticsEngine : null
            cameraId: root.cameraIp
            // Keep camera upright (camera is mounted inverted)
            // rotation: 180
            // transformOrigin: Item.Center
            transform: Scale { origin.x: player.width / 2; origin.y: player.height / 2; yScale: -1 }

            // Default MDK aspect; we rely on clipping by parent Item to avoid overflow
            
            // MdkPlayer doesn't have playbackState property yet in our wrapper, 
            // so we use running for visibility logic
            
            // onFrameReady removed in favor of direct C++ connection
            
            // Auto-reconnect logic
            // MDK MediaStatus: 0=NoMedia, 1=Unloaded, 2=Loading, 3=Loaded, 4=Prepared, 5=Stalled, 6=Buffering, 7=Buffered, 8=End, 9=Seekable, 10=Invalid
            onMediaStatusChanged: (status) => {
                if (status === 10 || status === 8) { // Invalid or End
                    console.warn("Stream ended or invalid, retrying in 5s...", root.cameraIp)
                    reconnectTimer.restart()
                }
            }
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
                            text: model.label + " " + Math.round(model.confidence * 100) + "%"
                            color: "white"
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }
                }
            }
        }

        /*
        Item {
            id: player
            anchors.fill: parent
            property string url: root.useHdPreview && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
            property bool running: root.streamUrl !== ""
            property int fillMode: -1
            property bool muted: root.isMuted
            property real volume: root.volume
            
            Text { anchors.centerIn: parent; text: "MDK Disabled"; color: "red" }
        }
        */
    }

    // Placeholder / Loading / Error
    Item {
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        visible: !player.running // Simplified logic
        
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 10
            
            Text {
                text: root.streamUrl === "" ? I18n.t("No Signal") : I18n.t("Loading...")
                color: "#666666"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignHCenter
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
            root.border.color = "#00ff00"
            drag.accept(Qt.MoveAction)
        }
        onExited: {
            root.border.color = root.isSelected ? "#1976d2" : "#444444"
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

            root.border.color = root.isSelected ? "#1976d2" : "#444444"
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
        }

        Item {
            anchors.fill: parent
            clip: true

            MdkPlayer {
                id: hdPlayer
                anchors.fill: parent
                url: root.useHdFullscreen && root.hdStreamUrl !== "" ? root.hdStreamUrl : (root.sdStreamUrl !== "" ? root.sdStreamUrl : root.streamUrl)
                // Use configured fillMode; default fit to keep aspect
                fillMode: SystemController.appSettings.playerFillMode === undefined ? 0 : SystemController.appSettings.playerFillMode
                muted: root.isMuted
                volume: root.volume
                hwDecoders: root.getHwDecoders(SystemController.appSettings.hwAccel)
                // Mirror rotation from preview so fullscreen is not upside down
                // rotation: player.rotation
                // transformOrigin: Item.Center
                transform: Scale { origin.x: hdPlayer.width / 2; origin.y: hdPlayer.height / 2; yScale: -1 }
                // Default MDK aspect; clipping via parent Item
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
                                text: model.label + " " + Math.round(model.confidence * 100) + "%"
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
                Text { anchors.centerIn: parent; text: "MDK Disabled (HD)"; color: "red" }
            }
            */

            // PTZ Overlay for Fullscreen
            Item {
                id: hdPtzOverlay
                anchors.fill: parent
                visible: false
                z: 10

                PtzControlPanel {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.bottomMargin: 50
                    
                    cameraIp: root.cameraIp
                    cameraPort: root.cameraOnvifPort
                    cameraLogin: root.cameraLogin
                    cameraPassword: root.cameraPassword
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
                        onClicked: hdPtzOverlay.visible = !hdPtzOverlay.visible
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
                    text: "Preview: " + (root.useHdPreview && root.hdStreamUrl !== "" ? "HD" : "SD") + "  |  Fullscreen: " + (root.useHdFullscreen && root.hdStreamUrl !== "" ? "HD" : "SD")
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        Keys.onEscapePressed: hdWindow.close()
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
        radius: 6
        visible: (SystemController.appSettings.showStatsOverlay === undefined || SystemController.appSettings.showStatsOverlay) && statsText() !== ""
        height: 30
        // Limit width to 45% of cell width (accounting for scale) to prevent overlap
        width: Math.min(Math.max(110, statsLabel.implicitWidth + 18), (root.width * 0.45) / root.uiScale)
        border.color: "#44ffffff"
        border.width: 1
        clip: true

        Text {
            id: statsLabel
            anchors.centerIn: parent
            text: statsText()
            color: "white"
            font.pixelSize: 14
            font.family: "monospace"
            font.bold: true
            elide: Text.ElideRight
            width: parent.width - 18
            horizontalAlignment: Text.AlignHCenter
        }
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
        visible: false
        z: 10

        PtzControlPanel {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 8
            
            cameraIp: root.cameraIp
            cameraPort: root.cameraOnvifPort
            cameraLogin: root.cameraLogin
            cameraPassword: root.cameraPassword
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
        radius: 6
        visible: hoverArea.containsMouse || controlsRow.hovered || hdWindow.visible
        border.color: "#55ffffff"
        border.width: 1
        
        Row {
            id: controlsRow
            anchors.centerIn: parent
            spacing: 6
            property bool hovered: false
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
                onClicked: ptzOverlay.visible = !ptzOverlay.visible
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
                property bool sliderShowing: !root.isMuted && volumeHover.hovered
                width: audioButton.width + (sliderShowing ? volumeSlider.width + 6 : 0)
                height: Math.max(audioButton.implicitHeight, volumeSlider.implicitHeight)

                Row {
                    anchors.centerIn: parent
                    spacing: volumeGroup.sliderShowing ? 6 : 0

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

                    // Volume slider (visible when hovering the audio cluster)
                    Slider {
                        id: volumeSlider
                        width: 110
                        height: 20
                        from: 0.0
                        to: 1.0
                        stepSize: 0.05
                        visible: volumeGroup.sliderShowing
                        value: root.volume
                        onValueChanged: root.volume = value
                    }
                }

                // Hover handler keeps slider visible while moving from the icon
                HoverHandler {
                    id: volumeHover
                }
            }

            // Record
            Button {
                width: 26
                height: 26
                background: Rectangle { color: "transparent"; radius: 3 }
                contentItem: Text {
                            text: "fiber_manual_record"
                            font.family: root.iconFontFamily
                            font.pixelSize: 18
                            color: root.isRecording ? "#f44336" : "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    root.isRecording = !root.isRecording
                    console.log(root.isRecording ? "Recording started" : "Recording stopped")
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
                        SystemController.takeDahuaSnapshot(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                    } else {
                        // Fallback to grabbing from video item
                        // Use videoHolder instead of player to capture the applied transform (flip)
                        videoHolder.grabToImage(function(result) {
                            var fileName = "snapshot_" + root.cameraIp.replace(/\./g, "_") + "_" + Qt.formatDateTime(new Date(), "yyyyMMdd_HHmmss_zzz") + ".png"
                            var fullPath = SystemController.getSnapshotPath(fileName)
                            
                            if (result.saveToFile(fullPath)) {
                                console.log("Snapshot saved to " + fullPath)
                                SystemController.notifySnapshotSaved(fullPath)
                            } else {
                                console.warn("Failed to save QML snapshot")
                            }
                        })
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
                    }
                }
            }
        }
        


        // Hover tracker so the panel stays visible while interacting
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            onEntered: {
                controlsRow.hovered = true
            }
            onExited: {
                controlsRow.hovered = false
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
        radius: 6
        visible: root.cameraName !== "" || root.cameraIp !== ""
        height: 24
        // Limit width to 45% of cell width (accounting for scale) to prevent overlap
        width: Math.min(Math.max(100, infoRow.implicitWidth + 16), (root.width * 0.45) / root.uiScale)
        border.color: "#44ffffff"
        border.width: 1
        clip: true

        Row {
            id: infoRow
            anchors.centerIn: parent
            spacing: 6
            width: parent.width - 16
            
            Text {
                text: root.cameraName !== "" ? root.cameraName : "Camera"
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

    // Build human-readable stats string from player properties
    function statsText() {
        if (root.streamUrl === "")
            return ""
        var parts = []
        if (player.videoCodec && player.videoCodec.length) {
            parts.push(player.videoCodec.toUpperCase())
        }
        if (player.videoResolution && player.videoResolution.length) {
            parts.push(player.videoResolution)
        }
        if (player.videoBitrate && player.videoBitrate > 0) {
            parts.push(player.videoBitrate + " kbps")
        }
        if (player.videoFps && player.videoFps > 0) {
            parts.push(player.videoFps.toFixed(1) + " fps")
        }
        return parts.join("  ")
    }

    // Stats for fullscreen player
    function hdStatsText() {
        if (root.streamUrl === "")
            return ""
        var parts = []
        if (hdPlayer.videoCodec && hdPlayer.videoCodec.length) {
            parts.push(hdPlayer.videoCodec.toUpperCase())
        }
        if (hdPlayer.videoResolution && hdPlayer.videoResolution.length) {
            parts.push(hdPlayer.videoResolution)
        }
        if (hdPlayer.videoBitrate && hdPlayer.videoBitrate > 0) {
            parts.push(hdPlayer.videoBitrate + " kbps")
        }
        if (hdPlayer.videoFps && hdPlayer.videoFps > 0) {
            parts.push(hdPlayer.videoFps.toFixed(1) + " fps")
        }
        return parts.join("  ")
    }

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
