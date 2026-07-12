pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var rows: []
    property string title: I18n.t("Safe actions")
    property string description: I18n.t("Действия сгруппированы по риску и текущей доступности для этой камеры.")

    Layout.fillWidth: true
    Layout.leftMargin: 16
    Layout.rightMargin: 16
    Layout.minimumHeight: 186
    Layout.preferredHeight: Math.max(Layout.minimumHeight, content.implicitHeight + 28)
    color: Theme.metroSurface
    border.color: Theme.metroStroke
    radius: Theme.metroTileRadius

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: root.title
                color: Theme.textPrimary
                font.bold: true
                font.pixelSize: 17
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: root.description
                color: Theme.textMuted
                wrapMode: Text.WordWrap
                font.pixelSize: 11
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width > 980 ? 3 : (width > 620 ? 2 : 1)
            rowSpacing: 8
            columnSpacing: 8

            Repeater {
                model: root.rows

                delegate: Rectangle {
                    id: actionCard

                    required property var modelData

                    Layout.fillWidth: true
                    Layout.minimumHeight: 96
                    Layout.preferredHeight: Math.max(Layout.minimumHeight, actionContent.implicitHeight + 22)
                    color: actionCard.modelData.enabled === true ? Theme.cardBackground : Theme.metroSurfaceAlt
                    border.color: actionCard.modelData.level === "danger" ? Theme.metroRed
                                  : actionCard.modelData.level === "warn" ? Theme.metroAmber
                                  : actionCard.modelData.enabled === true ? Theme.metroGreen
                                  : Theme.metroStroke
                    radius: Theme.metroTileRadius

                    ColumnLayout {
                        id: actionContent

                        anchors.fill: parent
                        anchors.margins: 11
                        spacing: 6

                        RowLayout {
                            Layout.fillWidth: true

                            Text {
                                Layout.fillWidth: true
                                text: actionCard.modelData.title
                                color: Theme.textPrimary
                                font.bold: true
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                implicitWidth: badgeText.implicitWidth + 16
                                implicitHeight: 22
                                radius: Theme.metroTileRadius
                                color: actionCard.modelData.enabled === true ? Theme.successSurface : Theme.warningSurface
                                border.color: actionCard.modelData.enabled === true ? Theme.metroGreen : Theme.metroAmber

                                Text {
                                    id: badgeText
                                    anchors.centerIn: parent
                                    text: actionCard.modelData.enabled === true ? I18n.t("доступно") : I18n.t("закрыто")
                                    color: actionCard.modelData.enabled === true ? Theme.success : Theme.warning
                                    font.bold: true
                                    font.pixelSize: 10
                                }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: actionCard.modelData.text
                            color: Theme.textMuted
                            wrapMode: Text.WordWrap
                            font.pixelSize: 11
                        }

                        Text {
                            Layout.fillWidth: true
                            text: actionCard.modelData.guard
                            color: actionCard.modelData.level === "danger" ? Theme.danger
                                   : actionCard.modelData.level === "warn" ? Theme.warning
                                   : Theme.textSecondary
                            wrapMode: Text.WordWrap
                            font.pixelSize: 10
                            visible: text.length > 0
                        }
                    }
                }
            }
        }
    }
}
