// js/i18n.js (полная исправленная версия)

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
        
        // VVVVVV --- ИЗМЕНЕННАЯ ФУНКЦИЯ --- VVVVVV
        function applyTranslationsToDOM(scopeElement = document) {
            // Ищем элементы с ключом для основного текста/HTML
            scopeElement.querySelectorAll('[data-i18n-key]').forEach(element => {
                const key = element.getAttribute('data-i18n-key');
                const attr = element.hasAttribute('data-i18n-is-html') ? 'innerHTML' : 'textContent';
                element[attr] = t(key);
            });
            // Ищем элементы с ключом для всплывающей подсказки (атрибут title)
            scopeElement.querySelectorAll('[data-i18n-tooltip]').forEach(element => {
                const key = element.getAttribute('data-i18n-tooltip');
                element.title = t(key);
            });
            // Ищем элементы с ключом для плейсхолдера (атрибут placeholder)
            scopeElement.querySelectorAll('[data-i18n-placeholder]').forEach(element => {
                const key = element.getAttribute('data-i18n-placeholder');
                element.placeholder = t(key);
            });
        }
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
        
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

        async function init() {
            const lang = stateManager.state.appSettings.language || getPreferredLanguage();
            await loadTranslations(lang);
            applyTranslationsToDOM();
        }

        return {
            init,
            t,
            setLanguage,
            applyTranslationsToDOM
        };
    };
})(window);