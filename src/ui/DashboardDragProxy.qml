import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    visible: false
    width: 300
    height: 50
    color: Theme.metroTile
    opacity: 0.9
    z: 1000
    radius: Theme.metroTileRadius
    border.color: Theme.metroBlue
    border.width: 1

    property string proxyIp: ""
    property string proxyName: ""
    property int dragIndex: -1

    Drag.active: visible
    Drag.keys: ["camera"]
    Drag.mimeData: { "application/camera-index": root.dragIndex }
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2
    Drag.source: root

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        spacing: 10

        CameraStatusBadge {
            Layout.preferredWidth: 8
            Layout.preferredHeight: 8
            dotSize: 8
            showText: false
            online: false
            statusText: "Offline"
        }

        ColumnLayout {
            Text {
                text: I18n.t("Камера") + " " + root.proxyIp
                color: Theme.textSecondary
                font.pixelSize: 12
            }
            Text {
                text: root.proxyIp
                color: Theme.textMuted
                font.pixelSize: 11
            }
        }
    }
}
