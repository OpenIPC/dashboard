const { ipcMain, Menu, clipboard, dialog, shell, protocol, app, BrowserWindow } = require('electron');
const path = require('path');
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
        if (result === undefined) {
            return { success: true };
        }
        return result;
    } catch (error) {
        services.handleError(error, context);
        return { success: false, error: error.message };
    }
};


function registerIpcHandlers() {
    // --- Window Controls ---
    const handleWindowAction = (action) => (event) => {
        const win = BrowserWindow.fromWebContents(event.sender);
        if (win) {
            if (action === 'maximize') {
                win.isMaximized() ? win.unmaximize() : win.maximize();
            } else {
                win[action]();
            }
        }
    };
    ipcMain.on('minimize-window', handleWindowAction('minimize'));
    ipcMain.on('maximize-window', handleWindowAction('maximize'));
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
    ipcMain.handle('toggle-analytics', withErrorHandling((event, cameraId) => processManager.toggleAnalytics(cameraId, getMainWindow()), 'toggleAnalytics'));
    
    // --- Recording & Archive ---
    ipcMain.handle('start-recording', withErrorHandling((event, camera) => processManager.startRecording(camera, getMainWindow()), 'startRecording'));
    ipcMain.handle('stop-recording', withErrorHandling((event, cameraId) => processManager.stopRecording(cameraId), 'stopRecording'));
    ipcMain.handle('open-recordings-folder', withErrorHandling(async () => {
        const settings = await configManager.getAppSettings();
        await shell.openPath(settings.recordingsPath);
    }, 'openRecordingsFolder'));
    ipcMain.handle('get-recordings-for-date', withErrorHandling((event, data) => configManager.getRecordingsForDate(data), 'getRecordingsForDate'));
    ipcMain.handle('export-archive-clip', withErrorHandling((event, data) => processManager.exportArchiveClip(data, getMainWindow()), 'exportArchiveClip'));
    ipcMain.handle('get-events-for-date', withErrorHandling((event, data) => configManager.getEventsForDate(data), 'getEventsForDate'));
    ipcMain.handle('get-dates-with-activity', withErrorHandling((event, cameraName) => configManager.getDatesWithActivity(cameraName), 'getDatesWithActivity'));

    // --- System & Events ---
    ipcMain.handle('get-system-stats', withErrorHandling(services.getSystemStats, 'getSystemStats'));
    ipcMain.handle('kill-all-ffmpeg', withErrorHandling(processManager.killAllFfmpeg, 'killAllFfmpeg'));
    ipcMain.handle('check-for-updates', withErrorHandling(services.checkForUpdates, 'checkForUpdates'));

    // --- Discovery ---
    ipcMain.handle('discover-devices', withErrorHandling(() => discoverDevices(getMainWindow()), 'discoverDevices'));

    // --- NETIP ---
    ipcMain.handle('get-netip-settings', withErrorHandling((event, camera) => cameraAPI.getNetipSettings(camera), 'getNetipSettings'));
    ipcMain.handle('set-netip-settings', withErrorHandling((event, data) => cameraAPI.setNetipSettings(data), 'setNetipSettings'));

    // --- Context Menu ---
    ipcMain.on('show-camera-context-menu', (event, { cameraId, labels }) => {
        const menu = Menu.buildFromTemplate([
            { label: labels.open_in_browser, click: () => event.sender.send('context-menu-command', { command: 'open_in_browser', cameraId }) },
            { type: 'separator' },
            { label: labels.files, click: () => event.sender.send('context-menu-command', { command: 'files', cameraId }) },
            { label: labels.ssh, click: () => event.sender.send('context-menu-command', { command: 'ssh', cameraId }) },
            { label: labels.archive, click: () => event.sender.send('context-menu-command', { command: 'archive', cameraId }) },
            { label: labels.settings, click: () => event.sender.send('context-menu-command', { command: 'settings', cameraId }) },
            { label: labels.edit, click: () => event.sender.send('context-menu-command', { command: 'edit', cameraId }) },
            { type: 'separator' },
            { label: labels.delete, click: () => event.sender.send('context-menu-command', { command: 'delete', cameraId }) },
        ]);
        menu.popup({ window: BrowserWindow.fromWebContents(event.sender) });
    });

    // --- Helper Windows (File Manager & SSH) ---
    ipcMain.handle('open-file-manager', (e, camera) => createFileManagerWindow(camera, fileManagerConnections));
    ipcMain.handle('open-ssh-terminal', (e, camera) => {
        try {
            const win = createSshTerminalWindow(camera, sshConnections);
            if (win) { // Если было создано новое окно
                cameraAPI.setupSshConnection(win, camera, sshConnections);
            }
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

    // --- Protocol registration ---
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
}

module.exports = { registerIpcHandlers };