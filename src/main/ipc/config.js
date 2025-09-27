const { ipcMain, dialog } = require('electron');
const configManager = require('../config-manager');
const CHANNELS = require('../../common/ipc-channels');
const { getMainWindow } = require('../window-manager');

const withErrorHandling = (handler, context) => async (event, ...args) => {
    try {
        const result = await handler(event, ...args);
        return result === undefined ? { success: true } : result;
    } catch (error) {
        require('../services').handleError(error, context);
        return { success: false, error: error.message };
    }
};

function registerConfigHandlers() {
  ipcMain.handle(CHANNELS.LOAD_APP_SETTINGS, withErrorHandling(configManager.getAppSettings, 'loadAppSettings'));
  
  // VVVVVV --- ВОЗВРАЩАЕМ HANDLE --- VVVVVV
  ipcMain.handle(CHANNELS.SAVE_APP_SETTINGS, withErrorHandling((event, settings) => {
    try {
      const keys = settings && typeof settings === 'object' ? Object.keys(settings) : [];
      console.log('[IPC] save-app-settings invoked, keys=', keys);
    } catch (e) { console.warn('[IPC] save-app-settings invoked (failed to extract keys)', e); }
    return configManager.saveAppSettings(settings);
  }, 'saveAppSettings'));
  // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
  
  ipcMain.handle(CHANNELS.LOAD_CONFIG, withErrorHandling(configManager.loadConfiguration, 'loadConfiguration'));
  
  // VVVVVV --- ВОЗВРАЩАЕМ HANDLE --- VVVVVV
  ipcMain.handle(CHANNELS.SAVE_CONFIG, withErrorHandling((event, config, meta) => configManager.saveConfiguration(config, meta), 'saveConfiguration'));
  // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
  
  ipcMain.handle(CHANNELS.EXPORT_CONFIG, withErrorHandling(() => configManager.exportConfig(getMainWindow()), 'exportConfig'));
  ipcMain.handle(CHANNELS.IMPORT_CONFIG, withErrorHandling(() => configManager.importConfig(getMainWindow()), 'importConfig'));
  
  ipcMain.handle(CHANNELS.GET_TRANSLATION, withErrorHandling((event, lang) => configManager.getTranslationFile(lang), 'getTranslationFile'));
  ipcMain.handle(CHANNELS.SELECT_DIRECTORY, withErrorHandling(() => dialog.showOpenDialog(getMainWindow(), { properties: ['openDirectory'] }), 'selectDirectory'));
  
  ipcMain.handle(CHANNELS.GET_LOCAL_DISK_LIST, withErrorHandling(configManager.getLocalDiskList, 'getLocalDiskList'));
  ipcMain.handle(CHANNELS.LIST_LOCAL_FILES, withErrorHandling((event, path) => configManager.listLocalFiles(path), 'listLocalFiles'));
}

module.exports = {
  registerConfigHandlers
};