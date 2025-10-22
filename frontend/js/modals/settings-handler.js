// --- START OF FILE js/modals/settings-handler.js ---
(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createSettingsModalHandler = function(App, utils) {
        const stateManager = App.stateManager;

        let settingsModal, settingsModalCloseBtn, saveSettingsBtn,
            languageSelect, selectRecPathBtn, checkForUpdatesBtn,
            settingsModalTitle, settingsIframe, reportIssueBtn;

        let selectScreenshotsPathBtn;

        const delay = (ms) => new Promise(resolve => setTimeout(resolve, ms));

        let runtimeOverlay;
        let runtimeOverlayText;
        let runtimeOverlayDetail;
        let runtimeOverlayBar;
        let runtimeOverlayPercent;
        let runtimeOverlayRestartNote;
        let runtimeOverlayHandler;

        function ensureRuntimeOverlay() {
            if (runtimeOverlay) return;

            runtimeOverlay = document.createElement('div');
            runtimeOverlay.id = 'license-plate-runtime-overlay';
            runtimeOverlay.style.position = 'fixed';
            runtimeOverlay.style.inset = '0';
            runtimeOverlay.style.background = 'rgba(12, 15, 32, 0.58)';
            runtimeOverlay.style.display = 'none';
            runtimeOverlay.style.alignItems = 'center';
            runtimeOverlay.style.justifyContent = 'center';
            runtimeOverlay.style.zIndex = '14000';
            runtimeOverlay.style.padding = '32px';

            const card = document.createElement('div');
            card.style.background = '#fff';
            card.style.borderRadius = '14px';
            card.style.boxShadow = '0 24px 60px rgba(16, 21, 46, 0.25)';
            card.style.padding = '26px 30px';
            card.style.width = '420px';
            card.style.maxWidth = '100%';
            card.style.color = '#1e2438';
            card.style.fontFamily = 'inherit';

            const title = document.createElement('h3');
            title.style.margin = '0 0 10px';
            title.style.fontSize = '20px';
            title.style.fontWeight = '600';
            title.textContent = App.t('license_plate_runtime_preparing_title') || 'Enabling license plate module';
            card.appendChild(title);

            runtimeOverlayText = document.createElement('div');
            runtimeOverlayText.style.fontSize = '14px';
            runtimeOverlayText.style.lineHeight = '1.55';
            runtimeOverlayText.style.marginBottom = '6px';
            runtimeOverlayText.textContent = App.t('license_plate_runtime_preparing') || 'Preparing runtime files...';
            card.appendChild(runtimeOverlayText);

            runtimeOverlayDetail = document.createElement('div');
            runtimeOverlayDetail.style.fontSize = '12px';
            runtimeOverlayDetail.style.color = '#5a6277';
            runtimeOverlayDetail.style.marginBottom = '14px';
            runtimeOverlayDetail.textContent = '';
            card.appendChild(runtimeOverlayDetail);

            const progressTrack = document.createElement('div');
            progressTrack.style.height = '8px';
            progressTrack.style.borderRadius = '4px';
            progressTrack.style.background = '#e4e7f2';
            progressTrack.style.overflow = 'hidden';
            const progressBar = document.createElement('div');
            progressBar.style.height = '100%';
            progressBar.style.width = '0%';
            progressBar.style.background = '#3665ff';
            progressBar.style.transition = 'width 0.25s ease';
            progressTrack.appendChild(progressBar);
            card.appendChild(progressTrack);
            runtimeOverlayBar = progressBar;

            runtimeOverlayPercent = document.createElement('div');
            runtimeOverlayPercent.style.fontSize = '12px';
            runtimeOverlayPercent.style.color = '#5a6277';
            runtimeOverlayPercent.style.marginTop = '8px';
            runtimeOverlayPercent.textContent = '';
            card.appendChild(runtimeOverlayPercent);

            runtimeOverlayRestartNote = document.createElement('div');
            runtimeOverlayRestartNote.style.marginTop = '16px';
            runtimeOverlayRestartNote.style.fontSize = '13px';
            runtimeOverlayRestartNote.style.color = '#1d6f3d';
            runtimeOverlayRestartNote.style.display = 'none';
            runtimeOverlayRestartNote.textContent = App.t('license_plate_runtime_restart_prompt') || 'Restart the application to finish enabling the module.';
            card.appendChild(runtimeOverlayRestartNote);

            runtimeOverlay.appendChild(card);
            document.body.appendChild(runtimeOverlay);
        }

        function formatRuntimeBytes(value) {
            if (!Number.isFinite(value) || value <= 0) return '';
            const units = ['B', 'KB', 'MB', 'GB'];
            let size = value;
            let unitIndex = 0;
            while (size >= 1024 && unitIndex < units.length - 1) {
                size /= 1024;
                unitIndex += 1;
            }
            const formatted = size >= 10 || unitIndex === 0 ? size.toFixed(0) : size.toFixed(1);
            return `${formatted} ${units[unitIndex]}`;
        }

        function setRuntimeOverlayVisibility(visible) {
            ensureRuntimeOverlay();
            runtimeOverlay.style.display = visible ? 'flex' : 'none';
            if (!visible) {
                runtimeOverlayDetail.textContent = '';
                runtimeOverlayPercent.textContent = '';
                runtimeOverlayRestartNote.style.display = 'none';
                runtimeOverlayBar.style.width = '0%';
                runtimeOverlayBar.style.background = '#3665ff';
            }
        }

        function updateRuntimeOverlayState({ message, detail, progress, tone, showRestart }) {
            if (!runtimeOverlay) return;
            if (message) runtimeOverlayText.textContent = message;
            if (detail !== undefined) runtimeOverlayDetail.textContent = detail;
            if (typeof progress === 'number') {
                const clamped = Math.max(0, Math.min(1, progress));
                runtimeOverlayBar.style.opacity = '1';
                runtimeOverlayBar.style.width = `${Math.round(clamped * 100)}%`;
                runtimeOverlayPercent.textContent = `${Math.round(clamped * 100)}%`;
            } else if (progress === null) {
                runtimeOverlayBar.style.opacity = '0.3';
                runtimeOverlayBar.style.width = '100%';
                runtimeOverlayPercent.textContent = '';
            }
            if (tone === 'error') {
                runtimeOverlayBar.style.background = '#d64545';
                runtimeOverlayText.style.color = '#b33232';
            } else {
                runtimeOverlayBar.style.background = '#3665ff';
                runtimeOverlayText.style.color = '#1e2438';
            }
            runtimeOverlayRestartNote.style.display = showRestart ? 'block' : 'none';
        }

        function handleRuntimeOverlayProgress(payload = {}) {
            if (!payload.status) return;
            switch (payload.status) {
                case 'checking':
                    updateRuntimeOverlayState({
                        message: App.t('license_plate_runtime_preparing') || 'Preparing runtime files...',
                        detail: '',
                        progress: null,
                        tone: 'info',
                        showRestart: false
                    });
                    break;
                case 'downloading': {
                    const ratio = typeof payload.progress === 'number' ? Math.max(0, Math.min(1, payload.progress)) : null;
                    const percentLabel = ratio !== null ? `${Math.round(ratio * 100)}%` : '';
                    const bytesInfo = payload.total ? `${formatRuntimeBytes(payload.downloaded || 0)} / ${formatRuntimeBytes(payload.total)}` : '';
                    updateRuntimeOverlayState({
                        message: (App.t('license_plate_runtime_downloading') || 'Downloading runtime...').replace('{percent}', percentLabel),
                        detail: bytesInfo,
                        progress: ratio !== null ? ratio : null,
                        tone: 'info',
                        showRestart: false
                    });
                    break;
                }
                case 'verifying':
                    updateRuntimeOverlayState({
                        message: App.t('license_plate_runtime_verifying') || 'Verifying files...',
                        detail: '',
                        progress: null,
                        tone: 'info',
                        showRestart: false
                    });
                    break;
                case 'extracting':
                    updateRuntimeOverlayState({
                        message: App.t('license_plate_runtime_extracting') || 'Extracting runtime...',
                        detail: '',
                        progress: payload.progress !== undefined ? payload.progress : null,
                        tone: 'info',
                        showRestart: false
                    });
                    break;
                case 'ready':
                    updateRuntimeOverlayState({
                        message: App.t('license_plate_runtime_ready') || 'Runtime installed successfully.',
                        detail: '',
                        progress: 1,
                        tone: 'success',
                        showRestart: true
                    });
                    break;
                case 'error':
                    updateRuntimeOverlayState({
                        message: (App.t('license_plate_runtime_failed') || 'Runtime installation failed: {message}').replace('{message}', payload.message || 'unknown error'),
                        detail: '',
                        progress: null,
                        tone: 'error',
                        showRestart: false
                    });
                    break;
                default:
                    break;
            }
        }

        async function prepareLicensePlateRuntimeWithUi() {
            ensureRuntimeOverlay();
            setRuntimeOverlayVisibility(true);
            updateRuntimeOverlayState({
                message: App.t('license_plate_runtime_preparing') || 'Preparing runtime files...',
                detail: '',
                progress: null,
                tone: 'info',
                showRestart: false
            });

            runtimeOverlayHandler = (payload) => handleRuntimeOverlayProgress(payload || {});
            window.api.on('module-license-plate-runtime-progress', runtimeOverlayHandler);

            try {
                const response = await window.api.prepareLicensePlateRuntime();
                if (!response || response.success === false) {
                    const errorMsg = response && response.error ? response.error : 'unknown error';
                    handleRuntimeOverlayProgress({ status: 'error', message: errorMsg });
                    await delay(1600);
                    throw new Error(errorMsg);
                }

                if (response.alreadyInstalled) {
                    updateRuntimeOverlayState({
                        message: App.t('license_plate_runtime_already_installed') || 'Runtime already installed.',
                        detail: '',
                        progress: 1,
                        tone: 'success',
                        showRestart: true
                    });
                }

                await delay(900);
                setRuntimeOverlayVisibility(false);
                return response;
            } catch (error) {
                handleRuntimeOverlayProgress({ status: 'error', message: error && error.message ? error.message : String(error) });
                await delay(1200);
                setRuntimeOverlayVisibility(false);
                throw error;
            } finally {
                if (runtimeOverlayHandler && window.api.off) {
                    window.api.off('module-license-plate-runtime-progress', runtimeOverlayHandler);
                }
                runtimeOverlayHandler = null;
            }
        }

        let updateStatusText, updateInfoContainer, updateVersionTitle, 
            updateChangelog, downloadUpdateBtn, quitAndInstallBtn;
        

        async function openSettingsModal(camera = null) {
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
                const screenshotsPathInput = document.getElementById('app-settings-screenshots-path');
                if(screenshotsPathInput) screenshotsPathInput.value = appSettings.screenshotsPath || '';
                const hwAccelSelect = document.getElementById('app-settings-hw-accel');
                if(hwAccelSelect) hwAccelSelect.value = appSettings.hwAccel || 'auto';
                const notificationsCheckbox = document.getElementById('app-settings-notifications-enabled');
                if(notificationsCheckbox) notificationsCheckbox.checked = appSettings.notifications_enabled !== false;
                const useWebRTCCheckbox = document.getElementById('app-settings-use-webrtc');
                if(useWebRTCCheckbox) useWebRTCCheckbox.checked = appSettings.useWebRTC === true;
                const qscaleSlider = document.getElementById('app-settings-qscale');
                const qscaleValue = document.getElementById('app-settings-qscale-value');
                if(qscaleSlider) qscaleSlider.value = appSettings.qscale || 8;
                if(qscaleValue) qscaleValue.textContent = appSettings.qscale || 8;
                const fpsInput = document.getElementById('app-settings-fps');
                if(fpsInput) fpsInput.value = appSettings.fps || 20;
                // --- Analytics ---
                const resizeWidthInput = document.getElementById('app-settings-analytics-resize-width');
                if(resizeWidthInput) resizeWidthInput.value = appSettings.analytics_resize_width || 640;
                const frameSkipInput = document.getElementById('app-settings-analytics-frame-skip');
                if(frameSkipInput) frameSkipInput.value = appSettings.analytics_frame_skip || 5;
                const recordDurationInput = document.getElementById('app-settings-analytics-record-duration');
                if(recordDurationInput) recordDurationInput.value = appSettings.analytics_record_duration || 30;
                // --- Plate Recognition ---
                const minScoreInput = document.getElementById('plate-min-score');
                if(minScoreInput) minScoreInput.value = appSettings.plate_min_score || 0.65;
                const minAreaInput = document.getElementById('plate-min-area');
                if(minAreaInput) minAreaInput.value = appSettings.plate_min_area || 1000;
                const minHeightInput = document.getElementById('plate-min-height');
                if(minHeightInput) minHeightInput.value = appSettings.plate_min_height || 30;
                const minAspectInput = document.getElementById('plate-min-aspect');
                if(minAspectInput) minAspectInput.value = appSettings.plate_min_aspect || 1.2;
                const maxAspectInput = document.getElementById('plate-max-aspect');
                if(maxAspectInput) maxAspectInput.value = appSettings.plate_max_aspect || 7.5;
                const allowlistInput = document.getElementById('plate-allowlist');
                if(allowlistInput) allowlistInput.value = appSettings.plate_allowlist || 'АВЕКМНОРСТУХABEKMHOPCTYX0123456789';
                const maxCropsInput = document.getElementById('plate-max-crops-per-plate');
                if(maxCropsInput) maxCropsInput.value = appSettings.plate_max_crops_per_plate || 10;

                try {
                    const appVersionSpan = document.getElementById('app-version');
                    if (appVersionSpan) {
                        const versionInfo = await window.api.getAppVersionInfo();
                        appVersionSpan.textContent = versionInfo.version;
                    }
                } catch (e) {
                    console.error("Failed to get app version:", e);
                }

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
            const { appSettings } = stateManager.state;
            const plateMaxCropsInput = document.getElementById('plate-max-crops-per-plate');
            const plateMaxCropsRaw = plateMaxCropsInput ? parseInt(plateMaxCropsInput.value, 10) : NaN;
            const plateMaxCrops = Number.isFinite(plateMaxCropsRaw) && plateMaxCropsRaw > 0 ? plateMaxCropsRaw : 10;
            const newSettings = {
                language: document.getElementById('app-settings-language').value,
                recordingsPath: document.getElementById('app-settings-recordings-path').value,
                screenshotsPath: document.getElementById('app-settings-screenshots-path').value,
                hwAccel: document.getElementById('app-settings-hw-accel').value,
                notifications_enabled: document.getElementById('app-settings-notifications-enabled').checked,
                useWebRTC: document.getElementById('app-settings-use-webrtc').checked,
                qscale: document.getElementById('app-settings-qscale').value,
                fps: document.getElementById('app-settings-fps').value,
                analytics_resize_width: document.getElementById('app-settings-analytics-resize-width').value,
                analytics_frame_skip: document.getElementById('app-settings-analytics-frame-skip').value,
                analytics_record_duration: document.getElementById('app-settings-analytics-record-duration').value,
                // --- Plate Recognition ---
                plate_min_score: parseFloat(document.getElementById('plate-min-score').value),
                plate_min_area: parseInt(document.getElementById('plate-min-area').value, 10),
                plate_min_height: parseInt(document.getElementById('plate-min-height').value, 10),
                plate_min_aspect: parseFloat(document.getElementById('plate-min-aspect').value),
                plate_max_aspect: parseFloat(document.getElementById('plate-max-aspect').value),
                plate_allowlist: document.getElementById('plate-allowlist').value,
                plate_max_crops_per_plate: plateMaxCrops
            };

            // Определяем, изменились ли "критичные" для стриминга настройки
            const streamingSettingsChanged = 
                appSettings.hwAccel !== newSettings.hwAccel ||
                appSettings.qscale !== newSettings.qscale ||
                appSettings.fps !== newSettings.fps ||
                appSettings.useWebRTC !== newSettings.useWebRTC;

            if (App.versionType === 'intellect') {
                newSettings.analytics_provider = document.getElementById('app-settings-analytics-provider').value;

                const enabledModuleIds = Array.from(document.querySelectorAll('.module-checkbox:checked')).map(cb => cb.dataset.id);
                newSettings.enabledModules = enabledModuleIds;

                const previouslyEnabledModules = Array.isArray(stateManager.state.appSettings.enabledModules) ? stateManager.state.appSettings.enabledModules : [];
                const modulesWereChanged = JSON.stringify(enabledModuleIds) !== JSON.stringify(previouslyEnabledModules);
                const licensePlateWasEnabled = previouslyEnabledModules.includes('license-plate');
                const licensePlateWillBeEnabled = enabledModuleIds.includes('license-plate');
                const enablingLicensePlate = !licensePlateWasEnabled && licensePlateWillBeEnabled;

                if (modulesWereChanged) {
                    if (enablingLicensePlate) {
                        if (saveSettingsBtn) saveSettingsBtn.disabled = true;
                        try {
                            const prepareResult = await prepareLicensePlateRuntimeWithUi();
                            if (prepareResult && prepareResult.alreadyInstalled) {
                                utils.showToast(App.t('license_plate_runtime_already_installed') || 'Runtime already installed.');
                            } else {
                                utils.showToast(App.t('license_plate_runtime_ready_toast') || 'Runtime downloaded successfully.');
                            }
                        } catch (prepError) {
                            console.error('[Settings] Failed to prepare license plate runtime:', prepError);
                            utils.showToast((App.t('license_plate_runtime_failed_toast') || 'Failed to prepare runtime') + `: ${prepError && prepError.message ? prepError.message : prepError}`, true);
                            if (saveSettingsBtn) saveSettingsBtn.disabled = false;
                            return;
                        } finally {
                            if (saveSettingsBtn) saveSettingsBtn.disabled = false;
                        }
                    }
                    await window.api.saveEnabledModules(enabledModuleIds);
                }
            }

            // Сохраняем все новые настройки
            // --- Сохраняем настройки модуля License Plate Detector ---
            const lpFrameSkipInput = document.getElementById('license-plate-frame-skip');
            const lpResizeWidthInput = document.getElementById('license-plate-resize-width');
            if (lpFrameSkipInput) {
                newSettings['module_license-plate_frameSkip'] = parseInt(lpFrameSkipInput.value, 10) || 2;
            }
            if (lpResizeWidthInput) {
                newSettings['module_license-plate_resizeWidth'] = parseInt(lpResizeWidthInput.value, 10) || 0;
            }
            // License Plate: Use ONNX Runtime (DirectML)
            const lpUseOrtCheckbox = document.getElementById('license-plate-use-ort');
            if (lpUseOrtCheckbox) {
                newSettings['module_license-plate_use_ort'] = !!lpUseOrtCheckbox.checked;
            }
            // Persist app settings and wait for write to finish before closing modal
            stateManager.setAppSettings(newSettings);
            try {
                if (App && typeof App.saveAppSettings === 'function') {
                    await App.saveAppSettings();
                }
                utils.showToast(App.t('app_settings_saved_success'));
            } catch (e) {
                console.error('[Settings] Failed to save app settings:', e);
                utils.showToast(App.t('app_settings_saved_error') || 'Failed to save settings', true);
            }
            utils.closeModal(settingsModal);

            // Если изменились настройки стриминга, перезапускаем ВСЕ потоки
            if (streamingSettingsChanged) {
                console.log('[Settings] Streaming settings changed. Restarting all streams...');
                
                (async () => {
                    // 1. Получаем состояние ВСЕХ активных аналитик
                    const activeAnalytics = await window.api.getAnalyticsStates();
                    const activeAnalyticsIds = Object.keys(activeAnalytics).map(id => parseInt(id, 10));
                    console.log('[Settings] Active analytics to restart:', activeAnalyticsIds);

                    // 2. Убиваем все FFmpeg процессы
                    await window.api.killAllFfmpeg();

                    // 3. Небольшая пауза, чтобы процессы успели завершиться
                    await new Promise(resolve => setTimeout(resolve, 500));

                    // 4. Запускаем полный рендер, который пересоздаст все потоки
                    await App.gridManager.render();

                    // 5. Восстанавливаем аналитику для тех камер, где она была активна
                    if (activeAnalyticsIds.length > 0) {
                        console.log('[Settings] Re-enabling analytics...');
                        // Запускаем с задержкой, чтобы потоки успели инициализироваться
                        setTimeout(() => {
                            activeAnalyticsIds.forEach(cameraId => {
                                console.log(`- Toggling analytics for camera ${cameraId}`);
                                window.api.toggleAnalytics(cameraId, 1); // Use SD for analytics restart
                            });
                        }, 2000);
                    }
                })();
            }
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
                    let moduleHtml = `<div class="form-check-inline" style="margin-bottom: 15px; padding-bottom: 10px; width: 100%; align-items: flex-start;"><input type="checkbox" id="module-${mod.id}" data-id="${mod.id}" class="form-check-input module-checkbox" ${isChecked ? 'checked' : ''} style="margin-top: 5px;"><div style="flex-grow: 1;"><label for="module-${mod.id}" style="font-weight: bold; font-size: 1.1em;">${mod.name} <span style="font-size: 0.8em; color: #666;">v${mod.version}</span></label><p style="margin: 5px 0 0 0; font-size: 0.9em; color: #888;">${description}</p><p style="margin: 5px 0 0 0; font-size: 0.8em; color: #888;">${App.t('author_prefix') || 'Автор'}: ${author}</p></div></div>`;
                    if (mod.id === 'face-detector' && isChecked) {
                        const savePathKey = `module_face-detector_savePath`;
                        const currentPath = appSettings[savePathKey] || App.t('module_face-detector_default_path');
                        moduleHtml += `<div class="form-grid simple with-button" style="grid-template-columns: 100px 1fr; padding-bottom: 9px; margin-left: 5px; margin-bottom: 15px; border-bottom: 1px solid #eee;"><span data-i18n-key="module_face-detector_save_path_label"></span><div class="form-input-wrapper"><input type="text" id="face-detector-path" data-key="${savePathKey}" value="${currentPath}" readonly><button class="select-face-path-btn" style="padding: 0 10px; min-width: 40px; height: 35px; position: relative; top: -7px;"><i class="material-icons" style="font-size: 20px;">folder_open</i></button></div></div>`;
                    }
                    if (mod.id === 'license-plate' && isChecked) {
                        const savePathKey = `module_license-plate_savePath`;
                        const frameSkipKey = `module_license-plate_frameSkip`;
                        const resizeWidthKey = `module_license-plate_resizeWidth`;
                        const currentPath = appSettings[savePathKey] || App.t('module_license-plate_default_path');
                        const currentFrameSkip = appSettings[frameSkipKey] || 2;
                        const currentResizeWidth = appSettings[resizeWidthKey] || 0;
                        moduleHtml += `<div class="form-grid simple with-button" style="grid-template-columns: 100px 1fr; padding-bottom: 9px; margin-left: 5px; margin-bottom: 8px; border-bottom: 1px solid #eee;">
                            <span data-i18n-key="module_license-plate_save_path_label"></span>
                            <div class="form-input-wrapper"><input type="text" id="license-plate-path" data-key="${savePathKey}" value="${currentPath}" readonly><button class="select-license-plate-path-btn" style="padding: 0 10px; min-width: 40px; height: 35px; position: relative; top: -7px;"><i class="material-icons" style="font-size: 20px;">folder_open</i></button></div>
                            <span data-i18n-key="module_license-plate_frame_skip_label"></span>
                            <input type="number" id="license-plate-frame-skip" data-key="${frameSkipKey}" value="${currentFrameSkip}" min="1" max="30" step="1" style="width: 100px;">
                            <span data-i18n-key="module_license-plate_resize_width_label"></span>
                            <input type="number" id="license-plate-resize-width" data-key="${resizeWidthKey}" value="${currentResizeWidth}" min="0" max="1920" step="10" style="width: 100px;">
                            <span data-i18n-key="module_license-plate_use_ort"></span>
                            <label style="display:flex; align-items:center;"><input type="checkbox" id="license-plate-use-ort" data-key="module_license-plate_use_ort" style="margin-right:8px;"> <span style="font-size:12px; color:#888;">${App.t('module_license-plate_use_ort')}</span></label>
                        </div>`;
                    }
                    modulesListEl.innerHTML += moduleHtml;
                });
                App.i18n.applyTranslationsToDOM(modulesListEl);
                // Initialize License Plate "Use ONNX Runtime (DirectML)" checkbox from saved settings
                try {
                    const lpUseOrtCheckbox = document.getElementById('license-plate-use-ort');
                    if (lpUseOrtCheckbox) {
                        const saved = !!(stateManager.state.appSettings && stateManager.state.appSettings['module_license-plate_use_ort']);
                        lpUseOrtCheckbox.checked = saved;
                        lpUseOrtCheckbox.addEventListener('change', () => {
                            stateManager.setAppSettings({ ['module_license-plate_use_ort']: lpUseOrtCheckbox.checked });
                        });
                    }
                } catch (e) {
                    console.warn('Failed to initialize license plate ORT checkbox:', e);
                }

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
                const selectLicensePlatePathBtn = document.querySelector('.select-license-plate-path-btn');
                if (selectLicensePlatePathBtn) {
                    selectLicensePlatePathBtn.addEventListener('click', async () => {
                        const result = await window.api.selectDirectory();
                        if (!result.canceled && result.filePaths.length > 0) {
                            const pathInput = document.getElementById('license-plate-path');
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
                    updateInfoContainer.classList.remove('hidden');
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
            // Кнопки экспорта/импорта конфигурации
            const exportConfigBtn = document.getElementById('export-config-btn');
            const importConfigBtn = document.getElementById('import-config-btn');

            if (exportConfigBtn) {
                exportConfigBtn.addEventListener('click', async () => {
                    try {
                        await window.api.exportConfig();
                        utils.showToast(App.t('settings_config_export_success'));
                    } catch (e) {
                        utils.showToast(App.t('settings_config_export_error'), true);
                    }
                });
            }
            if (importConfigBtn) {
                importConfigBtn.addEventListener('click', async () => {
                    try {
                        await window.api.importConfig();
                        utils.showToast(App.t('settings_config_import_success'));
                    } catch (e) {
                        utils.showToast(App.t('settings_config_import_error'), true);
                    }
                });
            }
            settingsModal = document.getElementById('settings-modal');
            settingsModalCloseBtn = document.getElementById('settings-modal-close-btn');
            settingsModalTitle = document.getElementById('settings-modal-title');
            settingsIframe = document.getElementById('settings-iframe');
            saveSettingsBtn = document.getElementById('save-settings-btn');
            languageSelect = document.getElementById('app-settings-language');
            selectRecPathBtn = document.getElementById('select-rec-path-btn');
            selectScreenshotsPathBtn = document.getElementById('select-screenshots-path-btn');
            checkForUpdatesBtn = document.getElementById('check-for-updates-btn');
            reportIssueBtn = document.getElementById('report-issue-btn');

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
                    const newLang = languageSelect.value;
                    App.i18n.setLanguage(newLang);
                    stateManager.setAppSettings({ language: newLang }); 
                });
            }

            if (reportIssueBtn) {
                reportIssueBtn.addEventListener('click', () => {
                    App.modalHandler.showReportModal();
                });
            }

            const donateBtn = document.getElementById('donate-btn');
            if (donateBtn) {
                donateBtn.addEventListener('click', () => {
                    window.api.openExternalLink('https://opencollective.com/openipc/projects/openipc-dashboard/donate?interval=oneTime&amount=20&contributeAs=me');
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

            if (selectScreenshotsPathBtn) {
                selectScreenshotsPathBtn.addEventListener('click', async () => {
                    const result = await window.api.selectDirectory();
                    if (!result.canceled && result.filePaths.length > 0) {
                        document.getElementById('app-settings-screenshots-path').value = result.filePaths[0];
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
            
            // Улучшенная обработка обновлений с загрузкой changelog из GitHub Releases
            window.api.onUpdateStatus((data) => {
                if (settingsModal && !settingsModal.classList.contains('hidden')) {
                    // Если есть releaseNotes, показываем их в updateChangelog
                    if (data.info && data.info.releaseNotes) {
                        updateChangelog.textContent = data.info.releaseNotes;
                    } else {
                        updateChangelog.textContent = App.t('update_no_changelog');
                    }
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