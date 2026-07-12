import QtQuick
import QtQuick.Controls
import OpenIPC

Button {
    id: control

    property string kind: "close" // minimize | maximize | close
    property bool maximized: false
    readonly property string tooltipText: kind === "minimize"
                                             ? I18n.t("Свернуть")
                                             : kind === "maximize"
                                               ? (maximized ? I18n.t("Восстановить") : I18n.t("Развернуть"))
                                               : I18n.t("Закрыть")

    implicitWidth: 40
    implicitHeight: 32
    padding: 0
    hoverEnabled: true
    focusPolicy: Qt.NoFocus

    ToolTip.visible: control.hovered
    ToolTip.text: control.tooltipText
    ToolTip.delay: 500

    background: Rectangle {
        radius: Theme.metroTileRadius
        color: !control.enabled
               ? Theme.metroTileDisabled
               : control.kind === "close"
                 ? (control.down ? Theme.metroRed : (control.hovered ? Theme.metroRed : "transparent"))
                 : (control.down ? Theme.metroTilePressed : (control.hovered ? Theme.metroTileHover : "transparent"))
        border.color: control.hovered
                      ? (control.kind === "close" ? Theme.metroRed : Theme.metroStrokeStrong)
                      : "transparent"
        border.width: control.hovered || control.visualFocus ? 1 : 0
    }

    contentItem: Item {
        Canvas {
            id: iconCanvas
            anchors.centerIn: parent
            width: 16
            height: 16
            opacity: control.enabled ? 1.0 : 0.45

            Connections {
                target: control
                function onKindChanged() { iconCanvas.requestPaint() }
                function onMaximizedChanged() { iconCanvas.requestPaint() }
                function onHoveredChanged() { iconCanvas.requestPaint() }
                function onDownChanged() { iconCanvas.requestPaint() }
            }

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.strokeStyle = Theme.textPrimary
                ctx.lineWidth = 1.7
                ctx.lineCap = "square"
                ctx.lineJoin = "miter"

                if (control.kind === "minimize") {
                    ctx.beginPath()
                    ctx.moveTo(4, 10.5)
                    ctx.lineTo(12, 10.5)
                    ctx.stroke()
                    return
                }

                if (control.kind === "maximize") {
                    if (control.maximized) {
                        ctx.strokeRect(4.5, 6.5, 7, 6)
                        ctx.strokeRect(6.5, 3.5, 7, 6)
                    } else {
                        ctx.strokeRect(4.5, 4.5, 8, 8)
                    }
                    return
                }

                ctx.beginPath()
                ctx.moveTo(4.5, 4.5)
                ctx.lineTo(11.5, 11.5)
                ctx.moveTo(11.5, 4.5)
                ctx.lineTo(4.5, 11.5)
                ctx.stroke()
            }
        }
    }
}
