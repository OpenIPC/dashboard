import QtQuick
import QtQuick.Controls
import OpenIPC

ComboBox {
    id: combo
    implicitHeight: 34
    contentItem: Text {
        leftPadding: 10
        rightPadding: 24
        text: combo.displayText
        color: combo.enabled ? Theme.textPrimary : Theme.textFaint
        font.pixelSize: 12
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    indicator: Text {
        x: combo.width - width - 10
        y: combo.topPadding + (combo.availableHeight - height) / 2
        text: "⌄"
        color: Theme.textMuted
        font.pixelSize: 14
    }
    background: Rectangle {
        radius: Theme.radiusMd
        color: Theme.controlBackground
        border.color: combo.hovered || combo.activeFocus ? Theme.accent : Theme.controlBorder
    }
    popup: Popup {
        y: combo.height + 4
        width: combo.width
        implicitHeight: Math.min(contentItem.implicitHeight + 2, 280)
        padding: 1
        background: Rectangle {
            radius: Theme.radiusMd
            color: Theme.panelSoftBackground
            border.color: Theme.controlBorder
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
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            color: comboDelegate.highlighted ? Theme.cardHover : "transparent"
            radius: Theme.radiusSm
        }
    }
}
