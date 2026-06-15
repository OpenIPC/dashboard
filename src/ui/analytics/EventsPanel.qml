import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root

    property var model
    property int moduleType: -1
    property string snapshotsDir: ""
    property string clipsDir: ""
    property string moduleBadgeText: ""
    property var eventItems: []
    property var moduleTelemetry: ({})
    property var counterSummary: []
    property var analyticsSettings: ({})
    property int enabledModulesCount: 0
    property int camerasWithAnalyticsCount: 0
    property int enabledCameraModulesCount: 0
    property int configuredRulesCount: 0
    property int selectedIndex: -1
    property string searchText: ""
    property string cameraFilter: ""
    property int eventTypeFilterIndex: 0
    readonly property int telemetryColumns: width >= 1350 ? 5 : width >= 900 ? 3 : 2
    readonly property real listPaneWidth: Math.max(320, Math.min(400, width * 0.38))
    readonly property int actionButtonsColumns: width >= 760 ? 4 : width >= 480 ? 2 : 1
    readonly property bool eventStoreReady: root.metricValue(root.moduleTelemetry, "eventStoreReady", false) === true
    readonly property bool evidenceEnabled: root.analyticsSettings
                                         && root.analyticsSettings.evidence
                                         && root.analyticsSettings.evidence.enabled === true

    readonly property var selectedEvent: (selectedIndex >= 0 && selectedIndex < eventItems.length) ? eventItems[selectedIndex] : null
    readonly property string eventTypeFilter: {
        if (eventTypeFilterIndex === 1) return "rule"
        if (eventTypeFilterIndex === 2) return "counter"
        if (eventTypeFilterIndex === 3) return "detection"
        return ""
    }

    function metricValue(map, key, fallback) {
        if (!map || map[key] === undefined || map[key] === null)
            return fallback
        return map[key]
    }

    function metricText(value, suffix) {
        var sfx = suffix || ""
        if (typeof value === "number") {
            if (Math.abs(value - Math.round(value)) < 0.01)
                return Math.round(value) + sfx
            return value.toFixed(1) + sfx
        }
        if (value === undefined || value === null || value === "")
            return "0" + sfx
        return value + sfx
    }

    function displayValue(value, fallback) {
        var fb = fallback === undefined ? "—" : fallback
        if (value === undefined || value === null || value === "")
            return fb
        return value
    }

    function eventColor(eventType) {
        if (eventType === "rule")
            return Theme.accent
        if (eventType === "counter")
            return Theme.success
        return "#6b7280"
    }

    function eventLabel(eventType) {
        if (eventType === "rule")
            return I18n.t("Правило")
        if (eventType === "counter")
            return I18n.t("Счетчик")
        return I18n.t("Детекция")
    }

    function localUrl(path, fallbackUrl) {
        if (fallbackUrl && fallbackUrl !== "")
            return fallbackUrl
        if (!path || path === "")
            return ""
        return "file:///" + String(path).replace(/\\/g, "/")
    }

    function cameraIdFromModel(camera) {
        if (!camera)
            return ""
        if (camera.cameraIp !== undefined && camera.cameraIp !== "")
            return camera.cameraIp
        if (camera.ip !== undefined && camera.ip !== "")
            return camera.ip
        if (camera.cameraName !== undefined && camera.cameraName !== "")
            return camera.cameraName
        return ""
    }

    function refreshPipelineStatus() {
        if (!root.model) {
            root.analyticsSettings = ({})
            root.enabledModulesCount = 0
            root.camerasWithAnalyticsCount = 0
            root.enabledCameraModulesCount = 0
            root.configuredRulesCount = 0
            return
        }

        root.analyticsSettings = root.model.getSettings ? root.model.getSettings() : ({})

        var enabledModules = 0
        var configuredRules = 0
        for (var moduleIndex = 0; moduleIndex < 3; ++moduleIndex) {
            if (root.model.isModuleEnabled && root.model.isModuleEnabled(moduleIndex))
                enabledModules += 1

            var moduleConfig = root.model.getModuleConfig ? root.model.getModuleConfig(moduleIndex) : ({})
            var rules = moduleConfig && moduleConfig.rules ? moduleConfig.rules : []
            for (var ruleIndex = 0; ruleIndex < rules.length; ++ruleIndex) {
                var rule = rules[ruleIndex]
                if (!rule || rule.enabled !== false)
                    configuredRules += 1
            }
        }

        var camerasWithAnalytics = 0
        var enabledAssignments = 0
        if (SystemController.cameraModel && root.model.isCameraModuleEnabled) {
            for (var cameraIndex = 0; cameraIndex < SystemController.cameraModel.rowCount(); ++cameraIndex) {
                var camera = SystemController.cameraModel.getCamera(cameraIndex)
                var cameraId = root.cameraIdFromModel(camera)
                if (cameraId === "")
                    continue

                var cameraHasAnalytics = false
                for (var type = 0; type < 3; ++type) {
                    if (root.model.isCameraModuleEnabled(cameraId, type)) {
                        cameraHasAnalytics = true
                        enabledAssignments += 1
                    }
                }
                if (cameraHasAnalytics)
                    camerasWithAnalytics += 1
            }
        }

        root.enabledModulesCount = enabledModules
        root.camerasWithAnalyticsCount = camerasWithAnalytics
        root.enabledCameraModulesCount = enabledAssignments
        root.configuredRulesCount = configuredRules
    }

    function pipelineHintText() {
        if (!root.eventStoreReady)
            return I18n.t("Хранилище событий недоступно: лента не сможет сохранить новые срабатывания.")
        if (!root.evidenceEnabled)
            return I18n.t("События выключены в настройках аналитики: включите сохранение событий, чтобы лента начала пополняться.")
        if (root.enabledModulesCount === 0)
            return I18n.t("Включите хотя бы один модуль аналитики: лица, объекты или номера.")
        if (root.camerasWithAnalyticsCount === 0)
            return I18n.t("Включите AI-модуль на нужной камере в видеосетке: после этого кадры начнут попадать в аналитику.")
        if (root.configuredRulesCount === 0)
            return I18n.t("События готовы: без правил лента будет показывать общие детекции; для точных сценариев добавьте правило во вкладке «Правила».")
        return I18n.t("События готовы: детекции будут проходить через правила, а совпадения появятся в ленте со снимками или клипами.")
    }

    function refreshData() {
        if (!root.model) {
            root.eventItems = []
            root.moduleTelemetry = ({})
            root.counterSummary = []
            root.selectedIndex = -1
            root.refreshPipelineStatus()
            return
        }

        var events = root.model.queryAnalyticsEvents
            ? root.model.queryAnalyticsEvents(root.moduleType, root.cameraFilter.trim(), root.searchText.trim(), 500)
            : (root.model.analyticsEvents || [])
        var filtered = []
        for (var i = 0; i < events.length; ++i) {
            var eventItem = events[i]
            if (root.moduleType >= 0 && Number(eventItem.moduleType) !== root.moduleType)
                continue
            if (root.eventTypeFilter !== "" && eventItem.eventType !== root.eventTypeFilter)
                continue
            filtered.push(eventItem)
        }

        root.eventItems = filtered
        root.moduleTelemetry = (root.moduleType < 0 && root.model.analyticsDiagnostics)
            ? root.model.analyticsDiagnostics
            : (root.model.getModuleTelemetry ? root.model.getModuleTelemetry(root.moduleType) : ({}))
        root.counterSummary = (root.moduleType === 1 && root.model.getObjectCounterSummary)
            ? root.model.getObjectCounterSummary()
            : []
        root.refreshPipelineStatus()

        if (root.eventItems.length === 0) {
            root.selectedIndex = -1
        } else if (root.selectedIndex < 0 || root.selectedIndex >= root.eventItems.length) {
            root.selectedIndex = 0
        }

        eventList.currentIndex = root.selectedIndex
    }

    Component.onCompleted: refreshData()
    onSelectedIndexChanged: eventList.currentIndex = selectedIndex

    Connections {
        target: root.model
        ignoreUnknownSignals: true
        function onAnalyticsEventsChanged() { root.refreshData() }
        function onAnalyticsTelemetryChanged() { root.refreshData() }
        function onSettingsChanged() { root.refreshData() }
        function onModuleConfigChanged(type) { root.refreshPipelineStatus() }
        function onModuleStatusChanged(type, status, progress, error) { root.refreshData() }
    }

    Connections {
        target: SystemController.cameraModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.refreshPipelineStatus() }
        function onRowsRemoved(parent, first, last) { root.refreshPipelineStatus() }
        function onModelReset() { root.refreshPipelineStatus() }
        function onDataChanged(topLeft, bottomRight, roles) { root.refreshPipelineStatus() }
    }

    component MetricTile: Rectangle {
        property string title: ""
        property string value: "0"

        radius: Theme.radiusLg
        color: Theme.cardBackground
        border.color: Theme.cardBorder
        implicitHeight: 68
        Layout.minimumWidth: 150
        Layout.fillWidth: true

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 9
            spacing: 2

            Text {
                text: parent.parent.title
                color: Theme.textMuted
                font.pixelSize: 11
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                Layout.fillWidth: true
            }

            Text {
                text: parent.parent.value
                color: Theme.textPrimary
                font.pixelSize: 19
                font.bold: true
            }
        }
    }

    component PanelButton: Button {
        implicitHeight: 30
        leftPadding: 12
        rightPadding: 12

        background: Rectangle {
            color: parent.enabled
                   ? (parent.hovered ? Theme.cardHover : Theme.controlBackground)
                   : Theme.controlBackgroundAlt
            radius: Theme.radiusSm
            border.color: parent.enabled ? Theme.controlBorder : Theme.panelBorder
            border.width: 1
        }

        contentItem: Text {
            text: parent.text
            color: parent.enabled ? Theme.textSecondary : Theme.textMuted
            font.pixelSize: 12
            font.bold: false
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component PipelineStep: Rectangle {
        id: pipelineStep

        property string title: ""
        property string value: ""
        property string hint: ""
        property bool ok: false
        property bool warning: false
        readonly property color statusColor: ok ? Theme.success : (warning ? Theme.warning : Theme.danger)

        Layout.fillWidth: true
        Layout.minimumWidth: 150
        implicitHeight: 58
        radius: Theme.radiusMd
        color: Theme.cardBackground
        border.color: Theme.cardBorder
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 9
            spacing: 9

            Rectangle {
                Layout.preferredWidth: 10
                Layout.preferredHeight: 10
                radius: 5
                color: pipelineStep.statusColor
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Text {
                    Layout.fillWidth: true
                    text: pipelineStep.title
                    color: Theme.textMuted
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: pipelineStep.value
                    color: Theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: pipelineStep.hint
                    color: Theme.textFaint
                    font.pixelSize: 9
                    elide: Text.ElideRight
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.panelBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorder
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: root.moduleBadgeText !== "" ? root.moduleBadgeText : I18n.t("События")
                color: Theme.textPrimary
                font.pixelSize: 18
                font.bold: true
            }

            Rectangle {
                color: Theme.panelSoftBackground
                border.color: Theme.controlBorder
                radius: Theme.radiusSm
                implicitHeight: 24
                implicitWidth: statusText.implicitWidth + 14

                Text {
                    id: statusText
                    anchors.centerIn: parent
                    text: I18n.t("Событий в сессии: %1", [root.eventItems.length])
                    color: Theme.textMuted
                    font.pixelSize: 11
                }
            }

            Item { Layout.fillWidth: true }

            PanelButton {
                text: I18n.t("Очистить список")
                enabled: root.model && root.eventItems.length > 0
                onClicked: root.model.clearAnalyticsEvents(root.moduleType)
            }
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("Runtime-события, правила, трекинг и живая телеметрия модуля.")
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: pipelineContent.implicitHeight + 22
            radius: Theme.radiusLg
            color: Theme.panelSoftBackground
            border.color: Theme.cardBorder
            border.width: 1

            ColumnLayout {
                id: pipelineContent
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 10
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: I18n.t("Как работают события")
                        color: Theme.textPrimary
                        font.pixelSize: 13
                        font.bold: true
                    }

                    Text {
                        Layout.fillWidth: true
                        text: I18n.t("Кадр камеры → AI-модуль → детекция → правило или общее событие → снимок/клип.")
                        color: Theme.textMuted
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: width >= 1180 ? 5 : width >= 780 ? 3 : 1
                    columnSpacing: 8
                    rowSpacing: 8

                    PipelineStep {
                        title: I18n.t("Хранилище")
                        value: root.eventStoreReady ? I18n.t("Готово") : I18n.t("Недоступно")
                        hint: root.eventStoreReady ? I18n.t("SQLite активен") : I18n.t("Проверьте путь")
                        ok: root.eventStoreReady
                    }

                    PipelineStep {
                        title: I18n.t("События")
                        value: root.evidenceEnabled ? I18n.t("Включены") : I18n.t("Выключены")
                        hint: root.evidenceEnabled ? I18n.t("Доказательства пишутся") : I18n.t("Включите в настройках")
                        ok: root.evidenceEnabled
                    }

                    PipelineStep {
                        title: I18n.t("Модули")
                        value: I18n.t("Активно: %1", [root.enabledModulesCount])
                        hint: I18n.t("Лица / объекты / номера")
                        ok: root.enabledModulesCount > 0
                    }

                    PipelineStep {
                        title: I18n.t("Камеры")
                        value: I18n.t("С AI: %1", [root.camerasWithAnalyticsCount])
                        hint: I18n.t("Назначений: %1", [root.enabledCameraModulesCount])
                        ok: root.camerasWithAnalyticsCount > 0
                    }

                    PipelineStep {
                        title: I18n.t("Правила")
                        value: root.configuredRulesCount > 0 ? I18n.t("Есть: %1", [root.configuredRulesCount]) : I18n.t("Нет правил")
                        hint: root.configuredRulesCount > 0 ? I18n.t("Фильтруют срабатывания") : I18n.t("Будут общие детекции")
                        ok: root.configuredRulesCount > 0
                        warning: root.configuredRulesCount === 0
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: root.pipelineHintText()
                    color: root.eventStoreReady && root.evidenceEnabled && root.enabledModulesCount > 0 && root.camerasWithAnalyticsCount > 0
                           ? Theme.success
                           : Theme.warning
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width >= 980 ? 4 : width >= 620 ? 2 : 1
            columnSpacing: 8
            rowSpacing: 8

            TextField {
                Layout.fillWidth: true
                placeholderText: I18n.t("Поиск")
                text: root.searchText
                color: Theme.textPrimary
                placeholderTextColor: Theme.textMuted
                selectionColor: Theme.accent
                selectedTextColor: Theme.textPrimary
                background: Rectangle {
                    color: Theme.controlBackground
                    radius: Theme.radiusSm
                    border.color: parent.activeFocus ? Theme.accent : Theme.controlBorder
                    border.width: 1
                }
                onTextChanged: {
                    root.searchText = text
                    root.refreshData()
                }
            }

            TextField {
                Layout.fillWidth: true
                placeholderText: I18n.t("Камера")
                text: root.cameraFilter
                color: Theme.textPrimary
                placeholderTextColor: Theme.textMuted
                selectionColor: Theme.accent
                selectedTextColor: Theme.textPrimary
                background: Rectangle {
                    color: Theme.controlBackground
                    radius: Theme.radiusSm
                    border.color: parent.activeFocus ? Theme.accent : Theme.controlBorder
                    border.width: 1
                }
                onTextChanged: {
                    root.cameraFilter = text
                    root.refreshData()
                }
            }

            StyledComboBox {
                Layout.fillWidth: true
                model: [I18n.t("Все типы"), I18n.t("Правила"), I18n.t("Счетчики"), I18n.t("Детекции")]
                currentIndex: root.eventTypeFilterIndex
                onCurrentIndexChanged: {
                    root.eventTypeFilterIndex = currentIndex
                    root.refreshData()
                }
            }

            PanelButton {
                Layout.fillWidth: true
                text: I18n.t("Сброс")
                enabled: root.searchText !== "" || root.cameraFilter !== "" || root.eventTypeFilterIndex !== 0
                onClicked: {
                    root.searchText = ""
                    root.cameraFilter = ""
                    root.eventTypeFilterIndex = 0
                    root.refreshData()
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.telemetryColumns
            rowSpacing: 8
            columnSpacing: 8

            MetricTile {
                title: I18n.t("Кадры")
                value: root.metricText(root.metricValue(root.moduleTelemetry, "processedFrames", 0))
            }

            MetricTile {
                title: I18n.t("Детекции")
                value: root.metricText(root.metricValue(root.moduleTelemetry, "detections", 0))
            }

            MetricTile {
                title: I18n.t("События")
                value: root.metricText(root.metricValue(root.moduleTelemetry, "events", 0))
            }

            MetricTile {
                title: I18n.t("Средняя задержка")
                value: root.metricText(root.metricValue(root.moduleTelemetry, "averageInferenceMs", 0), " ms")
            }

            MetricTile {
                title: root.moduleType === 1 ? I18n.t("Активные треки") : I18n.t("Пропущено")
                value: root.moduleType === 1
                    ? root.metricText(root.metricValue(root.moduleTelemetry, "activeTracks", 0))
                    : root.metricText(root.metricValue(root.moduleTelemetry, "skippedFrames", 0))
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Rectangle {
                Layout.preferredWidth: root.listPaneWidth
                Layout.minimumWidth: 300
                Layout.fillHeight: true
                color: Theme.cardBackground
                radius: Theme.radiusLg
                border.color: Theme.cardBorder

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    Text {
                        text: I18n.t("Лента событий")
                        color: Theme.textPrimary
                        font.pixelSize: 13
                        font.bold: true
                    }

                    ListView {
                        id: eventList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 6
                        model: root.eventItems
                        currentIndex: root.selectedIndex

                        delegate: Rectangle {
                            property var eventItem: modelData

                            width: ListView.view.width
                            height: 84
                            radius: Theme.radiusMd
                            color: ListView.isCurrentItem ? Theme.panelAltBackground : Theme.panelSoftBackground
                            border.color: ListView.isCurrentItem ? Theme.accent : Theme.controlBorder

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.selectedIndex = index
                                    eventList.currentIndex = index
                                }
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 6

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Rectangle {
                                        radius: Theme.radiusSm
                                        color: root.eventColor(eventItem.eventType)
                                        implicitHeight: 22
                                        implicitWidth: badgeText.implicitWidth + 10

                                        Text {
                                            id: badgeText
                                            anchors.centerIn: parent
                                            text: root.eventLabel(eventItem.eventType)
                                            color: "white"
                                            font.pixelSize: 10
                                            font.bold: true
                                        }
                                    }

                                    Text {
                                        text: root.displayValue(eventItem.ruleName, eventItem.label)
                                        color: Theme.textPrimary
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    Text {
                                        text: root.displayValue(eventItem.timestampText)
                                        color: Theme.textMuted
                                        font.pixelSize: 11
                                    }
                                }

                                Text {
                                    text: root.displayValue(eventItem.message)
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                    maximumLineCount: 2
                                    elide: Text.ElideRight
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Text {
                                        Layout.fillWidth: true
                                        text: I18n.t("Камера: %1", [root.displayValue(eventItem.cameraId)])
                                              + "  ·  "
                                              + I18n.t("Точность: %1%", [Math.round(Number(eventItem.confidence || 0) * 100)])
                                              + ((eventItem.trackId !== undefined && eventItem.trackId !== "")
                                                 ? ("  ·  " + I18n.t("Трек: %1", [eventItem.trackId]))
                                                 : "")
                                        color: Theme.textMuted
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        visible: eventItem.snapshotPath !== undefined && eventItem.snapshotPath !== ""
                                        text: I18n.t("Снимок")
                                        color: Theme.success
                                        font.pixelSize: 11
                                    }

                                    Text {
                                        visible: eventItem.clipPath !== undefined && eventItem.clipPath !== ""
                                        text: I18n.t("Клип")
                                        color: Theme.accent
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }

                        ScrollBar.vertical: StyledScrollBar {}
                    }

                    Text {
                        visible: root.eventItems.length === 0
                        text: I18n.t("Событий пока нет. Проверьте статус выше: события появляются, когда включены доказательства, модуль аналитики назначен камере и детектор получает кадры.")
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.cardBackground
                radius: Theme.radiusLg
                border.color: Theme.cardBorder

                ScrollView {
                    id: detailsScroll
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true
                    contentWidth: availableWidth

                    ColumnLayout {
                        width: detailsScroll.availableWidth
                        spacing: 10

                        Text {
                            text: I18n.t("Детали события")
                            color: Theme.textPrimary
                            font.pixelSize: 13
                            font.bold: true
                        }

                        Rectangle {
                            visible: root.selectedEvent === null
                            Layout.fillWidth: true
                            implicitHeight: 130
                            radius: Theme.radiusMd
                            color: Theme.panelSoftBackground
                            border.color: Theme.controlBorder

                            ColumnLayout {
                                anchors.centerIn: parent
                                width: Math.min(parent.width - 24, 420)
                                spacing: 6

                                Text {
                                    Layout.fillWidth: true
                                    text: I18n.t("Выберите событие слева")
                                    color: Theme.textMuted
                                    font.pixelSize: 14
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    wrapMode: Text.WordWrap
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: I18n.t("Здесь появятся снимок, параметры события и быстрые действия.")
                                    color: Theme.textFaint
                                    font.pixelSize: 11
                                    horizontalAlignment: Text.AlignHCenter
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        ColumnLayout {
                            visible: root.selectedEvent !== null
                            Layout.fillWidth: true
                            spacing: 10

                            Rectangle {
                                Layout.fillWidth: true
                                radius: Theme.radiusMd
                                color: Theme.panelSoftBackground
                                border.color: Theme.controlBorder
                                implicitHeight: 110

                                GridLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    columns: 2
                                    columnSpacing: 12
                                    rowSpacing: 6

                                    Text { text: I18n.t("Тип"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text { text: root.eventLabel(root.selectedEvent ? root.selectedEvent.eventType : ""); color: Theme.textPrimary; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }

                                    Text { text: I18n.t("Камера"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text { text: root.displayValue(root.selectedEvent ? root.selectedEvent.cameraId : ""); color: Theme.textPrimary; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }

                                    Text { text: I18n.t("Метка"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text { text: root.displayValue(root.selectedEvent ? root.selectedEvent.label : ""); color: Theme.textPrimary; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }

                                    Text { text: I18n.t("Правило"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text { text: root.displayValue(root.selectedEvent ? root.selectedEvent.ruleName : ""); color: Theme.textPrimary; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }

                                    Text { text: I18n.t("Трек"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text { text: root.displayValue(root.selectedEvent ? root.selectedEvent.trackId : ""); color: Theme.textPrimary; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }

                                    Text { text: I18n.t("Время"); color: Theme.textMuted; font.pixelSize: 11 }
                                    Text { text: root.displayValue(root.selectedEvent ? root.selectedEvent.timestampText : ""); color: Theme.textPrimary; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }

                                    Text {
                                        visible: root.selectedEvent && root.selectedEvent.countTotal !== undefined
                                        text: I18n.t("Счет")
                                        color: Theme.textMuted
                                        font.pixelSize: 11
                                    }
                                    Text {
                                        visible: root.selectedEvent && root.selectedEvent.countTotal !== undefined
                                        text: root.displayValue(root.selectedEvent ? root.selectedEvent.countTotal : "")
                                        color: Theme.textPrimary
                                        font.pixelSize: 11
                                    }
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: root.selectedEvent ? root.displayValue(root.selectedEvent.message) : ""
                                color: Theme.textSecondary
                                wrapMode: Text.WordWrap
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: 240
                                radius: Theme.radiusMd
                                color: Theme.panelSoftBackground
                                border.color: Theme.controlBorder
                                clip: true

                                Image {
                                    id: previewImage
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    fillMode: Image.PreserveAspectFit
                                    asynchronous: true
                                    cache: false
                                    source: root.selectedEvent
                                        ? root.localUrl(root.selectedEvent.snapshotPath, root.selectedEvent.snapshotUrl)
                                        : ""
                                    visible: source !== ""
                                }

                                Text {
                                    anchors.centerIn: parent
                                    visible: !previewImage.visible
                                    text: I18n.t("Для этого события нет снимка")
                                    color: Theme.textMuted
                                }
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: root.actionButtonsColumns
                                columnSpacing: 8
                                rowSpacing: 8

                                PanelButton {
                                    Layout.fillWidth: true
                                    text: I18n.t("Открыть снимок")
                                    enabled: root.selectedEvent && root.localUrl(root.selectedEvent.snapshotPath, root.selectedEvent.snapshotUrl) !== ""
                                    onClicked: Qt.openUrlExternally(root.localUrl(root.selectedEvent.snapshotPath, root.selectedEvent.snapshotUrl))
                                }

                                PanelButton {
                                    Layout.fillWidth: true
                                    text: I18n.t("Открыть клип")
                                    enabled: root.selectedEvent && root.localUrl(root.selectedEvent.clipPath, root.selectedEvent.clipUrl) !== ""
                                    onClicked: Qt.openUrlExternally(root.localUrl(root.selectedEvent.clipPath, root.selectedEvent.clipUrl))
                                }

                                PanelButton {
                                    Layout.fillWidth: true
                                    text: I18n.t("Папка снимков")
                                    enabled: root.snapshotsDir !== ""
                                    onClicked: SystemController.openFolder(root.snapshotsDir)
                                }

                                PanelButton {
                                    Layout.fillWidth: true
                                    text: I18n.t("Папка клипов")
                                    enabled: root.clipsDir !== ""
                                    onClicked: SystemController.openFolder(root.clipsDir)
                                }
                            }

                            Rectangle {
                                visible: root.moduleType === 1 && root.counterSummary.length > 0
                                Layout.fillWidth: true
                                radius: Theme.radiusMd
                                color: Theme.panelSoftBackground
                                border.color: Theme.controlBorder
                                implicitHeight: 160

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 8

                                    Text {
                                        text: I18n.t("Итоги счетчика по камерам")
                                        color: Theme.textPrimary
                                        font.bold: true
                                        font.pixelSize: 12
                                    }

                                    Repeater {
                                        model: root.counterSummary

                                        delegate: Rectangle {
                                            required property var modelData

                                            Layout.fillWidth: true
                                            implicitHeight: 34
                                            radius: Theme.radiusSm
                                            color: Theme.cardBackground
                                            border.color: Theme.cardBorder

                                            RowLayout {
                                                anchors.fill: parent
                                                anchors.margins: 8
                                                spacing: 8

                                                Text {
                                                    text: root.displayValue(modelData.cameraId)
                                                    color: Theme.textPrimary
                                                    font.bold: true
                                                }

                                                Text {
                                                    text: I18n.t("Треки: %1", [root.displayValue(modelData.activeTracks, 0)])
                                                    color: Theme.textMuted
                                                    font.pixelSize: 11
                                                }

                                                Text {
                                                    text: I18n.t("Всего: %1", [root.displayValue(modelData.totalUniqueObjects, 0)])
                                                    color: Theme.textMuted
                                                    font.pixelSize: 11
                                                }

                                                Item { Layout.fillWidth: true }

                                                Text {
                                                    text: root.displayValue(modelData.countsText, I18n.t("Нет данных"))
                                                    color: Theme.textSecondary
                                                    font.pixelSize: 11
                                                    elide: Text.ElideRight
                                                    Layout.fillWidth: true
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
