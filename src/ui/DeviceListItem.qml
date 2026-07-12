pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    required property string cameraName
    required property string cameraIp
    required property int cameraPort
    property string effectiveStatus: "Offline"
    property string effectiveDetail: ""
    property int cameraIndex: -1
    property int cameraDataVersion: 0
    property bool online: false
    property bool canSettings: false
    property Item dashboard: null
    property Item dragProxyItem: null
    property var systemController: null

    signal noAccessRequested()
    signal contextRequested(string cameraIp, string cameraName, int cameraIndex)
    signal addRequested(int cameraIndex)
    signal majesticRequested(int cameraIndex)
    signal healthRequested(string cameraIp)
    signal removeRequested(int cameraIndex)

    implicitHeight: root.effectiveDetail !== "" ? 92 : 76
    radius: Theme.metroTileRadius
    color: deviceHover.hovered ? Theme.metroTileHover : Theme.metroSurfaceAlt
    border.color: deviceHover.hovered ? Theme.metroStrokeStrong : Theme.metroStroke
    border.width: deviceHover.hovered ? 2 : 1
    clip: true

    readonly property string displayName: (root.cameraName && root.cameraName.trim() !== "")
                                          ? root.cameraName
                                          : I18n.t("Камера") + " " + root.cameraIp
    readonly property var healthResult: {
        var version = root.cameraDataVersion
        if (!root.systemController || !root.systemController.cameraHealthController
                || root.cameraIp === "")
            return ({})
        return root.systemController.cameraHealthController.resultForCamera(root.cameraIp)
    }
    readonly property real temperatureC: Number(root.healthResult.temperatureC)
    readonly property bool hasTemperature: root.healthResult.temperatureC !== undefined
                                           && root.healthResult.temperatureC !== null
                                           && Number.isFinite(root.temperatureC)
    readonly property string temperatureText: root.hasTemperature
                                              ? Math.round(root.temperatureC) + " °C"
                                              : "— °C"
    readonly property string healthStatus: String(root.healthResult.status || "")
    readonly property color healthColor: root.healthStatus === "error" ? Theme.metroRed
                                         : (root.healthStatus === "warning" ? Theme.warning
                                            : (root.healthStatus === "ok" ? Theme.metroGreen
                                               : (root.online ? Theme.metroGreen : Theme.metroRed)))
    readonly property bool layoutReady: root.implicitHeight >= 76
                                        && cameraIpText.width > 40
                                        && temperatureValue.width > 20

    HoverHandler {
        id: deviceHover
    }

    ToolTip.visible: deviceHover.hovered && root.effectiveDetail !== ""
    ToolTip.text: I18n.t(root.effectiveDetail)
    ToolTip.delay: 500

    Rectangle {
        width: 4
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: root.healthColor
    }

    MouseArea {
        id: dragArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                if (!root.canSettings) {
                    root.noAccessRequested()
                    return
                }
                root.contextRequested(root.cameraIp, root.cameraName, root.cameraIndex)
            }
        }

        onDoubleClicked: root.addRequested(root.cameraIndex)

        drag.target: root.canSettings && root.dragProxyItem ? root.dragProxyItem : null
        drag.axis: Drag.XAndYAxis
        drag.threshold: 10

        onPressed: (mouse) => {
            if (!root.canSettings) {
                root.noAccessRequested()
                return
            }
            if (mouse.button === Qt.LeftButton && root.dragProxyItem && root.dashboard) {
                var pos = mapToItem(root.dashboard, mouse.x, mouse.y)
                root.dragProxyItem.x = pos.x - root.dragProxyItem.width / 2
                root.dragProxyItem.y = pos.y - root.dragProxyItem.height / 2
                root.dragProxyItem.proxyIp = root.cameraIp
                root.dragProxyItem.proxyName = root.cameraName || ""
                root.dragProxyItem.dragIndex = root.cameraIndex
            }
        }

        onReleased: {
            if (root.dragProxyItem) {
                root.dragProxyItem.visible = false
                root.dragProxyItem.x = 0
                root.dragProxyItem.y = 0
            }
        }

        drag.onActiveChanged: {
            if (root.dragProxyItem)
                root.dragProxyItem.visible = drag.active
        }
    }

    RowLayout {
        id: mainLayout

        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 8
        anchors.topMargin: 7
        anchors.bottomMargin: 7
        spacing: 8

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            RowLayout {
                Layout.fillWidth: true
                spacing: 7

                CameraStatusBadge {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: 10
                    dotSize: 10
                    showText: false
                    online: root.online
                    statusText: root.effectiveStatus
                }

                Text {
                    id: cameraIpText

                    Layout.fillWidth: true
                    text: root.displayName
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 12
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    id: statusText

                    text: I18n.t(root.effectiveStatus
                                 || (root.online ? "Online" : "Offline"))
                    color: root.online ? Theme.metroGreen : Theme.metroRed
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 10
                    font.bold: true
                    horizontalAlignment: Text.AlignRight
                    elide: Text.ElideRight
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: "IP " + root.cameraIp
                    color: Theme.textMuted
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }

                Text {
                    visible: root.width >= 280
                    text: "RTSP " + (root.cameraPort > 0 ? root.cameraPort : 554)
                    color: Theme.textMuted
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 10
                }

                RowLayout {
                    spacing: 3

                    SidebarIcon {
                        Layout.preferredWidth: 13
                        Layout.preferredHeight: 13
                        name: "device_thermostat"
                        fallbackText: "T"
                        color: root.hasTemperature && root.temperatureC >= 75
                               ? Theme.warning : Theme.textMuted
                        pixelSize: 12
                    }
                    Text {
                        id: temperatureValue

                        text: root.temperatureText
                        color: root.hasTemperature && root.temperatureC >= 75
                               ? Theme.warning
                               : (root.hasTemperature ? Theme.textSecondary
                                                      : Theme.textMuted)
                        font.family: Theme.metroFontFamily
                        font.pixelSize: 10
                        font.bold: root.hasTemperature

                        ToolTip.visible: temperatureHover.hovered
                        ToolTip.delay: 450
                        ToolTip.text: root.hasTemperature
                                      ? I18n.t("Температура SoC")
                                      : (I18n.language === "ru"
                                         ? "Температура появится после профиля OpenIPC или Глубокий"
                                         : "Temperature appears after an OpenIPC or Deep health run")

                        HoverHandler { id: temperatureHover }
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                visible: root.effectiveDetail !== ""
                text: I18n.t(root.effectiveDetail)
                color: root.online ? Theme.textMuted : Theme.warning
                font.family: Theme.metroFontFamily
                font.pixelSize: 10
                elide: Text.ElideRight
            }
        }

        RowLayout {
            Layout.preferredWidth: root.canSettings && deviceHover.hovered ? 92 : 0
            Layout.minimumWidth: 0
            Layout.maximumWidth: root.canSettings && deviceHover.hovered ? 92 : 0
            Layout.alignment: Qt.AlignVCenter
            visible: root.canSettings
            opacity: deviceHover.hovered ? 1.0 : 0.0
            spacing: 4

            Behavior on opacity {
                NumberAnimation { duration: 120 }
            }

            Repeater {
                model: [
                    { icon: "add", fallback: "+", tooltip: I18n.t("Добавить в раскладку"), action: "grid" },
                    { icon: "settings_input_antenna", fallback: "O", tooltip: I18n.t("OpenIPC Control Center"), action: "majestic" },
                    { icon: "refresh", fallback: "R", tooltip: I18n.t("Проверить здоровье"), action: "health" }
                ]

                delegate: Rectangle {
                    id: quickActionButton

                    required property var modelData

                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 24
                    radius: Theme.metroTileRadius
                    color: quickActionMouse.containsMouse ? Theme.metroBlue : Theme.controlBackground
                    border.color: quickActionMouse.containsMouse ? Theme.metroBlue : Theme.metroStroke
                    z: 3

                    ToolTip.visible: quickActionMouse.containsMouse
                    ToolTip.text: quickActionButton.modelData.tooltip
                    ToolTip.delay: 350

                    SidebarIcon {
                        anchors.centerIn: parent
                        width: 17
                        height: 17
                        name: quickActionButton.modelData.icon
                        fallbackText: quickActionButton.modelData.fallback
                        color: Theme.textPrimary
                        pixelSize: 15
                    }

                    MouseArea {
                        id: quickActionMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (!root.canSettings) {
                                root.noAccessRequested()
                                return
                            }
                            if (quickActionButton.modelData.action === "grid")
                                root.addRequested(root.cameraIndex)
                            else if (quickActionButton.modelData.action === "majestic")
                                root.majesticRequested(root.cameraIndex)
                            else if (quickActionButton.modelData.action === "health")
                                root.healthRequested(root.cameraIp)
                        }
                    }
                }
            }
        }

        Text {
            Layout.preferredWidth: root.canSettings && deviceHover.hovered ? 18 : 0
            Layout.minimumWidth: 0
            Layout.maximumWidth: root.canSettings && deviceHover.hovered ? 18 : 0
            visible: root.canSettings && deviceHover.hovered
            text: "×"
            color: removeMouse.containsMouse ? Theme.danger : Theme.textMuted
            font.family: Theme.metroFontFamily
            font.pixelSize: 18
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            opacity: deviceHover.hovered ? 1.0 : 0.45

            MouseArea {
                id: removeMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (!root.canSettings) {
                        root.noAccessRequested()
                        return
                    }
                    root.removeRequested(root.cameraIndex)
                }
            }
        }
    }
}
