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
    property string returnPhase: "idle"
    property string returnHealthText: ""
    property bool firmwareBusy: false

    signal clearRequested()

    function phaseLabel() {
        if (returnPhase === "online") return I18n.t("камера вернулась")
        if (returnPhase === "degraded") return I18n.t("нужна проверка")
        if (returnPhase === "validating") return I18n.t("health probe")
        if (returnPhase === "failed") return I18n.t("не вернулась")
        if (returnPhase === "probing") return I18n.t("проверка")
        if (returnPhase === "waiting") return I18n.t("ожидание")
        if (returnPhase === "rebooting") return I18n.t("reboot")
        return I18n.t("idle")
    }

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

        RowLayout {
            visible: root.returnPolling || root.returnHealthText.length > 0
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                implicitWidth: phaseText.implicitWidth + 18
                implicitHeight: 24
                radius: 12
                color: root.returnPhase === "online" ? "#052e1b"
                                                      : (root.returnPhase === "failed" ? "#3f1212" : "#422006")
                border.color: root.returnPhase === "online" ? Theme.success
                                                            : (root.returnPhase === "failed" ? Theme.danger : Theme.warning)

                Text {
                    id: phaseText
                    anchors.centerIn: parent
                    text: root.phaseLabel()
                    color: root.returnPhase === "online" ? Theme.success
                                                         : (root.returnPhase === "failed" ? Theme.danger : Theme.warning)
                    font.bold: true
                    font.pixelSize: 10
                }
            }

            Text {
                Layout.fillWidth: true
                text: root.returnHealthText
                color: root.returnPhase === "failed" ? Theme.danger : Theme.textSecondary
                elide: Text.ElideRight
                font.pixelSize: 11
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
