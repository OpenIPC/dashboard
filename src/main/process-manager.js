/**
 * Генерирует массив превью (thumbnails) для видеофайла архива с помощью ffmpeg.
 * @param {Object} params - { sourceFilename: string, interval: number (сек), count: number }
 * @returns {Promise<{success: boolean, thumbnails?: Array<{time: number, url: string}>, error?: string}>}
 */
async function getArchiveThumbnails({ sourceFilename, interval = 600, count = 10 }) {
    const settings = await configManager.getAppSettings();
    const sourcePath = path.join(settings.recordingsPath, sourceFilename);
    try {
        // Получаем длительность видео через ffprobe
        const ffprobe = require('fluent-ffmpeg');
        const durationRaw = await new Promise((resolve, reject) => {
            ffprobe.ffprobe(sourcePath, (err, metadata) => {
                if (err) return reject(err);
                const videoStream = metadata.streams.find(s => s.codec_type === 'video');
                resolve(videoStream ? videoStream.duration : metadata.format.duration);
            });
        });
        const duration = Number(durationRaw) || 0;
        if (!duration || Number.isNaN(duration) || duration <= 0) return { success: false, error: 'Не удалось определить длительность видео.' };

        // Вычисляем таймкоды для превью: равномерно распределим `count` образцов по длительности,
        // чтобы получить предсказуемое количество миниатюр для коротких файлов.
        const times = [];
        for (let i = 0; i < Math.max(1, count); i++) {
            const t = Math.floor((i * duration) / Math.max(1, count));
            times.push(Math.max(0, Math.min(Math.floor(t), Math.floor(duration) - 1)));
        }
        // dedupe and ensure at least a 0 timestamp
        const uniq = Array.from(new Set(times));
        if (!uniq.includes(0)) uniq.unshift(0);
        const finalTimes = uniq.slice(0, count);

        // Папка для временных превью
        const tmpDir = path.join(app.getPath('temp'), 'openipc-thumbnails');
        await fsPromises.mkdir(tmpDir, { recursive: true });

        // Генерируем превью через ffmpeg (один кадр на каждый таймкод)
        const ffmpegPath = require('@ffmpeg-installer/ffmpeg').path.replace('app.asar', 'app.asar.unpacked');
        const thumbFiles = [];
        for (const t of finalTimes) {
            const safeName = path.basename(sourceFilename).replace(/[^a-zA-Z0-9-_\.]/g, '_');
            const thumbPath = path.join(tmpDir, `${safeName}_${t}.jpg`);
            thumbFiles.push({ time: t, path: thumbPath });
        }
        // Запускаем ffmpeg для каждого превью (можно оптимизировать батчем)
        await Promise.all(thumbFiles.map(({ time, path: thumbPath }) => {
            return new Promise((resolve, reject) => {
                const args = ['-ss', String(time), '-i', sourcePath, '-frames:v', '1', '-q:v', '2', '-vf', 'scale=120:68', '-y', thumbPath];
                const proc = require('child_process').spawn(ffmpegPath, args);
                let stderr = '';
                proc.stderr && proc.stderr.on('data', (chunk) => { stderr += chunk.toString(); });
                proc.on('close', code => code === 0 ? resolve() : reject(new Error(`ffmpeg error (code=${code}): ${stderr.slice(0, 200)}`)));
                proc.on('error', (err) => reject(err));
            });
        }));

        // Читаем превью как base64
        const thumbnails = await Promise.all(thumbFiles.map(async ({ time, path: thumbPath }) => {
            const data = await fsPromises.readFile(thumbPath);
            return { time, url: `data:image/jpeg;base64,${data.toString('base64')}` };
        }));
        return { success: true, thumbnails };
    } catch (err) {
        return { success: false, error: err.message };
    }
}
// --- ФАЙЛ: src/main/process-manager.js ---

const { spawn, exec } = require('child_process');
const path = require('path');
const fs = require('fs');
const fsPromises = require('fs').promises;
const { app, dialog } = require('electron');
const yaml = require('js-yaml');
const axios = require('axios');
const dgram = require('dgram');
const net = require('net');

const configManager = require('./config-manager');
const authManager = require('./auth-manager');
const services = require('./services');
const FfmpegCommandBuilder = require('./ffmpeg-builder');

let wss = null;
function setWebSocketServer(server) {
    console.log('[ProcessManager] WebSocket server instance received.');
    wss = server;
}

function broadcastToRenderers(channel, payload) {
    const mainWindow = require('./window-manager').getMainWindow();
    if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send(channel, payload);
    }
    if (wss) {
        const message = JSON.stringify({ type: 'event', channel, payload });
        wss.clients.forEach(client => {
            if (client.readyState === require('ws').OPEN) {
                client.send(message);
            }
        });
    }
}

// Probe for a free UDP port range base (returns first available base port)
// Use a high ephemeral port range by default to avoid collisions with common services.
async function findFreeUdpPort(start = 20000, end = 40000) {
    console.log(`[MediaMTX] Probing UDP base ports in range ${start}-${end} (step=8)`);
    for (let p = start; p <= end; p += 8) { // ports are used in groups of 8 in our config
        const base = p;
        const portsToCheck = [base, base + 1, base + 2, base + 3, base + 4, base + 5, base + 6, base + 7];
        // If this group intersects any previously-blocked UDP ports, skip it
        if (portsToCheck.some(pr => mediamtxBlockedUdpPorts.has(pr))) {
            console.log(`[MediaMTX] Skipping UDP base ${base} because it intersects blocked ports`);
            continue;
        }
        let ok = true;
        for (const port of portsToCheck) {
            // eslint-disable-next-line no-await-in-loop
            const available = await new Promise(resolve => {
                const s = dgram.createSocket('udp4');
                const onError = (err) => { try { s.close(); } catch (e) {} ; resolve(false); };
                const onListening = () => { try { s.close(); } catch (e) {} ; resolve(true); };
                s.once('error', onError);
                s.once('listening', onListening);
                try {
                    s.bind(port, '0.0.0.0');
                } catch (e) {
                    // synchronous bind error
                    try { s.close(); } catch (ee) {}
                    resolve(false);
                }
            });
            if (!available) { ok = false; console.log(`[MediaMTX] Base ${base} rejected because port ${port} is not available`); break; }
        }
        if (ok) {
            console.log(`[MediaMTX] Selected UDP base port: ${base}`);
            return base;
        }
    }
    // fallback: pick a random high port base to reduce chance of collision
    const randomBase = Math.floor(Math.random() * (end - start - 8)) + start;
    const fallback = randomBase - (randomBase % 8);
    console.warn(`[MediaMTX] Could not find a free port group in ${start}-${end}, falling back to ${fallback}`);
    return fallback;
}

const PROCESS_TYPES = { RECORDING: 'recording', ANALYTICS: 'analytics', HLS: 'hls' };
const processes = new Map();
const recordingManager = {};
const recordingStopTimers = {};
let mediamtxProcess = null;
let mediamtxRestartAttempts = 0;
let mediamtxLastStdout = '';
let mediamtxLastStderr = '';
let mediamtxSelectedPorts = {};
// Ports that we saw mediamtx fail to bind (help avoid repeated collisions)
const mediamtxBlockedUdpPorts = new Set();

