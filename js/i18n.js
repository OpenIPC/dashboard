// --- START OF FILE js/i18n.js ---
(function(window) {
    'use strict';
    window.AppModules = window.AppModules || {};

    AppModules.createI18n = function(App) {
        const stateManager = App.stateManager;
        let translations = {};
        const supportedLangs = ['en', 'ru'];
        let currentLang = 'en';

        const getPreferredLanguage = () => {
            const lang = (navigator.language || navigator.userLanguage).split('-')[0];
            return supportedLangs.includes(lang) ? lang : 'en';
        };

        async function loadTranslations(lang) {
            try {
                const loadedTranslations = await window.api.getTranslationFile(lang);
                if (!loadedTranslations) throw new Error(`Failed to load ${lang}.json`);
                
                translations = loadedTranslations;
                currentLang = lang;
                document.documentElement.lang = lang;
                console.log(`Translations for '${lang}' loaded.`);
                return true;
            } catch (error) {
                console.error('Error loading translation file:', error);
                if (lang !== 'en') {
                    console.log('Falling back to English.');
                    return await loadTranslations('en');
                }
                return false;
            }
        }

        function t(key, replacements = {}) {
            let translation = translations[key] || key;
            for (const placeholder in replacements) {
                translation = translation.replace(`{{${placeholder}}}`, replacements[placeholder]);
            }
            return translation;
        }
        
        function applyTranslationsToDOM(scopeElement = document) {
            scopeElement.querySelectorAll('[data-i18n-key]').forEach(element => {
                const key = element.getAttribute('data-i18n-key');
                const attr = element.hasAttribute('data-i18n-is-html') ? 'innerHTML' : 'textContent';
                element[attr] = t(key);
            });
            scopeElement.querySelectorAll('[data-i18n-tooltip]').forEach(element => {
                const key = element.getAttribute('data-i18n-tooltip');
                element.title = t(key);
            });
            scopeElement.querySelectorAll('[data-i18n-placeholder]').forEach(element => {
                const key = element.getAttribute('data-i18n-placeholder');
                element.placeholder = t(key);
            });
        }
        
        async function setLanguage(lang) {
            if (!supportedLangs.includes(lang) || lang === currentLang) {
                return;
            }
            const success = await loadTranslations(lang);
            if (success) {
                applyTranslationsToDOM();
                window.dispatchEvent(new CustomEvent('language-changed'));
            }
        }

        // START: ИСПРАВЛЕНИЕ - init теперь принимает язык
        async function init(initialLang = null) {
            const lang = initialLang || stateManager.state.appSettings.language || getPreferredLanguage();
            await loadTranslations(lang);
            applyTranslationsToDOM();
        }
        // END: ИСПРАВЛЕНИЕ

        return {
            init,
            t,
            setLanguage,
            applyTranslationsToDOM
        };
    };
})(window);
// --- END OF FILE js/i18n.js ---