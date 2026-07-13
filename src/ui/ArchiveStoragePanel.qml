import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property string recordingsPath: ""
    property var summary: ({})
    property var cleanupPreview: ({})

    color: Theme.metroSurfaceAlt
    border.color: Theme.metroStroke
    radius: Theme.metroTileRadius
    implicitHeight: 188

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
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                Layout.fillWidth: true
                text: I18n.t("Хранилище")
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 13
                elide: Text.ElideRight
            }

            Button {
                id: refreshButton
                Layout.preferredWidth: 74
                Layout.preferredHeight: 26
                text: I18n.t("Обновить")
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
            text: previewText()
            color: cleanupPreview && cleanupPreview.error ? Theme.warningText : Theme.textMuted
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        RowLayout {
            Layout.fillWidth: true
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
