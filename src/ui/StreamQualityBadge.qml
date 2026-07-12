import QtQuick

Rectangle {
    id: root

    property string previewQuality: ""
    property string fullscreenQuality: ""
    property int labelPixelSize: 12
    property int horizontalPadding: 20

    color: "#e0000000"
    radius: 6
    height: 26
    width: qualityLabel.implicitWidth + horizontalPadding
    border.color: "#44ffffff"
    border.width: 1
    clip: true

    Text {
        id: qualityLabel

        anchors.centerIn: parent
        text: "Preview: " + root.previewQuality + "  |  Fullscreen: " + root.fullscreenQuality
        color: "white"
        font.pixelSize: root.labelPixelSize
        font.bold: true
    }
}
