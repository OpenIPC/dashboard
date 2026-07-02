import QtQuick

Item {
    id: root

    width: 25
    height: 25

    // `path` is kept for backward compatibility with older call sites.
    // New sidebar tiles should pass Material Icons ligature names through
    // `name`, e.g. "search", "add", "folder", "settings".
    property string path: ""
    property string name: ""
    property string fallbackText: "•"
    property color color: "white"
    property int pixelSize: Math.max(12, Math.round(Math.min(root.width, root.height) * 0.82))

    readonly property string effectiveName: root.name.length > 0
                                          ? root.name
                                          : (root.path.length > 0 && root.path.charAt(0) !== "M"
                                             ? root.path
                                             : root.fallbackText)
    readonly property bool usingFallback: root.effectiveName === root.fallbackText
    readonly property bool iconFontReady: materialIcons.status === FontLoader.Ready

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    Text {
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        text: root.iconFontReady && !root.usingFallback ? root.effectiveName : root.fallbackText
        color: root.color
        font.family: root.iconFontReady && !root.usingFallback
                     ? materialIcons.name
                     : Qt.application.font.family
        font.pixelSize: root.usingFallback ? Math.round(root.pixelSize * 0.74) : root.pixelSize
        font.bold: root.usingFallback
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        renderType: Text.NativeRendering
        elide: Text.ElideRight
    }
}
