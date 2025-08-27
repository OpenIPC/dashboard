const { registerAuthHandlers } = require('./ipc/auth');
const { registerCameraHandlers } = require('./ipc/camera');
const { registerConfigHandlers } = require('./ipc/config');
const { registerSystemHandlers } = require('./ipc/system');
// Добавьте другие импорты, если вы разделили на большее количество файлов

/**
 * Регистрирует все IPC-обработчики, импортируя их из модулей.
 * @param {object} moduleManager - Экземпляр ModuleManager для передачи зависимостей.
 * @param {string} APP_VERSION - Текущая версия приложения ('lite' или 'intellect').
 */
function registerIpcHandlers(moduleManager, APP_VERSION) {
  registerAuthHandlers();
  registerConfigHandlers();
  registerCameraHandlers(moduleManager, APP_VERSION);
  registerSystemHandlers(APP_VERSION, moduleManager);
  
  console.log('[IPC] All handlers have been successfully registered.');
}

module.exports = { registerIpcHandlers };