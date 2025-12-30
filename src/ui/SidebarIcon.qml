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
        anchors.fill: parent
        // Scale the shape to fit the item size (assuming 24x24 viewbox for paths)
        scale: root.width / 25
        anchors.centerIn: parent
        
        ShapePath {
            strokeWidth: 0
            fillColor: root.color
            PathSvg { path: root.path }
        }
    }
}
