// --- START OF FILE preload.js ---
// Файл: /preload.js
// Этот скрипт служит безопасным мостом между рендер-процессом (UI) и main-процессом (бэкенд).

const { contextBridge, ipcRenderer } = require('electron');
const path = require('path'); // <-- 1. ДОБАВЛЯЕМ МОДУЛЬ PATH

// <-- 2. ИСПОЛЬЗУЕМ АБСОЛЮТНЫЙ ПУТЬ С ПОМОЩЬЮ __dirname
const CHANNELS = require(path.join(__dirname, 'src', 'common', 'ipc-channels.js'));

contextBridge.exposeInMainWorld('api', {
    // Window controls
    minimizeWindow: () => ipcRenderer.send(CHANNELS.MINIMIZE_WINDOW),
    maximizeWindow: () => ipcRenderer.send(CHANNELS.MAXIMIZE_WINDOW),
    closeWindow: () => ipcRenderer.send(CHANNELS.CLOSE_WINDOW),
    onWindowMaximized: (callback) => ipcRenderer.on(CHANNELS.ON_WINDOW_MAXIMIZED, callback),
    onWindowUnmaximized: (callback) => ipcRenderer.on(CHANNELS.ON_WINDOW_UNMAXIMIZED, callback),

    // Clipboard
    clipboardRead: () => ipcRenderer.invoke(CHANNELS.CLIPBOARD_READ),
    clipboardWrite: (text) => ipcRenderer.invoke(CHANNELS.CLIPBOARD_WRITE, text),

    // Authentication & Users
    login: (credentials) => ipcRenderer.invoke(CHANNELS.LOGIN, credentials),
    getUsers: () => ipcRenderer.invoke(CHANNELS.GET_USERS),
    addUser: (userData) => ipcRenderer.invoke(CHANNELS.ADD_USER, userData),
    updateUserPassword: (userData) => ipcRenderer.invoke(CHANNELS.UPDATE_USER_PASSWORD, userData),
    updateUserRole: (userData) => ipcRenderer.invoke(CHANNELS.UPDATE_USER_ROLE, userData),
    updateUserPermissions: (userData) => ipcRenderer.invoke(CHANNELS.UPDATE_USER_PERMISSIONS, userData),
    deleteUser: (userData) => ipcRenderer.invoke(CHANNELS.DELETE_USER, userData),
    onAutoLoginSuccess: (callback) => ipcRenderer.on(CHANNELS.ON_AUTO_LOGIN_SUCCESS, (event, user) => callback(user)),
    logoutClearCredentials: () => ipcRenderer.send(CHANNELS.LOGOUT_CLEAR_CREDS),
    rendererReady: () => ipcRenderer.send(CHANNELS.RENDERER_READY),

    // App Settings & Config
    loadAppSettings: () => ipcRenderer.invoke(CHANNELS.LOAD_APP_SETTINGS),
    saveAppSettings: (settings) => ipcRenderer.invoke(CHANNELS.SAVE_APP_SETTINGS, settings),
    loadConfiguration: () => ipcRenderer.invoke(CHANNELS.LOAD_CONFIG),
    saveConfiguration: (config) => ipcRenderer.invoke(CHANNELS.SAVE_CONFIG, config),
    selectDirectory: () => ipcRenderer.invoke(CHANNELS.SELECT_DIRECTORY),
    getTranslationFile: (lang) => ipcRenderer.invoke(CHANNELS.GET_TRANSLATION, lang),
    exportConfig: () => ipcRenderer.invoke(CHANNELS.EXPORT_CONFIG),
    importConfig: () => ipcRenderer.invoke(CHANNELS.IMPORT_CONFIG),
    
    getAppVersionInfo: () => ipcRenderer.invoke(CHANNELS.GET_APP_VERSION_INFO),
    getBrandingConfig: () => ipcRenderer.invoke(CHANNELS.GET_BRANDING_CONFIG),
    
    openExternalLink: (url) => ipcRenderer.send(CHANNELS.OPEN_EXTERNAL_LINK, url),

    // Camera Actions & Info
    getCameraPulse: (camera) => ipcRenderer.invoke(CHANNELS.GET_CAMERA_PULSE, camera),
    ptzControl: (data) => ipcRenderer.invoke(CHANNELS.PTZ_CONTROL, data),
    getCameraTime: (camera) => ipcRenderer.invoke(CHANNELS.GET_CAMERA_TIME, camera),
    getCameraSettings: (camera) => ipcRenderer.invoke(CHANNELS.GET_CAMERA_SETTINGS, camera),
    setCameraSettings: (data) => ipcRenderer.invoke(CHANNELS.SET_CAMERA_SETTINGS, data),
    restartMajestic: (camera) => ipcRenderer.invoke(CHANNELS.RESTART_MAJESTIC, camera),
    startVideoStream: (streamData) => ipcRenderer.invoke(CHANNELS.START_VIDEO_STREAM, streamData),
    stopVideoStream: (streamId) => ipcRenderer.invoke(CHANNELS.STOP_VIDEO_STREAM, streamId),
    pauseVideoStream: (streamId) => ipcRenderer.invoke(CHANNELS.PAUSE_VIDEO_STREAM, streamId),
    resumeVideoStream: (streamId) => ipcRenderer.invoke(CHANNELS.RESUME_VIDEO_STREAM, streamId),
    openInBrowser: (ip) => ipcRenderer.invoke(CHANNELS.OPEN_IN_BROWSER, ip),
    openFileManager: (camera) => ipcRenderer.invoke(CHANNELS.OPEN_FILE_MANAGER, camera),
    openSshTerminal: (camera) => ipcRenderer.invoke(CHANNELS.OPEN_SSH_TERMINAL, camera),

    // Video Analytics
    toggleAnalytics: (cameraId) => ipcRenderer.invoke(CHANNELS.TOGGLE_ANALYTICS, cameraId),
    onAnalyticsUpdate: (callback) => ipcRenderer.on(CHANNELS.ON_ANALYTICS_UPDATE, (event, data) => callback(data)),
    onAnalyticsStatusChange: (callback) => ipcRenderer.on(CHANNELS.ON_ANALYTICS_STATUS_CHANGE, (event, data) => callback(data)),
    onAnalyticsProviderInfo: (callback) => ipcRenderer.on(CHANNELS.ON_ANALYTICS_PROVIDER_INFO, (event, data) => callback(data)),

    // Recording & Archive
    toggleRecording: (camera) => ipcRenderer.invoke(CHANNELS.TOGGLE_RECORDING, camera),
    onRecordingStateChange: (callback) => ipcRenderer.on(CHANNELS.ON_RECORDING_STATE_CHANGE, (event, data) => callback(data)),
    openRecordingsFolder: () => ipcRenderer.invoke(CHANNELS.OPEN_RECORDINGS_FOLDER),
    getRecordingsForDate: (data) => ipcRenderer.invoke(CHANNELS.GET_RECORDINGS_FOR_DATE, data),
    exportArchiveClip: (data) => ipcRenderer.invoke(CHANNELS.EXPORT_ARCHIVE_CLIP, data),
    getEventsForDate: (data) => ipcRenderer.invoke(CHANNELS.GET_EVENTS_FOR_DATE, data),
    getDatesWithActivity: (cameraName) => ipcRenderer.invoke(CHANNELS.GET_DATES_WITH_ACTIVITY, cameraName),
    // START: ИСПРАВЛЕНИЕ - Принимаем один объект `data` и передаем его целиком
    prepareArchiveForHls: (data) => ipcRenderer.invoke(CHANNELS.PREPARE_ARCHIVE_FOR_HLS, data),
    // END: ИСПРАВЛЕНИЕ

    // System & Events
    getSystemStats: () => ipcRenderer.invoke(CHANNELS.GET_SYSTEM_STATS),
    onStreamInfoUpdate: (callback) => ipcRenderer.on(CHANNELS.ON_STREAM_INFO_UPDATE, (event, data) => callback(data)),
    onStreamDied: (callback) => ipcRenderer.on(CHANNELS.ON_STREAM_DIED, (event, data) => callback(data)),
    onStreamStats: (callback) => ipcRenderer.on(CHANNELS.ON_STREAM_STATS, (event, data) => callback(data)),
    onMainError: (callback) => ipcRenderer.on(CHANNELS.ON_MAIN_ERROR, (event, data) => callback(data)),
    showCameraContextMenu: (data) => ipcRenderer.send(CHANNELS.SHOW_CAMERA_CONTEXT_MENU, data),
    onContextMenuCommand: (callback) => ipcRenderer.on(CHANNELS.ON_CONTEXT_MENU_COMMAND, (event, data) => callback(data)),
    showGroupContextMenu: (data) => ipcRenderer.send(CHANNELS.SHOW_GROUP_CONTEXT_MENU, data),
    onGroupContextMenuCommand: (callback) => ipcRenderer.on(CHANNELS.ON_GROUP_CONTEXT_MENU_COMMAND, (event, data) => callback(data)),
    killAllFfmpeg: () => ipcRenderer.invoke(CHANNELS.KILL_ALL_FFMPEG),
    
    // Updates
    checkForUpdates: () => ipcRenderer.invoke(CHANNELS.CHECK_FOR_UPDATES),
    onUpdateStatus: (callback) => ipcRenderer.on(CHANNELS.ON_UPDATE_STATUS, (event, data) => callback(data)),
    downloadUpdate: () => ipcRenderer.invoke(CHANNELS.DOWNLOAD_UPDATE),
    quitAndInstallUpdate: () => ipcRenderer.send(CHANNELS.QUIT_AND_INSTALL_UPDATE),

    // Discovery
    discoverDevices: () => ipcRenderer.invoke(CHANNELS.DISCOVER_DEVICES),
    onDeviceFound: (callback) => ipcRenderer.on(CHANNELS.ON_DEVICE_FOUND, (event, data) => callback(data)),
    
    // NETIP
    getNetipSettings: (camera) => ipcRenderer.invoke(CHANNELS.GET_NETIP_SETTINGS, camera),
    setNetipSettings: (data) => ipcRenderer.invoke(CHANNELS.SET_NETIP_SETTINGS, data),
    
    // Reporting
    openImageFiles: () => ipcRenderer.invoke(CHANNELS.OPEN_IMAGE_FILES),
    submitReport: (data) => ipcRenderer.invoke(CHANNELS.SUBMIT_REPORT, data),

    // Logging from renderer
    log: {
        info: (text) => ipcRenderer.send(CHANNELS.LOG_FROM_RENDERER, { level: 'info', text }),
        warn: (text) => ipcRenderer.send(CHANNELS.LOG_FROM_RENDERER, { level: 'warn', text }),
        error: (text) => ipcRenderer.send(CHANNELS.LOG_FROM_RENDERER, { level: 'error', text }),
    },

    // Module System
    getAvailableModules: () => ipcRenderer.invoke(CHANNELS.GET_AVAILABLE_MODULES),
    saveEnabledModules: (enabledIds) => ipcRenderer.invoke(CHANNELS.SAVE_ENABLED_MODULES, enabledIds),
    getRendererModules: () => ipcRenderer.invoke(CHANNELS.GET_RENDERER_MODULES),
    
    // Подписка на события от модулей
    on: (channel, callback) => {
        const validChannels = [
            'module-object-counter-update',
            'module-object-counter-cleanup'
        ];
        if (validChannels.includes(channel)) {
            ipcRenderer.on(channel, (event, ...args) => callback(...args));
        }
    }
});