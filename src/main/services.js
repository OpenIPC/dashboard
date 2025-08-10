// --- ФАЙЛ: src/main/services.js ---

const { app, Notification, dialog } = require('electron');
const { autoUpdater } = require('electron-updater');
const path = require('path');

// VVV ИЗМЕНЕНИЕ: УДАЛЯЕМ эту строку, чтобы разорвать цикл VVV
// const { getMainWindow } = require('./window-manager'); 

// Прямой require, так как configManager не зависит от services,
// и циклической зависимости не возникнет.
const { getAppSettings } = require('./config-manager');

const notificationTimestamps = {};
const NOTIFICATION_COOLDOWN = 30000; // 30 секунд

/**
 * Централизованный обработчик ошибок.
 * Логирует полную ошибку в консоль main-процесса и отправляет
 * упрощенное сообщение в renderer для показа пользователю.
 * @param {Error} error - Объект ошибки.
 * @param {string} context - Контекст, в котором произошла ошибка (например, 'login-handler').
 */
function handleError(error, context = 'Unknown Context') {
    const errorMessage = error instanceof Error ? error.message : String(error);
    console.error(`[Error in ${context}]`, error);

    // VVV ИЗМЕНЕНИЕ: Получаем getMainWindow здесь, а не через import VVV
    const { getMainWindow } = require('./window-manager');
    const mainWindow = getMainWindow();
    if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('on-main-error', {
            context: context,
            message: errorMessage
        });
    }
}


/**
 * Показывает системное уведомление.
 * @param {object} param0
 * @param {string} param0.title - Заголовок уведомления.
 * @param {string} param0.body - Текст уведомления.
 */
async function showSystemNotification({ title, body }) {
    if (!Notification.isSupported()) return;

    const settings = await getAppSettings();
    if (!settings.notifications_enabled) return;

    new Notification({
        title,
        body,
        icon: path.join(__dirname, '../../build/icon.png')
    }).show();
}

/**
 * Показывает уведомление о событии видеоаналитики с защитой от спама.
 * @param {string} cameraName - Имя камеры.
 * @param {number} cameraId - ID камеры.
 * @param {string[]} objects - Массив с названиями обнаруженных объектов.
 */
async function showAnalyticsNotification(cameraName, cameraId, objects) {
    if (!Notification.isSupported()) return;

    const settings = await getAppSettings();
    if (!settings.notifications_enabled) return;

    const now = Date.now();
    const lastTime = notificationTimestamps[cameraId];

    if (lastTime && (now - lastTime < NOTIFICATION_COOLDOWN)) {
        return; // Слишком часто, пропускаем
    }

    notificationTimestamps[cameraId] = now;
    
    console.log(`[Notification] Showing notification for camera: ${cameraName}`);

    const notification = new Notification({
        title: `Обнаружение на камере: ${cameraName}`,
        body: `Обнаружены объекты: ${objects.join(', ')}`,
        icon: path.join(__dirname, '../../build/icon.png'),
        silent: true // Не проигрывать системный звук для частых событий
    });

    notification.show();
}

/**
 * Собирает и возвращает статистику по использованию CPU и RAM.
 * @returns {{cpu: string, ram: string}}
 */
function getSystemStats() {
    const metrics = app.getAppMetrics();
    let totalCpuUsage = 0;
    let totalRamUsage = 0;

    metrics.forEach(metric => {
        totalCpuUsage += metric.cpu.percentCPUUsage;
        totalRamUsage += metric.memory.workingSetSize; // в килобайтах
    });

    return {
        cpu: totalCpuUsage.toFixed(0),
        ram: (totalRamUsage / 1024).toFixed(0), // в мегабайтах
    };
}

/**
 * Запускает проверку обновлений.
 */
function checkForUpdates() {
    // Проверяем только для упакованного приложения
    if (app.isPackaged) {
        console.log('[Updater] Manually checking for updates...');
        autoUpdater.checkForUpdates();
    } else {
        console.log('[Updater] Skipping update check in development mode.');
    }
}

/**
 * Регистрирует обработчики событий для autoUpdater.
 * @param {BrowserWindow} mainWindow - Главное окно приложения для отправки сообщений.
 */
function registerUpdaterEvents(mainWindow) {
    autoUpdater.on('update-available', (info) => {
        console.log('[Updater] Update available.', info);
        if (mainWindow && !mainWindow.isDestroyed()) {
            mainWindow.webContents.send('update-status', { status: 'available', message: `Доступна версия ${info.version}` });
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
                message: `Загрузка... ${progressObj.percent.toFixed(0)}%`
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