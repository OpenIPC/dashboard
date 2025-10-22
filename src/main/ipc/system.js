// --- START OF FILE src/main/ipc/system.js ---
const { ipcMain, shell, clipboard, BrowserWindow, dialog, app } = require('electron');
const fs = require('fs');
const path = require('path');
const log = require('electron-log');
const axios = require('axios');
const services = require('../services');
const processManager = require('../process-manager');
const configManager = require('../config-manager');
const brandingManager = require('../branding-manager');
const { autoUpdater } = require('electron-updater');
const CHANNELS = require('../../common/ipc-channels');
const os = require('os');

const withErrorHandling = (handler, context) => async (event, ...args) => {
    try {
        const result = await handler(event, ...args);
        return result === undefined ? { success: true } : result;
    } catch (error) {
        require('../services').handleError(error, context);
        return { success: false, error: error.message };
    }
};

function anonymizeConfig(config) {
    const safeConfig = JSON.parse(JSON.stringify(config)); 
    if (safeConfig.cameras && Array.isArray(safeConfig.cameras)) {
        safeConfig.cameras = safeConfig.cameras.map((cam, index) => ({
            id: `camera_${index}`, protocol: cam.protocol, streamPath0: cam.streamPath0,
            streamPath1: cam.streamPath1, port: cam.port, onvifAuth: cam.onvifAuth,
            name: `[REDACTED_NAME]`, ip: `[REDACTED_IP]`, username: `[REDACTED_USER]`,
        }));
    }
    if (safeConfig.appSettings && safeConfig.appSettings.recordingsPath) {
        safeConfig.appSettings.recordingsPath = '[REDACTED_PATH]';
    }
    if (safeConfig.users) {
        safeConfig.users = { user_count: Array.isArray(safeConfig.users) ? safeConfig.users.length : 0 };
    }
    delete safeConfig.groups;
    delete safeConfig.layouts;
    delete safeConfig.activeLayoutId;
    delete safeConfig.gridState;
    return safeConfig;
}

async function submitReport(event, { description, screenshots }) {
    try {
        const logPath = log.transports.file.getFile().path;
        const fullConfig = await configManager.loadConfiguration();
        const anonymizedConfig = anonymizeConfig(fullConfig);
    
        const report = {
            description, screenshots,
            systemInfo: JSON.stringify({
                os: `${os.type()} ${os.release()}`, arch: os.arch(),
                cpu: os.cpus()[0].model, version: app.getVersion()
            }, null, 2),
            logContent: fs.readFileSync(logPath, 'utf-8'),
            config: JSON.stringify(anonymizedConfig, null, 2)
        };
    
        const postData = JSON.stringify(report);
        const SCRIPT_URL = "https://script.google.com/macros/s/AKfycbwOYrQ131378iHYwZHVrlswLzco4e1_BJlI1W4Xj0q1y1J2OmDuXuz_9Jl6gI5bCwCiAA/exec";
        log.info('Submitting report to Google Apps Script...');
        const response = await axios.post(SCRIPT_URL, postData, {
            headers: { 'Content-Type': 'application/json' }, timeout: 60000 
        });
        log.info(`Report submission response: ${response.status}`, response.data);
        if (response.status === 200 && response.data.status === 'success') {
            return { success: true, messageKey: 'report_issue_success' };
        } else {
            const serverMessage = response.data.message || JSON.stringify(response.data);
            dialog.showErrorBox('Ошибка отправки', `Сервер ответил со статусом ${response.status}. Ответ: ${serverMessage}`);
        }
        return { success: true };
    } catch (error) {
        log.error('Failed to prepare and send report:', error);
        let errorMessage = `Не удалось отправить отчет: ${error.message}`;
        if (error.response) {
            errorMessage += `\nСервер ответил: ${error.response.status} ${error.response.statusText}`;
        } else if (error.request) {
            errorMessage += `\nОтвет от сервера не получен. Проверьте подключение к интернету.`;
        }
        dialog.showErrorBox('Критическая ошибка', errorMessage);
        return { success: false, error: error.message };
    }
}

