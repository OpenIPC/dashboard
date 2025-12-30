import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    title: I18n.t("Управление пользователями")
    modal: true
    width: 700
    height: 500
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    background: Rectangle {
        color: "#252526"
        radius: 8
        border.color: "#3c3c3c"
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
            font.pixelSize: 18
            font.bold: true
        }
        
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "×"
            color: "#aaaaaa"
            font.pixelSize: 24
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.close()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 20
        
        ListView {
            id: userList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: SystemController.userManager.users
            spacing: 10
            
            delegate: Rectangle {
                width: userList.width
                height: 60
                color: "#333333"
                radius: 4
                border.color: "#444444"
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10
                    
                    Text {
                        text: modelData.username
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                    }
                    
                    Text {
                        text: "(" + (modelData.role === "admin" ? I18n.t("Администратор") : I18n.t("Оператор")) + ")"
                        color: "#aaaaaa"
                        font.pixelSize: 14
                        Layout.fillWidth: true
                    }
                    
                    Button {
                        text: I18n.t("Права")
                        visible: modelData.role !== "admin" // Admins have all rights
                        background: Rectangle {
                            color: parent.down ? "#555555" : "#444444"
                            radius: 4
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            permDialog.username = modelData.username
                            permDialog.currentPerms = modelData.permissions
                            permDialog.open()
                        }
                    }

                    Button {
                        text: I18n.t("Сменить пароль")
                        background: Rectangle {
                            color: parent.down ? "#555555" : "#444444"
                            radius: 4
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            changePasswordDialog.username = modelData.username
                            changePasswordDialog.open()
                        }
                    }
                    
                    Button {
                        text: I18n.t("Удалить пользователя")
                        visible: modelData.username !== "admin" // Cannot delete main admin
                        background: Rectangle {
                            color: parent.down ? "#b71c1c" : "#d32f2f"
                            radius: 4
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (SystemController.userManager.deleteUser(modelData.username)) {
                                // Success
                            }
                        }
                    }
                }
            }
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#3c3c3c"
        }
        
        Button {
            text: I18n.t("Добавить пользователя")
            Layout.preferredWidth: 200
            Layout.preferredHeight: 36
            background: Rectangle {
                color: parent.down ? "#388e3c" : "#4caf50"
                radius: 4
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: addUserDialog.open()
        }
    }
    
    AddUserDialog { id: addUserDialog }
    ChangePasswordDialog { id: changePasswordDialog }

    Dialog {
        id: permDialog
        title: I18n.t("Настройка прав доступа")
        modal: true
        width: 350
        height: 400
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        
        property string username: ""
        property int currentPerms: 0
        
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
                text: permDialog.title
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
                    onClicked: permDialog.close()
                }
            }
        }
        
        onOpened: {
            cbLive.checked = (currentPerms & 0x01)
            cbPlay.checked = (currentPerms & 0x02)
            cbPtz.checked = (currentPerms & 0x04)
            cbExport.checked = (currentPerms & 0x08)
            cbSettings.checked = (currentPerms & 0x10)
            cbUsers.checked = (currentPerms & 0x20)
        }
        
        contentItem: ColumnLayout {
            spacing: 10
            
            Text {
                text: I18n.t("Пользователь: ") + permDialog.username
                color: "#cccccc"
                font.bold: true
                Layout.bottomMargin: 10
            }

            component StyledCheckBox: CheckBox {
                hoverEnabled: false
                background: Item {}
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
            
            StyledCheckBox { id: cbLive; text: I18n.t("Просмотр (Live)") }
            StyledCheckBox { id: cbPlay; text: I18n.t("Архив (Playback)") }
            StyledCheckBox { id: cbPtz; text: I18n.t("Управление PTZ") }
            StyledCheckBox { id: cbExport; text: I18n.t("Экспорт") }
            StyledCheckBox { id: cbSettings; text: I18n.t("Настройки системы") }
            StyledCheckBox { id: cbUsers; text: I18n.t("Управление пользователями") }
            
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
                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: {
                        var p = 0
                        if (cbLive.checked) p |= 0x01
                        if (cbPlay.checked) p |= 0x02
                        if (cbPtz.checked) p |= 0x04
                        if (cbExport.checked) p |= 0x08
                        if (cbSettings.checked) p |= 0x10
                        if (cbUsers.checked) p |= 0x20
                        
                        SystemController.userManager.updateUserPermissions(permDialog.username, p)
                        permDialog.close()
                    }
                }
                Button {
                    text: I18n.t("Отмена")
                    background: Rectangle { 
                        color: parent.down ? "#555555" : "#666666"
                        radius: 4 
                    }
                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: permDialog.close()
                }
            }
        }
    }
}
