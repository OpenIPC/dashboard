// js/renderer.js (Полная версия с добавлением обработки статуса аналитики)

(function(window) {
    'use strict';
    
    const App = {};
    window.App = App;

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
                    gridState: Array(64).fill(null), // Новая пустая сетка
                    layout: { cols: 2, rows: 2 }      // Сетка 2x2 по умолчанию
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
                    layout.gridState = layout.gridState.map(cell => {
                        if (cell && cell.camera.id === cameraId) {
                            return null;
                        }
                        return cell;
                    });
                });
                
                state.cameras = state.cameras.filter(c => c.id !== cameraId); 
                App.saveConfiguration(); 
            },
            addGroup(state, helpers, group) { 
                state.groups = [...state.groups, { id: Date.now(), ...group }]; 
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
    App.modalHandler = AppModules.createModalHandler(App);
    App.cameraList = AppModules.createCameraList(App);
    App.gridManager = AppModules.createGridManager(App);
    App.archiveManager = AppModules.createArchiveManager(App);
    App.windowControls = AppModules.createWindowControls(App);

    // VVV НОВЫЙ БЛОК: Константы для ролей VVV
    const USER_ROLES = {
        ADMIN: 'admin',
        OPERATOR: 'operator'
    };
    // ^^^ КОНЕЦ НОВОГО БЛОКА ^^^

    const loginView = document.getElementById('login-view');
    const mainAppContainer = document.getElementById('main-app-container');
    const loginBtn = document.getElementById('login-btn');
    const loginUsername = document.getElementById('login-username');
    const loginPassword = document.getElementById('login-password');
    const loginRememberMe = document.getElementById('login-remember-me');
    const loginError = document.getElementById('login-error');
    const logoutBtn = document.getElementById('logout-btn');
    const statusInfo = document.getElementById('status-info');

    async function loadConfiguration() { const config = await window.api.loadConfiguration(); App.stateManager.setInitialConfig(config); }
    async function loadAppSettings() { App.stateManager.state.appSettings = await window.api.loadAppSettings(); }
    App.saveAppSettings = async () => { await window.api.saveAppSettings(App.stateManager.state.appSettings); };
    
    async function saveConfiguration() {
        const state = App.stateManager.state;
        if (state.isSaving) return;
        state.isSaving = true;
        
        const config = {
            cameras: state.cameras.map(c => { const { player, ...rest } = c; return rest; }),
            groups: state.groups,
            layouts: state.layouts,
            activeLayoutId: state.activeLayoutId,
        };
        try { await window.api.saveConfiguration(config); } finally { setTimeout(() => { state.isSaving = false; }, 100); }
    }
    App.saveConfiguration = saveConfiguration;

    async function toggleRecording(camera) {
        if (App.stateManager.state.recordingStates[camera.id]) { 
            await window.api.stopRecording(camera.id); 
        } 
        else { 
            const fullCameraInfo = App.stateManager.state.cameras.find(c => c.id === camera.id);
            await window.api.startRecording(fullCameraInfo); 
        }
    }
    App.toggleRecording = toggleRecording;

    function updateSystemStats() { window.api.getSystemStats().then(stats => { statusInfo.textContent = `${App.t('status_cpu')}: ${stats.cpu}% | ${App.t('status_ram')}: ${stats.ram} MB`; }); }

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

    window.api.onRecordingStateChange(({ cameraId, recording }) => App.stateManager.setRecordingState({ cameraId, recording }));
    window.api.onStreamDied(uniqueStreamIdentifier => App.gridManager.handleStreamDeath(uniqueStreamIdentifier));
    window.api.onStreamStats(({ uniqueStreamIdentifier, fps, bitrate }) => { const statsDiv = document.getElementById(`stats-${uniqueStreamIdentifier}`); if(statsDiv) statsDiv.textContent = `${Math.round(fps)}fps, ${Math.round(bitrate)}kbps`; });

    async function init() {
        await loadAppSettings();
        await App.i18n.init();
        App.t = App.i18n.t;

        window.api.onAutoLoginSuccess((user) => {
            console.log('[AutoLogin] Received user data from main process. Logging in...');
            App.stateManager.setCurrentUser(user);
            loginView.classList.add('hidden');
            mainAppContainer.classList.remove('hidden');
            loginPassword.value = '';
        });

        window.api.onAnalyticsStatusChange(({ cameraId, active }) => {
            const btn = document.getElementById(`analytics-btn-${cameraId}`);
            if (btn) {
                btn.classList.toggle('active', active);
                btn.querySelector('i').style.color = ''; // Сбрасываем цвет "запуска"
            }
        });

        App.modalHandler.init();
        App.cameraList.init();
        App.gridManager.init();
        App.archiveManager.init();
        App.windowControls.init();
        initPresentationMode();

        loginBtn.addEventListener('click', handleLogin);
        loginPassword.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') handleLogin();
        });
        logoutBtn.addEventListener('click', handleLogout);
        
        const saveLayoutBtn = document.getElementById('save-layout-btn');
        const deleteLayoutBtn = document.getElementById('delete-layout-btn');
        const renameLayoutBtn = document.getElementById('rename-layout-btn');
        const layoutTabsContainer = document.querySelector('.header .tabs');
        const addLayoutBtn = document.getElementById('add-layout-btn');

        function renderLayoutTabs() {
            const { layouts, activeLayoutId } = App.stateManager.state;
            layoutTabsContainer.innerHTML = '';
            
            if (!layouts) return;

            layouts.forEach(l => {
                const tab = document.createElement('button');
                tab.className = 'tab';
                if (l.id === activeLayoutId) {
                    tab.classList.add('active');
                }
                tab.dataset.layoutId = l.id;
                
                tab.draggable = true; 

                const tabName = document.createElement('span');
                tabName.textContent = l.name;
                tab.appendChild(tabName);

                const closeBtn = document.createElement('span');
                closeBtn.className = 'close-tab-btn';
                closeBtn.innerHTML = '×';
                closeBtn.addEventListener('click', (e) => {
                    e.stopPropagation();
                    if (confirm(App.t('confirm_delete_layout'))) {
                        App.stateManager.deleteLayout(l.id);
                    }
                });
                tab.appendChild(closeBtn);

                tab.addEventListener('click', () => {
                    App.stateManager.setActiveLayout(l.id);
                });

                tab.addEventListener('dragstart', (e) => {
                    e.dataTransfer.setData('application/x-layout-id', l.id);
                    e.dataTransfer.effectAllowed = 'move';
                    tab.classList.add('dragging');
                });
                tab.addEventListener('dragend', () => tab.classList.remove('dragging'));
                tab.addEventListener('dragover', (e) => {
                    e.preventDefault();
                    tab.classList.add('drag-over');
                });
                tab.addEventListener('dragleave', () => tab.classList.remove('drag-over'));
                tab.addEventListener('drop', (e) => {
                    e.preventDefault();
                    tab.classList.remove('drag-over');
                    const draggedId = Number(e.dataTransfer.getData('application/x-layout-id'));
                    const targetId = l.id;
                    if (draggedId !== targetId) {
                        App.stateManager.reorderLayouts({ draggedId, targetId });
                    }
                });

                layoutTabsContainer.appendChild(tab);
            });
        }
        
        addLayoutBtn.addEventListener('click', async () => {
            const layoutCount = App.stateManager.state.layouts ? App.stateManager.state.layouts.length : 0;
            const name = await App.modalHandler.showPrompt({
                title: App.t('add_layout_tooltip'),
                label: App.t('enter_layout_name_prompt'),
                defaultValue: App.t('new_layout_default_name', { count: layoutCount + 1 })
            });
            if (name && name.trim()) {
                App.stateManager.addLayout({ name: name.trim() });
            }
        });
        
        saveLayoutBtn.addEventListener('click', async () => {
            const layoutCount = App.stateManager.state.layouts ? App.stateManager.state.layouts.length : 0;
            const name = await App.modalHandler.showPrompt({
                title: App.t('save_layout_tooltip'),
                label: App.t('enter_layout_name_prompt'),
                defaultValue: `View ${layoutCount + 1}`
            });
            if (name && name.trim()) {
                App.stateManager.saveLayout({ name: name.trim() });
            }
        });

        renameLayoutBtn.addEventListener('click', async () => {
            const activeLayout = App.stateManager.state.layouts.find(l => l.id === App.stateManager.state.activeLayoutId);
            if (!activeLayout) return;
            
            const newName = await App.modalHandler.showPrompt({
                title: App.t('rename_layout_tooltip'),
                label: App.t('enter_new_layout_name'),
                defaultValue: activeLayout.name
            });
            if (newName && newName.trim() && newName.trim() !== activeLayout.name) {
                App.stateManager.renameLayout({ id: activeLayout.id, newName: newName.trim() });
            }
        });

        deleteLayoutBtn.addEventListener('click', () => {
            if (confirm(App.t('confirm_delete_layout'))) {
                const activeId = App.stateManager.state.activeLayoutId;
                App.stateManager.deleteLayout(activeId);
            }
        });

        let renderTimeout;
        App.stateManager.subscribe(() => {
            clearTimeout(renderTimeout);
            renderTimeout = setTimeout(() => {
                console.log("[Renderer] State change detected. Triggering re-render.");
                App.cameraList.render();
                App.gridManager.render();
            }, 50);

            App.gridManager.updateGridLayoutView(); 
            renderLayoutTabs();

            const user = App.stateManager.state.currentUser;
            document.body.className = document.body.className.replace(/role-\w+|can-\w+/g, '').trim();

            if (user) {
                document.body.classList.add(`role-${user.role}`);
                // VVV ИЗМЕНЕНИЕ: Используем константу VVV
                if (user.role === USER_ROLES.OPERATOR && user.permissions) {
                // ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^
                    for (const permission in user.permissions) {
                        if (user.permissions[permission]) {
                            document.body.classList.add(`can-${permission.replace(/_/g, '-')}`);
                        }
                    }
                }
            }
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

    init();

    (function() {
        const updateStatusInfo = document.createElement('div');
        updateStatusInfo.style.marginLeft = '15px'; updateStatusInfo.style.fontSize = '12px'; updateStatusInfo.style.color = 'var(--text-secondary)';
        const statusBar = document.getElementById('status-info').parentElement;
        if (statusBar) { statusBar.appendChild(updateStatusInfo); }
        window.api.onUpdateStatus(({ status, message }) => {
            const version = message.includes(' ') ? message.split(' ').pop() : '';
            switch (status) {
                case 'available': updateStatusInfo.innerHTML = `💡 <span style="text-decoration: underline; cursor: help;" title="${App.t('update_available', { version })}">${App.t('update_available_short')}</span>`; updateStatusInfo.style.color = '#ffc107'; break;
                case 'downloading': updateStatusInfo.textContent = `⏳ ${App.t('update_downloading', { percent: message.match(/\d+/)[0] })}`; updateStatusInfo.style.color = '#17a2b8'; break;
                case 'downloaded': updateStatusInfo.innerHTML = `✅ <span style="text-decoration: underline; cursor: help;" title="${App.t('update_downloaded')}">${App.t('update_downloaded_short')}</span>`; updateStatusInfo.style.color = '#28a745'; break;
                case 'error': updateStatusInfo.textContent = `❌ ${App.t('update_error_short', { message })}`; updateStatusInfo.style.color = '#dc3545'; break;
                case 'latest': updateStatusInfo.textContent = `👍 ${App.t('update_latest')}`; setTimeout(() => { if (updateStatusInfo.textContent.includes(App.t('update_latest'))) updateStatusInfo.textContent = ''; }, 5000); break;
                default: updateStatusInfo.textContent = ''; break;
            }
        });
    })();
})(window);