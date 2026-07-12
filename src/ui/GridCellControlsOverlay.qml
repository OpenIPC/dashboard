import QtQuick
import QtQuick.Controls
import OpenIPC

Rectangle {
    id: control

    property bool effectiveCanLive: false
    property bool hoverActive: false
    property bool fullscreenVisible: false
    property bool ptzVisible: false
    property bool canPtz: false
    property string iconFontFamily: "Material Icons"
    property string previewQualityText: ""
    property bool muted: true
    property real volume: 1.0
    property bool audioNormalization: false
    property bool recording: false
    property bool manualRecordingPending: false
    property bool analyticsFaceAvailable: false
    property bool analyticsObjectAvailable: false
    property bool analyticsPlateAvailable: false
    property bool analyticsFaceEnabled: false
    property bool analyticsObjectEnabled: false
    property bool analyticsPlateEnabled: false
    readonly property bool expanded: controlsHover.hovered || volumeGroup.sliderShowing

    signal permissionDenied()
    signal ptzToggleRequested()
    signal previewQualityToggleRequested()
    signal volumeEdited(real value)
    signal audioNormalizationToggled()
    signal muteToggled()
    signal recordToggleRequested()
    signal snapshotRequested()
    signal closeClicked()
    signal analyticsModuleToggleRequested(int moduleIndex)

    height: 40
    width: controlsRow.implicitWidth + 12
    color: "#cc000000"
    radius: Theme.radiusMd
    visible: effectiveCanLive && (hoverActive || expanded)
    border.color: Theme.overlayBorder
    border.width: 1

    HoverHandler {
        id: controlsHover
    }

    Row {
        id: controlsRow

        anchors.right: control.right
        anchors.rightMargin: 6
        anchors.verticalCenter: control.verticalCenter
        spacing: 6

        Button {
            id: ptzButton

            width: 32
            height: 26
            background: Rectangle { color: control.ptzVisible ? "#44ffffff" : "transparent"; radius: 3 }
            contentItem: Text {
                text: "control_camera"
                font.family: control.iconFontFamily
                font.pixelSize: 18
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                if (!control.canPtz) {
                    control.permissionDenied()
                    return
                }
                control.ptzToggleRequested()
            }
        }

        Button {
            id: qualityButton

            width: 32
            height: 26
            background: Rectangle { color: "transparent"; radius: 3 }
            contentItem: Text {
                text: control.previewQualityText
                font.family: control.iconFontFamily
                font.pixelSize: 18
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: control.previewQualityToggleRequested()
        }

        Item {
            id: volumeGroup

            property bool sliderShowing: volHover.hovered || volumeSlider.pressed || volumeSlider.hovered || normalizeBtn.hovered || audioButton.hovered

            width: 26 + (sliderShowing ? 148 : 0)
            height: 26

            HoverHandler {
                id: volHover
            }

            Row {
                anchors.right: volumeGroup.right
                anchors.verticalCenter: volumeGroup.verticalCenter
                spacing: volumeGroup.sliderShowing ? 6 : 0

                MetroSlider {
                    id: volumeSlider

                    visible: volumeGroup.sliderShowing
                    width: 110
                    height: 20
                    from: 0.0
                    to: 2.0
                    stepSize: 0.05
                    value: control.volume
                    showCenterMarker: true
                    activeColor: volumeSlider.visualPosition > 0.5 ? Theme.metroAmber : Theme.metroBlue
                    onValueChanged: control.volumeEdited(value)
                }

                Button {
                    id: normalizeBtn

                    visible: volumeGroup.sliderShowing
                    width: 26
                    height: 26
                    background: Rectangle {
                        color: control.audioNormalization ? "#2563eb" : "transparent"
                        radius: 3
                        border.color: "#999"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: "N"
                        font.bold: true
                        font.pixelSize: 12
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    ToolTip.visible: normalizeBtn.hovered
                    ToolTip.text: I18n.t("Нормализация")
                    onClicked: control.audioNormalizationToggled()
                }

                Button {
                    id: audioButton

                    width: 26
                    height: 26
                    background: Rectangle { color: "transparent"; radius: 3 }
                    contentItem: Text {
                        text: control.muted ? "volume_off" : "volume_up"
                        font.family: control.iconFontFamily
                        font.pixelSize: 18
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: control.muteToggled()
                }
            }
        }

        Button {
            id: recordBtn

            width: 26
            height: 26
            background: Rectangle { color: "transparent"; radius: 3 }
            contentItem: Text {
                text: "fiber_manual_record"
                font.family: control.iconFontFamily
                font.pixelSize: 18
                color: control.recording ? "#f44336" : (control.manualRecordingPending ? "#ffb300" : "white")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                SequentialAnimation on opacity {
                    running: control.recording
                    loops: Animation.Infinite

                    NumberAnimation { from: 1.0; to: 0.3; duration: 800; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 0.3; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                }
            }
            onClicked: control.recordToggleRequested()
        }

        Button {
            id: snapshotButton

            width: 26
            height: 26
            background: Rectangle { color: "transparent"; radius: 3 }
            contentItem: Text {
                text: "photo_camera"
                font.family: control.iconFontFamily
                font.pixelSize: 18
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: control.snapshotRequested()
        }

        Button {
            id: closeButton

            width: 26
            height: 26
            visible: !control.fullscreenVisible
            background: Rectangle { color: "transparent"; radius: 3 }
            contentItem: Text {
                text: "close"
                font.family: control.iconFontFamily
                font.pixelSize: 18
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: control.closeClicked()
        }

        Rectangle {
            width: 1
            height: 20
            color: "#55ffffff"
            anchors.verticalCenter: controlsRow.verticalCenter
            visible: analyticsRow.visible
        }

        Row {
            id: analyticsRow

            spacing: 6
            visible: control.analyticsFaceAvailable || control.analyticsObjectAvailable || control.analyticsPlateAvailable

            Button {
                id: faceButton

                width: 26
                height: 26
                visible: control.analyticsFaceAvailable
                background: Rectangle {
                    color: control.analyticsFaceEnabled ? "#4299e1" : "transparent"
                    radius: 3
                    border.color: "#55ffffff"
                    border.width: 1
                }
                contentItem: Text {
                    text: "face"
                    font.family: control.iconFontFamily
                    font.pixelSize: 18
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: control.analyticsModuleToggleRequested(0)
            }

            Button {
                id: objectButton

                width: 26
                height: 26
                visible: control.analyticsObjectAvailable
                background: Rectangle {
                    color: control.analyticsObjectEnabled ? "#4299e1" : "transparent"
                    radius: 3
                    border.color: "#55ffffff"
                    border.width: 1
                }
                contentItem: Text {
                    text: "category"
                    font.family: control.iconFontFamily
                    font.pixelSize: 18
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: control.analyticsModuleToggleRequested(1)
            }

            Button {
                id: plateButton

                width: 26
                height: 26
                visible: control.analyticsPlateAvailable
                background: Rectangle {
                    color: control.analyticsPlateEnabled ? "#4299e1" : "transparent"
                    radius: 3
                    border.color: "#55ffffff"
                    border.width: 1
                }
                contentItem: Text {
                    text: "directions_car"
                    font.family: control.iconFontFamily
                    font.pixelSize: 18
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: control.analyticsModuleToggleRequested(2)
            }
        }
    }
}