function registerSystemHandlers(APP_VERSION, moduleManager) {
  const featureNotAvailableHandler = () => Promise.resolve({ success: false, error: 'Feature not available in Lite version' });

  // Window
  ipcMain.on(CHANNELS.MINIMIZE_WINDOW, (event) => BrowserWindow.fromWebContents(event.sender)?.minimize());
  ipcMain.on(CHANNELS.MAXIMIZE_WINDOW, (event) => { const win = BrowserWindow.fromWebContents(event.sender); if(win) { win.isMaximized() ? win.unmaximize() : win.maximize(); } });
  ipcMain.on(CHANNELS.CLOSE_WINDOW, (event) => BrowserWindow.fromWebContents(event.sender)?.close());
  ipcMain.on(CHANNELS.OPEN_EXTERNAL_LINK, (event, url) => shell.openExternal(url));

  // App Info
  ipcMain.handle(CHANNELS.GET_APP_VERSION_INFO, () => ({ version: app.getVersion(), type: APP_VERSION }));
  ipcMain.handle(CHANNELS.GET_BRANDING_CONFIG, () => brandingManager.getBrandingConfig());

  // System
  ipcMain.handle(CHANNELS.GET_SYSTEM_STATS, withErrorHandling(services.getSystemStats, 'getSystemStats'));
  ipcMain.handle(CHANNELS.KILL_ALL_FFMPEG, withErrorHandling(processManager.killAllFfmpeg, 'killAllFfmpeg'));
  ipcMain.handle(CHANNELS.CLIPBOARD_READ, withErrorHandling(() => clipboard.readText(), 'clipboardRead'));
  ipcMain.handle(CHANNELS.CLIPBOARD_WRITE, withErrorHandling((event, text) => clipboard.writeText(text), 'clipboardWrite'));
  ipcMain.on(CHANNELS.LOG_FROM_RENDERER, (event, { level, text }) => { if (log[level]) { log[level](`[Renderer] ${text}`); } });

  // Updates
  // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ --- VVVVVV
  // Передаем APP_VERSION в обработчик
  ipcMain.handle(CHANNELS.CHECK_FOR_UPDATES, withErrorHandling(() => services.checkForUpdates(APP_VERSION), 'checkForUpdates'));
  // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^
  ipcMain.handle(CHANNELS.DOWNLOAD_UPDATE, withErrorHandling(() => autoUpdater.downloadUpdate(), 'downloadUpdate'));
  ipcMain.on(CHANNELS.QUIT_AND_INSTALL_UPDATE, () => autoUpdater.quitAndInstall());
  
  // Reporting and Images
  ipcMain.handle(CHANNELS.SUBMIT_REPORT, APP_VERSION === 'intellect' ? withErrorHandling(submitReport, 'submitReport') : featureNotAvailableHandler);
  ipcMain.handle(CHANNELS.OPEN_IMAGE_FILES, withErrorHandling(async () => { const { canceled, filePaths } = await dialog.showOpenDialog({ title: 'Select Screenshots', properties: ['openFile', 'multiSelections'], filters: [{ name: 'Images', extensions: ['png', 'jpg', 'jpeg', 'gif'] }] }); if (canceled || !filePaths) return []; return filePaths.map(filePath => { try { const buffer = fs.readFileSync(filePath); const base64 = buffer.toString('base64'); const extension = path.extname(filePath).substring(1); return `data:image/${extension};base64,${base64}`; } catch (e) { console.error(`Failed to read file ${filePath}:`, e); return null; } }).filter(Boolean); }, 'openImageFiles'));
  
  // Modules (Intellect only)
    if (APP_VERSION === 'intellect') {
        const runtimeManager = require(path.resolve(__dirname, '../../../modules/license-plate/runtime-manager'));
    ipcMain.handle(CHANNELS.GET_AVAILABLE_MODULES, withErrorHandling(() => moduleManager.availableModules.map(mod => ({ id: mod.id, name: mod.name, version: mod.version, description: mod.description, author: mod.author })), 'getAvailableModules'));
    ipcMain.handle(CHANNELS.SAVE_ENABLED_MODULES, withErrorHandling(async (event, enabledIds) => { 
        const settings = await configManager.getAppSettings(); 
        settings.enabledModules = enabledIds; 
        await configManager.saveAppSettings(settings); 
        await dialog.showMessageBox({ type: 'info', title: 'Требуется перезапуск', message: 'Настройки модулей сохранены.', detail: 'Для применения изменений приложение необходимо перезапустить.', buttons: ['Перезапустить сейчас', 'Позже'], defaultId: 0, cancelId: 1 }).then(result => { if (result.response === 0) { app.relaunch(); app.quit(); } }); 
        return { success: true }; 
    }, 'saveEnabledModules'));
    ipcMain.handle(CHANNELS.PREPARE_LICENSE_PLATE_RUNTIME, withErrorHandling(async (event) => {
        const win = BrowserWindow.fromWebContents(event.sender);
        const apiShim = {
            sendToRenderer(channel, payload) {
                if (win && !win.isDestroyed()) {
                    win.webContents.send(channel, { ...payload, origin: 'settings-runtime-prepare' });
                }
            }
        };

        try {
            const status = await runtimeManager.getRuntimeStatus();
            if (status && (status.runtime || status.installed)) {
                apiShim.sendToRenderer('module-license-plate-runtime-progress', {
                    status: 'ready',
                    mode: status.runtime ? status.runtime.mode : 'cached',
                    version: status.runtime ? status.runtime.version : status.manifestVersion
                });
                return { success: true, alreadyInstalled: true, data: status };
            }
        } catch (err) {
            log.warn('[IPC] Failed to read runtime status before prepare', err);
        }

        const info = await runtimeManager.ensureRuntimeReady(apiShim, { preferDownload: true });
        return { success: true, data: info };
    }, 'prepareLicensePlateRuntime'));
    ipcMain.handle(CHANNELS.GET_DETECTED_PLATES, withErrorHandling(async () => {
        const licensePlateModule = moduleManager.loadedModules.get('license-plate');
        if (licensePlateModule && licensePlateModule.code && typeof licensePlateModule.code.getDetectedPlates === 'function') {
            return await licensePlateModule.code.getDetectedPlates();
        }
        return { totalDetections: 0, uniquePlates: [], recentHistory: [] };
    }, 'getDetectedPlates'));
  } else {
    ipcMain.handle(CHANNELS.GET_AVAILABLE_MODULES, featureNotAvailableHandler);
    ipcMain.handle(CHANNELS.SAVE_ENABLED_MODULES, featureNotAvailableHandler);
    ipcMain.handle(CHANNELS.GET_DETECTED_PLATES, featureNotAvailableHandler);
  }
}

module.exports = {
  registerSystemHandlers
};