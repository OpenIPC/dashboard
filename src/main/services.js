// --- START OF FILE src/main/services.js ---
const { app, Notification, dialog } = require('electron');
const { autoUpdater } = require('electron-updater');
const path = require('path');
const { getAppSettings } = require('./config-manager');

const notificationTimestamps = {};
const NOTIFICATION_COOLDOWN = 30000; 

function handleError(error, context = 'Unknown Context') {
    const errorMessage = error instanceof Error ? error.message : String(error);
    console.error(`[Error in ${context}]`, error);
    const { getMainWindow } = require('./window-manager');
    const mainWindow = getMainWindow();
    if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('on-main-error', {
            context: context,
            message: errorMessage
        });
    }
}

async function showSystemNotification({ title, body }) {
    if (!Notification.isSupported()) return;
    const settings = await getAppSettings();
    if (!settings.notifications_enabled) return;
    new Notification({
        title,
        body,
        // START: ИСПРАВЛЕНИЕ - Обновляем путь к иконке
        icon: path.join(__dirname, '../../assets/icon.png')
        // END: ИСПРАВЛЕНИЕ
    }).show();
}

async function showAnalyticsNotification(cameraName, cameraId, objects) {
    if (!Notification.isSupported()) return;
    const settings = await getAppSettings();
    if (!settings.notifications_enabled) return;
    const now = Date.now();
    const lastTime = notificationTimestamps[cameraId];
    if (lastTime && (now - lastTime < NOTIFICATION_COOLDOWN)) {
        return;
    }
    notificationTimestamps[cameraId] = now;
    console.log(`[Notification] Showing notification for camera: ${cameraName}`);
    const notification = new Notification({
        title: `Обнаружение на камере: ${cameraName}`,
        body: `Обнаружены объекты: ${objects.join(', ')}`,
        // START: ИСПРАВЛЕНИЕ - Обновляем путь к иконке
        icon: path.join(__dirname, '../../assets/icon.png'),
        // END: ИСПРАВЛЕНИЕ
        silent: true
    });
    notification.show();
}

function getSystemStats() {
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
}

function checkForUpdates() {
    if (app.isPackaged) {
        // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЯ --- VVVVVV
        const { APP_VERSION } = require('./main.js'); // Получаем версию из главного файла
        
        // Устанавливаем имя канала в зависимости от версии.
        // Это заставит updater искать файлы latest-lite.yml или latest-intellect.yml
        autoUpdater.channel = APP_VERSION; 
        
        console.log(`[Updater] Manually checking for updates on channel: ${autoUpdater.channel}...`);
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
        autoUpdater.checkForUpdates();
    } else {
        console.log('[Updater] Skipping update check in development mode.');
    }
}

function registerUpdaterEvents(mainWindow) {
    autoUpdater.on('update-available', (info) => {
        console.log('[Updater] Update available.', info);
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('update-status', { 
                status: 'available', 
                message: `Доступна версия ${info.version}`,
                info: info
            });
        }
    });

    autoUpdater.on('update-not-available', (info) => {
        console.log('[Updater] No new update available.');
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('update-status', { status: 'latest', message: 'У вас последняя версия.' });
        }
    });

    autoUpdater.on('error', (err) => {
        console.error('[Updater] Error:', err ? (err.stack || err) : 'unknown error');
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('update-status', { status: 'error', message: `Ошибка обновления: ${err.message}` });
        }
    });

    autoUpdater.on('download-progress', (progressObj) => {
        const log_message = `Downloaded ${progressObj.percent.toFixed(2)}%`;
        console.log(`[Updater] ${log_message}`);
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('update-status', {
                status: 'downloading',
                message: `Загрузка... ${progressObj.percent.toFixed(0)}%`,
                info: progressObj
            });
        }
    });

    autoUpdater.on('update-downloaded', (info) => {
        console.log('[Updater] Update downloaded.', info);
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('update-status', { status: 'downloaded', message: `Версия ${info.version} загружена. Перезапустите для установки.` });
            dialog.showMessageBox(mainWindow, {
                type: 'info',
                title: 'Обновление готово',
                message: 'Новая версия загружена. Перезапустить приложение сейчас, чтобы установить обновление?',
                buttons: ['Перезапустить', 'Позже'],
                defaultId: 0,
                cancelId: 1
            }).then(({ response }) => {
                if (response === 0) {
                    autoUpdater.quitAndInstall();
                }
            });
        }
    });
}

module.exports = {
    handleError,
    showSystemNotification,
    showAnalyticsNotification,
    getSystemStats,
    checkForUpdates,
    registerUpdaterEvents,
};
// --- END OF FILE src/main/services.js ---