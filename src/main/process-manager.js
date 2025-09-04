// --- ФАЙЛ: src/main/process-manager.js ---

const { spawn, exec } = require('child_process');
const path = require('path');
const fs = require('fs');
const fsPromises = require('fs').promises;
const { app, dialog } = require('electron');
const yaml = require('js-yaml');
const axios = require('axios');

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

const PROCESS_TYPES = { RECORDING: 'recording', ANALYTICS: 'analytics', HLS: 'hls' };
const processes = new Map();
const recordingManager = {};
const recordingStopTimers = {};
let mediamtxProcess = null;

async function generateAndSaveMediaMTXConfig() {
    const { cameras } = await configManager.loadConfiguration();
    const settings = await configManager.getAppSettings();
    const paths = {};

    for (const camera of cameras) {
        const password = await authManager.getPasswordForCamera(camera.id);
        const fullCredentials = { ...camera, password };
        const builder = new FfmpegCommandBuilder({});
        
        const onDemandConfig = {
            sourceOnDemand: true,
            sourceOnDemandStartTimeout: '15s',
            sourceOnDemandCloseAfter: '60s',
            rtspTransport: 'tcp',
        };

        paths[`cam${camera.id}_0`] = {
            source: builder.buildRtspUrl(fullCredentials, fullCredentials.streamPath0 || '/stream0'),
            ...onDemandConfig
        };
        
        paths[`cam${camera.id}_1`] = {
            source: builder.buildRtspUrl(fullCredentials, fullCredentials.streamPath1 || '/stream1'),
            ...onDemandConfig
        };
    }

    const recordPath = path.join(settings.recordingsPath, '%path', '%Y-%m-%d_%H-%M-%S-%f').replace(/\\/g, '/');
    const webhookUrl = 'http://127.0.0.1:8080/mediamtx-webhook';

    const config = {
        rtmp: false,
        hls: true,
        webrtc: true,
        api: true,
        apiAddress: ':9997',
        webrtcAddress: '127.0.0.1:8889',
        hlsAddress: ':8888',
        
        externalAuthenticationURL: webhookUrl,

        pathDefaults: {
            record: false,
            recordPath: recordPath,
            recordFormat: 'fmp4',
            recordSegmentDuration: '1h'
        },

        paths: paths
    };

    const configPath = path.join(app.getPath('userData'), 'mediamtx.yml');
    try {
        await fs.promises.writeFile(configPath, yaml.dump(config, { noRefs: true, lineWidth: -1 }));
        console.log(`[MediaMTX] Config generated and saved to ${configPath}`);
        return configPath;
    } catch (e) {
        console.error('[MediaMTX] Failed to write config file:', e);
        return null;
    }
}

async function startMediaMTX() {
    if (mediamtxProcess) {
        console.log('[MediaMTX] Process is already running.');
        return;
    }

    const configPath = await generateAndSaveMediaMTXConfig();
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

    mediamtxProcess.stdout.on('data', (data) => {
        console.log(`[MediaMTX stdout]: ${data.toString().trim()}`);
            // ТЕСТОВАЯ ОТПРАВКА СТАТИСТИКИ В РЕНДЕРЕР
            try {
                const statsData = { test: true, message: 'Тестовая статистика MediaMTX', raw: data.toString().trim() };
                broadcastToRenderers('mediamtx-stats-update', statsData);
            } catch (e) { /* ignore */ }
        });

    mediamtxProcess.stderr.on('data', (data) => {
        console.error(`[MediaMTX stderr]: ${data.toString().trim()}`);
    });

    mediamtxProcess.on('close', (code) => {
        console.warn(`[MediaMTX] Process exited with code ${code}`);
        mediamtxProcess = null;
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
        await axios.patch(`http://127.0.0.1:9997/v3/config/paths/patch/${pathName}`, { record: true });
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
        await axios.patch(`http://127.0.0.1:9997/v3/config/paths/patch/${pathName}`, { record: false });
        const { type } = recordingManager[cameraId];
        delete recordingManager[cameraId];
        console.log(`[REC] Stopped ${type} recording for ${pathName}.`);
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

async function toggleAnalytics(cameraId, mainWindow, moduleManager) {
    const analyticsId = `analytics-${cameraId}`;
    if (processes.has(analyticsId)) {
        stopProcess(analyticsId);
        broadcastToRenderers('analytics-status-change', { cameraId, active: false });
        return { success: true, status: 'stopped' };
    }

    // --- НАЧАЛО ИСПРАВЛЕНИЙ ---

    // 1. Определяем базовый путь к папке с файлами аналитики
    const analyticsBasePath = app.isPackaged
      // Для УСТАНОВЛЕННОГО приложения: путь будет [папка_приложения]/resources/analytics
      ? path.join(process.resourcesPath, 'analytics')
      // Для РАЗРАБОТКИ: путь будет [корень_проекта]/extra/analytics
      : path.join(app.getAppPath(), 'extra', 'analytics');

    // 2. Формируем полный и корректный путь к нужному файлу
    const analyticsExecutableName = process.platform === 'win32' ? 'analytics_dml.exe' : 'analytics_cpu';
    const analyticsPath = path.join(analyticsBasePath, analyticsExecutableName);

    // --- КОНЕЦ ИСПРАВЛЕНИЙ ---

    if (!fs.existsSync(analyticsPath)) {
        const errorMsg = `Analytics executable not found at path: ${analyticsPath}`;
        console.error(`[Analytics ERROR] ${errorMsg}`);
        dialog.showErrorBox('Ошибка запуска аналитики', errorMsg);
        return { success: false, error: errorMsg };
    }

    const settings = await configManager.getAppSettings();
    const camera = await configManager.getCameraConfig(cameraId);
    if (!camera) return { success: false, error: 'Camera not found' };

    const mediaMtxStreamName = `cam${camera.id}_0`;
    const rtspUrl = `rtsp://127.0.0.1:8554/${mediaMtxStreamName}`;

    const configForScript = {
        objects: camera.analyticsConfig?.objects || ['person', 'car'],
        confidence: camera.analyticsConfig?.confidence || 0.5,
        frame_skip: parseInt(settings.analytics_frame_skip, 10) || 5,
        resize_width: parseInt(settings.analytics_resize_width, 10) || 640,
    };

    const configArg = Buffer.from(JSON.stringify(configForScript)).toString('base64');
    const providerChoice = settings.analytics_provider || 'auto';

    console.log('--- [Analytics DEBUG] ---');
    console.log(`[Analytics DEBUG] Starting analytics for camera ID: ${cameraId}`);
    console.log(`[Analytics DEBUG] Executable path: ${analyticsPath}`);
    console.log(`[Analytics DEBUG] Connecting to LOCAL MediaMTX stream URL: ${rtspUrl}`);
    console.log(`[Analytics DEBUG] Config object being sent:`, configForScript);
    console.log(`[Analytics DEBUG] Provider choice: ${providerChoice}`);
    console.log('-------------------------');

    const analyticsProcess = spawn(analyticsPath, [rtspUrl, configArg, providerChoice], { windowsHide: true });
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
                } catch (readError) {
                    console.error(`[Analytics] Failed to read temp frame file: ${tempImagePath}`, readError);
                }
            }


            const listeners = moduleManager.getListeners('analytics-update');
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
    stopMediaMTX,
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
    getAnalyticsStates
};