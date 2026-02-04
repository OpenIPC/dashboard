import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OpenIPC

Item {
    id: root
    
    // Grid layout state
    property int gridRows: SystemController.gridRows
    property int gridCols: SystemController.gridCols
    property int currentLayoutIndex: 0
    property int selectedPresetCells: 4
    property bool editingLayout: false
    property string layoutDialogTitle: I18n.t("Редактор шаблонов")
    property int cameraDataVersion: 0
    property bool isSidebarVisible: true

    // Permissions (live bindings)
    property int permToken: SystemController.userManager.permissionsVersion + (SystemController.userManager.isLoggedIn ? 1 : 0)
    function hasPerm(_) {
        var __ = permToken
        return true
    }
    property bool canLive: (permToken >= 0) && SystemController.userManager.canLiveView()
    property bool canPlayback: (permToken >= 0) && SystemController.userManager.canPlayback()
    property bool canPtz: (permToken >= 0) && SystemController.userManager.canPtz()
    property bool canExport: (permToken >= 0) && SystemController.userManager.canExport()
    property bool canSettings: (permToken >= 0) && SystemController.userManager.canSettings()
    property bool canUserManage: (permToken >= 0) && SystemController.userManager.canUserManage()

    function actionAllowed(action) {
        if (action === "search" || action === "add_folder" || action === "add_camera" || action === "settings") return canSettings
        if (action === "user") return canUserManage
        if (action === "analytics") return canLive
        return true
    }

    function showNoAccess(message) {
        noAccessDialog.message = message || I18n.t("У вас недостаточно прав для выполнения этого действия.")
        noAccessDialog.open()
    }

    function currentUsername() {
        var u = SystemController.userManager.currentUser
        if (u && u.username !== undefined) return u.username
        return ""
    }

    function isAdminUser() {
        var u = SystemController.userManager.currentUser
        if (u && u.username !== undefined && u.username === "admin") return true
        return SystemController.userManager.isAdmin()
    }
    
    // Propagate language changes if needed, though I18n is singleton
    property string appLanguage: I18n.language

    Connections {
        target: SystemController.cameraModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.cameraDataVersion++ }
        function onRowsRemoved(parent, first, last) { root.cameraDataVersion++ }
        function onModelReset() { root.cameraDataVersion++ }
        function onDataChanged(topLeft, bottomRight, roles) { root.cameraDataVersion++ }
    }

    function setGrid(r, c) {
        // Apply layout preset using ultra-fine grid (1200x1200 base)
        SystemController.applyLayoutPreset(Math.max(1, r), Math.max(1, c))
    }

    function presetDims(cells) {
        switch (cells) {
        case 1:
            return { rows: 1, cols: 1 }
        case 4:
            return { rows: 2, cols: 2 }
        case 9:
            return { rows: 3, cols: 3 }
        case 16:
            return { rows: 4, cols: 4 }
        case 32:
            return { rows: 4, cols: 8 } // favor wide grids for 32 cells
        case 64:
            return { rows: 6, cols: 11 } // 6x11 = 66 slots. Ratio 1.83 (close to 1.77)
        default:
            // Calculate rows/cols to maintain approx 16:9 aspect ratio for cells
            // c / r ~= 1.77  => c = 1.77 * r
            // c * r >= cells => 1.77 * r^2 >= cells => r = sqrt(cells / 1.77)
            var r = Math.ceil(Math.sqrt(cells / 1.77))
            var c = Math.ceil(cells / r)
            return { rows: r, cols: c }
        }
    }

    // Parallel array to store complex cell data that ListModel might drop
    property var layoutCells: []
    
    property var layoutPresets: [
        { id: "grid-1", label: "1", rows: 1, cols: 1, cells: [] },
        { id: "grid-4", label: "4", rows: 2, cols: 2, cells: [] },
        { id: "grid-9", label: "9", rows: 3, cols: 3, cells: [] },
        { id: "grid-16", label: "16", rows: 4, cols: 4, cells: [] },
        { id: "grid-32", label: "32", rows: 4, cols: 8, cells: [] },
        { id: "grid-64", label: "64", rows: 8, cols: 8, cells: [] },
        { 
            id: "complex-1+5", label: "1+5", rows: 3, cols: 3, 
            cells: [
                {rowSpan: 2, colSpan: 2}, 
                {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1},
                {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}
            ] 
        },
        { 
            id: "complex-1+7", label: "1+7", rows: 4, cols: 4, 
            cells: [
                {rowSpan: 3, colSpan: 3}, 
                {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1},
                {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}
            ] 
        },
        { 
            id: "complex-2+8", label: "2+8", rows: 4, cols: 4, 
            cells: [
                {rowSpan: 2, colSpan: 2}, {rowSpan: 2, colSpan: 2},
                {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1},
                {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}, {rowSpan: 1, colSpan: 1}
            ] 
        }
    ]
    
    property string selectedPresetId: "grid-4"

    function applyLayout(index) {
        if (index < 0 || index >= layoutModel.count)
            return
        currentLayoutIndex = index
        var item = layoutModel.get(index)
        
        // Construct full template object
        var template = {
            "rows": item.rows,
            "cols": item.cols
        }
        
        // Add cells if available in parallel array
        if (layoutCells[index] && layoutCells[index].length > 0) {
            template["cells"] = layoutCells[index]
        }
        
        SystemController.applyLayoutTemplate(template)
        selectedPresetCells = item.rows * item.cols
    }

    function closeLayout(index) {
        if (layoutModel.count <= 1)
            return
        layoutModel.remove(index)
        
        // Remove from parallel array
        var newCells = []
        for(var i=0; i<layoutCells.length; ++i) {
            if (i !== index) newCells.push(layoutCells[i])
        }
        layoutCells = newCells
        
        if (currentLayoutIndex >= layoutModel.count)
            currentLayoutIndex = layoutModel.count - 1
        applyLayout(currentLayoutIndex)
        syncLayoutsToBackend()
    }

    function loadEditorFromCells(rows, cols, savedCells) {
        var cells = []
        var covered = []
        for(var k=0; k<rows*cols; k++) covered.push(false)
        var cellIdx = 0
        
        for(var r=0; r<rows; r++) {
            for(var c=0; c<cols; c++) {
                var flatIdx = r * cols + c
                if (covered[flatIdx]) {
                    cells.push({r: r, c: c, spanR: 1, spanC: 1, visible: false})
                    continue
                }
                
                var spanR = 1
                var spanC = 1
                
                if (savedCells && cellIdx < savedCells.length) {
                    var sc = savedCells[cellIdx]
                    spanR = sc.rowSpan || 1
                    spanC = sc.colSpan || 1
                    cellIdx++
                }
                
                cells.push({r: r, c: c, spanR: spanR, spanC: spanC, visible: true})
                
                for(var rr=0; rr<spanR; rr++) {
                    for(var cc=0; cc<spanC; cc++) {
                        var targetR = r + rr
                        var targetC = c + cc
                        if (targetR < rows && targetC < cols) {
                            covered[targetR * cols + targetC] = true
                        }
                    }
                }
            }
        }
        
        layoutDialog.editorBaseRows = rows
        layoutDialog.editorBaseCols = cols
        layoutDialog.editorCells = cells
        layoutDialog.selectionStart = Qt.point(-1, -1)
        layoutDialog.selectionEnd = Qt.point(-1, -1)
    }

    function openLayoutEditor(editExisting) {
        editingLayout = editExisting
        layoutDialogTitle = editExisting ? I18n.t("Редактирование") : I18n.t("Новая раскладка")
        
        var initialRows = gridRows
        var initialCols = gridCols
        var initialName = I18n.t("Новая раскладка ") + (layoutModel.count + 1)
        var initialPreset = "grid-4"

        if (editExisting && currentLayoutIndex >= 0) {
            var target = layoutModel.get(currentLayoutIndex)
            initialName = target.name
            initialRows = target.rows
            initialCols = target.cols
            
            // Try to match existing layout to a preset
            var foundId = "custom"
            var targetCells = layoutCells[currentLayoutIndex] || []
            
            for (var i = 0; i < layoutPresets.length; i++) {
                var p = layoutPresets[i]
                if (p.rows === target.rows && p.cols === target.cols) {
                    // Check cells match
                    var pCells = p.cells || []
                    if (pCells.length === 0 && targetCells.length === 0) {
                        foundId = p.id
                        break
                    }
                    if (pCells.length > 0 && targetCells.length === pCells.length) {
                        foundId = p.id
                        break
                    }
                }
            }
            initialPreset = foundId
        }
        
        selectedPresetId = initialPreset
        layoutDialog.open()
        nameField.text = initialName
        
        if (initialPreset === "custom" && editExisting) {
             loadEditorFromCells(initialRows, initialCols, layoutCells[currentLayoutIndex] || [])
        } else {
             layoutDialog.initEditor(initialRows, initialCols)
        }
    }

    function saveLayoutTemplate() {
        // Only handles presets now. Custom layouts are handled in the Dialog's OK button.
        var r = 2, c = 2, cells = []
        
        var preset = null
        for(var i=0; i<layoutPresets.length; ++i) {
            if (layoutPresets[i].id === selectedPresetId) {
                preset = layoutPresets[i]
                break
            }
        }
        if (preset) {
            r = preset.rows
            c = preset.cols
            cells = preset.cells || []
        }
        
        var name = nameField.text.trim()
        if (name.length === 0)
            name = I18n.t("Новая раскладка ") + (layoutModel.count + 1)

        if (editingLayout && currentLayoutIndex >= 0) {
            layoutModel.setProperty(currentLayoutIndex, "name", name)
            layoutModel.setProperty(currentLayoutIndex, "rows", r)
            layoutModel.setProperty(currentLayoutIndex, "cols", c)
            layoutModel.setProperty(currentLayoutIndex, "isDefault", false)
            var newCells = layoutCells
            newCells[currentLayoutIndex] = cells
            layoutCells = newCells
            applyLayout(currentLayoutIndex)
        } else {
            layoutModel.append({ "name": name, "rows": r, "cols": c, "isDefault": false })
            var newCells = layoutCells
            newCells.push(cells)
            layoutCells = newCells
            applyLayout(layoutModel.count - 1)
        }
        syncLayoutsToBackend()
        layoutDialog.close()
    }

    Component.onCompleted: {
        // Load templates from SystemController
        layoutModel.clear()
        var templates = SystemController.layoutTemplates
        var cellsData = []
        for (var i = 0; i < templates.length; i++) {
            layoutModel.append(templates[i])
            // Store cells if present
            if (templates[i].cells) {
                cellsData.push(templates[i].cells)
            } else {
                cellsData.push([])
            }
        }
        layoutCells = cellsData
        
        // Try to match current grid size to a layout
        var found = false
        for (var i = 0; i < layoutModel.count; ++i) {
            var item = layoutModel.get(i)
            if (item.rows === gridRows && item.cols === gridCols) {
                currentLayoutIndex = i
                found = true
                break
            }
        }
        if (!found) {
            currentLayoutIndex = -1
        }
        
        // Ensure capacity
        var need = gridRows * gridCols
        if (SystemController.updateGridSize) {
            SystemController.updateGridSize(need)
        }
    }

    ListModel {
        id: layoutModel
        // Initial data loaded from SystemController
    }
    
    function syncLayoutsToBackend() {
        var list = []
        for (var i = 0; i < layoutModel.count; i++) {
            var item = layoutModel.get(i)
            var obj = {
                "name": item.name,
                "rows": item.rows,
                "cols": item.cols,
                "isDefault": item.isDefault
            }
            // Restore cells from parallel array
            if (layoutCells[i] && layoutCells[i].length > 0) {
                obj["cells"] = layoutCells[i]
            }
            list.push(obj)
        }
        SystemController.layoutTemplates = list
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ---------------------------------------------------------
        // TOP BAR
        // ---------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: "#252526"

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    if (Window.window) Window.window.startSystemMove()
                }
                onDoubleClicked: {
                    if (Window.window) {
                        if (Window.window.visibility === Window.Maximized)
                            Window.window.showNormal()
                        else
                            Window.window.showMaximized()
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: "#333333"
            }

            RowLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10
                anchors.bottomMargin: 8
                spacing: 8
                // Layout tabs
                RowLayout {
                    spacing: 6
                    Repeater {
                        model: layoutModel
                        delegate: Rectangle {
                            height: 32
                            radius: 6
                            color: index === currentLayoutIndex ? "#11151f" : "#2d3442"
                            border.color: index === currentLayoutIndex ? "#3b82f6" : "#3c4353"
                            border.width: 1
                            width: Math.max(96, nameText.implicitWidth + 30)
                            Text {
                                id: nameText
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 12
                                anchors.right: parent.right
                                anchors.rightMargin: 28
                                text: model.isDefault ? I18n.t(model.name) : model.name
                                color: "white"
                                font.pixelSize: 12
                                elide: Text.ElideRight
                                horizontalAlignment: Text.AlignLeft
                            }

                            Rectangle {
                                id: closeBtn
                                visible: layoutModel.count > 1
                                width: 20; height: 20; radius: 10
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.topMargin: 3
                                anchors.rightMargin: 4
                                color: "#3c4353"
                                border.color: "#4b556a"
                                z: 2
                                Text {
                                    anchors.centerIn: parent
                                    text: "×"
                                    color: "#b0b8c8"
                                    font.pixelSize: 11
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: (mouse) => { 
                                        mouse.accepted = true; 
                                        closeLayout(index);
                                        syncLayoutsToBackend();
                                    }
                                    onPressed: (mouse) => mouse.accepted = true
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: applyLayout(index)
                                onDoubleClicked: openLayoutEditor(true)
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Add layout
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 32
                    radius: 6
                    color: "#2d3442"
                    border.color: "#3c4353"
                    Text {
                        anchors.centerIn: parent
                        text: "+"
                        color: "white"
                        font.pixelSize: 16
                    }
                    ToolTip.visible: addHovered
                    ToolTip.text: I18n.t("Новая раскладка")
                    property bool addHovered: false
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: parent.addHovered = true
                        onExited: parent.addHovered = false
                        onClicked: openLayoutEditor(false)
                    }
                }

                // Edit template
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 32
                    radius: 6
                    color: "#2d3442"
                    border.color: "#3c4353"
                    Canvas {
                        anchors.centerIn: parent
                        width: 16; height: 16
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.fillStyle = "#b0b8c8";
                            var s = 2;
                            var w = width, h = height;
                            ctx.clearRect(0,0,w,h);
                            ctx.fillRect(0,0,s,s);
                            ctx.fillRect((w/2)-s/2,0,s,s);
                            ctx.fillRect(w-s,0,s,s);
                            ctx.fillRect(0,(h/2)-s/2,s,s);
                            ctx.fillRect((w/2)-s/2,(h/2)-s/2,s,s);
                            ctx.fillRect(w-s,(h/2)-s/2,s,s);
                            ctx.fillRect(0,h-s,s,s);
                            ctx.fillRect((w/2)-s/2,h-s,s,s);
                            ctx.fillRect(w-s,h-s,s,s);
                        }
                    }
                    ToolTip.visible: editHovered
                    ToolTip.text: I18n.t("Редактор шаблонов")
                    property bool editHovered: false
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: parent.editHovered = true
                        onExited: parent.editHovered = false
                        onClicked: openLayoutEditor(true)
                    }
                }


                // Toggle Sidebar
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 32
                    radius: 6
                    color: sideBg.hovered ? "#2d3442" : "transparent"
                    border.color: sideBg.hovered ? "#3c4353" : "transparent"
                    
                    Item { id: sideBg; property bool hovered: false }

                    Text {
                        anchors.centerIn: parent
                        text: isSidebarVisible ? "»" : "«" 
                        color: "white"
                        font.pixelSize: 18
                        rotation: 0
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                    ToolTip.visible: sideBg.hovered
                    ToolTip.text: isSidebarVisible ? I18n.t("Скрыть панель") : I18n.t("Показать панель")
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: sideBg.hovered = true
                        onExited: sideBg.hovered = false
                        onClicked: isSidebarVisible = !isSidebarVisible
                    }
                }

                Item { width: 260 } // Spacer to avoid overlap with window controls
            }

            // Window Controls
            RowLayout {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 6
                anchors.rightMargin: 10
                spacing: 4

                // Minimize
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 32
                    radius: 6
                    color: minHovered ? "#3e4654" : "#2d3442"
                    border.color: "#3c4353"
                    property bool minHovered: false
                    
                    Text {
                        anchors.centerIn: parent
                        text: "─"
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: parent.minHovered = true
                        onExited: parent.minHovered = false
                        onClicked: Window.window.showMinimized()
                    }
                }

                // Maximize/Restore
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 32
                    radius: 6
                    color: maxHovered ? "#3e4654" : "#2d3442"
                    border.color: "#3c4353"
                    property bool maxHovered: false
                    
                    Text {
                        anchors.centerIn: parent
                        text: Window.window.visibility === Window.Maximized ? "❐" : "☐"
                        color: "white"
                        font.pixelSize: 14
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: parent.maxHovered = true
                        onExited: parent.maxHovered = false
                        onClicked: {
                            if (Window.window.visibility === Window.Maximized)
                                Window.window.showNormal()
                            else
                                Window.window.showMaximized()
                        }
                    }
                }

                // Close
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 32
                    radius: 6
                    color: closeHovered ? "#c42b1c" : "#2d3442"
                    border.color: closeHovered ? "#c42b1c" : "#3c4353"
                    property bool closeHovered: false
                    
                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        color: "white"
                        font.pixelSize: 14
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: parent.closeHovered = true
                        onExited: parent.closeHovered = false
                        onClicked: Window.window.close()
                    }
                }
            }
        }

        // ---------------------------------------------------------
        // MAIN CONTENT (Grid + Sidebar)
        // ---------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // CONTENT AREA (Left)
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#1e1e1e"

                GridLayout {
                    id: cameraGrid
                    anchors.fill: parent
                    property int cellSpacing: 10
                    anchors.margins: cameraGrid.cellSpacing
                    columns: 1200 // Ultra-high resolution grid for pixel-perfect resizing
                    // rows: dynamic
                    rowSpacing: cameraGrid.cellSpacing
                    columnSpacing: cameraGrid.cellSpacing
                    flow: GridLayout.LeftToRight
                    
                    // Calculate unit sizes for precise resizing
                    // We use gridCols/gridRows for spacing calculation because GridLayout only adds spacing between actual items
                    property real unitWidth: (width - (Math.max(1, gridCols) - 1) * columnSpacing) / 1200
                    property real unitHeight: (height - (Math.max(1, gridRows) - 1) * rowSpacing) / 1200

                    Repeater {
                        model: SystemController.gridModel
                        delegate: GridCell {
                            // Strict sizing based on units
                            Layout.preferredWidth: Math.floor(Math.max(1, model.spanCols) * cameraGrid.unitWidth)
                            Layout.preferredHeight: Math.floor(Math.max(1, model.spanRows) * cameraGrid.unitHeight)
                            
                            // Normalize rowSpan to discrete grid rows to avoid creating thousands of rows
                            // spanRows is in 1200-units. We map it back to the logical grid row count.
                            // If gridRows is 0 or uninitialized, default to 2.
                            Layout.rowSpan: Math.max(1, Math.round((model.spanRows || 1) / (1200 / Math.max(1, gridRows || 2))))
                            
                            // columnSpan stays in 1200-units because GridLayout.columns is 1200
                            Layout.columnSpan: Math.max(1, model.spanCols || 1)
                            
                            // Pass grid info for resizing
                            gridParent: cameraGrid
                            totalRows: 1200 
                            totalCols: 1200
                            spanRows: model.spanRows || 1
                            spanCols: model.spanCols || 1
                            gridIndex: index
                            
                            // Pass unit sizes
                            unitWidth: cameraGrid.unitWidth
                            unitHeight: cameraGrid.unitHeight

                            cameraName: model.cameraName
                            cameraIp: model.cameraIp
                            cameraPort: model.cameraPort
                            cameraOnvifPort: model.cameraOnvifPort
                            cameraLogin: model.cameraLogin
                            cameraPassword: model.cameraPassword
                            streamUrl: model.streamUrl
                            sdStreamUrl: model.sdStreamUrl || model.streamUrl
                            hdStreamUrl: model.hdStreamUrl || model.streamUrl
                            status: model.status
                            isRecording: model.isRecording
                            manufacturer: model.manufacturer || ""

                            canLive: root.canLive
                            canPlayback: root.canPlayback
                            canPtz: root.canPtz
                            canExport: root.canExport
                            canSettings: root.canSettings

                            onPermissionDenied: root.showNoAccess()
                            
                            onCloseClicked: {
                                SystemController.removeCameraFromGrid(index)
                            }
                            
                            onEditRequested: {
                                if (!root.canSettings) {
                                    root.showNoAccess()
                                    return
                                }
                                var ip = model.cameraIp
                                var idx = SystemController.cameraModel.findIndexByIp(ip)
                                if (idx >= 0) {
                                    var cam = SystemController.cameraModel.getCamera(idx)
                                    addCameraDialog.isEditMode = true
                                    addCameraDialog.editIndex = idx
                                    addCameraDialog.initialName = cam.cameraName
                                    addCameraDialog.initialIp = cam.cameraIp
                                    addCameraDialog.initialPort = cam.cameraPort
                                    addCameraDialog.initialOnvifPort = cam.cameraOnvifPort
                                    addCameraDialog.initialLogin = cam.cameraLogin || "root"
                                    addCameraDialog.initialPassword = cam.cameraPassword || ""
                                    addCameraDialog.initialHdUrl = cam.hdStreamUrl || ""
                                    addCameraDialog.initialSdUrl = cam.sdStreamUrl || ""
                                    addCameraDialog.open()
                                }
                            }
                            
                            onDeleteRequested: {
                                if (!root.canSettings) {
                                    root.showNoAccess()
                                    return
                                }
                                var ip = model.cameraIp
                                var idx = SystemController.cameraModel.findIndexByIp(ip)
                                if (idx >= 0) {
                                    confirmDeleteDialog.cameraIndex = idx
                                    confirmDeleteDialog.open()
                                }
                            }
                            
                            onArchiveRequested: {
                                if (!root.canPlayback) {
                                    root.showNoAccess()
                                    return
                                }
                                archiveLoader.pendingCameraIp = model.cameraIp
                                archiveLoader.active = true
                            }
                            
                            // SystemController.toggleRecording is handled internally by GridCell
                        }
                    }
                }
            }

            // SIDEBAR (Right)
            Rectangle {
                Layout.preferredWidth: 300
                Layout.fillHeight: true
                visible: isSidebarVisible
                color: "#252526"
                
                ScrollView {
                    id: sidebarScrollView
                    anchors.fill: parent
                    clip: true
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    ColumnLayout {
                        id: sidebarContent
                        width: sidebarScrollView.availableWidth
                        spacing: 0

                        // Toolbar Icons
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            Layout.margins: 10
                            spacing: 10

                            Repeater {
                                model: [
                                    { iconPath: "M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z", action: "search", tooltip: "Поиск" },
                                    { iconPath: "M20 6h-8l-2-2H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2zm-1 8h-3v3h-2v-3h-3v-2h3V9h2v3h3v2z", action: "add_folder", tooltip: "Добавить группу" },
                                    { iconPath: "M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z", action: "add_camera", tooltip: "Добавить камеру" },
                                    { iconPath: "M19.87 18.73l-5.32-5.32C15.2 12.33 15.6 11.22 15.6 10c0-3.09-2.51-5.6-5.6-5.6S4.4 6.91 4.4 10s2.51 5.6 5.6 5.6c1.22 0 2.33-.4 3.41-1.05l5.32 5.32c.39.39 1.02.39 1.41 0l-.27-.27.27.27c.39-.39.39-1.02 0-1.41zM10 14.1c-2.26 0-4.1-1.84-4.1-4.1S7.74 5.9 10 5.9s4.1 1.84 4.1 4.1-1.84 4.1-4.1 4.1z", action: "analytics", tooltip: "Аналитика" },
                                    { iconPath: "M19.43 12.98c.04-.32.07-.64.07-.98 0-.34-.03-.66-.07-.98l2.11-1.65c.19-.15.24-.42.12-.64l-2-3.46c-.09-.16-.26-.25-.44-.25-.06 0-.12.01-.17.03l-2.49 1c-.52-.4-1.08-.73-1.69-.98l-.38-2.65C14.46 2.18 14.25 2 14 2h-4c-.25 0-.46.18-.49.42l-.38 2.65c-.61.25-1.17.59-1.69.98l-2.49-1c-.06-.02-.12-.03-.18-.03-.17 0-.34.09-.43.25l-2 3.46c-.13.22-.07.49.12.64l2.11 1.65c-.04.32-.07.65-.07.98 0 .33.03.66.07.98l-2.11 1.65c-.19.15-.24.42-.12.64l2 3.46c.09.16.26.25.44.25.06 0 .12-.01.17-.03l2.49-1c.52.4 1.08.73 1.69.98l.38 2.65c.03.24.24.42.49.42h4c.25 0 .46-.18.49-.42l.38-2.65c.61-.25 1.17-.59 1.69-.98l2.49 1c.06.02.12.03.18.03.17 0 .34-.09.43-.25l2-3.46c.13-.22.07-.49-.12-.64l-2.11-1.65zm-1.98-1.71c.04.31.05.52.05.73 0 .21-.02.43-.05.73l-.14 1.13.89.7 1.08.84-.7 1.21-1.27-.51-1.04-.42-.9.68c-.43.32-.84.56-1.25.73l-1.06.43-.16 1.13-.2 1.35h-1.4l-.19-1.35-.16-1.13-1.06-.43c-.43-.18-.83-.41-1.23-.71l-.91-.7-1.06.43-1.27.51-.7-1.21 1.08-.84.89-.7-.14-1.13c-.03-.31-.05-.54-.05-.74s.02-.43.05-.73l.14-1.13-.89-.7-1.08-.84.7-1.21 1.27.51 1.04.42.9-.68c.43-.32.84-.56 1.25-.73l1.06-.43.16-1.13.2-1.35h1.39l.19 1.35.16 1.13 1.06.43c.43.18.83.41 1.23.71l.91.7 1.06-.43 1.27-.51.7 1.21-1.07.85-.89.7.14 1.13zM12 8c-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4-1.79-4-4-4zm0 6c-1.1 0-2-.9-2-2s.9-2 2-2 2 .9 2 2-.9 2-2 2z", action: "settings", tooltip: "Настройки" },
                                    { iconPath: "M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm-5 14H7v-2h7v2zm3-4H7v-2h10v2zm0-4H7V7h10v2z", action: "logs", tooltip: "Логи" },
                                    { iconPath: "M10,4 A4,4 0 1,1 10,12 A4,4 0 1,1 10,4 M10.67,13.02C10.45,13.01,10.23,13,10,13c-2.42,0-4.68,0.67-6.61,1.82C2.51,15.34,2,16.32,2,17.35V20h9.26 C10.47,18.87,10,17.49,10,16C10,14.93,10.25,13.93,10.67,13.02z M20.75,16c0-0.22-0.03-0.42-0.06-0.63l1.14-1.01l-1-1.73l-1.45,0.49c-0.32-0.27-0.68-0.48-1.08-0.63L18,11h-2l-0.3,1.49 c-0.4,0.15-0.76,0.36-1.08,0.63l-1.45-0.49l-1,1.73l1.14,1.01c-0.03,0.21-0.06,0.41-0.06,0.63s0.03,0.42,0.06,0.63l-1.14,1.01 l1,1.73l1.45-0.49c0.32,0.27,0.68,0.48,1.08,0.63L16,21h2l0.3-1.49c0.4-0.15,0.76-0.36,1.08-0.63l1.45,0.49l1-1.73l-1.14-1.01 C20.72,16.42,20.75,16.22,20.75,16z M17,18c-1.1,0-2-0.9-2-2s0.9-2,2-2s2,0.9,2,2S18.1,18,17,18z", action: "user", tooltip: "Пользователь" },
                                    { iconPath: "M17 7l-1.41 1.41L18.17 11H8v2h10.17l-2.58 2.58L17 17l5-5zM4 5h8V3H4c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h8v-2H4V5z", action: "logout", tooltip: "Выход" }
                                ]

                                SidebarIcon {
                                    width: 20
                                    height: 20
                                    path: modelData.iconPath
                                    color: hovered ? "white" : "#aaaaaa"

                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onEntered: parent.hovered = true
                                        onExited: parent.hovered = false
                                        onClicked: {
                                            if (!actionAllowed(modelData.action)) {
                                                root.showNoAccess()
                                                return
                                            }
                                            if (modelData.action === "search") searchDialog.open()
                                            if (modelData.action === "add_folder") addGroupDialog.open()
                                            if (modelData.action === "add_camera") {
                                                addCameraDialog.isEditMode = false
                                                addCameraDialog.initialName = ""
                                                addCameraDialog.initialIp = ""
                                                addCameraDialog.initialPort = 554
                                                addCameraDialog.initialOnvifPort = 80
                                                addCameraDialog.initialLogin = "root"
                                                addCameraDialog.initialPassword = ""
                                                addCameraDialog.initialHdUrl = ""
                                                addCameraDialog.initialSdUrl = ""
                                                addCameraDialog.open()
                                            }
                                            if (modelData.action === "settings") settingsDialog.open()
                                            if (modelData.action === "analytics") analyticsDialog.open()
                                            if (modelData.action === "user") userManagementDialog.open()
                                            if (modelData.action === "logs") logView.open()
                                            if (modelData.action === "logout") SystemController.userManager.logout()

                                            console.log("Clicked: " + modelData.action)
                                        }
                                    }

                                    ToolTip.visible: hovered
                                    ToolTip.text: I18n.t(modelData.tooltip)
                                    ToolTip.delay: 500

                                    property bool hovered: false
                                }
                            }
                        }
                        // Current user label
                        Text {
                            Layout.fillWidth: true
                            Layout.leftMargin: 15
                            Layout.rightMargin: 15
                            Layout.bottomMargin: 6
                            text: I18n.t("Пользователь: ") + currentUsername()
                            color: "#a0aec0"
                            font.pixelSize: 12
                        }
                        Item { Layout.fillWidth: true } // Spacer

                        // Header "Устройства"
                        Text {
                            Layout.fillWidth: true
                            Layout.leftMargin: 15
                            Layout.rightMargin: 15
                            Layout.topMargin: 8
                            Layout.bottomMargin: 8
                            padding: 0
                            text: I18n.t("Устройства")
                            color: "white"
                            font.bold: true
                            font.pixelSize: 14
                        }

                        // Grouped device list with drag-to-group and group context menu
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignTop
                            spacing: 6

                        // Helper component for group block
                        Component {
                            id: groupBlock
                            Item {
                                Layout.fillWidth: true
                                implicitHeight: layout.implicitHeight
                                
                                property string groupName: modelData
                                readonly property bool isDefaultGroup: groupName === ""

                                property int groupCount: {
                                    var v = root.cameraDataVersion
                                    var count = 0
                                    for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
                                        var cam = SystemController.cameraModel.getCamera(i)
                                        var g = cam.cameraGroup || ""
                                        if (g === groupName) count++
                                    }
                                    return count
                                }

                                ColumnLayout {
                                    id: layout
                                    anchors.fill: parent
                                    spacing: 0

                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 32
                                        color: "#333333"
                                        radius: 4

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 10
                                            anchors.rightMargin: 8
                                            spacing: 8

                                            Text {
                                                text: isDefaultGroup ? I18n.t("Без группы") : groupName
                                                color: "#cccccc"
                                                font.pixelSize: 12
                                                elide: Text.ElideRight
                                                Layout.fillWidth: true
                                            }

                                            // Count indicator
                                            Rectangle {
                                                height: 18
                                                width: 30
                                                radius: 9
                                                color: "#444"
                                                border.color: "#555"
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: groupCount
                                                    color: "#ddd"
                                                    font.pixelSize: 11
                                                }
                                            }
                                        }

                                        // Right-click context for group management (except default)
                                        MouseArea {
                                            anchors.fill: parent
                                            acceptedButtons: Qt.RightButton
                                            hoverEnabled: true
                                            onClicked: (mouse) => {
                                                if (mouse.button === Qt.RightButton && !isDefaultGroup) {
                                                    groupContextMenu.targetGroup = groupName
                                                    groupContextMenu.popup()
                                                }
                                            }
                                        }
                                    }

                                    // Cameras inside this group
                                    ListView {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: contentHeight
                                        clip: true
                                        interactive: false
                                        spacing: 2
                                        model: SystemController.cameraModel

                                        delegate: Rectangle {
                                            property bool inGroup: (cameraGroup || "") === groupName
                                            width: ListView.view ? ListView.view.width : parent.width
                                            height: inGroup ? 50 : 0
                                            visible: inGroup
                                            color: "transparent"

                                            // Drag handle
                                            MouseArea {
                                                id: dragArea
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                acceptedButtons: Qt.LeftButton | Qt.RightButton

                                                onEntered: parent.color = "#2a2d2e"
                                                onExited: parent.color = "transparent"

                                                onClicked: (mouse) => {
                                                    if (mouse.button === Qt.RightButton) {
                                                        deviceContextMenu.cameraIp = cameraIp
                                                        deviceContextMenu.cameraName = cameraName
                                                        deviceContextMenu.cameraIndex = index
                                                        deviceContextMenu.popup()
                                                    }
                                                }
                                                onDoubleClicked: addToGrid()

                                                drag.target: dragProxy
                                                drag.axis: Drag.XAndYAxis
                                                drag.threshold: 10

                                                onPressed: (mouse) => {
                                                    if (mouse.button === Qt.LeftButton) {
                                                        var pos = mapToItem(root, mouse.x, mouse.y)
                                                        dragProxy.x = pos.x - dragProxy.width/2
                                                        dragProxy.y = pos.y - dragProxy.height/2
                                                        dragProxy.proxyIp = cameraIp
                                                        dragProxy.proxyName = cameraName || ""
                                                        dragProxy.dragIndex = index
                                                    }
                                                }
                                                
                                                onReleased: {
                                                    dragProxy.visible = false
                                                    dragProxy.x = 0
                                                    dragProxy.y = 0
                                                }
                                                
                                                drag.onActiveChanged: {
                                                    if (drag.active) {
                                                        dragProxy.visible = true
                                                    } else {
                                                        dragProxy.visible = false
                                                    }
                                                }

                                                function addToGrid() {
                                                    var slots = SystemController.gridCapacity()
                                                    var target = -1
                                                    for (var i = 0; i < slots; ++i) {
                                                        var camSlot = SystemController.gridModel.getCamera(i)
                                                        if (!camSlot.cameraIp || camSlot.cameraIp === "") {
                                                            target = i
                                                            break
                                                        }
                                                    }
                                                    if (target === -1) {
                                                        target = 0 // fallback overwrite first if full
                                                    }
                                                    SystemController.addCameraToGrid(index, target)
                                                }
                                            }

                                            RowLayout {
                                                anchors.fill: parent
                                                anchors.leftMargin: 15
                                                anchors.rightMargin: 10
                                                spacing: 10

                                                Rectangle {
                                                    width: 8; height: 8; radius: 4
                                                    color: status === "Online" ? "#4caf50" : "#f44336"
                                                }

                                                ColumnLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 2
                                                    Text {
                                                        text: I18n.t("Камера") + " " + cameraIp
                                                        color: "#cccccc"
                                                        font.pixelSize: 12
                                                    }
                                                    Text {
                                                        text: cameraIp
                                                        color: "#888888"
                                                        font.pixelSize: 11
                                                    }
                                                }

                                                Text {
                                                    text: "x"
                                                    color: "#888888"
                                                    font.pixelSize: 16
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        cursorShape: Qt.PointingHandCursor
                                                        onClicked: SystemController.removeDevice(index)
                                                    }
                                                }
                                            }
                                        }
                                    }

                                }

                                DropArea {
                                    anchors.fill: parent
                                    z: 1
                                    keys: ["camera"]
                                    onEntered: (drag) => {
                                        drag.accept(Qt.MoveAction)
                                    }
                                    onDropped: (drop) => {
                                        var idx = drop.mimeData.getData("application/camera-index")
                                        if (idx !== undefined && idx !== null) {
                                            SystemController.setCameraGroup(parseInt(idx), groupName)
                                            root.cameraDataVersion++
                                        }
                                    }
                                    
                                    Rectangle {
                                        anchors.fill: parent
                                        color: parent.containsDrag ? "#3d4450" : "transparent"
                                        opacity: 0.5
                                        radius: 4
                                        visible: parent.containsDrag
                                    }
                                }
                            }
                        }

                        // Default group + existing groups
                        Repeater {
                            model: [""] .concat(SystemController.cameraGroups)
                            delegate: groupBlock
                        }
                    }
                }
            }
        }
        }

        // ---------------------------------------------------------
        // BOTTOM STATUS BAR
        // ---------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 25
            color: "#1e1e1e" 

            Timer {
                interval: 1000
                running: true
                repeat: true
                onTriggered: {
                    var cpuVal = SystemController.processCpuPercent()
                    var ramVal = SystemController.processMemoryMB()
                    if (cpuVal === undefined || cpuVal < 0) cpuVal = 0
                    if (ramVal === undefined || ramVal < 0) ramVal = 0
                    cpuText.text = I18n.t("ЦП") + ": " + cpuVal.toFixed(1) + "%"
                    ramText.text = I18n.t("ОЗУ") + ": " + ramVal.toFixed(1) + " MB"
                    timeText.text = Qt.formatTime(new Date(), "hh:mm:ss")
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 15

                Text {
                    id: cpuText
                    text: I18n.t("ЦП") + ": 0%"
                    color: "white"
                    font.pixelSize: 11
                }
                Text {
                    id: ramText
                    text: I18n.t("ОЗУ") + ": 0 MB"
                    color: "white"
                    font.pixelSize: 11
                }

                Item { Layout.fillWidth: true }

                Text {
                    id: timeText
                    text: Qt.formatTime(new Date(), "hh:mm:ss")
                    color: "white"
                    font.pixelSize: 11
                }
            }
        }
    }

    Dialog {
        id: layoutDialog
        modal: true
        focus: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2 - 60
        padding: 0
        implicitWidth: 800
        implicitHeight: 600
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 10
            color: "#0f1219"
            border.color: "#1f2531"
        }
        
        // Editor State
        property int editorBaseRows: 6
        property int editorBaseCols: 6
        property var editorCells: [] // Array of {r, c, spanR, spanC, visible}
        property point selectionStart: Qt.point(-1, -1)
        property point selectionEnd: Qt.point(-1, -1)
        property bool isSelecting: false
        
        function initEditor(rows, cols) {
            editorBaseRows = rows
            editorBaseCols = cols
            var cells = []
            for(var r=0; r<rows; r++) {
                for(var c=0; c<cols; c++) {
                    cells.push({r: r, c: c, spanR: 1, spanC: 1, visible: true})
                }
            }
            editorCells = cells
            selectionStart = Qt.point(-1, -1)
            selectionEnd = Qt.point(-1, -1)
        }
        
        function getCellIndex(r, c) {
            return r * editorBaseCols + c
        }
        
        function unmergeSelection() {
            // Only works if a single merged cell is selected
            if (selectionStart.x < 0) return
            
            // Normalize selection to top-left
            var r = Math.min(selectionStart.y, selectionEnd.y)
            var c = Math.min(selectionStart.x, selectionEnd.x)
            
            var idx = getCellIndex(r, c)
            var cell = editorCells[idx]
            
            if (!cell.visible) return // Can't unmerge a hidden cell
            if (cell.spanR === 1 && cell.spanC === 1) return // Nothing to unmerge
            
            var newCells = editorCells.slice()
            
            // Restore all covered cells
            for(var rr=0; rr<cell.spanR; rr++) {
                for(var cc=0; cc<cell.spanC; cc++) {
                    var targetIdx = getCellIndex(r + rr, c + cc)
                    newCells[targetIdx].visible = true
                    newCells[targetIdx].spanR = 1
                    newCells[targetIdx].spanC = 1
                }
            }
            
            editorCells = newCells
            // Keep selection on the top-left cell
            selectionEnd = selectionStart
        }

        function mergeSelection() {
            if (selectionStart.x < 0) return
            
            var r1 = Math.min(selectionStart.y, selectionEnd.y)
            var r2 = Math.max(selectionStart.y, selectionEnd.y)
            var c1 = Math.min(selectionStart.x, selectionEnd.x)
            var c2 = Math.max(selectionStart.x, selectionEnd.x)
            
            var spanR = r2 - r1 + 1
            var spanC = c2 - c1 + 1
            
            if (spanR === 1 && spanC === 1) return // Nothing to merge
            
            var newCells = editorCells.slice() // Copy
            
            // First pass: Reset any cells that overlap the new selection
            // This handles "breaking" existing merges that are partially selected
            for(var i=0; i<newCells.length; i++) {
                var cell = newCells[i]
                if (!cell.visible) continue
                
                // Check overlap
                var cellR1 = cell.r
                var cellR2 = cell.r + cell.spanR - 1
                var cellC1 = cell.c
                var cellC2 = cell.c + cell.spanC - 1
                
                // Intersection
                var interR1 = Math.max(r1, cellR1)
                var interR2 = Math.min(r2, cellR2)
                var interC1 = Math.max(c1, cellC1)
                var interC2 = Math.min(c2, cellC2)
                
                if (interR1 <= interR2 && interC1 <= interC2) {
                    // Overlaps
                    // If it's partially outside, we reset it to 1x1s to avoid conflicts
                    if (cellR1 < r1 || cellR2 > r2 || cellC1 < c1 || cellC2 > c2) {
                        // Reset this merge.
                        for(var rr=cellR1; rr<=cellR2; rr++) {
                            for(var cc=cellC1; cc<=cellC2; cc++) {
                                var idx = getCellIndex(rr, cc)
                                newCells[idx].visible = true
                                newCells[idx].spanR = 1
                                newCells[idx].spanC = 1
                            }
                        }
                    }
                }
            }
            
            // Set top-left cell
            var mainIdx = getCellIndex(r1, c1)
            newCells[mainIdx].spanR = spanR
            newCells[mainIdx].spanC = spanC
            newCells[mainIdx].visible = true
            
            // Hide others
            for(var r=r1; r<=r2; r++) {
                for(var c=c1; c<=c2; c++) {
                    if (r === r1 && c === c1) continue
                    var idx = getCellIndex(r, c)
                    newCells[idx].visible = false
                    newCells[idx].spanR = 1
                    newCells[idx].spanC = 1
                }
            }
            
            editorCells = newCells
            selectionStart = Qt.point(-1, -1)
            selectionEnd = Qt.point(-1, -1)
        }
        
        function resetEditor() {
            initEditor(editorBaseRows, editorBaseCols)
        }
        
        onOpened: {
            // Initialize editor with default or current settings
            // Default to 8x5 to provide square-ish cells on 16:9 screens (Ratio 1.6)
            if (selectedPresetId === "custom") {
                 // Try to load from custom fields if set, else 8x5
                 var r = 5
                 var c = 8
                 if (customRowsField && customRowsField.text) r = parseInt(customRowsField.text) || 5
                 if (customColsField && customColsField.text) c = parseInt(customColsField.text) || 8
                 initEditor(r, c)
            } else {
                 initEditor(5, 8)
            }
        }

        function exportLayout() {
            // Convert editorCells to format for SystemController
            // We need to export visible cells in row-major order
            var cells = []
            for(var i=0; i<editorCells.length; i++) {
                if (editorCells[i].visible) {
                    cells.push({
                        rowSpan: editorCells[i].spanR,
                        colSpan: editorCells[i].spanC
                    })
                }
            }
            return cells
        }
        
        
        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 0
            spacing: 0

            // Header
            Rectangle {
                Layout.fillWidth: true
                height: 40
                color: "transparent"
                
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    spacing: 10
                    Text {
                        text: layoutDialogTitle.toUpperCase()
                        color: "#93a3c4"
                        font.pixelSize: 12
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        width: 26; height: 26; radius: 13
                        color: "#1f2531"
                        border.color: "#2b3344"
                        Text { anchors.centerIn: parent; text: "×"; color: "#b0b8c8"; font.pixelSize: 14 }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: layoutDialog.close() }
                    }
                }
                
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: "#1f2531"
                }
            }

            // Main Content (Split View)
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0
                
                // LEFT PANEL: Presets
                Rectangle {
                    Layout.preferredWidth: 200
                    Layout.fillHeight: true
                    color: "#131720"
                    
                    ListView {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5
                        clip: true
                        model: layoutPresets
                        delegate: Rectangle {
                            width: parent.width
                            height: 36
                            color: selectedPresetId === modelData.id ? "#2563eb" : "transparent"
                            radius: 4
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                spacing: 10
                                
                                // Icon
                                Rectangle {
                                    width: 20; height: 20
                                    color: "transparent"
                                    border.color: selectedPresetId === modelData.id ? "white" : "#4b556a"
                                    border.width: 1
                                    
                                    // Mini grid
                                    Grid {
                                        anchors.centerIn: parent
                                        columns: Math.min(modelData.cols, 3)
                                        rows: Math.min(modelData.rows, 3)
                                        spacing: 1
                                        Repeater {
                                            model: Math.min(modelData.cols * modelData.rows, 9)
                                            Rectangle {
                                                width: 4; height: 4
                                                color: selectedPresetId === modelData.id ? "white" : "#4b556a"
                                            }
                                        }
                                    }
                                }
                                
                                Text {
                                    text: modelData.type === "complex" ? modelData.label : I18n.t("%1 ячеек", [modelData.cells.length > 0 ? modelData.cells.length : (modelData.rows * modelData.cols)])
                                    color: selectedPresetId === modelData.id ? "white" : "#94a3b8"
                                    font.pixelSize: 13
                                }
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    selectedPresetId = modelData.id
                                    // If preset selected, update editor base to match preset for visualization?
                                    // Or keep editor independent? SmartPSS seems to have presets separate.
                                    // Let's switch to "Custom" mode if user interacts with editor.
                                }
                            }
                        }
                        
                        footer: Rectangle {
                            width: parent.width
                            height: 36
                            color: selectedPresetId === "custom" ? "#2563eb" : "transparent"
                            radius: 4
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                spacing: 10
                                Text { text: "?"; color: selectedPresetId === "custom" ? "white" : "#4b556a"; font.bold: true }
                                Text {
                                    text: I18n.t("Свой")
                                    color: selectedPresetId === "custom" ? "white" : "#94a3b8"
                                    font.pixelSize: 13
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: selectedPresetId = "custom"
                            }
                        }
                    }
                    
                    Rectangle {
                        anchors.right: parent.right
                        width: 1
                        height: parent.height
                        color: "#1f2531"
                    }
                }
                
                // RIGHT PANEL: Editor
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 15
                        
                        // Controls
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 15
                            
                            Text { text: I18n.t("Ряд"); color: "#94a3b8" }
                            TextField {
                                id: customRowsField
                                text: layoutDialog.editorBaseRows
                                color: "white"
                                Layout.preferredWidth: 60
                                background: Rectangle { color: "#121724"; border.color: "#243047"; radius: 4 }
                                onEditingFinished: {
                                    var val = parseInt(text)
                                    if (val > 0 && val <= 12) layoutDialog.initEditor(val, layoutDialog.editorBaseCols)
                                }
                            }
                            
                            Text { text: I18n.t("Столбцов"); color: "#94a3b8" }
                            TextField {
                                id: customColsField
                                text: layoutDialog.editorBaseCols
                                color: "white"
                                Layout.preferredWidth: 60
                                background: Rectangle { color: "#121724"; border.color: "#243047"; radius: 4 }
                                onEditingFinished: {
                                    var val = parseInt(text)
                                    if (val > 0 && val <= 12) layoutDialog.initEditor(layoutDialog.editorBaseRows, val)
                                }
                            }
                            
                            // Aspect Ratio Hint
                            Text {
                                property real ratio: (layoutDialog.editorBaseCols / layoutDialog.editorBaseRows) / (16/9)
                                text: "AR: " + ratio.toFixed(2) + " (1.0 = 16:9)"
                                color: (ratio > 0.8 && ratio < 1.2) ? "#4ade80" : "#f87171"
                                font.pixelSize: 11
                                visible: true
                            }
                            
                            Item { Layout.fillWidth: true }
                            
                            Button {
                                text: I18n.t("Сброс")
                                background: Rectangle { color: "#2d3442"; radius: 4 }
                                contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                onClicked: layoutDialog.resetEditor()
                            }
                            
                            Button {
                                text: I18n.t("Разбить")
                                // Enabled if single cell selected AND it is merged (span > 1)
                                enabled: {
                                    if (layoutDialog.selectionStart.x < 0) return false
                                    if (layoutDialog.selectionStart.x !== layoutDialog.selectionEnd.x || layoutDialog.selectionStart.y !== layoutDialog.selectionEnd.y) return false
                                    var idx = layoutDialog.getCellIndex(layoutDialog.selectionStart.y, layoutDialog.selectionStart.x)
                                    var cell = layoutDialog.editorCells[idx]
                                    return cell && (cell.spanR > 1 || cell.spanC > 1)
                                }
                                background: Rectangle { color: parent.enabled ? "#2d3442" : "#1f2531"; radius: 4; border.color: parent.enabled ? "#4b556a" : "transparent" }
                                contentItem: Text { text: parent.text; color: parent.enabled ? "white" : "#444"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                onClicked: layoutDialog.unmergeSelection()
                            }

                            Button {
                                text: I18n.t("Объединить")
                                enabled: layoutDialog.selectionStart.x >= 0
                                background: Rectangle { color: parent.enabled ? "#2563eb" : "#2d3442"; radius: 4 }
                                contentItem: Text { text: parent.text; color: parent.enabled ? "white" : "#666"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                onClicked: layoutDialog.mergeSelection()
                            }
                        }
                        
                        // Editor Canvas
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: "#0f1219"
                            border.color: "#2b3344"
                            
                            // Grid Container
                            Item {
                                id: gridContainer
                                anchors.centerIn: parent
                                width: Math.min(parent.width - 40, (parent.height - 40) * (layoutDialog.editorBaseCols / layoutDialog.editorBaseRows))
                                height: width * (layoutDialog.editorBaseRows / layoutDialog.editorBaseCols)
                                
                                property real cellW: width / layoutDialog.editorBaseCols
                                property real cellH: height / layoutDialog.editorBaseRows
                                
                                // Cells
                                Repeater {
                                    model: layoutDialog.editorCells
                                    delegate: Rectangle {
                                        visible: modelData.visible
                                        x: modelData.c * gridContainer.cellW
                                        y: modelData.r * gridContainer.cellH
                                        width: modelData.spanC * gridContainer.cellW
                                        height: modelData.spanR * gridContainer.cellH
                                        color: "#1f2937" // Darker base
                                        border.color: "#374151"
                                        border.width: 1
                                        
                                        // Inner "Screen" look
                                        Rectangle {
                                            anchors.fill: parent
                                            anchors.margins: 2
                                            color: "#111827"
                                            border.color: "#1f2937"
                                            
                                            // Cell Index
                                            Text {
                                                anchors.centerIn: parent
                                                text: (index + 1)
                                                color: "#374151"
                                                font.pixelSize: Math.min(parent.width, parent.height) * 0.4
                                                font.bold: true
                                            }
                                            
                                            // Merged Indicator
                                            Rectangle {
                                                visible: modelData.spanR > 1 || modelData.spanC > 1
                                                anchors.fill: parent
                                                color: "transparent"
                                                border.color: "#3b82f6"
                                                border.width: 2
                                                opacity: 0.5
                                            }
                                        }
                                    }
                                }
                                
                                // Selection Overlay
                                Rectangle {
                                    visible: layoutDialog.selectionStart.x >= 0
                                    x: Math.min(layoutDialog.selectionStart.x, layoutDialog.selectionEnd.x) * gridContainer.cellW
                                    y: Math.min(layoutDialog.selectionStart.y, layoutDialog.selectionEnd.y) * gridContainer.cellH
                                    width: (Math.abs(layoutDialog.selectionEnd.x - layoutDialog.selectionStart.x) + 1) * gridContainer.cellW
                                    height: (Math.abs(layoutDialog.selectionEnd.y - layoutDialog.selectionStart.y) + 1) * gridContainer.cellH
                                    color: "#2563eb"
                                    opacity: 0.3
                                    border.color: "#60a5fa"
                                    border.width: 2
                                }
                                
                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    
                                    function getGridPos(mouse) {
                                        var c = Math.floor(mouse.x / gridContainer.cellW)
                                        var r = Math.floor(mouse.y / gridContainer.cellH)
                                        c = Math.max(0, Math.min(c, layoutDialog.editorBaseCols - 1))
                                        r = Math.max(0, Math.min(r, layoutDialog.editorBaseRows - 1))
                                        return Qt.point(c, r)
                                    }
                                    
                                    onPressed: (mouse) => {
                                        selectedPresetId = "custom" // Switch to custom mode
                                        var pos = getGridPos(mouse)
                                        layoutDialog.selectionStart = pos
                                        layoutDialog.selectionEnd = pos
                                        layoutDialog.isSelecting = true
                                    }
                                    
                                    onPositionChanged: (mouse) => {
                                        if (layoutDialog.isSelecting) {
                                            var pos = getGridPos(mouse)
                                            layoutDialog.selectionEnd = pos
                                        }
                                    }
                                    
                                    onReleased: {
                                        layoutDialog.isSelecting = false
                                    }
                                }
                            }
                        }
                        
                        // Name Field
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: I18n.t("Имя:"); color: "#94a3b8" }
                            TextField {
                                id: nameField
                                Layout.fillWidth: true
                                placeholderText: I18n.t("Название раскладки")
                                color: "white"
                                background: Rectangle { color: "#121724"; border.color: "#243047"; radius: 4 }
                            }
                        }
                    }
                }
            }

            // Footer
            Rectangle {
                Layout.fillWidth: true
                height: 60
                color: "transparent"
                
                Rectangle {
                    anchors.top: parent.top
                    width: parent.width
                    height: 1
                    color: "#1f2531"
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 10
                    Item { Layout.fillWidth: true }
                    
                    Button {
                        text: I18n.t("Отмена")
                        background: Rectangle { color: "#2d3442"; radius: 4 }
                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        onClicked: layoutDialog.close()
                    }
                    
                    Button {
                        text: I18n.t("OK")
                        background: Rectangle { color: "#2563eb"; radius: 4 }
                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        onClicked: {
                            // If custom, use editor cells. If preset, use preset.
                            if (selectedPresetId === "custom") {
                                var cells = layoutDialog.exportLayout()
                                var r = layoutDialog.editorBaseRows
                                var c = layoutDialog.editorBaseCols
                                
                                var name = nameField.text.trim()
                                if (name.length === 0) name = I18n.t("Польз. план")
                                
                                if (editingLayout && currentLayoutIndex >= 0) {
                                    layoutModel.setProperty(currentLayoutIndex, "name", name)
                                    layoutModel.setProperty(currentLayoutIndex, "rows", r)
                                    layoutModel.setProperty(currentLayoutIndex, "cols", c)
                                    layoutModel.setProperty(currentLayoutIndex, "isDefault", false)
                                    var newCells = layoutCells
                                    newCells[currentLayoutIndex] = cells
                                    layoutCells = newCells
                                    applyLayout(currentLayoutIndex)
                                } else {
                                    layoutModel.append({ "name": name, "rows": r, "cols": c, "isDefault": false })
                                    var newCells = layoutCells
                                    newCells.push(cells)
                                    layoutCells = newCells
                                    applyLayout(layoutModel.count - 1)
                                }
                            } else {
                                saveLayoutTemplate() // Use existing logic for presets
                            }
                            syncLayoutsToBackend()
                            layoutDialog.close()
                        }
                    }
                }
            }
        }
    }

    SettingsDialog {
        id: settingsDialog
        language: appLanguage
        onLanguageChanged: {
            appLanguage = language
            I18n.language = language
            layoutDialogTitle = editingLayout ? I18n.t("Редактирование") : I18n.t("Редактор шаблонов")
        }
    }
    
    CameraSearchDialog {
        id: searchDialog
        onAddCameraRequested: (name, ip, port, onvifPort) => {
            addCameraDialog.isEditMode = false
            addCameraDialog.initialName = name
            addCameraDialog.initialIp = ip
            addCameraDialog.initialPort = port
            addCameraDialog.initialOnvifPort = onvifPort
            addCameraDialog.initialLogin = "root"
            addCameraDialog.initialPassword = ""
            addCameraDialog.initialHdUrl = ""
            addCameraDialog.initialSdUrl = ""
            addCameraDialog.open()
        }
    }
    
    AddCameraDialog {
        id: addCameraDialog
    }
    
    AddGroupDialog {
        id: addGroupDialog
    }
    
    AnalyticsView {
        id: analyticsDialog
    }

    UserManagementDialog {
        id: userManagementDialog
    }

    LogView {
        id: logView
    }

    Dialog {
        id: noAccessDialog
        modal: true
        title: I18n.t("Недостаточно прав")
        width: 420
        property string message: I18n.t("У вас недостаточно прав для выполнения этого действия.")
        parent: Overlay.overlay
        x: parent ? (parent.width - width) / 2 : 0
        y: parent ? (parent.height - height) / 2 : 0
        background: Rectangle {
            color: "#2a2f33"
            border.color: "#3c3c3c"
            radius: 6
        }
        header: Rectangle {
            height: 36
            color: "#252526"
            radius: 6
            border.color: "#3c3c3c"
            Text {
                anchors.centerIn: parent
                text: noAccessDialog.title
                color: "white"
                font.bold: true
                font.pixelSize: 14
            }
        }
        contentItem: Rectangle {
            anchors.fill: parent
            color: "transparent"
            clip: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12
                Text {
                    text: noAccessDialog.message
                    color: "#cbd5e1"
                    wrapMode: Text.WordWrap
                }
                Item { Layout.fillHeight: true }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.rightMargin: 24
                    Layout.bottomMargin: 12
                    Item { Layout.fillWidth: true }
                    Button {
                        text: I18n.t("ОК")
                        Layout.preferredWidth: 72
                        Layout.preferredHeight: 28
                        background: Rectangle { color: "#3b82f6"; radius: 4 }
                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        onClicked: noAccessDialog.close()
                    }
                }
            }
        }
    }

    Loader {
        id: archiveLoader
        active: false
        source: "ArchiveView.qml"
        property string pendingCameraIp: ""

        onStatusChanged: {
            if (status === Loader.Ready) {
                item.currentCameraIp = pendingCameraIp
                SystemController.isArchiveOpen = true
                item.open()
                item.closed.connect(function() {
                    active = false
                    SystemController.isArchiveOpen = false
                })
            }
        }
    }

    CameraContextMenu {
        id: deviceContextMenu
        canLive: root.canLive
        canPlayback: root.canPlayback
        canSettings: root.canSettings
        canExport: root.canExport
        onDeleteRequested: {
            if (!root.canSettings) {
                root.showNoAccess()
                return
            }
            confirmDeleteDialog.cameraIndex = cameraIndex
            confirmDeleteDialog.open()
        }
        onEditRequested: {
            if (!root.canSettings) {
                root.showNoAccess()
                return
            }
            var cam = SystemController.cameraModel.getCamera(cameraIndex)
            addCameraDialog.isEditMode = true
            addCameraDialog.editIndex = cameraIndex
            addCameraDialog.initialName = cam.cameraName
            addCameraDialog.initialIp = cam.cameraIp
            addCameraDialog.initialPort = cam.cameraPort
            addCameraDialog.initialOnvifPort = cam.cameraOnvifPort
            addCameraDialog.initialLogin = cam.cameraLogin || "root"
            addCameraDialog.initialPassword = cam.cameraPassword || ""
            addCameraDialog.initialHdUrl = cam.hdStreamUrl || ""
            addCameraDialog.initialSdUrl = cam.sdStreamUrl || ""
            addCameraDialog.open()
        }
        onGroupChanged: root.cameraDataVersion++
        onSshRequested: {
            if (!root.canSettings) {
                root.showNoAccess()
                return
            }
            // Find camera credentials
            var cam = SystemController.cameraModel.getCamera(cameraIndex)
            sshDialog.cameraIp = cam.cameraIp
            sshDialog.cameraUser = cam.cameraLogin || "root"
            sshDialog.cameraPassword = cam.cameraPassword || ""
            sshDialog.open()
        }
        onArchiveRequested: {
            if (!root.canPlayback) {
                root.showNoAccess()
                return
            }
            var cam = SystemController.cameraModel.getCamera(cameraIndex)
            archiveLoader.pendingCameraIp = cam.cameraIp
            archiveLoader.active = true
        }
        onFileManagerRequested: {
            if (!root.canExport) {
                root.showNoAccess()
                return
            }
            var cam = SystemController.cameraModel.getCamera(cameraIndex)
            fileManagerDialog.cameraIp = cam.cameraIp
            fileManagerDialog.cameraUser = cam.cameraLogin || "root"
            fileManagerDialog.cameraPassword = cam.cameraPassword || ""
            fileManagerDialog.open()
        }
        onPermissionDenied: root.showNoAccess()
    }

    // Group context menu
    Menu {
        id: groupContextMenu
        property string targetGroup: ""

        MenuItem {
            text: I18n.t("Переименовать группу")
            enabled: root.canSettings
            onTriggered: {
                renameGroupDialog.oldName = groupContextMenu.targetGroup
                renameGroupDialog.newName = groupContextMenu.targetGroup
                renameGroupDialog.open()
            }
        }
        MenuItem {
            text: I18n.t("Удалить группу")
            enabled: root.canSettings
            onTriggered: SystemController.removeCameraGroup(groupContextMenu.targetGroup)
        }
    }

    Dialog {
        id: renameGroupDialog
        modal: true
        width: 360
        height: 180
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        property string oldName: ""
        property string newName: ""
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle { color: "#252526"; radius: 8; border.color: "#3a3a3a" }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text { text: I18n.t("Переименовать группу"); color: "white"; font.pixelSize: 16; font.bold: true }

            TextField {
                id: renameField
                Layout.fillWidth: true
                text: renameGroupDialog.newName
                color: "white"
                placeholderText: I18n.t("Новое имя группы")
                background: Rectangle { color: "#1f1f1f"; radius: 4; border.color: "#3a3a3a" }
                onTextChanged: renameGroupDialog.newName = text
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                Button {
                    text: I18n.t("Отмена")
                    onClicked: renameGroupDialog.close()
                }
                Button {
                    text: I18n.t("Сохранить")
                    Layout.fillWidth: true
                    onClicked: {
                        var trimmed = renameGroupDialog.newName.trim()
                        if (trimmed)
                            SystemController.renameCameraGroup(renameGroupDialog.oldName, trimmed)
                        renameGroupDialog.close()
                    }
                }
            }
        }
    }
    
    Dialog {
        id: confirmDeleteDialog
        modal: true
        width: 400
        height: 180
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        
        property int cameraIndex: -1
        
        background: Rectangle {
            color: "#252526"
            border.color: "#3e3e42"
            radius: 6
        }
        
        header: Rectangle {
            color: "transparent"
            height: 50
            Text {
                anchors.centerIn: parent
                text: I18n.t("Удаление камеры")
                color: "white"
                font.bold: true
                font.pixelSize: 16
            }
        }
        
        contentItem: ColumnLayout {
            spacing: 20
            
            Text {
                text: I18n.t("Вы действительно хотите удалить эту камеру?")
                color: "#cccccc"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignHCenter
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20
                
                Button {
                    text: I18n.t("ОТМЕНА")
                    background: Rectangle { color: "#444444"; radius: 4 }
                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: confirmDeleteDialog.close()
                }
                
                Button {
                    text: I18n.t("УДАЛИТЬ")
                    background: Rectangle { color: "#d32f2f"; radius: 4 }
                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: {
                        if (confirmDeleteDialog.cameraIndex >= 0) {
                            SystemController.removeDevice(confirmDeleteDialog.cameraIndex)
                        }
                        confirmDeleteDialog.close()
                    }
                }
            }
        }
    }

    SshTerminalDialog {
        id: sshDialog
    }

    FileManagerDialog {
        id: fileManagerDialog
    }

    // Drag Proxy Item
    Rectangle {
        id: dragProxy
        visible: false
        width: 300
        height: 50
        color: "#2a2d2e"
        opacity: 0.9
        z: 1000
        radius: 4
        border.color: "#3b82f6"
        border.width: 1
        
        property string proxyIp: ""
        property string proxyName: ""
        property int dragIndex: -1
        
        Drag.active: visible // Simple binding: if visible (dragging), then active
        Drag.keys: ["camera"]
        Drag.mimeData: { "application/camera-index": dragProxy.dragIndex }
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Drag.source: dragProxy
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            spacing: 10
            Rectangle { width: 8; height: 8; radius: 4; color: "#f44336" }
            ColumnLayout {
                Text { text: I18n.t("Камера") + " " + dragProxy.proxyIp; color: "#cccccc"; font.pixelSize: 12 }
                Text { text: dragProxy.proxyIp; color: "#888888"; font.pixelSize: 11 }
            }
        }
    }

    // Toast Notification
    Rectangle {
        id: toast
        width: Math.min(parent.width - 40, 500)
        height: 50
        color: "#333333"
        radius: 8
        border.color: "#4caf50"
        border.width: 1
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 30
        opacity: 0
        visible: opacity > 0
        z: 2000

        property string message: ""
        property string filePath: ""

        RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 10
            
            Text {
                text: "check_circle"
                font.family: "Material Icons"
                font.pixelSize: 24
                color: "#4caf50"
            }
            
            Text {
                text: toast.message
                color: "white"
                font.pixelSize: 14
                Layout.fillWidth: true
                elide: Text.ElideMiddle
            }

            Button {
                text: I18n.t("Открыть папку")
                visible: toast.filePath !== ""
                background: Rectangle {
                    color: parent.down ? "#3e3e42" : "#2d2d30"
                    radius: 4
                    border.color: "#4caf50"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                }
                onClicked: {
                    // Open folder containing the file
                    var folder = toast.filePath.substring(0, toast.filePath.lastIndexOf("/"))
                    if (folder.length === 0) folder = toast.filePath.substring(0, toast.filePath.lastIndexOf("\\"))
                    Qt.openUrlExternally("file:///" + folder)
                }
            }
        }

        SequentialAnimation {
            id: toastAnim
            
            NumberAnimation { target: toast; property: "opacity"; to: 1; duration: 200 }
            PauseAnimation { duration: 5000 } // Increased duration to allow clicking
            NumberAnimation { target: toast; property: "opacity"; to: 0; duration: 500 }
        }

        function show(msg, path) {
            message = msg
            filePath = path || ""
            toastAnim.restart()
        }
    }

    Connections {
        target: SystemController
        function onSnapshotSaved(path) {
            toast.show(I18n.t("Скриншот сохранен: ") + path, path)
        }
    }
}
