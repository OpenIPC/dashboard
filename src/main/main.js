// Файл: /src/main/main.js
// Это основной файл логики приложения.

const { app, dialog, BrowserWindow, ipcMain } = require('electron');
const log = require('electron-log');
const path = require('path');
const fs = require('fs');
const EventEmitter = require('events');

// START: ИСПРАВЛЕНИЕ - Добавляем импорты для веб-сервера
const express = require('express');
const http = require('http');
const os = require('os');
const { WebSocketServer } = require('ws');
// END: ИСПРАВЛЕНИЕ

const { ModuleManager } = require('./module-manager');
const configManager = require('./config-manager'); 

const { initializeApp, onAppWillQuit } = require('./app-lifecycle');
const { createWindow, getMainWindow } = require('./window-manager');
const { registerIpcHandlers, handlerMap } = require('./ipc-handlers');
const { startHlsServer, stopHlsServer } = require('./hls-server');

// --- Определение версии приложения при старте ---
let APP_VERSION = 'intellect'; // Значение по умолчанию для режима разработки

try {
    // Этот файл создается скриптом сборки (scripts/set-version.js)
    const versionConfigPath = path.join(__dirname, '..', '..', 'version-config.json');
    if (fs.existsSync(versionConfigPath)) {
        const config = JSON.parse(fs.readFileSync(versionConfigPath, 'utf-8'));
        APP_VERSION = config.version;
    }
} catch (e) {
    console.error('Could not read version config, defaulting to "intellect".', e);
}

console.log(`--- Application starting in [${APP_VERSION.toUpperCase()}] mode ---`);
// --- КОНЕЦ ИЗМЕНЕНИЯ ---

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

// Объявляем wss глобально для доступа в appAPI
let wss = null;

