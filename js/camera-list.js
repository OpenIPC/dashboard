// js/camera-list.js (ПОЛНАЯ ВЕРСИЯ)

(function(window) {
    window.AppModules = window.AppModules || {};

    window.AppModules.createCameraList = function(App) {
        const stateManager = App.stateManager;
        const cameraListContainer = document.getElementById('camera-list-container');

        async function pollCameraStatuses() {
            const cameras = stateManager.state.cameras;
            const statusPromises = cameras.map(async (camera) => {
                const statusIcon = document.getElementById(`status-icon-${camera.id}`);
                if (statusIcon) {
                    try {
                        const pulse = await window.api.getCameraPulse(camera);
                        statusIcon.classList.toggle('online', pulse.success);
                    } catch (e) {
                        statusIcon.classList.remove('online');
                    }
                }
            });
            await Promise.all(statusPromises);
        }

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
                    await window.api.toggleRecording({ id: cameraId });
                }
                const analyticsBtn = document.getElementById(`analytics-btn-${cameraId}`);
                if (analyticsBtn && analyticsBtn.classList.contains('active')) {
                    await window.api.toggleAnalytics(cameraId);
                }
                stateManager.deleteCamera(cameraId);
            }
        }

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

        function render() {
            cameraListContainer.innerHTML = '';
            const { cameras, groups, recordingStates } = stateManager.state;
            const currentUser = stateManager.state.currentUser;
        
            const createGroupHTML = (group, camerasInGroup) => {
                const groupContainer = document.createElement('div');
                groupContainer.className = 'group-container';
                groupContainer.dataset.groupId = group.id;
        
                const groupHeader = document.createElement('div');
                groupHeader.className = 'group-header';
                groupHeader.innerHTML = `<i class="material-icons toggle-icon">arrow_drop_down</i><span class="group-name">${group.name}</span>`;
        
                const groupCamerasList = document.createElement('div');
                groupCamerasList.className = 'group-cameras';
        
                camerasInGroup.forEach(camera => {
                    const cameraItem = document.createElement('div');
                    cameraItem.className = 'camera-item';
                    cameraItem.dataset.cameraId = camera.id;
                    
                    if (currentUser?.role === 'admin' || currentUser?.permissions?.manage_layout) {
                        cameraItem.setAttribute('draggable', 'true');
                    }
                    
                    const analyticsButtonHTML = App.versionType === 'intellect' 
                        ? `<button class="analytics-btn icon-button" id="analytics-btn-${camera.id}" title="${App.i18n.t('toggle_analytics_tooltip')}">
                               <i class="material-icons" style="font-size: 18px;">insights</i>
                           </button>`
                        : '';

                    cameraItem.innerHTML = `
                        <i class="status-icon" id="status-icon-${camera.id}"></i>
                        <span style="flex-grow: 1;">${camera.name}</span>
                        <div class="rec-indicator"></div>
                        ${analyticsButtonHTML}
                    `;

                    if (recordingStates[camera.id]) {
                        cameraItem.classList.add('recording');
                    }

                    cameraItem.addEventListener('dragstart', (e) => {
                        e.dataTransfer.setData('application/x-camera-id', camera.id.toString());
                    });

                    groupCamerasList.appendChild(cameraItem);
                });
        
                groupContainer.appendChild(groupHeader);
                groupContainer.appendChild(groupCamerasList);
        
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
            cameraListContainer.addEventListener('click', async (e) => {
                const groupHeader = e.target.closest('.group-header');
                const analyticsBtn = e.target.closest('.analytics-btn');

                if (groupHeader) {
                    const groupContainer = groupHeader.closest('.group-container');
                    if (groupContainer) {
                        groupHeader.querySelector('.toggle-icon').classList.toggle('collapsed');
                        groupContainer.querySelector('.group-cameras').classList.toggle('collapsed');
                    }
                    return;
                }

                if (analyticsBtn) {
                    e.stopPropagation();
                    const cameraItem = analyticsBtn.closest('.camera-item');
                    if (cameraItem && cameraItem.dataset.cameraId) {
                        const cameraId = parseInt(cameraItem.dataset.cameraId, 10);
                        const btnIcon = analyticsBtn.querySelector('i');
                        if (btnIcon) btnIcon.style.color = '#ffc107';
                        console.log(`[Analytics] Toggling analytics for camera ID: ${cameraId}`);
                        
                        // VVVVVV --- ПРАВИЛЬНЫЙ ВЫЗОВ --- VVVVVV
                        await window.api.toggleAnalytics(cameraId);
                        // ^^^^^^ --- КОНЕЦ ПРАВИЛЬНОГО ВЫЗОВА --- ^^^^^^
                    }
                    return;
                }
            });
            
            cameraListContainer.addEventListener('dragover', (e) => {
                const groupHeader = e.target.closest('.group-header');
                if (groupHeader) {
                    e.preventDefault();
                    groupHeader.style.backgroundColor = 'var(--accent-color)';
                }
            });

            cameraListContainer.addEventListener('dragleave', (e) => {
                const groupHeader = e.target.closest('.group-header');
                if (groupHeader) {
                    groupHeader.style.backgroundColor = '';
                }
            });

            cameraListContainer.addEventListener('drop', (e) => {
                const groupHeader = e.target.closest('.group-header');
                if (groupHeader) {
                    e.preventDefault();
                    groupHeader.style.backgroundColor = '';
                    const groupIdStr = groupHeader.closest('.group-container')?.dataset.groupId;
                    const groupId = groupIdStr === 'null' ? null : parseInt(groupIdStr, 10);

                    const cameraId = parseInt(e.dataTransfer.getData('application/x-camera-id'), 10);
                    if (!isNaN(cameraId)) {
                        const camera = stateManager.state.cameras.find(c => c.id === cameraId);
                        if (camera && camera.groupId !== groupId) {
                            stateManager.updateCamera({ ...camera, groupId: groupId });
                        }
                    }
                }
            });
            
            cameraListContainer.addEventListener('contextmenu', (e) => {
                const currentUser = stateManager.state.currentUser;
                if (!currentUser) return;

                const cameraItem = e.target.closest('.camera-item');
                const groupHeader = e.target.closest('.group-header');

                if (cameraItem) {
                    e.preventDefault();
                    const cameraId = parseInt(cameraItem.dataset.cameraId, 10);
                    const menuItems = {
                        open_in_browser: `🌐  ${App.i18n.t('context_open_in_browser')}`,
                        files: `🗂️  ${App.i18n.t('context_file_manager')}`,
                        ssh: `💻  ${App.i18n.t('context_ssh')}`,
                        archive: `🗄️  ${App.i18n.t('archive_title')}`
                    };

                    if (currentUser.role === 'admin' || currentUser.permissions?.edit_cameras) {
                        menuItems.edit = `✏️  ${App.i18n.t('context_edit')}`;
                    }
                    if (currentUser.role === 'admin' || currentUser.permissions?.delete_cameras) {
                        menuItems.delete = `🗑️  ${App.i18n.t('context_delete')}`;
                    }

                    window.api.showCameraContextMenu({ cameraId, labels: menuItems });
                } else if (groupHeader) {
                    const groupIdStr = groupHeader.closest('.group-container')?.dataset.groupId;
                    if (groupIdStr && groupIdStr !== 'null') {
                        e.preventDefault();
                        if (currentUser?.role !== 'admin' && !currentUser?.permissions?.edit_cameras) {
                            return;
                        }
                        window.api.showGroupContextMenu({
                            groupId: parseInt(groupIdStr, 10),
                            labels: {
                                rename: App.i18n.t('context_rename_group'),
                                delete: App.i18n.t('context_delete_group')
                            }
                        });
                    }
                }
            });

            window.api.onContextMenuCommand(({ command, cameraId }) => {
                const camera = stateManager.state.cameras.find(c => c.id === cameraId);
                if (!camera) return;

                switch(command) {
                    case 'archive': 
                        App.archiveManager.openArchiveForCamera(camera); 
                        break;
                    case 'edit': 
                        App.modalHandler.openAddModal(camera); 
                        break;
                    case 'delete': 
                        deleteCamera(cameraId); 
                        break;
                }
            });

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
        }

        return {
            init,
            render,
            pollCameraStatuses
        }
    }
})(window);