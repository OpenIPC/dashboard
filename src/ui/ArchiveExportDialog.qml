import QtQuick
import QtQuick.Dialogs
import OpenIPC

FileDialog {
    id: root

    property string defaultDownloadPath: ""
    property string sourcePath: ""
    property var archiveFile: null
    property real exportStartMs: 0
    property real exportEndMs: 0

    signal exportAccepted(string sourcePath, string outputPath, real startMs, real endMs)

    title: I18n.t("Сохранить видео как...")
    fileMode: FileDialog.SaveFile
    nameFilters: [I18n.t("Видео файлы (*.mp4)"), I18n.t("Все файлы (*)")]

    function openForFile(file, inputPath, startMs, endMs) {
        archiveFile = file
        sourcePath = inputPath || ""
        exportStartMs = startMs
        exportEndMs = endMs

        if (file && file.fileName && defaultDownloadPath.length > 0) {
            var suggestedName = "cut_" + file.fileName
            var folder = defaultDownloadPath
            if (Qt.platform.os === "windows") folder = folder.replace(/\\/g, "/")
            if (!folder.startsWith("file:///")) folder = "file:///" + folder
            if (!folder.endsWith("/")) folder += "/"
            root.currentFile = folder + suggestedName
        }

        open()
    }

    onAccepted: {
        var outputPath = SystemController.normalizeLocalPath(selectedFile)
        if (sourcePath.length === 0 || outputPath.length === 0) return
        root.exportAccepted(sourcePath, outputPath, exportStartMs, exportEndMs)
    }
}