const appAPI = {
    on: (eventName, callback) => moduleManager.registerListener(eventName, callback),
    off: (eventName, callback) => moduleManager.unregisterListener(eventName, callback),
    sendToRenderer: (channel, ...args) => {
        const mainWindow = getMainWindow();
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send(channel, ...args);
        }
        // Отправляем события и веб-клиентам
        if (wss) {
            wss.clients.forEach(client => {
                if (client.readyState === require('ws').OPEN) {
                    client.send(JSON.stringify({ type: 'event', channel, payload: args[0] }));
                }
            });
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
    const mainWindow = await createWindow();

    // Эта функция теперь также заполняет нашу handlerMap для WebSocket
    registerIpcHandlers(moduleManager, APP_VERSION); 

    try {
        const hlsTempPath = path.join(app.getPath('temp'), 'hls');
        if (fs.existsSync(hlsTempPath)) {
            await fs.promises.rm(hlsTempPath, { recursive: true, force: true });
        }
        await fs.promises.mkdir(hlsTempPath, { recursive: true });
        await startHlsServer(hlsTempPath);
    } catch (error) {
        console.error("Failed to initialize HLS server:", error);
    }

    try {
        const webApp = express();
        const webServer = http.createServer(webApp);
        const rootDir = path.join(__dirname, '..', '..');
        
        // Явно указываем Express отдавать все необходимые папки
        webApp.use('/css', express.static(path.join(rootDir, 'css')));
        webApp.use('/js', express.static(path.join(rootDir, 'js')));
        webApp.use('/node_modules', express.static(path.join(rootDir, 'node_modules')));
        webApp.use('/assets', express.static(path.join(rootDir, 'assets')));
        // VVVVVV --- ИЗМЕНЕНИЕ: Добавляем раздачу папки с модулями --- VVVVVV
        webApp.use('/modules', express.static(path.join(rootDir, 'modules')));
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
        
        // Отдаем файлы из корневой папки (index.html, jsmpeg.min.js и т.д.)
        webApp.use(express.static(rootDir));

        const PORT = 8080;
        webServer.listen(PORT, '0.0.0.0', () => {
            console.log(`[Web Server] HTTP server is running on port ${PORT}`);
            const interfaces = os.networkInterfaces();
            console.log('[Web Server] Available at:');
            Object.keys(interfaces).forEach(ifaceName => {
                interfaces[ifaceName].forEach(iface => {
                    if (iface.family === 'IPv4' && !iface.internal) {
                        console.log(`  - http://${iface.address}:${PORT}`);
                    }
                });
            });
        });

        wss = new WebSocketServer({ port: 8081 });
        
        wss.on('connection', (ws) => {
            console.log('[WebSocket] Client connected.');
        
            ws.on('message', async (message) => {
                try {
                    const { type, channel, requestId, payload } = JSON.parse(message);
        
                    if (type === 'invoke') {
                        console.log(`[WebSocket] << INVOKE ${channel}`);
                        const handler = handlerMap.get(channel);

                        if (handler) {
                            const fakeEvent = { sender: getMainWindow()?.webContents }; 
                            const result = await handler(fakeEvent, payload);
                            console.log(`[WebSocket] >> RESPONSE for ${channel}`);
                            if (ws.readyState === require('ws').OPEN) {
                                ws.send(JSON.stringify({ type: 'response', requestId, payload: result }));
                            }
                        } else {
                            const errorMsg = `No handler found on server for invoked channel: ${channel}`;
                            console.error(`[WebSocket] ${errorMsg}`);
                            if (ws.readyState === require('ws').OPEN) {
                                ws.send(JSON.stringify({ type: 'response', requestId, payload: { success: false, error: errorMsg } }));
                            }
                        }
                    } else if (type === 'send') {
                        console.log(`[WebSocket] << SEND ${channel}`);
                        ipcMain.emit(channel, null, payload);
                    }
                } catch (e) {
                    console.error('[WebSocket] Error processing message:', e);
                }
            });
        
            ws.on('close', () => console.log('[WebSocket] Client disconnected.'));
        });

    } catch(e) {
        console.error('[Web Server] Failed to start:', e);
    }

    console.log('[Main] Инициализация модульной системы...');
    if (APP_VERSION === 'intellect') {
        moduleManager.discoverModules();
        const currentSettings = await configManager.getAppSettings();
        moduleManager.loadEnabledModules(currentSettings);
        console.log('[Main] Module system initialized for Intellect version.');
    } else {
        console.log('[Main] Lite version, skipping module system initialization.');
    }
    
    // VVVVVV --- ИЗМЕНЕНИЕ: Отдаем относительный URL вместо полного пути --- VVVVVV
    ipcMain.handle('get-renderer-modules', async (event) => {
        console.log('[Main] Renderer is ready and requesting module scripts.');

        if (APP_VERSION === 'lite') {
            console.log('[Modules] Lite version, returning no renderer scripts.');
            return [];
        }

        const rootDir = path.join(__dirname, '..', '..');
        const rendererScripts = [];
        const currentSettings = await configManager.getAppSettings();
        const enabledModules = currentSettings.enabledModules || [];

        enabledModules.forEach(moduleId => {
            const mod = moduleManager.loadedModules.get(moduleId);
            if (mod && mod.manifest.entryPoints?.renderer) {
                const fullScriptPath = path.join(mod.manifest.path, mod.manifest.entryPoints.renderer);
                // Преобразуем полный путь в относительный URL, который браузер сможет запросить
                const relativeUrl = path.relative(rootDir, fullScriptPath).replace(/\\/g, '/');
                console.log(`[Modules] Found renderer script, providing URL: /${relativeUrl}`);
                rendererScripts.push(relativeUrl);
            }
        });
        return rendererScripts;
    });
    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') { app.quit(); }
});

app.on('activate', async () => {
    if (BrowserWindow.getAllWindows().length === 0) { 
        await createWindow();
    }
});

app.on('will-quit', (event) => {
    stopHlsServer();
    onAppWillQuit(event);
});