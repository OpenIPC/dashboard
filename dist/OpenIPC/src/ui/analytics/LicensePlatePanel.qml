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
        cellWidth: 200
        cellHeight: 100
        clip: true
        
        model: root.model
        
        delegate: Item {
            width: 190
            height: 90
            
            Rectangle {
                anchors.fill: parent
                color: "#2d2d2d"
                radius: 8
                border.color: "#444"
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    
                    Rectangle {
                        width: 60
                        height: 40
                        color: "#000"
                        border.color: "#fff"
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: "PLATE"
                            color: "#555"
                            font.pixelSize: 10
                        }
                    }
                    
                    ColumnLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: model.label
                            color: "white"
                            font.pixelSize: 18
                            font.bold: true
                            font.family: "Consolas" // Monospace for plates
                        }
                        
                        Text {
                            text: Qt.formatDateTime(model.capturedAt, "HH:mm:ss")
                            color: "#aaa"
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }
}
