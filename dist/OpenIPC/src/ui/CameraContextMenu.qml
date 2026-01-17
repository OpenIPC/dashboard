import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Menu {
    id: contextMenu
    
    property string cameraIp: ""
    property string cameraName: ""
    property int cameraIndex: -1
    property bool isGridContext: false
    
    signal editRequested()
    signal deleteRequested()
    signal sshRequested()
    signal fileManagerRequested()
    signal archiveRequested()
    signal groupChanged()

    palette.text: "#cccccc"
    palette.windowText: "#cccccc"
    palette.buttonText: "#cccccc"
    palette.highlightedText: "white"
    
    MenuItem {
        text: I18n.t("Архив")
        // icon.source: "qrc:/OpenIPC/src/ui/icons/archive.svg" // Placeholder
        onTriggered: contextMenu.archiveRequested()
    }
    
    MenuSeparator {}
    
    MenuItem {
        text: I18n.t("Открыть в браузере")
        onTriggered: {
            console.log("Opening browser for IP: " + cameraIp)
            if (cameraIp !== "") {
                Qt.openUrlExternally("http://" + cameraIp)
            }
        }
    }
    
    MenuItem {
        text: I18n.t("SSH Терминал")
        onTriggered: contextMenu.sshRequested()
    }

    MenuItem {
        text: I18n.t("Файловый менеджер")
        onTriggered: contextMenu.fileManagerRequested()
    }
    
    MenuSeparator {}
    
    Menu {
        id: groupsMenu
        title: I18n.t("Группы")

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: 40
            color: "#252526"
            border.color: "#3c3c3c"
            radius: 4
        }

        MenuItem {
            contentItem: Text {
                text: I18n.t("Без группы")
                color: "#cccccc"
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
                leftPadding: 12
            }
            onTriggered: {
                SystemController.setCameraGroup(cameraIndex, "")
                contextMenu.groupChanged()
            }
        }

        MenuSeparator { visible: SystemController.cameraGroups.length > 0 }

        Instantiator {
            model: SystemController.cameraGroups
            delegate: MenuItem {
                contentItem: Text {
                    text: modelData
                    color: "#cccccc"
                    font.pixelSize: 14
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 12
                }
                onTriggered: {
                    SystemController.setCameraGroup(cameraIndex, modelData)
                    contextMenu.groupChanged()
                }
            }
            onObjectAdded: (index, object) => groupsMenu.insertItem(index + 2, object)
            onObjectRemoved: (index, object) => groupsMenu.removeItem(object)
        }
    }
    
    MenuSeparator {}
    
    MenuItem {
        text: I18n.t("Редактировать камеру")
        onTriggered: contextMenu.editRequested()
    }
    
    MenuItem {
        text: I18n.t("Удалить камеру")
        onTriggered: contextMenu.deleteRequested()
    }
    
    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 40
        color: "#252526"
        border.color: "#3c3c3c"
        radius: 4
    }
}
