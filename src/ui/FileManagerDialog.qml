import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import OpenIPC

Window {
    id: root
    title: I18n.t("Файловый менеджер") + (cameraIp ? " - " + cameraIp : "")
    width: 1000
    height: 700
    visible: false
    color: Theme.metroBackground
    flags: Qt.Window | Qt.FramelessWindowHint

    property string cameraIp: ""
    property string cameraUser: "root"

    function open() {
        remoteModel.connectAndList(cameraIp, cameraUser, SystemController.getCameraPassword(cameraIp))
        show()
        requestActivate()
    }
    
    // 0 = Local, 1 = Remote
    property int activePane: 0

    // Custom Header
    Rectangle {
        id: titleBar
        height: 40
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: Theme.metroSurface
        z: 100

        MouseArea {
            anchors.fill: parent
            onPressed: root.startSystemMove()
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            anchors.rightMargin: 5
            
            Text {
                text: root.title
                color: "white"
                font.bold: true
                Layout.fillWidth: true
            }
            
            MetroWindowButton {
                kind: "minimize"
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.showMinimized()
            }
            
            MetroWindowButton {
                kind: "maximize"
                maximized: root.visibility === Window.Maximized
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: {
                    if (root.visibility === Window.Maximized) root.showNormal()
                    else root.showMaximized()
                }
            }

            MetroWindowButton {
                kind: "close"
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                onClicked: root.close()
            }
        }
    }

    RemoteFsModel {
        id: remoteModel
        onErrorOccurred: (err) => console.log("RemoteFS Error: " + err)
        onFileDownloaded: (name) => {
            console.log("Downloaded " + name)
            // localModel.folder = localModel.folder // refresh?
        }
        onFileUploaded: (name) => {
            console.log("Uploaded " + name)
            // remoteModel.refresh() // Autorefresh is built in uploadFile
        }
    }

    FolderListModel {
        id: localModel
        // Default to home or C:
        folder: "file:///C:/"
        showDirsFirst: true
        showDotAndDotDot: true
        
        onFolderChanged: {
            console.log("Local folder changed to: " + folder)
            // Fix text update losing binding
            localPathField.text = folder.toString().replace("file:///", "")
        }
        onStatusChanged: {
            if (status === FolderListModel.Error) {
                console.log("Local FS Error at " + folder)
                // Go up if possible? or reset
            }
        }
    }

    // Helper to format bytes
    function formatSize(bytes) {
        if (bytes === 0) return "0 B";
        var k = 1024;
        var sizes = ["B", "KB", "MB", "GB", "TB"];
        var i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + " " + sizes[i];
    }

    ColumnLayout {
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: footerBar.top
        anchors.margins: 10
        spacing: 10

        // Toolbar / Status
        RowLayout {
            visible: remoteModel.isLoading
            BusyIndicator {
                 running: true
                 palette.dark: "white" // Make it visible on dark bg
                 palette.light: "gray"
                 implicitHeight: 24
                 implicitWidth: 24
            }
            Label { text: "working..." }
        }

        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            // LOCAL PANE
            Rectangle {
                SplitView.preferredWidth: parent.width / 2
                SplitView.minimumWidth: 200
                color: Theme.metroSidebarBackground
                border.color: root.activePane === 0 ? Theme.metroBlue : Theme.metroStroke
                border.width: root.activePane === 0 ? 2 : 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 2 // Margin for border
                    spacing: 0
                    
                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        color: Theme.metroTile
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 4
                            Button {
                                id: localNavigateUpButton
                                text: ".."
                                background: Rectangle { color: localNavigateUpButton.down ? Theme.metroTilePressed : Theme.metroStroke; radius: 2 }
                                contentItem: Text { text: ".."; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                implicitWidth: 30
                                implicitHeight: 22
                                onClicked: {
                                    var current = localModel.folder.toString()
                                    // Stop at drive root (e.g. file:///C:/) on Windows
                                    var isRoot = false;
                                    if (Qt.platform.os === "windows") {
                                        // Check for file:///X:/ or file:///X:
                                        if (/^file:\/\/\/[a-zA-Z]:\/?$/.test(current)) isRoot = true;
                                    } else {
                                        if (current === "file:///") isRoot = true;
                                    }

                                    if (!isRoot && localModel.parentFolder)
                                        localModel.folder = localModel.parentFolder
                                }
                            }
                            TextField {
                                id: localPathField
                                text: localModel.folder.toString().replace("file:///", "")
                                color: Theme.textSecondary
                                background: Rectangle { color: Theme.metroStroke; radius: 2 }
                                Layout.fillWidth: true
                                selectByMouse: true
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 5
                                
                                onAccepted: {
                                    var path = text.trim()
                                    if (path.length > 0) {
                                        if (Qt.platform.os === "windows") {
                                            path = path.replace(/\\/g, "/")
                                            if (path.indexOf("file:///") !== 0) {
                                                // Check for drive letter X:/
                                                if (path.match(/^[a-zA-Z]:/)) {
                                                    path = "file:///" + path
                                                }
                                            }
                                        } else {
                                            if (path.indexOf("file://") !== 0) {
                                                if (path.charAt(0) === '/')
                                                     path = "file://" + path
                                                else
                                                     path = "file:///" + path
                                            }
                                        }
                                        localModel.folder = path
                                    }
                                    focus = false
                                }
                            }
                        }
                    }

                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 24
                        color: Theme.metroTile
                        z: 2
                        RowLayout {
                            anchors.fill: parent
                            spacing: 5
                            Text { text: " " + I18n.t("Имя"); color: Theme.textSecondary; font.bold: true; Layout.fillWidth: true; leftPadding: 5 }
                            Text { text: I18n.t("Размер"); color: Theme.textSecondary; font.bold: true; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight }
                            Text { text: I18n.t("Дата"); color: Theme.textSecondary; font.bold: true; Layout.preferredWidth: 120; horizontalAlignment: Text.AlignRight; rightPadding: 5 }
                        }
                    }

                    // List
                    ListView {
                        id: localView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: localModel
                        focus: true
                        boundsBehavior: Flickable.StopAtBounds
                        
                        highlight: Rectangle { color: root.activePane === 0 ? Theme.metroBlueHover : Theme.metroTilePressed }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0
                        
                        delegate: Rectangle {
                            id: localDelegate
                            width: localView.width
                            height: 24
                            color: "transparent"
                            
                            required property string fileName
                            required property bool fileIsDir
                            required property string fileSize
                            required property var fileUrl
                            required property var fileModified
                            required property int index
                            
                            MouseArea {
                                id: localMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                
                                onDoubleClicked: {
                                    if (localDelegate.fileIsDir) {
                                        // Force string conversion to avoid potential QUrl binding issues
                                        var urlStr = localDelegate.fileUrl.toString()
                                        if (urlStr.startsWith("file://")) {
                                             localModel.folder = urlStr
                                        } else {
                                             // Fallback for weird paths
                                             localModel.folder = "file:///" + urlStr
                                        }
                                    }
                                }
                                
                                onClicked: (mouse) => {
                                    root.activePane = 0
                                    localView.currentIndex = index
                                    localView.forceActiveFocus()
                                    if (mouse.button === Qt.RightButton) {
                                        localMenu.popup()
                                    }
                                }
                            }

                            // Hover rect
                            Rectangle {
                                anchors.fill: parent
                                color: localMouse.containsMouse && !localDelegate.ListView.isCurrentItem ? Theme.metroTileHover : "transparent"
                            }
                            
                            RowLayout {
                                anchors.fill: parent
                                spacing: 5
                                
                                // Icon + Name
                                Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 5
                                        spacing: 5
                                        
                                        Text {
                                            text: localDelegate.fileIsDir ? "📁" : "📄"
                                            color: Theme.textSecondary
                                        }
                                        Text {
                                            text: localDelegate.fileName
                                            color: "white"
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                                
                                Text {
                                    text: localDelegate.fileIsDir ? "<DIR>" : formatSize(localDelegate.fileSize)
                                    color: Theme.textSecondary
                                    font.pixelSize: 12
                                    Layout.preferredWidth: 80
                                    horizontalAlignment: Text.AlignRight
                                }
                                
                                Text {
                                    text: Qt.formatDateTime(localDelegate.fileModified, "dd.MM.yy hh:mm")
                                    color: Theme.textSecondary
                                    font.pixelSize: 12
                                    Layout.preferredWidth: 120
                                    horizontalAlignment: Text.AlignRight
                                    rightPadding: 5
                                }
                            }
                            
                            Menu {
                                id: localMenu
                                background: Rectangle {
                                    implicitWidth: 220
                                    implicitHeight: 40
                                    color: Theme.metroSurface
                                    border.color: Theme.metroStroke
                                    radius: Theme.metroTileRadius
                                }
                                MetroMenuItem {
                                    text: I18n.t("Загрузить на камеру")
                                    onTriggered: {
                                        var path = localDelegate.fileUrl.toString();
                                        if (Qt.platform.os === "windows") {
                                             path = path.replace("file:///", "");
                                        } else {
                                             path = path.replace("file://", "");
                                        }
                                        
                                        remoteModel.uploadFile(path)
                                    }
                                }
                            }
                        }
                        
                        ScrollBar.vertical: StyledScrollBar {}
                    }
                }
            }

            // REMOTE PANE
            Rectangle {
                SplitView.preferredWidth: parent.width / 2
                SplitView.minimumWidth: 200
                color: Theme.metroSidebarBackground
                border.color: root.activePane === 1 ? Theme.metroBlue : Theme.metroStroke
                border.width: root.activePane === 1 ? 2 : 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 2 // Margin for border
                    spacing: 0
                    
                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        color: Theme.metroTile // Header color
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 4
                            Button {
                                id: remoteNavigateUpButton
                                text: ".."
                                background: Rectangle { color: remoteNavigateUpButton.down ? Theme.metroTilePressed : Theme.metroStroke; radius: 2 }
                                contentItem: Text { text: ".."; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                implicitWidth: 30
                                implicitHeight: 22
                                onClicked: remoteModel.navigateUp()
                            }
                            Text {
                                text: remoteModel.currentPath
                                color: Theme.textSecondary
                                elide: Text.ElideMiddle
                                Layout.fillWidth: true
                            }
                            Button {
                                id: remoteRefreshButton
                                text: "⟳"
                                background: Rectangle { color: remoteRefreshButton.down ? Theme.metroTilePressed : Theme.metroStroke; radius: 2 }
                                contentItem: Text { text: "⟳"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                implicitWidth: 30
                                implicitHeight: 22
                                onClicked: remoteModel.refresh()
                            }
                        }
                    }

                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 24
                        color: Theme.metroTile
                        z: 2
                        RowLayout {
                            anchors.fill: parent
                            spacing: 5
                            Text { text: " " + I18n.t("Имя"); color: Theme.textSecondary; font.bold: true; Layout.fillWidth: true; leftPadding: 5 }
                            Text { text: I18n.t("Размер"); color: Theme.textSecondary; font.bold: true; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight }
                            Text { text: I18n.t("Дата"); color: Theme.textSecondary; font.bold: true; Layout.preferredWidth: 120; horizontalAlignment: Text.AlignRight }
                            Text { text: I18n.t("Права"); color: Theme.textSecondary; font.bold: true; Layout.preferredWidth: 80; horizontalAlignment: Text.AlignRight; rightPadding: 5 }
                        }
                    }

                    // List
                    ListView {
                        id: remoteView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: remoteModel
                        focus: true
                        boundsBehavior: Flickable.StopAtBounds
                        
                        highlight: Rectangle { color: root.activePane === 1 ? Theme.metroBlueHover : Theme.metroTilePressed }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0
                        
                        delegate: Rectangle {
                            id: remoteDelegate
                            width: remoteView.width
                            height: 24
                            color: "transparent"
                            
                            required property string fileName
                            required property bool isDir
                            required property var fileSize 
                            required property string filePermissions
                            required property string fileDate
                            required property int index
                            
                            MouseArea {
                                id: remoteMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                
                                onDoubleClicked: {
                                    if (remoteDelegate.isDir) {
                                        remoteModel.navigate(remoteDelegate.fileName)
                                    }
                                }
                                onClicked: (mouse) => {
                                    root.activePane = 1
                                    remoteView.currentIndex = index
                                    remoteView.forceActiveFocus()
                                    if (mouse.button === Qt.RightButton) {
                                        remoteMenu.popup()
                                    }
                                }
                            }

                            // Hover
                            Rectangle {
                                anchors.fill: parent
                                color: remoteMouse.containsMouse && !remoteDelegate.ListView.isCurrentItem ? Theme.metroTileHover : "transparent"
                            }
                            
                            RowLayout {
                                anchors.fill: parent
                                spacing: 5
                                
                                // Name
                                Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 5
                                        spacing: 5
                                        
                                        Text {
                                            text: remoteDelegate.isDir ? "📁" : "📄"
                                            color: Theme.textSecondary
                                        }
                                        Text {
                                            text: remoteDelegate.fileName
                                            color: "white"
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                                
                                Text {
                                    text: remoteDelegate.isDir ? "<DIR>" : formatSize(remoteDelegate.fileSize)
                                    color: Theme.textSecondary
                                    font.pixelSize: 12
                                    Layout.preferredWidth: 80
                                    horizontalAlignment: Text.AlignRight
                                }
                                
                                Text {
                                    text: remoteDelegate.fileDate
                                    color: Theme.textSecondary
                                    font.pixelSize: 12
                                    Layout.preferredWidth: 120
                                    horizontalAlignment: Text.AlignRight
                                }
                                
                                Text {
                                    text: remoteDelegate.filePermissions
                                    color: Theme.textSecondary
                                    font.pixelSize: 12
                                    Layout.preferredWidth: 80
                                    horizontalAlignment: Text.AlignRight
                                    rightPadding: 5
                                }
                            }
                            
                            Menu {
                                id: remoteMenu
                                background: Rectangle {
                                    implicitWidth: 260
                                    implicitHeight: 40
                                    color: Theme.metroSurface
                                    border.color: Theme.metroStroke
                                    radius: Theme.metroTileRadius
                                }
                                MetroMenuItem {
                                    text: I18n.t("Скачать (в текущую локальную папку)")
                                    onTriggered: {
                                         // Get local path
                                        var dest = localModel.folder.toString();
                                        if (Qt.platform.os === "windows") {
                                             dest = dest.replace("file:///", "");
                                        } else {
                                             dest = dest.replace("file://", "");
                                        }
                                        remoteModel.downloadFile(remoteDelegate.fileName, dest)
                                    }
                                }
                                MetroMenuItem {
                                    text: I18n.t("Удалить")
                                    onTriggered: remoteModel.deleteItem(remoteDelegate.fileName)
                                }
                            }
                        }
                        
                        ScrollBar.vertical: StyledScrollBar {}
                    }

                }
            }
        }
    }
    
    // Helper Actions
    function copyAction() {
        if (root.activePane === 0) {
            // Local -> Remote
            if (!localView.currentItem) return;
            var path = localView.currentItem.fileUrl.toString();
            if (Qt.platform.os === "windows") path = path.replace("file:///", "");
            else path = path.replace("file://", "");
            remoteModel.uploadFile(path);
        } else {
            // Remote -> Local
            if (!remoteView.currentItem) return;
            var dest = localModel.folder.toString();
            if (Qt.platform.os === "windows") dest = dest.replace("file:///", "");
            else dest = dest.replace("file://", "");
            remoteModel.downloadFile(remoteView.currentItem.fileName, dest);
        }
    }
    
    function deleteAction() {
        if (root.activePane === 1) { // Remote Only
             if (!remoteView.currentItem) return;
             var name = remoteView.currentItem.fileName;
             if (name !== ".." && name !== ".") {
                 deleteDialog.fileName = name
                 deleteDialog.open()
             }
        } else if (root.activePane === 0) { // Local
             if (!localView.currentItem) return;
             var name = localView.currentItem.fileName;
             var url = localView.currentItem.fileUrl.toString();
             // Skip drives e.g. "C:/" or root
             // Usually FolderListModel returns files, not drives unless in root?
             
             deleteDialog.fileName = name
             deleteDialog.fileUrl = url
             deleteDialog.open()
        }
    }

    function mkdirAction() {
         mkdirDialog.open()
    }
    
    function renameAction() {
        if (root.activePane === 1) { // Remote
             if (!remoteView.currentItem) return;
             var name = remoteView.currentItem.fileName;
             if (name !== ".." && name !== ".") {
                 renameDialog.oldName = name
                 renameDialog.open()
             }
        } else if (root.activePane === 0) { // Local
             if (!localView.currentItem) return;
             var name = localView.currentItem.fileName;
             renameDialog.oldName = name
             renameDialog.fileUrl = localView.currentItem.fileUrl.toString()
             renameDialog.open()
        }
    }

    Shortcut { sequence: "F5"; onActivated: copyAction() }
    Shortcut { sequence: "F7"; onActivated: mkdirAction() }
    Shortcut { sequence: "F8"; onActivated: deleteAction() }
    Shortcut { sequence: "Delete"; onActivated: deleteAction() }
    Shortcut { sequence: "F2"; onActivated: renameAction() }

    // Dialogs
    Dialog {
        id: deleteDialog
        title: I18n.t("Удаление")
        modal: true
        dim: true
        palette.window: Theme.metroSidebarBackground
        palette.windowText: "#ffffff"
        palette.button: "#3E3E42"
        palette.buttonText: "#ffffff"
        
        background: Rectangle {
            color: Theme.metroSidebarBackground
            border.color: Theme.metroStroke
            radius: 2
        }
        
        header: Label {
            text: deleteDialog.title
            color: Theme.textSecondary
            font.bold: true
            padding: 10
            background: Rectangle { color: Theme.metroTile; radius: 2 }
        }

        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent
        property string fileName: ""
        property string fileUrl: ""
        
        contentItem: ColumnLayout {
            spacing: 15
            Label { 
                text: I18n.t("Вы уверены, что хотите удалить '%1'?").arg(deleteDialog.fileName)
                color: Theme.textSecondary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
        onAccepted: {
            if (root.activePane === 1) {
                remoteModel.deleteItem(fileName)
            } else {
                remoteModel.localDeleteItem(fileUrl)
            }
        }
    }
    
    Dialog {
        id: mkdirDialog
        title: I18n.t("Создание папки")
        modal: true
        dim: true
        palette.window: Theme.metroSidebarBackground
        palette.windowText: "#ffffff"
        palette.button: "#3E3E42"
        palette.buttonText: "#ffffff"

        background: Rectangle {
            color: Theme.metroSidebarBackground
            border.color: Theme.metroStroke
            radius: 2
        }
        
        header: Label {
            text: mkdirDialog.title
            color: Theme.textSecondary
            font.bold: true
            padding: 10
            background: Rectangle { color: Theme.metroTile; radius: 2 }
        }

        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: parent
        
        contentItem: ColumnLayout {
            spacing: 15
            Label { text: I18n.t("Имя папки:"); color: Theme.textSecondary }
            TextField { 
                id: mkdirField
                color: "white"
                background: Rectangle { color: Theme.metroTile; border.color: Theme.metroTilePressed }
                Layout.fillWidth: true
                onAccepted: mkdirDialog.accept()
            }
        }
        onOpened: { mkdirField.text = ""; mkdirField.forceActiveFocus() }
        onAccepted: {
            if (root.activePane === 1) {
                remoteModel.createDirectory(mkdirField.text)
            } else {
                remoteModel.localCreateDirectory(localModel.folder.toString(), mkdirField.text)
            }
        }
    }

    Dialog {
        id: renameDialog
        title: I18n.t("Переименование")
        modal: true
        dim: true
        palette.window: Theme.metroSidebarBackground
        palette.windowText: "#ffffff"
        palette.button: "#3E3E42"
        palette.buttonText: "#ffffff"

        background: Rectangle {
            color: Theme.metroSidebarBackground
            border.color: Theme.metroStroke
            radius: 2
        }
        
        header: Label {
            text: renameDialog.title
            color: Theme.textSecondary
            font.bold: true
            padding: 10
            background: Rectangle { color: Theme.metroTile; radius: 2 }
        }

        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: parent
        property string oldName: ""
        property string fileUrl: ""

        contentItem: ColumnLayout {
            spacing: 15
            Label { text: I18n.t("Новое имя:"); color: Theme.textSecondary }
            TextField { 
                id: renameField
                text: renameDialog.oldName
                color: "white"
                background: Rectangle { color: Theme.metroTile; border.color: Theme.metroTilePressed }
                Layout.fillWidth: true
                onAccepted: renameDialog.accept()
            }
        }
        onOpened: { renameField.text = oldName; renameField.forceActiveFocus(); renameField.selectAll() }
        onAccepted: {
            if (root.activePane === 1) {
                remoteModel.renameItem(oldName, renameField.text)
            } else {
                remoteModel.localRenameItem(fileUrl, renameField.text)
            }
        }
    }

    // Bottom Toolbar
    ToolBar {
        id: footerBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        background: Rectangle { color: Theme.metroTile }
        RowLayout {
            anchors.fill: parent
            spacing: 1
            
            Repeater {
                model: [
                    { text: I18n.t("F2 Имя"), action: renameAction },
                    { text: I18n.t("F5 Копия"), action: copyAction },
                    { text: I18n.t("F7 Папка"), action: mkdirAction },
                    { text: I18n.t("F8 Удал."), action: deleteAction },
                    { text: I18n.t("Выход"), action: () => root.close() }
                ]
                Button {
                    id: functionBarButton
                    text: modelData.text
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    background: Rectangle {
                        color: functionBarButton.down ? Theme.metroBlue : Theme.metroStroke
                        border.color: "#222"
                    }
                    contentItem: Text {
                        text: functionBarButton.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.bold: true
                    }
                    onClicked: modelData.action()
                }
            }
        }
    }
}
