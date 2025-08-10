// --- ФАЙЛ: src/main/window-manager.js ---

const { BrowserWindow, app } = require('electron');
const path = require('path');
const { autoUpdater } = require('electron-updater');
const { registerUpdaterEvents } = require('./services');

let mainWindow = null;
const sshWindows = {};
const fileManagerWindows = {};

function getMainWindow() {
    return mainWindow;
}

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1400,
        height: 900,
        minWidth: 1024,
        minHeight: 768,
        title: "DASHBOARD for OpenIPC",
        frame: false,
        titleBarStyle: 'hidden',
        icon: path.join(__dirname, '../../build/icon.png'),
        webPreferences: {
            preload: path.join(__dirname, '../../preload.js')
        }
    });

    mainWindow.loadFile('index.html');

    mainWindow.once('ready-to-show', () => {
        if (app.isPackaged) {
            autoUpdater.checkForUpdates();
        }
    });

    mainWindow.on('maximize', () => mainWindow.webContents.send('window-maximized'));
    mainWindow.on('unmaximize', () => mainWindow.webContents.send('window-unmaximized'));

    mainWindow.on('closed', () => {
        mainWindow = null;
    });

    registerUpdaterEvents(mainWindow);
    return mainWindow;
}

function createFileManagerWindow(camera, fileManagerConnections) {
    const win = new BrowserWindow({
        width: 1000,
        height: 700,
        minWidth: 800,
        minHeight: 500,
        title: `File Manager: ${camera.name}`,
        frame: false,
        titleBarStyle: 'hidden',
        parent: mainWindow,
        modal: true,
        webPreferences: {
            // VVV Используем новый preload-скрипт VVV
            preload: path.join(__dirname, '../../fm-preload.js'),
            contextIsolation: true,
            nodeIntegration: false,
        }
    });

    win.loadFile('file-manager.html', { query: { camera: JSON.stringify(camera) } });
    
    fileManagerWindows[camera.id] = win;
    win.on('closed', () => {
        const conn = fileManagerConnections[camera.id];
        if (conn) {
            conn.end();
        }
        delete fileManagerWindows[camera.id];
        delete fileManagerConnections[camera.id];
    });

    win.on('maximize', () => win.webContents.send('window-maximized'));
    win.on('unmaximize', () => win.webContents.send('window-unmaximized'));

    return win;
}

function createSshTerminalWindow(cameraData, sshConnections) {
    const { id, name } = cameraData;
    if (sshWindows[id] && !sshWindows[id].isDestroyed()) {
        sshWindows[id].focus();
        return null;
    }
    
    const win = new BrowserWindow({
        width: 800,
        height: 600,
        minWidth: 500,
        minHeight: 400,
        title: `SSH Terminal: ${name}`,
        frame: false,
        titleBarStyle: 'hidden',
        parent: mainWindow,
        webPreferences: {
            // VVV Используем новый preload-скрипт VVV
            preload: path.join(__dirname, '../../terminal-preload.js')
        }
    });

    win.loadFile('terminal.html', { query: { camera: JSON.stringify(cameraData) } });

    sshWindows[id] = win;
    win.on('closed', () => {
        const conn = sshConnections[id];
        if (conn) {
            conn.end();
        }
        delete sshConnections[id];
        delete sshWindows[id];
    });

    win.on('maximize', () => win.webContents.send('window-maximized'));
    win.on('unmaximize', () => win.webContents.send('window-unmaximized'));

    return win;
}

module.exports = {
    getMainWindow,
    createWindow,
    createFileManagerWindow,
    createSshTerminalWindow,
};