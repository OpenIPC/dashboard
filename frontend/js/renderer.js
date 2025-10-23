(function(window) {
    'use strict';
    
    let App;
    const detectedPlatesState = {
        items: [],
        uniqueTexts: new Set(),
        totalDetections: 0,
        maxItems: 200
    };

    const runnerEventsState = {
        items: [],
        limit: 150
    };

    const RUNNER_EVENT_TYPE_META = {
        'runtime-ready': { className: 'success', i18n: 'runner_event_type_runtime_ready', fallback: 'Runtime ready' },
        'runtime-error': { className: 'error', i18n: 'runner_event_type_runtime_error', fallback: 'Runtime error' },
        launch: { className: 'info', i18n: 'runner_event_type_launch', fallback: 'Launch' },
        error: { className: 'error', i18n: 'runner_event_type_error', fallback: 'Error' },
        stderr: { className: 'warning', i18n: 'runner_event_type_stderr', fallback: 'stderr' },
        fallback: { className: 'warning', i18n: 'runner_event_type_fallback', fallback: 'Fallback' },
        recognized: { className: 'success', i18n: 'runner_event_type_recognized', fallback: 'Recognized' },
        spawned: { className: 'info', i18n: 'runner_event_type_spawned', fallback: 'Spawned' },
        exit: { className: 'muted', i18n: 'runner_event_type_exit', fallback: 'Exit' },
        close: { className: 'muted', i18n: 'runner_event_type_close', fallback: 'Closed' },
        stop: { className: 'warning', i18n: 'runner_event_type_stop', fallback: 'Stopped' },
        stdout: { className: 'muted', i18n: 'runner_event_type_stdout', fallback: 'stdout' },
        info: { className: 'info', i18n: 'runner_event_type_info', fallback: 'Info' }
    };

    function applyBranding(config) {
        if (!config) {
            console.warn('[Branding] Branding config is missing. Using defaults.');
            return;
        }

        console.log('[Branding] Applying branding config to UI:', config);
        
        if (config.appName !== 'DASHBOARD for OpenIPC') {
            document.querySelectorAll('[data-brand="appName"]').forEach(el => {
                el.textContent = config.appName;
                el.removeAttribute('data-i18n-key'); 
            });
            document.title = config.appName;
        }
    
        const logoImg = document.querySelector('[data-brand="logo"]');
        if (logoImg) {
            if (config.logoDataUrl) {
                logoImg.src = config.logoDataUrl;
                logoImg.style.display = 'block';
            } else {
                logoImg.style.display = 'none';
            }
        }
        
        const featuresMap = {
            donationButton: config.features.showDonations,
            issueReportingButton: config.features.showIssueReporting,
            aboutTab: config.features.showAboutTab,
        };
    
        for (const feature in featuresMap) {
            document.querySelectorAll(`[data-brand-feature="${feature}"]`).forEach(el => {
                if (!featuresMap[feature]) {
                    el.classList.add('hidden');
                }
            });
        }
    }

    async function init() {
        console.log('[DEBUG] Renderer: init() started.');
        
        // Add direct event listener for modules toggle button
        setTimeout(() => {
            const modulesToggleBtn = document.getElementById('modules-toggle-btn');
            console.log('Modules toggle button found (delayed):', !!modulesToggleBtn);
            if (modulesToggleBtn) {
                console.log('Adding click listener to modules toggle button (delayed)');
                modulesToggleBtn.addEventListener('click', function(e) {
                    console.log('Modules toggle button clicked directly (delayed)');
                    e.stopPropagation();
                    const modulesPanel = document.getElementById('modules-panel');
                    console.log('Modules panel found (delayed):', !!modulesPanel);
                    if (modulesPanel) {
                        console.log('Toggling modules panel visibility (delayed)');
                        modulesPanel.classList.toggle('visible');
                        console.log('Modules panel classes after toggle (delayed):', modulesPanel.className);
                    } else {
                        console.error('Modules panel not found in DOM (delayed)');
                    }
                });
            } else {
                console.error('Modules toggle button not found in DOM (delayed)');
            }
            
            // Add click handler to hide modules panel when clicking outside
            document.addEventListener('click', function(e) {
                const modulesPanel = document.getElementById('modules-panel');
                const modulesToggleBtn = document.getElementById('modules-toggle-btn');
                
                if (modulesPanel && modulesPanel.classList.contains('visible')) {
                    // Check if click is outside the panel and toggle button
                    if (!modulesPanel.contains(e.target) && e.target !== modulesToggleBtn) {
                        console.log('Click outside modules panel, hiding it');
                        modulesPanel.classList.remove('visible');
                    }
                }
            });
        }, 2000);
        
        // --- Исправление: обработчик для кнопки "+" (создать новую раскладку)
        document.getElementById('add-layout-btn')?.addEventListener('click', async () => {
            const layoutName = await App.modalHandler.showPrompt({
                title: App.t('enter_layout_name_prompt'),
                label: App.t('enter_layout_name'),
                defaultValue: `${App.t('new_layout_default_name').replace('{{count}}', App.stateManager.state.layouts.length + 1)}`
            });
            if (layoutName && layoutName.trim()) {
                App.stateManager.addLayout({ name: layoutName.trim() });
            }
        });
        App = {};
        window.App = App;

        await new Promise(resolve => {
            const check = () => {
                if (window.AppModules && window.AppModules.templatesLoaded) {
                    console.log('[DEBUG] Renderer: Confirmed templates are loaded.');
                    resolve();
                } else { setTimeout(check, 50); }
            };
            check();
        });

        try {
            if (typeof window.api === 'undefined') {
                 await new Promise(resolve => window.addEventListener('api-ready', resolve, { once: true }));
            }
            const brandingConfig = await window.api.getBrandingConfig();
            applyBranding(brandingConfig);
        } catch (e) { 
            console.error("Failed to apply branding:", e); 
        }
        
        const versionInfo = await window.api.getAppVersionInfo();
        App.versionType = versionInfo.type;
        document.body.classList.add(`version-${App.versionType}`);
        
        App.USER_ROLES = { ADMIN: 'admin', OPERATOR: 'operator' };
        
        App.stateManager = AppModules.createStateManager({
            initialState: { cameras: [], groups: [], layouts: [], activeLayoutId: null, recordingStates: {}, appSettings: {}, isSaving: false, currentUser: null },
            // onChange will be called for deep mutations; avoid spamming saves by using debounced save here
            onChange: () => {
                try {
                    // Use debounced save for general state changes
                    if (App && typeof App.saveConfiguration === 'function') App.saveConfiguration('state-manager:onChange');
                } catch (e) { /* ignore */ }
            },
            ignoreKeys: ['isSaving','player'],
            mutations: {
                setInitialConfig(state, helpers, config) {
                    console.log('[Config][Load] Применяем конфиг:', JSON.stringify(config, null, 2));
                    state.cameras = config.cameras || [];
                    state.groups = config.groups || [];
                    if (config.layouts && config.layouts.length > 0) {
                        state.layouts = config.layouts;
                        state.activeLayoutId = config.activeLayoutId && config.layouts.some(l => l.id === config.activeLayoutId) ? config.activeLayoutId : config.layouts[0].id;
                    } else if (config.gridState) {
                        const defaultLayout = {
                            id: Date.now(),
                            name: 'Основной вид',
                            gridState: config.gridState,
                            layout: config.layout || { cols: 2, rows: 2 }
                        };
                        state.layouts = [defaultLayout];
                        state.activeLayoutId = defaultLayout.id;
                        App.saveConfiguration();
                    } else {
                        helpers.getActiveLayout(state);
                    }
                    console.log('[Config][Load] Конфиг применён. Камер:', state.cameras.length, 'Макет(ов):', state.layouts.length);
                },
                setAppSettings(state, helpers, settings) { state.appSettings = { ...state.appSettings, ...settings }; App.saveAppSettings(); },
                updateGridState(state, helpers, gridState) { const activeLayout = helpers.getActiveLayout(state); if (activeLayout) { activeLayout.gridState = gridState; App.saveConfigurationNow('mutation:updateGridState').catch(e => console.error('[Config][SaveNow] updateGridState save failed', e)); } },
                updateGridLayout(state, helpers, layout) { const activeLayout = helpers.getActiveLayout(state); if (activeLayout) { activeLayout.layout = layout; App.saveConfigurationNow('mutation:updateGridLayout').catch(e => console.error('[Config][SaveNow] updateGridLayout save failed', e)); } },
                setActiveLayout(state, helpers, layoutId) { if (state.layouts.some(l => l.id === layoutId)) { state.activeLayoutId = layoutId; App.saveConfigurationNow('mutation:setActiveLayout').catch(e => console.error('[Config][SaveNow] setActiveLayout save failed', e)); } },
                addLayout(state, helpers, { name }) { const newLayout = { id: Date.now(), name: name, gridState: Array(64).fill(null), layout: { cols: 2, rows: 2 } }; state.layouts = [...state.layouts, newLayout]; state.activeLayoutId = newLayout.id; App.saveConfigurationNow('mutation:addLayout').catch(e => console.error('[Config][SaveNow] addLayout save failed', e)); },
                deleteLayout(state, helpers, layoutId) { if (state.layouts.length <= 1) { alert(App.t('cannot_delete_last_layout')); return; } state.layouts = state.layouts.filter(l => l.id !== layoutId); if (state.activeLayoutId === layoutId) { state.activeLayoutId = state.layouts[0].id; } App.saveConfigurationNow('mutation:deleteLayout').catch(e => console.error('[Config][SaveNow] deleteLayout save failed', e)); },
                renameLayout(state, helpers, { id, newName }) { const layoutToRename = state.layouts.find(l => l.id === id); if (layoutToRename) { layoutToRename.name = newName; App.saveConfigurationNow('mutation:renameLayout').catch(e => console.error('[Config][SaveNow] renameLayout save failed', e)); } },
                reorderLayouts(state, helpers, { draggedId, targetId }) { const layouts = state.layouts; const draggedIndex = layouts.findIndex(l => l.id === draggedId); const targetIndex = layouts.findIndex(l => l.id === targetId); if (draggedIndex === -1 || targetIndex === -1) return; const [draggedItem] = layouts.splice(draggedIndex, 1); layouts.splice(targetIndex, 0, draggedItem); state.layouts = [...state.layouts]; App.saveConfigurationNow('mutation:reorderLayouts').catch(e => console.error('[Config][SaveNow] reorderLayouts save failed', e)); },
                addCamera(state, helpers, camera) {
                    const newCamera = { id: Date.now(), groupId: null, ...camera };
                    state.cameras = [...state.cameras, newCamera];
                    // critical: persist immediately to avoid losing camera additions
                    if (App && typeof App.saveConfigurationNow === 'function') App.saveConfigurationNow('mutation:addCamera').catch(e => console.error('[Config][SaveNow] addCamera save failed', e)); else App.saveConfiguration('mutation:addCamera');
                },
                updateCamera(state, helpers, updatedCamera) { state.cameras = state.cameras.map(c => c.id === updatedCamera.id ? { ...c, ...updatedCamera } : c); if (App && typeof App.saveConfigurationNow === 'function') App.saveConfigurationNow('mutation:updateCamera').catch(e => console.error('[Config][SaveNow] updateCamera save failed', e)); else App.saveConfiguration('mutation:updateCamera'); },
                deleteCamera(state, helpers, cameraId) { state.layouts.forEach(layout => { layout.gridState = layout.gridState.map(cell => (cell && cell.camera.id === cameraId) ? null : cell); }); state.cameras = state.cameras.filter(c => c.id !== cameraId); try { if (typeof App.saveConfigurationNow === 'function') { App.saveConfigurationNow('mutation:deleteCamera').catch(e => console.error('[Config][SaveNow] Error during immediate save:', e)); } else { App.saveConfiguration('mutation:deleteCamera'); } } catch (e) { console.error('[Config][Delete] Error triggering save after delete:', e); } },
                addGroup(state, helpers, group) { state.groups = [...state.groups, { id: Date.now(), ...group }]; if (App && typeof App.saveConfigurationNow === 'function') App.saveConfigurationNow('mutation:addGroup').catch(e => console.error('[Config][SaveNow] addGroup save failed', e)); else App.saveConfiguration('mutation:addGroup'); },
                renameGroup(state, helpers, { id, newName }) { const groupToRename = state.groups.find(g => g.id === id); if (groupToRename) { groupToRename.name = newName; App.saveConfigurationNow('mutation:renameGroup').catch(e => console.error('[Config][SaveNow] renameGroup save failed', e)); } },
                deleteGroup(state, helpers, groupId) { state.cameras = state.cameras.map(camera => { if (camera.groupId === groupId) { return { ...camera, groupId: null }; } return camera; }); state.groups = state.groups.filter(g => g.id !== groupId); App.saveConfigurationNow('mutation:deleteGroup').catch(e => console.error('[Config][SaveNow] deleteGroup save failed', e)); },
                setRecordingState(state, helpers, { cameraId, recording }) { state.recordingStates = { ...state.recordingStates, [cameraId]: recording }; },
                setCurrentUser(state, helpers, user) { state.currentUser = user; },
                logout(state, helpers) { state.currentUser = null; }
            }
        });
        
        App.t = (key, replacements) => key;
        App.i18n = AppModules.createI18n(App);

        async function loadAppSettings() {
            if (!window.api) {
                 await new Promise(resolve => window.addEventListener('api-ready', resolve, { once: true }));
            }
            const settings = await window.api.loadAppSettings();
            App.stateManager.state.appSettings = settings;
            return settings;
        }
        
        const settings = await loadAppSettings();
        await App.i18n.init(settings.language);

        App.t = App.i18n.t;

        console.log('[DEBUG] Renderer: Creating and initializing module handlers...');
        App.modalHandler = AppModules.createModalHandler(App);
        App.cameraList = AppModules.createCameraList(App);
        App.gridManager = AppModules.createGridManager(App);
        App.archiveManager = AppModules.createArchiveManager(App);
        App.windowControls = AppModules.createWindowControls(App);
        
        App.modalHandler.init();
        App.cameraList.init();
        App.gridManager.init();
        App.archiveManager.init();
        App.windowControls.init();
        console.log('[DEBUG] Renderer: All module handlers initialized.');

        // --- MediaMTX RTSP READY EVENT HANDLER ---
        if (window.api && typeof window.api.onMediamtxRtspReady === 'function') {
            window.api.onMediamtxRtspReady((payload) => {
                // payload.paths: array of ready cam paths (cam{id}_0, cam{id}_1, ...)
                console.log('[RTSP READY EVENT]', payload);
                if (!App.stateManager || !App.stateManager.state || !App.gridManager) return;
                const cameras = App.stateManager.state.cameras || [];
                cameras.forEach(cam => {
                    [0, 1].forEach(streamId => {
                        const streamPath = `cam${cam.id}_${streamId}`;
                        if (payload.paths && payload.paths.includes(streamPath)) {
                            console.log(`[RTSP PRELOAD] Preloading stream: ${streamPath}`);
                            // Preload stream via WHEP HEAD request
                            const whepUrl = `http://127.0.0.1:8889/${streamPath}/whep`;
                            fetch(whepUrl, { method: 'HEAD', cache: 'reload' }).catch(()=>{});
                        }
                    });
                });
            });
        }

        let loginView, mainAppContainer, loginBtn, loginUsername, loginPassword, loginRememberMe, loginError, logoutBtn, statusInfo, loginCloseBtn;

        async function loadConfiguration() {
            const config = await window.api.loadConfiguration();
            console.log('[Config][Load] Загружаем конфиг:', JSON.stringify(config, null, 2));
            App.stateManager.setInitialConfig(config);
            // Update MediaMTX paths after loading config
            try {
                await window.api.updateMediaMTXPaths();
            } catch (e) {
                console.error('[Renderer] Failed to update MediaMTX paths after loading config:', e);
            }
        }

        App.saveAppSettings = async () => { 
            // Safely clone appSettings to ensure only JSON-serializable data is sent over IPC/WS.
            const safeClone = (obj) => {
                try {
                    const seen = new WeakSet();
                    const str = JSON.stringify(obj, (k, v) => {
                        if (typeof v === 'function' || typeof v === 'symbol') return undefined;
                        if (v === undefined) return null;
                        if (v && typeof v === 'object') {
                            if (seen.has(v)) return undefined;
                            seen.add(v);
                        }
                        return v;
                    });
                    return JSON.parse(str);
                } catch (e) {
                    console.error('[App] safeClone failed:', e);
                    throw e;
                }
            };

            try {
                const payload = safeClone(App.stateManager.state.appSettings || {});
                const res = await window.api.saveAppSettings(payload);
                console.log('[App] saveAppSettings result:', res);
                return res;
            } catch (e) {
                console.error('[App] saveAppSettings failed:', e);
                throw e;
            }
        };

        // Filter functions for config serialization
        function filterGridState(gridState) {
            return Array.isArray(gridState)
                ? gridState.map(cell => {
                    if (!cell || typeof cell !== 'object') return null;
                    // streamId только 0 или 1
                    let streamId = 0;
                    if (typeof cell.streamId === 'number' && (cell.streamId === 0 || cell.streamId === 1)) {
                        streamId = cell.streamId;
                    }
                    return {
                        camera: cell.camera && typeof cell.camera.id === 'number' ? { id: cell.camera.id } : undefined,
                        streamId,
                        paused: cell.paused === true ? true : undefined
                    };
                })
                : [];
        }
        function filterCamera(cam) {
            if (!cam || typeof cam !== 'object') return {};
            const { id, groupId, name, ip, port, username, password, streamPath, streamPath0, streamPath1, protocol, onvifAuth } = cam;
            return { id, groupId, name, ip, port, username, password, streamPath, streamPath0, streamPath1, protocol, onvifAuth };
        }
        function filterLayout(l) {
            if (!l || typeof l !== 'object') return {};
            const { id, name, gridState, layout } = l;
            return {
                id,
                name,
                gridState: filterGridState(gridState),
                layout: layout && typeof layout === 'object' ? { cols: layout.cols, rows: layout.rows } : { cols: 2, rows: 2 }
            };
        }

        let saveTimeout;
        App.saveConfiguration = function(origin) {
            const state = App.stateManager.state;
            if (state.isSaving) return;
            clearTimeout(saveTimeout);
            saveTimeout = setTimeout(async () => {
                state.isSaving = true;
                const config = {
                    cameras: state.cameras.map(c => { const { player, ...rest } = c; return filterCamera(rest); }),
                    groups: state.groups,
                    layouts: state.layouts.map(filterLayout),
                    activeLayoutId: state.activeLayoutId,
                };
                console.log('[Config][Save] Сохраняем конфиг (origin=' + (origin||'unknown') + '):', JSON.stringify(config, null, 2));
                try {
                    await window.api.saveConfiguration(config, { origin: origin || 'renderer:saveConfiguration' });
                    console.log('[Config][Save] Конфиг успешно сохранён!');
                } catch (e) {
                    console.error('[Config][Save] Ошибка сохранения:', e);
                } finally {
                    state.isSaving = false;
                }
            }, 500);
        };

        // Immediate save helper: clears any debounce and writes config immediately.
        // Used for critical operations (like delete) to avoid losing changes on quick shutdown.
        App.saveConfigurationNow = async function() {
            const state = App.stateManager.state;
            // If another save is already in progress, wait a short while for it to finish
            if (state.isSaving) {
                await new Promise(resolve => {
                    const check = () => { if (!state.isSaving) return resolve(); setTimeout(check, 50); };
                    check();
                });
            }
            clearTimeout(saveTimeout);
            state.isSaving = true;
            const config = {
                cameras: state.cameras.map(filterCamera),
                groups: state.groups,
                layouts: state.layouts.map(filterLayout),
                activeLayoutId: state.activeLayoutId,
            };
            console.log('[Config][SaveNow] Immediate save of config:', JSON.stringify(config, null, 2));
            // Deep clone to remove any proxies or non-serializable objects
            let safeConfig;
            try {
                safeConfig = JSON.parse(JSON.stringify(config));
            } catch (e) {
                console.warn('[Config][SaveNow] Failed to deep clone config, using original (may contain proxies):', e);
                safeConfig = config;
            }
            try {
                await window.api.saveConfiguration(safeConfig, { origin: 'renderer:saveConfigurationNow' });
                console.log('[Config][SaveNow] Config successfully saved (immediate).');
            } catch (e) {
                console.error('[Config][SaveNow] Error saving config immediately:', e);
                throw e;
            } finally {
                state.isSaving = false;
            }
        };

        function updateSystemStats() {
            window.api.getSystemStats().then(stats => {
                if (statusInfo) {
                    statusInfo.textContent = `${App.t('status_cpu')}: ${stats.cpu}% | ${App.t('status_ram')}: ${stats.ram} MB`;
                }
            });
        }

        async function handleLogin() {
            const username = loginUsername.value.trim();
            const password = loginPassword.value;
            const rememberMe = loginRememberMe.checked;
            loginError.textContent = '';
            if (!username || !password) return;
            loginBtn.disabled = true;
            loginBtn.textContent = App.t('connecting');
            try {
                console.log('login call', username, password, rememberMe);
                const result = await window.api.login(username, password, rememberMe);
                console.log('login result', result);
                if (result.success) {
                    App.stateManager.setCurrentUser(result.user);
                    loginView.classList.add('hidden');
                    mainAppContainer.classList.remove('hidden');
                    loginPassword.value = '';
                } else {
                    loginError.textContent = App.t('invalid_credentials');
                }
            } finally {
                loginBtn.disabled = false;
                loginBtn.textContent = App.t('login_btn');
            }
        }
        function handleLogout() {
            window.api.logoutClearCredentials();
            App.stateManager.logout();
            mainAppContainer.classList.add('hidden');
            loginView.classList.remove('hidden');
            document.body.className = '';
            document.body.classList.add(`version-${App.versionType}`);
            loginUsername.focus();
        }

        loginView = document.getElementById('login-view');
        mainAppContainer = document.getElementById('main-app-container');
    loginBtn = document.getElementById('login-btn-main');
    loginUsername = document.getElementById('login-username');
    loginPassword = document.getElementById('login-password-main');
        loginRememberMe = document.getElementById('login-remember-me');
        loginError = document.getElementById('login-error');
        logoutBtn = document.getElementById('logout-btn');
        statusInfo = document.getElementById('status-info');
        loginCloseBtn = document.getElementById('login-close-btn');

        loginBtn.addEventListener('click', handleLogin);
        loginPassword.addEventListener('keypress', (e) => { if (e.key === 'Enter') handleLogin(); });
        logoutBtn.addEventListener('click', handleLogout);
        loginCloseBtn.addEventListener('click', () => window.api.closeWindow());
        
        // Only register Electron API events if available
        if (window.api && typeof window.api.onOnMainError === 'function') {
            window.api.onOnMainError(({ context, message }) => {
                console.error(`[Main Process Error in ${context}]`, message);
                App.modalHandler.showToast(`${App.t('error')}: ${message}`, true, 5000);
            });
        }
        // Show MediaMTX update status messages (hot-update / fallback) as toast notifications
        // The preload auto-generates on-methods from channel names. The channel
        // 'mediamtx-update-status' becomes window.api.onMediamtxUpdate
        if (window.api && typeof window.api.onMediamtxUpdate === 'function') {
            window.api.onMediamtxUpdate((payload) => {
                try {
                    const text = payload && payload.message ? payload.message : 'MediaMTX update';
                    const isError = payload && (payload.stage === 'failed');
                    App.modalHandler.showToast(text, isError, 6000);
                    // Если MediaMTX готов, сразу перерисовать сетку для мгновенного старта видео
                    if (payload && payload.stage === 'done' && App.gridManager && typeof App.gridManager.render === 'function') {
                        setTimeout(() => App.gridManager.render(), 0);
                    }
                } catch (e) { /* ignore */ }
            });
        }
        if (window.api && typeof window.api.onRecordingStateChange === 'function') {
            window.api.onRecordingStateChange(({ cameraId, recording }) => App.stateManager.setRecordingState({ cameraId, recording }));
        }
        if (window.api && typeof window.api.onAutoLoginSuccess === 'function') {
            window.api.onAutoLoginSuccess((user) => {
                console.log('[AutoLogin] Received user data. Logging in...');
                App.stateManager.setCurrentUser(user);
                loginView.classList.add('hidden');
                mainAppContainer.classList.remove('hidden');
                loginPassword.value = '';
            });
        }
        window.api.onAnalyticsUpdate((data) => {
            if (App.gridManager) {
                App.gridManager.handleAnalyticsUpdate(data);
            }
            const event = new CustomEvent('app-analytics-update', { detail: data });
            window.dispatchEvent(event);
        });
        window.api.onAnalyticsStatusChange(({ cameraId, active }) => {
            const btn = document.getElementById(`analytics-btn-${cameraId}`);
            if (btn) btn.classList.toggle('active', active);
        });
        
        async function loadRendererModules() {
            try {
                const scripts = await window.api.getRendererModules();
                scripts.forEach(scriptPath => {
                    const script = document.createElement('script');
                    const cleanPath = scriptPath.replace(/\\/g, '/');
                    script.src = cleanPath;
                    document.head.appendChild(script);
                });
            } catch (error) {
                console.error('[Modules] Failed to get renderer modules:', error);
            }
        }
        await loadRendererModules();

        App.stateManager.subscribe(() => {
            renderLayoutTabs();
            updateUserPermissionsUI();
            App.cameraList.render();
            App.gridManager.render();
            App.gridManager.updateGridLayoutView();
        });
        
        await loadConfiguration();
        
        window.addEventListener('language-changed', () => {
            App.cameraList.render();
            App.gridManager.updatePlaceholdersLanguage();
            updateSystemStats();
            if (!loginView.classList.contains('hidden')) {
                App.i18n.applyTranslationsToDOM();
            }
        });
        
        function initializeGlobalClickHandlers() {
            const mainAppContainer = document.getElementById('main-app-container');
            if (!mainAppContainer) {
                console.error('CRITICAL: main-app-container not found!');
                return;
            }

            console.log('Attaching global click listener to main-app-container...');

            mainAppContainer.addEventListener('click', async (e) => {
                const button = e.target.closest('button');
                if (!button) return;

                console.log(`[Global Click] Button with ID: #${button.id} was clicked.`);

                switch (button.id) {
                    case 'open-recordings-btn':
                        window.api.openRecordingsFolder();
                        break;
                    
                    case 'save-layout-btn':
                        const layoutName = await App.modalHandler.showPrompt({
                            title: App.t('enter_layout_name_prompt'),
                            label: App.t('enter_layout_name'),
                            defaultValue: `${App.t('new_layout_default_name').replace('{{count}}', App.stateManager.state.layouts.length + 1)}`
                        });
                        if (layoutName && layoutName.trim()) {
                            App.stateManager.addLayout({ name: layoutName.trim() });
                        }
                        break;

                    case 'modules-toggle-btn':
                        console.log('Modules toggle button clicked via global handler');
                        const modulesPanel = document.getElementById('modules-panel');
                        console.log('Modules panel found in global handler:', !!modulesPanel);
                        if (modulesPanel) {
                            console.log('Toggling modules panel visibility via global handler');
                            modulesPanel.classList.toggle('visible');
                            console.log('Modules panel classes after toggle (global):', modulesPanel.className);
                        } else {
                            console.error('Modules panel not found in DOM (global handler)');
                        }
                        break;

                    case 'rename-layout-btn':
                        const activeLayoutRename = App.stateManager.state.layouts.find(l => l.id === App.stateManager.state.activeLayoutId);
                        if (!activeLayoutRename) return;
                        const newName = await App.modalHandler.showPrompt({
                            title: App.t('rename_layout_tooltip'),
                            label: App.t('enter_new_layout_name'),
                            defaultValue: activeLayoutRename.name
                        });
                        if (newName && newName.trim()) {
                            App.stateManager.renameLayout({ id: activeLayoutRename.id, newName: newName.trim() });
                        }
                        break;

                    case 'delete-layout-btn':
                        const activeLayoutDelete = App.stateManager.state.layouts.find(l => l.id === App.stateManager.state.activeLayoutId);
                        if (!activeLayoutDelete) return;
                        const confirmation = await App.modalHandler.showPrompt({
                            title: App.t('delete_layout_tooltip'),
                            label: App.t('confirm_delete_layout'),
                            inputType: 'none',
                            okText: App.t('context_delete')
                        });
                        if (confirmation) {
                            App.stateManager.deleteLayout(activeLayoutDelete.id);
                        }
                        break;
                    
                    case 'presentation-mode-btn':
                        document.body.classList.toggle('presentation-mode');
                        setTimeout(() => {
                            window.dispatchEvent(new Event('resize'));
                        }, 50);
                        break;
                }
            });
        }

        initializeGlobalClickHandlers();

        window.addEventListener('keydown', (e) => {
            if (e.key === 'Escape' && document.body.classList.contains('presentation-mode')) {
                document.body.classList.remove('presentation-mode');
                setTimeout(() => {
                    window.dispatchEvent(new Event('resize'));
                }, 50);
            }
        });
        
        setInterval(updateSystemStats, 3000);
        setInterval(() => App.cameraList.pollCameraStatuses(), 10000);
        initializeDetectedPlatesPanel();
        updateSystemStats();

        console.log('[DEBUG] Renderer: Sending rendererReady signal...');
        // Only call Electron API if available
        if (window.api && typeof window.api.rendererReadyForAutologin === 'function') {
            window.api.rendererReadyForAutologin();
        }
        console.log('[DEBUG] Renderer: init() finished.');
    }

    function renderLayoutTabs() {
        const layoutTabsContainer = document.querySelector('.header .tabs');
        if (!layoutTabsContainer) return;
        const { layouts, activeLayoutId } = App.stateManager.state;
        layoutTabsContainer.innerHTML = '';
        if (App.versionType === 'lite' || !layouts) return;
        layouts.forEach(l => {
            const tab = document.createElement('button');
            tab.className = 'tab';
            if (l.id === activeLayoutId) tab.classList.add('active');
            tab.dataset.layoutId = l.id;
            tab.draggable = true;
            tab.innerHTML = `<span>${l.name}</span><span class="close-tab-btn">×</span>`;
            tab.querySelector('.close-tab-btn').addEventListener('click', e => { 
                e.stopPropagation(); 
                if (confirm(App.t('confirm_delete_layout'))) App.stateManager.deleteLayout(l.id); 
            });
            tab.addEventListener('click', () => App.stateManager.setActiveLayout(l.id));
            tab.addEventListener('contextmenu', async (e) => {
                e.preventDefault();
                // Простое контекстное меню только с пунктом "Переименовать"
                const menu = document.createElement('div');
                menu.className = 'custom-context-menu';
                menu.style.position = 'fixed';
                menu.style.zIndex = 10000;
                menu.style.left = e.clientX + 'px';
                menu.style.top = e.clientY + 'px';
                menu.style.background = '#222';
                menu.style.color = '#fff';
                menu.style.padding = '8px 18px';
                menu.style.borderRadius = '6px';
                menu.style.boxShadow = '0 2px 8px rgba(0,0,0,0.18)';
                menu.style.cursor = 'pointer';
                menu.textContent = App.t('context_rename_layout') || 'Переименовать';
                menu.addEventListener('click', async () => {
                    document.body.removeChild(menu);
                    const newName = await App.modalHandler.showPrompt({
                        title: App.t('context_rename_layout') || 'Переименовать',
                        label: App.t('enter_new_layout_name') || 'Введите новое имя:',
                        defaultValue: l.name
                    });
                    if (newName && newName.trim()) {
                        App.stateManager.renameLayout({ id: l.id, newName: newName.trim() });
                    }
                });
                document.body.appendChild(menu);
                const removeMenu = (ev) => {
                    if (ev.target !== menu) {
                        menu.remove();
                        document.removeEventListener('mousedown', removeMenu);
                        document.removeEventListener('scroll', removeMenu, true);
                    }
                };
                setTimeout(() => {
                    document.addEventListener('mousedown', removeMenu);
                    document.addEventListener('scroll', removeMenu, true);
                }, 0);
            });
            tab.addEventListener('dragstart', e => { e.dataTransfer.setData('application/x-layout-id', String(l.id)); tab.classList.add('dragging'); });
            tab.addEventListener('dragend', () => tab.classList.remove('dragging'));
            tab.addEventListener('dragover', e => { e.preventDefault(); tab.classList.add('drag-over'); });
            tab.addEventListener('dragleave', () => tab.classList.remove('drag-over'));
            tab.addEventListener('drop', e => { e.preventDefault(); tab.classList.remove('drag-over'); const draggedId = Number(e.dataTransfer.getData('application/x-layout-id')); if (draggedId && draggedId !== l.id) App.stateManager.reorderLayouts({ draggedId, targetId: l.id }); });
            layoutTabsContainer.appendChild(tab);
        });
    }

    function updateUserPermissionsUI() {
        const user = App.stateManager.state.currentUser;
        const body = document.body;
        body.className = body.className.split(' ').filter(c => !c.startsWith('role-') && !c.startsWith('can-')).join(' ');
        body.classList.add(`version-${App.versionType}`);
        if (user) {
            body.classList.add(`role-${user.role}`);
            if (user.role === App.USER_ROLES.OPERATOR && user.permissions) {
                Object.keys(user.permissions).forEach(permission => {
                    if (user.permissions[permission]) body.classList.add(`can-${permission.replace(/_/g, '-')}`);
                });
            }
        }
    }

    function translateOrFallback(key, fallback) {
        return App && typeof App.t === 'function' ? App.t(key) : fallback;
    }

    function initializeDetectedPlatesPanel() {
        renderDetectedPlatesPanel();
        renderRunnerEventsPanel();

        const clearBtn = document.getElementById('runner-events-clear');
        if (clearBtn && clearBtn.dataset.bound !== '1') {
            clearBtn.addEventListener('click', () => {
                runnerEventsState.items = [];
                renderRunnerEventsPanel();
            });
            clearBtn.dataset.bound = '1';
        }

        if (!window.api || typeof window.api.getDetectedPlates !== 'function') {
            console.warn('[Renderer] API for detected plates is not available in this build.');
            return;
        }

        refreshDetectedPlatesFromBackend();
        setInterval(() => refreshDetectedPlatesFromBackend(), 15000);

        if (typeof window.api.on === 'function') {
            window.api.on('module-license-plate-saved', handlePlateSavedEvent);
            window.api.on('module-license-plate-cleanup', resetDetectedPlatesPanel);
            window.api.on('module-license-plate-runner-event', handleRunnerEvent);
        }
    }

    function refreshDetectedPlatesFromBackend() {
        if (!window.api || typeof window.api.getDetectedPlates !== 'function') {
            return;
        }

        window.api.getDetectedPlates()
            .then(snapshot => applyDetectedPlatesSnapshot(snapshot))
            .catch(error => {
                console.error('[Renderer] Failed to refresh detected plates:', error);
                if (!detectedPlatesState.items.length) {
                    renderDetectedPlatesPanel(translateOrFallback('plates_load_error', 'Ошибка загрузки номеров'));
                }
            });
    }

    function applyDetectedPlatesSnapshot(snapshot) {
        if (!snapshot) {
            resetDetectedPlatesPanel();
            return;
        }

        detectedPlatesState.totalDetections = Number.isFinite(snapshot.totalDetections) ? snapshot.totalDetections : 0;
        detectedPlatesState.uniqueTexts = new Set();
        if (Array.isArray(snapshot.uniquePlates)) {
            snapshot.uniquePlates.forEach(cameraEntry => {
                if (!cameraEntry || !Array.isArray(cameraEntry.plates)) return;
                cameraEntry.plates.forEach(text => {
                    if (typeof text === 'string' && text.trim()) {
                        detectedPlatesState.uniqueTexts.add(text.trim());
                    }
                });
            });
        }

        const recent = Array.isArray(snapshot.recentHistory) ? snapshot.recentHistory : [];
        const normalized = recent
            .filter(item => item && item.text)
            .map(normalizePlateEntry)
            .sort((a, b) => (b.timestamp || '').localeCompare(a.timestamp || ''))
            .slice(0, detectedPlatesState.maxItems);

        normalized.forEach(entry => {
            if (entry.text) {
                detectedPlatesState.uniqueTexts.add(entry.text);
            }
        });

        detectedPlatesState.items = normalized;
        renderDetectedPlatesPanel();
    }

    function handlePlateSavedEvent(payload) {
        if (!payload || !payload.text) {
            return;
        }

        const entry = normalizePlateEntry(payload);
        const newKey = buildPlateKey(entry);
        const existingKeys = new Set();
        const deduped = [entry];
        existingKeys.add(newKey);

        let alreadyExisted = false;

        for (const item of detectedPlatesState.items) {
            const key = buildPlateKey(item);
            if (key === newKey) {
                alreadyExisted = true;
            }
            if (!existingKeys.has(key)) {
                deduped.push(item);
                existingKeys.add(key);
            }
            if (deduped.length >= detectedPlatesState.maxItems) {
                break;
            }
        }

        detectedPlatesState.items = deduped;
        detectedPlatesState.totalDetections = alreadyExisted
            ? Math.max(detectedPlatesState.totalDetections, deduped.length)
            : Math.max(detectedPlatesState.totalDetections + 1, deduped.length);

        if (entry.text) {
            detectedPlatesState.uniqueTexts.add(entry.text);
        }

        renderDetectedPlatesPanel();
    }

    function resetDetectedPlatesPanel() {
        detectedPlatesState.items = [];
        detectedPlatesState.uniqueTexts = new Set();
        detectedPlatesState.totalDetections = 0;
        renderDetectedPlatesPanel();
    }

    function renderDetectedPlatesPanel(message) {
        const listEl = document.getElementById('detected-plates-list');
        const totalEl = document.getElementById('detected-plates-total');
        const uniqueEl = document.getElementById('detected-plates-unique');
        if (!listEl) return;

        ensureDetectedPlateActionListener();

        if (totalEl) totalEl.textContent = detectedPlatesState.totalDetections.toString();
        if (uniqueEl) uniqueEl.textContent = detectedPlatesState.uniqueTexts.size.toString();

        listEl.innerHTML = '';

        if (typeof message === 'string' && message.trim()) {
            listEl.innerHTML = `<div class="no-plates">${escapeHtml(message)}</div>`;
            return;
        }

        if (!detectedPlatesState.items.length) {
            const emptyMessage = translateOrFallback('plates_none_message', 'Нет обнаруженных номеров');
            listEl.innerHTML = `<div class="no-plates">${escapeHtml(emptyMessage)}</div>`;
            return;
        }

        detectedPlatesState.items.forEach(item => {
            const card = document.createElement('div');
            card.className = 'plate-card';
            card.innerHTML = `
                <div class="plate-card-header">
                    <span class="plate-number">${escapeHtml(item.text)}</span>
                    ${item.score !== null ? `<span class="plate-score">${escapeHtml(formatPlateScore(item.score))}</span>` : ''}
                </div>
                <div class="plate-meta">
                    <span class="plate-camera">${escapeHtml(item.cameraName)}</span>
                    <span class="plate-time">${escapeHtml(item.displayTime)}</span>
                </div>
            `;

            if (item.path) {
                const actions = document.createElement('div');
                actions.className = 'plate-actions';

                const openBtn = document.createElement('button');
                openBtn.type = 'button';
                openBtn.className = 'plate-action-btn';
                openBtn.dataset.action = 'open';
                openBtn.dataset.path = item.path;
                openBtn.title = translateOrFallback('plate_action_open_image', 'Открыть изображение');
                openBtn.innerHTML = '<span class="material-icons">open_in_new</span>';
                actions.appendChild(openBtn);

                const revealBtn = document.createElement('button');
                revealBtn.type = 'button';
                revealBtn.className = 'plate-action-btn';
                revealBtn.dataset.action = 'reveal';
                revealBtn.dataset.path = item.path;
                revealBtn.title = translateOrFallback('plate_action_show_folder', 'Показать в папке');
                revealBtn.innerHTML = '<span class="material-icons">folder_open</span>';
                actions.appendChild(revealBtn);

                card.appendChild(actions);
            }

            listEl.appendChild(card);
        });
    }

    function handleRunnerEvent(payload) {
        const event = normalizeRunnerEvent(payload);
        runnerEventsState.items.unshift(event);
        if (runnerEventsState.items.length > runnerEventsState.limit) {
            runnerEventsState.items.length = runnerEventsState.limit;
        }
        renderRunnerEventsPanel();
    }

    function renderRunnerEventsPanel() {
        const logEl = document.getElementById('runner-events-log');
        if (!logEl) return;

        logEl.innerHTML = '';

        if (!runnerEventsState.items.length) {
            const empty = document.createElement('div');
            empty.className = 'runner-events-empty';
            empty.textContent = translateOrFallback('runner_events_empty', 'Пока нет событий.');
            logEl.appendChild(empty);
            return;
        }

        runnerEventsState.items.forEach(event => {
            const entry = document.createElement('div');
            const classSuffix = event.className || 'info';
            entry.className = `runner-event runner-event--${classSuffix}`;

            const header = document.createElement('div');
            header.className = 'runner-event-header';
            header.textContent = `[${event.timeLabel}] ${event.header}`;
            entry.appendChild(header);

            const message = document.createElement('div');
            message.className = 'runner-event-message';
            message.textContent = event.message;
            entry.appendChild(message);

            if (event.details && event.details.length) {
                const detailWrap = document.createElement('div');
                detailWrap.className = 'runner-event-details';
                event.details.forEach(detail => {
                    const span = document.createElement('span');
                    span.textContent = detail;
                    detailWrap.appendChild(span);
                });
                entry.appendChild(detailWrap);
            }

            logEl.appendChild(entry);
        });

        logEl.scrollTop = 0;
    }

    function normalizeRunnerEvent(raw) {
        const payload = raw && typeof raw === 'object' ? raw : {};
        const typeKey = typeof payload.type === 'string' ? payload.type.toLowerCase() : 'info';
        const meta = RUNNER_EVENT_TYPE_META[typeKey] || RUNNER_EVENT_TYPE_META.info;
        let timestamp = payload.timestamp ? new Date(payload.timestamp) : new Date();
        if (Number.isNaN(timestamp.getTime())) timestamp = new Date();
        const cameraId = Number.isFinite(payload.cameraId) ? Number(payload.cameraId) : null;
        const cameraName = cameraId !== null ? getCameraNameForPlate(cameraId) : null;
        const message = typeof payload.message === 'string' && payload.message.trim()
            ? payload.message.trim()
            : translateOrFallback('runner_event_message_missing', 'Сообщение отсутствует');

        const headerParts = [];
        if (cameraName) headerParts.push(cameraName);
        else if (cameraId !== null) headerParts.push(`${translateOrFallback('runner_event_camera_label', 'Камера')} ${cameraId}`);
        if (payload.context) headerParts.push(String(payload.context));
        headerParts.push(translateOrFallback(meta.i18n, meta.fallback));

        return {
            className: meta.className || 'info',
            timeLabel: formatRunnerTime(timestamp),
            header: headerParts.join(' · '),
            message,
            details: buildRunnerEventDetails(payload, { cameraName, cameraId })
        };
    }

    function buildRunnerEventDetails(payload, { cameraName, cameraId }) {
        const details = [];
        if (cameraId !== null) {
            const label = translateOrFallback('runner_event_camera_label', 'Камера');
            details.push(`${label}: ${cameraName || cameraId}`);
        }
        if (payload.mode) details.push(`${translateOrFallback('runner_event_mode_label', 'Режим')}: ${payload.mode}`);
        if (payload.version) details.push(`${translateOrFallback('runner_event_runtime_version_label', 'Версия')}: ${payload.version}`);
        if (payload.context) details.push(`${translateOrFallback('runner_event_context_label', 'Контекст')}: ${payload.context}`);
        if (payload.pythonPath) details.push(`${translateOrFallback('runner_event_python_label', 'Python')}: ${formatPathTail(payload.pythonPath)}`);
        if (payload.scriptPath) details.push(`${translateOrFallback('runner_event_script_label', 'Скрипт')}: ${formatPathTail(payload.scriptPath)}`);
        if (payload.videoSource) details.push(`${translateOrFallback('runner_event_video_label', 'Видео')}: ${truncateMiddle(String(payload.videoSource), 140)}`);
        if (payload.saveDir) details.push(`${translateOrFallback('runner_event_save_label', 'Папка')}: ${formatPathTail(payload.saveDir)}`);
        if (payload.pid !== undefined && payload.pid !== null) details.push(`${translateOrFallback('runner_event_pid_label', 'PID')}: ${payload.pid}`);
        if (payload.code !== undefined && payload.code !== null) details.push(`${translateOrFallback('runner_event_exit_label', 'Код выхода')}: ${payload.code}`);
        if (payload.signal) details.push(`${translateOrFallback('runner_event_signal_label', 'Сигнал')}: ${payload.signal}`);
        if (payload.frameSkip !== undefined) details.push(`${translateOrFallback('runner_event_frameskip_label', 'Пропуск кадров')}: ${payload.frameSkip}`);
        if (payload.resizeWidth !== undefined) details.push(`${translateOrFallback('runner_event_resize_label', 'Ширина ресайза')}: ${payload.resizeWidth}`);
        if (payload.useOrt !== undefined) {
            const ortValue = payload.useOrt
                ? translateOrFallback('runner_event_ort_on', 'вкл')
                : translateOrFallback('runner_event_ort_off', 'выкл');
            details.push(`${translateOrFallback('runner_event_ort_label', 'ORT')}: ${ortValue}`);
        }
        return details;
    }

    function formatRunnerTime(date) {
        try {
            const locale = App && App.stateManager && App.stateManager.state && App.stateManager.state.appSettings && App.stateManager.state.appSettings.language;
            return date.toLocaleTimeString(locale || undefined, {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit'
            });
        } catch (e) {
            return new Date().toLocaleTimeString();
        }
    }

    function formatPathTail(value) {
        if (typeof value !== 'string' || !value.trim()) return value;
        const normalized = value.replace(/\\+/g, '/');
        const parts = normalized.split('/').filter(Boolean);
        if (parts.length <= 3) return normalized;
        return `.../${parts.slice(-3).join('/')}`;
    }

    function truncateMiddle(value, maxLength) {
        if (typeof value !== 'string') return value;
        const limit = Number.isFinite(maxLength) && maxLength > 8 ? Math.floor(maxLength) : 120;
        if (value.length <= limit) return value;
        const half = Math.floor((limit - 3) / 2);
        return `${value.slice(0, half)}...${value.slice(-half)}`;
    }

    function ensureDetectedPlateActionListener() {
        const listEl = document.getElementById('detected-plates-list');
        if (!listEl || listEl.dataset.actionsBound === '1') {
            return;
        }
        listEl.addEventListener('click', handleDetectedPlateActionClick);
        listEl.dataset.actionsBound = '1';
    }

    function handleDetectedPlateActionClick(event) {
        const actionButton = event.target && typeof event.target.closest === 'function'
            ? event.target.closest('.plate-action-btn')
            : null;
        if (!actionButton) {
            return;
        }
        event.preventDefault();
        const { action, path } = actionButton.dataset || {};
        if (!path) {
            showPlateActionFeedback(translateOrFallback('plate_action_missing', 'Файл недоступен'), true);
            return;
        }
        performDetectedPlateAction(action || 'open', path);
    }

    async function performDetectedPlateAction(action, filePath) {
        if (!window.api || typeof window.api.invoke !== 'function') {
            showPlateActionFeedback(translateOrFallback('plate_action_failed', 'Не удалось открыть файл'), true);
            return;
        }
        try {
            const response = await window.api.invoke('module-license-plate-open-path', { action, path: filePath });
            if (response && response.success === false) {
                const messageKey = response.error === 'not-found' ? 'plate_action_missing' : 'plate_action_failed';
                const base = translateOrFallback(
                    messageKey,
                    response.error === 'not-found' ? 'Файл недоступен' : 'Не удалось открыть файл'
                );
                const detail = response.error && response.error !== 'not-found' ? ` (${response.error})` : '';
                showPlateActionFeedback(`${base}${detail}`, true);
            }
        } catch (error) {
            console.error('[Renderer] Plate action invocation failed', error);
            showPlateActionFeedback(translateOrFallback('plate_action_failed', 'Не удалось открыть файл'), true);
        }
    }

    function showPlateActionFeedback(message, isError) {
        if (App && App.modalHandler && typeof App.modalHandler.showToast === 'function') {
            App.modalHandler.showToast(message, !!isError, 6000);
        } else if (isError) {
            console.error(message);
        } else {
            console.log(message);
        }
    }

    function normalizePlateEntry(raw) {
        const safe = raw || {};
        const timestamp = normalizePlateTimestamp(safe.timestamp);
        const cameraId = safe.cameraId !== undefined ? Number(safe.cameraId) : undefined;
        return {
            cameraId,
            cameraName: getCameraNameForPlate(cameraId),
            text: typeof safe.text === 'string' ? safe.text.trim() : String(safe.text || ''),
            score: normalizePlateScore(safe.score),
            path: safe.path ? String(safe.path) : null,
            timestamp,
            displayTime: formatPlateTimestamp(timestamp)
        };
    }

    function normalizePlateTimestamp(value) {
        if (!value) {
            return new Date().toISOString();
        }
        const date = value instanceof Date ? value : new Date(value);
        if (Number.isNaN(date.getTime())) {
            return new Date().toISOString();
        }
        return date.toISOString();
    }

    function buildPlateKey(entry) {
        return `${entry.timestamp}|${entry.cameraId ?? 'na'}|${entry.text}`;
    }

    function normalizePlateScore(score) {
        if (typeof score === 'number' && Number.isFinite(score)) {
            return score;
        }
        const parsed = Number(score);
        return Number.isFinite(parsed) ? parsed : null;
    }

    function formatPlateScore(score) {
        if (typeof score !== 'number' || !Number.isFinite(score)) {
            return '';
        }
        const percent = score > 1 ? score : score * 100;
        return `~${Math.round(percent)}%`;
    }

    function formatPlateTimestamp(timestampIso) {
        try {
            const date = new Date(timestampIso);
            if (Number.isNaN(date.getTime())) return '';
            const locale = App && App.stateManager && App.stateManager.state && App.stateManager.state.appSettings && App.stateManager.state.appSettings.language;
            return date.toLocaleString(locale || undefined, {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit',
                day: '2-digit',
                month: '2-digit'
            });
        } catch (e) {
            return '';
        }
    }

    function getCameraNameForPlate(cameraId) {
        try {
            if (!App || !App.stateManager) return `ID ${cameraId ?? '?'}`;
            const cameras = App.stateManager.state.cameras || [];
            const camera = cameras.find(c => Number(c.id) === Number(cameraId));
            if (!camera) return `ID ${cameraId ?? '?'}`;
            return camera.name || `ID ${cameraId}`;
        } catch (e) {
            return `ID ${cameraId ?? '?'}`;
        }
    }

    function escapeHtml(value) {
        if (value === undefined || value === null) return '';
        return String(value)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#39;');
    }
    
    document.addEventListener('DOMContentLoaded', init);
    
})(window);