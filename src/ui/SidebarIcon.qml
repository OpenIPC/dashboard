import QtQuick
import QtQuick.Shapes

Item {
    id: root
    width: 25
    height: 25
    
    property string path: ""
    property color color: "white"
    
    layer.enabled: true
    layer.samples: 8
    
    Shape {
        width: 24
        height: 24
        anchors.centerIn: parent
        // Keep every path in the same centered 24x24 viewbox so sidebar tiles
        // do not visually jump when icons use slightly different geometry.
        scale: Math.min(root.width, root.height) / 24
        
        ShapePath {
            strokeWidth: 0
            fillColor: root.color
            PathSvg { path: root.path }
        }
    }
}
