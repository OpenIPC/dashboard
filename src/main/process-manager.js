// --- ФАЙЛ: src/main/process-manager.js ---

const { spawn, exec } = require('child_process');
const path = require('path');
const fs = require('fs');
const fsPromises = require('fs').promises;
const net = require('net');
const WebSocket = require('ws');
const { Mutex } = require('async-mutex');
const { app, dialog } = require('electron');

const si = require('systeminformation');

const configManager = require('./config-manager');
const authManager = require('./auth-manager');
const services = require('./services');
const FfmpegCommandBuilder = require('./ffmpeg-builder');

let gpuInfoCache = null;

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

const PROCESS_TYPES = { STREAM: 'stream', RECORDING: 'recording', ANALYTICS: 'analytics' };
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
            if (type === PROCESS_TYPES.RECORDING && childProcess.stdin && childProcess.stdin.writable) {
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

function getProcess(key) {
    return processes.get(key);
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

async function startVideoStream({ credentials, streamId }) {
    const uniqueStreamIdentifier = buildProcessId(PROCESS_TYPES.STREAM, `${credentials.id}_${streamId}`);
    if (getProcess(uniqueStreamIdentifier) || streamManager[uniqueStreamIdentifier]) {
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
    
    ffmpegProcess.stdout.on('data', (data) => wss.clients.forEach(c => c.readyState === WebSocket.OPEN && c.send(data)));
    
    let statsBuffer = '', lastErrorOutput = '';
    ffmpegProcess.stderr.on('data', (data) => {
        const errorString = data.toString();
        if (errorString.trim()) { lastErrorOutput = errorString.trim(); }
        statsBuffer += errorString;
        const statsBlocks = statsBuffer.split('progress=');
        if (statsBlocks.length > 1) {
            statsBlocks.slice(0, -1).forEach(block => {
                if (!block.trim()) return;
                const stats = Object.fromEntries(block.trim().split('\n').map(line => line.split('=').map(s => s.trim())));
                const mainWindow = require('./window-manager').getMainWindow();
                if (mainWindow && (stats.fps || stats.bitrate)) {
                    mainWindow.webContents.send('stream-stats', { 
                        uniqueStreamIdentifier, 
                        fps: parseFloat(stats.fps) || 0, 
                        bitrate: parseFloat(stats.bitrate) || 0 
                    });
                }
            });
            statsBuffer = statsBlocks.pop();
        }
    });

    ffmpegProcess.on('close', (code) => {
        console.warn(`[FFMPEG] Process ${uniqueStreamIdentifier} exited with code ${code}.`);
        if (code !== 0) console.error(`[FFMPEG Last Stderr] ${uniqueStreamIdentifier}: ${lastErrorOutput}`);
        
        if (streamManager[uniqueStreamIdentifier]) { 
            streamManager[uniqueStreamIdentifier].wss.close(); 
            releasePort(wsPort); 
            delete streamManager[uniqueStreamIdentifier]; 
        }
        
        stopProcess(uniqueStreamIdentifier);
        const mainWindow = require('./window-manager').getMainWindow();
        if (mainWindow && !mainWindow.isDestroyed()) mainWindow.webContents.send('stream-died', uniqueStreamIdentifier);
    });
    
    streamManager[uniqueStreamIdentifier] = { wss, port: wsPort };
    return { success: true, wsPort };
}

async function stopVideoStream(uniqueStreamIdentifier) {
    if (stopProcess(uniqueStreamIdentifier)) {
        return { success: true };
    }
    return { success: false, error: "Stream not found" };
}

async function startRecording(camera, mainWindow) {
    if (!camera || !camera.id) return { success: false, error: 'Invalid camera data' };
    const recordingId = buildProcessId(PROCESS_TYPES.RECORDING, camera.id);
    if (getProcess(recordingId) || recordingManager[camera.id]) {
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
    
    const ffmpegProcess = spawn(command, ffmpegArgs, { detached: false, windowsHide: true });
    addProcess(recordingId, ffmpegProcess, PROCESS_TYPES.RECORDING);
    recordingManager[camera.id] = { path: outputPath };
    
    services.showSystemNotification({ title: 'Запись начата', body: `Камера: "${fullCameraInfo.name}"` });
    
    ffmpegProcess.on('close', (code) => {
        delete recordingManager[camera.id];
        stopProcess(recordingId);
        if (recordingStopTimers[camera.id]) clearTimeout(recordingStopTimers[camera.id]);
        
        if (code !== 0) {
            services.showSystemNotification({ title: 'Ошибка записи', body: `Камера: "${fullCameraInfo.name}"` });
        } else {
            services.showSystemNotification({ title: 'Запись завершена', body: `Файл сохранен для камеры "${fullCameraInfo.name}"` });
        }
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('recording-state-change', { cameraId: camera.id, recording: false });
        }
    });

    if (mainWindow && !mainWindow.isDestroyed()) mainWindow.webContents.send('recording-state-change', { cameraId: camera.id, recording: true });
    return { success: true };
}

function stopRecording(cameraId) {
    if (recordingStopTimers[cameraId]) {
        clearTimeout(recordingStopTimers[cameraId]);
        delete recordingStopTimers[cameraId];
    }
    const recordingId = buildProcessId(PROCESS_TYPES.RECORDING, cameraId);
    if (stopProcess(recordingId)) return { success: true };
    return { success: false, error: 'Recording not found' };
}

async function exportArchiveClip({ sourceFilename, startTime, duration }, mainWindow) {
    const settings = await configManager.getAppSettings();
    const sourcePath = path.join(settings.recordingsPath, sourceFilename);

    try {
        await fsPromises.access(sourcePath);
    } catch (e) {
        return { success: false, error: `Source file not found: ${sourceFilename}` };
    }

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

async function handleAnalyticsDetection(cameraId, camera) {
    const settings = await configManager.getAppSettings();
    const autoStopDelay = (settings.analytics_record_duration || 30) * 1000;

    if (recordingStopTimers[cameraId]) clearTimeout(recordingStopTimers[cameraId]);

    if (!recordingManager[cameraId]) {
        console.log(`[Analytics] Object detected on camera ${cameraId}, starting recording.`);
        await startRecording(camera, require('./window-manager').getMainWindow());
    }

    recordingStopTimers[cameraId] = setTimeout(() => {
        console.log(`[REC] Auto-stopping recording for camera ${cameraId} due to inactivity.`);
        stopRecording(cameraId);
    }, autoStopDelay);
}

// VVVVVV --- ИЗМЕНЕНИЕ: Логика выбора EXE файла упрощена --- VVVVVV
/**
 * Определяет, какой исполняемый файл аналитики использовать.
 * @returns {string} Путь к исполняемому файлу.
 */
function getAnalyticsExecutablePath() {
    const platform = process.platform;
    let exeName = 'analytics_cpu'; // По умолчанию для Linux, macOS

    if (platform === 'win32') {
        // Для Windows по умолчанию используем DirectML, так как он более универсален
        exeName = 'analytics_dml';
        console.log('[Analytics] Windows system detected. Selecting DirectML executable.');
    } else {
        console.log(`[Analytics] ${platform} system detected. Selecting CPU executable.`);
    }

    if (platform === 'win32') {
        exeName += '.exe';
    }
    
    return app.isPackaged
        ? path.join(process.resourcesPath, 'analytics', exeName)
        : path.join(__dirname, '../../extra/analytics', exeName);
}
// ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^


async function toggleAnalytics(cameraId, mainWindow) {
    const analyticsId = buildProcessId(PROCESS_TYPES.ANALYTICS, cameraId);
    if (getProcess(analyticsId)) {
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
    
    const analyticsPath = getAnalyticsExecutablePath();
    
    if (!fs.existsSync(analyticsPath)) {
        const errorMsg = `Analytics executable not found: ${analyticsPath}`;
        dialog.showErrorBox('Ошибка аналитики', errorMsg);
        return { success: false, error: errorMsg };
    }
    
    const configForScript = {
        objects: camera.analyticsConfig?.objects || [],
        resize_width: settings.analytics_resize_width || 416,
        frame_skip: settings.analytics_frame_skip || 10,
    };
    const configArg = Buffer.from(JSON.stringify(configForScript)).toString('base64');
    
    // VVVVVV --- ИЗМЕНЕНИЕ: Передаем выбор провайдера как 3-й аргумент --- VVVVVV
    const providerChoice = settings.analytics_provider || 'auto'; // 'auto', 'dml', 'cpu'
    console.log(`[Analytics] Starting with provider choice: ${providerChoice}`);
    const analyticsProcess = spawn(analyticsPath, [rtspUrl, configArg, providerChoice], { windowsHide: true });
    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
    
    addProcess(analyticsId, analyticsProcess, PROCESS_TYPES.ANALYTICS);

    analyticsProcess.stdout.on('data', async (data) => {
        data.toString().split('\n').filter(Boolean).forEach(async line => {
            try {
                const result = JSON.parse(line);
                if (result.status === 'objects_detected' && result.objects.length > 0) {
                    await configManager.saveAnalyticsEvent({cameraId, ...result});
                    const labels = [...new Set(result.objects.map(o => o.label))];
                    services.showAnalyticsNotification(camera.name, cameraId, labels);
                    await handleAnalyticsDetection(cameraId, camera);
                }
                if (mainWindow && !mainWindow.isDestroyed()) {
                    const channel = result.status === 'info' ? 'analytics-provider-info' : 'analytics-update';
                    const payload = { cameraId, result: result };
                    mainWindow.webContents.send(channel, payload);
                }
            } catch (e) {
                console.warn(`[Analytics] Non-JSON output from script for camera ${cameraId}:`, line);
            }
        });
    });

    analyticsProcess.on('close', (code) => {
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
    stopAllProcesses(); 
    const builder = new FfmpegCommandBuilder({});
    const ffmpegProcessName = path.basename(builder.command);
    const command = process.platform === 'win32' ? `taskkill /IM ${ffmpegProcessName} /F` : `pkill -f ${ffmpegProcessName}`;
    return new Promise(resolve => {
        exec(command, () => {
            Object.values(streamManager).forEach(s => s.wss?.close());
            usedPorts.clear();
            Object.keys(streamManager).forEach(k => delete streamManager[k]);
            Object.keys(recordingManager).forEach(k => delete recordingManager[k]);
            resolve({ success: true, message: "Все потоки сброшены." });
        });
    });
}

module.exports = {
    addProcess,
    stopProcess,
    getProcess,
    getAllProcessesOfType,
    stopAllProcesses,
    startVideoStream,
    stopVideoStream,
    startRecording,
    stopRecording,
    exportArchiveClip,
    toggleAnalytics,
    killAllFfmpeg
};