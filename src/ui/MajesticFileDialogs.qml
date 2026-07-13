import QtQuick
import QtQuick.Dialogs
import OpenIPC

Item {
    id: root

    property string cameraHost: ""
    property int cameraPort: 80
    property string cameraUser: "root"
    property string cameraPassword: ""
    property var majesticClient: null
    property var originalConfig: ({})
    property var currentSchema: ({})
    property int snapshotWidth: 0
    property int snapshotHeight: 0
    property int snapshotQuality: 85
    property bool snapshotGray: false

    signal requestCreated(string requestId)
    signal firmwareBackupSelected(string path)
    signal firmwareUploadSelected(string path)

    function openSnapshot() { snapshotDialog.open() }
    function openSaveBackup() { saveBackupDialog.open() }
    function openBackupRestore() { openBackupDialog.open() }
    function openPcm() { pcmDialog.open() }
    function openFirmwareBackup() { firmwareBackupDialog.open() }
    function openFirmwareUpload() { firmwareUploadDialog.open() }

    FileDialog {
        id: snapshotDialog
        title: I18n.t("Сохранить снимок Majestic")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "jpg"
        nameFilters: [I18n.t("JPEG (*.jpg *.jpeg)"), I18n.t("Все файлы (*)")]
        onAccepted: root.requestCreated(root.majesticClient.takeSnapshot(
            root.cameraHost,
            root.cameraPort,
            root.cameraUser,
            root.cameraPassword,
            String(selectedFile),
            root.snapshotWidth,
            root.snapshotHeight,
            root.snapshotQuality,
            root.snapshotGray))
    }

    FileDialog {
        id: saveBackupDialog
        title: I18n.t("Сохранить backup Majestic")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: [I18n.t("JSON (*.json)"), I18n.t("Все файлы (*)")]
        onAccepted: root.requestCreated(root.majesticClient.saveConfigurationBackup(
            root.originalConfig, root.currentSchema, String(selectedFile)))
    }

    FileDialog {
        id: openBackupDialog
        title: I18n.t("Открыть backup Majestic")
        fileMode: FileDialog.OpenFile
        nameFilters: [I18n.t("JSON (*.json)"), I18n.t("Все файлы (*)")]
        onAccepted: root.requestCreated(root.majesticClient.loadConfigurationBackup(
            String(selectedFile)))
    }

    FileDialog {
        id: pcmDialog
        title: I18n.t("Выбрать PCM (S16 LE, 8 кГц, mono)")
        fileMode: FileDialog.OpenFile
        nameFilters: [I18n.t("PCM (*.pcm *.raw)"), I18n.t("Все файлы (*)")]
        onAccepted: root.requestCreated(root.majesticClient.playPcmFile(
            root.cameraHost,
            root.cameraPort,
            root.cameraUser,
            root.cameraPassword,
            String(selectedFile)))
    }

    FileDialog {
        id: firmwareBackupDialog
        title: I18n.t("Сохранить firmware backup OpenIPC")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "tgz"
        nameFilters: [I18n.t("OpenIPC backup (*.tgz *.tar.gz)"), I18n.t("Все файлы (*)")]
        onAccepted: root.firmwareBackupSelected(String(selectedFile))
    }

    FileDialog {
        id: firmwareUploadDialog
        title: I18n.t("Выбрать firmware archive OpenIPC")
        fileMode: FileDialog.OpenFile
        nameFilters: [I18n.t("OpenIPC firmware (*.tgz *.gz)"), I18n.t("Все файлы (*)")]
        onAccepted: root.firmwareUploadSelected(String(selectedFile))
    }
}
