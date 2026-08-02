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
            font.pixelSize: 18
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
                color: Theme.metroTile
                radius: Theme.metroTileRadius
                border.color: Theme.metroStroke
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10
                    
                    Text {
                        text: modelData.username
                        color: Theme.textPrimary
                        font.family: Theme.metroFontFamily
                        font.pixelSize: 14
                        font.bold: true
                    }
                    
                    Text {
                        text: "(" + (modelData.role === "admin" ? I18n.t("Администратор") : I18n.t("Оператор")) + ")"
                        color: Theme.textMuted
                        font.pixelSize: 14
                        Layout.fillWidth: true
                    }
                    
                    Button {
                        text: I18n.t("Права и камеры")
                        visible: modelData.role !== "admin" // Admins have all rights
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
                        onClicked: {
                            permDialog.username = modelData.username
                            permDialog.currentPerms = modelData.permissions
                            permDialog.currentScopes = modelData.cameraScopes || []
                            permDialog.open()
                        }
                    }

                    Button {
                        text: I18n.t("Сменить пароль")
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
                        onClicked: {
                            changePasswordDialog.username = modelData.username
                            changePasswordDialog.open()
                        }
                    }
                    
                    Button {
                        text: I18n.t("Удалить пользователя")
                        visible: modelData.username !== "admin" // Cannot delete main admin
                        hoverEnabled: true
                        background: Rectangle {
                            color: parent.down ? Qt.darker(Theme.metroRed, 1.2) : (parent.hovered ? Qt.lighter(Theme.metroRed, 1.08) : Theme.metroRed)
                            radius: Theme.metroTileRadius
                            border.color: Theme.metroRed
                        }
                        contentItem: Text {
                            text: parent.text
                            color: Theme.textPrimary
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
            color: Theme.metroStroke
        }
        
        Button {
            text: I18n.t("Добавить пользователя")
            Layout.preferredWidth: 200
            Layout.preferredHeight: 36
            hoverEnabled: true
            background: Rectangle {
                color: parent.down ? Theme.metroBlueHover : (parent.hovered ? Theme.metroBlueHover : Theme.metroBlue)
                radius: Theme.metroTileRadius
                border.color: Theme.metroBlue
            }
            contentItem: Text {
                text: parent.text
                color: Theme.textPrimary
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
        width: 520
        height: Math.min(680, root.parent ? root.parent.height - 32 : 680)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        
        property string username: ""
        property int currentPerms: 0
        property var currentScopes: []

        function scopeSelected(scopeKey) {
            for (var i = 0; i < currentScopes.length; ++i) {
                if (String(currentScopes[i]) === scopeKey) return true
            }
            return false
        }

        function selectedCameraScopes() {
            if (allCamerasScope.checked) return []
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
                text: permDialog.title
                color: Theme.textPrimary
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
                onClicked: permDialog.close()
            }
        }
        
        onOpened: {
            cbLive.checked = (currentPerms & 0x01)
            cbPlay.checked = (currentPerms & 0x02)
            cbPtz.checked = (currentPerms & 0x04)
            cbExport.checked = (currentPerms & 0x08)
            cbSettings.checked = (currentPerms & 0x10)
            cbUsers.checked = (currentPerms & 0x20)
            cbAnalytics.checked = (currentPerms & 0x40)
            cbTalk.checked = (currentPerms & 0x80)
            allCamerasScope.checked = currentScopes.length === 0
            for (var s = 0; s < siteScopeRepeater.count; ++s) {
                var siteItem = siteScopeRepeater.itemAt(s)
                if (siteItem) siteItem.checked = scopeSelected(siteItem.scopeKey)
            }
            for (var a = 0; a < areaScopeRepeater.count; ++a) {
                var areaItem = areaScopeRepeater.itemAt(a)
                if (areaItem) areaItem.checked = scopeSelected(areaItem.scopeKey)
            }
            for (var i = 0; i < scopeRepeater.count; ++i) {
                var item = scopeRepeater.itemAt(i)
                if (item) item.checked = scopeSelected(item.scopeKey)
            }
        }
        
        contentItem: ColumnLayout {
            spacing: 10
            
            Text {
                text: I18n.t("Пользователь: ") + permDialog.username
                color: Theme.textSecondary
                font.bold: true
                Layout.bottomMargin: 10
            }

            component StyledCheckBox: MetroCheckBox {
            }
            
            StyledCheckBox { id: cbLive; text: I18n.t("Просмотр (Live)") }
            StyledCheckBox { id: cbPlay; text: I18n.t("Архив (Playback)") }
            StyledCheckBox { id: cbPtz; text: I18n.t("Управление PTZ") }
            StyledCheckBox { id: cbExport; text: I18n.t("Экспорт") }
            StyledCheckBox { id: cbAnalytics; text: I18n.t("Аналитика") }
            StyledCheckBox { id: cbTalk; text: I18n.t("Разговор (Push-to-talk)") }
            StyledCheckBox { id: cbSettings; text: I18n.t("Настройки системы") }
            StyledCheckBox { id: cbUsers; text: I18n.t("Управление пользователями") }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.metroStroke
            }

            Text {
                text: I18n.t("Доступ к камерам")
                color: Theme.textPrimary
                font.bold: true
            }

            StyledCheckBox {
                id: allCamerasScope
                text: I18n.t("Все камеры")
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 130
                visible: !allCamerasScope.checked
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
                        delegate: StyledCheckBox {
                            required property var modelData
                            property string scopeKey: "site:" + modelData.id
                            width: parent ? parent.width : 0
                            text: modelData.name
                            checked: permDialog.scopeSelected(scopeKey)
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
                        delegate: StyledCheckBox {
                            required property var modelData
                            property string scopeKey: "area:" + modelData.id
                            width: parent ? parent.width : 0
                            text: modelData.name
                            checked: permDialog.scopeSelected(scopeKey)
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

                        delegate: StyledCheckBox {
                            required property int index
                            required property string cameraId
                            required property string cameraName
                            required property string cameraIp
                            property string scopeKey: cameraId !== "" ? cameraId : "index:" + index
                            width: parent ? parent.width : 0
                            text: (cameraName || cameraIp) + "  ·  " + cameraIp
                            checked: permDialog.scopeSelected(scopeKey)
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
                    hoverEnabled: true
                    background: Rectangle { 
                        color: parent.down ? Theme.metroBlueHover : (parent.hovered ? Theme.metroBlueHover : Theme.metroBlue)
                        radius: Theme.metroTileRadius
                        border.color: Theme.metroBlue
                    }
                    contentItem: Text { text: parent.text; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: {
                        var p = 0
                        if (cbLive.checked) p |= 0x01
                        if (cbPlay.checked) p |= 0x02
                        if (cbPtz.checked) p |= 0x04
                        if (cbExport.checked) p |= 0x08
                        if (cbSettings.checked) p |= 0x10
                        if (cbUsers.checked) p |= 0x20
                        if (cbAnalytics.checked) p |= 0x40
                        if (cbTalk.checked) p |= 0x80
                        
                        SystemController.userManager.updateUserPermissions(permDialog.username, p)
                        SystemController.userManager.updateUserCameraScopes(
                                    permDialog.username, permDialog.selectedCameraScopes())
                        permDialog.close()
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
                    contentItem: Text { text: parent.text; color: Theme.textPrimary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: permDialog.close()
                }
            }
        }
    }
}
