// js/camera-list.js (Полная версия с изменениями для системы пользователей и видеоаналитики)

(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createCameraList = function(App) {
        const stateManager = App.stateManager;
        const cameraListContainer = document.getElementById('camera-list-container');
        const openRecordingsBtn = document.getElementById('open-recordings-btn');

        // VVV ИЗМЕНЕНИЕ: Параллельный опрос статусов VVV
        // Эта функция теперь опрашивает все камеры одновременно, а не по очереди.
        // Это предотвращает задержки в обновлении UI, если одна из камер медленно отвечает.
        async function pollCameraStatuses() {
            const cameras = stateManager.state.cameras;
            const statusPromises = cameras.map(async (camera) => {
                const statusIcon = document.getElementById(`status-icon-${camera.id}`);
                if (statusIcon) {
                    try {
                        const pulse = await window.api.getCameraPulse(camera);
                        // Обновляем иконку сразу, как только получаем ответ от конкретной камеры
                        statusIcon.classList.toggle('online', pulse.success);
                    } catch (e) {
                        statusIcon.classList.remove('online');
                    }
                }
            });

            // Ждём, пока все запросы не будут выполнены, чтобы функция завершилась корректно
            await Promise.all(statusPromises);
        }
        // ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^

        async function deleteCamera(cameraId) {
            const confirmation = await App.modalHandler.showPrompt({
                title: App.i18n.t('context_delete'),
                label: App.i18n.t('confirm_delete_camera'),
                okText: App.i18n.t('context_delete'),
                cancelText: App.i18n.t('cancel'),
                inputType: 'none'
            });

            if (confirmation) {
                if (stateManager.state.recordingStates[cameraId]) {
                    await window.api.stopRecording(cameraId);
                }
                const analyticsBtn = document.getElementById(`analytics-btn-${cameraId}`);
                if (analyticsBtn && analyticsBtn.classList.contains('active')) {
                    await window.api.toggleAnalytics(cameraId);
                }
                stateManager.deleteCamera(cameraId);
            }
        }

        // VVVVVV --- НОВЫЕ ФУНКЦИИ ДЛЯ УПРАВЛЕНИЯ ГРУППАМИ --- VVVVVV
        async function renameGroup(groupId) {
            const group = stateManager.state.groups.find(g => g.id === groupId);
            if (!group) return;

            const newName = await App.modalHandler.showPrompt({
                title: App.i18n.t('context_rename_group'),
                label: App.i18n.t('enter_new_group_name'),
                defaultValue: group.name,
                okText: App.i18n.t('save'),
                cancelText: App.i18n.t('cancel')
            });

            if (newName && newName.trim() !== '' && newName.trim() !== group.name) {
                stateManager.renameGroup({ id: groupId, newName: newName.trim() });
            }
        }

        async function deleteGroup(groupId) {
            const group = stateManager.state.groups.find(g => g.id === groupId);
            if (!group) return;

            const confirmation = await App.modalHandler.showPrompt({
                title: App.i18n.t('context_delete_group'),
                label: App.i18n.t('confirm_delete_group', { groupName: group.name }) + '\n' + App.i18n.t('confirm_delete_group_detail'),
                okText: App.i18n.t('context_delete'),
                cancelText: App.i18n.t('cancel'),
                inputType: 'none'
            });

            if (confirmation) {
                stateManager.deleteGroup(groupId);
            }
        }
        // ^^^^^^ --- КОНЕЦ НОВЫХ ФУНКЦИЙ --- ^^^^^^

        function render() {
            cameraListContainer.innerHTML = '';
            const { cameras, groups, recordingStates } = stateManager.state;
        
            const createGroupHTML = (group, camerasInGroup) => {
                const groupContainer = document.createElement('div');
                groupContainer.className = 'group-container';
        
                const groupHeader = document.createElement('div');
                groupHeader.className = 'group-header';
                groupHeader.innerHTML = `<i class="material-icons toggle-icon">arrow_drop_down</i><span class="group-name">${group.name}</span>`;
        
                // VVVVVV --- ИЗМЕНЕНИЕ ЗДЕСЬ --- VVVVVV
                // Добавляем обработчик правого клика только для реальных групп (не для "Камеры без группы")
                if (group.id !== null) {
                    groupHeader.addEventListener('contextmenu', (e) => {
                        e.preventDefault();
                        
                        const currentUser = App.stateManager.state.currentUser;
                        if (currentUser?.role !== 'admin' && !currentUser.permissions?.edit_cameras) {
                            return;
                        }

                        window.api.showGroupContextMenu({
                            groupId: group.id,
                            labels: {
                                rename: App.i18n.t('context_rename_group'),
                                delete: App.i18n.t('context_delete_group')
                            }
                        });
                    });
                }
                // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

                const groupCamerasList = document.createElement('div');
                groupCamerasList.className = 'group-cameras';
        
                camerasInGroup.forEach(camera => {
                    const cameraItem = document.createElement('div');
                    cameraItem.className = 'camera-item';
                    cameraItem.dataset.cameraId = camera.id;
                    cameraItem.draggable = App.stateManager.state.currentUser?.role === 'admin';
                    
                    cameraItem.innerHTML = `
                        <i class="status-icon" id="status-icon-${camera.id}"></i>
                        <span style="flex-grow: 1;">${camera.name}</span>
                        <div class="rec-indicator"></div>
                        <button class="analytics-btn icon-button" id="analytics-btn-${camera.id}" title="Toggle Analytics">
                            <i class="material-icons" style="font-size: 18px;">insights</i>
                        </button>
                    `;

                    if (recordingStates[camera.id]) {
                        cameraItem.classList.add('recording');
                    }
                    cameraItem.addEventListener('dragstart', (e) => { e.dataTransfer.setData('text/plain', camera.id.toString()); });
                    groupCamerasList.appendChild(cameraItem);

                    const analyticsBtn = cameraItem.querySelector('.analytics-btn');
                    if (analyticsBtn) {
                        analyticsBtn.disabled = false;
                        analyticsBtn.title = App.i18n.t('toggle_analytics_tooltip');

                        analyticsBtn.addEventListener('click', async (e) => {
                            e.stopPropagation();
                            const btnIcon = analyticsBtn.querySelector('i');
                            btnIcon.style.color = '#ffc107';
                            await window.api.toggleAnalytics(camera.id);
                        });
                    }
                });
        
                groupContainer.appendChild(groupHeader);
                groupContainer.appendChild(groupCamerasList);
        
                groupHeader.addEventListener('click', () => {
                    groupHeader.querySelector('.toggle-icon').classList.toggle('collapsed');
                    groupCamerasList.classList.toggle('collapsed');
                });
        
                if (group.id !== null) {
                     groupHeader.addEventListener('dragover', (e) => { e.preventDefault(); groupHeader.style.backgroundColor = 'var(--accent-color)'; });
                     groupHeader.addEventListener('dragleave', (e) => { groupHeader.style.backgroundColor = ''; });
                     groupHeader.addEventListener('drop', (e) => {
                        e.preventDefault();
                        groupHeader.style.backgroundColor = '';
                        const cameraId = parseInt(e.dataTransfer.getData('text/plain'), 10);
                        const camera = cameras.find(c => c.id === cameraId);
                        if (camera && camera.groupId !== group.id) {
                            stateManager.updateCamera({ ...camera, groupId: group.id });
                        }
                    });
                }
        
                return groupContainer;
            };

            groups.forEach(group => {
                const camerasInGroup = cameras.filter(c => c.groupId === group.id);
                cameraListContainer.appendChild(createGroupHTML(group, camerasInGroup));
            });

            const ungroupedCameras = cameras.filter(c => !c.groupId);
            if (ungroupedCameras.length > 0) {
                const ungroupedPseudoGroup = { id: null, name: App.i18n.t('ungrouped_cameras') };
                cameraListContainer.appendChild(createGroupHTML(ungroupedPseudoGroup, ungroupedCameras));
            }

            if (cameraListContainer.innerHTML === '') {
                cameraListContainer.innerHTML = `<p style="padding: 10px; color: var(--text-secondary);">${App.i18n.t('no_cameras_or_groups')}</p>`;
            }

            pollCameraStatuses();
        }

        function init() {
            openRecordingsBtn.addEventListener('click', () => window.api.openRecordingsFolder());
            
            cameraListContainer.addEventListener('contextmenu', (e) => {
                const currentUser = App.stateManager.state.currentUser;
                if (currentUser?.role !== 'admin' && !(currentUser.permissions?.edit_cameras || currentUser.permissions?.delete_cameras || currentUser.permissions?.access_settings || currentUser.permissions?.view_archive)) {
                    e.preventDefault();
                    return;
                }

                const cameraItem = e.target.closest('.camera-item');
                if (cameraItem) {
                    e.preventDefault();
                    const cameraId = parseInt(cameraItem.dataset.cameraId, 10);
                    const menuItems = {};
                    
                    menuItems.open_in_browser = `🌐  ${App.i18n.t('context_open_in_browser')}`;
                    menuItems.files = `🗂️  ${App.i18n.t('context_file_manager')}`;
                    menuItems.ssh = `💻  ${App.i18n.t('context_ssh')}`;

                    if (currentUser.role === 'admin' || currentUser.permissions?.view_archive) {
                        menuItems.archive = `🗄️  ${App.i18n.t('archive_title')}`;
                    }
                    if (currentUser.role === 'admin' || currentUser.permissions?.access_settings) {
                        menuItems.settings = `⚙️  ${App.i18n.t('context_settings')}`;
                    }
                    if (currentUser.role === 'admin' || currentUser.permissions?.edit_cameras) {
                        menuItems.edit = `✏️  ${App.i18n.t('context_edit')}`;
                    }
                    if (currentUser.role === 'admin' || currentUser.permissions?.delete_cameras) {
                        menuItems.delete = `🗑️  ${App.i18n.t('context_delete')}`;
                    }

                    window.api.showCameraContextMenu({ cameraId, labels: menuItems });
                }
            });

            window.api.onContextMenuCommand(({ command, cameraId }) => {
                const camera = stateManager.state.cameras.find(c => c.id === cameraId);
                if (!camera) return;

                const cameraDataForIPC = {
                    id: camera.id,
                    name: camera.name,
                    ip: camera.ip,
                    port: camera.port,
                    username: camera.username,
                    streamPath0: camera.streamPath0,
                    streamPath1: camera.streamPath1,
                    groupId: camera.groupId
                };

                switch(command) {
                    case 'open_in_browser': 
                        window.api.openInBrowser(cameraDataForIPC.ip); 
                        break;
                    case 'files': window.api.openFileManager(cameraDataForIPC); break;
                    case 'ssh': window.api.openSshTerminal(cameraDataForIPC); break;
                    case 'archive': App.archiveManager.openArchiveForCamera(camera); break;
                    case 'settings': App.modalHandler.openSettingsModal(cameraId); break;
                    case 'edit': App.modalHandler.openAddModal(cameraDataForIPC); break;
                    case 'delete': deleteCamera(cameraId); break;
                }
            });

            // VVVVVV --- НОВЫЙ КОД --- VVVVVV
            // Слушаем команды от контекстного меню группы
            window.api.onGroupContextMenuCommand(({ command, groupId }) => {
                switch (command) {
                    case 'rename':
                        renameGroup(groupId);
                        break;
                    case 'delete':
                        deleteGroup(groupId);
                        break;
                }
            });
            // ^^^^^^ --- КОНЕЦ НОВОГО КОДА --- ^^^^^^
        }

        return {
            init,
            render,
            pollCameraStatuses
        }
    }
})(window);