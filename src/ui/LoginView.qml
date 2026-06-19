import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root
    color: "#1e1e1e" // Main background

    property bool authenticating: false
    property string errorMessage: ""

    function submitLogin() {
        root.errorMessage = ""
        if (usernameInput.text === "" || passwordInput.text === "") {
            root.errorMessage = I18n.t("Введите логин и пароль")
            return
        }

        if (!SystemController.userManager.hasUsers) {
            if (confirmPasswordInput.text === "") {
                root.errorMessage = I18n.t("Подтвердите пароль")
                return
            }
            if (passwordInput.text !== confirmPasswordInput.text) {
                root.errorMessage = I18n.t("Пароли не совпадают")
                return
            }
            if (!SystemController.userManager.setupInitialAdmin(usernameInput.text, passwordInput.text, rememberMeCheck.checked)) {
                root.errorMessage = I18n.t("Не удалось создать администратора")
            }
            return
        }

        if (!SystemController.userManager.login(usernameInput.text, passwordInput.text, rememberMeCheck.checked)) {
            root.errorMessage = I18n.t("Неверный логин или пароль")
        }
    }

    function syncFormState() {
        usernameInput.text = SystemController.userManager.rememberedUsername
        passwordInput.text = SystemController.userManager.rememberedPassword
        confirmPasswordInput.text = ""
        rememberMeCheck.checked = SystemController.userManager.rememberedUsername !== ""
                                  || SystemController.userManager.rememberedPassword !== ""
        root.errorMessage = ""
    }

    Component.onCompleted: syncFormState()

    Connections {
        target: SystemController.userManager

        function onIsLoggedInChanged() {
            if (!SystemController.userManager.isLoggedIn) {
                root.syncFormState()
            }
        }

        function onRememberedUsernameChanged() {
            if (!SystemController.userManager.isLoggedIn) {
                root.syncFormState()
            }
        }

        function onRememberedPasswordChanged() {
            if (!SystemController.userManager.isLoggedIn) {
                root.syncFormState()
            }
        }
    }

    // Center Card
    Rectangle {
        width: 360
        height: SystemController.userManager.hasUsers ? 440 : 520
        anchors.centerIn: parent
        color: "#2a2f33"
        radius: 4
        
        // Shadow effect (simulated with border for now, or just dark background)
        border.color: "#3c3c3c"
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 25
            spacing: 12

            // Title
            Text {
                Layout.fillWidth: true
                text: SystemController.userManager.hasUsers
                    ? I18n.t("Пожалуйста, войдите для продолжения")
                    : I18n.t("Создайте первого администратора")
                color: "white"
                font.pixelSize: 20
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                lineHeight: 1.2
            }

            // Username Field
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5
                Text {
                    text: I18n.t("Логин")
                    color: "#2196f3" // Blue label color
                    font.pixelSize: 12
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    color: "transparent"
                    border.color: "#2196f3" // Blue border for active/filled
                    border.width: 1
                    radius: 4
                    
                    TextInput {
                        id: usernameInput
                        anchors.fill: parent
                        anchors.margins: 10
                        text: ""
                        color: "white"
                        font.pixelSize: 14
                        verticalAlignment: Text.AlignVCenter
                        activeFocusOnPress: true
                        
                        // Placeholder behavior
                        Text {
                            anchors.fill: parent
                            text: "" // No placeholder in screenshot, label is above
                            color: "#666666"
                            visible: !parent.text && !parent.activeFocus
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Password Field
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5
                Text {
                    text: I18n.t("Пароль")
                    color: "#666666" // Grey label when not focused (simulated)
                    font.pixelSize: 12
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    color: "#333333" // Darker background for inactive
                    border.color: "#444444"
                    border.width: 1
                    radius: 4
                    
                    TextInput {
                        id: passwordInput
                        anchors.fill: parent
                        anchors.margins: 10
                        text: ""
                        echoMode: TextInput.Password
                        color: "white"
                        font.pixelSize: 14
                        verticalAlignment: Text.AlignVCenter
                        activeFocusOnPress: true
                        onAccepted: root.submitLogin()
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5
                visible: !SystemController.userManager.hasUsers

                Text {
                    text: I18n.t("Подтвердите пароль")
                    color: "#666666"
                    font.pixelSize: 12
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    color: "#333333"
                    border.color: "#444444"
                    border.width: 1
                    radius: 4

                    TextInput {
                        id: confirmPasswordInput
                        anchors.fill: parent
                        anchors.margins: 10
                        text: ""
                        echoMode: TextInput.Password
                        color: "white"
                        font.pixelSize: 14
                        verticalAlignment: Text.AlignVCenter
                        activeFocusOnPress: true
                        onAccepted: root.submitLogin()
                    }
                }
            }

            // Remember Me
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                CheckBox {
                    id: rememberMeCheck
                    text: I18n.t("Запомнить меня")
                    hoverEnabled: false
                    background: Item {}
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 14
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: parent.indicator.width + parent.spacing
                    }
                    
                    indicator: Rectangle {
                        implicitWidth: 18
                        implicitHeight: 18
                        x: parent.leftPadding
                        y: parent.height / 2 - height / 2
                        radius: 2
                        color: "transparent"
                        border.color: "#666666"
                        
                        Rectangle {
                            width: 10
                            height: 10
                            anchors.centerIn: parent
                            color: "#2196f3"
                            visible: parent.parent.checked
                        }
                    }
                }
            }

            // Error Message
            Text {
                Layout.fillWidth: true
                text: root.errorMessage
                color: "#f44336"
                font.pixelSize: 12
                visible: root.errorMessage !== ""
                horizontalAlignment: Text.AlignHCenter
            }

            // Default Credentials Hint
            Text {
                Layout.fillWidth: true
                text: SystemController.userManager.hasUsers
                    ? I18n.t("Будет запомнен логин и пароль для следующего входа.")
                    : I18n.t("Для первого запуска создайте учетную запись администратора.")
                color: "#888888"
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Text {
                visible: false
                Layout.fillWidth: true
                text: SystemController.userManager.hasUsers
                    ? I18n.t("Будет запомнено только имя пользователя. Пароль всегда требуется вводить заново.")
                    : I18n.t("Для первого запуска создайте учетную запись администратора.")
                color: "#888888"
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            // Login Button
            Button {
                id: loginButton
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                
                background: Rectangle {
                    color: parent.down ? "#1976d2" : "#2196f3"
                    radius: 4
                }
                
                contentItem: Text {
                    text: SystemController.userManager.hasUsers
                        ? I18n.t("ВОЙТИ")
                        : I18n.t("СОЗДАТЬ АДМИНИСТРАТОРА")
                    color: "white"
                    font.bold: true
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: root.submitLogin()
            }
        }
    }
}
