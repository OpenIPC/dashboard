import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import OpenIPC 1.0

Item {
    id: root

    property string cameraIp
    property int cameraPort
    property string cameraLogin
    property string cameraPassword
    property string iconFontFamily: "Material Icons"
    property bool compact: false

    width: compact ? 170 : 220
    height: compact ? 220 : 280

    property real ptzSpeed: speedSlider.value

    Rectangle {
        anchors.fill: parent
        color: "#cc000000"
        radius: 10
        border.color: "#55ffffff"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: compact ? 5 : 10
            spacing: compact ? 5 : 10

            // D-Pad
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: root.compact ? 80 : 120
                Layout.preferredHeight: root.compact ? 80 : 120

                // Background circle
                Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: "#33ffffff"
                    border.color: "#55ffffff"
                }

                // Up
                Button {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: root.compact ? 30 : 40; height: root.compact ? 30 : 40
                    background: Item {}
                    contentItem: Text { text: "keyboard_arrow_up"; font.family: root.iconFontFamily; color: "white"; font.pixelSize: root.compact ? 24 : 32; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword, 0, root.ptzSpeed, 0)
                    onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                }
                // Down
                Button {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: root.compact ? 30 : 40; height: root.compact ? 30 : 40
                    background: Item {}
                    contentItem: Text { text: "keyboard_arrow_down"; font.family: root.iconFontFamily; color: "white"; font.pixelSize: root.compact ? 24 : 32; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword, 0, -root.ptzSpeed, 0)
                    onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                }
                // Left
                Button {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: root.compact ? 30 : 40; height: root.compact ? 30 : 40
                    background: Item {}
                    contentItem: Text { text: "keyboard_arrow_left"; font.family: root.iconFontFamily; color: "white"; font.pixelSize: root.compact ? 24 : 32; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword, -root.ptzSpeed, 0, 0)
                    onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                }
                // Right
                Button {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: root.compact ? 30 : 40; height: root.compact ? 30 : 40
                    background: Item {}
                    contentItem: Text { text: "keyboard_arrow_right"; font.family: root.iconFontFamily; color: "white"; font.pixelSize: root.compact ? 24 : 32; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword, root.ptzSpeed, 0, 0)
                    onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                }
            }

            // Zoom and Focus Row
            RowLayout {
                Layout.fillWidth: true
                spacing: root.compact ? 5 : 10
                Layout.alignment: Qt.AlignHCenter

                // Zoom
                ColumnLayout {
                    spacing: 2
                    Text { text: I18n.t("Zoom"); color: "white"; font.pixelSize: root.compact ? 10 : 12; Layout.alignment: Qt.AlignHCenter }
                    RowLayout {
                        Button {
                            implicitWidth: root.compact ? 30 : 40; implicitHeight: root.compact ? 24 : 30
                            background: Rectangle { color: parent.down ? "#66ffffff" : "#33ffffff"; radius: 4 }
                            contentItem: Text { text: "-"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword, 0, 0, -root.ptzSpeed)
                            onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                        }
                        Button {
                            implicitWidth: root.compact ? 30 : 40; implicitHeight: root.compact ? 24 : 30
                            background: Rectangle { color: parent.down ? "#66ffffff" : "#33ffffff"; radius: 4 }
                            contentItem: Text { text: "+"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword, 0, 0, root.ptzSpeed)
                            onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                        }
                    }
                }

                // Focus
                ColumnLayout {
                    spacing: 2
                    Text { text: I18n.t("Focus"); color: "white"; font.pixelSize: root.compact ? 10 : 12; Layout.alignment: Qt.AlignHCenter }
                    RowLayout {
                        Button {
                            implicitWidth: root.compact ? 30 : 40; implicitHeight: root.compact ? 24 : 30
                            background: Rectangle { color: parent.down ? "#66ffffff" : "#33ffffff"; radius: 4 }
                            contentItem: Text { text: "-"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onPressed: SystemController.ptzController.focus(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword, -root.ptzSpeed)
                            onReleased: SystemController.ptzController.stopFocus(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                        }
                        Button {
                            implicitWidth: root.compact ? 30 : 40; implicitHeight: root.compact ? 24 : 30
                            background: Rectangle { color: parent.down ? "#66ffffff" : "#33ffffff"; radius: 4 }
                            contentItem: Text { text: "+"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onPressed: SystemController.ptzController.focus(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword, root.ptzSpeed)
                            onReleased: SystemController.ptzController.stopFocus(root.cameraIp, root.cameraPort, root.cameraLogin, root.cameraPassword)
                        }
                    }
                }
            }

            // Speed Slider
            RowLayout {
                Layout.fillWidth: true
                Text { text: I18n.t("Speed"); color: "white"; font.pixelSize: root.compact ? 10 : 12 }
                Slider {
                    id: speedSlider
                    Layout.fillWidth: true
                    from: 0.1
                    to: 1.0
                    value: 0.5
                }
            }
        }
    }
}
