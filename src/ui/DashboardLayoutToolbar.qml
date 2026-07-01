pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

RowLayout {
    id: layoutToolbar

    property var layoutsModel: null
    property int currentLayoutIndex: -1
    signal applyRequested(int index)
    signal closeRequested(int index)
    signal addRequested()
    signal editRequested()

    spacing: 8

    RowLayout {
        spacing: 6

        Repeater {
            model: layoutToolbar.layoutsModel

            delegate: Rectangle {
                id: layoutButton

                required property int index
                required property string name
                required property bool isDefault

                height: 32
                radius: Theme.radiusMd
                color: layoutButton.index === layoutToolbar.currentLayoutIndex ? "#11151f" : "#2d3442"
                border.color: layoutButton.index === layoutToolbar.currentLayoutIndex ? Theme.accent : "#3c4353"
                border.width: 1
                width: Math.max(96, nameText.implicitWidth + 30)

                Text {
                    id: nameText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.rightMargin: 28
                    text: layoutButton.isDefault ? I18n.t(layoutButton.name) : layoutButton.name
                    color: Theme.textPrimary
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                }

                Rectangle {
                    id: closeBtn
                    visible: layoutToolbar.layoutsModel && layoutToolbar.layoutsModel.count > 1
                    width: 20
                    height: 20
                    radius: 10
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.topMargin: 3
                    anchors.rightMargin: 4
                    color: "#3c4353"
                    border.color: "#4b556a"
                    z: 2

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: Theme.textSecondary
                        font.pixelSize: 11
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: (mouse) => {
                            mouse.accepted = true
                            layoutToolbar.closeRequested(layoutButton.index)
                        }
                        onPressed: (mouse) => mouse.accepted = true
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: layoutToolbar.applyRequested(layoutButton.index)
                    onDoubleClicked: layoutToolbar.editRequested()
                }
            }
        }
    }

    Rectangle {
        Layout.preferredWidth: 1
        Layout.preferredHeight: 24
        visible: layoutToolbar.layoutsModel && layoutToolbar.layoutsModel.count > 0
        color: Theme.panelBorder
    }

    Rectangle {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        radius: Theme.radiusMd
        color: addMouse.containsMouse ? "#3e4654" : "#2d3442"
        border.color: "#3c4353"

        Text {
            anchors.centerIn: parent
            text: "+"
            color: Theme.textPrimary
            font.pixelSize: 16
        }

        ToolTip.visible: addMouse.containsMouse
        ToolTip.text: I18n.t("Новая раскладка")

        MouseArea {
            id: addMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: layoutToolbar.addRequested()
        }
    }

    Rectangle {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        radius: Theme.radiusMd
        color: editMouse.containsMouse ? "#3e4654" : "#2d3442"
        border.color: "#3c4353"

        Canvas {
            anchors.centerIn: parent
            width: 16
            height: 16
            onPaint: {
                var ctx = getContext("2d")
                ctx.fillStyle = Theme.textSecondary
                var s = 2
                var w = width
                var h = height
                ctx.clearRect(0, 0, w, h)
                ctx.fillRect(0, 0, s, s)
                ctx.fillRect((w / 2) - s / 2, 0, s, s)
                ctx.fillRect(w - s, 0, s, s)
                ctx.fillRect(0, (h / 2) - s / 2, s, s)
                ctx.fillRect((w / 2) - s / 2, (h / 2) - s / 2, s, s)
                ctx.fillRect(w - s, (h / 2) - s / 2, s, s)
                ctx.fillRect(0, h - s, s, s)
                ctx.fillRect((w / 2) - s / 2, h - s, s, s)
                ctx.fillRect(w - s, h - s, s, s)
            }
        }

        ToolTip.visible: editMouse.containsMouse
        ToolTip.text: I18n.t("Редактор шаблонов")

        MouseArea {
            id: editMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: layoutToolbar.editRequested()
        }
    }

    Item { Layout.fillWidth: true }
}
