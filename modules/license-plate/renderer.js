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
