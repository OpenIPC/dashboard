// js/state-manager.js

(function(window) {
    'use strict';
    window.AppModules = window.AppModules || {};

    AppModules.createStateManager = function(config) {
        const subscribers = new Set();
        const mutations = config.mutations || {};
        const onChangeCallback = typeof config.onChange === 'function' ? config.onChange : null;
        const ignoreKeys = config.ignoreKeys || ['isSaving', 'player'];

        // Helper: create a reactive proxy for objects/arrays that notifies subscribers
        function createReactive(obj, path = '') {
            if (obj === null || typeof obj !== 'object') return obj;

            // Avoid double-wrapping
            if (obj.__isReactive) return obj;

            const handler = {
                get(target, prop, receiver) {
                    if (prop === '__isReactive') return true;
                    const val = Reflect.get(target, prop, receiver);
                    // Wrap nested objects/arrays lazily
                    return createReactive(val, path ? `${path}.${String(prop)}` : String(prop));
                },
                set(target, prop, value, receiver) {
                    const prev = target[prop];
                    const res = Reflect.set(target, prop, value, receiver);
                    try {
                        // Notify subscribers
                        subscribers.forEach(cb => { try { cb(); } catch (e) { console.error('subscriber cb error', e); } });
                        // Call onChange unless the property is ignored
                        if (onChangeCallback && !ignoreKeys.includes(String(prop))) {
                            try { onChangeCallback(); } catch (e) { console.error('onChange callback failed', e); }
                        }
                    } catch (e) { console.error('reactive set error', e); }
                    return res;
                },
                deleteProperty(target, prop) {
                    const res = Reflect.deleteProperty(target, prop);
                    try {
                        subscribers.forEach(cb => { try { cb(); } catch (e) { console.error('subscriber cb error', e); } });
                        if (onChangeCallback && !ignoreKeys.includes(String(prop))) {
                            try { onChangeCallback(); } catch (e) { console.error('onChange callback failed', e); }
                        }
                    } catch (e) { console.error('reactive delete error', e); }
                    return res;
                }
            };

            // For arrays, intercept mutating methods to trigger notifications
            if (Array.isArray(obj)) {
                const arr = obj;
                const arrayProxy = new Proxy(arr, handler);
                const mutating = ['push','pop','shift','unshift','splice','sort','reverse'];
                mutating.forEach(fn => {
                    Object.defineProperty(arrayProxy, fn, {
                        value: function(...args) {
                            const result = Array.prototype[fn].apply(arr, args);
                            try {
                                subscribers.forEach(cb => { try { cb(); } catch (e) { console.error('subscriber cb error', e); } });
                                if (onChangeCallback) { try { onChangeCallback(); } catch (e) { console.error('onChange callback failed', e); } }
                            } catch (e) { console.error('array mutator notify failed', e); }
                            return result;
                        },
                        writable: true,
                        configurable: true,
                    });
                });
                return arrayProxy;
            }

            return new Proxy(obj, handler);
        }
        
        // Основной объект состояния. Оборачиваем его в глубокий реактивный прокси.
        const initial = config.initialState || {};
        const state = createReactive(initial);

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