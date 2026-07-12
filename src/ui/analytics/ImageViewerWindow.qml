import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Window {
    id: root
    width: 1200
    height: 800
    visible: false
    color: Theme.metroBackground
    flags: Qt.Window | Qt.FramelessWindowHint

    property var itemsModel: null
    property int currentIndex: -1
    property bool slideshowRunning: false
    property int slideIntervalMs: 2500
    property bool editMode: false
    property bool drawMode: false
    property bool cropMode: false
    property color penColor: Theme.metroRed
    property int penWidth: 3
    property real zoom: 1.0
    property real fitScale: 1.0
    property real rotation: 0
    property bool flipH: false
    property bool flipV: false
    property bool showProps: false
    property var fileInfo: ({})
    property rect cropRect: Qt.rect(0, 0, 0, 0)
    property rect cropSourceRect: Qt.rect(0, 0, 0, 0)
    property bool cropApplied: false
    property rect appliedCropRect: Qt.rect(0, 0, 0, 0)
    property var strokes: ([])
    property bool textMode: false
    property var textAnnotations: ([])
    property int textSize: 18
    property int selectedTextIndex: -1
    property url pendingSaveUrl: ""

    FontLoader { id: materialIcons; source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf" }

    component IconButton: ToolButton {
        property string tip: ""
        implicitWidth: 30
        implicitHeight: 30
        font.family: materialIcons.name
        font.pixelSize: 20
        contentItem: Text {
            text: parent.text
            color: Theme.textSecondary
            font.family: materialIcons.name
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: parent.hovered ? Theme.metroTileHover : "transparent"
            radius: 4
            border.color: parent.down ? Theme.metroBlue : "transparent"
        }
        ToolTip.visible: hovered
        ToolTip.delay: 300
        ToolTip.text: tip
    }

    onClosing: {
        slideshowRunning = false
    }

    function openWithModel(model, index) {
        itemsModel = model
        currentIndex = index
        updateForCurrent()
        show()
        requestActivate()
    }

    function currentItem() {
        if (!itemsModel || currentIndex < 0 || currentIndex >= itemsModel.count) return null
        return itemsModel.get(currentIndex)
    }

    function updateForCurrent() {
        var item = currentItem()
        if (!item) return
        fileInfo = SystemController.getFileInfo(item.fileUrl)
        resetView()
    }

    function resetView() {
        zoom = 1.0
        rotation = 0
        flipH = false
        flipV = false
        cropMode = false
        cropRect = Qt.rect(0, 0, 0, 0)
        cropSourceRect = Qt.rect(0, 0, 0, 0)
        cropApplied = false
        appliedCropRect = Qt.rect(0, 0, 0, 0)
        strokes = []
        textAnnotations = []
        selectedTextIndex = -1
        drawCanvas.requestPaint()
        centerContent()
    }

    function updateCropSourceRect() {
        if (!hasCrop()) {
            cropSourceRect = Qt.rect(0, 0, 0, 0)
            return
        }
        var baseW = getBaseWidth()
        var baseH = getBaseHeight()
        if (baseW <= 0 || baseH <= 0 || imageItem.width <= 0 || imageItem.height <= 0) return
        var sx = cropRect.x / imageItem.width * baseW
        var sy = cropRect.y / imageItem.height * baseH
        var sw = cropRect.width / imageItem.width * baseW
        var sh = cropRect.height / imageItem.height * baseH
        cropSourceRect = Qt.rect(sx, sy, sw, sh)
    }

    function applyCrop() {
        updateCropSourceRect()
        if (cropSourceRect.width <= 0 || cropSourceRect.height <= 0) return
        appliedCropRect = cropSourceRect
        cropApplied = true
        cropMode = false
        cropRect = Qt.rect(0, 0, 0, 0)
        updateFitScale()
        centerContent()
    }

    function undoStroke() {
        if (strokes.length === 0) return
        strokes.pop()
        drawCanvas.requestPaint()
    }

    function clearCrop() {
        cropRect = Qt.rect(0, 0, 0, 0)
        cropSourceRect = Qt.rect(0, 0, 0, 0)
        cropApplied = false
        appliedCropRect = Qt.rect(0, 0, 0, 0)
    }

    function addTextBoxAt(x, y) {
        var w = 180
        var h = 44
        var list = textAnnotations.slice(0)
        list.push({ text: "", x: x, y: y, w: w, h: h, color: penColor, size: textSize })
        textAnnotations = list
        selectedTextIndex = textAnnotations.length - 1
    }

    function next() {
        if (!itemsModel || itemsModel.count === 0) return
        currentIndex = (currentIndex + 1) % itemsModel.count
        updateForCurrent()
    }

    function previous() {
        if (!itemsModel || itemsModel.count === 0) return
        currentIndex = (currentIndex - 1 + itemsModel.count) % itemsModel.count
        updateForCurrent()
    }

    function toggleFullscreen() {
        if (root.visibility === Window.FullScreen) root.showNormal()
        else root.showFullScreen()
    }

    function formatBytes(bytes) {
        if (!bytes || bytes <= 0) return "0 B"
        var units = ["B", "KB", "MB", "GB", "TB"]
        var i = Math.floor(Math.log(bytes) / Math.log(1024))
        var val = bytes / Math.pow(1024, i)
        return val.toFixed(val >= 10 || i === 0 ? 0 : 1) + " " + units[i]
    }

    function centerContent() {
        var cx = Math.max(0, (viewerFlick.contentWidth - viewerFlick.width) / 2)
        var cy = Math.max(0, (viewerFlick.contentHeight - viewerFlick.height) / 2)
        viewerFlick.contentX = cx
        viewerFlick.contentY = cy
    }

    function updateFitScale() {
        var baseW = getBaseWidth()
        var baseH = getBaseHeight()
        if (baseW > 0 && baseH > 0) {
            var scaleW = viewerFlick.width / baseW
            var scaleH = viewerFlick.height / baseH
            fitScale = Math.min(scaleW, scaleH)
            if (fitScale <= 0 || isNaN(fitScale)) fitScale = 1.0
        }
    }

    function getBaseWidth() {
        if (cropApplied && appliedCropRect.width > 0) return appliedCropRect.width
        return previewImage.sourceSize.width
    }

    function getBaseHeight() {
        if (cropApplied && appliedCropRect.height > 0) return appliedCropRect.height
        return previewImage.sourceSize.height
    }

    function makeEditedPath(filePath) {
        if (!filePath) return ""
        var dot = filePath.lastIndexOf(".")
        if (dot < 0) return filePath + "_edited"
        return filePath.substring(0, dot) + "_edited" + filePath.substring(dot)
    }

    function hasCrop() {
        return cropRect.width > 10 && cropRect.height > 10
    }

    function saveEditedCopy() {
        var item = currentItem()
        if (!item || !item.filePath) return
        var suggested = makeEditedPath(item.filePath)
        saveDialog.currentFile = "file:///" + suggested.replace(/\\/g, "/")
        saveDialog.open()
    }

    function saveToUrl(url) {
        if (!url || url === "") return
        pendingSaveUrl = url
        exportCanvas.requestPaint()
    }

    function updateSelectedTextSize(size) {
        textSize = size
        if (selectedTextIndex < 0) return
        if (textAnnotations[selectedTextIndex]) {
            textAnnotations[selectedTextIndex].size = size
        }
        if (textRepeater && textRepeater.itemAt) {
            var item = textRepeater.itemAt(selectedTextIndex)
            if (item) {
                item.boxFontSize = size
                if (item.boxField) item.boxField.font.pixelSize = size
            }
        }
    }

    function wrapText(ctx, text, maxWidth) {
        if (!text || maxWidth <= 0) return [""]
        var lines = []
        var paragraphs = text.split("\n")
        for (var pi = 0; pi < paragraphs.length; pi++) {
            var words = paragraphs[pi].split(/\s+/)
            var line = ""
            for (var wi = 0; wi < words.length; wi++) {
                var testLine = line === "" ? words[wi] : line + " " + words[wi]
                if (ctx.measureText(testLine).width > maxWidth && line !== "") {
                    lines.push(line)
                    line = words[wi]
                } else {
                    line = testLine
                }
            }
            lines.push(line)
        }
        return lines
    }

    FileDialog {
        id: saveDialog
        title: I18n.t("Сохранить как")
        nameFilters: ["PNG (*.png)", "JPG (*.jpg *.jpeg)", "Все файлы (*)"]
        fileMode: FileDialog.SaveFile
        onAccepted: saveToUrl(selectedFile)
    }

    ColorDialog {
        id: colorDialog
        title: I18n.t("Выбор цвета")
        onAccepted: penColor = selectedColor
    }

    Timer {
        id: slideshowTimer
        interval: root.slideIntervalMs
        running: root.slideshowRunning
        repeat: true
        onTriggered: root.next()
    }

    Rectangle {
        id: titleBar
        height: 40
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: Theme.metroSurface
        z: 10

        MouseArea {
            anchors.fill: parent
            onPressed: root.startSystemMove()
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 6
            spacing: 8

            Text {
                text: currentItem() ? currentItem().fileName : ""
                color: "white"
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            MetroWindowButton {
                kind: "minimize"
                Layout.preferredWidth: 36
                Layout.fillHeight: true
                onClicked: root.showMinimized()
            }

            MetroWindowButton {
                kind: "maximize"
                maximized: root.visibility === Window.FullScreen || root.visibility === Window.Maximized
                Layout.preferredWidth: 36
                Layout.fillHeight: true
                onClicked: toggleFullscreen()
            }

            MetroWindowButton {
                kind: "close"
                Layout.preferredWidth: 36
                Layout.fillHeight: true
                onClicked: root.close()
            }
        }
    }

    ColumnLayout {
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: "#1f1f1f"
            border.color: Theme.metroTile

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                IconButton { text: "\ue5cb"; tip: I18n.t("Назад"); onClicked: previous() }
                IconButton { text: "\ue5cc"; tip: I18n.t("Вперед"); onClicked: next() }
                Rectangle { width: 1; height: 28; color: Theme.metroTile }
                IconButton { text: "\ue900"; tip: "-"; onClicked: { zoom = Math.max(0.2, zoom - 0.1); centerContent() } }
                IconButton { text: "\ue8ff"; tip: "+"; onClicked: { zoom = Math.min(5, zoom + 0.1); centerContent() } }
                IconButton { text: "\uea10"; tip: I18n.t("Подогнать"); onClicked: { zoom = 1.0; centerContent() } }
                ToolButton {
                    text: "100%"
                    implicitWidth: 48
                    implicitHeight: 30
                    contentItem: Text {
                        text: parent.text
                        color: Theme.textSecondary
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle { color: parent.hovered ? Theme.metroTileHover : "transparent"; radius: 4 }
                    ToolTip.visible: hovered
                    ToolTip.delay: 300
                    ToolTip.text: "100%"
                    onClicked: { zoom = fitScale > 0 ? 1.0 / fitScale : 1.0; centerContent() }
                }
                Rectangle { width: 1; height: 28; color: Theme.metroTile }
                IconButton { text: "\ue419"; tip: I18n.t("Повернуть влево"); onClicked: rotation = (rotation - 90) % 360 }
                IconButton { text: "\ue41a"; tip: I18n.t("Повернуть вправо"); onClicked: rotation = (rotation + 90) % 360 }
                IconButton { text: "\ue8d4"; tip: I18n.t("Отразить по горизонтали"); onClicked: flipH = !flipH }
                IconButton { text: "\ue8d5"; tip: I18n.t("Отразить по вертикали"); onClicked: flipV = !flipV }
                Rectangle { width: 1; height: 28; color: Theme.metroTile }
                IconButton { text: "\ue41b"; tip: I18n.t("Слайд-шоу"); checkable: true; checked: slideshowRunning; onClicked: slideshowRunning = checked }
                IconButton { text: "\ue5d0"; tip: I18n.t("Полный экран"); onClicked: toggleFullscreen() }
                Rectangle { width: 1; height: 28; color: Theme.metroTile }
                IconButton {
                    text: "\ue02c"
                    tip: I18n.t("Открыть клип")
                    enabled: {
                        var item = currentItem()
                        return item && item.clipUrl && item.clipUrl !== ""
                    }
                    onClicked: {
                        var item = currentItem()
                        if (item && item.clipUrl) Qt.openUrlExternally(item.clipUrl)
                    }
                }
                IconButton {
                    text: "\ue872"
                    tip: I18n.t("Удалить")
                    onClicked: {
                        var item = currentItem()
                        if (item && item.fileUrl && SystemController.deleteLocalFile(item.fileUrl)) {
                            itemsModel.remove(currentIndex)
                            if (itemsModel.count === 0) { root.close(); return }
                            currentIndex = Math.min(currentIndex, itemsModel.count - 1)
                            updateForCurrent()
                        }
                    }
                }
                IconButton { text: "\ue14d"; tip: I18n.t("Копировать"); onClicked: { var item = currentItem(); if (item) SystemController.copyImageToClipboard(item.fileUrl) } }
                IconButton { text: "\ue89e"; tip: I18n.t("Открыть в..."); onClicked: { var item = currentItem(); if (item) SystemController.openWithDialog(item.fileUrl) } }
                IconButton { text: "\ue8ad"; tip: I18n.t("Печать"); onClicked: { var item = currentItem(); if (item) SystemController.printImage(item.fileUrl) } }
                IconButton { text: "\ue88e"; tip: I18n.t("Свойства"); checkable: true; checked: showProps; onClicked: showProps = checked }
                IconButton { text: "\ue3c9"; tip: I18n.t("Редактировать"); checkable: true; checked: editMode; onClicked: editMode = checked }
                Item { Layout.fillWidth: true }
                Text { text: (currentIndex + 1) + " / " + (itemsModel ? itemsModel.count : 0); color: Theme.textMuted }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: editMode ? 40 : 0
            visible: editMode
            color: "#1b1b1b"
            border.color: Theme.metroTile

            RowLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 6

                IconButton { text: "\ue3c9"; tip: I18n.t("Рисовать"); checkable: true; checked: drawMode; onClicked: { drawMode = checked; if (checked) cropMode = false } }
                IconButton { text: "\ue3c6"; tip: I18n.t("Обрезка"); checkable: true; checked: cropMode; onClicked: { cropMode = checked; if (checked) { drawMode = false; textMode = false } } }
                IconButton { text: "\ue262"; tip: I18n.t("Текст"); checkable: true; checked: textMode; onClicked: { textMode = checked; if (checked) { drawMode = false; cropMode = false } } }
                IconButton { text: "\ue166"; tip: I18n.t("Отменить"); onClicked: undoStroke() }
                IconButton { text: "\ue5cd"; tip: I18n.t("Отмена обрезки"); onClicked: clearCrop() }
                IconButton { text: "\ue5ca"; tip: I18n.t("Применить обрезку"); enabled: cropMode && hasCrop(); onClicked: applyCrop() }
                Rectangle { width: 1; height: 24; color: Theme.metroTile }
                RowLayout {
                    spacing: 4
                    visible: drawMode || textMode
                    Rectangle {
                        width: 16
                        height: 16
                        radius: 3
                        color: penColor
                        border.color: Theme.metroTile
                    }
                    IconButton { text: "\ue3c7"; tip: I18n.t("Цвет"); onClicked: colorDialog.open() }
                }
                MetroSlider {
                    visible: drawMode
                    from: 1
                    to: 10
                    value: penWidth
                    stepSize: 1
                    implicitWidth: 90
                    onValueChanged: penWidth = Math.round(value)
                }
                MetroSlider {
                    visible: textMode
                    from: 8
                    to: 64
                    value: textSize
                    stepSize: 1
                    implicitWidth: 120
                    onValueChanged: {
                        updateSelectedTextSize(Math.round(value))
                        drawCanvas.requestPaint()
                    }
                }
                Text {
                    visible: textMode
                    text: textSize + "px"
                    color: Theme.textMuted
                    font.pixelSize: 12
                }
                IconButton { text: "\ue2c6"; tip: I18n.t("Сохранить как"); onClicked: saveEditedCopy() }
                IconButton { text: "\ue14d"; tip: I18n.t("Копировать"); onClicked: { var item = currentItem(); if (item) SystemController.copyImageToClipboard(item.fileUrl) } }
                Item { Layout.fillWidth: true }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#111"

                Flickable {
                    id: viewerFlick
                    anchors.fill: parent
                    clip: true
                    contentWidth: Math.max(imageItem.width, viewerFlick.width)
                    contentHeight: Math.max(imageItem.height, viewerFlick.height)
                    onWidthChanged: { updateFitScale(); centerContent() }
                    onHeightChanged: { updateFitScale(); centerContent() }

                    Item {
                        id: imageItem
                        width: Math.max(1, getBaseWidth() * fitScale * zoom)
                        height: Math.max(1, getBaseHeight() * fitScale * zoom)
                        x: Math.max(0, (viewerFlick.contentWidth - width) / 2)
                        y: Math.max(0, (viewerFlick.contentHeight - height) / 2)

                        Image {
                            id: previewImage
                            anchors.fill: parent
                            source: currentItem() ? currentItem().fileUrl : ""
                            fillMode: Image.Stretch
                            asynchronous: true
                            mipmap: true
                            sourceClipRect: cropApplied && appliedCropRect.width > 0 ? appliedCropRect : Qt.rect(0, 0, sourceSize.width, sourceSize.height)
                            onStatusChanged: {
                                if (status === Image.Ready && sourceSize.width > 0 && sourceSize.height > 0) {
                                    updateFitScale()
                                    centerContent()
                                }
                            }
                            transform: [
                                Rotation { origin.x: width / 2; origin.y: height / 2; angle: root.rotation },
                                Scale { origin.x: width / 2; origin.y: height / 2; xScale: root.flipH ? -1 : 1; yScale: root.flipV ? -1 : 1 }
                            ]
                        }

                        Item {
                            id: editSurface
                            anchors.fill: parent
                            clip: true

                            Canvas {
                                id: drawCanvas
                                anchors.fill: parent
                                contextType: "2d"
                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)
                                    for (var i = 0; i < strokes.length; i++) {
                                        var s = strokes[i]
                                        ctx.lineWidth = s.width
                                        ctx.strokeStyle = s.color
                                        ctx.lineCap = "round"
                                        ctx.lineJoin = "round"
                                        ctx.beginPath()
                                        for (var p = 0; p < s.points.length; p++) {
                                            var pt = s.points[p]
                                            if (p === 0) ctx.moveTo(pt.x, pt.y)
                                            else ctx.lineTo(pt.x, pt.y)
                                        }
                                        ctx.stroke()
                                    }
                                }
                            }

                            Item {
                                id: textOverlay
                                anchors.fill: parent
                                visible: textAnnotations.length > 0 || textMode
                                enabled: textMode
                                z: 1

                                Repeater {
                                    id: textRepeater
                                    model: textAnnotations
                                    delegate: Item {
                                        id: textBox
                                        property var modelItem: modelData
                                        property string boxText: textField.text
                                        property real boxFontSize: (modelItem && modelItem.size ? modelItem.size : textSize)
                                        property color boxColor: textField.color
                                        property alias boxField: textField
                                        x: modelItem && modelItem.x !== undefined ? modelItem.x : 0
                                        y: modelItem && modelItem.y !== undefined ? modelItem.y : 0
                                        width: Math.max(80, modelItem && modelItem.w ? modelItem.w : 180)
                                        height: Math.max(28, modelItem && modelItem.h ? modelItem.h : 44)
                                        visible: modelItem && (editMode || (modelItem.text && modelItem.text.length > 0))

                                        onXChanged: if (modelItem) modelItem.x = x
                                        onYChanged: if (modelItem) modelItem.y = y
                                        onWidthChanged: if (modelItem) modelItem.w = width
                                        onHeightChanged: if (modelItem) modelItem.h = height

                                        Rectangle {
                                            anchors.fill: parent
                                            color: selectedTextIndex === index ? Theme.metroStroke : "#11111199"
                                            border.color: selectedTextIndex === index ? Theme.metroBlue : Theme.metroTilePressed
                                            radius: 4
                                        }

                                        Rectangle {
                                            id: moveHandle
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            height: 18
                                            color: "transparent"

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: editMode && textMode
                                                cursorShape: Qt.SizeAllCursor
                                                drag.target: textBox
                                                drag.axis: Drag.XAndYAxis
                                                onPressed: {
                                                    selectedTextIndex = index
                                                    if (modelItem && modelItem.size) textSize = modelItem.size
                                                    if (textBox.boxFontSize) textSize = textBox.boxFontSize
                                                    mouse.accepted = true
                                                }
                                            }
                                        }

                                        TextArea {
                                            id: textField
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.top: moveHandle.bottom
                                            anchors.bottom: parent.bottom
                                            text: modelItem && modelItem.text !== undefined ? modelItem.text : ""
                                            color: modelItem && modelItem.color ? modelItem.color : "#ffffff"
                                            font.pixelSize: textBox.boxFontSize
                                            wrapMode: TextEdit.Wrap
                                            padding: 6
                                            background: Rectangle { color: "transparent" }
                                            selectByMouse: true
                                            readOnly: !editMode || !textMode
                                            onTextChanged: {
                                                if (modelItem) modelItem.text = text
                                                drawCanvas.requestPaint()
                                            }
                                            onActiveFocusChanged: if (activeFocus) {
                                                selectedTextIndex = index
                                                if (modelItem && modelItem.size) textSize = modelItem.size
                                                if (!modelItem.size) modelItem.size = textSize
                                                textBox.boxFontSize = modelItem.size
                                            }
                                        }

                                        Rectangle {
                                            id: resizeHandle
                                            width: 12
                                            height: 12
                                            anchors.right: parent.right
                                            anchors.bottom: parent.bottom
                                            color: Theme.metroBlue
                                            radius: 2
                                            visible: editMode
                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: editMode && textMode
                                                cursorShape: Qt.SizeFDiagCursor
                                                property real startW: 0
                                                property real startH: 0
                                                property real startX: 0
                                                property real startY: 0
                                                onPressed: {
                                                    selectedTextIndex = index
                                                    startW = textBox.width
                                                    startH = textBox.height
                                                    startX = mouse.x
                                                    startY = mouse.y
                                                    mouse.accepted = true
                                                }
                                                onPositionChanged: {
                                                    if (!pressed) return
                                                    var newW = Math.max(80, startW + (mouse.x - startX))
                                                    var newH = Math.max(28, startH + (mouse.y - startY))
                                                    textBox.width = newW
                                                    textBox.height = newH
                                                }
                                            }
                                        }

                                        Component.onCompleted: {
                                            if (index === selectedTextIndex) {
                                                textField.forceActiveFocus()
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                id: cropOverlay
                                visible: cropMode && hasCrop()
                                x: cropRect.x
                                y: cropRect.y
                                width: cropRect.width
                                height: cropRect.height
                                color: "transparent"
                                border.color: Theme.metroBlue
                                border.width: 2
                                z: 2
                            }

                            MouseArea {
                                anchors.fill: parent
                                enabled: editMode && (drawMode || cropMode || textMode)
                                z: 0
                                onPressed: function(mouse) {
                                    if (drawMode) {
                                        strokes.push({ color: penColor, width: penWidth, points: [{ x: mouse.x, y: mouse.y }] })
                                        drawCanvas.requestPaint()
                                    } else if (cropMode) {
                                        cropRect = Qt.rect(mouse.x, mouse.y, 0, 0)
                                    } else if (textMode) {
                                        addTextBoxAt(mouse.x, mouse.y)
                                    }
                                }
                                onPositionChanged: function(mouse) {
                                    if (drawMode && strokes.length > 0) {
                                        strokes[strokes.length - 1].points.push({ x: mouse.x, y: mouse.y })
                                        drawCanvas.requestPaint()
                                    } else if (cropMode) {
                                        cropRect.width = mouse.x - cropRect.x
                                        cropRect.height = mouse.y - cropRect.y
                                    }
                                }
                                onReleased: function(mouse) {
                                    if (cropMode && cropRect.width < 0) {
                                        cropRect = Qt.rect(cropRect.x + cropRect.width, cropRect.y, Math.abs(cropRect.width), cropRect.height)
                                    }
                                    if (cropMode && cropRect.height < 0) {
                                        cropRect = Qt.rect(cropRect.x, cropRect.y + cropRect.height, cropRect.width, Math.abs(cropRect.height))
                                    }
                                    updateCropSourceRect()
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: showProps ? 220 : 0
                Layout.fillHeight: true
                visible: showProps
                color: Theme.metroBackground
                border.color: Theme.metroTile

                ScrollView {
                    anchors.fill: parent
                    contentWidth: availableWidth
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ColumnLayout {
                        width: parent.width
                        spacing: 6
                        Layout.margins: 10

                        Text { text: I18n.t("Свойства"); color: "#bbb"; font.pixelSize: 14 }
                        Text { text: fileInfo.fileName || ""; color: "white"; wrapMode: Text.Wrap; font.pixelSize: 12 }
                        Text { text: fileInfo.filePath || ""; color: Theme.textMuted; wrapMode: Text.Wrap; font.pixelSize: 11 }
                        Text { text: I18n.t("Размер") + ": " + (fileInfo.size ? formatBytes(fileInfo.size) : "-"); color: Theme.textMuted; font.pixelSize: 11 }
                        Text { text: I18n.t("Разрешение") + ": " + (fileInfo.width ? fileInfo.width + " x " + fileInfo.height : "-"); color: Theme.textMuted; font.pixelSize: 11 }
                        Text { text: I18n.t("Создан") + ": " + (fileInfo.createdText || "-"); color: Theme.textMuted; font.pixelSize: 11 }
                        Text { text: I18n.t("Изменен") + ": " + (fileInfo.modifiedText || "-"); color: Theme.textMuted; font.pixelSize: 11 }

                        Rectangle { height: 1; Layout.fillWidth: true; color: Theme.metroTile }

                        Text { text: I18n.t("Редактирование"); color: "#bbb"; font.pixelSize: 14; visible: editMode }

                        RowLayout {
                            visible: editMode && drawMode
                            spacing: 8
                            Text { text: I18n.t("Цвет"); color: Theme.textMuted }
                            Rectangle { width: 18; height: 18; color: penColor; radius: 3; border.color: Theme.metroTile }
                            MetroSlider {
                                from: 1; to: 8
                                value: penWidth
                                onValueChanged: penWidth = Math.round(value)
                            }
                        }

                        RowLayout {
                            visible: editMode
                            spacing: 8
                            Button { text: I18n.t("Сбросить"); onClicked: resetView() }
                        }

                        Item { Layout.fillHeight: true }
                    }
                }
            }
        }
    }

    Item {
        id: exportSurface
        visible: true
        opacity: 0.0
        width: cropApplied ? appliedCropRect.width : (cropSourceRect.width > 0 ? cropSourceRect.width : editSurface.width)
        height: cropApplied ? appliedCropRect.height : (cropSourceRect.height > 0 ? cropSourceRect.height : editSurface.height)
        clip: true

        Item {
            anchors.fill: parent
            x: cropApplied ? -appliedCropRect.x : (cropSourceRect.width > 0 ? -cropSourceRect.x : 0)
            y: cropApplied ? -appliedCropRect.y : (cropSourceRect.height > 0 ? -cropSourceRect.y : 0)

            Image {
                anchors.fill: parent
                source: currentItem() ? currentItem().fileUrl : ""
                fillMode: Image.Stretch
                asynchronous: false
                mipmap: false
                transform: [
                    Rotation { origin.x: width / 2; origin.y: height / 2; angle: root.rotation },
                    Scale { origin.x: width / 2; origin.y: height / 2; xScale: root.flipH ? -1 : 1; yScale: root.flipV ? -1 : 1 }
                ]
            }

            Canvas {
                id: exportCanvas
                anchors.fill: parent
                contextType: "2d"
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    var sx = editSurface.width > 0 ? width / editSurface.width : 1
                    var sy = editSurface.height > 0 ? height / editSurface.height : 1
                    for (var i = 0; i < strokes.length; i++) {
                        var s = strokes[i]
                        ctx.lineWidth = s.width * Math.max(sx, sy)
                        ctx.strokeStyle = s.color
                        ctx.lineCap = "round"
                        ctx.lineJoin = "round"
                        ctx.beginPath()
                        for (var p = 0; p < s.points.length; p++) {
                            var pt = s.points[p]
                            if (p === 0) ctx.moveTo(pt.x * sx, pt.y * sy)
                            else ctx.lineTo(pt.x * sx, pt.y * sy)
                        }
                        ctx.stroke()
                    }

                    for (var t = 0; t < textAnnotations.length; t++) {
                        var ta = textAnnotations[t]
                        if (!ta.text || ta.text.trim() === "") continue
                        var fontSize = (ta.size ? ta.size : textSize) * sy
                        ctx.fillStyle = ta.color
                        ctx.font = fontSize + "px sans-serif"
                        var padding = 6 * sy
                        var headerH = 18 * sy
                        var maxW = ((ta.w ? ta.w : 180) * sx) - (padding * 2)
                        var lines = wrapText(ctx, ta.text, maxW)
                        var lineH = fontSize * 1.2
                        var baseX = (ta.x * sx) + padding
                        var baseY = (ta.y * sy) + headerH + padding + fontSize
                        for (var li = 0; li < lines.length; li++) {
                            ctx.fillText(lines[li], baseX, baseY + (li * lineH))
                        }
                    }

                    var items = textOverlay.children
                    for (var i2 = 0; i2 < items.length; i2++) {
                        var item = items[i2]
                        if (!item || !item.boxText || item.boxText.trim() === "") continue
                        var fontSize2 = (item.boxFontSize ? item.boxFontSize : textSize) * sy
                        ctx.fillStyle = item.boxColor ? item.boxColor : "#ffffff"
                        ctx.font = fontSize2 + "px sans-serif"
                        var padding2 = 6 * sy
                        var headerH2 = 18 * sy
                        var maxW2 = (item.width * sx) - (padding2 * 2)
                        var lines2 = wrapText(ctx, item.boxText, maxW2)
                        var lineH2 = fontSize2 * 1.2
                        var baseX2 = (item.x * sx) + padding2
                        var baseY2 = (item.y * sy) + headerH2 + padding2 + fontSize2
                        for (var li2 = 0; li2 < lines2.length; li2++) {
                            ctx.fillText(lines2[li2], baseX2, baseY2 + (li2 * lineH2))
                        }
                    }

                    if (pendingSaveUrl && pendingSaveUrl !== "") {
                        var url = pendingSaveUrl
                        pendingSaveUrl = ""
                        exportSurface.grabToImage(function(result) {
                            var path = SystemController.normalizeLocalPath(url)
                            if (path.indexOf(".") === -1) {
                                path = path + ".png"
                            }
                            result.saveToFile(path)
                        })
                    }
                }
                Component.onCompleted: requestPaint()
            }
        }
    }
}
