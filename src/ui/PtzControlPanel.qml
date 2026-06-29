import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import OpenIPC 1.0

Item {
    id: root

    property string cameraIp
    property int cameraPort
    property string cameraLogin
    property string iconFontFamily: "Material Icons"
    property bool compact: false
    property bool discovering: false
    property bool ptzStatusError: false
    property string ptzStatusText: I18n.t("ONVIF/PTZ не проверен")
    property string ptzEndpointText: ""

    width: compact ? 170 : 220
    height: compact ? 270 : 340

    property real ptzSpeed: speedSlider.value

    function currentPassword() {
        return SystemController.getCameraPassword(root.cameraIp)
    }

    function matches(ip, port) {
        return ip === root.cameraIp && port === root.cameraPort
    }

    function refreshCachedSummary() {
        var summary = SystemController.ptzController.cachedSummary(root.cameraIp, root.cameraPort)
        if (summary && summary.length) {
            ptzStatusError = false
            ptzStatusText = I18n.t("ONVIF/PTZ готов")
            ptzEndpointText = summary
        } else {
            ptzStatusError = false
            ptzStatusText = I18n.t("ONVIF/PTZ не проверен")
            ptzEndpointText = ""
        }
    }

    function probeOnvif() {
        discovering = true
        ptzStatusError = false
        ptzStatusText = I18n.t("Проверка ONVIF/PTZ…")
        ptzEndpointText = ""
        SystemController.ptzController.probe(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
    }

    Component.onCompleted: refreshCachedSummary()
    onCameraIpChanged: refreshCachedSummary()
    onCameraPortChanged: refreshCachedSummary()

    Connections {
        target: SystemController.ptzController
        function onDiscoveryStarted(ip, port) {
            if (!root.matches(ip, port)) return
            root.discovering = true
            root.ptzStatusError = false
            root.ptzStatusText = I18n.t("Проверка ONVIF/PTZ…")
            root.ptzEndpointText = ""
        }
        function onDiscoveryFinished(ip, port, success, message, profileToken, ptzUrl, imagingUrl) {
            if (!root.matches(ip, port)) return
            root.discovering = false
            root.ptzStatusError = !success
            root.ptzStatusText = success ? I18n.t("ONVIF/PTZ готов") : I18n.t(message)
            root.ptzEndpointText = success
                    ? (I18n.t("Профиль: ") + profileToken + " · PTZ: " + ptzUrl
                       + (imagingUrl && imagingUrl.length ? " · Imaging: " + imagingUrl : ""))
                    : ""
        }
        function onCommandFailed(ip, port, operation, message) {
            if (!root.matches(ip, port)) return
            root.discovering = false
            root.ptzStatusError = true
            root.ptzStatusText = I18n.t(message)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#cc000000"
        radius: 10
        border.color: "#55ffffff"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: root.compact ? 5 : 10
            spacing: root.compact ? 5 : 10

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
                    onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword(), 0, root.ptzSpeed, 0)
                    onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
                }
                // Down
                Button {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: root.compact ? 30 : 40; height: root.compact ? 30 : 40
                    background: Item {}
                    contentItem: Text { text: "keyboard_arrow_down"; font.family: root.iconFontFamily; color: "white"; font.pixelSize: root.compact ? 24 : 32; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword(), 0, -root.ptzSpeed, 0)
                    onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
                }
                // Left
                Button {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: root.compact ? 30 : 40; height: root.compact ? 30 : 40
                    background: Item {}
                    contentItem: Text { text: "keyboard_arrow_left"; font.family: root.iconFontFamily; color: "white"; font.pixelSize: root.compact ? 24 : 32; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword(), -root.ptzSpeed, 0, 0)
                    onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
                }
                // Right
                Button {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: root.compact ? 30 : 40; height: root.compact ? 30 : 40
                    background: Item {}
                    contentItem: Text { text: "keyboard_arrow_right"; font.family: root.iconFontFamily; color: "white"; font.pixelSize: root.compact ? 24 : 32; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword(), root.ptzSpeed, 0, 0)
                    onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
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
                            id: zoomOutButton
                            implicitWidth: root.compact ? 30 : 40; implicitHeight: root.compact ? 24 : 30
                            background: Rectangle { color: zoomOutButton.down ? "#66ffffff" : "#33ffffff"; radius: 4 }
                            contentItem: Text { text: "-"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword(), 0, 0, -root.ptzSpeed)
                            onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
                        }
                        Button {
                            id: zoomInButton
                            implicitWidth: root.compact ? 30 : 40; implicitHeight: root.compact ? 24 : 30
                            background: Rectangle { color: zoomInButton.down ? "#66ffffff" : "#33ffffff"; radius: 4 }
                            contentItem: Text { text: "+"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onPressed: SystemController.ptzController.move(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword(), 0, 0, root.ptzSpeed)
                            onReleased: SystemController.ptzController.stop(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
                        }
                    }
                }

                // Focus
                ColumnLayout {
                    spacing: 2
                    Text { text: I18n.t("Focus"); color: "white"; font.pixelSize: root.compact ? 10 : 12; Layout.alignment: Qt.AlignHCenter }
                    RowLayout {
                        Button {
                            id: focusNearButton
                            implicitWidth: root.compact ? 30 : 40; implicitHeight: root.compact ? 24 : 30
                            background: Rectangle { color: focusNearButton.down ? "#66ffffff" : "#33ffffff"; radius: 4 }
                            contentItem: Text { text: "-"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onPressed: SystemController.ptzController.focus(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword(), -root.ptzSpeed)
                            onReleased: SystemController.ptzController.stopFocus(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
                        }
                        Button {
                            id: focusFarButton
                            implicitWidth: root.compact ? 30 : 40; implicitHeight: root.compact ? 24 : 30
                            background: Rectangle { color: focusFarButton.down ? "#66ffffff" : "#33ffffff"; radius: 4 }
                            contentItem: Text { text: "+"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onPressed: SystemController.ptzController.focus(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword(), root.ptzSpeed)
                            onReleased: SystemController.ptzController.stopFocus(root.cameraIp, root.cameraPort, root.cameraLogin, root.currentPassword())
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

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#33ffffff"
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                Button {
                    id: probeButton
                    Layout.fillWidth: true
                    implicitHeight: root.compact ? 24 : 28
                    text: root.discovering ? I18n.t("Проверка…") : I18n.t("Проверить ONVIF")
                    enabled: !root.discovering
                    background: Rectangle {
                        color: probeButton.enabled ? (probeButton.down ? "#4477aaee" : "#335ba7ff") : "#22444444"
                        radius: 4
                        border.color: "#5577bbff"
                    }
                    contentItem: Text {
                        text: probeButton.text
                        color: "white"
                        font.pixelSize: root.compact ? 9 : 11
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    onClicked: root.probeOnvif()
                }
                Button {
                    id: resetButton
                    implicitWidth: root.compact ? 34 : 42
                    implicitHeight: root.compact ? 24 : 28
                    text: "↺"
                    background: Rectangle {
                        color: resetButton.down ? "#66ffffff" : "#22ffffff"
                        radius: 4
                        border.color: "#44ffffff"
                    }
                    contentItem: Text {
                        text: resetButton.text
                        color: "white"
                        font.pixelSize: root.compact ? 12 : 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        SystemController.ptzController.clearCache(root.cameraIp, root.cameraPort)
                        root.refreshCachedSummary()
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: root.ptzStatusText
                color: root.ptzStatusError ? "#ff6b6b" : "#a7f3d0"
                font.pixelSize: root.compact ? 9 : 11
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                visible: root.ptzEndpointText.length > 0
                text: root.ptzEndpointText
                color: "#cbd5e1"
                font.pixelSize: root.compact ? 8 : 10
                elide: Text.ElideRight
            }
        }
    }
}
