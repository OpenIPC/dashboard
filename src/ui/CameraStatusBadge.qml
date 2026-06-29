import QtQuick
import QtQuick.Layouts
import OpenIPC

Item {
    id: root

    default property alias extraContent: ignoredContent.data

    property bool online: false
    property string statusText: online ? "Online" : "Offline"
    property bool showText: true
    property int dotSize: 10
    property color onlineColor: Theme.success
    property color offlineColor: Theme.danger
    property color onlineBackground: "#1f5d35"
    property color offlineBackground: "#5c1f1f"

    implicitWidth: showText ? 92 : dotSize
    implicitHeight: showText ? 26 : dotSize

    Item {
        id: ignoredContent
        visible: false
    }

    Rectangle {
        anchors.fill: parent
        visible: root.showText
        radius: height / 2
        color: root.online ? root.onlineBackground : root.offlineBackground
        border.color: root.online ? root.onlineColor : root.offlineColor
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 9
            anchors.rightMargin: 9
            spacing: 6

            Rectangle {
                Layout.preferredWidth: Math.max(6, Math.min(8, root.dotSize))
                Layout.preferredHeight: Math.max(6, Math.min(8, root.dotSize))
                radius: width / 2
                color: root.online ? root.onlineColor : root.offlineColor
            }

            Text {
                Layout.fillWidth: true
                text: I18n.t(root.statusText || (root.online ? "Online" : "Offline"))
                color: Theme.textPrimary
                font.pixelSize: 11
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }
    }

    Rectangle {
        anchors.centerIn: parent
        visible: !root.showText
        width: root.dotSize
        height: root.dotSize
        radius: root.dotSize / 2
        color: root.online ? root.onlineColor : root.offlineColor
    }
}
