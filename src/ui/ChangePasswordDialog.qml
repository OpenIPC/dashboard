import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    property string username: ""
    title: I18n.t("Сменить пароль для") + " '" + username + "'"
    modal: true
    width: 400
    height: 350
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    background: Rectangle {
        color: Theme.metroSidebarBackground
        radius: Theme.metroTileRadius
        border.color: Theme.metroStroke
    }
    
    header: Rectangle {
        height: 50
        color: "transparent"
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: root.title
            color: Theme.textPrimary
            font.family: Theme.metroFontFamily
            font.pixelSize: 16
            font.bold: true
        }
        MetroWindowButton {
            kind: "close"
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            width: 38
            height: 34
            onClicked: root.close()
        }
    }

    contentItem: ColumnLayout {
        spacing: 15
        
        // Old Password
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Старый пароль")
                color: Theme.textSecondary
                Layout.preferredWidth: 130
            }
            TextField {
                id: oldPasswordField
                Layout.fillWidth: true
                placeholderText: I18n.t("Старый пароль")
                placeholderTextColor: Theme.textMuted
                echoMode: TextInput.Password
                color: Theme.textPrimary
                selectionColor: Theme.metroBlue
                selectedTextColor: Theme.textPrimary
                background: Rectangle {
                    color: Theme.metroSurfaceAlt
                    border.color: oldPasswordField.activeFocus ? Theme.metroBlue : Theme.metroStroke
                    radius: Theme.metroTileRadius
                }
            }
        }

        // New Password
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Новый пароль")
                color: Theme.textSecondary
                Layout.preferredWidth: 130
            }
            TextField {
                id: newPasswordField
                Layout.fillWidth: true
                placeholderText: I18n.t("Новый пароль")
                placeholderTextColor: Theme.textMuted
                echoMode: TextInput.Password
                color: Theme.textPrimary
                selectionColor: Theme.metroBlue
                selectedTextColor: Theme.textPrimary
                background: Rectangle {
                    color: Theme.metroSurfaceAlt
                    border.color: newPasswordField.activeFocus ? Theme.metroBlue : Theme.metroStroke
                    radius: Theme.metroTileRadius
                }
            }
        }

        // Confirm Password
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Подтвердите пароль")
                color: Theme.textSecondary
                Layout.preferredWidth: 130
            }
            TextField {
                id: confirmPasswordField
                Layout.fillWidth: true
                placeholderText: I18n.t("Подтвердите пароль")
                placeholderTextColor: Theme.textMuted
                echoMode: TextInput.Password
                color: Theme.textPrimary
                selectionColor: Theme.metroBlue
                selectedTextColor: Theme.textPrimary
                background: Rectangle {
                    color: Theme.metroSurfaceAlt
                    border.color: confirmPasswordField.activeFocus ? Theme.metroBlue : Theme.metroStroke
                    radius: Theme.metroTileRadius
                }
            }
        }

        Text {
            id: errorText
            Layout.fillWidth: true
            text: ""
            color: Theme.metroRed
            font.pixelSize: 12
            visible: text !== ""
            horizontalAlignment: Text.AlignHCenter
        }
        
        Item { Layout.fillHeight: true }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Button {
                text: I18n.t("Сохранить")
                hoverEnabled: true
                background: Rectangle {
                    color: parent.down ? Theme.metroBlueHover : (parent.hovered ? Theme.metroBlueHover : Theme.metroBlue)
                    radius: Theme.metroTileRadius
                    border.color: Theme.metroBlue
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    errorText.text = ""
                    if (newPasswordField.text !== confirmPasswordField.text) {
                        errorText.text = I18n.t("Пароли не совпадают")
                        return
                    }
                    if (newPasswordField.text === "") {
                        return
                    }

                    if (SystemController.userManager.changePassword(username, oldPasswordField.text, newPasswordField.text)) {
                        oldPasswordField.text = ""
                        newPasswordField.text = ""
                        confirmPasswordField.text = ""
                        errorText.text = ""
                        root.close()
                    } else {
                        errorText.text = I18n.t("Неверный старый пароль")
                    }
                }
            }
            
            Button {
                text: I18n.t("Отмена")
                hoverEnabled: true
                background: Rectangle {
                    color: parent.down ? Theme.metroTilePressed : (parent.hovered ? Theme.metroTileHover : Theme.metroTile)
                    radius: Theme.metroTileRadius
                    border.color: Theme.metroStroke
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.close()
            }
        }
    }
}
