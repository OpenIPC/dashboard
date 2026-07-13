import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property alias currentIndex: resultsList.currentIndex
    property alias selectedFile: resultsList.selectedFile
    property string currentCameraIp: ""
    property string defaultDownloadPath: ""
    property bool searchStarted: false
    property bool exactRangeExpanded: false
    property string sourceFilter: "all"
    property string sortMode: "newest"
    property var visibleResults: []

    readonly property int totalCount: {
        var results = SystemController.archiveController.searchResults
        return results && results.length !== undefined ? results.length : 0
    }

    signal fileSelected(var file, int index)
    signal folderRequested(var file)

    color: Theme.metroTile

    onCurrentCameraIpChanged: selectCameraIp(currentCameraIp)

    function selectCameraIp(cameraIp) {
        if (cameraIp === "") return
        for (var i = 0; i < cameraSelector.count; i++) {
            var cam = SystemController.cameraModel.getCamera(i)
            if (cam && cam.cameraIp === cameraIp) {
                cameraSelector.currentIndex = i
                break
            }
        }
    }

    function dateTimeText(date) {
        return date.toLocaleString(Qt.locale(), "yyyy-MM-dd HH:mm:ss")
    }

    function setQuickRange(days) {
        var end = new Date()
        end.setHours(23, 59, 59, 999)
        var start = new Date(end)
        start.setDate(start.getDate() - Math.max(1, days) + 1)
        start.setHours(0, 0, 0, 0)
        startTimeField.text = dateTimeText(start)
        endTimeField.text = dateTimeText(end)
    }

    function sourceMatches(file) {
        if (sourceFilter === "all") return true
        var source = file && file.source ? String(file.source) : "manual"
        return sourceFilter === source
    }

    function fileTimeMs(file) {
        if (!file || !file.startTime) return 0
        return new Date(file.startTime).getTime()
    }

    function refreshVisibleResults(clearSelection) {
        var source = SystemController.archiveController.searchResults || []
        var list = []

        for (var i = 0; i < source.length; i++) {
            var file = source[i]
            if (sourceMatches(file)) list.push(file)
        }

        list.sort(function(a, b) {
            return sortMode === "oldest" ? fileTimeMs(a) - fileTimeMs(b) : fileTimeMs(b) - fileTimeMs(a)
        })

        visibleResults = list
        if (clearSelection) {
            root.currentIndex = -1
            root.selectedFile = null
        }
    }

    function runSearch() {
        var camIndex = cameraSelector.currentIndex
        if (camIndex < 0) return

        var cam = SystemController.cameraModel.getCamera(camIndex)
        if (!cam) return

        SystemController.archiveController.login(cam.cameraIp, cam.cameraPort, cam.cameraLogin, "")

        var start = Date.fromLocaleString(Qt.locale(), startTimeField.text, "yyyy-MM-dd HH:mm:ss")
        var end = Date.fromLocaleString(Qt.locale(), endTimeField.text, "yyyy-MM-dd HH:mm:ss")
        var settings = SystemController.getAppSettings()
        var recPath = (settings && settings.recordingsPath) ? settings.recordingsPath : ""

        searchStarted = true
        root.currentIndex = -1
        root.selectedFile = null
        visibleResults = []
        SystemController.archiveController.search(start, end, cam.cameraIp, recPath)
    }

    onSourceFilterChanged: refreshVisibleResults(true)
    onSortModeChanged: refreshVisibleResults(true)

    Connections {
        target: SystemController.archiveController
        function onSearchResultsChanged() {
            root.refreshVisibleResults(true)
        }
    }

    Component.onCompleted: refreshVisibleResults(false)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        anchors.topMargin: 30
        spacing: 8

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            Label {
                text: I18n.t("Камера")
                color: Theme.textSecondary
                font.bold: true
            }

            StyledComboBox {
                id: cameraSelector
                Layout.fillWidth: true
                textRole: "cameraName"
                model: SystemController.cameraModel
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    Layout.fillWidth: true
                    text: I18n.t("Период")
                    color: Theme.textSecondary
                    font.bold: true
                }

                Button {
                    id: exactRangeButton
                    Layout.preferredWidth: 82
                    Layout.preferredHeight: 26
                    text: root.exactRangeExpanded ? I18n.t("Скрыть") : I18n.t("Точно")
                    onClicked: root.exactRangeExpanded = !root.exactRangeExpanded
                    background: Rectangle {
                        color: exactRangeButton.down ? Theme.metroTilePressed : Theme.metroTile
                        border.color: root.exactRangeExpanded ? Theme.metroBlue : Theme.metroStroke
                        radius: Theme.metroTileRadius
                    }
                    contentItem: Text {
                        text: exactRangeButton.text
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 5

                EmptyStateButton {
                    Layout.fillWidth: true
                    text: I18n.t("Сегодня")
                    onClicked: setQuickRange(1)
                }

                EmptyStateButton {
                    Layout.fillWidth: true
                    text: I18n.t("7 дней")
                    onClicked: setQuickRange(7)
                }

                EmptyStateButton {
                    Layout.fillWidth: true
                    text: I18n.t("30 дней")
                    onClicked: setQuickRange(30)
                }
            }

            Label {
                text: I18n.t("Начало")
                color: Theme.textMuted
                font.pixelSize: 11
                visible: root.exactRangeExpanded
            }

            RowLayout {
                Layout.fillWidth: true
                visible: root.exactRangeExpanded

                TextField {
                    id: startTimeField
                    Layout.fillWidth: true
                    text: new Date(new Date().setHours(0, 0, 0, 0)).toLocaleString(Qt.locale(), "yyyy-MM-dd HH:mm:ss")
                    color: Theme.textSecondary
                    background: Rectangle {
                        color: Theme.metroBackground
                        border.color: Theme.metroStroke
                        radius: Theme.metroTileRadius
                    }
                }

                Button {
                    id: startCalendarButton
                    text: "..."
                    Layout.preferredWidth: 30
                    background: Rectangle { color: Theme.metroTile; radius: Theme.metroTileRadius }
                    contentItem: Text {
                        text: startCalendarButton.text
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        calendarPopup.targetField = startTimeField
                        calendarPopup.open()
                    }
                }
            }

            Label {
                text: I18n.t("Конец")
                color: Theme.textMuted
                font.pixelSize: 11
                visible: root.exactRangeExpanded
            }

            RowLayout {
                Layout.fillWidth: true
                visible: root.exactRangeExpanded

                TextField {
                    id: endTimeField
                    Layout.fillWidth: true
                    text: new Date(new Date().setHours(23, 59, 59, 999)).toLocaleString(Qt.locale(), "yyyy-MM-dd HH:mm:ss")
                    color: Theme.textSecondary
                    background: Rectangle {
                        color: Theme.metroBackground
                        border.color: Theme.metroStroke
                        radius: Theme.metroTileRadius
                    }
                }

                Button {
                    id: endCalendarButton
                    text: "..."
                    Layout.preferredWidth: 30
                    background: Rectangle { color: Theme.metroTile; radius: Theme.metroTileRadius }
                    contentItem: Text {
                        text: endCalendarButton.text
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        calendarPopup.targetField = endTimeField
                        calendarPopup.open()
                    }
                }
            }
        }

        Button {
            id: archiveSearchButton
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            text: SystemController.archiveController.isSearching ? I18n.t("Поиск") + "..." : I18n.t("Найти")
            enabled: !SystemController.archiveController.isSearching

            background: Rectangle {
                color: archiveSearchButton.down ? Theme.metroBlueHover : Theme.metroBlue
                radius: Theme.metroTileRadius
                opacity: archiveSearchButton.enabled ? 1 : 0.5
            }

            contentItem: Text {
                text: archiveSearchButton.text
                color: Theme.textPrimary
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: runSearch()
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: I18n.t("Тип записи")
                color: Theme.textMuted
                font.pixelSize: 11
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 5

                EmptyStateButton {
                    Layout.fillWidth: true
                    text: I18n.t("Все")
                    buttonColor: root.sourceFilter === "all" ? Theme.metroBlue : Theme.metroTile
                    buttonHoverColor: root.sourceFilter === "all" ? Theme.metroBlueHover : Theme.metroTileHover
                    onClicked: root.sourceFilter = "all"
                }

                EmptyStateButton {
                    Layout.fillWidth: true
                    text: I18n.t("Ручные")
                    buttonColor: root.sourceFilter === "manual" ? Theme.metroBlue : Theme.metroTile
                    buttonHoverColor: root.sourceFilter === "manual" ? Theme.metroBlueHover : Theme.metroTileHover
                    onClicked: root.sourceFilter = "manual"
                }

                EmptyStateButton {
                    Layout.fillWidth: true
                    text: I18n.t("События")
                    buttonColor: root.sourceFilter === "event" ? Theme.metroBlue : Theme.metroTile
                    buttonHoverColor: root.sourceFilter === "event" ? Theme.metroBlueHover : Theme.metroTileHover
                    onClicked: root.sourceFilter = "event"
                }
            }

            StyledComboBox {
                id: sortCombo
                Layout.fillWidth: true
                model: [I18n.t("Сначала новые"), I18n.t("Сначала старые")]
                currentIndex: root.sortMode === "newest" ? 0 : 1
                onCurrentIndexChanged: root.sortMode = currentIndex === 0 ? "newest" : "oldest"
            }
        }

        ArchiveStoragePanel {
            id: storagePanel
            Layout.fillWidth: true
            Layout.preferredHeight: storagePanel.implicitHeight
            recordingsPath: root.defaultDownloadPath
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                Layout.fillWidth: true
                text: searchStarted
                      ? I18n.t("Показано: %1 / %2", [root.visibleResults.length, root.totalCount])
                      : I18n.t("Архив записей")
                color: Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            Text {
                text: defaultDownloadPath.length > 0 ? I18n.t("локально") : I18n.t("по умолчанию")
                color: Theme.textFaint
                font.pixelSize: 10
            }
        }

        ArchiveResultsList {
            id: resultsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            results: root.visibleResults
            isSearching: SystemController.archiveController.isSearching
            searchStarted: root.searchStarted
            onFileSelected: (file, index) => root.fileSelected(file, index)
            onFolderRequested: (file) => root.folderRequested(file)
        }
    }

    Popup {
        id: calendarPopup
        width: 300
        height: 320
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        property var targetField: null
        property date selectedDate: new Date()

        background: Rectangle {
            color: Theme.metroSidebarBackground
            border.color: Theme.metroBlue
            radius: Theme.metroTileRadius
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10

            RowLayout {
                Layout.fillWidth: true

                Button {
                    id: previousMonthButton
                    text: "<"
                    Layout.preferredWidth: 30
                    onClicked: {
                        var d = new Date(calendarPopup.selectedDate)
                        d.setMonth(d.getMonth() - 1)
                        calendarPopup.selectedDate = d
                    }
                    background: Rectangle { color: Theme.metroTile; radius: Theme.metroTileRadius }
                    contentItem: Text {
                        text: previousMonthButton.text
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Label {
                    text: calendarPopup.selectedDate.toLocaleString(Qt.locale(), "MMMM yyyy")
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.bold: true
                }

                Button {
                    id: nextMonthButton
                    text: ">"
                    Layout.preferredWidth: 30
                    onClicked: {
                        var d = new Date(calendarPopup.selectedDate)
                        d.setMonth(d.getMonth() + 1)
                        calendarPopup.selectedDate = d
                    }
                    background: Rectangle { color: Theme.metroTile; radius: Theme.metroTileRadius }
                    contentItem: Text {
                        text: nextMonthButton.text
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            DayOfWeekRow {
                Layout.fillWidth: true
                delegate: Text {
                    text: model.shortName
                    color: Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 12
                }
            }

            MonthGrid {
                Layout.fillWidth: true
                Layout.fillHeight: true
                month: calendarPopup.selectedDate.getMonth()
                year: calendarPopup.selectedDate.getFullYear()

                delegate: ItemDelegate {
                    id: calendarDayDelegate
                    text: model.day
                    highlighted: {
                        var d = model.date
                        return d.getDate() === calendarPopup.selectedDate.getDate()
                            && d.getMonth() === calendarPopup.selectedDate.getMonth()
                            && d.getFullYear() === calendarPopup.selectedDate.getFullYear()
                    }

                    onClicked: {
                        calendarPopup.selectedDate = model.date

                        var currentText = calendarPopup.targetField.text
                        var timePart = "00:00:00"
                        if (currentText.includes(" ")) {
                            timePart = currentText.split(" ")[1]
                        }

                        var newDateStr = model.date.toLocaleString(Qt.locale(), "yyyy-MM-dd")
                        calendarPopup.targetField.text = newDateStr + " " + timePart
                        calendarPopup.close()
                    }

                    contentItem: Text {
                        text: calendarDayDelegate.text
                        color: calendarDayDelegate.highlighted ? Theme.textPrimary : Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: calendarDayDelegate.highlighted ? Theme.metroBlue : "transparent"
                        radius: Theme.metroTileRadius
                    }
                }
            }
        }
    }
}
