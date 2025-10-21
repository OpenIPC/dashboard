// --- START OF FILE src/main/config-manager.js ---
const { app, dialog } = require('electron');
const path = require('path');
const fs = require('fs');
const fsPromises = fs.promises;
const os = require('os');
const { exec } = require('child_process');
const { Mutex } = require('async-mutex');
const ffmpeg = require('fluent-ffmpeg');

try {
    const ffmpegInstaller = require('@ffmpeg-installer/ffmpeg');
    const ffprobeInstaller = require('@ffprobe-installer/ffprobe');
    ffmpeg.setFfmpegPath(ffmpegInstaller.path.replace('app.asar', 'app.asar.unpacked'));
    ffmpeg.setFfprobePath(ffprobeInstaller.path.replace('app.asar', 'app.asar.unpacked'));
} catch (e) {
    // Use system ffmpeg if bundled not available
    console.log('Bundled ffmpeg not available, using system ffmpeg');
}

const authManager = require('./auth-manager');

const eventsMutex = new Mutex();
let appSettingsCache = null;

function getDataPath() {
    if (process.env.PORTABLE_EXECUTABLE_DIR) {
        return process.env.PORTABLE_EXECUTABLE_DIR;
    }
    // Если Electron app недоступен, используем текущую рабочую директорию
    try {
        if (app && typeof app.getPath === 'function') {
            return app.getPath('userData');
        }
    } catch {}
    return process.cwd();
}

// allow other modules to get the application data path
exports.getDataPath = getDataPath;

const dataPathRoot = getDataPath();
console.log(`[Config] Data path is: ${dataPathRoot}`);
const configPath = path.join(dataPathRoot, 'config.json');
const appSettingsPath = path.join(dataPathRoot, 'app-settings.json');
const usersPath = path.join(dataPathRoot, 'users.json');
const eventsPath = path.join(dataPathRoot, 'events.json');
const oldCamerasPath = path.join(dataPathRoot, 'cameras.json');

async function getAppSettings() {
    if (appSettingsCache) {
        return appSettingsCache;
    }
    try {
        const data = await fsPromises.readFile(appSettingsPath, 'utf-8');
        try {
            appSettingsCache = JSON.parse(data);
        } catch (parseError) {
            console.error('[Config] Failed to parse app-settings.json, using defaults.', parseError);
            appSettingsCache = {};
        }
    } catch (e) {
        appSettingsCache = {};
    }
    
    const recordingsDefaultPath = path.join(app.getPath('videos'), 'OpenIPC-VMS');
    const defaults = { 
        recordingsPath: recordingsDefaultPath,
        screenshotsPath: path.join(recordingsDefaultPath, 'Screenshots'),
        hwAccel: 'auto',
        language: 'en',
        qscale: 8,
        fps: 20,
        analytics_record_duration: 30,
        notifications_enabled: true,
        analytics_provider: 'auto',
        useWebRTC: false  // New: Enable WebRTC for low-latency streaming
    };

    appSettingsCache = { ...defaults, ...appSettingsCache };
    return appSettingsCache;
}

async function saveAppSettings(settings) {
    try {
        // Load current settings (from cache or disk) and merge shallowly to avoid accidental overwrites
        let current = {};
        try {
            current = await getAppSettings();
        } catch (e) {
            console.warn('[Config] Could not load existing app settings for merge, proceeding with provided settings.', e && e.message);
            current = {};
        }

        const merged = { ...current, ...settings };

        // Safe stringify: remove functions, symbols and handle cycles gracefully
        const safeStringify = (obj) => {
            const seen = new WeakSet();
            return JSON.stringify(obj, (key, value) => {
                if (typeof value === 'function') return undefined;
                if (typeof value === 'symbol') return undefined;
                if (typeof value === 'undefined') return null;
                if (value && typeof value === 'object') {
                    if (seen.has(value)) return undefined;
                    seen.add(value);
                }
                return value;
            }, 2);
        };

        let payload;
        try {
            payload = safeStringify(merged);
        } catch (serr) {
            console.error('[Config] Failed to serialize merged app settings:', serr);
            return { success: false, error: 'Serialization error: ' + (serr && serr.message) };
        }

        // Atomic write: write to temp file then rename
        const tmpPath = `${appSettingsPath}.tmp`;
        await fsPromises.writeFile(tmpPath, payload, 'utf-8');
        await fsPromises.rename(tmpPath, appSettingsPath);

        appSettingsCache = merged;
        console.log('[Config] app-settings.json saved to', appSettingsPath, 'keys=', Object.keys(merged));
        return { success: true };
    } catch (e) {
        console.error('Failed to save app settings:', e);
        return { success: false, error: e && e.message };
    }
}

