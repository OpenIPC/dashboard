import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: editor
    required property var controller
    property var field: ({})
    property bool compact: false
    property bool dirty: { editor.controller.revision; return !editor.controller.equal(editor.field.value, editor.controller.draftValues[editor.field.path]) }
    property int rowHeight: editor.field.type === "array" ? 118 : (editor.compact ? 66 : 86)

    function arrayRows() {
        var values = editor.controller.arrayValue(editor.field)
        var rows = []
        for (var i = 0; i < values.length; ++i) {
            rows[i] = { index: i, value: values[i], controller: editor.controller, field: editor.field }
        }
        return rows
    }

    Layout.fillWidth: true
    Layout.preferredHeight: editor.rowHeight
    color: editor.dirty ? Theme.metroTilePressed : "transparent"
    border.color: editor.dirty ? Theme.metroBlue : "transparent"
    radius: Theme.metroTileRadius

    RowLayout {
        anchors.fill: parent
        anchors.margins: editor.compact ? 6 : 10
        spacing: 12

        ColumnLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: editor.compact ? 150 : 270
            spacing: 3

            RowLayout {
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    text: editor.compact ? editor.controller.liveFieldLabel(editor.field) : editor.controller.localizedFieldTitle(editor.field)
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: editor.compact ? 12 : 13
                    elide: Text.ElideRight
                }
                Rectangle {
                    visible: editor.field.live === true
                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 18
                    radius: Theme.metroTileRadius
                    color: Theme.infoSurface
                    Text { anchors.centerIn: parent; text: "LIVE"; color: Theme.infoText; font.pixelSize: 8; font.bold: true }
                }
                Rectangle {
                    visible: editor.dirty
                    Layout.preferredWidth: 70
                    Layout.preferredHeight: 18
                    radius: Theme.metroTileRadius
                    color: Theme.changedSurface
                    Text { anchors.centerIn: parent; text: I18n.t("изменено"); color: Theme.warningText; font.pixelSize: 9; font.bold: true }
                }
            }
            Text {
                visible: !editor.compact
                text: editor.field.path
                color: Theme.metroBlue
                font.family: "Consolas"
                font.pixelSize: 10
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Text {
                visible: !editor.compact
                Layout.fillWidth: true
                text: editor.controller.localizedFieldHint(editor.field)
                color: Theme.textMuted
                font.pixelSize: 10
                elide: Text.ElideRight
                maximumLineCount: 2
            }
            RowLayout {
                visible: !editor.compact
                spacing: 6
                Text { text: editor.field.type; color: Theme.textFaint; font.pixelSize: 10 }
                Text { visible: editor.field.minimum !== undefined && editor.field.maximum !== undefined; text: editor.field.minimum + " … " + editor.field.maximum; color: Theme.textFaint; font.pixelSize: 10 }
                Text { visible: editor.field.live !== true; text: I18n.t("требует reload"); color: Theme.metroAmber; font.family: Theme.metroFontFamily; font.pixelSize: 10 }
            }
        }

        MajesticCheckBox {
            visible: editor.field.type === "boolean"
            Layout.preferredWidth: visible ? 48 : 0
            text: ""
            checked: { editor.controller.revision; return editor.controller.boolValue(editor.controller.draftValues[editor.field.path]) }
            onToggled: if (visible && checked !== editor.controller.boolValue(editor.controller.draftValues[editor.field.path])) editor.controller.updateDraft(editor.field, checked)
        }

        RowLayout {
            visible: editor.controller.isRangeField(editor.field)
            Layout.preferredWidth: visible ? (editor.compact ? 290 : 360) : 0
            MetroSlider {
                id: rangeSlider
                Layout.fillWidth: true
                from: Number(editor.field.minimum)
                to: Number(editor.field.maximum)
                stepSize: editor.field.step !== undefined ? Number(editor.field.step) : 1
                value: Number(editor.controller.draftValues[editor.field.path])
                onMoved: editor.controller.updateDraft(editor.field, editor.field.type === "integer" ? Math.round(value) : value)
            }
            Text {
                text: String(editor.controller.draftValues[editor.field.path])
                color: Theme.textSecondary
                font.pixelSize: 12
                Layout.preferredWidth: 34
                horizontalAlignment: Text.AlignRight
            }
        }

        MajesticComboBox {
            visible: editor.field.enumValues && editor.field.enumValues.length > 0
            Layout.preferredWidth: visible ? (editor.compact ? 210 : 260) : 0
            model: editor.field.enumValues || []
            currentIndex: editor.controller.enumIndex(editor.field)
            onActivated: function(index) { editor.controller.updateDraft(editor.field, model[index]) }
        }

        MajesticComboBox {
            visible: editor.controller.isResolutionField(editor.field) && (!editor.field.enumValues || editor.field.enumValues.length === 0)
            Layout.preferredWidth: visible ? 260 : 0
            model: editor.controller.resolutionValues(editor.field)
            currentIndex: editor.controller.resolutionIndex(editor.field)
            onActivated: function(index) { editor.controller.updateDraft(editor.field, model[index]) }
        }

        MajesticTextField {
            visible: editor.field.type !== "boolean"
                     && !editor.controller.isRangeField(editor.field)
                     && !editor.controller.isResolutionField(editor.field)
                     && editor.field.type !== "array"
                     && (!editor.field.enumValues || editor.field.enumValues.length === 0)
            Layout.preferredWidth: visible ? (editor.compact ? 230 : 300) : 0
            text: { editor.controller.revision; return editor.controller.valueText(editor.field, editor.controller.draftValues[editor.field.path]) }
            echoMode: editor.field.sensitive ? TextInput.Password : TextInput.Normal
            placeholderText: editor.field.minimum !== undefined && editor.field.maximum !== undefined ? editor.field.minimum + " … " + editor.field.maximum : ""
            onEditingFinished: editor.controller.updateDraft(editor.field, editor.controller.parsedValue(editor.field, text))
        }

        ColumnLayout {
            visible: editor.field.type === "array"
            Layout.preferredWidth: visible ? 360 : 0
            spacing: 4
            Repeater {
                model: editor.arrayRows()
                delegate: RowLayout {
                    id: arrayDelegate
                    required property var modelData
                    MajesticTextField {
                        Layout.fillWidth: true
                        text: String(arrayDelegate.modelData.value)
                        placeholderText: "XxYxWxH"
                        onEditingFinished: arrayDelegate.modelData.controller.updateArrayValue(arrayDelegate.modelData.field, arrayDelegate.modelData.index, text)
                    }
                    MajesticButton {
                        text: "×"
                        danger: true
                        onClicked: arrayDelegate.modelData.controller.removeArrayValue(arrayDelegate.modelData.field, arrayDelegate.modelData.index)
                    }
                }
            }
            MajesticButton {
                text: I18n.t("+ Добавить")
                subtle: true
                onClicked: editor.controller.addArrayValue(editor.field)
            }
        }

        MajesticButton {
            text: "↺"
            subtle: true
            ToolTip.visible: hovered
            ToolTip.text: I18n.t("Сбросить к значению прошивки")
            enabled: editor.controller.capabilities.resetDefaults === true && editor.field.hasDefault === true && !editor.controller.loading
            onClicked: editor.controller.requestReset(editor.field.path)
        }
    }
}
