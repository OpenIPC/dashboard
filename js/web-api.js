// --- START OF FILE js/web-api.js (НОВЫЙ ФАЙЛ) ---
(function() {
    // Проверяем, не был ли API уже создан (например, через preload.js в Electron)
    if (window.api) {
        return;
    }

    console.log('[Web API] Running in browser mode. Initializing WebSocket connection...');

    const ws = new WebSocket(`ws://${window.location.hostname}:8081`);
    let requestIdCounter = 0;
    const pendingRequests = new Map();
    const eventListeners = new Map();

    ws.onopen = () => {
        console.log('[WebSocket] Connection to the server is open.');
        // Сообщаем UI, что API готово (если есть компоненты, которые этого ждут)
        window.dispatchEvent(new CustomEvent('api-ready'));
    };

    ws.onclose = () => {
        console.error('[WebSocket] Connection to the server was closed.');
        // Можно добавить логику для уведомления пользователя об обрыве связи
    };

    ws.onerror = (error) => {
        console.error('[WebSocket] An error occurred:', error);
    };

    ws.onmessage = (event) => {
        try {
            const { type, requestId, payload, channel } = JSON.parse(event.data);

            if (type === 'response' && pendingRequests.has(requestId)) {
                const { resolve } = pendingRequests.get(requestId);
                resolve(payload);
                pendingRequests.delete(requestId);
            } else if (type === 'event' && eventListeners.has(channel)) {
                // Эмулируем ipcRenderer.on
                const listeners = eventListeners.get(channel);
                listeners.forEach(callback => callback(payload));
            }
        } catch (e) {
            console.error('Error parsing WebSocket message:', e);
        }
    };

    const invoke = (channel, payload) => {
        return new Promise((resolve, reject) => {
            if (ws.readyState !== WebSocket.OPEN) {
                return reject(new Error('WebSocket is not connected.'));
            }

            const id = requestIdCounter++;
            pendingRequests.set(id, { resolve, reject });
            ws.send(JSON.stringify({ type: 'invoke', channel, requestId: id, payload }));

            // Устанавливаем таймаут на случай, если сервер не ответит
            setTimeout(() => {
                if (pendingRequests.has(id)) {
                    reject(new Error(`Request to channel "${channel}" timed out.`));
                    pendingRequests.delete(id);
                }
            }, 30000); // 30 секунд
        });
    };

    const send = (channel, payload) => {
        if (ws.readyState !== WebSocket.OPEN) {
            console.error('Cannot send message, WebSocket is not connected.');
            return;
        }
        ws.send(JSON.stringify({ type: 'send', channel, payload }));
    };

    const on = (channel, callback) => {
        if (!eventListeners.has(channel)) {
            eventListeners.set(channel, []);
        }
        eventListeners.get(channel).push(callback);
    };

    // Воссоздаем объект window.api, полностью повторяя структуру из preload.js
    window.api = {
        // Window controls (в браузере они не будут работать, но должны быть для совместимости)
        minimizeWindow: () => console.warn('minimizeWindow is not available in browser.'),
        maximizeWindow: () => console.warn('maximizeWindow is not available in browser.'),
        closeWindow: () => console.warn('closeWindow is not available in browser.'),
        onWindowMaximized: (callback) => on('window-maximized', callback),
        onWindowUnmaximized: (callback) => on('window-unmaximized', callback),

        // Clipboard
        clipboardRead: () => navigator.clipboard.readText(),
        clipboardWrite: (text) => navigator.clipboard.writeText(text),

        // Authentication & Users
        login: (credentials) => invoke('login', credentials),
        getUsers: () => invoke('get-users'),
        addUser: (userData) => invoke('add-user', userData),
        updateUserPassword: (userData) => invoke('update-user-password', userData),
        updateUserRole: (userData) => invoke('update-user-role', userData),
        updateUserPermissions: (userData) => invoke('update-user-permissions', userData),
        deleteUser: (userData) => invoke('delete-user', userData),
        onAutoLoginSuccess: (callback) => on('auto-login-success', callback),
        logoutClearCredentials: () => send('logout-clear-credentials'),
        rendererReady: () => send('renderer-ready-for-autologin'),

        // App Settings & Config
        loadAppSettings: () => invoke('load-app-settings'),
        saveAppSettings: (settings) => invoke('save-app-settings', settings),
        loadConfiguration: () => invoke('load-configuration'),
        saveConfiguration: (config) => invoke('save-configuration', config),
        selectDirectory: () => invoke('select-directory'), // В браузере вернет ошибку, но API должно быть
        getTranslationFile: (lang) => invoke('get-translation-file', lang),
        exportConfig: () => invoke('export-config'),
        importConfig: () => invoke('import-config'),
        
        getAppVersionInfo: () => invoke('get-app-version-info'),
        getBrandingConfig: () => invoke('get-branding-config'),
        
        openExternalLink: (url) => window.open(url, '_blank'), // В браузере это просто

        // Camera Actions & Info
        getCameraPulse: (camera) => invoke('get-camera-pulse', camera),
        ptzControl: (data) => invoke('ptz-control', data),
        getCameraTime: (camera) => invoke('get-camera-time', camera),
        getCameraSettings: (camera) => invoke('get-camera-settings', camera),
        setCameraSettings: (data) => invoke('set-camera-settings', data),
        restartMajestic: (camera) => invoke('restart-majestic', camera),
        startVideoStream: (streamData) => invoke('start-video-stream', streamData),
        stopVideoStream: (streamId) => invoke('stop-video-stream', streamId),
        pauseVideoStream: (streamId) => invoke('pause-video-stream', streamId),
        resumeVideoStream: (streamId) => invoke('resume-video-stream', streamId),
        openInBrowser: (ip) => window.open(ip.startsWith('http') ? ip : `http://${ip}`, '_blank'),
        openFileManager: (camera) => alert('File Manager is available only in the desktop app.'),
        openSshTerminal: (camera) => alert('SSH Terminal is available only in the desktop app.'),

        // Video Analytics
        toggleAnalytics: (cameraId) => invoke('toggle-analytics', cameraId),
        onAnalyticsUpdate: (callback) => on('analytics-update', callback),
        onAnalyticsStatusChange: (callback) => on('analytics-status-change', callback),
        onAnalyticsProviderInfo: (callback) => on('analytics-provider-info', callback),

        // Recording & Archive
        toggleRecording: (camera) => invoke('toggle-recording', camera),
        onRecordingStateChange: (callback) => on('recording-state-change', callback),
        openRecordingsFolder: () => invoke('open-recordings-folder'), // В браузере вернет ошибку
        getRecordingsForDate: (data) => invoke('get-recordings-for-date', data),
        exportArchiveClip: (data) => invoke('export-archive-clip', data),
        getEventsForDate: (data) => invoke('get-events-for-date', data),
        getDatesWithActivity: (cameraName) => invoke('get-dates-with-activity', cameraName),
        prepareArchiveForHls: (data) => invoke('prepare-archive-for-hls', data),

        // System & Events
        getSystemStats: () => invoke('get-system-stats'),
        onStreamInfoUpdate: (callback) => on('stream-info-update', callback),
        onStreamDied: (callback) => on('stream-died', callback),
        onStreamStats: (callback) => on('stream-stats', callback),
        onMainError: (callback) => on('on-main-error', callback),
        showCameraContextMenu: (data) => console.warn('Context menus are not available in browser.'),
        onContextMenuCommand: (callback) => {}, // Пустая функция-заглушка
        showGroupContextMenu: (data) => console.warn('Context menus are not available in browser.'),
        onGroupContextMenuCommand: (callback) => {}, // Пустая функция-заглушка
        killAllFfmpeg: () => invoke('kill-all-ffmpeg'),
        
        // Updates
        checkForUpdates: () => invoke('check-for-updates'),
        onUpdateStatus: (callback) => on('update-status', callback),
        downloadUpdate: () => invoke('download-update'),
        quitAndInstallUpdate: () => send('quit-and-install-update'),

        // Discovery
        discoverDevices: () => invoke('discover-devices'),
        onDeviceFound: (callback) => on('device-found', callback),
        
        // NETIP
        getNetipSettings: (camera) => invoke('get-netip-settings', camera),
        setNetipSettings: (data) => invoke('set-netip-settings', data),
        
        // Reporting
        openImageFiles: () => alert('Please attach files manually. This feature is not available in browser.'),
        submitReport: (data) => invoke('submit-report', data),

        // Logging from renderer
        log: {
            info: (text) => console.log(`[Renderer] ${text}`),
            warn: (text) => console.warn(`[Renderer] ${text}`),
            error: (text) => console.error(`[Renderer] ${text}`),
        },

        // Module System
        getAvailableModules: () => invoke('get-available-modules'),
        saveEnabledModules: (enabledIds) => invoke('save-enabled-modules', enabledIds),
        getRendererModules: () => invoke('get-renderer-modules'),
        
        // Подписка на события от модулей
        on: (channel, callback) => {
            const validChannels = [
                'module-object-counter-update',
                'module-object-counter-cleanup'
                // Добавляйте сюда другие каналы от модулей по мере необходимости
            ];
            if (validChannels.includes(channel)) {
                on(channel, callback);
            }
        }
    };
})();