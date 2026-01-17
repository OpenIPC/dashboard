import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Window {
    id: root
    title: I18n.t("Настройки")
    width: 680
    height: 600
    visible: false
    color: "#1e1e1e"
    flags: Qt.Window | Qt.FramelessWindowHint

    // Make it non-modal but behave like a standalone window
    function open() {
        show()
        requestActivate()
    }

    function close() {
        hide()
    }

    component StyledCheckBox: CheckBox {
        hoverEnabled: false
        background: Item {}
        indicator: Rectangle {
            implicitWidth: 18
            implicitHeight: 18
            x: parent.leftPadding
            y: parent.height / 2 - height / 2
            radius: 3
            color: "#252526"
            border.color: parent.checked ? "#4caf50" : "#666666"
            
            Rectangle {
                width: 10
                height: 10
                anchors.centerIn: parent
                radius: 2
                color: "#4caf50"
                visible: parent.parent.checked
            }
        }
        contentItem: Text {
            text: parent.text
            font: parent.font
            opacity: parent.enabled ? 1.0 : 0.5
            color: "white"
            verticalAlignment: Text.AlignVCenter
            leftPadding: parent.indicator.width + parent.spacing
        }
    }

    property string language: "ru"
    property string recordingsPath: "C:/Users/User/Videos/OpenIPC"
    property string screenshotsPath: "C:/Users/User/Pictures/OpenIPC"
    property string hwAccel: "auto"
    property bool notificationsEnabled: true
    property string updateStatus: "idle"
    property int updateProgress: 0
    property string updateError: ""

    // Streaming tab state (UI-only for now)
    property string preferredStream: "auto" // auto | hd | sd
    property real playerFillMode: -1.0 // -1 crop/fill, 1 fit, 0 stretch
    property bool showStatsOverlay: true
    property bool defaultAutoplay: true
    property int playerBufferMode: 1
    property string playerRtspTransport: "tcp"
    property string playerHwDecoding: "auto"
    property int recordingSegmentDuration: 15 // Default 15 minutes
    
    // Video Adjustments
    property real playerBrightness: 1.0
    property real playerContrast: 1.0
    property int playerHue: 0
    property real playerSaturation: 1.0
    property real playerGamma: 1.0
    property int playerOrientation: 0
    property bool playerMirror: false

    property var tabLabels: [I18n.t("Общие"), I18n.t("Трансляция"), I18n.t("Аналитика"), I18n.t("Модули"), I18n.t("О программе")]

    // Helper to apply current settings
    function applyCurrentSettings() {
        var settings = {
            "language": language,
            "recordingsPath": recordingsPath,
            "screenshotsPath": screenshotsPath,
            "hwAccel": hwAccel,
            "notificationsEnabled": notificationsEnabled,
            "preferredStream": preferredStream,
            "playerFillMode": playerFillMode,
            "showStatsOverlay": showStatsOverlay,
            "defaultAutoplay": defaultAutoplay,
            "playerBufferMode": playerBufferMode,
            "playerRtspTransport": playerRtspTransport,
            "playerHwDecoding": playerHwDecoding,
            "recordingSegmentDuration": recordingSegmentDuration,
            "playerBrightness": playerBrightness,
            "playerContrast": playerContrast,
            "playerHue": playerHue,
            "playerSaturation": playerSaturation,
            "playerGamma": playerGamma,
            "playerOrientation": playerOrientation,
            "playerMirror": playerMirror
        }
        SystemController.saveAppSettings(settings)
    }

    function loadSettings() {
        var settings = SystemController.getAppSettings()
        if (settings) {
            if (settings.language) language = settings.language
            if (settings.recordingsPath) recordingsPath = settings.recordingsPath
            if (settings.screenshotsPath) screenshotsPath = settings.screenshotsPath
            if (settings.hwAccel) hwAccel = settings.hwAccel
            if (settings.notificationsEnabled !== undefined) notificationsEnabled = settings.notificationsEnabled
            if (settings.preferredStream) preferredStream = settings.preferredStream
            if (settings.playerFillMode !== undefined) playerFillMode = settings.playerFillMode
            if (settings.showStatsOverlay !== undefined) showStatsOverlay = settings.showStatsOverlay
            if (settings.defaultAutoplay !== undefined) defaultAutoplay = settings.defaultAutoplay
            if (settings.playerBufferMode !== undefined) playerBufferMode = settings.playerBufferMode
            if (settings.playerRtspTransport) playerRtspTransport = settings.playerRtspTransport
            if (settings.playerHwDecoding) playerHwDecoding = settings.playerHwDecoding
            if (settings.recordingSegmentDuration !== undefined) recordingSegmentDuration = settings.recordingSegmentDuration
            
            if (settings.playerBrightness !== undefined) playerBrightness = settings.playerBrightness
            if (settings.playerContrast !== undefined) playerContrast = settings.playerContrast
            if (settings.playerHue !== undefined) playerHue = settings.playerHue
            if (settings.playerSaturation !== undefined) playerSaturation = settings.playerSaturation
            if (settings.playerGamma !== undefined) playerGamma = settings.playerGamma
            if (settings.playerOrientation !== undefined) playerOrientation = settings.playerOrientation
            if (settings.playerMirror !== undefined) playerMirror = settings.playerMirror
        }
    }

    Component.onCompleted: {
        loadSettings()
    }

    onLanguageChanged: {
        I18n.language = language
        tabLabels = [I18n.t("Общие"), I18n.t("Трансляция"), I18n.t("Аналитика"), I18n.t("Модули"), I18n.t("О программе")]
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    readonly property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"

    function updateStatusText() {
        switch (updateStatus) {
        case "checking": return I18n.t("Проверка обновлений...");
        case "available": return I18n.t("Доступно обновление");
        case "downloading": return I18n.t("Загрузка обновления...");
        case "downloaded": return I18n.t("Обновление скачано — готово к установке");
        case "installing": return I18n.t("Установка обновления...");
        case "done": return I18n.t("Обновление установлено");
        case "latest": return I18n.t("Установлена последняя версия");
        case "error": return I18n.t("Ошибка") + (updateError !== "" ? ": " + updateError : "");
        default: return I18n.t("Нажмите \"Проверить обновления\"");
        }
    }

    function startUpdateCheck() {
        updateStatus = "checking";
        updateError = "";
        updateProgress = 0;
        updateCheckTimer.restart();
    }

    function trimFileUrl(url) {
        if (!url)
            return "";
        var str = url.toString();
        return str.startsWith("file:///") ? str.substring(8) : str;
    }

    function fileOnly(path) {
        if (!path)
            return "";
        return path.split("/").pop();
    }

    FolderDialog {
        id: recordingsDialog
        title: I18n.t("Выбор папки для записей")
        onAccepted: recordingsPath = trimFileUrl(selectedFolder)
    }

    FolderDialog {
        id: screenshotsDialog
        title: I18n.t("Выбор папки для снимков")
        onAccepted: screenshotsPath = trimFileUrl(selectedFolder)
    }

    FileDialog {
        id: exportConfigDialog
        title: I18n.t("Сохранить конфигурацию")
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON (*.json)", I18n.t("Все файлы (*)")]
        defaultSuffix: "json"
        onAccepted: {
            var path = trimFileUrl(selectedFile)
            console.log("Export config to", path)
            var success = SystemController.exportConfiguration(path)
            if (success) {
                 // Maybe show notification
            }
        }
    }

    FileDialog {
        id: importConfigDialog
        title: I18n.t("Импортировать конфигурацию")
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON (*.json)", I18n.t("Все файлы (*)")]
        onAccepted: {
            var path = trimFileUrl(selectedFile)
            console.log("Import config from", path)
            var success = SystemController.importConfiguration(path)
            if (success) {
                loadSettings()
                root.close()
                // Optional: Notify user
            }
        }
    }

    Timer {
        id: updateCheckTimer
        interval: 1200
        repeat: false
        onTriggered: {
            updateStatus = "latest"
            updateProgress = 0
        }
    }
    
    // Center logic removed, Window centers itself or uses system positioning
    // x: (parent.width - width) / 2 
    // y: (parent.height - height) / 2
    
    // Duplicated width/height removed (defined in root properties)
    
    
    
    // Background Rectangle
    Rectangle {
        id: bgRect
        anchors.fill: parent
        color: "#252526"
        border.color: "#444444"
        border.width: 1
        radius: 8
        z: -1
    }
    
    // Custom Window Header
    Rectangle {
        id: titleBar
        height: 40
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: "#2d2d30"
        z: 100

        MouseArea {
            anchors.fill: parent
            onPressed: root.startSystemMove()
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            anchors.rightMargin: 5
            
            Text {
                text: I18n.t("Настройки — ") + tabLabels[bar.currentIndex]
                color: "#ffffff"
                font.bold: true
                Layout.fillWidth: true
            }
            
            Button {
                text: "—"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.showMinimized()
                contentItem: Text { text: "—"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: parent.down ? "#444" : (parent.hovered ? "#3e3e40" : "transparent") }
            }
            
            Button {
                text: "□"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: {
                    if (root.visibility === Window.Maximized) root.showNormal()
                    else root.showMaximized()
                }
                contentItem: Text { text: "□"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: parent.down ? "#444" : (parent.hovered ? "#3e3e40" : "transparent") }
            }

            Button {
                text: "✕"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.close()
                contentItem: Text { text: "✕"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: parent.down ? "#c42b1c" : (parent.hovered ? "#e81123" : "transparent") }
            }
        }
    }

    // Main Content
    ColumnLayout {
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: footerBar.visible ? footerBar.top : parent.bottom
        spacing: 0
        
        // Tabs
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "transparent"
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 10
                
                Repeater {
                    model: tabLabels

                    Button {
                        text: modelData
                        Layout.preferredHeight: 35
                        Layout.preferredWidth: 100
                        
                        background: Rectangle {
                            color: bar.currentIndex === index ? "#4caf50" : "transparent"
                            radius: 4
                            border.width: 1
                            border.color: bar.currentIndex === index ? "#4caf50" : "#444444"
                        }
                        
                        contentItem: Text {
                            text: parent.text
                            color: bar.currentIndex === index ? "#ffffff" : "#a0aec0"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                        }
                        
                        onClicked: bar.currentIndex = index
                    }
                }
                Item { Layout.fillWidth: true }
            }
        }
        
        // TabBar logic (hidden, used for state)
        TabBar {
            id: bar
            visible: false
                TabButton { text: I18n.t("Общие") }
                TabButton { text: I18n.t("Трансляция") }
                TabButton { text: I18n.t("Аналитика") }
                TabButton { text: I18n.t("Модули") }
                TabButton { text: I18n.t("О программе") }
        }
        
        // Content
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            
            StackLayout {
                anchors.fill: parent
                // margins: 0 to allow scrollbar to hit the edge
                currentIndex: bar.currentIndex
                
                // -------------------------------------------------
                // General Tab
                // -------------------------------------------------
                ScrollView {
                    // Padding for content, but scrollbar stays at right edge
                    leftPadding: 20
                    rightPadding: 20
                    topPadding: 16
                    bottomPadding: 16
                    
                    contentWidth: availableWidth
                    clip: true
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    ScrollBar.vertical: StyledScrollBar {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                    }

                    ColumnLayout {
                        id: generalCol
                        width: parent.width - 40 // Adjust for padding
                        spacing: 14

                        Text {
                            text: I18n.t("Приложение")
                            color: "#ffffff"
                            font.pixelSize: 16
                            font.bold: true
                        }

                        GridLayout {
                            id: generalGrid
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 12
                            Layout.fillWidth: true
                            property int labelWidth: 160

                            Text {
                                text: I18n.t("Язык")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: generalGrid.labelWidth
                            }
                            ComboBox {
                                id: langCombo
                                model: ["English", "Русский"]
                                currentIndex: language === "ru" ? 1 : 0
                                Layout.fillWidth: true
                                Layout.preferredHeight: 32
                                onActivated: language = index === 1 ? "ru" : "en"
                                background: Rectangle { color: "#1f2733"; radius: 4; border.color: "#4a5568" }
                                contentItem: Text {
                                    text: langCombo.displayText
                                    color: "white"
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 8
                                    rightPadding: 24
                                }
                                indicator: Canvas {
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 8
                                    width: 12; height: 8
                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.fillStyle = "#a0aec0";
                                        ctx.beginPath();
                                        ctx.moveTo(0, 0);
                                        ctx.lineTo(width, 0);
                                        ctx.lineTo(width/2, height);
                                        ctx.closePath();
                                        ctx.fill();
                                    }
                                }
                            }

                            Text {
                                text: I18n.t("Папка записей")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: generalGrid.labelWidth
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                TextField {
                                    text: recordingsPath
                                    readOnly: true
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 30
                                    color: "white"
                                    background: Rectangle { color: "#1f2733"; border.color: "#4a5568"; radius: 4 }
                                }
                                Button {
                                    Layout.preferredHeight: 30
                                    Layout.preferredWidth: 34
                                    background: Rectangle { color: "#4a5568"; radius: 4 }
                                    contentItem: Text {
                                        text: "folder_open"
                                        font.family: iconFontFamily
                                        font.pixelSize: 15
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: recordingsDialog.open()
                                }
                            }

                            Text {
                                text: I18n.t("Папка снимков")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: generalGrid.labelWidth
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                TextField {
                                    text: screenshotsPath
                                    readOnly: true
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 30
                                    color: "white"
                                    background: Rectangle { color: "#1f2733"; border.color: "#4a5568"; radius: 4 }
                                }
                                Button {
                                    Layout.preferredHeight: 30
                                    Layout.preferredWidth: 34
                                    background: Rectangle { color: "#4a5568"; radius: 4 }
                                    contentItem: Text {
                                        text: "folder_open"
                                        font.family: iconFontFamily
                                        font.pixelSize: 15
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: screenshotsDialog.open()
                                }
                            }

                            Text {
                                text: I18n.t("Длительность записи (мин)")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: generalGrid.labelWidth
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                
                                SpinBox {
                                    id: segmentSpin
                                    from: 5
                                    to: 60
                                    stepSize: 5
                                    value: recordingSegmentDuration
                                    onValueModified: recordingSegmentDuration = value
                                    editable: true
                                    Layout.preferredHeight: 32
                                    Layout.preferredWidth: 120
                                    
                                    // Make room for indicators
                                    leftPadding: 30
                                    rightPadding: 30
                                    
                                    contentItem: TextInput {
                                        text: segmentSpin.textFromValue(segmentSpin.value, segmentSpin.locale)
                                        font: segmentSpin.font
                                        color: "white"
                                        selectionColor: "#2196F3"
                                        selectedTextColor: "#ffffff"
                                        horizontalAlignment: Qt.AlignHCenter
                                        verticalAlignment: Qt.AlignVCenter
                                        readOnly: !segmentSpin.editable
                                        validator: segmentSpin.validator
                                        inputMethodHints: Qt.ImhDigitsOnly
                                    }

                                    background: Rectangle { 
                                        color: "#1f2733"
                                        border.color: "#4a5568"
                                        radius: 4 
                                    }
                                    
                                    up.indicator: Rectangle {
                                        x: parent.width - width
                                        height: parent.height
                                        width: 30
                                        color: parent.up.pressed ? "#3e3e40" : "transparent"
                                        Text { text: "+"; color: "#a0aec0"; font.pixelSize: 18; anchors.centerIn: parent }
                                    }
                                    down.indicator: Rectangle {
                                        x: 0
                                        height: parent.height
                                        width: 30
                                        color: parent.down.pressed ? "#3e3e40" : "transparent"
                                        Text { text: "-"; color: "#a0aec0"; font.pixelSize: 18; anchors.centerIn: parent }
                                    }
                                }
                                
                                Text {
                                    text: I18n.t("5-60 мин")
                                    color: "#666"
                                    font.pixelSize: 12
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#3b4657"
                        }

                        GridLayout {
                            id: notificationsGrid
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 8
                            Layout.fillWidth: true
                            property int labelWidth: 160

                            Text {
                                text: I18n.t("Уведомления")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.alignment: Qt.AlignVCenter
                                Layout.preferredWidth: notificationsGrid.labelWidth
                            }
                            RowLayout {
                                spacing: 8
                                Layout.alignment: Qt.AlignVCenter
                                Layout.fillWidth: true
                                StyledCheckBox {
                                    checked: notificationsEnabled
                                    onToggled: notificationsEnabled = checked
                                    text: I18n.t("Включить уведомления")
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#3b4657"
                        }

                        ColumnLayout {
                            spacing: 12
                            Text {
                                text: I18n.t("Управление конфигурацией")
                                color: "#ffffff"
                                font.pixelSize: 16
                                font.bold: true
                            }
                            RowLayout {
                                spacing: 10
                                Button {
                                    text: I18n.t("Экспорт конфигурации")
                                    Layout.preferredHeight: 32
                                    Layout.preferredWidth: 190
                                    background: Rectangle { color: "#3b82f6"; radius: 6 }
                                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    onClicked: exportConfigDialog.open()
                                }
                                Button {
                                    text: I18n.t("Импорт конфигурации")
                                    Layout.preferredHeight: 32
                                    Layout.preferredWidth: 190
                                    background: Rectangle { color: "#e53e3e"; radius: 6 }
                                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    onClicked: importConfigDialog.open()
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#3b4657"
                        }

                        ColumnLayout {
                            spacing: 12
                            Text {
                                text: I18n.t("Обновления")
                                color: "#ffffff"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            GridLayout {
                                id: updatesGrid
                                columns: 2
                                columnSpacing: 14
                                rowSpacing: 10
                                Layout.fillWidth: true
                                property int labelWidth: 160

                                Text { text: I18n.t("Статус"); color: "#a0aec0"; Layout.preferredWidth: updatesGrid.labelWidth }
                                Text { text: updateStatusText(); color: "#e2e8f0"; wrapMode: Text.WordWrap; Layout.fillWidth: true }

                                Text { text: I18n.t("Действие"); color: "#a0aec0"; Layout.preferredWidth: updatesGrid.labelWidth }
                                Button {
                                    text: I18n.t("Проверить обновления")
                                    enabled: updateStatus !== "checking"
                                    Layout.preferredHeight: 34
                                    Layout.preferredWidth: 190
                                    background: Rectangle { color: "#4299e1"; radius: 6 }
                                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    onClicked: startUpdateCheck()
                                }

                                // Progress bar
                                Text {
                                    visible: updateStatus === "downloading" || updateStatus === "downloaded"
                                    text: I18n.t("Прогресс")
                                    color: "#a0aec0"
                                    Layout.preferredWidth: updatesGrid.labelWidth
                                    Layout.preferredHeight: visible ? implicitHeight : 0
                                }
                                ColumnLayout {
                                    visible: updateStatus === "downloading" || updateStatus === "downloaded"
                                    spacing: 6
                                    Layout.preferredHeight: visible ? implicitHeight : 0
                                    Rectangle {
                                        color: "#2f3338"
                                        border.color: "#333"
                                        radius: 4
                                        width: 240
                                        height: 10
                                        Rectangle {
                                            width: (updateProgress / 100) * parent.width
                                            height: parent.height
                                            radius: 4
                                            color: "#4caf50"
                                            anchors.left: parent.left
                                        }
                                    }
                                    Text {
                                        text: updateStatus === "downloading" ? (updateProgress + "%") : ""
                                        color: "#9da3ad"
                                        font.pixelSize: 12
                                    }
                                }

                                // Error message
                                Text { text: updateStatus === "error" ? I18n.t("Ошибка") : ""; color: updateStatus === "error" ? "#e53e3e" : "transparent"; Layout.preferredWidth: updatesGrid.labelWidth }
                                Text { text: updateStatus === "error" ? updateError : ""; color: "#e53e3e"; wrapMode: Text.WordWrap; Layout.fillWidth: true }

                                // Install actions when downloaded
                                Text { text: updateStatus === "downloaded" ? I18n.t("Установка") : ""; color: updateStatus === "downloaded" ? "#a0aec0" : "transparent"; Layout.preferredWidth: updatesGrid.labelWidth }
                                RowLayout {
                                    visible: updateStatus === "downloaded"
                                    spacing: 8
                                    Button {
                                        text: I18n.t("Установить и перезапустить")
                                        Layout.preferredHeight: 34
                                        Layout.preferredWidth: 210
                                        background: Rectangle { color: "#3b82f6"; radius: 6 }
                                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    }
                                    Button {
                                        text: I18n.t("Позже")
                                        Layout.preferredHeight: 34
                                        Layout.preferredWidth: 110
                                        background: Rectangle { color: "#e53e3e"; radius: 6 }
                                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    }
                                }
                            }
                        }
                    }
                }
                
                // -------------------------------------------------
                // Streaming Tab
                // -------------------------------------------------
                Item {
                    ScrollView {
                        anchors.fill: parent
                        // Padding for content
                        leftPadding: 20
                        rightPadding: 20
                        topPadding: 16
                        bottomPadding: 16
                        
                        contentWidth: availableWidth
                        contentHeight: streamingCol.height
                        clip: true
                        ScrollBar.vertical: StyledScrollBar {
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                        }

                        ColumnLayout {
                            id: streamingCol
                            width: parent.width - 40 // Adjust for padding
                            spacing: 14
                            // anchors.top: parent.top
                            // anchors.left: parent.left
                            // anchors.leftMargin: 4

                        Text {
                            text: I18n.t("Трансляция (GStreamer)")
                            color: "#ffffff"
                            font.pixelSize: 16
                            font.bold: true
                        }

                        GridLayout {
                            id: streamingGrid
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10
                            Layout.fillWidth: true
                            property int labelWidth: 180

                            Text {
                                text: I18n.t("Режим буферизации (Latency)")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: streamingGrid.labelWidth
                            }
                            ComboBox {
                                id: bufferModeCombo
                                model: [I18n.t("Стандартная (Stable / 2s)"), I18n.t("Минимальная (Low Latency / 200ms)"), I18n.t("Ультра-низкая (Realtime / 0ms)")]
                                currentIndex: playerBufferMode
                                Layout.fillWidth: true
                                Layout.preferredHeight: 32
                                background: Rectangle { color: "#1f2733"; radius: 4; border.color: "#4a5568" }
                                contentItem: Text {
                                    text: bufferModeCombo.displayText
                                    color: "white"
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 8
                                    rightPadding: 24
                                }
                                indicator: Canvas {
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 8
                                    width: 12; height: 8
                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.fillStyle = "#a0aec0";
                                        ctx.beginPath();
                                        ctx.moveTo(0, 0);
                                        ctx.lineTo(width, 0);
                                        ctx.lineTo(width/2, height);
                                        ctx.closePath();
                                        ctx.fill();
                                    }
                                }
                                onActivated: {
                                    playerBufferMode = index
                                }
                            }

                            Text {
                                text: I18n.t("Протокол RTSP")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: streamingGrid.labelWidth
                            }
                            ComboBox {
                                id: rtspTransportCombo
                                model: ["TCP (Interleaved)", "UDP (Unicast)", "UDP (Multicast)", "HTTP (Tunneling)"]
                                currentIndex: playerRtspTransport === "tcp" ? 0 : (playerRtspTransport === "udp" ? 1 : (playerRtspTransport === "udp_mcast" ? 2 : 3))
                                Layout.fillWidth: true
                                Layout.preferredHeight: 32
                                background: Rectangle { color: "#1f2733"; radius: 4; border.color: "#4a5568" }
                                contentItem: Text {
                                    text: rtspTransportCombo.displayText
                                    color: "white"
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 8
                                    rightPadding: 24
                                }
                                indicator: Canvas {
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 8
                                    width: 12; height: 8
                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.fillStyle = "#a0aec0";
                                        ctx.beginPath();
                                        ctx.moveTo(0, 0);
                                        ctx.lineTo(width, 0);
                                        ctx.lineTo(width/2, height);
                                        ctx.closePath();
                                        ctx.fill();
                                    }
                                }
                                onActivated: {
                                    if (index === 0) playerRtspTransport = "tcp"
                                    else if (index === 1) playerRtspTransport = "udp"
                                    else if (index === 2) playerRtspTransport = "udp_mcast"
                                    else if (index === 3) playerRtspTransport = "http"
                                }
                            }

                            Text {
                                text: I18n.t("Аппаратное декодирование")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: streamingGrid.labelWidth
                            }
                            ComboBox {
                                id: hwDecodingCombo
                                model: [I18n.t("Авто"), "D3D11", "DXVA2", "Off (None)"]
                                currentIndex: {
                                    if (playerHwDecoding === "d3d11") return 1
                                    if (playerHwDecoding === "dxva2") return 2
                                    if (playerHwDecoding === "none") return 3
                                    return 0 // auto
                                }
                                Layout.fillWidth: true
                                Layout.preferredHeight: 32
                                background: Rectangle { color: "#1f2733"; radius: 4; border.color: "#4a5568" }
                                contentItem: Text {
                                    text: hwDecodingCombo.displayText
                                    color: "white"
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 8
                                    rightPadding: 24
                                }
                                indicator: Canvas {
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 8
                                    width: 12; height: 8
                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.fillStyle = "#a0aec0";
                                        ctx.beginPath();
                                        ctx.moveTo(0, 0);
                                        ctx.lineTo(width, 0);
                                        ctx.lineTo(width/2, height);
                                        ctx.closePath();
                                        ctx.fill();
                                    }
                                }
                                onActivated: {
                                    if (index === 1) playerHwDecoding = "d3d11"
                                    else if (index === 2) playerHwDecoding = "dxva2"
                                    else if (index === 3) playerHwDecoding = "none"
                                    else playerHwDecoding = "auto"
                                }
                            }

                            Text {
                                text: I18n.t("Предпочтительный поток")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: streamingGrid.labelWidth
                            }
                            ComboBox {
                                id: streamPrefCombo
                                model: [I18n.t("Авто"), "HD", "SD"]
                                currentIndex: preferredStream === "hd" ? 1 : preferredStream === "sd" ? 2 : 0
                                Layout.fillWidth: true
                                Layout.preferredHeight: 32
                                background: Rectangle { color: "#1f2733"; radius: 4; border.color: "#4a5568" }
                                contentItem: Text {
                                    text: streamPrefCombo.displayText
                                    color: "white"
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 8
                                    rightPadding: 24
                                }
                                indicator: Canvas {
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 8
                                    width: 12; height: 8
                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.fillStyle = "#a0aec0";
                                        ctx.beginPath();
                                        ctx.moveTo(0, 0);
                                        ctx.lineTo(width, 0);
                                        ctx.lineTo(width/2, height);
                                        ctx.closePath();
                                        ctx.fill();
                                    }
                                }
                                onActivated: {
                                    if (index === 1) preferredStream = "hd";
                                    else if (index === 2) preferredStream = "sd";
                                    else preferredStream = "auto";
                                }
                            }
                            
                            Text {
                                text: I18n.t("Отображать статистику")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: streamingGrid.labelWidth
                            }
                            RowLayout {
                                spacing: 8
                                Layout.alignment: Qt.AlignVCenter
                                Layout.fillWidth: true
                                Layout.preferredHeight: 26
                                StyledCheckBox {
                                    checked: showStatsOverlay
                                    onToggled: showStatsOverlay = checked
                                    text: I18n.t("Показывать codec/res/bitrate/fps")
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#3b4657"
                        }
/*
                            Text {
                                text: I18n.t("Режим кадра")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: streamingGrid.labelWidth
                            }
                            ComboBox {
                                id: fillModeCombo
                                model: [I18n.t("Обрезать по краям"), I18n.t("Сохранять пропорции"), I18n.t("Растянуть")]
                                currentIndex: playerFillMode === 1 ? 1 : (playerFillMode === 0 ? 2 : 0)
                                Layout.fillWidth: true
                                Layout.preferredHeight: 32
                                background: Rectangle { color: "#1f2733"; radius: 4; border.color: "#4a5568" }
                                contentItem: Text {
                                    text: fillModeCombo.displayText
                                    color: "white"
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 8
                                    rightPadding: 24
                                }
                                indicator: Canvas {
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 8
                                    width: 12; height: 8
                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.fillStyle = "#a0aec0";
                                        ctx.beginPath();
                                        ctx.moveTo(0, 0);
                                        ctx.lineTo(width, 0);
                                        ctx.lineTo(width/2, height);
                                        ctx.closePath();
                                        ctx.fill();
                                    }
                                }
                                onActivated: {
                                    if (index === 1) playerFillMode = 1.0; // fit/keep aspect
                                    else if (index === 2) playerFillMode = 0.0; // stretch
                                    else playerFillMode = -1.0; // crop/fill
                                }
                            }
*/

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#3b4657"
                        }

                        Text {
                            text: I18n.t("Настройки изображения")
                            color: "#ffffff"
                            font.pixelSize: 16
                            font.bold: true
                        }

                        GridLayout {
                            id: adjustmentsGrid
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10
                            Layout.fillWidth: true
                            property int labelWidth: 180
                            
                            // Transform
                            Text {
                                text: I18n.t("Поворот / Зеркало")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: adjustmentsGrid.labelWidth
                            }
                            RowLayout {
                                spacing: 10
                                Layout.fillWidth: true
                                ComboBox {
                                    model: ["0°", "90°", "180°", "270°"]
                                    currentIndex: {
                                        if (playerOrientation === 90) return 1
                                        if (playerOrientation === 180) return 2
                                        if (playerOrientation === 270) return 3
                                        return 0
                                    }
                                    onActivated: {
                                        if (index === 1) playerOrientation = 90
                                        else if (index === 2) playerOrientation = 180
                                        else if (index === 3) playerOrientation = 270
                                        else playerOrientation = 0
                                    }
                                    Layout.preferredWidth: 100
                                    Layout.preferredHeight: 32
                                    background: Rectangle { color: "#1f2733"; radius: 4; border.color: "#4a5568" }
                                    contentItem: Text {
                                        text: parent.displayText
                                        color: "white"
                                        verticalAlignment: Text.AlignVCenter
                                        leftPadding: 8
                                    }
                                }
                                StyledCheckBox {
                                    text: I18n.t("Зеркально")
                                    checked: playerMirror
                                    onToggled: playerMirror = checked
                                }
                            }

                            // Brightness
                            Text {
                                text: I18n.t("Яркость") + " (" + playerBrightness.toFixed(2) + ")"
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: adjustmentsGrid.labelWidth
                            }
                            Slider {
                                from: 0.0; to: 2.0
                                value: playerBrightness
                                Layout.fillWidth: true
                                onMoved: playerBrightness = value
                                onPressedChanged: if (!pressed) applyCurrentSettings()
                            }
                            
                            // Contrast
                            Text {
                                text: I18n.t("Контраст") + " (" + playerContrast.toFixed(2) + ")"
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: adjustmentsGrid.labelWidth
                            }
                            Slider {
                                from: 0.0; to: 2.0
                                value: playerContrast
                                Layout.fillWidth: true
                                onMoved: playerContrast = value
                                onPressedChanged: if (!pressed) applyCurrentSettings()
                            }
                            
                            // Saturation
                            Text {
                                text: I18n.t("Насыщенность") + " (" + playerSaturation.toFixed(2) + ")"
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: adjustmentsGrid.labelWidth
                            }
                            Slider {
                                from: 0.0; to: 2.0
                                value: playerSaturation
                                Layout.fillWidth: true
                                onMoved: playerSaturation = value
                                onPressedChanged: if (!pressed) applyCurrentSettings()
                            }
                            
                            // Gamma
                            Text {
                                text: I18n.t("Гамма") + " (" + playerGamma.toFixed(2) + ")"
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: adjustmentsGrid.labelWidth
                            }
                            Slider {
                                from: 0.01; to: 3.0
                                value: playerGamma
                                Layout.fillWidth: true
                                onMoved: playerGamma = value
                                onPressedChanged: if (!pressed) applyCurrentSettings()
                            }

                            // Hue
                            Text {
                                text: I18n.t("Оттенок") + " (" + playerHue + ")"
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: adjustmentsGrid.labelWidth
                            }
                            Slider {
                                from: -180; to: 180
                                stepSize: 1
                                value: playerHue
                                Layout.fillWidth: true
                                onMoved: playerHue = value
                                onPressedChanged: if (!pressed) applyCurrentSettings()
                            }

                            // Reset Button
                            Item { Layout.fillWidth: true; Layout.columnSpan: 2; height: 10 }
                            Button {
                                text: I18n.t("Сбросить настройки изображения")
                                Layout.columnSpan: 2
                                Layout.alignment: Qt.AlignHCenter
                                onClicked: {
                                    playerBrightness = 1.0
                                    playerContrast = 1.0
                                    playerHue = 0
                                    playerSaturation = 1.0
                                    playerGamma = 1.0
                                    playerOrientation = 0
                                    playerMirror = false
                                    applyCurrentSettings()
                                }
                                background: Rectangle { color: "#4a5568"; radius: 4 }
                                contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#3b4657"
                        }

                        Item { Layout.fillHeight: true }
                    }
                    }
                }
                
                // -------------------------------------------------
                // Analytics Tab
                // -------------------------------------------------
                Item {
                    Text { text: "Analytics Settings (Placeholder)"; color: "#a0aec0"; anchors.centerIn: parent }
                }
                
                // -------------------------------------------------
                // Modules Tab
                // -------------------------------------------------
                ModulesSettingsPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                
                // -------------------------------------------------
                // About Tab
                // -------------------------------------------------
                Item {
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 15
                        
                        Text { 
                            text: "OpenIPC Dashboard"
                            color: "#ffffff"
                            font.pixelSize: 24
                            font.bold: true
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text { 
                            text: "Version 0.1.0"
                            color: "#a0aec0"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text { 
                            text: "© 2025 OpenIPC Project"
                            color: "#626974"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Rectangle {
                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 40
                            Layout.alignment: Qt.AlignHCenter
                            color: supportArea.containsMouse ? "#1565c0" : "#1976d2" // Blue color
                            radius: 4

                            RowLayout {
                                anchors.centerIn: parent
                                spacing: 8
                                Text {
                                    text: "favorite" // Heart icon
                                    font.family: "Material Icons"
                                    color: "white"
                                    font.pixelSize: 16
                                }
                                Text {
                                    text: I18n.t("Поддержать проект")
                                    color: "white"
                                    font.bold: true
                                    font.pixelSize: 14
                                }
                            }

                            MouseArea {
                                id: supportArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally("https://opencollective.com/openipc/projects/openipc-dashboard/donate?interval=oneTime&amount=20&contributeAs=me")
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: footerBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60
        color: "transparent"
        visible: bar.currentIndex !== 4 // Hide on About tab
        z: 10
        
        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: "#626974"
        }

        Button {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 20
            width: 120
            height: 36
            text: I18n.t("Сохранить")
            
            background: Rectangle {
                color: "#4299e1"
                radius: 4
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.bold: true
                font.pixelSize: 14
            }
            
            onClicked: {
                applyCurrentSettings()
                root.close()
            }
        }
    }
    
    Popup {
        id: saveNotification
        anchors.centerIn: parent
        width: 240
        height: 40
        modal: false
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        
        background: Rectangle {
            color: "#2d3748"
            radius: 4
            border.color: "#48bb78" // Green border
            border.width: 1
        }
        
        contentItem: RowLayout {
            anchors.centerIn: parent
            spacing: 8
            Text {
                text: "✓"
                color: "#48bb78"
                font.bold: true
                font.pixelSize: 16
            }
            Text {
                text: I18n.t("Настройки успешно сохранены")
                color: "white"
                font.pixelSize: 14
            }
        }
        
        Timer {
            interval: 2000
            running: saveNotification.visible
            onTriggered: saveNotification.close()
        }
    }
}
