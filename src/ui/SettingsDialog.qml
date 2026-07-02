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
    color: Theme.appBackground
    flags: Qt.Window | Qt.FramelessWindowHint
    modality: Qt.ApplicationModal

    // Make it non-modal but behave like a standalone window
    function open() {
        show()
        requestActivate()
    }

    function close() {
        hide()
    }

    component StyledCheckBox: CheckBox {
        id: checkBoxControl

        hoverEnabled: false
        background: Item {}
        indicator: Rectangle {
            implicitWidth: 18
            implicitHeight: 18
            x: checkBoxControl.leftPadding
            y: checkBoxControl.height / 2 - height / 2
            radius: Theme.radiusXs
            color: Theme.topBarBackground
            border.color: checkBoxControl.checked ? Theme.accent : Theme.textFaint
            
            Rectangle {
                width: 10
                height: 10
                anchors.centerIn: parent
                radius: 2
                color: Theme.accent
                visible: checkBoxControl.checked
            }
        }
        contentItem: Text {
            text: checkBoxControl.text
            font: checkBoxControl.font
            opacity: checkBoxControl.enabled ? 1.0 : 0.5
            color: Theme.textPrimary
            verticalAlignment: Text.AlignVCenter
            leftPadding: checkBoxControl.indicator.width + checkBoxControl.spacing
        }
    }

    component StyledSpinBox: SpinBox {
        id: spinRoot
        editable: true
        implicitHeight: 32
        implicitWidth: 120

        leftPadding: 30
        rightPadding: 30

        contentItem: TextInput {
            text: spinRoot.textFromValue(spinRoot.value, spinRoot.locale)
            font: spinRoot.font
            color: Theme.textSecondary
            selectionColor: Theme.accent
            selectedTextColor: Theme.textPrimary
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            readOnly: !spinRoot.editable
            validator: spinRoot.validator
            inputMethodHints: Qt.ImhDigitsOnly
        }

        background: Rectangle {
            color: Theme.panelSoftBackground
            border.color: Theme.controlBorderStrong
            border.width: 1
            radius: Theme.radiusSm
        }

        up.indicator: Rectangle {
            x: spinRoot.width - width
            height: spinRoot.height
            width: 30
            color: spinRoot.up.pressed ? Theme.cardHover : "transparent"
            Text {
                text: "+"
                color: Theme.textMuted
                font.pixelSize: 16
                anchors.centerIn: parent
            }
        }

        down.indicator: Rectangle {
            x: 0
            height: spinRoot.height
            width: 30
            color: spinRoot.down.pressed ? Theme.cardHover : "transparent"
            Text {
                text: "-"
                color: Theme.textMuted
                font.pixelSize: 16
                anchors.centerIn: parent
            }
        }
    }

    property string language: "ru"
    property string recordingsPath: "C:/Users/User/Videos/OpenIPC"
    property string screenshotsPath: "C:/Users/User/Pictures/OpenIPC"
    property string hwAccel: "auto"
    property bool notificationsEnabled: true
    property string updateStatus: "idle"
    property string updateError: ""
    property bool updateInstallAfterDownload: false

    // Streaming tab state (UI-only for now)
    property string preferredStream: "auto" // auto | hd | sd
    property real playerFillMode: -1.0 // -1 crop/fill, 1 fit, 0 stretch
    property bool showStatsOverlay: true
    property bool defaultAutoplay: true
    property bool smartStreamBudget: true
    property int maxPreviewStreams: 16
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

    // Evidence settings (analytics-driven)
    property bool evidenceEnabled: false
    property bool evidenceSnapshotsEnabled: true
    property bool evidenceClipsEnabled: true
    property string evidenceSnapshotsPath: ""
    property string evidenceClipsPath: ""
    property int evidencePreSeconds: 5
    property int evidencePostSeconds: 5
    property real evidenceMinConfidence: 0.6
    property int evidenceClipFps: 10
    property string analyticsPerformancePreset: "balanced"
    property int analyticsTargetFps: 3
    property int analyticsMaxParallelJobs: 2
    property bool evidenceUploadEnabled: false
    property string evidenceUploadProvider: "local"
    property string evidenceUploadTarget: ""
    property string evidenceUploadClientId: ""
    property string evidenceUploadClientSecret: ""
    property string oauthUrl: ""
    property string oauthProvider: ""
    property string evidenceUploadFolder: ""
    property string evidenceUploadPath: ""
    property string evidenceUploadAccessToken: ""
    property string evidenceUploadRefreshToken: ""
    property string evidenceUploadExpiresAt: ""
    property bool evidenceOAuthAdvanced: false

    readonly property var defaultOAuthClientIds: ({
        "gdrive": "742960614023-ac2alrmie0lmm0pf208ieatgilkkt1el.apps.googleusercontent.com",
        "dropbox": "4oz9zu8h8s30ofu"
    })

    function effectiveClientId(provider) {
        if (!evidenceOAuthAdvanced && defaultOAuthClientIds[provider]) return defaultOAuthClientIds[provider]
        return evidenceUploadClientId
    }

    function analyticsPresetLabel(preset) {
        if (preset === "eco") return I18n.t("Экономный")
        if (preset === "balanced") return I18n.t("Сбалансированный")
        if (preset === "max") return I18n.t("Максимум")
        return I18n.t("Ручной")
    }

    function analyticsPresetDescription(preset) {
        if (preset === "eco") return I18n.t("Минимальная нагрузка: подходит для слабого CPU или большого числа камер.")
        if (preset === "balanced") return I18n.t("Оптимальный режим для постоянной работы нескольких камер.")
        if (preset === "max") return I18n.t("Больше кадров и параллельных задач: используйте при запасе CPU/GPU.")
        return I18n.t("Пользовательские значения FPS и параллельных задач.")
    }

    function applyAnalyticsPreset(preset) {
        analyticsPerformancePreset = preset
        if (preset === "eco") {
            analyticsTargetFps = 1
            analyticsMaxParallelJobs = 1
        } else if (preset === "max") {
            analyticsTargetFps = 8
            analyticsMaxParallelJobs = 4
        } else {
            analyticsPerformancePreset = "balanced"
            analyticsTargetFps = 3
            analyticsMaxParallelJobs = 2
        }
        applyCurrentSettings()
    }

    property var tabLabels: [I18n.t("Общие"), I18n.t("Трансляция"), I18n.t("Аналитика"), I18n.t("О программе")]

    function languageIndex(value) {
        return value === "ru" ? 1 : 0
    }

    function languageFromIndex(index) {
        return index === 1 ? "ru" : "en"
    }

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
            "smartStreamBudget": smartStreamBudget,
            "maxPreviewStreams": maxPreviewStreams,
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

        var analyticsSettings = SystemController.analyticsEngine.getSettings()
        var currentEvidence = (analyticsSettings && analyticsSettings.evidence) ? analyticsSettings.evidence : {}
        var snapshotsDirToSave = evidenceSnapshotsPath
        var clipsDirToSave = evidenceClipsPath
        if (!snapshotsDirToSave || snapshotsDirToSave.trim() === "") {
            snapshotsDirToSave = currentEvidence.snapshotsDir || ""
        }
        if (!clipsDirToSave || clipsDirToSave.trim() === "") {
            clipsDirToSave = currentEvidence.clipsDir || ""
        }
        analyticsSettings["evidence"] = {
            "enabled": evidenceEnabled,
            "snapshotsEnabled": evidenceSnapshotsEnabled,
            "clipsEnabled": evidenceClipsEnabled,
            "snapshotsDir": snapshotsDirToSave,
            "clipsDir": clipsDirToSave,
            "preSeconds": evidencePreSeconds,
            "postSeconds": evidencePostSeconds,
            "minConfidence": evidenceMinConfidence,
            "clipFps": evidenceClipFps,
            "uploadEnabled": evidenceUploadEnabled,
            "uploadProvider": evidenceUploadProvider,
            "uploadTarget": evidenceUploadTarget,
            "uploadClientId": evidenceUploadClientId,
            "uploadClientSecret": evidenceUploadClientSecret,
            "uploadAccessToken": evidenceUploadAccessToken,
            "uploadRefreshToken": evidenceUploadRefreshToken,
            "uploadExpiresAt": evidenceUploadExpiresAt
        }
        analyticsSettings["performance"] = {
            "preset": analyticsPerformancePreset,
            "targetFps": analyticsTargetFps,
            "maxParallelJobs": analyticsMaxParallelJobs
        }
        SystemController.analyticsEngine.setSettings(analyticsSettings)
    }

    function loadSettings() {
        var settings = SystemController.getAppSettings()
        if (settings) {
            if (settings.language) language = settings.language
            if (settings.recordingsPath) recordingsPath = normalizePath(settings.recordingsPath)
            if (settings.screenshotsPath) screenshotsPath = normalizePath(settings.screenshotsPath)
            if (settings.hwAccel) hwAccel = settings.hwAccel
            if (settings.notificationsEnabled !== undefined) notificationsEnabled = settings.notificationsEnabled
            if (settings.preferredStream) preferredStream = settings.preferredStream
            if (settings.playerFillMode !== undefined) playerFillMode = settings.playerFillMode
            if (settings.showStatsOverlay !== undefined) showStatsOverlay = settings.showStatsOverlay
            if (settings.defaultAutoplay !== undefined) defaultAutoplay = settings.defaultAutoplay
            if (settings.smartStreamBudget !== undefined) smartStreamBudget = settings.smartStreamBudget
            if (settings.maxPreviewStreams !== undefined) maxPreviewStreams = settings.maxPreviewStreams
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

        var analyticsSettings = SystemController.analyticsEngine.getSettings()
        if (analyticsSettings && analyticsSettings.evidence) {
            var ev = analyticsSettings.evidence
            if (ev.enabled !== undefined) evidenceEnabled = ev.enabled
            if (ev.snapshotsEnabled !== undefined) evidenceSnapshotsEnabled = ev.snapshotsEnabled
            if (ev.clipsEnabled !== undefined) evidenceClipsEnabled = ev.clipsEnabled
            if (ev.snapshotsDir) evidenceSnapshotsPath = normalizePath(ev.snapshotsDir)
            if (ev.clipsDir) evidenceClipsPath = normalizePath(ev.clipsDir)
            if (ev.preSeconds !== undefined) evidencePreSeconds = ev.preSeconds
            if (ev.postSeconds !== undefined) evidencePostSeconds = ev.postSeconds
            if (ev.minConfidence !== undefined) evidenceMinConfidence = ev.minConfidence
            if (ev.clipFps !== undefined) evidenceClipFps = ev.clipFps
            if (ev.uploadEnabled !== undefined) evidenceUploadEnabled = ev.uploadEnabled
            if (ev.uploadProvider) evidenceUploadProvider = ev.uploadProvider
            if (ev.uploadTarget) evidenceUploadTarget = ev.uploadTarget
            if (ev.uploadClientId) evidenceUploadClientId = ev.uploadClientId
            if (ev.uploadClientSecret) evidenceUploadClientSecret = ev.uploadClientSecret
            if (ev.uploadAccessToken) evidenceUploadAccessToken = ev.uploadAccessToken
            if (ev.uploadRefreshToken) evidenceUploadRefreshToken = ev.uploadRefreshToken
            if (ev.uploadExpiresAt) evidenceUploadExpiresAt = ev.uploadExpiresAt
            var map = parseTarget(evidenceUploadTarget)
            evidenceUploadFolder = map.folder || ""
            evidenceUploadPath = map.path || ""
        }
        if (analyticsSettings && analyticsSettings.performance) {
            var performance = analyticsSettings.performance
            if (performance.preset) analyticsPerformancePreset = performance.preset
            if (performance.targetFps !== undefined) analyticsTargetFps = performance.targetFps
            if (performance.maxParallelJobs !== undefined) analyticsMaxParallelJobs = performance.maxParallelJobs
        }
    }

    Component.onCompleted: {
        loadSettings()
    }

    Connections {
        target: SystemController.analyticsEngine
        function onOauthUrlReady(provider, url) {
            oauthProvider = provider
            oauthUrl = url
            oauthDialog.open()
        }
        function onOauthCompleted(provider, accessToken, refreshToken, expiresIn) {
            oauthProvider = provider
            var map = parseTarget(evidenceUploadTarget)
            evidenceUploadAccessToken = accessToken
            if (refreshToken) evidenceUploadRefreshToken = refreshToken
            if (expiresIn) evidenceUploadExpiresAt = String(Math.floor(Date.now() / 1000) + expiresIn)
            if (provider === "gdrive") {
                map.folder = evidenceUploadFolder || map.folder || ""
            } else if (provider === "onedrive" || provider === "dropbox" || provider === "yadisk") {
                map.path = evidenceUploadPath || map.path || "/OpenIPC"
            }
            evidenceUploadTarget = buildTarget(map)
            applyCurrentSettings()
            oauthDialog.close()
        }
        function onOauthError(provider, message) {
            oauthDialog.close()
        }
    }

    function parseTarget(target) {
        var map = {}
        if (!target) return map
        var parts = target.split(";")
        for (var i = 0; i < parts.length; i++) {
            var p = parts[i]
            var idx = p.indexOf("=")
            if (idx > 0) {
                var k = p.substring(0, idx).trim()
                var v = p.substring(idx + 1).trim()
                map[k] = v
            }
        }
        return map
    }

    function buildTarget(map) {
        var out = []
        for (var k in map) {
            if (map[k] !== undefined && map[k] !== "") out.push(k + "=" + map[k])
        }
        return out.join(";")
    }

    onLanguageChanged: {
        I18n.language = language
        tabLabels = [I18n.t("Общие"), I18n.t("Трансляция"), I18n.t("Аналитика"), I18n.t("О программе")]
        if (typeof langCombo !== "undefined" && langCombo.currentIndex !== languageIndex(language)) {
            langCombo.currentIndex = languageIndex(language)
        }
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    readonly property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"

    function updateStatusText() {
        var checker = SystemController.appUpdateChecker
        if (checker) {
            if (checker.checking) return I18n.t("Проверка обновлений...")
            if (checker.installing) return I18n.t("Запуск установки...")
            if (checker.downloading) return I18n.t("Загружено %1 из %2", [formatBytes(checker.downloadReceivedBytes), formatBytes(checker.downloadTotalBytes)])
            if (checker.errorString !== "") return I18n.t("Ошибка") + ": " + checker.errorString
            if (checker.downloadedFilePath !== "") return I18n.t("Файл загружен. Готово к установке.")
            if (checker.hasUpdate) return I18n.t("Доступно обновление: %1", [checker.latestVersion])
        }

        switch (updateStatus) {
        case "checking": return I18n.t("Проверка обновлений...");
        case "available": return checker ? I18n.t("Доступно обновление: %1", [checker.latestVersion]) : I18n.t("Доступно обновление");
        case "latest": return I18n.t("Установлена последняя версия");
        case "error": return I18n.t("Ошибка") + (updateError !== "" ? ": " + updateError : "");
        default: return I18n.t("Нажмите \"Проверить обновления\"");
        }
    }

    function startUpdateCheck() {
        updateStatus = "checking";
        updateError = "";
        SystemController.appUpdateChecker.checkNow();
    }

    function formatBytes(bytes) {
        if (!bytes || bytes <= 0) return "—"
        if (bytes < 1024) return bytes + " B"
        var kib = bytes / 1024
        if (kib < 1024) return kib.toFixed(1) + " KiB"
        var mib = kib / 1024
        if (mib < 1024) return mib.toFixed(1) + " MiB"
        return (mib / 1024).toFixed(2) + " GiB"
    }

    function trimFileUrl(url) {
        if (!url)
            return "";
        return SystemController.normalizeLocalPath((typeof url === "string") ? url : url.toString());
    }

    function normalizePath(path) {
        if (!path)
            return "";
        return trimFileUrl(path);
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

    FolderDialog {
        id: evidenceSnapshotsDialog
        title: I18n.t("Папка снимков (детекции)")
        onAccepted: { evidenceSnapshotsPath = trimFileUrl(selectedFolder); applyCurrentSettings() }
    }

    FolderDialog {
        id: evidenceClipsDialog
        title: I18n.t("Папка клипов (детекции)")
        onAccepted: { evidenceClipsPath = trimFileUrl(selectedFolder); applyCurrentSettings() }
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

    Connections {
        target: SystemController.appUpdateChecker
        function onCheckingChanged() {
            if (SystemController.appUpdateChecker.checking) {
                updateStatus = "checking"
                updateError = ""
            }
        }
        function onCheckFinished(hasUpdate) {
            if (SystemController.appUpdateChecker.errorString !== "") {
                updateStatus = "error"
                updateError = SystemController.appUpdateChecker.errorString
            } else {
                updateStatus = hasUpdate ? "available" : "latest"
                updateError = ""
            }
        }
        function onDownloadFinished(success) {
            if (success && root.updateInstallAfterDownload) {
                root.updateInstallAfterDownload = false
                SystemController.appUpdateChecker.installDownloadedUpdate()
            }
        }
    }
    
    
    // Background Rectangle
    Rectangle {
        id: bgRect
        anchors.fill: parent
        color: Theme.appBackground
        border.color: Theme.controlBorder
        border.width: 1
        radius: Theme.radiusLg
        z: -1
    }
    
    // Custom Window Header
    Rectangle {
        id: titleBar
        height: 40
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: Theme.topBarBackground
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
                color: Theme.textPrimary
                font.bold: true
                Layout.fillWidth: true
            }
            
            Button {
                id: minimizeSettingsButton

                text: "—"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.showMinimized()
                contentItem: Text { text: "—"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: minimizeSettingsButton.down ? "#444" : (minimizeSettingsButton.hovered ? "#3e3e40" : "transparent") }
            }
            
            Button {
                id: maximizeSettingsButton

                text: "□"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: {
                    if (root.visibility === Window.Maximized) root.showNormal()
                    else root.showMaximized()
                }
                contentItem: Text { text: "□"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: maximizeSettingsButton.down ? "#444" : (maximizeSettingsButton.hovered ? "#3e3e40" : "transparent") }
            }

            Button {
                id: closeSettingsButton

                text: "✕"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.close()
                contentItem: Text { text: "✕"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: closeSettingsButton.down ? "#c42b1c" : (closeSettingsButton.hovered ? "#e81123" : "transparent") }
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
                        id: tabButton

                        text: modelData
                        Layout.preferredHeight: 35
                        Layout.preferredWidth: 100
                        
                        background: Rectangle {
                            color: bar.currentIndex === index ? "#2d3442" : "transparent"
                            radius: Theme.radiusSm
                            border.width: 1
                            border.color: bar.currentIndex === index ? Theme.accent : Theme.textFaint
                        }
                        
                        contentItem: Text {
                            text: tabButton.text
                            color: bar.currentIndex === index ? Theme.textPrimary : Theme.textMuted
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
                            StyledComboBox {
                                id: langCombo
                                model: ["English", "Русский"]
                                currentIndex: languageIndex(language)
                                Layout.fillWidth: true
                                Layout.preferredHeight: 32
                                onUserSelected: function(index) {
                                    language = languageFromIndex(index)
                                }
                                onCurrentIndexChanged: {
                                    if (currentIndex < 0) {
                                        return
                                    }
                                    var selectedLanguage = languageFromIndex(currentIndex)
                                    if (language !== selectedLanguage) {
                                        language = selectedLanguage
                                    }
                                }
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
                                        x: segmentSpin.width - width
                                        height: segmentSpin.height
                                        width: 30
                                        color: segmentSpin.up.pressed ? "#3e3e40" : "transparent"
                                        Text { text: "+"; color: "#a0aec0"; font.pixelSize: 18; anchors.centerIn: parent }
                                    }
                                    down.indicator: Rectangle {
                                        x: 0
                                        height: segmentSpin.height
                                        width: 30
                                        color: segmentSpin.down.pressed ? "#3e3e40" : "transparent"
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
                            Layout.preferredHeight: 1
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
                            Layout.preferredHeight: 1
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
                                    id: exportConfigButton
                                    text: I18n.t("Экспорт конфигурации")
                                    Layout.preferredHeight: 32
                                    Layout.preferredWidth: 190
                                    enabled: SystemController.userManager.canExport()
                                    background: Rectangle { color: "#3b82f6"; radius: 6 }
                                    contentItem: Text { text: exportConfigButton.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    onClicked: exportConfigDialog.open()
                                }
                                Button {
                                    id: importConfigButton
                                    text: I18n.t("Импорт конфигурации")
                                    Layout.preferredHeight: 32
                                    Layout.preferredWidth: 190
                                    enabled: SystemController.userManager.canExport()
                                    background: Rectangle { color: "#e53e3e"; radius: 6 }
                                    contentItem: Text { text: importConfigButton.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                    onClicked: importConfigDialog.open()
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
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

                                Text { text: I18n.t("Текущая версия"); color: "#a0aec0"; Layout.preferredWidth: updatesGrid.labelWidth }
                                Text {
                                    text: SystemController.appUpdateChecker.currentVersion
                                    color: "#e2e8f0"
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }

                                Text {
                                    visible: SystemController.appUpdateChecker.hasUpdate
                                    text: I18n.t("Новая версия")
                                    color: "#a0aec0"
                                    Layout.preferredWidth: updatesGrid.labelWidth
                                    Layout.preferredHeight: visible ? implicitHeight : 0
                                }
                                RowLayout {
                                    visible: SystemController.appUpdateChecker.hasUpdate
                                    spacing: 8
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: visible ? implicitHeight : 0

                                    Text {
                                        text: SystemController.appUpdateChecker.latestVersion
                                        color: Theme.success
                                        font.bold: true
                                        Layout.fillWidth: true
                                    }

                                    Rectangle {
                                        visible: SystemController.appUpdateChecker.latestPrerelease
                                        Layout.preferredWidth: prereleaseSettingsLabel.implicitWidth + 16
                                        Layout.preferredHeight: 22
                                        radius: 11
                                        color: Theme.warning

                                        Text {
                                            id: prereleaseSettingsLabel
                                            anchors.centerIn: parent
                                            text: I18n.t("Предварительный релиз")
                                            color: "black"
                                            font.pixelSize: 10
                                            font.bold: true
                                        }
                                    }
                                }

                                Text {
                                    visible: SystemController.appUpdateChecker.hasUpdate
                                    text: I18n.t("Файл обновления")
                                    color: "#a0aec0"
                                    Layout.preferredWidth: updatesGrid.labelWidth
                                    Layout.preferredHeight: visible ? implicitHeight : 0
                                }
                                Text {
                                    visible: SystemController.appUpdateChecker.hasUpdate
                                    text: SystemController.appUpdateChecker.assetName !== ""
                                          ? SystemController.appUpdateChecker.assetName
                                          : I18n.t("Для этой платформы нет подходящего файла обновления.")
                                    color: SystemController.appUpdateChecker.downloadAvailable ? "#e2e8f0" : Theme.warning
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: visible ? implicitHeight : 0
                                }

                                Text {
                                    visible: SystemController.appUpdateChecker.downloading || SystemController.appUpdateChecker.downloadedFilePath !== "" || SystemController.appUpdateChecker.installing
                                    text: I18n.t("Прогресс")
                                    color: "#a0aec0"
                                    Layout.preferredWidth: updatesGrid.labelWidth
                                    Layout.preferredHeight: visible ? implicitHeight : 0
                                }
                                ProgressBar {
                                    visible: SystemController.appUpdateChecker.downloading || SystemController.appUpdateChecker.downloadedFilePath !== "" || SystemController.appUpdateChecker.installing
                                    from: 0
                                    to: 100
                                    value: SystemController.appUpdateChecker.downloadProgress
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: visible ? 20 : 0
                                }

                                Text { text: I18n.t("Действие"); color: "#a0aec0"; Layout.preferredWidth: updatesGrid.labelWidth }
                                RowLayout {
                                    spacing: 8
                                    Layout.fillWidth: true

                                    Button {
                                        id: checkUpdatesButton
                                        text: SystemController.appUpdateChecker.checking
                                              ? I18n.t("Проверка...")
                                              : I18n.t("Проверить обновления")
                                        enabled: !SystemController.appUpdateChecker.checking
                                                 && !SystemController.appUpdateChecker.downloading
                                                 && !SystemController.appUpdateChecker.installing
                                        Layout.preferredHeight: 34
                                        Layout.preferredWidth: 190
                                        background: Rectangle { color: "#3b82f6"; radius: 6 }
                                        contentItem: Text { text: checkUpdatesButton.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                        onClicked: startUpdateCheck()
                                    }

                                    Button {
                                        id: openReleaseButton
                                        visible: SystemController.appUpdateChecker.hasUpdate
                                        text: I18n.t("Открыть релиз")
                                        Layout.preferredHeight: 34
                                        Layout.preferredWidth: 140
                                        enabled: !SystemController.appUpdateChecker.downloading
                                                 && !SystemController.appUpdateChecker.installing
                                        background: Rectangle { color: Theme.success; radius: 6 }
                                        contentItem: Text { text: openReleaseButton.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                        onClicked: SystemController.appUpdateChecker.openReleasePage()
                                    }

                                    Button {
                                        id: downloadUpdateButton
                                        visible: SystemController.appUpdateChecker.hasUpdate
                                        text: SystemController.appUpdateChecker.installing
                                              ? I18n.t("Запуск установки...")
                                              : SystemController.appUpdateChecker.downloadedFilePath !== ""
                                                ? I18n.t("Установить и перезапустить")
                                                : SystemController.appUpdateChecker.downloading
                                                  ? I18n.t("Отмена")
                                                  : I18n.t("Скачать и установить")
                                        Layout.preferredHeight: 34
                                        Layout.preferredWidth: 210
                                        enabled: !SystemController.appUpdateChecker.installing
                                                 && (SystemController.appUpdateChecker.downloadAvailable
                                                     || SystemController.appUpdateChecker.downloadedFilePath !== ""
                                                     || SystemController.appUpdateChecker.downloading)
                                        background: Rectangle { color: SystemController.appUpdateChecker.downloading ? "#475569" : Theme.accent; radius: 6 }
                                        contentItem: Text { text: downloadUpdateButton.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                        onClicked: {
                                            if (SystemController.appUpdateChecker.downloading) {
                                                root.updateInstallAfterDownload = false
                                                SystemController.appUpdateChecker.cancelDownload()
                                            } else if (SystemController.appUpdateChecker.downloadedFilePath !== "") {
                                                SystemController.appUpdateChecker.installDownloadedUpdate()
                                            } else {
                                                root.updateInstallAfterDownload = true
                                                SystemController.appUpdateChecker.downloadUpdate()
                                            }
                                        }
                                    }
                                }

                                // Error message
                                Text { text: updateStatus === "error" ? I18n.t("Ошибка") : ""; color: updateStatus === "error" ? "#e53e3e" : "transparent"; Layout.preferredWidth: updatesGrid.labelWidth }
                                Text { text: updateStatus === "error" ? updateError : ""; color: "#e53e3e"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
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
                            StyledComboBox {
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
                                onUserSelected: {
                                    playerBufferMode = index
                                }
                            }

                            Text {
                                text: I18n.t("Протокол RTSP")
                                color: "#a0aec0"
                                font.pixelSize: 14
                                Layout.preferredWidth: streamingGrid.labelWidth
                            }
                            StyledComboBox {
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
                                onUserSelected: {
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
                            StyledComboBox {
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
                                onUserSelected: {
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
                            StyledComboBox {
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
                                onUserSelected: {
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

                            Text {
                                text: I18n.t("Умный бюджет превью")
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
                                    checked: smartStreamBudget
                                    onToggled: smartStreamBudget = checked
                                    text: I18n.t("Ограничивать одновременные live-preview")
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }

                            Text {
                                text: I18n.t("Максимум активных превью")
                                color: smartStreamBudget ? "#a0aec0" : "#64748b"
                                font.pixelSize: 14
                                Layout.preferredWidth: streamingGrid.labelWidth
                            }
                            StyledSpinBox {
                                from: 1
                                to: 64
                                value: maxPreviewStreams
                                enabled: smartStreamBudget
                                Layout.preferredWidth: 120
                                onValueModified: maxPreviewStreams = value
                            }

                            Text {
                                text: I18n.t("Fullscreen, запись и аналитика не ограничиваются.")
                                color: "#7f8ea3"
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                                Layout.columnSpan: 2
                                Layout.fillWidth: true
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
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
                            Layout.preferredHeight: 1
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
                                StyledComboBox {
                                    id: imageTransformCombo
                                    model: ["0°", "90°", "180°", "270°"]
                                    currentIndex: {
                                        if (playerOrientation === 90) return 1
                                        if (playerOrientation === 180) return 2
                                        if (playerOrientation === 270) return 3
                                        return 0
                                    }
                                    onUserSelected: {
                                        if (index === 1) playerOrientation = 90
                                        else if (index === 2) playerOrientation = 180
                                        else if (index === 3) playerOrientation = 270
                                        else playerOrientation = 0
                                    }
                                    Layout.preferredWidth: 100
                                    Layout.preferredHeight: 32
                                    background: Rectangle { color: "#1f2733"; radius: 4; border.color: "#4a5568" }
                                    contentItem: Text {
                                        text: imageTransformCombo.displayText
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
                            Item { Layout.fillWidth: true; Layout.columnSpan: 2; Layout.preferredHeight: 10 }
                            Button {
                                id: resetImageSettingsButton
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
                                contentItem: Text { text: resetImageSettingsButton.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
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
                    ScrollView {
                        anchors.fill: parent
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
                            id: analyticsContent
                            width: parent.width - 40
                            spacing: 16

                            Text {
                                text: I18n.t("Производительность аналитики")
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }

                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Ограничьте частоту обработки и количество параллельных задач, чтобы камеры не перегружали CPU/GPU.")
                                color: "#94a3b8"
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: I18n.t("Пресет нагрузки")
                                        color: "#cbd5e1"
                                        Layout.preferredWidth: 220
                                    }

                                    Repeater {
                                        model: ["eco", "balanced", "max"]

                                        Button {
                                            id: analyticsPresetButton
                                            Layout.preferredWidth: 132
                                            Layout.preferredHeight: 34
                                            text: root.analyticsPresetLabel(modelData)
                                            hoverEnabled: true
                                            onClicked: root.applyAnalyticsPreset(modelData)

                                            background: Rectangle {
                                                color: analyticsPerformancePreset === modelData
                                                       ? Theme.accent
                                                       : (analyticsPresetButton.hovered ? Theme.cardHover : Theme.controlBackground)
                                                radius: Theme.radiusSm
                                                border.color: analyticsPerformancePreset === modelData ? Theme.accentHover : Theme.controlBorder
                                            }

                                            contentItem: Text {
                                                text: analyticsPresetButton.text
                                                color: Theme.textPrimary
                                                font.pixelSize: 12
                                                font.bold: analyticsPerformancePreset === modelData
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }

                                    Item { Layout.fillWidth: true }
                                }

                                Text {
                                    Layout.fillWidth: true
                                    Layout.leftMargin: 232
                                    text: I18n.t("Текущий режим: %1", [root.analyticsPresetLabel(analyticsPerformancePreset)])
                                          + " · "
                                          + root.analyticsPresetDescription(analyticsPerformancePreset)
                                    color: "#94a3b8"
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: I18n.t("FPS аналитики")
                                        color: "#cbd5e1"
                                        Layout.preferredWidth: 220
                                    }

                                    StyledSpinBox {
                                        from: 1
                                        to: 15
                                        value: analyticsTargetFps
                                        Layout.preferredWidth: 110
                                        onValueModified: {
                                            analyticsPerformancePreset = "custom"
                                            analyticsTargetFps = value
                                            applyCurrentSettings()
                                        }
                                    }

                                    Text {
                                        text: I18n.t("%1 кадр/с", [analyticsTargetFps])
                                        color: "#94a3b8"
                                        Layout.preferredWidth: 90
                                    }

                                    Item { Layout.fillWidth: true }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: I18n.t("Параллельные задачи")
                                        color: "#cbd5e1"
                                        Layout.preferredWidth: 220
                                    }

                                    StyledSpinBox {
                                        from: 1
                                        to: 8
                                        value: analyticsMaxParallelJobs
                                        Layout.preferredWidth: 110
                                        onValueModified: {
                                            analyticsPerformancePreset = "custom"
                                            analyticsMaxParallelJobs = value
                                            applyCurrentSettings()
                                        }
                                    }

                                    Text {
                                        text: I18n.t("до %1 задач", [analyticsMaxParallelJobs])
                                        color: "#94a3b8"
                                        Layout.preferredWidth: 120
                                    }

                                    Item { Layout.fillWidth: true }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                                color: "#3b4657"
                            }

                            Text {
                                text: I18n.t("События аналитики")
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }

                            StyledCheckBox {
                                text: I18n.t("Включить события")
                                checked: evidenceEnabled
                                onToggled: { evidenceEnabled = checked; applyCurrentSettings() }
                            }

                            RowLayout {
                                spacing: 12
                                StyledCheckBox {
                                    text: I18n.t("Снимки")
                                    checked: evidenceSnapshotsEnabled
                                    enabled: evidenceEnabled
                                    onToggled: { evidenceSnapshotsEnabled = checked; applyCurrentSettings() }
                                }
                                StyledCheckBox {
                                    text: I18n.t("Клипы")
                                    checked: evidenceClipsEnabled
                                    enabled: evidenceEnabled
                                    onToggled: { evidenceClipsEnabled = checked; applyCurrentSettings() }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                Text { text: I18n.t("Папка снимков (детекции)"); color: "#cbd5e1"; Layout.preferredWidth: 220 }
                                TextField {
                                    Layout.fillWidth: true
                                    text: evidenceSnapshotsPath
                                    color: "#f8fafc"
                                    placeholderText: I18n.t("Выберите папку")
                                    placeholderTextColor: "#94a3b8"
                                    selectionColor: "#3b82f6"
                                    selectedTextColor: "white"
                                    background: Rectangle {
                                        color: "#1f2733"
                                        radius: 4
                                        border.color: "#4a5568"
                                        border.width: 1
                                    }
                                    onEditingFinished: { evidenceSnapshotsPath = text; applyCurrentSettings() }
                                }
                                Button {
                                    Layout.preferredHeight: 30
                                    Layout.preferredWidth: 34
                                    ToolTip.visible: hovered
                                    ToolTip.text: I18n.t("Выберите папку")
                                    background: Rectangle { color: "#4a5568"; radius: 4 }
                                    contentItem: Text {
                                        text: "folder_open"
                                        font.family: iconFontFamily
                                        font.pixelSize: 15
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: evidenceSnapshotsDialog.open()
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                Text { text: I18n.t("Папка клипов (детекции)"); color: "#cbd5e1"; Layout.preferredWidth: 220 }
                                TextField {
                                    Layout.fillWidth: true
                                    text: evidenceClipsPath
                                    color: "#f8fafc"
                                    placeholderText: I18n.t("Выберите папку")
                                    placeholderTextColor: "#94a3b8"
                                    selectionColor: "#3b82f6"
                                    selectedTextColor: "white"
                                    background: Rectangle {
                                        color: "#1f2733"
                                        radius: 4
                                        border.color: "#4a5568"
                                        border.width: 1
                                    }
                                    onEditingFinished: { evidenceClipsPath = text; applyCurrentSettings() }
                                }
                                Button {
                                    Layout.preferredHeight: 30
                                    Layout.preferredWidth: 34
                                    ToolTip.visible: hovered
                                    ToolTip.text: I18n.t("Выберите папку")
                                    background: Rectangle { color: "#4a5568"; radius: 4 }
                                    contentItem: Text {
                                        text: "folder_open"
                                        font.family: iconFontFamily
                                        font.pixelSize: 15
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: evidenceClipsDialog.open()
                                }
                            }

                            RowLayout {
                                spacing: 12
                                Text { text: I18n.t("До события (сек)"); color: "#cbd5e1"; Layout.preferredWidth: 180 }
                                StyledSpinBox {
                                    from: 0; to: 30
                                    value: evidencePreSeconds
                                    Layout.preferredWidth: 110
                                    onValueModified: { evidencePreSeconds = value; applyCurrentSettings() }
                                }
                                Text { text: I18n.t("После события (сек)"); color: "#cbd5e1"; Layout.preferredWidth: 180 }
                                StyledSpinBox {
                                    from: 0; to: 30
                                    value: evidencePostSeconds
                                    Layout.preferredWidth: 110
                                    onValueModified: { evidencePostSeconds = value; applyCurrentSettings() }
                                }
                            }

                            RowLayout {
                                spacing: 12
                                Text { text: I18n.t("Минимальная уверенность"); color: "#cbd5e1"; Layout.preferredWidth: 220 }
                                Slider {
                                    id: evidenceMinConfidenceSlider
                                    Layout.fillWidth: true
                                    from: 0.1; to: 0.95
                                    value: evidenceMinConfidence
                                    onMoved: evidenceMinConfidence = value
                                    onPressedChanged: if (!pressed) applyCurrentSettings()
                                    background: Rectangle {
                                        x: evidenceMinConfidenceSlider.leftPadding
                                        y: evidenceMinConfidenceSlider.topPadding + evidenceMinConfidenceSlider.availableHeight / 2 - height / 2
                                        width: evidenceMinConfidenceSlider.availableWidth
                                        height: 4
                                        radius: 2
                                        color: "#334155"

                                        Rectangle {
                                            width: parent.width * ((evidenceMinConfidenceSlider.value - evidenceMinConfidenceSlider.from) / (evidenceMinConfidenceSlider.to - evidenceMinConfidenceSlider.from))
                                            height: parent.height
                                            radius: 2
                                            color: "#3b82f6"
                                        }
                                    }
                                    handle: Rectangle {
                                        x: evidenceMinConfidenceSlider.leftPadding + (evidenceMinConfidenceSlider.availableWidth - width) * ((evidenceMinConfidenceSlider.value - evidenceMinConfidenceSlider.from) / (evidenceMinConfidenceSlider.to - evidenceMinConfidenceSlider.from))
                                        y: evidenceMinConfidenceSlider.topPadding + evidenceMinConfidenceSlider.availableHeight / 2 - height / 2
                                        width: 14
                                        height: 14
                                        radius: 7
                                        color: evidenceMinConfidenceSlider.pressed ? "#60a5fa" : "#3b82f6"
                                        border.width: 1
                                        border.color: "#93c5fd"
                                    }
                                }
                                Text { text: Math.round(evidenceMinConfidence * 100) + "%"; color: "#cbd5e1"; Layout.preferredWidth: 50 }
                            }

                            RowLayout {
                                spacing: 12
                                Text { text: I18n.t("FPS клипа"); color: "#cbd5e1"; Layout.preferredWidth: 220 }
                                StyledSpinBox {
                                    from: 5; to: 25
                                    value: evidenceClipFps
                                    Layout.preferredWidth: 110
                                    onValueModified: { evidenceClipFps = value; applyCurrentSettings() }
                                }
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }
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
                            text: "Version " + (AppVersion ? AppVersion : "Unknown")
                            color: "#a0aec0"
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Text { 
                            text: "Author: " + (AppAuthor ? AppAuthor : "Rinat Ibragimov")
                            color: "#a0aec0"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text { 
                            text: (AppBuildYear ? AppBuildYear : "2026")
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

    Dialog {
        id: oauthDialog
        modal: true
        dim: true
        width: parent.width * 0.6
        height: parent.height * 0.3
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        onRejected: SystemController.analyticsEngine.cancelOAuth()

        background: Rectangle { color: Theme.panelAltBackground; radius: Theme.radiusLg }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            Text {
                text: I18n.t("Авторизация открыта в браузере. Завершите вход и вернитесь в приложение.")
                color: Theme.textPrimary
                wrapMode: Text.Wrap
            }

            Text {
                text: "<a href=\"" + oauthUrl + "\">" + oauthUrl + "</a>"
                color: Theme.textMuted
                wrapMode: Text.Wrap
                font.pixelSize: 12
                textFormat: Text.RichText
                onLinkActivated: Qt.openUrlExternally(link)
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                Button {
                    text: I18n.t("Открыть браузер")
                    onClicked: Qt.openUrlExternally(oauthUrl)
                }
                Button {
                    text: I18n.t("Отмена")
                    onClicked: { SystemController.analyticsEngine.cancelOAuth(); oauthDialog.close(); }
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
        visible: bar.currentIndex !== 3 // Hide on About tab
        z: 10
        
        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: Theme.controlBorder
        }

        Button {
            id: saveSettingsButton
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 20
            width: 120
            height: 36
            text: I18n.t("Сохранить")
            
            background: Rectangle {
                color: Theme.accent
                radius: Theme.radiusSm
            }
            contentItem: Text {
                text: saveSettingsButton.text
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
            radius: Theme.radiusSm
            border.color: Theme.accent
            border.width: 1
        }
        
        contentItem: RowLayout {
            anchors.centerIn: parent
            spacing: 8
            Text {
                text: "✓"
                color: Theme.accent
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
