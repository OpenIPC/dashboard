import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    signal openWebUiRequested(string path)
    signal backupRequested()
    signal rebootRequested()
    signal copyWebUiRequested()

    Layout.fillWidth: true
    Layout.minimumHeight: 252
    Layout.preferredHeight: Math.max(Layout.minimumHeight, quickActionsContent.implicitHeight + 32)
    color: Theme.metroSurface
    border.color: Theme.metroStroke
    radius: Theme.metroTileRadius

    ColumnLayout {
        id: quickActionsContent

        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
            text: I18n.t("Быстрые действия")
            color: Theme.textPrimary
            font.bold: true
            font.pixelSize: 18
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("Здесь собраны штатные разделы OpenIPC WebUI и нативные операции firmware-client: status, network, time, logs, backup, reboot и update.")
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            MajesticButton {
                text: I18n.t("WebUI")
                primary: true
                onClicked: root.openWebUiRequested("")
            }

            MajesticButton {
                text: I18n.t("Network")
                onClicked: root.openWebUiRequested("/cgi-bin/fw-network.cgi")
            }

            MajesticButton {
                text: I18n.t("Time")
                onClicked: root.openWebUiRequested("/cgi-bin/fw-time.cgi")
            }

            MajesticButton {
                text: I18n.t("Update")
                onClicked: root.openWebUiRequested("/cgi-bin/fw-update.cgi")
            }

            MajesticButton {
                text: I18n.t("Settings")
                onClicked: root.openWebUiRequested("/cgi-bin/fw-settings.cgi")
            }

            MajesticButton {
                text: I18n.t("Firmware backup")
                onClicked: root.backupRequested()
            }

            MajesticButton {
                text: I18n.t("Reboot")
                danger: true
                onClicked: root.rebootRequested()
            }

            MajesticButton {
                text: I18n.t("Скопировать адрес")
                onClicked: root.copyWebUiRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.metroStroke
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("Write-операции используют те же CGI/JSON endpoints, что и WebUI камеры. Опасные действия требуют подтверждения.")
            color: Theme.metroAmber
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }
    }
}
