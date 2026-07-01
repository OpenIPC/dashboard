import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property bool webSocketsAvailable: false

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    Layout.preferredHeight: 116
    color: "#422006"
    border.color: Theme.warning
    radius: Theme.radiusLg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 6

        Text {
            Layout.fillWidth: true
            text: I18n.t("Updater OpenIPC останавливает видео и перезагружает камеру. Не выключайте питание во время прошивки.")
            color: "#fde68a"
            wrapMode: Text.WordWrap
            font.pixelSize: 12
            font.bold: true
        }

        Text {
            Layout.fillWidth: true
            text: I18n.t("Прямой upload архива уже пишет файл в /tmp/firmware.tgz. Финальная прошивка штатно идёт через /ws/upgrade; если модуль WebSockets недоступен, используйте кнопку WebUI.")
            color: "#fde68a"
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }

        Text {
            Layout.fillWidth: true
            text: root.webSocketsAvailable
                  ? I18n.t("Native /ws/upgrade готов: приложение может запускать WebSocket updater.")
                  : I18n.t("Native /ws/upgrade недоступен в этой сборке: CMake соберёт его автоматически при наличии Qt WebSockets.")
            color: root.webSocketsAvailable ? Theme.success : "#fde68a"
            wrapMode: Text.WordWrap
            font.pixelSize: 11
        }
    }
}
