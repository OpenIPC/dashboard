pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: sidebar

    property var dashboard: null
    property Item dragProxyItem: null
    property real sidebarWidth: 300
    property real sidebarOpenProgress: 1.0
    property int cameraDataVersion: 0
    property string deviceFilterText: ""
    property bool canSettings: false
    property var systemController: null

    signal closeSidebarRequested()
    signal noAccessRequested()
    signal searchRequested()
    signal healthRequested()
    signal addGroupRequested()
    signal addCameraRequested()
    signal analyticsRequested()
    signal settingsRequested()
    signal userManagementRequested()
    signal logsRequested()
    signal camexRequested()
    signal deviceContextRequested(string cameraIp, string cameraName, int cameraIndex)
    signal groupContextRequested(string groupName)
    signal addCameraToGridRequested(int cameraIndex)
    signal majesticRequested(int cameraIndex)
    signal cameraDataChangedRequested()
    signal deviceFilterChanged(string text)

    Layout.preferredWidth: sidebar.sidebarWidth
    Layout.fillHeight: true
    visible: true
    enabled: sidebar.sidebarOpenProgress > 0.01
    opacity: sidebar.sidebarOpenProgress
    clip: true
    color: Theme.topBarBackground
    radius: Theme.radiusLg
    border.color: Theme.panelBorderStrong
    border.width: 1

    function actionAllowed(action) {
        return dashboard && dashboard.actionAllowed ? dashboard.actionAllowed(action) : false
    }

    function cameraCount() {
        return dashboard && dashboard.cameraCount ? dashboard.cameraCount() : 0
    }

    function onlineCameraCount() {
        return dashboard && dashboard.onlineCameraCount ? dashboard.onlineCameraCount() : 0
    }

    function filteredCameraCount() {
        return dashboard && dashboard.filteredCameraCount ? dashboard.filteredCameraCount() : 0
    }

    function currentUsername() {
        return dashboard && dashboard.currentUsername ? dashboard.currentUsername() : ""
    }

    function smartStreamBudgetEnabled() {
        return dashboard && dashboard.smartStreamBudgetEnabled ? dashboard.smartStreamBudgetEnabled() : false
    }

    function activePreviewCount() {
        return dashboard && dashboard.activePreviewCount ? dashboard.activePreviewCount() : 0
    }

    function budgetPausedPreviewCount() {
        return dashboard && dashboard.budgetPausedPreviewCount ? dashboard.budgetPausedPreviewCount() : 0
    }

    function previewBudgetLimit() {
        return dashboard && dashboard.previewBudgetLimit ? dashboard.previewBudgetLimit() : 0
    }

    function isOnlineStatus(statusText) {
        return dashboard && dashboard.isOnlineStatus ? dashboard.isOnlineStatus(statusText) : statusText === "Online"
    }

    function effectiveCameraStatus(ip, fallbackStatus) {
        return dashboard && dashboard.effectiveCameraStatus ? dashboard.effectiveCameraStatus(ip, fallbackStatus) : fallbackStatus
    }

    function effectiveCameraDetail(ip, fallbackStatus) {
        return dashboard && dashboard.effectiveCameraDetail ? dashboard.effectiveCameraDetail(ip, fallbackStatus) : ""
    }

    function cameraStatusSearchText(ip, fallbackStatus) {
        return dashboard && dashboard.cameraStatusSearchText ? dashboard.cameraStatusSearchText(ip, fallbackStatus) : fallbackStatus
    }

    function cameraMatchesDeviceFilter(name, ip, statusText, groupName) {
        return dashboard && dashboard.cameraMatchesDeviceFilter
                ? dashboard.cameraMatchesDeviceFilter(name, ip, statusText, groupName)
                : true
    }

    function triggerAction(action) {
        if (!actionAllowed(action)) {
            noAccessRequested()
            return
        }

        if (action === "search") searchRequested()
        else if (action === "health") healthRequested()
        else if (action === "add_folder") addGroupRequested()
        else if (action === "add_camera") addCameraRequested()
        else if (action === "settings") settingsRequested()
        else if (action === "analytics") analyticsRequested()
        else if (action === "user") userManagementRequested()
        else if (action === "logs") logsRequested()
        else if (action === "logout" && sidebar.systemController) sidebar.systemController.userManager.logout()
        else if (action === "camex") camexRequested()
    }

    Rectangle {
        visible: sidebar.sidebarOpenProgress > 0.01
        width: 18
        height: 84
        radius: 9
        anchors.left: parent.left
        anchors.leftMargin: -9
        anchors.verticalCenter: parent.verticalCenter
        color: hideArea.containsMouse ? Theme.cardHover : Theme.cardBackground
        border.color: Theme.controlBorderStrong
        z: 6

        Text {
            anchors.centerIn: parent
            text: "»"
            color: Theme.textSecondary
            font.pixelSize: 16
        }

        MouseArea {
            id: hideArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: sidebar.closeSidebarRequested()
        }
    }

    ScrollView {
        id: sidebarScrollView
        anchors.fill: parent
        visible: sidebar.sidebarOpenProgress > 0.01
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            id: sidebarContent
            width: sidebarScrollView.availableWidth
            spacing: 0

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 12
                Layout.bottomMargin: 10
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Действия")
                    color: Theme.textSecondary
                    font.bold: true
                    font.pixelSize: 12
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 3
                    columnSpacing: 7
                    rowSpacing: 7

                    Repeater {
                        model: [
                            { iconPath: "M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z", action: "search", label: "Поиск камер", tooltip: "Поиск камер в сети", primary: true },
                            { iconPath: "M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z", action: "add_camera", label: "Камера", tooltip: "Добавить камеру" },
                            { iconPath: "M20 6h-8l-2-2H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2zm-1 8h-3v3h-2v-3h-3v-2h3V9h2v3h3v2z", action: "add_folder", label: "Группа", tooltip: "Добавить группу" },
                            { iconPath: "M12 2L4 5v6c0 5.55 3.84 10.74 8 12 4.16-1.26 8-6.45 8-12V5l-8-3zm0 2.18L18 6.4V11c0 4.52-2.98 8.69-6 9.82-3.02-1.13-6-5.3-6-9.82V6.4l6-2.22zm-1 4.82h2v5h-2V9zm0 6h2v2h-2v-2z", action: "health", label: "Здоровье", tooltip: "Здоровье камер и диагностика" },
                            { iconPath: "M19.87 18.73l-5.32-5.32C15.2 12.33 15.6 11.22 15.6 10c0-3.09-2.51-5.6-5.6-5.6S4.4 6.91 4.4 10s2.51 5.6 5.6 5.6c1.22 0 2.33-.4 3.41-1.05l5.32 5.32c.39.39 1.02.39 1.41 0l-.27-.27.27.27c.39-.39.39-1.02 0-1.41zM10 14.1c-2.26 0-4.1-1.84-4.1-4.1S7.74 5.9 10 5.9s4.1 1.84 4.1 4.1-1.84 4.1-4.1 4.1z", action: "analytics", label: "Аналитика", tooltip: "Аналитика" },
                            { iconPath: "M19.43 12.98c.04-.32.07-.64.07-.98 0-.34-.03-.66-.07-.98l2.11-1.65c.19-.15.24-.42.12-.64l-2-3.46c-.09-.16-.26-.25-.44-.25-.06 0-.12.01-.17.03l-2.49 1c-.52-.4-1.08-.73-1.69-.98l-.38-2.65C14.46 2.18 14.25 2 14 2h-4c-.25 0-.46.18-.49.42l-.38 2.65c-.61.25-1.17.59-1.69.98l-2.49-1c-.06-.02-.12-.03-.18-.03-.17 0-.34.09-.43.25l-2 3.46c-.13.22-.07.49.12.64l2.11 1.65c-.04.32-.07.65-.07.98 0 .33.03.66.07.98l-2.11 1.65c-.19.15-.24.42-.12.64l2 3.46c.09.16.26.25.44.25.06 0 .12-.01.17-.03l2.49-1c.52.4 1.08.73 1.69.98l.38 2.65c.03.24.24.42.49.42h4c.25 0 .46-.18.49-.42l.38-2.65c.61-.25 1.17-.59 1.69-.98l2.49 1c.06.02.12.03.18.03.17 0 .34-.09.43-.25l2-3.46c.13-.22.07-.49-.12-.64l-2.11-1.65zm-1.98-1.71c.04.31.05.52.05.73 0 .21-.02.43-.05.73l-.14 1.13.89.7 1.08.84-.7 1.21-1.27-.51-1.04-.42-.9.68c-.43.32-.84.56-1.25.73l-1.06.43-.16 1.13-.2 1.35h-1.4l-.19-1.35-.16-1.13-1.06-.43c-.43-.18-.83-.41-1.23-.71l-.91-.7-1.06.43-1.27.51-.7-1.21 1.08-.84.89-.7-.14-1.13c-.03-.31-.05-.54-.05-.74s.02-.43.05-.73l.14-1.13-.89-.7-1.08-.84.7-1.21 1.27.51 1.04.42.9-.68c.43-.32.84-.56 1.25-.73l1.06-.43.16-1.13.2-1.35h1.39l.19 1.35.16 1.13 1.06.43c.43.18.83.41 1.23.71l.91.7 1.06-.43 1.27-.51.7 1.21-1.07.85-.89.7.14 1.13zM12 8c-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4-1.79-4-4-4zm0 6c-1.1 0-2-.9-2-2s.9-2 2-2 2 .9 2 2-.9 2-2 2z", action: "settings", label: "Настройки", tooltip: "Настройки" },
                            { iconPath: "M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm-5 14H7v-2h7v2zm3-4H7v-2h10v2zm0-4H7V7h10v2z", action: "logs", label: "Логи", tooltip: "Логи" },
                            { iconPath: "M10,4 A4,4 0 1,1 10,12 A4,4 0 1,1 10,4 M10.67,13.02C10.45,13.01,10.23,13,10,13c-2.42,0-4.68,0.67-6.61,1.82C2.51,15.34,2,16.32,2,17.35V20h9.26 C10.47,18.87,10,17.49,10,16C10,14.93,10.25,13.93,10.67,13.02z M20.75,16c0-0.22-0.03-0.42-0.06-0.63l1.14-1.01l-1-1.73l-1.45,0.49c-0.32-0.27-0.68-0.48-1.08-0.63L18,11h-2l-0.3,1.49 c-0.4,0.15-0.76,0.36-1.08,0.63l-1.45-0.49l-1,1.73l1.14,1.01c-0.03,0.21-0.06,0.41-0.06,0.63s0.03,0.42,0.06,0.63l-1.14,1.01 l1,1.73l1.45-0.49c0.32,0.27,0.68,0.48,1.08,0.63L16,21h2l0.3-1.49c0.4-0.15,0.76-0.36,1.08-0.63l1.45,0.49l1-1.73l-1.14-1.01 C20.72,16.42,20.75,16.22,20.75,16z M17,18c-1.1,0-2-0.9-2-2s0.9-2,2-2s2,0.9,2,2S18.1,18,17,18z", action: "user", label: "Пользователи", tooltip: "Пользователь" },
                            { iconPath: "M12 2C8.13 2 5 5.13 5 9c0 1.47.45 2.83 1.22 3.95L3.6 15.57 5.03 17l2.62-2.62C8.84 15.39 10.36 16 12 16s3.16-.61 4.35-1.62L18.97 17l1.43-1.43-2.62-2.62C18.55 11.83 19 10.47 19 9c0-3.87-3.13-7-7-7zm0 2c2.76 0 5 2.24 5 5s-2.24 5-5 5-5-2.24-5-5 2.24-5 5-5zm-1 2v2H9v2h2v2h2v-2h2V8h-2V6h-2z", action: "camex", label: "Camex", tooltip: "Удаленный доступ Camex" },
                            { iconPath: "M17 7l-1.41 1.41L18.17 11H8v2h10.17l-2.58 2.58L17 17l5-5zM4 5h8V3H4c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h8v-2H4V5z", action: "logout", label: "Выход", tooltip: "Выход" }
                        ]

                        SidebarCommandButton {
                            id: actionTile

                            required property var modelData

                            iconPath: actionTile.modelData.iconPath
                            label: actionTile.modelData.label
                            tooltip: actionTile.modelData.tooltip
                            primary: actionTile.modelData.primary === true
                            compact: true
                            enabled: sidebar.actionAllowed(actionTile.modelData.action)
                            onClicked: sidebar.triggerAction(actionTile.modelData.action)
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 15
                Layout.rightMargin: 15
                Layout.bottomMargin: 8
                Layout.preferredHeight: 42
                radius: Theme.radiusMd
                color: Theme.panelSoftBackground
                border.color: Theme.panelBorder
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    anchors.topMargin: 5
                    anchors.bottomMargin: 5
                    spacing: 1

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Состояние")
                            color: Theme.textSecondary
                            font.pixelSize: 11
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        Text {
                            text: sidebar.currentUsername()
                            color: Theme.textMuted
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: I18n.t("%1 всего · %2 онлайн · %3 офлайн", [
                                         sidebar.cameraCount(),
                                         sidebar.onlineCameraCount(),
                                         Math.max(0, sidebar.cameraCount() - sidebar.onlineCameraCount())
                                     ])
                        color: Theme.textMuted
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 15
                Layout.rightMargin: 15
                Layout.bottomMargin: 8
                Layout.preferredHeight: 32
                visible: sidebar.smartStreamBudgetEnabled()
                radius: Theme.radiusMd
                color: Theme.panelSoftBackground
                border.color: Theme.panelBorder
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        text: I18n.t("Live-preview")
                        color: Theme.textSecondary
                        font.pixelSize: 11
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        text: I18n.t("%1/%2 активно · %3 пауза", [
                                         sidebar.activePreviewCount(),
                                         sidebar.previewBudgetLimit(),
                                         sidebar.budgetPausedPreviewCount()
                                     ])
                        color: Theme.textMuted
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                }
            }

            TextField {
                id: deviceFilterField
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.bottomMargin: 8
                implicitHeight: 34
                text: sidebar.deviceFilterText
                placeholderText: I18n.t("Поиск устройств")
                color: Theme.textPrimary
                placeholderTextColor: Theme.textMuted
                selectionColor: Theme.accent
                selectedTextColor: Theme.textPrimary
                selectByMouse: true
                leftPadding: 10
                rightPadding: 10
                background: Rectangle {
                    color: Theme.controlBackground
                    radius: Theme.radiusSm
                    border.color: deviceFilterField.activeFocus ? Theme.accent : Theme.controlBorder
                    border.width: 1
                }
                onTextChanged: {
                    if (text !== sidebar.deviceFilterText)
                        sidebar.deviceFilterChanged(text)
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                Layout.fillWidth: true
                Layout.leftMargin: 15
                Layout.rightMargin: 15
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                padding: 0
                text: I18n.t("Устройства")
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 14
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: 6

                Component {
                    id: groupBlock

                    Item {
                        id: groupBlockItem

                        required property string modelData

                        Layout.fillWidth: true
                        implicitHeight: groupBlockItem.blockVisible ? layout.implicitHeight : 0
                        visible: groupBlockItem.blockVisible

                        property string groupName: groupBlockItem.modelData
                        readonly property bool isDefaultGroup: groupBlockItem.groupName === ""
                        readonly property bool blockVisible: sidebar.deviceFilterText.trim() === "" || groupBlockItem.groupCount > 0

                        property int groupCount: {
                            var v = sidebar.cameraDataVersion
                            var count = 0
                            if (!sidebar.systemController)
                                return 0

                            for (var i = 0; i < sidebar.systemController.cameraModel.rowCount(); ++i) {
                                var cam = sidebar.systemController.cameraModel.getCamera(i)
                                var g = cam.cameraGroup || ""
                                if (g === groupBlockItem.groupName
                                        && sidebar.cameraMatchesDeviceFilter(cam.cameraName, cam.cameraIp,
                                                                            sidebar.cameraStatusSearchText(cam.cameraIp, cam.status),
                                                                            cam.cameraGroup)) {
                                    count++
                                }
                            }
                            return count
                        }

                        ColumnLayout {
                            id: layout
                            anchors.fill: parent
                            spacing: 0

                            SidebarSectionHeader {
                                title: groupBlockItem.isDefaultGroup ? I18n.t("Без группы") : groupBlockItem.groupName
                                count: groupBlockItem.groupCount
                                interactive: !groupBlockItem.isDefaultGroup
                                onContextRequested: sidebar.groupContextRequested(groupBlockItem.groupName)
                            }

                            ListView {
                                Layout.fillWidth: true
                                Layout.preferredHeight: contentHeight
                                clip: true
                                interactive: false
                                spacing: 2
                                model: sidebar.systemController ? sidebar.systemController.cameraModel : null

                                delegate: DeviceListItem {
                                    id: deviceRow

                                    required property int index
                                    required property string cameraGroup
                                    required property string cameraIp
                                    required property string cameraName
                                    required property string status

                                    property int rowCameraIndex: deviceRow.index
                                    property string effectiveStatusValue: sidebar.effectiveCameraStatus(deviceRow.cameraIp, deviceRow.status)
                                    property string effectiveDetailValue: sidebar.effectiveCameraDetail(deviceRow.cameraIp, deviceRow.status)
                                    property bool inGroup: (deviceRow.cameraGroup || "") === groupBlockItem.groupName
                                                           && sidebar.cameraMatchesDeviceFilter(deviceRow.cameraName, deviceRow.cameraIp,
                                                                                                deviceRow.effectiveStatusValue + " " + deviceRow.effectiveDetailValue,
                                                                                                deviceRow.cameraGroup)

                                    width: ListView.view ? ListView.view.width : parent.width
                                    height: inGroup ? implicitHeight : 0
                                    visible: inGroup
                                    cameraName: deviceRow.cameraName
                                    cameraIp: deviceRow.cameraIp
                                    effectiveStatus: deviceRow.effectiveStatusValue
                                    effectiveDetail: deviceRow.effectiveDetailValue
                                    cameraIndex: deviceRow.rowCameraIndex
                                    online: sidebar.isOnlineStatus(deviceRow.effectiveStatusValue)
                                    canSettings: sidebar.canSettings
                                    dashboard: sidebar.dashboard
                                    dragProxyItem: sidebar.dragProxyItem
                                    systemController: sidebar.systemController

                                    onNoAccessRequested: sidebar.noAccessRequested()
                                    onContextRequested: (cameraIp, cameraName, cameraIndex) => sidebar.deviceContextRequested(cameraIp, cameraName, cameraIndex)
                                    onAddRequested: (cameraIndex) => sidebar.addCameraToGridRequested(cameraIndex)
                                    onMajesticRequested: (cameraIndex) => sidebar.majesticRequested(cameraIndex)
                                    onHealthRequested: (cameraIp) => {
                                        if (sidebar.systemController)
                                            sidebar.systemController.refreshCameraHealth(cameraIp)
                                    }
                                    onRemoveRequested: (cameraIndex) => {
                                        if (sidebar.systemController)
                                            sidebar.systemController.removeDevice(cameraIndex)
                                    }
                                }
                            }
                        }

                        DropArea {
                            anchors.fill: parent
                            z: 1
                            keys: ["camera"]
                            enabled: sidebar.canSettings
                            onEntered: (drag) => drag.accept(Qt.MoveAction)
                            onDropped: (drop) => {
                                if (!sidebar.canSettings) {
                                    sidebar.noAccessRequested()
                                    return
                                }
                                var idx = drop.mimeData.getData("application/camera-index")
                                if (idx !== undefined && idx !== null) {
                                    sidebar.systemController.setCameraGroup(parseInt(idx), groupBlockItem.groupName)
                                    sidebar.cameraDataChangedRequested()
                                }
                            }

                            Rectangle {
                                anchors.fill: parent
                                color: parent.containsDrag ? "#3d4450" : "transparent"
                                opacity: 0.5
                                radius: Theme.radiusSm
                                visible: parent.containsDrag
                            }
                        }
                    }
                }

                Repeater {
                    model: [""].concat(sidebar.systemController ? sidebar.systemController.cameraGroups : [])
                    delegate: groupBlock
                }

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.topMargin: 10
                    visible: sidebar.cameraCount() > 0 && sidebar.filteredCameraCount() === 0
                    text: I18n.t("Ничего не найдено")
                    color: Theme.textMuted
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
