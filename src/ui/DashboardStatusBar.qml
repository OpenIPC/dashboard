import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var systemController: null

    implicitHeight: 25
    color: Theme.statusBarBackground

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.panelBorder
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            if (!root.systemController)
                return

            var cpuVal = root.systemController.processCpuPercent()
            var ramVal = root.systemController.processMemoryMB()
            if (cpuVal === undefined || cpuVal < 0) cpuVal = 0
            if (ramVal === undefined || ramVal < 0) ramVal = 0
            cpuText.text = I18n.t("ЦП") + ": " + cpuVal.toFixed(1) + "%"
            ramText.text = I18n.t("ОЗУ") + ": " + ramVal.toFixed(1) + " MB"
            timeText.text = Qt.formatTime(new Date(), "hh:mm:ss")
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 15

        Text {
            id: cpuText
            text: I18n.t("ЦП") + ": 0%"
            color: Theme.textPrimary
            font.pixelSize: 11
        }

        Text {
            id: ramText
            text: I18n.t("ОЗУ") + ": 0 MB"
            color: Theme.textPrimary
            font.pixelSize: 11
        }

        Item { Layout.fillWidth: true }

        Text {
            id: timeText
            text: Qt.formatTime(new Date(), "hh:mm:ss")
            color: Theme.textPrimary
            font.pixelSize: 11
        }
    }
}
