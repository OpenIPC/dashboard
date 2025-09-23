// --- START OF FILE js/modals/camera-handler.js ---
(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createCameraModalHandler = function(App, utils) {
        const stateManager = App.stateManager;

        let addModal, saveCameraBtn, cancelAddBtn, addModalCloseBtn,
            addGroupModal, newGroupNameInput, saveGroupBtn, cancelGroupBtn, addGroupModalCloseBtn,
            discoverBtn, discoverModal, discoverModalCloseBtn, discoverList, addDiscoveredBtn, rediscoverBtn,
            newCamProtocolSelect;

        let editingCameraId = null;
        let selectedDiscoveredDevice = null;
        let isDiscovering = false;

        function openAddModal(cameraToEdit = null) {
            editingCameraId = cameraToEdit ? cameraToEdit.id : null;
            const modalTitle = document.getElementById('add-modal-title');
            const camera = cameraToEdit || {};
            modalTitle.textContent = editingCameraId ? App.i18n.t('edit_camera_title') : App.i18n.t('add_camera_title');
            document.getElementById('new-cam-name').value = camera.name || '';
            document.getElementById('new-cam-ip').value = camera.ip || '';
            if (newCamProtocolSelect) {
                newCamProtocolSelect.value = camera.protocol || 'openipc';
            }
            document.getElementById('new-cam-port').value = camera.port || '554';
            document.getElementById('new-cam-user').value = camera.username || 'root';
            document.getElementById('new-cam-pass').value = '';
            document.getElementById('new-cam-onvif-auth').checked = camera.onvifAuth !== false;
            document.getElementById('new-cam-stream-path0').value = camera.streamPath0 !== undefined ? camera.streamPath0 : '/stream=0';
            document.getElementById('new-cam-stream-path1').value = camera.streamPath1 !== undefined ? camera.streamPath1 : '/stream=1';
            utils.openModal(addModal);
            document.getElementById('new-cam-name').focus();
        }

        async function saveCamera() {
            const cameraDataToUpdate = {
                name: document.getElementById('new-cam-name').value.trim(),
                ip: document.getElementById('new-cam-ip').value.trim(),
                port: document.getElementById('new-cam-port').value.trim(),
                username: document.getElementById('new-cam-user').value.trim(),
                streamPath0: document.getElementById('new-cam-stream-path0').value.trim(),
                streamPath1: document.getElementById('new-cam-stream-path1').value.trim(),
                protocol: newCamProtocolSelect ? newCamProtocolSelect.value : 'openipc',
                onvifAuth: document.getElementById('new-cam-onvif-auth').checked
            };
            const password = document.getElementById('new-cam-pass').value;
            if (password) {
                cameraDataToUpdate.password = password;
            }

            if (!cameraDataToUpdate.name || !cameraDataToUpdate.ip) {
                App.modalHandler.showToast(App.i18n.t('name_and_ip_required'), true);
                return;
            }
            // ...RTSP test logic removed. Camera will be saved without any RTSP probe or prompt...

            if (editingCameraId) {
                const oldCam = stateManager.state.cameras.find(c => c.id === editingCameraId);
                const needsRestart = oldCam.ip !== cameraDataToUpdate.ip || oldCam.port !== cameraDataToUpdate.port || oldCam.username !== cameraDataToUpdate.username || (cameraDataToUpdate.password) || oldCam.streamPath0 !== cameraDataToUpdate.streamPath0 || oldCam.streamPath1 !== cameraDataToUpdate.streamPath1 || oldCam.protocol !== cameraDataToUpdate.protocol || oldCam.onvifAuth !== cameraDataToUpdate.onvifAuth;
                
                // Сначала обновляем данные в state
                stateManager.updateCamera({ id: editingCameraId, ...cameraDataToUpdate });
                
                if (needsRestart) {
                    console.log(`[Camera Save] Settings changed for camera ${editingCameraId}, initiating restart.`);
                    
                    // Закрываем модальное окно сразу
                    utils.closeModal(addModal);

                    // Показываем уведомление пользователю
                    App.modalHandler.showToast(`Настройки для камеры "${cameraDataToUpdate.name}" сохранены. Ожидание перезагрузки камеры...`, false, 5000);

                    // Запускаем асинхронную функцию ожидания и перезапуска
                    (async () => {
                        const MAX_ATTEMPTS = 20; // 20 попыток * 2 секунды = 40 секунд ожидания
                        const RETRY_DELAY = 2000; // 2 секунды между попытками
                        let attempts = 0;
                        let cameraIsBack = false;

                        // Получаем обновленные данные камеры из state, чтобы использовать новый IP, если он изменился
                        const updatedCam = stateManager.state.cameras.find(c => c.id === editingCameraId);

                        while (attempts < MAX_ATTEMPTS) {
                            attempts++;
                            console.log(`[Camera Restart] Attempt ${attempts} to ping camera ${updatedCam.name} at ${updatedCam.ip}`);
                            try {
                                const pulse = await window.api.getCameraPulse(updatedCam);
                                if (pulse.success) {
                                    console.log(`[Camera Restart] Camera ${updatedCam.name} is back online!`);
                                    cameraIsBack = true;
                                    break;
                                }
                            } catch (e) {
                                // Игнорируем ошибки, это ожидаемо, пока камера перезагружается
                            }
                            await new Promise(resolve => setTimeout(resolve, RETRY_DELAY));
                        }

                        if (cameraIsBack) {
                            App.modalHandler.showToast(`Камера "${updatedCam.name}" снова в сети. Перезапускаем видеопоток...`);
                            // Небольшая дополнительная пауза, чтобы веб-сервер на камере успел полностью запуститься
                            await new Promise(resolve => setTimeout(resolve, 1000));
                            App.gridManager.restartStreamsForCamera(editingCameraId);
                        } else {
                            console.error(`[Camera Restart] Camera ${updatedCam.name} did not come back online after ${MAX_ATTEMPTS} attempts.`);
                            App.modalHandler.showToast(`Не удалось дождаться перезагрузки камеры "${updatedCam.name}". Пожалуйста, проверьте ее состояние.`, true, 8000);
                        }
                    })();
                    
                    return; // Выходим из функции, так как модальное окно уже закрыто
                }
            } else {
                stateManager.addCamera(cameraDataToUpdate);
            }
            utils.closeModal(addModal);
        }

        function openAddGroupModal() {
            if (!addGroupModal) return;
            newGroupNameInput.value = '';
            utils.openModal(addGroupModal);
            newGroupNameInput.focus();
        }

        async function saveNewGroup() {
            const name = newGroupNameInput.value.trim();
            if (!name) { App.modalHandler.showToast(App.i18n.t('group_name_empty_error'), true); return; }
            stateManager.addGroup({ name });
            utils.closeModal(addGroupModal);
        }

        async function startDiscovery() {
            if (isDiscovering || !discoverModal) return;
            isDiscovering = true;
            utils.openModal(discoverModal);
            discoverList.innerHTML = `<li style="padding: 10px; color: #666;">${App.i18n.t('searching_for_cameras')}</li>`;
            addDiscoveredBtn.disabled = true;
            rediscoverBtn.disabled = true;
            selectedDiscoveredDevice = null;
            
            await window.api.discoverDevices();

            setTimeout(() => {
                isDiscovering = false;
                if(rediscoverBtn) rediscoverBtn.disabled = false;
                const initialSearchMessage = App.i18n.t('searching_for_cameras');
                if (discoverList && discoverList.innerHTML.includes(initialSearchMessage)) {
                    discoverList.innerHTML = `<li style="padding: 10px; color: #666;">${App.i18n.t('no_cameras_found')}</li>`;
                }
            }, 20000);
        }

        function addDiscoveredCamera() {
            if (!selectedDiscoveredDevice) return;
            const { ip, name, protocol } = selectedDiscoveredDevice;
            const cameraToEdit = { name: protocol === 'rtsp' ? `RTSP Camera ${ip}` : name, ip: ip, protocol: protocol, streamPath0: '/stream=0', streamPath1: '/stream=1' };
            if (protocol === 'rtsp' || protocol === 'onvif') {
                cameraToEdit.protocol = 'openipc';
            }
            utils.closeModal(discoverModal);
            openAddModal(cameraToEdit);
        }
        
        function init() {
            addModal = document.getElementById('add-camera-modal');
            saveCameraBtn = document.getElementById('save-camera-btn');
            cancelAddBtn = document.getElementById('cancel-camera-btn');
            addModalCloseBtn = document.getElementById('add-modal-close-btn');
            newCamProtocolSelect = document.getElementById('new-cam-protocol');

            addGroupModal = document.getElementById('add-group-modal');
            newGroupNameInput = document.getElementById('new-group-name');
            saveGroupBtn = document.getElementById('save-group-btn');
            cancelGroupBtn = document.getElementById('cancel-group-btn');
            addGroupModalCloseBtn = document.getElementById('add-group-modal-close-btn');
            
            discoverBtn = document.getElementById('discover-btn');
            discoverModal = document.getElementById('discover-modal');
            discoverModalCloseBtn = document.getElementById('discover-modal-close-btn');
            discoverList = document.getElementById('discover-list');
            addDiscoveredBtn = document.getElementById('add-discovered-btn');
            rediscoverBtn = document.getElementById('rediscover-btn');
            
            window.api.onDeviceFound((device) => {
                if (!discoverList) return;
                const placeholderMessage = App.i18n.t('searching_for_cameras');
                if (discoverList.children.length > 0 && discoverList.children[0].textContent.includes(placeholderMessage)) {
                    discoverList.innerHTML = '';
                }
                const existingItem = Array.from(discoverList.children).find(li => li.dataset.ip === device.ip);
                if (existingItem) return;
                const li = document.createElement('li');
                li.style.cssText = "padding: 10px; cursor: pointer; border-bottom: 1px solid #eee;";
                li.dataset.ip = device.ip;
                const protocolTag = `[${device.protocol.toUpperCase()}]`;
                li.innerHTML = `<strong>${device.name}</strong> ${protocolTag}<br><small>${device.ip}</small>`;
                li.addEventListener('click', () => {
                    discoverList.querySelectorAll('li').forEach(el => el.style.backgroundColor = '');
                    li.style.backgroundColor = '#d4e6f1';
                    selectedDiscoveredDevice = device;
                    if(addDiscoveredBtn) addDiscoveredBtn.disabled = false;
                });
                discoverList.appendChild(li);
            });

            const addCameraSidebarBtn = document.getElementById('add-camera-sidebar-btn');
            if (addCameraSidebarBtn) addCameraSidebarBtn.addEventListener('click', () => openAddModal());

            if (saveCameraBtn) saveCameraBtn.addEventListener('click', saveCamera);
            const autofillBtn = document.getElementById('autofill-camera-btn');
            if (autofillBtn) autofillBtn.addEventListener('click', async () => {
                // Read current inputs
                const ip = document.getElementById('new-cam-ip').value.trim();
                const port = document.getElementById('new-cam-port').value.trim() || '80';
                const username = document.getElementById('new-cam-user').value.trim();
                const password = document.getElementById('new-cam-pass').value;
                if (!ip) { App.modalHandler.showToast(App.i18n.t('ip_required_for_autofill'), true); return; }

                App.modalHandler.showToast(App.i18n.t('probing_camera'), false, 3000);
                const info = await window.api.probeCameraInfo({ ip, port: parseInt(port, 10), username, password, timeout: 3000 });
                if (!info || !info.success) {
                    App.modalHandler.showToast(`${App.i18n.t('probe_failed')}: ${info && info.error ? info.error : 'unknown'}`, true, 5000);
                    return;
                }

                // Candidate RTSP paths to try (ordered)
                const candidatePaths = ['/stream=0','/stream=1','/stream0','/stream1','/live','/h264','/ch0','/ch1','/1','/0','/onvif-media','/media/video1'];
                const buildUrl = (path, p) => `rtsp://${encodeURIComponent(username)}:${encodeURIComponent(password || '')}@${ip}:${p || 554}${path}`;

                let found0 = null;
                let found1 = null;
                // try to find two working streams; try common pairs first
                for (const p0 of candidatePaths) {
                    const url0 = buildUrl(p0, document.getElementById('new-cam-port').value || 554);
                    const res0 = await window.api.testRtspUrl({ url: url0, timeout: 2500 });
                    if (res0 && res0.success && !(res0.statusCode >= 400)) {
                        found0 = p0;
                        break;
                    }
                }
                for (const p1 of candidatePaths) {
                    if (p1 === found0) continue;
                    const url1 = buildUrl(p1, document.getElementById('new-cam-port').value || 554);
                    const res1 = await window.api.testRtspUrl({ url: url1, timeout: 2500 });
                    if (res1 && res1.success && !(res1.statusCode >= 400)) {
                        found1 = p1;
                        break;
                    }
                }

                if (found0) document.getElementById('new-cam-stream-path0').value = found0;
                if (found1) document.getElementById('new-cam-stream-path1').value = found1;

                // If neither found, try ONVIF GetStreamUri fallback
                if (!found0 && !found1) {
                    App.modalHandler.showToast(App.i18n.t('probing_onvif'), false, 3000);
                    try {
                        const onvif = await window.api.probeOnvifStreamUri({ ip, port: parseInt(port, 10), username, password, timeout: 4000 });
                        if (onvif && onvif.success && onvif.uri) {
                            // Try to parse RTSP path from returned URI
                            try {
                                const m = onvif.uri.match(/^rtsp:\/\/([^@]+@)?([^:\/]+)(?::(\d+))?(\/.*)$/);
                                if (m) {
                                    const path = m[4];
                                    // Fill both paths with discovered path or variants
                                    document.getElementById('new-cam-stream-path0').value = path;
                                    document.getElementById('new-cam-stream-path1').value = path.replace(/0/, '1');
                                    App.modalHandler.showToast(App.i18n.t('autofill_success_onvif'), false, 4000);
                                } else {
                                    App.modalHandler.showToast(App.i18n.t('autofill_not_found'), true, 6000);
                                }
                            } catch (e) {
                                App.modalHandler.showToast(App.i18n.t('autofill_not_found'), true, 6000);
                            }
                        } else {
                            App.modalHandler.showToast(App.i18n.t('autofill_not_found'), true, 6000);
                        }
                    } catch (e) {
                        App.modalHandler.showToast(App.i18n.t('autofill_not_found'), true, 6000);
                    }
                }

                if (found0 || found1) {
                    App.modalHandler.showToast(App.i18n.t('autofill_success'), false, 4000);
                } else {
                    App.modalHandler.showToast(App.i18n.t('autofill_not_found'), true, 6000);
                }
            });
            if (addModalCloseBtn) addModalCloseBtn.addEventListener('click', () => utils.closeModal(addModal));
            if (cancelAddBtn) cancelAddBtn.addEventListener('click', () => utils.closeModal(addModal));
            if (addModal) addModal.addEventListener('click', (e) => { if (e.target === addModal) utils.closeModal(addModal); });

            const addGroupBtn = document.getElementById('add-group-btn');
            if (addGroupBtn) addGroupBtn.addEventListener('click', openAddGroupModal);
            if (saveGroupBtn) saveGroupBtn.addEventListener('click', saveNewGroup);
            if (cancelGroupBtn) cancelGroupBtn.addEventListener('click', () => utils.closeModal(addGroupModal));
            if (addGroupModalCloseBtn) addGroupModalCloseBtn.addEventListener('click', () => utils.closeModal(addGroupModal));
            if (addGroupModal) addGroupModal.addEventListener('click', (e) => { if (e.target === addGroupModal) utils.closeModal(addGroupModal); });
            
            if (discoverBtn) discoverBtn.addEventListener('click', startDiscovery);
            if (rediscoverBtn) rediscoverBtn.addEventListener('click', startDiscovery);
            // При любом закрытии модального окна поиска камер сбрасываем isDiscovering
            function closeDiscoverModal() {
                utils.closeModal(discoverModal);
                isDiscovering = false;
                if(rediscoverBtn) rediscoverBtn.disabled = false;
            }
            if (discoverModalCloseBtn) discoverModalCloseBtn.addEventListener('click', closeDiscoverModal);
            if (discoverModal) discoverModal.addEventListener('click', (e) => { if (e.target === discoverModal) closeDiscoverModal(); });
            if (addDiscoveredBtn) addDiscoveredBtn.addEventListener('click', addDiscoveredCamera);
            
            window.addEventListener('language-changed', () => {
                if (addModal && !addModal.classList.contains('hidden')) {
                    const cam = editingCameraId ? stateManager.state.cameras.find(c => c.id === editingCameraId) : null;
                    openAddModal(cam);
                }
                if (addGroupModal && !addGroupModal.classList.contains('hidden')) {
                    document.getElementById('add-group-modal-title').textContent = App.i18n.t('create_group_title');
                }
                if (discoverModal && !discoverModal.classList.contains('hidden')) {
                     document.querySelector('#discover-modal h2').textContent = App.i18n.t('discover_modal_title');
                }
            });
        }
        
        return {
            init,
            openAddModal,
            closeAll: () => {
                if (addModal) utils.closeModal(addModal);
                if (addGroupModal) utils.closeModal(addGroupModal);
                if (discoverModal) utils.closeModal(discoverModal);
            }
        };
    };
})(window);