async function loadConfiguration() {
    const defaultConfig = { cameras: [], groups: [], layouts: [], gridState: Array(64).fill(null) };
    
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
        let config;
        try {
            config = { ...defaultConfig, ...JSON.parse(data) };
        } catch (parseError) {
            console.error('[Config] Failed to parse config.json, using default config.', parseError);
            return defaultConfig;
        }

        // Log loaded config (mask passwords if present)
        try {
            const safeConfig = JSON.parse(JSON.stringify(config));
            if (Array.isArray(safeConfig.cameras)) {
                safeConfig.cameras = safeConfig.cameras.map(c => {
                    const copy = { ...c };
                    if (copy.password) copy.password = '***masked***';
                    return copy;
                });
            }
            console.log('[ConfigManager][Load] Loaded config from', configPath, JSON.stringify(safeConfig, null, 2));
        } catch (logErr) {
            console.error('[ConfigManager][Load] Failed to stringify loaded config for logging:', logErr);
        }
        
        if (!config.gridState || config.gridState.length < 64) {
            config.gridState = Array(64).fill(null);
        }
        return config;
    } catch (e) {
        const migratedConfig = await migrateOldFile();
        if (migratedConfig) {
            await saveConfiguration(migratedConfig, { origin: 'migration:oldCameras' });
            await fsPromises.rename(oldCamerasPath, `${oldCamerasPath}.bak`);
            console.log('Migration successful and new config saved.');
            return migratedConfig;
        }
        return defaultConfig;
    }
}

async function saveConfiguration(config, meta = {}) {
    try {
        const configToSave = JSON.parse(JSON.stringify(config));
        // Log config being saved (mask passwords)
        try {
            const safeConfig = JSON.parse(JSON.stringify(configToSave));
            if (Array.isArray(safeConfig.cameras)) {
                safeConfig.cameras = safeConfig.cameras.map(c => {
                    const copy = { ...c };
                    if (copy.password) copy.password = '***masked***';
                    return copy;
                });
            }
            console.log('[ConfigManager][Save] Writing config to', configPath, 'meta=', JSON.stringify(meta || {}), JSON.stringify(safeConfig, null, 2));
        } catch (logErr) {
            console.error('[ConfigManager][Save] Failed to stringify config for logging:', logErr);
        }
        for (const camera of configToSave.cameras) {
            if (camera.password) {
                await authManager.setPasswordForCamera(camera.id.toString(), camera.password);
                delete camera.password;
            }
        }
        // Atomic write: write to temp file then rename
        const tmpPath = `${configPath}.tmp`;
        await fsPromises.writeFile(tmpPath, JSON.stringify(configToSave, null, 2));
        await fsPromises.rename(tmpPath, configPath);

        // Автоматически обновляем конфиг MediaMTX после сохранения основной конфигурации

        try {
            const processManager = require('./process-manager');
            // Если MediaMTX уже запущен — попробуем обновить пути через API (hot-update),
            // иначе сгенерируем конфиг и перезапустим процесс как раньше.
            if (processManager.isMediaMTXRunning && processManager.isMediaMTXRunning()) {
                // update persistent masked config but do not write runtime plaintext file
                await processManager.generateAndSaveMediaMTXConfig({ writeRuntime: false });
                // Hot-update running MediaMTX via API
                await processManager.updateMediaMTXPaths();
            } else {
                await processManager.generateAndSaveMediaMTXConfig({ writeRuntime: true });
                // Перезапуск MediaMTX для применения нового конфига
                processManager.stopMediaMTX();
                setTimeout(() => {
                    processManager.startMediaMTX();
                }, 1000); // небольшая пауза для корректного завершения
            }
        } catch (err) {
            console.error('[ConfigManager] Не удалось обновить/перезапустить MediaMTX:', err);
        }

        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
}

async function getCameraConfig(cameraId) {
    const config = await loadConfiguration();
    return config.cameras.find(c => c.id === cameraId);
}

async function initializeUsers() {
    try {
        await fsPromises.access(usersPath);
    } catch (e) {
        console.log('[Users] users.json not found, creating default admin user (admin/admin).');
        const { salt, hash } = authManager.hashPassword('admin');
        const defaultUser = [{
            username: 'admin',
            hashedPassword: hash,
            salt: salt,
            role: 'admin'
        }];
        await fsPromises.writeFile(usersPath, JSON.stringify(defaultUser, null, 2));
    }
}

async function saveAnalyticsEvent(eventData) {
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
        
        const settings = await getAppSettings();
        const duration = settings.analytics_record_duration || 30;

        allEvents[dateKey].push({
            cameraId: eventData.cameraId,
            timestamp: eventData.timestamp,
            objects: [...new Set(eventData.objects.map(obj => obj.label))],
            duration: duration
        });

        await fsPromises.writeFile(eventsPath, JSON.stringify(allEvents, null, 2));
    } catch (e) {
        console.error('[Events] Failed to save analytics event:', e);
    } finally {
        release();
    }
}

