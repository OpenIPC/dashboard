import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OpenIPC

Rectangle {
    id: root

    property var settings: null
    property var status: ({})
    readonly property bool cloudProvider: settings
        && ["gdrive", "onedrive", "dropbox", "yadisk"].indexOf(settings.evidenceUploadProvider) >= 0
    readonly property var providerValues: ["local", "ftp", "gdrive", "dropbox", "onedrive", "yadisk"]
    readonly property var providerLabels: [
        I18n.t("Локальная папка (NAS/SMB)"),
        I18n.t("FTP"),
        I18n.t("Google Drive"),
        I18n.t("Dropbox"),
        I18n.t("OneDrive"),
        I18n.t("Yandex Disk")
    ]

    function refreshStatus() {
        var engine = SystemController.analyticsEngine
        status = engine && engine.uploadStatus ? engine.uploadStatus : ({})
    }

    function providerIndex() {
        if (!settings)
            return 0
        var index = providerValues.indexOf(settings.evidenceUploadProvider)
        return index >= 0 ? index : 0
    }

    function destinationText() {
        if (!settings)
            return ""
        var provider = settings.evidenceUploadProvider
        if (provider === "local" || provider === "ftp")
            return settings.evidenceUploadTarget
        var target = settings.parseTarget(settings.evidenceUploadTarget)
        return provider === "gdrive" ? (target.folder || "") : (target.path || "/OpenIPC")
    }

    function saveDestination(value) {
        if (!settings)
            return
        var provider = settings.evidenceUploadProvider
        if (provider === "local" || provider === "ftp") {
            settings.evidenceUploadTarget = value.trim()
        } else {
            var target = settings.parseTarget(settings.evidenceUploadTarget)
            if (provider === "gdrive") {
                target.folder = value.trim()
                settings.evidenceUploadFolder = value.trim()
            } else {
                target.path = value.trim()
                settings.evidenceUploadPath = value.trim()
            }
            settings.evidenceUploadTarget = settings.buildTarget(target)
        }
        settings.applyCurrentSettings()
    }

    function statusText() {
        var state = String(status.state || "idle")
        if (state === "queued") return I18n.t("В очереди")
        if (state === "uploading") return I18n.t("Выгрузка выполняется")
        if (state === "retry_wait") return I18n.t("Ожидание повторной попытки")
        if (state === "success") return I18n.t("Последняя выгрузка завершена")
        if (state === "failed") return I18n.t("Последняя выгрузка завершилась ошибкой")
        return I18n.t("Ожидание файлов")
    }

    Component.onCompleted: refreshStatus()

    Connections {
        target: SystemController.analyticsEngine
        ignoreUnknownSignals: true
        function onUploadStatusChanged() { root.refreshStatus() }
    }

    Layout.fillWidth: true
    implicitHeight: content.implicitHeight + 24
    color: Theme.metroSurfaceAlt
    border.color: Theme.metroStroke
    border.width: 1
    radius: Theme.metroTileRadius

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: I18n.t("Очередь выгрузки")
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 14
                }
                Text {
                    text: I18n.t("Копирует снимки и клипы в локальное или облачное хранилище с повтором при временной ошибке.")
                    color: Theme.textMuted
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            MetroCheckBox {
                text: I18n.t("Включить выгрузку")
                checked: root.settings ? root.settings.evidenceUploadEnabled : false
                onToggled: {
                    if (!root.settings) return
                    root.settings.evidenceUploadEnabled = checked
                    root.settings.applyCurrentSettings()
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width < 650 ? 1 : 2
            columnSpacing: 12
            rowSpacing: 8

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: I18n.t("Провайдер"); color: Theme.textSecondary; font.pixelSize: 11 }
                StyledComboBox {
                    Layout.fillWidth: true
                    model: root.providerLabels
                    currentIndex: root.providerIndex()
                    onUserSelected: {
                        if (!root.settings) return
                        root.settings.evidenceUploadProvider = root.providerValues[currentIndex]
                        root.settings.evidenceUploadTarget = ""
                        root.settings.applyCurrentSettings()
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text {
                    text: root.settings && root.settings.evidenceUploadProvider === "gdrive"
                        ? I18n.t("ID папки (Drive)")
                        : I18n.t("Путь назначения")
                    color: Theme.textSecondary
                    font.pixelSize: 11
                }
                TextField {
                    id: destinationField
                    Layout.fillWidth: true
                    text: root.destinationText()
                    placeholderText: root.settings && root.settings.evidenceUploadProvider === "ftp"
                        ? "ftp://user:pass@host:21/path/"
                        : (root.cloudProvider ? "/OpenIPC" : "E:/OpenIPC/Evidence")
                    color: Theme.textPrimary
                    selectionColor: Theme.metroBlue
                    background: Rectangle {
                        color: Theme.controlBackground
                        border.color: destinationField.activeFocus ? Theme.accent : Theme.controlBorder
                        radius: Theme.radiusSm
                    }
                    onEditingFinished: root.saveDestination(text)
                }
            }
        }

        GridLayout {
            visible: root.cloudProvider
            Layout.fillWidth: true
            columns: width < 650 ? 1 : 2
            columnSpacing: 12
            rowSpacing: 8

            TextField {
                id: clientIdField
                Layout.fillWidth: true
                visible: root.settings ? root.settings.evidenceOAuthAdvanced : false
                text: root.settings ? root.settings.evidenceUploadClientId : ""
                placeholderText: I18n.t("Введите Client ID приложения")
                color: Theme.textPrimary
                background: Rectangle { color: Theme.controlBackground; border.color: Theme.controlBorder; radius: Theme.radiusSm }
                onEditingFinished: {
                    if (!root.settings) return
                    root.settings.evidenceUploadClientId = text.trim()
                    root.settings.applyCurrentSettings()
                }
            }

            TextField {
                id: clientSecretField
                Layout.fillWidth: true
                visible: root.settings ? root.settings.evidenceOAuthAdvanced : false
                text: root.settings ? root.settings.evidenceUploadClientSecret : ""
                placeholderText: I18n.t("Введите Client Secret")
                echoMode: TextInput.Password
                color: Theme.textPrimary
                background: Rectangle { color: Theme.controlBackground; border.color: Theme.controlBorder; radius: Theme.radiusSm }
                onEditingFinished: {
                    if (!root.settings) return
                    root.settings.evidenceUploadClientSecret = text
                    root.settings.applyCurrentSettings()
                }
            }

            MetroCheckBox {
                text: I18n.t("Расширенный режим (свой Client ID)")
                checked: root.settings ? root.settings.evidenceOAuthAdvanced : false
                onToggled: if (root.settings) root.settings.evidenceOAuthAdvanced = checked
            }

            Button {
                text: I18n.t("Авторизоваться")
                enabled: root.settings
                    && root.settings.effectiveClientId(root.settings.evidenceUploadProvider).trim() !== ""
                focusPolicy: Qt.StrongFocus
                onClicked: {
                    var engine = SystemController.analyticsEngine
                    if (!root.settings || !engine) return
                    engine.startOAuth(root.settings.evidenceUploadProvider,
                                      root.settings.effectiveClientId(root.settings.evidenceUploadProvider),
                                      root.settings.evidenceUploadClientSecret)
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: statusColumn.implicitHeight + 16
            color: Theme.controlBackground
            border.color: root.status.state === "failed" ? Theme.danger : Theme.controlBorder
            radius: Theme.radiusSm

            ColumnLayout {
                id: statusColumn
                anchors.fill: parent
                anchors.margins: 8
                spacing: 3

                Text {
                    Layout.fillWidth: true
                    text: root.statusText()
                        + " · " + I18n.t("Очередь: %1", [String(root.status.queueDepth || 0)])
                        + " · " + I18n.t("Успешно: %1", [String(root.status.completed || 0)])
                        + " · " + I18n.t("Ошибок: %1", [String(root.status.failed || 0)])
                    color: root.status.state === "failed" ? Theme.danger : Theme.textSecondary
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                }
                Text {
                    visible: String(root.status.lastError || "") !== ""
                    Layout.fillWidth: true
                    text: String(root.status.lastError || "")
                    color: Theme.textMuted
                    wrapMode: Text.WordWrap
                    font.pixelSize: 10
                }
            }
        }
    }
}
