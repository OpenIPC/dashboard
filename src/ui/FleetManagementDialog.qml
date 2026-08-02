pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root

    modal: true
    title: I18n.t("Сайты и парк камер")
    width: Math.min(1220, parent ? parent.width - 32 : 1220)
    height: Math.min(790, parent ? parent.height - 32 : 790)
    x: parent ? Math.max(0, (parent.width - width) / 2) : 0
    y: parent ? Math.max(0, (parent.height - height) / 2) : 0
    closePolicy: Popup.CloseOnEscape
    padding: 0

    property var fleet: SystemController.fleetManager
    property var inventoryRows: []
    property var selectedCameraIds: []
    property string selectedCameraId: ""
    property string selectedSiteId: ""
    property string selectedAreaId: ""
    property string selectedBaselineId: ""
    property var lastPreflight: ({})
    property string notice: ""
    property bool noticeError: false
    readonly property bool contentLayoutReady: width <= (parent ? parent.width : width)
                                               && height <= (parent ? parent.height : height)

    component FleetButton: DashboardDialogButton {
        property bool primary: false
        property bool destructive: false

        leftPadding: 14
        rightPadding: 14
        buttonColor: primary ? Theme.metroBlue
                             : (destructive ? Theme.dangerSurface : Theme.metroTile)
        buttonHoverColor: primary ? Theme.metroBlueHover
                                  : (destructive ? Theme.dangerSurfacePressed : Theme.metroTileHover)
        buttonBorderColor: primary ? Theme.metroBlue
                                   : (destructive ? Theme.metroRed : Theme.metroStroke)
        buttonTextBold: primary || destructive
    }

    component FleetTextField: TextField {
        id: field

        implicitHeight: 36
        leftPadding: 11
        rightPadding: 11
        selectByMouse: true
        hoverEnabled: true
        color: field.enabled ? Theme.textPrimary : Theme.textFaint
        placeholderTextColor: Theme.textFaint
        selectionColor: Theme.metroBlue
        selectedTextColor: Theme.textPrimary
        font.family: Theme.metroFontFamily

        background: Rectangle {
            color: field.enabled ? Theme.controlBackground : Theme.metroTileDisabled
            radius: Theme.metroTileRadius
            border.color: field.activeFocus ? Theme.metroStrokeStrong
                                              : (field.hovered ? Theme.textFaint : Theme.metroStroke)
            border.width: field.activeFocus ? 2 : 1
        }
    }

    component FleetTabButton: TabButton {
        id: tabButton

        implicitHeight: 40
        padding: 0
        hoverEnabled: true
        focusPolicy: Qt.StrongFocus

        background: Rectangle {
            color: tabButton.checked ? Theme.metroTile
                                     : (tabButton.hovered ? Theme.metroTileHover : Theme.metroSurfaceAlt)
            border.color: tabButton.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke
            border.width: 1

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: tabButton.checked ? 3 : 0
                color: Theme.metroBlue
            }
        }

        contentItem: Text {
            text: tabButton.text
            color: tabButton.checked ? Theme.textPrimary : Theme.textSecondary
            font.family: Theme.metroFontFamily
            font.pixelSize: 12
            font.bold: tabButton.checked
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component FleetTextArea: TextArea {
        id: textArea

        leftPadding: 11
        rightPadding: 11
        topPadding: 9
        bottomPadding: 9
        selectByMouse: true
        color: textArea.enabled ? Theme.textSecondary : Theme.textFaint
        placeholderTextColor: Theme.textFaint
        selectionColor: Theme.metroBlue
        selectedTextColor: Theme.textPrimary
        font.family: Theme.metroFontFamily

        background: Rectangle {
            color: Theme.controlBackground
            radius: Theme.metroTileRadius
            border.color: textArea.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
            border.width: textArea.activeFocus ? 2 : 1
        }
    }

    component FleetProgressBar: ProgressBar {
        id: progress

        implicitHeight: 10
        background: Rectangle {
            implicitHeight: 8
            color: Theme.controlBackground
            radius: Theme.metroTileRadius
            border.color: Theme.metroStroke
        }
        contentItem: Item {
            implicitHeight: 8
            Rectangle {
                width: parent.width * progress.visualPosition
                height: parent.height
                radius: Theme.metroTileRadius
                color: Theme.metroBlue
            }
        }
    }

    component FleetPromptDialog: Dialog {
        id: prompt

        modal: true
        padding: 16
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            color: Theme.metroSidebarBackground
            radius: Theme.metroTileRadius
            border.color: Theme.metroStroke
            border.width: 1
        }

        header: Rectangle {
            width: prompt.width
            implicitWidth: prompt.width
            implicitHeight: 46
            color: Theme.metroSurface

            Text {
                anchors.left: parent.left
                anchors.right: promptClose.left
                anchors.leftMargin: 16
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: prompt.title
                color: Theme.textPrimary
                font.family: Theme.metroFontFamily
                font.pixelSize: 15
                font.bold: true
                elide: Text.ElideRight
            }

            MetroWindowButton {
                id: promptClose
                anchors.top: parent.top
                anchors.right: parent.right
                width: 42
                height: parent.height
                kind: "close"
                onClicked: prompt.close()
            }
        }
    }

    function siteOptions(includeAll) {
        var result = []
        if (includeAll) result[result.length] = { text: I18n.t("Все сайты"), value: "" }
        for (var i = 0; i < fleet.sites.length; ++i)
            result[result.length] = { text: fleet.sites[i].name, value: fleet.sites[i].id }
        return result
    }

    function siteSummary(siteId) {
        var total = 0
        var offline = 0
        var drift = 0
        var rows = fleet.inventory
        for (var i = 0; i < rows.length; ++i) {
            if (rows[i].siteId !== siteId) continue
            total++
            if (!rows[i].online) offline++
            if (rows[i].firmwareDrift || Number(rows[i].driftCount || 0) > 0) drift++
        }
        return I18n.t("%1 камер · %2 офлайн · %3 дрейф", [total, offline, drift])
    }

    function areaOptions(siteId, includeAll) {
        var result = []
        if (includeAll) result[result.length] = { text: I18n.t("Все зоны"), value: "" }
        for (var i = 0; i < fleet.areas.length; ++i) {
            var area = fleet.areas[i]
            if (!siteId || area.siteId === siteId)
                result[result.length] = { text: area.name, value: area.id }
        }
        return result
    }

    function currentFilters() {
        return {
            search: inventorySearch.text,
            siteId: filterSite.currentValue || "",
            areaId: filterArea.currentValue || "",
            tag: filterTag.text,
            model: filterModel.text,
            offlineOnly: offlineOnly.checked,
            firmwareDrift: firmwareDriftOnly.checked,
            driftOnly: configDriftOnly.checked,
            maintenanceState: maintenanceOnly.checked ? "maintenance" : ""
        }
    }

    function refreshInventory() {
        fleet.refreshInventory()
        inventoryRows = fleet.filterInventory(currentFilters())
    }

    function isSelected(cameraId) {
        return selectedCameraIds.indexOf(cameraId) >= 0
    }

    function toggleSelected(cameraId, checked) {
        var copy = selectedCameraIds.slice(0)
        var index = copy.indexOf(cameraId)
        if (checked && index < 0) copy.push(cameraId)
        else if (!checked && index >= 0) copy.splice(index, 1)
        selectedCameraIds = copy
        selectedCameraId = checked ? cameraId : (copy.length ? copy[copy.length - 1] : "")
    }

    function selectVisible(checked) {
        var copy = []
        if (checked) {
            for (var i = 0; i < inventoryRows.length; ++i) copy.push(inventoryRows[i].cameraId)
        }
        selectedCameraIds = copy
        selectedCameraId = copy.length ? copy[copy.length - 1] : ""
    }

    function showResult(ok, successText) {
        noticeError = !ok
        notice = ok ? successText : I18n.t(fleet.lastError || "Operation failed")
        noticeTimer.restart()
    }

    function formatPreflight(report) {
        var output = I18n.t("Камер: %1 · блокировок: %2 · предупреждений: %3", [
                                report.deviceCount || 0,
                                report.blockerCount || 0,
                                report.warningCount || 0
                            ]) + "\n\n"
        var devices = report.devices || []
        for (var i = 0; i < devices.length; ++i) {
            var device = devices[i]
            output += (device.ready ? "✓ " : "✕ ") + (device.name || device.ip || device.cameraId) + "\n"
            var blockers = device.blockers || []
            for (var b = 0; b < blockers.length; ++b) output += "  • " + I18n.t(blockers[b]) + "\n"
            var warnings = device.warnings || []
            for (var w = 0; w < warnings.length; ++w) output += "  ⚠ " + I18n.t(warnings[w]) + "\n"
        }
        output += "\n" + I18n.t(report.recoveryGuidance || "")
        return output
    }

    function applySavedView(viewId) {
        for (var i = 0; i < fleet.savedViews.length; ++i) {
            var view = fleet.savedViews[i]
            if (view.id !== viewId) continue
            var filters = view.filters || {}
            inventorySearch.text = filters.search || ""
            filterTag.text = filters.tag || ""
            filterModel.text = filters.model || ""
            offlineOnly.checked = filters.offlineOnly === true
            firmwareDriftOnly.checked = filters.firmwareDrift === true
            configDriftOnly.checked = filters.driftOnly === true
            maintenanceOnly.checked = filters.maintenanceState === "maintenance"
            for (var s = 0; s < filterSite.count; ++s) {
                if (filterSite.model[s].value === (filters.siteId || "")) filterSite.currentIndex = s
            }
            for (var a = 0; a < filterArea.count; ++a) {
                if (filterArea.model[a].value === (filters.areaId || "")) filterArea.currentIndex = a
            }
            refreshInventory()
            return
        }
    }

    onOpened: {
        notice = ""
        noticeError = false
        refreshInventory()
    }

    Timer {
        id: noticeTimer
        interval: 5000
        onTriggered: {
            root.notice = ""
            root.noticeError = false
        }
    }

    Connections {
        target: root.fleet
        function onInventoryChanged() { root.inventoryRows = root.fleet.filterInventory(root.currentFilters()) }
        function onOperationMessage(message) {
            root.noticeError = false
            root.notice = I18n.t(message)
            noticeTimer.restart()
        }
        function onBatchResultsChanged() {
            root.noticeError = false
            root.notice = I18n.t("Выполнено: %1 из %2", [root.fleet.batchState.completed || 0,
                                                         root.fleet.batchState.total || 0])
            noticeTimer.restart()
        }
    }

    background: Rectangle {
        color: Theme.metroSidebarBackground
        radius: Theme.metroTileRadius
        border.color: Theme.metroStroke
        border.width: 1
    }

    header: Rectangle {
        x: 0
        width: root.width
        implicitWidth: root.width
        implicitHeight: 56
        color: Theme.metroSurface
        radius: Theme.metroTileRadius

        Column {
            anchors.left: parent.left
            anchors.right: closeButton.left
            anchors.leftMargin: 20
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Text {
                width: parent.width
                text: root.title
                color: Theme.textPrimary
                font.family: Theme.metroFontFamily
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
            }
            Text {
                width: parent.width
                text: I18n.t("Инвентаризация, конфигурации и безопасные пакетные операции")
                color: Theme.textMuted
                font.family: Theme.metroFontFamily
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }

        MetroWindowButton {
            id: closeButton
            kind: "close"
            width: 46
            height: parent.height
            anchors.top: parent.top
            anchors.right: parent.right
            onClicked: root.close()
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        TabBar {
            id: tabs
            Layout.fillWidth: true
            background: Rectangle { color: Theme.metroSurfaceAlt }
            FleetTabButton { text: I18n.t("Инвентарь") }
            FleetTabButton { text: I18n.t("Сайты и зоны") }
            FleetTabButton { text: I18n.t("Эталоны") }
            FleetTabButton { text: I18n.t("Операции") }
            FleetTabButton { text: I18n.t("Импорт и экспорт") }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            currentIndex: tabs.currentIndex

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 7

                    GridLayout {
                        Layout.fillWidth: true
                        columns: root.width > 1050 ? 5 : (root.width > 760 ? 3 : 2)
                        columnSpacing: 8
                        rowSpacing: 6

                        FleetTextField {
                            id: inventorySearch
                            Layout.fillWidth: true
                            placeholderText: I18n.t("Имя, IP, модель или тег")
                            onTextChanged: root.refreshInventory()
                        }
                        StyledComboBox {
                            id: filterSite
                            Layout.fillWidth: true
                            model: root.siteOptions(true)
                            textRole: "text"; valueRole: "value"
                            onCurrentValueChanged: { filterArea.model = root.areaOptions(currentValue, true); root.refreshInventory() }
                        }
                        StyledComboBox {
                            id: filterArea
                            Layout.fillWidth: true
                            model: root.areaOptions("", true)
                            textRole: "text"; valueRole: "value"
                            onCurrentValueChanged: root.refreshInventory()
                        }
                        FleetTextField {
                            id: filterTag
                            Layout.fillWidth: true
                            placeholderText: I18n.t("Тег")
                            onTextChanged: root.refreshInventory()
                        }
                        FleetTextField {
                            id: filterModel
                            Layout.fillWidth: true
                            placeholderText: I18n.t("Модель")
                            onTextChanged: root.refreshInventory()
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        MetroCheckBox { id: offlineOnly; text: I18n.t("Только офлайн"); onToggled: root.refreshInventory() }
                        MetroCheckBox { id: firmwareDriftOnly; text: I18n.t("Версии вне эталона"); onToggled: root.refreshInventory() }
                        MetroCheckBox { id: configDriftOnly; text: I18n.t("Дрейф конфигурации"); onToggled: root.refreshInventory() }
                        MetroCheckBox { id: maintenanceOnly; text: I18n.t("На обслуживании"); onToggled: root.refreshInventory() }
                        Item { Layout.fillWidth: true }
                        FleetButton { text: I18n.t("Обновить"); onClicked: root.refreshInventory() }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: I18n.t("Сохранённое представление"); color: Theme.textSecondary }
                        StyledComboBox { id: savedViewBox; Layout.fillWidth: true; model: root.fleet.savedViews; textRole: "name"; valueRole: "id" }
                        FleetButton { text: I18n.t("Применить"); enabled: savedViewBox.currentValue !== undefined && savedViewBox.currentValue !== ""; onClicked: root.applySavedView(savedViewBox.currentValue) }
                        FleetButton { text: I18n.t("Удалить"); destructive: true; enabled: savedViewBox.currentValue !== undefined && savedViewBox.currentValue !== ""; onClicked: root.showResult(root.fleet.removeSavedView(savedViewBox.currentValue), I18n.t("Представление удалено")) }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Theme.metroSurface
                        border.color: Theme.metroStroke

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0
                            Rectangle {
                                Layout.fillWidth: true; Layout.preferredHeight: 36
                                color: Theme.metroTile
                                RowLayout {
                                    anchors.fill: parent; anchors.leftMargin: 9; anchors.rightMargin: 9
                                    MetroCheckBox { checked: root.inventoryRows.length > 0 && root.selectedCameraIds.length === root.inventoryRows.length; onToggled: root.selectVisible(checked) }
                                    Text { Layout.preferredWidth: 210; text: I18n.t("Устройство"); color: Theme.textSecondary; font.bold: true }
                                    Text { Layout.preferredWidth: 170; text: I18n.t("Расположение"); color: Theme.textSecondary; font.bold: true }
                                    Text { Layout.fillWidth: true; text: I18n.t("Версии и состояние"); color: Theme.textSecondary; font.bold: true }
                                }
                            }
                            ListView {
                                id: inventoryList
                                Layout.fillWidth: true; Layout.fillHeight: true
                                clip: true; spacing: 2
                                model: root.inventoryRows
                                delegate: Rectangle {
                                    id: inventoryDelegate
                                    required property var modelData
                                    width: inventoryList.width; height: 58
                                    color: root.isSelected(modelData.cameraId) ? Theme.metroTileHover : Theme.controlBackground
                                    border.color: Theme.metroStroke
                                    RowLayout {
                                        anchors.fill: parent; anchors.margins: 8
                                        MetroCheckBox {
                                            checked: root.isSelected(inventoryDelegate.modelData.cameraId)
                                            onToggled: root.toggleSelected(inventoryDelegate.modelData.cameraId, checked)
                                        }
                                        ColumnLayout {
                                            Layout.preferredWidth: 200; spacing: 1
                                            Text { text: inventoryDelegate.modelData.name; color: Theme.textPrimary; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
                                            Text { text: inventoryDelegate.modelData.ip + "  ·  " + (inventoryDelegate.modelData.model || I18n.t("Модель не определена")); color: Theme.textMuted; font.pixelSize: 11; elide: Text.ElideRight; Layout.fillWidth: true }
                                        }
                                        ColumnLayout {
                                            Layout.preferredWidth: 160; spacing: 1
                                            Text { text: inventoryDelegate.modelData.siteName || I18n.t("Без сайта"); color: Theme.textSecondary; elide: Text.ElideRight; Layout.fillWidth: true }
                                            Text { text: inventoryDelegate.modelData.areaName || I18n.t("Без зоны"); color: Theme.textMuted; font.pixelSize: 11; elide: Text.ElideRight; Layout.fillWidth: true }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true; spacing: 1
                                            Text { text: (inventoryDelegate.modelData.firmwareVersion || "—") + " / " + (inventoryDelegate.modelData.majesticVersion || "—"); color: inventoryDelegate.modelData.firmwareDrift ? Theme.metroOrange : Theme.textSecondary; elide: Text.ElideRight; Layout.fillWidth: true }
                                            Text { text: I18n.t(inventoryDelegate.modelData.online ? "Онлайн" : "Офлайн") + "  ·  " + (inventoryDelegate.modelData.healthStatus || "—") + "  ·  " + I18n.t("дрейф: %1", [inventoryDelegate.modelData.driftCount || 0]); color: inventoryDelegate.modelData.online ? Theme.metroGreen : Theme.metroRed; font.pixelSize: 11; elide: Text.ElideRight; Layout.fillWidth: true }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: I18n.t("Выбрано: %1", [root.selectedCameraIds.length]); color: Theme.textSecondary }
                        Item { Layout.fillWidth: true }
                        FleetButton { text: I18n.t("Сохранить представление"); onClicked: savedViewDialog.open() }
                        FleetButton { text: I18n.t("Назначить расположение"); primary: true; enabled: root.selectedCameraId !== ""; onClicked: assignmentDialog.open() }
                        FleetButton { text: I18n.t("Экспорт инвентаря"); onClicked: inventoryExportDialog.open() }
                    }
                }
            }

            Item {
                RowLayout {
                    anchors.fill: parent
                    spacing: 10
                    Rectangle {
                        Layout.fillHeight: true; Layout.preferredWidth: 330
                        color: Theme.metroSurface; border.color: Theme.metroStroke
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 10
                            Text { text: I18n.t("Сайты"); color: Theme.textPrimary; font.pixelSize: 16; font.bold: true }
                            ListView {
                                Layout.fillWidth: true; Layout.fillHeight: true
                                model: root.fleet.sites; clip: true; spacing: 4
                                delegate: FleetButton {
                                    required property var modelData
                                    width: ListView.view.width; height: 44
                                    text: modelData.name + "\n" + root.siteSummary(modelData.id)
                                    onClicked: { root.selectedSiteId = modelData.id; root.selectedAreaId = "" }
                                }
                            }
                            FleetTextField { id: newSiteName; Layout.fillWidth: true; placeholderText: I18n.t("Название нового сайта") }
                            FleetButton {
                                Layout.fillWidth: true; text: I18n.t("Создать сайт")
                                primary: true
                                onClicked: { var id = root.fleet.createSite(newSiteName.text); root.showResult(id !== "", I18n.t("Сайт создан")); if (id) { root.selectedSiteId = id; newSiteName.clear() } }
                            }
                            FleetButton {
                                Layout.fillWidth: true; text: I18n.t("Удалить выбранный сайт")
                                destructive: true
                                enabled: root.selectedSiteId !== ""
                                onClicked: { deleteTopologyDialog.targetType = "site"; deleteTopologyDialog.targetId = root.selectedSiteId; deleteTopologyDialog.open() }
                            }
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        color: Theme.metroSurface; border.color: Theme.metroStroke
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 14; spacing: 10
                            Text { text: I18n.t("Зоны и политика сайта"); color: Theme.textPrimary; font.pixelSize: 16; font.bold: true }
                            Text { text: root.selectedSiteId ? I18n.t("Выбран сайт: %1", [root.selectedSiteId]) : I18n.t("Выберите сайт слева"); color: Theme.textSecondary; elide: Text.ElideRight; Layout.fillWidth: true }
                            ListView {
                                Layout.fillWidth: true; Layout.preferredHeight: 130; clip: true; spacing: 3
                                model: root.areaOptions(root.selectedSiteId, false)
                                delegate: FleetButton {
                                    required property var modelData
                                    width: ListView.view.width; height: 34; text: modelData.text
                                    onClicked: root.selectedAreaId = modelData.value
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                FleetTextField { id: newAreaName; Layout.fillWidth: true; placeholderText: I18n.t("Название новой зоны") }
                                FleetButton { text: I18n.t("Добавить зону"); primary: true; enabled: root.selectedSiteId !== ""; onClicked: { var id = root.fleet.createArea(root.selectedSiteId, newAreaName.text); root.showResult(id !== "", I18n.t("Зона создана")); if (id) newAreaName.clear() } }
                                FleetButton { text: I18n.t("Удалить зону"); destructive: true; enabled: root.selectedAreaId !== ""; onClicked: { deleteTopologyDialog.targetType = "area"; deleteTopologyDialog.targetId = root.selectedAreaId; deleteTopologyDialog.open() } }
                            }
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.metroStroke }
                            Text { text: I18n.t("Эталон версий"); color: Theme.textPrimary; font.bold: true }
                            RowLayout {
                                Layout.fillWidth: true
                                FleetTextField { id: firmwareBaseline; Layout.fillWidth: true; placeholderText: I18n.t("Версия прошивки") }
                                FleetTextField { id: majesticBaseline; Layout.fillWidth: true; placeholderText: I18n.t("Версия Majestic") }
                                FleetButton { text: I18n.t("Сохранить"); primary: true; enabled: root.selectedSiteId !== ""; onClicked: root.showResult(root.fleet.setSiteVersionBaseline(root.selectedSiteId, firmwareBaseline.text, majesticBaseline.text), I18n.t("Эталон версий сохранён")) }
                            }
                            Text { text: I18n.t("Окно обслуживания"); color: Theme.textPrimary; font.bold: true }
                            RowLayout {
                                Layout.fillWidth: true
                                MetroCheckBox { id: maintenanceEnabled; text: I18n.t("Включено") }
                                FleetTextField { id: maintenanceStart; text: "02:00"; Layout.preferredWidth: 90; placeholderText: "HH:mm" }
                                Text { text: "—"; color: Theme.textMuted }
                                FleetTextField { id: maintenanceEnd; text: "04:00"; Layout.preferredWidth: 90; placeholderText: "HH:mm" }
                                FleetTextField { id: maintenanceDays; text: "1,2,3,4,5,6,7"; Layout.fillWidth: true; placeholderText: I18n.t("Дни: 1–7") }
                                FleetButton {
                                    text: I18n.t("Применить"); enabled: root.selectedSiteId !== ""
                                    onClicked: {
                                        root.showResult(root.fleet.setSiteMaintenanceWindow(root.selectedSiteId, { enabled: maintenanceEnabled.checked, start: maintenanceStart.text, end: maintenanceEnd.text, days: maintenanceDays.text.split(",") }), I18n.t("Окно обслуживания сохранено"))
                                    }
                                }
                            }
                            Item { Layout.fillHeight: true }
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent; spacing: 8
                    RowLayout {
                        Layout.fillWidth: true
                        FleetTextField { id: baselineName; Layout.fillWidth: true; placeholderText: I18n.t("Название эталона") }
                        StyledComboBox { id: baselineCamera; Layout.preferredWidth: 280; model: root.inventoryRows; textRole: "name"; valueRole: "cameraId" }
                        FleetButton { text: I18n.t("Снять эталон с камеры"); primary: true; onClicked: { var requestId = root.fleet.captureBaselineFromCamera(baselineCamera.currentValue || "", baselineName.text); root.showResult(requestId !== "", I18n.t("Чтение конфигурации запущено")) } }
                    }
                    RowLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        ListView {
                            id: baselineList
                            Layout.preferredWidth: 360; Layout.fillHeight: true; clip: true; spacing: 4
                            model: root.fleet.baselines
                            delegate: FleetButton {
                                required property var modelData
                                width: ListView.view.width; height: 52
                                text: modelData.name
                                onClicked: { root.selectedBaselineId = modelData.id; driftText.text = JSON.stringify(root.fleet.driftPreview(modelData.id, root.selectedCameraIds), null, 2) }
                            }
                        }
                        FleetTextArea {
                            id: driftText
                            Layout.fillWidth: true; Layout.fillHeight: true
                            readOnly: true; wrapMode: TextEdit.Wrap; color: Theme.textSecondary
                            text: I18n.t("Выберите эталон для безопасного просмотра расхождений. Секреты автоматически скрываются.")
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent; spacing: 10
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Text { text: I18n.t("Операция"); color: Theme.textSecondary }
                        StyledComboBox {
                            id: operationBox; Layout.fillWidth: true; textRole: "text"; valueRole: "value"
                            model: [
                                { text: I18n.t("Обновить инвентарь"), value: "inventory" },
                                { text: I18n.t("Проверить здоровье"), value: "health" },
                                { text: I18n.t("Прочитать конфигурацию"), value: "configuration-read" },
                                { text: I18n.t("Применить эталон"), value: "apply-baseline" }
                            ]
                        }
                        Text { text: I18n.t("Параллельность"); color: Theme.textSecondary }
                        SettingsSpinBox { id: concurrencyBox; from: 1; to: 8; value: 2 }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        MetroCheckBox { id: dryRunCheck; text: I18n.t("Только проверка (dry-run)"); checked: true }
                        MetroCheckBox { id: backupCheck; text: I18n.t("Резервная копия перед изменением"); checked: true }
                        MetroCheckBox { id: maintenanceOverrideCheck; text: I18n.t("Разрешить вне окна обслуживания") }
                        Item { Layout.fillWidth: true }
                        Text { text: I18n.t("Эталон конфигурации"); color: Theme.textSecondary }
                        StyledComboBox { id: operationBaseline; Layout.preferredWidth: 250; model: root.fleet.baselines; textRole: "name"; valueRole: "id" }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        FleetTextField { id: backupPath; Layout.fillWidth: true; readOnly: true; placeholderText: I18n.t("Каталог резервных копий") }
                        FleetButton { text: I18n.t("Выбрать…"); onClicked: backupFolderDialog.open() }
                        FleetButton { text: I18n.t("Предварительная проверка"); enabled: root.selectedCameraIds.length > 0; onClicked: { root.lastPreflight = root.fleet.preflightBatch(operationBox.currentValue, root.selectedCameraIds, root.batchOptions()); preflightText.text = root.formatPreflight(root.lastPreflight) } }
                        FleetButton { text: I18n.t("Запустить"); primary: true; enabled: root.selectedCameraIds.length > 0 && !root.fleet.batchState.running; onClicked: root.showResult(root.fleet.startBatch(operationBox.currentValue, root.selectedCameraIds, root.batchOptions()), I18n.t("Пакетная операция запущена")) }
                        FleetButton { text: I18n.t("Отмена"); destructive: true; enabled: root.fleet.batchState.running === true; onClicked: root.fleet.cancelBatch() }
                    }
                    FleetProgressBar {
                        Layout.fillWidth: true
                        from: 0; to: Math.max(1, root.fleet.batchState.total || 1)
                        value: root.fleet.batchState.completed || 0
                    }
                    RowLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        FleetTextArea {
                            id: preflightText
                            Layout.fillWidth: true; Layout.fillHeight: true; readOnly: true; wrapMode: TextEdit.Wrap
                            color: Theme.textSecondary
                            text: I18n.t("Выберите камеры в Инвентаре, затем выполните предварительную проверку.")
                        }
                        ListView {
                            Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 4
                            model: root.fleet.batchResults
                            delegate: Rectangle {
                                id: batchResultDelegate
                                required property var modelData
                                width: ListView.view.width; height: 58
                                color: Theme.controlBackground; border.color: Theme.metroStroke
                                ColumnLayout {
                                    anchors.fill: parent; anchors.margins: 7; spacing: 1
                                    Text { Layout.fillWidth: true; text: batchResultDelegate.modelData.name + " · " + I18n.t(batchResultDelegate.modelData.status); color: Theme.textPrimary; font.bold: true; elide: Text.ElideRight }
                                    Text { Layout.fillWidth: true; text: I18n.t(batchResultDelegate.modelData.message); color: Theme.textMuted; font.pixelSize: 11; elide: Text.ElideRight }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 14
                    Text { text: I18n.t("Перенос структуры между серверами"); color: Theme.textPrimary; font.pixelSize: 17; font.bold: true }
                    Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: I18n.t("Файл содержит сайты, зоны, назначения, представления и безопасные эталоны. Пароли, токены и другие секреты не экспортируются."); color: Theme.textSecondary }
                    RowLayout {
                        Layout.fillWidth: true
                        FleetButton { text: I18n.t("Экспортировать структуру"); primary: true; onClicked: siteExportDialog.open() }
                        FleetButton { text: I18n.t("Предпросмотр импорта"); onClicked: siteImportDialog.open() }
                        Item { Layout.fillWidth: true }
                        FleetButton { text: I18n.t("Экспорт диагностики выбранных"); enabled: root.selectedCameraIds.length > 0; onClicked: diagnosticsExportDialog.open() }
                    }
                    FleetTextArea {
                        id: importPreviewText
                        Layout.fillWidth: true; Layout.fillHeight: true; readOnly: true; wrapMode: TextEdit.Wrap
                        color: Theme.textSecondary
                        text: I18n.t("Сначала выберите файл для проверки конфликтов.")
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.bottomMargin: 10
            Layout.preferredHeight: visible ? 34 : 0
            visible: root.notice !== ""
            color: root.noticeError ? Theme.dangerSurface : Theme.infoSurface
            radius: Theme.metroTileRadius
            border.color: root.noticeError ? Theme.metroRed : Theme.metroStrokeStrong

            Text {
                anchors.fill: parent
                anchors.leftMargin: 11
                anchors.rightMargin: 11
                text: root.notice
                color: Theme.textPrimary
                font.family: Theme.metroFontFamily
                font.pixelSize: 11
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    }

    function batchOptions() {
        return {
            dryRun: dryRunCheck.checked,
            concurrency: concurrencyBox.value,
            baselineId: operationBaseline.currentValue || "",
            backupBeforeChange: backupCheck.checked,
            backupDirectory: backupPath.text,
            maintenanceOverride: maintenanceOverrideCheck.checked
        }
    }

    FleetPromptDialog {
        id: assignmentDialog
        title: I18n.t("Назначение камеры")
        modal: true; width: 440; height: 300
        x: (root.width - width) / 2; y: (root.height - height) / 2
        contentItem: ColumnLayout {
            StyledComboBox { id: assignmentSite; Layout.fillWidth: true; model: root.siteOptions(true); textRole: "text"; valueRole: "value"; onCurrentValueChanged: assignmentArea.model = root.areaOptions(currentValue, true) }
            StyledComboBox { id: assignmentArea; Layout.fillWidth: true; model: root.areaOptions("", true); textRole: "text"; valueRole: "value" }
            FleetTextField { id: assignmentTags; Layout.fillWidth: true; placeholderText: I18n.t("Теги через запятую") }
            StyledComboBox { id: assignmentState; Layout.fillWidth: true; textRole: "text"; valueRole: "value"; model: [{text:I18n.t("Активна"),value:"active"},{text:I18n.t("На обслуживании"),value:"maintenance"},{text:I18n.t("Выведена из эксплуатации"),value:"retired"}] }
            Item { Layout.fillHeight: true }
            RowLayout {
                Layout.fillWidth: true; Item { Layout.fillWidth: true }
                FleetButton { text: I18n.t("Отмена"); onClicked: assignmentDialog.close() }
                FleetButton { text: I18n.t("Сохранить"); primary: true; onClicked: { var tags = assignmentTags.text ? assignmentTags.text.split(",") : []; root.showResult(root.fleet.assignCamera(root.selectedCameraId, assignmentSite.currentValue || "", assignmentArea.currentValue || "", tags, assignmentState.currentValue), I18n.t("Назначение сохранено")); assignmentDialog.close(); root.refreshInventory() } }
            }
        }
    }

    FleetPromptDialog {
        id: savedViewDialog
        title: I18n.t("Сохранить представление"); modal: true; width: 400; height: 180
        x: (root.width - width) / 2; y: (root.height - height) / 2
        contentItem: ColumnLayout {
            FleetTextField { id: savedViewName; Layout.fillWidth: true; placeholderText: I18n.t("Название представления") }
            Item { Layout.fillHeight: true }
            FleetButton { Layout.alignment: Qt.AlignRight; text: I18n.t("Сохранить"); primary: true; onClicked: { var id = root.fleet.createSavedView(savedViewName.text, root.currentFilters()); root.showResult(id !== "", I18n.t("Представление сохранено")); savedViewDialog.close() } }
        }
    }

    FolderDialog { id: backupFolderDialog; title: I18n.t("Каталог резервных копий"); onAccepted: backupPath.text = selectedFolder }
    FileDialog { id: inventoryExportDialog; title: I18n.t("Экспорт инвентаря"); fileMode: FileDialog.SaveFile; nameFilters: ["CSV (*.csv)", "JSON (*.json)"]; onAccepted: root.showResult(root.fleet.exportInventory(selectedFile, root.currentFilters()), I18n.t("Инвентарь экспортирован")) }
    FileDialog { id: diagnosticsExportDialog; title: I18n.t("Экспорт диагностики"); fileMode: FileDialog.SaveFile; nameFilters: ["JSON (*.json)"]; onAccepted: root.showResult(root.fleet.exportDiagnostics(selectedFile, root.selectedCameraIds), I18n.t("Диагностика экспортирована")) }
    FileDialog { id: siteExportDialog; title: I18n.t("Экспорт структуры сайтов"); fileMode: FileDialog.SaveFile; nameFilters: ["JSON (*.json)"]; onAccepted: root.showResult(root.fleet.exportSiteDefinitions(selectedFile), I18n.t("Структура экспортирована")) }
    FileDialog {
        id: siteImportDialog; title: I18n.t("Импорт структуры сайтов"); fileMode: FileDialog.OpenFile; nameFilters: ["JSON (*.json)"]
        onAccepted: {
            var preview = root.fleet.previewSiteImport(selectedFile)
            importPreviewText.text = JSON.stringify(preview, null, 2)
            if (!preview.valid || preview.containsCredentials) {
                root.noticeError = true
                root.notice = I18n.t(preview.containsCredentials
                                     ? "Site definition contains credential fields"
                                     : (root.fleet.lastError || "Unsupported site definition format"))
                noticeTimer.restart()
                return
            }
            importConfirmDialog.importFile = selectedFile
            importConfirmDialog.open()
        }
    }
    FleetPromptDialog {
        id: importConfirmDialog
        property url importFile
        title: I18n.t("Подтверждение импорта"); modal: true; width: 450; height: 200
        x: (root.width - width) / 2; y: (root.height - height) / 2
        contentItem: ColumnLayout {
            Text { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: I18n.t("Импортировать структуру? Совпадающие идентификаторы и названия можно безопасно объединить."); color: Theme.textSecondary }
            MetroCheckBox { id: mergeImport; text: I18n.t("Объединить конфликты") }
            Item { Layout.fillHeight: true }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                FleetButton { text: I18n.t("Отмена"); onClicked: importConfirmDialog.close() }
                FleetButton {
                    text: I18n.t("Импортировать")
                    primary: true
                    onClicked: {
                        root.showResult(root.fleet.importSiteDefinitions(importConfirmDialog.importFile,
                                                                         mergeImport.checked),
                                        I18n.t("Структура импортирована"))
                        importConfirmDialog.close()
                        root.refreshInventory()
                    }
                }
            }
        }
    }

    FleetPromptDialog {
        id: deleteTopologyDialog
        property string targetType: ""
        property string targetId: ""
        title: I18n.t("Подтверждение удаления")
        modal: true; width: 450; height: 190
        x: (root.width - width) / 2; y: (root.height - height) / 2
        contentItem: ColumnLayout {
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.textSecondary
                text: I18n.t(deleteTopologyDialog.targetType === "site"
                             ? "Удалить сайт? Его зоны будут удалены, а камеры останутся без назначения."
                             : "Удалить зону? Камеры останутся в сайте без зоны.")
            }
            Item { Layout.fillHeight: true }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                FleetButton { text: I18n.t("Отмена"); onClicked: deleteTopologyDialog.close() }
                FleetButton {
                    text: I18n.t("Удалить")
                    destructive: true
                    onClicked: {
                        var ok = deleteTopologyDialog.targetType === "site"
                                ? root.fleet.removeSite(deleteTopologyDialog.targetId)
                                : root.fleet.removeArea(deleteTopologyDialog.targetId)
                        root.showResult(ok, I18n.t("Структура обновлена"))
                        if (ok && deleteTopologyDialog.targetType === "site") root.selectedSiteId = ""
                        if (ok) root.selectedAreaId = ""
                        deleteTopologyDialog.close()
                        root.refreshInventory()
                    }
                }
            }
        }
    }
}
