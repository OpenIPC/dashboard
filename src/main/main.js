// Файл: /src/main/main.js
// Это основной файл логики приложения.

const { app, dialog, BrowserWindow, ipcMain } = require('electron');
const log = require('electron-log');
const path = require('path');
const fs = require('fs'); // Добавляем fs
const EventEmitter = require('events');

const { ModuleManager } = require('./module-manager');
const configManager = require('./config-manager'); 

const { initializeApp, onAppWillQuit } = require('./app-lifecycle');
const { createWindow, getMainWindow } = require('./window-manager');
const { registerIpcHandlers } = require('./ipc-handlers');
// VVVVVV --- ИЗМЕНЕНИЕ: Импортируем HLS-сервер --- VVVVVV
const { startHlsServer, stopHlsServer } = require('./hls-server');
// ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

// Настройка логов
log.transports.file.resolvePathFn = () => path.join(app.getPath('userData'), 'logs', 'main.log');
log.transports.file.level = 'info';
log.transports.file.format = '[{y}-{m}-{d} {h}:{i}:{s}.{l}] [{processType}] {text}';
log.errorHandler.startCatching({
    showDialog: false,
    onError({ error }) {
        dialog.showMessageBox({
            title: 'Критическая ошибка',
            message: 'Произошла непредвиденная ошибка в основном процессе. Приложение будет закрыто.',
            detail: error.stack,
        }).then(() => { app.quit(); });
    }
});
Object.assign(console, log.functions);
console.log('--- Application starting ---');

// Стандартная инициализация Electron
if (!app.requestSingleInstanceLock()) { app.quit(); }
if (process.platform === 'linux' || process.env.ELECTRON_FORCE_NO_SANDBOX) { app.commandLine.appendSwitch('--no-sandbox'); }
app.commandLine.appendSwitch('force_high_performance_gpu');
initializeApp();

const moduleManager = new ModuleManager();

const appAPI = {
    on: (eventName, callback) => moduleManager.registerListener(eventName, callback),
    off: (eventName, callback) => moduleManager.unregisterListener(eventName, callback),
    sendToRenderer: (channel, ...args) => {
        const mainWindow = getMainWindow();
        if (mainWindow) {
            mainWindow.webContents.send(channel, ...args);
        }
    },
    configManager: configManager,
    getAppState: async () => {
        const config = await configManager.loadConfiguration();
        const appSettings = await configManager.getAppSettings();
        return {
            cameras: config.cameras || [],
            layouts: config.layouts || [],
            activeLayoutId: config.activeLayoutId,
            appSettings
        };
    },
    getDataPath: () => configManager.getDataPath(),
    getAppSettings: () => configManager.getAppSettings(),
    saveAppSettings: async (newSettings) => {
        const currentSettings = await configManager.getAppSettings();
        const updatedSettings = { ...currentSettings, ...newSettings };
        return configManager.saveAppSettings(updatedSettings);
    }
};

moduleManager.appAPI = appAPI;

// Основной жизненный цикл приложения
app.whenReady().then(async () => {
    const mainWindow = createWindow();
    registerIpcHandlers(moduleManager); 

    // VVVVVV --- ИЗМЕНЕНИЕ: Запускаем HLS-сервер при старте --- VVVVVV
    try {
        const hlsTempPath = path.join(app.getPath('temp'), 'hls');
        // Очищаем папку от старых сегментов при запуске
        if (fs.existsSync(hlsTempPath)) {
            await fs.promises.rm(hlsTempPath, { recursive: true, force: true });
        }
        await fs.promises.mkdir(hlsTempPath, { recursive: true });
        await startHlsServer(hlsTempPath);
    } catch (error) {
        console.error("Failed to initialize HLS server:", error);
    }
    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

    console.log('[Main] Инициализация модульной системы...');
    moduleManager.discoverModules();
    
    const currentSettings = await configManager.getAppSettings();
    moduleManager.loadEnabledModules(currentSettings);
    
    ipcMain.handle('get-renderer-modules', async (event) => {
        console.log('[Main] Renderer is ready and requesting module scripts.');
        const rendererScripts = [];
        const currentSettings = await configManager.getAppSettings();
        const enabledModules = currentSettings.enabledModules || [];

        enabledModules.forEach(moduleId => {
            const mod = moduleManager.loadedModules.get(moduleId);
            if (mod && mod.manifest.entryPoints?.renderer) {
                const scriptPath = path.join(mod.manifest.path, mod.manifest.entryPoints.renderer);
                console.log(`[Modules] Found renderer script to send: ${scriptPath}`);
                rendererScripts.push(scriptPath);
            }
        });
        return rendererScripts;
    });
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') { app.quit(); }
});

app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) { createWindow(); }
});

// VVVVVV --- ИЗМЕНЕНИЕ: Останавливаем HLS-сервер при выходе --- VVVVVV
app.on('will-quit', (event) => {
    stopHlsServer();
    onAppWillQuit(event);
});
// ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^