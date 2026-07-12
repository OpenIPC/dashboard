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
        color: Theme.metroSurface
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
            onClicked: root.close()
        }
    }

    contentItem: ColumnLayout {
        spacing: 15
        
        // Group Name
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: I18n.t("Название группы")
                color: Theme.textSecondary
                font.family: Theme.metroFontFamily
                Layout.preferredWidth: 120
            }
            TextField {
                id: groupNameField
                Layout.fillWidth: true
                placeholderText: I18n.t("Название")
                placeholderTextColor: Theme.textFaint
                color: Theme.textPrimary
                font.family: Theme.metroFontFamily
                background: Rectangle {
                    color: Theme.controlBackground
                    border.color: groupNameField.activeFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                    border.width: groupNameField.activeFocus ? 2 : 1
                    radius: Theme.metroTileRadius
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
                    color: parent.down ? Theme.metroBlueHover : (parent.hovered ? Theme.metroBlueHover : Theme.metroBlue)
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 12
                    font.bold: true
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
                    color: parent.down ? Theme.metroTilePressed : (parent.hovered ? Theme.metroTileHover : Theme.metroTile)
                    radius: Theme.metroTileRadius
                    border.color: parent.hovered ? Theme.metroStrokeStrong : Theme.metroStroke
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.family: Theme.metroFontFamily
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.close()
            }
        }
    }
}
