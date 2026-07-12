import QtQuick
import QtQuick.Controls
import OpenIPC

ComboBox {
    id: combo

    signal userSelected(int index)

    implicitHeight: 36
    focusPolicy: Qt.StrongFocus

    onActivated: function(index) {
        combo.userSelected(index)
    }

    contentItem: Text {
        text: combo.displayText
        color: combo.enabled ? Theme.textSecondary : Theme.textFaint
        font.family: Theme.metroFontFamily
        font.pixelSize: 12
        verticalAlignment: Text.AlignVCenter
        leftPadding: 10
        rightPadding: 26
        elide: Text.ElideRight
    }

    background: Rectangle {
        color: combo.enabled ? Theme.controlBackground : Theme.metroTileDisabled
        radius: Theme.metroTileRadius
        border.color: combo.visualFocus || combo.hovered ? Theme.metroStrokeStrong : Theme.metroStroke
        border.width: combo.visualFocus || combo.hovered ? 2 : 1
    }

    indicator: Canvas {
        width: 16
        height: 10
        x: combo.width - width - 10
        y: (combo.height - height) / 2
        opacity: combo.enabled ? 1.0 : 0.45
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.beginPath()
            ctx.moveTo(3, 3)
            ctx.lineTo(width / 2, height - 2)
            ctx.lineTo(width - 3, 3)
            ctx.strokeStyle = Theme.textMuted
            ctx.lineWidth = 1.8
            ctx.lineCap = "square"
            ctx.stroke()
        }
    }

    popup: Popup {
        y: combo.height + 4
        width: combo.width
        padding: 4
        background: Rectangle {
            color: Theme.metroSurface
            radius: Theme.metroTileRadius
            border.color: Theme.metroStroke
        }

        contentItem: ListView {
            implicitHeight: Math.min(contentHeight, 260)
            model: combo.popup.visible ? combo.count : 0
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: StyledScrollBar {}

            delegate: Rectangle {
                id: row
                width: ListView.view ? ListView.view.width : combo.width
                height: 32
                radius: Theme.metroTileRadius
                color: mouse.containsMouse || combo.currentIndex === index ? Theme.metroTileHover : "transparent"

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    text: combo.textAt(index)
                    color: Theme.textSecondary
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        combo.currentIndex = index
                        combo.userSelected(index)
                        combo.popup.close()
                    }
                }
            }
        }
    }
}
