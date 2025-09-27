const fs = require('fs/promises');
const path = require('path');
const sharp = require('sharp');
const { spawn } = require('child_process');
// Use central auth-manager for reliable password lookup
const authManagerMain = require(path.join(__dirname, '..', '..', 'src', 'main', 'auth-manager'));

// Map cameraId -> child process
const analyticsProcesses = new Map();
const currentStreamIds = new Map();
const SAVE_COOLDOWN_MS = 5000;
const lastSaveTimestamps = {};

// Storage for detected license plates
const detectedPlates = new Map(); // cameraId -> Set of detected texts
const platesHistory = []; // Array of all detections with timestamps

async function ensureDir(dir) {
  try { await fs.mkdir(dir, { recursive: true }); } catch (e) { }
}

async function savePlatesToFile(api) {
  try {
    const dataPath = api.configManager.getDataPath();
    const platesFile = path.join(dataPath, 'detected_plates.json');
    
    const data = {
      lastUpdated: new Date().toISOString(),
      totalDetections: platesHistory.length,
      uniquePlates: Array.from(detectedPlates.entries()).map(([cameraId, plates]) => ({
        cameraId: parseInt(cameraId),
        plates: Array.from(plates),
        count: plates.size
      })),
      history: platesHistory.slice(-1000) // Keep last 1000 detections
    };
    
    await fs.writeFile(platesFile, JSON.stringify(data, null, 2));
    console.log('[Module: LicensePlate] Saved plates data to', platesFile);
  } catch (e) {
    console.error('[Module: LicensePlate] Failed to save plates data', e);
  }
}

function findPythonExecutable() {
  try {
    const repoRoot = path.join(__dirname, '..', '..');
    // Prefer the project's venv (created at repoRoot/venv) as the default Python executable.
    // This venv is used during development and is where onnxruntime-directml was installed.
    const projectVenv = path.join(repoRoot, 'venv', 'Scripts', 'python.exe');
    if (require('fs').existsSync(projectVenv)) return projectVenv;
    // Fallback to legacy analytics venv (if present)
    const analyticsVenv = path.join(repoRoot, '.analytics_venvs', 'dml', 'Scripts', 'python.exe');
    if (require('fs').existsSync(analyticsVenv)) return analyticsVenv;
    // Final fallback to system python on PATH
    return 'python';
  } catch (e) { return 'python'; }
}

async function getCurrentStreamIdForCamera(api, cameraId) {
  try {
    const state = await api.getAppState();
    if (!state || !state.layouts) return 0;
    const activeLayout = state.layouts.find(l => l.id === state.activeLayoutId);
    if (!activeLayout || !activeLayout.gridState) return 0;
    for (const cell of activeLayout.gridState) {
      if (cell && cell.camera && cell.camera.id === cameraId) {
        return cell.streamId || 0;
      }
    }
  } catch (e) {
    console.error('[Module: LicensePlate] Failed to get current streamId for camera', cameraId, e);
  }
  return 0;
}

