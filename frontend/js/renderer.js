(function(window) {
    'use strict';
    
    let App;

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
        setInterval(updatePlatesDisplay, 5000); // Обновлять список номеров каждые 5 секунд
        updateSystemStats();
        updatePlatesDisplay(); // Первоначальное обновление списка номеров

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

    function updatePlatesDisplay() {
        const platesContainer = document.getElementById('detected-plates-list');
        if (!platesContainer) return;

        window.api.getDetectedPlates().then(plates => {
            platesContainer.innerHTML = '';
            
            if (!plates || !plates.recentHistory || plates.recentHistory.length === 0) {
                platesContainer.innerHTML = '<div class="no-plates">Нет обнаруженных номеров</div>';
                return;
            }

            plates.recentHistory.forEach(plate => {
                const plateItem = document.createElement('div');
                plateItem.className = 'plate-item';
                plateItem.innerHTML = `
                    <div class="plate-text">${plate.text}</div>
                    <div class="plate-timestamp">${new Date(plate.timestamp).toLocaleString()}</div>
                `;
                platesContainer.appendChild(plateItem);
            });
        }).catch(error => {
            console.error('Error updating plates display:', error);
            platesContainer.innerHTML = '<div class="no-plates">Ошибка загрузки номеров</div>';
        });
    }
    
    document.addEventListener('DOMContentLoaded', init);
    
})(window);