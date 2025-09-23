const { ipcMain, Menu, BrowserWindow, shell, app, dialog } = require('electron');
const fs = require('fs');
const path = require('path');
const { exec } = require('child_process');
const cameraAPI = require('../camera-api');
const processManager = require('../process-manager');
const configManager = require('../config-manager');
const { discoverDevices } = require('../discovery');
const { getMainWindow, createFileManagerWindow, createSshTerminalWindow } = require('../window-manager');
const CHANNELS = require('../../common/ipc-channels');

const withErrorHandling = (handler, context) => async (event, ...args) => {
    try {
        const result = await handler(event, ...args);
        return result === undefined ? { success: true } : result;
    } catch (error) {
        require('../services').handleError(error, context);
        return { success: false, error: error.message };
    }
};

const sshConnections = {};
const fileManagerConnections = {};

// Получение превью архива (thumbnails)
// (перемещено ниже всех require)
ipcMain.handle(CHANNELS.GET_ARCHIVE_THUMBNAILS, withErrorHandling((event, data) => processManager.getArchiveThumbnails(data), 'getArchiveThumbnails'));
function registerCameraHandlers(moduleManager, APP_VERSION) {
  const featureNotAvailableHandler = () => Promise.resolve({ success: false, error: 'Feature not available in Lite version' });
  
  ipcMain.handle(CHANNELS.OPEN_IN_BROWSER, withErrorHandling((event, ip) => {
      if (!ip) {
          throw new Error('IP address is required to open in browser.');
      }
      const url = ip.startsWith('http') ? ip : `http://${ip}`;
      console.log(`[IPC] Opening external URL: ${url}`);
      return shell.openExternal(url);
  }, 'openInBrowser'));

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
  
  ipcMain.handle('save-screenshot', withErrorHandling(async (event, { dataUrl, cameraName }) => {
    const settings = await configManager.getAppSettings();
    const screenshotsPath = settings.screenshotsPath;
    await fs.promises.mkdir(screenshotsPath, { recursive: true });
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    const safeCameraName = cameraName.replace(/[<>:"/\\|?*]/g, '_');
    const filename = `${safeCameraName}_${timestamp}.jpg`;
    const filePath = path.join(screenshotsPath, filename);
    const base64Data = dataUrl.replace(/^data:image\/jpeg;base64,/, "");
    const buffer = Buffer.from(base64Data, 'base64');
    await fs.promises.writeFile(filePath, buffer);
    return { success: true, path: filePath };
  }, 'saveScreenshot'));
  
  // VVVVVV --- НАЧАЛО ИСПРАВЛЕННОГО БЛОКА --- VVVVVV
  ipcMain.handle(CHANNELS.TOGGLE_ANALYTICS, async (event, cameraId, streamId) => {
      console.log(`[IPC Handler] Received '${CHANNELS.TOGGLE_ANALYTICS}' with cameraId: ${cameraId}, streamId: ${streamId}`);

      if (APP_VERSION !== 'intellect') {
          return featureNotAvailableHandler();
      }
      
      try {
          const result = await processManager.toggleAnalytics(cameraId, streamId, getMainWindow(), moduleManager);
          return result === undefined ? { success: true } : result;
      } catch (error) {
          require('../services').handleError(error, 'toggleAnalytics');
          return { success: false, error: error.message };
      }
  });
  // ^^^^^^ --- КОНЕЦ ИСПРАВЛЕННОГО БЛОКА --- ^^^^^^
  
  ipcMain.handle(CHANNELS.TOGGLE_RECORDING, withErrorHandling((event, camera) => processManager.toggleRecording(camera), 'toggleRecording'));
  
  ipcMain.on(CHANNELS.OPEN_RECORDINGS_FOLDER, async () => {
      try {
        const settings = await configManager.getAppSettings();
        const recordingsPath = settings.recordingsPath;
        
        console.log(`[IPC ON] Attempting to open recordings folder at: ${recordingsPath}`);

        await fs.promises.mkdir(recordingsPath, { recursive: true });
        
        const errorMessage = await shell.openPath(recordingsPath);
        
        if (errorMessage) {
            console.error(`[IPC ON] shell.openPath failed: ${errorMessage}`);
        } else {
            console.log(`[IPC ON] shell.openPath command issued successfully.`);
        }
      } catch (error) {
          console.error(`[IPC ON] Error in openRecordingsFolder:`, error);
          dialog.showErrorBox('Ошибка', `Не удалось открыть папку с записями: ${error.message}`);
      }
  });

  ipcMain.handle(CHANNELS.GET_RECORDINGS_FOR_DATE, withErrorHandling((event, data) => configManager.getRecordingsForDate(data), 'getRecordingsForDate'));
  ipcMain.handle(CHANNELS.EXPORT_ARCHIVE_CLIP, withErrorHandling((event, data) => processManager.exportArchiveClip(data, getMainWindow()), 'exportArchiveClip'));
  ipcMain.handle(CHANNELS.GET_EVENTS_FOR_DATE, withErrorHandling((event, data) => configManager.getEventsForDate(data), 'getEventsForDate'));
  ipcMain.handle(CHANNELS.GET_DATES_WITH_ACTIVITY, withErrorHandling((event, cameraName) => configManager.getDatesWithActivity(cameraName), 'getDatesWithActivity'));
  ipcMain.handle(CHANNELS.PREPARE_ARCHIVE_FOR_HLS, withErrorHandling((event, data) => processManager.prepareArchiveForHls(data), 'prepareArchiveForHls'));

  ipcMain.handle(CHANNELS.DISCOVER_DEVICES, withErrorHandling(() => discoverDevices(getMainWindow()), 'discoverDevices'));
  
  ipcMain.handle(CHANNELS.GET_NETIP_SETTINGS, withErrorHandling((event, camera) => cameraAPI.getNetipSettings(camera), 'getNetipSettings'));
  ipcMain.handle(CHANNELS.SET_NETIP_SETTINGS, withErrorHandling((event, data) => cameraAPI.setNetipSettings(data), 'setNetipSettings'));

  ipcMain.on(CHANNELS.SHOW_CAMERA_CONTEXT_MENU, (event, { cameraId, labels }) => {
      const mainWindow = getMainWindow();
      if (!mainWindow) return;

      const template = [];
      const commands = ['open_in_browser', 'files', 'ssh', 'archive', 'edit', 'delete'];
      
      const clickHandler = async (command) => {
          try {
              const camera = await configManager.getCameraConfig(cameraId);
              if (!camera) throw new Error(`Camera with ID ${cameraId} not found.`);

              switch(command) {
                  case 'open_in_browser':
                      const url = camera.ip.startsWith('http') ? camera.ip : `http://${camera.ip}`;
                      shell.openExternal(url);
                      break;
                  case 'files':
                      createFileManagerWindow(camera, fileManagerConnections);
                      break;
                  case 'ssh':
                      const win = createSshTerminalWindow(camera, sshConnections);
                      if(win) cameraAPI.setupSshConnection(win, camera, sshConnections);
                      break;
                  case 'archive':
                  case 'edit':
                  case 'delete':
                      mainWindow.webContents.send(CHANNELS.ON_CONTEXT_MENU_COMMAND, { command, cameraId });
                      break;
              }
          } catch (error) {
              require('../services').handleError(error, `contextMenu:${command}`);
          }
      };

      commands.forEach(command => {
          if ((command === 'files' || command === 'delete') && template.length > 0 && template[template.length - 1].type !== 'separator') {
              template.push({ type: 'separator' });
          }
          if (labels[command]) {
              template.push({ label: labels[command], click: () => clickHandler(command) });
          }
      });

      if (template.length > 0) {
          Menu.buildFromTemplate(template).popup({ window: mainWindow });
      }
  });

  ipcMain.on(CHANNELS.SHOW_GROUP_CONTEXT_MENU, (event, { groupId, labels }) => {
      Menu.buildFromTemplate([
          { label: labels.rename, click: () => event.sender.send(CHANNELS.ON_GROUP_CONTEXT_MENU_COMMAND, { command: 'rename', groupId }) },
          { label: labels.delete, click: () => event.sender.send(CHANNELS.ON_GROUP_CONTEXT_MENU_COMMAND, { command: 'delete', groupId }) },
      ]).popup({ window: BrowserWindow.fromWebContents(event.sender) });
  });

  ipcMain.handle(CHANNELS.OPEN_FILE_MANAGER, (e, camera) => createFileManagerWindow(camera, fileManagerConnections));
  ipcMain.handle(CHANNELS.OPEN_SSH_TERMINAL, (e, camera) => { try { const win = createSshTerminalWindow(camera, sshConnections); if(win) cameraAPI.setupSshConnection(win, camera, sshConnections); } catch(err) { require('../services').handleError(err, 'openSshTerminal'); } });
  ipcMain.handle(CHANNELS.SCP_CONNECT, withErrorHandling((e, camera) => cameraAPI.scp.connect(camera, fileManagerConnections), 'scpConnect'));
  ipcMain.handle(CHANNELS.SCP_LIST, withErrorHandling((e, data) => cameraAPI.scp.list(data, fileManagerConnections), 'scpList'));
  ipcMain.handle(CHANNELS.SCP_DOWNLOAD, withErrorHandling((e, data) => cameraAPI.scp.download(e, data, fileManagerConnections), 'scpDownload'));
  ipcMain.handle(CHANNELS.SCP_UPLOAD, withErrorHandling((e, data) => cameraAPI.scp.upload(e, data, fileManagerConnections), 'scpUpload'));
  ipcMain.handle(CHANNELS.SCP_MKDIR, withErrorHandling((e, data) => cameraAPI.scp.mkdir(data, fileManagerConnections), 'scpMkdir'));
  ipcMain.handle(CHANNELS.SCP_DELETE_FILE, withErrorHandling((e, data) => cameraAPI.scp.deleteFile(data, fileManagerConnections), 'scpDeleteFile'));
  ipcMain.handle(CHANNELS.SCP_DELETE_DIR, withErrorHandling((e, data) => cameraAPI.scp.deleteDir(data, fileManagerConnections), 'scpDeleteDir'));

  // Simple RTSP URL probe: send OPTIONS or DESCRIBE request and read response status/raw
  const net = require('net');
    const crypto = require('crypto');
  ipcMain.handle(CHANNELS.TEST_RTSP_URL, async (event, { url, timeout = 3000, method = 'OPTIONS' }) => {
      return new Promise((resolve) => {
          try {
              // Parse URL: rtsp://user:pass@host:port/path
              const m = url.match(/^rtsp:\/\/([^@]+)@([^:\/\]]+)(?::(\d+))?(\/.*)$/);
              if (!m) return resolve({ success: false, error: 'Invalid RTSP URL format' });
              const creds = decodeURIComponent(m[1]);
              const host = m[2];
              const port = parseInt(m[3] || '8554', 10);
              const pathPart = m[4];

              const client = new net.Socket();
              let isResolved = false;
              client.setTimeout(timeout);

              const sendRequest = (authHeader) => {
                  let req = '';
                  if (method === 'DESCRIBE') {
                      req = `DESCRIBE ${url} RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: OpenIPC-Test\r\nAccept: application/sdp\r\n`;
                  } else {
                      req = `OPTIONS ${url} RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: OpenIPC-Test\r\n`;
                  }
                  if (authHeader) req += authHeader + '\r\n';
                  req += '\r\n';
                  try {
                      client.write(req);
                  } catch (e) {
                      // ignore write errors if socket closed
                  }
              };

              client.connect(port, host, () => sendRequest());

              let buffer = Buffer.alloc(0);
              client.on('data', (data) => {
                  buffer = Buffer.concat([buffer, data]);
                  const txt = buffer.toString();
                  const mStatus = txt.match(/^RTSP\/1\.0\s+(\d{3})\s+(.*)$/m);
                  if (mStatus) {
                      const code = parseInt(mStatus[1], 10);
                      if (!isResolved) {
                          isResolved = true;
                          client.destroy();
                          // Try to extract headers (until first empty line)
                          const headerEnd = txt.indexOf('\r\n\r\n');
                          const headersTxt = headerEnd !== -1 ? txt.slice(0, headerEnd) : txt;
                          const headers = {};
                          headersTxt.split('\r\n').slice(1).forEach(line => {
                              const idx = line.indexOf(':');
                              if (idx > 0) {
                                  const k = line.slice(0, idx).trim();
                                  const v = line.slice(idx + 1).trim();
                                  headers[k] = v;
                              }
                          });
                          // If 401 with Digest challenge, attempt Digest auth and retry once
                          if (code === 401 && headers['WWW-Authenticate'] && headers['WWW-Authenticate'].toLowerCase().includes('digest') && creds && creds.includes(':')) {
                              // Parse Digest params
                              const wa = headers['WWW-Authenticate'];
                              const params = {};
                              wa.replace(/(\w+)=(?:"([^"]*)"|([^,]*))/g, (_, k, v1, v2) => { params[k] = v1 || v2; });
                              const realm = params.realm || '';
                              const nonce = params.nonce || '';
                              const qop = params.qop || '';
                              const uri = pathPart;
                              const [username, password] = creds.split(':');
                              const ha1 = crypto.createHash('md5').update(`${username}:${realm}:${password}`).digest('hex');
                              const ha2 = crypto.createHash('md5').update(`${method}:${uri}`).digest('hex');
                              const nc = '00000001';
                              const cnonce = crypto.randomBytes(8).toString('hex');
                              let response;
                              if (qop) {
                                  response = crypto.createHash('md5').update(`${ha1}:${nonce}:${nc}:${cnonce}:${qop}:${ha2}`).digest('hex');
                              } else {
                                  response = crypto.createHash('md5').update(`${ha1}:${nonce}:${ha2}`).digest('hex');
                              }
                              let authHeader = `Authorization: Digest username="${username}", realm="${realm}", nonce="${nonce}", uri="${uri}", response="${response}"`;
                              if (qop) authHeader += `, qop=${qop}, nc=${nc}, cnonce="${cnonce}"`;

                              // Retry with Digest on a fresh socket
                              const retryClient = new net.Socket();
                              let retrResolved = false;
                              retryClient.setTimeout(timeout);
                              retryClient.connect(port, host, () => {
                                  // send request with Digest auth
                                  let retryReq = '';
                                  if (method === 'DESCRIBE') {
                                      retryReq = `DESCRIBE ${url} RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: OpenIPC-Test\r\nAccept: application/sdp\r\n`;
                                  } else {
                                      retryReq = `OPTIONS ${url} RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: OpenIPC-Test\r\n`;
                                  }
                                  retryReq += authHeader + '\r\n\r\n';
                                  retryClient.write(retryReq);
                              });
                              let rbuf = Buffer.alloc(0);
                              retryClient.on('data', (d2) => {
                                  rbuf = Buffer.concat([rbuf, d2]);
                                  const rtxt = rbuf.toString();
                                  const m2 = rtxt.match(/^RTSP\/1\.0\s+(\d{3})\s+(.*)$/m);
                                  if (m2 && !retrResolved) {
                                      retrResolved = true;
                                      retryClient.destroy();
                                      const code2 = parseInt(m2[1], 10);
                                      const headerEnd2 = rtxt.indexOf('\r\n\r\n');
                                      const headersTxt2 = headerEnd2 !== -1 ? rtxt.slice(0, headerEnd2) : rtxt;
                                      const headers2 = {};
                                      headersTxt2.split('\r\n').slice(1).forEach(line => {
                                          const idx = line.indexOf(':');
                                          if (idx > 0) {
                                              const k = line.slice(0, idx).trim();
                                              const v = line.slice(idx + 1).trim();
                                              headers2[k] = v;
                                          }
                                      });
                                      return resolve({ success: true, statusCode: code2, statusText: m2[2].trim(), raw: rtxt.slice(0, 8192), headers: headers2 });
                                  }
                                  if (rbuf.length > 8192 && !retrResolved) {
                                      retrResolved = true;
                                      retryClient.destroy();
                                      return resolve({ success: false, error: 'No RTSP status in retry response', raw: rtxt.slice(0, 8192) });
                                  }
                              });
                              retryClient.on('timeout', () => { if (!retrResolved) { retrResolved = true; retryClient.destroy(); return resolve({ success: false, error: 'Timeout' }); } });
                              retryClient.on('error', (e2) => { if (!retrResolved) { retrResolved = true; return resolve({ success: false, error: e2.message }); } });
                              return;
                          }

                          resolve({ success: true, statusCode: code, statusText: mStatus[2].trim(), raw: txt.slice(0, 8192), headers });
                      }
                  } else {
                      // If buffer got reasonably large and no RTSP status, fail early
                      if (buffer.length > 8192 && !isResolved) {
                          isResolved = true;
                          client.destroy();
                          resolve({ success: false, error: 'No RTSP status in response', raw: txt.slice(0, 8192) });
                      }
                  }
              });

              client.on('timeout', () => {
                  if (!isResolved) {
                      isResolved = true;
                      client.destroy();
                      resolve({ success: false, error: 'Timeout' });
                  }
              });

              client.on('error', (err) => {
                  if (!isResolved) {
                      isResolved = true;
                      resolve({ success: false, error: err.message });
                  }
              });
          } catch (e) {
              resolve({ success: false, error: e.message });
          }
      });
  });

  // Probe camera HTTP endpoints with provided credentials to gather info useful for auto-filling RTSP paths
  const http = require('http');
  const https = require('https');
  ipcMain.handle(CHANNELS.PROBE_CAMERA_INFO, async (event, { ip, port = 80, username, password, timeout = 3000 }) => {
      return new Promise((resolve) => {
          try {
              const opts = {
                  hostname: ip,
                  port: port,
                  method: 'GET',
                  path: '/',
                  timeout: timeout,
                  headers: {}
              };
              if (username && password) {
                  const auth = Buffer.from(`${username}:${password}`, 'utf8').toString('base64');
                  opts.headers['Authorization'] = `Basic ${auth}`;
              }

              const client = port === 443 ? https : http;
              const req = client.request(opts, (res) => {
                  const chunks = [];
                  res.on('data', (d) => chunks.push(d));
                  res.on('end', () => {
                      const body = Buffer.concat(chunks).toString();
                      // Return some useful headers and a snippet of body
                      const headers = res.headers;
                      const snippet = body.slice(0, 4096);
                      resolve({ success: true, statusCode: res.statusCode, headers, snippet });
                  });
              });
              req.on('error', (err) => resolve({ success: false, error: err.message }));
              req.on('timeout', () => {
                  req.destroy();
                  resolve({ success: false, error: 'Timeout' });
              });
              req.end();
          } catch (e) {
              resolve({ success: false, error: e.message });
          }
      });
  });

  // ONVIF fallback: try to call /onvif/device_service to get profiles and stream URIs
  const { URL } = require('url');
  ipcMain.handle(CHANNELS.PROBE_ONVIF_STREAM_URI, async (event, { ip, port = 80, username, password, timeout = 4000 }) => {
      return new Promise((resolve) => {
          try {
              const deviceService = `http://${ip}:${port}/onvif/device_service`;
              // Build SOAP request for GetProfiles
              const getProfilesBody = `<?xml version="1.0" encoding="utf-8"?>\n<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">\n<SOAP-ENV:Body>\n<tds:GetProfiles/>\n</SOAP-ENV:Body>\n</SOAP-ENV:Envelope>`;

              const opts = new URL(deviceService);
              const client = opts.protocol === 'https:' ? https : http;
              const requestOpts = {
                  hostname: opts.hostname,
                  port: opts.port || (opts.protocol === 'https:' ? 443 : 80),
                  path: opts.pathname,
                  method: 'POST',
                  headers: {
                      'Content-Type': 'application/soap+xml; charset=utf-8',
                      'Content-Length': Buffer.byteLength(getProfilesBody)
                  },
                  timeout: timeout
              };
              if (username && password) {
                  const auth = Buffer.from(`${username}:${password}`, 'utf8').toString('base64');
                  requestOpts.headers['Authorization'] = `Basic ${auth}`;
              }

              const req = client.request(requestOpts, (res) => {
                  const chunks = [];
                  res.on('data', d => chunks.push(d));
                  res.on('end', async () => {
                      const body = Buffer.concat(chunks).toString();
                      // Try to extract profile token(s)
                      const tokenMatch = body.match(/<tt:Profile[^>]*token=\"([^\"]+)\"/i) || body.match(/token=\"([^\"]+)\"/i);
                      if (!tokenMatch) return resolve({ success: false, error: 'No ONVIF profiles found' });
                      const token = tokenMatch[1];

                      // Build SOAP request for GetStreamUri using the profile token
                      const getStreamUriBody = `<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\">\n<SOAP-ENV:Body>\n<trt:GetStreamUri>\n<trt:StreamSetup>\n<trt:Stream>RTP-Unicast</trt:Stream>\n<trt:Transport>\n<trt:Protocol>RTSP</trt:Protocol>\n</trt:Transport>\n</trt:StreamSetup>\n<trt:ProfileToken>${token}</trt:ProfileToken>\n</trt:GetStreamUri>\n</SOAP-ENV:Body>\n</SOAP-ENV:Envelope>`;

                      const opts2 = Object.assign({}, requestOpts, { headers: { 'Content-Type': 'application/soap+xml; charset=utf-8', 'Content-Length': Buffer.byteLength(getStreamUriBody) } });
                      if (username && password) opts2.headers['Authorization'] = requestOpts.headers['Authorization'];

                      const req2 = client.request(opts2, (res2) => {
                          const chunks2 = [];
                          res2.on('data', d => chunks2.push(d));
                          res2.on('end', () => {
                              const body2 = Buffer.concat(chunks2).toString();
                              // Extract URI from <tt:Uri> or <Uri>
                              const uriMatch = body2.match(/<tt:Uri>([^<]+)<\/tt:Uri>/i) || body2.match(/<Uri>([^<]+)<\/Uri>/i);
                              if (uriMatch) {
                                  return resolve({ success: true, uri: uriMatch[1] });
                              }
                              return resolve({ success: false, error: 'No stream URI in GetStreamUri response' });
                          });
                      });
                      req2.on('error', (err) => resolve({ success: false, error: err.message }));
                      req2.on('timeout', () => { req2.destroy(); resolve({ success: false, error: 'Timeout' }); });
                      req2.write(getStreamUriBody);
                      req2.end();
                  });
              });
              req.on('error', (err) => resolve({ success: false, error: err.message }));
              req.on('timeout', () => { req.destroy(); resolve({ success: false, error: 'Timeout' }); });
              req.write(getProfilesBody);
              req.end();
          } catch (e) {
              resolve({ success: false, error: e.message });
          }
      });
  });

  ipcMain.handle('get-analytics-states', withErrorHandling(() => processManager.getAnalyticsStates(), 'getAnalyticsStates'));
}

module.exports = {
  registerCameraHandlers
};