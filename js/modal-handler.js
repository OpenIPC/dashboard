// js/modal-handler.js

(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createModalHandler = function(App) {
        // --- Общие утилиты для всех модальных окон ---
        let toastTimeout;
        const settingsToast = document.getElementById('settings-toast');

        const utils = {
            openModal: (modalElement) => modalElement.classList.remove('hidden'),
            closeModal: (modalElement) => {
                if (modalElement) modalElement.classList.add('hidden');
            },
            showToast: (message, isError = false, duration = 3000) => {
                if (toastTimeout) clearTimeout(toastTimeout);
                settingsToast.textContent = message;
                settingsToast.className = 'toast-notification';
                if (isError) settingsToast.classList.add('error');
                settingsToast.classList.add('show');
                toastTimeout = setTimeout(() => { settingsToast.classList.remove('show'); }, duration);
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

        // VVV ИЗМЕНЕНИЕ: Улучшенная логика асинхронного prompt VVV
        function showPrompt({ title, label, defaultValue = '', okText = App.t('save'), cancelText = App.t('cancel') }) {
            return new Promise((resolve) => {
                promptModalTitle.textContent = title;
                promptModalLabel.textContent = label;
                promptModalInput.value = defaultValue;
                promptModalOkBtn.textContent = okText;
                promptModalCancelBtn.textContent = cancelText;

                utils.openModal(promptModal);
                promptModalInput.focus();
                promptModalInput.select();
                
                // Переменная, чтобы гарантировать вызов resolve только один раз
                let isResolved = false;

                const cleanupAndResolve = (value) => {
                    if (isResolved) return;
                    isResolved = true;
                    
                    // Удаляем обработчики
                    promptModalOkBtn.removeEventListener('click', onOk);
                    promptModalCancelBtn.removeEventListener('click', onCancel);
                    promptModal.removeEventListener('keydown', onKeydown);
                    
                    utils.closeModal(promptModal);
                    resolve(value);
                };

                const onOk = () => {
                    cleanupAndResolve(promptModalInput.value);
                };

                const onCancel = () => {
                    cleanupAndResolve(null); // Возвращаем null при отмене, как и стандартный prompt
                };
                
                const onKeydown = (e) => {
                    if (e.key === 'Enter') {
                        onOk();
                    } else if (e.key === 'Escape') {
                        onCancel();
                    }
                };

                // Назначаем обработчики
                promptModalOkBtn.addEventListener('click', onOk);
                promptModalCancelBtn.addEventListener('click', onCancel);
                promptModal.addEventListener('keydown', onKeydown);
            });
        }
        // ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^

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
                // Имитируем клик по кнопке отмены, чтобы Promise завершился
                promptModalCancelBtn.click();
            });
            promptModal.addEventListener('click', (e) => { 
                if (e.target === promptModal) {
                    promptModalCancelBtn.click();
                }
            });

            // Закрытие любого открытого модального окна по клавише Escape
            window.addEventListener('keydown', (e) => {
                if (e.key === 'Escape') {
                    // Проверяем, не открыт ли наш промпт, чтобы избежать двойного закрытия
                    if (!promptModal.classList.contains('hidden')) {
                        // Даем собственному обработчику prompt-а сработать, он сам вызовет отмену
                        promptModalCancelBtn.click();
                        return; 
                    }
                    cameraHandler.closeAll();
                    settingsHandler.closeAll();
                    userHandler.closeAll();
                }
            });
        }

        // --- Публичный API модуля ---
        // Предоставляем доступ к публичным методам дочерних обработчиков
        return { 
            init,
            openAddModal: cameraHandler.openAddModal,
            openSettingsModal: settingsHandler.openSettingsModal,
            showPrompt,
        };
    };
})(window);