async function generateAndSaveMediaMTXConfig(options = { writeRuntime: true, baseRtpPort: 8000 }) {
    const { cameras } = await configManager.loadConfiguration();
    const settings = await configManager.getAppSettings();
    const paths = {};
    const runtimePaths = {};
    // Store required camera paths globally for RTSP ready tracking
    try {
        global.lastMediamtxRequiredPaths = [];
    } catch (e) { global.lastMediamtxRequiredPaths = []; }

    for (const camera of cameras) {
        // Track required camera paths for RTSP ready event
        try { global.lastMediamtxRequiredPaths.push(`cam${camera.id}_0`); global.lastMediamtxRequiredPaths.push(`cam${camera.id}_1`); } catch (e) {}
        const password = await authManager.getPasswordForCamera(camera.id);
        const fullCredentials = { ...camera, password };
        const builder = new FfmpegCommandBuilder({});
        // Поток всегда активен, задержка минимальна
        const alwaysOnConfig = {
            sourceOnDemand: false,
            sourceOnDemandCloseAfter: '5s', // на всякий случай, если где-то останется onDemand
            rtspTransport: 'tcp',
        };

        const src0 = builder.buildRtspUrl(fullCredentials, fullCredentials.streamPath0 || '/stream=0');
        const src1 = builder.buildRtspUrl(fullCredentials, fullCredentials.streamPath1 || '/stream=1');

        // runtimePaths include plaintext credentials (used only at startup)
        runtimePaths[`cam${camera.id}_0`] = { source: src0, ...alwaysOnConfig };
        runtimePaths[`cam${camera.id}_1`] = { source: src1, ...alwaysOnConfig };

        // For persistent config we mask passwords so they are not stored in clear
        const mask = (url) => {
            try {
                const start = url.indexOf('://');
                const at = url.indexOf('@');
                if (start >= 0 && at > start) {
                    const before = url.substring(0, start + 3);
                    const after = url.substring(at);
                    const creds = url.substring(start + 3, at);
                    const user = creds.split(':')[0] || 'user';
                    return `${before}${user}:****${after}`;
                }
            } catch (e) { /* ignore */ }
            return url;
        };

        paths[`cam${camera.id}_0`] = { source: mask(src0), ...alwaysOnConfig };
        paths[`cam${camera.id}_1`] = { source: mask(src1), ...alwaysOnConfig };

        // Log generated sources with masked password for easier debugging
        try {
            const mask = (url) => {
                const start = url.indexOf('://');
                const at = url.indexOf('@');
                if (start >= 0 && at > start) {
                    const before = url.substring(0, start + 3);
                    const after = url.substring(at);
                    const creds = url.substring(start + 3, at); // username:password@host...
                    const user = creds.split(':')[0] || 'user';
                    return `${before}${user}:****${after}`;
                }
                return url;
            };
            console.log(`[MediaMTX] Generated source for cam${camera.id}_0: ${mask(src0)}`);
            console.log(`[MediaMTX] Generated source for cam${camera.id}_1: ${mask(src1)}`);
        } catch (e) {
            console.log('[MediaMTX] Could not mask source URLs for logging.');
        }
    }

    const recordPath = path.join(settings.recordingsPath, '%path', '%Y-%m-%d_%H-%M-%S-%f').replace(/\\/g, '/');
    // Diagnostic: log resolved recordings path so we can verify it's what we expect on the host
    try {
        console.log(`[MediaMTX] Configured recordPath: ${recordPath}`);
    } catch (e) { /* ignore logging errors */ }
    const webhookUrl = 'http://127.0.0.1:8080/mediamtx-webhook';

    const base = options.baseRtpPort || 8000;
    const config = {
        rtmp: false,
        hls: true,
        webrtc: true,
        api: true,
    apiAddress: `:${options.apiPort || 9997}`,
        webrtcAddress: `127.0.0.1:${options.webrtcPort || 8889}`,
        hlsAddress: `:${options.hlsPort || 8888}`,
        // Allow overriding RTSP TCP listener port (default 8554)
        rtspAddress: `:${options.rtspPort || 8554}`,
    // RTP/RTCP and multicast ports will be derived from base
    rtpAddress: `:${base}`,
    rtcpAddress: `:${base + 1}`,
        
    externalAuthenticationURL: webhookUrl,

        pathDefaults: {
            record: false,
            recordPath: recordPath,
            recordFormat: 'fmp4',
            recordSegmentDuration: '1h'
        },

        paths: paths,
        // Multicast / SRTP ports
        multicastRTPPort: base + 2,
        multicastRTCPPort: base + 3,
        srtpAddress: `:${base + 4}`,
        srtcpAddress: `:${base + 5}`,
        multicastSRTPPort: base + 6,
        multicastSRTCPPort: base + 7,
    };

    // Persistent (masked) config
    const persistentConfig = Object.assign({}, config, { paths: paths });
    const persistentPath = path.join(app.getPath('userData'), 'mediamtx.yml');

    // Runtime (plaintext) config path - written only briefly before starting MediaMTX
    const runtimeConfig = Object.assign({}, config, { paths: runtimePaths });
    const runtimePath = path.join(app.getPath('userData'), 'mediamtx_runtime.yml');

    try {
        // Write persistent masked config
        await fs.promises.writeFile(persistentPath, yaml.dump(persistentConfig, { noRefs: true, lineWidth: -1 }));
        // Write runtime config with real credentials only when requested
        if (options.writeRuntime) {
            await fs.promises.writeFile(runtimePath, yaml.dump(runtimeConfig, { noRefs: true, lineWidth: -1 }));
            // Try to restrict permissions (best-effort, mainly for POSIX)
            try { fs.chmodSync(runtimePath, 0o600); } catch (e) { /* ignore on windows */ }
        }
        console.log(`[MediaMTX] Config generated and saved to ${persistentPath} (masked)`);
        return options.writeRuntime ? runtimePath : persistentPath;
    } catch (e) {
        console.error('[MediaMTX] Failed to write config file:', e);
        return null;
    }
}

// Probe for a free TCP port (returns first available port in range)
async function findFreeTcpPort(start = 8000, end = 9000, host = '0.0.0.0') {
    for (let p = start; p <= end; p++) {
        // eslint-disable-next-line no-await-in-loop
        const available = await new Promise(resolve => {
            const server = net.createServer();
            server.once('error', () => { try { server.close(); } catch (e) {} ; resolve(false); });
            server.once('listening', () => { try { server.close(); } catch (e) {} ; resolve(true); });
            try {
                server.listen(p, host);
            } catch (e) {
                try { server.close(); } catch (ee) {}
                resolve(false);
            }
        });
        if (available) return p;
    }
    return null;
}

