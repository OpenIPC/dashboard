import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property int currentPage: 0
    property int pageCount: 1
    property bool cycling: false
    property bool kioskActive: false
    property bool showKioskButton: true

    signal previousRequested()
    signal nextRequested()
    signal cyclingToggleRequested()
    signal kioskToggleRequested()

    implicitWidth: controls.implicitWidth + 12
    implicitHeight: 34
    radius: Theme.metroTileRadius
    color: "#d9151b26"
    border.color: Theme.metroStroke
    border.width: 1

    RowLayout {
        id: controls
        anchors.centerIn: parent
        spacing: 3

        component PageButton: Button {
            implicitWidth: 30
            implicitHeight: 26
            padding: 0
            hoverEnabled: true
            focusPolicy: Qt.StrongFocus
            background: Rectangle {
                radius: Theme.metroTileRadius
                color: parent.down ? Theme.metroTilePressed
                                   : (parent.hovered || parent.visualFocus ? Theme.metroTileHover : "transparent")
                border.color: parent.visualFocus ? Theme.metroStrokeStrong : "transparent"
            }
            contentItem: Text {
                text: parent.text
                color: parent.enabled ? Theme.textPrimary : Theme.textFaint
                font.family: Theme.metroFontFamily
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        PageButton {
            text: "‹"
            enabled: root.pageCount > 1
            onClicked: root.previousRequested()
            ToolTip.visible: hovered
            ToolTip.text: I18n.t("Предыдущая страница")
        }

        Text {
            Layout.minimumWidth: 46
            text: (root.currentPage + 1) + " / " + Math.max(1, root.pageCount)
            color: Theme.textSecondary
            font.family: Theme.metroFontFamily
            font.pixelSize: 12
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        PageButton {
            text: "›"
            enabled: root.pageCount > 1
            onClicked: root.nextRequested()
            ToolTip.visible: hovered
            ToolTip.text: I18n.t("Следующая страница")
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 20
            color: Theme.metroStroke
        }

        PageButton {
            text: root.cycling ? "Ⅱ" : "▶"
            enabled: root.pageCount > 1
            onClicked: root.cyclingToggleRequested()
            ToolTip.visible: hovered
            ToolTip.text: I18n.t(root.cycling ? "Остановить смену страниц" : "Автоматическая смена страниц")
        }

        PageButton {
            visible: root.showKioskButton
            text: root.kioskActive ? "×" : "⛶"
            onClicked: root.kioskToggleRequested()
            ToolTip.visible: hovered
            ToolTip.text: I18n.t(root.kioskActive ? "Выйти из режима киоска" : "Режим киоска")
        }
    }
}
