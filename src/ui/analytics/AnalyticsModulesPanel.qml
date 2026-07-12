import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root

    property int refreshToken: 0
    property string actionStatus: ""

    function refresh() {
        refreshToken += 1
    }

    function moduleName(type) {
        if (type === 0) return I18n.t("Лица")
        if (type === 1) return I18n.t("Объекты")
        if (type === 2) return I18n.t("Номера")
        return I18n.t("Модуль")
    }

    function moduleDescription(type) {
        if (type === 0) return I18n.t("Поиск лиц, сохранение снимков и события по появлению человека.")
        if (type === 1) return I18n.t("Обнаружение и счет объектов: люди, транспорт и другие классы модели.")
        if (type === 2) return I18n.t("Поиск и фиксация автомобильных номеров.")
        return ""
    }

    function moduleEnabled(type) {
        var token = refreshToken
        return SystemController.analyticsEngine.isModuleEnabled(type)
    }

    function moduleStatus(type) {
        var token = refreshToken
        return SystemController.analyticsEngine.getModuleStatus(type)
    }

    function moduleProgress(type) {
        var token = refreshToken
        return SystemController.analyticsEngine.getModuleProgress(type)
    }

    function statusLabel(status) {
        if (status === "ready") return I18n.t("Готов")
        if (status === "downloading") return I18n.t("Загрузка")
        if (status === "error") return I18n.t("Ошибка")
        return I18n.t("Не установлен")
    }

    function statusColor(status) {
        if (status === "ready") return Theme.success
        if (status === "downloading") return Theme.warning
        if (status === "error") return Theme.danger
        return Theme.textMuted
    }

    function moduleTelemetry(type) {
        var token = refreshToken
        return SystemController.analyticsEngine.getModuleTelemetry(type)
    }

    function moduleDiagnostics(type) {
        var token = refreshToken
        return SystemController.analyticsEngine.getModuleDiagnostics(type)
    }

    function moduleInventoryItem(type) {
        var token = refreshToken
        if (!SystemController.analyticsEngine.moduleInventory)
            return ({})

        var inventory = SystemController.analyticsEngine.moduleInventory()
        for (var i = 0; i < inventory.length; ++i) {
            if (Number(inventory[i].type) === Number(type))
                return inventory[i]
        }
        return ({})
    }

    function artifactStateText(inventory) {
        var state = inventory ? inventory.verificationState : ""
        if (state === "trusted") return I18n.t("Проверена")
        if (state === "size_ok") return I18n.t("Размер OK")
        if (state === "size_mismatch") return I18n.t("Размер не совпадает")
        return I18n.t("Нет файла")
    }

    function artifactStateColor(inventory) {
        var state = inventory ? inventory.verificationState : ""
        if (state === "trusted" || state === "size_ok") return Theme.success
        if (state === "size_mismatch") return Theme.danger
        return Theme.warning
    }

    function verifyModule(type) {
        if (!SystemController.analyticsEngine.verifyModuleArtifact)
            return
        var result = SystemController.analyticsEngine.verifyModuleArtifact(type)
        actionStatus = result.ok
            ? I18n.t("Модель «%1» прошла проверку SHA-256.", [moduleName(type)])
            : I18n.t("Проверка «%1» не пройдена: %2", [moduleName(type), result.message || I18n.t("нет деталей")])
        refresh()
    }

    function cleanupArtifacts(type) {
        if (!SystemController.analyticsEngine.cleanupModuleArtifacts)
            return
        var result = SystemController.analyticsEngine.cleanupModuleArtifacts(type)
        actionStatus = result.ok
            ? I18n.t("Очистка «%1»: удалено %2, освобождено %3.", [
                moduleName(type),
                result.removedCount || 0,
                sizeText(result.freedBytes || 0)
            ])
            : I18n.t("Очистка «%1» не выполнена: %2", [moduleName(type), result.message || I18n.t("нет деталей")])
        refresh()
    }

    function sizeText(bytes) {
        var value = Number(bytes || 0)
        if (value <= 0) return I18n.t("нет данных")
        if (value < 1024 * 1024) return Math.round(value / 1024) + " KB"
        return (value / (1024 * 1024)).toFixed(1) + " MB"
    }

    function dateText(ms) {
        var value = Number(ms || 0)
        if (value <= 0) return I18n.t("нет данных")
        return Qt.formatDateTime(new Date(value), "yyyy-MM-dd HH:mm")
    }

    function assignedCamerasCount(type) {
        var total = 0
        var token = refreshToken
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            var camera = SystemController.cameraModel.getCamera(i)
            if (SystemController.analyticsEngine.isCameraModuleEnabled(camera.cameraIp, type))
                total += 1
        }
        return total
    }

    function configuredRulesCount(type) {
        var token = refreshToken
        var cfg = SystemController.analyticsEngine.getModuleConfig(type)
        var rules = cfg && cfg.rules ? cfg.rules : []
        var total = 0
        for (var i = 0; i < rules.length; ++i) {
            if (!rules[i] || rules[i].enabled !== false)
                total += 1
        }
        return total
    }

    function toggleModule(type) {
        var next = !moduleEnabled(type)
        SystemController.analyticsEngine.setModuleEnabled(type, next)
        refresh()
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

    component Metric: Rectangle {
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

    component PanelButton: Button {
        property color normalColor: Theme.controlBackground
        property color hoverColor: Theme.cardHover
        property color textColor: Theme.textSecondary

        implicitHeight: 34
        leftPadding: 14
        rightPadding: 14
        hoverEnabled: true

        background: Rectangle {
            color: parent.enabled
                   ? (parent.hovered ? parent.hoverColor : parent.normalColor)
                   : Theme.controlBackgroundAlt
            radius: Theme.radiusSm
            border.color: parent.enabled ? Theme.controlBorder : Theme.panelBorder
        }

        contentItem: Text {
            text: parent.text
            color: parent.enabled ? parent.textColor : Theme.textMuted
            font.pixelSize: 12
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
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
                spacing: 4

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Модули ИИ")
                    color: Theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Сначала включите нужный модуль, дождитесь статуса «Готов», затем назначьте его камерам.")
                    color: Theme.textMuted
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 10
            model: [0, 1, 2]
            ScrollBar.vertical: StyledScrollBar {}

            delegate: Rectangle {
                id: moduleCard

                property int moduleType: modelData
                property string currentStatus: root.moduleStatus(moduleType)
                property bool currentEnabled: root.moduleEnabled(moduleType)
                property var telemetry: root.moduleTelemetry(moduleType)
                property var diagnostics: root.moduleDiagnostics(moduleType)
                property var inventory: root.moduleInventoryItem(moduleType)
                property real progress: root.moduleProgress(moduleType)
                property real displayProgress: currentStatus === "downloading"
                    ? Math.max(0, Math.min(0.99, Number(progress || 0)))
                    : Math.max(0, Math.min(1, Number(progress || 0)))

                width: ListView.view.width
                height: content.implicitHeight + 24
                radius: Theme.radiusLg
                color: Theme.cardBackground
                border.color: currentEnabled ? Theme.accent : Theme.cardBorder

                ColumnLayout {
                    id: content
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 12
                            Layout.preferredHeight: 12
                            radius: 6
                            color: root.statusColor(moduleCard.currentStatus)
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3

                            Text {
                                Layout.fillWidth: true
                                text: root.moduleName(moduleCard.moduleType)
                                color: Theme.textPrimary
                                font.pixelSize: 16
                                font.bold: true
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                text: root.moduleDescription(moduleCard.moduleType)
                                color: Theme.textMuted
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Text {
                            Layout.fillWidth: true
                            text: root.statusLabel(moduleCard.currentStatus)
                            color: root.statusColor(moduleCard.currentStatus)
                            font.pixelSize: 12
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        PanelButton {
                            Layout.preferredWidth: Math.min(180, Math.max(128, moduleCard.width * 0.22))
                            text: moduleCard.currentEnabled ? I18n.t("Отключить") : I18n.t("Включить")
                            normalColor: moduleCard.currentEnabled ? Theme.controlBackground : Theme.accent
                            hoverColor: moduleCard.currentEnabled ? Theme.cardHover : Theme.accentHover
                            textColor: Theme.textPrimary
                            onClicked: root.toggleModule(moduleCard.moduleType)
                        }

                        PanelButton {
                            Layout.preferredWidth: Math.min(180, Math.max(128, moduleCard.width * 0.22))
                            text: I18n.t("Перезагрузить")
                            onClicked: {
                                SystemController.analyticsEngine.reloadModule(moduleCard.moduleType)
                                root.refresh()
                            }
                        }

                        PanelButton {
                            Layout.preferredWidth: Math.min(180, Math.max(128, moduleCard.width * 0.22))
                            text: I18n.t("Папка моделей")
                            onClicked: SystemController.openFolder(moduleCard.diagnostics.modulesDir || moduleCard.diagnostics.modelPath)
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: moduleCard.width >= 860 ? 3 : 1
                        columnSpacing: 8
                        rowSpacing: 8

                        PanelButton {
                            Layout.fillWidth: true
                            text: I18n.t("Проверить SHA")
                            enabled: moduleCard.inventory.installed === true
                            onClicked: root.verifyModule(moduleCard.moduleType)
                        }

                        PanelButton {
                            Layout.fillWidth: true
                            text: I18n.t("Очистить хвосты")
                            enabled: moduleCard.inventory.partialExists === true || moduleCard.inventory.previousExists === true
                            onClicked: root.cleanupArtifacts(moduleCard.moduleType)
                        }

                        PanelButton {
                            Layout.fillWidth: true
                            text: I18n.t("Источник")
                            enabled: moduleCard.diagnostics.sourceUrl !== undefined && moduleCard.diagnostics.sourceUrl !== ""
                            onClicked: Qt.openUrlExternally(moduleCard.diagnostics.sourceUrl)
                        }
                    }

                    ProgressBar {
                        id: downloadProgress
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                        visible: moduleCard.currentStatus === "downloading"
                        from: 0
                        to: 1
                        value: moduleCard.displayProgress

                        Behavior on value {
                            enabled: moduleCard.displayProgress >= downloadProgress.value
                            NumberAnimation {
                                duration: 180
                                easing.type: Easing.OutCubic
                            }
                        }

                        background: Rectangle {
                            radius: Theme.radiusSm
                            color: Theme.controlBackground
                            border.color: Theme.controlBorder
                        }

                        contentItem: Item {
                            Rectangle {
                                width: downloadProgress.visualPosition * parent.width
                                height: parent.height
                                radius: Theme.radiusSm
                                color: Theme.accent
                            }

                            Text {
                                anchors.centerIn: parent
                                text: Math.round(downloadProgress.value * 100) + "%"
                                color: Theme.textPrimary
                                font.pixelSize: 11
                                font.bold: true
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: moduleCard.currentStatus === "error"
                        text: SystemController.analyticsEngine.getModuleError(moduleCard.moduleType)
                        color: Theme.danger
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: moduleCard.width >= 1120 ? 8 : moduleCard.width >= 820 ? 4 : moduleCard.width >= 620 ? 3 : 2
                        columnSpacing: 8
                        rowSpacing: 8

                        Metric {
                            title: I18n.t("Модель")
                            value: moduleCard.diagnostics.installed ? I18n.t("ONNX") : I18n.t("Нет файла")
                            accent: moduleCard.diagnostics.installed ? Theme.success : Theme.warning
                        }
                        Metric {
                            title: I18n.t("Размер")
                            value: root.sizeText(moduleCard.diagnostics.modelSizeBytes)
                        }
                        Metric {
                            title: I18n.t("Backend")
                            value: moduleCard.diagnostics.loaded ? I18n.t("Загружен") : I18n.t("Не загружен")
                            accent: moduleCard.diagnostics.loaded ? Theme.success : Theme.warning
                        }
                        Metric {
                            title: I18n.t("Камер")
                            value: String(root.assignedCamerasCount(moduleCard.moduleType))
                            accent: root.assignedCamerasCount(moduleCard.moduleType) > 0 ? Theme.success : Theme.warning
                        }
                        Metric {
                            title: I18n.t("Правил")
                            value: String(root.configuredRulesCount(moduleCard.moduleType))
                        }
                        Metric {
                            title: I18n.t("Кадры")
                            value: String(moduleCard.telemetry.processedFrames || 0)
                        }
                        Metric {
                            title: I18n.t("Детекции")
                            value: String(moduleCard.telemetry.detections || 0)
                        }
                        Metric {
                            title: I18n.t("Задержка")
                            value: Number(moduleCard.telemetry.averageInferenceMs || 0).toFixed(1) + " ms"
                        }
                        Metric {
                            title: I18n.t("Артефакт")
                            value: root.artifactStateText(moduleCard.inventory)
                            accent: root.artifactStateColor(moduleCard.inventory)
                        }
                        Metric {
                            title: I18n.t("Диск")
                            value: root.sizeText(moduleCard.inventory.storageBytes)
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: modelDetails.implicitHeight + 16
                        radius: Theme.radiusMd
                        color: Theme.panelSoftBackground
                        border.color: Theme.panelBorder

                        ColumnLayout {
                            id: modelDetails
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Файл модели: %1", [moduleCard.diagnostics.modelPath || I18n.t("Путь не задан")])
                                color: Theme.textMuted
                                font.pixelSize: 10
                                wrapMode: Text.WrapAnywhere
                            }

                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Классы: %1", [moduleCard.diagnostics.classCount || 0])
                                      + " · "
                                      + I18n.t("Confidence: %1", [Number(moduleCard.diagnostics.confidenceThreshold || 0).toFixed(2)])
                                      + " · "
                                      + I18n.t("NMS: %1", [Number(moduleCard.diagnostics.nmsThreshold || 0).toFixed(2)])
                                      + " · "
                                      + I18n.t("Изменен: %1", [root.dateText(moduleCard.diagnostics.lastModifiedMs)])
                                color: Theme.textMuted
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }

                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Целостность: %1 · ожидаемый размер: %2 · временные файлы: %3", [
                                    root.artifactStateText(moduleCard.inventory),
                                    root.sizeText(moduleCard.diagnostics.expectedModelSizeBytes),
                                    root.sizeText((moduleCard.inventory.partialBytes || 0) + (moduleCard.inventory.previousBytes || 0))
                                ])
                                color: root.artifactStateColor(moduleCard.inventory)
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: root.actionStatus !== ""
                        text: root.actionStatus
                        color: root.actionStatus.indexOf(I18n.t("не")) >= 0 ? Theme.warning : Theme.textMuted
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        Layout.fillWidth: true
                        text: !moduleCard.currentEnabled
                              ? I18n.t("Модуль выключен: его нельзя назначить камерам.")
                              : (moduleCard.currentStatus === "ready"
                                 ? I18n.t("Модуль готов: назначьте его камерам во вкладке «Камеры».")
                                 : I18n.t("Дождитесь готовности модуля перед назначением камерам."))
                        color: moduleCard.currentEnabled && moduleCard.currentStatus === "ready" ? Theme.success : Theme.textMuted
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
