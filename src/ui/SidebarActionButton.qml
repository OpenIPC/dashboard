import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Button {
    id: root

    property string iconPath: ""
    property string label: ""
    property string tooltip: ""

    Layout.fillWidth: true
    implicitHeight: 48
    padding: 0
    hoverEnabled: true

    background: Rectangle {
        color: root.enabled
               ? (root.hovered ? Theme.cardHover : Theme.panelSoftBackground)
               : Theme.controlBackgroundAlt
        radius: Theme.radiusMd
        border.color: root.hovered ? Theme.accent : Theme.controlBorder
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: 2

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 23

            SidebarIcon {
                anchors.centerIn: parent
                width: 18
                height: 18
                path: root.iconPath
                color: root.enabled
                       ? (root.hovered ? Theme.accent : Theme.textSecondary)
                       : Theme.textMuted
            }
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t(root.label)
            color: root.enabled ? Theme.textSecondary : Theme.textMuted
            font.pixelSize: 10
            font.bold: root.label.indexOf("Поиск") === 0
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }

    ToolTip.visible: root.hovered
    ToolTip.text: I18n.t(root.tooltip)
    ToolTip.delay: 500
}
