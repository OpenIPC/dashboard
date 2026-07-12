import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var controller: null
    property string selectedProfile: "quick"

    signal profileSelected(string profileId)

    implicitHeight: 58
    radius: Theme.radiusMd
    color: Theme.panelAltBackground
    border.color: Theme.panelBorder

    function profileName(profileId, fallback) {
        if (I18n.language !== "ru")
            return fallback
        if (profileId === "quick") return "Быстрый"
        if (profileId === "deep") return "Глубокий"
        if (profileId === "openipc") return "OpenIPC / Majestic"
        if (profileId === "rtsp") return "Только RTSP"
        return fallback
    }

    function profileDescription(profileId, fallback) {
        if (I18n.language !== "ru")
            return fallback
        if (profileId === "quick") return "Основной RTSP и снимок"
        if (profileId === "deep") return "Все потоки, API, прошивка, метрики и логи"
        if (profileId === "openipc") return "WebUI прошивки и Majestic API"
        if (profileId === "rtsp") return "Основной и дополнительный потоки"
        return fallback
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 9
        spacing: 8

        ColumnLayout {
            Layout.preferredWidth: 108
            spacing: 1

            Text {
                text: I18n.language === "ru" ? "Профиль" : "Profile"
                color: Theme.textPrimary
                font.pixelSize: 12
                font.bold: true
            }
            Text {
                text: I18n.language === "ru" ? "Глубина проверки" : "Diagnostic depth"
                color: Theme.textMuted
                font.pixelSize: 10
            }
        }

        Repeater {
            model: root.controller ? root.controller.profiles : []

            delegate: Button {
                id: profileButton

                required property var modelData
                readonly property string profileId: String(modelData.id || "")
                readonly property bool selected: root.selectedProfile === profileId

                Layout.fillWidth: true
                Layout.preferredHeight: 40
                enabled: !root.controller || !root.controller.running
                text: root.profileName(profileId, String(modelData.label || profileId))
                onClicked: {
                    root.selectedProfile = profileId
                    root.profileSelected(profileId)
                }

                ToolTip.visible: hovered
                ToolTip.delay: 450
                ToolTip.text: root.profileDescription(
                                  profileId, String(modelData.description || ""))

                background: Rectangle {
                    radius: Theme.radiusSm
                    color: profileButton.selected ? Theme.accent
                                                  : (profileButton.hovered
                                                     ? Theme.cardHover
                                                     : Theme.controlBackground)
                    border.color: profileButton.selected ? Theme.accentHover
                                                         : Theme.controlBorder
                }
                contentItem: Text {
                    text: profileButton.text
                    color: Theme.textPrimary
                    font.pixelSize: 11
                    font.bold: profileButton.selected
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: 120
            spacing: 4
            visible: root.controller && root.controller.running

            Text {
                Layout.fillWidth: true
                text: (I18n.language === "ru" ? "Выполнено " : "Completed ")
                      + root.controller.completedProbes + "/"
                      + root.controller.totalProbes
                color: Theme.textSecondary
                font.pixelSize: 10
                horizontalAlignment: Text.AlignRight
            }
            ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: Math.max(1, root.controller.totalProbes)
                value: root.controller.completedProbes
            }
        }
    }
}