function buildRtspForCamera(camera, streamId) {
  // Prefer explicit stream path from camera config when present
  try {
    if (!camera) return `rtsp://127.0.0.1:8554/cam${camera.id}_${streamId}`;
    const streamPath = streamId === 1 ? camera.streamPath1 : camera.streamPath0;
    if (streamPath && typeof streamPath === 'string') {
      const sp = streamPath.trim();
      if (sp.startsWith('rtsp://') || sp.startsWith('rtsps://')) return sp;
      // If streamPath looks like a relative path (e.g. /stream=0) prefer using MediaMTX local source
      if (sp.startsWith('/')) {
        console.log('[Module: LicensePlate] Using local MediaMTX path for camera', camera.id, 'stream', streamId, sp);
        return `rtsp://127.0.0.1:8554/cam${camera.id}_${streamId}`;
      }
      // if streamPath is a path like stream=0 (no leading slash), try build direct RTSP URL from camera.ip with credentials
      if (camera.ip) {
        const port = camera.port || 554;
        const user = camera.username || '';
        const pass = camera.password || '';
        const url = `rtsp://${user}${pass ? ':' + pass : ''}${user || pass ? '@' : ''}${camera.ip}:${port}${sp.startsWith('/') ? '' : '/'}${sp}`;
        console.log('[Module: LicensePlate] Using direct RTSP URL for camera', camera.id, 'stream', streamId, url.replace(/:[^:]+@/, ':****@'));
        return url;
      }
    }
  } catch (e) {
    // ignore and fallback to mediamtx path
  }
  // Fallback to local MediaMTX path which is usually available
  console.log('[Module: LicensePlate] Falling back to local MediaMTX path for camera', camera && camera.id, 'stream', streamId);
  return `rtsp://127.0.0.1:8554/cam${camera.id}_${streamId}`;
}

