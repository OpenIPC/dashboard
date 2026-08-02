import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    title: I18n.t("Добавить нового пользователя")
    modal: true
    width: 520
    height: Math.min(650, parent ? parent.height - 32 : 650)
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    function selectedCameraScopes() {
        if (roleBox.currentIndex === 0 || allCamerasScope.checked) return []
        var scopes = []
        for (var s = 0; s < siteScopeRepeater.count; ++s) {
            var siteItem = siteScopeRepeater.itemAt(s)
            if (siteItem && siteItem.checked) scopes.push(siteItem.scopeKey)
        }
        for (var a = 0; a < areaScopeRepeater.count; ++a) {
            var areaItem = areaScopeRepeater.itemAt(a)
            if (areaItem && areaItem.checked) scopes.push(areaItem.scopeKey)
        }
        for (var i = 0; i < scopeRepeater.count; ++i) {
            var item = scopeRepeater.itemAt(i)
            if (item && item.checked) scopes.push(item.scopeKey)
        }
        return scopes
    }

    onOpened: {
        allCamerasScope.checked = true
        for (var s = 0; s < siteScopeRepeater.count; ++s) {
            var siteItem = siteScopeRepeater.itemAt(s)
            if (siteItem) siteItem.checked = false
        }
        for (var a = 0; a < areaScopeRepeater.count; ++a) {
            var areaItem = areaScopeRepeater.itemAt(a)
            if (areaItem) areaItem.checked = false
        }
        for (var i = 0; i < scopeRepeater.count; ++i) {
            var item = scopeRepeater.itemAt(i)
            if (item) item.checked = false
        }
    }
    
    background: Rectangle {
        color: Theme.metroSurface
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
        
        // Login
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Логин")
                color: Theme.textSecondary
                font.family: Theme.metroFontFamily
                Layout.preferredWidth: 80
            }
            TextField {
                id: loginField
                Layout.fillWidth: true
                placeholderText: I18n.t("Логин")
                placeholderTextColor: Theme.textFaint
                color: Theme.textPrimary
                font.family: Theme.metroFontFamily
                background: Rectangle {
                    color: Theme.controlBackground
                    border.color: loginField.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                    border.width: loginField.activeFocus ? 2 : 1
                    radius: Theme.metroTileRadius
                }
            }
        }
        
        // Password
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Пароль")
                color: Theme.textSecondary
                font.family: Theme.metroFontFamily
                Layout.preferredWidth: 80
            }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                placeholderText: I18n.t("Пароль")
                placeholderTextColor: Theme.textFaint
                echoMode: TextInput.Password
                color: Theme.textPrimary
                font.family: Theme.metroFontFamily
                background: Rectangle {
                    color: Theme.controlBackground
                    border.color: passwordField.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                    border.width: passwordField.activeFocus ? 2 : 1
                    radius: Theme.metroTileRadius
                }
            }
        }
        
        // Role
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Роль")
                color: Theme.textSecondary
                font.family: Theme.metroFontFamily
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
            color: Theme.textPrimary
            font.family: Theme.metroFontFamily
            font.bold: true
        }

        GridLayout {
            columns: 2
            Layout.fillWidth: true
            rowSpacing: 5
            columnSpacing: 10

            property bool isAdmin: roleBox.currentValue === "admin" || roleBox.currentIndex === 0

            component StyledCheckBox: MetroCheckBox {
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
                id: pAnalytics
                text: I18n.t("Аналитика")
                checked: true
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
            StyledCheckBox {
                id: pTalk
                text: I18n.t("Разговор (Push-to-talk)")
                checked: false
                enabled: !parent.isAdmin
            }
        }

        Label {
            visible: roleBox.currentIndex !== 0
            text: I18n.t("Доступ к камерам")
            color: Theme.textPrimary
            font.family: Theme.metroFontFamily
            font.bold: true
        }

        MetroCheckBox {
            id: allCamerasScope
            visible: roleBox.currentIndex !== 0
            text: I18n.t("Все камеры")
            checked: true
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 130
            visible: roleBox.currentIndex !== 0 && !allCamerasScope.checked
            clip: true

            Column {
                width: parent.width
                spacing: 3

                Text {
                    text: I18n.t("Сайты")
                    color: Theme.textMuted
                    font.bold: true
                }
                Repeater {
                    id: siteScopeRepeater
                    model: SystemController.fleetManager.sites
                    delegate: MetroCheckBox {
                        required property var modelData
                        property string scopeKey: "site:" + modelData.id
                        width: parent ? parent.width : 0
                        text: modelData.name
                    }
                }

                Text {
                    text: I18n.t("Зоны")
                    color: Theme.textMuted
                    font.bold: true
                }
                Repeater {
                    id: areaScopeRepeater
                    model: SystemController.fleetManager.areas
                    delegate: MetroCheckBox {
                        required property var modelData
                        property string scopeKey: "area:" + modelData.id
                        width: parent ? parent.width : 0
                        text: modelData.name
                    }
                }

                Text {
                    text: I18n.t("Отдельные камеры")
                    color: Theme.textMuted
                    font.bold: true
                }

                Repeater {
                    id: scopeRepeater
                    model: SystemController.cameraModel

                    delegate: MetroCheckBox {
                        required property int index
                        required property string cameraId
                        required property string cameraName
                        required property string cameraIp
                        property string scopeKey: cameraId !== "" ? cameraId : "index:" + index
                        width: parent ? parent.width : 0
                        text: (cameraName || cameraIp) + "  ·  " + cameraIp
                    }
                }
            }
        }
        
        Item { Layout.fillHeight: true }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Button {
                text: I18n.t("Сохранить")
                background: Rectangle {
                    color: parent.down ? Theme.metroBlueHover : (parent.hovered ? Theme.metroBlueHover : Theme.metroBlue)
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 12
                    font.bold: true
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
                            if (pAnalytics.checked) perms |= 0x40
                            if (pTalk.checked) perms |= 0x80
                        }

                        var newUsername = loginField.text
                        if (SystemController.userManager.addUser(newUsername, passwordField.text, roleBox.currentValue, perms)) {
                            SystemController.userManager.updateUserCameraScopes(newUsername, root.selectedCameraScopes())
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
                    color: parent.down ? Theme.metroTilePressed : (parent.hovered ? Theme.metroTileHover : Theme.metroTile)
                    radius: Theme.metroTileRadius
                    border.color: parent.hovered ? Theme.metroStrokeStrong : Theme.metroStroke
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.close()
            }
        }
    }
}
