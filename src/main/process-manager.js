// --- START OF FILE src/main/process-manager.js ---
const { spawn, exec } = require('child_process');
const path = require('path');
const fs = require('fs');
const fsPromises = require('fs').promises;
const net = require('net');
const WebSocket = require('ws');
const { Mutex } = require('async-mutex');
const { app, dialog } = require('electron');

const configManager = require('./config-manager');
const authManager = require('./auth-manager');
const services = require('./services');
const FfmpegCommandBuilder = require('./ffmpeg-builder');

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

const buildProcessId = (type, id) => `${type}-${id}`;

function addProcess(key, process, type) {
    console.log(`[ProcessManager] Adding ${type} process with key: ${key}`);
    processes.set(key, { process, type });
}

function stopProcess(key) {
    if (processes.has(key)) {
        const { process: childProcess, type } = processes.get(key);
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
                childProcess.kill('SIGKILL');
            }
        } catch (e) {
            console.error(`[ProcessManager] Error killing process ${key}: ${e.message}`);
        }
    }
    processes.clear();
}

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

async function startVideoStream({ credentials, streamId, uniqueStreamIdentifier }) {
    if (!uniqueStreamIdentifier) {
        return { success: false, error: 'uniqueStreamIdentifier is required' };
    }
    if (processes.has(uniqueStreamIdentifier) || streamManager[uniqueStreamIdentifier]) {
        return { success: true, wsPort: streamManager[uniqueStreamIdentifier].port };
    }
    const cameraConfig = await configManager.getCameraConfig(credentials.id);
    if (!cameraConfig) return { success: false, error: `Camera with ID ${credentials.id} not found.` };
    const password = await authManager.getPasswordForCamera(credentials.id);
    const fullCredentials = { ...cameraConfig, password };
    const wsPort = await getAndReserveFreePort();
    if (wsPort === null) return { success: false, error: 'Failed to find a free port.' };
    const wss = new WebSocket.Server({ port: wsPort });
    wss.on('connection', (ws) => console.log(`[WSS] Client connected to port ${wsPort}`));
    const settings = await configManager.getAppSettings();
    const builder = new FfmpegCommandBuilder(settings);
    const { command, args: ffmpegArgs } = builder.buildForStream(fullCredentials, streamId);
    console.log(`[FFMPEG] Starting stream ${uniqueStreamIdentifier} with command:`, command, ffmpegArgs.join(' '));
    const ffmpegProcess = spawn(command, ffmpegArgs, { detached: false, windowsHide: true });
    addProcess(uniqueStreamIdentifier, ffmpegProcess, PROCESS_TYPES.STREAM);
    ffmpegProcess.on('error', (err) => {
        console.error(`[FFMPEG] Failed to start process for ${uniqueStreamIdentifier}:`, err);
        services.handleError(err, `ffmpeg-spawn-${uniqueStreamIdentifier}`);
        wss.close(); 
        releasePort(wsPort); 
        delete streamManager[uniqueStreamIdentifier];
        stopProcess(uniqueStreamIdentifier);
    });
    ffmpegProcess.stdout.on('data', (data) => wss.clients.forEach(c => c.readyState === WebSocket.OPEN && c.send(data)));
    let statsBuffer = '', lastErrorOutput = '', streamInfoSent = false;
    ffmpegProcess.stderr.on('data', (data) => {
        const stderrString = data.toString();
        if (!streamInfoSent) {
            const infoMatch = stderrString.match(/Stream #\d:\d.*: Video: (\w+)(?:\s\([^)]+\))?,.*?\s(\d{3,4}x\d{3,4})/);
            if (infoMatch && infoMatch[1] && infoMatch[2]) {
                const [_, codec, resolution] = infoMatch;
                console.log(`[FFMPEG] Parsed Stream Info for ${uniqueStreamIdentifier}: ${codec}, ${resolution}`);
                const mainWindow = require('./window-manager').getMainWindow();
                if (mainWindow) {
                    mainWindow.webContents.send('stream-info-update', { uniqueStreamIdentifier, codec, resolution });
                }
                streamInfoSent = true;
            }
        }
        if (stderrString.trim()) { lastErrorOutput = stderrString.trim(); }
        statsBuffer += stderrString;
        const statsBlocks = statsBuffer.split('progress=');
        if (statsBlocks.length > 1) {
            statsBlocks.slice(0, -1).forEach(block => {
                if (!block.trim()) return;
                const stats = Object.fromEntries(block.trim().split('\n').map(line => line.split('=').map(s => s.trim())));
                const mainWindow = require('./window-manager').getMainWindow();
                if (mainWindow && (stats.fps || stats.bitrate)) {
                    mainWindow.webContents.send('stream-stats', { uniqueStreamIdentifier, fps: parseFloat(stats.fps) || 0, bitrate: parseFloat(stats.bitrate) || 0 });
                }
            });
            statsBuffer = statsBlocks.pop();
        }
    });
    ffmpegProcess.on('close', (code, signal) => {
        console.warn(`[FFMPEG] Process ${uniqueStreamIdentifier} exited. Code: ${code}, Signal: ${signal}.`);
        if (code !== 0 && code !== null) {
            console.error(`[FFMPEG Last Stderr] ${uniqueStreamIdentifier}: ${lastErrorOutput}`);
        }
        if (streamManager[uniqueStreamIdentifier]) { 
            streamManager[uniqueStreamIdentifier].wss.close(); 
            releasePort(wsPort); 
            delete streamManager[uniqueStreamIdentifier]; 
        }
        stopProcess(uniqueStreamIdentifier);
        const mainWindow = require('./window-manager').getMainWindow();
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('stream-died', { uniqueStreamIdentifier: uniqueStreamIdentifier, error: lastErrorOutput || `Process exited with code ${code}` });
        }
    });
    streamManager[uniqueStreamIdentifier] = { wss, port: wsPort };
    return { success: true, wsPort };
}

async function stopVideoStream(uniqueStreamIdentifier) {
    if (stopProcess(uniqueStreamIdentifier)) {
        const streamData = streamManager[uniqueStreamIdentifier];
        if (streamData) {
            streamData.wss.close();
            releasePort(streamData.port);
            delete streamManager[uniqueStreamIdentifier];
        }
        return { success: true };
    }
    return { success: false, error: "Stream not found" };
}

async function startRecording(camera, type = 'manual') {
    const mainWindow = require('./window-manager').getMainWindow();
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
    if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('recording-state-change', { cameraId: camera.id, recording: true });
    }
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
        const mainWindow = require('./window-manager').getMainWindow();
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('recording-state-change', { cameraId, recording: false });
        }
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

async function prepareArchiveForHls({ filename }) {
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
    const { command, args } = builder.buildForHls(sourcePath, hlsTempPath, 0, sourceCodec);
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
            if (mainWindow && !mainWindow.isDestroyed()) {
                const channel = result.status === 'info' ? 'analytics-provider-info' : 'analytics-update';
                mainWindow.webContents.send(channel, { cameraId, result });
            }
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
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('analytics-status-change', { cameraId, active: false });
        }
    });
    if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('analytics-status-change', { cameraId, active: true });
    }
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
    toggleRecording,
    exportArchiveClip,
    toggleAnalytics,
    killAllFfmpeg,
    prepareArchiveForHls,
    getRecordingStates,
};
// --- END OF FILE src/main/process-manager.js ---