async function handleAnalyticsLine(api, cameraId, line) {
  let result;
  try { result = JSON.parse(line); } catch (e) { return; }
  console.log('[Module: LicensePlate] Incoming analytics line:', line);
  // If runner provided recognized plates (runner saved crops and returned paths), forward them
  if (result.recognized && Array.isArray(result.recognized) && result.recognized.length > 0) {
    const now = Date.now();
    const last = lastSaveTimestamps[cameraId] || 0;
    if (now - last < SAVE_COOLDOWN_MS) {
      try { if (result.frame_path) await fs.unlink(result.frame_path); } catch (e) {}
      return;
    }
    for (const r of result.recognized) {
      try {
        // r: {path, score, text}
        const plateText = r.text || '';
        console.log('[Module: LicensePlate] Processing plate text:', plateText);
        if (plateText.trim()) {
          // Add to detected plates set
          if (!detectedPlates.has(cameraId.toString())) {
            detectedPlates.set(cameraId.toString(), new Set());
          }
          detectedPlates.get(cameraId.toString()).add(plateText);
          
          // Add to history
          platesHistory.push({
            cameraId,
            text: plateText,
            score: r.score,
            timestamp: new Date().toISOString(),
            path: r.path
          });
          
          console.log('[Module: LicensePlate] Added plate to history. Total history:', platesHistory.length);
          
          // Save to file periodically (every 10 detections) or immediately for testing
          if (platesHistory.length % 10 === 0 || platesHistory.length < 5) {
            console.log('[Module: LicensePlate] Saving plates to file...');
            savePlatesToFile(api);
          }
        }
        
        api.sendToRenderer('module-license-plate-saved', { cameraId, path: r.path, text: r.text, score: r.score });
        console.log('[Module: LicensePlate] Recognized and saved', r.path, r.text || 'no text', `score: ${r.score}`);
        lastSaveTimestamps[cameraId] = now;
      } catch (e) { console.error('[Module: LicensePlate] Failed to notify renderer for recognized plate', e); }
    }
    try { if (result.frame_path) await fs.unlink(result.frame_path); } catch (e) {}
    return;
  }

  if (!result.objects || !result.frame_path) {
    console.log('[Module: LicensePlate] Skipping: missing objects or frame_path', { hasObjects: !!result.objects, frame_path: result.frame_path });
    // If the runner wrote a debug frame (nodet) copy it into the save dir for inspection
    try {
      if (result.frame_path && result.frame_path.endsWith('_nodet.jpg')) {
        const moduleFolderName = path.basename(__dirname);
        const savePathKey = `module_${moduleFolderName}_savePath`;
        const settings = await api.configManager.getAppSettings();
        const saveDir = settings[savePathKey] || path.join(api.configManager.getDataPath(), 'plates');
        await ensureDir(saveDir);
        const basename = path.basename(result.frame_path);
        const dest = path.join(saveDir, `debug_${cameraId}_${basename}`);
        try { await fs.copyFile(result.frame_path, dest); console.log('[Module: LicensePlate] Copied debug frame to', dest); } catch (e) { console.error('[Module: LicensePlate] Failed to copy debug frame', e); }
      }
    } catch (e) {
      console.error('[Module: LicensePlate] Error while attempting to copy debug frame', e);
    }
    return;
  }
  const plates = result.objects.filter(o => o.label === 'license_plate' || o.label === 'plate' || o.label === 'number_plate');
  if (plates.length === 0) {
    // No plate detections. If the runner provided a debug frame (ends with _nodet.jpg), copy it to save dir
    try {
      if (result.frame_path && result.frame_path.endsWith('_nodet.jpg')) {
        try {
          const moduleFolderName = path.basename(__dirname);
          const savePathKey = `module_${moduleFolderName}_savePath`;
          const settings = await api.configManager.getAppSettings();
          const saveDir = settings[savePathKey] || path.join(api.configManager.getDataPath(), 'plates');
          await ensureDir(saveDir);
          const basename = path.basename(result.frame_path);
          const dest = path.join(saveDir, `debug_${cameraId}_${basename}`);
          await fs.copyFile(result.frame_path, dest);
          console.log('[Module: LicensePlate] Copied debug frame to', dest);
        } catch (e) {
          console.error('[Module: LicensePlate] Failed to copy debug frame', e);
        }
      }
    } catch (e) {
      console.error('[Module: LicensePlate] Error while attempting to copy debug frame', e);
    }
    try { await fs.unlink(result.frame_path); } catch (e) {}
    return;
  }

  const now = Date.now();
  const last = lastSaveTimestamps[cameraId] || 0;
  if (now - last < SAVE_COOLDOWN_MS) { try { await fs.unlink(result.frame_path); } catch (e) {} ; return; }

  let saveDir;
  try {
    const settings = await api.configManager.getAppSettings();
    const moduleFolderName = path.basename(__dirname);
    const savePathKey = `module_${moduleFolderName}_savePath`;
    if (settings[savePathKey]) saveDir = settings[savePathKey];
    else saveDir = path.join(api.configManager.getDataPath(), 'plates');
    await ensureDir(saveDir);
  } catch (e) {
    console.error('[Module: LicensePlate] Error determining save dir', e);
    try { await fs.unlink(result.frame_path); } catch (e) {}
    return;
  }

  try {
    const image = sharp(result.frame_path);
    const p = plates[0];
    const { x, y, w, h } = p.box;
    if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > result.frame_width || y + h > result.frame_height) {
      try { await fs.unlink(result.frame_path); } catch (e) {}
      return;
    }

    const paddingX = Math.round(w * 0.2);
    const paddingY = Math.round(h * 0.4);
    let ex = x - paddingX;
    let ey = y - paddingY;
    let ew = w + paddingX * 2;
    let eh = h + paddingY * 2;
    if (ex < 0) ex = 0;
    if (ey < 0) ey = 0;
    if (ex + ew > result.frame_width) ew = result.frame_width - ex;
    if (ey + eh > result.frame_height) eh = result.frame_height - ey;

    const buf = await image.extract({ left: ex, top: ey, width: ew, height: eh }).toBuffer();
    const timestamp = new Date().toISOString().replace(/:/g,'-').slice(0,-5);
    const suffix = Math.random().toString(36).substring(2,7);
    const filename = `plate_${cameraId}_${timestamp}_${suffix}.jpg`;
    const fp = path.join(saveDir, filename);
    await fs.writeFile(fp, buf);

    lastSaveTimestamps[cameraId] = now;

    api.sendToRenderer('module-license-plate-saved', { cameraId, path: fp });
    console.log('[Module: LicensePlate] Saved plate to', fp);
  } catch (e) {
    console.error('[Module: LicensePlate] Error processing frame', e);
  } finally {
    try { await fs.unlink(result.frame_path); } catch (e) {}
  }
}

