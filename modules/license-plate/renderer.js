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
    `;
  } else {
    // Fallback: create separate container if modules panel is not found
    const container = document.createElement('div');
    container.className = 'module-license-plate';
    container.innerHTML = `
      <h4>License Plate Module</h4>
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
    `;
    document.body.appendChild(container);
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
