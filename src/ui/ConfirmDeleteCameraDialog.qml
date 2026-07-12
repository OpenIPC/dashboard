import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root

    modal: true
    width: 400
    height: 180
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    property int cameraIndex: -1

    signal deleteAccepted(int cameraIndex)

    background: Rectangle {
        color: Theme.metroSurface
        border.color: Theme.metroStroke
        radius: Theme.metroTileRadius
    }

    header: Rectangle {
        color: "transparent"
        height: 50

        Text {
            anchors.centerIn: parent
            text: I18n.t("Удаление камеры")
            color: Theme.textPrimary
            font.bold: true
            font.pixelSize: 16
        }
    }

    contentItem: ColumnLayout {
        spacing: 20

        Text {
            text: I18n.t("Вы действительно хотите удалить эту камеру?")
            color: Theme.textSecondary
            font.pixelSize: 14
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            DashboardDialogButton {
                text: I18n.t("ОТМЕНА")
                buttonColor: Theme.metroTile
                buttonHoverColor: Theme.metroTileHover
                buttonBorderColor: Theme.metroStroke
                buttonTextColor: Theme.textPrimary
                onClicked: root.close()
            }

            DashboardDialogButton {
                text: I18n.t("УДАЛИТЬ")
                buttonColor: Theme.metroRed
                buttonHoverColor: Theme.metroRed
                buttonBorderColor: Theme.metroRed
                buttonTextColor: Theme.textPrimary
                buttonTextBold: true
                onClicked: {
                    if (root.cameraIndex >= 0) {
                        root.deleteAccepted(root.cameraIndex)
                    }
                    root.close()
                }
            }
        }
    }
}
