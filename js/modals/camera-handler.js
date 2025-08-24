// --- ФАЙЛ: camera-handler.js ---

(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createCameraModalHandler = function(App, utils) {
        const stateManager = App.stateManager;

        // --- ИЗМЕНЕНИЕ: Объявляем переменные здесь, но не ищем элементы ---
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
            
            if (editingCameraId) {
                const oldCam = stateManager.state.cameras.find(c => c.id === editingCameraId);
                const needsRestart = oldCam.ip !== cameraDataToUpdate.ip || 
                                     oldCam.port !== cameraDataToUpdate.port || 
                                     oldCam.username !== cameraDataToUpdate.username || 
                                     (cameraDataToUpdate.password) || 
                                     oldCam.streamPath0 !== cameraDataToUpdate.streamPath0 || 
                                     oldCam.streamPath1 !== cameraDataToUpdate.streamPath1 ||
                                     oldCam.protocol !== cameraDataToUpdate.protocol ||
                                     oldCam.onvifAuth !== cameraDataToUpdate.onvifAuth;
                stateManager.updateCamera({ id: editingCameraId, ...cameraDataToUpdate });
                if (needsRestart) {
                    setTimeout(() => App.gridManager.restartStreamsForCamera(editingCameraId), 100);
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
            
            const cameraToEdit = { 
                name: protocol === 'rtsp' ? `RTSP Camera ${ip}` : name,
                ip: ip, 
                protocol: protocol,
                streamPath0: '/stream=0', 
                streamPath1: '/stream=1' 
            };
            
            if (protocol === 'rtsp' || protocol === 'onvif') {
                cameraToEdit.protocol = 'openipc';
            }

            utils.closeModal(discoverModal);
            openAddModal(cameraToEdit);
        }
        
        function init() {
            // --- ИЗМЕНЕНИЕ: Ищем элементы DOM только здесь, внутри init() ---
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
            if (discoverModalCloseBtn) discoverModalCloseBtn.addEventListener('click', () => utils.closeModal(discoverModal));
            if (discoverModal) discoverModal.addEventListener('click', (e) => { if (e.target === discoverModal) utils.closeModal(discoverModal); });
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