import QtQuick
import QtQuick.Layouts
import OpenIPC

GridLayout {
    id: root

    property var updateInfo: ({})
    property string socText: ""
    property string flashText: ""
    readonly property bool githubUpdateAvailable: root.updateInfo.githubAvailable === true

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    columns: width > 900 ? 4 : 2
    rowSpacing: 12
    columnSpacing: 12

    Repeater {
        model: [
            { title: I18n.t("Installed"), value: root.updateInfo.installed || "—", percent: root.githubUpdateAvailable ? 100 : 40, accent: root.githubUpdateAvailable ? Theme.success : Theme.warning },
            { title: I18n.t("Latest GitHub"), value: root.updateInfo.latest || "—", percent: root.githubUpdateAvailable ? 100 : 40, accent: root.githubUpdateAvailable ? Theme.success : Theme.warning },
            { title: "SoC", value: root.socText || "—", percent: root.githubUpdateAvailable ? 100 : 40, accent: root.githubUpdateAvailable ? Theme.success : Theme.warning },
            { title: I18n.t("Flash"), value: root.flashText || "—", percent: root.githubUpdateAvailable ? 100 : 40, accent: root.githubUpdateAvailable ? Theme.success : Theme.warning }
        ]

        delegate: MajesticStatusCard {
            required property var modelData

            title: modelData.title
            value: modelData.value
            subtitle: ""
            percent: modelData.percent
            accent: modelData.accent
        }
    }
}
