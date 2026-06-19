import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import OpenIPC

Item {
    id: root
    property string snapshotsDir: ""
    property string moduleBadgeText: ""
    property string clipsDir: ""

    property bool sortNewest: true

    // Filters
    property string cameraFilter: ""
    property string labelFilter: ""
    property string startDateFilter: ""
    property string endDateFilter: ""

    // Derived
    property int totalCount: folderModel.count
    property int filteredCount: filteredModel.count
    property int currentIndex: -1
    property int selectedDateIndex: 0
    property bool selectionMode: false
    property var selectedMap: ({})
    property int selectedCount: 0
    property int selectedVersion: 0
    readonly property color panelColor: Theme.panelBackground
    readonly property color panelBorderColor: Theme.panelBorder
    readonly property color controlBgColor: Theme.panelSoftBackground
    readonly property color controlBorderColor: Theme.controlBorder
    readonly property bool hasActiveFilters: cameraFilter !== "" || labelFilter.trim() !== "" || startDateFilter.trim() !== "" || endDateFilter.trim() !== "" || !sortNewest

    function parseMeta(fileName) {
        var result = { cameraId: "", label: "", capturedAtMs: 0, capturedAtText: "", timestampKey: "" }
        if (!fileName) return result

        var re = /^(.+)_([0-9]{4})-([0-9]{2})-([0-9]{2})_([0-9]{2})-([0-9]{2})-([0-9]{2})-([0-9]{3})_(.+)\.(jpg|jpeg|png)$/i
        var m = re.exec(fileName)
        if (m && m.length >= 10) {
            result.cameraId = m[1]
            var y = parseInt(m[2], 10)
            var mo = parseInt(m[3], 10) - 1
            var d = parseInt(m[4], 10)
            var h = parseInt(m[5], 10)
            var mi = parseInt(m[6], 10)
            var s = parseInt(m[7], 10)
            var ms = parseInt(m[8], 10)
            result.label = m[9]
            result.timestampKey = m[2] + "-" + m[3] + "-" + m[4] + "_" + m[5] + "-" + m[6] + "-" + m[7] + "-" + m[8]
            var dt = new Date(y, mo, d, h, mi, s, ms)
            if (!isNaN(dt.getTime())) {
                result.capturedAtMs = dt.getTime()
                result.capturedAtText = Qt.formatDateTime(dt, "yyyy-MM-dd HH:mm:ss")
            }
        }
        return result
    }

    function parseDateInput(str) {
        if (!str || str.trim() === "") return null
        var t = str.trim()
        var re = /^([0-9]{4})-([0-9]{2})-([0-9]{2})(?:\s+([0-9]{2}):([0-9]{2})(?::([0-9]{2}))?)?$/
        var m = re.exec(t)
        if (!m) return null
        var y = parseInt(m[1], 10)
        var mo = parseInt(m[2], 10) - 1
        var d = parseInt(m[3], 10)
        var h = m[4] ? parseInt(m[4], 10) : 0
        var mi = m[5] ? parseInt(m[5], 10) : 0
        var s = m[6] ? parseInt(m[6], 10) : 0
        var dt = new Date(y, mo, d, h, mi, s, 0)
        if (isNaN(dt.getTime())) return null
        return dt
    }

    function dateToEndOfDay(d) {
        return new Date(d.getFullYear(), d.getMonth(), d.getDate(), 23, 59, 59, 999)
    }

    function applyDateFilter(val) {
        if (val === "") {
            startDateField.text = ""
            endDateField.text = ""
        } else {
            startDateField.text = val
            endDateField.text = val
        }
        rebuildFilteredModel()
    }

    function isSelected(fileUrl) {
        var _ = selectedVersion
        return selectedMap[fileUrl] === true
    }

    function toggleSelected(fileUrl) {
        if (!fileUrl) return
        if (selectedMap[fileUrl] === true) {
            delete selectedMap[fileUrl]
            selectedCount = Math.max(0, selectedCount - 1)
        } else {
            selectedMap[fileUrl] = true
            selectedCount += 1
        }
        selectedVersion += 1
    }

    function clearSelection() {
        selectedMap = ({})
        selectedCount = 0
        selectedVersion += 1
    }

    function deleteSelected() {
        for (var key in selectedMap) {
            if (selectedMap.hasOwnProperty(key)) {
                SystemController.deleteLocalFile(key)
            }
        }
        clearSelection()
        if (folderModel.refresh) folderModel.refresh()
        rebuildFilteredModel()
    }

    function clearFilters() {
        cameraCombo.currentIndex = 0
        cameraFilter = ""
        labelField.text = ""
        startDateField.text = ""
        endDateField.text = ""
        sortCombo.currentIndex = 0
        sortNewest = true
        rebuildFilteredModel()
    }

    function rebuildFilteredModel() {
        filteredModel.clear()
        cameraOptionsModel.clear()
        cameraOptionsModel.append({ text: I18n.t("Все"), value: "" })
        dateGroupsModel.clear()
        dateGroupsModel.append({ text: I18n.t("Все"), value: "" })

        var cameraSet = {}
        var startDate = parseDateInput(startDateFilter)
        var endDate = parseDateInput(endDateFilter)
        if (endDate) endDate = dateToEndOfDay(endDate)

        var dateMap = {}
        for (var i = 0; i < folderModel.count; i++) {
            var fileName = folderModel.get(i, "fileName")
            var fileUrl = folderModel.get(i, "fileUrl")
            var filePath = folderModel.get(i, "filePath")
            var fileModified = folderModel.get(i, "fileModified")
            if (!fileName) continue

            var meta = parseMeta(fileName)
            var capturedAtMs = meta.capturedAtMs
            if (!capturedAtMs && fileModified) {
                capturedAtMs = fileModified.getTime ? fileModified.getTime() : new Date(fileModified).getTime()
                meta.capturedAtText = Qt.formatDateTime(fileModified, "yyyy-MM-dd HH:mm:ss")
            }

            if (capturedAtMs) {
                var d = new Date(capturedAtMs)
                var dateKey = Qt.formatDateTime(d, "yyyy-MM-dd")
                if (!dateMap[dateKey]) {
                    dateMap[dateKey] = 0
                }
                dateMap[dateKey] += 1
            }

            var cameraId = meta.cameraId || ""
            var label = meta.label || ""
            if (cameraId && !cameraSet[cameraId]) {
                cameraSet[cameraId] = true
                cameraOptionsModel.append({ text: cameraId, value: cameraId })
            }

            var matchesCamera = true
            if (cameraFilter && cameraFilter.trim() !== "") {
                matchesCamera = cameraId === cameraFilter
            }

            var matchesLabel = true
            if (labelFilter && labelFilter.trim() !== "") {
                matchesLabel = label.toLowerCase().indexOf(labelFilter.toLowerCase()) !== -1
            }

            var matchesDate = true
            if (startDate && capturedAtMs) {
                matchesDate = capturedAtMs >= startDate.getTime()
            }
            if (matchesDate && endDate && capturedAtMs) {
                matchesDate = capturedAtMs <= endDate.getTime()
            }

            if (matchesCamera && matchesLabel && matchesDate) {
                var clipUrl = ""
                if (root.clipsDir && meta.timestampKey) {
                    var clipsBase = root.clipsDir.replace(/\\/g, "/")
                    var candidates = []
                    if (meta.cameraId) {
                        candidates.push("file:///" + clipsBase + "/" + meta.cameraId + "_" + meta.timestampKey + "_clip.mp4")
                    }
                    if (meta.label) {
                        candidates.push("file:///" + clipsBase + "/" + meta.label + "_" + meta.timestampKey + "_clip.mp4")
                    }
                    candidates.push("file:///" + clipsBase + "/" + meta.timestampKey + "_clip.mp4")

                    for (var ci = 0; ci < candidates.length; ci++) {
                        if (SystemController.localFileExists(candidates[ci])) {
                            clipUrl = candidates[ci]
                            break
                        }
                    }
                }
                filteredModel.append({
                    fileUrl: fileUrl,
                    fileName: fileName,
                    filePath: filePath,
                    fileModified: fileModified,
                    cameraId: cameraId,
                    label: label,
                    capturedAtMs: capturedAtMs,
                    capturedAtText: meta.capturedAtText,
                    clipUrl: clipUrl
                })
            }
        }

        // Build date groups (newest first)
        var dateKeys = Object.keys(dateMap)
        dateKeys.sort(function(a, b) { return a < b ? 1 : -1 })
        for (var dk = 0; dk < dateKeys.length; dk++) {
            var key = dateKeys[dk]
            dateGroupsModel.append({ text: key + " (" + dateMap[key] + ")", value: key })
        }

        // Keep camera selection in sync
        var idx = 0
        if (cameraFilter && cameraFilter.trim() !== "") {
            for (var j = 0; j < cameraOptionsModel.count; j++) {
                if (cameraOptionsModel.get(j).value === cameraFilter) {
                    idx = j
                    break
                }
            }
            if (idx === 0) {
                cameraFilter = ""
            }
        }
        cameraCombo.currentIndex = idx

        // Keep date selection in sync
        var dateIdx = 0
        if (startDateFilter && endDateFilter && startDateFilter === endDateFilter) {
            for (var di = 0; di < dateGroupsModel.count; di++) {
                if (dateGroupsModel.get(di).value === startDateFilter) {
                    dateIdx = di
                    break
                }
            }
        }
        selectedDateIndex = dateIdx

        if (currentIndex >= filteredModel.count) {
            currentIndex = filteredModel.count - 1
        }
    }

    FolderListModel {
        id: folderModel
        folder: root.snapshotsDir ? "file:///" + root.snapshotsDir : ""
        nameFilters: ["*.jpg", "*.jpeg", "*.png"]
        showDirs: false
        sortField: FolderListModel.Time
        sortReversed: root.sortNewest
        onCountChanged: rebuildFilteredModel()
        onFolderChanged: rebuildFilteredModel()
    }

    ListModel { id: filteredModel }
    ListModel { id: cameraOptionsModel }
    ListModel { id: dateGroupsModel }

    Rectangle {
        anchors.fill: parent
        color: root.panelColor
        radius: Theme.radiusLg
        border.color: root.panelBorderColor
        border.width: 1
        z: -1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        // Filters + quick period
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 112
            color: root.panelColor
            radius: Theme.radiusLg
            border.color: root.panelBorderColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                Text {
                    text: I18n.t("Лента + фильтры")
                    color: Theme.textMuted
                    font.pixelSize: 11
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ColumnLayout {
                        Layout.preferredWidth: 190
                        spacing: 2
                        Text { text: I18n.t("Камера"); color: "#aaa"; font.pixelSize: 11 }
                        StyledComboBox {
                            id: cameraCombo
                            Layout.fillWidth: true
                            model: cameraOptionsModel
                            textRole: "text"
                            implicitHeight: 30
                            onUserSelected: function(index) {
                                cameraFilter = cameraOptionsModel.get(index).value
                                rebuildFilteredModel()
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.preferredWidth: 190
                        spacing: 2
                        Text { text: I18n.t("Объект"); color: "#aaa"; font.pixelSize: 11 }
                        TextField {
                            id: labelField
                            Layout.fillWidth: true
                            implicitHeight: 30
                            placeholderText: I18n.t("Введите для фильтра")
                            onTextChanged: {
                                labelFilter = text
                                rebuildFilteredModel()
                            }
                            color: Theme.textSecondary
                            selectionColor: Theme.accent
                            placeholderTextColor: "#777"
                            background: Rectangle { color: root.controlBgColor; radius: Theme.radiusSm; border.color: root.controlBorderColor }
                        }
                    }

                    ColumnLayout {
                        Layout.preferredWidth: 150
                        spacing: 2
                        Text { text: I18n.t("Дата от"); color: "#aaa"; font.pixelSize: 11 }
                        TextField {
                            id: startDateField
                            Layout.fillWidth: true
                            implicitHeight: 30
                            placeholderText: "2026-02-17"
                            onTextChanged: {
                                startDateFilter = text
                                rebuildFilteredModel()
                            }
                            color: Theme.textSecondary
                            selectionColor: Theme.accent
                            placeholderTextColor: "#777"
                            background: Rectangle { color: root.controlBgColor; radius: Theme.radiusSm; border.color: root.controlBorderColor }
                        }
                    }

                    ColumnLayout {
                        Layout.preferredWidth: 150
                        spacing: 2
                        Text { text: I18n.t("Дата до"); color: "#aaa"; font.pixelSize: 11 }
                        TextField {
                            id: endDateField
                            Layout.fillWidth: true
                            implicitHeight: 30
                            placeholderText: "2026-02-17"
                            onTextChanged: {
                                endDateFilter = text
                                rebuildFilteredModel()
                            }
                            color: Theme.textSecondary
                            selectionColor: Theme.accent
                            placeholderTextColor: "#777"
                            background: Rectangle { color: root.controlBgColor; radius: Theme.radiusSm; border.color: root.controlBorderColor }
                        }
                    }

                    ColumnLayout {
                        Layout.preferredWidth: 150
                        spacing: 2
                        Text { text: I18n.t("Сортировка"); color: "#aaa"; font.pixelSize: 11 }
                        StyledComboBox {
                            id: sortCombo
                            Layout.fillWidth: true
                            implicitHeight: 30
                            model: [I18n.t("Сначала новые"), I18n.t("Сначала старые")]
                            onUserSelected: function(index) {
                                sortNewest = (currentIndex === 0)
                                rebuildFilteredModel()
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: I18n.t("Очистить")
                        enabled: root.hasActiveFilters
                        implicitHeight: 30
                        contentItem: Text {
                            text: parent.text
                            color: "#ddd"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.hovered ? Theme.cardHover : root.controlBgColor
                            radius: Theme.radiusSm
                            border.color: root.controlBorderColor
                        }
                        onClicked: clearFilters()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Text {
                        text: I18n.t("Быстрый период")
                        color: "#888"
                        font.pixelSize: 11
                    }

                    Button {
                        text: I18n.t("Сегодня")
                        implicitHeight: 28
                        contentItem: Text {
                            text: parent.text
                            color: "#ddd"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.hovered ? Theme.cardHover : root.controlBgColor
                            radius: Theme.radiusSm
                            border.color: root.controlBorderColor
                        }
                        onClicked: {
                            var now = new Date()
                            var start = new Date(now.getFullYear(), now.getMonth(), now.getDate())
                            var end = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 23, 59, 59, 999)
                            startDateField.text = Qt.formatDateTime(start, "yyyy-MM-dd")
                            endDateField.text = Qt.formatDateTime(end, "yyyy-MM-dd")
                        }
                    }

                    Button {
                        text: I18n.t("7 дней")
                        implicitHeight: 28
                        contentItem: Text {
                            text: parent.text
                            color: "#ddd"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.hovered ? Theme.cardHover : root.controlBgColor
                            radius: Theme.radiusSm
                            border.color: root.controlBorderColor
                        }
                        onClicked: {
                            var now = new Date()
                            var start = new Date(now.getTime() - 6 * 24 * 60 * 60 * 1000)
                            startDateField.text = Qt.formatDateTime(start, "yyyy-MM-dd")
                            endDateField.text = Qt.formatDateTime(now, "yyyy-MM-dd")
                        }
                    }

                    Button {
                        text: I18n.t("30 дней")
                        implicitHeight: 28
                        contentItem: Text {
                            text: parent.text
                            color: "#ddd"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.hovered ? Theme.cardHover : root.controlBgColor
                            radius: Theme.radiusSm
                            border.color: root.controlBorderColor
                        }
                        onClicked: {
                            var now = new Date()
                            var start = new Date(now.getTime() - 29 * 24 * 60 * 60 * 1000)
                            startDateField.text = Qt.formatDateTime(start, "yyyy-MM-dd")
                            endDateField.text = Qt.formatDateTime(now, "yyyy-MM-dd")
                        }
                    }

                    Button {
                        text: I18n.t("За все время")
                        implicitHeight: 28
                        contentItem: Text {
                            text: parent.text
                            color: "#ddd"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.hovered ? Theme.cardHover : root.controlBgColor
                            radius: Theme.radiusSm
                            border.color: root.controlBorderColor
                        }
                        onClicked: {
                            startDateField.text = ""
                            endDateField.text = ""
                        }
                    }

                    Item { Layout.fillWidth: true }
                }
            }
        }

        // Info row
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: root.panelColor
            radius: Theme.radiusLg
            border.color: root.panelBorderColor

            RowLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 8

                Text {
                    text: I18n.t("Показано %1 из %2", [filteredCount, totalCount])
                    color: "#888"
                    font.pixelSize: 11
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: I18n.t("Открыть папку снимков")
                    implicitHeight: 26
                    contentItem: Text {
                        text: parent.text
                        color: "#ddd"
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? Theme.cardHover : root.controlBgColor
                        radius: Theme.radiusSm
                        border.color: root.controlBorderColor
                    }
                    visible: root.snapshotsDir !== ""
                    onClicked: Qt.openUrlExternally("file:///" + root.snapshotsDir)
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            color: root.panelColor
            radius: Theme.radiusLg
            border.color: root.panelBorderColor

            RowLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 8

                Button {
                    text: I18n.t("Режим выбора")
                    checkable: true
                    checked: root.selectionMode
                    implicitHeight: 26
                    onClicked: {
                        root.selectionMode = checked
                        if (!root.selectionMode) clearSelection()
                    }
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "#3b82f6" : "#ddd"
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.checked ? Theme.panelSoftBackground : (parent.hovered ? Theme.cardHover : root.controlBgColor)
                        radius: Theme.radiusSm
                        border.color: parent.checked ? Theme.accent : root.controlBorderColor
                    }
                }

                Text {
                    text: root.selectedCount > 0 ? I18n.t("Выбрано: %1", [root.selectedCount]) : ""
                    color: "#aaa"
                    font.pixelSize: 11
                    visible: root.selectedCount > 0
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: I18n.t("Удалить выбранные") + " (" + root.selectedCount + ")"
                    visible: root.selectedCount > 0
                    implicitHeight: 26
                    contentItem: Text {
                        text: parent.text
                        color: "#ddd"
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? "#3b1f1f" : "#2a1f1f"
                        radius: Theme.radiusSm
                        border.color: "#6b2a2a"
                    }
                    onClicked: deleteSelected()
                }
            }
        }

        // Timeline + Grid
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 170
                Layout.fillHeight: true
                color: root.panelColor
                radius: Theme.radiusLg
                border.color: root.panelBorderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    Text {
                        text: I18n.t("Даты")
                        color: "#aaa"
                        font.pixelSize: 12
                    }

                    ListView {
                        id: dateList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: dateGroupsModel
                        clip: true
                        currentIndex: root.selectedDateIndex

                        delegate: ItemDelegate {
                            width: dateList.width
                            onClicked: {
                                root.selectedDateIndex = index
                                root.applyDateFilter(model.value)
                            }
                            contentItem: Text {
                                text: model.text
                                color: "#ddd"
                                elide: Text.ElideRight
                            }
                            background: Rectangle {
                                color: index === root.selectedDateIndex ? "#2d2d2d" : "transparent"
                                radius: Theme.radiusSm
                            }
                        }
                    }

                    Text {
                        text: dateGroupsModel.count <= 1 ? I18n.t("Нет событий") : ""
                        color: "#666"
                        font.pixelSize: 12
                        visible: dateGroupsModel.count <= 1
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    anchors.fill: parent
                    color: root.panelColor
                    radius: Theme.radiusLg
                    border.color: root.panelBorderColor
                }

                GridView {
                    id: gridView
                    anchors.fill: parent
                    anchors.margins: 8
                    cellWidth: 220
                    cellHeight: 260
                    clip: true
                    model: filteredModel

                    delegate: Item {
                        width: 210
                        height: 250

                        Rectangle {
                            anchors.fill: parent
                            color: Theme.cardBackground
                            radius: Theme.radiusLg
                            border.color: Theme.cardBorder
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

                                    // Badges
                                    Row {
                                        anchors.left: parent.left
                                        anchors.top: parent.top
                                        anchors.margins: 6
                                        spacing: 6

                                        Rectangle {
                                                color: Theme.accent
                                                radius: Theme.radiusSm
                                            visible: cameraId !== ""
                                            Text {
                                                text: cameraId
                                                color: "white"
                                                font.pixelSize: 10
                                                padding: 4
                                            }
                                        }

                                        Rectangle {
                                            color: Theme.success
                                            radius: Theme.radiusSm
                                            visible: label !== ""
                                            Text {
                                                text: label
                                                color: "white"
                                                font.pixelSize: 10
                                                padding: 4
                                            }
                                        }

                                        Rectangle {
                                            color: "#6b7280"
                                            radius: Theme.radiusSm
                                            visible: label === "" && root.moduleBadgeText !== ""
                                            Text {
                                                text: root.moduleBadgeText
                                                color: "white"
                                                font.pixelSize: 10
                                                padding: 4
                                            }
                                        }
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
                                    text: capturedAtText !== "" ? capturedAtText : (fileModified ? Qt.formatDateTime(fileModified, "yyyy-MM-dd HH:mm:ss") : "")
                                    color: "#aaa"
                                    font.pixelSize: 10
                                    Layout.fillWidth: true
                                    horizontalAlignment: Text.AlignHCenter
                                }
                            }

                            Rectangle {
                                width: 18
                                height: 18
                                radius: 3
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.margins: 8
                                color: "#252526"
                                border.color: "#666666"
                                visible: root.selectionMode
                                z: 3

                                Rectangle {
                                    width: 10
                                    height: 10
                                    anchors.centerIn: parent
                                    radius: 2
                                    color: "#4caf50"
                                    visible: root.isSelected(fileUrl)
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    if (root.selectionMode) {
                                        root.toggleSelected(fileUrl)
                                    } else {
                                        root.currentIndex = index
                                        imageViewer.openWithModel(filteredModel, index)
                                    }
                                }
                                onEntered: parent.border.color = Theme.accent
                                onExited: parent.border.color = Theme.cardBorder
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar { }
                }

                // Empty state
                Item {
                    anchors.fill: parent
                    visible: filteredModel.count === 0 && root.snapshotsDir !== ""

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "🔍"
                            color: Theme.textMuted
                            font.pixelSize: 26
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: I18n.t("Снимки не найдены")
                            color: Theme.textMuted
                            font.pixelSize: 16
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: I18n.t("Попробуйте очистить фильтры или выбрать другой период")
                            color: Theme.textFaint
                            font.pixelSize: 12
                        }
                    }
                }

                Item {
                    anchors.fill: parent
                    visible: root.snapshotsDir === ""

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "📁"
                            color: Theme.textMuted
                            font.pixelSize: 26
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: I18n.t("Папка снимков не настроена")
                            color: Theme.textMuted
                            font.pixelSize: 16
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: I18n.t("Укажите путь в настройках аналитики")
                            color: Theme.textFaint
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: previewPopup
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        property bool maximized: false
        property real normalX: 0
        property real normalY: 0
        property real normalWidth: 0
        property real normalHeight: 0
        property real dragStartX: 0
        property real dragStartY: 0
        property real dragStartWindowX: 0
        property real dragStartWindowY: 0

        function resetGeometry() {
            if (maximized) return
            width = Math.min(1200, Math.max(600, root.width - 80))
            height = Math.min(800, Math.max(420, root.height - 80))
            x = Math.round((root.width - width) / 2)
            y = Math.round((root.height - height) / 2)
        }

        function toggleMaximize() {
            if (!maximized) {
                normalX = x
                normalY = y
                normalWidth = width
                normalHeight = height
                x = 20
                y = 20
                width = Math.max(600, root.width - 40)
                height = Math.max(420, root.height - 40)
                maximized = true
            } else {
                x = normalX
                y = normalY
                width = normalWidth
                height = normalHeight
                maximized = false
            }
        }

        onOpened: resetGeometry()

        Overlay.modal: Rectangle { color: "#00000099" }
        background: Rectangle { color: "#1b1b1b"; radius: 8; border.color: "#3a3a3a" }

        function currentItem() {
            if (root.currentIndex < 0 || root.currentIndex >= filteredModel.count) return null
            return filteredModel.get(root.currentIndex)
        }

        FocusScope {
            anchors.fill: parent
            focus: true

            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Left) {
                    previewPopup.previous()
                    event.accepted = true
                } else if (event.key === Qt.Key_Right) {
                    previewPopup.next()
                    event.accepted = true
                }
            }
        }

        function previous() {
            if (filteredModel.count === 0) return
            root.currentIndex = Math.max(0, root.currentIndex - 1)
        }

        function next() {
            if (filteredModel.count === 0) return
            root.currentIndex = Math.min(filteredModel.count - 1, root.currentIndex + 1)
        }

        Rectangle {
            id: previewTitleBar
            height: 38
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#2d2d30"
            z: 10

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    if (previewPopup.maximized) return
                    previewPopup.dragStartX = mouse.x
                    previewPopup.dragStartY = mouse.y
                    previewPopup.dragStartWindowX = previewPopup.x
                    previewPopup.dragStartWindowY = previewPopup.y
                }
                onPositionChanged: {
                    if (previewPopup.maximized) return
                    if (mouse.buttons & Qt.LeftButton) {
                        previewPopup.x = previewPopup.dragStartWindowX + (mouse.x - previewPopup.dragStartX)
                        previewPopup.y = previewPopup.dragStartWindowY + (mouse.y - previewPopup.dragStartY)
                    }
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 6
                spacing: 8

                Text {
                    color: "white"
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    text: {
                        var item = previewPopup.currentItem()
                        return item ? item.fileName : ""
                    }
                }

                Button {
                    text: "□"
                    flat: true
                    Layout.preferredWidth: 34
                    Layout.fillHeight: true
                    onClicked: previewPopup.toggleMaximize()
                    contentItem: Text { text: "□"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { color: parent.down ? "#444" : (parent.hovered ? "#3e3e40" : "transparent") }
                }

                Button {
                    text: "✕"
                    flat: true
                    Layout.preferredWidth: 34
                    Layout.fillHeight: true
                    onClicked: previewPopup.close()
                    contentItem: Text { text: "✕"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { color: parent.down ? "#c42b1c" : (parent.hovered ? "#e81123" : "transparent") }
                }
            }
        }

        Item {
            id: previewCanvas
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: infoBar.bottom
            anchors.bottom: filmstripBar.top
            anchors.margins: 12

            Image {
                id: previewImage
                anchors.fill: parent
                source: {
                    var item = previewPopup.currentItem()
                    return item ? item.fileUrl : ""
                }
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                mipmap: true
            }

            Item {
                id: imageFrame
                width: previewImage.paintedWidth > 0 ? previewImage.paintedWidth : previewCanvas.width
                height: previewImage.paintedHeight > 0 ? previewImage.paintedHeight : previewCanvas.height
                anchors.centerIn: previewCanvas
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: filmstripBar.top
            anchors.bottomMargin: 12
            spacing: 12

            Button {
                text: I18n.t("Назад")
                contentItem: Text {
                    text: parent.text
                    color: "#ddd"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.hovered ? "#2d2d2d" : "#252526"
                    radius: 4
                    border.color: "#3a3a3a"
                }
                onClicked: previewPopup.previous()
            }
            Button {
                text: I18n.t("Вперед")
                contentItem: Text {
                    text: parent.text
                    color: "#ddd"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.hovered ? "#2d2d2d" : "#252526"
                    radius: 4
                    border.color: "#3a3a3a"
                }
                onClicked: previewPopup.next()
            }
            Button {
                text: I18n.t("Открыть файл")
                contentItem: Text {
                    text: parent.text
                    color: "#ddd"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.hovered ? "#2d2d2d" : "#252526"
                    radius: 4
                    border.color: "#3a3a3a"
                }
                onClicked: {
                    var item = previewPopup.currentItem()
                    if (item) Qt.openUrlExternally(item.fileUrl)
                }
            }
            Button {
                text: I18n.t("Открыть клип")
                enabled: {
                    var item = previewPopup.currentItem()
                    return item && item.clipUrl && item.clipUrl !== "" && root.clipsDir !== ""
                }
                opacity: enabled ? 1 : 0.5
                contentItem: Text {
                    text: parent.text
                    color: "#ddd"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.hovered ? "#2d2d2d" : "#252526"
                    radius: 4
                    border.color: "#3a3a3a"
                }
                onClicked: {
                    var item = previewPopup.currentItem()
                    if (item && item.clipUrl) Qt.openUrlExternally(item.clipUrl)
                }
            }
        }

        Rectangle {
            id: infoBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: previewTitleBar.bottom
            height: 48
            color: "#111111cc"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Text {
                    color: "#ddd"
                    text: {
                        var item = previewPopup.currentItem()
                        if (!item) return ""
                        return (item.cameraId ? item.cameraId + " • " : "") +
                               (item.label ? item.label + " • " : "") +
                               (item.capturedAtText || "")
                    }
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Text {
                    color: "#888"
                    text: {
                        var item = previewPopup.currentItem()
                        if (!item) return ""
                        return (root.currentIndex + 1) + " / " + filteredModel.count
                    }
                }
            }
        }

        Rectangle {
            id: filmstripBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 100
            color: "#0f0f0fcc"

            ListView {
                id: filmstrip
                anchors.fill: parent
                anchors.margins: 8
                orientation: ListView.Horizontal
                spacing: 8
                clip: true
                model: filteredModel
                currentIndex: root.currentIndex

                delegate: Rectangle {
                    width: 120
                    height: 84
                    radius: 4
                    color: index === root.currentIndex ? "#3b82f6" : "#222"
                    border.color: "#333"
                    border.width: 1

                    Image {
                        anchors.fill: parent
                        anchors.margins: 2
                        source: fileUrl
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        mipmap: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.currentIndex = index
                        }
                    }
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            z: -1
            onClicked: previewPopup.close()
        }
    }

    ImageViewerWindow {
        id: imageViewer
    }

    Component.onCompleted: rebuildFilteredModel()
}
