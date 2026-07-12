import QtQuick
import QtQuick.Controls
import OpenIPC

ComboBox {
    id: combo
    implicitHeight: 36
    contentItem: Text {
        leftPadding: 10
        rightPadding: 24
        text: combo.displayText
        color: combo.enabled ? Theme.textPrimary : Theme.textFaint
        font.family: Theme.metroFontFamily
        font.pixelSize: 12
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    indicator: Canvas {
        x: combo.width - width - 10
        y: combo.topPadding + (combo.availableHeight - height) / 2
        width: 10
        height: 7
        opacity: combo.enabled ? 1 : 0.45
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.beginPath()
            ctx.moveTo(1, 1)
            ctx.lineTo(width / 2, height - 1)
            ctx.lineTo(width - 1, 1)
            ctx.strokeStyle = Theme.textMuted
            ctx.lineWidth = 1.8
            ctx.lineCap = "square"
            ctx.stroke()
        }
    }
    background: Rectangle {
        radius: Theme.metroTileRadius
        color: combo.enabled ? Theme.controlBackground : Theme.metroTileDisabled
        border.color: combo.hovered || combo.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
        border.width: combo.hovered || combo.activeFocus ? 2 : 1
    }
    popup: Popup {
        y: combo.height + 4
        width: combo.width
        implicitHeight: Math.min(contentItem.implicitHeight + 2, 280)
        padding: 1
        background: Rectangle {
            radius: Theme.metroTileRadius
            color: Theme.metroSurface
            border.color: Theme.metroStroke
        }
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: combo.popup.visible ? combo.delegateModel : null
            currentIndex: combo.highlightedIndex
        }
    }
    delegate: ItemDelegate {
        id: comboDelegate
        required property int index
        required property var modelData

        width: ListView.view ? ListView.view.width : implicitWidth
        height: 32
        highlighted: ListView.isCurrentItem
        contentItem: Text {
            text: String(comboDelegate.modelData)
            color: comboDelegate.highlighted ? Theme.textPrimary : Theme.textSecondary
            font.family: Theme.metroFontFamily
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            color: comboDelegate.highlighted ? Theme.metroTileHover : "transparent"
            radius: Theme.metroTileRadius
        }
    }
}
