import QtQuick
import QtQuick.Controls
import OpenIPC

ComboBox {
    id: combo

    signal userSelected(int index)

    implicitHeight: 34

    onActivated: function(index) {
        combo.userSelected(index)
    }

    contentItem: Text {
        text: combo.displayText
        color: Theme.textSecondary
        verticalAlignment: Text.AlignVCenter
        leftPadding: 10
        rightPadding: 26
        elide: Text.ElideRight
    }

    background: Rectangle {
        color: Theme.controlBackground
        radius: Theme.radiusSm
        border.color: combo.visualFocus ? Theme.accent : Theme.controlBorder
        border.width: 1
    }

    indicator: Text {
        text: "\u25BE"
        color: Theme.textMuted
        font.pixelSize: 13
        width: 16
        height: combo.height
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        x: combo.width - width - 10
        y: 0
        opacity: combo.enabled ? 1.0 : 0.45
    }

    popup: Popup {
        y: combo.height + 4
        width: combo.width
        padding: 4
        background: Rectangle {
            color: Theme.panelAltBackground
            radius: Theme.radiusSm
            border.color: Theme.controlBorder
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
                radius: Theme.radiusXs
                color: mouse.containsMouse || combo.currentIndex === index ? Theme.cardHover : "transparent"

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    text: combo.textAt(index)
                    color: Theme.textSecondary
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
