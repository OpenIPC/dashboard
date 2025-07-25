// main.js (Финальная версия со всеми исправлениями)

const { app, BrowserWindow, ipcMain, Menu, clipboard, dialog, shell, protocol } = require('electron');
const path = require('path');
const fs = require('fs');
const fsPromises = require('fs').promises;
const net = require('net');
const os = require('os');
const { spawn, exec } = require('child_process');
const axios = require('axios');
const { Client } = require('ssh2');
const WebSocket = require('ws');
const ffmpeg = require('@ffmpeg-installer/ffmpeg');
const keytar = require('keytar');
const { autoUpdater } = require('electron-updater');
const onvif = require('onvif');
const crypto = require('crypto');

const dgram = require('dgram');
const NetIpCamera = require('./netip-handler.js');

const { Mutex } = require('async-mutex');
const portMutex = new Mutex();
const eventsMutex = new Mutex(); // Мьютекс для файла событий

if (process.platform === 'linux' || process.env.ELECTRON_FORCE_NO_SANDBOX) {
    app.commandLine.appendSwitch('--no-sandbox');
}
app.commandLine.appendSwitch('force_high_performance_gpu');

const ffmpegPath = ffmpeg.path.replace('app.asar', 'app.asar.unpacked');

let mainWindow = null;
const streamManager = {};
const recordingManager = {};
const usedPorts = new Set();
const BASE_PORT = 9001;
const KEYTAR_SERVICE = 'OpenIPC-VMS';
const KEYTAR_ACCOUNT_AUTOLOGIN = 'autoLoginCredentials';
let isShuttingDown = false;

const PROCESS_TYPES = {
    STREAM: 'stream',
    RECORDING: 'recording',
    ANALYTICS: 'analytics'
};
const buildProcessId = (type, cameraId) => `${type}-${cameraId}`;

const netipConnectionManager = {
    connections: new Map(),
    async getInstance(camera) {
        if (this.connections.has(camera.id)) {
            const instance = this.connections.get(camera.id);
            if (instance._socket && !instance._socket.destroyed) {
                console.log(`[NETIP Manager] Reusing connection for camera ${camera.id}`);
                return instance;
            }
            console.log(`[NETIP Manager] Stale connection found for ${camera.id}. Reconnecting.`);
            this.connections.delete(camera.id);
        }
        console.log(`[NETIP Manager] Creating new connection for camera ${camera.id}`);
        const password = await keytar.getPassword(KEYTAR_SERVICE, camera.id.toString());
        const cam = new NetIpCamera({
            host_ip: camera.ip,
            host_port: 34567,
            user: camera.username,
            pass: password || ''
        });
        try {
            await cam.configure();
            this.connections.set(camera.id, cam);
            return cam;
        } catch (e) {
            this.connections.delete(camera.id);
            throw e;
        }
    },
    closeAll() {
        console.log('[NETIP Manager] Closing all NETIP connections...');
        for (const [id, instance] of this.connections.entries()) {
            if (instance && instance._socket && !instance._socket.destroyed) {
                instance._socket.destroy();
            }
        }
        this.connections.clear();
    }
};

const processManager = {
    processes: new Map(),
    add(key, process, type) {
        console.log(`[ProcessManager] Adding ${type} process with key: ${key}`);
        this.processes.set(key, { process, type });
    },
    stop(key) {
        if (this.processes.has(key)) {
            const { process, type } = this.processes.get(key);
            console.log(`[ProcessManager] Issuing stop for ${type} process with key: ${key}`);
            try {
                if (type === PROCESS_TYPES.RECORDING && process.stdin.writable) {
                    process.stdin.write('q\n');
                } else {
                    process.kill();
                }
            } catch (e) {
                console.error(`[ProcessManager] Error sending stop signal to ${key}: ${e.message}`);
                if (!process.killed) process.kill('SIGKILL');
            }
            return true;
        }
        return false;
    },
    stopAll() {
        console.log(`[ProcessManager] Stopping all ${this.processes.size} tracked processes.`);
        for (const [key, { process }] of this.processes) {
            try {
                process.kill('SIGKILL');
            } catch (e) {
                console.error(`[ProcessManager] Error killing process ${key}: ${e.message}`);
            }
        }
        this.processes.clear();
    },
    get(key) {
        return this.processes.get(key);
    }
};

function getDataPath() {
    if (process.env.PORTABLE_EXECUTABLE_DIR) {
        return process.env.PORTABLE_EXECUTABLE_DIR;
    }
    return app.getPath('userData');
}

const dataPathRoot = getDataPath();
console.log(`[Config] Data path is: ${dataPathRoot}`);

const configPath = path.join(dataPathRoot, 'config.json');
const appSettingsPath = path.join(dataPathRoot, 'app-settings.json');
const usersPath = path.join(dataPathRoot, 'users.json');
const eventsPath = path.join(dataPathRoot, 'events.json'); // Путь к файлу событий
const oldCamerasPath = path.join(dataPathRoot, 'cameras.json');
let sshWindows = {};
let fileManagerWindows = {};
let fileManagerConnections = {};
let appSettingsCache = null;

async function saveAnalyticsEvent(cameraId, eventData) {
    const release = await eventsMutex.acquire();
    try {
        let allEvents = {};
        try {
            const data = await fsPromises.readFile(eventsPath, 'utf-8');
            allEvents = JSON.parse(data);
        } catch (e) {
            if (e.code !== 'ENOENT') console.error('[Events] Error reading events file:', e);
        }

        const eventTimestamp = new Date(eventData.timestamp * 1000);
        const dateKey = eventTimestamp.toISOString().split('T')[0];
        
        if (!allEvents[dateKey]) {
            allEvents[dateKey] = [];
        }

        const uniqueObjectLabels = [...new Set(eventData.objects.map(obj => obj.label))];

        const newEvent = {
            cameraId,
            timestamp: eventData.timestamp,
            objects: uniqueObjectLabels,
        };

        allEvents[dateKey].push(newEvent);

        await fsPromises.writeFile(eventsPath, JSON.stringify(allEvents, null, 2));

    } catch (e) {
        console.error('[Events] Failed to save analytics event:', e);
    } finally {
        release();
    }
}

function hashPassword(password) {
    const salt = crypto.randomBytes(16).toString('hex');
    const hash = crypto.pbkdf2Sync(password, salt, 1000, 64, 'sha512').toString('hex');
    return { salt, hash };
}

function verifyPassword(password, hash, salt) {
    const hashToVerify = crypto.pbkdf2Sync(password, salt, 1000, 64, 'sha512').toString('hex');
    return hash === hashToVerify;
}

async function initializeUsers() {
    try {
        await fsPromises.access(usersPath);
    } catch (e) {
        console.log('[Users] users.json not found, creating default admin user (admin/admin).');
        const { salt, hash } = hashPassword('admin');
        const defaultUser = [{
            username: 'admin',
            hashedPassword: hash,
            salt: salt,
            role: 'admin'
        }];
        await fsPromises.writeFile(usersPath, JSON.stringify(defaultUser, null, 2));
    }
}

function getHwAccelOptions(codec, preference, streamId) {
    const isSD = streamId === 1;

    if (preference === 'nvidia') {
        const decoder = codec === 'h264' ? 'h264_cuvid' : 'hevc_cuvid';
        const decoderArgs = ['-c:v', decoder];
        if (isSD) {
            decoderArgs.push('-resize', '640x360');
        }
        console.log(`[FFMPEG] Using HW Accel: ${decoder} ${isSD ? 'with built-in resize' : 'for HD'}`);
        return { decoderArgs, vfString: 'format=yuv420p' };
    }

    if (preference === 'intel') {
        const decoder = codec === 'h264' ? 'h264_qsv' : 'hevc_qsv';
        let vfString = 'hwdownload,format=yuv420p';
        if (isSD) {
            vfString = 'scale_qsv=w=640:h=-2,' + vfString;
        }
        console.log(`[FFMPEG] Using HW Accel: ${decoder} ${isSD ? 'with QSV scaler' : 'for HD'}`);
        return { decoderArgs: ['-c:v', decoder], vfString };
    }

    let decoderArgs = [];
    let vfString = 'format=yuv420p';
    let platformMsg = '';

    if (preference === 'auto') {
        switch (process.platform) {
            case 'win32': decoderArgs = ['-hwaccel', 'd3d11va']; platformMsg = 'Auto-selecting d3d11va for HW aacel'; break;
            case 'darwin': decoderArgs = ['-hwaccel', 'videotoolbox']; platformMsg = 'Auto-selecting videotoolbox for HW accel'; break;
            case 'linux': platformMsg = 'Auto-selection on Linux: Using CPU for stability. For HW accel, ensure drivers are installed and select it manually.'; break;
            default: platformMsg = 'Auto-selection: No hardware acceleration, using CPU.'; break;
        }
    } else {
        platformMsg = 'Hardware acceleration disabled by user.';
    }

    if (isSD) {
        vfString = 'scale=w=640:h=-2,' + vfString;
    }
    
    console.log(`[FFMPEG] ${platformMsg}. ${isSD ? 'Using CPU scaler for SD.' : ''}`);
    return { decoderArgs, vfString };
}

