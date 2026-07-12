import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var controller: null
    property string selectedRunId: ""

    signal runSelected(string runId)

    implicitHeight: 82
    radius: Theme.radiusMd
    color: Theme.panelAltBackground
    border.color: Theme.panelBorder

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        ColumnLayout {
            Layout.preferredWidth: 154
            Layout.fillHeight: true
            spacing: 3

            Text {
                text: I18n.language === "ru" ? "История проверок" : "Diagnostic history"
                color: Theme.textPrimary
                font.pixelSize: 12
                font.bold: true
            }
            Text {
                Layout.fillWidth: true
                text: root.controller && root.controller.history.length > 0
                      ? (I18n.language === "ru" ? "Сохранено запусков: " : "Saved runs: ")
                        + root.controller.history.length
                      : (I18n.language === "ru" ? "История пока пуста" : "No saved runs")
                color: Theme.textMuted
                font.pixelSize: 10
                elide: Text.ElideRight
            }
            Button {
                visible: root.controller && root.controller.history.length > 0
                text: I18n.language === "ru" ? "Очистить" : "Clear"
                flat: true
                Layout.preferredHeight: 24
                onClicked: root.controller.clearHistory()
            }
        }

        ListView {
            id: historyList

            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: ListView.Horizontal
            spacing: 8
            clip: true
            model: root.controller ? root.controller.history : []

            delegate: Rectangle {
                id: historyCard

                required property var modelData
                readonly property var run: modelData || ({})
                readonly property string runId: String(run.id || "")
                readonly property bool selected: root.selectedRunId === runId

                width: 226
                height: historyList.height
                radius: Theme.radiusSm
                color: selected ? Theme.controlActive : Theme.controlBackground
                border.color: selected ? Theme.accent : Theme.controlBorder

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 7
                    spacing: 2

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Text {
                            Layout.fillWidth: true
                            text: String(historyCard.run.completedAtLabel
                                         || historyCard.run.completedAt || "")
                            color: Theme.textPrimary
                            font.pixelSize: 10
                            font.bold: true
                            elide: Text.ElideRight
                        }
                        Text {
                            text: String(historyCard.run.profileLabel || "")
                            color: Theme.accentHover
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: String(historyCard.run.summary || "")
                        color: Theme.textMuted
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                }

                TapHandler {
                    onTapped: {
                        root.selectedRunId = historyCard.runId
                        root.runSelected(historyCard.runId)
                    }
                }
            }
        }
    }
}
