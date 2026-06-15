import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root

    property var diagnostics: ({})
    property var settings: ({})
    property int refreshToken: 0

    function refresh() {
        refreshToken += 1
        if (SystemController.analyticsEngine && SystemController.analyticsEngine.getSettings) {
            settings = SystemController.analyticsEngine.getSettings()
        } else {
            settings = ({})
        }
    }

    function moduleName(type) {
        if (type === 0) return I18n.t("Лица")
        if (type === 1) return I18n.t("Объекты")
        if (type === 2) return I18n.t("Номера")
        return I18n.t("Модуль")
    }

    function moduleStatusLabel(status) {
        if (status === "ready") return I18n.t("Готов")
        if (status === "downloading") return I18n.t("Загрузка")
        if (status === "error") return I18n.t("Ошибка")
        return I18n.t("Не установлен")
    }

    function evidenceEnabled() {
        var token = refreshToken
        return settings && settings.evidence && settings.evidence.enabled === true
    }

    function moduleEnabled(type) {
        var token = refreshToken
        return SystemController.analyticsEngine.isModuleEnabled(type)
    }

    function moduleReady(type) {
        var token = refreshToken
        return SystemController.analyticsEngine.getModuleStatus(type) === "ready"
    }

    function enabledModulesCount() {
        var total = 0
        for (var type = 0; type < 3; ++type) {
            if (moduleEnabled(type))
                total += 1
        }
        return total
    }

    function cameraHasAi(cameraId) {
        var token = refreshToken
        if (!cameraId || cameraId === "")
            return false
        for (var type = 0; type < 3; ++type) {
            if (SystemController.analyticsEngine.isCameraModuleEnabled(cameraId, type))
                return true
        }
        return false
    }

    function camerasWithAiCount() {
        var total = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var camera = SystemController.cameraModel.getCamera(i)
            if (cameraHasAi(camera.cameraIp))
                total += 1
        }
        return total
    }

    function assignedModulesCount() {
        var total = 0
        var token = refreshToken
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var camera = SystemController.cameraModel.getCamera(i)
            for (var type = 0; type < 3; ++type) {
                if (SystemController.analyticsEngine.isCameraModuleEnabled(camera.cameraIp, type))
                    total += 1
            }
        }
        return total
    }

    function assignedCamerasForModule(type) {
        var total = 0
        var token = refreshToken
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var camera = SystemController.cameraModel.getCamera(i)
            if (SystemController.analyticsEngine.isCameraModuleEnabled(camera.cameraIp, type))
                total += 1
        }
        return total
    }

    function configuredRulesCount() {
        var total = 0
        var token = refreshToken
        for (var type = 0; type < 3; ++type) {
            var cfg = SystemController.analyticsEngine.getModuleConfig(type)
            var rules = cfg && cfg.rules ? cfg.rules : []
            for (var i = 0; i < rules.length; ++i) {
                if (!rules[i] || rules[i].enabled !== false)
                    total += 1
            }
        }
        return total
    }

    function telemetryCamerasCount() {
        return diagnostics && diagnostics.cameraStats ? diagnostics.cameraStats.length : 0
    }

    function readinessText() {
        if (!diagnostics || diagnostics.eventStoreReady !== true)
            return I18n.t("Хранилище событий недоступно.")
        if (!evidenceEnabled())
            return I18n.t("События выключены в настройках.")
        if (enabledModulesCount() === 0)
            return I18n.t("Включите хотя бы один AI-модуль.")
        if (camerasWithAiCount() === 0)
            return I18n.t("Назначьте AI-модуль нужным камерам.")
        return I18n.t("Аналитика готова к работе.")
    }

    Component.onCompleted: refresh()

    Connections {
        target: SystemController.analyticsEngine
        ignoreUnknownSignals: true
        function onSettingsChanged() { root.refresh() }
        function onModuleStatusChanged(type, status, progress, error) { root.refresh() }
        function onModuleConfigChanged(type) { root.refresh() }
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

    component StatCard: Rectangle {
        property string title: ""
        property string value: ""
        property string hint: ""
        property color accent: Theme.accent

        Layout.fillWidth: true
        Layout.minimumWidth: 160
        implicitHeight: 78
        radius: Theme.radiusLg
        color: Theme.cardBackground
        border.color: Theme.cardBorder

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 3

            Text {
                Layout.fillWidth: true
                text: parent.parent.title
                color: Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: parent.parent.value
                color: parent.parent.accent
                font.pixelSize: 20
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: parent.parent.hint
                color: Theme.textFaint
                font.pixelSize: 10
                elide: Text.ElideRight
            }
        }
    }

    component StepCard: Rectangle {
        property string title: ""
        property string body: ""
        property bool ok: false
        property bool warning: false
        readonly property color statusColor: ok ? Theme.success : (warning ? Theme.warning : Theme.danger)

        Layout.fillWidth: true
        Layout.minimumWidth: 210
        implicitHeight: 96
        radius: Theme.radiusLg
        color: Theme.cardBackground
        border.color: Theme.cardBorder

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 12
                Layout.preferredHeight: 12
                radius: 6
                color: parent.parent.statusColor
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    Layout.fillWidth: true
                    text: parent.parent.parent.title
                    color: Theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: parent.parent.parent.body
                    color: Theme.textMuted
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                }
            }
        }
    }

    ScrollView {
        id: overviewScroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: overviewScroll.availableWidth
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: overviewHeader.implicitHeight + 24
                radius: Theme.radiusLg
                color: Theme.panelSoftBackground
                border.color: Theme.cardBorder

                ColumnLayout {
                    id: overviewHeader
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 6

                    Text {
                        Layout.fillWidth: true
                        text: I18n.t("Центр управления аналитикой")
                        color: Theme.textPrimary
                        font.pixelSize: 18
                        font.bold: true
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.readinessText()
                        color: diagnostics && diagnostics.eventStoreReady && root.evidenceEnabled() && root.enabledModulesCount() > 0 && root.camerasWithAiCount() > 0
                               ? Theme.success
                               : Theme.warning
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width >= 1160 ? 5 : width >= 740 ? 3 : 1
                columnSpacing: 8
                rowSpacing: 8

                StatCard {
                    title: I18n.t("Камеры")
                    value: String(SystemController.cameraModel.rowCount())
                    hint: I18n.t("В списке устройств")
                    accent: Theme.textPrimary
                }

                StatCard {
                    title: I18n.t("Камеры с AI")
                    value: String(root.camerasWithAiCount())
                    hint: I18n.t("Назначены модули")
                    accent: root.camerasWithAiCount() > 0 ? Theme.success : Theme.warning
                }

                StatCard {
                    title: I18n.t("AI-модули")
                    value: String(root.enabledModulesCount())
                    hint: I18n.t("Глобально включены")
                    accent: root.enabledModulesCount() > 0 ? Theme.success : Theme.warning
                }

                StatCard {
                    title: I18n.t("Детекции")
                    value: String(diagnostics.detections || 0)
                    hint: I18n.t("За текущую сессию")
                    accent: Theme.accent
                }

                StatCard {
                    title: I18n.t("События")
                    value: String(diagnostics.events || 0)
                    hint: I18n.t("В ленте")
                    accent: Theme.accent
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width >= 1100 ? 4 : width >= 760 ? 2 : 1
                columnSpacing: 8
                rowSpacing: 8

                StepCard {
                    title: I18n.t("1. События")
                    body: root.evidenceEnabled()
                          ? I18n.t("Сохранение событий включено.")
                          : I18n.t("Включите события в настройках аналитики.")
                    ok: root.evidenceEnabled()
                }

                StepCard {
                    title: I18n.t("2. Модули")
                    body: root.enabledModulesCount() > 0
                          ? I18n.t("AI-модули готовы к назначению.")
                          : I18n.t("Включите лица, объекты или номера.")
                    ok: root.enabledModulesCount() > 0
                }

                StepCard {
                    title: I18n.t("3. Камеры")
                    body: root.camerasWithAiCount() > 0
                          ? I18n.t("Камеры получают AI-назначения.")
                          : I18n.t("Назначьте модули нужным камерам.")
                    ok: root.camerasWithAiCount() > 0
                }

                StepCard {
                    title: I18n.t("4. Правила")
                    body: root.configuredRulesCount() > 0
                          ? I18n.t("Правила настроены.")
                          : I18n.t("Без правил будут общие детекции.")
                    ok: root.configuredRulesCount() > 0
                    warning: root.configuredRulesCount() === 0
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: modulesGrid.implicitHeight + 24
                radius: Theme.radiusLg
                color: Theme.cardBackground
                border.color: Theme.cardBorder

                ColumnLayout {
                    id: modulesGrid
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Text {
                        text: I18n.t("Состояние AI-модулей")
                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.bold: true
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width >= 960 ? 3 : 1
                        columnSpacing: 8
                        rowSpacing: 8

                        Repeater {
                            model: [0, 1, 2]

                            delegate: Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: 84
                                radius: Theme.radiusMd
                                color: Theme.panelSoftBackground
                                border.color: Theme.controlBorder

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 4

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Text {
                                            Layout.fillWidth: true
                                            text: root.moduleName(modelData)
                                            color: Theme.textPrimary
                                            font.pixelSize: 13
                                            font.bold: true
                                        }
                                        Text {
                                            text: root.moduleEnabled(modelData) ? I18n.t("Вкл") : I18n.t("Выкл")
                                            color: root.moduleEnabled(modelData) ? Theme.success : Theme.textMuted
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: root.moduleStatusLabel(SystemController.analyticsEngine.getModuleStatus(modelData))
                                              + " · "
                                              + I18n.t("камер: %1", [root.assignedCamerasForModule(modelData)])
                                        color: Theme.textMuted
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: I18n.t("Кадры: %1 · Детекции: %2", [
                                            SystemController.analyticsEngine.getModuleTelemetry(modelData).processedFrames || 0,
                                            SystemController.analyticsEngine.getModuleTelemetry(modelData).detections || 0
                                        ])
                                        color: Theme.textFaint
                                        font.pixelSize: 10
                                        elide: Text.ElideRight
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
