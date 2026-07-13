import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Window {
    id: root
    title: I18n.t("Настройки")
    width: 760
    height: 680
    minimumWidth: 560
    minimumHeight: 480
    visible: false
    color: Theme.appBackground
    flags: Qt.Window | Qt.FramelessWindowHint
    modality: Qt.ApplicationModal
    readonly property bool contentLayoutReady: mainLayout.width > 0
                                               && mainLayout.height > 0
                                               && contentPanel.width > 0
                                               && contentPanel.height > 0
                                               && settingsStack.width > 0
                                               && settingsStack.height > 0
                                               && settingsStack.count === tabLabels.length

    // Make it non-modal but behave like a standalone window
    function open() {
        show()
        requestActivate()
    }

    function close() {
        hide()
    }

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: root.close()
    }

    Shortcut {
        sequence: StandardKey.Save
        onActivated: {
            root.applyCurrentSettings()
            root.close()
        }
    }

    component StyledCheckBox: MetroCheckBox {
    }

    component StyledSpinBox: SpinBox {
        id: spinRoot
        editable: true
        implicitHeight: 32
        implicitWidth: 120
        focusPolicy: Qt.StrongFocus

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
    property bool evidenceEnabled: true
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

    function normalizeRecordingSegmentDuration(value) {
        var minutes = Number(value)
        if (!isFinite(minutes)) minutes = 15
        minutes = Math.round(minutes / 5) * 5
        return Math.max(5, Math.min(60, minutes))
    }

    // Helper to apply current settings
    function applyCurrentSettings() {
        recordingSegmentDuration = normalizeRecordingSegmentDuration(recordingSegmentDuration)
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
            if (settings.recordingSegmentDuration !== undefined) {
                recordingSegmentDuration = normalizeRecordingSegmentDuration(settings.recordingSegmentDuration)
            }
            
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
            
            MetroWindowButton {
                kind: "minimize"

                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.showMinimized()
            }
            
            MetroWindowButton {
                kind: "maximize"
                maximized: root.visibility === Window.Maximized

                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: {
                    if (root.visibility === Window.Maximized) root.showNormal()
                    else root.showMaximized()
                }
            }

            MetroWindowButton {
                kind: "close"

                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.close()
            }
        }
    }

    // Main Content
    ColumnLayout {
        id: mainLayout

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
                        focusPolicy: Qt.StrongFocus
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredHeight: 35
                        
                        background: Rectangle {
                            color: bar.currentIndex === index ? Theme.metroTile : "transparent"
                            radius: Theme.radiusSm
                            border.width: tabButton.visualFocus ? 2 : 1
                            border.color: tabButton.visualFocus || bar.currentIndex === index
                                          ? Theme.accent
                                          : Theme.textFaint
                        }
                        
                        contentItem: Text {
                            text: tabButton.text
                            color: bar.currentIndex === index ? Theme.textPrimary : Theme.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                            elide: Text.ElideRight
                        }
                        
                        onClicked: bar.currentIndex = index
                    }
                }
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
            id: contentPanel

            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            
            StackLayout {
                id: settingsStack

                anchors.fill: parent
                // margins: 0 to allow scrollbar to hit the edge
                currentIndex: bar.currentIndex
                
                // -------------------------------------------------
                // General Tab
                // -------------------------------------------------
                ScrollView {
                    id: generalScroll

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
                        width: generalScroll.availableWidth
                        spacing: 14

                        Text {
                            text: I18n.t("Приложение")
                            color: Theme.textPrimary
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
                                color: Theme.textMuted
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
                                background: Rectangle { color: Theme.metroSurfaceAlt; radius: 4; border.color: Theme.metroStroke }
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
                                        ctx.fillStyle = Theme.textMuted;
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
                                color: Theme.textMuted
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
                                    background: Rectangle { color: Theme.metroSurfaceAlt; border.color: Theme.metroStroke; radius: 4 }
                                }
                                Button {
                                    id: recordingsFolderButton

                                    Layout.preferredHeight: 30
                                    Layout.preferredWidth: 34
                                    focusPolicy: Qt.StrongFocus
                                    background: Rectangle {
                                        color: recordingsFolderButton.hovered ? Theme.metroTileHover : Theme.metroStroke
                                        border.color: recordingsFolderButton.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                                        border.width: recordingsFolderButton.visualFocus ? 2 : 1
                                        radius: Theme.metroTileRadius
                                    }
                                    contentItem: Text {
                                        text: "folder_open"
                                        font.family: iconFontFamily
                                        font.pixelSize: 15
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    ToolTip.visible: hovered || visualFocus
                                    ToolTip.text: I18n.t("Выберите папку")
                                    ToolTip.delay: 450
                                    onClicked: recordingsDialog.open()
                                }
                            }

                            Text {
                                text: I18n.t("Папка снимков")
                                color: Theme.textMuted
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
                                    background: Rectangle { color: Theme.metroSurfaceAlt; border.color: Theme.metroStroke; radius: 4 }
                                }
                                Button {
                                    id: screenshotsFolderButton

                                    Layout.preferredHeight: 30
                                    Layout.preferredWidth: 34
                                    focusPolicy: Qt.StrongFocus
                                    background: Rectangle {
                                        color: screenshotsFolderButton.hovered ? Theme.metroTileHover : Theme.metroStroke
                                        border.color: screenshotsFolderButton.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                                        border.width: screenshotsFolderButton.visualFocus ? 2 : 1
                                        radius: Theme.metroTileRadius
                                    }
                                    contentItem: Text {
                                        text: "folder_open"
                                        font.family: iconFontFamily
                                        font.pixelSize: 15
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    ToolTip.visible: hovered || visualFocus
                                    ToolTip.text: I18n.t("Выберите папку")
                                    ToolTip.delay: 450
                                    onClicked: screenshotsDialog.open()
                                }
                            }

                            Text {
                                text: I18n.t("Длительность записи (мин)")
                                color: Theme.textMuted
                                font.pixelSize: 14
                                Layout.preferredWidth: generalGrid.labelWidth
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                
                                StyledSpinBox {
                                    id: segmentSpin
                                    from: 5
                                    to: 60
                                    stepSize: 5
                                    value: normalizeRecordingSegmentDuration(recordingSegmentDuration)
                                    onValueModified: recordingSegmentDuration = normalizeRecordingSegmentDuration(value)
                                    Layout.preferredHeight: 32
                                    Layout.preferredWidth: 120
                                }
                                
                                Text {
                                    text: I18n.t("5-60 мин")
                                    color: Theme.textFaint
                                    font.pixelSize: 12
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: Theme.metroStroke
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
                                color: Theme.textMuted
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
                            color: Theme.metroStroke
                        }

                        ColumnLayout {
                            spacing: 12
                            Text {
                                text: I18n.t("Управление конфигурацией")
                                color: Theme.textPrimary
                                font.pixelSize: 16
                                font.bold: true
                            }
                            RowLayout {
                                spacing: 10
                                Button {
                                    id: exportConfigButton
                                    text: I18n.t("Экспорт конфигурации")
                                    focusPolicy: Qt.StrongFocus
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredHeight: 32
                                    enabled: SystemController.userManager.canExport()
                                    background: Rectangle {
                                        color: exportConfigButton.enabled ? Theme.metroBlue : Theme.metroTileDisabled
                                        border.color: exportConfigButton.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                                        border.width: exportConfigButton.visualFocus ? 2 : 1
                                        radius: Theme.metroTileRadius
                                    }
                                    contentItem: Text {
                                        text: exportConfigButton.text
                                        color: exportConfigButton.enabled ? Theme.textPrimary : Theme.textFaint
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }
                                    ToolTip.visible: hovered && !enabled
                                    ToolTip.text: I18n.t("Недостаточно прав")
                                    ToolTip.delay: 450
                                    onClicked: exportConfigDialog.open()
                                }
                                Button {
                                    id: importConfigButton
                                    text: I18n.t("Импорт конфигурации")
                                    focusPolicy: Qt.StrongFocus
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredHeight: 32
                                    enabled: SystemController.userManager.canExport()
                                    background: Rectangle {
                                        color: importConfigButton.enabled ? Theme.metroRed : Theme.metroTileDisabled
                                        border.color: importConfigButton.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                                        border.width: importConfigButton.visualFocus ? 2 : 1
                                        radius: Theme.metroTileRadius
                                    }
                                    contentItem: Text {
                                        text: importConfigButton.text
                                        color: importConfigButton.enabled ? Theme.textPrimary : Theme.textFaint
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }
                                    ToolTip.visible: hovered && !enabled
                                    ToolTip.text: I18n.t("Недостаточно прав")
                                    ToolTip.delay: 450
                                    onClicked: importConfigDialog.open()
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: Theme.metroStroke
                        }

                        SettingsUpdatePanel {
                            settings: root
                            updateChecker: SystemController.appUpdateChecker
                        }
                    }
                }
                
                // -------------------------------------------------
                // Streaming Tab
                // -------------------------------------------------
                SettingsStreamingPage {
                    settings: root
                }

                // -------------------------------------------------
                // Analytics Tab
                // -------------------------------------------------
                Item {
                    ScrollView {
                        id: analyticsScroll

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
                            width: analyticsScroll.availableWidth
                            spacing: 16

                            SettingsAnalyticsPerformancePanel {
                                settings: root
                            }

                            SettingsEvidencePanel {
                                settings: root
                                iconFontFamily: root.iconFontFamily
                                onSnapshotsFolderRequested: evidenceSnapshotsDialog.open()
                                onClipsFolderRequested: evidenceClipsDialog.open()
                            }
                        }
                    }
                }

                // -------------------------------------------------
                // About Tab
                // -------------------------------------------------
                SettingsAboutPage {
                    appVersion: AppVersion ? AppVersion : "Unknown"
                    appAuthor: AppAuthor ? AppAuthor : "Rinat Ibragimov"
                    appBuildYear: AppBuildYear ? AppBuildYear : "2026"
                }
            }
        }
    }

    SettingsOAuthDialog {
        id: oauthDialog
        analyticsEngine: SystemController.analyticsEngine
        oauthUrl: root.oauthUrl
    }

    SettingsFooterBar {
        id: footerBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: bar.currentIndex !== 3

        onSaveRequested: {
            applyCurrentSettings()
            root.close()
        }
    }
    
    SettingsSaveNotification {
        id: saveNotification
    }
}
