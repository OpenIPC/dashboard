pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: root

    property var controller: null
    property alias text: rawEditor.text

    spacing: 10

    ColumnLayout {
        Layout.fillWidth: true
        Layout.margins: 12
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: I18n.t("На камеру уйдёт только diff; неизвестные поля не удаляются.")
            color: Theme.textMuted
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            MajesticButton {
                text: I18n.t("Форматировать")
                enabled: root.controller !== null
                onClicked: {
                    var p = root.controller.parseMajesticJson(rawEditor.text)
                    if (p.ok) {
                        rawEditor.text = JSON.stringify(p.value, null, 2)
                    } else if (root.controller) {
                        root.controller.statusError = true
                        root.controller.statusText = p.error
                    }
                }
            }

            MajesticButton {
                text: I18n.t("Вернуть оригинал")
                enabled: root.controller !== null
                onClicked: rawEditor.text = JSON.stringify(root.controller.originalConfig, null, 2)
            }

            MajesticButton {
                text: I18n.t("Копировать без секретов")
                enabled: root.controller !== null
                onClicked: root.controller.copyRedactedRawJson()
            }

            MajesticButton {
                text: I18n.t("Проверить и применить")
                primary: true
                enabled: root.controller
                         && root.controller.capabilities.configWrite === true
                         && !root.controller.loading
                onClicked: {
                    var p = root.controller.parseMajesticJson(rawEditor.text)
                    if (!p.ok) {
                        root.controller.statusError = true
                        root.controller.statusText = I18n.t("Ошибка JSON: ") + p.error
                        return
                    }
                    root.controller.prepareApply(p.value)
                }
            }
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 12
        Layout.topMargin: 0

        TextArea {
            id: rawEditor

            text: "{}"
            color: Theme.textSecondary
            selectionColor: Theme.accent
            selectedTextColor: Theme.textPrimary
            font.family: "Consolas"
            font.pixelSize: 12
            wrapMode: TextEdit.NoWrap

            background: Rectangle {
                color: Theme.controlBackground
                border.color: Theme.controlBorder
                radius: Theme.radiusMd
            }
        }
    }
}

