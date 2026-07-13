import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root

    property var results: []
    property bool isSearching: false
    property bool searchStarted: false
    property int currentIndex: -1
    property var selectedFile: null

    readonly property int count: results && results.length !== undefined ? results.length : 0

    signal fileSelected(var file, int index)
    signal folderRequested(var file)

    onResultsChanged: {
        if (currentIndex >= count) {
            currentIndex = -1
            selectedFile = null
        }
    }

    function fileSizeText(bytes) {
        var value = Number(bytes || 0)
        var units = ["B", "KB", "MB", "GB", "TB"]
        var index = 0
        while (value >= 1024 && index < units.length - 1) {
            value = value / 1024
            index++
        }
        return value.toFixed(value >= 10 || index === 0 ? 0 : 1) + " " + units[index]
    }

    function durationText(ms) {
        var value = Number(ms || 0)
        if (value <= 0) return ""
        var totalSeconds = Math.floor(value / 1000)
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        return (minutes < 10 ? "0" : "") + minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }

    function timeText(file) {
        if (!file || !file.startTime || !file.endTime) return ""
        return file.startTime.toLocaleString(Qt.locale(), "HH:mm:ss")
            + " - " + file.endTime.toLocaleString(Qt.locale(), "HH:mm:ss")
    }

    function metaText(file) {
        var text = ""
        var time = timeText(file)
        if (time.length > 0) text = time
        if (file && (file.sizeBytes || file.size)) {
            if (text.length > 0) text += " / "
            text += fileSizeText(file.sizeBytes || file.size)
        }
        if (file && file.durationMs) {
            if (text.length > 0) text += " / "
            text += durationText(file.durationMs)
        }
        return text
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.metroBackground
        border.color: Theme.metroTile
        radius: Theme.metroTileRadius
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: 1
        clip: true
        model: root.results
        currentIndex: root.currentIndex

        section.property: "dateKey"
        section.criteria: ViewSection.FullString
        section.delegate: Rectangle {
            id: sectionHeader
            required property string section
            width: ListView.view.width
            height: 28
            color: Theme.metroSurfaceAlt
            border.color: Theme.metroStroke

            Text {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                text: sectionHeader.section.length > 0 ? sectionHeader.section : I18n.t("Без даты")
                color: Theme.textSecondary
                font.pixelSize: 11
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }

        delegate: ItemDelegate {
            width: ListView.view.width
            height: 70
            highlighted: root.currentIndex === index

            background: Rectangle {
                color: highlighted ? Theme.metroBlue : (hovered ? Theme.metroTileHover : Theme.metroTile)
                border.color: highlighted ? Theme.metroStrokeStrong : Theme.metroStroke
                border.width: highlighted ? 2 : 1
            }

            contentItem: RowLayout {
                spacing: 8

                Rectangle {
                    Layout.preferredWidth: 4
                    Layout.fillHeight: true
                    color: modelData.source === "event" ? Theme.metroAmber : Theme.metroBlue
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            Layout.fillWidth: true
                            text: modelData.fileName
                            color: Theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                            elide: Text.ElideMiddle
                        }

                        Rectangle {
                            Layout.preferredWidth: sourceText.implicitWidth + 12
                            Layout.preferredHeight: 20
                            color: modelData.source === "event" ? Theme.warningSurfaceSoft : Theme.metroDeepBlue
                            border.color: modelData.source === "event" ? Theme.metroAmber : Theme.metroBlue
                            radius: Theme.metroTileRadius

                            Text {
                                id: sourceText
                                anchors.centerIn: parent
                                text: modelData.source === "event" ? I18n.t("Событие") : I18n.t("Ручная")
                                color: modelData.source === "event" ? Theme.warningText : Theme.textSecondary
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: metaText(modelData)
                        color: Theme.textMuted
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                }

                Button {
                    id: folderButton
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    hoverEnabled: true
                    background: Rectangle {
                        color: folderButton.hovered ? Theme.metroTileHover : "transparent"
                        border.color: folderButton.hovered ? Theme.metroStrokeStrong : Theme.metroStroke
                        radius: Theme.metroTileRadius
                    }
                    contentItem: SidebarIcon {
                        anchors.centerIn: parent
                        width: 20
                        height: 20
                        color: Theme.textSecondary
                        name: "folder_open"
                        path: "M10 4H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2h-8l-2-2z"
                    }
                    onClicked: root.folderRequested(modelData)
                }
            }

            onClicked: {
                root.currentIndex = index
                root.selectedFile = modelData
                root.fileSelected(modelData, index)
            }
        }

        ScrollBar.vertical: ScrollBar { }
    }

    Rectangle {
        anchors.fill: parent
        visible: root.count === 0
        color: Theme.metroBackground
        border.color: Theme.metroTile
        radius: Theme.metroTileRadius

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 32, 240)
            spacing: 10

            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                running: root.isSearching
                visible: root.isSearching
            }

            Text {
                Layout.fillWidth: true
                text: root.isSearching
                      ? I18n.t("Идет поиск записей...")
                      : (root.searchStarted
                         ? I18n.t("Записи не найдены")
                         : I18n.t("Выберите камеру и период"))
                color: Theme.textSecondary
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Text {
                Layout.fillWidth: true
                text: root.searchStarted
                      ? I18n.t("Попробуйте другой период или проверьте папку записей.")
                      : I18n.t("После поиска здесь появятся записи архива.")
                color: Theme.textMuted
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
        }
    }
}
