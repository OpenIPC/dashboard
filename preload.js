const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
    // Window controls
    minimizeWindow: () => ipcRenderer.send('minimize-window'),
    maximizeWindow: () => ipcRenderer.send('maximize-window'),
    closeWindow: () => ipcRenderer.send('close-window'),
    onWindowMaximized: (callback) => ipcRenderer.on('window-maximized', callback),
    onWindowUnmaximized: (callback) => ipcRenderer.on('window-unmaximized', callback),

    // Clipboard
    clipboardRead: () => ipcRenderer.invoke('clipboardRead'),
    clipboardWrite: (text) => ipcRenderer.invoke('clipboardWrite', text),

    // Authentication & Users
    login: (credentials) => ipcRenderer.invoke('login', credentials),
    getUsers: () => ipcRenderer.invoke('get-users'),
    addUser: (userData) => ipcRenderer.invoke('add-user', userData),
    updateUserPassword: (userData) => ipcRenderer.invoke('update-user-password', userData),
    updateUserRole: (userData) => ipcRenderer.invoke('update-user-role', userData),
    updateUserPermissions: (userData) => ipcRenderer.invoke('update-user-permissions', userData),
    deleteUser: (userData) => ipcRenderer.invoke('delete-user', userData),
    onAutoLoginSuccess: (callback) => ipcRenderer.on('auto-login-success', (event, user) => callback(user)),
    logoutClearCredentials: () => ipcRenderer.send('logout-clear-credentials'),
    rendererReady: () => ipcRenderer.send('renderer-ready-for-autologin'),

    // App Settings & Config
    loadAppSettings: () => ipcRenderer.invoke('load-app-settings'),
    saveAppSettings: (settings) => ipcRenderer.invoke('save-app-settings', settings),
    loadConfiguration: () => ipcRenderer.invoke('load-configuration'),
    saveConfiguration: (config) => ipcRenderer.invoke('save-configuration', config),
    selectDirectory: () => ipcRenderer.invoke('select-directory'),
    getTranslationFile: (lang) => ipcRenderer.invoke('get-translation-file', lang),
    exportConfig: () => ipcRenderer.invoke('export-config'),
    importConfig: () => ipcRenderer.invoke('import-config'),

    // Camera Actions & Info
    getCameraPulse: (camera) => ipcRenderer.invoke('get-camera-pulse', camera),
    ptzControl: (data) => ipcRenderer.invoke('ptz-control', data),
    getCameraTime: (camera) => ipcRenderer.invoke('get-camera-time', camera),
    getCameraSettings: (camera) => ipcRenderer.invoke('get-camera-settings', camera),
    setCameraSettings: (data) => ipcRenderer.invoke('set-camera-settings', data),
    restartMajestic: (camera) => ipcRenderer.invoke('restart-majestic', camera),
    startVideoStream: (streamData) => ipcRenderer.invoke('start-video-stream', streamData),
    stopVideoStream: (streamId) => ipcRenderer.invoke('stop-video-stream', streamId),
    openInBrowser: (ip) => ipcRenderer.invoke('open-in-browser', ip),
    openFileManager: (camera) => ipcRenderer.invoke('open-file-manager', camera),
    openSshTerminal: (camera) => ipcRenderer.invoke('open-ssh-terminal', camera),

    // Video Analytics
    toggleAnalytics: (cameraId) => ipcRenderer.invoke('toggle-analytics', cameraId),
    onAnalyticsUpdate: (callback) => ipcRenderer.on('analytics-update', (event, data) => callback(data)),
    onAnalyticsStatusChange: (callback) => ipcRenderer.on('analytics-status-change', (event, data) => callback(data)),
    onAnalyticsProviderInfo: (callback) => ipcRenderer.on('analytics-provider-info', (event, data) => callback(data)),

    // Recording & Archive
    startRecording: (camera) => ipcRenderer.invoke('start-recording', camera),
    stopRecording: (cameraId) => ipcRenderer.invoke('stop-recording', cameraId),
    onRecordingStateChange: (callback) => ipcRenderer.on('recording-state-change', (event, data) => callback(data)),
    openRecordingsFolder: () => ipcRenderer.invoke('open-recordings-folder'),
    getRecordingsForDate: (data) => ipcRenderer.invoke('get-recordings-for-date', data),
    exportArchiveClip: (data) => ipcRenderer.invoke('export-archive-clip', data),
    getEventsForDate: (data) => ipcRenderer.invoke('get-events-for-date', data),
    getDatesWithActivity: (cameraName) => ipcRenderer.invoke('get-dates-with-activity', cameraName),

    // System & Events
    getSystemStats: () => ipcRenderer.invoke('get-system-stats'),
    onStreamDied: (callback) => ipcRenderer.on('stream-died', (event, streamId) => callback(streamId)),
    onStreamStats: (callback) => ipcRenderer.on('stream-stats', (event, data) => callback(data)),
    onMainError: (callback) => ipcRenderer.on('on-main-error', (event, data) => callback(data)),
    showCameraContextMenu: (data) => ipcRenderer.send('show-camera-context-menu', data),
    onContextMenuCommand: (callback) => ipcRenderer.on('context-menu-command', (event, data) => callback(data)),
    killAllFfmpeg: () => ipcRenderer.invoke('kill-all-ffmpeg'),
    
    // Updates
    checkForUpdates: () => ipcRenderer.invoke('check-for-updates'),
    onUpdateStatus: (callback) => ipcRenderer.on('update-status', (event, data) => callback(data)),

    // Discovery
    discoverDevices: () => ipcRenderer.invoke('discover-devices'),
    onDeviceFound: (callback) => ipcRenderer.on('device-found', (event, data) => callback(data)),
    
    // NETIP (остается для прямого взаимодействия, если понадобится в будущем)
    getNetipSettings: (camera) => ipcRenderer.invoke('get-netip-settings', camera),
    setNetipSettings: (data) => ipcRenderer.invoke('set-netip-settings', data),
});