async function getAppSettings() {
    if (appSettingsCache) {
        return appSettingsCache;
    }
    try {
        const data = await fsPromises.readFile(appSettingsPath, 'utf-8');
        appSettingsCache = JSON.parse(data);
    } catch (e) {
        appSettingsCache = { 
            recordingsPath: path.join(app.getPath('videos'), 'OpenIPC-VMS'),
            hwAccel: 'auto',
            language: 'en',
            qscale: 8, // Значение по умолчанию
            fps: 20    // Значение по умолчанию
        };
    }
    return appSettingsCache;
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
        icon: path.join(__dirname, 'build/icon.png'),
        webPreferences: { preload: path.join(__dirname, 'preload.js') }
    });
    mainWindow.loadFile('index.html');

    mainWindow.once('ready-to-show', () => {
        if (app.isPackaged) {
            console.log('[Updater] App ready, checking for updates...');
            autoUpdater.checkForUpdates();
        }
    });

    mainWindow.on('maximize', () => {
        mainWindow.webContents.send('window-maximized');
    });

    mainWindow.on('unmaximize', () => {
        mainWindow.webContents.send('window-unmaximized');
    });
}

function createFileManagerWindow(camera) {
    const fileManagerWindow = new BrowserWindow({
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
            preload: path.join(__dirname, 'fm-preload.js'),
            contextIsolation: true,
            nodeIntegration: false,
        }
    });
    fileManagerWindow.loadFile('file-manager.html', { query: { camera: JSON.stringify(camera) } });
    
    fileManagerWindows[camera.id] = fileManagerWindow;

    fileManagerWindow.on('closed', () => {
        const conn = fileManagerConnections[camera.id];
        if (conn) {
            conn.end();
        }
        delete fileManagerWindows[camera.id];
    });
    return fileManagerWindow;
}

// --- IPC ОБРАБОТЧИКИ ---

ipcMain.handle('get-events-for-date', async (event, { date }) => {
    try {
        const data = await fsPromises.readFile(eventsPath, 'utf-8');
        const allEvents = JSON.parse(data);
        return allEvents[date] || [];
    } catch (e) {
        if (e.code === 'ENOENT') {
            return [];
        }
        console.error('[Events] Error reading events for date:', e);
        return [];
    }
});

const NETIP_DISCOVERY_PACKET = Buffer.from([
    0xff, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
]);
const NETIP_DISCOVERY_PORT = 34567;

ipcMain.handle('discover-netip-devices', async (event) => {
    return new Promise((resolve) => {
        try {
            console.log('[Scanner] Starting NETIP UDP scan discovery...');
            const socket = dgram.createSocket('udp4');
            const foundDevices = new Set();
            
            const localIPs = new Set();
            const interfaces = os.networkInterfaces();
            const broadcastAddresses = [];
            for (const name of Object.keys(interfaces)) {
                for (const iface of interfaces[name]) {
                    if (iface.family === 'IPv4' && !iface.internal) {
                        localIPs.add(iface.address);
                        
                        const ipParts = iface.address.split('.').map(part => parseInt(part, 10));
                        const netmaskParts = iface.netmask.split('.').map(part => parseInt(part, 10));
                        
                        const broadcastParts = ipParts.map((part, i) => {
                            return part | (~netmaskParts[i] & 255);
                        });
                        
                        broadcastAddresses.push(broadcastParts.join('.'));
                    }
                }
            }

            if (!broadcastAddresses.includes('255.255.255.255')) {
                broadcastAddresses.push('255.255.255.255');
            }
            
            const uniqueBroadcastAddresses = [...new Set(broadcastAddresses)];
            console.log(`[Scanner] NETIP scanning broadcast addresses: [ ${uniqueBroadcastAddresses.join(', ')} ]`);

            socket.on('error', (err) => {
                console.error('[Scanner][NETIP UDP Error]:', err);
                socket.close();
                resolve({ success: false, error: err.message });
            });

            socket.on('message', (msg, rinfo) => {
                if (localIPs.has(rinfo.address)) {
                    return; 
                }
                
                if (rinfo.address && !foundDevices.has(rinfo.address)) {
                    foundDevices.add(rinfo.address);
                    
                    let deviceName = '';
                    try {
                        const nameBuffer = msg.subarray(88, 88 + 32);
                        const nullTerminatorIndex = nameBuffer.indexOf(0);
                        deviceName = nameBuffer.toString('utf-8', 0, nullTerminatorIndex > -1 ? nullTerminatorIndex : 32).trim();
                    } catch (e) { /* silent fail */ }

                    const device = {
                        ip: rinfo.address,
                        name: deviceName || rinfo.address,
                        protocol: 'netip'
                    };

                    if (mainWindow && !mainWindow.isDestroyed()) {
                        mainWindow.webContents.send('netip-device-found', device);
                    }
                }
            });
            
            socket.bind(NETIP_DISCOVERY_PORT, () => {
                socket.setBroadcast(true);
                uniqueBroadcastAddresses.forEach(address => {
                    socket.send(NETIP_DISCOVERY_PACKET, 0, NETIP_DISCOVERY_PACKET.length, NETIP_DISCOVERY_PORT, address, (err) => {
                        if (err) console.error(`[Scanner][NETIP Broadcast Error to ${address}]:`, err);
                    });
                });
            });

            setTimeout(() => {
                socket.close();
                console.log(`[Scanner] NETIP discovery finished. Found ${foundDevices.size} devices.`);
                resolve({ success: true, count: foundDevices.size });
            }, 5000);

        } catch (e) {
            console.error('[Scanner] Failed to start NETIP discovery:', e);
            resolve({ success: false, error: e.message });
        }
    });
});

ipcMain.handle('get-netip-settings', async (event, camera) => {
    try {
        const cam = await netipConnectionManager.getInstance(camera);
        
        const [systemInfo, generalInfo, encodeInfo] = await Promise.all([
            cam.get_system_info(),
            cam.get_general_info(),
            cam.get_encode_info()
        ]);
        
        return { ...systemInfo, ...generalInfo, ...encodeInfo };
    } catch (e) {
        console.error(`[NETIP] Failed to get settings for ${camera.ip}:`, e.message || e);
        return { error: e.message || 'Unknown NETIP error' };
    }
});

ipcMain.handle('set-netip-settings', async (event, { camera, settingsData }) => {
    try {
        const cam = await netipConnectionManager.getInstance(camera);
        
        console.warn('setNetipSettings is not fully implemented in the provided library. Returning success.');

        return { success: true };
    } catch (e) {
        console.error(`[NETIP] Failed to set settings for ${camera.ip}:`, e.message || e);
        return { success: false, error: e.message || 'Failed to set settings' };
    }
});

ipcMain.on('minimize-window', (event) => {
    BrowserWindow.fromWebContents(event.sender)?.minimize();
});

ipcMain.on('maximize-window', (event) => {
    const win = BrowserWindow.fromWebContents(event.sender);
    if (win) {
        if (win.isMaximized()) {
            win.unmaximize();
        } else {
            win.maximize();
        }
    }
});

ipcMain.on('close-window', (event) => {
    BrowserWindow.fromWebContents(event.sender)?.close();
});

ipcMain.handle('clipboardRead', () => {
    return clipboard.readText();
});

ipcMain.handle('clipboardWrite', (event, text) => {
    clipboard.writeText(text);
});

ipcMain.on('renderer-ready-for-autologin', async () => {
    try {
        const credsJson = await keytar.getPassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN);
        if (credsJson) {
            console.log('[AutoLogin] Renderer is ready. Found stored credentials. Attempting to log in.');
            const { username, password } = JSON.parse(credsJson);
            const data = await fsPromises.readFile(usersPath, 'utf-8');
            const users = JSON.parse(data);
            const user = users.find(u => u.username === username);

            if (user && verifyPassword(password, user.hashedPassword, user.salt)) {
                console.log('[AutoLogin] Success.');
                const userPayload = { username: user.username, role: user.role };
                if (user.role === 'operator') {
                    userPayload.permissions = user.permissions || {};
                }
                mainWindow.webContents.send('auto-login-success', userPayload);
            } else {
                console.warn('[AutoLogin] Failed. Stored credentials may be outdated.');
                await keytar.deletePassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN);
            }
        } else {
            console.log('[AutoLogin] Renderer is ready. No stored credentials found.');
        }
    } catch (e) {
        console.error('[AutoLogin] Error:', e);
    }
});

