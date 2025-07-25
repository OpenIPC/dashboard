const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('scpApi', {
    // VVV ИЗМЕНЕНИЕ: Добавлены функции управления окном VVV
    minimize: () => ipcRenderer.send('minimize-window'),
    maximize: () => ipcRenderer.send('maximize-window'),
    close: () => ipcRenderer.send('close-window'),
    onWindowMaximized: (callback) => ipcRenderer.on('window-maximized', callback),
    onWindowUnmaximized: (callback) => ipcRenderer.on('window-unmaximized', callback),
    // ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^

    connect: (camera) => ipcRenderer.invoke('scp-connect', camera),
    list: (cameraId, path) => ipcRenderer.invoke('scp-list', { cameraId, path }),
    download: (cameraId, remotePath) => ipcRenderer.invoke('scp-download', { cameraId, remotePath }),
    upload: (cameraId, remoteDir) => ipcRenderer.invoke('scp-upload', { cameraId, remotePath: remoteDir }),
    createDirectory: (cameraId, path) => ipcRenderer.invoke('scp-mkdir', { cameraId, path }),
    deleteFile: (cameraId, path) => ipcRenderer.invoke('scp-delete-file', { cameraId, path }),
    deleteDirectory: (cameraId, path) => ipcRenderer.invoke('scp-delete-dir', { cameraId, path }),
    
    // Для локальной файловой системы
    getLocalDiskList: () => ipcRenderer.invoke('get-local-disk-list'),
    listLocal: (path) => ipcRenderer.invoke('list-local-files', path),

    // События
    onProgress: (callback) => ipcRenderer.on('scp-progress', (event, progress) => callback(progress)),
    onClose: (callback) => ipcRenderer.on('scp-close', () => callback())
});