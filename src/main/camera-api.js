// --- ФАЙЛ: src/main/camera-api.js ---

const axios = require('axios');
const { DigestAuth } = require('@mhoc/axios-digest-auth');
const onvif = require('node-onvif');
const { Client } = require('ssh2');
const NetIpCamera = require('../../netip-handler.js');
const fs = require('fs');
const path = require('path');
const { dialog, ipcMain } = require('electron');
const authManager = require('./auth-manager');
const { getCameraConfig } = require('./config-manager');

const PTZ_COMMANDS_OPENIPC = {
    pan_left: 'left', pan_right: 'right', tilt_up: 'up', tilt_down: 'down',
    zoom_in: 'zoomin', zoom_out: 'zoomout', stop: 'stop', go_home: 'home',
    tilt_up_left: 'upleft', tilt_up_right: 'upright',
    tilt_down_left: 'downleft', tilt_down_right: 'downright'
};

const PTZ_COMMANDS_NETIP = {
    stop: 0, tilt_up: 1, tilt_down: 2, pan_left: 3, pan_right: 4,
    tilt_up_left: 5, tilt_up_right: 6, tilt_down_left: 7, tilt_down_right: 8,
    go_home: 9, zoom_in: 11, zoom_out: 12
};

// Кэш для хранения правильного пути PTZ API для каждой камеры
const ptzApiEndpoints = {};

// --- Менеджеры соединений ---

const onvifConnectionManager = {
    connections: new Map(),
    async getInstance(camera) { /* ... (Код из main.js) ... */ },
    closeAll() {
        console.log('[ONVIF Manager] Clearing all ONVIF connections...');
        this.connections.clear();
    }
};

