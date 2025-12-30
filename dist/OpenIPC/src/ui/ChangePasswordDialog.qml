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
        color: "#252526"
        radius: 8
        border.color: "#444444"
    }
    
    header: Rectangle {
        height: 50
        color: "transparent"
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: root.title
            color: "white"
            font.pixelSize: 16
            font.bold: true
        }
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "×"
            color: "#aaaaaa"
            font.pixelSize: 20
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.close()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 15
        
        // Old Password
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Старый пароль")
                color: "white"
                Layout.preferredWidth: 130
            }
            TextField {
                id: oldPasswordField
                Layout.fillWidth: true
                placeholderText: I18n.t("Старый пароль")
                placeholderTextColor: "#aaaaaa"
                echoMode: TextInput.Password
                color: "white"
                background: Rectangle {
                    color: "#252526"
                    border.color: "#444444"
                    radius: 4
                }
            }
        }

        // New Password
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Новый пароль")
                color: "white"
                Layout.preferredWidth: 130
            }
            TextField {
                id: newPasswordField
                Layout.fillWidth: true
                placeholderText: I18n.t("Новый пароль")
                placeholderTextColor: "#aaaaaa"
                echoMode: TextInput.Password
                color: "white"
                background: Rectangle {
                    color: "#252526"
                    border.color: "#444444"
                    radius: 4
                }
            }
        }

        // Confirm Password
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Подтвердите пароль")
                color: "white"
                Layout.preferredWidth: 130
            }
            TextField {
                id: confirmPasswordField
                Layout.fillWidth: true
                placeholderText: I18n.t("Подтвердите пароль")
                placeholderTextColor: "#aaaaaa"
                echoMode: TextInput.Password
                color: "white"
                background: Rectangle {
                    color: "#252526"
                    border.color: "#444444"
                    radius: 4
                }
            }
        }

        Text {
            id: errorText
            Layout.fillWidth: true
            text: ""
            color: "#f44336"
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
                background: Rectangle {
                    color: parent.down ? "#388e3c" : "#4caf50"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
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
                background: Rectangle {
                    color: parent.down ? "#555555" : "#666666"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.close()
            }
        }
    }
}
