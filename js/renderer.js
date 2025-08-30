// --- START OF FILE js/renderer.js ---
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
            mutations: {
                setInitialConfig(state, helpers, config) { state.cameras = config.cameras || []; state.groups = config.groups || []; if (config.layouts && config.layouts.length > 0) { state.layouts = config.layouts; state.activeLayoutId = config.activeLayoutId && config.layouts.some(l => l.id === config.activeLayoutId) ? config.activeLayoutId : config.layouts[0].id; } else if (config.gridState) { const defaultLayout = { id: Date.now(), name: 'Основной вид', gridState: config.gridState, layout: config.layout || { cols: 2, rows: 2 } }; state.layouts = [defaultLayout]; state.activeLayoutId = defaultLayout.id; App.saveConfiguration(); } else { helpers.getActiveLayout(state); } },
                setAppSettings(state, helpers, settings) { state.appSettings = { ...state.appSettings, ...settings }; App.saveAppSettings(); },
                updateGridState(state, helpers, gridState) { const activeLayout = helpers.getActiveLayout(state); if (activeLayout) { activeLayout.gridState = gridState; App.saveConfiguration(); } },
                updateGridLayout(state, helpers, layout) { const activeLayout = helpers.getActiveLayout(state); if (activeLayout) { activeLayout.layout = layout; App.saveConfiguration(); } },
                setActiveLayout(state, helpers, layoutId) { if (state.layouts.some(l => l.id === layoutId)) { state.activeLayoutId = layoutId; App.saveConfiguration(); } },
                addLayout(state, helpers, { name }) { const newLayout = { id: Date.now(), name: name, gridState: Array(64).fill(null), layout: { cols: 2, rows: 2 } }; state.layouts = [...state.layouts, newLayout]; state.activeLayoutId = newLayout.id; App.saveConfiguration(); },
                deleteLayout(state, helpers, layoutId) { if (state.layouts.length <= 1) { alert(App.t('cannot_delete_last_layout')); return; } state.layouts = state.layouts.filter(l => l.id !== layoutId); if (state.activeLayoutId === layoutId) { state.activeLayoutId = state.layouts[0].id; } App.saveConfiguration(); },
                renameLayout(state, helpers, { id, newName }) { const layoutToRename = state.layouts.find(l => l.id === id); if (layoutToRename) { layoutToRename.name = newName; App.saveConfiguration(); } },
                reorderLayouts(state, helpers, { draggedId, targetId }) { const layouts = state.layouts; const draggedIndex = layouts.findIndex(l => l.id === draggedId); const targetIndex = layouts.findIndex(l => l.id === targetId); if (draggedIndex === -1 || targetIndex === -1) return; const [draggedItem] = layouts.splice(draggedIndex, 1); layouts.splice(targetIndex, 0, draggedItem); state.layouts = [...state.layouts]; App.saveConfiguration(); },
                addCamera(state, helpers, camera) { state.cameras = [...state.cameras, { id: Date.now(), groupId: null, ...camera }]; App.saveConfiguration(); },
                updateCamera(state, helpers, updatedCamera) { state.cameras = state.cameras.map(c => c.id === updatedCamera.id ? { ...c, ...updatedCamera } : c); App.saveConfiguration(); },
                deleteCamera(state, helpers, cameraId) { state.layouts.forEach(layout => { layout.gridState = layout.gridState.map(cell => (cell && cell.camera.id === cameraId) ? null : cell); }); state.cameras = state.cameras.filter(c => c.id !== cameraId); App.saveConfiguration(); },
                addGroup(state, helpers, group) { state.groups = [...state.groups, { id: Date.now(), ...group }]; App.saveConfiguration(); },
                renameGroup(state, helpers, { id, newName }) { const groupToRename = state.groups.find(g => g.id === id); if (groupToRename) { groupToRename.name = newName; App.saveConfiguration(); } },
                deleteGroup(state, helpers, groupId) { state.cameras = state.cameras.map(camera => { if (camera.groupId === groupId) { return { ...camera, groupId: null }; } return camera; }); state.groups = state.groups.filter(g => g.id !== groupId); App.saveConfiguration(); },
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
        
        let loginView, mainAppContainer, loginBtn, loginUsername, loginPassword, loginRememberMe, loginError, logoutBtn, statusInfo, loginCloseBtn;
        
        async function loadConfiguration() { const config = await window.api.loadConfiguration(); App.stateManager.setInitialConfig(config); }
        App.saveAppSettings = async () => { await window.api.saveAppSettings(App.stateManager.state.appSettings); };
        let saveTimeout;
        App.saveConfiguration = function() {
            const state = App.stateManager.state;
            if (state.isSaving) return;
            clearTimeout(saveTimeout);
            saveTimeout = setTimeout(async () => {
                state.isSaving = true;
                console.log(`[Config] Debounced save triggered for ${App.versionType} version.`);
                const config = {
                    cameras: state.cameras.map(c => { const { player, ...rest } = c; return rest; }),
                    groups: state.groups,
                    layouts: state.layouts,
                    activeLayoutId: state.activeLayoutId,
                };
                try { 
                    await window.api.saveConfiguration(config); 
                } finally { 
                    setTimeout(() => { state.isSaving = false; }, 100); 
                }
            }, 500);
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
                const result = await window.api.login({ username, password, rememberMe });
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
        loginBtn = document.getElementById('login-btn');
        loginUsername = document.getElementById('login-username');
        loginPassword = document.getElementById('login-password');
        loginRememberMe = document.getElementById('login-remember-me');
        loginError = document.getElementById('login-error');
        logoutBtn = document.getElementById('logout-btn');
        statusInfo = document.getElementById('status-info');
        loginCloseBtn = document.getElementById('login-close-btn');

        loginBtn.addEventListener('click', handleLogin);
        loginPassword.addEventListener('keypress', (e) => { if (e.key === 'Enter') handleLogin(); });
        logoutBtn.addEventListener('click', handleLogout);
        loginCloseBtn.addEventListener('click', () => window.api.closeWindow());
        
        window.api.onMainError(({ context, message }) => {
            console.error(`[Main Process Error in ${context}]`, message);
            App.modalHandler.showToast(`${App.t('error')}: ${message}`, true, 5000);
        });
        window.api.onRecordingStateChange(({ cameraId, recording }) => App.stateManager.setRecordingState({ cameraId, recording }));
        window.api.onStreamDied(data => App.gridManager.handleStreamDeath(data));
        
        // VVVVVV --- ИЗМЕНЕНИЕ: ЭТОТ ОБРАБОТЧИК УДАЛЁН/ЗАКОММЕНТИРОВАН --- VVVVVV
        // window.api.onStreamReconnected(data => App.gridManager.handleStreamReconnect(data));
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

        window.api.onForceRender(() => App.gridManager.render());
        window.api.onStreamInfoUpdate(data => App.gridManager.updateStreamInfo(data));
        window.api.onStreamStats((data) => {
            if (App.gridManager) {
                App.gridManager.updateStreamStats(data);
            }
        });
        window.api.onAutoLoginSuccess((user) => {
            console.log('[AutoLogin] Received user data. Logging in...');
            App.stateManager.setCurrentUser(user);
            loginView.classList.add('hidden');
            mainAppContainer.classList.remove('hidden');
            loginPassword.value = '';
        });
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
        
        setInterval(updateSystemStats, 3000);
        setInterval(() => App.cameraList.pollCameraStatuses(), 10000);
        updateSystemStats();

        console.log('[DEBUG] Renderer: Sending rendererReady signal...');
        window.api.rendererReady();
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
            tab.querySelector('.close-tab-btn').addEventListener('click', e => { e.stopPropagation(); if (confirm(App.t('confirm_delete_layout'))) App.stateManager.deleteLayout(l.id); });
            tab.addEventListener('click', () => App.stateManager.setActiveLayout(l.id));
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
    
    document.addEventListener('DOMContentLoaded', init);
    
})(window);