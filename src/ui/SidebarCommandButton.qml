import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Button {
    id: root

    property string iconPath: ""
    property string label: ""
    property string subtitle: ""
    property string tooltip: ""
    property bool primary: false
    property bool compact: false

    Layout.fillWidth: true
    implicitHeight: root.compact ? 50 : 64
    padding: 0
    hoverEnabled: true

    background: Rectangle {
        radius: Theme.radiusLg
        color: !root.enabled
               ? Theme.controlBackgroundAlt
               : root.primary
                 ? (root.hovered ? Qt.rgba(37 / 255, 99 / 255, 235 / 255, 0.24)
                                 : Qt.rgba(37 / 255, 99 / 255, 235 / 255, 0.14))
                 : (root.hovered ? Theme.cardHover : Theme.panelSoftBackground)
        border.color: !root.enabled
                      ? Theme.panelBorder
                      : root.primary
                        ? (root.hovered ? Theme.accentHover : Theme.accent)
                        : (root.hovered ? Theme.accent : Theme.panelBorder)
        border.width: 1

        Rectangle {
            width: 3
            radius: 2
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 8
            visible: root.primary
            color: Theme.accent
            opacity: root.enabled ? 1 : 0.35
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: root.compact ? 6 : 8
        anchors.rightMargin: root.compact ? 6 : 8
        anchors.topMargin: root.compact ? 6 : 8
        anchors.bottomMargin: root.compact ? 5 : 7
        spacing: root.compact ? 3 : 5

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: root.compact ? 22 : 26
            Layout.preferredHeight: root.compact ? 22 : 26
            radius: root.compact ? 7 : 8
            color: root.primary ? Qt.rgba(96 / 255, 165 / 255, 250 / 255, 0.15) : Theme.controlBackground
            border.color: root.primary ? Theme.accent : Theme.controlBorder

            SidebarIcon {
                anchors.centerIn: parent
                width: root.compact ? 13 : 15
                height: root.compact ? 13 : 15
                path: root.iconPath
                color: root.enabled
                       ? (root.primary ? Theme.accentHover : (root.hovered ? Theme.accentHover : Theme.textSecondary))
                       : Theme.textMuted
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Text {
                anchors.fill: parent
                text: I18n.t(root.label)
                color: root.enabled ? Theme.textPrimary : Theme.textMuted
                font.pixelSize: root.compact ? 11 : 12
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

    ToolTip.visible: root.hovered && root.tooltip.length > 0
    ToolTip.text: I18n.t(root.tooltip)
    ToolTip.delay: 450
}
