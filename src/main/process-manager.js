// --- START OF FILE src/main/process-manager.js ---
const { spawn, exec } = require('child_process');
const path = require('path');
const fs = require('fs');
const fsPromises = require('fs').promises;
const net = require('net');
const WebSocket = require('ws');
const { Mutex } = require('async-mutex');
const { app, dialog, Notification } = require('electron');
const os = require('os');

const configManager = require('./config-manager');
const authManager = require('./auth-manager');
const services = require('./services');
const cameraAPI = require('./camera-api');
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

function parseBitrate(bitrateString) {
    if (!bitrateString || typeof bitrateString !== 'string') {
        return 0;
    }
    const value = parseFloat(bitrateString);
    if (isNaN(value)) {
        return 0;
    }
    if (bitrateString.toLowerCase().includes('kbits/s')) {
        return value;
    }
    if (bitrateString.toLowerCase().includes('bits/s')) {
        return value / 1000;
    }
    return value;
}

function getLocalTimestampForFilename() {
    const d = new Date();
    const pad = (n) => String(n).padStart(2, '0');
    const date = `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}`;
    const time = `${pad(d.getHours())}-${pad(d.getMinutes())}-${pad(d.getSeconds())}`;
    return `${date}T${time}`;
}

const portMutex = new Mutex();
const usedPorts = new Set();
const BASE_PORT = 9001;

const PROCESS_TYPES = { STREAM: 'stream', RECORDING: 'recording', ANALYTICS: 'analytics', HLS: 'hls' };
const processes = new Map();
const streamManager = {};
const recordingManager = {};
const recordingStopTimers = {};
const deadStreamWatchdog = new Map();

