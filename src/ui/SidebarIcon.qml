import QtQuick
import QtQuick.Shapes

Item {
    id: root

    width: 25
    height: 25

    // Prefer SVG path icons for release builds: they do not depend on a
    // platform font cache and therefore survive clean Windows machines better.
    // `name` remains supported for older call sites that still use ligatures.
    property string path: ""
    property string name: ""
    property string fallbackText: "•"
    property color color: "white"
    property int pixelSize: Math.max(12, Math.round(Math.min(root.width, root.height) * 0.82))

    readonly property bool hasSvgPath: root.path.length > 0 && root.path.charAt(0) === "M"
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

    Shape {
        id: vectorIcon

        visible: root.hasSvgPath
        width: 24
        height: 24
        anchors.centerIn: parent
        scale: Math.min(root.width, root.height) / 24
        transformOrigin: Item.Center
        antialiasing: true

        ShapePath {
            fillColor: root.color
            strokeColor: "transparent"
            strokeWidth: 0

            PathSvg {
                path: root.path
            }
        }
    }

    Text {
        visible: !root.hasSvgPath
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