// Probe for a free TCP port but avoid any ports listed in the exclude set.
async function findFreeTcpPortAvoiding(exclude = new Set(), start = 8000, end = 9000, host = '0.0.0.0') {
    // First try sequential scan within the requested range
    for (let p = start; p <= end; p++) {
        if (exclude.has(p)) continue;
        // eslint-disable-next-line no-await-in-loop
        const available = await new Promise(resolve => {
            const server = net.createServer();
            server.once('error', () => { try { server.close(); } catch (e) {} ; resolve(false); });
            server.once('listening', () => { try { server.close(); } catch (e) {} ; resolve(true); });
            try {
                server.listen(p, host);
            } catch (e) {
                try { server.close(); } catch (ee) {}
                resolve(false);
            }
        });
        if (available) return p;
    }

    // If none found in range, try a number of random high ports to reduce collision chances
    const maxAttempts = 80;
    for (let i = 0; i < maxAttempts; i++) {
        const p = Math.floor(Math.random() * (60000 - 12000)) + 12000;
        if (exclude.has(p)) continue;
        // eslint-disable-next-line no-await-in-loop
        const available = await new Promise(resolve => {
            const server = net.createServer();
            server.once('error', () => { try { server.close(); } catch (e) {} ; resolve(false); });
            server.once('listening', () => { try { server.close(); } catch (e) {} ; resolve(true); });
            try {
                server.listen(p, host);
            } catch (e) {
                try { server.close(); } catch (ee) {}
                resolve(false);
            }
        });
        if (available) return p;
    }

    return null;
}

/**
 * Returns true if mediamtxProcess is currently running.
 */
function isMediaMTXRunning() {
    return !!mediamtxProcess;
}

/**
 * Hot-update MediaMTX paths by calling its REST API (PATCH /v3/config/paths/patch/<pathName>)
 * This avoids restarting the MediaMTX process on every config change.
 */
async function updateMediaMTXPaths() {
    // Глобальная защита от параллельных вызовов
    if (global.__updateMediaMTXPathsRunning) {
        console.warn('[ProcessManager] updateMediaMTXPaths already running, skipping duplicate call.');
        return false;
    }
    global.__updateMediaMTXPathsRunning = true;
    try {
        const { cameras } = await configManager.loadConfiguration();
        const runtimePaths = {};
        for (const camera of cameras) {
            const password = await authManager.getPasswordForCamera(camera.id);
            const fullCredentials = { ...camera, password };
            const builder = new FfmpegCommandBuilder({});
            const onDemandConfig = {
                sourceOnDemand: true,
                sourceOnDemandStartTimeout: '3s', // ускоряем ожидание старта источника
                sourceOnDemandCloseAfter: '60s',
                rtspTransport: 'tcp',
            };
            const src0 = builder.buildRtspUrl(fullCredentials, fullCredentials.streamPath0 || '/stream=0');
            const src1 = builder.buildRtspUrl(fullCredentials, fullCredentials.streamPath1 || '/stream=1');
            runtimePaths[`cam${camera.id}_0`] = { source: src0, ...onDemandConfig };
            runtimePaths[`cam${camera.id}_1`] = { source: src1, ...onDemandConfig };
        }

        // Notify renderers we're starting the hot-update
        const CHANNELS = require('../common/ipc-channels');
        broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'start', message: 'Updating MediaMTX paths via API...' });

        // Try PATCH/PUT/POST for the first path only to check API support
        const pathEntries = Object.entries(runtimePaths);
        let apiSupported = true;
        if (pathEntries.length > 0) {
            const [firstPathName, firstCfg] = pathEntries[0];
            try {
                const res = await axios.patch(`http://127.0.0.1:9997/v3/config/paths/patch/${firstPathName}`, firstCfg, { timeout: 3000 });
                if (!(res && res.status >= 200 && res.status < 300)) apiSupported = false;
            } catch (e) {
                const is404 = e && e.response && e.response.status === 404;
                if (is404) {
                    try {
                        const createRes = await axios.put(`http://127.0.0.1:9997/v3/config/paths/${firstPathName}`, firstCfg, { timeout: 3000 });
                        if (!(createRes && createRes.status >= 200 && createRes.status < 300)) apiSupported = false;
                    } catch (createErr) {
                        apiSupported = false;
                    }
                } else {
                    apiSupported = false;
                }
            }
        }

        if (!apiSupported) {
            // Защита от повторных рестартов: если уже идёт рестарт — не запускать второй раз
            if (global.__mediamtxRestarting) {
                console.warn('[MediaMTX API] Restart already in progress, skipping duplicate restart.');
                return false;
            }
            global.__mediamtxRestarting = true;
            console.warn('[MediaMTX API] API not supported, writing config and restarting MediaMTX.');
            broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'fallback', message: 'API not supported, applying runtime config and restarting MediaMTX...' });
            try {
                const runtimePath = await generateAndSaveMediaMTXConfig({ writeRuntime: true });
                if (runtimePath) {
                    try { stopMediaMTX(); } catch (e) { /* ignore */ }
                    await new Promise(r => setTimeout(r, 700));
                    await startMediaMTX();
                    // После рестарта MediaMTX сразу уведомляем фронтенд о готовности
                    broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'done', message: 'MediaMTX paths available after restart.' });
                    global.__mediamtxRestarting = false;
                    // Параллельно делаем короткую проверку путей (best-effort)
                    const start2 = Date.now();
                    const timeout2 = 1000; // 1 секунда
                    const requiredPaths2 = Object.keys(runtimePaths);
                    while (Date.now() - start2 < timeout2) {
                        try {
                            const res2 = await axios.get('http://127.0.0.1:9997/v3/config/paths');
                            const present2 = Object.keys(res2.data || {}).filter(p => requiredPaths2.includes(p));
                            if (present2.length === requiredPaths2.length) {
                                console.log('[MediaMTX API] All paths confirmed after restart.');
                                return true;
                            }
                        } catch (e) { /* ignore */ }
                        await new Promise(r => setTimeout(r, 200));
                    }
                    // Если не все пути подтвердились — просто логируем, но не блокируем UI
                    console.warn('[MediaMTX API] Fallback restart did not produce all paths in time (non-blocking).');
                    return true;
                }
            } catch (e) {
                console.error('[MediaMTX API] Fallback restart failed:', e && e.message ? e.message : e);
                broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'failed', message: `Fallback failed: ${e && e.message ? e.message : e}` });
            }
            global.__mediamtxRestarting = false;
            return false;
        }

        // Если API поддерживается — продолжаем обновлять все пути через API
        const failedPaths = [];
        for (const [pathName, cfg] of pathEntries) {
            try {
                const res = await axios.patch(`http://127.0.0.1:9997/v3/config/paths/patch/${pathName}`, cfg, { timeout: 3000 });
                if (res && res.status >= 200 && res.status < 300) {
                    console.log(`[MediaMTX API] Patched path: ${pathName}`);
                    continue;
                }
                console.warn(`[MediaMTX API] Unexpected response patching ${pathName}: ${res && res.status}`);
                failedPaths.push(pathName);
            } catch (e) {
                const is404 = e && e.response && e.response.status === 404;
                if (is404) {
                    try {
                        const createRes = await axios.put(`http://127.0.0.1:9997/v3/config/paths/${pathName}`, cfg, { timeout: 3000 });
                        if (createRes && createRes.status >= 200 && createRes.status < 300) {
                            console.log(`[MediaMTX API] Created path via PUT: ${pathName}`);
                            continue;
                        }
                        console.warn(`[MediaMTX API] Unexpected response creating ${pathName}: ${createRes && createRes.status}`);
                        failedPaths.push(pathName);
                    } catch (createErr) {
                        console.warn(`[MediaMTX API] Failed to create ${pathName} via PUT after 404: ${createErr.message}`);
                        failedPaths.push(pathName);
                    }
                } else {
                    console.warn(`[MediaMTX API] Failed to patch ${pathName}: ${e.message}`);
                    failedPaths.push(pathName);
                }
            }
        }

        if (failedPaths.length === 0) {
            console.log('[MediaMTX API] All paths patched successfully.');
            broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'done', message: 'MediaMTX paths updated.' });
            return true;
        }

        // Otherwise, fallback as раньше
        console.warn('[MediaMTX API] Not all paths could be patched, falling back to config write and restart.');
        broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'fallback', message: 'Not all paths patched, applying runtime config and restarting MediaMTX...' });
        try {
            const runtimePath = await generateAndSaveMediaMTXConfig({ writeRuntime: true });
            if (runtimePath) {
                try { stopMediaMTX(); } catch (e) { /* ignore */ }
                await new Promise(r => setTimeout(r, 700));
                await startMediaMTX();
                // Poll GET /v3/config/paths for a short period
                const start2 = Date.now();
                const timeout2 = 5000;
                const requiredPaths2 = Object.keys(runtimePaths);
                while (Date.now() - start2 < timeout2) {
                    try {
                        const res2 = await axios.get('http://127.0.0.1:9997/v3/config/paths');
                        const present2 = Object.keys(res2.data || {}).filter(p => requiredPaths2.includes(p));
                        if (present2.length === requiredPaths2.length) {
                            console.log('[MediaMTX API] All paths confirmed after restart.');
                            broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'done', message: 'MediaMTX paths available after restart.' });
                            return true;
                        }
                    } catch (e) { /* ignore */ }
                    await new Promise(r => setTimeout(r, 400));
                }
                console.warn('[MediaMTX API] Fallback restart did not produce all paths in time.');
                broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'failed', message: 'Fallback restart did not produce all MediaMTX paths in time.' });
            }
        } catch (e) {
            console.error('[MediaMTX API] Fallback restart failed:', e && e.message ? e.message : e);
            broadcastToRenderers(CHANNELS.ON_MEDIAMTX_UPDATE, { stage: 'failed', message: `Fallback failed: ${e && e.message ? e.message : e}` });
        }
        return false;
    } catch (e) {
        console.error('[ProcessManager] updateMediaMTXPaths failed:', e);
        return false;
    } finally {
        global.__updateMediaMTXPathsRunning = false;
    }
}

