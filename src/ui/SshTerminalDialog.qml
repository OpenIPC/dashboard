import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    title: I18n.t("SSH Терминал") + (cameraIp ? " - " + cameraIp : "")
    modal: true
    width: 800
    height: 600
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    property string cameraIp: ""
    property string cameraUser: "root"
    property string cameraPassword: ""
    
    onOpened: {
        outputArea.text = ""
        inputField.text = ""
        sshClient.connectToHost(cameraIp, cameraUser, cameraPassword)
        inputField.forceActiveFocus()
    }
    
    onClosed: {
        sshClient.disconnectFromHost()
    }
    
    SshClient {
        id: sshClient
        onDataReceived: (data) => {
            outputArea.append(data)
        }
        onErrorOccurred: (error) => {
            outputArea.append("\n[ERROR] " + error)
        }
        onConnectedChanged: {
            if (connected) {
                outputArea.append("\n[CONNECTED]\n")
            } else {
                outputArea.append("\n[DISCONNECTED]\n")
            }
        }
    }
    
    background: Rectangle {
        color: "#1e1e1e"
        border.color: "#3c3c3c"
        radius: 4
    }
    
    header: Rectangle {
        height: 40
        color: "#2d2d30"
        
        Text {
            anchors.centerIn: parent
            text: root.title
            color: "white"
            font.bold: true
        }
        
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 15
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
        spacing: 0
        
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            TextArea {
                id: outputArea
                readOnly: true
                color: "#00ff00" // Terminal green
                font.family: "Consolas, monospace"
                font.pixelSize: 14
                background: null
                wrapMode: Text.WrapAnywhere
            }
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#3c3c3c"
        }
        
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.margins: 5
            
            Text {
                text: ">"
                color: "#00ff00"
                font.family: "Consolas, monospace"
                font.bold: true
            }
            
            TextField {
                id: inputField
                Layout.fillWidth: true
                color: "white"
                font.family: "Consolas, monospace"
                background: null
                placeholderText: I18n.t("Введите команду...")
                placeholderTextColor: "#555555"
                
                onAccepted: {
                    if (text.trim() !== "") {
                        sshClient.sendCommand(text)
                        // outputArea.append("> " + text) // Local echo? SSH usually echoes back
                        text = ""
                    }
                }
                
                // Handle Ctrl+C?
                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_C && (event.modifiers & Qt.ControlModifier)) {
                        sshClient.sendCommand("\x03") // Ctrl+C
                        event.accepted = true
                    }
                }
            }
        }
    }
}
