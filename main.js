// Файл: /main.js (в корне проекта)
// Это точка входа для Electron. Его задача - запустить основную логику.

const { app } = require('electron');

// Подключаем и выполняем основной файл приложения из папки src/main
require('./src/main/main.js');

// Здесь можно добавить обработчик 'second-instance', если нужно
// чтобы при повторном запуске приложения фокус передавался уже открытому окну.
app.on('second-instance', (event, commandLine, workingDirectory) => {
    const { getMainWindow } = require('./src/main/window-manager');
    const mainWindow = getMainWindow();
    if (mainWindow) {
        if (mainWindow.isMinimized()) mainWindow.restore();
        mainWindow.focus();
    }
});