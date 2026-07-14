pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: panel

    property var settings: null
    property string iconFontFamily: "Material Icons"
    readonly property bool compactLayout: width < 620

    signal snapshotsFolderRequested()
    signal clipsFolderRequested()

    component EvidenceToggle: RowLayout {
        id: option

        property string title: ""
        property string description: ""
        property bool optionChecked: false
        property bool optionEnabled: true

        signal optionToggled(bool checked)

        Layout.fillWidth: true
        spacing: 10

        MetroCheckBox {
            id: toggle

            text: ""
            checked: option.optionChecked
            enabled: option.optionEnabled
            ToolTip.visible: hovered || visualFocus
            ToolTip.text: option.description
            onToggled: option.optionToggled(checked)
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: option.title
                color: option.optionEnabled ? Theme.textPrimary : Theme.textFaint
                font.pixelSize: 13
                font.bold: true
                wrapMode: Text.WordWrap
            }

            Text {
                Layout.fillWidth: true
                text: option.description
                color: option.optionEnabled ? Theme.textSecondary : Theme.textFaint
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
        }
    }

    Layout.fillWidth: true
    spacing: 16

    Text {
        text: I18n.t("События аналитики")
        color: "white"
        font.pixelSize: 18
        font.bold: true
    }

    EvidenceToggle {
        title: I18n.t("Записывать события аналитики")
        description: I18n.t("Включает постоянную ленту событий: детекции проходят через правила и сохраняются в журнале и архиве.")
        optionChecked: panel.settings ? panel.settings.evidenceEnabled : false
        onOptionToggled: function(checked) {
            if (!panel.settings) return
            panel.settings.evidenceEnabled = checked
            panel.settings.applyCurrentSettings()
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8

        EvidenceToggle {
            title: I18n.t("Сохранять снимки детекций")
            description: I18n.t("Сохраняет кадр PNG при срабатывании правила или общей детекции.")
            optionChecked: panel.settings ? panel.settings.evidenceSnapshotsEnabled : true
            optionEnabled: panel.settings ? panel.settings.evidenceEnabled : false
            onOptionToggled: function(checked) {
                if (!panel.settings) return
                panel.settings.evidenceSnapshotsEnabled = checked
                panel.settings.applyCurrentSettings()
            }
        }

        EvidenceToggle {
            title: I18n.t("Сохранять клипы детекций")
            description: I18n.t("Сохраняет короткий MP4-фрагмент вокруг события с учетом времени до и после события.")
            optionChecked: panel.settings ? panel.settings.evidenceClipsEnabled : true
            optionEnabled: panel.settings ? panel.settings.evidenceEnabled : false
            onOptionToggled: function(checked) {
                if (!panel.settings) return
                panel.settings.evidenceClipsEnabled = checked
                panel.settings.applyCurrentSettings()
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Text {
            text: I18n.t("Папка снимков (детекции)")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.preferredWidth: panel.compactLayout ? 170 : 220
            Layout.minimumWidth: 0
        }
        TextField {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            text: panel.settings ? panel.settings.evidenceSnapshotsPath : ""
            color: Theme.textPrimary
            placeholderText: I18n.t("Выберите папку")
            placeholderTextColor: Theme.textMuted
            selectionColor: Theme.metroBlue
            selectedTextColor: "white"
            background: Rectangle {
                color: Theme.metroSurfaceAlt
                radius: 4
                border.color: Theme.metroStroke
                border.width: 1
            }
            onEditingFinished: {
                if (!panel.settings) return
                panel.settings.evidenceSnapshotsPath = text
                panel.settings.applyCurrentSettings()
            }
        }
        Button {
            id: snapshotsFolderButton

            focusPolicy: Qt.StrongFocus
            Layout.preferredHeight: 30
            Layout.preferredWidth: 34
            ToolTip.visible: hovered || visualFocus
            ToolTip.text: I18n.t("Выберите папку")
            background: Rectangle {
                color: snapshotsFolderButton.hovered ? Theme.metroTileHover : Theme.metroStroke
                border.color: snapshotsFolderButton.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                border.width: snapshotsFolderButton.visualFocus ? 2 : 1
                radius: Theme.metroTileRadius
            }
            contentItem: Text {
                text: "folder_open"
                font.family: panel.iconFontFamily
                font.pixelSize: 15
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: panel.snapshotsFolderRequested()
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Text {
            text: I18n.t("Папка клипов (детекции)")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.preferredWidth: panel.compactLayout ? 170 : 220
            Layout.minimumWidth: 0
        }
        TextField {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            text: panel.settings ? panel.settings.evidenceClipsPath : ""
            color: Theme.textPrimary
            placeholderText: I18n.t("Выберите папку")
            placeholderTextColor: Theme.textMuted
            selectionColor: Theme.metroBlue
            selectedTextColor: "white"
            background: Rectangle {
                color: Theme.metroSurfaceAlt
                radius: 4
                border.color: Theme.metroStroke
                border.width: 1
            }
            onEditingFinished: {
                if (!panel.settings) return
                panel.settings.evidenceClipsPath = text
                panel.settings.applyCurrentSettings()
            }
        }
        Button {
            id: clipsFolderButton

            focusPolicy: Qt.StrongFocus
            Layout.preferredHeight: 30
            Layout.preferredWidth: 34
            ToolTip.visible: hovered || visualFocus
            ToolTip.text: I18n.t("Выберите папку")
            background: Rectangle {
                color: clipsFolderButton.hovered ? Theme.metroTileHover : Theme.metroStroke
                border.color: clipsFolderButton.visualFocus ? Theme.metroStrokeStrong : Theme.metroStroke
                border.width: clipsFolderButton.visualFocus ? 2 : 1
                radius: Theme.metroTileRadius
            }
            contentItem: Text {
                text: "folder_open"
                font.family: panel.iconFontFamily
                font.pixelSize: 15
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: panel.clipsFolderRequested()
        }
    }

    GridLayout {
        columns: panel.compactLayout ? 2 : 4
        columnSpacing: 12
        rowSpacing: 8

        Text {
            text: I18n.t("До события (сек)")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: panel.compactLayout
            Layout.preferredWidth: panel.compactLayout ? 150 : 180
            Layout.minimumWidth: 0
        }
        SettingsSpinBox {
            from: 0
            to: 30
            value: panel.settings ? panel.settings.evidencePreSeconds : 5
            Layout.preferredWidth: 110
            onValueModified: {
                if (!panel.settings) return
                panel.settings.evidencePreSeconds = value
                panel.settings.applyCurrentSettings()
            }
        }

        Text {
            text: I18n.t("После события (сек)")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.fillWidth: panel.compactLayout
            Layout.preferredWidth: panel.compactLayout ? 150 : 180
            Layout.minimumWidth: 0
        }
        SettingsSpinBox {
            from: 0
            to: 30
            value: panel.settings ? panel.settings.evidencePostSeconds : 5
            Layout.preferredWidth: 110
            onValueModified: {
                if (!panel.settings) return
                panel.settings.evidencePostSeconds = value
                panel.settings.applyCurrentSettings()
            }
        }
    }

    RowLayout {
        spacing: 12

        Text {
            text: I18n.t("Минимальная уверенность")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            Layout.preferredWidth: panel.compactLayout ? 170 : 220
            Layout.minimumWidth: 0
        }
        MetroSlider {
            id: evidenceMinConfidenceSlider
            Layout.fillWidth: true
            Layout.minimumWidth: 80
            from: 0.1
            to: 0.95
            value: panel.settings ? panel.settings.evidenceMinConfidence : 0.6
            onMoved: if (panel.settings) panel.settings.evidenceMinConfidence = value
            onPressedChanged: {
                if (!pressed && panel.settings) panel.settings.applyCurrentSettings()
            }
        }
        Text {
            text: Math.round((panel.settings ? panel.settings.evidenceMinConfidence : 0.6) * 100) + "%"
            color: Theme.textSecondary
            Layout.preferredWidth: 50
        }
    }

    RowLayout {
        spacing: 12

        Text {
            text: I18n.t("FPS клипа")
            color: Theme.textSecondary
            Layout.preferredWidth: panel.compactLayout ? 170 : 220
            Layout.minimumWidth: 0
        }
        SettingsSpinBox {
            from: 5
            to: 25
            value: panel.settings ? panel.settings.evidenceClipFps : 10
            Layout.preferredWidth: 110
            onValueModified: {
                if (!panel.settings) return
                panel.settings.evidenceClipFps = value
                panel.settings.applyCurrentSettings()
            }
        }
    }

    SettingsEvidenceUploadPanel {
        settings: panel.settings
    }

    Item { Layout.fillHeight: true }
}