const netipConnectionManager = {
    connections: new Map(),
    async getInstance(camera) { /* ... (Код из main.js) ... */ },
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


// --- Вспомогательные функции для Axios ---

const getAxiosJsonConfig = (credentials) => ({
    auth: { username: credentials.username, password: credentials.password || '' },
    timeout: 7000,
});

const getAxiosCgiConfig = (credentials) => ({
    auth: { username: credentials.username, password: credentials.password || '' },
    timeout: 30000,
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
});


// --- PTZ Управление ---

async function sendOpenIpcPtzCommand(camera, command, action) {
    let ptzAction;
    if (action === 'stop') ptzAction = 'stop';
    else if (action === 'absolute' && command === 'go_home') ptzAction = 'home';
    else ptzAction = PTZ_COMMANDS_OPENIPC[command];

    if (!ptzAction) return { success: false, error: 'Unknown OpenIPC PTZ command' };

    try {
        const password = await authManager.getPasswordForCamera(camera.id);
        const fullCameraInfo = { ...camera, password };
        
        let triedNewApi = false;
        let endpoint = ptzApiEndpoints[camera.id];

        if (!endpoint) {
            endpoint = '/api/v1/ptz';
            triedNewApi = true;
        }

        const url = `http://${fullCameraInfo.ip}${endpoint}?move=${ptzAction}`;
        console.log(`[PTZ OpenIPC] Trying command to ${url}`);

        try {
            await axios.get(url, getAxiosJsonConfig(fullCameraInfo));
            ptzApiEndpoints[camera.id] = endpoint;
            return { success: true };
        } catch (e) {
            if (triedNewApi && e.response && e.response.status === 404) {
                const legacyEndpoint = '/cgi-bin/ptz.cgi';
                const legacyUrl = `http://${fullCameraInfo.ip}${legacyEndpoint}?move=${ptzAction}`;
                await axios.get(legacyUrl, getAxiosJsonConfig(fullCameraInfo));
                ptzApiEndpoints[camera.id] = legacyEndpoint;
                return { success: true };
            }
            throw e;
        }
    } catch (e) {
        const errorMessage = e.response ? `status ${e.response.status}` : e.message;
        console.error(`[PTZ OpenIPC] Error for ${camera.name}:`, errorMessage);
        return { success: false, error: errorMessage };
    }
}

async function sendNetipPtzCommand(camera, command, action) {
    const ptzActionCode = (action === 'stop') ? PTZ_COMMANDS_NETIP.stop : PTZ_COMMANDS_NETIP[command];
    if (ptzActionCode === undefined) return { success: false, error: 'Unknown NETIP PTZ command' };

    try {
        const cam = await netipConnectionManager.getInstance(camera);
        await cam.ptz_control(ptzActionCode, 4);
        return { success: true };
    } catch (e) {
        console.error(`[PTZ NETIP] Error for ${camera.name}:`, e.message);
        return { success: false, error: e.message };
    }
}

async function sendFinalPtzCgiCommand(camera, command, action) {
    const password = await authManager.getPasswordForCamera(camera.id);
    const fullCameraInfo = { ...camera, password };

    const COMMAND_MAP = {
        tilt_up: 0, tilt_down: 1, pan_left: 2, pan_right: 3,
        zoom_in: 13, zoom_out: 12, go_home: 27
    };
    
    let cmd, data1 = 0, data2 = 0;
    const speed = 4;

    if (action === 'stop') cmd = 14;
    else if (action === 'start') {
        cmd = COMMAND_MAP[command];
        if ([0, 1].includes(cmd)) data2 = speed;
        else if ([2, 3].includes(cmd)) data1 = speed;
    } else if (action === 'absolute' && command === 'go_home') {
        cmd = COMMAND_MAP[command];
    } else {
        return { success: false, error: 'Unsupported action.' };
    }

    if (cmd === undefined) return { success: false, error: `Unsupported command "${command}"` };

    const xmlData = `<?xml version="1.0" encoding="utf-8"?><root><ptz_ctral cmd=${cmd} data1=${data1} data2=${data2} /></root>`;
    const url = `http://${camera.ip}/cgi/ptz.cgi?act=ptz_ctrl&data=${encodeURIComponent(xmlData)}`;
    
    try {
        if (camera.onvifAuth !== false) {
            const digestAuth = new DigestAuth({ username: fullCameraInfo.username, password: fullCameraInfo.password });
            await digestAuth.get(url);
        } else {
            await axios.get(url, { timeout: 5000 });
        }
        return { success: true };
    } catch(e) {
        const errorMessage = e.response ? `status ${e.response.status}` : e.message;
        console.error(`[PTZ CGI] Command failed for ${camera.name}:`, errorMessage);
        return { success: false, error: `CGI command failed: ${errorMessage}` };
    }
}

async function ptzControl({ cameraId, command, action }) {
    try {
        const camera = await getCameraConfig(cameraId);
        if (!camera) return { success: false, error: 'Camera not found' };

        switch (camera.protocol) {
            case 'openipc': return sendOpenIpcPtzCommand(camera, command, action);
            case 'netip': return sendNetipPtzCommand(camera, command, action);
            case 'onvif':
            default: return sendFinalPtzCgiCommand(camera, command, action);
        }
    } catch (e) {
        console.error('[PTZ] Global error:', e);
        return { success: false, error: 'Internal PTZ error' };
    }
}


// --- API запросы к камерам ---

async function getCameraPulse(credentials) {
    try {
        const password = await authManager.getPasswordForCamera(credentials.id);
        const response = await axios.get(`http://${credentials.ip}/api/v1/soc`, { 
            auth: { username: credentials.username, password: password || '' },
            timeout: 3000 
        });
        return { success: true, soc_temp: response.data.temp_c ? `${response.data.temp_c.toFixed(1)}°C` : null };
    } catch (error) {
        return { error: 'Camera is offline or not responding' };
    }
}

async function getCameraTime(credentials) {
    try {
        const password = await authManager.getPasswordForCamera(credentials.id);
        const fullCredentials = { ...credentials, password };
        const response = await axios.get(`http://${fullCredentials.ip}/api/v1/info`, getAxiosJsonConfig(fullCredentials));
        
        if (response.data && (response.data.localtime || response.data.system_time)) {
            return { success: true, cameraTimestamp: response.data.localtime, systemTime: response.data.system_time };
        } else {
            return { success: false, error: 'timestamp not found in camera response' };
        }
    } catch (error) {
        return { success: false, error: `Failed to get camera time: ${error.message}` };
    }
}

async function getCameraSettings(credentials) {
    try {
        const password = await authManager.getPasswordForCamera(credentials.id);
        const response = await axios.get(`http://${credentials.ip}/api/v1/config.json`, getAxiosJsonConfig({...credentials, password}));
        return response.data;
    } catch (error) {
        return { error: `Failed to get settings: ${error.message}` };
    }
}

async function setCameraSettings({ credentials, settingsData }) {
    try {
        const password = await authManager.getPasswordForCamera(credentials.id);
        const url = `http://${credentials.ip}/cgi-bin/mj-settings.cgi`;
        const postData = Object.entries(settingsData)
            .map(([key, value]) => `${encodeURIComponent(key)}=${encodeURIComponent(value)}`)
            .join('&');
            
        const config = getAxiosCgiConfig({...credentials, password});
        config.validateStatus = (status) => (status >= 200 && status < 300) || status === 303;

        await axios.post(url, `action=update&${postData}`, config);
        return { success: true };
    } catch (error) {
        return { success: false, error: error.response?.data?.error || error.message };
    }
}

async function restartMajestic(credentials) {
    if (credentials.protocol === 'netip') {
        return { success: false, error: 'Restart is not supported for NETIP cameras.' };
    }
    try {
        const password = await authManager.getPasswordForCamera(credentials.id);
        const url = `http://${credentials.ip}/cgi-bin/mj-settings.cgi`;
        const formData = new URLSearchParams({ action: 'restart' }).toString();
        const config = getAxiosCgiConfig({...credentials, password});
        config.validateStatus = (status) => (status >= 200 && status < 300) || status === 303;
        await axios.post(url, formData, config);
        return { success: true };
    } catch (error) {
        return { success: false, error: error.response?.data?.error || error.message };
    }
}


// --- NETIP API ---

async function getNetipSettings(camera) {
    try {
        const cam = await netipConnectionManager.getInstance(camera);
        const [systemInfo, generalInfo, encodeInfo] = await Promise.all([
            cam.get_system_info(),
            cam.get_general_info(),
            cam.get_encode_info()
        ]);
        return { ...systemInfo, ...generalInfo, ...encodeInfo };
    } catch (e) {
        return { error: e.message || 'Unknown NETIP error' };
    }
}

async function setNetipSettings({ camera, settingsData }) {
    try {
        const cam = await netipConnectionManager.getInstance(camera);
        console.warn('setNetipSettings is not yet implemented.');
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message || 'Failed to set settings' };
    }
}


