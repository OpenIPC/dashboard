pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: layoutDialog

    property string dialogTitle: I18n.t("Редактор шаблонов")
    property var presets: []
    property string selectedPresetId: "grid-4"
    property string layoutName: ""

    property int editorBaseRows: 6
    property int editorBaseCols: 6
    property var editorCells: []
    property point selectionStart: Qt.point(-1, -1)
    property point selectionEnd: Qt.point(-1, -1)
    property bool isSelecting: false

    signal customAccepted(string name, int rows, int cols, var cells)
    signal presetAccepted(string name, string presetId)

    modal: true
    focus: true
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    padding: 0
    width: parent ? Math.min(parent.width - 96, 1000) : 1000
    height: parent ? Math.min(parent.height - 96, 680) : 680
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: Theme.radiusXl
        color: Theme.panelAltBackground
        border.color: Theme.panelBorderStrong
        border.width: 1
    }

    function openEditor(title, presetId, name, rows, cols, savedCells) {
        dialogTitle = title || I18n.t("Редактор шаблонов")
        selectedPresetId = presetId || "grid-4"
        layoutName = name || ""

        if (selectedPresetId === "custom" && savedCells && savedCells.length > 0) {
            loadEditorFromCells(rows || 5, cols || 8, savedCells)
        } else {
            initEditor(rows || 5, cols || 8)
        }

        selectionStart = Qt.point(-1, -1)
        selectionEnd = Qt.point(-1, -1)
        open()
    }

    function initEditor(rows, cols) {
        editorBaseRows = rows
        editorBaseCols = cols
        var cells = []
        for (var r = 0; r < rows; r++) {
            for (var c = 0; c < cols; c++) {
                cells[cells.length] = { r: r, c: c, spanR: 1, spanC: 1, visible: true }
            }
        }
        editorCells = cells
        selectionStart = Qt.point(-1, -1)
        selectionEnd = Qt.point(-1, -1)
    }

    function loadEditorFromCells(rows, cols, savedCells) {
        var cells = []
        var covered = []
        for (var k = 0; k < rows * cols; k++) {
            covered[k] = false
        }

        var cellIdx = 0
        for (var r = 0; r < rows; r++) {
            for (var c = 0; c < cols; c++) {
                var flatIdx = r * cols + c
                if (covered[flatIdx]) {
                    cells[cells.length] = { r: r, c: c, spanR: 1, spanC: 1, visible: false }
                    continue
                }

                var spanR = 1
                var spanC = 1
                if (savedCells && cellIdx < savedCells.length) {
                    var sc = savedCells[cellIdx]
                    spanR = sc.rowSpan || 1
                    spanC = sc.colSpan || 1
                    cellIdx++
                }

                cells[cells.length] = { r: r, c: c, spanR: spanR, spanC: spanC, visible: true }

                for (var rr = 0; rr < spanR; rr++) {
                    for (var cc = 0; cc < spanC; cc++) {
                        var targetR = r + rr
                        var targetC = c + cc
                        if (targetR < rows && targetC < cols) {
                            covered[targetR * cols + targetC] = true
                        }
                    }
                }
            }
        }

        editorBaseRows = rows
        editorBaseCols = cols
        editorCells = cells
        selectionStart = Qt.point(-1, -1)
        selectionEnd = Qt.point(-1, -1)
    }

    function getCellIndex(r, c) {
        return r * editorBaseCols + c
    }

    function resetEditor() {
        initEditor(editorBaseRows, editorBaseCols)
    }

    function unmergeSelection() {
        if (selectionStart.x < 0) {
            return
        }

        var r = Math.min(selectionStart.y, selectionEnd.y)
        var c = Math.min(selectionStart.x, selectionEnd.x)
        var idx = getCellIndex(r, c)
        var cell = editorCells[idx]

        if (!cell || !cell.visible) {
            return
        }
        if (cell.spanR === 1 && cell.spanC === 1) {
            return
        }

        var newCells = editorCells.slice()
        for (var rr = 0; rr < cell.spanR; rr++) {
            for (var cc = 0; cc < cell.spanC; cc++) {
                var targetIdx = getCellIndex(r + rr, c + cc)
                newCells[targetIdx].visible = true
                newCells[targetIdx].spanR = 1
                newCells[targetIdx].spanC = 1
            }
        }

        editorCells = newCells
        selectionEnd = selectionStart
    }

    function mergeSelection() {
        if (selectionStart.x < 0) {
            return
        }

        var r1 = Math.min(selectionStart.y, selectionEnd.y)
        var r2 = Math.max(selectionStart.y, selectionEnd.y)
        var c1 = Math.min(selectionStart.x, selectionEnd.x)
        var c2 = Math.max(selectionStart.x, selectionEnd.x)

        var spanR = r2 - r1 + 1
        var spanC = c2 - c1 + 1
        if (spanR === 1 && spanC === 1) {
            return
        }

        var newCells = editorCells.slice()

        for (var i = 0; i < newCells.length; i++) {
            var cell = newCells[i]
            if (!cell.visible) {
                continue
            }

            var cellR1 = cell.r
            var cellR2 = cell.r + cell.spanR - 1
            var cellC1 = cell.c
            var cellC2 = cell.c + cell.spanC - 1
            var interR1 = Math.max(r1, cellR1)
            var interR2 = Math.min(r2, cellR2)
            var interC1 = Math.max(c1, cellC1)
            var interC2 = Math.min(c2, cellC2)

            if (interR1 <= interR2 && interC1 <= interC2) {
                if (cellR1 < r1 || cellR2 > r2 || cellC1 < c1 || cellC2 > c2) {
                    for (var rr = cellR1; rr <= cellR2; rr++) {
                        for (var cc = cellC1; cc <= cellC2; cc++) {
                            var idx = getCellIndex(rr, cc)
                            newCells[idx].visible = true
                            newCells[idx].spanR = 1
                            newCells[idx].spanC = 1
                        }
                    }
                }
            }
        }

        var mainIdx = getCellIndex(r1, c1)
        newCells[mainIdx].spanR = spanR
        newCells[mainIdx].spanC = spanC
        newCells[mainIdx].visible = true

        for (var r = r1; r <= r2; r++) {
            for (var c = c1; c <= c2; c++) {
                if (r === r1 && c === c1) {
                    continue
                }
                var hiddenIdx = getCellIndex(r, c)
                newCells[hiddenIdx].visible = false
                newCells[hiddenIdx].spanR = 1
                newCells[hiddenIdx].spanC = 1
            }
        }

        editorCells = newCells
        selectionStart = Qt.point(-1, -1)
        selectionEnd = Qt.point(-1, -1)
    }

    function exportLayout() {
        var cells = []
        for (var i = 0; i < editorCells.length; i++) {
            if (editorCells[i].visible) {
                cells.push({
                    rowSpan: editorCells[i].spanR,
                    colSpan: editorCells[i].spanC
                })
            }
        }
        return cells
    }

    function normalizedCustomLayoutName() {
        var name = nameField.text.trim()
        if (name.length === 0) {
            name = I18n.t("Польз. план")
        }
        return name
    }

    function acceptLayout() {
        if (selectedPresetId === "custom") {
            customAccepted(normalizedCustomLayoutName(), editorBaseRows, editorBaseCols, exportLayout())
        } else {
            presetAccepted(nameField.text.trim(), selectedPresetId)
        }
        close()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: Theme.panelBackground

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 22
                anchors.rightMargin: 18
                spacing: 10

                Text {
                    text: layoutDialog.dialogTitle.toUpperCase()
                    color: Theme.textMuted
                    font.pixelSize: 13
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    radius: 15
                    color: Theme.panelSoftBackground
                    border.color: Theme.controlBorder

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: Theme.textSecondary
                        font.pixelSize: 14
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: layoutDialog.close()
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Theme.panelBorder
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 230
                Layout.fillHeight: true
                color: Theme.panelBackground

                ListView {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 6
                    clip: true
                    model: layoutDialog.presets

                    delegate: Rectangle {
                        id: presetDelegate

                        required property var modelData

                        width: ListView.view.width
                        height: 42
                        color: layoutDialog.selectedPresetId === presetDelegate.modelData.id ? Theme.accent : (presetHover.containsMouse ? Theme.cardHover : "transparent")
                        radius: Theme.radiusMd
                        border.color: layoutDialog.selectedPresetId === presetDelegate.modelData.id ? Theme.accentHover : "transparent"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 10
                            spacing: 12

                            Rectangle {
                                Layout.preferredWidth: 22
                                Layout.preferredHeight: 22
                                radius: Theme.radiusXs
                                color: "transparent"
                                border.color: layoutDialog.selectedPresetId === presetDelegate.modelData.id ? Theme.textPrimary : Theme.controlBorderStrong
                                border.width: 1

                                Grid {
                                    anchors.centerIn: parent
                                    columns: Math.min(presetDelegate.modelData.cols, 3)
                                    rows: Math.min(presetDelegate.modelData.rows, 3)
                                    spacing: 1

                                    Repeater {
                                        model: Math.min(presetDelegate.modelData.cols * presetDelegate.modelData.rows, 9)

                                        Rectangle {
                                            width: 4
                                            height: 4
                                            color: layoutDialog.selectedPresetId === presetDelegate.modelData.id ? Theme.textPrimary : Theme.textMuted
                                        }
                                    }
                                }
                            }

                            Text {
                                property var modelData: presetDelegate.modelData

                                text: modelData.type === "complex" ? modelData.label : I18n.t("%1 ячеек", [modelData.cells.length > 0 ? modelData.cells.length : (modelData.rows * modelData.cols)])
                                color: layoutDialog.selectedPresetId === modelData.id ? Theme.textPrimary : Theme.textMuted
                                font.pixelSize: 13
                                font.bold: layoutDialog.selectedPresetId === modelData.id
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        MouseArea {
                            id: presetHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: layoutDialog.selectedPresetId = presetDelegate.modelData.id
                        }
                    }

                    footer: Rectangle {
                        width: parent.width
                        height: 42
                        color: layoutDialog.selectedPresetId === "custom" ? Theme.accent : "transparent"
                        radius: Theme.radiusMd
                        border.color: layoutDialog.selectedPresetId === "custom" ? Theme.accentHover : "transparent"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 10
                            spacing: 12

                            Text {
                                text: "?"
                                color: layoutDialog.selectedPresetId === "custom" ? Theme.textPrimary : Theme.textMuted
                                font.bold: true
                            }

                            Text {
                                text: I18n.t("Свой")
                                color: layoutDialog.selectedPresetId === "custom" ? Theme.textPrimary : Theme.textMuted
                                font.pixelSize: 13
                                font.bold: layoutDialog.selectedPresetId === "custom"
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: layoutDialog.selectedPresetId = "custom"
                        }
                    }
                }

                Rectangle {
                    anchors.right: parent.right
                    width: 1
                    height: parent.height
                    color: Theme.panelBorder
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "transparent"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        spacing: 10

                        Text {
                            text: I18n.t("Ряд")
                            color: Theme.textMuted
                            font.pixelSize: 13
                        }

                        TextField {
                            id: customRowsField
                            text: String(layoutDialog.editorBaseRows)
                            color: Theme.textPrimary
                            horizontalAlignment: TextInput.AlignHCenter
                            verticalAlignment: TextInput.AlignVCenter
                            Layout.preferredWidth: 74
                            Layout.preferredHeight: 32
                            background: Rectangle {
                                color: Theme.controlBackground
                                border.color: customRowsField.activeFocus ? Theme.accent : Theme.controlBorder
                                radius: Theme.radiusMd
                            }
                            onEditingFinished: {
                                var val = parseInt(text)
                                if (val > 0 && val <= 12) {
                                    layoutDialog.initEditor(val, layoutDialog.editorBaseCols)
                                } else {
                                    text = String(layoutDialog.editorBaseRows)
                                }
                            }
                        }

                        Text {
                            text: I18n.t("Столбцов")
                            color: Theme.textMuted
                            font.pixelSize: 13
                        }

                        TextField {
                            id: customColsField
                            text: String(layoutDialog.editorBaseCols)
                            color: Theme.textPrimary
                            horizontalAlignment: TextInput.AlignHCenter
                            verticalAlignment: TextInput.AlignVCenter
                            Layout.preferredWidth: 74
                            Layout.preferredHeight: 32
                            background: Rectangle {
                                color: Theme.controlBackground
                                border.color: customColsField.activeFocus ? Theme.accent : Theme.controlBorder
                                radius: Theme.radiusMd
                            }
                            onEditingFinished: {
                                var val = parseInt(text)
                                if (val > 0 && val <= 12) {
                                    layoutDialog.initEditor(layoutDialog.editorBaseRows, val)
                                } else {
                                    text = String(layoutDialog.editorBaseCols)
                                }
                            }
                        }

                        Text {
                            property real ratio: (layoutDialog.editorBaseCols / layoutDialog.editorBaseRows) / (16 / 9)
                            text: "AR: " + ratio.toFixed(2) + " (1.0 = 16:9)"
                            color: (ratio > 0.8 && ratio < 1.2) ? Theme.success : Theme.danger
                            font.pixelSize: 11
                        }

                        Item { Layout.fillWidth: true }

                        DashboardDialogButton {
                            text: I18n.t("Сброс")
                            Layout.preferredWidth: 78
                            Layout.preferredHeight: 32
                            onClicked: layoutDialog.resetEditor()
                        }

                        DashboardDialogButton {
                            text: I18n.t("Разбить")
                            enabled: {
                                if (layoutDialog.selectionStart.x < 0) {
                                    return false
                                }
                                if (layoutDialog.selectionStart.x !== layoutDialog.selectionEnd.x || layoutDialog.selectionStart.y !== layoutDialog.selectionEnd.y) {
                                    return false
                                }
                                var idx = layoutDialog.getCellIndex(layoutDialog.selectionStart.y, layoutDialog.selectionStart.x)
                                var cell = layoutDialog.editorCells[idx]
                                return cell && (cell.spanR > 1 || cell.spanC > 1)
                            }
                            Layout.preferredWidth: 90
                            Layout.preferredHeight: 32
                            buttonBorderColor: Theme.controlBorderStrong
                            onClicked: layoutDialog.unmergeSelection()
                        }

                        DashboardDialogButton {
                            text: I18n.t("Объединить")
                            enabled: layoutDialog.selectionStart.x >= 0
                            Layout.preferredWidth: 112
                            Layout.preferredHeight: 32
                            buttonColor: Theme.accent
                            buttonHoverColor: Theme.accentHover
                            buttonBorderColor: Theme.accentHover
                            buttonTextBold: true
                            onClicked: layoutDialog.mergeSelection()
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Theme.controlBackground
                        border.color: Theme.panelBorderStrong
                        border.width: 1
                        radius: Theme.radiusLg

                        Item {
                            id: gridContainer
                            anchors.centerIn: parent
                            width: Math.min(parent.width - 40, (parent.height - 40) * (layoutDialog.editorBaseCols / layoutDialog.editorBaseRows))
                            height: width * (layoutDialog.editorBaseRows / layoutDialog.editorBaseCols)

                            property real cellW: width / layoutDialog.editorBaseCols
                            property real cellH: height / layoutDialog.editorBaseRows

                            Repeater {
                                model: layoutDialog.editorCells

                                delegate: Rectangle {
                                    id: editorCellDelegate

                                    required property int index
                                    required property var modelData

                                    visible: modelData.visible
                                    x: modelData.c * gridContainer.cellW
                                    y: modelData.r * gridContainer.cellH
                                    width: modelData.spanC * gridContainer.cellW
                                    height: modelData.spanR * gridContainer.cellH
                                    color: Theme.cardBackground
                                    border.color: Theme.panelBorderStrong
                                    border.width: 1

                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        color: Theme.panelAltBackground
                                        border.color: Theme.panelBorder

                                        Text {
                                            property int index: editorCellDelegate.index

                                            anchors.centerIn: parent
                                            text: (index + 1)
                                            color: Theme.controlBorderStrong
                                            font.pixelSize: Math.min(parent.width, parent.height) * 0.4
                                            font.bold: true
                                        }

                                        Rectangle {
                                            property var modelData: editorCellDelegate.modelData

                                            visible: modelData.spanR > 1 || modelData.spanC > 1
                                            anchors.fill: parent
                                            color: "transparent"
                                            border.color: Theme.accent
                                            border.width: 2
                                            opacity: 0.5
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                visible: layoutDialog.selectionStart.x >= 0
                                x: Math.min(layoutDialog.selectionStart.x, layoutDialog.selectionEnd.x) * gridContainer.cellW
                                y: Math.min(layoutDialog.selectionStart.y, layoutDialog.selectionEnd.y) * gridContainer.cellH
                                width: (Math.abs(layoutDialog.selectionEnd.x - layoutDialog.selectionStart.x) + 1) * gridContainer.cellW
                                height: (Math.abs(layoutDialog.selectionEnd.y - layoutDialog.selectionStart.y) + 1) * gridContainer.cellH
                                color: Theme.accent
                                opacity: 0.3
                                border.color: Theme.accentHover
                                border.width: 2
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true

                                function getGridPos(mouse) {
                                    var c = Math.floor(mouse.x / gridContainer.cellW)
                                    var r = Math.floor(mouse.y / gridContainer.cellH)
                                    c = Math.max(0, Math.min(c, layoutDialog.editorBaseCols - 1))
                                    r = Math.max(0, Math.min(r, layoutDialog.editorBaseRows - 1))
                                    return Qt.point(c, r)
                                }

                                onPressed: (mouse) => {
                                    layoutDialog.selectedPresetId = "custom"
                                    var pos = getGridPos(mouse)
                                    layoutDialog.selectionStart = pos
                                    layoutDialog.selectionEnd = pos
                                    layoutDialog.isSelecting = true
                                }

                                onPositionChanged: (mouse) => {
                                    if (layoutDialog.isSelecting) {
                                        layoutDialog.selectionEnd = getGridPos(mouse)
                                    }
                                }

                                onReleased: {
                                    layoutDialog.isSelecting = false
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Text {
                            text: I18n.t("Имя:")
                            color: Theme.textMuted
                            font.pixelSize: 13
                        }

                        TextField {
                            id: nameField
                            text: layoutDialog.layoutName
                            Layout.fillWidth: true
                            placeholderText: I18n.t("Название раскладки")
                            Layout.preferredHeight: 32
                            color: Theme.textPrimary
                            verticalAlignment: TextInput.AlignVCenter
                            onTextChanged: layoutDialog.layoutName = text
                            background: Rectangle {
                                color: Theme.controlBackground
                                border.color: nameField.activeFocus ? Theme.accent : Theme.controlBorder
                                radius: Theme.radiusMd
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: Theme.panelBackground

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.panelBorder
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 10

                Item { Layout.fillWidth: true }

                DashboardDialogButton {
                    text: I18n.t("Отмена")
                    Layout.preferredWidth: 92
                    Layout.preferredHeight: 34
                    onClicked: layoutDialog.close()
                }

                DashboardDialogButton {
                    text: I18n.t("OK")
                    Layout.preferredWidth: 74
                    Layout.preferredHeight: 34
                    buttonColor: Theme.accent
                    buttonHoverColor: Theme.accentHover
                    buttonBorderColor: Theme.accentHover
                    buttonTextBold: true
                    onClicked: layoutDialog.acceptLayout()
                }
            }
        }
    }
}
