import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Popup {
    id: popup

    anchors.centerIn: parent
    width: 240
    height: 40
    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    background: Rectangle {
        color: Theme.metroTile
        radius: Theme.radiusSm
        border.color: Theme.accent
        border.width: 1
    }

    contentItem: RowLayout {
        anchors.centerIn: parent
        spacing: 8

        Text {
            text: "✓"
            color: Theme.accent
            font.bold: true
            font.pixelSize: 16
        }

        Text {
            text: I18n.t("Настройки успешно сохранены")
            color: "white"
            font.pixelSize: 14
        }
    }

    Timer {
        interval: 2000
        running: popup.visible
        onTriggered: popup.close()
    }
}
