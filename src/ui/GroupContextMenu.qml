import QtQuick
import QtQuick.Controls
import OpenIPC

Menu {
    id: menuRoot

    property string targetGroup: ""
    property bool canSettings: false

    signal renameRequested(string groupName)
    signal deleteRequested(string groupName)

    MetroMenuItem {
        text: I18n.t("Переименовать группу")
        enabled: menuRoot.canSettings
        onTriggered: menuRoot.renameRequested(menuRoot.targetGroup)
    }

    MetroMenuItem {
        text: I18n.t("Удалить группу")
        enabled: menuRoot.canSettings
        onTriggered: menuRoot.deleteRequested(menuRoot.targetGroup)
    }
    background: Rectangle {
        implicitWidth: 210
        implicitHeight: 40
        color: Theme.metroSurface
        border.color: Theme.metroStroke
        radius: Theme.metroTileRadius
    }
}
