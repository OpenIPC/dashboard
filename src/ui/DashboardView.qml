import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import OpenIPC

Item {
    id: root
    
    // Grid layout state
    property int gridRows: SystemController.gridRows
    property int gridCols: SystemController.gridCols
    property int currentLayoutIndex: 0
    property int selectedPresetCells: 4
    property int currentPage: 0
    readonly property int pageSize: Math.max(1, selectedPresetCells)
    readonly property int pageCount: {
        var version = cameraDataVersion
        return Math.max(1, Math.ceil(SystemController.gridCapacity() / pageSize))
    }
    property bool pageCycling: false
    property bool kioskMode: false
    property int previousWindowVisibility: Window.Maximized
    property bool editingLayout: false
    property string layoutDialogTitle: I18n.t("Редактор шаблонов")
    property int cameraDataVersion: 0
    property int activeGridIndex: -1
    property bool isSidebarVisible: SystemController.appSettings.sidebarVisible !== false
    property bool sidebarToolsExpanded: SystemController.appSettings.sidebarToolsExpanded !== false
    readonly property real sidebarExpandedWidth: Math.max(264, Math.min(320, width * 0.25))
    property real sidebarWidth: !kioskMode && isSidebarVisible ? sidebarExpandedWidth : 0
    property real sidebarOpenProgress: sidebarExpandedWidth > 0 ? (sidebarWidth / sidebarExpandedWidth) : 0
    readonly property bool layoutReady: width > 0
                                        && height > 0
                                        && dashboardGridPanel.width > 0
                                        && dashboardGridPanel.height > 0
                                        && (!isSidebarVisible || dashboardSidebar.width >= 260)
    readonly property bool sidebarToolsSectionVisible: dashboardSidebar.toolsContentVisible

    Behavior on sidebarWidth {
        NumberAnimation {
            duration: 220
            easing.type: Easing.InOutQuad
        }
    }

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
    property bool canAnalytics: (permToken >= 0) && SystemController.userManager.canAnalytics()
    property bool canTalk: (permToken >= 0) && SystemController.userManager.canTalk()
    readonly property bool restrictedCameraScope: {
        var token = permToken
        var user = SystemController.userManager.currentUser
        return user && user.cameraScopes && user.cameraScopes.length > 0
    }
    readonly property bool canGlobalSettings: canSettings && !restrictedCameraScope
    readonly property bool canManageUsers: canUserManage && !restrictedCameraScope

    focus: true
    Keys.onEscapePressed: {
        if (root.kioskMode) {
            root.setKioskMode(false)
            event.accepted = true
        }
    }

    function cameraAccessAllowed(cameraId, cameraIp, cameraIndex) {
        var token = permToken
        return SystemController.userManager.canAccessCamera(cameraId || "", cameraIp || "",
                                                            cameraIndex === undefined ? -1 : cameraIndex)
    }

    function cameraAccessAllowedByIndex(cameraIndex) {
        if (cameraIndex < 0 || cameraIndex >= SystemController.cameraModel.rowCount()) return false
        var camera = SystemController.cameraModel.getCamera(cameraIndex)
        return cameraAccessAllowed(camera.cameraId || "", camera.cameraIp || "", cameraIndex)
    }

    function setCurrentPage(page, persist) {
        var next = Math.max(0, Math.min(Math.max(0, pageCount - 1), page))
        if (SystemController.pushToTalkActive) SystemController.stopPushToTalk()
        currentPage = next
        activeGridIndex = next * pageSize
        cameraDataVersion++
        if (persist !== false) {
            var settings = SystemController.getAppSettings()
            settings.dashboardPage = currentPage
            SystemController.saveAppSettings(settings)
        }
    }

    function setPageCycling(enabled) {
        pageCycling = enabled && pageCount > 1
        var settings = SystemController.getAppSettings()
        settings.dashboardPageCycling = pageCycling
        SystemController.saveAppSettings(settings)
    }

    function setKioskMode(enabled) {
        if (kioskMode === enabled) return
        if (!Window.window) {
            kioskMode = enabled
            return
        }
        if (enabled) {
            previousWindowVisibility = Window.window.visibility
            kioskMode = true
            Window.window.showFullScreen()
        } else {
            kioskMode = false
            if (previousWindowVisibility === Window.Maximized)
                Window.window.showMaximized()
            else
                Window.window.showNormal()
        }
        root.forceActiveFocus()
    }

    Timer {
        interval: 10000
        repeat: true
        running: root.pageCycling && root.pageCount > 1 && root.visible
        onTriggered: root.setCurrentPage((root.currentPage + 1) % root.pageCount, false)
    }

    function actionAllowed(action) {
        if (action === "fleet") return canLive || canGlobalSettings
        if (action === "search" || action === "add_folder" || action === "add_camera" || action === "settings") return canGlobalSettings
        if (action === "camex") return canGlobalSettings
        if (action === "user") return canManageUsers
        if (action === "analytics") return canAnalytics
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

    property string deviceFilterText: ""
    property bool emptyHintDismissed: SystemController.appSettings.hideEmptyDashboardHint === true

    function openAddCameraDialog() {
        if (!canGlobalSettings) {
            showNoAccess()
            return
        }
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

    function openSearchDialog() {
        if (!canGlobalSettings) {
            showNoAccess()
            return
        }
        searchDialog.open()
    }

    function openHealthDialog() {
        cameraHealthDialog.open()
    }

    function openAnalyticsDialog() {
        if (!canAnalytics) {
            showNoAccess()
            return
        }
        analyticsDialog.open()
    }

    function openSettingsDialog() {
        if (!canGlobalSettings) {
            showNoAccess()
            return
        }
        settingsDialog.open()
    }

    function openCamexDialog() {
        if (!canGlobalSettings) {
            showNoAccess()
            return
        }
        if (camexDialog.visible) {
            return
        }
        camexDialog.open()
    }

    function editCameraByIndex(cameraIndex) {
        if (!canGlobalSettings) {
            showNoAccess()
            return
        }
        if (cameraIndex < 0 || cameraIndex >= SystemController.cameraModel.rowCount()) {
            return
        }
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
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
        addCameraDialog.initialPassword = SystemController.getCameraPassword(cam.cameraIp)
        addCameraDialog.initialHdUrl = cam.hdStreamUrl || ""
        addCameraDialog.initialSdUrl = cam.sdStreamUrl || ""
        addCameraDialog.open()
    }

    function openMajesticByIndex(cameraIndex) {
        if (!canSettings) {
            showNoAccess()
            return
        }
        if (cameraIndex < 0 || cameraIndex >= SystemController.cameraModel.rowCount()) {
            return
        }
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
            return
        }
        var cam = SystemController.cameraModel.getCamera(cameraIndex)
        openMajesticForCamera(cam.cameraName || cam.cameraIp, cam.cameraIp, cam.cameraOnvifPort || 80, cam.cameraLogin || "root")
    }

    function openMajesticForCamera(cameraName, cameraIp, cameraPort, cameraLogin) {
        if (!canSettings) {
            showNoAccess()
            return
        }
        majesticDialog.cameraName = cameraName || cameraIp
        majesticDialog.cameraHost = cameraIp
        majesticDialog.cameraPort = cameraPort || 80
        majesticDialog.cameraUser = cameraLogin || "root"
        majesticDialog.cameraPassword = SystemController.getCameraPassword(cameraIp)
        majesticDialog.open()
    }

    function confirmDeleteCamera(cameraIndex) {
        if (!canGlobalSettings) {
            showNoAccess()
            return
        }
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
            return
        }
        confirmDeleteDialog.cameraIndex = cameraIndex
        confirmDeleteDialog.open()
    }

    function openArchiveForIp(cameraIp) {
        if (!canPlayback) {
            showNoAccess()
            return
        }
        var cameraIndex = SystemController.cameraModel.findIndexByIp(cameraIp)
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
            return
        }
        archiveLoader.pendingCameraIp = cameraIp
        archiveLoader.active = true
    }

    function openArchiveByIndex(cameraIndex) {
        if (!canPlayback) {
            showNoAccess()
            return
        }
        if (cameraIndex < 0 || cameraIndex >= SystemController.cameraModel.rowCount()) {
            return
        }
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
            return
        }
        var cam = SystemController.cameraModel.getCamera(cameraIndex)
        openArchiveForIp(cam.cameraIp)
    }

    function openSshByIndex(cameraIndex) {
        if (!canSettings) {
            showNoAccess()
            return
        }
        if (cameraIndex < 0 || cameraIndex >= SystemController.cameraModel.rowCount()) {
            return
        }
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
            return
        }
        var cam = SystemController.cameraModel.getCamera(cameraIndex)
        sshDialog.cameraIp = cam.cameraIp
        sshDialog.cameraUser = cam.cameraLogin || "root"
        sshDialog.open()
    }

    function openFileManagerByIndex(cameraIndex) {
        if (!canExport) {
            showNoAccess()
            return
        }
        if (cameraIndex < 0 || cameraIndex >= SystemController.cameraModel.rowCount()) {
            return
        }
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
            return
        }
        var cam = SystemController.cameraModel.getCamera(cameraIndex)
        fileManagerDialog.cameraIp = cam.cameraIp
        fileManagerDialog.cameraUser = cam.cameraLogin || "root"
        fileManagerDialog.open()
    }

    function refreshHealthByIndex(cameraIndex) {
        if (!canSettings) {
            showNoAccess()
            return
        }
        if (cameraIndex < 0 || cameraIndex >= SystemController.cameraModel.rowCount()) {
            return
        }
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
            return
        }
        var cam = SystemController.cameraModel.getCamera(cameraIndex)
        SystemController.refreshCameraHealth(cam.cameraIp)
        openHealthDialog()
    }

    function addCameraIndexToFirstFreeGrid(cameraIndex) {
        if (!canGlobalSettings) {
            showNoAccess()
            return
        }
        if (cameraIndex < 0 || cameraIndex >= SystemController.cameraModel.rowCount()) {
            return
        }
        if (!cameraAccessAllowedByIndex(cameraIndex)) {
            showNoAccess()
            return
        }
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
            var previousCapacity = slots
            SystemController.ensureGridPageCapacity(pageSize, true)
            if (SystemController.gridCapacity() <= previousCapacity) {
                return
            }
            target = previousCapacity
        }
        SystemController.addCameraToGrid(cameraIndex, target)
        setCurrentPage(Math.floor(target / pageSize))
    }

    function setSidebarVisible(visible) {
        if (isSidebarVisible === visible) {
            return
        }
        isSidebarVisible = visible
        var settings = SystemController.getAppSettings()
        settings.sidebarVisible = visible
        SystemController.saveAppSettings(settings)
    }

    function setSidebarToolsExpanded(expanded) {
        if (sidebarToolsExpanded === expanded) {
            return
        }
        sidebarToolsExpanded = expanded
        var settings = SystemController.getAppSettings()
        settings.sidebarToolsExpanded = expanded
        SystemController.saveAppSettings(settings)
    }

    function closeEmptyHint(remember) {
        emptyHintDismissed = true
        if (remember) {
            var settings = SystemController.getAppSettings()
            settings.hideEmptyDashboardHint = true
            SystemController.saveAppSettings(settings)
        }
    }

    function cameraCount() {
        var version = cameraDataVersion
        var count = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            if (cameraAccessAllowedByIndex(i)) count++
        }
        return count
    }

    function isOnlineStatus(statusText) {
        if (typeof SystemController.isCameraOnline === "function")
            return SystemController.isCameraOnline("", statusText || "")
        return String(statusText || "").toLowerCase() === "online"
    }

    function onlineCameraCount() {
        var version = cameraDataVersion
        var count = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            if (!cameraAccessAllowedByIndex(i)) continue
            var camera = SystemController.cameraModel.getCamera(i)
            if (isOnlineStatus(camera.status)) count++
        }
        return count
    }

    function filteredCameraCount() {
        var version = cameraDataVersion
        var count = 0
        for (var i = 0; i < SystemController.cameraModel.rowCount(); ++i) {
            if (!cameraAccessAllowedByIndex(i)) continue
            var cam = SystemController.cameraModel.getCamera(i)
            if (cameraMatchesDeviceFilter(cam.cameraName, cam.cameraIp,
                                          cameraStatusSearchText(cam.cameraIp, cam.status),
                                          cam.cameraGroup)) count++
        }
        return count
    }

    function effectiveCameraStatus(ip, fallbackStatus) {
        var version = cameraDataVersion
        if (!ip || ip === "") return fallbackStatus || ""
        if (typeof SystemController.effectiveCameraStatus === "function")
            return SystemController.effectiveCameraStatus(ip, fallbackStatus || "")
        return fallbackStatus || ""
    }

    function effectiveCameraDetail(ip, fallbackStatus) {
        var version = cameraDataVersion
        if (!ip || ip === "") return ""
        if (typeof SystemController.cameraAttentionReason === "function")
            return SystemController.cameraAttentionReason(ip, fallbackStatus || "")
        if (typeof SystemController.cameraStatusDetail !== "function") return ""
        return SystemController.cameraStatusDetail(ip) || ""
    }

    function cameraStatusSearchText(ip, fallbackStatus) {
        var version = cameraDataVersion
        if (!ip || ip === "") return fallbackStatus || ""
        if (typeof SystemController.cameraStatusSearchText === "function")
            return SystemController.cameraStatusSearchText(ip, fallbackStatus || "")
        return effectiveCameraStatus(ip, fallbackStatus) + " " + effectiveCameraDetail(ip, fallbackStatus)
    }

    function cameraMatchesDeviceFilter(name, ip, statusText, groupName) {
        var query = deviceFilterText.trim().toLowerCase()
        if (query === "") return true
        var haystack = [
            name || "",
            ip || "",
            statusText || "",
            groupName || ""
        ].join(" ").toLowerCase()
        return haystack.indexOf(query) !== -1
    }

    function gridSlotHasCamera(slot) {
        return slot && ((slot.cameraIp && slot.cameraIp !== "")
                        || (slot.cameraName && slot.cameraName !== "")
                        || (slot.streamUrl && slot.streamUrl !== ""))
    }

    function gridSlotAnalyticsActive(slot) {
        if (!slot || !slot.cameraIp || slot.cameraIp === "") return false
        if (SystemController.analyticsEngine.isModuleEnabled(0)
                && SystemController.analyticsEngine.isCameraModuleEnabled(slot.cameraIp, 0)) return true
        if (SystemController.analyticsEngine.isModuleEnabled(1)
                && SystemController.analyticsEngine.isCameraModuleEnabled(slot.cameraIp, 1)) return true
        if (SystemController.analyticsEngine.isModuleEnabled(2)
                && SystemController.analyticsEngine.isCameraModuleEnabled(slot.cameraIp, 2)) return true
        return false
    }

    function gridSlotConsumesPreviewBudget(slot) {
        if (!gridSlotHasCamera(slot)) return false
        if (slot.isRecording === true) return false
        if (gridSlotAnalyticsActive(slot)) return false
        return true
    }

    function previewPriorityScoreForSlot(slot, slotIndex) {
        return SystemController.streamPreviewPriorityScore(
                    slotIndex,
                    slot && slot.spanRows ? slot.spanRows : 1,
                    slot && slot.spanCols ? slot.spanCols : 1,
                    slotIndex === activeGridIndex,
                    slot && slot.isRecording === true,
                    gridSlotAnalyticsActive(slot),
                    isOnlineStatus(slot ? slot.status : ""))
    }

    function previewBudgetRankFor(slotIndex) {
        var version = cameraDataVersion
        var active = activeGridIndex
        if (version < 0 || active < -2) return 999999
        var target = SystemController.gridModel.getCamera(slotIndex)
        if (!gridSlotHasCamera(target)) return 999999
        if (slotIndex < currentPage * pageSize || slotIndex >= (currentPage + 1) * pageSize) return 999999
        if (!gridSlotConsumesPreviewBudget(target)) return 0

        var targetScore = previewPriorityScoreForSlot(target, slotIndex)
        var rank = 0
        var first = currentPage * pageSize
        var count = Math.min(SystemController.gridModel.rowCount(), first + pageSize)
        for (var i = first; i < count; ++i) {
            if (i === slotIndex) continue
            var other = SystemController.gridModel.getCamera(i)
            if (!gridSlotConsumesPreviewBudget(other)) continue
            var otherScore = previewPriorityScoreForSlot(other, i)
            if (otherScore > targetScore || (otherScore === targetScore && i < slotIndex)) {
                rank++
            }
        }
        return rank
    }

    function smartStreamBudgetEnabled() {
        return SystemController.appSettings.smartStreamBudget !== undefined
                ? SystemController.appSettings.smartStreamBudget
                : true
    }

    function previewBudgetLimit() {
        var value = SystemController.appSettings.maxPreviewStreams
        return value !== undefined ? Math.max(1, value) : 16
    }

    function canRunLiveForBudget() {
        return root.canLive || root.isAdminUser()
    }

    function previewPauseReasonForSlot(slotIndex) {
        if (slotIndex < currentPage * pageSize || slotIndex >= (currentPage + 1) * pageSize)
            return "page"
        var slot = SystemController.gridModel.getCamera(slotIndex)
        return SystemController.previewPauseReasonCode(
                    smartStreamBudgetEnabled(),
                    previewBudgetLimit(),
                    previewBudgetRankFor(slotIndex),
                    gridSlotHasCamera(slot),
                    canRunLiveForBudget(),
                    false,
                    SystemController.isArchiveOpen,
                    slot && slot.isRecording === true,
                    gridSlotAnalyticsActive(slot))
    }

    function activePreviewCount() {
        var version = cameraDataVersion
        var active = activeGridIndex
        if (version < 0 || active < -2) return 0
        var count = 0
        var first = currentPage * pageSize
        var last = Math.min(SystemController.gridModel.rowCount(), first + pageSize)
        for (var i = first; i < last; ++i) {
            var slot = SystemController.gridModel.getCamera(i)
            if (!gridSlotHasCamera(slot)) continue
            var reason = previewPauseReasonForSlot(i)
            if (reason === "") count++
        }
        return count
    }

    function budgetPausedPreviewCount() {
        var version = cameraDataVersion
        var active = activeGridIndex
        if (version < 0 || active < -2) return 0
        var count = 0
        var first = currentPage * pageSize
        var last = Math.min(SystemController.gridModel.rowCount(), first + pageSize)
        for (var i = first; i < last; ++i) {
            if (previewPauseReasonForSlot(i) === "budget") count++
        }
        return count
    }

    // Propagate language changes if needed, though I18n is singleton
    property string appLanguage: I18n.language

    Connections {
        target: SystemController.cameraModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.cameraDataVersion++; Qt.callLater(function() { root.setCurrentPage(root.currentPage, false) }) }
        function onRowsRemoved(parent, first, last) { root.cameraDataVersion++; Qt.callLater(function() { root.setCurrentPage(root.currentPage, false) }) }
        function onModelReset() { root.cameraDataVersion++ }
        function onDataChanged(topLeft, bottomRight, roles) { root.cameraDataVersion++ }
    }

    Connections {
        target: SystemController.gridModel
        ignoreUnknownSignals: true
        function onRowsInserted(parent, first, last) { root.cameraDataVersion++ }
        function onRowsRemoved(parent, first, last) { root.cameraDataVersion++ }
        function onModelReset() { root.cameraDataVersion++ }
        function onDataChanged(topLeft, bottomRight, roles) { root.cameraDataVersion++ }
    }

    Connections {
        target: SystemController.analyticsEngine
        ignoreUnknownSignals: true
        function onModuleStatusChanged() { root.cameraDataVersion++ }
    }

    Connections {
        target: SystemController
        ignoreUnknownSignals: true
        function onCameraStatusDetailsChanged() { root.cameraDataVersion++ }
        function onAppSettingsChanged() {
            var nextVisible = SystemController.appSettings.sidebarVisible !== false
            if (root.isSidebarVisible !== nextVisible) {
                root.isSidebarVisible = nextVisible
            }
            var nextToolsExpanded = SystemController.appSettings.sidebarToolsExpanded !== false
            if (root.sidebarToolsExpanded !== nextToolsExpanded) {
                root.sidebarToolsExpanded = nextToolsExpanded
            }
        }
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
        
        selectedPresetCells = (layoutCells[index] && layoutCells[index].length > 0) ? layoutCells[index].length : (item.rows * item.cols)
        SystemController.applyLayoutTemplate(template)
        setCurrentPage(currentPage, false)
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

    function openLayoutEditor(editExisting) {
        editingLayout = editExisting
        root.layoutDialogTitle = editExisting ? I18n.t("Редактирование") : I18n.t("Новая раскладка")
        
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
            
            for (var i = 0; i < root.layoutPresets.length; i++) {
                var p = root.layoutPresets[i]
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
        
        root.selectedPresetId = initialPreset
        var savedCells = (initialPreset === "custom" && editExisting) ? (layoutCells[currentLayoutIndex] || []) : []
        layoutDialog.openEditor(root.layoutDialogTitle, initialPreset, initialName, initialRows, initialCols, savedCells)
    }

    function saveLayoutTemplate(nameOverride, presetIdOverride) {
        // Only handles presets now. Custom layouts are handled in the Dialog's OK button.
        var r = 2, c = 2, cells = []
        var activePresetId = presetIdOverride || root.selectedPresetId
        
        var preset = null
        for(var i=0; i<root.layoutPresets.length; ++i) {
            if (root.layoutPresets[i].id === activePresetId) {
                preset = root.layoutPresets[i]
                break
            }
        }
        if (preset) {
            r = preset.rows
            c = preset.cols
            cells = preset.cells || []
        }
        
        var name = (nameOverride || "").trim()
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
    }

    function applyCustomLayoutTemplate(name, rows, cols, cells) {
        var templateName = (name || "").trim()
        if (templateName.length === 0)
            templateName = I18n.t("Польз. план")

        if (editingLayout && currentLayoutIndex >= 0) {
            layoutModel.setProperty(currentLayoutIndex, "name", templateName)
            layoutModel.setProperty(currentLayoutIndex, "rows", rows)
            layoutModel.setProperty(currentLayoutIndex, "cols", cols)
            layoutModel.setProperty(currentLayoutIndex, "isDefault", false)
            var updatedCells = layoutCells
            updatedCells[currentLayoutIndex] = cells
            layoutCells = updatedCells
            applyLayout(currentLayoutIndex)
        } else {
            layoutModel.append({ "name": templateName, "rows": rows, "cols": cols, "isDefault": false })
            var newCells = layoutCells
            newCells.push(cells)
            layoutCells = newCells
            applyLayout(layoutModel.count - 1)
        }

        syncLayoutsToBackend()
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
        
        // Ensure capacity: for complex templates use actual visible cell count
        var need = gridRows * gridCols
        if (currentLayoutIndex >= 0 && layoutCells[currentLayoutIndex] && layoutCells[currentLayoutIndex].length > 0) {
            need = layoutCells[currentLayoutIndex].length
        }
        selectedPresetCells = need
        SystemController.ensureGridPageCapacity(need)
        var savedPage = Number(SystemController.appSettings.dashboardPage || 0)
        pageCycling = SystemController.appSettings.dashboardPageCycling === true
        setCurrentPage(savedPage, false)
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
            list[i] = obj
        }
        SystemController.layoutTemplates = list
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        DashboardTopBar {
            id: dashboardTopBar
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? 64 : 0
            visible: !root.kioskMode
            layoutsModel: layoutModel
            currentLayoutIndex: root.currentLayoutIndex
            currentPage: root.currentPage
            pageCount: root.pageCount
            pageCycling: root.pageCycling
            onApplyLayoutRequested: (index) => root.applyLayout(index)
            onCloseLayoutRequested: (index) => root.closeLayout(index)
            onAddLayoutRequested: root.openLayoutEditor(false)
            onEditLayoutRequested: root.openLayoutEditor(true)
            onPreviousPageRequested: root.setCurrentPage((root.currentPage - 1 + root.pageCount) % root.pageCount)
            onNextPageRequested: root.setCurrentPage((root.currentPage + 1) % root.pageCount)
            onPageCyclingToggleRequested: root.setPageCycling(!root.pageCycling)
            onKioskToggleRequested: root.setKioskMode(true)
        }

        RowLayout {
            id: dashboardContentRow

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: root.kioskMode ? 0 : 8
            Layout.rightMargin: root.kioskMode ? 0 : 8
            Layout.topMargin: root.kioskMode ? 0 : 8
            Layout.bottomMargin: root.kioskMode ? 0 : 8
            spacing: Math.max(0, 8 * root.sidebarOpenProgress)

            DashboardGridPanel {
                id: dashboardGridPanel

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 0
                systemController: SystemController
                gridRows: root.gridRows
                gridCols: root.gridCols
                activeGridIndex: root.activeGridIndex
                pageStart: root.currentPage * root.pageSize
                pageSize: root.pageSize
                kioskMode: root.kioskMode
                canLive: root.canLive
                canPlayback: root.canPlayback
                canPtz: root.canPtz
                canTalk: root.canTalk
                canExport: root.canExport
                canSettings: root.canGlobalSettings
                canAnalytics: root.canAnalytics
                emptyHintDismissed: root.emptyHintDismissed
                sidebarOpenProgress: root.sidebarOpenProgress
                previewBudgetRankProvider: root.previewBudgetRankFor
                cameraAccessProvider: function(cameraId, cameraIp) {
                    var index = SystemController.cameraModel.findIndexByIp(cameraIp)
                    return root.cameraAccessAllowed(cameraId, cameraIp, index)
                }

                onSelectedByUser: (index) => {
                    root.activeGridIndex = index
                    root.cameraDataVersion++
                }
                onPermissionDenied: root.showNoAccess()
                onEditCameraRequested: (cameraIndex) => root.editCameraByIndex(cameraIndex)
                onDeleteCameraRequested: (cameraIndex) => root.confirmDeleteCamera(cameraIndex)
                onArchiveRequested: (cameraIp) => root.openArchiveForIp(cameraIp)
                onMajesticRequested: (cameraName, cameraIp, cameraPort, cameraLogin) => {
                    root.openMajesticForCamera(cameraName, cameraIp, cameraPort, cameraLogin)
                }
                onSearchRequested: root.openSearchDialog()
                onAddCameraRequested: root.openAddCameraDialog()
                onAnalyticsRequested: root.openAnalyticsDialog()
                onSettingsRequested: root.openSettingsDialog()
                onSidebarOpenRequested: root.setSidebarVisible(true)
                onEmptyHintClosed: (dontShowAgain) => root.closeEmptyHint(dontShowAgain)
                onGridCameraRemoved: {
                    SystemController.compactGridPages(root.pageSize)
                    root.setCurrentPage(root.currentPage, false)
                }
                onMessageRequested: (message) => root.showNoAccess(message)
            }

            DashboardSidebar {
                id: dashboardSidebar

                Layout.minimumWidth: 0
                dashboard: root
                systemController: SystemController
                dragProxyItem: dragProxy
                sidebarWidth: root.sidebarWidth
                sidebarOpenProgress: root.kioskMode ? 0 : root.sidebarOpenProgress
                cameraDataVersion: root.cameraDataVersion
                deviceFilterText: root.deviceFilterText
                canSettings: root.canGlobalSettings
                toolsExpanded: root.sidebarToolsExpanded

                onCloseSidebarRequested: root.setSidebarVisible(false)
                onToolsExpandedToggleRequested: root.setSidebarToolsExpanded(!root.sidebarToolsExpanded)
                onNoAccessRequested: root.showNoAccess()
                onSearchRequested: root.openSearchDialog()
                onHealthRequested: root.openHealthDialog()
                onFleetRequested: fleetDialog.open()
                onAddGroupRequested: addGroupDialog.open()
                onAddCameraRequested: root.openAddCameraDialog()
                onAnalyticsRequested: root.openAnalyticsDialog()
                onSettingsRequested: root.openSettingsDialog()
                onUserManagementRequested: userManagementDialog.open()
                onLogsRequested: logView.open()
                onCamexRequested: root.openCamexDialog()
                onDeviceContextRequested: (cameraIp, cameraName, cameraIndex) => {
                    deviceContextMenu.cameraIp = cameraIp
                    deviceContextMenu.cameraName = cameraName
                    deviceContextMenu.cameraIndex = cameraIndex
                    deviceContextMenu.popup()
                }
                onGroupContextRequested: (groupName) => {
                    groupContextMenu.targetGroup = groupName
                    groupContextMenu.popup()
                }
                onAddCameraToGridRequested: (cameraIndex) => root.addCameraIndexToFirstFreeGrid(cameraIndex)
                onMajesticRequested: (cameraIndex) => root.openMajesticByIndex(cameraIndex)
                onCameraDataChangedRequested: root.cameraDataVersion++
                onDeviceFilterChanged: (text) => root.deviceFilterText = text
            }
        }

        DashboardStatusBar {
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? 25 : 0
            visible: !root.kioskMode
            systemController: SystemController
        }
    }

    DashboardPageControls {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        z: 100
        visible: root.kioskMode
        currentPage: root.currentPage
        pageCount: root.pageCount
        cycling: root.pageCycling
        kioskActive: true
        onPreviousRequested: root.setCurrentPage((root.currentPage - 1 + root.pageCount) % root.pageCount)
        onNextRequested: root.setCurrentPage((root.currentPage + 1) % root.pageCount)
        onCyclingToggleRequested: root.setPageCycling(!root.pageCycling)
        onKioskToggleRequested: root.setKioskMode(false)
    }

    LayoutEditorDialog {
        id: layoutDialog
        presets: root.layoutPresets
        onSelectedPresetIdChanged: root.selectedPresetId = selectedPresetId
        onCustomAccepted: (name, rows, cols, cells) => {
            root.selectedPresetId = "custom"
            root.applyCustomLayoutTemplate(name, rows, cols, cells)
        }
        onPresetAccepted: (name, presetId) => {
            root.selectedPresetId = presetId
            root.saveLayoutTemplate(name, presetId)
        }
    }

    SettingsDialog {
        id: settingsDialog
        language: root.appLanguage
        onLanguageChanged: {
            root.appLanguage = language
            I18n.language = language
            root.layoutDialogTitle = root.editingLayout ? I18n.t("Редактирование") : I18n.t("Редактор шаблонов")
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

    CameraHealthDialog {
        id: cameraHealthDialog
        onEditRequested: (cameraIndex) => root.editCameraByIndex(cameraIndex)
        onMajesticRequested: (cameraIndex) => root.openMajesticByIndex(cameraIndex)
        onAddToGridRequested: (cameraIndex) => root.addCameraIndexToFirstFreeGrid(cameraIndex)
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

    CamexView {
        id: camexDialog
    }

    UserManagementDialog {
        id: userManagementDialog
    }

    LogView {
        id: logView
        logModel: SystemController.logModel
    }

    FleetManagementDialog {
        id: fleetDialog
    }

    NoAccessDialog {
        id: noAccessDialog
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
        canSettings: root.canGlobalSettings
        canExport: root.canExport
        onDeleteRequested: root.confirmDeleteCamera(cameraIndex)
        onEditRequested: root.editCameraByIndex(cameraIndex)
        onGroupChanged: root.cameraDataVersion++
        onSshRequested: root.openSshByIndex(cameraIndex)
        onMajesticRequested: root.openMajesticByIndex(cameraIndex)
        onHealthRequested: root.refreshHealthByIndex(cameraIndex)
        onArchiveRequested: root.openArchiveByIndex(cameraIndex)
        onFileManagerRequested: root.openFileManagerByIndex(cameraIndex)
        onPermissionDenied: root.showNoAccess()
    }

    GroupContextMenu {
        id: groupContextMenu
        canSettings: root.canGlobalSettings
        onRenameRequested: (groupName) => {
            renameGroupDialog.oldName = groupName
            renameGroupDialog.newName = groupName
            renameGroupDialog.open()
        }
        onDeleteRequested: (groupName) => SystemController.removeCameraGroup(groupName)
    }

    RenameGroupDialog {
        id: renameGroupDialog
        onRenameAccepted: (oldName, newName) => SystemController.renameCameraGroup(oldName, newName)
    }
    
    ConfirmDeleteCameraDialog {
        id: confirmDeleteDialog
        onDeleteAccepted: (cameraIndex) => {
            if (!root.canGlobalSettings) {
                root.showNoAccess()
                return
            }
            SystemController.removeDevice(cameraIndex)
        }
    }

    SshTerminalDialog {
        id: sshDialog
    }

    MajesticControlDialog {
        id: majesticDialog
    }

    FileManagerDialog {
        id: fileManagerDialog
    }

    // Drag Proxy Item
    DashboardDragProxy {
        id: dragProxy
    }

    // Toast Notification
    DashboardToast {
        id: toast
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 30
    }

    AppUpdateDialog {
        id: appUpdateDialog
        updateChecker: SystemController.appUpdateChecker
    }

    Connections {
        target: SystemController
        function onSnapshotSaved(path) {
            if (SystemController.appSettings.notificationsEnabled === false) return
            toast.show(I18n.t("Скриншот сохранен: ") + path, path)
        }
    }

    Connections {
        target: SystemController.appUpdateChecker
        function onUpdateAvailable() {
            appUpdateDialog.openDialog()
        }
    }
}
