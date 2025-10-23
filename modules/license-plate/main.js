const fs = require('fs/promises');
const fsSync = require('fs');
const path = require('path');
const { shell } = require('electron');
let sharp = null;
try {
  sharp = require('sharp');
} catch (e) {
  console.warn('[Module: LicensePlate] Optional dependency "sharp" is not installed; falling back to copying full frames. Install via `npm install sharp` to enable cropped plates.');
}
const { spawn } = require('child_process');
const runtimeManager = require('./runtime-manager');
// Use central auth-manager for reliable password lookup
// Map cameraId -> child process
const analyticsProcesses = new Map();
const currentStreamIds = new Map();
const SAVE_COOLDOWN_MS = 5000;
const lastSaveTimestamps = {};

// Storage for detected license plates
const detectedPlates = new Map(); // cameraId -> Set of detected texts
const platesHistory = []; // Array of all detections with timestamps
// Track cameras we have already logged forced HD overrides for (avoid noisy logs)
const forcedHdLog = new Set();
const plateSaveStats = new Map();
const DEFAULT_MAX_SAVED_CROPS_PER_PLATE = 10;
let maxSavedCropsPerPlate = DEFAULT_MAX_SAVED_CROPS_PER_PLATE;
const PLATE_SAVE_RESET_MS = 30 * 60 * 1000;
const PLATE_SKIP_LOG_THROTTLE_MS = 60 * 1000;
const DEFAULT_ALLOWLIST = 'АВЕКМНОРСТУХABEKMHOPCTYX0123456789';
const RUNNER_EVENT_CHANNEL = 'module-license-plate-runner-event';
const MAX_RUNNER_MESSAGE_LENGTH = 800;
let moduleApiRef = null;

function formatPathTailForLog(input) {
  if (!input || typeof input !== 'string') return '';
  const normalized = input.replace(/\\+/g, '/');
  const parts = normalized.split('/').filter(Boolean);
  if (parts.length <= 3) return normalized;
  return `.../${parts.slice(-3).join('/')}`;
}

function maskRtspCredentials(url) {
  if (!url || typeof url !== 'string') return url;
  try {
    const schemeIdx = url.indexOf('://');
    if (schemeIdx === -1) return url;
    const authStart = schemeIdx + 3;
    const atIdx = url.indexOf('@', authStart);
    if (atIdx === -1) return url;
    const colonIdx = url.indexOf(':', authStart);
    if (colonIdx === -1 || colonIdx > atIdx) return url;
    return `${url.slice(0, colonIdx + 1)}****${url.slice(atIdx)}`;
  } catch (e) {
    return url;
  }
}

function emitRunnerEvent(api, cameraId, payload = {}) {
  if (!api || typeof api.sendToRenderer !== 'function') {
    return;
  }
  const event = {
    cameraId,
    timestamp: new Date().toISOString(),
    ...payload
  };
  if (typeof event.message === 'string') {
    event.message = event.message.trim();
    if (event.message.length > MAX_RUNNER_MESSAGE_LENGTH) {
      event.message = `${event.message.slice(0, MAX_RUNNER_MESSAGE_LENGTH)}…`;
    }
  }
  try {
    api.sendToRenderer(RUNNER_EVENT_CHANNEL, event);
  } catch (err) {
    console.warn('[Module: LicensePlate] Failed to emit runner event', err);
  }
}

function clearPlateStatsForCamera(cameraId) {
  const prefix = `${cameraId}|`;
  for (const key of Array.from(plateSaveStats.keys())) {
    if (key.startsWith(prefix)) {
      plateSaveStats.delete(key);
    }
  }
}

function buildMediaMtxUrl(cameraId, streamId = 0) {
  return `rtsp://127.0.0.1:8554/cam${cameraId}_${streamId}`;
}

function replaceVideoArg(args, videoUrl) {
  const idx = args.indexOf('--video');
  if (idx !== -1 && idx + 1 < args.length) {
    const updated = [...args];
    updated[idx + 1] = videoUrl;
    return updated;
  }
  return ['--video', videoUrl, ...args];
}

