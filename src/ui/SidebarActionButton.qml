import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Button {
    id: root

    property string iconPath: ""
    property string iconName: ""
    property string label: ""
    property string tooltip: ""
    property bool primary: false

    Layout.fillWidth: true
    implicitHeight: root.primary ? 48 : 44
    padding: 0
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    background: Rectangle {
        color: root.enabled
               ? (root.primary
                  ? (root.hovered ? Theme.metroBlueHover : Theme.metroBlue)
                  : (root.hovered ? Theme.metroTileHover : Theme.metroTile))
               : Theme.metroTileDisabled
        radius: Theme.metroTileRadius
        border.color: root.primary
                      ? Theme.metroBlue
                      : (root.hovered ? Theme.metroStrokeStrong : Theme.metroStroke)
        border.width: root.hovered || root.activeFocus ? 2 : 1
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
                name: root.iconName
                path: root.iconPath
                color: root.enabled
                       ? (root.primary ? Theme.textPrimary : (root.hovered ? Theme.accent : Theme.textSecondary))
                       : Theme.textMuted
            }
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t(root.label)
            color: root.enabled ? (root.primary ? Theme.textPrimary : Theme.textSecondary) : Theme.textMuted
            font.family: Theme.metroFontFamily
            font.pixelSize: 10
            font.bold: root.primary
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }

    ToolTip.visible: root.hovered || root.visualFocus
    ToolTip.text: root.enabled
                  ? I18n.t(root.tooltip)
                  : I18n.t(root.tooltip) + "\n" + I18n.t("Недостаточно прав")
    ToolTip.delay: 500
}