async function startMediaMTX(startOptions = {}) {
    if (mediamtxProcess) {
        console.log('[MediaMTX] Process is already running.');
        return;
    }
    // Allow callers (restart handler) to pass explicit ports to avoid race where
    // a regenerated runtime config overwrites a previously chosen alternative port.
    const basePort = startOptions.baseRtpPort || await findFreeUdpPort(20000, 40000);
    console.log(`[MediaMTX] Using UDP base port: ${basePort}`);

    // Also probe common TCP listeners that mediamtx will bind (RTSP/HLS/WebRTC/API)
    const rtspPort = startOptions.rtspPort || await findFreeTcpPort(8554, 8600) || 8554;
    const hlsPort = startOptions.hlsPort || await findFreeTcpPort(8888, 9000) || 8888;
    const webrtcHttpPort = startOptions.webrtcPort || await findFreeTcpPort(8889, 8900, '127.0.0.1') || 8889;
    const apiPort = startOptions.apiPort || await findFreeTcpPort(9997, 10010) || 9997;

    console.log(`[MediaMTX] Selected TCP ports - RTSP: ${rtspPort}, HLS: ${hlsPort}, WebRTC HTTP: ${webrtcHttpPort}, API: ${apiPort}`);
    mediamtxSelectedPorts = { rtspPort, hlsPort, webrtcPort: webrtcHttpPort, apiPort, baseRtpPort: basePort };

    const configPath = await generateAndSaveMediaMTXConfig({ writeRuntime: true, baseRtpPort: basePort, apiPort, webrtcPort: webrtcHttpPort, hlsPort, rtspPort: rtspPort });
    if (!configPath) {
        return;
    }

    const mediamtxPath = app.isPackaged
        ? path.join(process.resourcesPath, 'mediamtx', process.platform === 'win32' ? 'mediamtx.exe' : 'mediamtx')
        : path.join(__dirname, '../../mediamtx', process.platform === 'win32' ? 'mediamtx.exe' : 'mediamtx');

    if (!fs.existsSync(mediamtxPath)) {
        console.error(`[MediaMTX] Executable not found at ${mediamtxPath}`);
        dialog.showErrorBox('Ошибка MediaMTX', `Исполняемый файл mediamtx не найден по пути: ${mediamtxPath}`);
        return;
    }

    console.log(`[MediaMTX] Starting process from: ${mediamtxPath}`);
    mediamtxProcess = spawn(mediamtxPath, [configPath]);

    // Remove the runtime config file once MediaMTX signals it is ready (safer than fixed delay).
    try {
        const runtimeConfigPath = configPath;
        let removed = false;

        const tryRemove = () => {
            if (removed) return;
            try {
                fs.unlinkSync(runtimeConfigPath);
                removed = true;
                console.log('[MediaMTX] Runtime config removed from disk.');
            } catch (e) {
                // ignore failure
            }
        };

        // Watch stdout for a readiness indicator. MediaMTX prints the API listener line when ready.
        // Strategy: prefer deterministic confirmation via MediaMTX API.
        // 1) Wait for API listener opened line on stdout (indicates API available)
        // 2) Poll GET /v3/config/paths to verify our cam paths are present
        // 3) Only after confirmation remove the runtime config file
        const requiredPaths = Object.keys(runtimePaths);

        let apiAvailable = false;
        const onStdoutReadyHandler = (data) => {
            try {
                const text = data.toString();
                if (text.includes('[API] listener opened')) {
                    apiAvailable = true;
                }
            } catch (e) { /* ignore parsing errors */ }
        };

        mediamtxProcess.stdout.on('data', onStdoutReadyHandler);

        const checkPathsViaApi = async () => {
            try {
                const res = await axios.get('http://127.0.0.1:9997/v3/config/paths');
                if (res && res.data) {
                    const present = Object.keys(res.data || {}).filter(p => requiredPaths.includes(p));
                    return present.length === requiredPaths.length;
                }
            } catch (e) {
                // API might not be ready yet
            }
            return false;
        };

        // Poll API for up to 20s once apiAvailable is true, then fallback to stdout-based grace removal.
        (async () => {
            const start = Date.now();
            const maxWait = 20000;
            while (Date.now() - start < maxWait) {
                if (apiAvailable) {
                    const ok = await checkPathsViaApi();
                    if (ok) {
                        tryRemove();
                        try { mediamtxProcess.stdout.removeListener('data', onStdoutReadyHandler); } catch (e) { }
                        return;
                    }
                }
                await new Promise(r => setTimeout(r, 500));
            }

            // Fallback: if API didn't confirm, wait additional short grace period and remove.
            setTimeout(() => {
                tryRemove();
                try { mediamtxProcess.stdout.removeListener('data', onStdoutReadyHandler); } catch (e) { }
            }, 3000);
        })();

        // Fallback: if readiness not detected within 30s, attempt to remove anyway (best-effort)
        setTimeout(() => {
            tryRemove();
            try { mediamtxProcess.stdout.removeListener('data', onStdoutReadyHandler); } catch (e) { }
        }, 30000);
    } catch (e) { /* ignore */ }

    // --- RTSP READY TRACKING ---
    let lastAnnouncedRtspReadyPaths = [];
    mediamtxProcess.stdout.on('data', (data) => {
        const text = data.toString();
        mediamtxLastStdout += text;
        // keep buffer bounded
        if (mediamtxLastStdout.length > 16 * 1024) mediamtxLastStdout = mediamtxLastStdout.slice(-16 * 1024);
        console.log(`[MediaMTX stdout]: ${text.trim()}`);
        // ТЕСТОВАЯ ОТПРАВКА СТАТИСТИКИ В РЕНДЕРЕР
        try {
            const statsData = { test: true, message: 'Тестовая статистика MediaMTX', raw: data.toString().trim() };
            broadcastToRenderers('mediamtx-stats-update', statsData);
        } catch (e) { /* ignore */ }

        // --- RTSP READY LOGIC ---
        // Detect all '[path ...] [RTSP source] ready' lines for all required camera paths
        try {
            // Only run this logic if we know which paths to expect (from last config)
            if (global.lastMediamtxRequiredPaths && Array.isArray(global.lastMediamtxRequiredPaths) && global.lastMediamtxRequiredPaths.length > 0) {
                // Find all '[path ...] [RTSP source] ready' lines in the current stdout buffer
                const readyLines = mediamtxLastStdout.match(/\[path ([^\]]+)\] \[RTSP source\] ready/gi) || [];
                // Extract path names
                const readyPaths = readyLines.map(line => {
                    const m = line.match(/\[path ([^\]]+)\]/i);
                    return m ? m[1] : null;
                }).filter(Boolean);
                // Deduplicate
                const uniqueReadyPaths = Array.from(new Set(readyPaths));
                // If all required paths are present and this is a new state, emit event
                if (
                    global.lastMediamtxRequiredPaths.every(p => uniqueReadyPaths.includes(p)) &&
                    (lastAnnouncedRtspReadyPaths.join(',') !== uniqueReadyPaths.join(','))
                ) {
                    lastAnnouncedRtspReadyPaths = uniqueReadyPaths;
                    broadcastToRenderers('mediamtx-rtsp-ready', { paths: uniqueReadyPaths });
                }
            }
        } catch (e) { /* ignore */ }
    });

    mediamtxProcess.stderr.on('data', (data) => {
        const text = data.toString();
        mediamtxLastStderr += text;
        if (mediamtxLastStderr.length > 16 * 1024) mediamtxLastStderr = mediamtxLastStderr.slice(-16 * 1024);
        console.error(`[MediaMTX stderr]: ${text.trim()}`);
    });

    mediamtxProcess.on('close', (code) => {
        console.warn(`[MediaMTX] Process exited with code ${code}`);
        const combined = `${mediamtxLastStdout}\n${mediamtxLastStderr}`;
        mediamtxProcess = null;

        // Detect bind errors and attempt a limited number of automatic retries with different ports
        // Match both forms: "listen tcp :8554: bind" and "listen tcp 127.0.0.1:8889: bind"
        const bindMatch = combined.match(/listen\s+(tcp|udp)\s+[^\d]*(\d+): bind/i);
        if (bindMatch && mediamtxRestartAttempts < 3) {
            mediamtxRestartAttempts += 1;
            const proto = bindMatch[1];
            const badPort = parseInt(bindMatch[2], 10);
            // If this was a UDP bind failure, record the specific port so we can avoid
            // any UDP base groups that include it on subsequent probes.
            if (proto === 'udp' && badPort) {
                try {
                    mediamtxBlockedUdpPorts.add(badPort);
                    console.warn(`[MediaMTX] Marked UDP port ${badPort} as blocked to avoid future conflicts.`);
                } catch (e) { /* ignore */ }
            }
            console.warn(`[MediaMTX] Detected bind failure on ${proto.toUpperCase()} port ${badPort}. Attempting restart #${mediamtxRestartAttempts} with alternative ports.`);
            (async () => {
                try {
                    // Prepare a fresh full set of ports to avoid overlaps or partial conflicts
                    const newUdpBase = await findFreeUdpPort(20000, 40000);
                    // Avoid reusing the last selected ports and the detected badPort
                    const exclude = new Set();
                    try { if (mediamtxSelectedPorts.rtspPort) exclude.add(mediamtxSelectedPorts.rtspPort); } catch (e) {}
                    try { if (mediamtxSelectedPorts.hlsPort) exclude.add(mediamtxSelectedPorts.hlsPort); } catch (e) {}
                    try { if (mediamtxSelectedPorts.webrtcPort) exclude.add(mediamtxSelectedPorts.webrtcPort); } catch (e) {}
                    try { if (mediamtxSelectedPorts.apiPort) exclude.add(mediamtxSelectedPorts.apiPort); } catch (e) {}
                    if (badPort) exclude.add(badPort);

                    const newRtsp = await findFreeTcpPortAvoiding(exclude, 8554, 8900) || (mediamtxSelectedPorts.rtspPort ? mediamtxSelectedPorts.rtspPort + 1 : 8555);
                    exclude.add(newRtsp);
                    const newHls = await findFreeTcpPortAvoiding(exclude, 8888, 8999) || (mediamtxSelectedPorts.hlsPort ? mediamtxSelectedPorts.hlsPort + 1 : 8889);
                    exclude.add(newHls);
                    const newWeb = await findFreeTcpPortAvoiding(exclude, 8889, 8999, '127.0.0.1') || (mediamtxSelectedPorts.webrtcPort ? mediamtxSelectedPorts.webrtcPort + 1 : 8890);
                    exclude.add(newWeb);
                    const newApi = await findFreeTcpPortAvoiding(exclude, 9997, 10050) || (mediamtxSelectedPorts.apiPort ? mediamtxSelectedPorts.apiPort + 1 : 9998);
                    exclude.add(newApi);
                    const startOpts = { baseRtpPort: newUdpBase, rtspPort: newRtsp, hlsPort: newHls, webrtcPort: newWeb, apiPort: newApi };
                    console.log(`[MediaMTX] Restarting with fresh ports RTSP:${newRtsp}, HLS:${newHls}, WebRTC:${newWeb}, API:${newApi}, UDP base:${newUdpBase}`);
                    // Use fresh ports for restart to avoid partial conflicts; note which listener failed
                    console.log(`[MediaMTX] Bind failure detected on ${proto.toUpperCase()} port ${badPort}. Proceeding with fresh port set.`);
                    // Regenerate runtime config and log masked contents for post-mortem
                    const runtimePath = await generateAndSaveMediaMTXConfig({ writeRuntime: true, baseRtpPort: startOpts.baseRtpPort, rtspPort: startOpts.rtspPort, hlsPort: startOpts.hlsPort, webrtcPort: startOpts.webrtcPort, apiPort: startOpts.apiPort });
                    if (runtimePath) {
                        try {
                            const raw = await fsPromises.readFile(runtimePath, 'utf8');
                            // Mask any passwords present
                            const masked = raw.replace(/:(?:[^\n@]+)@/g, ':****@');
                            console.log('[MediaMTX] Runtime config about to be used (masked):\n' + masked.split('\n').slice(0,50).join('\n'));
                        } catch (e) { /* ignore */ }
                    }
                    // small delay to avoid tight restart loop
                    await new Promise(r => setTimeout(r, 500));
                    // Start with explicit ports so startMediaMTX does not overwrite our choice
                    await startMediaMTX(startOpts);
                } catch (e) {
                    console.error('[MediaMTX] Automatic restart after bind failure failed:', e && e.message ? e.message : e);
                }
            })();
        } else {
            mediamtxRestartAttempts = 0;
            mediamtxLastStdout = '';
            mediamtxLastStderr = '';
        }
    });
}

