// --- ФАЙЛ: js/modal-handler.js (Оригинальная версия до Svelte) ---

(function(window) {
    window.AppModules = window.AppModules || {};

    window.AppModules.createModalHandler = function(App) {
        // --- Общие утилиты для всех модальных окон ---
        let toastTimeout;
        const appToast = document.getElementById('app-toast');

        const utils = {
            openModal: (modalElement) => {
                if (modalElement) modalElement.classList.remove('hidden');
            },
            closeModal: (modalElement) => {
                if (modalElement) modalElement.classList.add('hidden');
            },
            showToast: (message, isError = false, duration = 3000) => {
                if (toastTimeout) clearTimeout(toastTimeout);
                appToast.textContent = message;
                appToast.className = 'toast-notification';
                if (isError) appToast.classList.add('error');
                appToast.classList.add('show');
                toastTimeout = setTimeout(() => { appToast.classList.remove('show'); }, duration);
            }
        };

        // --- Создание дочерних обработчиков ---
        const cameraHandler = AppModules.createCameraModalHandler(App, utils);
        const settingsHandler = AppModules.createSettingsModalHandler(App, utils);
        const userHandler = AppModules.createUserModalHandler(App, utils);

        // --- Элементы для кастомного prompt-окна ---
        const promptModal = document.getElementById('prompt-modal');
        const promptModalTitle = document.getElementById('prompt-modal-title');
        const promptModalLabel = document.getElementById('prompt-modal-label');
        const promptModalInput = document.getElementById('prompt-modal-input');
        const promptModalOkBtn = document.getElementById('prompt-modal-ok-btn');
        const promptModalCancelBtn = document.getElementById('prompt-modal-cancel-btn');
        const promptModalCloseBtn = document.getElementById('prompt-modal-close-btn');

        // --- Элементы для окна отправки отчета ---
        const reportModal = document.getElementById('report-issue-modal');
        const sendReportBtn = document.getElementById('send-report-btn');
        const cancelReportBtn = document.getElementById('cancel-report-btn');
        const reportIssueCloseBtn = document.getElementById('report-issue-close-btn');
        const issueDescription = document.getElementById('issue-description');
        const addScreenshotBtn = document.getElementById('add-screenshot-btn');
        const screenshotsPreview = document.getElementById('screenshots-preview');
        
        let attachedScreenshots = []; // Массив для хранения Base64 скриншотов

        function showPrompt({ title, label, defaultValue = '', okText = App.t('save'), cancelText = App.t('cancel'), inputType = 'text' }) {
            return new Promise((resolve) => {
                promptModalTitle.textContent = title;
                promptModalLabel.textContent = label;
                promptModalOkBtn.textContent = okText;
                promptModalCancelBtn.textContent = cancelText;

                if (inputType === 'none') {
                    promptModalInput.classList.add('hidden');
                    promptModalInput.value = '';
                } else {
                    promptModalInput.classList.remove('hidden');
                    promptModalInput.value = defaultValue;
                    promptModalInput.focus();
                    promptModalInput.select();
                }

                utils.openModal(promptModal);
                
                let isResolved = false;

                const cleanupAndResolve = (value) => {
                    if (isResolved) return;
                    isResolved = true;
                    
                    promptModalOkBtn.removeEventListener('click', onOk);
                    promptModalCancelBtn.removeEventListener('click', onCancel);
                    promptModal.removeEventListener('keydown', onKeydown);
                    
                    utils.closeModal(promptModal);
                    resolve(value);
                };

                const onOk = () => {
                    const valueToResolve = (inputType === 'none') ? true : promptModalInput.value;
                    cleanupAndResolve(valueToResolve);
                };

                const onCancel = () => {
                    cleanupAndResolve(null);
                };
                
                const onKeydown = (e) => {
                    if (e.key === 'Enter') {
                        onOk();
                    } else if (e.key === 'Escape') {
                        onCancel();
                    }
                };

                promptModalOkBtn.addEventListener('click', onOk);
                promptModalCancelBtn.addEventListener('click', onCancel);
                promptModal.addEventListener('keydown', onKeydown);
            });
        }
        
        function showReportModal() {
            if (issueDescription) issueDescription.value = '';
            // Очищаем предыдущие скриншоты при открытии
            attachedScreenshots = [];
            if(screenshotsPreview) screenshotsPreview.innerHTML = '';
            utils.openModal(reportModal);
        }

        function addScreenshotToPreview(dataUrl) {
            const index = attachedScreenshots.length - 1;
            const thumb = document.createElement('div');
            thumb.className = 'screenshot-thumb';
            thumb.innerHTML = `
                <img src="${dataUrl}" alt="${App.t('report_issue_screenshot_alt', { index: index + 1 })}">
                <span class="remove-screenshot-btn" title="${App.t('report_issue_remove_tooltip')}">&times;</span>
            `;
            
            thumb.querySelector('.remove-screenshot-btn').onclick = () => {
                // Удаляем из массива и из DOM
                attachedScreenshots.splice(index, 1);
                thumb.remove();
                // Перерисовываем превью, чтобы индексы были правильными
                redrawScreenshotsPreview();
            };
            screenshotsPreview.appendChild(thumb);
        }

        function redrawScreenshotsPreview() {
            screenshotsPreview.innerHTML = '';
            const currentScreenshots = [...attachedScreenshots];
            attachedScreenshots = [];
            currentScreenshots.forEach(dataUrl => {
                attachedScreenshots.push(dataUrl);
                addScreenshotToPreview(dataUrl);
            });
        }

        function init() {
            // --- Инициализация всех дочерних обработчиков ---
            cameraHandler.init();
            settingsHandler.init();
            userHandler.init();
            
            // --- Глобальные обработчики, управляющие всеми модальными окнами ---
            const generalSettingsBtn = document.getElementById('general-settings-btn');
            const userManagementBtn = document.getElementById('user-management-btn');
            
            generalSettingsBtn.addEventListener('click', () => settingsHandler.openSettingsModal(null));
            userManagementBtn.addEventListener('click', () => userHandler.openUserManagementModal());
            
            // --- Обработчики для кастомного prompt ---
            promptModalCloseBtn.addEventListener('click', () => {
                promptModalCancelBtn.click();
            });
            promptModal.addEventListener('click', (e) => { 
                if (e.target === promptModal) {
                    promptModalCancelBtn.click();
                }
            });

            // --- Обработчики для окна отчета об ошибке ---
            if (sendReportBtn) {
                sendReportBtn.addEventListener('click', async () => {
                    const description = issueDescription.value.trim();
                    if (!description) {
                        utils.showToast(App.t('report_issue_desc_required'), true);
                        return;
                    }
                    sendReportBtn.disabled = true;
                    sendReportBtn.textContent = App.t('sending_text');
                    try {
                        const result = await window.api.submitReport({ description, screenshots: attachedScreenshots });
                        
                        if (result.success && result.messageKey) {
                            utils.showToast(App.t(result.messageKey));
                        }
                        
                        utils.closeModal(reportModal);
                    } catch (e) {
                        utils.showToast(`${App.t('error')}: ${e.message}`, true);
                    } finally {
                        sendReportBtn.disabled = false;
                        sendReportBtn.textContent = App.t('report_issue_send_btn');
                        attachedScreenshots = [];
                        screenshotsPreview.innerHTML = '';
                    }
                });
            }

            if (addScreenshotBtn) {
                addScreenshotBtn.addEventListener('click', async () => {
                    try {
                        addScreenshotBtn.disabled = true;
                        const dataUrls = await window.api.openImageFiles();

                        if (dataUrls && dataUrls.length > 0) {
                            dataUrls.forEach(dataUrl => {
                                if (attachedScreenshots.length < 5) { 
                                    attachedScreenshots.push(dataUrl);
                                    addScreenshotToPreview(dataUrl);
                                } else {
                                    utils.showToast(App.t('report_issue_limit_error'), true);
                                }
                            });
                        }
                    } finally {
                        addScreenshotBtn.disabled = false;
                    }
                });
            }

            if (cancelReportBtn) cancelReportBtn.addEventListener('click', () => utils.closeModal(reportModal));
            if (reportIssueCloseBtn) reportIssueCloseBtn.addEventListener('click', () => utils.closeModal(reportModal));
            if (reportModal) reportModal.addEventListener('click', (e) => { 
                if (e.target === reportModal) utils.closeModal(reportModal);
            });

            // Закрытие любого открытого модального окна по клавише Escape
            window.addEventListener('keydown', (e) => {
                if (e.key === 'Escape') {
                    if (!promptModal.classList.contains('hidden')) {
                        promptModalCancelBtn.click();
                        return; 
                    }
                    if (reportModal && !reportModal.classList.contains('hidden')) {
                        utils.closeModal(reportModal);
                        return;
                    }
                    cameraHandler.closeAll();
                    settingsHandler.closeAll();
                    userHandler.closeAll();
                }
            });
        }

        // --- Публичный API модуля ---
        return { 
            init,
            openAddModal: cameraHandler.openAddModal,
            openSettingsModal: settingsHandler.openSettingsModal,
            showPrompt,
            showToast: utils.showToast,
            showReportModal,
        };
    };
})(window);