function applyPlatePersistenceSettings(settings) {
  if (!settings) return;
  const rawLimit = parseInt(
    settings.plate_max_crops_per_plate ?? settings['module_license-plate_max_crops'],
    10
  );
  const sanitizedLimit = Number.isFinite(rawLimit) && rawLimit > 0 ? rawLimit : DEFAULT_MAX_SAVED_CROPS_PER_PLATE;
  if (sanitizedLimit !== maxSavedCropsPerPlate) {
    maxSavedCropsPerPlate = sanitizedLimit;
    plateSaveStats.clear();
    console.log('[Module: LicensePlate] Updated max saved crops per plate to', maxSavedCropsPerPlate);
  }
}

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

async function getCurrentStreamIdForCamera(api, cameraId) {
  let preferredStreamId = 0;
  try {
    const state = await api.getAppState();
    const cameras = Array.isArray(state && state.cameras) ? state.cameras : [];
    const camera = cameras.find(c => c && c.id === cameraId);

    if (camera) {
      const analyticsPreferred = camera.analyticsConfig ? camera.analyticsConfig.preferredStreamId : undefined;
      const parsedAnalyticsPreferred = analyticsPreferred !== undefined && analyticsPreferred !== null ? Number(analyticsPreferred) : NaN;
      if (Number.isInteger(parsedAnalyticsPreferred) && parsedAnalyticsPreferred >= 0) {
        preferredStreamId = parsedAnalyticsPreferred;
      }
    }

    if (preferredStreamId === 0 && camera) {
      const hasExplicitStream0 = Object.prototype.hasOwnProperty.call(camera, 'streamPath0') && camera.streamPath0 != null && camera.streamPath0 !== '';
      const hasExplicitStream1 = Object.prototype.hasOwnProperty.call(camera, 'streamPath1') && camera.streamPath1 != null && camera.streamPath1 !== '';
      if (!hasExplicitStream0 && hasExplicitStream1) {
        preferredStreamId = 1;
      }
    }

    const layouts = Array.isArray(state && state.layouts) ? state.layouts : [];
    const activeLayoutId = state ? state.activeLayoutId : undefined;
    const activeLayout = layouts.find(l => l && l.id === activeLayoutId);
    const gridState = activeLayout && Array.isArray(activeLayout.gridState) ? activeLayout.gridState : null;
    if (gridState) {
      const cell = gridState.find(item => item && item.camera && item.camera.id === cameraId);
      if (cell && typeof cell.streamId === 'number') {
        const layoutStreamId = cell.streamId;
        if (layoutStreamId === preferredStreamId) {
          return layoutStreamId;
        }
        if (preferredStreamId === 0) {
          if (!forcedHdLog.has(cameraId)) {
            console.log('[Module: LicensePlate] Forcing HD stream for analytics on camera', cameraId, '(UI stream:', layoutStreamId, ')');
            forcedHdLog.add(cameraId);
          }
          return 0;
        }
        return preferredStreamId;
      }
    }

    return preferredStreamId;
  } catch (e) {
    console.error('[Module: LicensePlate] Failed to get current streamId for camera', cameraId, e);
  }
  return preferredStreamId;
}