function stopMediaMTX() {
    if (mediamtxProcess) {
        console.log('[MediaMTX] Stopping process...');
        mediamtxProcess.kill('SIGTERM');
        mediamtxProcess = null;
    }
}


function addProcess(key, process, type) {
    console.log(`[ProcessManager] Adding ${type} process with key: ${key}`);
    processes.set(key, { process, type });
}

function stopProcess(key) {
    if (processes.has(key)) {
        const { process: childProcess, type } = processes.get(key);
        childProcess.isManuallyStopping = true;
        console.log(`[ProcessManager] Issuing stop for ${type} process with key: ${key}`);
        try {
            if ((type === PROCESS_TYPES.RECORDING || type === PROCESS_TYPES.HLS) && childProcess.stdin && childProcess.stdin.writable) {
                childProcess.stdin.write('q\n');
            } else if (!childProcess.killed) {
                if (process.platform === 'win32') {
                    exec(`taskkill /pid ${childProcess.pid} /f /t`);
                } else {
                    childProcess.kill('SIGKILL');
                }
            }
        } catch (e) {
            console.error(`[ProcessManager] Error sending stop signal to ${key}: ${e.message}`);
        }
        processes.delete(key);
        return true;
    }
    return false;
}

function getAllProcessesOfType(type) {
    return Array.from(processes.entries())
        .filter(([key, value]) => value.type === type)
        .map(([key, value]) => ({ key, process: value.process }));
}