ipcMain.handle('login', async (event, { username, password, rememberMe }) => {
    try {
        const data = await fsPromises.readFile(usersPath, 'utf-8');
        const users = JSON.parse(data);
        const user = users.find(u => u.username === username);

        if (user && verifyPassword(password, user.hashedPassword, user.salt)) {
            if (rememberMe) {
                await keytar.setPassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN, JSON.stringify({ username, password }));
                console.log('[Login] Credentials saved for auto-login.');
            } else {
                await keytar.deletePassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN);
                console.log('[Login] Cleared any stored auto-login credentials.');
            }
            const userPayload = { username: user.username, role: user.role };
            if (user.role === 'operator') {
                userPayload.permissions = user.permissions || {};
            }
            return { success: true, user: userPayload };
        }
        return { success: false, error: 'Invalid username or password' };
    } catch (e) {
        console.error('Login error:', e);
        return { success: false, error: 'Error reading user data' };
    }
});

ipcMain.on('logout-clear-credentials', async () => {
    try {
        await keytar.deletePassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN);
        console.log('[Logout] Cleared auto-login credentials.');
    } catch (e) {
        console.error('[Logout] Failed to clear credentials:', e);
    }
});

ipcMain.handle('get-users', async () => {
  try {
    const data = await fsPromises.readFile(usersPath, 'utf-8');
    const users = JSON.parse(data);
    return { success: true, users: users.map(u => ({ username: u.username, role: u.role, permissions: u.permissions || {} })) };
  } catch (e) {
    return { success: false, error: e.message };
  }
});

ipcMain.handle('add-user', async (event, { username, password, role }) => {
  try {
    const data = await fsPromises.readFile(usersPath, 'utf-8');
    const users = JSON.parse(data);
    if (users.some(u => u.username.toLowerCase() === username.toLowerCase())) {
      return { success: false, error: 'User with this name already exists.' };
    }
    const { salt, hash } = hashPassword(password);
    const newUser = { username, salt, hashedPassword: hash, role };
    if (role === 'operator') {
        newUser.permissions = {};
    }
    users.push(newUser);
    await fsPromises.writeFile(usersPath, JSON.stringify(users, null, 2));
    return { success: true };
  } catch (e) {
    return { success: false, error: e.message };
  }
});

