import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root
    title: I18n.t("Создать новую группу")
    modal: true
    width: 400
    height: 200
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    background: Rectangle {
        color: "#333333"
        radius: 8
        border.color: "#444444"
    }
    
    header: Rectangle {
        height: 50
        color: "transparent"
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: root.title
            color: "white"
            font.pixelSize: 16
            font.bold: true
        }
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "×"
            color: "#aaaaaa"
            font.pixelSize: 20
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.close()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 15
        
        // Group Name
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Название группы")
                color: "white"
                Layout.preferredWidth: 120
            }
            TextField {
                id: groupNameField
                Layout.fillWidth: true
                placeholderText: I18n.t("Название")
                placeholderTextColor: "#aaaaaa"
                color: "white"
                background: Rectangle {
                    color: "#252526"
                    border.color: "#444444"
                    radius: 4
                }
            }
        }
        
        Item { Layout.fillHeight: true }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Button {
                text: I18n.t("Создать")
                background: Rectangle {
                    color: parent.down ? "#388e3c" : "#4caf50"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    var name = groupNameField.text.trim()
                    if (!name)
                        return
                    SystemController.addCameraGroup(name)
                    groupNameField.text = ""
                    root.close()
                }
            }
            
            Button {
                text: I18n.t("Отмена")
                background: Rectangle {
                    color: parent.down ? "#555555" : "#666666"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.close()
            }
        }
    }
}
