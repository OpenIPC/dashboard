import QtQuick
import QtQuick.Controls
import OpenIPC

MenuItem {
    id: item

    implicitWidth: Math.max(210, itemContent.implicitWidth + 28)
    implicitHeight: 34

    contentItem: Text {
        id: itemContent
        text: item.text
        color: item.enabled ? Theme.textSecondary : Theme.textFaint
        opacity: item.enabled ? 1.0 : 0.55
        font.family: Theme.metroFontFamily
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        leftPadding: 10
        rightPadding: 10
    }

    background: Rectangle {
        radius: Theme.metroTileRadius
        color: item.highlighted ? Theme.metroTileHover : "transparent"
        border.color: item.highlighted ? Theme.metroStrokeStrong : "transparent"
        border.width: item.highlighted ? 1 : 0
    }
}
