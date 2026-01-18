import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC
import "analytics"

Dialog {
    id: root
    modal: true
    dim: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    // Center in parent
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: parent.width * 0.9
    height: parent.height * 0.9
    
    background: Rectangle {
        color: "#1e1e1e"
        radius: 8
        border.color: "#333"
        border.width: 1
    }
    
    header: Rectangle {
        height: 60
        color: "transparent"
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            
            Text {
                text: I18n.t("Аналитика")
                color: "white"
                font.pixelSize: 20
                font.bold: true
                Layout.fillWidth: true
            }
            
            Button {
                text: "✕"
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                background: Rectangle {
                    color: parent.hovered ? "#c42b1c" : "transparent"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.close()
            }
        }
        
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "#333"
        }
    }
    
    property string evidenceSnapshotsDir: ""
    property string faceSnapshotsDir: ""
    property string objectSnapshotsDir: ""
    property string plateSnapshotsDir: ""

    function refreshEvidenceDir() {
        var settings = SystemController.analyticsEngine.getSettings()
        if (settings && settings.evidence && settings.evidence.snapshotsDir) {
            evidenceSnapshotsDir = settings.evidence.snapshotsDir
        } else {
            evidenceSnapshotsDir = ""
        }
        refreshModuleDirs()
    }

    function moduleSnapshotsDir(type, fallbackFolder) {
        var cfg = SystemController.analyticsEngine.getModuleConfig(type)
        if (cfg && cfg.snapshotsDir) return cfg.snapshotsDir
        if (evidenceSnapshotsDir && evidenceSnapshotsDir !== "") {
            return evidenceSnapshotsDir + "/" + fallbackFolder
        }
        return ""
    }

    function refreshModuleDirs() {
        faceSnapshotsDir = moduleSnapshotsDir(0, "Face_Detector")
        objectSnapshotsDir = moduleSnapshotsDir(1, "Object_Counter")
        plateSnapshotsDir = moduleSnapshotsDir(2, "License_Plate")
    }

    Component.onCompleted: refreshEvidenceDir()

    Connections {
        target: SystemController.analyticsEngine
        function onSettingsChanged() { refreshEvidenceDir() }
        function onModuleConfigChanged(type) { refreshModuleDirs() }
    }
    
    contentItem: ColumnLayout {
        spacing: 0
        
        // Custom Tab Bar
        TabBar {
            id: bar
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            background: Rectangle { color: "#252526" }
            
            component CustomTabButton: TabButton {
                id: tabBtn
                width: implicitWidth + 40
                background: Rectangle {
                    color: tabBtn.checked ? "#1e1e1e" : "#2d2d2d"
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: tabBtn.checked ? "#3b82f6" : "transparent"
                    }
                }
                contentItem: Text {
                    text: tabBtn.text
                    color: tabBtn.checked ? "#3b82f6" : "#aaaaaa"
                    font.pixelSize: 14
                    font.bold: tabBtn.checked
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
            
            CustomTabButton { text: I18n.t("Лица") }
            CustomTabButton { text: I18n.t("Объекты") }
            CustomTabButton { text: I18n.t("Номера") }
        }
        
        // Content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: bar.currentIndex
            
            FaceSnapshotsPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: SystemController.analyticsEngine
                snapshotsDirOverride: faceSnapshotsDir
            }
            
            ObjectCounterPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: SystemController.analyticsEngine
                snapshotsDirOverride: objectSnapshotsDir
            }
            
            LicensePlatePanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: SystemController.analyticsEngine
                snapshotsDirOverride: plateSnapshotsDir
            }
        }
    }
}
