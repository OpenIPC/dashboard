import QtQuick
import QtQuick.Controls
import OpenIPC

Menu {
    id: contextMenu
    
    property string cameraIp: ""
    property string cameraName: ""
    property int cameraIndex: -1
    property bool isGridContext: false
    property bool canPlayback: true
    property bool canSettings: true
    property bool canExport: true
    property bool canLive: true
    
    signal editRequested()
    signal deleteRequested()
    signal sshRequested()
    signal majesticRequested()
    signal fileManagerRequested()
    signal archiveRequested()
    signal healthRequested()
    signal groupChanged()
    signal permissionDenied()

    palette.text: Theme.textSecondary
    palette.windowText: Theme.textSecondary
    palette.buttonText: Theme.textSecondary
    palette.highlightedText: Theme.textPrimary
    
    MetroMenuItem {
        text: I18n.t("Архив")
        // icon.source: "qrc:/OpenIPC/src/ui/icons/archive.svg" // Placeholder
        onTriggered: {
            if (!canPlayback) { contextMenu.permissionDenied(); return }
            contextMenu.archiveRequested()
        }
    }
    
    MetroMenuSeparator {}
    
    MetroMenuItem {
        text: I18n.t("Открыть в браузере")
        onTriggered: {
            if (!canSettings) { contextMenu.permissionDenied(); return }
            console.log("Opening browser for IP: " + cameraIp)
            if (cameraIp !== "") {
                Qt.openUrlExternally("http://" + cameraIp)
            }
        }
    }
    
    MetroMenuItem {
        text: I18n.t("SSH Терминал")
        onTriggered: {
            if (!canSettings) { contextMenu.permissionDenied(); return }
            contextMenu.sshRequested()
        }
    }

    MetroMenuItem {
        text: I18n.t("OpenIPC Control Center")
        onTriggered: {
            if (!canSettings) { contextMenu.permissionDenied(); return }
            contextMenu.majesticRequested()
        }
    }

    MetroMenuItem {
        text: I18n.t("Проверить здоровье")
        onTriggered: {
            if (!contextMenu.canSettings) { contextMenu.permissionDenied(); return }
            contextMenu.healthRequested()
        }
    }

    MetroMenuItem {
        text: I18n.t("Файловый менеджер")
        onTriggered: {
            if (!canExport) { contextMenu.permissionDenied(); return }
            contextMenu.fileManagerRequested()
        }
    }
    
    MetroMenuSeparator {}
    
    Menu {
        id: groupsMenu
        title: I18n.t("Группы")

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: 40
            color: Theme.metroSidebarBackground
            border.color: Theme.metroStroke
            radius: Theme.metroTileRadius
        }

        MetroMenuItem {
            text: I18n.t("Без группы")
            onTriggered: {
                if (!canSettings) { contextMenu.permissionDenied(); return }
                SystemController.setCameraGroup(cameraIndex, "")
                contextMenu.groupChanged()
            }
        }

        MetroMenuSeparator { visible: SystemController.cameraGroups.length > 0 }

        Instantiator {
            model: SystemController.cameraGroups
            delegate: MetroMenuItem {
                text: modelData
                onTriggered: {
                    if (!canSettings) { contextMenu.permissionDenied(); return }
                    SystemController.setCameraGroup(cameraIndex, modelData)
                    contextMenu.groupChanged()
                }
            }
            onObjectAdded: (index, object) => groupsMenu.insertItem(index + 2, object)
            onObjectRemoved: (index, object) => groupsMenu.removeItem(object)
        }
    }
    
    MetroMenuSeparator {}
    
    MetroMenuItem {
        text: I18n.t("Редактировать камеру")
        onTriggered: {
            if (!canSettings) { contextMenu.permissionDenied(); return }
            contextMenu.editRequested()
        }
    }
    
    MetroMenuItem {
        text: I18n.t("Удалить камеру")
        onTriggered: {
            if (!canSettings) { contextMenu.permissionDenied(); return }
            contextMenu.deleteRequested()
        }
    }
    
    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 40
        color: Theme.metroSidebarBackground
        border.color: Theme.metroStroke
        radius: Theme.metroTileRadius
    }
}
