// --- START OF FILE js/modals/settings-handler.js ---
(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createSettingsModalHandler = function(App, utils) {
        const stateManager = App.stateManager;

        let settingsModal, settingsModalCloseBtn, saveSettingsBtn,
            languageSelect, selectRecPathBtn, checkForUpdatesBtn,
            settingsModalTitle, settingsIframe;

        let updateStatusText, updateInfoContainer, updateVersionTitle, 
            updateChangelog, downloadUpdateBtn, quitAndInstallBtn;
        
        function openSettingsModal(camera = null) {
            if (!settingsModal) return;

            const isCameraSettings = !!(camera && camera.ip);
            
            settingsModal.classList.toggle('camera-mode', isCameraSettings);
            
            if (isCameraSettings) {
                settingsModalTitle.textContent = `${App.t('context_settings')}: ${camera.name}`;
                if (settingsIframe) {
                    settingsIframe.src = 'about:blank';
                    setTimeout(() => {
                        settingsIframe.src = `http://${camera.ip}`;
                    }, 50);
                }
            } else {
                settingsModalTitle.textContent = App.t('general_settings_title');
                const { appSettings } = stateManager.state;
                if(languageSelect) languageSelect.value = appSettings.language || 'en';
                const recordingsPathInput = document.getElementById('app-settings-recordings-path');
                if(recordingsPathInput) recordingsPathInput.value = appSettings.recordingsPath || '';
                const hwAccelSelect = document.getElementById('app-settings-hw-accel');
                if(hwAccelSelect) hwAccelSelect.value = appSettings.hwAccel || 'auto';
                const notificationsCheckbox = document.getElementById('app-settings-notifications-enabled');
                if(notificationsCheckbox) notificationsCheckbox.checked = appSettings.notifications_enabled !== false;
                const qscaleSlider = document.getElementById('app-settings-qscale');
                const qscaleValue = document.getElementById('app-settings-qscale-value');
                if(qscaleSlider) qscaleSlider.value = appSettings.qscale || 8;
                if(qscaleValue) qscaleValue.textContent = appSettings.qscale || 8;
                const fpsInput = document.getElementById('app-settings-fps');
                if(fpsInput) fpsInput.value = appSettings.fps || 20;

                if (App.versionType === 'intellect') {
                    const analyticsProviderSelect = document.getElementById('app-settings-analytics-provider');
                    if (analyticsProviderSelect) analyticsProviderSelect.value = appSettings.analytics_provider || 'auto';
                    renderModulesTab();
                }
            }

            const firstVisibleTab = settingsModal.querySelector('.tab-button:not(.hidden)');
            if (firstVisibleTab) {
                firstVisibleTab.click();
            }
            
            utils.openModal(settingsModal);
        }
        
        async function saveGeneralSettings() {
            const appSettingsToSave = {
                language: document.getElementById('app-settings-language').value,
                recordingsPath: document.getElementById('app-settings-recordings-path').value,
                hwAccel: document.getElementById('app-settings-hw-accel').value,
                notifications_enabled: document.getElementById('app-settings-notifications-enabled').checked,
                qscale: document.getElementById('app-settings-qscale').value,
                fps: document.getElementById('app-settings-fps').value,
            };

            if (App.versionType === 'intellect') {
                appSettingsToSave.analytics_provider = document.getElementById('app-settings-analytics-provider').value;

                const enabledModuleIds = Array.from(document.querySelectorAll('.module-checkbox:checked')).map(cb => cb.dataset.id);
                appSettingsToSave.enabledModules = enabledModuleIds;
                
                const modulesWereChanged = JSON.stringify(enabledModuleIds) !== JSON.stringify(stateManager.state.appSettings.enabledModules || []);

                if (modulesWereChanged) {
                    await window.api.saveEnabledModules(enabledModuleIds);
                }
            }

            stateManager.setAppSettings(appSettingsToSave);
            utils.showToast(App.t('app_settings_saved_success'));
            utils.closeModal(settingsModal);
        }
        
        async function renderModulesTab() {
            const modulesListEl = document.getElementById('modules-list');
            if (!modulesListEl) return;
            modulesListEl.innerHTML = `<p>${App.i18n.t('loading_text')}</p>`;
            try {
                if (App.versionType === 'lite') {
                    modulesListEl.innerHTML = ''; return;
                }
                const availableModules = await window.api.getAvailableModules();
                const { appSettings } = stateManager.state;
                const enabledModules = new Set(appSettings.enabledModules || []);
                modulesListEl.innerHTML = ''; 
                if (availableModules.length === 0) {
                    modulesListEl.innerHTML = `<p>Модули не найдены в папке /modules.</p>`; return;
                }
                availableModules.forEach(mod => {
                    const isChecked = enabledModules.has(mod.id);
                    const description = App.t(`module_${mod.id}_description`) || mod.description;
                    const author = App.t(`module_${mod.id}_author`) || mod.author;
                    let moduleHtml = `<div class="form-check-inline" style="margin-bottom: 15px; border-bottom: 1px solid #eee; padding-bottom: 10px; width: 100%; align-items: flex-start;"><input type="checkbox" id="module-${mod.id}" data-id="${mod.id}" class="form-check-input module-checkbox" ${isChecked ? 'checked' : ''} style="margin-top: 5px;"><div style="flex-grow: 1;"><label for="module-${mod.id}" style="font-weight: bold; font-size: 1.1em;">${mod.name} <span style="font-size: 0.8em; color: #666;">v${mod.version}</span></label><p style="margin: 5px 0 0 0; font-size: 0.9em; color: #333;">${description}</p><p style="margin: 5px 0 0 0; font-size: 0.8em; color: #888;">${App.t('author_prefix') || 'Автор'}: ${author}</p></div></div>`;
                    if (mod.id === 'face-detector' && isChecked) {
                        const savePathKey = `module_face-detector_savePath`;
                        const currentPath = appSettings[savePathKey] || App.t('module_face-detector_default_path');
                        moduleHtml += `<div class="form-grid simple with-button" style="grid-template-columns: 150px 1fr; margin-left: 35px; margin-bottom: 15px;"><span data-i18n-key="module_face-detector_save_path_label"></span><div class="form-input-wrapper"><input type="text" id="face-detector-path" data-key="${savePathKey}" value="${currentPath}" readonly><button class="select-face-path-btn" style="padding: 0 10px; min-width: 40px; height: 35px;"><i class="material-icons" style="font-size: 20px;">folder_open</i></button></div></div>`;
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
                            pathInput.value = result.filePaths[0];
                            stateManager.setAppSettings({ [pathInput.dataset.key]: result.filePaths[0] });
                        }
                    });
                }
            } catch (error) {
                console.error('Failed to render modules tab:', error);
                modulesListEl.innerHTML = `<p style="color: var(--danger-color);">${App.i18n.t('error')}: ${error.message}</p>`;
            }
        }

        function handleUpdateStatus(status, info) {
            if (!updateStatusText) return;

            updateInfoContainer.classList.add('hidden');
            downloadUpdateBtn.classList.add('hidden');
            if (quitAndInstallBtn) quitAndInstallBtn.classList.add('hidden');

            switch(status) {
                case 'checking':
                    updateStatusText.textContent = App.t('update_checking');
                    checkForUpdatesBtn.disabled = true;
                    break;
                case 'available':
                    updateStatusText.textContent = App.t('update_available', { version: info.version });
                    updateInfoContainer.classList.remove('hidden');
                    updateVersionTitle.textContent = App.t('update_version_title', { version: info.version });
                    updateChangelog.textContent = info.releaseNotes || App.t('update_no_changelog');
                    downloadUpdateBtn.classList.remove('hidden');
                    downloadUpdateBtn.disabled = false;
                    checkForUpdatesBtn.disabled = false;
                    break;
                case 'downloading':
                    updateStatusText.textContent = App.t('update_downloading', { percent: info.percent.toFixed(0) });
                    updateInfoContainer.classList.remove('hidden'); // Показываем, чтобы видеть прогресс
                    downloadUpdateBtn.classList.remove('hidden');
                    downloadUpdateBtn.disabled = true;
                    checkForUpdatesBtn.disabled = true;
                    break;
                case 'downloaded':
                    updateStatusText.textContent = App.t('update_downloaded');
                    updateInfoContainer.classList.remove('hidden');
                    checkForUpdatesBtn.disabled = true;
                    downloadUpdateBtn.classList.add('hidden');
                    if (!quitAndInstallBtn) {
                        quitAndInstallBtn = document.createElement('button');
                        quitAndInstallBtn.id = 'quit-and-install-btn';
                        quitAndInstallBtn.textContent = 'Перезапустить и установить';
                        quitAndInstallBtn.style.marginTop = '10px';
                        quitAndInstallBtn.onclick = () => window.api.quitAndInstallUpdate();
                        updateInfoContainer.appendChild(quitAndInstallBtn);
                    }
                    quitAndInstallBtn.classList.remove('hidden');
                    break;
                case 'error':
                    updateStatusText.textContent = App.t('update_error', { message: info.message });
                    checkForUpdatesBtn.disabled = false;
                    break;
                case 'latest':
                    updateStatusText.textContent = App.t('update_latest');
                    checkForUpdatesBtn.disabled = false;
                    break;
                default:
                    updateStatusText.textContent = App.t('update_check_prompt');
                    checkForUpdatesBtn.disabled = false;
            }
        }

        function init() {
            settingsModal = document.getElementById('settings-modal');
            settingsModalCloseBtn = document.getElementById('settings-modal-close-btn');
            settingsModalTitle = document.getElementById('settings-modal-title');
            settingsIframe = document.getElementById('settings-iframe');
            saveSettingsBtn = document.getElementById('save-settings-btn');
            languageSelect = document.getElementById('app-settings-language');
            selectRecPathBtn = document.getElementById('select-rec-path-btn');
            checkForUpdatesBtn = document.getElementById('check-for-updates-btn');

            updateStatusText = document.getElementById('update-status-text');
            updateInfoContainer = document.getElementById('update-info-container');
            updateVersionTitle = document.getElementById('update-version-title');
            updateChangelog = document.getElementById('update-changelog');
            downloadUpdateBtn = document.getElementById('download-update-btn');
            
            if (settingsModalCloseBtn) {
                settingsModalCloseBtn.addEventListener('click', () => {
                    if (settingsIframe) settingsIframe.src = 'about:blank';
                    utils.closeModal(settingsModal);
                });
            }
            
            if (saveSettingsBtn) {
                saveSettingsBtn.addEventListener('click', saveGeneralSettings);
            }

            if (languageSelect) {
                languageSelect.addEventListener('change', () => {
                    App.i18n.setLanguage(languageSelect.value);
                });
            }

            if (settingsModal) {
                const modalFooter = settingsModal.querySelector('.modal-footer');
                const saveButton = modalFooter.querySelector('#save-settings-btn');
                const reportButton = modalFooter.querySelector('#report-issue-btn');

                settingsModal.querySelectorAll('.tab-button').forEach(button => { 
                    button.addEventListener('click', () => { 
                        settingsModal.querySelectorAll('.tab-button, .tab-content').forEach(el => el.classList.remove('active')); 
                        button.classList.add('active'); 
                        const content = document.getElementById(button.dataset.tab);
                        if (content) content.classList.add('active'); 
                        if (modalFooter) {
                            const isAboutTab = button.dataset.tab === 'tab-about';
                            const isGeneralTab = button.dataset.tab === 'tab-general';
                            modalFooter.style.display = isAboutTab ? 'none' : 'flex';
                            if (saveButton) saveButton.style.display = 'flex';
                            if (reportButton) reportButton.style.display = isGeneralTab ? 'flex' : 'none';
                        }
                    }); 
                });
            }
            
            if (selectRecPathBtn) {
                selectRecPathBtn.addEventListener('click', async () => {
                    const result = await window.api.selectDirectory();
                    if (!result.canceled && result.filePaths.length > 0) {
                        document.getElementById('app-settings-recordings-path').value = result.filePaths[0];
                    }
                });
            }

            if (checkForUpdatesBtn) {
                checkForUpdatesBtn.addEventListener('click', () => {
                    handleUpdateStatus('checking');
                    window.api.checkForUpdates();
                });
            }

            if (downloadUpdateBtn) {
                downloadUpdateBtn.addEventListener('click', () => {
                    downloadUpdateBtn.disabled = true;
                    window.api.downloadUpdate();
                });
            }
            
            window.api.onUpdateStatus((data) => {
                if (settingsModal && !settingsModal.classList.contains('hidden')) {
                    handleUpdateStatus(data.status, data.info || data);
                }
            });

            const qscaleSlider = document.getElementById('app-settings-qscale');
            const qscaleValue = document.getElementById('app-settings-qscale-value');
            if (qscaleSlider && qscaleValue) {
                qscaleSlider.addEventListener('input', () => {
                    qscaleValue.textContent = qscaleSlider.value;
                });
            }
        }
        
        return {
            init,
            openSettingsModal,
            closeAll: () => {
                if (settingsModal) utils.closeModal(settingsModal);
            }
        };
    };
})(window);
// --- END OF FILE js/modals/settings-handler.js ---