async function getEventsForDate({ date }) {
    try {
        const data = await fsPromises.readFile(eventsPath, 'utf-8');
        const allEvents = JSON.parse(data);
        return allEvents[date] || [];
    } catch (e) {
        if (e.code !== 'ENOENT') console.error('[Events] Error reading events for date:', e);
        return [];
    }
}

async function getRecordingsForDate({ cameraId, cameraName, date }) {
    try {
        const settings = await getAppSettings();
        const recordingsPath = settings.recordingsPath;
        const { cameras } = await loadConfiguration();
        let camera = null;
        if (typeof cameraId !== 'undefined' && cameraId !== null) {
            camera = cameras.find(c => c.id === Number(cameraId));
        } else if (cameraName) {
            camera = cameras.find(c => c.name === cameraName);
        }
        if (!camera) {
            console.warn(`[Archive] Camera not found (cameraId=${cameraId}, cameraName=${cameraName}).`);
            return [];
        }

        const streamName = `cam${camera.id}_0`;
        const cameraRecordingsPath = path.join(recordingsPath, streamName);
        
        if (!fs.existsSync(cameraRecordingsPath)) {
            return [];
        }

        const dirents = await fsPromises.readdir(cameraRecordingsPath, { withFileTypes: true });
        
        const datePrefix = `${date}`;

        const filePromises = dirents
            .filter(d => d.isFile() && d.name.startsWith(datePrefix) && d.name.endsWith('.mp4'))
            .map(d => new Promise((resolve) => {
                const filePath = path.join(cameraRecordingsPath, d.name);
                ffmpeg.ffprobe(filePath, (err, metadata) => {
                    if (err) {
                        console.error(`ffprobe error for ${d.name}:`, err.message);
                        return resolve(null);
                    }
                    
                    const match = d.name.match(/(\d{4}-\d{2}-\d{2})_(\d{2})-(\d{2})-(\d{2})/);
                    if (!match) return resolve(null);
                    // Build ISO-like string: YYYY-MM-DDTHH:MM:SS
                    const datePart = match[1];
                    const hh = match[2];
                    const mm = match[3];
                    const ss = match[4];
                    const startTimeString = `${datePart}T${hh}:${mm}:${ss}`;

                    // Normalize returned name to use forward slashes so renderer can treat it as a URL-like path
                    const normalizedName = `${streamName}/${d.name}`.replace(/\\/g, '/');
                    resolve({
                        name: normalizedName,
                        startTimeString: startTimeString,
                        duration: metadata.format.duration || 0
                    });
                });
            }));
        
        const results = await Promise.all(filePromises);

        return results
            .filter(Boolean)
            .sort((a, b) => a.startTimeString.localeCompare(b.startTimeString));

    } catch (e) {
        console.error('Failed to get recordings for date:', e);
        return [];
    }
}


async function getDatesWithActivity(cameraIdOrName) {
    const activeDates = new Set();
    const settings = await getAppSettings();
    const { cameras } = await loadConfiguration();
    let currentCamera = null;
    if (typeof cameraIdOrName === 'number' || String(Number(cameraIdOrName)) === String(cameraIdOrName)) {
        currentCamera = cameras.find(c => c.id === Number(cameraIdOrName));
    } else {
        currentCamera = cameras.find(c => c.name === cameraIdOrName);
    }

    if (currentCamera) {
        const streamName = `cam${currentCamera.id}_0`;
        const cameraRecordingsPath = path.join(settings.recordingsPath, streamName);

        try {
            if (fs.existsSync(cameraRecordingsPath)) {
                const files = await fsPromises.readdir(cameraRecordingsPath);
                files.forEach(file => {
                    if (file.endsWith('.mp4')) {
                        const match = file.match(/^\d{4}-\d{2}-\d{2}/);
                        if (match) activeDates.add(match[0]);
                    }
                });
            }
        } catch (e) {
            console.error(`[Archive] Error reading activity for ${cameraName}:`, e.message);
        }
    }

    try {
        const eventsData = await fsPromises.readFile(eventsPath, 'utf-8');
        const allEventsByDate = JSON.parse(eventsData);
        if (currentCamera) {
            for (const dateKey in allEventsByDate) {
                if (allEventsByDate[dateKey].some(event => event.cameraId === currentCamera.id)) {
                     activeDates.add(dateKey);
                }
            }
        }
    } catch (e) { /* Игнорируем, если файла нет */ }
    
    return Array.from(activeDates);
}