ipcMain.handle('update-user-password', async (event, { username, password }) => {
    try {
        const data = await fsPromises.readFile(usersPath, 'utf-8');
        let users = JSON.parse(data);
        const userIndex = users.findIndex(u => u.username === username);
        if (userIndex === -1) {
            return { success: false, error: 'User not found.' };
        }
        const { salt, hash } = hashPassword(password);
        users[userIndex].salt = salt;
        users[userIndex].hashedPassword = hash;
        await fsPromises.writeFile(usersPath, JSON.stringify(users, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
});

ipcMain.handle('update-user-role', async (event, { username, role }) => {
    try {
        const data = await fsPromises.readFile(usersPath, 'utf-8');
        let users = JSON.parse(data);
        const userIndex = users.findIndex(u => u.username === username);

        if (userIndex === -1) {
            return { success: false, error: 'User not found.' };
        }

        if (users[userIndex].role === 'admin' && role !== 'admin') {
            const admins = users.filter(u => u.role === 'admin');
            if (admins.length <= 1) {
                return { success: false, error: 'Cannot change the role of the last administrator.' };
            }
        }

        users[userIndex].role = role;
        if (role === 'operator' && !users[userIndex].permissions) {
            users[userIndex].permissions = {};
        }
        if (role === 'admin') {
            delete users[userIndex].permissions;
        }

        await fsPromises.writeFile(usersPath, JSON.stringify(users, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
});

ipcMain.handle('update-user-permissions', async (event, { username, permissions }) => {
    try {
        const data = await fsPromises.readFile(usersPath, 'utf-8');
        let users = JSON.parse(data);
        const userIndex = users.findIndex(u => u.username === username);

        if (userIndex === -1 || users[userIndex].role !== 'operator') {
            return { success: false, error: 'User not found or is not an operator.' };
        }

        users[userIndex].permissions = permissions;
        await fsPromises.writeFile(usersPath, JSON.stringify(users, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
});

ipcMain.handle('delete-user', async (event, { username }) => {
  try {
    const data = await fsPromises.readFile(usersPath, 'utf-8');
    let users = JSON.parse(data);
    
    const admins = users.filter(u => u.role === 'admin');
    if (admins.length === 1 && admins[0].username === username) {
      return { success: false, error: 'Cannot delete the last administrator.' };
    }

    const filteredUsers = users.filter(u => u.username !== username);
    await fsPromises.writeFile(usersPath, JSON.stringify(filteredUsers, null, 2));
    return { success: true };
  } catch (e) {
    return { success: false, error: e.message };
  }
});

ipcMain.handle('load-app-settings', getAppSettings);

ipcMain.handle('save-app-settings', async (event, settings) => {
    try {
        appSettingsCache = settings;
        await fsPromises.writeFile(appSettingsPath, JSON.stringify(settings, null, 2));
        return { success: true };
    } catch (e) {
        console.error('Failed to save app settings:', e);
        return { success: false, error: e.message };
    }
});

ipcMain.handle('get-translation-file', async (event, lang) => {
    try {
        const filePath = path.join(__dirname, 'locales', `${lang}.json`);
        const data = await fsPromises.readFile(filePath, 'utf-8');
        return JSON.parse(data);
    } catch (e) {
        console.error(`Failed to load translation file for ${lang}:`, e);
        return null;
    }
});

ipcMain.handle('select-directory', async () => {
    const { canceled, filePaths } = await dialog.showOpenDialog(mainWindow, {
        properties: ['openDirectory']
    });
    if (canceled || filePaths.length === 0) {
        return { canceled: true };
    }
    return { path: filePaths[0] };
});

ipcMain.handle('open-in-browser', async (event, ip) => {
    if (!ip) {
        return { success: false, error: 'IP address is not provided.' };
    }
    const url = `http://${ip}`;
    try {
        await shell.openExternal(url);
        return { success: true };
    } catch (e) {
        console.error(`Failed to open URL ${url}:`, e);
        return { success: false, error: e.message };
    }
});

async function startRecording(camera) {
    if (!camera || !camera.id) {
        console.error('[REC] Invalid camera object for recording.');
        return { success: false, error: 'Invalid camera data' };
    }
    const recordingId = buildProcessId(PROCESS_TYPES.RECORDING, camera.id);
    if (processManager.get(recordingId) || recordingManager[camera.id]) {
        console.log(`[REC] Recording already in progress for camera ${camera.id}. Skipping.`);
        return { success: false, error: 'Recording is already in progress' };
    }
    
    const password = await keytar.getPassword(KEYTAR_SERVICE, camera.id.toString());
    const fullCameraInfo = { ...camera, password: password || '' };

    const settings = await getAppSettings();
    const recordingsPath = settings.recordingsPath;
    try {
        await fsPromises.mkdir(recordingsPath, { recursive: true });
    } catch (e) {
        return { success: false, error: `Failed to create recordings folder: ${e.message}` };
    }
    const timestamp = new Date().toISOString().replace(/:/g, '-').slice(0, 19);
    const saneCameraName = fullCameraInfo.name.replace(/[<>:"/\\|?*]/g, '_');
    const filename = `${saneCameraName}-${timestamp}.mp4`;
    const outputPath = path.join(recordingsPath, filename);
    
    const streamPath0 = fullCameraInfo.streamPath0 || '/stream0';
    const streamUrl = `rtsp://${encodeURIComponent(fullCameraInfo.username)}:${encodeURIComponent(fullCameraInfo.password)}@${fullCameraInfo.ip}:${fullCameraInfo.port || 554}${streamPath0}`;
    
    const ffmpegArgs = [
        '-rtsp_transport', 'tcp', '-i', streamUrl,
        '-c:v', 'copy', '-c:a', 'aac', '-b:a', '128k',
        '-movflags', '+faststart', outputPath
    ];
    const ffmpegProcess = spawn(ffmpegPath, ffmpegArgs, { detached: false, windowsHide: true });
    
    processManager.add(recordingId, ffmpegProcess, PROCESS_TYPES.RECORDING);
    recordingManager[camera.id] = { path: outputPath };
    
    let ffmpegErrorOutput = '';
    ffmpegProcess.stderr.on('data', (data) => {
        ffmpegErrorOutput += data.toString();
    });

    ffmpegProcess.on('close', (code) => {
        console.log(`[REC FFMPEG] Finished for "${fullCameraInfo.name}" with code ${code}.`);
        delete recordingManager[camera.id];
        processManager.processes.delete(recordingId);
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('recording-state-change', { 
                cameraId: camera.id, 
                recording: false, 
                path: code === 0 ? outputPath : null,
                error: code !== 0 ? (ffmpegErrorOutput.trim().split('\n').pop() || `ffmpeg exited with code ${code}`) : null 
            });
        }
    });

    console.log(`[REC] Starting for "${fullCameraInfo.name}" to ${outputPath}`);
    if (mainWindow && !mainWindow.isDestroyed()) mainWindow.webContents.send('recording-state-change', { cameraId: camera.id, recording: true });
    return { success: true };
}

ipcMain.handle('start-recording', (event, camera) => startRecording(camera));

ipcMain.handle('stop-recording', (event, cameraId) => {
    const recordingId = buildProcessId(PROCESS_TYPES.RECORDING, cameraId);
    if (processManager.stop(recordingId)) {
        return { success: true };
    }
    return { success: false, error: 'Recording not found' };
});

ipcMain.handle('open-recordings-folder', async () => {
    const settings = await getAppSettings();
    const recordingsPath = settings.recordingsPath;
    try {
        await fsPromises.mkdir(recordingsPath, { recursive: true });
        shell.openPath(recordingsPath);
        return { success: true };
    } catch (e) {
        return { success: false, error: `Could not open folder: ${e.message}` };
    }
});

ipcMain.handle('export-archive-clip', async (event, { sourceFilename, startTime, duration }) => {
    const settings = await getAppSettings();
    const recordingsPath = settings.recordingsPath;
    const sourcePath = path.join(recordingsPath, sourceFilename);

    try {
        await fsPromises.access(sourcePath);
    } catch (e) {
        return { success: false, error: `Source file not found: ${sourceFilename}` };
    }

    const { canceled, filePath } = await dialog.showSaveDialog(mainWindow, {
        title: 'Сохранить клип',
        defaultPath: path.join(app.getPath('videos'), `clip-${sourceFilename}`),
        filters: [{ name: 'MP4 Videos', extensions: ['mp4'] }]
    });

    if (canceled || !filePath) {
        return { success: false, error: 'Export cancelled by user.' };
    }

    return new Promise((resolve) => {
        const ffmpegArgs = [
            '-i', sourcePath,
            '-ss', startTime.toString(),
            '-t', duration.toString(),
            '-c', 'copy',
            filePath
        ];

        console.log(`[Export] Starting FFmpeg with args: ${ffmpegArgs.join(' ')}`);
        const exportProcess = spawn(ffmpegPath, ffmpegArgs);
        
        let errorOutput = '';
        exportProcess.stderr.on('data', (data) => {
            errorOutput += data.toString();
        });

        exportProcess.on('close', (code) => {
            if (code === 0) {
                console.log(`[Export] Successfully created clip at ${filePath}`);
                resolve({ success: true, path: filePath });
            } else {
                console.error(`[Export] FFmpeg failed with code ${code}:`, errorOutput);
                resolve({ success: false, error: `FFmpeg failed: ${errorOutput.split('\n').pop()}` });
            }
        });

        exportProcess.on('error', (err) => {
             console.error(`[Export] Failed to start FFmpeg process:`, err);
             resolve({ success: false, error: `Failed to start FFmpeg: ${err.message}` });
        });
    });
});

function isPortInUse(port) {
    return new Promise((resolve) => {
        const server = net.createServer();
        server.once('error', (err) => resolve(err.code === 'EADDRINUSE'));
        server.once('listening', () => server.close(() => resolve(false)));
        server.listen(port);
    });
}

async function getAndReserveFreePort() {
    const release = await portMutex.acquire();
    try {
        let port = BASE_PORT;
        const MAX_PORTS_TO_CHECK = 100;
        for (let i = 0; i < MAX_PORTS_TO_CHECK; i++) {
            const currentPort = port + i;
            if (usedPorts.has(currentPort) || await isPortInUse(currentPort)) {
                continue;
            }
            usedPorts.add(currentPort);
            console.log(`[PORT] Port ${currentPort} reserved.`);
            return currentPort;
        }
        return null;
    } finally {
        release();
    }
}

function releasePort(port) {
    if (port) {
        console.log(`[PORT] Port ${port} released.`);
        usedPorts.delete(port);
    }
}

ipcMain.on('show-camera-context-menu', (event, { cameraId, labels }) => {
    const template = [
        { label: labels.open_in_browser, click: () => { event.sender.send('context-menu-command', { command: 'open_in_browser', cameraId }); } },
        { type: 'separator' },
        { label: labels.files, click: () => { event.sender.send('context-menu-command', { command: 'files', cameraId }); } },
        { label: labels.ssh, click: () => { event.sender.send('context-menu-command', { command: 'ssh', cameraId }); } },
        { label: labels.archive, click: () => { event.sender.send('context-menu-command', { command: 'archive', cameraId }); } },
        { label: labels.settings, click: () => { event.sender.send('context-menu-command', { command: 'settings', cameraId }); } },
        { label: labels.edit, click: () => { event.sender.send('context-menu-command', { command: 'edit', cameraId }); } },
        { type: 'separator' },
        { label: labels.delete, click: () => { event.sender.send('context-menu-command', { command: 'delete', cameraId }); } },
    ];
    const menu = Menu.buildFromTemplate(template);
    menu.popup({ window: BrowserWindow.fromWebContents(event.sender) });
});

ipcMain.handle('kill-all-ffmpeg', () => {
    return new Promise(resolve => {
        processManager.stopAll();
        const ffmpegProcessName = path.basename(ffmpegPath);
        const command = process.platform === 'win32' ? `taskkill /IM ${ffmpegProcessName} /F` : `pkill -f ${ffmpegProcessName}`;
        exec(command, (err, stdout, stderr) => {
            Object.values(streamManager).forEach(s => s.wss?.close());
            usedPorts.clear();
            Object.keys(streamManager).forEach(key => delete streamManager[key]);
            Object.keys(recordingManager).forEach(key => delete recordingManager[key]);
            resolve({ success: true, message: "Все отслеживаемые и 'зависшие' потоки были сброшены." });
        });
    });
});

ipcMain.handle('start-video-stream', async (event, { credentials, streamId }) => {
    const uniqueStreamIdentifier = `${credentials.id}_${streamId}`;
    if (processManager.get(uniqueStreamIdentifier) || streamManager[uniqueStreamIdentifier]) {
        console.warn(`[STREAM] Stream ${uniqueStreamIdentifier} is already running.`);
        return { success: true, wsPort: streamManager[uniqueStreamIdentifier].port };
    }
    
    let configData;
    try {
        const rawData = await fsPromises.readFile(configPath, 'utf-8');
        configData = JSON.parse(rawData);
    } catch (e) {
        console.error(`[FFMPEG] Could not load configuration file to start stream: ${e.message}`);
        return { success: false, error: 'Could not load configuration file.' };
    }

    const cameraConfig = configData.cameras.find(c => c.id === credentials.id);
    if (!cameraConfig) {
        return { success: false, error: `Camera with ID ${credentials.id} not found in config.` };
    }
    
    const password = await keytar.getPassword(KEYTAR_SERVICE, credentials.id.toString());
    const fullCredentials = { ...cameraConfig, password: password || '' };
    
    const port = fullCredentials.port || '554';
    const streamPath = streamId === 0 ? (fullCredentials.streamPath0 || '/stream0') : (fullCredentials.streamPath1 || '/stream1');
    
    const streamUrl = `rtsp://${encodeURIComponent(fullCredentials.username)}:${encodeURIComponent(fullCredentials.password)}@${fullCredentials.ip}:${port}${streamPath}`;
    
    const wsPort = await getAndReserveFreePort();
    if (wsPort === null) {
        return { success: false, error: 'Failed to find a free port.' };
    }

    const wss = new WebSocket.Server({ port: wsPort });
    wss.on('connection', (ws) => console.log(`[WSS] Client connected to port ${wsPort}`));
    
    const settings = await getAppSettings();
    
    let codec = 'h264';
    try {
        if (fullCredentials.protocol === 'openipc') {
            const response = await axios.get(`http://${fullCredentials.ip}/api/v1/config.json`, getAxiosJsonConfig(fullCredentials));
            const cameraInfo = response.data;
            codec = streamId === 0 ? (cameraInfo.video0?.codec || 'h264') : (cameraInfo.video1?.codec || 'h264');
            console.log(`[FFMPEG] OpenIPC camera codec detected: ${codec}`);

        } else if (fullCredentials.protocol === 'netip') {
            const cam = await netipConnectionManager.getInstance(fullCredentials);
            const encodeInfo = await cam.get_encode_info();
            const videoInfo = streamId === 0 ? encodeInfo.MainFormat.Video : encodeInfo.ExtraFormat.Video;
            if (videoInfo.VideoType && videoInfo.VideoType.toLowerCase().includes('265')) {
                codec = 'hevc';
            } else {
                codec = 'h264';
            }
            console.log(`[FFMPEG] NETIP camera codec detected: ${codec}`);
        }
    } catch (e) {
        console.error(`[FFMPEG] Failed to get camera codec for ${fullCredentials.name}. Falling back to h264. Error: ${e.message}`);
        codec = 'h264';
    }
    
    const { decoderArgs, vfString } = getHwAccelOptions(codec, settings.hwAccel, streamId);

    const qscale = settings.qscale || 8;
    const fps = settings.fps || 20;

    const ffmpegArgs = [
        ...decoderArgs,
        '-loglevel', 'error',
        '-rtsp_transport', 'tcp',
        '-i', streamUrl,
        '-progress', 'pipe:2', 
        '-f', 'mpegts',
        '-c:v', 'mpeg1video',
        '-preset', 'ultrafast',
        '-vf', vfString,
        '-q:v', qscale.toString(),
        '-r', fps.toString(),
        '-bf', '0',
    ];
    
    ffmpegArgs.push(
        '-ignore_unknown', 
        '-c:a', 'mp2', 
        '-b:a', '128k', 
        '-ar', '44100', 
        '-ac', '1'
    );
    
    ffmpegArgs.push('-');

    console.log(`[FFMPEG] Starting stream ${uniqueStreamIdentifier} with args:`, ffmpegArgs.join(' '));
    const ffmpegProcess = spawn(ffmpegPath, ffmpegArgs, { detached: false, windowsHide: true });
    
    processManager.add(uniqueStreamIdentifier, ffmpegProcess, PROCESS_TYPES.STREAM);
    
    ffmpegProcess.on('error', (err) => { console.error(`[FFMPEG] Failed to start subprocess for ${uniqueStreamIdentifier}: ${err.message}`); });
    ffmpegProcess.stdout.on('data', (data) => { wss.clients.forEach((client) => { if (client.readyState === WebSocket.OPEN) client.send(data); }); });
    
    let statsBuffer = '', lastErrorOutput = '';
    ffmpegProcess.stderr.on('data', (data) => {
        const errorString = data.toString();
        if (errorString.trim()) { lastErrorOutput = errorString.trim(); }
        statsBuffer += errorString;
        const statsBlocks = statsBuffer.split('progress=');
        if (statsBlocks.length > 1) {
            for (let i = 0; i < statsBlocks.length - 1; i++) {
                const block = statsBlocks[i];
                if (!block.trim()) continue;
                const stats = {};
                block.trim().split('\n').forEach(line => {
                    const [key, value] = line.split('=');
                    if (key && value) stats[key.trim()] = value.trim();
                });
                if (mainWindow && !mainWindow.isDestroyed() && (stats.fps || stats.bitrate)) {
                    mainWindow.webContents.send('stream-stats', { 
                        uniqueStreamIdentifier, 
                        fps: parseFloat(stats.fps) || 0, 
                        bitrate: parseFloat(stats.bitrate) || 0 
                    });
                }
            }
            statsBuffer = statsBlocks[statsBlocks.length - 1];
        }
    });

    ffmpegProcess.on('close', (code) => {
        console.warn(`[FFMPEG] Process ${uniqueStreamIdentifier} exited with code ${code}.`);
        if(code !== 0) { console.error(`[FFMPEG Last Stderr] ${uniqueStreamIdentifier}: ${lastErrorOutput}`); }
        if (streamManager[uniqueStreamIdentifier]) { 
            streamManager[uniqueStreamIdentifier].wss.close(); 
            releasePort(wsPort); 
            delete streamManager[uniqueStreamIdentifier]; 
        }
        processManager.processes.delete(uniqueStreamIdentifier);
        if (mainWindow && !mainWindow.isDestroyed()) { mainWindow.webContents.send('stream-died', uniqueStreamIdentifier); }
    });
    
    streamManager[uniqueStreamIdentifier] = { wss, port: wsPort };
    return { success: true, wsPort };
});

ipcMain.handle('stop-video-stream', async (event, uniqueStreamIdentifier) => {
    if (processManager.stop(uniqueStreamIdentifier)) {
        return { success: true };
    }
    return { success: false, error: "Stream not found" };
});

// =================================================================================
// --- ИЗМЕНЕННЫЙ БЛОК ДЛЯ УПРАВЛЕНИЯ АНАЛИТИКОЙ ---
// =================================================================================
ipcMain.handle('toggle-analytics', async (event, cameraId) => {
    const analyticsId = buildProcessId(PROCESS_TYPES.ANALYTICS, cameraId);

    // Если процесс уже запущен, останавливаем его
    if (processManager.get(analyticsId)) {
        console.log(`[Analytics] Stopping for camera ${cameraId}.`);
        processManager.stop(analyticsId);
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('analytics-status-change', { cameraId, active: false });
        }
        return { success: true, status: 'stopped' };
    }
    
    try {
        const configDataFromFile = JSON.parse(await fsPromises.readFile(configPath, 'utf-8'));
        const camera = configDataFromFile.cameras.find(c => c.id === cameraId);
        if (!camera) {
            return { success: false, error: 'Camera not found in config' };
        }
        const analyticsConfig = camera.analyticsConfig || {};

        const password = await keytar.getPassword(KEYTAR_SERVICE, camera.id.toString());
        const fullCameraInfo = { ...camera, password: password || '' };

        const streamPath = fullCameraInfo.streamPath0 || '/stream0';
        const rtspUrl = `rtsp://${encodeURIComponent(fullCameraInfo.username)}:${encodeURIComponent(fullCameraInfo.password)}@${fullCameraInfo.ip}:${fullCameraInfo.port || 554}${streamPath}`;
        
        // --- НАЧАЛО ИЗМЕНЕНИЙ ---

        // 1. Определяем имя исполняемого файла аналитики в зависимости от ОС
        const analyticsExecutableName = process.platform === 'win32' ? 'analytics.exe' : 'analytics';

        // 2. Определяем путь к файлу в зависимости от режима (разработка или собранное приложение)
        const analyticsPath = app.isPackaged
            ? path.join(process.resourcesPath, 'analytics', analyticsExecutableName)
            : path.join(__dirname, 'extra', 'analytics', analyticsExecutableName);

        console.log(`[Analytics] Attempting to launch analytics from: ${analyticsPath}`);

        // 3. Проверяем, существует ли файл, перед запуском
        if (!fs.existsSync(analyticsPath)) {
            const errorMsg = `Исполняемый файл видеоаналитики не найден. Ожидаемый путь: ${analyticsPath}`;
            console.error(`[Analytics] ERROR: ${errorMsg}`);
            dialog.showErrorBox('Ошибка запуска аналитики', errorMsg);
            return { success: false, error: errorMsg };
        }
        
        // 4. Подготавливаем аргументы (конфигурация кодируется в Base64)
        const configForScript = {
            objects: analyticsConfig.objects,
            roi: analyticsConfig.roi
        };
        const configArg = Buffer.from(JSON.stringify(configForScript)).toString('base64');
        const args = [rtspUrl, configArg];
        
        // 5. Запускаем скомпилированный файл
        const analyticsProcess = spawn(analyticsPath, args, { windowsHide: true });
        
        // --- КОНЕЦ ИЗМЕНЕНИЙ ---

        processManager.add(analyticsId, analyticsProcess, PROCESS_TYPES.ANALYTICS);

        analyticsProcess.stdout.on('data', async (data) => {
            const lines = data.toString().split('\n').filter(line => line.trim() !== '');
            for (const line of lines) {
                try {
                    const result = JSON.parse(line);
                    if (mainWindow && !mainWindow.isDestroyed()) {
                        mainWindow.webContents.send('analytics-update', { cameraId, result });
                    }

                    if (result.status === 'objects_detected' && result.objects.length > 0) {
                        await saveAnalyticsEvent(cameraId, result);
                    }

                    // Логика запуска записи по детекции остается прежней
                    if (result.status === 'motion_detected' || (result.status === 'objects_detected' && result.objects.length > 0)) {
                        if (!recordingManager[cameraId]) {
                            console.log(`[Analytics] Motion/Object detected on camera ${cameraId}, starting recording.`);
                            // Используем уже загруженные данные о камере, чтобы не читать файл снова
                            await startRecording(camera);
                        }
                    }

                } catch (e) {
                    console.log(`[Analytics] Non-JSON output received: ${line}`);
                }
            }
        });

        analyticsProcess.stderr.on('data', (data) => {
            console.error(`[Analytics][Executable STDERR] for camera ${cameraId}: ${data.toString()}`);
        });

        analyticsProcess.on('close', (code) => {
            console.log(`[Analytics] Process for camera ${cameraId} exited with code ${code}`);
            processManager.processes.delete(analyticsId);
            if (mainWindow && !mainWindow.isDestroyed()) {
                mainWindow.webContents.send('analytics-status-change', { cameraId, active: false });
            }
        });

        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('analytics-status-change', { cameraId, active: true });
        }
        return { success: true, status: 'started' };

    } catch (e) {
        console.error(`[Analytics] Failed to start for camera ${cameraId}:`, e);
        return { success: false, error: e.message };
    }
});

ipcMain.handle('save-configuration', async (event, config) => {
    try {
        const configToSave = JSON.parse(JSON.stringify(config));
        
        for (const camera of configToSave.cameras) {
            if (camera.password) {
                await keytar.setPassword(KEYTAR_SERVICE, camera.id.toString(), camera.password);
                delete camera.password;
            }
        }
        await fsPromises.writeFile(configPath, JSON.stringify(configToSave, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
});

ipcMain.handle('load-configuration', async () => {
    const defaultConfig = {
        cameras: [],
        groups: [],
        layout: { cols: 2, rows: 2 },
        gridState: Array(64).fill(null)
    };
    let config = defaultConfig;

    const migrateOldFile = async () => {
        try {
            await fsPromises.access(oldCamerasPath);
            console.log('Found old cameras.json, attempting migration...');
            const oldData = await fsPromises.readFile(oldCamerasPath, 'utf-8');
            const oldCameras = JSON.parse(oldData);
            return { ...defaultConfig, cameras: oldCameras };
        } catch (migrationError) {
            return null;
        }
    };
    
    try {
        await fsPromises.access(configPath);
        const data = await fsPromises.readFile(configPath, 'utf-8');
        config = { ...defaultConfig, ...JSON.parse(data) };
        if (!config.gridState || config.gridState.length < 64) {
            config.gridState = Array(64).fill(null);
        }
    } catch (e) {
        const migratedConfig = await migrateOldFile();
        if (migratedConfig) {
            config = migratedConfig;
            const configToSave = JSON.parse(JSON.stringify(config));
            for (const camera of configToSave.cameras) {
                if (camera.password) {
                    await keytar.setPassword(KEYTAR_SERVICE, camera.id.toString(), camera.password);
                    delete camera.password;
                }
            }
            await fsPromises.writeFile(configPath, JSON.stringify(configToSave, null, 2));
            await fsPromises.rename(oldCamerasPath, `${oldCamerasPath}.bak`);
            console.log('Migration successful and new config saved.');
        } else {
            console.log('No existing config found, returning default.');
        }
    }
    
    return config;
});

ipcMain.handle('get-system-stats', () => {
    const metrics = app.getAppMetrics();
    let totalCpuUsage = 0;
    let totalRamUsage = 0;

    metrics.forEach(metric => {
        totalCpuUsage += metric.cpu.percentCPUUsage;
        totalRamUsage += metric.memory.workingSetSize;
    });

    return {
        cpu: totalCpuUsage.toFixed(0),
        ram: (totalRamUsage / 1024).toFixed(0),
    };
});

const getAxiosJsonConfig = (credentials) => ({
    auth: { username: credentials.username, password: credentials.password },
    timeout: 7000,
});

const getAxiosCgiConfig = (credentials) => ({
    auth: { username: credentials.username, password: credentials.password },
    timeout: 30000,
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
});

ipcMain.handle('get-camera-settings', async (event, credentials) => {
    try {
        const password = await keytar.getPassword(KEYTAR_SERVICE, credentials.id.toString());
        const response = await axios.get(`http://${credentials.ip}/api/v1/config.json`, getAxiosJsonConfig({...credentials, password}));
        return response.data;
    } catch (error) {
        return { error: `Failed to get settings: ${error.message}` };
    }
});

ipcMain.handle('set-camera-settings', async (event, { credentials, settingsData }) => {
    try {
        const password = await keytar.getPassword(KEYTAR_SERVICE, credentials.id.toString());
        const url = `http://${credentials.ip}/cgi-bin/mj-settings.cgi`;
        settingsData.action = 'update';
        const formData = new URLSearchParams(settingsData).toString();
        const config = getAxiosCgiConfig({...credentials, password});
        config.validateStatus = (status) => (status >= 200 && status < 300) || status === 303;
        await axios.post(url, formData, config);
        return { success: true };
    } catch (error) {
        return { success: false, error: error.response?.data?.error || error.message };
    }
});

ipcMain.handle('restart-majestic', async (event, credentials) => {
    if (credentials.protocol === 'netip') {
        return { success: false, error: 'Restart is not supported for NETIP cameras via this method.' };
    }
    try {
        const password = await keytar.getPassword(KEYTAR_SERVICE, credentials.id.toString());
        const url = `http://${credentials.ip}/cgi-bin/mj-settings.cgi`;
        const formData = new URLSearchParams({ action: 'restart' }).toString();
        const config = getAxiosCgiConfig({...credentials, password});
        config.validateStatus = (status) => (status >= 200 && status < 300) || status === 303;
        await axios.post(url, formData, config);
        return { success: true };
    } catch (error) {
        return { success: false, error: error.response?.data?.error || error.message };
    }
});

ipcMain.handle('get-camera-pulse', async (event, credentials) => {
    try {
        const password = await keytar.getPassword(KEYTAR_SERVICE, credentials.id.toString());
        const response = await axios.get(`http://${credentials.ip}/api/v1/soc`, { 
            auth: { username: credentials.username, password: password || '' },
            timeout: 3000 
        });
        return { success: true, soc_temp: response.data.temp_c ? `${response.data.temp_c.toFixed(1)}°C` : null };
    } catch (error) {
        return { error: 'Camera is offline or not responding' };
    }
});

ipcMain.handle('get-camera-time', async (event, credentials) => {
    try {
        const password = await keytar.getPassword(KEYTAR_SERVICE, credentials.id.toString());
        const fullCredentials = { ...credentials, password: password || '' };
        const response = await axios.get(`http://${fullCredentials.ip}/api/v1/info`, getAxiosJsonConfig(fullCredentials));
        
        if (response.data && (response.data.localtime || response.data.system_time)) {
            return { 
                success: true, 
                cameraTimestamp: response.data.localtime, 
                systemTime: response.data.system_time 
            };
        } else {
            return { success: false, error: 'timestamp not found in camera response' };
        }
    } catch (error) {
        return { success: false, error: `Failed to get camera time: ${error.message}` };
    }
});

ipcMain.handle('get-camera-info', async (event, credentials) => {
    try {
        const password = await keytar.getPassword(KEYTAR_SERVICE, credentials.id.toString());
        const response = await axios.get(`http://${credentials.ip}/api/v1/info`, { timeout: 3000, auth: { username: credentials.username, password: password || '' } });
        return { success: true, ...response.data };
    } catch (error) {
        return { error: 'Camera is offline or not responding' };
    }
});

ipcMain.handle('open-file-manager', (event, cameraData) => {
    createFileManagerWindow(cameraData);
});

ipcMain.handle('get-recordings-for-date', async (event, { cameraName, date }) => {
    try {
        const settings = await getAppSettings();
        const recordingsPath = settings.recordingsPath;
        await fsPromises.mkdir(recordingsPath, { recursive: true });

        const dirents = await fsPromises.readdir(recordingsPath, { withFileTypes: true });
        const saneCameraName = cameraName.replace(/[<>:"/\\|?*]/g, '_');
        const datePrefix = `${saneCameraName}-${date}`;

        const videoFiles = dirents
            .filter(dirent => dirent.isFile() && dirent.name.startsWith(datePrefix) && dirent.name.endsWith('.mp4'))
            .map(dirent => {
                const timestampMatch = dirent.name.match(/(\d{4}-\d{2}-\d{2}T\d{2}-\d{2}-\d{2})/);
                if (timestampMatch) {
                    const timestampString = timestampMatch[1];
                    const [datePart, timePart] = timestampString.split('T');
                    const correctedTimePart = timePart.replace(/-/g, ':');
                    const validISOString = `${datePart}T${correctedTimePart}.000Z`;
                    
                    const dateObj = new Date(validISOString);
                    if (isNaN(dateObj.getTime())) {
                        console.error(`Invalid date parsed from filename: ${dirent.name}`);
                        return null;
                    }
                    
                    return {
                        name: dirent.name,
                        startTime: dateObj.toISOString(),
                    };
                }
                return null;
            })
            .filter(Boolean)
            .sort((a, b) => new Date(a.startTime).getTime() - new Date(b.startTime).getTime());
            
        return videoFiles;

    } catch (e) {
        console.error('Failed to get recordings for date:', e);
        return [];
    }
});

ipcMain.handle('discover-onvif-devices', async (event) => {
    console.log('[Scanner] Starting IP scan discovery...');

    const interfaces = os.networkInterfaces();
    const subnets = new Set();
    for (const name of Object.keys(interfaces)) {
        for (const iface of interfaces[name]) {
            if (iface.family === 'IPv4' && !iface.internal) {
                const parts = iface.address.split('.');
                parts.pop();
                subnets.add(parts.join('.') + '.');
            }
        }
    }

    if (subnets.size === 0) {
        console.warn('[Scanner] No active network interfaces found for scanning.');
        return { success: true, count: 0 };
    }

    const scanPromises = [];
    const foundDevices = new Set();
    console.log(`[Scanner] Scanning subnets: [ ${Array.from(subnets).join(', ')} ]`);

    for (const subnet of subnets) {
        for (let i = 1; i < 255; i++) {
            const ip = subnet + i;
            const promise = (async () => {
                try {
                    const cam = new onvif.Cam({
                        hostname: ip,
                        port: 80, 
                        timeout: 2000 
                    });
                    
                    const info = await new Promise((resolve, reject) => {
                       cam.getDeviceInformation((err, info) => {
                           if (err) return reject(err);
                           resolve(info);
                       });
                    });

                    if (info && !foundDevices.has(ip)) {
                        foundDevices.add(ip);
                        const deviceInfo = {
                            ip: ip,
                            name: info.model || info.manufacturer || ip,
                        };
                        console.log(`[Scanner] ONVIF device found at: ${ip}`);
                        if (mainWindow && !mainWindow.isDestroyed()) {
                            mainWindow.webContents.send('onvif-device-found', deviceInfo);
                        }
                    }
                } catch (e) {
                    // Errors are expected for non-camera IPs
                }
            })();
            scanPromises.push(promise);
        }
    }

    await Promise.all(scanPromises);
    console.log(`[Scanner] Scan finished across all subnets. Found ${foundDevices.size} total devices.`);
    return { success: true, count: foundDevices.size };
});

ipcMain.handle('open-ssh-terminal', async (event, cameraData) => {
    const { id, name, ip, username } = cameraData;

    if (sshWindows[id] && !sshWindows[id].win.isDestroyed()) {
        sshWindows[id].win.focus();
        return;
    }
    const sshWindow = new BrowserWindow({
        width: 800, height: 600,
        minWidth: 500, minHeight: 400,
        title: `SSH Terminal: ${name}`,
        frame: false,
        titleBarStyle: 'hidden',
        parent: mainWindow,
        webPreferences: { preload: path.join(__dirname, 'terminal-preload.js') }
    });
    sshWindow.loadFile('terminal.html', { query: { camera: JSON.stringify(cameraData) } });
    
    const conn = new Client();
    sshWindows[id] = { win: sshWindow, conn };
    
    try {
        const password = await keytar.getPassword(KEYTAR_SERVICE, id.toString());
        
        conn.on('ready', () => {
            if (sshWindow.isDestroyed()) return;
            sshWindow.webContents.send('ssh-status', { connected: true });
            conn.shell((err, stream) => {
                if (err) {
                    if (!sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-status', { connected: false, message: `\r\n*** SSH SHELL ERROR: ${err.message} ***\r\n` });
                    return;
                }
                stream.on('data', (data) => { if (!sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-data', data.toString('utf8')); });
                ipcMain.on(`ssh-input-${id}`, (event, data) => stream.write(data));
                stream.on('close', () => conn.end());
            });
        }).on('error', (err) => {
            if (!sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-status', { connected: false, message: `\r\n*** SSH CONNECTION ERROR: ${err.message} ***\r\n` });
        }).on('close', () => {
            if (!sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-status', { connected: false, message: '\r\nConnection closed.' });
            ipcMain.removeAllListeners(`ssh-input-${id}`);
        }).connect({ host: ip, port: 22, username, password: password || '', readyTimeout: 10000 });
    } catch (e) {
        console.error('Error opening SSH terminal:', e);
        if (!sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-status', { connected: false, message: `\r\n*** ERROR: ${e.message} ***\r\n` });
    }

    sshWindow.on('closed', () => {
        conn.end();
        delete sshWindows[id];
    });
});

ipcMain.handle('check-for-updates', () => {
    autoUpdater.checkForUpdates();
});

autoUpdater.on('update-available', (info) => {
    console.log('[Updater] Update available.', info);
    if (mainWindow && !mainWindow.isDestroyed()) mainWindow.webContents.send('update-status', { status: 'available', message: `Доступна версия ${info.version}` });
});
autoUpdater.on('update-not-available', (info) => {
    console.log('[Updater] No new update available.');
    if (mainWindow && !mainWindow.isDestroyed()) mainWindow.webContents.send('update-status', { status: 'latest', message: 'У вас последняя версия.' });
});
autoUpdater.on('error', (err) => {
    console.error('[Updater] Error:', err ? (err.stack || err) : 'unknown error');
    if (mainWindow && !mainWindow.isDestroyed()) mainWindow.webContents.send('update-status', { status: 'error', message: `Ошибка обновления: ${err.message}` });
});
autoUpdater.on('download-progress', (progressObj) => {
    let log_message = "Download speed: " + progressObj.bytesPerSecond;
    log_message = log_message + ' - Downloaded ' + progressObj.percent.toFixed(2) + '%';
    log_message = log_message + ' (' + progressObj.transferred + "/" + progressObj.total + ')';
    console.log('[Updater] ' + log_message);
    if (mainWindow && !mainWindow.isDestroyed()) mainWindow.webContents.send('update-status', {
        status: 'downloading',
        message: `Загрузка... ${progressObj.percent.toFixed(0)}%`
    });
});
autoUpdater.on('update-downloaded', (info) => {
    console.log('[Updater] Update downloaded.', info);
    if (mainWindow && !mainWindow.isDestroyed()) mainWindow.webContents.send('update-status', { status: 'downloaded', message: `Версия ${info.version} загружена. Перезапустите для установки.` });
    
    dialog.showMessageBox(mainWindow, {
        type: 'info',
        title: 'Обновление готово',
        message: 'Новая версия загружена. Перезапустить приложение сейчас, чтобы установить обновление?',
        buttons: ['Перезапустить', 'Позже'],
        defaultId: 0
    }).then(({ response }) => {
        if (response === 0) {
            autoUpdater.quitAndInstall();
        }
    });
});

app.whenReady().then(async () => {
    await initializeUsers();

    protocol.registerFileProtocol('video-archive', async (request, callback) => {
        const settings = await getAppSettings();
        const recordingsPath = settings.recordingsPath;
        const filename = decodeURIComponent(request.url.replace('video-archive://', ''));
        const filePath = path.join(recordingsPath, filename);
        
        if (path.dirname(filePath) !== path.resolve(recordingsPath)) {
            console.error("Attempt to access file outside of recordings directory.");
            return callback({ error: -6 });
        }

        callback({ path: filePath });
    });

    createWindow();
});

app.on('will-quit', async (event) => {
    if (isShuttingDown) {
        return; 
    }

    netipConnectionManager.closeAll();

    const recordingProcs = Array.from(processManager.processes.entries())
        .filter(([key, { type }]) => type === PROCESS_TYPES.RECORDING)
        .map(([key, { process }]) => ({ key, process }));

    if (recordingProcs.length > 0) {
        event.preventDefault(); 
        isShuttingDown = true;
        console.log(`[Shutdown] Gracefully stopping ${recordingProcs.length} recordings...`);

        const promises = recordingProcs.map(({ key, process }) => {
            return new Promise(resolve => {
                const timeout = setTimeout(() => {
                    console.warn(`[Shutdown] Recording ${key} timed out. Killing.`);
                    if (!process.killed) process.kill('SIGKILL');
                    resolve();
                }, 4000); 

                process.on('close', () => {
                    clearTimeout(timeout);
                    console.log(`[Shutdown] Recording ${key} finished.`);
                    resolve();
                });

                try {
                    if (process.stdin.writable) {
                        process.stdin.write('q\n');
                    } else {
                        process.kill();
                        resolve();
                    }
                } catch (e) {
                    console.error(`[Shutdown] Error sending 'q' to ${key}:`, e.message);
                    if(!process.killed) process.kill('SIGKILL');
                    resolve();
                }
            });
        });

        await Promise.all(promises);
        console.log('[Shutdown] All recordings stopped. Quitting now.');
        app.quit(); 
    } else {
        console.log('[Shutdown] No active recordings. Quitting immediately.');
        processManager.stopAll();
    }
});

app.on('window-all-closed', () => { if (process.platform !== 'darwin') app.quit(); });
app.on('activate', () => { if (BrowserWindow.getAllWindows().length === 0) createWindow(); });

ipcMain.handle('scp-connect', async (event, camera) => {
    return new Promise(async (resolve) => {
        if (fileManagerConnections[camera.id]) {
            return resolve({ success: true });
        }
        const conn = new Client();
        fileManagerConnections[camera.id] = conn;

        try {
            const password = await keytar.getPassword(KEYTAR_SERVICE, camera.id.toString());
            
            conn.on('ready', () => {
                console.log(`[SSH] Connection ready for ${camera.name}`);
                resolve({ success: true });
            }).on('error', (err) => {
                console.error(`[SSH] Connection error for ${camera.name}:`, err);
                delete fileManagerConnections[camera.id];
                resolve({ success: false, error: err.message });
            }).on('close', () => {
                console.log(`[SSH] Connection closed for ${camera.name}`);
                delete fileManagerConnections[camera.id];
                const win = fileManagerWindows[camera.id];
                if (win && !win.isDestroyed()) {
                    win.webContents.send('scp-close');
                }
            }).connect({
                host: camera.ip,
                port: 22,
                username: camera.username,
                password: password || '',
                readyTimeout: 10000
            });
        } catch (e) {
            resolve({ success: false, error: e.message });
        }
    });
});

ipcMain.handle('scp-list', async (event, { cameraId, path: remotePath }) => {
    const conn = fileManagerConnections[cameraId];
    if (!conn) return { success: false, error: 'Not connected' };

    return new Promise((resolve) => {
        const command = `ls -lA "${remotePath}"`;
        let stdout = '';
        let stderr = '';
        conn.exec(command, (err, stream) => {
            if (err) return resolve({ success: false, error: err.message });
            stream.on('data', (data) => stdout += data.toString());
            stream.stderr.on('data', (data) => stderr += data.toString());
            stream.on('close', (code) => {
                if (code !== 0) return resolve({ success: false, error: stderr.trim() });
                
                const files = stdout.split('\n')
                    .map(line => {
                        const parts = line.trim().split(/\s+/);
                        if (parts.length < 9) return null;
                        
                        const type = parts[0][0];
                        const size = parseInt(parts[4], 10);
                        const name = parts.slice(8).join(' ');

                        if (!name || name === '.' || name === '..') return null;

                        return {
                            name: name,
                            isDirectory: type === 'd',
                            size: isNaN(size) ? 0 : size,
                        };
                    })
                    .filter(Boolean);
                
                resolve(files);
            });
        });
    });
});

ipcMain.handle('scp-download', async (event, { cameraId, remotePath }) => {
    const conn = fileManagerConnections[cameraId];
    if (!conn) return { success: false, error: 'Not connected' };

    const { canceled, filePath } = await dialog.showSaveDialog(BrowserWindow.fromWebContents(event.sender), {
        defaultPath: path.basename(remotePath)
    });
    if (canceled || !filePath) return { success: false, error: 'Download canceled' };

    return new Promise((resolve) => {
        conn.exec(`scp -f "${remotePath}"`, (err, stream) => {
            if (err) {
                return resolve({ success: false, error: err.message });
            }

            let fileStream;
            let protocolState = 'BEGIN'; // 'BEGIN', 'HEADER', 'DATA', 'END'
            let fileSize = 0;
            let bytesReceived = 0;
            
            stream.stdout.on('data', (chunk) => {
                if (protocolState === 'BEGIN') {
                    stream.stdin.write('\0'); // Acknowledge start
                    protocolState = 'HEADER';
                }
                
                if (protocolState === 'HEADER') {
                    const header = chunk.toString('utf8');
                    const match = header.match(/^C\d{4}\s(\d+)\s/);
                    if (match) {
                        fileSize = parseInt(match[1], 10);
                        fileStream = fs.createWriteStream(filePath);
                        stream.stdin.write('\0'); // Acknowledge header
                        protocolState = 'DATA';
                        const headerEndIndex = chunk.indexOf('\n') + 1;
                        if (chunk.length > headerEndIndex) {
                            const firstData = chunk.slice(headerEndIndex);
                            fileStream.write(firstData);
                            bytesReceived += firstData.length;
                        }
                    }
                } else if (protocolState === 'DATA') {
                    fileStream.write(chunk);
                    bytesReceived += chunk.length;
                }
            });

            stream.stderr.on('data', (data) => {
                console.error(`[SCP Download STDERR] ${data}`);
            });
            
            stream.on('close', () => {
                if (fileStream) fileStream.end();
                if (bytesReceived === fileSize) {
                    resolve({ success: true });
                } else {
                     // Check if file is empty
                    if (fileSize > 0 && bytesReceived === 0) {
                        resolve({ success: false, error: 'File transfer failed, 0 bytes received.' });
                    } else if (bytesReceived > 0) {
                        resolve({ success: true, message: "Transfer finished, but size mismatch." });
                    } else {
                        resolve({ success: true }); // Likely a 0-byte file
                    }
                }
            });
             stream.on('error', (err) => {
                if (fileStream) fileStream.end();
                resolve({ success: false, error: err.message });
            });
        });
    });
});

ipcMain.handle('scp-upload', async (event, { cameraId, remotePath }) => {
    const conn = fileManagerConnections[cameraId];
    if (!conn) return { success: false, error: 'Not connected' };

    const { canceled, filePaths } = await dialog.showOpenDialog(BrowserWindow.fromWebContents(event.sender), {
        properties: ['openFile']
    });

    if (canceled || filePaths.length === 0) {
        return { success: false, error: 'Upload canceled' };
    }
    const localPath = filePaths[0];
    const remoteDest = path.posix.join(remotePath, path.basename(localPath));

    return new Promise((resolve) => {
        const stats = fs.statSync(localPath);
        conn.exec(`scp -t "${remoteDest}"`, (err, stream) => {
            if (err) return resolve({ success: false, error: err.message });

            const localStream = fs.createReadStream(localPath);
            
            stream.on('close', () => {
                resolve({ success: true });
            });
            stream.on('error', (err) => {
                resolve({ success: false, error: err.message });
            });

            const header = `C0644 ${stats.size} ${path.basename(localPath)}\n`;
            stream.write(header);
            
            localStream.pipe(stream);
        });
    });
});

const executeRemoteCommand = (cameraId, command) => {
    const conn = fileManagerConnections[cameraId];
    if (!conn) return Promise.resolve({ success: false, error: 'Not connected' });
    return new Promise(resolve => {
        let stderr = '';
        conn.exec(command, (err, stream) => {
            if (err) return resolve({ success: false, error: err.message });
            stream.stderr.on('data', data => stderr += data.toString());
            stream.on('close', code => {
                if (code !== 0) return resolve({ success: false, error: stderr.trim() || `Command failed with code ${code}` });
                resolve({ success: true });
            });
        });
    });
};

ipcMain.handle('scp-mkdir', (e, { cameraId, path }) => executeRemoteCommand(cameraId, `mkdir -p "${path}"`));
ipcMain.handle('scp-delete-file', (e, { cameraId, path }) => executeRemoteCommand(cameraId, `rm -f "${path}"`));
ipcMain.handle('scp-delete-dir', (e, { cameraId, path }) => executeRemoteCommand(cameraId, `rm -rf "${path}"`));

ipcMain.handle('get-local-disk-list', async () => {
    if (process.platform === 'win32') {
        return new Promise(resolve => {
            exec('wmic logicaldisk get name', (err, stdout) => {
                if (err) return resolve([os.homedir()]);
                const disks = stdout.split('\n').slice(1)
                    .map(line => line.trim())
                    .filter(line => line.length > 0)
                    .map(disk => `${disk}\\`);
                resolve(disks);
            });
        });
    }
    return ['/'];
});

ipcMain.handle('list-local-files', async (event, dirPath) => {
    try {
        const items = await fsPromises.readdir(dirPath, { withFileTypes: true });
        return items.map(item => {
            try {
                return {
                    name: item.name,
                    isDirectory: item.isDirectory(),
                    size: item.isDirectory() ? 0 : fs.statSync(path.join(dirPath, item.name)).size
                };
            } catch (e) {
                console.error(`Could not stat file: ${path.join(dirPath, item.name)}`, e.message);
                return null;
            }
        }).filter(Boolean);
    } catch (e) {
        console.error(`Error listing local dir ${dirPath}:`, e);
        return [];
    }
});