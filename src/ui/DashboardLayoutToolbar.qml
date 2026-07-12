pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

RowLayout {
    id: layoutToolbar

    property var layoutsModel: null
    property int currentLayoutIndex: -1
    readonly property bool layoutReady: width > 0
                                        && layoutsFlick.width >= 0
                                        && addButton.x >= 0
                                        && editButton.x + editButton.width <= width + 1
                                        && (layoutsFlick.contentWidth > layoutsFlick.width
                                            || addButton.x <= layoutsFlick.x + layoutsFlick.contentWidth + 16)
    signal applyRequested(int index)
    signal closeRequested(int index)
    signal addRequested()
    signal editRequested()

    spacing: 6

    Flickable {
        id: layoutsFlick

        Layout.fillWidth: false
        Layout.minimumWidth: 0
        Layout.preferredWidth: Math.min(layoutsRow.implicitWidth, Math.max(0, layoutToolbar.width - 96))
        Layout.maximumWidth: Math.max(0, layoutToolbar.width - 96)
        Layout.preferredHeight: 32
        clip: true
        contentWidth: layoutsRow.implicitWidth
        contentHeight: height
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds

        RowLayout {
            id: layoutsRow

            height: layoutsFlick.height
            spacing: 6

            Repeater {
                model: layoutToolbar.layoutsModel

                delegate: Rectangle {
                id: layoutButton

                required property int index
                required property string name
                required property bool isDefault

                height: 32
                activeFocusOnTab: true
                radius: Theme.metroTileRadius
                color: layoutButton.index === layoutToolbar.currentLayoutIndex ? Theme.metroTilePressed : Theme.metroTile
                border.color: layoutButton.activeFocus || layoutButton.index === layoutToolbar.currentLayoutIndex
                              ? Theme.metroStrokeStrong
                              : Theme.metroStroke
                border.width: layoutButton.activeFocus || layoutButton.index === layoutToolbar.currentLayoutIndex ? 2 : 1
                width: Math.min(180, Math.max(88, nameText.implicitWidth + 30))

                Keys.onReturnPressed: layoutToolbar.applyRequested(layoutButton.index)
                Keys.onSpacePressed: layoutToolbar.applyRequested(layoutButton.index)

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 4
                    visible: layoutButton.index === layoutToolbar.currentLayoutIndex
                    color: Theme.metroBlue
                }

                Text {
                    id: nameText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.rightMargin: 28
                    text: layoutButton.isDefault ? I18n.t(layoutButton.name) : layoutButton.name
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                }

                Rectangle {
                    id: closeBtn
                    visible: layoutToolbar.layoutsModel && layoutToolbar.layoutsModel.count > 1
                    width: 20
                    height: 20
                    radius: Theme.metroTileRadius
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.topMargin: 3
                    anchors.rightMargin: 4
                    color: Theme.metroSurface
                    border.color: Theme.metroStroke
                    z: 2

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: Theme.textSecondary
                        font.pixelSize: 11
                    }

                    MouseArea {
                        id: closeMouse

                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: (mouse) => {
                            mouse.accepted = true
                            layoutToolbar.closeRequested(layoutButton.index)
                        }
                        onPressed: (mouse) => mouse.accepted = true
                    }

                    ToolTip.visible: closeMouse.containsMouse
                    ToolTip.text: I18n.t("Удалить раскладку")
                    ToolTip.delay: 450
                }

                MouseArea {
                    id: layoutMouse

                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: layoutToolbar.applyRequested(layoutButton.index)
                    onDoubleClicked: layoutToolbar.editRequested()
                }

                ToolTip.visible: layoutMouse.containsMouse || layoutButton.activeFocus
                ToolTip.text: layoutButton.isDefault ? I18n.t(layoutButton.name) : layoutButton.name
                ToolTip.delay: 500
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
        id: addButton

        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        activeFocusOnTab: true
        radius: Theme.metroTileRadius
        color: addMouse.containsMouse || addButton.activeFocus ? Theme.metroBlue : Theme.metroTile
        border.color: addMouse.containsMouse || addButton.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
        border.width: addButton.activeFocus ? 2 : 1

        Keys.onReturnPressed: layoutToolbar.addRequested()
        Keys.onSpacePressed: layoutToolbar.addRequested()

        Text {
            anchors.centerIn: parent
            text: "+"
            color: Theme.textPrimary
            font.family: Theme.metroFontFamily
            font.pixelSize: 16
        }

        ToolTip.visible: addMouse.containsMouse || addButton.activeFocus
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
        id: editButton

        Layout.preferredWidth: 36
        Layout.preferredHeight: 32
        activeFocusOnTab: true
        radius: Theme.metroTileRadius
        color: editMouse.containsMouse || editButton.activeFocus ? Theme.metroBlue : Theme.metroTile
        border.color: editMouse.containsMouse || editButton.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
        border.width: editButton.activeFocus ? 2 : 1

        Keys.onReturnPressed: layoutToolbar.editRequested()
        Keys.onSpacePressed: layoutToolbar.editRequested()

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

        ToolTip.visible: editMouse.containsMouse || editButton.activeFocus
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
