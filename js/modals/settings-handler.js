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
        const analyticsProviderSelect = document.getElementById('app-settings-analytics-provider');
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

        const FIELD_DEFINITIONS = {
            // System
            logLevel: { type: 'select', options: ['verbose', 'debug', 'info', 'warn', 'error'] },
            // ISP
            slowShutter: { type: 'select', options: ['auto', 'fast', 'medium', 'slow'] },
            rawMode: { type: 'select', options: ['auto', 'raw', 'yuv', 'slow'] },
            memMode: { type: 'select', options: ['single', 'continuous', 'reduction'] },
            antiFlicker: { type: 'select', options: ['disabled', '50hz', '60hz'] },
            // Image
            contrast: { type: 'range', min: 0, max: 100 },
            hue: { type: 'range', min: 0, max: 100 },
            saturation: { type: 'range', min: 0, max: 100 },
            luminance: { type: 'range', min: 0, max: 100 },
            // --- Специальные правила для полей с одинаковыми именами ---
            codec: {
                type: 'select',
                _perSection: {
                    video0: { options: ['h264', 'h265', 'mjpeg'] },
                    video1: { options: ['h264', 'h265', 'mjpeg'] },
                    audio: { options: ['g711a', 'g711u', 'aac'] }
                }
            },
            // Video0 & Video1
            rcMode: { type: 'select', options: ['cbr', 'vbr'] },
            profile: { type: 'select', options: ['baseline', 'main', 'high'] },
            gopMode: { type: 'select', options: ['normal', 'dual', 'smart'] },
            // Audio
            srate: {
                type: 'select',
                _perSection: {
                    audio: { options: [8000, 16000, 32000, 44100, 48000] }
                }
            }
        };
        
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

        function clearDynamicSettings() {
            settingsModal.querySelectorAll('.tab-content.dynamic').forEach(tab => {
                tab.innerHTML = '';
                tab.classList.remove('dynamic');
            });
        }
        
        function formatLabel(key) {
            const result = key.replace(/([A-Z])/g, ' $1');
            return result.charAt(0).toUpperCase() + result.slice(1);
        }

        function createSettingInput(section, key, value) {
            const id = `${section}.${key}`;
            let definition = FIELD_DEFINITIONS[key] || {};
            if (definition._perSection && definition._perSection[section]) {
                definition = { ...definition, ...definition._perSection[section] };
            }
            
            const type = definition.type || (typeof value === 'boolean' ? 'boolean' : (typeof value === 'number' ? 'number' : 'string'));
            
            const labelText = formatLabel(key);
            const labelHtml = `<label class="form-label" for="${id}">${labelText}</label>`;
            let inputHtml = '';
            
            switch (type) {
                case 'boolean':
                    const checked = value ? 'checked' : '';
                    inputHtml = `
                        <div class="p-boolean">
                            ${labelHtml}
                            <div class="form-check form-switch">
                                <input type="checkbox" id="${id}" name="${id}" class="form-check-input" ${checked}>
                            </div>
                        </div>`;
                    break;
                
                case 'select':
                    let optionsHtml = '';
                    (definition.options || []).forEach(opt => {
                        const selected = opt == value ? 'selected' : '';
                        optionsHtml += `<option value="${opt}" ${selected}>${opt}</option>`;
                    });
                    inputHtml = `
                        <div class="p-setting">
                            ${labelHtml}
                            <div class="input-group">
                                <select class="form-select" id="${id}" name="${id}">${optionsHtml}</select>
                            </div>
                        </div>`;
                    break;
                
                case 'range':
                    const min = definition.min !== undefined ? definition.min : 0;
                    const max = definition.max !== undefined ? definition.max : 100;
                    inputHtml = `
                        <div class="p-setting">
                            ${labelHtml}
                            <div class="input-group range-slider-wrapper">
                                <input type="range" id="${id}" name="${id}" class="form-range" value="${value}" min="${min}" max="${max}">
                                <span class="range-value">${value}</span>
                            </div>
                        </div>`;
                    break;
                
                case 'number':
                     inputHtml = `
                        <div class="p-setting">
                            ${labelHtml}
                            <div class="input-group">
                                <input type="number" id="${id}" name="${id}" class="form-control text-end" value="${value}">
                            </div>
                        </div>`;
                    break;

                default: // string
                     inputHtml = `
                        <div class="p-setting">
                            ${labelHtml}
                            <div class="input-group">
                                <input type="text" id="${id}" name="${id}" class="form-control" value="${value}">
                            </div>
                        </div>`;
                    break;
            }
            return inputHtml;
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
        
        async function renderModulesTab() {
            const modulesListEl = document.getElementById('modules-list');
            if (!modulesListEl) return;

            modulesListEl.innerHTML = `<p>${App.i18n.t('loading_text')}</p>`;
            
            try {
                const availableModules = await window.api.getAvailableModules();
                const { appSettings } = stateManager.state;
                const enabledModules = new Set(appSettings.enabledModules || []);

                modulesListEl.innerHTML = ''; 

                if (availableModules.length === 0) {
                    modulesListEl.innerHTML = `<p>Модули не найдены в папке /modules.</p>`;
                    return;
                }

                availableModules.forEach(mod => {
                    const isChecked = enabledModules.has(mod.id);
                    const description = App.t(`module_${mod.id}_description`) || mod.description;
                    const author = App.t(`module_${mod.id}_author`) || mod.author;

                    let moduleHtml = `
                        <div class="form-check-inline" style="margin-bottom: 15px; border-bottom: 1px solid #eee; padding-bottom: 10px; width: 100%; align-items: flex-start;">
                            <input type="checkbox" id="module-${mod.id}" data-id="${mod.id}" class="form-check-input module-checkbox" ${isChecked ? 'checked' : ''} style="margin-top: 5px;">
                            <div style="flex-grow: 1;">
                                <label for="module-${mod.id}" style="font-weight: bold; font-size: 1.1em;">${mod.name} <span style="font-size: 0.8em; color: #666;">v${mod.version}</span></label>
                                <p style="margin: 5px 0 0 0; font-size: 0.9em; color: #333;">${description}</p>
                                <p style="margin: 5px 0 0 0; font-size: 0.8em; color: #888;">${App.t('author_prefix') || 'Автор'}: ${author}</p>
                            </div>
                        </div>
                    `;
                    
                    if (mod.id === 'face-detector' && isChecked) {
                        const savePathKey = `module_face-detector_savePath`;
                        const currentPath = appSettings[savePathKey] || App.t('module_face-detector_default_path');

                        moduleHtml += `
                            <div class="form-grid simple with-button" style="grid-template-columns: 150px 1fr; margin-left: 35px; margin-bottom: 15px;">
                                <span data-i18n-key="module_face-detector_save_path_label"></span>
                                <div class="form-input-wrapper">
                                    <input type="text" id="face-detector-path" data-key="${savePathKey}" value="${currentPath}" readonly>
                                    <button class="select-face-path-btn" style="padding: 0 10px; min-width: 40px; height: 35px;"><i class="material-icons" style="font-size: 20px;">folder_open</i></button>
                                </div>
                            </div>
                        `;
                    }
                    modulesListEl.innerHTML += moduleHtml;
                });

                App.i18n.applyTranslationsToDOM(modulesListEl);

                const selectFacePathBtn = document.querySelector('.select-face-path-btn');
                if (selectFacePathBtn) {
                    selectFacePathBtn.addEventListener('click', async () => {
                        const result = await window.api.selectDirectory();
                        if (!result.canceled && result.filePaths.length > 0) {
                            const pathInput = document.getElementById('face-detector-path');
                            const newPath = result.filePaths[0];
                            pathInput.value = newPath;
                            
                            stateManager.setAppSettings({ [pathInput.dataset.key]: newPath });
                        }
                    });
                }
            } catch (error) {
                console.error('Failed to render modules tab:', error);
                modulesListEl.innerHTML = `<p style="color: var(--danger-color);">${App.i18n.t('error')}: ${error.message}</p>`;
            }
        }
        
        async function openSettingsModal(cameraId = null) {
            App.i18n.applyTranslationsToDOM();
            clearDynamicSettings();
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
                const isModulesTab = tab === 'tab-modules';
                // VVVVVV --- ИЗМЕНЕНИЕ --- VVVVVV
                const isAboutTab = tab === 'tab-about'; 
                const isMajesticOrNetipTab = !isGeneralTab && !isAnalyticsTab && !isStreamingTab && !isModulesTab && !isAboutTab;
                // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

                let show = false;
                if (isGeneralSettings) {
                    // VVVVVV --- ИЗМЕНЕНИЕ --- VVVVVV
                    show = isGeneralTab || isStreamingTab || isAnalyticsTab || isModulesTab || isAboutTab;
                    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
                } else {
                    if (isNetipCamera) {
                        show = tab === 'tab-netip' || isAnalyticsTab;
                    } else {
                        show = isMajesticOrNetipTab || isAnalyticsTab;
                    }
                }
                btn.style.display = show ? 'flex' : 'none';
            });
            
            settingsModal.querySelectorAll('.tab-content, .tab-button').forEach(el => el.classList.remove('active'));
            
            let activeTab = isGeneralSettings ? 'tab-general' : (isNetipCamera ? 'tab-netip' : 'tab-system');
            
            const activeButton = tabsContainer.querySelector(`[data-tab="${activeTab}"]`);
            const activeContent = document.getElementById(activeTab);

            if (activeButton) activeButton.classList.add('active');
            if (activeContent) activeContent.classList.add('active');

            const { appSettings } = stateManager.state;
            recordingsPathInput.value = appSettings.recordingsPath || '';
            languageSelect.value = appSettings.language || 'en';
            hwAccelSelect.value = appSettings.hwAccel || 'auto';
            analyticsProviderSelect.value = appSettings.analytics_provider || 'auto';
            setFormValue('app-settings-notifications-enabled', appSettings.notifications_enabled, true);
            setFormValue('app-settings-qscale', appSettings.qscale, 8);
            setFormValue('app-settings-fps', appSettings.fps, 20);
            setFormValue('app-settings-analytics-resize-width', appSettings.analytics_resize_width, 416);
            setFormValue('app-settings-analytics-frame-skip', appSettings.analytics_frame_skip, 10);
            setFormValue('app-settings-analytics-record-duration', appSettings.analytics_record_duration, 30);
            
            document.getElementById('global-analytics-settings').style.display = isGeneralSettings ? 'block' : 'none';
            document.getElementById('camera-specific-analytics-settings').style.display = isGeneralSettings ? 'none' : 'block';
            restartMajesticBtn.style.display = isGeneralSettings || (camera && camera.protocol === 'netip') ? 'none' : 'inline-flex';
            killAllBtnModal.style.display = isGeneralSettings ? 'inline-flex' : 'none';
            
            utils.openModal(settingsModal);

            // VVVVVV --- ИЗМЕНЕНИЕ: ЛОГИКА ДЛЯ ВКЛАДКИ "О ПРОГРАММЕ" --- VVVVVV
            if (isGeneralSettings) {
                window.api.getAppVersion().then(version => {
                    const versionEl = document.getElementById('app-version');
                    if (versionEl) {
                        versionEl.textContent = version;
                    }
                });
                
                const donateBtn = document.getElementById('donate-btn');
                if (donateBtn) {
                    donateBtn.onclick = () => {
                        window.api.openExternalLink('https://pay.web.money/d/r8qq'); 
                    };
                }
                renderModulesTab();
            }
            // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

            if (isGeneralSettings) {
                saveSettingsBtn.disabled = false;
                saveSettingsBtn.textContent = App.i18n.t('save');
                return;
            }

            const analyticsConfig = camera.analyticsConfig || {};
            setFormValue('analytics.enabled', analyticsConfig.enabled, false);
            analyticsObjectsListEl.innerHTML = '';
            availableAnalyticsObjects.forEach(obj => {
                const isChecked = analyticsConfig.objects && analyticsConfig.objects.includes(obj.key);
                analyticsObjectsListEl.innerHTML += `
                    <div class="form-check-inline">
                        <input type="checkbox" id="analytics.objects.${obj.key}" class="form-check-input" data-object-key="${obj.key}" ${isChecked ? 'checked' : ''}>
                        <label for="analytics.objects.${obj.key}">${obj.label}</label>
                    </div>`;
            });

            saveSettingsBtn.disabled = true;
            saveSettingsBtn.textContent = App.i18n.t('loading_text');
            try {
                if (!isNetipCamera) {
                    const settings = await window.api.getCameraSettings(camera);
                    if (settings && !settings.error) {
                        for (const section in settings) {
                            const tabContent = document.getElementById(`tab-${section}`);
                            if (tabContent && typeof settings[section] === 'object' && settings[section] !== null) {
                                tabContent.classList.add('dynamic');
                                let sectionHtml = '';
                                for (const key in settings[section]) {
                                    sectionHtml += createSettingInput(section, key, settings[section][key]);
                                }
                                tabContent.innerHTML = sectionHtml;
                            }
                        }
                        
                        settingsModal.querySelectorAll('.form-range').forEach(slider => {
                            const valueSpan = slider.parentElement.querySelector('.range-value');
                            if (valueSpan) {
                                slider.addEventListener('input', () => {
                                    valueSpan.textContent = slider.value;
                                });
                            }
                        });
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
                const currentlyEnabled = stateManager.state.appSettings.enabledModules || [];
                const checkedCheckboxes = document.querySelectorAll('#modules-list .module-checkbox:checked');
                const newEnabledIds = Array.from(checkedCheckboxes).map(cb => cb.dataset.id);

                const hasChanges = JSON.stringify([...currentlyEnabled].sort()) !== JSON.stringify([...newEnabledIds].sort());

                if (hasChanges) {
                    await window.api.saveEnabledModules(newEnabledIds);
                }

                const moduleSettingsInputs = document.querySelectorAll('#modules-list [data-key]');
                const newModuleSettings = {};
                moduleSettingsInputs.forEach(input => {
                    newModuleSettings[input.dataset.key] = input.value;
                });

                stateManager.setAppSettings({
                    ...newModuleSettings,
                    recordingsPath: recordingsPathInput.value.trim(),
                    hwAccel: hwAccelSelect.value,
                    language: languageSelect.value,
                    analytics_provider: analyticsProviderSelect.value,
                    notifications_enabled: notificationsEnabledInput.checked,
                    qscale: parseInt(qscaleInput.value, 10) || 8,
                    fps: parseInt(fpsInput.value, 10) || 20,
                    analytics_resize_width: parseInt(globalAnalyticsResizeWidthInput.value, 10) || 416,
                    analytics_frame_skip: parseInt(globalAnalyticsFrameSkipInput.value, 10) || 10,
                    analytics_record_duration: parseInt(document.getElementById('app-settings-analytics-record-duration').value, 10) || 30,
                    enabledModules: newEnabledIds
                });

                if (!hasChanges) {
                    utils.showToast(App.i18n.t('app_settings_saved_success'));
                }
            } else {
                const camera = stateManager.state.cameras.find(c => c.id === settingsCameraId);
                if (!camera) { saveSettingsBtn.disabled = false; saveSettingsBtn.textContent = App.i18n.t('save'); return; }
                
                if (camera.protocol !== 'netip') {
                    const settingsDataToSend = {};
                    settingsModal.querySelectorAll('.tab-content.dynamic [name]').forEach(el => {
                        const [section, key] = el.name.split('.');
                        if (!section || !key) return;
                        if (!settingsDataToSend[section]) settingsDataToSend[section] = {};

                        if (el.type === 'checkbox') {
                            settingsDataToSend[section][key] = el.checked;
                        } else if (el.value !== '' && el.value !== null) {
                            const isNumeric = el.type === 'number' || el.type === 'range' || !isNaN(el.value);
                            const val = isNumeric ? Number(el.value) : el.value;
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
            if (!document.querySelector('#settings-modal.hidden')) {
                saveSettingsBtn.disabled = false;
                saveSettingsBtn.textContent = App.i18n.t('save');
            }
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
            
            const reportIssueBtn = document.getElementById('report-issue-btn');
            if (reportIssueBtn) {
                reportIssueBtn.addEventListener('click', () => {
                    utils.closeModal(settingsModal);
                    App.modalHandler.showReportModal();
                });
            }
            
            killAllBtnModal.addEventListener('click', async () => {
                const confirmation = await App.modalHandler.showPrompt({
                    title: App.i18n.t('settings_kill_all'),
                    label: App.i18n.t('kill_all_confirm'),
                    okText: App.i18n.t('settings_kill_all'),
                    cancelText: App.i18n.t('cancel'),
                    inputType: 'none'
                });

                if (confirmation !== null) {
                    const result = await window.api.killAllFfmpeg();
                    App.modalHandler.showToast(result.message); 
                    setTimeout(() => window.location.reload(), 1500); 
                }
            });

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
                if (!result.canceled && result.filePaths.length > 0) { // <-- ИСПРАВЛЕНИЕ ЗДЕСЬ
                    recordingsPathInput.value = result.filePaths[0]; // <-- И ИСПРАВЛЕНИЕ ЗДЕСЬ
                    stateManager.setAppSettings({ recordingsPath: result.filePaths[0] });
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