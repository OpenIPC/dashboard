// --- START OF FILE js/template-loader.js ---
(function() {
    'use strict';
    // Этот скрипт должен выполняться самым первым.
    
    // Создаем глобальный объект для модулей, если его нет
    window.AppModules = window.AppModules || {};

    // Проверяем, не были ли шаблоны уже загружены
    if (window.AppModules.templatesLoaded) {
        return;
    }

    async function loadTemplates() {
        try {
            const response = await fetch('templates.html?v=' + Date.now());
            if (!response.ok) {
                throw new Error(`Network response was not ok: ${response.statusText}`);
            }
            const templatesHTML = await response.text();
            document.body.insertAdjacentHTML('beforeend', templatesHTML);
            
            // Устанавливаем флаг, что всё прошло успешно
            window.AppModules.templatesLoaded = true;
            console.log('[TemplateLoader] All templates have been successfully loaded and injected.');

        } catch (error) {
            console.error('Fatal Error: Could not load templates.html. Modals and other UI components will not work.', error);
            alert('Критическая ошибка: Не удалось загрузить файл templates.html. Приложение не может работать корректно.');
        }
    }

    loadTemplates();
})();
// --- END OF FILE js/template-loader.js ---