function stopAllProcesses() {
    console.log(`[ProcessManager] Stopping all ${processes.size} tracked processes (excluding MediaMTX).`);
    for (const [key, { process: childProcess }] of processes) {
        try {
            if (!childProcess.killed) {
                childProcess.isManuallyStopping = true;
                childProcess.kill('SIGKILL');
            }
        } catch (e) {
            console.error(`[ProcessManager] Error killing process ${key}: ${e.message}`);
        }
    }
    processes.clear();
}

async function startRecording(camera, type) {
    const cameraId = camera.id;
    if (recordingManager[cameraId]) {
        console.log(`[REC] Recording for camera ${cameraId} is already active.`);
        return;
    }

    const pathName = `cam${cameraId}_0`;
    try {
        const res = await axios.patch(`http://127.0.0.1:9997/v3/config/paths/patch/${pathName}`, { record: true });
        // Log response for diagnostics
        console.log(`[REC] PATCH /v3/config/paths/patch/${pathName} -> status=${res.status}`);
        // Try to fetch path config from MediaMTX to inspect recordPath
        try {
            const cfg = await axios.get(`http://127.0.0.1:9997/v3/config/paths/${pathName}`);
            console.log(`[REC] MediaMTX path config for ${pathName}:`, cfg && cfg.data ? cfg.data : cfg);
        } catch (e) {
            console.warn(`[REC] Could not fetch MediaMTX path config for ${pathName}: ${e && e.message ? e.message : e}`);
        }
        recordingManager[cameraId] = { type };
        console.log(`[REC] Started ${type} recording for ${pathName}.`);
        if (type === 'manual') {
            services.showSystemNotification({ title: 'Запись начата', body: `Камера: "${camera.name}"` });
        }
        broadcastToRenderers('recording-state-change', { cameraId, recording: true });
    } catch (e) {
        console.error(`[REC] Failed to start recording for ${pathName}:`, e.message);
    }
}

async function stopRecording(cameraId) {
    if (!recordingManager[cameraId]) return;

    const camera = await configManager.getCameraConfig(cameraId);
    const pathName = `cam${cameraId}_0`;
    try {
        const res = await axios.patch(`http://127.0.0.1:9997/v3/config/paths/patch/${pathName}`, { record: false });
        console.log(`[REC] PATCH /v3/config/paths/patch/${pathName} -> status=${res.status}`);
        const { type } = recordingManager[cameraId];
        delete recordingManager[cameraId];
        console.log(`[REC] Stopped ${type} recording for ${pathName}.`);
        // After stopping, attempt to list files in the configured camera recordings folder for quick diagnostics
        try {
            const settings = await configManager.getAppSettings();
            const cameraDir = path.join(settings.recordingsPath, pathName);
            if (fs.existsSync(cameraDir)) {
                const files = await fsPromises.readdir(cameraDir);
                console.log(`[REC] Files in ${cameraDir}: ${JSON.stringify(files.slice(0, 50))}`);
                try {
                    // Broadcast an event so renderer can refresh archive view automatically
                    broadcastToRenderers('recordings-updated', { cameraId: Number(cameraId), files });
                } catch (e) {
                    console.warn('[REC] Failed to broadcast recordings-updated event:', e && e.message ? e.message : e);
                }
            } else {
                console.log(`[REC] Camera recordings directory does not exist yet: ${cameraDir}`);
            }
        } catch (e) {
            console.warn('[REC] Failed to list recordings directory after stop:', e && e.message ? e.message : e);
        }
        if (type === 'manual' && camera) {
            services.showSystemNotification({ title: 'Запись завершена', body: `Камера: "${camera.name}"` });
        }
        broadcastToRenderers('recording-state-change', { cameraId, recording: false });
    } catch (e) {
        console.error(`[REC] Failed to stop recording for ${pathName}:`, e.message);
    }
}

async function toggleRecording(camera) {
    const cameraId = camera.id;
    const isCurrentlyRecording = !!recordingManager[cameraId];

    if (isCurrentlyRecording) {
        if (recordingStopTimers[cameraId]) {
            clearTimeout(recordingStopTimers[cameraId]);
            delete recordingStopTimers[cameraId];
        }
        await stopRecording(cameraId);
    } else {
        await startRecording(camera, 'manual');
    }
}

async function handleAnalyticsDetection(cameraId) {
    const camera = await configManager.getCameraConfig(cameraId);
    if (!camera) return;

    await startRecording(camera, 'auto');

    if (recordingStopTimers[cameraId]) {
        clearTimeout(recordingStopTimers[cameraId]);
    }

    const settings = await configManager.getAppSettings();
    const autoStopDelay = (settings.analytics_record_duration || 30) * 1000;

    console.log(`[REC] Setting auto-stop timer for camera ${cameraId} in ${autoStopDelay / 1000}s.`);
    
    recordingStopTimers[cameraId] = setTimeout(() => {
        if (recordingManager[cameraId]?.type === 'auto') {
            console.log(`[REC] Auto-stopping recording for camera ${cameraId} due to inactivity.`);
            stopRecording(cameraId);
        }
        delete recordingStopTimers[cameraId];
    }, autoStopDelay);
}

