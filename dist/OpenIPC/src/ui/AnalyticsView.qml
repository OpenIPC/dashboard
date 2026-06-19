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
        color: Theme.panelAltBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorder
        border.width: 1
    }
    
    header: Rectangle {
        height: 56
        color: "transparent"
        
        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 14
            anchors.rightMargin: 54
            anchors.topMargin: 8
            anchors.bottomMargin: 8
            spacing: 0
            
            Text {
                text: I18n.t("Аналитика")
                color: Theme.textPrimary
                font.pixelSize: 18
                font.bold: true
            }

            Text {
                text: I18n.t("Просмотр и фильтрация")
                color: Theme.textMuted
                font.pixelSize: 11
            }
        }

        Button {
            text: "✕"
            width: 32
            height: 32
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 8
            anchors.rightMargin: 10
            background: Rectangle {
                color: parent.hovered ? "#c42b1c" : "transparent"
                radius: Theme.radiusSm
            }
            contentItem: Text {
                text: parent.text
                color: Theme.textPrimary
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: root.close()
        }
        
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Theme.panelBorder
        }
    }
    
    property string evidenceSnapshotsDir: ""
    property string evidenceClipsDir: ""
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
        if (settings && settings.evidence && settings.evidence.clipsDir) {
            evidenceClipsDir = settings.evidence.clipsDir
        } else {
            evidenceClipsDir = ""
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
            Layout.preferredHeight: 40
            background: Rectangle {
                color: Theme.panelBackground
                radius: Theme.radiusLg
                border.color: Theme.panelBorder
                border.width: 1
            }
            
            component CustomTabButton: TabButton {
                id: tabBtn
                width: implicitWidth + 28
                background: Rectangle {
                    color: tabBtn.checked ? Theme.panelAltBackground : Theme.cardBackground
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: tabBtn.checked ? Theme.accent : "transparent"
                    }
                }
                contentItem: Text {
                    text: tabBtn.text
                    color: tabBtn.checked ? Theme.accent : Theme.textMuted
                    font.pixelSize: 13
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
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.panelBackground
            radius: Theme.radiusLg
            border.color: Theme.panelBorder
            border.width: 1

            StackLayout {
                anchors.fill: parent
                anchors.margins: 8
                currentIndex: bar.currentIndex

                FaceSnapshotsPanel {
                    model: SystemController.analyticsEngine
                    snapshotsDirOverride: faceSnapshotsDir
                    clipsDirOverride: evidenceClipsDir
                }

                ObjectCounterPanel {
                    model: SystemController.analyticsEngine
                    snapshotsDirOverride: objectSnapshotsDir
                    clipsDirOverride: evidenceClipsDir
                }

                LicensePlatePanel {
                    model: SystemController.analyticsEngine
                    snapshotsDirOverride: plateSnapshotsDir
                    clipsDirOverride: evidenceClipsDir
                }
            }
        }
    }
}
