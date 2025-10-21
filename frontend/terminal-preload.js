const { contextBridge, ipcRenderer } = require('electron');
const path = require('path');
const { pathToFileURL } = require('url');

const xtermCssHref = pathToFileURL(path.join(__dirname, '../node_modules/@xterm/xterm/css/xterm.css')).href;
const xtermJsHref = pathToFileURL(path.join(__dirname, '../node_modules/@xterm/xterm/lib/xterm.js')).href;
const xtermAddonFitHref = pathToFileURL(path.join(__dirname, '../node_modules/@xterm/addon-fit/lib/addon-fit.js')).href;

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
    notifyReady: (cameraId) => ipcRenderer.send('ssh-terminal-ready', cameraId),
});

contextBridge.exposeInMainWorld('terminalEnv', {
    xtermCssHref,
    xtermJsHref,
    xtermAddonFitHref,
});