import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    modal: true
    width: 800
    height: 600
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.NoAutoClose
    
    signal addCameraRequested(string name, string ip, int port, int onvifPort)

    Dialog {
        id: batchLoginDialog
        title: I18n.t("Пакетное добавление")
        modal: true
        width: 400
        height: 250
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        
        background: Rectangle {
            color: "#2b2b2b"
            radius: 8
            border.color: "#444444"
        }
        
        header: Rectangle {
            color: "transparent"
            height: 50
            Text {
                anchors.centerIn: parent
                text: batchLoginDialog.title
                color: "white"
                font.bold: true
                font.pixelSize: 16
            }
        }
        
        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            anchors.topMargin: 60
            spacing: 15
            
            Text {
                text: I18n.t("Введите логин и пароль для выбранных камер:")
                color: "#cccccc"
                font.pixelSize: 12
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            Text {
                text: "⚠️ " + I18n.t("Сторонние камеры могут быть добавлены с ошибками")
                color: "#ffab00"
                font.pixelSize: 12
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            
            ColumnLayout {
                spacing: 5
                Layout.fillWidth: true
                Text { text: I18n.t("Логин"); color: "#aaaaaa"; font.pixelSize: 12 }
                TextField {
                    id: batchLoginField
                    text: "root"
                    Layout.fillWidth: true
                    color: "white"
                    background: Rectangle { color: "#333"; radius: 4; border.color: "#555" }
                }
            }
            
            ColumnLayout {
                spacing: 5
                Layout.fillWidth: true
                Text { text: I18n.t("Пароль"); color: "#aaaaaa"; font.pixelSize: 12 }
                TextField {
                    id: batchPasswordField
                    text: ""
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                    color: "white"
                    background: Rectangle { color: "#333"; radius: 4; border.color: "#555" }
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 10
                spacing: 10
                
                Button {
                    text: I18n.t("Отмена")
                    Layout.fillWidth: true
                    background: Rectangle { color: "#444"; radius: 4 }
                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: batchLoginDialog.close()
                }
                
                Button {
                    text: I18n.t("Добавить")
                    Layout.fillWidth: true
                    background: Rectangle { color: "#1976d2"; radius: 4 }
                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true }
                    onClicked: {
                        root.processBatchAdd(batchLoginField.text, batchPasswordField.text)
                        batchLoginDialog.close()
                    }
                }
            }
        }
    }

    function processBatchAdd(login, password) {
        var count = 0
        for (var key in root.selectedIndices) {
            if (root.selectedIndices[key] === true) {
                var index = parseInt(key)
                var cam = SystemController.discoveryModel.getCamera(index)
                
                var port = cam.cameraPort ? cam.cameraPort : 554
                var onvifPort = cam.cameraOnvifPort ? cam.cameraOnvifPort : 80
                
                // Generate RTSP URL (OpenIPC default)
                var auth = ""
                if (login !== "") {
                    auth = encodeURIComponent(login)
                    if (password !== "") auth += ":" + encodeURIComponent(password)
                    auth += "@"
                }
                var hdUrl = "rtsp://" + auth + cam.cameraIp + ":" + port + "/stream=0"
                
                SystemController.addManualCamera(
                    cam.cameraName,
                    cam.cameraIp,
                    hdUrl,
                    port,
                    onvifPort,
                    login,
                    password
                )
                count++
            }
        }
        console.log("Batch added " + count + " cameras")
        root.close()
    }

    background: Rectangle {
        color: "#2b2b2b"
        radius: 8
        border.color: "#444444"
    }
    
    header: Rectangle {
        color: "transparent"
        height: 60 // Increased height to prevent overlap
        
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 20
            text: I18n.t("Найденные камеры")
            color: "white"
            font.pixelSize: 18
            font.bold: true
        }
        
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 20
            text: "✕"
            color: "#aaaaaa"
            font.pixelSize: 18
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.close()
            }
        }
    }
    
    property var interfaces: []
    property var selectedIndices: ({}) // Map to track selected indices
    property int selectedCount: 0
    
    onOpened: {
        refreshInterfaces()
        selectedIndices = ({})
        selectedCount = 0
    }
    
    function toggleSelection(index, isSelected) {
        if (isSelected) {
            selectedIndices[index] = true
        } else {
            delete selectedIndices[index]
        }
        // Update count
        var count = 0
        for (var key in selectedIndices) {
            count++
        }
        selectedCount = count
    }
    
    function refreshInterfaces() {
        interfaces = SystemController.getNetworkInterfaces()
        interfaceModel.clear()
        interfaceModel.append({text: I18n.t("Все интерфейсы"), value: ""})
        for (var i = 0; i < interfaces.length; i++) {
            interfaceModel.append({
                text: interfaces[i].name + " (" + interfaces[i].ip + ")",
                value: interfaces[i].id
            })
        }
        interfaceCombo.currentIndex = 0
    }

    contentItem: ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.bottomMargin: 20
        anchors.topMargin: 80 // Header height (60) + spacing (20)
        spacing: 15
        
        // Interface Selection Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5
            
            RowLayout {
                Layout.fillWidth: true
                Text { 
                    text: I18n.t("Сетевой интерфейс")
                    color: "white"
                    font.bold: true
                    font.pixelSize: 14
                }
                
                Item { Layout.fillWidth: true }
                
                Text {
                    text: I18n.t("ОБНОВИТЬ ИНТЕРФЕЙСЫ")
                    color: "#1976d2"
                    font.pixelSize: 12
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.refreshInterfaces()
                    }
                }
                
                Rectangle {
                    width: 40
                    height: 24
                    color: "transparent"
                    border.color: "#444444"
                    radius: 4
                    Text {
                        anchors.centerIn: parent
                        text: I18n.t("ВСЕ")
                        color: "#aaaaaa"
                        font.pixelSize: 11
                    }
                }
            }
            
            Text {
                text: I18n.t("Выберите адаптер, который нужно сканировать.")
                color: "#888888"
                font.pixelSize: 12
            }
            
            ComboBox {
                id: interfaceCombo
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                textRole: "text"
                valueRole: "value"
                model: ListModel { id: interfaceModel }
                
                background: Rectangle {
                    color: "#333333"
                    border.color: "#444444"
                    radius: 4
                }
                contentItem: Text {
                    leftPadding: 10
                    text: interfaceCombo.displayText
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
        
        // Results Header
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Найдено устройств: ") + resultsList.count
                color: "white"
                font.bold: true
                font.pixelSize: 14
            }
            Item { Layout.fillWidth: true }
            Text {
                text: I18n.t("Найдено камер: ") + resultsList.count
                color: "#888888"
                font.pixelSize: 12
            }
        }
        
        // Table Header
        Rectangle {
            Layout.fillWidth: true
            height: 30
            color: "transparent"
            
            RowLayout {
                anchors.fill: parent
                spacing: 10
                
                CheckBox {
                    checked: false
                    hoverEnabled: false
                    background: Item {}
                    indicator: Rectangle {
                        implicitWidth: 18
                        implicitHeight: 18
                        x: parent.leftPadding
                        y: parent.height / 2 - height / 2
                        radius: 3
                        color: "transparent"
                        border.color: parent.checked ? "#3f89d6" : "#666"
                        
                        Rectangle {
                            width: 10
                            height: 10
                            anchors.centerIn: parent
                            radius: 2
                            color: "#3f89d6"
                            visible: parent.parent.checked
                        }
                    }
                }
                
                Text { Layout.preferredWidth: 200; text: I18n.t("Устройство"); color: "#aaaaaa"; font.pixelSize: 12 }
                Text { Layout.preferredWidth: 100; text: I18n.t("Сеть"); color: "#aaaaaa"; font.pixelSize: 12 }
                Text { Layout.preferredWidth: 150; text: I18n.t("Порты"); color: "#aaaaaa"; font.pixelSize: 12 }
                Text { Layout.fillWidth: true; text: I18n.t("Протокол"); color: "#aaaaaa"; font.pixelSize: 12 }
            }
        }
        
        // Results List
        ListView {
            id: resultsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: SystemController.discoveryModel
            spacing: 5
            
            delegate: Rectangle {
                width: resultsList.width
                height: 60
                color: "#333333"
                radius: 4
                
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        selectionCheck.checked = !selectionCheck.checked
                    }
                    onEntered: parent.color = "#3d3d3d"
                    onExited: parent.color = "#333333"
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    
                    CheckBox {
                        id: selectionCheck
                        checked: root.selectedIndices[index] === true
                        hoverEnabled: false
                        background: Item {}
                        indicator: Rectangle {
                            implicitWidth: 18
                            implicitHeight: 18
                            x: parent.leftPadding
                            y: parent.height / 2 - height / 2
                            radius: 3
                            color: "transparent"
                            border.color: parent.checked ? "#3f89d6" : "#666"
                            
                            Rectangle {
                                width: 10
                                height: 10
                                anchors.centerIn: parent
                                radius: 2
                                color: "#3f89d6"
                                visible: parent.parent.checked
                            }
                        }
                        onCheckedChanged: {
                            root.toggleSelection(index, checked)
                        }
                    }
                    
                    // Device Info
                    ColumnLayout {
                        Layout.preferredWidth: 200
                        spacing: 2
                        Text { 
                            text: model.cameraName 
                            color: "white"
                            font.bold: true
                            font.pixelSize: 13
                        }
                        Text { 
                            text: model.cameraIp
                            color: "#aaaaaa"
                            font.pixelSize: 12
                        }
                        Text {
                            text: (model.manufacturer ? model.manufacturer : "") + (model.serialNumber ? " (" + model.serialNumber + ")" : "")
                            color: "#888888"
                            font.pixelSize: 10
                            visible: model.manufacturer !== undefined || model.serialNumber !== undefined
                        }
                    }
                    
                    // Network Info
                    Row {
                        Layout.preferredWidth: 100
                        spacing: 5
                        Rectangle {
                            width: 60
                            height: 20
                            color: "#444444"
                            radius: 10
                            Text {
                                anchors.centerIn: parent
                                text: "TCP " + (model.cameraPort ? model.cameraPort : "80")
                                color: "#cccccc"
                                font.pixelSize: 10
                            }
                        }
                    }
                    
                    // Ports Info
                    Row {
                        Layout.preferredWidth: 150
                        spacing: 5
                        
                        // SDK Tag
                        Rectangle {
                            width: 40
                            height: 20
                            color: "transparent"
                            border.color: "#ff9800"
                            radius: 10
                            visible: model.manufacturer !== undefined && model.manufacturer !== ""
                            Text {
                                anchors.centerIn: parent
                                text: "SDK"
                                color: "#ff9800"
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }

                        // RTSP Tag
                        Rectangle {
                            width: 65
                            height: 20
                            color: "transparent"
                            border.color: "#4caf50"
                            radius: 10
                            visible: true
                            Text {
                                anchors.centerIn: parent
                                text: "RTSP 554"
                                color: "#4caf50"
                                font.pixelSize: 10
                            }
                        }
                        
                        // ONVIF Tag
                        Rectangle {
                            width: 65
                            height: 20
                            color: "transparent"
                            border.color: "#2196f3"
                            radius: 10
                            visible: true
                            Text {
                                anchors.centerIn: parent
                                text: "ONVIF 80"
                                color: "#2196f3"
                                font.pixelSize: 10
                            }
                        }
                    }
                    
                    // Protocol Info
                    Row {
                        Layout.fillWidth: true
                        Rectangle {
                            width: 50
                            height: 20
                            color: "#555555"
                            radius: 10
                            Text {
                                anchors.centerIn: parent
                                text: "onvif"
                                color: "white"
                                font.pixelSize: 10
                            }
                        }
                    }
                }
            }
            
            // Empty State
            Text {
                anchors.centerIn: parent
                text: I18n.t("Нажмите \"Сканировать\", чтобы найти камеры.\nПроверьте, что сеть помечена как «Частная»...")
                color: "#888888"
                horizontalAlignment: Text.AlignHCenter
                visible: resultsList.count === 0
            }
        }
        
        // Footer Actions
        RowLayout {
            Layout.fillWidth: true
            
            Button {
                text: I18n.t("СКАНИРОВАТЬ")
                Layout.preferredWidth: 150
                Layout.preferredHeight: 40
                
                background: Rectangle {
                    color: "#1976d2"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
                
                onClicked: {
                    var iface = interfaceCombo.currentValue
                    SystemController.scanNetwork(iface)
                }
            }
            
            Item { Layout.fillWidth: true }
            
            Button {
                text: I18n.t("ДОБАВИТЬ ВЫБРАННЫЕ (") + root.selectedCount + ")"
                Layout.preferredWidth: 200
                Layout.preferredHeight: 40
                enabled: root.selectedCount > 0
                
                background: Rectangle {
                    color: parent.enabled ? "#1976d2" : "#444444"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? "white" : "#888888"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
                onClicked: {
                    if (root.selectedCount === 1) {
                        for (var key in root.selectedIndices) {
                            if (root.selectedIndices[key] === true) {
                                var index = parseInt(key)
                                var cam = SystemController.discoveryModel.getCamera(index)
                                var port = cam.cameraPort ? cam.cameraPort : 554
                                var onvifPort = cam.cameraOnvifPort ? cam.cameraOnvifPort : 80
                                root.addCameraRequested(cam.cameraName, cam.cameraIp, port, onvifPort)
                                root.close()
                                return
                            }
                        }
                    } else {
                        batchPasswordField.text = ""
                        batchLoginDialog.open()
                    }
                }
            }
        }
    }
}
