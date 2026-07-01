pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property string cameraName: ""
    property string cameraIp: ""
    property string effectiveStatus: "Offline"
    property string effectiveDetail: ""
    property int cameraIndex: -1
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

    implicitHeight: root.effectiveDetail !== "" ? 72 : 58
    radius: Theme.radiusLg
    color: deviceHover.hovered ? Theme.cardHover : Theme.panelSoftBackground
    border.color: deviceHover.hovered ? (root.online ? Theme.success : Theme.warning) : Theme.panelBorder
    border.width: 1
    clip: true

    readonly property string displayName: (root.cameraName && root.cameraName.trim() !== "")
                                          ? root.cameraName
                                          : I18n.t("Камера") + " " + root.cameraIp

    HoverHandler {
        id: deviceHover
    }

    ToolTip.visible: deviceHover.hovered && root.effectiveDetail !== ""
    ToolTip.text: I18n.t(root.effectiveDetail)
    ToolTip.delay: 500

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
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 8
        spacing: 10

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
                    Layout.fillWidth: true
                    text: root.displayName
                    color: Theme.textPrimary
                    font.pixelSize: 12
                    font.bold: true
                    elide: Text.ElideRight
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Text {
                    Layout.fillWidth: true
                    text: root.cameraIp
                    color: Theme.textMuted
                    font.pixelSize: 11
                    font.family: "Consolas"
                    elide: Text.ElideRight
                }

                Rectangle {
                    Layout.preferredWidth: statusText.implicitWidth + 16
                    Layout.preferredHeight: 20
                    radius: 10
                    color: root.online ? "#0f3f27" : "#3f1212"
                    border.color: root.online ? Theme.success : Theme.danger

                    Text {
                        id: statusText
                        anchors.centerIn: parent
                        text: I18n.t(root.effectiveStatus || (root.online ? "Online" : "Offline"))
                        color: root.online ? Theme.success : Theme.danger
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                visible: root.effectiveDetail !== ""
                text: I18n.t(root.effectiveDetail)
                color: root.online ? Theme.textMuted : Theme.warning
                font.pixelSize: 10
                elide: Text.ElideRight
            }
        }

        RowLayout {
            Layout.preferredWidth: root.canSettings ? 92 : 0
            Layout.alignment: Qt.AlignVCenter
            visible: root.canSettings
            opacity: deviceHover.hovered ? 1.0 : 0.0
            spacing: 4

            Behavior on opacity {
                NumberAnimation { duration: 120 }
            }

            Repeater {
                model: [
                    { text: "+", tooltip: I18n.t("Добавить в раскладку"), action: "grid" },
                    { text: "O", tooltip: I18n.t("OpenIPC Control Center"), action: "majestic" },
                    { text: "↻", tooltip: I18n.t("Проверить здоровье"), action: "health" }
                ]

                delegate: Rectangle {
                    id: quickActionButton

                    required property var modelData

                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 24
                    radius: Theme.radiusMd
                    color: quickActionMouse.containsMouse ? Theme.accent : Theme.controlBackground
                    border.color: quickActionMouse.containsMouse ? Theme.accentHover : Theme.controlBorder
                    z: 3

                    ToolTip.visible: quickActionMouse.containsMouse
                    ToolTip.text: quickActionButton.modelData.tooltip
                    ToolTip.delay: 350

                    Text {
                        anchors.centerIn: parent
                        text: quickActionButton.modelData.text
                        color: Theme.textPrimary
                        font.bold: true
                        font.pixelSize: quickActionButton.modelData.action === "majestic" ? 12 : 15
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
            Layout.preferredWidth: root.canSettings ? 18 : 0
            visible: root.canSettings
            text: "×"
            color: removeMouse.containsMouse ? Theme.danger : Theme.textMuted
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
