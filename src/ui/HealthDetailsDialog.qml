import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: root

    property var healthResult: ({})

    modal: true
    width: Math.min(parent ? parent.width - 80 : 840, 900)
    height: Math.min(parent ? parent.height - 80 : 620, 680)
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    leftPadding: 14
    rightPadding: 14
    topPadding: detailsHeader.height + 14
    bottomPadding: 14
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    function statusColor(status) {
        if (status === "ok") return Theme.success
        if (status === "error") return Theme.danger
        if (status === "warning") return Theme.warning
        return Theme.textMuted
    }

    background: Rectangle {
        color: Theme.panelBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorderStrong
    }

    header: Rectangle {
        id: detailsHeader

        height: 62
        color: Theme.topBarBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorderStrong

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 10
            spacing: 10

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    Layout.fillWidth: true
                    text: String(root.healthResult.name || (I18n.language === "ru"
                                                      ? "Детали диагностики"
                                                      : "Diagnostic details"))
                    color: Theme.textPrimary
                    font.pixelSize: 18
                    font.bold: true
                    elide: Text.ElideRight
                }
                Text {
                    text: String(root.healthResult.ip || "")
                          + "  |  " + String(root.healthResult.profile || "")
                    color: Theme.textMuted
                    font.pixelSize: 11
                }
            }

            MetroWindowButton {
                kind: "close"
                Layout.preferredWidth: 38
                Layout.preferredHeight: 34
                onClicked: root.close()
            }
        }
    }

    contentItem: ScrollView {
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: parent.width
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: recommendationText.implicitHeight + 30
                radius: Theme.radiusMd
                color: Theme.panelSoftBackground
                border.color: root.statusColor(String(root.healthResult.status || ""))

                Text {
                    id: recommendationText
                    anchors.fill: parent
                    anchors.margins: 14
                    text: I18n.t(String(root.healthResult.recommendation || ""))
                    color: Theme.textPrimary
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignVCenter
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: 6
                columnSpacing: 12

                Text { text: "RTSP"; color: Theme.textMuted; font.pixelSize: 11 }
                Text {
                    Layout.fillWidth: true
                    text: String(root.healthResult.mainStreamUrl || "-")
                    color: Theme.textSecondary
                    font.pixelSize: 11
                    elide: Text.ElideMiddle
                }
                Text {
                    text: I18n.language === "ru" ? "Прошивка" : "Firmware"
                    color: Theme.textMuted
                    font.pixelSize: 11
                }
                Text {
                    text: String(root.healthResult.firmwareVersion || "-")
                    color: Theme.textSecondary
                    font.pixelSize: 11
                }
                Text { text: "Majestic"; color: Theme.textMuted; font.pixelSize: 11 }
                Text {
                    text: String(root.healthResult.majesticVersion || "-")
                    color: Theme.textSecondary
                    font.pixelSize: 11
                }
            }

            Text {
                text: I18n.language === "ru" ? "Результаты проверок" : "Probe results"
                color: Theme.textPrimary
                font.pixelSize: 13
                font.bold: true
            }

            Repeater {
                model: root.healthResult.probes || []

                delegate: Rectangle {
                    id: probeRow

                    required property var modelData
                    readonly property var probe: modelData || ({})

                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    radius: Theme.radiusSm
                    color: Theme.controlBackground
                    border.color: root.statusColor(String(probe.status || ""))

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 9
                        spacing: 9

                        Rectangle {
                            Layout.preferredWidth: 8
                            Layout.preferredHeight: 8
                            radius: 4
                            color: root.statusColor(String(probeRow.probe.status || ""))
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            Text {
                                Layout.fillWidth: true
                                text: String(probeRow.probe.label || "")
                                color: Theme.textPrimary
                                font.pixelSize: 11
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: String(probeRow.probe.message || "")
                                color: Theme.textMuted
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }
                        Text {
                            text: String(probeRow.probe.elapsedMs || 0) + " ms"
                            color: Theme.textMuted
                            font.pixelSize: 10
                        }
                    }
                }
            }

            Text {
                visible: String(root.healthResult.lastLogs || "") !== ""
                text: I18n.language === "ru" ? "Последние логи" : "Recent logs"
                color: Theme.textPrimary
                font.pixelSize: 13
                font.bold: true
            }
            TextArea {
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                visible: String(root.healthResult.lastLogs || "") !== ""
                readOnly: true
                text: String(root.healthResult.lastLogs || "")
                color: Theme.textSecondary
                font.family: "Consolas"
                font.pixelSize: 10
                wrapMode: TextEdit.WrapAnywhere
                background: Rectangle {
                    color: Theme.controlBackground
                    border.color: Theme.controlBorder
                    radius: Theme.radiusSm
                }
            }
        }
    }
}
