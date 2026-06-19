import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    title: isEditMode ? I18n.t("Редактировать камеру") : I18n.t("Добавить новую камеру")
    modal: true
    width: 600
    height: 650
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    
    property string initialName: ""
    property string initialIp: ""
    property int initialPort: 554
    property int initialOnvifPort: 80
    
    property bool isEditMode: false
    property int editIndex: -1
    property string initialLogin: "root"
    property string initialPassword: ""
    property string initialHdUrl: ""
    property string initialSdUrl: ""
    
    onOpened: {
        nameField.text = initialName
        ipField.text = initialIp
        rtspPortField.text = initialPort.toString()
        onvifPortField.text = initialOnvifPort.toString()
        loginField.text = initialLogin
        passwordField.text = initialPassword
        
        if (isEditMode) {
            var stdHd = generateRtspUrl(initialLogin, initialPassword, initialIp, initialPort.toString(), hdProfileField.text, "OpenIPC")
            var hikHd = generateRtspUrl(initialLogin, initialPassword, initialIp, initialPort.toString(), hdProfileField.text, "Hikvision")
            var dahuaHd = generateRtspUrl(initialLogin, initialPassword, initialIp, initialPort.toString(), hdProfileField.text, "Dahua")
            
            if (initialHdUrl === stdHd) {
                urlTemplateCombo.currentIndex = 0
            } else if (initialHdUrl === hikHd) {
                urlTemplateCombo.currentIndex = 1
            } else if (initialHdUrl === dahuaHd) {
                urlTemplateCombo.currentIndex = 2
            } else {
                urlTemplateCombo.currentIndex = 3 // Custom
                hdUrlField.text = initialHdUrl
                sdUrlField.text = initialSdUrl
            }
            
            if (urlTemplateCombo.currentIndex !== 3) {
                updateUrl()
            }
        } else {
            urlTemplateCombo.currentIndex = 0
            updateUrl()
        }
    }
    
    background: Rectangle {
        color: "#252526"
        border.color: "#3e3e42"
        radius: 6
    }
    
    header: Rectangle {
        color: "transparent"
        height: 50
        
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: root.title
            color: "white"
            font.bold: true
            font.pixelSize: 16
        }
        
        Button {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: "✕"
            background: Rectangle { color: "transparent" }
            contentItem: Text {
                text: parent.text
                color: "#aaaaaa"
                font.pixelSize: 16
            }
            onClicked: root.close()
        }
    }
    
    contentItem: ColumnLayout {
        spacing: 15
        
        // Name
        ColumnLayout {
            spacing: 5
            Text { text: I18n.t("Название"); color: "#aaaaaa"; font.pixelSize: 12 }
            TextField {
                id: nameField
                Layout.fillWidth: true
                color: "white"
                selectByMouse: true
                background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
            }
        }
        
        // IP
        ColumnLayout {
            spacing: 5
            Text { text: I18n.t("IP / Host"); color: "#aaaaaa"; font.pixelSize: 12 }
            TextField {
                id: ipField
                Layout.fillWidth: true
                color: "white"
                selectByMouse: true
                background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
                onTextChanged: updateUrl()
                validator: RegularExpressionValidator { regularExpression: /^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$/ }
            }
        }
        
        // Ports Row
        RowLayout {
            spacing: 20
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 5
                Text { text: I18n.t("RTSP порт"); color: "#aaaaaa"; font.pixelSize: 12 }
                TextField {
                    id: rtspPortField
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: "554"
                    color: "white"
                    selectByMouse: true
                    background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
                    validator: IntValidator { bottom: 1; top: 65535 }
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 5
                Text { text: I18n.t("ONVIF порт"); color: "#aaaaaa"; font.pixelSize: 12 }
                TextField {
                    id: onvifPortField
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: "80"
                    color: "white"
                    selectByMouse: true
                    background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
                    validator: IntValidator { bottom: 1; top: 65535 }
                }
            }
        }
        
        // Auth Row
        RowLayout {
            spacing: 20
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 5
                Text { text: I18n.t("Логин"); color: "#aaaaaa"; font.pixelSize: 12 }
                TextField {
                    id: loginField
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: "root"
                    color: "white"
                    selectByMouse: true
                    background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
                    onTextChanged: updateUrl()
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 5
                Text { text: I18n.t("Пароль"); color: "#aaaaaa"; font.pixelSize: 12 }
                TextField {
                    id: passwordField
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    placeholderText: I18n.t("Пароль")
                    placeholderTextColor: "#aaaaaa"
                    echoMode: TextInput.Password
                    color: "white"
                    selectByMouse: true
                    background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
                    onTextChanged: updateUrl()
                }
            }
        }
        
        // Channel/Profile Row
        RowLayout {
            spacing: 20
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 5
                Text { text: I18n.t("Канал"); color: "#aaaaaa"; font.pixelSize: 12 }
                TextField {
                    id: channelField
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: "1"
                    color: "white"
                    selectByMouse: true
                    background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
                    onTextChanged: updateUrl()
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 5
                Text { text: I18n.t("HD профиль"); color: "#aaaaaa"; font.pixelSize: 12 }
                TextField {
                    id: hdProfileField
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: "0"
                    color: "white"
                    selectByMouse: true
                    background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
                    onTextChanged: updateUrl()
                }
            }
        }
        
        // SD Profile
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5
            Text { text: I18n.t("SD профиль"); color: "#aaaaaa"; font.pixelSize: 12 }
            TextField {
                id: sdProfileField
                Layout.fillWidth: true
                text: "1"
                color: "white"
                selectByMouse: true
                background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
                onTextChanged: updateUrl()
            }
        }
        
        // URL Template
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5
            Text { text: I18n.t("Шаблон URL"); color: "#aaaaaa"; font.pixelSize: 12 }
            StyledComboBox {
                id: urlTemplateCombo
                Layout.fillWidth: true
                model: ["OpenIPC", "Hikvision", "Dahua", "Custom"]
                currentIndex: 0
                onCurrentIndexChanged: updateUrl()
            }
        }
        
        // Generated URLs
        ColumnLayout {
            spacing: 5
            Text { text: I18n.t("RTSP HD URL"); color: "#aaaaaa"; font.pixelSize: 11 }
            TextField {
                id: hdUrlField
                Layout.fillWidth: true
                readOnly: urlTemplateCombo.currentText !== "Custom"
                color: !readOnly ? "white" : "#aaaaaa"
                selectByMouse: true
                background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
            }
        }
        
        ColumnLayout {
            spacing: 5
            Text { text: I18n.t("RTSP SD URL"); color: "#aaaaaa"; font.pixelSize: 11 }
            TextField {
                id: sdUrlField
                Layout.fillWidth: true
                readOnly: urlTemplateCombo.currentText !== "Custom"
                color: !readOnly ? "white" : "#aaaaaa"
                selectByMouse: true
                background: Rectangle { color: "#333333"; radius: 4; border.color: "#444444" }
            }
        }
        
        Item { Layout.fillHeight: true }
        
        // Buttons
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 10
            
            Button {
                text: I18n.t("ОТМЕНА")
                background: Rectangle { color: "#444444"; radius: 4 }
                contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                onClicked: root.close()
            }
            
            Button {
                text: I18n.t("СОХРАНИТЬ")
                background: Rectangle { color: "#4caf50"; radius: 4 }
                contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                onClicked: {
                    if (isEditMode) {
                        SystemController.updateCamera(
                            editIndex,
                            nameField.text,
                            ipField.text,
                            hdUrlField.text,
                            parseInt(rtspPortField.text),
                            parseInt(onvifPortField.text),
                            loginField.text,
                            passwordField.text,
                            sdUrlField.text
                        )
                    } else {
                        SystemController.addManualCamera(
                            nameField.text,
                            ipField.text,
                            hdUrlField.text,
                            parseInt(rtspPortField.text),
                            parseInt(onvifPortField.text),
                            loginField.text,
                            passwordField.text,
                            sdUrlField.text
                        )
                    }
                    root.close()
                }
            }
        }
    }
    
    function generateRtspUrl(user, pass, ip, port, stream, template) {
        var auth = ""
        if (user !== "") {
            auth = encodeURIComponent(user)
            if (pass !== "") auth += ":" + encodeURIComponent(pass)
            auth += "@"
        }
        var baseUrl = "rtsp://" + auth + ip + ":" + port
        
        if (template === "Hikvision") {
             // Hikvision: /Streaming/Channels/101 (channel 1, stream 01)
             // Support direct stream ID if value is > 9 (e.g. user enters 102)
             var val = parseInt(stream)
             if (val > 9) {
                 return baseUrl + "/Streaming/Channels/" + val
             }
             var ch = channelField.text || "1"
             var profile = val + 1 // 0->1, 1->2
             return baseUrl + "/Streaming/Channels/" + ch + "0" + profile
        } else if (template === "Dahua") {
             // Dahua: /cam/realmonitor?channel=1&subtype=0
             var ch = channelField.text || "1"
             return baseUrl + "/cam/realmonitor?channel=" + ch + "&subtype=" + stream
        } else {
             // OpenIPC / Default
             return baseUrl + "/stream=" + stream
        }
    }

    function updateUrl() {
        // Use model/index directly to avoid lag with currentText update
        var template = urlTemplateCombo.model[urlTemplateCombo.currentIndex]
        if (template === "Custom") return;
        
        var user = loginField.text
        var pass = passwordField.text
        var ip = ipField.text
        var stream = hdProfileField.text
        var sdStream = sdProfileField.text
        var rtspPort = rtspPortField.text || "554"
        
        // Basic validation for button state
        var isValid = ip.length > 0 && rtspPort.length > 0
        // You could bind the "Save" button enabled state to this logic if you expose a property
        
        hdUrlField.text = generateRtspUrl(user, pass, ip, rtspPort, stream, template)
        sdUrlField.text = generateRtspUrl(user, pass, ip, rtspPort, sdStream, template)
    }
}