async function spawnAnalyticsForCamera(api, camera) {
  const cameraId = camera.id;
  if (analyticsProcesses.has(cameraId)) return;
  const streamId = await getCurrentStreamIdForCamera(api, cameraId);
  console.log('[Module: LicensePlate] Starting analytics for camera', cameraId, 'on stream', streamId);
  currentStreamIds.set(cameraId, streamId);
  const repoRoot = path.join(__dirname, '..', '..');
  const python = findPythonExecutable();
  const script = path.join(repoRoot, 'python_src', 'test_plate_yunet.py');
  const fsSync = require('fs');
  if (!fsSync.existsSync(python)) {
    console.error(`[LicensePlate-Analytics] Python executable not found: ${python}`);
    return;
  }
  if (!fsSync.existsSync(script)) {
    console.error(`[LicensePlate-Analytics] Script not found: ${script}`);
    return;
  }

  const settings = await api.configManager.getAppSettings();
  const moduleFolderName = path.basename(__dirname);
  const savePathKey = `module_${moduleFolderName}_savePath`;
  const configuredSave = settings[savePathKey] || path.join(api.configManager.getDataPath(), 'plates');
  await ensureDir(configuredSave);

  const configForScript = {
    objects: ['license_plate'],
    confidence: camera.analyticsConfig?.confidence || 0.5,
    frame_skip: 1,
    // enable debug frame writes so we can inspect frames even when no detections
    debug_write_frames: true,
    save_path: configuredSave,
  };
  const configArg = Buffer.from(JSON.stringify(configForScript)).toString('base64');
  // Try to construct a direct RTSP URL (with credentials) using configManager and authManager.
  // Always use MediaMTX for seamless stream switching
  const rtspUrl = buildRtspForCamera(camera, streamId);

  console.log('[Module: LicensePlate] Spawning analytics runner for camera', cameraId, '->', script);
  console.log('[Module: LicensePlate] Using python executable:', python);
  console.log('[Module: LicensePlate] RTSP URL:', rtspUrl);
  console.log('[Module: LicensePlate] Config(base64):', configArg.length ? `${configArg.substring(0,8)}...(${configArg.length}b)` : '<none>');

  // Получаем настройки frame_skip и resize_width из глобальных настроек
  const frameSkip = parseInt(settings.analytics_frame_skip, 10) || 5;
  const resizeWidth = parseInt(settings.analytics_resize_width, 10) || 640;
  // --- Передача параметров распознавания номеров из appSettings ---
  const minScore = settings.plate_min_score != null ? settings.plate_min_score : 0.65;
  const minArea = settings.plate_min_area != null ? settings.plate_min_area : 1000;
  const minHeight = settings.plate_min_height != null ? settings.plate_min_height : 30;
  const minAspect = settings.plate_min_aspect != null ? settings.plate_min_aspect : 1.2;
  const maxAspect = settings.plate_max_aspect != null ? settings.plate_max_aspect : 7.5;
  const allowlist = settings.plate_allowlist || 'АБВЕКМНОРСТУХ0123456789';

  const args = [
    script,
    '--video', rtspUrl,
    '--frame-skip', String(frameSkip),
    '--save-dir', configuredSave,
    '--min-score', String(minScore),
    '--min-area', String(minArea),
    '--min-height', String(minHeight),
    '--min-aspect', String(minAspect),
    '--max-aspect', String(maxAspect),
    '--allowlist', allowlist
  ];
  if (resizeWidth > 0) args.push('--resize-width', String(resizeWidth));
  // Respect module setting to prefer ONNX Runtime (DirectML) if available
  try {
    const useOrtSetting = settings['module_license-plate_use_ort'];
    if (useOrtSetting === true || useOrtSetting === 'true' || useOrtSetting === '1') {
      args.push('--use-ort');
      console.log('[Module: LicensePlate] Passing --use-ort to runner');
    }
  } catch (e) { /* ignore */ }
  const pythonSrcDir = path.join(repoRoot, 'python_src');
  console.log('[LicensePlate-Analytics] Using cwd for runner:', pythonSrcDir);
  let proc;
  try {
    proc = spawn(python, args, { windowsHide: true, cwd: pythonSrcDir });
  } catch (err) {
    console.error('[LicensePlate-Analytics] Synchronous spawn error for camera', cameraId, err);
    return;
  }
  proc.on('error', (err) => {
    console.error('[LicensePlate-Analytics] Failed to spawn process for camera', cameraId, err);
  });
  analyticsProcesses.set(cameraId, proc);

  proc.stdout.on('data', async (data) => {
    const lines = data.toString().split('\n').filter(Boolean);
    for (const line of lines) {
      try { await handleAnalyticsLine(api, cameraId, line); } catch (e) { console.error(e); }
    }
  });

  proc.stderr.on('data', (data) => {
    const msg = data.toString().trim();
    if (msg.length > 0) {
      console.error('[LicensePlate-Analytics] stderr:', msg);
    } else {
      console.error('[LicensePlate-Analytics] stderr: <empty>');
    }
  });

  // Watch for RTSP auth errors (401) in stderr and restart using MediaMTX fallback if needed
  let sawUnauthorized = false;
  proc.stderr.on('data', (d) => {
    const s = d.toString();
      if (s.includes('401 Unauthorized') || s.toLowerCase().includes('method describe failed: 401')) {
      sawUnauthorized = true;
      console.warn('[LicensePlate-Analytics] Detected RTSP 401 Unauthorized for camera', cameraId, '- will restart using MediaMTX fallback');
      try {
        console.warn('[LicensePlate-Analytics] About to kill process for camera', cameraId, 'due to RTSP 401; stack:\n', new Error().stack);
        proc.kill();
      } catch (e) { console.error('[LicensePlate-Analytics] Failed to kill process after 401:', e); }
      // respawn with MediaMTX URL
      const mediaMtxUrl = `rtsp://127.0.0.1:8554/cam${cameraId}_0`;
      console.log('[Module: LicensePlate] Respawning runner for camera', cameraId, 'with MediaMTX URL:', mediaMtxUrl);
      const newProc = spawn(python, [script, '--video', mediaMtxUrl, '--frame-skip', String(frameSkip), '--save-dir', configuredSave], { windowsHide: true, cwd: pythonSrcDir });
      analyticsProcesses.set(cameraId, newProc);
      // wire up handlers for the new process (reuse existing handlers lightly)
      newProc.stdout.on('data', async (data2) => {
        const lines = data2.toString().split('\n').filter(Boolean);
        for (const line of lines) { try { await handleAnalyticsLine(api, cameraId, line); } catch (e) { console.error(e); } }
      });
      newProc.stderr.on('data', (d2) => { const m = d2.toString().trim(); if (m) console.error('[LicensePlate-Analytics] stderr (fallback):', m); });
      newProc.on('exit', (c, s) => { console.log(`[LicensePlate-Analytics] Fallback process for camera ${cameraId} exit (code=${c})`); analyticsProcesses.delete(cameraId); });
    }
  });

  // Additionally, if the runner reports inability to open the RTSP source (common when direct camera
  // connection is blocked or required OpenCV backends are not available), attempt to respawn using MediaMTX.
  proc.stderr.on('data', (d) => {
    const s = d.toString();
    // If we've already attempted fallback for this process, don't loop
    if (proc.__attemptedMediaMtxFallback) return;
    if (s.includes('Failed to open video source') || s.includes('All backends failed to open source')) {
      proc.__attemptedMediaMtxFallback = true;
      try {
        console.warn('[LicensePlate-Analytics] Detected failure to open direct RTSP for camera', cameraId, '- will restart using MediaMTX fallback');
        proc.kill();
      } catch (e) { console.error('[LicensePlate-Analytics] Failed to kill process before MediaMTX respawn:', e); }
      const mediaMtxUrl = `rtsp://127.0.0.1:8554/cam${cameraId}_0`;
      console.log('[Module: LicensePlate] Respawning runner for camera', cameraId, 'with MediaMTX URL due to open failure:', mediaMtxUrl);
      const newProc = spawn(python, [script, '--video', mediaMtxUrl, '--frame-skip', String(frameSkip), '--save-dir', configuredSave], { windowsHide: true, cwd: pythonSrcDir });
      analyticsProcesses.set(cameraId, newProc);
      newProc.stdout.on('data', async (data2) => {
        const lines = data2.toString().split('\n').filter(Boolean);
        for (const line of lines) { try { await handleAnalyticsLine(api, cameraId, line); } catch (e) { console.error(e); } }
      });
      newProc.stderr.on('data', (d2) => { const m = d2.toString().trim(); if (m) console.error('[LicensePlate-Analytics] stderr (fallback):', m); });
      newProc.on('exit', (c, s) => { console.log(`[LicensePlate-Analytics] Fallback process for camera ${cameraId} exit (code=${c})`); analyticsProcesses.delete(cameraId); });
    }
  });

  proc.on('exit', (code, signal) => {
    console.log(`[LicensePlate-Analytics] Exit event for camera ${cameraId} (code=${code}, signal=${signal})`);
  });

  proc.on('close', (code, signal) => {
    console.log(`[LicensePlate-Analytics] Process for camera ${cameraId} closed (code=${code}, signal=${signal})`);
    analyticsProcesses.delete(cameraId);
    currentStreamIds.delete(cameraId);
    // Auto-restart if not explicitly stopped and camera still exists
    if (code !== null || signal !== 'SIGTERM') {
      console.log(`[LicensePlate-Analytics] Auto-restarting process for camera ${cameraId}`);
      setTimeout(() => {
        // Check if camera still exists and analytics should be running
        api.getAppState().then(state => {
          const camera = (state.cameras || []).find(c => c.id === cameraId);
          if (camera && !analyticsProcesses.has(cameraId)) {
            spawnAnalyticsForCamera(api, camera);
          }
        }).catch(e => console.error('[LicensePlate-Analytics] Failed to check camera state for restart', e));
      }, 5000); // Delay restart by 5 seconds
    }
  });
}