function shouldPersistPlate(cameraId, plateText) {
  if (!plateText) return true;
  const key = `${cameraId}|${plateText}`;
  const now = Date.now();
  const cached = plateSaveStats.get(key);
  const limit = Math.max(1, maxSavedCropsPerPlate);

  if (cached && (now - cached.firstSeen) > PLATE_SAVE_RESET_MS) {
    plateSaveStats.delete(key);
  }

  const entry = plateSaveStats.get(key);
  if (entry) {
    if (entry.count >= limit) {
      if (!entry.lastLog || (now - entry.lastLog) > PLATE_SKIP_LOG_THROTTLE_MS) {
        console.log('[Module: LicensePlate] Skip saving plate crop: limit reached', { cameraId, plateText, max: limit });
        entry.lastLog = now;
      }
      return false;
    }
    entry.count += 1;
    entry.lastSeen = now;
    return true;
  }

  plateSaveStats.set(key, { count: 1, firstSeen: now, lastSeen: now, lastLog: 0 });
  return true;
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

async function handleAnalyticsLine(api, cameraId, line, contextTag = 'primary') {
  const trimmedLine = typeof line === 'string' ? line.trim() : '';
  if (!trimmedLine) return;
  let result;
  try {
    result = JSON.parse(trimmedLine);
  } catch (e) {
    emitRunnerEvent(api, cameraId, { type: 'stdout', message: trimmedLine, context: contextTag });
    return;
  }
  console.log('[Module: LicensePlate] Incoming analytics line:', line);
  // If runner provided recognized plates (runner saved crops and returned paths), forward them
  if (result.recognized && Array.isArray(result.recognized) && result.recognized.length > 0) {
    const now = Date.now();
    const last = lastSaveTimestamps[cameraId] || 0;
    if (now - last < SAVE_COOLDOWN_MS) {
      try { if (result.frame_path) await fs.unlink(result.frame_path); } catch (e) {}
      return;
    }
    let savedAnyThisBatch = false;
    for (const r of result.recognized) {
      try {
        const plateTextRaw = r.text || '';
        const plateText = plateTextRaw.trim();
        console.log('[Module: LicensePlate] Processing plate text:', plateTextRaw);
        if (!plateText) {
          continue;
        }
        if (!shouldPersistPlate(cameraId, plateText)) {
          try { if (r.path) await fs.unlink(r.path); } catch (delErr) { console.error('[Module: LicensePlate] Failed to delete skipped plate crop', delErr); }
          continue;
        }

        if (!detectedPlates.has(cameraId.toString())) {
          detectedPlates.set(cameraId.toString(), new Set());
        }
        detectedPlates.get(cameraId.toString()).add(plateText);

        const detectionTimestamp = r.timestamp || new Date().toISOString();
        platesHistory.push({
          cameraId,
          text: plateText,
          score: r.score,
          timestamp: detectionTimestamp,
          path: r.path
        });

        console.log('[Module: LicensePlate] Added plate to history. Total history:', platesHistory.length);

        if (platesHistory.length % 10 === 0 || platesHistory.length < 5) {
          console.log('[Module: LicensePlate] Saving plates to file...');
          savePlatesToFile(api);
        }

        api.sendToRenderer('module-license-plate-saved', { cameraId, path: r.path, text: plateText, score: r.score, timestamp: detectionTimestamp });
        console.log('[Module: LicensePlate] Recognized and saved', r.path, r.text || 'no text', `score: ${r.score}`);
        savedAnyThisBatch = true;
      } catch (e) {
        console.error('[Module: LicensePlate] Failed to handle recognized plate result', e);
      }
    }
    try {
      const summaryTexts = result.recognized
        .map(rec => (rec && rec.text ? String(rec.text).trim() : ''))
        .filter(Boolean);
      emitRunnerEvent(api, cameraId, {
        type: 'recognized',
        message: summaryTexts.length
          ? `Recognized ${summaryTexts.length} plate(s): ${summaryTexts.join(', ')}`
          : `Recognized ${result.recognized.length} plate(s)`,
        context: contextTag
      });
    } catch (emitErr) {
      console.warn('[Module: LicensePlate] Failed to emit recognized summary event', emitErr);
    }
    if (savedAnyThisBatch) {
      lastSaveTimestamps[cameraId] = now;
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
    const p = plates[0];
    const { x, y, w, h } = p.box;
    if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > result.frame_width || y + h > result.frame_height) {
      try { await fs.unlink(result.frame_path); } catch (e) {}
      return;
    }

    const timestamp = new Date().toISOString().replace(/:/g,'-').slice(0,-5);
    const suffix = Math.random().toString(36).substring(2,7);
    const filename = `plate_${cameraId}_${timestamp}_${suffix}.jpg`;
    const fp = path.join(saveDir, filename);

    if (sharp) {
      const image = sharp(result.frame_path);
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
      await fs.writeFile(fp, buf);
    } else {
      await fs.copyFile(result.frame_path, fp);
    }

    lastSaveTimestamps[cameraId] = now;

    api.sendToRenderer('module-license-plate-saved', { cameraId, path: fp, timestamp: new Date().toISOString() });
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
  let runtimeInfo;
  try {
    runtimeInfo = await runtimeManager.ensureRuntimeReady(api);
  } catch (err) {
    console.error('[LicensePlate-Analytics] Failed to prepare runtime for camera', cameraId, err);
    emitRunnerEvent(api, cameraId, {
      type: 'runtime-error',
      message: err && err.message ? err.message : String(err),
      context: 'primary'
    });
    api.sendToRenderer('module-license-plate-runtime-error', { cameraId, message: err.message || String(err) });
    return;
  }
  emitRunnerEvent(api, cameraId, {
    type: 'runtime-ready',
    message: `Runtime ready (${runtimeInfo.mode || 'unknown'})`,
    mode: runtimeInfo.mode,
    pythonPath: runtimeInfo.pythonPath,
    version: runtimeInfo.version,
    context: 'primary'
  });
  const python = runtimeInfo.pythonPath;
  const script = runtimeManager.resolvePythonScript('test_plate_yunet.py', runtimeInfo);
  const pythonSrcDir = runtimeInfo.scriptRoot || path.dirname(script);
  console.log('[Module: LicensePlate] Using runtime mode:', runtimeInfo.mode, 'python:', python);
  if (!fsSync.existsSync(python)) {
    console.error(`[LicensePlate-Analytics] Python executable not found: ${python}`);
    emitRunnerEvent(api, cameraId, {
      type: 'runtime-error',
      message: 'Python executable not found',
      pythonPath: python,
      context: 'primary'
    });
    api.sendToRenderer('module-license-plate-runtime-error', {
      cameraId,
      message: 'python-not-found',
      path: python
    });
    return;
  }
  if (!fsSync.existsSync(script)) {
    console.error(`[LicensePlate-Analytics] Script not found: ${script}`);
    emitRunnerEvent(api, cameraId, {
      type: 'runtime-error',
      message: 'Runner script not found',
      scriptPath: script,
      context: 'primary'
    });
    api.sendToRenderer('module-license-plate-runtime-error', {
      cameraId,
      message: 'script-not-found',
      path: script
    });
    return;
  }
  if (!fsSync.existsSync(pythonSrcDir)) {
    console.error(`[LicensePlate-Analytics] Script root not found: ${pythonSrcDir}`);
    emitRunnerEvent(api, cameraId, {
      type: 'runtime-error',
      message: 'Script root not found',
      scriptRoot: pythonSrcDir,
      context: 'primary'
    });
    api.sendToRenderer('module-license-plate-runtime-error', {
      cameraId,
      message: 'script-root-not-found',
      path: pythonSrcDir
    });
    return;
  }

  const settings = await api.configManager.getAppSettings();
  applyPlatePersistenceSettings(settings);
  const moduleFolderName = path.basename(__dirname);
  const savePathKey = `module_${moduleFolderName}_savePath`;
  const configuredSave = settings[savePathKey] || path.join(api.configManager.getDataPath(), 'plates');
  await ensureDir(configuredSave);

  const registerLifecycleHandlers = (childProc, { tag = 'primary' } = {}) => {
    childProc.once('spawn', () => {
      emitRunnerEvent(api, cameraId, {
        type: 'spawned',
        message: `Runner started (pid=${childProc.pid || 'unknown'}) [${tag}]`,
        pid: childProc.pid,
        context: tag
      });
    });

    childProc.on('exit', (code, signal) => {
      console.log(`[LicensePlate-Analytics] Exit event for camera ${cameraId} (code=${code}, signal=${signal})`);
      emitRunnerEvent(api, cameraId, {
        type: 'exit',
        message: `Runner exited (code=${code}, signal=${signal || 'none'}) [${tag}]`,
        code,
        signal,
        context: tag
      });
    });

    childProc.on('close', (code, signal) => {
      console.log(`[LicensePlate-Analytics] Process for camera ${cameraId} closed (code=${code}, signal=${signal})`);
      emitRunnerEvent(api, cameraId, {
        type: 'close',
        message: `Runner closed (code=${code}, signal=${signal || 'none'}) [${tag}]`,
        code,
        signal,
        context: tag
      });
      analyticsProcesses.delete(cameraId);
      currentStreamIds.delete(cameraId);
      forcedHdLog.delete(cameraId);
      clearPlateStatsForCamera(cameraId);
      if (code !== null || signal !== 'SIGTERM') {
        console.log(`[LicensePlate-Analytics] Auto-restarting process for camera ${cameraId}`);
        setTimeout(() => {
          api.getAppState().then(state => {
            const cameraState = (state.cameras || []).find(c => c.id === cameraId);
            if (cameraState && !analyticsProcesses.has(cameraId)) {
              spawnAnalyticsForCamera(api, cameraState);
            }
          }).catch(e => console.error('[LicensePlate-Analytics] Failed to check camera state for restart', e));
        }, 5000);
      }
    });
  };

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
  const allowlist = settings.plate_allowlist || DEFAULT_ALLOWLIST;

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
  const useOrtEnabled = args.includes('--use-ort');
  console.log('[LicensePlate-Analytics] Using cwd for runner:', pythonSrcDir);
  const spawnEnv = {
    ...process.env,
    PYTHONIOENCODING: 'utf-8',
    PYTHONUTF8: '1'
  };
  // Remove host Python launch hints so bundled interpreter resolves correctly
  delete spawnEnv.__PYVENV_LAUNCHER__;
  delete spawnEnv.PYTHONHOME;
  delete spawnEnv.PYTHONPATH;
  delete spawnEnv.PYTHONEXECUTABLE;
  delete spawnEnv.PYENV_ROOT;
  delete spawnEnv.PYENV;
  delete spawnEnv.VIRTUAL_ENV;
  const spawnOptions = { windowsHide: true, cwd: pythonSrcDir, env: spawnEnv };
  const sanitizedRtspUrl = maskRtspCredentials(rtspUrl);
  emitRunnerEvent(api, cameraId, {
    type: 'launch',
    message: `Launching runner (stream ${streamId}) in ${runtimeInfo.mode || 'unknown'} mode`,
    pythonPath: python,
    scriptPath: script,
    videoSource: sanitizedRtspUrl,
    frameSkip,
    resizeWidth,
    useOrt: useOrtEnabled,
    saveDir: configuredSave,
    config: configForScript,
    mode: runtimeInfo.mode,
    context: 'primary'
  });
  let proc;
  try {
    proc = spawn(python, args, spawnOptions);
  } catch (err) {
    console.error('[LicensePlate-Analytics] Synchronous spawn error for camera', cameraId, err);
    emitRunnerEvent(api, cameraId, {
      type: 'error',
      message: `Failed to spawn runner: ${err && err.message ? err.message : err}`,
      pythonPath: python,
      scriptPath: script
    });
    return;
  }
  proc.on('error', (err) => {
    console.error('[LicensePlate-Analytics] Failed to spawn process for camera', cameraId, err);
    emitRunnerEvent(api, cameraId, {
      type: 'error',
      message: `Runner process error: ${err && err.message ? err.message : err}`,
      context: 'primary'
    });
  });
  analyticsProcesses.set(cameraId, proc);
  registerLifecycleHandlers(proc, { tag: 'primary' });

  proc.stdout.on('data', async (data) => {
    const lines = data.toString().split('\n').filter(Boolean);
    for (const line of lines) {
      try { await handleAnalyticsLine(api, cameraId, line, 'primary'); } catch (e) { console.error(e); }
    }
  });

  proc.stderr.on('data', (data) => {
    const msg = data.toString().trim();
    if (msg.length > 0) {
      console.error('[LicensePlate-Analytics] stderr:', msg);
      emitRunnerEvent(api, cameraId, {
        type: 'stderr',
        message: msg,
        context: 'primary'
      });
    } else {
      console.error('[LicensePlate-Analytics] stderr: <empty>');
      emitRunnerEvent(api, cameraId, {
        type: 'stderr',
        message: '<empty>',
        context: 'primary'
      });
    }
  });

  // Watch for RTSP auth errors (401) in stderr and restart using MediaMTX fallback if needed
  let sawUnauthorized = false;
  proc.stderr.on('data', (d) => {
    const s = d.toString();
    if (sawUnauthorized) return;
    if (s.includes('401 Unauthorized') || s.toLowerCase().includes('method describe failed: 401')) {
      sawUnauthorized = true;
      console.warn('[LicensePlate-Analytics] Detected RTSP 401 Unauthorized for camera', cameraId, '- will restart using MediaMTX fallback');
      emitRunnerEvent(api, cameraId, {
        type: 'fallback',
        message: 'RTSP 401 Unauthorized detected. Switching to MediaMTX.',
        context: 'primary'
      });
      try {
        console.warn('[LicensePlate-Analytics] About to kill process for camera', cameraId, 'due to RTSP 401; stack:\n', new Error().stack);
        proc.kill();
      } catch (e) { console.error('[LicensePlate-Analytics] Failed to kill process after 401:', e); }
      // respawn with MediaMTX URL
      const desiredStreamId = currentStreamIds.get(cameraId) ?? streamId;
      const mediaMtxUrl = buildMediaMtxUrl(cameraId, desiredStreamId);
      console.log('[Module: LicensePlate] Respawning runner for camera', cameraId, 'with MediaMTX URL:', mediaMtxUrl);
      const fallbackArgs = replaceVideoArg(args, mediaMtxUrl);
      emitRunnerEvent(api, cameraId, {
        type: 'fallback',
        message: 'Respawning runner with MediaMTX after 401 Unauthorized',
        context: 'fallback-401',
        videoSource: maskRtspCredentials(mediaMtxUrl)
      });
      let newProc;
      try {
        newProc = spawn(python, fallbackArgs, spawnOptions);
      } catch (spawnErr) {
        console.error('[LicensePlate-Analytics] Failed to spawn fallback process after 401:', spawnErr);
        emitRunnerEvent(api, cameraId, {
          type: 'error',
          message: `Failed to spawn fallback runner: ${spawnErr && spawnErr.message ? spawnErr.message : spawnErr}`,
          context: 'fallback-401'
        });
        return;
      }
      analyticsProcesses.set(cameraId, newProc);
      currentStreamIds.set(cameraId, desiredStreamId);
      registerLifecycleHandlers(newProc, { tag: 'fallback-401' });
      newProc.on('error', (err) => {
        console.error('[LicensePlate-Analytics] Fallback process error (401) for camera', cameraId, err);
        emitRunnerEvent(api, cameraId, {
          type: 'error',
          message: `Fallback runner error (401): ${err && err.message ? err.message : err}`,
          context: 'fallback-401'
        });
      });
      // wire up handlers for the new process (reuse existing handlers lightly)
      newProc.stdout.on('data', async (data2) => {
        const lines = data2.toString().split('\n').filter(Boolean);
        for (const line of lines) { try { await handleAnalyticsLine(api, cameraId, line, 'fallback-401'); } catch (e) { console.error(e); } }
      });
      newProc.stderr.on('data', (d2) => {
        const m = d2.toString().trim();
        if (m) {
          console.error('[LicensePlate-Analytics] stderr (fallback):', m);
          emitRunnerEvent(api, cameraId, {
            type: 'stderr',
            message: m,
            context: 'fallback-401'
          });
        }
      });
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
        emitRunnerEvent(api, cameraId, {
          type: 'fallback',
          message: 'Direct RTSP failed to open. Switching to MediaMTX.',
          context: 'primary'
        });
        proc.kill();
      } catch (e) { console.error('[LicensePlate-Analytics] Failed to kill process before MediaMTX respawn:', e); }
      const desiredStreamId = currentStreamIds.get(cameraId) ?? streamId;
      const mediaMtxUrl = buildMediaMtxUrl(cameraId, desiredStreamId);
      console.log('[Module: LicensePlate] Respawning runner for camera', cameraId, 'with MediaMTX URL due to open failure:', mediaMtxUrl);
      const fallbackArgs = replaceVideoArg(args, mediaMtxUrl);
      emitRunnerEvent(api, cameraId, {
        type: 'fallback',
        message: 'Respawning runner with MediaMTX after open failure',
        context: 'fallback-open',
        videoSource: maskRtspCredentials(mediaMtxUrl)
      });
      let newProc;
      try {
        newProc = spawn(python, fallbackArgs, spawnOptions);
      } catch (spawnErr) {
        console.error('[LicensePlate-Analytics] Failed to spawn fallback process after open failure:', spawnErr);
        emitRunnerEvent(api, cameraId, {
          type: 'error',
          message: `Failed to spawn fallback runner: ${spawnErr && spawnErr.message ? spawnErr.message : spawnErr}`,
          context: 'fallback-open'
        });
        return;
      }
      analyticsProcesses.set(cameraId, newProc);
      currentStreamIds.set(cameraId, desiredStreamId);
      registerLifecycleHandlers(newProc, { tag: 'fallback-open' });
      newProc.on('error', (err) => {
        console.error('[LicensePlate-Analytics] Fallback process error (open failure) for camera', cameraId, err);
        emitRunnerEvent(api, cameraId, {
          type: 'error',
          message: `Fallback runner error (open failure): ${err && err.message ? err.message : err}`,
          context: 'fallback-open'
        });
      });
      newProc.stdout.on('data', async (data2) => {
        const lines = data2.toString().split('\n').filter(Boolean);
        for (const line of lines) { try { await handleAnalyticsLine(api, cameraId, line, 'fallback-open'); } catch (e) { console.error(e); } }
      });
      newProc.stderr.on('data', (d2) => {
        const m = d2.toString().trim();
        if (m) {
          console.error('[LicensePlate-Analytics] stderr (fallback):', m);
          emitRunnerEvent(api, cameraId, {
            type: 'stderr',
            message: m,
            context: 'fallback-open'
          });
        }
      });
    }
  });
}

function stopAnalyticsForCamera(cameraId) {
  const proc = analyticsProcesses.get(cameraId);
  if (!proc) return;
  if (moduleApiRef) {
    emitRunnerEvent(moduleApiRef, cameraId, {
      type: 'stop',
      message: 'Stopping runner for camera',
      context: 'primary'
    });
  }
  try {
    console.log('[LicensePlate-Analytics] stopAnalyticsForCamera: killing process for camera', cameraId, 'stack:\n', new Error().stack);
    proc.kill();
  } catch (e) { console.error('[LicensePlate-Analytics] stopAnalyticsForCamera: failed to kill process', e); }
  analyticsProcesses.delete(cameraId);
  currentStreamIds.delete(cameraId);
  forcedHdLog.delete(cameraId);
  clearPlateStatsForCamera(cameraId);
}

async function activate(api) {
  console.log('[Module: LicensePlate] Activated.');
  moduleApiRef = api;
  emitRunnerEvent(api, undefined, {
    type: 'info',
    message: 'License plate module activating',
    context: 'module'
  });

  try {
    const initialSettings = await api.configManager.getAppSettings();
    applyPlatePersistenceSettings(initialSettings);
  } catch (e) {
    console.warn('[Module: LicensePlate] Failed to read initial settings for plate persistence', e);
  }

  runtimeManager.ensureRuntimeReady(api).catch((err) => {
    console.error('[Module: LicensePlate] Failed to prepare runtime during activation', err);
    emitRunnerEvent(api, undefined, {
      type: 'runtime-error',
      message: err && err.message ? err.message : String(err),
      context: 'activation'
    });
    api.sendToRenderer('module-license-plate-runtime-error', {
      message: err && err.message ? err.message : String(err)
    });
  });
  
  // Register IPC handler for getting detected plates
  api.registerIpcHandler('module-license-plate-get-detected', async () => {
    return getDetectedPlates();
  });

  api.registerIpcHandler('module-license-plate-open-path', async (event, payload) => {
    try {
      const input = payload && typeof payload === 'object' ? payload : { path: payload };
      const rawPath = input && typeof input.path === 'string' ? input.path.trim() : '';
      const action = input && typeof input.action === 'string' ? input.action : 'open';
      if (!rawPath) {
        return { success: false, error: 'missing-path' };
      }

      const resolvedPath = path.resolve(rawPath);
      let stats;
      try {
        stats = await fs.stat(resolvedPath);
      } catch (e) {
        return { success: false, error: 'not-found' };
      }

      if (action === 'reveal' && stats && stats.isFile()) {
        shell.showItemInFolder(resolvedPath);
        return { success: true };
      }

      const target = stats && stats.isDirectory() ? resolvedPath : resolvedPath;
      const errorMessage = await shell.openPath(target);
      if (typeof errorMessage === 'string' && errorMessage.trim()) {
        console.error('[Module: LicensePlate] shell.openPath error:', errorMessage);
        return { success: false, error: errorMessage };
      }
      return { success: true };
    } catch (e) {
      console.error('[Module: LicensePlate] Failed to open requested plate path', e);
      return { success: false, error: e && e.message ? e.message : String(e) };
    }
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

  api.registerIpcHandler('module-license-plate-runtime-status', async () => {
    try {
      const status = await runtimeManager.getRuntimeStatus();
      return { success: true, data: status };
    } catch (e) {
      console.error('[Module: LicensePlate] Failed to get runtime status', e);
      return { success: false, error: e && e.message ? e.message : String(e) };
    }
  });

  api.registerIpcHandler('module-license-plate-runtime-reinstall', async () => {
    try {
      const info = await runtimeManager.reinstallRuntime(api);
      return { success: true, data: info };
    } catch (e) {
      console.error('[Module: LicensePlate] Failed to reinstall runtime', e);
      return { success: false, error: e && e.message ? e.message : String(e) };
    }
  });

  // Diagnostic: test whether ONNX Runtime DirectML is available by running a short python probe
  api.registerIpcHandler('module-license-plate-test-ort', async () => {
    try {
      const runtimeInfo = await runtimeManager.ensureRuntimeReady(api);
      const python = runtimeInfo.pythonPath;
      const probeScript = runtimeManager.resolvePythonScript('probe_ort_providers.py', runtimeInfo);
      const pythonSrcDir = runtimeInfo.scriptRoot || path.dirname(probeScript);
      if (!fsSync.existsSync(python)) return { success: false, error: 'python-not-found' };
      if (!fsSync.existsSync(probeScript)) return { success: false, error: 'probe-script-missing' };

      return await new Promise((resolve) => {
        const proc = spawn(python, [probeScript], {
          windowsHide: true,
          cwd: pythonSrcDir,
          env: {
            ...process.env,
            PYTHONIOENCODING: 'utf-8',
            PYTHONUTF8: '1'
          }
        });
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
    try {
      const refreshedSettings = await api.configManager.getAppSettings();
      applyPlatePersistenceSettings(refreshedSettings);
    } catch (e) {
      console.warn('[Module: LicensePlate] Failed to refresh plate persistence settings', e);
    }
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
  moduleApiRef = api;

  emitRunnerEvent(api, undefined, {
    type: 'info',
    message: 'License plate module deactivating',
    context: 'module'
  });
  
  // Save plates data before deactivation
  await savePlatesToFile(api);
  
  for (const cameraId of Array.from(analyticsProcesses.keys())) stopAnalyticsForCamera(cameraId);
  plateSaveStats.clear();
  forcedHdLog.clear();
  api.sendToRenderer('module-license-plate-cleanup');
  moduleApiRef = null;
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
