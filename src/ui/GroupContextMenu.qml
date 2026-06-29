import QtQuick.Controls
import OpenIPC

Menu {
    id: menuRoot

    property string targetGroup: ""
    property bool canSettings: false

    signal renameRequested(string groupName)
    signal deleteRequested(string groupName)

    MenuItem {
        text: I18n.t("Переименовать группу")
        enabled: menuRoot.canSettings
        onTriggered: menuRoot.renameRequested(menuRoot.targetGroup)
    }

    MenuItem {
        text: I18n.t("Удалить группу")
        enabled: menuRoot.canSettings
        onTriggered: menuRoot.deleteRequested(menuRoot.targetGroup)
    }
}
