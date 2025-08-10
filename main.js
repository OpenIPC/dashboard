const { app } = require('electron');
const { initializeApp, onAppWillQuit } = require('./src/main/app-lifecycle');
const { createWindow } = require('./src/main/window-manager');
const { registerIpcHandlers } = require('./src/main/ipc-handlers');

// Предотвращаем запуск нескольких экземпляров приложения
if (!app.requestSingleInstanceLock()) {
  app.quit();
}

// Принудительно отключаем аппаратное ускорение, если есть проблемы
// app.disableHardwareAcceleration();

if (process.platform === 'linux' || process.env.ELECTRON_FORCE_NO_SANDBOX) {
    app.commandLine.appendSwitch('--no-sandbox');
}
app.commandLine.appendSwitch('force_high_performance_gpu');

initializeApp();

app.whenReady().then(() => {
    createWindow();
    registerIpcHandlers();
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
    }
});

app.on('will-quit', onAppWillQuit);
