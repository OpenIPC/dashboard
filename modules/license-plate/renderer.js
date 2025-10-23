// Simple renderer for license-plate module

// Renderer runs in browser context; use exposed `window.api` instead of require

window.addEventListener('DOMContentLoaded', () => {
  // Debug: report runtime API availability
  try {
    console.debug('[LP Renderer] window.api type:', typeof window.api);
    console.debug('[LP Renderer] window.api keys:', Object.keys(window.api || {}));
    console.debug('[LP Renderer] window.api.invoke exists:', typeof (window.api && window.api.invoke) === 'function');
  } catch (e) {
    console.error('[LP Renderer] Error while inspecting window.api', e);
  }
  // Instead of creating a separate container, add content to the modules panel
  const modulesPanelContent = document.getElementById('detected-plates-list');
  if (modulesPanelContent) {
    modulesPanelContent.innerHTML = `
      <div id="runtime-status" style="margin-bottom:12px; font-size:13px; color:#444;">
        <div id="runtime-status-text">Preparing runtime...</div>
        <div id="runtime-progress-container" style="display:none; height:6px; background:#ddd; border-radius:3px; overflow:hidden; margin-top:6px;">
          <div id="runtime-progress-bar" style="height:100%; width:0%; background:#4caf50;"></div>
        </div>
        <div style="margin-top:8px;">
          <button id="runtime-reinstall-btn" style="padding:4px 8px; font-size:12px;" disabled>Reinstall Runtime</button>
        </div>
      </div>
      <div id="plates-stats" style="margin-bottom: 10px; font-size: 14px; color: #666;"></div>
      <div style="margin-bottom:10px;">
        <label style="font-size:13px; color:#333;"><input id="use-ort-checkbox" type="checkbox" style="margin-right:6px;">Use ONNX Runtime (DirectML)</label>
        <button id="ort-test-button" style="margin-left:10px; padding:4px 8px; font-size:12px;">Test DirectML</button>
        <span id="ort-test-result" style="margin-left:10px; font-size:12px; color:#444;"></span>
      </div>
      <div id="unique-plates" style="margin-bottom: 15px;">
        <h5>Unique Detected Plates:</h5>
        <div id="unique-plates-list" style="max-height: 200px; overflow-y: auto; border: 1px solid #ccc; padding: 5px;"></div>
      </div>
      <div id="recent-detections">
        <h5>Recent Detections:</h5>
        <ul id="license-plate-list" style="max-height: 200px; overflow-y: auto;"></ul>
      </div>
      <div id="runner-events" style="margin-top:15px;">
        <h5>Runner Diagnostics:</h5>
        <div id="runner-events-log" style="max-height:200px; overflow-y:auto; border:1px solid #ccc; padding:5px; font-size:12px; background:#fafafa;"></div>
      </div>
    `;
  } else {
    // Fallback: create separate container if modules panel is not found
    const container = document.createElement('div');
    container.className = 'module-license-plate';
    container.innerHTML = `
      <h4>License Plate Module</h4>
      <div id="runtime-status" style="margin-bottom:12px; font-size:13px; color:#444;">
        <div id="runtime-status-text">Preparing runtime...</div>
        <div id="runtime-progress-container" style="display:none; height:6px; background:#ddd; border-radius:3px; overflow:hidden; margin-top:6px;">
          <div id="runtime-progress-bar" style="height:100%; width:0%; background:#4caf50;"></div>
        </div>
        <div style="margin-top:8px;">
          <button id="runtime-reinstall-btn" style="padding:4px 8px; font-size:12px;" disabled>Reinstall Runtime</button>
        </div>
      </div>
      <div id="plates-stats" style="margin-bottom: 10px; font-size: 14px; color: #666;"></div>
      <div style="margin-bottom:10px;">
        <label style="font-size:13px; color:#333;"><input id="use-ort-checkbox" type="checkbox" style="margin-right:6px;">Use ONNX Runtime (DirectML)</label>
      </div>
      <div id="unique-plates" style="margin-bottom: 15px;">
        <h5>Unique Detected Plates:</h5>
        <div id="unique-plates-list" style="max-height: 200px; overflow-y: auto; border: 1px solid #ccc; padding: 5px;"></div>
      </div>
      <div id="recent-detections">
        <h5>Recent Detections:</h5>
        <ul id="license-plate-list" style="max-height: 200px; overflow-y: auto;"></ul>
      </div>
      <div id="runner-events" style="margin-top:15px;">
        <h5>Runner Diagnostics:</h5>
        <div id="runner-events-log" style="max-height:200px; overflow-y:auto; border:1px solid #ccc; padding:5px; font-size:12px; background:#fafafa;"></div>
      </div>
    `;
    document.body.appendChild(container);
  }

  const runtimeUi = {
    container: document.getElementById('runtime-status'),
    text: document.getElementById('runtime-status-text'),
    progressContainer: document.getElementById('runtime-progress-container'),
    progressBar: document.getElementById('runtime-progress-bar'),
    reinstallButton: document.getElementById('runtime-reinstall-btn')
  };

  const runnerEventsUi = {
    log: document.getElementById('runner-events-log')
  };

  const RUNNER_EVENT_LIMIT = 120;

  function setRuntimeStatus(message, options = {}) {
    const {
      progress = null,
      showProgress = progress !== null,
      busy = false,
      tone = 'info',
      allowReinstall = true
    } = options;
    if (runtimeUi.text) {
      runtimeUi.text.textContent = message;
      runtimeUi.text.style.color = tone === 'error' ? '#c0392b' : '#444';
    }
    if (runtimeUi.progressContainer && runtimeUi.progressBar) {
      if (showProgress) {
        runtimeUi.progressContainer.style.display = 'block';
        const width = progress === null ? 0 : Math.round(Math.max(0, Math.min(1, progress)) * 100);
        runtimeUi.progressBar.style.width = `${width}%`;
      } else {
        runtimeUi.progressContainer.style.display = 'none';
        runtimeUi.progressBar.style.width = '0%';
      }
    }
    if (runtimeUi.reinstallButton) {
      runtimeUi.reinstallButton.disabled = !!busy || !allowReinstall;
    }
  }

  function formatBytes(bytes) {
    if (!Number.isFinite(bytes) || bytes <= 0) return '';
    const units = ['B', 'KB', 'MB', 'GB', 'TB'];
    let value = bytes;
    let unitIndex = 0;
    while (value >= 1024 && unitIndex < units.length - 1) {
      value /= 1024;
      unitIndex += 1;
    }
    const fixed = value >= 10 || unitIndex === 0 ? value.toFixed(0) : value.toFixed(1);
    return `${fixed} ${units[unitIndex]}`;
  }

  function formatPathTail(input) {
    if (!input || typeof input !== 'string') return '';
    const normalized = input.replace(/\\+/g, '/');
    const parts = normalized.split('/').filter(Boolean);
    if (parts.length <= 3) return normalized;
    return `.../${parts.slice(-3).join('/')}`;
  }

  function truncateValue(value, maxLength = 160) {
    if (!value || typeof value !== 'string') return value;
    if (value.length <= maxLength) return value;
    const slice = Math.floor(maxLength / 2) - 1;
    return `${value.slice(0, slice)}…${value.slice(-slice)}`;
  }

  function appendRunnerEvent(payload = {}) {
    if (!runnerEventsUi.log) return;
    const entry = document.createElement('div');
    entry.className = 'runner-event-entry';
    entry.style.padding = '4px 0';
    entry.style.borderBottom = '1px solid rgba(0,0,0,0.06)';
    entry.style.fontSize = '12px';

    const type = typeof payload.type === 'string' ? payload.type : 'info';
    const typeLower = type.toLowerCase();
    if (typeLower === 'stderr' || typeLower === 'error' || typeLower === 'runtime-error') entry.style.color = '#c0392b';
    else if (typeLower === 'recognized') entry.style.color = '#1b5e20';
    else entry.style.color = '#34495e';

    const ts = payload.timestamp ? new Date(payload.timestamp) : new Date();
    const tsText = Number.isNaN(ts.getTime()) ? new Date().toLocaleTimeString() : ts.toLocaleTimeString();

    const metaParts = [];
    if (payload.cameraId !== undefined && payload.cameraId !== null) metaParts.push(`Cam ${payload.cameraId}`);
    if (payload.context) metaParts.push(String(payload.context));
    metaParts.push(type);

    const header = document.createElement('div');
    header.style.fontWeight = '600';
    header.textContent = `[${tsText}] ${metaParts.join(' · ')}`;
    entry.appendChild(header);

    if (payload.message) {
      const message = document.createElement('div');
      message.style.marginTop = '2px';
      message.textContent = payload.message;
      entry.appendChild(message);
    }

    const detailParts = [];
    const pushDetail = (label, value, formatter) => {
      if (value === undefined || value === null || value === '') return;
      const formatted = formatter ? formatter(value) : value;
      if (formatted === undefined || formatted === null || formatted === '') return;
      detailParts.push(`${label}: ${formatted}`);
    };

    pushDetail('python', payload.pythonPath, formatPathTail);
    pushDetail('script', payload.scriptPath, formatPathTail);
    pushDetail('video', payload.videoSource, (v) => truncateValue(v, 140));
    pushDetail('save', payload.saveDir, formatPathTail);
    pushDetail('mode', payload.mode);
    if (payload.pid !== undefined) pushDetail('pid', payload.pid);
    if (payload.frameSkip !== undefined) pushDetail('frameSkip', payload.frameSkip);
    if (payload.resizeWidth !== undefined) pushDetail('resizeWidth', payload.resizeWidth);
    if (payload.useOrt !== undefined) pushDetail('ORT', payload.useOrt ? 'on' : 'off');

    if (detailParts.length) {
      const detail = document.createElement('div');
      detail.style.marginTop = '2px';
      detail.style.fontSize = '11px';
      detail.style.opacity = '0.75';
      detail.textContent = detailParts.join(' · ');
      entry.appendChild(detail);
    }

    runnerEventsUi.log.appendChild(entry);
    while (runnerEventsUi.log.children.length > RUNNER_EVENT_LIMIT) {
      runnerEventsUi.log.removeChild(runnerEventsUi.log.firstChild);
    }
    runnerEventsUi.log.scrollTop = runnerEventsUi.log.scrollHeight;
  }

  function setReadyStatus(payload = {}) {
    const mode = payload.mode || 'ready';
    let label;
    if (mode === 'development') label = 'local Python';
    else if (mode === 'cached') label = 'cached runtime';
    else if (mode === 'downloaded') label = 'downloaded runtime';
    else label = 'runtime';
    let versionLabel = '';
    if (payload.version) {
      versionLabel = payload.version === 'dev' ? ' (dev)' : ` (${payload.version})`;
    }
    const suffix = payload.pythonPath ? ` (${formatPathTail(payload.pythonPath)})` : '';
    setRuntimeStatus(`Runtime ready — ${label}${versionLabel}${suffix}`, { busy: false, showProgress: false });
  }

  function handleRuntimeProgress(payload = {}) {
    if (!payload || !payload.status) return;
    switch (payload.status) {
      case 'checking':
        setRuntimeStatus('Checking runtime...', { busy: true, showProgress: false });
        break;
      case 'downloading': {
        const ratio = typeof payload.progress === 'number' ? Math.max(0, Math.min(1, payload.progress)) : null;
        const percent = ratio !== null ? ` — ${Math.round(ratio * 100)}%` : '';
        const bytesInfo = payload.total ? ` (${formatBytes(payload.downloaded || 0)} / ${formatBytes(payload.total)})` : '';
        setRuntimeStatus(`Downloading runtime${percent}${bytesInfo}`, { busy: true, showProgress: true, progress: ratio });
        break;
      }
      case 'verifying':
        setRuntimeStatus('Verifying archive checksum...', { busy: true, showProgress: true });
        break;
      case 'extracting':
        setRuntimeStatus('Extracting runtime files...', { busy: true, showProgress: true });
        break;
      case 'resetting':
        setRuntimeStatus('Removing existing runtime...', { busy: true, showProgress: true });
        break;
      case 'ready':
        setReadyStatus(payload);
        setTimeout(() => { refreshRuntimeStatus(); }, 300);
        break;
      case 'error':
        setRuntimeStatus(`Runtime error: ${payload.message || 'unknown error'}`, { busy: false, showProgress: false, tone: 'error' });
        setTimeout(() => { refreshRuntimeStatus(); }, 500);
        break;
      default:
        break;
    }
  }

  function handleRuntimeError(payload = {}) {
    const cameraScope = payload && Number.isFinite(payload.cameraId) ? ` (camera ${payload.cameraId})` : '';
    const msg = payload && payload.message ? payload.message : 'unknown error';
    setRuntimeStatus(`Runtime error${cameraScope}: ${msg}`, { busy: false, showProgress: false, tone: 'error' });
    setTimeout(() => { refreshRuntimeStatus(); }, 500);
    appendRunnerEvent({
      type: 'runtime-error',
      message: `Runtime error${cameraScope}: ${msg}`,
      cameraId: payload && Number.isFinite(payload.cameraId) ? payload.cameraId : undefined,
      context: payload && payload.context ? payload.context : 'primary'
    });
  }

  function applyRuntimeStatus(status) {
    if (!status) {
      setRuntimeStatus('Runtime status unavailable', { busy: false, tone: 'error' });
      return;
    }
    if (status.runtime) {
      const mode = status.runtime.mode || 'ready';
      setReadyStatus({ mode, pythonPath: status.runtime.pythonPath, version: status.runtime.version });
      return;
    }
    if (status.installed) {
      setRuntimeStatus('Downloaded runtime detected. It will be verified automatically.', { busy: false, showProgress: false });
      return;
    }
    if (status.developmentAvailable) {
      const suffix = status.pythonPath ? ` (${formatPathTail(status.pythonPath)})` : '';
      setRuntimeStatus(`Local Python available${suffix}. Download runtime for offline use if needed.`, { busy: false, showProgress: false });
      return;
    }
    setRuntimeStatus('Runtime not installed yet. Use "Reinstall Runtime" to download it.', { busy: false, showProgress: false, tone: 'error' });
  }

  async function refreshRuntimeStatus() {
    if (!window.api || typeof window.api.invoke !== 'function') return;
    try {
      const response = await window.api.invoke('module-license-plate-runtime-status');
      if (response && response.success && response.data) {
        applyRuntimeStatus(response.data);
      } else {
        const message = response && response.error ? response.error : 'unknown error';
        setRuntimeStatus(`Runtime status unavailable: ${message}`, { busy: false, showProgress: false, tone: 'error' });
      }
    } catch (e) {
      console.error('[LP Renderer] Failed to query runtime status', e);
      setRuntimeStatus('Runtime status unavailable', { busy: false, showProgress: false, tone: 'error' });
    }
  }

  async function requestRuntimeReinstall() {
    if (!window.api || typeof window.api.invoke !== 'function') return;
    setRuntimeStatus('Starting runtime reinstall...', { busy: true, showProgress: true, progress: 0 });
    try {
      const response = await window.api.invoke('module-license-plate-runtime-reinstall');
      if (!response || response.success === false) {
        const message = response && response.error ? response.error : 'unknown error';
        setRuntimeStatus(`Failed to start reinstall: ${message}`, { busy: false, showProgress: false, tone: 'error' });
      }
    } catch (e) {
      console.error('[LP Renderer] Failed to start runtime reinstall', e);
      setRuntimeStatus(`Failed to start reinstall: ${e && e.message ? e.message : e}`, { busy: false, showProgress: false, tone: 'error' });
    }
  }

  if (runtimeUi.reinstallButton) {
    runtimeUi.reinstallButton.addEventListener('click', requestRuntimeReinstall);
  }

  if (runtimeUi.reinstallButton && (!window.api || typeof window.api.invoke !== 'function')) {
    runtimeUi.reinstallButton.disabled = true;
  }

  if (window.api && typeof window.api.on === 'function') {
    window.api.on('module-license-plate-runtime-progress', handleRuntimeProgress);
    window.api.on('module-license-plate-runtime-error', handleRuntimeError);
    window.api.on('module-license-plate-runner-event', (payload) => appendRunnerEvent(payload || {}));
  }

  if (window.api && typeof window.api.invoke === 'function') {
    refreshRuntimeStatus();
  } else {
    setRuntimeStatus('Runtime controls are unavailable in this build.', { busy: false, showProgress: false, allowReinstall: false });
  }

  // Update stats and unique plates
  function updatePlatesDisplay() {
      // Request plates data from main process via exposed window.api
        window.api.invoke('module-license-plate-get-detected').then(data => {
          if (data) {
          // Update stats
          const statsDiv = document.getElementById('plates-stats');
          statsDiv.textContent = `Total detections: ${data.totalDetections}`;
          
          // Update unique plates
          const uniqueDiv = document.getElementById('unique-plates-list');
          uniqueDiv.innerHTML = '';
          
          data.uniquePlates.forEach(cameraData => {
            const cameraDiv = document.createElement('div');
            cameraDiv.innerHTML = `<strong>Camera ${cameraData.cameraId}:</strong> ${cameraData.count} unique plates`;
            uniqueDiv.appendChild(cameraDiv);
            
            cameraData.plates.forEach(plate => {
              const plateDiv = document.createElement('div');
              plateDiv.style.marginLeft = '20px';
              plateDiv.textContent = plate;
              uniqueDiv.appendChild(plateDiv);
            });
          });
        }
        
      }).catch(err => console.error('Failed to get plates data:', err));
  } // <-- Properly close updatePlatesDisplay function

  // Initialize ORT checkbox state from app settings and save on change (run once)
  async function initOrtCheckbox() {
    try {
      const key = 'module_license-plate_use_ort';
      const checkbox = document.getElementById('use-ort-checkbox');
      if (!checkbox) return;
      // Load current settings for initialization
      try {
        const settings = await window.api.getAppSettings();
        checkbox.checked = !!settings[key];
      } catch (e) {
        console.warn('[LP Renderer] Could not read initial app settings for ORT checkbox', e);
      }

      // On change, prefer using the global state manager so the save goes through App.saveAppSettings
      // (that function performs safe cloning before sending to main). If App isn't ready, fall back
      // to fetching and saving via window.api directly.
      checkbox.addEventListener('change', async () => {
        try {
          if (window.App && window.App.stateManager && typeof window.App.stateManager.setAppSettings === 'function') {
            window.App.stateManager.setAppSettings({ [key]: checkbox.checked });
            console.log('[LP Renderer] module_license-plate_use_ort set via App.stateManager to', checkbox.checked);
          } else {
            const current = await window.api.getAppSettings();
            const updated = Object.assign({}, current || {}, { [key]: checkbox.checked });
            await window.api.saveAppSettings(updated);
            console.log('[LP Renderer] module_license-plate_use_ort set (fallback) to', checkbox.checked);
          }
        } catch (e) { console.error('[LP Renderer] Failed to save app settings', e); }
      });
    } catch (e) { console.error('[LP Renderer] Failed to initialize ORT checkbox', e); }
  }

  initOrtCheckbox();

  // Initialize Test DirectML button
  function initOrtTestButton() {
    const btn = document.getElementById('ort-test-button');
    const resSpan = document.getElementById('ort-test-result');
    if (!btn || !resSpan) return;
    btn.addEventListener('click', async () => {
      resSpan.textContent = 'Checking...';
      try {
        const r = await window.api.invoke('module-license-plate-test-ort');
        if (!r) { resSpan.textContent = 'No response'; return; }
        if (r.success) {
          const data = r.data || {};
          const providers = (data.providers || []).join(', ');
          const sess = (data.session_providers || []).join(', ');
          resSpan.textContent = `Available: ${providers || '<none>'}; Session: ${sess || '<none>'}`;
        } else {
          resSpan.textContent = `Error: ${r.error || 'unknown'}`;
        }
      } catch (e) { resSpan.textContent = 'Probe failed'; console.error(e); }
      setTimeout(() => { resSpan.textContent = ''; }, 10000);
    });
  }

  initOrtTestButton();

  // Update display every 5 seconds
  setInterval(updatePlatesDisplay, 5000);
  updatePlatesDisplay(); // Initial update

  if (runnerEventsUi.log) {
    appendRunnerEvent({ type: 'info', message: 'Runner diagnostics initialized' });
  }

  // Listen for saved plates via window.api.on
  window.api.on('module-license-plate-saved', (data) => {
    const list = document.getElementById('license-plate-list');
    const li = document.createElement('li');
    const timestamp = new Date().toLocaleTimeString();
    li.textContent = `[${timestamp}] Cam ${data.cameraId}: ${data.text || 'N/A'} (Score: ${data.score?.toFixed(2) || 'N/A'})`;
    list.appendChild(li);
    
    // Keep only last 20 items
    while (list.children.length > 20) {
      list.removeChild(list.firstChild);
    }
    
    // Update stats after new detection
    setTimeout(updatePlatesDisplay, 1000);
  });

  window.api.on('module-license-plate-cleanup', () => {
    const list = document.getElementById('license-plate-list');
    list.innerHTML = '';
    const uniqueDiv = document.getElementById('unique-plates-list');
    uniqueDiv.innerHTML = '';
    const statsDiv = document.getElementById('plates-stats');
    statsDiv.textContent = '';
  });
});
