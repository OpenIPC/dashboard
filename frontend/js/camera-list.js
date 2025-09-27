// js/camera-list.js (cleaned version)

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
                    await window.api.toggleAnalytics(cameraId, 1); // SD для превью
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

        function createGroupHTML(group, camerasInGroup, currentUser, recordingStates) {
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
                    ? `<button class="analytics-btn icon-button" id="analytics-btn-${camera.id}" title="${App.i18n.t('toggle_analytics_tooltip')}">\n                                   <i class="material-icons" style="font-size: 18px;">insights</i>\n                               </button>`
                    : '';

                const plateButtonHTML = `<button class="plate-btn icon-button" id="plate-btn-${camera.id}" title="Распознать номер">\n                        <i class="material-icons" style="font-size: 18px;">directions_car</i>\n                    </button>`;

                cameraItem.innerHTML = `\n                        <i class="status-icon" id="status-icon-${camera.id}"></i>\n                        <span style="flex-grow: 1;">${camera.name}</span>\n                        <div class="rec-indicator"></div>\n                        ${analyticsButtonHTML}\n                        ${plateButtonHTML}\n                    `;

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
        }

        function render() {
            cameraListContainer.innerHTML = '';
            const { cameras, groups, recordingStates } = stateManager.state;
            const currentUser = stateManager.state.currentUser;

            groups.forEach(group => {
                const camerasInGroup = cameras.filter(c => c.groupId === group.id);
                cameraListContainer.appendChild(createGroupHTML(group, camerasInGroup, currentUser, recordingStates));
            });

            const ungroupedCameras = cameras.filter(c => !c.groupId);
            if (ungroupedCameras.length > 0) {
                const ungroupedPseudoGroup = { id: null, name: App.i18n.t('ungrouped_cameras') };
                cameraListContainer.appendChild(createGroupHTML(ungroupedPseudoGroup, ungroupedCameras, currentUser, recordingStates));
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
                const plateBtn = e.target.closest('.plate-btn');

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
                        await window.api.toggleAnalytics(cameraId, 1);
                    }
                    return;
                }

                if (plateBtn) {
                    e.stopPropagation();
                    const cameraItem = plateBtn.closest('.camera-item');
                    if (cameraItem && cameraItem.dataset.cameraId) {
                        const cameraId = parseInt(cameraItem.dataset.cameraId, 10);
                        const camera = stateManager.state.cameras.find(c => c.id === cameraId);
                        // Временный вывод camera-объекта для отладки RTSP URL
                        console.log('DEBUG camera object for RTSP:', camera);
                        // Try to derive a working RTSP URL (use stored rtspUrl, build from fields, or probe ONVIF)
                        console.debug('[CameraList] plate button clicked. window.api type:', typeof window.api);
                        try { console.debug('[CameraList] window.api keys:', Object.keys(window.api || {})); } catch(e){}
                        console.debug('[CameraList] window.api.invoke type:', typeof (window.api && window.api.invoke));
                        const rtspUrl = await getRtspUrlForCamera(camera);
                        if (rtspUrl) {
                            // Start recognition in main process (safe: main has access to stored passwords)
                            try {
                                // Debug before invoking
                                console.debug('[CameraList] invoking module-license-plate-start for cameraId', cameraId);
                                const res = await (window.api && typeof window.api.invoke === 'function' ? window.api.invoke('module-license-plate-start', cameraId) : Promise.reject(new Error('window.api.invoke is not a function')));
                                if (res && res.success) {
                                    App.modalHandler.showToast(App.i18n.t('plate_started') || 'Распознавание запущено', false, 3500);
                                } else {
                                    App.modalHandler.showToast((res && res.error) ? res.error : App.i18n.t('plate_start_failed') || 'Не удалось запустить распознавание', true, 5000);
                                }
                            } catch (e) {
                                App.modalHandler.showToast((e && e.message) || App.i18n.t('plate_start_failed') || 'Ошибка запуска', true);
                            }
                        } else {
                            App.modalHandler.showToast(App.i18n.t('plate_no_rtsp_url') || 'RTSP URL не найден для камеры', true);
                        }
                    }
                    return;
                }
            });
        }

        // Try to derive a working RTSP URL for a camera by testing common paths and ONVIF
        async function getRtspUrlForCamera(camera) {
            if (!camera) return null;

            // If an explicit URL is stored, try it first
            if (camera.rtspUrl) {
                try {
                    const test = await window.api.testRtspUrl({ url: camera.rtspUrl, timeout: 2500 });
                    if (test && test.success && !(test.statusCode >= 400)) return camera.rtspUrl;
                } catch (e) {
                    // fallthrough to build attempts
                }
            }

            const ip = camera.ip;
            const port = camera.port || 554;
            const username = camera.username;
            const password = camera.password;

            // Candidate paths: try configured ones first, then common defaults
            const candidatePaths = [];
            if (camera.streamPath0) candidatePaths.push(camera.streamPath0);
            if (camera.streamPath1) candidatePaths.push(camera.streamPath1);
            candidatePaths.push('/stream=0','/stream=1','/stream0','/stream1','/live','/h264','/ch0','/ch1','/1','/0','/onvif-media','/media/video1');

            const buildUrl = (path) => {
                let auth = '';
                if (username) {
                    auth = encodeURIComponent(username);
                    if (password) auth += ':' + encodeURIComponent(password);
                    auth += '@';
                }
                return `rtsp://${auth}${ip}:${port}${path}`;
            };

            // Try each candidate with available credentials (if any), then without credentials
            for (const p of candidatePaths) {
                const urlWithCreds = buildUrl(p);
                try {
                    const res = await window.api.testRtspUrl({ url: urlWithCreds, timeout: 2500 });
                    if (res && res.success && !(res.statusCode >= 400)) return urlWithCreds;
                } catch (e) {
                    // ignore and continue
                }
                // If we included creds and they were present, also try without creds
                if (username) {
                    const urlNoCreds = `rtsp://${ip}:${port}${p}`;
                    try {
                        const res2 = await window.api.testRtspUrl({ url: urlNoCreds, timeout: 2500 });
                        if (res2 && res2.success && !(res2.statusCode >= 400)) return urlNoCreds;
                    } catch (e) {}
                }
            }

            // As a fallback, try ONVIF GetStreamUri which may return a usable RTSP URI
            try {
                const onvif = await window.api.probeOnvifStreamUri({ ip, port: parseInt(port, 10), username: username || '', password: password || '', timeout: 4000 });
                if (onvif && onvif.success && onvif.uri) {
                    return onvif.uri;
                }
            } catch (e) {
                // ignore
            }

            // Final fallback: use local MediaMTX source (the main process generates these paths at startup)
            try {
                const mediaMtxUrl = `rtsp://127.0.0.1:8554/cam${camera.id}_0`;
                console.debug('[CameraList] Falling back to MediaMTX URL for camera', camera.id, mediaMtxUrl);
                return mediaMtxUrl;
            } catch (e) {
                return null;
            }
        }

        async function recognizePlate(rtspUrl, cameraId) {
            const btn = document.getElementById(`plate-btn-${cameraId}`);
            if (btn) btn.disabled = true;
            try {
                console.debug('[recognizePlate] window.api type:', typeof window.api);
                try { console.debug('[recognizePlate] window.api keys:', Object.keys(window.api || {})); } catch(e){}
                console.debug('[recognizePlate] window.api.invoke type:', typeof (window.api && window.api.invoke));
                // Use main process IPC to start recognition which has access to stored passwords
                const res = await (window.api && typeof window.api.invoke === 'function' ? window.api.invoke('module-license-plate-start', cameraId) : Promise.reject(new Error('window.api.invoke is not a function')));
                if (res && res.success) {
                    App.modalHandler.showToast(App.i18n.t('plate_started') || 'Распознавание запущено', false, 3500);
                } else {
                    App.modalHandler.showToast((res && res.error) ? res.error : App.i18n.t('plate_start_failed') || 'Не удалось запустить распознавание', true, 5000);
                }
            } catch (err) {
                App.modalHandler.showToast((err && err.message) || App.i18n.t('plate_recognition_error') || 'Ошибка распознавания номера', true);
            } finally {
                if (btn) btn.disabled = false;
            }
        }

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

                // Pass only serializable fields of camera object
                const cameraObjRaw = stateManager.state.cameras.find(c => c.id === cameraId) || null;
                let cameraObj = null;
                if (cameraObjRaw) {
                    const { id, groupId, name, ip, port, username, streamPath, streamPath0, streamPath1, protocol, onvifAuth } = cameraObjRaw;
                    cameraObj = { id, groupId, name, ip, port, username, streamPath, streamPath0, streamPath1, protocol, onvifAuth };
                }
                console.log('[CameraList][ContextMenu] cameraObj:', cameraObj);
                window.api.showCameraContextMenu({ cameraId, labels: menuItems, camera: cameraObj });
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
            if (!camera) {
                App.modalHandler.showToast(App.i18n.t('camera_not_found_error') || 'Камера не найдена. Попробуйте обновить страницу.', true);
                return;
            }

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

        return {
            init,
            render,
            pollCameraStatuses
        };
    };
})(window);