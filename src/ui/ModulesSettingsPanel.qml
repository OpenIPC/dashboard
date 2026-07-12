import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: Theme.metroBackground
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    readonly property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"

    function normalizePath(path) {
        if (!path)
            return ""
        return SystemController.normalizeLocalPath((typeof path === "string") ? path : path.toString())
    }
    
    component StyledCheckBox: MetroCheckBox {
    }

    // Enum mapping from C++
    readonly property int faceDetector: 0
    readonly property int objectCounter: 1
    readonly property int licensePlate: 2
    
    Component.onCompleted: {
        console.log("ModulesSettingsPanel loaded")
    }
    
    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: 20
        anchors.rightMargin: 4 // Scrollbar on the right edge
        spacing: 10
        clip: true

        ScrollBar.vertical: StyledScrollBar {}
        
        model: ListModel {
            id: modulesModel
            ListElement { type: 0; name: "Face Detector"; description: "Detects faces in video stream"; version: "0.0.1"; author: "Rinibr" }
            ListElement { type: 1; name: "Object Counter"; description: "Counts people and vehicles"; version: "0.0.1"; author: "Rinibr" }
            ListElement { type: 2; name: "License Plate"; description: "Recognizes license plates"; version: "0.0.1"; author: "Rinibr" }
        }
        
        delegate: Rectangle {
            id: moduleDelegate
            width: ListView.view.width
            height: contentLayout.implicitHeight + 32
            color: Theme.metroSurfaceAlt
            radius: 6
            border.color: Theme.metroStroke
            
            property int moduleType: model.type
            property bool isEnabled: SystemController.analyticsEngine.isModuleEnabled(moduleType)
            property string status: SystemController.analyticsEngine.getModuleStatus(moduleType)
            property real progress: SystemController.analyticsEngine.getModuleProgress(moduleType)
            property string errorMsg: SystemController.analyticsEngine.getModuleError(moduleType)
            property var diagnostics: SystemController.analyticsEngine.getModuleDiagnostics(moduleType)
            
            // Configuration properties
            property var config: SystemController.analyticsEngine.getModuleConfig(moduleType)
            property string snapshotsDir: config ? (normalizePath(config.snapshotsDir || "Default directory")) : "Default directory"
            property string faceSnapshotsMode: config ? (config.faceSnapshotsMode || "standard") : "standard"
            
            Connections {
                target: SystemController.analyticsEngine
                function onModuleStatusChanged(type, status, progress, error) {
                    if (type === moduleType) {
                        moduleDelegate.status = status
                        moduleDelegate.progress = progress
                        moduleDelegate.errorMsg = error
                        moduleDelegate.isEnabled = SystemController.analyticsEngine.isModuleEnabled(moduleType)
                        moduleDelegate.diagnostics = SystemController.analyticsEngine.getModuleDiagnostics(moduleType)
                    }
                }
                function onModuleConfigChanged(type) {
                    if (type === moduleType) {
                        moduleDelegate.config = SystemController.analyticsEngine.getModuleConfig(moduleType)
                    }
                }
            }
            
            ColumnLayout {
                id: contentLayout
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 16
                spacing: 16
                
                // Header Row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16
                    
                    StyledCheckBox {
                        checked: moduleDelegate.isEnabled
                        onToggled: {
                            SystemController.analyticsEngine.setModuleEnabled(moduleType, checked)
                            moduleDelegate.isEnabled = checked // Optimistic update
                        }
                    }
                    
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        
                        RowLayout {
                            spacing: 8
                            Text {
                                text: I18n.t(model.name)
                                color: "white"
                                font.pixelSize: 16
                                font.bold: true
                            }
                            Text {
                                text: "v" + (moduleDelegate.diagnostics.version || model.version)
                                color: Theme.textFaint
                                font.pixelSize: 12
                            }
                        }
                        
                        Text {
                            text: I18n.t(model.description)
                            color: Theme.textSecondary
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                        
                        Text {
                            text: I18n.t("Author: ") + model.author
                            color: "#b5c1d6"
                            font.pixelSize: 12
                        }

                        Text {
                            text: I18n.t("License: ") + (moduleDelegate.diagnostics.licenseId || "—")
                            color: Theme.textMuted
                            font.pixelSize: 11
                        }

                        Text {
                            text: moduleDelegate.diagnostics.sourceUrl || ""
                            color: Theme.metroBlueHover
                            font.pixelSize: 11
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                            visible: text !== ""

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Qt.openUrlExternally(parent.text)
                            }
                        }
                        
                        // Status / Progress
                        Item {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 20
                            visible: moduleDelegate.status === "downloading" || moduleDelegate.status === "error" || moduleDelegate.status === "ready"
                            
                            RowLayout {
                                anchors.fill: parent
                                spacing: 10
                                
                                ProgressBar {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 4
                                    from: 0
                                    to: 1
                                    value: moduleDelegate.progress
                                    visible: moduleDelegate.status === "downloading"
                                }
                                
                                Text {
                                    text: {
                                        if (moduleDelegate.status === "ready") return I18n.t("Ready")
                                        if (moduleDelegate.status === "downloading") return I18n.t("Downloading...")
                                        if (moduleDelegate.status === "error") return I18n.t("Error: ") + moduleDelegate.errorMsg
                                        return I18n.t("Disabled")
                                    }
                                    color: moduleDelegate.status === "error" ? Theme.metroRed : (moduleDelegate.status === "ready" ? Theme.metroGreen : Theme.textFaint)
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }
                
                // Configuration Section
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#374151"
                }
                
                // Snapshot Directory
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    
                    Text {
                        text: moduleType === 2 ? I18n.t("License plate snapshots") : (moduleType === 1 ? I18n.t("Object snapshots") : I18n.t("Snapshots directory"))
                        color: "#b5c1d6"
                        font.pixelSize: 12
                        font.bold: true
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Text {
                            text: moduleDelegate.snapshotsDir === "Default directory" ? I18n.t("Default directory") : moduleDelegate.snapshotsDir
                            color: Theme.textSecondary
                            font.pixelSize: 13
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                        }
                        
                        Button {
                            Layout.preferredHeight: 30
                            Layout.preferredWidth: 34
                            ToolTip.visible: hovered
                            ToolTip.text: I18n.t("Choose...")
                            background: Rectangle { color: Theme.metroStroke; radius: 4 }
                            contentItem: Text {
                                text: "folder_open"
                                font.family: iconFontFamily
                                font.pixelSize: 15
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: folderDialog.open()
                            
                            FolderDialog {
                                id: folderDialog
                                title: I18n.t("Choose snapshots directory")
                                onAccepted: {
                                    var path = normalizePath(folderDialog.selectedFolder)
                                    SystemController.analyticsEngine.setModuleConfig(moduleType, { "snapshotsDir": path })
                                }
                            }
                        }
                        
                        Button {
                            text: I18n.t("Use default")
                            background: Rectangle {
                                radius: 4
                                color: parent.down ? Theme.metroStroke : Theme.metroSurfaceAlt
                                border.color: Theme.metroStroke
                                border.width: 1
                            }
                            contentItem: Text {
                                text: parent.text
                                color: Theme.textSecondary
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 12
                            }
                            onClicked: {
                                SystemController.analyticsEngine.setModuleConfig(moduleType, { "snapshotsDir": "" })
                            }
                        }
                    }
                }
                
                // Face Detector Specific Settings
                ColumnLayout {
                    visible: moduleType === 0 // Face Detector
                    Layout.fillWidth: true
                    spacing: 16
                    
                    // Face Snapshot Mode
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        
                        Text {
                            text: I18n.t("Face snapshot mode")
                            color: "#b5c1d6"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        
                        StyledComboBox {
                            Layout.fillWidth: true
                            Layout.maximumWidth: 300
                            model: [I18n.t("Disabled"), I18n.t("Standard"), I18n.t("Anonymized")]
                            currentIndex: {
                                switch(moduleDelegate.faceSnapshotsMode) {
                                    case "disabled": return 0;
                                    case "standard": return 1;
                                    case "anonymized": return 2;
                                    default: return 1;
                                }
                            }
                            onUserSelected: {
                                var modes = ["disabled", "standard", "anonymized"]
                                SystemController.analyticsEngine.setModuleConfig(moduleType, { "faceSnapshotsMode": modes[index] })
                            }
                        }
                        
                        Text {
                            text: {
                                switch(moduleDelegate.faceSnapshotsMode) {
                                    case "disabled": return I18n.t("Face snapshots are not captured.");
                                    case "standard": return I18n.t("Faces are saved as-is without additional processing.");
                                    case "anonymized": return I18n.t("Snapshots are blurred before being stored.");
                                    default: return "";
                                }
                            }
                            color: Theme.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                        
                        Text {
                            visible: false
                            text: ""
                            color: Theme.metroAmber
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
