import QtQuick
import QtQuick.Layouts
import OpenIPC

GridLayout {
    id: root

    property var rows: []

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    columns: width > 900 ? 4 : 2
    rowSpacing: 12
    columnSpacing: 12

    Repeater {
        model: root.rows

        delegate: MajesticStatusCard {
            required property var modelData

            title: modelData.title
            value: modelData.value
            subtitle: modelData.subtitle
            percent: modelData.percent
            accent: modelData.accent
        }
    }
}
