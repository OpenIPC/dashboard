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
    readonly property int boxSize: 18
    readonly property real labelImplicitWidth: text.length > 0 ? label.implicitWidth : 0
    readonly property real labelImplicitHeight: text.length > 0 ? label.implicitHeight : 0

    implicitWidth: boxSize + (text.length > 0 ? spacing + labelImplicitWidth : 0)
    implicitHeight: Math.max(28, Math.max(boxSize, labelImplicitHeight) + 8)
    padding: 0

    background: Item {}

    contentItem: Item {
        Rectangle {
            id: box

            width: check.boxSize
            height: check.boxSize
            x: 0
            y: Math.round((parent.height - height) / 2)
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
            x: box.width + check.spacing
            y: Math.round((parent.height - height) / 2)
            width: check.width > check.implicitWidth
                   ? Math.max(0, check.width - box.width - check.spacing)
                   : implicitWidth
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
