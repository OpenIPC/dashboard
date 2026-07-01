import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: emptyHint

    property bool canSettings: false
    property bool canAnalytics: false

    signal closeRequested(bool dontShowAgain)
    signal searchRequested()
    signal addRequested()
    signal analyticsRequested()
    signal settingsRequested()

    z: 4

    Rectangle {
        anchors.fill: parent
        color: "#33000000"
        radius: Theme.radiusLg
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(560, Math.max(260, parent.width - 48))
        height: emptyHintLayout.implicitHeight + 28
        radius: Theme.radiusLg
        color: "#e5111620"
        border.color: Theme.panelBorderStrong
        border.width: 1

        Button {
            id: emptyHintCloseButton
            width: 30
            height: 30
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 8
            anchors.rightMargin: 8
            text: "x"
            background: Rectangle {
                radius: Theme.radiusSm
                color: emptyHintCloseButton.hovered ? Theme.cardHover : "transparent"
                border.color: "transparent"
            }
            contentItem: Text {
                text: emptyHintCloseButton.text
                color: Theme.textMuted
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: emptyHint.closeRequested(dontShowEmptyHint.checked)
        }

        ColumnLayout {
            id: emptyHintLayout
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            anchors.topMargin: 18
            spacing: 14

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 52
                Layout.preferredHeight: 52
                radius: 12
                color: Theme.panelSoftBackground
                border.color: Theme.controlBorderStrong

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    color: Theme.accentHover
                    font.pixelSize: 28
                    font.bold: true
                }
            }

            Text {
                Layout.fillWidth: true
                text: I18n.t("Добавьте первую камеру")
                color: Theme.textPrimary
                font.pixelSize: 22
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Text {
                Layout.fillWidth: true
                text: I18n.t("Начните с поиска OpenIPC/ONVIF камер — дальше приложение подскажет поток, статус и доступные действия.")
                color: Theme.textMuted
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            GridLayout {
                id: quickStartGrid

                Layout.fillWidth: true
                Layout.maximumWidth: 500
                Layout.alignment: Qt.AlignHCenter
                columns: quickStartGrid.width > 460 ? 3 : 1
                columnSpacing: 8
                rowSpacing: 8

                Repeater {
                    model: [
                        { step: "1", text: I18n.t("Найдите камеры") },
                        { step: "2", text: I18n.t("Перетащите в сетку") },
                        { step: "3", text: I18n.t("Откройте Control Center") }
                    ]

                    delegate: Rectangle {
                        id: quickStartStep

                        required property var modelData

                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        radius: Theme.radiusMd
                        color: Theme.panelSoftBackground
                        border.color: Theme.panelBorder

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 9
                            anchors.rightMargin: 9
                            spacing: 8

                            Rectangle {
                                Layout.preferredWidth: 22
                                Layout.preferredHeight: 22
                                radius: 11
                                color: Theme.controlBackground
                                border.color: Theme.accent

                                Text {
                                    anchors.centerIn: parent
                                    text: quickStartStep.modelData.step
                                    color: Theme.accentHover
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: quickStartStep.modelData.text
                                color: Theme.textSecondary
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                spacing: 8

                EmptyStateButton {
                    text: I18n.t("Поиск камер")
                    enabled: emptyHint.canSettings
                    buttonColor: Theme.accent
                    buttonHoverColor: Theme.accentHover
                    onClicked: emptyHint.searchRequested()
                }

                EmptyStateButton {
                    text: I18n.t("Добавить")
                    enabled: emptyHint.canSettings
                    onClicked: emptyHint.addRequested()
                }

                EmptyStateButton {
                    text: I18n.t("Аналитика")
                    enabled: emptyHint.canAnalytics
                    onClicked: emptyHint.analyticsRequested()
                }

                EmptyStateButton {
                    text: I18n.t("Настройки")
                    enabled: emptyHint.canSettings
                    onClicked: emptyHint.settingsRequested()
                }
            }

            CheckBox {
                id: dontShowEmptyHint
                Layout.alignment: Qt.AlignHCenter
                text: I18n.t("Не показывать при следующем запуске")
                checked: false
                contentItem: Text {
                    text: dontShowEmptyHint.text
                    color: Theme.textMuted
                    font.pixelSize: 12
                    leftPadding: dontShowEmptyHint.indicator.width + dontShowEmptyHint.spacing
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
