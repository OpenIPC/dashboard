// --- ФАЙЛ: src/main/window-manager.js ---

const { BrowserWindow, app } = require('electron');
const path = require('path');
const { autoUpdater } = require('electron-updater');
const { registerUpdaterEvents } = require('./services');
// START: ИСПРАВЛЕНИЕ - Импортируем branding-manager
const brandingManager = require('./branding-manager');
// END: ИСПРАВЛЕНИЕ

let mainWindow = null;
const sshWindows = {};
const fileManagerWindows = {};

function getMainWindow() {
    return mainWindow;
}

// START: ИСПРАВЛЕНИЕ - Делаем функцию асинхронной
async function createWindow() {
    // Получаем конфиг брендинга ДО создания окна
    const brandingConfig = await brandingManager.getBrandingConfig();
// END: ИСПРАВЛЕНИЕ

    mainWindow = new BrowserWindow({
        width: 1400,
        height: 900,
        minWidth: 1024,
        minHeight: 768,
        // START: ИСПРАВЛЕНИЕ - Используем appName из конфига
        title: brandingConfig.appName,
        // END: ИСПРАВЛЕНИЕ
        frame: false,
        titleBarStyle: 'hidden',
        // START: ИСПРАВЛЕНИЕ - Обновляем путь к иконке
        icon: brandingConfig.iconPath
            ? path.isAbsolute(brandingConfig.iconPath)
                ? brandingConfig.iconPath
                : path.join(__dirname, '../../', brandingConfig.iconPath)
            : path.join(__dirname, '../../assets/icon.ico'),
        // END: ИСПРАВЛЕНИЕ
        webPreferences: {
            // Подключаем основной preload-скрипт
            preload: path.join(__dirname, '../../preload.js'),
            
            // --- ВОТ ИСПРАВЛЕНИЕ ---
            // Отключаем песочницу, чтобы в preload-скрипте
            // были доступны модули Node.js, такие как 'path'.
            sandbox: false
        }
    });

    mainWindow.loadFile('index.html');

    // Отправляем события в renderer для обновления иконки максимизации
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
            // Подключаем preload-скрипт специально для файлового менеджера
            preload: path.join(__dirname, '../../fm-preload.js'),
            contextIsolation: true,
            nodeIntegration: false,
            // Для этого окна тоже отключаем песочницу, если понадобятся модули Node
            sandbox: false
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
            // Подключаем preload-скрипт специально для SSH терминала
            preload: path.join(__dirname, '../../terminal-preload.js'),
            // И здесь тоже на всякий случай
            sandbox: false
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