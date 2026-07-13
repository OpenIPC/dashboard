import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var player: null
    property var currentFile: null
    property bool isFullScreen: false
    property bool exportMode: false
    property real exportStartMs: 0
    property real exportEndMs: 0

    readonly property real playerDuration: player && player.duration !== undefined ? player.duration : 0
    readonly property real playerPosition: player && player.position !== undefined ? player.position : 0
    readonly property bool playerRunning: player && player.running !== undefined ? player.running : false
    readonly property real playerVolume: player && player.volume !== undefined ? player.volume : 1
    readonly property bool audioNormalization: player && player.audioNormalization !== undefined ? player.audioNormalization : false

    signal resumeRequested()
    signal stopRequested()
    signal exportModeRequested()
    signal exportSaveRequested()
    signal exportCancelRequested()
    signal exportStartRequested(real ms)
    signal exportEndRequested(real ms)
    signal fullscreenToggled()

    height: 120
    color: root.isFullScreen ? "#80000000" : Theme.metroTile

    function formatTime(ms) {
        var totalSeconds = Math.floor(Math.max(0, ms) / 1000)
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        return (minutes < 10 ? "0" : "") + minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }

    function clamp(value, minimum, maximum) {
        return Math.max(minimum, Math.min(maximum, value))
    }

    function markerX(valueMs, trackWidth, markerWidth) {
        if (root.playerDuration <= 0 || trackWidth <= 0) return -markerWidth / 2
        return (clamp(valueMs, 0, root.playerDuration) / root.playerDuration) * trackWidth - markerWidth / 2
    }

    MouseArea { anchors.fill: parent }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "transparent"
            visible: root.playerDuration > 0

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text {
                    text: root.formatTime(root.playerPosition)
                    color: Theme.textSecondary
                }

                MetroSlider {
                    id: timelineSlider
                    Layout.fillWidth: true
                    from: 0
                    to: Math.max(1, root.playerDuration)
                    value: root.playerPosition

                    onMoved: {
                        if (root.player) root.player.position = value
                    }

                    background: Rectangle {
                        id: timelineTrack
                        x: timelineSlider.leftPadding
                        y: timelineSlider.topPadding + timelineSlider.availableHeight / 2 - height / 2
                        implicitWidth: 200
                        implicitHeight: 4
                        width: timelineSlider.availableWidth
                        height: implicitHeight
                        radius: 2
                        color: Theme.metroStroke

                        Rectangle {
                            width: timelineSlider.visualPosition * timelineTrack.width
                            height: timelineTrack.height
                            color: Theme.metroBlue
                            radius: 2
                        }

                        Rectangle {
                            visible: root.exportMode && root.playerDuration > 0
                            x: (root.exportStartMs / root.playerDuration) * timelineTrack.width
                            width: ((root.exportEndMs - root.exportStartMs) / root.playerDuration) * timelineTrack.width
                            height: timelineTrack.height
                            color: Theme.textPrimary
                            opacity: 0.3
                        }

                        Rectangle {
                            id: startMarker
                            visible: root.exportMode && root.playerDuration > 0
                            width: 20
                            height: 24
                            color: "transparent"
                            anchors.verticalCenter: timelineTrack.verticalCenter

                            property bool dragging: startMouseArea.drag.active
                            x: dragging ? x : root.markerX(root.exportStartMs, timelineTrack.width, width)

                            Rectangle {
                                anchors.centerIn: parent
                                width: 2
                                height: 24
                                color: Theme.success
                            }

                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: 10
                                height: 10
                                radius: 5
                                color: Theme.success
                            }

                            MouseArea {
                                id: startMouseArea
                                anchors.fill: parent
                                drag.target: startMarker
                                drag.axis: Drag.XAxis
                                drag.minimumX: -startMarker.width / 2
                                drag.maximumX: timelineTrack.width - startMarker.width / 2

                                onPositionChanged: {
                                    if (!drag.active || root.playerDuration <= 0) return
                                    var pos = startMarker.x + startMarker.width / 2
                                    var ms = (pos / timelineTrack.width) * root.playerDuration
                                    root.exportStartRequested(root.clamp(ms, 0, root.exportEndMs))
                                }
                            }
                        }

                        Rectangle {
                            id: endMarker
                            visible: root.exportMode && root.playerDuration > 0
                            width: 20
                            height: 24
                            color: "transparent"
                            anchors.verticalCenter: timelineTrack.verticalCenter

                            property bool dragging: endMouseArea.drag.active
                            x: dragging ? x : root.markerX(root.exportEndMs, timelineTrack.width, width)

                            Rectangle {
                                anchors.centerIn: parent
                                width: 2
                                height: 24
                                color: Theme.danger
                            }

                            Rectangle {
                                anchors.top: parent.top
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: 10
                                height: 10
                                radius: 5
                                color: Theme.danger
                            }

                            MouseArea {
                                id: endMouseArea
                                anchors.fill: parent
                                drag.target: endMarker
                                drag.axis: Drag.XAxis
                                drag.minimumX: -endMarker.width / 2
                                drag.maximumX: timelineTrack.width - endMarker.width / 2

                                onPositionChanged: {
                                    if (!drag.active || root.playerDuration <= 0) return
                                    var pos = endMarker.x + endMarker.width / 2
                                    var ms = (pos / timelineTrack.width) * root.playerDuration
                                    root.exportEndRequested(root.clamp(ms, root.exportStartMs, root.playerDuration))
                                }
                            }
                        }
                    }

                    handle: Rectangle {
                        x: timelineSlider.leftPadding + timelineSlider.visualPosition * (timelineSlider.availableWidth - width)
                        y: timelineSlider.topPadding + timelineSlider.availableHeight / 2 - height / 2
                        implicitWidth: 16
                        implicitHeight: 16
                        radius: 8
                        color: timelineSlider.pressed ? Theme.metroBlueHover : Theme.metroBlue
                        border.color: timelineSlider.hovered ? Theme.textPrimary : Theme.metroStrokeStrong
                    }
                }

                Text {
                    text: root.formatTime(root.playerDuration)
                    color: Theme.textSecondary
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                Button {
                    id: rewindButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: "-10s"
                    enabled: root.playerDuration > 0
                    hoverEnabled: false
                    background: Rectangle {
                        color: "transparent"
                        border.color: Theme.textFaint
                        radius: 20
                    }
                    contentItem: Text {
                        text: rewindButton.text
                        color: rewindButton.enabled ? Theme.textSecondary : Theme.textFaint
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 10
                    }
                    onClicked: {
                        if (root.player) root.player.position = Math.max(0, root.playerPosition - 10000)
                    }
                }

                Button {
                    id: playPauseButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: root.playerRunning ? "II" : ">"
                    enabled: root.currentFile !== null
                    hoverEnabled: false

                    background: Rectangle {
                        color: playPauseButton.down ? Theme.metroBlueHover : Theme.metroBlue
                        radius: 20
                        opacity: playPauseButton.enabled ? 1 : 0.45
                    }

                    contentItem: Text {
                        text: playPauseButton.text
                        color: Theme.textPrimary
                        font.pixelSize: root.playerRunning ? 17 : 20
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (!root.player) return
                        if (root.playerRunning) {
                            root.player.running = false
                        } else {
                            root.resumeRequested()
                        }
                    }
                }

                Button {
                    id: forwardButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: "+10s"
                    enabled: root.playerDuration > 0
                    hoverEnabled: false
                    background: Rectangle {
                        color: "transparent"
                        border.color: Theme.textFaint
                        radius: 20
                    }
                    contentItem: Text {
                        text: forwardButton.text
                        color: forwardButton.enabled ? Theme.textSecondary : Theme.textFaint
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 10
                    }
                    onClicked: {
                        if (root.player) root.player.position = Math.min(root.playerDuration, root.playerPosition + 10000)
                    }
                }

                Button {
                    id: stopButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: "Stop"
                    enabled: root.playerDuration > 0 || root.playerRunning
                    hoverEnabled: false
                    background: Rectangle {
                        color: "transparent"
                        border.color: Theme.textFaint
                        radius: 20
                    }
                    contentItem: Text {
                        text: stopButton.text
                        color: stopButton.enabled ? Theme.textSecondary : Theme.textFaint
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 9
                    }
                    onClicked: root.stopRequested()
                }

                RowLayout {
                    spacing: 8

                    Text {
                        text: "Vol"
                        color: Theme.textSecondary
                        font.pixelSize: 11
                    }

                    Button {
                        id: normalizationButton
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        enabled: root.player !== null
                        background: Rectangle {
                            color: root.audioNormalization ? Theme.metroBlue : "transparent"
                            radius: Theme.metroTileRadius
                            border.color: Theme.textFaint
                            border.width: 1
                            opacity: normalizationButton.enabled ? 1 : 0.45
                        }
                        contentItem: Text {
                            text: "N"
                            font.bold: true
                            font.pixelSize: 12
                            color: Theme.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        ToolTip.visible: normalizationButton.hovered
                        ToolTip.text: I18n.t("Нормализация (усиление тихих звуков)")

                        onClicked: {
                            if (root.player) root.player.audioNormalization = !root.player.audioNormalization
                        }
                    }

                    MetroSlider {
                        id: volumeSlider
                        from: 0
                        to: 2.0
                        value: root.playerVolume
                        Layout.preferredWidth: 100
                        enabled: root.player !== null

                        onMoved: {
                            if (root.player) root.player.volume = value
                        }

                        background: Rectangle {
                            id: volumeTrack
                            x: volumeSlider.leftPadding
                            y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                            implicitWidth: 200
                            implicitHeight: 4
                            width: volumeSlider.availableWidth
                            height: implicitHeight
                            radius: 2
                            color: Theme.metroStroke

                            Rectangle {
                                width: volumeSlider.visualPosition * volumeTrack.width
                                height: volumeTrack.height
                                color: volumeSlider.visualPosition > 0.5 ? Theme.metroAmber : Theme.metroBlue
                                radius: 2
                            }

                            Rectangle {
                                x: volumeTrack.width * 0.5
                                width: 1
                                height: 8
                                anchors.verticalCenter: volumeTrack.verticalCenter
                                color: Theme.textMuted
                            }
                        }

                        handle: Rectangle {
                            x: volumeSlider.leftPadding + volumeSlider.visualPosition * (volumeSlider.availableWidth - width)
                            y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: 8
                            color: volumeSlider.pressed ? Theme.metroBlueHover : Theme.metroBlue
                            border.color: volumeSlider.hovered ? Theme.textPrimary : Theme.metroStrokeStrong
                        }
                    }
                }

                StyledComboBox {
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 30
                    model: ["0.5x", "1.0x", "2.0x", "4.0x", "8.0x"]
                    currentIndex: 1
                    enabled: root.player !== null
                    onCurrentTextChanged: {
                        if (!root.player) return
                        root.player.playbackRate = parseFloat(currentText.replace("x", ""))
                    }
                }

                Item { Layout.fillWidth: true }

                RowLayout {
                    visible: root.exportMode
                    spacing: 10

                    Button {
                        id: saveExportButton
                        text: I18n.t("Сохранить")
                        enabled: root.exportEndMs > root.exportStartMs
                        hoverEnabled: false
                        onClicked: root.exportSaveRequested()
                        background: Rectangle {
                            color: Theme.metroBlue
                            radius: Theme.metroTileRadius
                            opacity: saveExportButton.enabled ? 1 : 0.45
                        }
                        contentItem: Text {
                            text: saveExportButton.text
                            color: Theme.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Button {
                        id: cancelExportButton
                        text: I18n.t("Отмена")
                        hoverEnabled: false
                        onClicked: root.exportCancelRequested()
                        background: Rectangle {
                            color: "transparent"
                            radius: Theme.metroTileRadius
                        }
                        contentItem: Text {
                            text: cancelExportButton.text
                            color: Theme.textSecondary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                Button {
                    id: exportModeButton
                    visible: !root.exportMode
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    enabled: root.currentFile !== null && root.playerDuration > 0
                    hoverEnabled: false
                    background: Rectangle {
                        color: "transparent"
                        radius: Theme.metroTileRadius
                        opacity: exportModeButton.enabled ? 1 : 0.45
                    }
                    contentItem: Text {
                        text: "Cut"
                        color: Theme.textSecondary
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: root.exportModeRequested()
                }

                Button {
                    id: fullscreenButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    hoverEnabled: false
                    background: Rectangle {
                        color: "transparent"
                        radius: Theme.metroTileRadius
                    }
                    contentItem: SidebarIcon {
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        color: Theme.textSecondary
                        name: root.isFullScreen ? "fullscreen_exit" : "fullscreen"
                        path: root.isFullScreen
                              ? "M5 16h3v3h2v-5H5v2zm3-8H5v2h5V5H8v3zm6 11h2v-3h3v-2h-5v5zm2-11V5h-2v5h5V8h-3z"
                              : "M7 14H5v5h5v-2H7v-3zm-2-4h2V7h3V5H5v5zm12 7h-3v2h5v-5h-2v3zM14 5v2h3v3h2V5h-5z"
                    }
                    onClicked: root.fullscreenToggled()
                }
            }
        }
    }
}
