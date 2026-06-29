import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root

    modal: true
    title: I18n.t("Недостаточно прав")
    width: 420
    parent: Overlay.overlay
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    property string message: I18n.t("У вас недостаточно прав для выполнения этого действия.")

    background: Rectangle {
        color: Theme.panelAltBackground
        border.color: Theme.panelBorder
        radius: Theme.radiusMd
    }

    header: Rectangle {
        height: 36
        color: Theme.topBarBackground
        radius: Theme.radiusMd
        border.color: Theme.panelBorder

        Text {
            anchors.centerIn: parent
            text: root.title
            color: Theme.textPrimary
            font.bold: true
            font.pixelSize: 14
        }
    }

    contentItem: Rectangle {
        color: "transparent"
        clip: true

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: root.message
                color: Theme.textSecondary
                wrapMode: Text.WordWrap
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                Layout.rightMargin: 24
                Layout.bottomMargin: 12

                Item { Layout.fillWidth: true }

                DashboardDialogButton {
                    text: I18n.t("ОК")
                    Layout.preferredWidth: 72
                    Layout.preferredHeight: 28
                    buttonColor: Theme.accent
                    buttonHoverColor: Theme.accentHover
                    buttonBorderColor: Theme.accent
                    buttonTextColor: Theme.textPrimary
                    onClicked: root.close()
                }
            }
        }
    }
}
