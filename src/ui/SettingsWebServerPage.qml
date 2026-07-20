pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Item {
    id: page
    required property var settings
    readonly property bool compact: width < 590
    readonly property var server: SystemController.webServer

    ScrollView {
        anchors.fill: parent
        leftPadding: page.compact ? 12 : 20
        rightPadding: page.compact ? 12 : 20
        topPadding: 16
        bottomPadding: 16
        contentWidth: availableWidth
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical: StyledScrollBar {}

        ColumnLayout {
            width: parent.width
            spacing: 14

            Text {
                text: I18n.language === "ru" ? "Web-сервер" : "Web server"
                color: Theme.textPrimary
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: statusColumn.implicitHeight + 24
                color: Theme.panelSoftBackground
                border.color: page.server.running ? Theme.success : Theme.panelBorder
                radius: Theme.radiusSm

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: 10
                        radius: 5
                        color: page.server.running ? Theme.success : Theme.textFaint
                    }
                    ColumnLayout {
                        id: statusColumn
                        Layout.fillWidth: true
                        spacing: 3
                        Text {
                            text: page.server.running
                                  ? (I18n.language === "ru" ? "Сервер работает" : "Server is running")
                                  : (I18n.language === "ru" ? "Сервер остановлен" : "Server is stopped")
                            color: Theme.textPrimary
                            font.bold: true
                        }
                        Text {
                            text: page.server.running ? page.server.url
                                                      : (page.server.lastError || (I18n.language === "ru" ? "Доступ из браузера выключен" : "Browser access is disabled"))
                            color: page.server.lastError ? Theme.danger : Theme.textMuted
                            wrapMode: Text.WrapAnywhere
                            Layout.fillWidth: true
                        }
                    }
                    Button {
                        id: openButton
                        text: I18n.language === "ru" ? "Открыть" : "Open"
                        enabled: page.server.running
                        onClicked: page.server.openInBrowser()
                    }
                }
            }

            MetroCheckBox {
                text: I18n.language === "ru" ? "Включить Web-сервер" : "Enable Web server"
                checked: page.settings.webServerEnabled
                onToggled: page.settings.webServerEnabled = checked
            }

            MetroCheckBox {
                text: I18n.language === "ru" ? "Разрешить подключения из локальной сети" : "Allow local network connections"
                checked: page.settings.webServerAllowRemote
                enabled: page.settings.webServerEnabled
                onToggled: page.settings.webServerAllowRemote = checked
            }

            Rectangle {
                visible: page.settings.webServerAllowRemote
                Layout.fillWidth: true
                Layout.preferredHeight: warningText.implicitHeight + 20
                color: Theme.warningSurfaceSoft
                border.color: Theme.warning
                radius: Theme.radiusSm
                Text {
                    id: warningText
                    anchors.fill: parent
                    anchors.margins: 10
                    text: I18n.language === "ru"
                          ? "LAN-доступ открывает панель другим устройствам. Используйте VPN или HTTPS reverse proxy; не публикуйте HTTP-порт напрямую в Интернет."
                          : "LAN access exposes the dashboard to other devices. Use a VPN or HTTPS reverse proxy; never publish the HTTP port directly to the Internet."
                    color: Theme.warningText
                    wrapMode: Text.WordWrap
                }
            }

            GridLayout {
                columns: page.compact ? 1 : 2
                columnSpacing: 16
                rowSpacing: 10
                Layout.fillWidth: true

                Text { text: I18n.language === "ru" ? "Адрес привязки" : "Bind address"; color: Theme.textMuted }
                TextField {
                    text: page.settings.webServerBindAddress
                    enabled: page.settings.webServerEnabled && page.settings.webServerAllowRemote
                    Layout.fillWidth: true
                    placeholderText: "0.0.0.0"
                    onTextEdited: page.settings.webServerBindAddress = text.trim()
                    color: Theme.textSecondary
                    background: Rectangle { color: Theme.controlBackground; border.color: Theme.controlBorder; radius: Theme.radiusSm }
                }

                Text { text: I18n.language === "ru" ? "HTTP-порт" : "HTTP port"; color: Theme.textMuted }
                SettingsSpinBox {
                    from: 1024; to: 65535; editable: true
                    value: page.settings.webServerPort
                    enabled: page.settings.webServerEnabled
                    onValueModified: page.settings.webServerPort = value
                }

                Text { text: I18n.language === "ru" ? "WebSocket-порт" : "WebSocket port"; color: Theme.textMuted }
                SettingsSpinBox {
                    from: 1024; to: 65535; editable: true
                    value: page.settings.webSocketPort
                    enabled: page.settings.webServerEnabled && page.server.webSocketsAvailable
                    onValueModified: page.settings.webSocketPort = value
                }

                Text { text: I18n.language === "ru" ? "Срок сессии (мин)" : "Session lifetime (min)"; color: Theme.textMuted }
                SettingsSpinBox {
                    from: 5; to: 1440; stepSize: 5; editable: true
                    value: page.settings.webSessionTimeoutMinutes
                    enabled: page.settings.webServerEnabled
                    onValueModified: page.settings.webSessionTimeoutMinutes = value
                }
            }

            MetroCheckBox {
                text: I18n.language === "ru" ? "Secure cookie (только при HTTPS)" : "Secure cookie (HTTPS only)"
                checked: page.settings.webSecureCookies
                enabled: page.settings.webServerEnabled
                onToggled: page.settings.webSecureCookies = checked
                ToolTip.visible: hovered
                ToolTip.delay: 450
                ToolTip.text: I18n.language === "ru"
                              ? "Включайте только за HTTPS reverse proxy, иначе браузер не отправит cookie по HTTP."
                              : "Enable only behind an HTTPS reverse proxy, otherwise browsers will not send the cookie over HTTP."
            }

            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.panelBorder }

            RowLayout {
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    text: (I18n.language === "ru" ? "Сессии: " : "Sessions: ") + page.server.activeSessions
                          + " · " + (I18n.language === "ru" ? "WebSocket-клиенты: " : "WebSocket clients: ") + page.server.connectedClients
                    color: Theme.textMuted
                }
                Button {
                    text: I18n.language === "ru" ? "Применить и перезапустить" : "Apply and restart"
                    enabled: page.settings.webServerEnabled
                    onClicked: {
                        page.settings.applyCurrentSettings()
                        page.server.restart()
                    }
                }
            }

            ColumnLayout {
                visible: page.server.running && page.server.accessUrls.length > 0
                Layout.fillWidth: true
                spacing: 6
                Text {
                    text: I18n.language === "ru" ? "Адреса подключения" : "Access URLs"
                    color: Theme.textPrimary
                    font.bold: true
                }
                Repeater {
                    model: page.server.accessUrls
                    delegate: Text {
                        required property var modelData
                        text: modelData.label + ": " + modelData.url
                        color: Theme.infoText
                        wrapMode: Text.WrapAnywhere
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
