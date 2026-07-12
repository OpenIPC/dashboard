import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Button {
    id: root

    property string iconPath: ""
    property string iconName: ""
    property string label: ""
    property string subtitle: ""
    property string tooltip: ""
    property bool primary: false
    property bool compact: false

    Layout.fillWidth: true
    implicitHeight: root.compact ? 46 : 58
    padding: 0
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    background: Rectangle {
        radius: Theme.metroTileRadius
        color: !root.enabled
               ? Theme.metroTileDisabled
               : root.primary
                 ? (root.hovered ? Theme.metroBlueHover : Theme.metroBlue)
                 : (root.down ? Theme.metroTilePressed : (root.hovered ? Theme.metroTileHover : Theme.metroTile))
        border.color: !root.enabled
                      ? Theme.metroStroke
                      : root.primary
                        ? Theme.metroBlue
                        : (root.hovered ? Theme.metroStrokeStrong : Theme.metroStroke)
        border.width: root.hovered || root.activeFocus ? 2 : 1

        Rectangle {
            width: 4
            radius: 0
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            visible: root.primary
            color: Theme.textPrimary
            opacity: root.enabled ? 1 : 0.35
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: root.compact ? 5 : 8
        anchors.rightMargin: root.compact ? 5 : 8
        anchors.topMargin: root.compact ? 5 : 7
        anchors.bottomMargin: root.compact ? 4 : 6
        spacing: 2

        Item {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: root.compact ? 20 : 24
            Layout.preferredHeight: root.compact ? 19 : 23

            SidebarIcon {
                anchors.centerIn: parent
                width: root.compact ? 16 : 18
                height: root.compact ? 16 : 18
                name: root.iconName
                path: root.iconPath
                color: root.enabled
                       ? (root.primary ? Theme.textPrimary : (root.hovered ? Theme.textPrimary : Theme.textSecondary))
                       : Theme.textMuted
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Text {
                anchors.fill: parent
                text: I18n.t(root.label)
                color: root.enabled ? (root.primary ? Theme.textPrimary : Theme.textSecondary) : Theme.textMuted
                font.family: Theme.metroFontFamily
                font.pixelSize: root.compact ? 10 : 11
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                maximumLineCount: 2
                wrapMode: Text.WordWrap
            }
        }

        Text {
            Layout.fillWidth: true
            visible: false
            text: I18n.t(root.subtitle)
            color: Theme.textMuted
            font.pixelSize: 10
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }

    ToolTip.visible: (root.hovered || root.visualFocus) && root.tooltip.length > 0
    ToolTip.text: root.enabled
                  ? I18n.t(root.tooltip)
                  : I18n.t(root.tooltip) + "\n" + I18n.t("Недостаточно прав")
    ToolTip.delay: 450
}
