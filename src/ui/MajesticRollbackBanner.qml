pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var controller: null

    visible: root.controller && root.controller.rollbackAvailable
    Layout.fillWidth: true
    Layout.leftMargin: 12
    Layout.rightMargin: 12
    Layout.preferredHeight: visible ? 74 : 0
    color: "#1f2937"
    border.color: Theme.warning
    radius: Theme.radiusSm

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            Text {
                text: I18n.t("Safe rollback доступен")
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 13
            }

            Text {
                Layout.fillWidth: true
                text: root.controller && root.controller.rollbackReason.length
                      ? root.controller.rollbackReason
                      : I18n.t("Можно вернуть предыдущую конфигурацию Majestic одним diff-запросом.")
                color: Theme.textMuted
                wrapMode: Text.WordWrap
                font.pixelSize: 11
            }

            Text {
                text: I18n.t("Критичных полей: %1", [root.controller ? root.controller.rollbackChanges.length : 0])
                color: Theme.warning
                font.pixelSize: 10
            }
        }

        MajesticButton {
            text: I18n.t("Откатить")
            danger: true
            enabled: root.controller && !root.controller.loading
            onClicked: root.controller.openRollbackConfirm()
        }

        MajesticButton {
            text: I18n.t("Скрыть")
            subtle: true
            enabled: root.controller && !root.controller.loading
            onClicked: root.controller.clearRollbackSnapshot()
        }
    }
}

