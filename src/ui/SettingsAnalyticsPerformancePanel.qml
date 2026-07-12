pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: panel

    property var settings: null
    readonly property bool compactLayout: width < 620

    Layout.fillWidth: true
    spacing: 10

    Text {
        text: I18n.t("Производительность аналитики")
        color: "white"
        font.pixelSize: 18
        font.bold: true
    }

    Text {
        Layout.fillWidth: true
        text: I18n.t("Ограничьте частоту обработки и количество параллельных задач, чтобы камеры не перегружали CPU/GPU.")
        color: Theme.textMuted
        font.pixelSize: 13
        wrapMode: Text.WordWrap
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Text {
            text: I18n.t("Пресет нагрузки")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.preferredWidth: panel.compactLayout ? 150 : 220
            Layout.minimumWidth: 0
        }

        Repeater {
            model: ["eco", "balanced", "max"]

            Button {
                id: analyticsPresetButton

                required property string modelData

                focusPolicy: Qt.StrongFocus
                Layout.fillWidth: true
                Layout.minimumWidth: 72
                Layout.preferredWidth: panel.compactLayout ? 92 : 132
                Layout.preferredHeight: 34
                text: panel.settings ? panel.settings.analyticsPresetLabel(analyticsPresetButton.modelData) : analyticsPresetButton.modelData
                hoverEnabled: true
                onClicked: if (panel.settings) panel.settings.applyAnalyticsPreset(analyticsPresetButton.modelData)

                background: Rectangle {
                    color: panel.settings && panel.settings.analyticsPerformancePreset === analyticsPresetButton.modelData
                           ? Theme.accent
                           : (analyticsPresetButton.hovered ? Theme.cardHover : Theme.controlBackground)
                    radius: Theme.radiusSm
                    border.color: analyticsPresetButton.visualFocus
                                  ? Theme.textPrimary
                                  : panel.settings && panel.settings.analyticsPerformancePreset === analyticsPresetButton.modelData
                                    ? Theme.accentHover
                                    : Theme.controlBorder
                    border.width: analyticsPresetButton.visualFocus ? 2 : 1
                }

                contentItem: Text {
                    text: analyticsPresetButton.text
                    color: Theme.textPrimary
                    font.pixelSize: 12
                    font.bold: panel.settings && panel.settings.analyticsPerformancePreset === analyticsPresetButton.modelData
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
        }

        Item { Layout.fillWidth: true }
    }

    Text {
        Layout.fillWidth: true
        Layout.leftMargin: panel.compactLayout ? 0 : 232
        text: panel.settings
              ? I18n.t("Текущий режим: %1", [panel.settings.analyticsPresetLabel(panel.settings.analyticsPerformancePreset)])
                + " · "
                + panel.settings.analyticsPresetDescription(panel.settings.analyticsPerformancePreset)
              : ""
        color: Theme.textMuted
        font.pixelSize: 11
        wrapMode: Text.WordWrap
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Text {
            text: I18n.t("FPS аналитики")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.preferredWidth: panel.compactLayout ? 150 : 220
            Layout.minimumWidth: 0
        }

        SettingsSpinBox {
            from: 1
            to: 15
            value: panel.settings ? panel.settings.analyticsTargetFps : 3
            Layout.preferredWidth: 110
            onValueModified: {
                if (!panel.settings) return
                panel.settings.analyticsPerformancePreset = "custom"
                panel.settings.analyticsTargetFps = value
                panel.settings.applyCurrentSettings()
            }
        }

        Text {
            text: I18n.t("%1 кадр/с", [panel.settings ? panel.settings.analyticsTargetFps : 3])
            color: Theme.textMuted
            Layout.preferredWidth: 90
        }

        Item { Layout.fillWidth: true }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Text {
            text: I18n.t("Параллельные задачи")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.preferredWidth: panel.compactLayout ? 150 : 220
            Layout.minimumWidth: 0
        }

        SettingsSpinBox {
            from: 1
            to: 8
            value: panel.settings ? panel.settings.analyticsMaxParallelJobs : 2
            Layout.preferredWidth: 110
            onValueModified: {
                if (!panel.settings) return
                panel.settings.analyticsPerformancePreset = "custom"
                panel.settings.analyticsMaxParallelJobs = value
                panel.settings.applyCurrentSettings()
            }
        }

        Text {
            text: I18n.t("до %1 задач", [panel.settings ? panel.settings.analyticsMaxParallelJobs : 2])
            color: Theme.textMuted
            Layout.preferredWidth: 120
        }

        Item { Layout.fillWidth: true }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        color: Theme.metroStroke
    }
}
