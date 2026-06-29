import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    width: parent ? Math.min(parent.width - 40, 500) : 500
    height: 50
    color: "#333333"
    radius: Theme.radiusMd
    border.color: Theme.success
    border.width: 1
    opacity: 0
    visible: opacity > 0
    z: 2000

    property string message: ""
    property string filePath: ""

    function show(msg, path) {
        root.message = msg
        root.filePath = path || ""
        toastAnim.restart()
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        Text {
            text: "check_circle"
            font.family: "Material Icons"
            font.pixelSize: 24
            color: Theme.success
        }

        Text {
            text: root.message
            color: Theme.textPrimary
            font.pixelSize: 14
            Layout.fillWidth: true
            elide: Text.ElideMiddle
        }

        DashboardDialogButton {
            text: I18n.t("Открыть папку")
            visible: root.filePath !== ""
            buttonColor: Theme.controlBackground
            buttonHoverColor: Theme.cardHover
            buttonBorderColor: Theme.success
            buttonTextColor: Theme.textPrimary
            onClicked: {
                var folder = root.filePath.substring(0, root.filePath.lastIndexOf("/"))
                if (folder.length === 0) {
                    folder = root.filePath.substring(0, root.filePath.lastIndexOf("\\"))
                }
                if (folder.length > 0) {
                    Qt.openUrlExternally("file:///" + folder)
                }
            }
        }
    }

    SequentialAnimation {
        id: toastAnim
        NumberAnimation { target: root; property: "opacity"; to: 1; duration: 200 }
        PauseAnimation { duration: 5000 }
        NumberAnimation { target: root; property: "opacity"; to: 0; duration: 500 }
    }
}
