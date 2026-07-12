import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: root

    property var model // AnalyticsEngine
    property int moduleType: 0
    property string moduleName: ""
    property int selectedRuleIndex: -1
    property bool savingRules: false
    readonly property bool hasSelection: selectedRuleIndex >= 0 && selectedRuleIndex < rulesModel.count
    readonly property real ruleListWidth: Math.max(240, Math.min(320, width * 0.32))

    function availableLabels() {
        if (moduleType === 0)
            return ["Face"]
        if (moduleType === 1)
            return ["person", "bicycle", "car", "motorcycle", "bus", "truck"]
        if (moduleType === 2)
            return ["License Plate"]
        return ["person"]
    }

    function defaultLabel() {
        var labels = availableLabels()
        return labels.length > 0 ? labels[0] : "person"
    }

    function loadRules() {
        selectedRuleIndex = -1
        rulesModel.clear()
        if (!model || !model.getModuleConfig)
            return

        var cfg = model.getModuleConfig(moduleType)
        var list = cfg && cfg.rules ? cfg.rules : []
        for (var i = 0; i < list.length; ++i) {
            var r = list[i]
            rulesModel.append({
                id: (r.id !== undefined ? String(r.id) : String(Date.now() + i)),
                name: r.name || (I18n.t("Правило") + " " + (i + 1)),
                label: r.label || defaultLabel(),
                minConfidence: (r.minConfidence !== undefined ? Number(r.minConfidence) : 0.6),
                cooldownMs: (r.cooldownMs !== undefined ? Number(r.cooldownMs) : 5000),
                enabled: (r.enabled !== undefined ? !!r.enabled : true),
                zonePreset: r.zonePreset || "full",
                actionSnapshot: (r.actionSnapshot !== undefined ? !!r.actionSnapshot : true),
                actionClip: (r.actionClip !== undefined ? !!r.actionClip : true),
                actionNotify: (r.actionNotify !== undefined ? !!r.actionNotify : false)
            })
        }

        selectedRuleIndex = rulesModel.count > 0 ? 0 : -1
    }

    function normalizedRule(index) {
        if (index < 0 || index >= rulesModel.count)
            return {}

        var r = rulesModel.get(index)
        return {
            id: String(r.id || Date.now()),
            name: String(r.name || I18n.t("Правило")),
            label: String(r.label || defaultLabel()),
            minConfidence: Number(r.minConfidence !== undefined ? r.minConfidence : 0.6),
            cooldownMs: Math.max(1000, Number(r.cooldownMs !== undefined ? r.cooldownMs : 5000)),
            enabled: r.enabled !== false,
            zonePreset: String(r.zonePreset || "full"),
            actionSnapshot: r.actionSnapshot !== false,
            actionClip: r.actionClip !== false,
            actionNotify: r.actionNotify === true
        }
    }

    function selectedValue(field, fallback) {
        if (!hasSelection)
            return fallback

        var r = rulesModel.get(selectedRuleIndex)
        if (!r || r[field] === undefined)
            return fallback

        return r[field]
    }

    function persistRules() {
        if (!model || !model.setModuleConfig)
            return

        var out = []
        for (var i = 0; i < rulesModel.count; ++i) {
            out.push(normalizedRule(i))
        }
        savingRules = true
        model.setModuleConfig(moduleType, { "rules": out })
        savingRules = false
    }

    Component.onCompleted: loadRules()
    onModuleTypeChanged: loadRules()

    function addRule() {
        var idx = rulesModel.count + 1
        rulesModel.append({
            id: String(Date.now()),
            name: I18n.t("Правило") + " " + idx,
            label: defaultLabel(),
            minConfidence: 0.6,
            cooldownMs: 5000,
            enabled: true,
            zonePreset: "full",
            actionSnapshot: true,
            actionClip: true,
            actionNotify: false
        })
        selectedRuleIndex = rulesModel.count - 1
        persistRules()
    }

    function removeSelectedRule() {
        if (!hasSelection) return
        rulesModel.remove(selectedRuleIndex)
        if (rulesModel.count === 0) {
            selectedRuleIndex = -1
        } else {
            selectedRuleIndex = Math.max(0, Math.min(selectedRuleIndex, rulesModel.count - 1))
        }
        persistRules()
    }

    function updateField(field, value) {
        if (!hasSelection) return
        rulesModel.setProperty(selectedRuleIndex, field, value)
        persistRules()
    }

    Connections {
        target: root.model
        ignoreUnknownSignals: true
        function onModuleConfigChanged(type) {
            if (type === root.moduleType && !root.savingRules) {
                root.loadRules()
            }
        }
    }

    ListModel { id: rulesModel }

    component PanelButton: Button {
        implicitHeight: 30
        leftPadding: 12
        rightPadding: 12

        background: Rectangle {
            color: parent.enabled
                   ? (parent.hovered ? Theme.cardHover : Theme.controlBackground)
                   : Theme.controlBackgroundAlt
            radius: Theme.radiusSm
            border.color: parent.enabled ? Theme.controlBorder : Theme.panelBorder
            border.width: 1
        }

        contentItem: Text {
            text: parent.text
            color: parent.enabled ? Theme.textSecondary : Theme.textMuted
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component CompactSpinBox: SpinBox {
        id: spin
        implicitHeight: 34
        editable: true

        background: Rectangle {
            color: Theme.controlBackground
            radius: Theme.radiusSm
            border.color: spin.activeFocus ? Theme.accent : Theme.controlBorder
            border.width: 1
        }

        contentItem: TextInput {
            text: spin.textFromValue(spin.value, spin.locale)
            color: spin.enabled ? Theme.textSecondary : Theme.textMuted
            font: spin.font
            selectionColor: Theme.accent
            selectedTextColor: Theme.textPrimary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            readOnly: !spin.editable
            validator: spin.validator
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }
    }

    component RuleCheckBox: MetroCheckBox {
        implicitHeight: 24
        font.pixelSize: 13
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        Text {
            text: I18n.t("Правила аналитики") + (root.moduleName !== "" ? (" — " + root.moduleName) : "")
            color: "white"
            font.pixelSize: 18
            font.bold: true
        }

        Text {
            text: I18n.t("Блочный редактор: настройте три блока для каждого правила — Триггер, Зона, Действия.")
            color: "#9ca3af"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Rectangle {
                Layout.preferredWidth: root.ruleListWidth
                Layout.minimumWidth: 240
                Layout.fillHeight: true
                color: Theme.metroSurfaceAlt
                radius: 8
                border.color: Theme.metroStroke

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true

                        Text {
                            text: I18n.t("Список правил")
                            color: Theme.textSecondary
                            font.bold: true
                            font.pixelSize: 13
                        }

                        Item { Layout.fillWidth: true }

                        PanelButton {
                            text: I18n.t("Добавить")
                            onClicked: addRule()
                        }

                        PanelButton {
                            text: I18n.t("Удалить")
                            enabled: root.hasSelection
                            onClicked: removeSelectedRule()
                        }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 6
                        model: rulesModel

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 56
                            radius: 8
                            color: index === root.selectedRuleIndex ? "#1e293b" : Theme.metroSurfaceAlt
                            border.color: index === root.selectedRuleIndex ? Theme.metroBlue : Theme.metroStroke

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 8

                                RuleCheckBox {
                                    checked: model.enabled
                                    onToggled: {
                                        rulesModel.setProperty(index, "enabled", checked)
                                        root.persistRules()
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Text {
                                        text: model.name
                                        color: Theme.textSecondary
                                        font.bold: true
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        text: model.label + "  ·  ≥ " + Math.round(Number(model.minConfidence) * 100) + "%"
                                              + "  ·  " + Math.max(1, Math.round(Number(model.cooldownMs || 5000) / 1000)) + "s"
                                        color: Theme.textMuted
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.selectedRuleIndex = index
                            }
                        }
                    }

                    Text {
                        visible: rulesModel.count === 0
                        text: I18n.t("Пока нет правил")
                        color: Theme.textFaint
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            ScrollView {
                id: rulesEditorScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: availableWidth

                ColumnLayout {
                    width: rulesEditorScroll.availableWidth
                    spacing: 10

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: Theme.metroSurfaceAlt
                        border.color: Theme.metroStroke
                        implicitHeight: triggerBlock.implicitHeight + 16

                        ColumnLayout {
                            id: triggerBlock
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8

                            Text {
                                text: I18n.t("Блок 1 — Триггер")
                                color: Theme.textSecondary
                                font.bold: true
                            }

                            TextField {
                                Layout.fillWidth: true
                                enabled: root.hasSelection
                                text: root.selectedValue("name", "")
                                placeholderText: I18n.t("Название правила")
                                color: "white"
                                background: Rectangle { color: Theme.metroSurfaceAlt; border.color: Theme.metroStroke; radius: 4 }
                                onEditingFinished: root.updateField("name", text.trim() === "" ? I18n.t("Правило") : text.trim())
                            }

                            RowLayout {
                                id: triggerControls
                                Layout.fillWidth: true
                                spacing: 8

                                StyledComboBox {
                                    id: labelBox
                                    Layout.preferredWidth: 320
                                    Layout.maximumWidth: 360
                                    enabled: root.hasSelection
                                    model: root.availableLabels()
                                    currentIndex: {
                                        if (!root.hasSelection)
                                            return 0
                                        var labels = root.availableLabels()
                                        var lbl = root.selectedValue("label", root.defaultLabel())
                                        var idx = labels.indexOf(lbl)
                                        return idx >= 0 ? idx : 0
                                    }
                                    onUserSelected: root.updateField("label", currentText)
                                }

                                CompactSpinBox {
                                    id: confSpin
                                    enabled: root.hasSelection
                                    from: 10
                                    to: 99
                                    value: root.hasSelection ? Math.round(Number(root.selectedValue("minConfidence", 0.6)) * 100) : 60
                                    Layout.preferredWidth: 140
                                    textFromValue: function(v) { return v + "%" }
                                    valueFromText: function(t) {
                                        var n = parseInt(String(t).replace("%", "").trim())
                                        if (isNaN(n)) return 60
                                        return Math.max(10, Math.min(99, n))
                                    }
                                    onValueModified: root.updateField("minConfidence", value / 100.0)
                                }

                                CompactSpinBox {
                                    id: cooldownSpin
                                    enabled: root.hasSelection
                                    from: 1
                                    to: 60
                                    value: root.hasSelection ? Math.max(1, Math.round(Number(root.selectedValue("cooldownMs", 5000)) / 1000)) : 5
                                    Layout.preferredWidth: 140
                                    textFromValue: function(v) { return v + "s" }
                                    valueFromText: function(t) {
                                        var n = parseInt(String(t).replace("s", "").trim())
                                        if (isNaN(n)) return 5
                                        return Math.max(1, Math.min(60, n))
                                    }
                                    onValueModified: root.updateField("cooldownMs", value * 1000)
                                }

                                Item { Layout.fillWidth: true }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: Theme.metroSurfaceAlt
                        border.color: Theme.metroStroke
                        implicitHeight: zoneBlock.implicitHeight + 16

                        ColumnLayout {
                            id: zoneBlock
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8

                            Text {
                                text: I18n.t("Блок 2 — Зона")
                                color: Theme.metroGreen
                                font.bold: true
                            }

                            StyledComboBox {
                                id: zoneBox
                                Layout.fillWidth: true
                                enabled: root.hasSelection
                                model: ["full", "center", "left", "right", "top", "bottom"]
                                currentIndex: {
                                    if (!root.hasSelection)
                                        return 0
                                    var z = root.selectedValue("zonePreset", "full")
                                    var i = model.indexOf(z)
                                    return i >= 0 ? i : 0
                                }
                                onUserSelected: root.updateField("zonePreset", currentText)
                            }

                            Text {
                                text: I18n.t("Скоро вернём полигональные зоны; пока пресеты зоны уже сохраняются в правиле.")
                                color: Theme.textMuted
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                font.pixelSize: 11
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: Theme.metroSurfaceAlt
                        border.color: Theme.metroStroke
                        implicitHeight: actionsBlock.implicitHeight + 16

                        ColumnLayout {
                            id: actionsBlock
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8

                            Text {
                                text: I18n.t("Блок 3 — Действия")
                                color: Theme.metroRed
                                font.bold: true
                            }

                            RuleCheckBox {
                                enabled: root.hasSelection
                                checked: root.hasSelection ? !!root.selectedValue("actionSnapshot", true) : true
                                text: I18n.t("Сохранить снимок")
                                onToggled: root.updateField("actionSnapshot", checked)
                            }

                            RuleCheckBox {
                                enabled: root.hasSelection
                                checked: root.hasSelection ? !!root.selectedValue("actionClip", true) : true
                                text: I18n.t("Сохранить клип")
                                onToggled: root.updateField("actionClip", checked)
                            }

                            RuleCheckBox {
                                enabled: root.hasSelection
                                checked: root.hasSelection ? !!root.selectedValue("actionNotify", false) : false
                                text: I18n.t("Уведомление")
                                onToggled: root.updateField("actionNotify", checked)
                            }
                        }
                    }
                }
            }
        }
    }
}
