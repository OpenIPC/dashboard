// --- START OF FILE js/web-api.js ---
(function() {
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
        window.dispatchEvent(new CustomEvent('api-ready'));
    };

    ws.onclose = () => {
        console.error('[WebSocket] Connection to the server was closed.');
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
            setTimeout(() => {
                if (pendingRequests.has(id)) {
                    reject(new Error(`Request to channel "${channel}" timed out.`));
                    pendingRequests.delete(id);
                }
            }, 30000);
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

    window.api = {
        // Функции, которые возвращают Promise (invoke)
        login: (credentials) => invoke('login', credentials),
        getUsers: () => invoke('get-users'),
        addUser: (userData) => invoke('add-user', userData),
        updateUserPassword: (userData) => invoke('update-user-password', userData),
        updateUserRole: (userData) => invoke('update-user-role', userData),
        updateUserPermissions: (userData) => invoke('update-user-permissions', userData),
        deleteUser: (userData) => invoke('delete-user', userData),
        loadAppSettings: () => invoke('load-app-settings'),
        saveAppSettings: (settings) => invoke('save-app-settings', settings),
        loadConfiguration: () => invoke('load-configuration'),
        saveConfiguration: (config) => invoke('save-configuration', config),
        exportConfig: () => invoke('export-config'),
        importConfig: () => invoke('import-config'),
        getTranslationFile: (lang) => invoke('get-translation-file', lang),
        selectDirectory: () => invoke('select-directory'),
        getAppVersionInfo: () => invoke('get-app-version-info'),
        getBrandingConfig: () => invoke('get-branding-config'),
        getCameraPulse: (camera) => invoke('get-camera-pulse', camera),
        ptzControl: (data) => invoke('ptz-control', data),
        toggleRecording: (camera) => invoke('toggle-recording', camera),
        getRecordingsForDate: (data) => invoke('get-recordings-for-date', data),
        exportArchiveClip: (data) => invoke('export-archive-clip', data),
        getEventsForDate: (data) => invoke('get-events-for-date', data),
        getDatesWithActivity: (cameraName) => invoke('get-dates-with-activity', cameraName),
        prepareArchiveForHls: (data) => invoke('prepare-archive-for-hls', data),
        discoverDevices: () => invoke('discover-devices'),
        getSystemStats: () => invoke('get-system-stats'),
        killAllFfmpeg: () => invoke('kill-all-ffmpeg'),
        checkForUpdates: () => invoke('check-for-updates'),
        downloadUpdate: () => invoke('download-update'),
        submitReport: (data) => invoke('submit-report', data),
        openImageFiles: () => invoke('open-and-read-image-files'),
        getAvailableModules: () => invoke('get-available-modules'),
        saveEnabledModules: (ids) => invoke('save-enabled-modules', ids),
        getRendererModules: () => invoke('get-renderer-modules'),
        toggleAnalytics: (id) => invoke('toggle-analytics', id),
        getAnalyticsStates: () => invoke('get-analytics-states'),
        openInBrowser: (ip) => invoke('open-in-browser', ip),
        openFileManager: (camera) => invoke('open-file-manager', camera),
        openSshTerminal: (camera) => invoke('open-ssh-terminal', camera),
        saveScreenshot: (data) => invoke('save-screenshot', data),


        // Функции, которые просто отправляют данные (send)
        minimizeWindow: () => send('minimize-window'),
        maximizeWindow: () => send('maximize-window'),
        closeWindow: () => send('close-window'),
        openRecordingsFolder: () => send('open-recordings-folder'),
        openExternalLink: (url) => send('open-external-link', url),
        rendererReady: () => send('renderer-ready-for-autologin'),
        logoutClearCredentials: () => send('logout-clear-credentials'),
        quitAndInstallUpdate: () => send('quit-and-install-update'),
        showCameraContextMenu: (data) => send('show-camera-context-menu', data),
        showGroupContextMenu: (data) => send('show-group-context-menu', data),

        // Функции для подписки на события (on)
        onWindowMaximized: (callback) => on('window-maximized', callback),
        onWindowUnmaximized: (callback) => on('window-unmaximized', callback),
        onAutoLoginSuccess: (callback) => on('auto-login-success', callback),
        onRecordingStateChange: (callback) => on('recording-state-change', callback),
        onAnalyticsUpdate: (callback) => on('analytics-update', callback),
        onAnalyticsStatusChange: (callback) => on('analytics-status-change', callback),
        onStreamDied: (callback) => on('stream-died', callback),
        onMainError: (callback) => on('on-main-error', callback),
        onUpdateStatus: (callback) => on('update-status', callback),
        onDeviceFound: (callback) => on('device-found', callback),
        onContextMenuCommand: (callback) => on('context-menu-command', callback),
        onGroupContextMenuCommand: (callback) => on('group-context-menu-command', callback),
        onMediaMtxStatsUpdate: (callback) => on('mediamtx-stats-update', callback),
    };
})();