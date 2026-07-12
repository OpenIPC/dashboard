import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    property real hostWidth: 720
    property real hostHeight: 560
    property var pendingChanges: []
    property bool pipelineReloadNeeded: false
    property bool pipelineReloadAvailable: false
    property bool autoReloadAfterApply: true

    signal applyRequested()
    signal autoReloadAfterApplyToggled(bool checked)

    modal: true
    anchors.centerIn: parent
    width: Math.min(Math.max(root.hostWidth - 80, 520), 720)
    height: Math.min(Math.max(root.hostHeight - 100, 420), 560)
    title: I18n.t("Проверка изменений")
    standardButtons: Dialog.Ok | Dialog.Cancel
    onAccepted: root.applyRequested()

    contentItem: ColumnLayout {
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: I18n.t("На камеру будет отправлен только этот patch (%1 изменений):",
                         [root.pendingChanges.length])
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.pendingChanges

            delegate: Rectangle {
                required property var modelData
                required property int index

                property bool alternate: (index & 1) === 1

                width: ListView.view.width
                height: 62
                color: alternate ? Theme.panelSoftBackground : Theme.cardBackground

                ColumnLayout {
                    property var row: parent.modelData

                    anchors.fill: parent
                    anchors.margins: 7

                    Text {
                        text: parent.row.path
                        color: Theme.accentHover
                        font.family: "Consolas"
                        font.pixelSize: 11
                    }

                    Text {
                        Layout.fillWidth: true
                        text: String(parent.row.before) + "  ->  " + String(parent.row.after)
                        color: Theme.textSecondary
                        elide: Text.ElideRight
                        font.pixelSize: 11
                    }
                }
            }
        }

        Rectangle {
            visible: root.pipelineReloadNeeded
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? 72 : 0
            color: Theme.panelSoftBackground
            border.color: Theme.warning
            border.width: 1
            radius: Theme.radiusSm

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 4

                MajesticCheckBox {
                    text: I18n.t("После сохранения сразу выполнить reload pipeline")
                    checked: root.autoReloadAfterApply
                    enabled: root.pipelineReloadAvailable
                    onToggled: root.autoReloadAfterApplyToggled(checked)
                }

                Text {
                    Layout.fillWidth: true
                    text: root.pipelineReloadAvailable
                          ? I18n.t("Reload применит codec/resolution/fps без reboot камеры; видеопоток кратко мигнет.")
                          : I18n.t("Эта сборка Majestic не сообщает endpoint reload pipeline - после сохранения примените изменения вручную на камере.")
                    color: Theme.textMuted
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("Видеопоток может кратковременно прерваться после применения pipeline.")
            color: Theme.warning
            font.pixelSize: 11
        }
    }
}
