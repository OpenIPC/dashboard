// js/state-manager.js

(function(window) {
    'use strict';
    window.AppModules = window.AppModules || {};

    AppModules.createStateManager = function(config) {
        const subscribers = new Set();
        const mutations = config.mutations || {};
        
        // Основной объект состояния. Оборачиваем его в Proxy.
        const state = new Proxy(config.initialState || {}, {
            set(target, property, value) {
                // Устанавливаем новое значение
                target[property] = value;
                
                // Уведомляем всех "подписчиков" о том, что состояние изменилось
                console.log(`[State Change]: ${String(property)} changed. Notifying ${subscribers.size} subscribers.`);
                subscribers.forEach(callback => callback());
                
                return true;
            }
        });

        // Функция для подписки на изменения состояния
        const subscribe = (callback) => {
            subscribers.add(callback);
            // Возвращаем функцию для отписки
            return () => subscribers.delete(callback);
        };
        
        // VVV НОВЫЙ ХЕЛПЕР VVV
        // Хелпер для поиска и гарантированного получения активной раскладки.
        // Если раскладок нет, он создает одну по умолчанию.
        function getActiveLayout(state) {
            if (!state.layouts || state.layouts.length === 0) {
                // Создаем раскладку по умолчанию, если ничего нет
                const defaultLayout = { id: Date.now(), name: 'Default', gridState: Array(64).fill(null), layout: { cols: 2, rows: 2 } };
                state.layouts = [defaultLayout];
                state.activeLayoutId = defaultLayout.id;
            }
            return state.layouts.find(l => l.id === state.activeLayoutId) || state.layouts[0];
        }
        // ^^^ КОНЕЦ НОВОГО ХЕЛПЕРА ^^^

        // Привязываем мутации к нашему менеджеру, чтобы они могли изменять состояние
        const boundMutations = {};
        // VVV ИЗМЕНЕНИЕ: Создаем объект с хелперами для передачи в мутации VVV
        const helpers = { getActiveLayout };
        for (const key in mutations) {
            // Передаем state и helpers как первые аргументы в каждую мутацию
            boundMutations[key] = mutations[key].bind(null, state, helpers);
        }

        return {
            state,
            subscribe,
            ...boundMutations
        };
    };
})(window);