async function exportConfig(mainWindow) {
    try {
        const { canceled, filePath } = await dialog.showSaveDialog(mainWindow, {
            title: 'Экспорт конфигурации',
            defaultPath: `dashboard-backup-${new Date().toISOString().slice(0, 10)}.json`,
            filters: [{ name: 'JSON Files', extensions: ['json'] }]
        });
        if (canceled || !filePath) return { success: false };

        const [config, appSettings, users] = await Promise.all([
            loadConfiguration(), getAppSettings(), authManager.getUsers()
        ]);
        
        await fsPromises.writeFile(filePath, JSON.stringify({ config, appSettings, users: users.users }, null, 2));
        dialog.showMessageBox(mainWindow, { type: 'info', title: 'Экспорт успешен', message: `Конфигурация сохранена в:\n${filePath}` });
        return { success: true };
    } catch (e) {
        dialog.showErrorBox('Ошибка экспорта', e.message);
        return { success: false, error: e.message };
    }
}

async function importConfig(mainWindow) {
    try {
        const { canceled, filePaths } = await dialog.showOpenDialog(mainWindow, {
            title: 'Импорт конфигурации', properties: ['openFile'], filters: [{ name: 'JSON Files', extensions: ['json'] }]
        });
        if (canceled || !filePaths.length) return { success: false };

        const backupData = JSON.parse(await fsPromises.readFile(filePaths[0], 'utf-8'));
        if (!backupData.config || !backupData.appSettings || !backupData.users) {
            throw new Error('Неверный формат файла резервной копии.');
        }
        
        const { response } = await dialog.showMessageBox(mainWindow, {
            type: 'warning', title: 'Подтверждение импорта',
            message: 'Вы уверены, что хотите импортировать конфигурацию?',
            detail: 'Все текущие данные будут заменены. Это действие необратимо.',
            buttons: ['Импортировать', 'Отмена'], defaultId: 1, cancelId: 1
        });
        if (response !== 0) return { success: false };

        await fsPromises.writeFile(configPath, JSON.stringify(backupData.config, null, 2));
        await fsPromises.writeFile(appSettingsPath, JSON.stringify(backupData.appSettings, null, 2));
        await fsPromises.writeFile(usersPath, JSON.stringify(backupData.users, null, 2));
        
        appSettingsCache = null;
        dialog.showMessageBox(mainWindow, { type: 'info', title: 'Импорт успешен', message: 'Приложение будет перезагружено.' })
            .then(() => { app.relaunch(); app.quit(); });
        return { success: true };
    } catch (e) {
        dialog.showErrorBox('Ошибка импорта', e.message);
        return { success: false, error: e.message };
    }
}

async function getLocalDiskList() {
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
}

async function listLocalFiles(dirPath) {
    try {
        const items = await fsPromises.readdir(dirPath, { withFileTypes: true });
        return items.map(item => {
            try {
                const stats = fs.statSync(path.join(dirPath, item.name));
                return { name: item.name, isDirectory: item.isDirectory(), size: stats.size };
            } catch { return null; }
        }).filter(Boolean);
    } catch (e) {
        console.error(`Error listing local dir ${dirPath}:`, e);
        return [];
    }
}

async function getTranslationFile(lang) {
    try {
        const filePath = path.join(app.getAppPath(), 'locales', `${lang}.json`);
        const data = await fsPromises.readFile(filePath, 'utf-8');
        return JSON.parse(data);
    } catch (e) {
        return null;
    }
}

async function getArchiveVideoInfo(filename) {
    const settings = await getAppSettings();
    const filePath = path.join(settings.recordingsPath, filename);

    return new Promise((resolve) => {
        ffmpeg.ffprobe(filePath, (err, metadata) => {
            if (err) {
                console.error(`[ffprobe] Error getting info for ${filename}:`, err.message);
                return resolve(null);
            }
            const videoStream = metadata.streams.find(s => s.codec_type === 'video');
            resolve(videoStream || null);
        });
    });
}


module.exports = {
    getDataPath,
    getAppSettings,
    saveAppSettings,
    loadConfiguration,
    saveConfiguration,
    getCameraConfig,
    initializeUsers,
    saveAnalyticsEvent,
    getEventsForDate,
    getRecordingsForDate,
    getDatesWithActivity,
    exportConfig,
    importConfig,
    getLocalDiskList,
    listLocalFiles,
    getTranslationFile,
    getArchiveVideoInfo
};