// preload.js (ПОЛНАЯ ИСПРАВЛЕННАЯ ВЕРСИЯ)

const { contextBridge, ipcRenderer } = require('electron');
// Используем общий файл с каналами, чтобы избежать рассинхронизации
const CHANNELS = require('../src/common/ipc-channels');

/**
 * Преобразует строку из kebab-case в camelCase.
 * Пример: 'toggle-analytics' -> 'toggleAnalytics'
 * @param {string} str Строка в kebab-case.
 * @returns {string} Строка в camelCase.
 */
const kebabToCamel = (str) => str.replace(/-([a-z])/g, (g) => g[1].toUpperCase());

// Создаем пустой объект API, который будем наполнять
const api = {};

// --- СПИСКИ КАНАЛОВ ДЛЯ АВТОМАТИЧЕСКОЙ ГЕНЕРАЦИИ API ---

// Каналы, которые возвращают Promise (используют ipcRenderer.invoke)
const invokeChannels = [
    CHANNELS.LOGIN, CHANNELS.GET_USERS, CHANNELS.ADD_USER, CHANNELS.UPDATE_USER_PASSWORD,
    CHANNELS.UPDATE_USER_ROLE, CHANNELS.UPDATE_USER_PERMISSIONS, CHANNELS.DELETE_USER,
    CHANNELS.LOAD_APP_SETTINGS, CHANNELS.SAVE_APP_SETTINGS, CHANNELS.LOAD_CONFIG,
    CHANNELS.SAVE_CONFIG, CHANNELS.EXPORT_CONFIG, CHANNELS.IMPORT_CONFIG,
    CHANNELS.GET_TRANSLATION, CHANNELS.SELECT_DIRECTORY, CHANNELS.GET_APP_VERSION_INFO,
    CHANNELS.GET_BRANDING_CONFIG, CHANNELS.GET_CAMERA_PULSE, CHANNELS.PTZ_CONTROL,
    CHANNELS.TOGGLE_RECORDING, CHANNELS.GET_RECORDINGS_FOR_DATE, CHANNELS.EXPORT_ARCHIVE_CLIP,
    CHANNELS.GET_ARCHIVE_THUMBNAILS,
    CHANNELS.GET_EVENTS_FOR_DATE, CHANNELS.GET_DATES_WITH_ACTIVITY, CHANNELS.PREPARE_ARCHIVE_FOR_HLS,
    CHANNELS.DISCOVER_DEVICES, CHANNELS.GET_SYSTEM_STATS, CHANNELS.KILL_ALL_FFMPEG,
    CHANNELS.CHECK_FOR_UPDATES, CHANNELS.DOWNLOAD_UPDATE, CHANNELS.SUBMIT_REPORT,
    CHANNELS.OPEN_IMAGE_FILES, CHANNELS.GET_AVAILABLE_MODULES, CHANNELS.SAVE_ENABLED_MODULES,
    CHANNELS.GET_RENDERER_MODULES, CHANNELS.GET_DETECTED_PLATES, CHANNELS.TOGGLE_ANALYTICS, 'get-analytics-states', // 'get-analytics-states' не было в твоем CHANNELS, добавил напрямую
    CHANNELS.OPEN_IN_BROWSER, CHANNELS.OPEN_FILE_MANAGER, CHANNELS.OPEN_SSH_TERMINAL,
    CHANNELS.SAVE_SCREENSHOT,
    CHANNELS.TEST_RTSP_URL // <--- добавлено для теста RTSP
];

// Каналы, которые просто отправляют данные (используют ipcRenderer.send)
const sendChannels = [
    CHANNELS.MINIMIZE_WINDOW, CHANNELS.MAXIMIZE_WINDOW, CHANNELS.CLOSE_WINDOW,
    CHANNELS.OPEN_RECORDINGS_FOLDER, CHANNELS.OPEN_EXTERNAL_LINK, CHANNELS.RENDERER_READY,
    CHANNELS.LOGOUT_CLEAR_CREDS, CHANNELS.QUIT_AND_INSTALL_UPDATE,
    CHANNELS.SHOW_CAMERA_CONTEXT_MENU, CHANNELS.SHOW_GROUP_CONTEXT_MENU
];

// Каналы для подписки на события от Main процесса (используют ipcRenderer.on)
const onChannels = [
    CHANNELS.ON_WINDOW_MAXIMIZED, CHANNELS.ON_WINDOW_UNMAXIMIZED, CHANNELS.ON_AUTO_LOGIN_SUCCESS,
    CHANNELS.ON_RECORDING_STATE_CHANGE, CHANNELS.ON_ANALYTICS_UPDATE, CHANNELS.ON_ANALYTICS_STATUS_CHANGE,
    CHANNELS.ON_STREAM_DIED, CHANNELS.ON_MAIN_ERROR, CHANNELS.ON_UPDATE_STATUS,
    CHANNELS.ON_DEVICE_FOUND, CHANNELS.ON_CONTEXT_MENU_COMMAND, CHANNELS.ON_GROUP_CONTEXT_MENU_COMMAND,
    CHANNELS.ON_MEDIAMTX_STATS_UPDATE, CHANNELS.ON_MEDIAMTX_UPDATE,
    'mediamtx-rtsp-ready' // <--- добавлено для RTSP READY события
];


// --- АВТОМАТИЧЕСКОЕ СОЗДАНИЕ ФУНКЦИЙ API ---

// Создаем функции для invoke-каналов
invokeChannels.forEach(channel => {
    if (!channel) return;
    const camelCaseName = kebabToCamel(channel);
    // Эта строка - ключевое исправление. Она принимает все аргументы (...args)
    // и передает их дальше в ipcRenderer.invoke.
    api[camelCaseName] = (...args) => ipcRenderer.invoke(channel, ...args);
});

// Создаем функции для send-каналов
sendChannels.forEach(channel => {
    if (!channel) return;
    const camelCaseName = kebabToCamel(channel);
    api[camelCaseName] = (...args) => ipcRenderer.send(channel, ...args);
});

// Создаем функции для подписки на события
onChannels.forEach(channel => {
    if (!channel) return;
    // Превращаем 'on-window-maximized' в 'onWindowMaximized'
    const camelCaseName = 'on' + kebabToCamel(channel).charAt(0).toUpperCase() + kebabToCamel(channel).slice(1).replace(/^On/, '');
    api[camelCaseName] = (callback) => ipcRenderer.on(channel, (event, ...args) => callback(...args));
});

// Provide a small generic invoke/on helpers so renderer modules can call
// module-specific IPC channels that are not enumerated in CHANNELS above.
// This keeps renderer code simple and avoids using `require` or direct
// access to ipcRenderer from the renderer context.
api.invoke = (channel, ...args) => ipcRenderer.invoke(channel, ...args);
api.on = (channel, callback) => ipcRenderer.on(channel, (event, ...args) => callback(...args));

// Пробрасываем созданный и наполненный объект 'api' в глобальный объект window
contextBridge.exposeInMainWorld('api', api);

console.log('Preload script executed and API exposed.');