// --- SSH & SCP ---

function setupSshConnection(sshWindow, cameraData, sshConnections) {
    const { id, ip, username } = cameraData;
    const conn = new Client();
    sshConnections[id] = conn;

    authManager.getPasswordForCamera(id).then(password => {
        conn.on('ready', () => {
            if (sshWindow.isDestroyed()) return;
            sshWindow.webContents.send('ssh-status', { connected: true });
            conn.shell((err, stream) => {
                if (err) {
                    if (!sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-status', { connected: false, message: `\r\n*** SSH SHELL ERROR: ${err.message} ***\r\n` });
                    return;
                }
                stream.on('data', data => {
                    if (!sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-data', data.toString('utf8'));
                });
                ipcMain.on(`ssh-input-${id}`, (event, data) => stream.write(data));
                stream.on('close', () => conn.end());
            });
        }).on('error', err => {
            if (!sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-status', { connected: false, message: `\r\n*** SSH CONNECTION ERROR: ${err.message} ***\r\n` });
        }).on('close', () => {
            if (sshWindow && !sshWindow.isDestroyed()) {
                sshWindow.webContents.send('ssh-status', { connected: false, message: '\r\nConnection closed.' });
            }
            ipcMain.removeAllListeners(`ssh-input-${id}`);
        }).connect({ host: ip, port: 22, username, password: password || '', readyTimeout: 10000 });
    }).catch(e => {
        if (sshWindow && !sshWindow.isDestroyed()) sshWindow.webContents.send('ssh-status', { connected: false, message: `\r\n*** ERROR: ${e.message} ***\r\n` });
    });
}

const scp = {
    connect: (camera, fileManagerConnections) => new Promise(async (resolve, reject) => {
        if (fileManagerConnections[camera.id] && fileManagerConnections[camera.id]._readableState) {
            return resolve();
        }
        const conn = new Client();
        fileManagerConnections[camera.id] = conn;
        const password = await authManager.getPasswordForCamera(camera.id);
        conn.on('ready', resolve)
            .on('error', reject)
            .on('close', () => delete fileManagerConnections[camera.id])
            .connect({ 
                host: camera.ip, 
                port: 22, 
                username: camera.username, 
                password: password || '',
                readyTimeout: 10000
            });
    }),

    list: ({ cameraId, path: remotePath }, fileManagerConnections) => new Promise((resolve, reject) => {
        const conn = fileManagerConnections[cameraId];
        if (!conn) return reject(new Error('Not connected'));

        const command = `ls -lA --full-time "${remotePath}"`;
        let output = '';
        let errorOutput = '';

        conn.exec(command, (err, stream) => {
            if (err) return reject(err);
            stream.on('data', (data) => { output += data.toString(); });
            stream.stderr.on('data', (data) => { errorOutput += data.toString(); });
            stream.on('close', (code) => {
                if (code !== 0) return reject(new Error(errorOutput || `Command failed with code ${code}`));
                const lines = output.trim().split('\n');
                const files = lines.slice(1).map(line => {
                    const parts = line.split(/\s+/);
                    if (parts.length < 9) return null;
                    const perms = parts[0];
                    const size = parseInt(parts[4], 10);
                    const name = parts.slice(8).join(' ');
                    return {
                        name: name.replace(/\/$/, ''),
                        isDirectory: perms.startsWith('d'),
                        size: isNaN(size) ? 0 : size
                    };
                }).filter(Boolean);
                resolve(files);
            });
        });
    }),

    download: async (event, { cameraId, remotePath }, fileManagerConnections) => {
        const conn = fileManagerConnections[cameraId];
        if (!conn) throw new Error('Not connected');

        const { canceled, filePath } = await dialog.showSaveDialog({ defaultPath: path.basename(remotePath) });
        if (canceled || !filePath) return { success: false, error: 'User cancelled' };

        return new Promise((resolve, reject) => {
            const command = `scp -f "${remotePath}"`;
            conn.exec(command, (err, stream) => {
                if (err) return reject(err);

                const fileStream = fs.createWriteStream(filePath);
                let scpParser = { header: null, fileSize: 0, received: 0 };

                stream.stdout.on('data', (data) => {
                    if (!scpParser.header) {
                        const headerStr = data.toString('utf8');
                        const match = headerStr.match(/^C\d{4}\s(\d+)\s(.*)\n/);
                        if (match) {
                            scpParser.header = { mode: match[0], size: parseInt(match[1]), name: match[2] };
                            scpParser.fileSize = scpParser.header.size;
                            stream.stdin.write('\x00');
                            const fileData = data.slice(headerStr.indexOf('\n') + 1);
                            if (fileData.length > 0) {
                                fileStream.write(fileData);
                                scpParser.received += fileData.length;
                            }
                        }
                    } else {
                        fileStream.write(data);
                        scpParser.received += data.length;
                    }

                    if (scpParser.received >= scpParser.fileSize) {
                        stream.stdin.write('\x00');
                    }
                });

                stream.on('close', () => {
                    fileStream.end();
                    if (scpParser.received >= scpParser.fileSize) {
                        resolve({ success: true, path: filePath });
                    } else {
                        reject(new Error('File transfer incomplete.'));
                    }
                });

                stream.stderr.on('data', (data) => reject(new Error(data.toString())));
                stream.stdin.write('\x00');
            });
        });
    },

    upload: async (event, { cameraId, remotePath }, fileManagerConnections) => {
        const conn = fileManagerConnections[cameraId];
        if (!conn) throw new Error('Not connected');

        const { canceled, filePaths } = await dialog.showOpenDialog({ properties: ['openFile'] });
        if (canceled || !filePaths.length) return { success: false, error: 'User cancelled' };

        const localPath = filePaths[0];
        const remoteFinalPath = path.posix.join(remotePath, path.basename(localPath));
        const stats = fs.statSync(localPath);

        return new Promise((resolve, reject) => {
            const command = `scp -t "${remoteFinalPath}"`;
            conn.exec(command, (err, stream) => {
                if (err) return reject(err);

                const fileStream = fs.createReadStream(localPath);
                let stdoutBuffer = '';

                stream.stdout.on('data', (data) => {
                    stdoutBuffer += data.toString();
                    if (stdoutBuffer.includes('\x00')) {
                        fileStream.pipe(stream.stdin);
                    }
                });

                fileStream.on('end', () => {
                    stream.stdin.write('\x00');
                });

                stream.on('close', () => resolve({ success: true }));
                stream.stderr.on('data', (data) => reject(new Error(data.toString())));

                stream.stdin.write(`C0644 ${stats.size} ${path.basename(localPath)}\n`);
            });
        });
    },

    executeRemoteCommand: ({ cameraId, command }, connections) => new Promise((resolve, reject) => {
        const conn = connections[cameraId];
        if (!conn) return reject(new Error('Not connected'));
        conn.exec(command, (err, stream) => {
            if (err) return reject(err);
            let stderr = '';
            stream.on('close', (code) => {
                if (code !== 0) return reject(new Error(stderr || `Command failed with code ${code}`));
                resolve({ success: true });
            }).on('data', () => {}).stderr.on('data', (data) => {
                stderr += data.toString();
            });
        });
    }),
};

scp.mkdir = (data, c) => scp.executeRemoteCommand({ ...data, command: `mkdir -p "${data.path}"` }, c);
scp.deleteFile = (data, c) => scp.executeRemoteCommand({ ...data, command: `rm -f "${data.path}"` }, c);
scp.deleteDir = (data, c) => scp.executeRemoteCommand({ ...data, command: `rm -rf "${data.path}"` }, c);

module.exports = {
    onvifConnectionManager,
    netipConnectionManager,
    ptzControl,
    getCameraPulse,
    getCameraTime,
    getCameraSettings,
    setCameraSettings,
    restartMajestic,
    getNetipSettings,
    setNetipSettings,
    setupSshConnection,
    scp,
};