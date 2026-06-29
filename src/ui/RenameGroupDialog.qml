import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root

    modal: true
    width: 360
    height: 180
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property string oldName: ""
    property string newName: ""

    signal renameAccepted(string oldName, string newName)

    background: Rectangle {
        color: Theme.panelAltBackground
        radius: Theme.radiusMd
        border.color: Theme.panelBorderStrong
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: I18n.t("Переименовать группу")
            color: Theme.textPrimary
            font.pixelSize: 16
            font.bold: true
        }

        TextField {
            Layout.fillWidth: true
            text: root.newName
            color: Theme.textPrimary
            placeholderText: I18n.t("Новое имя группы")
            placeholderTextColor: Theme.textMuted
            selectByMouse: true
            background: Rectangle {
                color: Theme.controlBackground
                radius: Theme.radiusSm
                border.color: Theme.controlBorder
            }
            onTextChanged: root.newName = text
            Component.onCompleted: selectAll()
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            DashboardDialogButton {
                text: I18n.t("Отмена")
                Layout.preferredWidth: 92
                onClicked: root.close()
            }

            DashboardDialogButton {
                text: I18n.t("Сохранить")
                Layout.fillWidth: true
                buttonColor: Theme.accent
                buttonHoverColor: Theme.accentHover
                buttonBorderColor: Theme.accent
                buttonTextBold: true
                onClicked: {
                    var trimmed = root.newName.trim()
                    if (trimmed.length > 0) {
                        root.renameAccepted(root.oldName, trimmed)
                    }
                    root.close()
                }
            }
        }
    }
}
