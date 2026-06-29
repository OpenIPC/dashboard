import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: probeCard
    required property var controller
    property string title: ""
    property string state: "idle"
    property string message: ""
    property int elapsedMs: 0
    property string buttonText: I18n.t("Проверить")
    signal run()

    Layout.fillWidth: true
    Layout.preferredHeight: 92
    color: Theme.cardBackground
    border.color: probeCard.controller.probeStateColor(probeCard.state)
    radius: Theme.radiusLg

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10
        Rectangle {
            Layout.preferredWidth: 10
            Layout.preferredHeight: 10
            radius: 5
            color: probeCard.controller.probeStateColor(probeCard.state)
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3
            Text {
                text: probeCard.title
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 13
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: probeCard.controller.probeStateText(probeCard.state, probeCard.message, probeCard.elapsedMs)
                color: Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }
        MajesticButton {
            text: probeCard.state === "running" ? I18n.t("Проверка…") : probeCard.buttonText
            enabled: probeCard.state !== "running"
            onClicked: probeCard.run()
        }
    }
}
