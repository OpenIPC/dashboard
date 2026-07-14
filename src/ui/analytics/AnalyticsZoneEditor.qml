import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

ColumnLayout {
    id: root

    property string polygonJson: ""
    property bool loading: false
    readonly property int minimumPoints: 3
    readonly property int maximumPoints: 8

    signal polygonEdited(string json)

    function defaultPoints() {
        return [
            { "x": 0.18, "y": 0.18 },
            { "x": 0.82, "y": 0.18 },
            { "x": 0.82, "y": 0.82 },
            { "x": 0.18, "y": 0.82 }
        ]
    }

    function parsePoints(value) {
        if (!value || value.trim() === "")
            return defaultPoints()
        try {
            var parsed = JSON.parse(value)
            if (!Array.isArray(parsed) || parsed.length < minimumPoints)
                return defaultPoints()
            var result = []
            for (var i = 0; i < parsed.length && i < maximumPoints; ++i) {
                var point = parsed[i]
                if (!point || !isFinite(Number(point.x)) || !isFinite(Number(point.y)))
                    return defaultPoints()
                result.push({
                    "x": Math.max(0, Math.min(1, Number(point.x))),
                    "y": Math.max(0, Math.min(1, Number(point.y)))
                })
            }
            return result
        } catch (error) {
            return defaultPoints()
        }
    }

    function loadPolygon() {
        loading = true
        pointsModel.clear()
        var points = parsePoints(polygonJson)
        for (var i = 0; i < points.length; ++i)
            pointsModel.append({ "px": points[i].x, "py": points[i].y })
        loading = false
        zoneCanvas.requestPaint()
    }

    function serializePolygon() {
        var points = []
        for (var i = 0; i < pointsModel.count; ++i) {
            var point = pointsModel.get(i)
            points[points.length] = ({
                "x": Math.round(Number(point.px) * 1000) / 1000,
                "y": Math.round(Number(point.py) * 1000) / 1000
            })
        }
        return JSON.stringify(points)
    }

    function publishPolygon() {
        if (loading)
            return
        zoneCanvas.requestPaint()
        polygonEdited(serializePolygon())
    }

    onPolygonJsonChanged: loadPolygon()
    Component.onCompleted: loadPolygon()

    spacing: 8

    Rectangle {
        id: editorSurface

        Layout.fillWidth: true
        Layout.preferredHeight: Math.max(190, Math.min(300, width * 0.34))
        color: Theme.controlBackground
        border.color: Theme.controlBorder
        border.width: 1
        radius: Theme.radiusSm
        clip: true

        Canvas {
            id: gridCanvas
            anchors.fill: parent

            onPaint: {
                var context = getContext("2d")
                context.reset()
                context.strokeStyle = Theme.controlBorder
                context.lineWidth = 1
                for (var i = 1; i < 4; ++i) {
                    var x = width * i / 4
                    var y = height * i / 4
                    context.beginPath()
                    context.moveTo(x, 0)
                    context.lineTo(x, height)
                    context.stroke()
                    context.beginPath()
                    context.moveTo(0, y)
                    context.lineTo(width, y)
                    context.stroke()
                }
            }
        }

        Canvas {
            id: zoneCanvas
            anchors.fill: parent

            onPaint: {
                var context = getContext("2d")
                context.reset()
                if (pointsModel.count < root.minimumPoints)
                    return

                context.beginPath()
                var first = pointsModel.get(0)
                context.moveTo(first.px * width, first.py * height)
                for (var i = 1; i < pointsModel.count; ++i) {
                    var point = pointsModel.get(i)
                    context.lineTo(point.px * width, point.py * height)
                }
                context.closePath()
                context.fillStyle = Qt.rgba(0.12, 0.42, 0.94, 0.22)
                context.strokeStyle = Theme.accent
                context.lineWidth = 2
                context.fill()
                context.stroke()
            }
        }

        Repeater {
            model: ListModel { id: pointsModel }

            delegate: Rectangle {
                id: handle

                required property int index
                required property real px
                required property real py

                width: 16
                height: 16
                radius: 8
                color: Theme.accent
                border.color: "white"
                border.width: 2
                x: px * editorSurface.width - width / 2
                y: py * editorSurface.height - height / 2

                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -6
                    cursorShape: Qt.SizeAllCursor
                    drag.target: handle
                    drag.minimumX: -handle.width / 2
                    drag.maximumX: editorSurface.width - handle.width / 2
                    drag.minimumY: -handle.height / 2
                    drag.maximumY: editorSurface.height - handle.height / 2

                    onReleased: {
                        pointsModel.setProperty(handle.index, "px",
                            Math.max(0, Math.min(1, (handle.x + handle.width / 2) / editorSurface.width)))
                        pointsModel.setProperty(handle.index, "py",
                            Math.max(0, Math.min(1, (handle.y + handle.height / 2) / editorSurface.height)))
                        root.publishPolygon()
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        Button {
            text: I18n.t("Сбросить зону")
            focusPolicy: Qt.StrongFocus
            onClicked: {
                root.loading = true
                pointsModel.clear()
                var points = root.defaultPoints()
                for (var i = 0; i < points.length; ++i)
                    pointsModel.append({ "px": points[i].x, "py": points[i].y })
                root.loading = false
                root.publishPolygon()
            }
        }

        Button {
            text: I18n.t("Добавить вершину")
            enabled: pointsModel.count < root.maximumPoints
            focusPolicy: Qt.StrongFocus
            onClicked: {
                pointsModel.append({ "px": 0.5, "py": 0.5 })
                root.publishPolygon()
            }
        }

        Button {
            text: I18n.t("Удалить вершину")
            enabled: pointsModel.count > root.minimumPoints
            focusPolicy: Qt.StrongFocus
            onClicked: {
                pointsModel.remove(pointsModel.count - 1)
                root.publishPolygon()
            }
        }

        Item { Layout.fillWidth: true }

        Text {
            text: I18n.t("Перетаскивайте точки, чтобы задать область срабатывания.")
            color: Theme.textMuted
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            Layout.maximumWidth: 360
        }
    }
}
