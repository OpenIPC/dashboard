import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: page

    property string appVersion: "Unknown"
    property string appAuthor: "Rinat Ibragimov"
    property string appBuildYear: "2026"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 15

        Text {
            text: "OpenIPC Dashboard"
            color: Theme.textPrimary
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Version " + page.appVersion
            color: Theme.textMuted
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Author: " + page.appAuthor
            color: Theme.textMuted
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: page.appBuildYear
            color: Theme.textFaint
            Layout.alignment: Qt.AlignHCenter
        }

        Button {
            id: supportButton

            Layout.preferredWidth: 200
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignHCenter
            text: I18n.t("Поддержать проект")
            hoverEnabled: true
            focusPolicy: Qt.StrongFocus

            background: Rectangle {
                color: supportButton.down
                       ? Theme.metroTilePressed
                       : supportButton.hovered ? Theme.metroBlueHover : Theme.metroBlue
                border.color: supportButton.visualFocus ? Theme.textPrimary : Theme.metroBlue
                border.width: supportButton.visualFocus ? 2 : 1
                radius: Theme.metroTileRadius
            }

            contentItem: RowLayout {
                spacing: 8

                Text {
                    Layout.alignment: Qt.AlignVCenter
                    text: "favorite"
                    font.family: "Material Icons"
                    color: Theme.textPrimary
                    font.pixelSize: 16
                }

                Text {
                    Layout.fillWidth: true
                    text: supportButton.text
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            onClicked: Qt.openUrlExternally("https://opencollective.com/openipc/projects/openipc-dashboard/donate?interval=oneTime&amount=20&contributeAs=me")
        }
    }
}
