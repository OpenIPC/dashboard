const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('terminalApi', {
    // Функции управления окном
    minimize: () => ipcRenderer.send('minimize-window'),
    maximize: () => ipcRenderer.send('maximize-window'),
    close: () => ipcRenderer.send('close-window'),
    onWindowMaximized: (callback) => ipcRenderer.on('window-maximized', callback),
    onWindowUnmaximized: (callback) => ipcRenderer.on('window-unmaximized', callback),
    
    // API для терминала
    onData: (callback) => ipcRenderer.on('ssh-data', (event, data) => callback(data)),
    onStatus: (callback) => ipcRenderer.on('ssh-status', (event, status) => callback(status)),
    sendInput: (cameraId, data) => ipcRenderer.send(`ssh-input-${cameraId}`, data),
    readClipboard: () => ipcRenderer.invoke('clipboardRead'),
    writeClipboard: (text) => ipcRenderer.invoke('clipboardWrite', text),
});