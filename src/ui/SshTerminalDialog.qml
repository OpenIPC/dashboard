import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Window {
    id: root
    title: I18n.t("SSH Терминал") + (cameraIp ? " - " + cameraIp : "")
    width: 800
    height: 600
    visible: false
    color: "#1e1e1e"
    flags: Qt.Window | Qt.FramelessWindowHint
    
    property string cameraIp: ""
    property string cameraUser: "root"
    
    function open() {
        outputArea.text = ""
        inputField.text = ""
        sshClient.connectToHost(cameraIp, cameraUser, SystemController.getCameraPassword(cameraIp))
        inputField.forceActiveFocus()
        show()
        requestActivate()
    }
    
    onClosing: {
        sshClient.disconnectFromHost()
    }

    // Custom Header
    Rectangle {
        id: titleBar
        height: 40
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: "#2d2d30"
        z: 100

        MouseArea {
            anchors.fill: parent
            onPressed: root.startSystemMove()
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            anchors.rightMargin: 5
            
            Text {
                text: root.title
                color: "white"
                font.bold: true
                Layout.fillWidth: true
            }
            
            Button {
                text: "—"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.showMinimized()
                contentItem: Text { text: "—"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: parent.down ? "#444" : (parent.hovered ? "#3e3e40" : "transparent") }
            }
            
            Button {
                text: "□"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: {
                    if (root.visibility === Window.Maximized) root.showNormal()
                    else root.showMaximized()
                }
                contentItem: Text { text: "□"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: parent.down ? "#444" : (parent.hovered ? "#3e3e40" : "transparent") }
            }

            Button {
                text: "✕"
                flat: true
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.close()
                contentItem: Text { text: "✕"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                background: Rectangle { color: parent.down ? "#c42b1c" : (parent.hovered ? "#e81123" : "transparent") }
            }
        }
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
    
    // Background handled by window color

    // header removed to use native OS title bar
    
    ColumnLayout {
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
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
