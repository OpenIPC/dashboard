// --- ФАЙЛ: settings-handler.js ---

(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createSettingsModalHandler = function(App, utils) {
        const stateManager = App.stateManager;

        const settingsModal = document.getElementById('settings-modal');
        const settingsModalCloseBtn = document.getElementById('settings-modal-close-btn');
        const saveSettingsBtn = document.getElementById('save-settings-btn');
        const restartMajesticBtn = document.getElementById('restart-majestic-btn');
        const killAllBtnModal = document.getElementById('kill-all-btn-modal');
        const recordingsPathInput = document.getElementById('app-settings-recordings-path');
        const selectRecPathBtn = document.getElementById('select-rec-path-btn');
        const languageSelect = document.getElementById('app-settings-language');
        const hwAccelSelect = document.getElementById('app-settings-hw-accel');
        const notificationsEnabledInput = document.getElementById('app-settings-notifications-enabled');
        
        const qscaleInput = document.getElementById('app-settings-qscale');
        const fpsInput = document.getElementById('app-settings-fps');

        const globalAnalyticsResizeWidthInput = document.getElementById('app-settings-analytics-resize-width');
        const globalAnalyticsFrameSkipInput = document.getElementById('app-settings-analytics-frame-skip');
        const globalAnalyticsRecordDurationInput = document.getElementById('app-settings-analytics-record-duration');
        
        const checkForUpdatesBtn = document.getElementById('check-for-updates-btn');
        const updateStatusText = document.getElementById('update-status-text');
        const analyticsObjectsListEl = document.getElementById('analytics-objects-list');

        const exportConfigBtn = document.getElementById('export-config-btn');
        const importConfigBtn = document.getElementById('import-config-btn');
        
        let settingsCameraId = null;
        let rangeSyncFunctions = {};

        const availableAnalyticsObjects = [
            { key: 'person', label: 'Человек' },
            { key: 'car', label: 'Автомобиль' },
            { key: 'motorbike', label: 'Мотоцикл' },
            { key: 'bus', label: 'Автобус' },
            { key: 'truck', label: 'Грузовик' },
            { key: 'bicycle', label: 'Велосипед' },
            { key: 'dog', label: 'Собака' },
            { key: 'cat', label: 'Кошка' },
            { key: 'backpack', label: 'Рюкзак' },
        ];
        
        function setupRangeSync(rangeId) {
            const rangeInput = document.getElementById(rangeId);
            const valueSpan = document.getElementById(`${rangeId}-value`);
            if (!rangeInput || !valueSpan) return () => {};
            const updateValue = () => { valueSpan.textContent = rangeInput.value; };
            rangeInput.addEventListener('input', updateValue);
            const syncFunc = (value) => { if (value !== undefined) { rangeInput.value = value; updateValue(); } };
            rangeSyncFunctions[rangeId] = syncFunc;
            return syncFunc;
        }

        function setFormValue(id, value, defaultValue) {
            const finalValue = value !== undefined && value !== null ? value : defaultValue;
            if (finalValue === undefined) return;

            const el = document.getElementById(id);
            if (!el) return;
        
            if (el.type === 'checkbox') el.checked = !!finalValue;
            else if (el.type === 'range') {
                const syncFunc = rangeSyncFunctions[id] || setupRangeSync(id);
                syncFunc(finalValue);
            }
            else el.value = finalValue;
        }

        async function openSettingsModal(cameraId = null) {
            // VVVVVV --- ВОТ ИСПРАВЛЕНИЕ --- VVVVVV
            App.i18n.applyTranslationsToDOM(); // Принудительно переводим все элементы перед показом
            // ^^^^^^ --- КОНЕЦ ИСПРАВЛЕНИЯ --- ^^^^^^

            settingsCameraId = cameraId;
            rangeSyncFunctions = {};
            const isGeneralSettings = !cameraId;
            const camera = isGeneralSettings ? null : stateManager.state.cameras.find(c => c.id === cameraId);
            const isNetipCamera = camera && camera.protocol === 'netip';

            document.getElementById('settings-modal-title').textContent = isGeneralSettings ? App.i18n.t('general_settings_title') : `${App.i18n.t('camera_settings_title_prefix')}: ${camera.name}`;
            const tabsContainer = settingsModal.querySelector('.tabs');
            
            tabsContainer.querySelectorAll('.tab-button').forEach(btn => {
                const tab = btn.dataset.tab;
                const isGeneralTab = tab === 'tab-general';
                const isStreamingTab = tab === 'tab-streaming';
                const isAnalyticsTab = tab === 'tab-analytics';
                const isMajesticOrNetipTab = !isGeneralTab && !isAnalyticsTab && !isStreamingTab;

                let show = false;
                if (isGeneralSettings) {
                    show = isGeneralTab || isStreamingTab || isAnalyticsTab;
                } else {
                    if (isNetipCamera) {
                        show = isAnalyticsTab;
                    } else {
                        show = isMajesticOrNetipTab || isAnalyticsTab;
                    }
                }
                btn.style.display = show ? 'flex' : 'none';
            });
            
            settingsModal.querySelectorAll('.tab-content, .tab-button').forEach(el => el.classList.remove('active'));
            
            let activeTab;
            if (isGeneralSettings) {
                activeTab = 'tab-general';
            } else {
                activeTab = isNetipCamera ? 'tab-analytics' : 'tab-system';
            }
            tabsContainer.querySelector(`[data-tab="${activeTab}"]`).classList.add('active');
            document.getElementById(activeTab).classList.add('active');

            // --- Заполнение общих настроек ---
            const { appSettings } = stateManager.state;
            recordingsPathInput.value = appSettings.recordingsPath || '';
            languageSelect.value = appSettings.language || 'en';
            hwAccelSelect.value = appSettings.hwAccel || 'auto';
            setFormValue('app-settings-notifications-enabled', appSettings.notifications_enabled, true);
            setFormValue('app-settings-qscale', appSettings.qscale, 8);
            setFormValue('app-settings-fps', appSettings.fps, 20);
            
            setFormValue('app-settings-analytics-resize-width', appSettings.analytics_resize_width, 416);
            setFormValue('app-settings-analytics-frame-skip', appSettings.analytics_frame_skip, 10);
            setFormValue('app-settings-analytics-record-duration', appSettings.analytics_record_duration, 30);
            
            // --- Управление видимостью блоков и кнопок ---
            document.getElementById('global-analytics-settings').style.display = isGeneralSettings ? 'block' : 'none';
            document.getElementById('camera-specific-analytics-settings').style.display = isGeneralSettings ? 'none' : 'block';
            restartMajesticBtn.style.display = isGeneralSettings || (camera && camera.protocol === 'netip') ? 'none' : 'inline-flex';
            killAllBtnModal.style.display = isGeneralSettings ? 'inline-flex' : 'none';
            
            utils.openModal(settingsModal);

            if (isGeneralSettings) {
                saveSettingsBtn.disabled = false;
                saveSettingsBtn.textContent = App.i18n.t('save');
                return;
            }

            // --- Загрузка и отображение настроек для конкретной камеры ---
            
            analyticsObjectsListEl.innerHTML = '';
            availableAnalyticsObjects.forEach(obj => {
                analyticsObjectsListEl.innerHTML += `
                    <div class="form-check-inline">
                        <input type="checkbox" id="analytics.objects.${obj.key}" class="form-check-input" data-object-key="${obj.key}">
                        <label for="analytics.objects.${obj.key}">${obj.label}</label>
                    </div>`;
            });

            const analyticsConfig = camera.analyticsConfig || {};
            setFormValue('analytics.enabled', analyticsConfig.enabled, false);
            if (analyticsConfig.objects) {
                analyticsConfig.objects.forEach(key => {
                    const checkbox = document.getElementById(`analytics.objects.${key}`);
                    if (checkbox) checkbox.checked = true;
                });
            }

            saveSettingsBtn.disabled = true;
            saveSettingsBtn.textContent = App.i18n.t('loading_text');
            try {
                if (!isNetipCamera) {
                    const settings = await window.api.getCameraSettings(camera);
                    if (settings && !settings.error) {
                        for (const section in settings) {
                            if (typeof settings[section] === 'object' && settings[section] !== null) {
                                for (const key in settings[section]) {
                                    setFormValue(`${section}.${key}`, settings[section][key]);
                                }
                            }
                        }
                    } else {
                        throw new Error(settings?.error || App.i18n.t('unknown_error'));
                    }
                }
            } catch (e) {
                alert(`${App.i18n.t('loading_settings_error')}: ${e.message}`);
                utils.closeModal(settingsModal);
            } finally {
                saveSettingsBtn.disabled = false;
                saveSettingsBtn.textContent = App.i18n.t('save');
            }
        }
        
        async function saveSettings() {
            saveSettingsBtn.disabled = true;
            saveSettingsBtn.textContent = App.i18n.t('saving_text');
            if (settingsCameraId === null) {
                // Сохранение ОБЩИХ настроек
                stateManager.setAppSettings({
                    recordingsPath: recordingsPathInput.value.trim(),
                    hwAccel: hwAccelSelect.value,
                    language: languageSelect.value,
                    notifications_enabled: notificationsEnabledInput.checked,
                    qscale: parseInt(qscaleInput.value, 10) || 8,
                    fps: parseInt(fpsInput.value, 10) || 20,
                    analytics_resize_width: parseInt(globalAnalyticsResizeWidthInput.value, 10) || 416,
                    analytics_frame_skip: parseInt(globalAnalyticsFrameSkipInput.value, 10) || 10,
                    analytics_record_duration: parseInt(document.getElementById('app-settings-analytics-record-duration').value, 10) || 30,
                });
                utils.showToast(App.i18n.t('app_settings_saved_success'));
            } else {
                // Сохранение настроек КАМЕРЫ
                const camera = stateManager.state.cameras.find(c => c.id === settingsCameraId);
                if (!camera) { saveSettingsBtn.disabled = false; saveSettingsBtn.textContent = App.i18n.t('save'); return; }
                
                if (camera.protocol !== 'netip') {
                    const settingsDataToSend = {};
                    settingsModal.querySelectorAll('[id*="."]').forEach(el => {
                        const [section, key] = el.id.split('.');
                        if (!section || !key || el.id.startsWith('app-settings-') || el.id.startsWith('analytics.')) return;
                        if (!settingsDataToSend[section]) settingsDataToSend[section] = {};
                        if (el.type === 'checkbox') settingsDataToSend[section][key] = el.checked;
                        else if (el.value !== '' && el.value !== null) {
                            const val = (el.type === 'number' || el.type === 'range') ? Number(el.value) : el.value;
                            settingsDataToSend[section][key] = val;
                        }
                    });
                    const result = await window.api.setCameraSettings({ credentials: camera, settingsData: settingsDataToSend });
                    if (result.success) utils.showToast(App.i18n.t('camera_settings_saved_success'));
                    else utils.showToast(`${App.i18n.t('save_settings_error')}: ${result.error}`, true, 5000);
                }

                const analyticsConfig = {
                    enabled: document.getElementById('analytics.enabled').checked,
                    objects: [],
                    roi: camera.analyticsConfig?.roi || null,
                };
                analyticsObjectsListEl.querySelectorAll('input[type="checkbox"]:checked').forEach(checkbox => {
                    analyticsConfig.objects.push(checkbox.dataset.objectKey);
                });
                stateManager.updateCamera({ id: settingsCameraId, analyticsConfig });
                
                const analyticsProcessRunning = Array.from(document.querySelectorAll('.analytics-btn.active'))
                                                  .some(btn => btn.id === `analytics-btn-${settingsCameraId}`);
                                                  
                if (analyticsProcessRunning && !analyticsConfig.enabled) {
                    window.api.toggleAnalytics(settingsCameraId);
                }
            }
            saveSettingsBtn.disabled = false;
            saveSettingsBtn.textContent = App.i18n.t('save');
        }

        async function restartMajestic() { 
            if (!settingsCameraId) return; 
            const camera = stateManager.state.cameras.find(c => c.id === settingsCameraId); 
            if (!camera || camera.protocol === 'netip') return;
            const result = await window.api.restartMajestic(camera); 
            if (result.success) utils.showToast(App.i18n.t('restart_command_sent')); 
            else utils.showToast(`${App.i18n.t('restart_error')}: ${result.error}`, true); 
        }

        function init() {
            settingsModalCloseBtn.addEventListener('click', () => utils.closeModal(settingsModal));
            settingsModal.addEventListener('click', (e) => { if (e.target === settingsModal) utils.closeModal(settingsModal); });
            saveSettingsBtn.addEventListener('click', saveSettings);
            restartMajesticBtn.addEventListener('click', restartMajestic);
            killAllBtnModal.addEventListener('click', async () => { if (confirm(App.i18n.t('kill_all_confirm'))) { const result = await window.api.killAllFfmpeg(); alert(result.message); window.location.reload(); } });

            if (exportConfigBtn) {
                exportConfigBtn.addEventListener('click', () => window.api.exportConfig());
            }
            if (importConfigBtn) {
                importConfigBtn.addEventListener('click', () => window.api.importConfig());
            }

            languageSelect.addEventListener('change', async (e) => {
                const newLang = e.target.value;
                stateManager.setAppSettings({ language: newLang });
                await App.i18n.setLanguage(newLang);
                if (!settingsModal.classList.contains('hidden')) {
                    openSettingsModal(settingsCameraId);
                }
                utils.showToast(App.i18n.t('app_settings_saved_success'));
            });

            selectRecPathBtn.addEventListener('click', async () => { 
                const result = await window.api.selectDirectory(); 
                if (!result.canceled) { 
                    recordingsPathInput.value = result.path; 
                    stateManager.setAppSettings({ recordingsPath: result.path });
                } 
            });

            settingsModal.querySelectorAll('.tab-button').forEach(button => { 
                button.addEventListener('click', () => { 
                    settingsModal.querySelectorAll('.tab-button, .tab-content').forEach(el => el.classList.remove('active')); 
                    button.classList.add('active'); 
                    document.getElementById(button.dataset.tab)?.classList.add('active'); 
                }); 
            });

            checkForUpdatesBtn.addEventListener('click', () => { updateStatusText.textContent = App.i18n.t('update_checking'); checkForUpdatesBtn.disabled = true; window.api.checkForUpdates(); });
            window.api.onUpdateStatus(({ status, message }) => { checkForUpdatesBtn.disabled = false; let version = message.includes(' ') ? message.split(' ').pop() : ''; switch (status) { case 'available': updateStatusText.textContent = App.i18n.t('update_available', { version }); break; case 'downloading': updateStatusText.textContent = App.i18n.t('update_downloading', { percent: message.match(/\d+/)[0] }); checkForUpdatesBtn.disabled = true; break; case 'downloaded': updateStatusText.textContent = App.i18n.t('update_downloaded'); break; case 'error': updateStatusText.textContent = App.i18n.t('update_error', { message }); break; case 'latest': updateStatusText.textContent = App.i18n.t('update_latest'); break; default: updateStatusText.textContent = App.i18n.t('update_check_prompt'); } });
        }
        
        return {
            init,
            openSettingsModal,
            closeAll: () => utils.closeModal(settingsModal)
        };
    };
})(window);