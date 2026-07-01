import QtQuick
import QtQuick.Layouts
import OpenIPC

GridLayout {
    id: root

    property var rows: []

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    columns: width > 900 ? 2 : 1
    rowSpacing: 12
    columnSpacing: 12

    Repeater {
        model: root.rows

        delegate: Rectangle {
            id: rowCard

            required property var modelData

            Layout.fillWidth: true
            Layout.preferredHeight: 112
            color: Theme.cardBackground
            border.color: rowCard.modelData.state === "block" ? Theme.danger : (rowCard.modelData.state === "warn" ? Theme.warning : Theme.cardBorder)
            radius: Theme.radiusLg

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        Layout.fillWidth: true
                        text: rowCard.modelData.title
                        color: Theme.textPrimary
                        font.bold: true
                        font.pixelSize: 16
                    }

                    Rectangle {
                        implicitWidth: badgeText.implicitWidth + 18
                        implicitHeight: 24
                        radius: 12
                        color: rowCard.modelData.state === "block" ? "#3f1212" : (rowCard.modelData.state === "warn" ? "#422006" : "#052e1b")
                        border.color: rowCard.modelData.state === "block" ? Theme.danger : (rowCard.modelData.state === "warn" ? Theme.warning : Theme.success)

                        Text {
                            id: badgeText
                            anchors.centerIn: parent
                            text: rowCard.modelData.state === "block" ? I18n.t("заблокировано") : (rowCard.modelData.state === "warn" ? I18n.t("внимание") : "OK")
                            color: rowCard.modelData.state === "block" ? Theme.danger : (rowCard.modelData.state === "warn" ? Theme.warning : Theme.success)
                            font.bold: true
                            font.pixelSize: 10
                        }
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: rowCard.modelData.text
                    color: Theme.textMuted
                    wrapMode: Text.WordWrap
                    font.pixelSize: 12
                }
            }
        }
    }
}
