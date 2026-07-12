import QtQuick
import QtQuick.Controls
import OpenIPC

CheckBox {
    id: check

    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    spacing: 8
    font.family: Theme.metroFontFamily
    font.pixelSize: 12
    implicitHeight: Math.max(28, contentItem.implicitHeight + 8)

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: check.leftPadding
        y: check.height / 2 - height / 2
        radius: Theme.metroTileRadius
        color: check.checked ? Theme.metroBlue : Theme.controlBackground
        border.color: check.checked ? Theme.metroBlueHover : (check.hovered || check.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke)
        border.width: check.hovered || check.visualFocus ? 2 : 1

        Canvas {
            anchors.centerIn: parent
            width: 11
            height: 9
            visible: check.checked
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
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

    contentItem: Text {
        text: check.text
        color: check.enabled ? Theme.textSecondary : Theme.textFaint
        font: check.font
        verticalAlignment: Text.AlignVCenter
        leftPadding: check.indicator.width + check.spacing
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideRight
    }
}
