(function(window) {
    'use strict';
    
    const App = {};
    window.App = App;

    App.USER_ROLES = {
        ADMIN: 'admin',
        OPERATOR: 'operator'
    };

    App.stateManager = AppModules.createStateManager({
        initialState: {
            cameras: [],
            groups: [],
            layouts: [],
            activeLayoutId: null,
            recordingStates: {},
            appSettings: {},
            isSaving: false,
            currentUser: null,
        },
        mutations: {
            setInitialConfig(state, helpers, config) { 
                state.cameras = config.cameras || []; 
                state.groups = config.groups || []; 

                if (config.layouts && config.layouts.length > 0) {
                    state.layouts = config.layouts;
                    state.activeLayoutId = config.activeLayoutId && config.layouts.some(l => l.id === config.activeLayoutId)
                        ? config.activeLayoutId
                        : config.layouts[0].id;
                } else if (config.gridState) {
                    console.log("[State] Migrating old config format to new layout structure.");
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
            },
            setAppSettings(state, helpers, settings) { 
                state.appSettings = { ...state.appSettings, ...settings }; 
                App.saveAppSettings(); 
            },
            updateGridState(state, helpers, gridState) { 
                const activeLayout = helpers.getActiveLayout(state);
                if (activeLayout) {
                    activeLayout.gridState = gridState;
                    App.saveConfiguration(); 
                }
            },
            updateGridLayout(state, helpers, layout) { 
                const activeLayout = helpers.getActiveLayout(state);
                if (activeLayout) {
                    activeLayout.layout = layout;
                    App.saveConfiguration();
                }
            },
            setActiveLayout(state, helpers, layoutId) {
                if (state.layouts.some(l => l.id === layoutId)) {
                    state.activeLayoutId = layoutId;
                    App.saveConfiguration();
                }
            },
            saveLayout(state, helpers, { name }) {
                const activeLayout = helpers.getActiveLayout(state);
                const newLayout = {
                    ...JSON.parse(JSON.stringify(activeLayout)),
                    id: Date.now(),
                    name: name
                };
                state.layouts = [...state.layouts, newLayout];
                state.activeLayoutId = newLayout.id;
                App.saveConfiguration();
            },
            addLayout(state, helpers, { name }) {
                const newLayout = {
                    id: Date.now(),
                    name: name,
                    gridState: Array(64).fill(null),
                    layout: { cols: 2, rows: 2 }
                };
                state.layouts = [...state.layouts, newLayout];
                state.activeLayoutId = newLayout.id;
                App.saveConfiguration();
            },
            deleteLayout(state, helpers, layoutId) {
                if (state.layouts.length <= 1) {
                    alert(App.t('cannot_delete_last_layout'));
                    return;
                }
                state.layouts = state.layouts.filter(l => l.id !== layoutId);
                if (state.activeLayoutId === layoutId) {
                    state.activeLayoutId = state.layouts[0].id;
                }
                App.saveConfiguration();
            },
            renameLayout(state, helpers, { id, newName }) {
                const layoutToRename = state.layouts.find(l => l.id === id);
                if (layoutToRename) {
                    layoutToRename.name = newName;
                    App.saveConfiguration();
                }
            },
            reorderLayouts(state, helpers, { draggedId, targetId }) {
                const layouts = state.layouts;
                const draggedIndex = layouts.findIndex(l => l.id === draggedId);
                const targetIndex = layouts.findIndex(l => l.id === targetId);
                if (draggedIndex === -1 || targetIndex === -1) return;
                const [draggedItem] = layouts.splice(draggedIndex, 1);
                layouts.splice(targetIndex, 0, draggedItem);
                state.layouts = [...layouts];
                App.saveConfiguration();
            },
            addCamera(state, helpers, camera) { 
                state.cameras = [...state.cameras, { id: Date.now(), groupId: null, ...camera }]; 
                App.saveConfiguration(); 
            },
            updateCamera(state, helpers, updatedCamera) { 
                state.cameras = state.cameras.map(c => c.id === updatedCamera.id ? { ...c, ...updatedCamera } : c); 
                App.saveConfiguration(); 
            },
            deleteCamera(state, helpers, cameraId) {
                state.layouts.forEach(layout => {
                    layout.gridState = layout.gridState.map(cell => (cell && cell.camera.id === cameraId) ? null : cell);
                });
                state.cameras = state.cameras.filter(c => c.id !== cameraId); 
                App.saveConfiguration(); 
            },
            addGroup(state, helpers, group) { 
                state.groups = [...state.groups, { id: Date.now(), ...group }]; 
                App.saveConfiguration(); 
            },
            renameGroup(state, helpers, { id, newName }) {
                const groupToRename = state.groups.find(g => g.id === id);
                if (groupToRename) {
                    groupToRename.name = newName;
                    App.saveConfiguration();
                }
            },
            deleteGroup(state, helpers, groupId) {
                state.cameras = state.cameras.map(camera => {
                    if (camera.groupId === groupId) {
                        return { ...camera, groupId: null };
                    }
                    return camera;
                });
                state.groups = state.groups.filter(g => g.id !== groupId);
                App.saveConfiguration();
            },
            setRecordingState(state, helpers, { cameraId, recording }) { 
                state.recordingStates = { ...state.recordingStates, [cameraId]: recording }; 
            },
            setCurrentUser(state, helpers, user) {
                state.currentUser = user;
            },
            logout(state, helpers) {
                state.currentUser = null;
            }
        }
    });
    
    App.t = (key, replacements) => key;
    
    App.i18n = AppModules.createI18n(App);
    
    let loginView, mainAppContainer, loginBtn, loginUsername, loginPassword,
        loginRememberMe, loginError, logoutBtn, statusInfo, loginCloseBtn;

    async function loadConfiguration() { const config = await window.api.loadConfiguration(); App.stateManager.setInitialConfig(config); }
    async function loadAppSettings() { App.stateManager.state.appSettings = await window.api.loadAppSettings(); }
    App.saveAppSettings = async () => { await window.api.saveAppSettings(App.stateManager.state.appSettings); };

    let saveTimeout;

    async function saveConfiguration() {
        const state = App.stateManager.state;
        if (state.isSaving) return;

        clearTimeout(saveTimeout);

        saveTimeout = setTimeout(async () => {
            state.isSaving = true;
            console.log('[Config] Debounced save triggered. Writing to disk...');
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
    }

    App.saveConfiguration = saveConfiguration;
    async function toggleRecording(camera) {
        if (App.stateManager.state.recordingStates[camera.id]) { 
            await window.api.stopRecording(camera.id); 
        } else { 
            const fullCameraInfo = App.stateManager.state.cameras.find(c => c.id === camera.id);
            await window.api.startRecording(fullCameraInfo); 
        }
    }
    App.toggleRecording = toggleRecording;
    function updateSystemStats() {
        window.api.getSystemStats().then(stats => {
            if (statusInfo) {
                statusInfo.textContent = `${App.t('status_cpu')}: ${stats.cpu}% | ${App.t('status_ram')}: ${stats.ram} MB`;
            }
        });
    }

    function initPresentationMode() {
        const presentationBtn = document.getElementById('presentation-mode-btn');
        presentationBtn.addEventListener('click', () => {
            document.body.classList.toggle('presentation-mode');
            setTimeout(() => window.dispatchEvent(new Event('resize')), 50); 
        });
        window.addEventListener('keydown', (e) => {
            if (e.key === 'Escape' && document.body.classList.contains('presentation-mode')) {
                document.body.classList.remove('presentation-mode');
                setTimeout(() => window.dispatchEvent(new Event('resize')), 50);
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
        loginUsername.focus();
    }
    
    async function init() {
        try {
            const response = await fetch('./templates.html');
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            const templatesHTML = await response.text();
            
            const templateContainer = document.createElement('div');
            templateContainer.innerHTML = templatesHTML;
            
            templateContainer.querySelectorAll('template').forEach(template => {
                const content = document.importNode(template.content, true);
                document.body.appendChild(content);
            });
        } catch (error) {
            console.error('Failed to load UI templates:', error);
            alert('Критическая ошибка: Не удалось загрузить шаблоны интерфейса. Приложение не будет работать корректно.');
            return;
        }
        
        await loadAppSettings();
        await App.i18n.init();
        App.t = App.i18n.t;

        App.modalHandler = AppModules.createModalHandler(App);
        App.cameraList = AppModules.createCameraList(App);
        App.gridManager = AppModules.createGridManager(App);
        App.archiveManager = AppModules.createArchiveManager(App);
        App.windowControls = AppModules.createWindowControls(App);

        loginView = document.getElementById('login-view');
        mainAppContainer = document.getElementById('main-app-container');
        loginBtn = document.getElementById('login-btn');
        loginUsername = document.getElementById('login-username');
        loginPassword = document.getElementById('login-password');
        loginRememberMe = document.getElementById('login-remember-me');
        loginError = document.getElementById('login-error');
        logoutBtn = document.getElementById('logout-btn');
        statusInfo = document.getElementById('status-info');
        
        // VVVVVV --- ИЗМЕНЕНИЕ ЗДЕСЬ (1/2) --- VVVVVV
        loginCloseBtn = document.getElementById('login-close-btn');
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

        App.modalHandler.init();
        App.cameraList.init();
        App.gridManager.init();
        App.archiveManager.init();
        App.windowControls.init();
        initPresentationMode();

        loginBtn.addEventListener('click', handleLogin);
        loginPassword.addEventListener('keypress', (e) => { if (e.key === 'Enter') handleLogin(); });
        logoutBtn.addEventListener('click', handleLogout);
        
        // VVVVVV --- ИЗМЕНЕНИЕ ЗДЕСЬ (2/2) --- VVVVVV
        loginCloseBtn.addEventListener('click', () => window.api.closeWindow());
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
        
        initLayoutControls();
        
        window.api.onMainError(({ context, message }) => {
            console.error(`[Main Process Error in ${context}]`, message);
            App.modalHandler.showToast(`${App.t('error')}: ${message}`, true, 5000);
        });
        
        window.api.onRecordingStateChange(({ cameraId, recording }) => App.stateManager.setRecordingState({ cameraId, recording }));
        window.api.onStreamDied(uniqueStreamIdentifier => App.gridManager.handleStreamDeath(uniqueStreamIdentifier));
        
        window.api.onStreamStats((data) => {
            if (App.gridManager) {
                App.gridManager.updateStreamStats(data);
            }
        });

        window.api.onAutoLoginSuccess((user) => {
            console.log('[AutoLogin] Received user data from main process. Logging in...');
            App.stateManager.setCurrentUser(user);
            loginView.classList.add('hidden');
            mainAppContainer.classList.remove('hidden');
            loginPassword.value = '';
        });
        window.api.onAnalyticsStatusChange(({ cameraId, active }) => {
            const btn = document.getElementById(`analytics-btn-${cameraId}`);
            if (btn) btn.classList.toggle('active', active);
        });
        window.api.onAnalyticsProviderInfo(({ cameraId, provider, error }) => {
            const camera = App.stateManager.state.cameras.find(c => c.id === cameraId);
            const cameraName = camera ? camera.name : `ID ${cameraId}`;
            if (error) {
                App.modalHandler.showToast(`Ошибка аналитики для "${cameraName}": ${error}`, true, 6000);
                return;
            }
            if (provider) {
                const isGpu = provider.includes('CUDA') || provider.includes('Dml');
                const message = isGpu
                    ? `Аналитика для "${cameraName}": используется GPU (${provider.includes('CUDA') ? 'NVIDIA CUDA' : 'DirectML'}).`
                    : `Аналитика для "${cameraName}": используется CPU (GPU недоступен).`;
                App.modalHandler.showToast(message, !isGpu, 5000);
            }
        });

        App.stateManager.subscribe(() => {
            renderLayoutTabs();
            updateUserPermissionsUI();
            
            setTimeout(() => {
                App.cameraList.render();
                App.gridManager.render();
                App.gridManager.updateGridLayoutView();
            }, 50);
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

        window.api.rendererReady();
    }

    function initLayoutControls() {
        document.getElementById('add-layout-btn').addEventListener('click', async () => {
            const name = await App.modalHandler.showPrompt({
                title: App.t('add_layout_tooltip'),
                label: App.t('enter_layout_name_prompt'),
                defaultValue: App.t('new_layout_default_name', { count: (App.stateManager.state.layouts?.length || 0) + 1 })
            });
            if (name?.trim()) App.stateManager.addLayout({ name: name.trim() });
        });
        document.getElementById('save-layout-btn').addEventListener('click', async () => {
            const name = await App.modalHandler.showPrompt({
                title: App.t('save_layout_tooltip'),
                label: App.t('enter_layout_name_prompt'),
                defaultValue: `View ${(App.stateManager.state.layouts?.length || 0) + 1}`
            });
            if (name?.trim()) App.stateManager.saveLayout({ name: name.trim() });
        });
        document.getElementById('rename-layout-btn').addEventListener('click', async () => {
            const activeLayout = App.stateManager.state.layouts.find(l => l.id === App.stateManager.state.activeLayoutId);
            if (!activeLayout) return;
            const newName = await App.modalHandler.showPrompt({
                title: App.t('rename_layout_tooltip'),
                label: App.t('enter_new_layout_name'),
                defaultValue: activeLayout.name
            });
            if (newName?.trim() && newName.trim() !== activeLayout.name) {
                App.stateManager.renameLayout({ id: activeLayout.id, newName: newName.trim() });
            }
        });
        document.getElementById('delete-layout-btn').addEventListener('click', () => {
            if (confirm(App.t('confirm_delete_layout'))) {
                App.stateManager.deleteLayout(App.stateManager.state.activeLayoutId);
            }
        });
    }

    function renderLayoutTabs() {
        const layoutTabsContainer = document.querySelector('.header .tabs');
        const { layouts, activeLayoutId } = App.stateManager.state;
        layoutTabsContainer.innerHTML = '';
        if (!layouts) return;
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
            tab.addEventListener('drop', e => {
                e.preventDefault();
                tab.classList.remove('drag-over');
                const draggedId = Number(e.dataTransfer.getData('application/x-layout-id'));
                if (draggedId && draggedId !== l.id) App.stateManager.reorderLayouts({ draggedId, targetId: l.id });
            });
            layoutTabsContainer.appendChild(tab);
        });
    }

    function updateUserPermissionsUI() {
        const user = App.stateManager.state.currentUser;
        document.body.className = document.body.className.replace(/role-\w+|can-\w+/g, '').trim();
        if (user) {
            document.body.classList.add(`role-${user.role}`);
            if (user.role === App.USER_ROLES.OPERATOR && user.permissions) {
                Object.keys(user.permissions).forEach(permission => {
                    if (user.permissions[permission]) {
                        document.body.classList.add(`can-${permission.replace(/_/g, '-')}`);
                    }
                });
            }
        }
    }
    
    init();

    (function() {
        const updateStatusInfo = document.createElement('div');
        updateStatusInfo.style.cssText = 'margin-left: 15px; font-size: 12px; color: var(--text-secondary);';
        const statusBar = document.getElementById('status-info')?.parentElement;
        if (statusBar) statusBar.appendChild(updateStatusInfo);
        
        window.api.onUpdateStatus((data) => {
            if (typeof data !== 'object' || data === null || typeof data.status === 'undefined') {
                return;
            }
            const { status, message } = data;
            const version = (message && message.includes(' ')) ? message.split(' ').pop() : '';
            switch (status) {
                case 'available': updateStatusInfo.innerHTML = `💡 <span style="text-decoration: underline; cursor: help;" title="${App.t('update_available', { version })}">${App.t('update_available_short')}</span>`; updateStatusInfo.style.color = '#ffc107'; break;
                case 'downloading': updateStatusInfo.textContent = `⏳ ${App.t('update_downloading', { percent: message.match(/\d+/)?.[0] || '0' })}`; updateStatusInfo.style.color = '#17a2b8'; break;
                case 'downloaded': updateStatusInfo.innerHTML = `✅ <span style="text-decoration: underline; cursor: help;" title="${App.t('update_downloaded')}">${App.t('update_downloaded_short')}</span>`; updateStatusInfo.style.color = '#28a745'; break;
                case 'error': updateStatusInfo.textContent = `❌ ${App.t('update_error_short', { message })}`; updateStatusInfo.style.color = '#dc3545'; break;
                case 'latest': updateStatusInfo.textContent = `👍 ${App.t('update_latest')}`; setTimeout(() => { if (updateStatusInfo.textContent.includes(App.t('update_latest'))) updateStatusInfo.textContent = ''; }, 5000); break;
                default: updateStatusInfo.textContent = ''; break;
            }
        });
    })();

})(window);