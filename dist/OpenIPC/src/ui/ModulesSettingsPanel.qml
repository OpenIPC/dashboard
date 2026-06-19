import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: "#1e1e1e"
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    readonly property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"

    function normalizePath(path) {
        if (!path)
            return ""
        var str = (typeof path === "string") ? path : path.toString()
        if (str.startsWith("file:///")) {
            str = str.substring(8)
        } else if (str.startsWith("file://")) {
            str = str.substring(7)
        }
        try {
            str = decodeURIComponent(str)
        } catch (e) {
            // keep original
        }
        return str
    }
    
    component StyledCheckBox: CheckBox {
        hoverEnabled: false
        background: Item {}
        indicator: Rectangle {
            implicitWidth: 18
            implicitHeight: 18
            x: parent.leftPadding
            y: parent.height / 2 - height / 2
            radius: 3
            color: "#252526"
            border.color: parent.checked ? "#3b82f6" : "#666666"
            
            Rectangle {
                width: 10
                height: 10
                anchors.centerIn: parent
                radius: 2
                color: "#3b82f6"
                visible: parent.parent.checked
            }
        }
        contentItem: Text {
            text: parent.text
            font: parent.font
            opacity: parent.enabled ? 1.0 : 0.5
            color: "white"
            verticalAlignment: Text.AlignVCenter
            leftPadding: parent.indicator.width + parent.spacing
        }
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
            color: "#1f2733"
            radius: 6
            border.color: "#334155"
            
            property int moduleType: model.type
            property bool isEnabled: SystemController.analyticsEngine.isModuleEnabled(moduleType)
            property string status: SystemController.analyticsEngine.getModuleStatus(moduleType)
            property real progress: SystemController.analyticsEngine.getModuleProgress(moduleType)
            property string errorMsg: SystemController.analyticsEngine.getModuleError(moduleType)
            
            // Configuration properties
            property var config: SystemController.analyticsEngine.getModuleConfig(moduleType)
            property string snapshotsDir: config ? (normalizePath(config.snapshotsDir || "Default directory")) : "Default directory"
            property string faceSnapshotsMode: config ? (config.faceSnapshotsMode || "standard") : "standard"
            property bool faceSnapshotKeyConfigured: config ? config.faceSnapshotKeyConfigured : false
            
            Connections {
                target: SystemController.analyticsEngine
                function onModuleStatusChanged(type, status, progress, error) {
                    if (type === moduleType) {
                        moduleDelegate.status = status
                        moduleDelegate.progress = progress
                        moduleDelegate.errorMsg = error
                        moduleDelegate.isEnabled = SystemController.analyticsEngine.isModuleEnabled(moduleType)
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
                                text: "v" + model.version
                                color: "#666"
                                font.pixelSize: 12
                            }
                        }
                        
                        Text {
                            text: I18n.t(model.description)
                            color: "#d8dee9"
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                        
                        Text {
                            text: I18n.t("Author: ") + model.author
                            color: "#b5c1d6"
                            font.pixelSize: 12
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
                                    color: moduleDelegate.status === "error" ? "#dc3545" : (moduleDelegate.status === "ready" ? "#28a745" : "#6c757d")
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
                            color: "#d8dee9"
                            font.pixelSize: 13
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                        }
                        
                        Button {
                            Layout.preferredHeight: 30
                            Layout.preferredWidth: 34
                            ToolTip.visible: hovered
                            ToolTip.text: I18n.t("Choose...")
                            background: Rectangle { color: "#4a5568"; radius: 4 }
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
                                color: parent.down ? "#334155" : "#1f2733"
                                border.color: "#475569"
                                border.width: 1
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#e2e8f0"
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
                            model: [I18n.t("Disabled"), I18n.t("Standard"), I18n.t("Anonymized"), I18n.t("Encrypted")]
                            currentIndex: {
                                switch(moduleDelegate.faceSnapshotsMode) {
                                    case "disabled": return 0;
                                    case "standard": return 1;
                                    case "anonymized": return 2;
                                    case "encrypted": return 3;
                                    default: return 1;
                                }
                            }
                            onActivated: {
                                var modes = ["disabled", "standard", "anonymized", "encrypted"]
                                SystemController.analyticsEngine.setModuleConfig(moduleType, { "faceSnapshotsMode": modes[index] })
                            }
                        }
                        
                        Text {
                            text: {
                                switch(moduleDelegate.faceSnapshotsMode) {
                                    case "disabled": return I18n.t("Face snapshots are not captured.");
                                    case "standard": return I18n.t("Faces are saved as-is without additional processing.");
                                    case "anonymized": return I18n.t("Snapshots are blurred before being stored.");
                                    case "encrypted": return I18n.t("Snapshots are encrypted with your key and stored as .bin files.");
                                    default: return "";
                                }
                            }
                            color: "#888"
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                        
                        Text {
                            visible: moduleDelegate.faceSnapshotsMode === "encrypted"
                            text: I18n.t("Encryption requires a 64-character hexadecimal key.")
                            color: "#f0ad4e"
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                    
                    // Encryption Key
                    ColumnLayout {
                        visible: moduleType === 0 // Face Detector
                        Layout.fillWidth: true
                        spacing: 4
                        
                        Text {
                            text: I18n.t("Snapshot encryption key")
                            color: "#b5c1d6"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            
                            TextField {
                                id: keyInput
                                Layout.fillWidth: true
                                placeholderText: I18n.t("64 hex characters")
                                maximumLength: 64
                                color: "white"
                                placeholderTextColor: "#94a3b8"
                                selectionColor: "#3b82f6"
                                selectedTextColor: "#ffffff"
                                background: Rectangle {
                                    color: "#1f2733"
                                    border.color: "#4a5568"
                                    border.width: 1
                                    radius: 4
                                }
                            }
                            
                            Button {
                                text: I18n.t("Save")
                                background: Rectangle {
                                    radius: 4
                                    color: parent.down ? "#2563eb" : "#3b82f6"
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    var key = keyInput.text.trim()
                                    if (key.length === 64 && /^[0-9a-fA-F]+$/.test(key)) {
                                        SystemController.analyticsEngine.setModuleConfig(moduleType, { "faceSnapshotKeyHex": key })
                                        keyInput.text = ""
                                    } else {
                                        // Show error (maybe a tooltip or text below)
                                        console.error("Invalid key format")
                                    }
                                }
                            }
                            
                            Button {
                                text: I18n.t("Reset key")
                                enabled: moduleDelegate.faceSnapshotKeyConfigured
                                background: Rectangle {
                                    radius: 4
                                    color: parent.enabled ? (parent.down ? "#334155" : "#1f2733") : "#374151"
                                    border.color: parent.enabled ? "#475569" : "#4b5563"
                                    border.width: 1
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: parent.enabled ? "#e2e8f0" : "#9ca3af"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    SystemController.analyticsEngine.setModuleConfig(moduleType, { "resetFaceSnapshotKey": true })
                                }
                            }
                        }
                        
                        Text {
                            text: moduleDelegate.faceSnapshotKeyConfigured 
                                ? I18n.t("Key configured. Saving a new key will replace it.") 
                                : I18n.t("No key configured. Provide one to enable encryption.")
                            color: moduleDelegate.faceSnapshotKeyConfigured ? "#28a745" : "#b5c1d6"
                            font.pixelSize: 11
                        }
                    }
                }
            }
        }
    }
}
