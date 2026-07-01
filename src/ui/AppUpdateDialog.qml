pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Window {
    id: root

    width: 720
    height: 620
    minimumWidth: 560
    minimumHeight: 420
    title: I18n.t("Доступно обновление")
    color: Theme.appBackground
    flags: Qt.Window | Qt.FramelessWindowHint
    modality: Qt.ApplicationModal

    property var updateChecker: null
    readonly property string latestVersion: updateChecker ? updateChecker.latestVersion : ""
    readonly property string latestName: updateChecker ? updateChecker.latestName : ""
    readonly property string releaseNotes: updateChecker ? updateChecker.releaseNotes : ""
    readonly property bool latestPrerelease: updateChecker ? updateChecker.latestPrerelease : false
    readonly property string iconFontFamily: materialIcons.status === FontLoader.Ready ? materialIcons.name : "Material Icons"

    function openDialog() {
        show()
        requestActivate()
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/OpenIPC/src/ui/fonts/MaterialIcons-Regular.ttf"
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.appBackground
        radius: Theme.radiusLg
        border.color: Theme.panelBorderStrong
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 78
            color: Theme.topBarBackground
            radius: Theme.radiusLg

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: Theme.radiusLg
                color: Theme.topBarBackground
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 22
                anchors.rightMargin: 12
                spacing: 14

                Rectangle {
                    Layout.preferredWidth: 44
                    Layout.preferredHeight: 44
                    radius: 14
                    color: Theme.accent

                    Text {
                        anchors.centerIn: parent
                        text: "system_update"
                        font.family: root.iconFontFamily
                        font.pixelSize: 24
                        color: "white"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    Text {
                        Layout.fillWidth: true
                        text: root.latestName.length > 0 ? root.latestName : I18n.t("Доступна новая версия")
                        color: Theme.textPrimary
                        font.pixelSize: 20
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: I18n.t("Текущая версия: %1", [root.updateChecker ? root.updateChecker.currentVersion : "—"])
                            color: Theme.textSecondary
                            font.pixelSize: 13
                        }

                        Text {
                            text: "→"
                            color: Theme.textMuted
                            font.pixelSize: 13
                        }

                        Text {
                            text: I18n.t("Новая версия: %1", [root.latestVersion.length > 0 ? root.latestVersion : "—"])
                            color: Theme.success
                            font.pixelSize: 13
                            font.bold: true
                        }

                        Rectangle {
                            visible: root.latestPrerelease
                            Layout.preferredWidth: prereleaseLabel.implicitWidth + 16
                            Layout.preferredHeight: 22
                            radius: 11
                            color: Theme.warning
                            opacity: 0.95

                            Text {
                                id: prereleaseLabel
                                anchors.centerIn: parent
                                text: I18n.t("Предварительный релиз")
                                color: "black"
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }
                    }
                }

                DashboardDialogButton {
                    text: "×"
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    buttonColor: Theme.controlBackground
                    buttonHoverColor: Theme.cardHover
                    buttonBorderColor: Theme.controlBorder
                    buttonTextColor: Theme.textPrimary
                    onClicked: root.hide()
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 18
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: I18n.t("Описание релиза")
                color: Theme.textPrimary
                font.pixelSize: 16
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: Theme.radiusMd
                color: Theme.controlBackground
                border.color: Theme.controlBorder
                border.width: 1

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 12
                    clip: true

                    TextArea {
                        text: root.releaseNotes.length > 0
                              ? root.releaseNotes
                              : I18n.t("GitHub Release не содержит описания.")
                        readOnly: true
                        wrapMode: TextEdit.Wrap
                        selectByMouse: true
                        color: Theme.textSecondary
                        selectedTextColor: Theme.textPrimary
                        selectionColor: Theme.accent
                        font.pixelSize: 13
                        background: Item {}
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: I18n.t("Откройте страницу релиза, чтобы скачать установщик или AppImage. Автоматическая установка будет добавлена отдельным безопасным шагом.")
                color: Theme.textMuted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item { Layout.fillWidth: true }

                DashboardDialogButton {
                    text: I18n.t("Пропустить эту версию")
                    Layout.preferredWidth: 170
                    Layout.preferredHeight: 38
                    buttonColor: Theme.controlBackground
                    buttonHoverColor: Theme.cardHover
                    buttonBorderColor: Theme.controlBorder
                    buttonTextColor: Theme.textSecondary
                    onClicked: {
                        if (root.updateChecker) root.updateChecker.dismissCurrentUpdate()
                        root.hide()
                    }
                }

                DashboardDialogButton {
                    text: I18n.t("Напомнить позже")
                    Layout.preferredWidth: 145
                    Layout.preferredHeight: 38
                    buttonColor: Theme.controlBackground
                    buttonHoverColor: Theme.cardHover
                    buttonBorderColor: Theme.controlBorderStrong
                    buttonTextColor: Theme.textPrimary
                    onClicked: {
                        if (root.updateChecker) root.updateChecker.remindLater()
                        root.hide()
                    }
                }

                DashboardDialogButton {
                    text: I18n.t("Открыть релиз")
                    Layout.preferredWidth: 140
                    Layout.preferredHeight: 38
                    buttonColor: Theme.accent
                    buttonHoverColor: Qt.lighter(Theme.accent, 1.15)
                    buttonBorderColor: Theme.accent
                    buttonTextColor: "white"
                    onClicked: {
                        if (root.updateChecker) root.updateChecker.openReleasePage()
                        root.hide()
                    }
                }
            }
        }
    }
}
