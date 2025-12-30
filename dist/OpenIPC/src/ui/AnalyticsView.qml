import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC
import "analytics"

Dialog {
    id: root
    modal: true
    dim: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    // Center in parent
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: parent.width * 0.9
    height: parent.height * 0.9
    
    background: Rectangle {
        color: "#1e1e1e"
        radius: 8
        border.color: "#333"
        border.width: 1
    }
    
    header: Rectangle {
        height: 60
        color: "transparent"
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            
            Text {
                text: I18n.t("Аналитика")
                color: "white"
                font.pixelSize: 20
                font.bold: true
                Layout.fillWidth: true
            }
            
            Button {
                text: "✕"
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                background: Rectangle {
                    color: parent.hovered ? "#c42b1c" : "transparent"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.close()
            }
        }
        
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "#333"
        }
    }
    
    AnalyticsModel {
        id: analyticsModel
        filterType: {
            if (bar.currentIndex === 0) return "face"
            if (bar.currentIndex === 1) return "object"
            if (bar.currentIndex === 2) return "plate"
            return ""
        }
        Component.onCompleted: generateMockData()
    }
    
    contentItem: ColumnLayout {
        spacing: 0
        
        // Custom Tab Bar
        TabBar {
            id: bar
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            background: Rectangle { color: "#252526" }
            
            component CustomTabButton: TabButton {
                id: tabBtn
                width: implicitWidth + 40
                background: Rectangle {
                    color: tabBtn.checked ? "#1e1e1e" : "#2d2d2d"
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: tabBtn.checked ? "#3b82f6" : "transparent"
                    }
                }
                contentItem: Text {
                    text: tabBtn.text
                    color: tabBtn.checked ? "#3b82f6" : "#aaaaaa"
                    font.pixelSize: 14
                    font.bold: tabBtn.checked
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
            
            CustomTabButton { text: I18n.t("Лица") }
            CustomTabButton { text: I18n.t("Объекты") }
            CustomTabButton { text: I18n.t("Номера") }
        }
        
        // Content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: bar.currentIndex
            
            FaceSnapshotsPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: analyticsModel
            }
            
            ObjectCounterPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: analyticsModel
            }
            
            LicensePlatePanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: analyticsModel
            }
        }
    }
}