async function prepareArchiveForHls({ filename, startTime = 0 }) {
    const HLS_PROCESS_KEY = 'hls-conversion';
    if (processes.has(HLS_PROCESS_KEY)) {
        stopProcess(HLS_PROCESS_KEY);
    }
    const settings = await configManager.getAppSettings();
    const sourcePath = path.join(settings.recordingsPath, filename);
    const hlsTempPath = path.join(app.getPath('temp'), 'hls');
    try {
        await fsPromises.rm(hlsTempPath, { recursive: true, force: true });
        await fsPromises.mkdir(hlsTempPath, { recursive: true });
    } catch (e) {
        return { success: false, error: `Failed to clean HLS temp directory: ${e.message}` };
    }
    const videoInfo = await configManager.getArchiveVideoInfo(filename);
    const sourceCodec = videoInfo ? videoInfo.codec_name : null;
    const builder = new FfmpegCommandBuilder(settings);
    const { command, args } = builder.buildForHls(sourcePath, hlsTempPath, startTime, sourceCodec);
    const ffmpegProcess = spawn(command, args, { windowsHide: true });
    processes.set(HLS_PROCESS_KEY, { process: ffmpegProcess, type: 'hls' });
    return new Promise((resolve) => {
        const playlistPath = path.join(hlsTempPath, 'playlist.m3u8');
        const timeout = setTimeout(() => {
            stopProcess(HLS_PROCESS_KEY);
            resolve({ success: false, error: 'HLS conversion timed out.' });
        }, 60000);
        const checkFile = () => {
            if (fs.existsSync(playlistPath)) {
                clearTimeout(timeout);
                const hlsServer = require('./hls-server');
                if (!hlsServer.serverPort?.port) {
                    resolve({ success: false, error: 'HLS server is not running.' });
                    return;
                }
                resolve({ success: true, url: `http://127.0.0.1:${hlsServer.serverPort.port}/hls/playlist.m3u8` });
            } else {
                setTimeout(checkFile, 200);
            }
        };
        checkFile();
    });
}

function getRecordingStates() {
    const states = {};
    Object.keys(recordingManager).forEach(cameraId => {
        states[cameraId] = !!recordingManager[cameraId];
    });
    return states;
}

function getAnalyticsStates() {
    const states = {};
    for (const key of processes.keys()) {
        if (key.startsWith('analytics-')) {
            const cameraId = parseInt(key.replace('analytics-', ''), 10);
            if (!isNaN(cameraId)) {
                states[cameraId] = true;
            }
        }
    }
    return states;
}

async function exportArchiveClip({ sourceFilename, startTime, duration }, mainWindow) {
    const settings = await configManager.getAppSettings();
    const sourcePath = path.join(settings.recordingsPath, sourceFilename);
    const { canceled, filePath } = await dialog.showSaveDialog(mainWindow, {
        title: 'Сохранить клип', defaultPath: path.join(app.getPath('videos'), `clip-${sourceFilename}`),
        filters: [{ name: 'MP4 Videos', extensions: ['mp4'] }]
    });
    if (canceled || !filePath) return { success: false, error: 'Export cancelled' };
    return new Promise((resolve) => {
        const builder = new FfmpegCommandBuilder(settings);
        const { command, args: ffmpegArgs } = builder.buildForExport(sourcePath, startTime, duration, filePath);
        const exportProcess = spawn(command, ffmpegArgs);
        exportProcess.on('close', code => {
            if (code === 0) resolve({ success: true, path: filePath });
            else resolve({ success: false, error: `FFmpeg failed with code ${code}` });
        });
    });
}

