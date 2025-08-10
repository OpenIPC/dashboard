// --- ФАЙЛ: src/main/config-manager.js ---

const { app, dialog } = require('electron');
const path = require('path');
const fs = require('fs');
const fsPromises = fs.promises;
const os = require('os');
const { exec } = require('child_process');
const { Mutex } = require('async-mutex');

// Зависимости от других наших модулей
const authManager = require('./auth-manager');

const eventsMutex = new Mutex();
let appSettingsCache = null;

// --- Управление путями ---

function getDataPath() {
    // Для portable-версии данные хранятся рядом с exe
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
const eventsPath = path.join(dataPathRoot, 'events.json');
const oldCamerasPath = path.join(dataPathRoot, 'cameras.json'); // для миграции

// --- Настройки приложения ---

async function getAppSettings() {
    if (appSettingsCache) {
        return appSettingsCache;
    }
    try {
        const data = await fsPromises.readFile(appSettingsPath, 'utf-8');
        appSettingsCache = JSON.parse(data);
    } catch (e) {
        // Значения по умолчанию, если файл не найден или поврежден
        appSettingsCache = { 
            recordingsPath: path.join(app.getPath('videos'), 'OpenIPC-VMS'),
            hwAccel: 'auto',
            language: 'en',
            qscale: 8,
            fps: 20,
            analytics_record_duration: 30,
            notifications_enabled: true
        };
    }
    return appSettingsCache;
}

async function saveAppSettings(settings) {
    try {
        appSettingsCache = settings; // Обновляем кэш
        await fsPromises.writeFile(appSettingsPath, JSON.stringify(settings, null, 2));
        return { success: true };
    } catch (e) {
        console.error('Failed to save app settings:', e);
        return { success: false, error: e.message };
    }
}

// --- Основная конфигурация (камеры, группы, раскладки) ---

async function loadConfiguration() {
    const defaultConfig = { cameras: [], groups: [], layouts: [], gridState: Array(64).fill(null) };
    
    // Функция для миграции со старого формата cameras.json
    const migrateOldFile = async () => {
        try {
            await fsPromises.access(oldCamerasPath);
            console.log('Found old cameras.json, attempting migration...');
            const oldData = await fsPromises.readFile(oldCamerasPath, 'utf-8');
            const oldCameras = JSON.parse(oldData);
            return { ...defaultConfig, cameras: oldCameras };
        } catch (migrationError) {
            return null; // Файла для миграции нет
        }
    };
    
    try {
        await fsPromises.access(configPath);
        const data = await fsPromises.readFile(configPath, 'utf-8');
        let config = { ...defaultConfig, ...JSON.parse(data) };
        // Убедимся, что сетка всегда имеет правильный размер
        if (!config.gridState || config.gridState.length < 64) {
            config.gridState = Array(64).fill(null);
        }
        return config;
    } catch (e) {
        const migratedConfig = await migrateOldFile();
        if (migratedConfig) {
            await saveConfiguration(migratedConfig); // Сохраняем в новом формате
            await fsPromises.rename(oldCamerasPath, `${oldCamerasPath}.bak`);
            console.log('Migration successful and new config saved.');
            return migratedConfig;
        }
        // Если ничего нет, возвращаем дефолтный конфиг
        return defaultConfig;
    }
}

async function saveConfiguration(config) {
    try {
        const configToSave = JSON.parse(JSON.stringify(config)); // Глубокая копия
        
        // Пароли храним отдельно и безопасно
        for (const camera of configToSave.cameras) {
            if (camera.password) {
                await authManager.setPasswordForCamera(camera.id.toString(), camera.password);
                delete camera.password;
            }
        }
        await fsPromises.writeFile(configPath, JSON.stringify(configToSave, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
}

async function getCameraConfig(cameraId) {
    const config = await loadConfiguration();
    return config.cameras.find(c => c.id === cameraId);
}

// --- Пользователи ---

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

// --- События аналитики и записи ---

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

        allEvents[dateKey].push({
            cameraId: eventData.cameraId,
            timestamp: eventData.timestamp,
            objects: [...new Set(eventData.objects.map(obj => obj.label))],
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

async function getRecordingsForDate({ cameraName, date }) {
    try {
        const settings = await getAppSettings();
        const recordingsPath = settings.recordingsPath;
        await fsPromises.mkdir(recordingsPath, { recursive: true });

        const dirents = await fsPromises.readdir(recordingsPath, { withFileTypes: true });
        const saneCameraName = cameraName.replace(/[<>:"/\\|?*]/g, '_');
        const datePrefix = `${saneCameraName}-${date}`;

        return dirents
            .filter(d => d.isFile() && d.name.startsWith(datePrefix) && d.name.endsWith('.mp4'))
            .map(d => {
                const match = d.name.match(/(\d{4}-\d{2}-\d{2}T\d{2}-\d{2}-\d{2})/);
                if (!match) return null;
                
                // VVVVVV --- ИЗМЕНЕНИЕ: Возвращаем парсинг к первоначальному виду --- VVVVVV
                const [datePart, timePart] = match[1].split('T');
                const fixedTimePart = timePart.replace(/-/g, ':');
                const isoString = `${datePart}T${fixedTimePart}`;
                // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
                
                const dateObj = new Date(isoString);
                if (isNaN(dateObj.getTime())) {
                    console.warn(`[Archive] Invalid date parsed from filename: ${d.name}`);
                    return null;
                }

                return { name: d.name, startTimeString: match[1] };
            })
            .filter(Boolean)
            .sort((a, b) => a.startTimeString.localeCompare(b.startTimeString));
    } catch (e) {
        console.error('Failed to get recordings for date:', e);
        return [];
    }
}

async function getDatesWithActivity(cameraName) {
    const activeDates = new Set();
    const settings = await getAppSettings();
    const saneCameraName = cameraName.replace(/[<>:"/\\|?*]/g, '_');

    try {
        const files = await fsPromises.readdir(settings.recordingsPath);
        files.forEach(file => {
            if (file.startsWith(saneCameraName) && file.endsWith('.mp4')) {
                const match = file.match(/\d{4}-\d{2}-\d{2}/);
                if (match) activeDates.add(match[0]);
            }
        });
    } catch (e) { /* Игнорируем, если папки нет */ }

    try {
        const eventsData = await fsPromises.readFile(eventsPath, 'utf-8');
        Object.keys(JSON.parse(eventsData)).forEach(dateKey => activeDates.add(dateKey));
    } catch (e) { /* Игнорируем, если файла нет */ }
    
    return Array.from(activeDates);
}

// --- Экспорт/Импорт ---

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

// --- Локальная файловая система ---

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

// --- Интернационализация ---

async function getTranslationFile(lang) {
    try {
        const filePath = path.join(app.getAppPath(), 'locales', `${lang}.json`);
        const data = await fsPromises.readFile(filePath, 'utf-8');
        return JSON.parse(data);
    } catch (e) {
        return null;
    }
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
    getTranslationFile
};