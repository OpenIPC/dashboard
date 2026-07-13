import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property bool isExporting: false
    property int progress: 0
    property string status: ""
    property string errorText: ""
    property string outputFile: ""

    signal closeRequested()
    signal openFolderRequested(string path)

    readonly property bool hasMessage: isExporting || status.length > 0 || errorText.length > 0

    visible: hasMessage
    height: hasMessage ? 92 : 0
    color: Theme.metroSurface
    border.color: errorText.length > 0 ? Theme.danger : Theme.metroBlue
    border.width: 1
    radius: Theme.metroTileRadius
    opacity: 0.98

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 8
                Layout.preferredHeight: 8
                radius: 4
                color: errorText.length > 0 ? Theme.danger : (root.isExporting ? Theme.warning : Theme.success)
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: errorText.length > 0 ? I18n.t("Ошибка экспорта") : (status.length > 0 ? I18n.t(status) : I18n.t("Экспорт"))
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 14
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: errorText.length > 0 ? errorText : outputFile
                    color: errorText.length > 0 ? Theme.warningText : Theme.textMuted
                    font.pixelSize: 11
                    elide: Text.ElideMiddle
                    visible: text.length > 0
                }
            }

            Button {
                id: openButton
                visible: !root.isExporting && root.outputFile.length > 0 && root.errorText.length === 0
                Layout.preferredWidth: 120
                Layout.preferredHeight: 32
                text: I18n.t("Открыть папку")
                onClicked: root.openFolderRequested(root.outputFile)
                background: Rectangle {
                    color: openButton.down ? Theme.metroBlueHover : Theme.metroTile
                    border.color: Theme.metroBlue
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: openButton.text
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            Button {
                id: closeButton
                visible: !root.isExporting
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                text: "x"
                onClicked: root.closeRequested()
                background: Rectangle {
                    color: closeButton.down ? Theme.metroTilePressed : Theme.metroTile
                    border.color: Theme.metroStroke
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: closeButton.text
                    color: Theme.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
            }
        }

        ProgressBar {
            id: exportProgressBar
            Layout.fillWidth: true
            Layout.preferredHeight: 8
            from: 0
            to: 100
            value: Math.max(0, Math.min(100, root.progress))
            visible: root.isExporting || root.progress > 0
            background: Rectangle {
                color: Theme.metroBackground
                border.color: Theme.metroStroke
                radius: Theme.metroTileRadius
            }
            contentItem: Item {
                Rectangle {
                    width: exportProgressBar.visualPosition * parent.width
                    height: parent.height
                    radius: Theme.metroTileRadius
                    color: root.errorText.length > 0 ? Theme.danger : Theme.metroBlue
                }
            }
        }
    }
}
