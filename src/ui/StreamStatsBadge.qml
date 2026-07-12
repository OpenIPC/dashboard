import QtQuick
import OpenIPC

Rectangle {
    id: root

    property var streamPlayer: null
    property real cellWidth: 0
    property real uiScale: 1.0
    property bool statsEnabled: true
    property int minWidth: 110
    property int horizontalPadding: 18
    property int labelPixelSize: 10
    property int badgeHeight: 30
    readonly property real constrainedMaxWidth: cellWidth > 0 && uiScale > 0 ? (cellWidth * 0.45) / uiScale : 100000

    function normalizedStatsCodec(codecValue) {
        var codec = String(codecValue || "").trim()
        var lower = codec.toLowerCase()
        if (codec === ""
                || lower.indexOf("metadata") >= 0
                || lower.indexOf("onvif") >= 0
                || lower.indexOf("application/") >= 0) {
            codec = "H264"
            lower = codec.toLowerCase()
        }

        if (lower.indexOf("265") >= 0 || lower.indexOf("hevc") >= 0 || lower.indexOf("hvc1") >= 0)
            return "H265"
        if (lower.indexOf("264") >= 0 || lower.indexOf("avc") >= 0)
            return "H264"
        if (lower.indexOf("mjpeg") >= 0 || lower.indexOf("motion jpeg") >= 0)
            return "MJPEG"
        if (lower.indexOf("jpeg") >= 0)
            return "JPEG"

        return codec.toUpperCase()
    }

    function statsTextFor(player) {
        if (!player || !player.running)
            return ""

        var w = player.videoWidth || 0
        var h = player.videoHeight || 0
        if (w === 0 || h === 0)
            return ""

        var parts = [
            normalizedStatsCodec(player.videoCodec),
            w + "x" + h
        ]
        var bitrate = player.videoBitrate || 0
        var fps = player.videoFps || 0

        if (bitrate > 0)
            parts.push(bitrate + " kbps")
        if (fps > 0)
            parts.push(fps + " FPS")

        return parts.join(", ")
    }

    function statsText() {
        return statsTextFor(streamPlayer)
    }

    color: "#e0000000"
    radius: Theme.radiusMd
    visible: statsEnabled && statsText() !== ""
    height: badgeHeight
    width: Math.min(Math.max(minWidth, statsLabel.implicitWidth + horizontalPadding), constrainedMaxWidth)
    border.color: Theme.overlayBorder
    border.width: 1
    clip: true

    Text {
        id: statsLabel

        anchors.centerIn: parent
        text: root.statsText()
        color: "white"
        font.pixelSize: root.labelPixelSize
        font.family: "monospace"
        font.bold: true
        elide: Text.ElideRight
        width: parent.width - root.horizontalPadding
        horizontalAlignment: Text.AlignHCenter
    }
}