async function toggleAnalytics(cameraId, streamId, mainWindow, moduleManager) {


    const analyticsId = `analytics-${cameraId}`;
    if (processes.has(analyticsId)) {
        stopProcess(analyticsId);
        broadcastToRenderers('analytics-status-change', { cameraId, active: false });
        return { success: true, status: 'stopped' };
    }

    // Получаем настройки и конфиг камеры
    const settings = await configManager.getAppSettings();
    const camera = await configManager.getCameraConfig(cameraId);
    if (!camera) return { success: false, error: 'Camera not found' };

    // Determine effective stream id. Analytics must always run on HD (0) so overlays map correctly.
    // Ignore the caller-provided streamId (which often corresponds to the SD preview).
    let effectiveStreamId = 0;
    let mediaMtxStreamName = `cam${camera.id}_${effectiveStreamId}`;
    let rtspUrl = `rtsp://127.0.0.1:8554/${mediaMtxStreamName}`;

    // --- ДОБАВЛЯЕМ ПАРАМЕТРЫ ДЛЯ FACE DETECTOR ---
    const configForScript = {
        objects: camera.analyticsConfig?.objects || ['person', 'car'],
        confidence: camera.analyticsConfig?.confidence || 0.5,
        frame_skip: parseInt(settings.analytics_frame_skip, 10) || 5,
        resize_width: parseInt(settings.analytics_resize_width, 10) || 640,
    };

    // Проверяем, активен ли FaceDetector
    let faceDetectorActive = false;
    try {
        const enabledModules = settings.enabledModules || [];
        faceDetectorActive = enabledModules.includes('face-detector');
    } catch (e) {}

    if (faceDetectorActive) {
        // Определяем папку для faces_dir
        let facesDir;
        try {
            const moduleFolderName = 'face-detector';
            const savePathKey = `module_${moduleFolderName}_savePath`;
            if (settings[savePathKey]) {
                facesDir = settings[savePathKey];
            } else {
                facesDir = path.join(configManager.getDataPath(), 'faces');
            }
        } catch (e) {
            facesDir = path.join(configManager.getDataPath(), 'faces');
        }
        configForScript.save_face_frame = true;
        configForScript.faces_dir = facesDir;
    }

    const configArg = Buffer.from(JSON.stringify(configForScript)).toString('base64');
    const providerChoice = settings.analytics_provider || 'auto';

    // --- ВЫБОР МОДУЛЯ АНАЛИТИКИ ---
    // Если в objects есть 'license_plate', используем analytics_plate.py
    const usePlate = (configForScript.objects || []).includes('license_plate');
    // We always run analytics on HD (effectiveStreamId === 0). For license-plate analytics
    // we must connect to the camera's direct RTSP URL (not MediaMTX) so the script has
    // direct access for fast, full-resolution frames required by OCR/cropping.
    if (usePlate) {
        try {
            // obtain camera password and build a direct RTSP URL
            const password = await authManager.getPasswordForCamera(camera.id);
            const fullCredentials = { ...camera, password };
            const builder = new FfmpegCommandBuilder(settings || {});
            const directStreamPath = fullCredentials.streamPath0 || '/stream=0';
            const directRtsp = builder.buildRtspUrl(fullCredentials, directStreamPath);
            // Mask credentials for logging
            const masked = directRtsp.replace(/:\S+@/, ':****@');
            rtspUrl = directRtsp;
            console.log(`[Analytics] Using DIRECT RTSP for license-plate analytics camera ${cameraId}: ${masked}`);
        } catch (e) {
            // Fallback to MediaMTX HD stream if direct build fails for any reason
            mediaMtxStreamName = `cam${camera.id}_0`;
            rtspUrl = `rtsp://127.0.0.1:8554/${mediaMtxStreamName}`;
            console.warn(`[Analytics] Failed to build direct RTSP for camera ${cameraId}, falling back to MediaMTX:`, e.message);
        }
    } else {
        // Non-plate analytics will use MediaMTX HD stream
        mediaMtxStreamName = `cam${camera.id}_0`;
        rtspUrl = `rtsp://127.0.0.1:8554/${mediaMtxStreamName}`;
    }
    let analyticsProcess;
    if (usePlate) {
        // Путь к python и скрипту
        const pythonCmd = process.platform === 'win32' ? 'python' : 'python3';
        // Абсолютный путь к скрипту test_plate_yunet.py
        const scriptPath = path.join(app.getAppPath(), 'python_src', 'test_plate_yunet.py');
        if (!fs.existsSync(scriptPath)) {
            const errorMsg = `test_plate_yunet.py not found at path: ${scriptPath}`;
            console.error(`[Analytics ERROR] ${errorMsg}`);
            dialog.showErrorBox('Ошибка запуска аналитики', errorMsg);
            return { success: false, error: errorMsg };
        }
        console.log('--- [Analytics DEBUG] ---');
        console.log(`[Analytics DEBUG] Starting license plate analytics (test_plate_yunet.py) for camera ID: ${cameraId}`);
        console.log(`[Analytics DEBUG] Script path: ${scriptPath}`);
        console.log(`[Analytics DEBUG] Connecting to LOCAL MediaMTX stream URL: ${rtspUrl}`);
        console.log(`[Analytics DEBUG] Config object being sent:`, configForScript);
        console.log(`[Analytics DEBUG] Provider choice: ${providerChoice}`);
        console.log('-------------------------');
        analyticsProcess = spawn(pythonCmd, [scriptPath, '--video', rtspUrl, '--ocr-engine', 'auto'], { windowsHide: true });
    } else {
        // --- СТАРЫЙ ВАРИАНТ: exe ---
        const analyticsBasePath = app.isPackaged
            ? path.join(process.resourcesPath, 'analytics')
            : path.join(app.getAppPath(), 'python_src', 'extra', 'analytics');
        const analyticsExecutableName = process.platform === 'win32' ? 'analytics_dml.exe' : 'analytics_cpu';
        const analyticsPath = path.join(analyticsBasePath, analyticsExecutableName);
        if (!fs.existsSync(analyticsPath)) {
            const errorMsg = `Analytics executable not found at path: ${analyticsPath}`;
            console.error(`[Analytics ERROR] ${errorMsg}`);
            dialog.showErrorBox('Ошибка запуска аналитики', errorMsg);
            return { success: false, error: errorMsg };
        }
        console.log('--- [Analytics DEBUG] ---');
        console.log(`[Analytics DEBUG] Starting analytics for camera ID: ${cameraId}`);
        console.log(`[Analytics DEBUG] Executable path: ${analyticsPath}`);
        console.log(`[Analytics DEBUG] Connecting to LOCAL MediaMTX stream URL: ${rtspUrl}`);
        console.log(`[Analytics DEBUG] Config object being sent:`, configForScript);
        console.log(`[Analytics DEBUG] Provider choice: ${providerChoice}`);
        console.log('-------------------------');
        analyticsProcess = spawn(analyticsPath, [rtspUrl, configArg, providerChoice], { windowsHide: true });
    }
    addProcess(analyticsId, analyticsProcess, PROCESS_TYPES.ANALYTICS);

    analyticsProcess.on('error', (err) => {
        console.error('[Analytics ERROR] Failed to start analytics subprocess.', err);
        broadcastToRenderers('on-main-error', { context: 'Analytics', message: `Не удалось запустить подпроцесс: ${err.message}` });
        stopProcess(analyticsId);
        broadcastToRenderers('analytics-status-change', { cameraId, active: false });
    });

    analyticsProcess.stderr.on('data', (data) => {
        console.error(`[Analytics Stderr] Camera ${cameraId}: ${data.toString().trim()}`);
    });

    analyticsProcess.stdout.on('data', async (data) => {
        data.toString().split('\n').filter(Boolean).forEach(async line => {
            console.log(`[Analytics Stdout] Camera ${cameraId}: ${line.trim()}`);
            let result;
            try {
                result = JSON.parse(line);
            } catch (e) {
                return;
            }


            if (result.frame_path && typeof result.frame_path === 'string') {
                const tempImagePath = result.frame_path;
                try {
                    const imageBuffer = await fsPromises.readFile(tempImagePath);
                    const imageBase64 = imageBuffer.toString('base64');
                    result.frame_base64 = `data:image/jpeg;base64,${imageBase64}`;
                    console.log('[analytics-debug] frame_base64 generated for camera', cameraId, 'length:', imageBase64.length);
                } catch (readError) {
                    console.error(`[Analytics] Failed to read temp frame file: ${tempImagePath}`, readError);
                }
            } else {
                console.warn('[analytics-debug] No frame_path in result for camera', cameraId);
            }


            const listeners = moduleManager.getListeners('analytics-update');
            console.log('[analytics-debug] Sending result to listeners:', JSON.stringify(result).slice(0, 500));
            for (const listener of listeners) {
                try { await listener({ cameraId, result }); } catch (e) { console.error(e) }
            }

            // Теперь удаляем временный файл кадра после всех слушателей
            if (result.frame_path && typeof result.frame_path === 'string') {
                const tempImagePath = result.frame_path;
                try {
                    await fsPromises.unlink(tempImagePath);
                } catch (unlinkError) {
                    // Ignore
                }
            }

            const channel = result.status === 'info' ? 'analytics-provider-info' : 'analytics-update';
            broadcastToRenderers(channel, { cameraId, result });
            
            if (result.status === 'objects_detected' && result.objects.length > 0) {
                await configManager.saveAnalyticsEvent({ cameraId, ...result });
                await handleAnalyticsDetection(cameraId);
                const labels = [...new Set(result.objects.map(o => o.label))];
                services.showAnalyticsNotification(camera.name, cameraId, labels);
            }
        });
    });

    analyticsProcess.on('close', (code, signal) => {
        console.log(`[Analytics] Process for camera ${cameraId} closed with code: ${code}, signal: ${signal}`);
        if (!analyticsProcess.isManuallyStopping && code !== 0) {
            const errorMessage = `Процесс аналитики для камеры ${camera.name} неожиданно завершился с кодом ошибки ${code}. Проверьте логи.`;
            console.error(`[Analytics ERROR] ${errorMessage}`);
            broadcastToRenderers('on-main-error', { context: 'Analytics', message: errorMessage });
        }
        stopProcess(analyticsId);
        broadcastToRenderers('analytics-status-change', { cameraId, active: false });
    });

    broadcastToRenderers('analytics-status-change', { cameraId, active: true });
    return { success: true, status: 'started' };
}

async function killAllFfmpeg() {
    console.log('[ProcessManager] Received kill-all command. Stopping all tracked processes.');
    stopAllProcesses(); 
    return { success: true, message: "Все потоки и процессы сброшены." };
}

module.exports = {
    startMediaMTX,
    generateAndSaveMediaMTXConfig,
    stopMediaMTX,
    isMediaMTXRunning,
    updateMediaMTXPaths,
    addProcess,
    stopProcess,
    getAllProcessesOfType,
    stopAllProcesses,
    toggleRecording,
    exportArchiveClip,
    toggleAnalytics,
    killAllFfmpeg,
    prepareArchiveForHls,
    setWebSocketServer,
    getRecordingStates,
    getAnalyticsStates,
    getArchiveThumbnails
};