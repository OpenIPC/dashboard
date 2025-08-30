// Файл: src/main/ipc-handlers.js

const { ipcMain } = require('electron');

// Создаем карту, которая будет хранить все обработчики для WebSocket
const handlerMap = new Map();

// "Патчим" оригинальную функцию ipcMain.handle
// Это позволит нам "перехватить" регистрацию всех обработчиков
const originalHandle = ipcMain.handle;
ipcMain.handle = (channel, listener) => {
    console.log(`[IPC Bridge] Registering handler for WebSocket channel: ${channel}`);
    // Сохраняем обработчик в нашу карту, чтобы WebSocket-сервер мог его найти
    handlerMap.set(channel, listener);
    // Вызываем оригинальную функцию, чтобы IPC для самого Electron продолжал работать как обычно
    originalHandle.call(ipcMain, channel, listener);
};

// Теперь, когда мы "перехватываем" handle, мы можем импортировать остальные модули.
// Когда они вызовут ipcMain.handle, будет использована наша измененная версия.
const { registerAuthHandlers } = require('./ipc/auth');
const { registerCameraHandlers } = require('./ipc/camera');
const { registerConfigHandlers } = require('./ipc/config');
const { registerSystemHandlers } = require('./ipc/system');

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
  
  console.log(`[IPC] All handlers registered. Bridge has ${handlerMap.size} handlers ready for WebSocket.`);
}

// Экспортируем и функцию регистрации, и нашу карту с обработчиками,
// чтобы главный файл (main.js) мог их использовать.
module.exports = { registerIpcHandlers, handlerMap };