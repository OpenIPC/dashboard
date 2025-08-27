const { ipcMain } = require('electron');
const authManager = require('../auth-manager');
const CHANNELS = require('../../common/ipc-channels');
const { getMainWindow } = require('../window-manager');

// Эту функцию можно вынести в отдельный файл утилит, если она будет использоваться в нескольких местах
const withErrorHandling = (handler, context) => async (event, ...args) => {
    try {
        const result = await handler(event, ...args);
        return result === undefined ? { success: true } : result;
    } catch (error) {
        require('../services').handleError(error, context);
        return { success: false, error: error.message };
    }
};

function registerAuthHandlers() {
  ipcMain.handle(CHANNELS.LOGIN, withErrorHandling((event, creds) => authManager.handleLogin(creds), 'login'));
  ipcMain.on(CHANNELS.RENDERER_READY, () => authManager.handleAutoLogin(getMainWindow()));
  ipcMain.on(CHANNELS.LOGOUT_CLEAR_CREDS, authManager.clearAutoLoginCredentials);

  ipcMain.handle(CHANNELS.GET_USERS, withErrorHandling(authManager.getUsers, 'getUsers'));
  ipcMain.handle(CHANNELS.ADD_USER, withErrorHandling((event, data) => authManager.addUser(data), 'addUser'));
  ipcMain.handle(CHANNELS.UPDATE_USER_PASSWORD, withErrorHandling((event, data) => authManager.updateUserPassword(data), 'updateUserPassword'));
  ipcMain.handle(CHANNELS.UPDATE_USER_ROLE, withErrorHandling((event, data) => authManager.updateUserRole(data), 'updateUserRole'));
  ipcMain.handle(CHANNELS.UPDATE_USER_PERMISSIONS, withErrorHandling((event, data) => authManager.updateUserPermissions(data), 'updateUserPermissions'));
  ipcMain.handle(CHANNELS.DELETE_USER, withErrorHandling((event, data) => authManager.deleteUser(data), 'deleteUser'));
}

module.exports = {
  registerAuthHandlers
};