function stopAnalyticsForCamera(cameraId) {
  const proc = analyticsProcesses.get(cameraId);
  if (!proc) return;
  try {
    console.log('[LicensePlate-Analytics] stopAnalyticsForCamera: killing process for camera', cameraId, 'stack:\n', new Error().stack);
    proc.kill();
  } catch (e) { console.error('[LicensePlate-Analytics] stopAnalyticsForCamera: failed to kill process', e); }
  analyticsProcesses.delete(cameraId);
  currentStreamIds.delete(cameraId);
}

async function activate(api) {
  console.log('[Module: LicensePlate] Activated.');
  
  // Register IPC handler for getting detected plates
  api.registerIpcHandler('module-license-plate-get-detected', async () => {
    return getDetectedPlates();
  });
  
  // Register IPC handlers to start/stop recognition per-camera on demand.
  api.registerIpcHandler('module-license-plate-start', async (event, cameraId) => {
    try {
      const state = await api.getAppState();
      const camera = (state.cameras || []).find(c => c.id === cameraId);
      if (!camera) return { success: false, error: 'Camera not found' };
      await spawnAnalyticsForCamera(api, camera);
      return { success: true };
    } catch (e) {
      console.error('[Module: LicensePlate] Failed to start analytics for camera', cameraId, e);
      return { success: false, error: e && e.message ? e.message : String(e) };
    }
  });

  api.registerIpcHandler('module-license-plate-stop', async (event, cameraId) => {
    try {
      stopAnalyticsForCamera(cameraId);
      return { success: true };
    } catch (e) {
      console.error('[Module: LicensePlate] Failed to stop analytics for camera', cameraId, e);
      return { success: false, error: e && e.message ? e.message : String(e) };
    }
  });

  // Diagnostic: test whether ONNX Runtime DirectML is available by running a short python probe
  api.registerIpcHandler('module-license-plate-test-ort', async () => {
    try {
      const repoRoot = path.join(__dirname, '..', '..');
      const python = findPythonExecutable();
      const probeScript = path.join(repoRoot, 'python_src', 'probe_ort_providers.py');
      const fsSync = require('fs');
      if (!fsSync.existsSync(python)) return { success: false, error: 'python-not-found' };
      if (!fsSync.existsSync(probeScript)) return { success: false, error: 'probe-script-missing' };

      return await new Promise((resolve) => {
        const proc = require('child_process').spawn(python, [probeScript], { windowsHide: true, cwd: path.join(repoRoot, 'python_src') });
        let out = '';
        let err = '';
        proc.stdout.on('data', (d) => { out += d.toString(); });
        proc.stderr.on('data', (d) => { err += d.toString(); });
        proc.on('close', (code) => {
          if (code === 0) {
            try {
              const parsed = JSON.parse(out.trim());
              resolve({ success: true, data: parsed });
            } catch (e) {
              resolve({ success: false, error: 'invalid-probe-output', raw: out, stderr: err });
            }
          } else {
            resolve({ success: false, error: 'probe-failed', code, raw: out, stderr: err });
          }
        });
      });
    } catch (e) {
      return { success: false, error: e && e.message ? e.message : String(e) };
    }
  });

  // Periodically check if streamId changed for running processes and restart if needed
  setInterval(async () => {
    for (const [cameraId, proc] of analyticsProcesses.entries()) {
      const currentStreamId = await getCurrentStreamIdForCamera(api, parseInt(cameraId));
      const storedStreamId = currentStreamIds.get(parseInt(cameraId));
      if (currentStreamId !== storedStreamId) {
        console.log(`[Module: LicensePlate] Stream changed for camera ${cameraId} from ${storedStreamId} to ${currentStreamId}, restarting process`);
        stopAnalyticsForCamera(parseInt(cameraId));
        // Restart will happen via auto-restart if process closes, but to be safe, spawn new one
        api.getAppState().then(state => {
          const camera = (state.cameras || []).find(c => c.id === parseInt(cameraId));
          if (camera) {
            spawnAnalyticsForCamera(api, camera);
          }
        }).catch(e => console.error('[Module: LicensePlate] Failed to restart on stream change', e));
      }
    }
  }, 5000); // Check every 5 seconds
}

async function deactivate(api) {
  console.log('[Module: LicensePlate] Deactivated.');
  
  // Save plates data before deactivation
  await savePlatesToFile(api);
  
  for (const cameraId of Array.from(analyticsProcesses.keys())) stopAnalyticsForCamera(cameraId);
  api.sendToRenderer('module-license-plate-cleanup');
}

async function getDetectedPlates() {
  console.log('[Module: LicensePlate] getDetectedPlates called');
  console.log('[Module: LicensePlate] detectedPlates:', Array.from(detectedPlates.entries()));
  console.log('[Module: LicensePlate] platesHistory length:', platesHistory.length);
  
  const result = {
    totalDetections: platesHistory.length,
    uniquePlates: Array.from(detectedPlates.entries()).map(([cameraId, plates]) => ({
      cameraId: parseInt(cameraId),
      plates: Array.from(plates),
      count: plates.size
    })),
    recentHistory: platesHistory.slice(-50) // Last 50 detections
  };
  
  console.log('[Module: LicensePlate] Returning data:', JSON.stringify(result, null, 2));
  return result;
}

module.exports = { activate, deactivate, spawnAnalyticsForCamera, getDetectedPlates };
