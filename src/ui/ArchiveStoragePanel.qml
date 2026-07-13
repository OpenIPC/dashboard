import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property string recordingsPath: ""
    property var summary: ({})
    property var cleanupPreview: ({})
    property bool expanded: false

    color: Theme.metroSurfaceAlt
    border.color: Theme.metroStroke
    radius: Theme.metroTileRadius
    implicitHeight: expanded ? 188 : 58
    clip: true

    function bytesLimit() {
        if (maxSizeSpin.value <= 0) return 0
        return maxSizeSpin.value * 1024 * 1024 * 1024
    }

    function refreshSummary() {
        summary = SystemController.archiveController.storageSummary(recordingsPath)
    }

    function previewCleanup() {
        cleanupPreview = SystemController.archiveController.cleanupRecordings(
                    recordingsPath, keepDaysSpin.value, bytesLimit(), true)
    }

    function applyCleanup() {
        cleanupPreview = SystemController.archiveController.cleanupRecordings(
                    recordingsPath, keepDaysSpin.value, bytesLimit(), false)
        refreshSummary()
    }

    function valueText(key, fallbackText) {
        if (!summary || summary[key] === undefined || summary[key] === null) return fallbackText
        return String(summary[key])
    }

    function previewText() {
        if (!cleanupPreview || cleanupPreview.wouldDeleteCount === undefined) return I18n.t("Сначала рассчитайте план очистки")
        if (cleanupPreview.error && String(cleanupPreview.error).length > 0) return String(cleanupPreview.error)
        var count = cleanupPreview.dryRun ? cleanupPreview.wouldDeleteCount : cleanupPreview.deletedCount
        var size = cleanupPreview.dryRun ? cleanupPreview.wouldDeleteSizeText : cleanupPreview.deletedSizeText
        return cleanupPreview.dryRun
                ? I18n.t("Будет удалено: %1 файлов, %2", [count, size])
                : I18n.t("Удалено: %1 файлов, %2", [count, size])
    }

    function shortPath(path) {
        var text = String(path || "")
        if (text.length <= 42) return text
        return "..." + text.slice(text.length - 39)
    }

    Component.onCompleted: refreshSummary()
    onRecordingsPathChanged: refreshSummary()

    Connections {
        target: SystemController.archiveController
        function onCleanupFinished(result) {
            root.cleanupPreview = result
            root.refreshSummary()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: root.expanded ? 8 : 4

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Text {
                    Layout.fillWidth: true
                    text: I18n.t("Хранилище")
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: valueText("totalSizeText", "0 B") + I18n.t(" · файлов ") + valueText("fileCount", "0")
                    color: Theme.textMuted
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
            }

            Button {
                id: expandButton
                Layout.preferredWidth: 32
                Layout.preferredHeight: 28
                padding: 0
                hoverEnabled: true
                onClicked: root.expanded = !root.expanded
                background: Rectangle {
                    color: expandButton.down ? Theme.metroTilePressed
                           : (expandButton.hovered ? Theme.metroTileHover : Theme.metroTile)
                    border.color: root.expanded ? Theme.metroBlue : Theme.metroStroke
                    radius: Theme.metroTileRadius
                }
                contentItem: SidebarIcon {
                    name: "menu"
                    fallbackText: "\u2261"
                    color: root.expanded || expandButton.hovered ? Theme.textPrimary : Theme.textSecondary
                    pixelSize: 20
                }
                ToolTip.visible: hovered
                ToolTip.delay: 350
                ToolTip.text: I18n.t(root.expanded ? "Свернуть хранилище" : "Развернуть хранилище")
            }

            Button {
                id: refreshButton
                Layout.preferredWidth: root.expanded ? 74 : 30
                Layout.preferredHeight: 26
                text: root.expanded ? I18n.t("Обновить") : "R"
                onClicked: root.refreshSummary()
                background: Rectangle {
                    color: refreshButton.down ? Theme.metroTilePressed : Theme.metroTile
                    border.color: Theme.metroStroke
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: refreshButton.text
                    color: Theme.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            visible: root.expanded
            columns: 2
            columnSpacing: 8
            rowSpacing: 5

            Text {
                text: I18n.t("Размер")
                color: Theme.textMuted
                font.pixelSize: 10
            }
            Text {
                Layout.fillWidth: true
                text: valueText("totalSizeText", "0 B")
                color: Theme.textPrimary
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                text: I18n.t("Файлы")
                color: Theme.textMuted
                font.pixelSize: 10
            }
            Text {
                Layout.fillWidth: true
                text: valueText("fileCount", "0") + I18n.t("  | ручн. ") + valueText("manualCount", "0") + I18n.t(" / событ. ") + valueText("eventCount", "0")
                color: Theme.textSecondary
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            Text {
                text: I18n.t("Путь")
                color: Theme.textMuted
                font.pixelSize: 10
            }
            Text {
                Layout.fillWidth: true
                text: shortPath(valueText("rootPath", ""))
                color: Theme.textFaint
                font.pixelSize: 10
                elide: Text.ElideMiddle
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: root.expanded
            spacing: 6

            Text {
                text: I18n.t("Дней")
                color: Theme.textMuted
                font.pixelSize: 10
            }

            SpinBox {
                id: keepDaysSpin
                from: 0
                to: 3650
                value: 30
                editable: true
                Layout.preferredWidth: 76
                Layout.preferredHeight: 30
            }

            Text {
                text: I18n.t("ГБ")
                color: Theme.textMuted
                font.pixelSize: 10
            }

            SpinBox {
                id: maxSizeSpin
                from: 0
                to: 10000
                value: 0
                editable: true
                Layout.preferredWidth: 76
                Layout.preferredHeight: 30
            }
        }

        Text {
            Layout.fillWidth: true
            visible: root.expanded
            text: previewText()
            color: cleanupPreview && cleanupPreview.error ? Theme.warningText : Theme.textMuted
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        RowLayout {
            Layout.fillWidth: true
            visible: root.expanded
            spacing: 6

            Button {
                id: planButton
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                text: I18n.t("План")
                onClicked: root.previewCleanup()
                background: Rectangle {
                    color: planButton.down ? Theme.metroBlueHover : Theme.metroTile
                    border.color: Theme.metroBlue
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: planButton.text
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 11
                }
            }

            Button {
                id: cleanupButton
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                text: I18n.t("Очистить")
                enabled: cleanupPreview && cleanupPreview.wouldDeleteCount > 0 && summary && summary.safe
                onClicked: root.applyCleanup()
                background: Rectangle {
                    color: cleanupButton.enabled
                           ? (cleanupButton.down ? Theme.dangerSurfacePressed : Theme.dangerSurface)
                           : Theme.metroTileDisabled
                    border.color: cleanupButton.enabled ? Theme.danger : Theme.metroStroke
                    radius: Theme.metroTileRadius
                }
                contentItem: Text {
                    text: cleanupButton.text
                    color: cleanupButton.enabled ? Theme.textPrimary : Theme.textFaint
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }
        }
    }
}
