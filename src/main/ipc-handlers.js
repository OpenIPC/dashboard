// --- START OF FILE src/main/ipc-handlers.js ---
// Файл: /src/main/ipc-handlers.js
// Это центральный файл для всех IPC-обработчиков.

const { ipcMain, Menu, clipboard, dialog, shell, protocol, app, BrowserWindow } = require('electron');
const path = require('path');
const os = require('os');
const fs = require('fs');
const log = require('electron-log');
const axios = require('axios');

const { getMainWindow, createFileManagerWindow, createSshTerminalWindow } = require('./window-manager');
const configManager = require('./config-manager');
const authManager = require('./auth-manager');
const cameraAPI = require('./camera-api');
const processManager = require('./process-manager');
const services = require('./services');
const { discoverDevices } = require('./discovery');

const sshConnections = {};
const fileManagerConnections = {};

const withErrorHandling = (handler, context) => async (event, ...args) => {
    try {
        const result = await handler(event, ...args);
        return result === undefined ? { success: true } : result;
    } catch (error) {
        services.handleError(error, context);
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

function registerIpcHandlers(moduleManager) {
    // --- Window Controls ---
    const handleWindowAction = (action) => (event) => {
        const win = BrowserWindow.fromWebContents(event.sender);
        if (win) win[action]();
    };
    ipcMain.on('minimize-window', handleWindowAction('minimize'));
    ipcMain.on('maximize-window', (event) => {
        const win = BrowserWindow.fromWebContents(event.sender);
        if (win) win.isMaximized() ? win.unmaximize() : win.maximize();
    });
    ipcMain.on('close-window', handleWindowAction('close'));

    // --- Clipboard ---
    ipcMain.handle('clipboardRead', withErrorHandling(() => clipboard.readText(), 'clipboardRead'));
    ipcMain.handle('clipboardWrite', withErrorHandling((event, text) => clipboard.writeText(text), 'clipboardWrite'));
    
    // --- Authentication & Users ---
    ipcMain.handle('login', withErrorHandling((event, creds) => authManager.handleLogin(creds), 'login'));
    ipcMain.on('renderer-ready-for-autologin', () => authManager.handleAutoLogin(getMainWindow()));
    ipcMain.on('logout-clear-credentials', authManager.clearAutoLoginCredentials);
    ipcMain.handle('get-users', withErrorHandling(authManager.getUsers, 'getUsers'));
    ipcMain.handle('add-user', withErrorHandling((event, data) => authManager.addUser(data), 'addUser'));
    ipcMain.handle('update-user-password', withErrorHandling((event, data) => authManager.updateUserPassword(data), 'updateUserPassword'));
    ipcMain.handle('update-user-role', withErrorHandling((event, data) => authManager.updateUserRole(data), 'updateUserRole'));
    ipcMain.handle('update-user-permissions', withErrorHandling((event, data) => authManager.updateUserPermissions(data), 'updateUserPermissions'));
    ipcMain.handle('delete-user', withErrorHandling((event, data) => authManager.deleteUser(data), 'deleteUser'));

    // --- App Settings & Config ---
    ipcMain.handle('load-app-settings', withErrorHandling(configManager.getAppSettings, 'loadAppSettings'));
    ipcMain.handle('save-app-settings', withErrorHandling((event, settings) => configManager.saveAppSettings(settings), 'saveAppSettings'));
    ipcMain.handle('load-configuration', withErrorHandling(configManager.loadConfiguration, 'loadConfiguration'));
    ipcMain.handle('save-configuration', withErrorHandling((event, config) => configManager.saveConfiguration(config), 'saveConfiguration'));
    ipcMain.handle('select-directory', withErrorHandling(() => dialog.showOpenDialog(getMainWindow(), { properties: ['openDirectory'] }), 'selectDirectory'));
    ipcMain.handle('get-translation-file', withErrorHandling((event, lang) => configManager.getTranslationFile(lang), 'getTranslationFile'));
    ipcMain.handle('export-config', withErrorHandling(() => configManager.exportConfig(getMainWindow()), 'exportConfig'));
    ipcMain.handle('import-config', withErrorHandling(() => configManager.importConfig(getMainWindow()), 'importConfig'));

    // VVVVVV --- ДОБАВЛЕНЫ НОВЫЕ ОБРАБОТЧИКИ --- VVVVVV
    ipcMain.handle('get-app-version', () => {
        return app.getVersion();
    });
    ipcMain.on('open-external-link', (event, url) => {
        shell.openExternal(url);
    });
    // ^^^^^^ --- КОНЕЦ НОВЫХ ОБРАБОТЧИКОВ --- ^^^^^^

    // --- Camera Actions & Info ---
    ipcMain.handle('get-camera-pulse', withErrorHandling((event, camera) => cameraAPI.getCameraPulse(camera), 'getCameraPulse'));
    ipcMain.handle('ptz-control', withErrorHandling((event, data) => cameraAPI.ptzControl(data), 'ptzControl'));
    ipcMain.handle('get-camera-time', withErrorHandling((event, camera) => cameraAPI.getCameraTime(camera), 'getCameraTime'));
    ipcMain.handle('get-camera-settings', withErrorHandling((event, camera) => cameraAPI.getCameraSettings(camera), 'getCameraSettings'));
    ipcMain.handle('set-camera-settings', withErrorHandling((event, data) => cameraAPI.setCameraSettings(data), 'setCameraSettings'));
    ipcMain.handle('restart-majestic', withErrorHandling((event, camera) => cameraAPI.restartMajestic(camera), 'restartMajestic'));
    ipcMain.handle('open-in-browser', withErrorHandling((event, ip) => shell.openExternal(`http://${ip}`), 'openInBrowser'));

    // --- Video Streaming ---
    ipcMain.handle('start-video-stream', withErrorHandling((event, data) => processManager.startVideoStream(data), 'startVideoStream'));
    ipcMain.handle('stop-video-stream', withErrorHandling((event, streamId) => processManager.stopVideoStream(streamId), 'stopVideoStream'));

    // --- Video Analytics ---
    ipcMain.handle('toggle-analytics', withErrorHandling((event, cameraId) => processManager.toggleAnalytics(cameraId, getMainWindow(), moduleManager), 'toggleAnalytics'));
    
    // --- Recording & Archive ---
    ipcMain.handle('start-recording', withErrorHandling(async (event, camera) => {
        const result = await processManager.startRecording(camera, getMainWindow());
        getMainWindow()?.webContents.send('recording-state-change', { cameraId: camera.id, recording: true });
        return result;
    }, 'startRecording'));
    ipcMain.handle('stop-recording', withErrorHandling(async (event, cameraId) => {
        const result = await processManager.stopRecording(cameraId);
        getMainWindow()?.webContents.send('recording-state-change', { cameraId: cameraId, recording: false });
        return result;
    }, 'stopRecording'));
    
    ipcMain.handle('open-recordings-folder', withErrorHandling(async () => {
        const settings = await configManager.getAppSettings();
        await shell.openPath(settings.recordingsPath);
    }, 'openRecordingsFolder'));
    ipcMain.handle('get-recordings-for-date', withErrorHandling((event, data) => configManager.getRecordingsForDate(data), 'getRecordingsForDate'));
    ipcMain.handle('export-archive-clip', withErrorHandling((event, data) => processManager.exportArchiveClip(data, getMainWindow()), 'exportArchiveClip'));
    ipcMain.handle('get-events-for-date', withErrorHandling((event, data) => configManager.getEventsForDate(data), 'getEventsForDate'));
    ipcMain.handle('get-dates-with-activity', withErrorHandling((event, cameraName) => configManager.getDatesWithActivity(cameraName), 'getDatesWithActivity'));
    // VVVVVV --- ИЗМЕНЕНИЕ: НОВЫЙ ОБРАБОТЧИК ДЛЯ HLS --- VVVVVV
    ipcMain.handle('prepare-archive-for-hls', withErrorHandling((event, filename) => processManager.prepareArchiveForHls(filename), 'prepareArchiveForHls'));
    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

    // --- System, Updates, Reporting ---
    ipcMain.handle('get-system-stats', withErrorHandling(services.getSystemStats, 'getSystemStats'));
    ipcMain.handle('kill-all-ffmpeg', withErrorHandling(processManager.killAllFfmpeg, 'killAllFfmpeg'));
    ipcMain.handle('check-for-updates', withErrorHandling(services.checkForUpdates, 'checkForUpdates'));
    ipcMain.handle('submit-report', withErrorHandling(submitReport, 'submitReport'));
    ipcMain.handle('open-and-read-image-files', withErrorHandling(async () => {
        const { canceled, filePaths } = await dialog.showOpenDialog({
            title: 'Select Screenshots', properties: ['openFile', 'multiSelections'],
            filters: [{ name: 'Images', extensions: ['png', 'jpg', 'jpeg', 'gif'] }]
        });
        if (canceled || !filePaths) return [];
        return filePaths.map(filePath => {
            try {
                const buffer = fs.readFileSync(filePath);
                const base64 = buffer.toString('base64');
                const extension = path.extname(filePath).substring(1);
                return `data:image/${extension};base64,${base64}`;
            } catch (e) {
                console.error(`Failed to read file ${filePath}:`, e);
                return null;
            }
        }).filter(Boolean);
    }, 'openImageFiles'));

    // --- Discovery & NETIP ---
    ipcMain.handle('discover-devices', withErrorHandling(() => discoverDevices(getMainWindow()), 'discoverDevices'));
    ipcMain.handle('get-netip-settings', withErrorHandling((event, camera) => cameraAPI.getNetipSettings(camera), 'getNetipSettings'));
    ipcMain.handle('set-netip-settings', withErrorHandling((event, data) => cameraAPI.setNetipSettings(data), 'setNetipSettings'));

    // --- Context Menu ---
    ipcMain.on('show-camera-context-menu', (event, { cameraId, labels }) => {
        const template = [];
        const commands = [
            'open_in_browser', 'files', 'ssh', 'archive', 
            'settings', 'edit', 'delete'
        ];
        
        commands.forEach(command => {
            if (command === 'files' || command === 'delete') {
                if (template.length > 0 && template[template.length - 1].type !== 'separator') {
                    template.push({ type: 'separator' });
                }
            }
            
            if (labels[command]) {
                template.push({
                    label: labels[command],
                    click: () => {
                        getMainWindow()?.webContents.send('context-menu-command', { command, cameraId });
                    }
                });
            }
        });

        if (template.length > 0) {
            const menu = Menu.buildFromTemplate(template);
            menu.popup({ window: getMainWindow() });
        }
    });

    ipcMain.on('show-group-context-menu', (event, { groupId, labels }) => {
        const menu = Menu.buildFromTemplate([
            {
                label: labels.rename,
                click: () => event.sender.send('group-context-menu-command', { command: 'rename', groupId })
            },
            {
                label: labels.delete,
                click: () => event.sender.send('group-context-menu-command', { command: 'delete', groupId })
            },
        ]);
        menu.popup({ window: BrowserWindow.fromWebContents(event.sender) });
    });

    // --- Helper Windows (File Manager & SSH) ---
    ipcMain.handle('open-file-manager', (e, camera) => createFileManagerWindow(camera, fileManagerConnections));
    ipcMain.handle('open-ssh-terminal', (e, camera) => { 
        try {
            const win = createSshTerminalWindow(camera, sshConnections);
            if (win) cameraAPI.setupSshConnection(win, camera, sshConnections);
        } catch (error) {
            services.handleError(error, 'openSshTerminal');
        }
    });
    
    // --- SCP Handlers ---
    ipcMain.handle('scp-connect', withErrorHandling((e, camera) => cameraAPI.scp.connect(camera, fileManagerConnections), 'scpConnect'));
    ipcMain.handle('scp-list', withErrorHandling((e, data) => cameraAPI.scp.list(data, fileManagerConnections), 'scpList'));
    ipcMain.handle('scp-download', withErrorHandling((e, data) => cameraAPI.scp.download(e, data, fileManagerConnections), 'scpDownload'));
    ipcMain.handle('scp-upload', withErrorHandling((e, data) => cameraAPI.scp.upload(e, data, fileManagerConnections), 'scpUpload'));
    ipcMain.handle('scp-mkdir', withErrorHandling((e, data) => cameraAPI.scp.mkdir(data, fileManagerConnections), 'scpMkdir'));
    ipcMain.handle('scp-delete-file', withErrorHandling((e, data) => cameraAPI.scp.deleteFile(data, fileManagerConnections), 'scpDeleteFile'));
    ipcMain.handle('scp-delete-dir', withErrorHandling((e, data) => cameraAPI.scp.deleteDir(data, fileManagerConnections), 'scpDeleteDir'));

    // --- Local Filesystem Handlers ---
    ipcMain.handle('get-local-disk-list', withErrorHandling(configManager.getLocalDiskList, 'getLocalDiskList'));
    ipcMain.handle('list-local-files', withErrorHandling((e, path) => configManager.listLocalFiles(path), 'listLocalFiles'));

    // --- Protocol & Logging ---
    protocol.registerFileProtocol('video-archive', async (request, callback) => {
        try {
            const settings = await configManager.getAppSettings();
            const recordingsPath = settings.recordingsPath;
            const filename = decodeURIComponent(request.url.replace('video-archive://', ''));
            const filePath = path.join(recordingsPath, filename);
            if (path.dirname(filePath) !== path.resolve(recordingsPath)) {
                console.error(`[Security] Blocked path traversal attempt: ${filePath}`);
                return callback({ error: -6 });
            }
            callback({ path: filePath });
        } catch (error) {
            services.handleError(error, 'videoArchiveProtocol');
            callback({ error: -2 });
        }
    });
    ipcMain.on('log', (event, { level, text }) => { if (log[level]) { log[level](`[Renderer] ${text}`); } });

    // --- Module System Handlers ---
    ipcMain.handle('get-available-modules', withErrorHandling(() => {
        return moduleManager.availableModules.map(mod => ({
            id: mod.id, name: mod.name, version: mod.version,
            description: mod.description, author: mod.author
        }));
    }, 'getAvailableModules'));

    ipcMain.handle('save-enabled-modules', withErrorHandling(async (event, enabledIds) => {
        const settings = await configManager.getAppSettings();
        settings.enabledModules = enabledIds;
        await configManager.saveAppSettings(settings);
        await dialog.showMessageBox({
            type: 'info', title: 'Требуется перезапуск',
            message: 'Настройки модулей сохранены.',
            detail: 'Для применения изменений приложение необходимо перезапустить.',
            buttons: ['Перезапустить сейчас', 'Позже'], defaultId: 0, cancelId: 1
        }).then(result => {
            if (result.response === 0) {
                app.relaunch();
                app.quit();
            }
        });
        return { success: true };
    }, 'saveEnabledModules'));
}

module.exports = { registerIpcHandlers };
// --- END OF FILE src/main/ipc-handlers.js ---