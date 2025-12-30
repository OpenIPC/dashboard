import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root
    property var model
    
    GridView {
        anchors.fill: parent
        anchors.margins: 20
        cellWidth: 160
        cellHeight: 200
        clip: true
        
        model: root.model
        
        delegate: Item {
            width: 150
            height: 190
            
            Rectangle {
                anchors.fill: parent
                color: "#2d2d2d"
                radius: 8
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120
                        color: "#000"
                        
                        // Image would go here
                        Text {
                            anchors.centerIn: parent
                            text: "IMG"
                            color: "#555"
                        }
                    }
                    
                    Text {
                        text: Qt.formatDateTime(model.capturedAt, "HH:mm:ss")
                        color: "white"
                        font.pixelSize: 12
                    }
                    
                    Text {
                        text: (model.confidence * 100).toFixed(1) + "%"
                        color: "#aaa"
                        font.pixelSize: 11
                    }
                }
            }
        }
    }
}
