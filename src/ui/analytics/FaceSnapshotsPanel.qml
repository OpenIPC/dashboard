import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import OpenIPC

Item {
    id: root
    property var model // AnalyticsEngine
    
    // Get the snapshots directory from the engine
    property string snapshotsDir: {
        if (root.model) {
            var config = root.model.getModuleConfig(0); // 0 = FaceDetector
            // Handle if config is valid
            if (config && config.snapshotsDir) {
                return config.snapshotsDir;
            }
        }
        return "";
    }

    FolderListModel {
        id: folderModel
        folder: root.snapshotsDir ? "file:///" + root.snapshotsDir : ""
        nameFilters: ["*.jpg", "*.jpeg", "*.png"]
        showDirs: false
        sortField: FolderListModel.Time
        sortReversed: true // Newest first
    }
    
    GridView {
        id: gridView
        anchors.fill: parent
        anchors.margins: 20
        cellWidth: 220
        cellHeight: 260
        clip: true
        
        model: folderModel
        
        delegate: Item {
            width: 210
            height: 250
            
            Rectangle {
                anchors.fill: parent
                color: "#2d2d2d"
                radius: 8
                border.color: "#444"
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4
                    
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#000"
                        radius: 4
                        clip: true
                        
                        Image {
                            anchors.fill: parent
                            source: fileUrl
                            fillMode: Image.PreserveAspectFit
                            asynchronous: true
                            mipmap: true
                        }
                    }
                    
                    Text {
                        text: fileName
                        color: "white"
                        font.pixelSize: 11
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                    
                    Text {
                        text: Qt.formatDateTime(fileModified, "yyyy-MM-dd HH:mm:ss")
                        color: "#aaa"
                        font.pixelSize: 10
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
                
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onDoubleClicked: Qt.openUrlExternally(fileUrl)
                    
                    // Simple hover effect
                    onEntered: parent.border.color = "#3b82f6"
                    onExited: parent.border.color = "#444"
                }
            }
        }
        
        ScrollBar.vertical: ScrollBar { }
    }
    
    // Empty state
    Text {
        anchors.centerIn: parent
        text: qsTr("No snapshots found")
        color: "#666"
        visible: folderModel.count === 0 && root.snapshotsDir !== ""
        font.pixelSize: 16
    }
    
    Text {
        anchors.centerIn: parent
        text: qsTr("Snapshot directory not configured")
        color: "#666"
        visible: root.snapshotsDir === ""
        font.pixelSize: 16
    }

    Button {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        text: qsTr("Open Folder")
        visible: root.snapshotsDir !== ""
        onClicked: {
             Qt.openUrlExternally("file:///" + root.snapshotsDir)
        }
    }
}
