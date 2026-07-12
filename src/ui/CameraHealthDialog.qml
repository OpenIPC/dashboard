import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root

    modal: true
    width: Math.min(parent ? parent.width - 80 : 1100, 1120)
    height: Math.min(parent ? parent.height - 80 : 720, 760)
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    leftPadding: 16
    rightPadding: 16
    topPadding: healthHeader.height + 16
    bottomPadding: 16

    property int dataVersion: 0
    readonly property var healthController: SystemController.cameraHealthController
    readonly property int diagnosticProfileCount: healthController
                                                  ? healthController.profiles.length : 0
    readonly property bool layoutReady: healthStatsRow.width > 0
                                        && healthStatsRow.height >= 60
                                        && totalStatCard.height >= 60
                                        && healthProfileBar.height >= 50
                                        && root.statsTopInDialog >= root.headerBottomInDialog
    property string filterText: ""
    property string filterMode: "all"
    property string selectedProfile: "quick"
    property string selectedRunId: ""
    property var detailsResult: ({})
    property bool autoRefreshOnOpen: true
    property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"
    property bool reportCopied: false
    property bool reportSaved: false
    property bool reportSaveFailed: false
    property string lastReportPath: ""
    property double lastAutoRefreshMs: 0
    readonly property int autoRefreshCooldownMs: 15000
    readonly property real statsTopInDialog: healthStatsRow.mapToItem(root.parent, 0, 0).y
    readonly property real headerHeight: healthHeader.height
    readonly property real headerBottomInDialog: healthHeader.mapToItem(root.parent, 0, 0).y + healthHeader.height

    signal editRequested(int cameraIndex)
    signal majesticRequested(int cameraIndex)
    signal addToGridRequested(int cameraIndex)

    onOpened: {
        root.reportCopied = false
        var now = Date.now()
        if (root.autoRefreshOnOpen && now - root.lastAutoRefreshMs >= root.autoRefreshCooldownMs) {
            root.lastAutoRefreshMs = now
            root.recheckAll()
        }
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    function isOnlineStatus(statusText) {
        if (typeof SystemController.isCameraOnline === "function")
            return SystemController.isCameraOnline("", statusText || "")
        return String(statusText || "").toLowerCase() === "online"
    }

    function effectiveCameraStatus(ip, fallbackStatus) {
        var version = dataVersion
        if (!ip || ip === "") return fallbackStatus || ""
        if (typeof SystemController.effectiveCameraStatus === "function")
            return SystemController.effectiveCameraStatus(ip, fallbackStatus || "")
        return fallbackStatus || ""
    }

    function latestHealthResult(ip) {
        var version = dataVersion
        if (!ip || ip === "" || !root.healthController)
            return ({})
        return root.healthController.resultForCamera(ip)
    }

    function cameraDetail(ip) {
        var version = dataVersion
        if (!ip || ip === "" || typeof SystemController.cameraStatusDetail !== "function") return ""
        return SystemController.cameraStatusDetail(ip) || ""
    }

    function cameraAttentionReason(ip, fallbackStatus) {
        var version = dataVersion
        if (!ip || ip === "") return ""
        var healthResult = root.latestHealthResult(ip)
        if (healthResult && healthResult.recommendation)
            return String(healthResult.recommendation)
        if (typeof SystemController.cameraAttentionReason === "function")
            return SystemController.cameraAttentionReason(ip, fallbackStatus || "")
        return cameraDetail(ip)
    }

    function cameraStatusSearchText(ip, fallbackStatus) {
        var version = dataVersion
        if (!ip || ip === "") return fallbackStatus || ""
        if (typeof SystemController.cameraStatusSearchText === "function")
            return SystemController.cameraStatusSearchText(ip, fallbackStatus || "")
        return effectiveCameraStatus(ip, fallbackStatus) + " " + cameraAttentionReason(ip, fallbackStatus)
    }

    function isCameraInGrid(ip) {
        var version = dataVersion
        if (!ip || ip === "") return false
        if (typeof SystemController.isCameraInGrid === "function")
            return SystemController.isCameraInGrid(ip)
        return false
    }

    function onlineCount() {
        var version = dataVersion
        var count = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var camera = SystemController.cameraModel.getCamera(i)
            if (!camera) continue
            var healthResult = root.latestHealthResult(camera.cameraIp)
            var healthKnown = healthResult.status === "ok"
                    || healthResult.status === "warning"
                    || healthResult.status === "error"
            if (healthResult.status === "ok" || healthResult.status === "warning"
                    || (!healthKnown
                        && root.isOnlineStatus(root.effectiveCameraStatus(
                                                   camera.cameraIp, camera.status)))) {
                count++
            }
        }
        return count
    }

    function issueCount() {
        var version = dataVersion
        var count = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var camera = SystemController.cameraModel.getCamera(i)
            if (!camera) continue
            var healthResult = root.latestHealthResult(camera.cameraIp)
            var healthKnown = healthResult.status === "ok"
                    || healthResult.status === "warning"
                    || healthResult.status === "error"
            if (healthResult.status === "error" || healthResult.status === "warning"
                    || (!healthKnown
                        && typeof SystemController.cameraNeedsAttention === "function"
                        && SystemController.cameraNeedsAttention(
                            camera.cameraIp, camera.status))) {
                count++
            }
        }
        return count
    }

    function checkHistoryLabel() {
        if (!root.healthController || root.healthController.history.length === 0)
            return I18n.t("История проверок пуста")
        var run = root.healthController.history[0]
        return I18n.t("Последняя проверка") + ": "
                + String(run.completedAtLabel || run.completedAt || "")
    }

    function issueKind(ip, fallbackStatus) {
        var healthResult = root.latestHealthResult(ip)
        if (healthResult.status === "error")
            return "offline"
        if (healthResult.status === "warning")
            return "warning"
        if (healthResult.status === "ok")
            return "ok"
        if (healthResult.status === "running")
            return "checking"
        var statusText = root.effectiveCameraStatus(ip, fallbackStatus)
        var detailText = root.cameraAttentionReason(ip, fallbackStatus)
        var text = (statusText + " " + detailText).toLowerCase()

        if (detailText.indexOf("Проверка") === 0 || text.indexOf("checking") >= 0)
            return "checking"
        if (text.indexOf("auth") >= 0 || text.indexOf("401") >= 0 || text.indexOf("403") >= 0
                || text.indexOf("парол") >= 0 || text.indexOf("автор") >= 0)
            return "auth"
        if (text.indexOf("rtsp") >= 0 || text.indexOf("stream") >= 0 || text.indexOf("frame") >= 0
                || text.indexOf("поток") >= 0 || text.indexOf("кадр") >= 0 || text.indexOf("stalled") >= 0)
            return "stream"
        if (!root.isOnlineStatus(statusText))
            return "offline"
        if (detailText !== "")
            return "warning"
        return "ok"
    }

    function filterMatchesForMode(mode, ip, fallbackStatus) {
        var version = dataVersion
        var statusText = root.effectiveCameraStatus(ip, fallbackStatus)
        var healthResult = root.latestHealthResult(ip)
        var healthOnline = healthResult.status === "ok" || healthResult.status === "warning"
        var healthKnown = healthOnline || healthResult.status === "error"
        var online = healthKnown ? healthOnline : root.isOnlineStatus(statusText)
        var kind = root.issueKind(ip, fallbackStatus)
        if (mode === "all") return true
        if (mode === "issues") return kind !== "ok"
        if (mode === "offline") return !online
        if (mode === "online") return online
        if (mode === "inGrid") return root.isCameraInGrid(ip)
        if (mode === "notInGrid") return !root.isCameraInGrid(ip)
        if (mode === "auth") return kind === "auth"
        if (mode === "stream") return kind === "stream"
        return true
    }

    function filterMatches(ip, fallbackStatus) {
        return root.filterMatchesForMode(root.filterMode, ip, fallbackStatus)
    }

    function countByFilter(mode) {
        var version = dataVersion
        var count = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var cam = SystemController.cameraModel.getCamera(i)
            if (!cam) continue
            if (root.filterMatchesForMode(mode, cam.cameraIp, cam.status))
                count++
        }
        return count
    }

    function rowMatches(name, ip, fallbackStatus, searchText, detailText) {
        if (!root.filterMatches(ip, fallbackStatus))
            return false
        var query = filterText.trim().toLowerCase()
        if (query === "") return true
        return [name || "", ip || "", searchText || "", detailText || ""].join(" ").toLowerCase().indexOf(query) !== -1
    }

    function recheckAll() {
        if (root.healthController)
            root.healthController.runAll(root.selectedProfile)
    }

    function recheckCamera(ip, name) {
        if (!ip || ip === "")
            return
        if (root.healthController)
            root.healthController.runCamera(ip, root.selectedProfile)
    }

    function healthReport() {
        return root.healthController
                ? root.healthController.reportText(root.selectedRunId)
                : ""
    }

    function copyHealthReport() {
        SystemController.copyTextToClipboard(root.healthReport())
        root.reportCopied = true
        copyReportFeedbackTimer.restart()
    }

    function openExportDialog() {
        var stamp = Qt.formatDateTime(new Date(), "yyyyMMdd-HHmmss")
        healthReportDialog.currentFile = root.localFileUrl(SystemController.normalizeLocalPath("~/health-report-" + stamp + ".txt"))
        healthReportDialog.open()
    }

    function localFileUrl(path) {
        var normalized = String(path || "").replace(/\\/g, "/")
        if (normalized.charAt(0) === "/")
            return "file://" + normalized
        return "file:///" + normalized
    }

    function saveHealthReport(pathOrUrl) {
        root.reportSaved = false
        root.reportSaveFailed = false
        var ok = SystemController.saveTextFile(String(pathOrUrl), root.healthReport())
        if (ok) {
            root.lastReportPath = String(pathOrUrl)
            root.reportSaved = true
        } else {
            root.reportSaveFailed = true
        }
        saveReportFeedbackTimer.restart()
    }

    Timer {
        id: copyReportFeedbackTimer
        interval: 1800
        repeat: false
        onTriggered: root.reportCopied = false
    }

    Timer {
        id: saveReportFeedbackTimer
        interval: 2200
        repeat: false
        onTriggered: {
            root.reportSaved = false
            root.reportSaveFailed = false
        }
    }

    component StatCard: Rectangle {
        id: statCard
        property string title: ""
        property string value: "0"
        property color accent: Theme.accent

        Layout.fillWidth: true
        implicitHeight: 64
        Layout.preferredHeight: 64
        Layout.minimumHeight: 64
        radius: Theme.radiusMd
        color: Theme.panelSoftBackground
        border.color: Theme.panelBorder

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 2
            Text {
                text: statCard.value
                color: statCard.accent
                font.pixelSize: 22
                font.bold: true
            }
            Text {
                text: I18n.t(statCard.title)
                color: Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
    }

    component FilterChip: Button {
        id: chip
        property string mode: "all"
        property string label: ""
        property int count: 0
        checkable: true
        checked: root.filterMode === mode
        text: I18n.t(label) + " " + count
        Layout.preferredHeight: 30
        onClicked: root.filterMode = mode
        background: Rectangle {
            radius: 15
            color: chip.checked ? Theme.accent : (chip.hovered ? Theme.cardHover : Theme.controlBackground)
            border.color: chip.checked ? Theme.accentHover : Theme.controlBorder
        }
        contentItem: Text {
            text: chip.text
            color: chip.checked ? Theme.textPrimary : Theme.textSecondary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 11
            font.bold: chip.checked
            leftPadding: 12
            rightPadding: 12
        }
    }

    Connections {
        target: SystemController.cameraModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.dataVersion++ }
        function onRowsRemoved(parent, first, last) { root.dataVersion++ }
        function onModelReset() { root.dataVersion++ }
        function onDataChanged(topLeft, bottomRight, roles) { root.dataVersion++ }
    }

    Connections {
        target: SystemController.gridModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.dataVersion++ }
        function onRowsRemoved(parent, first, last) { root.dataVersion++ }
        function onModelReset() { root.dataVersion++ }
        function onDataChanged(topLeft, bottomRight, roles) { root.dataVersion++ }
    }

    Connections {
        target: SystemController
        ignoreUnknownSignals: true
        function onCameraStatusDetailsChanged() { root.dataVersion++ }
    }

    Connections {
        target: root.healthController
        ignoreUnknownSignals: true
        function onCurrentResultsChanged() { root.dataVersion++ }
        function onHistoryChanged() { root.dataVersion++ }
        function onRunCompleted(runId) {
            root.selectedRunId = runId
            root.dataVersion++
        }
    }

    background: Rectangle {
        color: Theme.panelBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorderStrong
        border.width: 1
    }

    header: Rectangle {
        id: healthHeader

        height: 70
        color: Theme.topBarBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorderStrong
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 14
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                radius: 20
                color: Theme.controlBackground
                border.color: Theme.controlBorder

                Text {
                    anchors.centerIn: parent
                    text: "health_and_safety"
                    color: Theme.accentHover
                    font.family: root.iconFontFamily
                    font.pixelSize: 22
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: I18n.t("Здоровье камер")
                    color: Theme.textPrimary
                    font.pixelSize: 20
                    font.bold: true
                }
                Text {
                    text: I18n.t("Статусы, причины проблем и быстрые проверки RTSP/Majestic")
                    color: Theme.textMuted
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            Button {
                id: checkAllButton
                text: root.healthController && root.healthController.running
                      ? (I18n.language === "ru" ? "Проверка "
                         : "Checking ")
                        + root.healthController.completedProbes + "/"
                        + root.healthController.totalProbes
                      : I18n.t("Проверить все")
                Layout.preferredHeight: 34
                enabled: !root.healthController || !root.healthController.running
                onClicked: root.recheckAll()
                background: Rectangle {
                    color: checkAllButton.down ? Theme.accent : Theme.controlBackground
                    radius: Theme.radiusMd
                    border.color: Theme.accent
                }
                contentItem: Text {
                    text: checkAllButton.text
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            Button {
                id: copyReportButton
                text: root.reportCopied ? I18n.t("Скопировано") : I18n.t("Скопировать отчет")
                Layout.preferredHeight: 34
                onClicked: root.copyHealthReport()
                background: Rectangle {
                    color: copyReportButton.down ? Theme.accent : Theme.controlBackground
                    radius: Theme.radiusMd
                    border.color: root.reportCopied ? Theme.success : Theme.controlBorderStrong
                }
                contentItem: Text {
                    text: copyReportButton.text
                    color: root.reportCopied ? Theme.success : Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    font.bold: true
                    leftPadding: 10
                    rightPadding: 10
                }
            }

            Button {
                id: exportReportButton
                text: root.reportSaved ? I18n.t("Сохранено") : (root.reportSaveFailed ? I18n.t("Ошибка сохранения") : I18n.t("Экспорт"))
                Layout.preferredHeight: 34
                onClicked: root.openExportDialog()
                background: Rectangle {
                    color: exportReportButton.down ? Theme.accent : Theme.controlBackground
                    radius: Theme.radiusMd
                    border.color: root.reportSaved ? Theme.success : (root.reportSaveFailed ? Theme.danger : Theme.controlBorderStrong)
                }
                contentItem: Text {
                    text: exportReportButton.text
                    color: root.reportSaved ? Theme.success : (root.reportSaveFailed ? Theme.danger : Theme.textPrimary)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    font.bold: true
                    leftPadding: 10
                    rightPadding: 10
                }
            }

            MetroWindowButton {
                id: closeButton
                kind: "close"
                Layout.preferredWidth: 38
                Layout.preferredHeight: 34
                onClicked: root.close()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            id: healthStatsRow

            Layout.fillWidth: true
            Layout.preferredHeight: 64
            Layout.minimumHeight: 64
            spacing: 10

            StatCard {
                id: totalStatCard

                title: "Всего"
                value: String(SystemController.cameraModel.rowCount())
                accent: Theme.textPrimary
            }
            StatCard {
                title: "Онлайн"
                value: String(root.onlineCount())
                accent: Theme.success
            }
            StatCard {
                title: "Офлайн"
                value: String(Math.max(0, SystemController.cameraModel.rowCount() - root.onlineCount()))
                accent: Theme.danger
            }
            StatCard {
                title: "Требуют внимания"
                value: String(root.issueCount())
                accent: root.issueCount() > 0 ? Theme.warning : Theme.success
            }
        }

        HealthProfileBar {
            id: healthProfileBar

            Layout.fillWidth: true
            Layout.preferredHeight: 58
            controller: root.healthController
            selectedProfile: root.selectedProfile
            onProfileSelected: profileId => root.selectedProfile = profileId
        }

        TextField {
            Layout.fillWidth: true
            implicitHeight: 36
            text: root.filterText
            placeholderText: I18n.t("Фильтр по имени, IP, статусу или причине…")
            color: Theme.textPrimary
            placeholderTextColor: Theme.textMuted
            selectionColor: Theme.accent
            selectedTextColor: Theme.textPrimary
            leftPadding: 12
            rightPadding: 12
            background: Rectangle {
                color: Theme.controlBackground
                radius: Theme.radiusMd
                border.color: parent.activeFocus ? Theme.accent : Theme.controlBorder
            }
            onTextChanged: root.filterText = text
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            FilterChip { mode: "all"; label: "Все"; count: root.countByFilter("all") }
            FilterChip { mode: "issues"; label: "Проблемы"; count: root.countByFilter("issues") }
            FilterChip { mode: "offline"; label: "Офлайн"; count: root.countByFilter("offline") }
            FilterChip { mode: "online"; label: "Онлайн"; count: root.countByFilter("online") }
            FilterChip { mode: "inGrid"; label: "В раскладке"; count: root.countByFilter("inGrid") }
            FilterChip { mode: "notInGrid"; label: "Вне раскладки"; count: root.countByFilter("notInGrid") }
            FilterChip { mode: "auth"; label: "Auth"; count: root.countByFilter("auth") }
            FilterChip { mode: "stream"; label: "Поток"; count: root.countByFilter("stream") }
        }

        HealthHistoryStrip {
            id: healthHistoryStrip

            Layout.fillWidth: true
            Layout.preferredHeight: 82
            controller: root.healthController
            selectedRunId: root.selectedRunId
            onRunSelected: runId => root.selectedRunId = runId
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            radius: Theme.radiusMd
            color: Theme.panelAltBackground
            border.color: Theme.panelBorder

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10
                Text { text: I18n.t("Камера"); color: Theme.textMuted; font.pixelSize: 11; Layout.fillWidth: true }
                Text { text: "IP"; color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 120 }
                Text { text: I18n.t("Статус"); color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 92 }
                Text { text: I18n.t("Причина"); color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 250 }
                Text { text: I18n.t("В раскладке"); color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 82 }
                Text { text: I18n.t("Действия"); color: Theme.textMuted; font.pixelSize: 11; Layout.preferredWidth: 250 }
            }
        }

        ListView {
            id: cameraList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: SystemController.cameraModel

            delegate: Rectangle {
                id: row
                width: cameraList.width
                height: rowVisible ? 94 : 0
                visible: rowVisible
                radius: Theme.radiusMd
                color: hover.hovered ? Theme.cardHover : Theme.cardBackground
                border.color: row.online ? Theme.controlBorder : Theme.warning

                required property int index
                required property string cameraName
                required property string cameraIp
                required property string status

                property int cameraIndex: row.index
                property string rowName: row.cameraName && row.cameraName.trim() !== "" ? row.cameraName : (I18n.t("Камера") + " " + row.cameraIp)
                property var healthResult: {
                    var version = root.dataVersion
                    return root.healthController
                            ? root.healthController.resultForCamera(row.cameraIp)
                            : ({})
                }
                property string rowStatus: root.effectiveCameraStatus(row.cameraIp, row.status)
                property string rowDetail: String(healthResult.recommendation || "")
                                           || root.cameraAttentionReason(row.cameraIp, row.status)
                property string rowIssueKind: healthResult.status === "error" ? "offline"
                                              : (healthResult.status === "warning" ? "warning"
                                                 : root.issueKind(row.cameraIp, row.status))
                property bool online: healthResult.status === "ok"
                                      || healthResult.status === "warning"
                                      || root.isOnlineStatus(rowStatus)
                property bool inGrid: root.isCameraInGrid(row.cameraIp)
                property bool rowVisible: root.rowMatches(rowName, row.cameraIp, row.status,
                                                          root.cameraStatusSearchText(row.cameraIp, row.status),
                                                          rowDetail)

                HoverHandler { id: hover }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 9
                        CameraStatusBadge {
                            Layout.preferredWidth: 10
                            Layout.preferredHeight: 10
                            dotSize: 10
                            showText: false
                            online: row.online
                            statusText: row.rowStatus
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                Layout.fillWidth: true
                                text: row.rowName
                                color: Theme.textPrimary
                                font.pixelSize: 13
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: row.rowDetail !== "" ? I18n.t(row.rowDetail) : I18n.t("Диагностика без замечаний")
                                color: row.rowDetail !== "" ? Theme.warning : Theme.textMuted
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                visible: row.rowIssueKind !== "ok"
                                text: I18n.t("Категория") + ": " + I18n.t(row.rowIssueKind)
                                color: Theme.textMuted
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }
                    }

                    Text {
                        Layout.preferredWidth: 120
                        text: row.cameraIp
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }

                    CameraStatusBadge {
                        Layout.preferredWidth: 92
                        Layout.preferredHeight: 26
                        online: row.online
                        statusText: row.rowStatus || "Неизвестно"
                        Text {
                            anchors.centerIn: parent
                            text: I18n.t(row.rowStatus || "Неизвестно")
                            color: Theme.textPrimary
                            font.pixelSize: 11
                            font.bold: true
                        }
                    }

                    Text {
                        Layout.preferredWidth: 250
                        text: row.rowDetail !== "" ? I18n.t(row.rowDetail) : "—"
                        color: row.rowDetail !== "" ? Theme.warning : Theme.textMuted
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.preferredWidth: 82
                        text: row.inGrid ? I18n.t("Да") : I18n.t("Нет")
                        color: row.inGrid ? Theme.success : Theme.textMuted
                        font.pixelSize: 12
                        font.bold: row.inGrid
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Flow {
                        Layout.preferredWidth: 250
                        Layout.preferredHeight: 62
                        spacing: 6

                        Button {
                            text: I18n.t("Проверить")
                            height: 28
                            enabled: !root.healthController || !root.healthController.running
                            onClicked: root.recheckCamera(row.cameraIp, row.rowName)
                        }
                        Button {
                            text: I18n.language === "ru" ? "Детали" : "Details"
                            height: 28
                            enabled: Boolean(row.healthResult.ip)
                            onClicked: {
                                root.detailsResult = row.healthResult
                                healthDetailsDialog.open()
                            }
                        }
                        Button {
                            text: I18n.t("OpenIPC")
                            height: 28
                            onClicked: root.majesticRequested(row.cameraIndex)
                        }
                        Button {
                            text: row.inGrid ? I18n.t("В сетке") : I18n.t("В сетку")
                            enabled: !row.inGrid
                            height: 28
                            onClicked: root.addToGridRequested(row.cameraIndex)
                        }
                        Button {
                            text: I18n.t("Правка")
                            height: 28
                            onClicked: root.editRequested(row.cameraIndex)
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
        }

        Text {
            Layout.fillWidth: true
            visible: SystemController.cameraModel.rowCount() === 0
            text: I18n.t("Камер пока нет. Добавьте камеру или запустите поиск.")
            color: Theme.textMuted
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 13
        }
    }

    HealthDetailsDialog {
        id: healthDetailsDialog
        parent: root.parent
        healthResult: root.detailsResult
    }

    FileDialog {
        id: healthReportDialog
        title: I18n.t("Сохранить диагностический отчет")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "txt"
        nameFilters: [I18n.t("Текстовые файлы") + " (*.txt)", I18n.t("Все файлы") + " (*)"]
        onAccepted: root.saveHealthReport(String(selectedFile))
    }
}
