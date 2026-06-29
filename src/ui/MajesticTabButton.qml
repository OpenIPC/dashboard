import QtQuick
import QtQuick.Controls
import OpenIPC

TabButton {
    id: tabButton
    implicitHeight: 42
    contentItem: Text {
        text: tabButton.text
        color: tabButton.checked ? Theme.textPrimary : Theme.textMuted
        font.pixelSize: 13
        font.bold: tabButton.checked
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        color: tabButton.checked ? Theme.panelSoftBackground
             : (tabButton.hovered ? Theme.cardHover : Theme.panelAltBackground)
        border.color: tabButton.checked ? Theme.accent : Theme.panelBorder
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 2
            color: tabButton.checked ? Theme.accent : "transparent"
        }
    }
}
