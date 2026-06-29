import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: statusCard
    property string title: ""
    property string value: "—"
    property string subtitle: ""
    property real percent: 0
    property color accent: Theme.accent

    Layout.fillWidth: true
    Layout.preferredHeight: 142
    color: Theme.cardBackground
    border.color: Theme.cardBorder
    radius: Theme.radiusLg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 7

        Text {
            text: statusCard.title
            color: Theme.textMuted
            font.pixelSize: 11
            font.letterSpacing: 1.1
            font.capitalization: Font.AllUppercase
        }
        Text {
            text: statusCard.value
            color: Theme.textPrimary
            font.bold: true
            font.pixelSize: 24
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 5
            radius: 3
            color: Theme.controlBackgroundAlt
            Rectangle {
                width: parent.width * Math.max(0, Math.min(100, statusCard.percent)) / 100
                height: parent.height
                radius: parent.radius
                color: statusCard.accent
            }
        }
        Text {
            Layout.fillWidth: true
            text: statusCard.subtitle
            color: Theme.textMuted
            font.pixelSize: 12
            elide: Text.ElideRight
        }
    }
}