function getServerIp() {
    const interfaces = os.networkInterfaces();
    const candidates = [];

    for (const name of Object.keys(interfaces)) {
        for (const iface of interfaces[name]) {
            if (iface.family === 'IPv4' && !iface.internal) {
                candidates.push(iface.address);
            }
        }
    }

    if (candidates.length === 0) {
        console.log('[Network] No network interfaces found, falling back to localhost.');
        return '127.0.0.1';
    }

    const lanIp = candidates.find(ip => ip.startsWith('192.168.'));
    if (lanIp) {
        console.log(`[Network] Prioritized LAN IP found: ${lanIp}`);
        return lanIp;
    }

    const corporateIp = candidates.find(ip => ip.startsWith('10.'));
    if (corporateIp) {
        console.log(`[Network] Found corporate LAN IP: ${corporateIp}`);
        return corporateIp;
    }

    const carrierGradeNatIp = candidates.find(ip => /^172\.(1[6-9]|2[0-9]|3[0-1])\./.test(ip));
    if (carrierGradeNatIp) {
        console.log(`[Network] Found Carrier-grade NAT IP: ${carrierGradeNatIp}`);
        return carrierGradeNatIp;
    }
    
    const firstCandidate = candidates[0];
    console.log(`[Network] No prioritized IP found, using first available: ${firstCandidate}`);
    return firstCandidate;
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
    console.log(`[ProcessManager] Stopping all ${processes.size} tracked processes.`);
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

async function getAndReserveFreePort() {
    const release = await portMutex.acquire();
    try {
        let port = BASE_PORT;
        const MAX_PORTS_TO_CHECK = 200;
        for (let i = 0; i < MAX_PORTS_TO_CHECK; i++) {
            const currentPort = port + i;
            if (usedPorts.has(currentPort)) {
                continue;
            }
            const inUse = await new Promise(resolve => {
                const server = net.createServer();
                server.once('error', () => resolve(true));
                server.once('listening', () => server.close(() => resolve(false)));
                server.listen(currentPort);
            });
            if (inUse) continue;
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

async function startVideoStream({ credentials, uniqueStreamIdentifier }) {
    const cameraConfig = await configManager.getCameraConfig(credentials.id);
    if (!cameraConfig) return { success: false, error: `Camera with ID ${credentials.id} not found.` };
    const password = await authManager.getPasswordForCamera(credentials.id);
    const fullCredentials = { ...cameraConfig, password };
    
    const settings = await configManager.getAppSettings();
    const builder = new FfmpegCommandBuilder(settings);

    const streamsToStart = [
        { id: 0, quality: 'hd' },
        { id: 1, quality: 'sd' }
    ];

    const results = {};

    for (const streamInfo of streamsToStart) {
        const specificIdentifier = `${uniqueStreamIdentifier}_${streamInfo.id}`;
        
        if (streamManager[specificIdentifier]) {
            results[streamInfo.quality] = { success: true, wsPort: streamManager[specificIdentifier].port };
            continue;
        }

        const wsPort = await getAndReserveFreePort();
        const statsPort = await getAndReserveFreePort();

        if (wsPort === null || statsPort === null) {
            releasePort(wsPort);
            releasePort(statsPort);
            Object.values(results).forEach(res => { if (res.wsPort) releasePort(res.wsPort); });
            return { success: false, error: 'Failed to reserve necessary ports.' };
        }

        const { command, args } = builder.buildForStream(fullCredentials, streamInfo.id, statsPort);
        const ffmpegProcess = spawn(command, args, { detached: false, windowsHide: true });
        addProcess(specificIdentifier, ffmpegProcess, PROCESS_TYPES.STREAM);

        const statsServer = net.createServer(socket => {
            let statsBuffer = '';
            socket.on('data', data => {
                statsBuffer += data.toString();
                const statsBlocks = statsBuffer.split('progress=');
                if (statsBlocks.length > 1) {
                    statsBlocks.slice(0, -1).forEach(block => {
                        if (!block.trim()) return;
                        const stats = Object.fromEntries(block.trim().split('\n').map(line => line.split('=').map(s => s.trim())));
                        if (stats.fps || stats.bitrate) {
                            const payload = { 
                                uniqueStreamIdentifier: specificIdentifier, 
                                fps: parseFloat(stats.fps) || 0, 
                                bitrate: parseBitrate(stats.bitrate)
                            };
                            broadcastToRenderers('stream-stats', payload);
                        }
                    });
                    statsBuffer = statsBlocks.pop();
                }
            });
            socket.on('error', (err) => {
                console.error(`[Stats TCP Server] Error on socket for ${specificIdentifier}:`, err.message);
            });
        }).listen(statsPort, '127.0.0.1');

        statsServer.on('error', (err) => {
            console.error(`[Stats TCP Server] Failed to start for ${specificIdentifier}:`, err.message);
        });
        
        const wss = new WebSocket.Server({ port: wsPort });
        wss.on('connection', () => console.log(`[WSS] Client connected to ${streamInfo.quality.toUpperCase()} on port ${wsPort}`));
        ffmpegProcess.stdout.on('data', (data) => wss.clients.forEach(c => c.readyState === WebSocket.OPEN && c.send(data)));
        
        let lastErrorOutput = '', streamInfoSent = false;
        ffmpegProcess.stderr.on('data', (data) => {
            const stderrString = data.toString();

            if (!streamInfoSent) {
                const infoMatch = stderrString.match(/Stream #\d:\d.*: Video: (\w+)(?:\s\([^)]+\))?,.*?\s(\d{3,4}x\d{3,4})/);
                if (infoMatch && infoMatch[1] && infoMatch[2]) {
                    const [_, codec, resolution] = infoMatch;
                    const payload = { uniqueStreamIdentifier: specificIdentifier, codec, resolution };
                    if (streamManager[specificIdentifier]) {
                        streamManager[specificIdentifier].info = payload;
                    }
                    broadcastToRenderers('stream-info-update', payload);
                    streamInfoSent = true;
                }
            }
            if (stderrString.trim()) { lastErrorOutput = stderrString.trim(); }
        });

        ffmpegProcess.on('close', (code, signal) => {
            console.warn(`[FFMPEG] Process ${specificIdentifier} exited. Code: ${code}. Last error: ${lastErrorOutput}`);
            
            const streamData = streamManager[specificIdentifier];
            if (streamData) {
                streamData.wss.close();
                streamData.statsServer.close();
                releasePort(streamData.port);
                releasePort(streamData.statsPort);
                delete streamManager[specificIdentifier];
            }
            processes.delete(specificIdentifier);

            if (!ffmpegProcess.isManuallyStopping && !deadStreamWatchdog.has(uniqueStreamIdentifier)) {
                console.log(`[Watchdog] Stream set ${uniqueStreamIdentifier} died unexpectedly. Starting reconnection attempts.`);
                
                broadcastToRenderers('stream-died', { 
                    uniqueStreamIdentifier: `${uniqueStreamIdentifier}_0`, 
                    error: `Stream lost. Reconnecting...`
                });
                broadcastToRenderers('stream-died', { 
                    uniqueStreamIdentifier: `${uniqueStreamIdentifier}_1`, 
                    error: `Stream lost. Reconnecting...`
                });

                deadStreamWatchdog.set(uniqueStreamIdentifier, {
                    credentials: fullCredentials,
                    attempts: 0,
                    timer: setTimeout(() => attemptReconnect(uniqueStreamIdentifier), 5000)
                });
            }
        });

        streamManager[specificIdentifier] = { wss, port: wsPort, statsPort, statsServer, info: null };
        results[streamInfo.quality] = { success: true, wsPort };
    }

    const hdInfo = streamManager[`${uniqueStreamIdentifier}_0`]?.info;
    const sdInfo = streamManager[`${uniqueStreamIdentifier}_1`]?.info;

    return { 
        success: (results.hd?.success && results.sd?.success),
        hdPort: results.hd?.wsPort,
        sdPort: results.sd?.wsPort,
        serverIp: getServerIp(),
        hdInfo: hdInfo,
        sdInfo: sdInfo,
        error: (results.hd?.error) || (results.sd?.error)
    };
}

async function stopVideoStream(uniqueStreamIdentifier) {
    console.log(`[ProcessManager] Stopping stream set for base ID: ${uniqueStreamIdentifier}`);
    
    const watchdogEntry = deadStreamWatchdog.get(uniqueStreamIdentifier);
    if (watchdogEntry && watchdogEntry.timer) {
        clearTimeout(watchdogEntry.timer);
        deadStreamWatchdog.delete(uniqueStreamIdentifier);
        console.log(`[Watchdog] Canceled reconnect attempts for ${uniqueStreamIdentifier}.`);
    }

    ['0', '1'].forEach(streamIndex => {
        const streamId = `${uniqueStreamIdentifier}_${streamIndex}`;
        const streamData = streamManager[streamId];
        if (streamData) {
            stopProcess(streamId);
            streamData.wss.close();
            streamData.statsServer.close();
            releasePort(streamData.port);
            releasePort(streamData.statsPort);
            delete streamManager[streamId];
        }
    });
    return { success: true };
}

async function attemptReconnect(baseIdentifier) {
    const watchdogEntry = deadStreamWatchdog.get(baseIdentifier);
    if (!watchdogEntry) {
        return;
    }

    watchdogEntry.attempts++;
    console.log(`[Watchdog] Reconnect attempt #${watchdogEntry.attempts} for stream set ${baseIdentifier}`);

    const { credentials } = watchdogEntry;
    
    let cameraIsBack = false;
    try {
        const pulse = await cameraAPI.getCameraPulse(credentials);
        if (pulse.success) {
            console.log(`[Watchdog] Camera ${credentials.name} is back online!`);
            cameraIsBack = true;
        }
    } catch (e) {
        // Камера еще недоступна
    }

    if (cameraIsBack) {
        // VVVVVV --- ИЗМЕНЕНИЕ ЗДЕСЬ --- VVVVVV
        // Вместо того чтобы пытаться запустить поток здесь и отправлять 'stream-reconnected',
        // мы просто говорим фронтенду полностью перерисовать сетку.
        // Это более чистый подход, который избегает гонки состояний.
        console.log(`[Watchdog] Reconnect successful for ${baseIdentifier}. Triggering force render.`);
        deadStreamWatchdog.delete(baseIdentifier);
        broadcastToRenderers('force-render', {}); // Отправляем команду на полную перерисовку
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
    } else {
        scheduleNextAttempt(baseIdentifier, watchdogEntry);
    }
}

function scheduleNextAttempt(baseIdentifier, watchdogEntry) {
    console.warn(`[Watchdog] Reconnect attempt #${watchdogEntry.attempts} failed for ${baseIdentifier}.`);
    if (watchdogEntry.attempts >= 15) {
        console.error(`[Watchdog] Max reconnect attempts reached for ${baseIdentifier}. Giving up.`);
        broadcastToRenderers('stream-died', { 
            uniqueStreamIdentifier: `${baseIdentifier}_0`, 
            error: `Failed to reconnect after ${watchdogEntry.attempts} attempts.`
        });
        broadcastToRenderers('stream-died', { 
            uniqueStreamIdentifier: `${baseIdentifier}_1`, 
            error: `Failed to reconnect after ${watchdogEntry.attempts} attempts.`
        });
        deadStreamWatchdog.delete(baseIdentifier);
    } else {
        watchdogEntry.timer = setTimeout(() => attemptReconnect(baseIdentifier), 10000);
    }
}

async function pauseVideoStream(uniqueStreamIdentifier) {
    console.log(`[ProcessManager] Pausing stream set by stopping processes for base ID: ${uniqueStreamIdentifier}`);
    // Эта функция теперь просто останавливает ffmpeg, но НЕ удаляет данные из streamManager
    ['0', '1'].forEach(streamIndex => {
        const streamId = `${uniqueStreamIdentifier}_${streamIndex}`;
        stopProcess(streamId); // Убиваем процесс
        const streamData = streamManager[streamId];
        if (streamData) {
            streamData.wss.close(); // Закрываем WebSocket сервер
            streamData.statsServer.close();
            releasePort(streamData.port);
            releasePort(streamData.statsPort);
            delete streamManager[streamId];
        }
    });
    return { success: true };
}

async function resumeVideoStream(uniqueStreamIdentifier) {
    console.log(`[ProcessManager] Resuming stream set by restarting processes for base ID: ${uniqueStreamIdentifier}`);
    
    const parts = uniqueStreamIdentifier.match(/stream-(\d+)_(\d+)/);
    if (!parts) {
        return { success: false, error: 'Invalid stream identifier' };
    }
    const cameraId = parseInt(parts[1], 10);

    const camera = await configManager.getCameraConfig(cameraId);
    if (!camera) {
        return { success: false, error: 'Camera not found' };
    }

    // Запускаем потоки заново.
    const result = await startVideoStream({ credentials: camera, uniqueStreamIdentifier });

    // Отправляем событие, чтобы фронтенд пересоздал плеер.
    if (result.success) {
        broadcastToRenderers('stream-reconnected', { uniqueStreamIdentifier });
    }
    
    return result;
}

async function saveScreenshot(uniqueStreamIdentifier) {
    const parts = uniqueStreamIdentifier.match(/stream-(\d+)_(\d+)/);
    if (!parts) {
        return { success: false, error: 'Invalid stream identifier for screenshot' };
    }
    const cameraId = parseInt(parts[1], 10);

    try {
        const camera = await configManager.getCameraConfig(cameraId);
        if (!camera) {
            return { success: false, error: 'Camera not found for screenshot.' };
        }
        const password = await authManager.getPasswordForCamera(camera.id);
        const fullCredentials = { ...camera, password };

        const settings = await configManager.getAppSettings();
        const screenshotsPath = settings.screenshotsPath || path.join(settings.recordingsPath, 'Screenshots');
        await fsPromises.mkdir(screenshotsPath, { recursive: true });

        const saneCameraName = camera.name.replace(/[<>:"/\\|?*]/g, '_');
        const timestamp = getLocalTimestampForFilename();
        const outputPath = path.join(screenshotsPath, `${saneCameraName}-${timestamp}.jpg`);

        const builder = new FfmpegCommandBuilder(settings);
        const rtspUrl = builder.buildRtspUrl(fullCredentials, fullCredentials.streamPath0 || '/stream0');

        const ffmpegPath = require('@ffmpeg-installer/ffmpeg').path.replace('app.asar', 'app.asar.unpacked');
        const args = [
            '-rtsp_transport', 'tcp',
            '-y',
            '-i', rtspUrl,
            '-vframes', '1',
            '-q:v', '2',
            outputPath
        ];

        return new Promise((resolve) => {
            const process = spawn(ffmpegPath, args, { windowsHide: true });
            let errorOutput = '';
            process.stderr.on('data', (data) => {
                errorOutput += data.toString();
            });

            process.on('close', (code) => {
                if (code === 0) {
                    console.log(`[Screenshot] Successfully saved to ${outputPath}`);
                    resolve({ success: true, path: outputPath });
                } else {
                    console.error(`[Screenshot] FFmpeg failed with code ${code}:`, errorOutput);
                    resolve({ success: false, error: `FFmpeg failed: ${errorOutput.split('\n').pop()}` });
                }
            });

            process.on('error', (err) => {
                console.error('[Screenshot] Failed to start FFmpeg process:', err);
                resolve({ success: false, error: err.message });
            });
        });

    } catch (error) {
        console.error('[Screenshot] General error:', error);
        return { success: false, error: error.message };
    }
}

async function startRecording(camera, type = 'manual') {
    if (!camera || !camera.id) return { success: false, error: 'Invalid camera data' };
    const recordingId = `recording-${camera.id}`;
    if (recordingManager[camera.id]) {
        console.log(`[REC] Recording for camera ${camera.id} is already active.`);
        return { success: true, message: 'Recording already in progress' };
    }
    const password = await authManager.getPasswordForCamera(camera.id);
    const fullCameraInfo = { ...camera, password };
    const settings = await configManager.getAppSettings();
    await fsPromises.mkdir(settings.recordingsPath, { recursive: true });
    const saneCameraName = fullCameraInfo.name.replace(/[<>:"/\\|?*]/g, '_');
    const timestamp = getLocalTimestampForFilename();
    const outputPath = path.join(settings.recordingsPath, `${saneCameraName}-${timestamp}.mp4`);
    const builder = new FfmpegCommandBuilder(settings);
    const { command, args: ffmpegArgs } = builder.buildForRecording(fullCameraInfo, outputPath);
    console.log(`[REC] Starting ${type} recording for camera ${camera.id}`);
    const ffmpegProcess = spawn(command, ffmpegArgs, { detached: false, windowsHide: true });
    let ffmpegErrorOutput = '';
    ffmpegProcess.stderr.on('data', (data) => {
        const errorLine = data.toString();
        ffmpegErrorOutput += errorLine;
        console.error(`[FFMPEG REC ERROR] ${errorLine.trim()}`);
    });
    processes.set(recordingId, { process: ffmpegProcess, type: 'recording' });
    recordingManager[camera.id] = { path: outputPath, process: ffmpegProcess, type: type };
    if (type === 'manual') {
        services.showSystemNotification({ title: 'Запись начата', body: `Камера: "${fullCameraInfo.name}"` });
    }
    ffmpegProcess.on('close', (code) => {
        console.log(`[REC] FFmpeg process for camera ${camera.id} closed with code ${code}.`);
        const wasStoppedIntentionally = !recordingManager[camera.id];
        if (wasStoppedIntentionally) {
            console.log(`[REC] Recording for ${camera.id} was stopped intentionally.`);
            services.showSystemNotification({ title: 'Запись завершена', body: `Файл сохранен для камеры "${fullCameraInfo.name}"` });
        } else {
            console.error(`[REC] Recording process for ${camera.id} exited unexpectedly.`);
            if (ffmpegErrorOutput) {
                console.error(`[REC] Full FFmpeg stderr output:\n${ffmpegErrorOutput}`);
            }
            services.showSystemNotification({ title: 'Ошибка записи', body: `Камера: "${fullCameraInfo.name}"` });
            stopRecording(camera.id);
        }
    });
    broadcastToRenderers('recording-state-change', { cameraId: camera.id, recording: true });
    return { success: true };
}

function stopRecording(cameraId) {
    const recordingId = `recording-${cameraId}`;
    if (recordingStopTimers[cameraId]) {
        clearTimeout(recordingStopTimers[cameraId]);
        delete recordingStopTimers[cameraId];
    }
    if (recordingManager[cameraId]) {
        delete recordingManager[cameraId];
    }
    if (stopProcess(recordingId)) {
        broadcastToRenderers('recording-state-change', { cameraId, recording: false });
        return { success: true };
    }
    return { success: false, error: 'Recording not found' };
}

async function toggleRecording(camera) {
    const isCurrentlyRecording = !!recordingManager[camera.id];
    if (isCurrentlyRecording) {
        return stopRecording(camera.id);
    } else {
        return startRecording(camera, 'manual');
    }
}

async function handleAnalyticsDetection(cameraId, camera) {
    const settings = await configManager.getAppSettings();
    const autoStopDelay = (settings.analytics_record_duration || 30) * 1000;
    if (recordingManager[cameraId]?.type === 'manual') {
        return;
    }
    if (!recordingManager[cameraId]) {
        await startRecording(camera, 'auto');
    }
    if (recordingStopTimers[cameraId]) {
        clearTimeout(recordingStopTimers[cameraId]);
    }
    recordingStopTimers[cameraId] = setTimeout(() => {
        if (recordingManager[cameraId]?.type === 'auto') {
            console.log(`[REC] Auto-stopping recording for camera ${cameraId} due to inactivity.`);
            stopRecording(cameraId);
        }
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
                resolve({ success: true, url: `http://localhost:${hlsServer.serverPort.port}/hls/playlist.m3u8` });
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

async function toggleAnalytics(cameraId, mainWindow, moduleManager) {
    const analyticsId = `analytics-${cameraId}`;
    if (processes.has(analyticsId)) {
        stopProcess(analyticsId);
        if (recordingStopTimers[cameraId]) stopRecording(cameraId);
        return { success: true, status: 'stopped' };
    }
    const settings = await configManager.getAppSettings();
    const camera = await configManager.getCameraConfig(cameraId);
    if (!camera) return { success: false, error: 'Camera not found' };
    const password = await authManager.getPasswordForCamera(camera.id);
    const fullCameraInfo = { ...camera, password };
    const builder = new FfmpegCommandBuilder(settings);
    const rtspUrl = builder.buildRtspUrl(fullCameraInfo, fullCameraInfo.streamPath0 || '/stream0');
    const analyticsPath = path.join(app.isPackaged ? process.resourcesPath : 'extra', 'analytics', process.platform === 'win32' ? 'analytics_dml.exe' : 'analytics_cpu');
    if (!fs.existsSync(analyticsPath)) {
        const errorMsg = `Analytics executable not found: ${analyticsPath}`;
        dialog.showErrorBox('Ошибка аналитики', errorMsg);
        return { success: false, error: errorMsg };
    }
    const configForScript = {
        objects: camera.analyticsConfig?.objects || ['person'],
        confidence: camera.analyticsConfig?.confidence || 0.5,
        frame_skip: settings.analytics_frame_skip || 5,
    };
    const configArg = Buffer.from(JSON.stringify(configForScript)).toString('base64');
    const providerChoice = settings.analytics_provider || 'auto';
    const analyticsProcess = spawn(analyticsPath, [rtspUrl, configArg, providerChoice], { windowsHide: true });
    processes.set(analyticsId, { process: analyticsProcess, type: 'analytics' });
    analyticsProcess.stdout.on('data', async (data) => {
        data.toString().split('\n').filter(Boolean).forEach(async line => {
            let result;
            try { result = JSON.parse(line); } catch (e) { return; }
            const listeners = moduleManager.getListeners('analytics-update');
            for (const listener of listeners) {
                try { await listener({ cameraId, result }); } catch (e) { console.error(e) }
            }
            const channel = result.status === 'info' ? 'analytics-provider-info' : 'analytics-update';
            broadcastToRenderers(channel, { cameraId, result });
            
            if (result.status === 'objects_detected' && result.objects.length > 0) {
                await configManager.saveAnalyticsEvent({ cameraId, ...result });
                const labels = [...new Set(result.objects.map(o => o.label))];
                services.showAnalyticsNotification(camera.name, cameraId, labels);
                await handleAnalyticsDetection(cameraId, camera);
            }
        });
    });
    analyticsProcess.on('close', (code, signal) => {
        stopProcess(analyticsId);
        broadcastToRenderers('analytics-status-change', { cameraId, active: false });
    });
    broadcastToRenderers('analytics-status-change', { cameraId, active: true });
    return { success: true, status: 'started' };
}

async function killAllFfmpeg() {
    console.log('[ProcessManager] Received kill-all command. Stopping all tracked processes.');
    stopAllProcesses(); 
    Object.values(streamManager).forEach(s => s.wss?.close());
    usedPorts.clear();
    Object.keys(streamManager).forEach(k => delete streamManager[k]);
    Object.keys(recordingManager).forEach(k => delete recordingManager[k]);
    return { success: true, message: "Все потоки и процессы сброшены." };
}

module.exports = {
    addProcess,
    stopProcess,
    getAllProcessesOfType,
    stopAllProcesses,
    startVideoStream,
    stopVideoStream,
    pauseVideoStream,
    resumeVideoStream,
    saveScreenshot,
    toggleRecording,
    exportArchiveClip,
    toggleAnalytics,
    killAllFfmpeg,
    prepareArchiveForHls,
    setWebSocketServer,
    getRecordingStates,
    getAnalyticsStates
};