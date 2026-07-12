import QtQuick
import QtQuick.Controls
import OpenIPC

TabButton {
    id: tabButton
    implicitHeight: 44
    contentItem: Text {
        text: tabButton.text
        color: tabButton.checked ? Theme.textPrimary : Theme.textMuted
        font.family: Theme.metroFontFamily
        font.pixelSize: 13
        font.bold: tabButton.checked
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: 0
        color: tabButton.checked ? Theme.metroSurface
             : (tabButton.hovered ? Theme.metroTileHover : Theme.metroSurfaceAlt)
        border.color: tabButton.checked ? Theme.metroStrokeStrong : Theme.metroStroke
        border.width: tabButton.checked || tabButton.hovered ? 2 : 1
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 3
            color: tabButton.checked ? Theme.metroBlue : "transparent"
        }
    }
}
