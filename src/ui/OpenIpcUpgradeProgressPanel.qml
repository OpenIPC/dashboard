import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property string progressText: ""
    property bool upgradeRebooting: false
    property bool returnPolling: false
    property int returnPollTries: 0
    property int returnPollMaxTries: 0
    property bool firmwareBusy: false

    signal clearRequested()

    visible: root.progressText.length > 0 || root.upgradeRebooting || root.returnPolling
    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    Layout.preferredHeight: visible ? 220 : 0
    color: Theme.cardBackground
    border.color: (root.upgradeRebooting || root.returnPolling) ? Theme.warning : Theme.cardBorder
    radius: Theme.radiusLg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                text: I18n.t("Firmware upgrade progress")
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 15
            }

            MajesticButton {
                text: I18n.t("Очистить")
                subtle: true
                enabled: !root.firmwareBusy
                onClicked: root.clearRequested()
            }
        }

        Text {
            visible: root.returnPolling
            Layout.fillWidth: true
            text: I18n.t("Ожидание возврата камеры… попытка %1/%2", [root.returnPollTries, root.returnPollMaxTries])
            color: Theme.warning
            font.pixelSize: 11
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            TextArea {
                readOnly: true
                wrapMode: TextEdit.NoWrap
                text: root.progressText.length ? root.progressText : I18n.t("Progress появится после старта /ws/upgrade.")
                color: Theme.textSecondary
                selectedTextColor: Theme.textPrimary
                selectionColor: Theme.accent
                font.family: "Consolas"
                font.pixelSize: 11
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: Theme.controlBackground
                    border.color: Theme.controlBorder
                }
            }
        }
    }
}
