import QtQuick
import QtQuick.Controls
import OpenIPC

Rectangle {
    id: footer

    signal saveRequested()

    height: 60
    color: "transparent"
    z: 10

    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: Theme.controlBorder
    }

    Button {
        id: saveSettingsButton

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 20
        width: 120
        height: 36
        text: I18n.t("Сохранить")
        focusPolicy: Qt.StrongFocus

        background: Rectangle {
            color: saveSettingsButton.down
                   ? Theme.accentHover
                   : saveSettingsButton.hovered ? Theme.metroBlueHover : Theme.accent
            border.color: saveSettingsButton.visualFocus ? Theme.textPrimary : Theme.accent
            border.width: saveSettingsButton.visualFocus ? 2 : 1
            radius: Theme.radiusSm
        }

        contentItem: Text {
            text: saveSettingsButton.text
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.bold: true
            font.pixelSize: 14
        }

        onClicked: footer.saveRequested()
    }
}
