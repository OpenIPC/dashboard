import QtQuick
import QtQuick.Controls
import OpenIPC

AbstractButton {
    id: check

    checkable: true
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    font.family: Theme.metroFontFamily
    font.pixelSize: 12
    implicitWidth: contentItem.implicitWidth
    implicitHeight: Math.max(28, contentItem.implicitHeight + 8)
    padding: 0

    background: Item {}

    contentItem: Item {
        implicitWidth: box.width + (label.visible ? check.spacing + label.implicitWidth : 0)
        implicitHeight: Math.max(box.height, label.visible ? label.implicitHeight : 0)

        Rectangle {
            id: box

            width: 18
            height: 18
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            radius: Theme.metroTileRadius
            color: check.checked ? Theme.metroBlue : Theme.controlBackground
            border.color: check.checked
                          ? Theme.metroBlueHover
                          : (check.hovered || check.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke)
            border.width: check.hovered || check.visualFocus ? 2 : 1

            Canvas {
                anchors.centerIn: parent
                width: 11
                height: 9
                visible: check.checked
                onVisibleChanged: requestPaint()
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    if (!visible) return
                    ctx.beginPath()
                    ctx.moveTo(1, 5)
                    ctx.lineTo(4, 8)
                    ctx.lineTo(10, 1)
                    ctx.strokeStyle = Theme.textPrimary
                    ctx.lineWidth = 2
                    ctx.lineCap = "square"
                    ctx.lineJoin = "miter"
                    ctx.stroke()
                }
            }
        }

        Text {
            id: label

            visible: check.text.length > 0
            anchors.left: box.right
            anchors.leftMargin: check.spacing
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: check.text
            color: check.enabled ? Theme.textSecondary : Theme.textFaint
            font: check.font
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }
    }
}
