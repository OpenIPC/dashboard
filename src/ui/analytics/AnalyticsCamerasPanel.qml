import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root

    property var diagnostics: ({})
    property int refreshToken: 0
    property string filterText: ""

    function refresh() {
        refreshToken += 1
    }

    function moduleName(type) {
        if (type === 0) return I18n.t("Лица")
        if (type === 1) return I18n.t("Объекты")
        if (type === 2) return I18n.t("Номера")
        return I18n.t("AI")
    }

    function moduleUsable(type) {
        var token = refreshToken
        return SystemController.analyticsEngine.isModuleEnabled(type)
               && SystemController.analyticsEngine.getModuleStatus(type) === "ready"
    }

    function cameraModuleEnabled(cameraId, type) {
        var token = refreshToken
        if (!cameraId || cameraId === "")
            return false
        return SystemController.analyticsEngine.isCameraModuleEnabled(cameraId, type)
    }

    function cameraHasAnyAi(cameraId) {
        for (var type = 0; type < 3; ++type) {
            if (cameraModuleEnabled(cameraId, type))
                return true
        }
        return false
    }

    function cameraTelemetry(cameraId) {
        var token = refreshToken
        var stats = diagnostics && diagnostics.cameraStats ? diagnostics.cameraStats : []
        for (var i = 0; i < stats.length; ++i) {
            if (stats[i].cameraId === cameraId)
                return stats[i]
        }
        return ({})
    }

    function ageText(timestampMs) {
        var value = Number(timestampMs || 0)
        if (value <= 0)
            return I18n.t("нет данных")

        var deltaSec = Math.max(0, Math.floor((Date.now() - value) / 1000))
        if (deltaSec < 2)
            return I18n.t("сейчас")
        if (deltaSec < 60)
            return I18n.t("%1 сек назад", [deltaSec])

        var deltaMin = Math.floor(deltaSec / 60)
        if (deltaMin < 60)
            return I18n.t("%1 мин назад", [deltaMin])

        var deltaHour = Math.floor(deltaMin / 60)
        return I18n.t("%1 ч назад", [deltaHour])
    }

    function pipelineState(online, hasAi, telemetry) {
        if (!hasAi)
            return "unassigned"
        if (!online)
            return "offline"
        return telemetry.pipelineState || "waiting"
    }

    function pipelineText(state) {
        if (state === "offline") return I18n.t("Камера offline")
        if (state === "module_not_ready") return I18n.t("Модуль не готов")
        if (state === "processing") return I18n.t("AI обрабатывает кадр")
        if (state === "receiving") return I18n.t("AI получает кадры")
        if (state === "throttled") return I18n.t("Кадры ограничены")
        if (state === "waiting") return I18n.t("AI назначен, ожидает кадры")
        return I18n.t("AI не назначен")
    }

    function pipelineHint(state, telemetry) {
        if (state === "offline") return I18n.t("Проверьте подключение камеры")
        if (state === "module_not_ready") return I18n.t("Включите модуль и дождитесь статуса «Готов»")
        if (state === "processing") return I18n.t("Задача сейчас выполняется")
        if (state === "receiving") return I18n.t("Последний AI-кадр: %1", [ageText(telemetry.lastProcessedMs || telemetry.lastAcceptedFrameMs)])
        if (state === "throttled") return I18n.t("Сработал лимит FPS или параллельных задач")
        if (state === "waiting") return I18n.t("Поток есть, но AI еще не получил первый кадр")
        return I18n.t("Назначьте модуль этой камере")
    }

    function pipelineColor(state) {
        if (state === "receiving" || state === "processing")
            return Theme.success
        if (state === "waiting" || state === "throttled" || state === "module_not_ready")
            return Theme.warning
        if (state === "offline")
            return Theme.danger
        return Theme.textMuted
    }

    function cameraMatches(name, ip, status) {
        var q = filterText.trim().toLowerCase()
        if (q === "")
            return true
        return [name || "", ip || "", status || ""].join(" ").toLowerCase().indexOf(q) !== -1
    }

    function cameraAccessAllowed(camera, cameraIndex) {
        var token = SystemController.userManager.permissionsVersion
        return camera && SystemController.userManager.canAccessCamera(
                    camera.cameraId || "", camera.cameraIp || "", cameraIndex)
    }

    function accessibleCameraCount() {
        var token = refreshToken
        var total = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            if (cameraAccessAllowed(SystemController.cameraModel.getCamera(i), i)) total++
        }
        return total
    }

    function camerasWithAiCount() {
        var total = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var camera = SystemController.cameraModel.getCamera(i)
            if (!cameraAccessAllowed(camera, i)) continue
            if (cameraHasAnyAi(camera.cameraIp))
                total += 1
        }
        return total
    }

    function telemetryCamerasCount() {
        var stats = diagnostics && diagnostics.cameraStats ? diagnostics.cameraStats : []
        var total = 0
        for (var i = 0; i < stats.length; ++i) {
            var sourceIndex = SystemController.cameraModel.findIndexByIp(stats[i].cameraId || "")
            if (sourceIndex >= 0 && cameraAccessAllowed(
                        SystemController.cameraModel.getCamera(sourceIndex), sourceIndex)) total++
        }
        return total
    }

    function toggleCameraModule(cameraId, type) {
        if (!moduleUsable(type) || !cameraId || cameraId === "")
            return
        var next = !cameraModuleEnabled(cameraId, type)
        SystemController.analyticsEngine.setCameraModuleEnabled(cameraId, type, next)
        refresh()
    }

    Component.onCompleted: refresh()

    Connections {
        target: SystemController.analyticsEngine
        ignoreUnknownSignals: true
        function onSettingsChanged() { root.refresh() }
        function onModuleStatusChanged(type, status, progress, error) { root.refresh() }
        function onAnalyticsTelemetryChanged() { root.refresh() }
        function onAnalyticsEventsChanged() { root.refresh() }
    }

    Connections {
        target: SystemController.cameraModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.refresh() }
        function onRowsRemoved(parent, first, last) { root.refresh() }
        function onModelReset() { root.refresh() }
        function onDataChanged(topLeft, bottomRight, roles) { root.refresh() }
    }

    Connections {
        target: SystemController.userManager
        function onPermissionsVersionChanged() { root.refresh() }
    }

    component SmallStat: Rectangle {
        property string title: ""
        property string value: ""
        property color accent: Theme.textPrimary

        Layout.fillWidth: true
        implicitHeight: 58
        radius: Theme.radiusMd
        color: Theme.panelSoftBackground
        border.color: Theme.controlBorder

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: parent.parent.title
                color: Theme.textMuted
                font.pixelSize: 10
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: parent.parent.value
                color: parent.parent.accent
                font.pixelSize: 16
                font.bold: true
                elide: Text.ElideRight
            }
        }
    }

    component ModuleToggle: Button {
        id: toggle

        property int moduleType: 0
        property string cameraId: ""
        property bool active: false
        property bool usable: false

        Layout.preferredWidth: 92
        implicitHeight: 32
        enabled: usable
        hoverEnabled: true

        background: Rectangle {
            radius: Theme.radiusSm
            color: !toggle.enabled
                   ? Theme.controlBackgroundAlt
                   : (toggle.active ? Theme.accent : (toggle.hovered ? Theme.cardHover : Theme.controlBackground))
            border.color: toggle.active ? Theme.accentHover : Theme.controlBorder
        }

        contentItem: Text {
            text: root.moduleName(toggle.moduleType)
            color: toggle.enabled
                   ? (toggle.active ? Theme.textPrimary : Theme.textSecondary)
                   : Theme.textMuted
            font.pixelSize: 11
            font.bold: toggle.active
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        ToolTip.visible: hovered && !usable
        ToolTip.text: I18n.t("Сначала включите модуль во вкладке «Модули».")
        ToolTip.delay: 400
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: headerLayout.implicitHeight + 20
            radius: Theme.radiusLg
            color: Theme.panelSoftBackground
            border.color: Theme.cardBorder

            ColumnLayout {
                id: headerLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: I18n.t("Камеры и AI-назначения")
                            color: Theme.textPrimary
                            font.pixelSize: 17
                            font.bold: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Здесь видно, какие камеры реально подключены к аналитике и получает ли AI кадры.")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                        }
                    }

                    TextField {
                        Layout.preferredWidth: 260
                        implicitHeight: 34
                        text: root.filterText
                        placeholderText: I18n.t("Поиск камеры")
                        color: Theme.textPrimary
                        placeholderTextColor: Theme.textMuted
                        selectionColor: Theme.accent
                        selectedTextColor: Theme.textPrimary
                        onTextChanged: root.filterText = text
                        background: Rectangle {
                            color: Theme.controlBackground
                            radius: Theme.radiusSm
                            border.color: parent.activeFocus ? Theme.accent : Theme.controlBorder
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: width >= 900 ? 4 : 2
                    columnSpacing: 8
                    rowSpacing: 8

                    SmallStat {
                        title: I18n.t("Всего камер")
                        value: String(root.accessibleCameraCount())
                    }
                    SmallStat {
                        title: I18n.t("Камер с AI")
                        value: String(root.camerasWithAiCount())
                        accent: root.camerasWithAiCount() > 0 ? Theme.success : Theme.warning
                    }
                    SmallStat {
                        title: I18n.t("Камер в телеметрии")
                        value: String(root.telemetryCamerasCount())
                        accent: root.telemetryCamerasCount() > 0 ? Theme.success : Theme.textPrimary
                    }
                    SmallStat {
                        title: I18n.t("Событий")
                        value: String(diagnostics.events || 0)
                        accent: Theme.accent
                    }
                }
            }
        }

        ListView {
            id: cameraList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: SystemController.cameraModel
            ScrollBar.vertical: StyledScrollBar {}

            delegate: Rectangle {
                id: cameraRow

                required property int index
                required property string cameraId
                property bool rowVisible: root.refreshToken >= 0
                                          && SystemController.userManager.canAccessCamera(
                                              cameraId, cameraIp, index)
                                          && root.cameraMatches(cameraName, cameraIp, status)
                property var telemetry: root.cameraTelemetry(cameraIp)
                property bool hasAi: root.cameraHasAnyAi(cameraIp)
                property bool online: String(status || "").toLowerCase() === "online"
                property bool receivingFrames: Number(telemetry.processedFrames || 0) > 0
                property string aiState: root.pipelineState(online, hasAi, telemetry)

                width: ListView.view.width
                height: rowVisible ? 138 : 0
                visible: rowVisible
                radius: Theme.radiusLg
                color: Theme.cardBackground
                border.color: hasAi ? root.pipelineColor(aiState) : Theme.cardBorder
                clip: true

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: 10
                        radius: 5
                        color: cameraRow.online ? Theme.success : Theme.danger
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 220
                        spacing: 4

                        Text {
                            Layout.fillWidth: true
                            text: (cameraName && cameraName !== "") ? cameraName : I18n.t("Камера %1", [cameraIp])
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: cameraIp + " · " + I18n.t(status || "Неизвестно")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: root.pipelineText(cameraRow.aiState)
                            color: root.pipelineColor(cameraRow.aiState)
                            font.pixelSize: 11
                            font.bold: cameraRow.hasAi
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: root.pipelineHint(cameraRow.aiState, cameraRow.telemetry)
                            color: Theme.textFaint
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }

                    RowLayout {
                        Layout.preferredWidth: 292
                        spacing: 8

                        Repeater {
                            model: [0, 1, 2]
                            ModuleToggle {
                                moduleType: modelData
                                cameraId: cameraIp
                                active: root.cameraModuleEnabled(cameraIp, modelData)
                                usable: root.moduleUsable(modelData)
                                onClicked: root.toggleCameraModule(cameraIp, modelData)
                            }
                        }
                    }

                    GridLayout {
                        Layout.preferredWidth: 470
                        columns: 5
                        columnSpacing: 8
                        rowSpacing: 8

                        SmallStat {
                            title: I18n.t("Кадры")
                            value: String(cameraRow.telemetry.processedFrames || 0)
                        }
                        SmallStat {
                            title: I18n.t("Пропущено")
                            value: String(cameraRow.telemetry.skippedFrames || 0)
                            accent: Number(cameraRow.telemetry.skippedFrames || 0) > 0 ? Theme.warning : Theme.textPrimary
                        }
                        SmallStat {
                            title: I18n.t("Детекции")
                            value: String(cameraRow.telemetry.detections || 0)
                        }
                        SmallStat {
                            title: I18n.t("События")
                            value: String(cameraRow.telemetry.events || 0)
                        }
                        SmallStat {
                            title: I18n.t("AI-кадр")
                            value: root.ageText(cameraRow.telemetry.lastProcessedMs || cameraRow.telemetry.lastAcceptedFrameMs)
                            accent: cameraRow.receivingFrames ? Theme.success : Theme.textMuted
                        }
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                visible: root.accessibleCameraCount() === 0
                text: I18n.t("Добавьте камеру, чтобы назначить ей AI-модули.")
                color: Theme.textMuted
                font.pixelSize: 14
            }
        }
    }
}
