import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    title: I18n.t("Добавить нового пользователя")
    modal: true
    width: 400
    height: 450
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
        
        // Login
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Логин")
                color: "white"
                Layout.preferredWidth: 80
            }
            TextField {
                id: loginField
                Layout.fillWidth: true
                placeholderText: I18n.t("Логин")
                placeholderTextColor: "#aaaaaa"
                color: "white"
                background: Rectangle {
                    color: "#252526"
                    border.color: "#444444"
                    radius: 4
                }
            }
        }
        
        // Password
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Пароль")
                color: "white"
                Layout.preferredWidth: 80
            }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                placeholderText: I18n.t("Пароль")
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
        
        // Role
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Роль")
                color: "white"
                Layout.preferredWidth: 80
            }
            StyledComboBox {
                id: roleBox
                Layout.fillWidth: true
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: I18n.t("Администратор"), value: "admin" },
                    { text: I18n.t("Оператор"), value: "operator" }
                ]
            }
        }

        // Permissions
        Label {
            text: I18n.t("Права доступа")
            color: "white"
            font.bold: true
        }

        GridLayout {
            columns: 2
            Layout.fillWidth: true
            rowSpacing: 5
            columnSpacing: 10

            property bool isAdmin: roleBox.currentValue === "admin" || roleBox.currentIndex === 0

            component StyledCheckBox: CheckBox {
                hoverEnabled: false
                background: Item {} // Remove default background/ripple
                indicator: Rectangle {
                    implicitWidth: 18
                    implicitHeight: 18
                    x: parent.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 3
                    color: "#252526"
                    border.color: parent.checked ? "#4caf50" : "#666666"
                    
                    Rectangle {
                        width: 10
                        height: 10
                        anchors.centerIn: parent
                        radius: 2
                        color: "#4caf50"
                        visible: parent.parent.checked
                    }
                }
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    opacity: parent.enabled ? 1.0 : 0.5
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: parent.indicator.width + parent.spacing
                }
            }

            StyledCheckBox { 
                id: pLive
                text: I18n.t("Просмотр (Live)")
                checked: true
                enabled: !parent.isAdmin
            }
            StyledCheckBox { 
                id: pPlay
                text: I18n.t("Архив (Playback)")
                checked: true
                enabled: !parent.isAdmin
            }
            StyledCheckBox { 
                id: pPtz
                text: I18n.t("Управление PTZ")
                checked: true
                enabled: !parent.isAdmin
            }
            StyledCheckBox { 
                id: pExport
                text: I18n.t("Экспорт")
                checked: false
                enabled: !parent.isAdmin
            }
            StyledCheckBox { 
                id: pSettings
                text: I18n.t("Настройки")
                checked: false
                enabled: !parent.isAdmin
            }
            StyledCheckBox { 
                id: pUsers
                text: I18n.t("Пользователи")
                checked: false
                enabled: !parent.isAdmin
            }
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
                    if (loginField.text && passwordField.text) {
                        var perms = 0
                        if (roleBox.currentIndex === 0) { // Admin
                            perms = 0xFF // Perm_All
                        } else {
                            if (pLive.checked) perms |= 0x01
                            if (pPlay.checked) perms |= 0x02
                            if (pPtz.checked) perms |= 0x04
                            if (pExport.checked) perms |= 0x08
                            if (pSettings.checked) perms |= 0x10
                            if (pUsers.checked) perms |= 0x20
                        }

                        if (SystemController.userManager.addUser(loginField.text, passwordField.text, roleBox.currentValue, perms)) {
                            loginField.text = ""
                            passwordField.text = ""
                            root.close()
                        } else {
                            // Show error (user exists)
                        }
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
