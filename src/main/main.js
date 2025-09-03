// Файл: /src/main/main.js
// Это основной файл логики приложения.

const { app, dialog, BrowserWindow, ipcMain } = require('electron');
const log = require('electron-log');
const path = require('path');
const fs = require('fs');
const EventEmitter = require('events');

const express = require('express');
const http = require('http');
const os = require('os');
const { WebSocketServer } = require('ws');
const axios = require('axios');

const { ModuleManager } = require('./module-manager');
const configManager = require('./config-manager'); 

const { initializeApp, onAppWillQuit } = require('./app-lifecycle');
const { createWindow, getMainWindow } = require('./window-manager');
const { registerIpcHandlers, handlerMap } = require('./ipc-handlers');
const { startHlsServer, stopHlsServer } = require('./hls-server');
const processManager = require('./process-manager');

let APP_VERSION = 'intellect';

try {
    const versionConfigPath = path.join(__dirname, '..', '..', 'version-config.json');
    if (fs.existsSync(versionConfigPath)) {
        const config = JSON.parse(fs.readFileSync(versionConfigPath, 'utf-8'));
        APP_VERSION = config.version;
    }
} catch (e) {
    console.error('Could not read version config, defaulting to "intellect".', e);
}

module.exports = { APP_VERSION };

console.log(`--- Application starting in [${APP_VERSION.toUpperCase()}] mode ---`);

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

if (!app.requestSingleInstanceLock()) { app.quit(); }
if (process.platform === 'linux' || process.env.ELECTRON_FORCE_NO_SANDBOX) { app.commandLine.appendSwitch('--no-sandbox'); }
app.commandLine.appendSwitch('force_high_performance_gpu');
initializeApp();

const moduleManager = new ModuleManager();

let wss = null;

const appAPI = {
    on: (eventName, callback) => moduleManager.registerListener(eventName, callback),
    off: (eventName, callback) => moduleManager.unregisterListener(eventName, callback),
    sendToRenderer: (channel, ...args) => {
        const mainWindow = getMainWindow();
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send(channel, ...args);
        }
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

app.whenReady().then(async () => {
    const mainWindow = await createWindow();

    registerIpcHandlers(moduleManager, APP_VERSION); 

    await processManager.startMediaMTX();

    // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ --- VVVVVV
    // Возвращаем setInterval для регулярного получения статистики
    setInterval(() => {
        const options = {
            hostname: '127.0.0.1',
            port: 9997,
            path: '/v3/paths/list',
            method: 'GET',
            timeout: 2000
        };

        const req = http.request(options, (res) => {
            let data = '';
            res.on('data', (chunk) => { data += chunk; });
            res.on('end', () => {
                if (res.statusCode === 200) {
                    try {
                        const statsData = JSON.parse(data);
                        // Отправляем ПОЛНЫЙ список потоков в рендерер, как и раньше
                        if (mainWindow && !mainWindow.isDestroyed()) {
                            mainWindow.webContents.send('mediamtx-stats-update', statsData);
                        }
                    } catch (e) {
                        console.error('[MediaMTX Stats] Ошибка парсинга JSON:', e.message);
                    }
                }
            });
        });

        req.on('error', (e) => {
            if (e.code !== 'ECONNREFUSED') {
                console.error(`[MediaMTX Stats] Ошибка HTTP-запроса: ${e.message}`);
            }
        });
        
        req.on('timeout', () => {
            req.destroy();
            console.error('[MediaMTX Stats] Запрос статистики превысил таймаут.');
        });

        req.end();
    }, 3000);
    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^

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
        
        webApp.use(express.json());

        // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ --- VVVVVV
        // Упрощаем обработчик вебхуков, оставляя только то, что нужно для событий
        webApp.post('/mediamtx-webhook', async (req, res) => {
            const event = req.body;

            switch (event.event) {
                case 'onRecordSegmentCreate':
                    console.log(`[MediaMTX Webhook] New recording segment created: ${event.path.path}`);
                    const match = event.path.name.match(/^cam(\d+)_/);
                    if (match) {
                        const cameraId = parseInt(match[1], 10);
                        const camera = await configManager.getCameraConfig(cameraId);
                        if (camera) {
                            appAPI.sendToRenderer('mediamtx-archive-update', { cameraName: camera.name });
                        }
                    }
                    break;
                // Другие события (onReady, onRead и т.д.) нам здесь больше не нужны,
                // так как статистика получается через опрос.
            }

            res.sendStatus(200);
        });
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^

        webApp.use('/css', express.static(path.join(rootDir, 'css')));
        webApp.use('/js', express.static(path.join(rootDir, 'js')));
        webApp.use('/node_modules', express.static(path.join(rootDir, 'node_modules')));
        webApp.use('/assets', express.static(path.join(rootDir, 'assets')));
        webApp.use('/modules', express.static(path.join(rootDir, 'modules')));
        
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
        processManager.setWebSocketServer(wss);
        
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
                        ipcMain.emit(channel, { sender: getMainWindow()?.webContents }, payload);
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
                const relativeUrl = path.relative(rootDir, fullScriptPath).replace(/\\/g, '/');
                console.log(`[Modules] Found renderer script, providing URL: /${relativeUrl}`);
                rendererScripts.push(relativeUrl);
            }
        });
        return rendererScripts;
    });
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
    processManager.stopMediaMTX();
    stopHlsServer();
    onAppWillQuit(event);
});