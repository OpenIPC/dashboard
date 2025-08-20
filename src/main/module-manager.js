// --- ФАЙЛ: src/main/module-manager.js (ИЗМЕНЕННАЯ ВЕРСИЯ) ---

const { ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');

class ModuleManager {
    constructor(appAPI) {
        this.modulesDir = path.join(__dirname, '..', '..', 'modules');
        this.availableModules = [];
        this.loadedModules = new Map();
        this.appAPI = appAPI;
        // VVVVVV --- НОВОЕ СВОЙСТВО ДЛЯ ХРАНЕНИЯ СЛУШАТЕЛЕЙ --- VVVVVV
        this.eventListeners = new Map();
    }

    // VVVVVV --- НОВЫЙ МЕТОД ДЛЯ ПОДПИСКИ --- VVVVVV
    registerListener(eventName, callback) {
        if (!this.eventListeners.has(eventName)) {
            this.eventListeners.set(eventName, []);
        }
        this.eventListeners.get(eventName).push(callback);
    }

    // VVVVVV --- НОВЫЙ МЕТОД ДЛЯ ПОЛУЧЕНИЯ СЛУШАТЕЛЕЙ --- VVVVVV
    getListeners(eventName) {
        return this.eventListeners.get(eventName) || [];
    }
    
    // VVVVVV --- НОВЫЙ МЕТОД ДЛЯ ОТПИСКИ (ВАЖНО!) --- VVVVVV
    unregisterListener(eventName, callback) {
        if (!this.eventListeners.has(eventName)) {
            return;
        }
        const listeners = this.eventListeners.get(eventName);
        const index = listeners.indexOf(callback);
        if (index > -1) {
            listeners.splice(index, 1);
        }
    }

    discoverModules() {
        // ... код без изменений ...
        if (!fs.existsSync(this.modulesDir)) {
            fs.mkdirSync(this.modulesDir);
            return;
        }
        const moduleNames = fs.readdirSync(this.modulesDir);
        for (const name of moduleNames) {
            const modulePath = path.join(this.modulesDir, name);
            const manifestPath = path.join(modulePath, 'plugin.json');
            if (fs.existsSync(manifestPath)) {
                try {
                    const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf-8'));
                    manifest.id = name;
                    manifest.path = modulePath;
                    this.availableModules.push(manifest);
                } catch (e) {
                    console.error(`Ошибка чтения манифеста для модуля ${name}:`, e);
                }
            }
        }
        console.log(`[ModuleManager] Найдено модулей: ${this.availableModules.length}`);
    }

    loadEnabledModules(currentAppSettings) {
        const enabledModules = currentAppSettings.enabledModules || [];
        
        this.availableModules.forEach(mod => {
            if (enabledModules.includes(mod.id) && !this.loadedModules.has(mod.id)) {
                this.loadModule(mod);
            }
        });
    }

    loadModule(manifest) {
        try {
            const mainEntryPoint = manifest.entryPoints?.main;
            if (mainEntryPoint) {
                const modulePath = path.join(manifest.path, mainEntryPoint);
                const moduleCode = require(modulePath);
                
                if (typeof moduleCode.activate === 'function') {
                    moduleCode.activate(this.appAPI);
                    this.loadedModules.set(manifest.id, { manifest, main: moduleCode });
                    console.log(`[ModuleManager] Модуль "${manifest.name}" успешно загружен (main).`);
                }
            }
        } catch (e) {
            console.error(`Ошибка загрузки main-части модуля ${manifest.name}:`, e);
        }
    }
}

module.exports = { ModuleManager };