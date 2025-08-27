const { ipcMain, Menu, BrowserWindow } = require('electron');
const cameraAPI = require('../camera-api');
const processManager = require('../process-manager');
const configManager = require('../config-manager');
const { discoverDevices } = require('../discovery');
const { getMainWindow, createFileManagerWindow, createSshTerminalWindow } = require('../window-manager');
const CHANNELS = require('../../common/ipc-channels');

// --- ВОТ ИСПРАВЛЕНИЕ: Добавляем эту функцию в начало файла ---
const withErrorHandling = (handler, context) => async (event, ...args) => {
    try {
        const result = await handler(event, ...args);
        return result === undefined ? { success: true } : result;
    } catch (error) {
        require('../services').handleError(error, context);
        return { success: false, error: error.message };
    }
};
// --- КОНЕЦ ИСПРАВЛЕНИЯ ---

const sshConnections = {};
const fileManagerConnections = {};

function registerCameraHandlers(moduleManager, APP_VERSION) {
  const featureNotAvailableHandler = () => Promise.resolve({ success: false, error: 'Feature not available in Lite version' });
  
  ipcMain.handle(CHANNELS.GET_CAMERA_PULSE, withErrorHandling((event, camera) => cameraAPI.getCameraPulse(camera), 'getCameraPulse'));
  ipcMain.handle(CHANNELS.PTZ_CONTROL, withErrorHandling((event, data) => cameraAPI.ptzControl(data), 'ptzControl'));
  ipcMain.handle(CHANNELS.GET_CAMERA_TIME, withErrorHandling((event, camera) => cameraAPI.getCameraTime(camera), 'getCameraTime'));
  ipcMain.handle(CHANNELS.GET_CAMERA_SETTINGS, withErrorHandling((event, camera) => cameraAPI.getCameraSettings(camera), 'getCameraSettings'));
  ipcMain.handle(CHANNELS.SET_CAMERA_SETTINGS, withErrorHandling((event, data) => cameraAPI.setCameraSettings(data), 'setCameraSettings'));
  ipcMain.handle(CHANNELS.RESTART_MAJESTIC, withErrorHandling((event, camera) => cameraAPI.restartMajestic(camera), 'restartMajestic'));
  
  ipcMain.handle(CHANNELS.START_VIDEO_STREAM, withErrorHandling((event, data) => processManager.startVideoStream(data), 'startVideoStream'));
  ipcMain.handle(CHANNELS.STOP_VIDEO_STREAM, withErrorHandling((event, streamId) => processManager.stopVideoStream(streamId), 'stopVideoStream'));
  ipcMain.handle(CHANNELS.PAUSE_VIDEO_STREAM, withErrorHandling((event, streamId) => processManager.pauseVideoStream(streamId), 'pauseVideoStream'));
  ipcMain.handle(CHANNELS.RESUME_VIDEO_STREAM, withErrorHandling((event, streamId) => processManager.resumeVideoStream(streamId), 'resumeVideoStream'));
  
  ipcMain.handle(CHANNELS.TOGGLE_ANALYTICS, APP_VERSION === 'intellect' ? withErrorHandling((event, cameraId) => processManager.toggleAnalytics(cameraId, getMainWindow(), moduleManager), 'toggleAnalytics') : featureNotAvailableHandler);
  
  ipcMain.handle(CHANNELS.TOGGLE_RECORDING, withErrorHandling((event, camera) => processManager.toggleRecording(camera), 'toggleRecording'));
  ipcMain.handle(CHANNELS.OPEN_RECORDINGS_FOLDER, withErrorHandling(async () => { const s = await configManager.getAppSettings(); require('electron').shell.openPath(s.recordingsPath); }, 'openRecordingsFolder'));
  ipcMain.handle(CHANNELS.GET_RECORDINGS_FOR_DATE, withErrorHandling((event, data) => configManager.getRecordingsForDate(data), 'getRecordingsForDate'));
  ipcMain.handle(CHANNELS.EXPORT_ARCHIVE_CLIP, withErrorHandling((event, data) => processManager.exportArchiveClip(data, getMainWindow()), 'exportArchiveClip'));
  ipcMain.handle(CHANNELS.GET_EVENTS_FOR_DATE, withErrorHandling((event, data) => configManager.getEventsForDate(data), 'getEventsForDate'));
  ipcMain.handle(CHANNELS.GET_DATES_WITH_ACTIVITY, withErrorHandling((event, cameraName) => configManager.getDatesWithActivity(cameraName), 'getDatesWithActivity'));
  ipcMain.handle(CHANNELS.PREPARE_ARCHIVE_FOR_HLS, withErrorHandling((event, { filename }) => processManager.prepareArchiveForHls(filename), 'prepareArchiveForHls'));

  ipcMain.handle(CHANNELS.DISCOVER_DEVICES, withErrorHandling(() => discoverDevices(getMainWindow()), 'discoverDevices'));
  
  ipcMain.handle(CHANNELS.GET_NETIP_SETTINGS, withErrorHandling((event, camera) => cameraAPI.getNetipSettings(camera), 'getNetipSettings'));
  ipcMain.handle(CHANNELS.SET_NETIP_SETTINGS, withErrorHandling((event, data) => cameraAPI.setNetipSettings(data), 'setNetipSettings'));

  ipcMain.on(CHANNELS.SHOW_CAMERA_CONTEXT_MENU, (event, { cameraId, labels }) => {
      const template = [];
      const commands = ['open_in_browser', 'files', 'ssh', 'archive', 'edit', 'delete'];
      commands.forEach(command => {
          if ((command === 'files' || command === 'delete') && template.length > 0 && template[template.length - 1].type !== 'separator') {
              template.push({ type: 'separator' });
          }
          if (labels[command]) {
              template.push({ label: labels[command], click: () => getMainWindow()?.webContents.send(CHANNELS.ON_CONTEXT_MENU_COMMAND, { command, cameraId }) });
          }
      });
      if (template.length > 0) {
          Menu.buildFromTemplate(template).popup({ window: getMainWindow() });
      }
  });

  ipcMain.on(CHANNELS.SHOW_GROUP_CONTEXT_MENU, (event, { groupId, labels }) => {
      Menu.buildFromTemplate([
          { label: labels.rename, click: () => event.sender.send(CHANNELS.ON_GROUP_CONTEXT_MENU_COMMAND, { command: 'rename', groupId }) },
          { label: labels.delete, click: () => event.sender.send(CHANNELS.ON_GROUP_CONTEXT_MENU_COMMAND, { command: 'delete', groupId }) },
      ]).popup({ window: BrowserWindow.fromWebContents(event.sender) });
  });

  // SCP & SSH
  ipcMain.handle(CHANNELS.OPEN_FILE_MANAGER, (e, camera) => createFileManagerWindow(camera, fileManagerConnections));
  ipcMain.handle(CHANNELS.OPEN_SSH_TERMINAL, (e, camera) => { try { const win = createSshTerminalWindow(camera, sshConnections); if(win) cameraAPI.setupSshConnection(win, camera, sshConnections); } catch(err) { require('../services').handleError(err, 'openSshTerminal'); } });
  ipcMain.handle(CHANNELS.SCP_CONNECT, withErrorHandling((e, camera) => cameraAPI.scp.connect(camera, fileManagerConnections), 'scpConnect'));
  ipcMain.handle(CHANNELS.SCP_LIST, withErrorHandling((e, data) => cameraAPI.scp.list(data, fileManagerConnections), 'scpList'));
  ipcMain.handle(CHANNELS.SCP_DOWNLOAD, withErrorHandling((e, data) => cameraAPI.scp.download(e, data, fileManagerConnections), 'scpDownload'));
  ipcMain.handle(CHANNELS.SCP_UPLOAD, withErrorHandling((e, data) => cameraAPI.scp.upload(e, data, fileManagerConnections), 'scpUpload'));
  ipcMain.handle(CHANNELS.SCP_MKDIR, withErrorHandling((e, data) => cameraAPI.scp.mkdir(data, fileManagerConnections), 'scpMkdir'));
  ipcMain.handle(CHANNELS.SCP_DELETE_FILE, withErrorHandling((e, data) => cameraAPI.scp.deleteFile(data, fileManagerConnections), 'scpDeleteFile'));
  ipcMain.handle(CHANNELS.SCP_DELETE_DIR, withErrorHandling((e, data) => cameraAPI.scp.deleteDir(data, fileManagerConnections), 'scpDeleteDir'));
}

module.exports = {
  registerCameraHandlers
};