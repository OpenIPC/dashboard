// --- START OF FILE js/grid-manager.js ---
(function(window) {
    window.AppModules = window.AppModules || {};

    window.AppModules.createGridManager = function(App) {
        const stateManager = App.stateManager;
        const gridContainer = document.getElementById('grid-container');
        const layoutControls = document.getElementById('layout-controls');
        const MAX_GRID_SIZE = 64;

        let localPlayers = {};
        let gridCells = [];
        let fullscreenCellIndex = null;
        let currentAudioPlayer = null;
        let isRendering = false;
        const analyticsState = {};
        let animationFrameId = null;

        function startRenderLoop() {
            if (animationFrameId) {
                cancelAnimationFrame(animationFrameId);
            }
            function renderAnalytics() {
                const now = Date.now();
                const gridState = getGridState();
                if (!gridState) {
                    animationFrameId = requestAnimationFrame(renderAnalytics);
                    return;
                }
                for (const cell of gridCells) {
                    const cellId = parseInt(cell.dataset.cellId, 10);
                    const cellState = gridState[cellId];
                    const cameraId = cellState?.camera?.id;
                    if (!cameraId || !analyticsState[cameraId] || (now - analyticsState[cameraId].timestamp > 2000)) {
                         const overlayCanvas = cell.querySelector('.overlay-canvas');
                         if (overlayCanvas) {
                             const ctx = overlayCanvas.getContext('2d');
                             ctx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
                         }
                         if(analyticsState[cameraId]) {
                            delete analyticsState[cameraId];
                         }
                    }
                }
                for (const cameraId in analyticsState) {
                    const state = analyticsState[cameraId];
                    const cellsForCamera = gridState.map((s, i) => ({s, i})).filter(item => item.s && item.s.camera.id == cameraId);
                    for (const cellInfo of cellsForCamera) {
                        const cellElement = gridCells[cellInfo.i];
                        if (!cellElement) continue;
                        const videoCanvas = cellElement.querySelector('.video-canvas');
                        const overlayCanvas = cellElement.querySelector('.overlay-canvas');
                        if (!videoCanvas || !overlayCanvas) continue;
                        if (overlayCanvas.width !== videoCanvas.clientWidth || overlayCanvas.height !== videoCanvas.clientHeight) {
                            overlayCanvas.width = videoCanvas.clientWidth;
                            overlayCanvas.height = videoCanvas.clientHeight;
                        }
                        const ctx = overlayCanvas.getContext('2d');
                        ctx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
                        if (!state.frame_width || !state.frame_height) continue;
                        const scaleX = overlayCanvas.width / state.frame_width;
                        const scaleY = overlayCanvas.height / state.frame_height;
                        state.objects.forEach(obj => {
                            const x = obj.box.x * scaleX;
                            const y = obj.box.y * scaleY;
                            const w = obj.box.w * scaleX;
                            const h = obj.box.h * scaleY;
                            const translatedLabel = App.t('object_' + obj.label) || obj.label;
                            const label = `${translatedLabel} (${Math.round(obj.confidence * 100)}%)`;
                            ctx.strokeStyle = (obj.label === 'person') ? '#3498db' : '#f1c40f';
                            ctx.lineWidth = 2;
                            ctx.strokeRect(x, y, w, h);
                            ctx.fillStyle = 'rgba(0, 0, 0, 0.7)';
                            const textMetrics = ctx.measureText(label);
                            ctx.fillRect(x, y > 18 ? y - 18 : y, textMetrics.width + 8, 16);
                            ctx.fillStyle = 'white';
                            ctx.font = '12px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif';
                            ctx.fillText(label, x + 4, y > 18 ? y - 5 : y + 12);
                        });
                    }
                }
                animationFrameId = requestAnimationFrame(renderAnalytics);
            }
            renderAnalytics();
        }

        function getActiveLayoutState() {
            const { layouts, activeLayoutId } = stateManager.state;
            if (!layouts || layouts.length === 0) return null;
            return layouts.find(l => l.id === activeLayoutId) || layouts[0];
        }

        function getGridState() {
            const activeLayout = getActiveLayoutState();
            return activeLayout ? activeLayout.gridState : Array(64).fill(null);
        }

        function updatePlaceholdersLanguage() {
            const placeholderHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
            gridCells.forEach(cell => {
                const isOccupied = Object.values(localPlayers).some(p => p.cell === cell);
                if (!isOccupied) {
                    cell.innerHTML = placeholderHTML;
                }
            });
        }

        function initializeLayoutControls() {
            const layouts = ["1x1", "2x2", "3x3", "4x4", "5x5", "8x4", "8x8"];
            layoutControls.innerHTML = '';
            layouts.forEach(layout => {
                const btn = document.createElement('button');
                btn.className = 'layout-btn';
                btn.dataset.layout = layout;
                btn.textContent = layout.split('x').reduce((a, b) => a * b, 1);
                btn.title = `Layout ${layout}`;
                btn.onclick = () => {
                    const [cols, rows] = layout.split('x').map(Number);
                    stateManager.updateGridLayout({ cols, rows });
                };
                layoutControls.appendChild(btn);
            });
        }

        function updateActiveLayoutButton() {
            const activeLayout = getActiveLayoutState();
            if (!activeLayout || !activeLayout.layout) return;
            const { layout } = activeLayout;
            const currentLayout = `${layout.cols}x${layout.rows}`;
            layoutControls.querySelectorAll('.layout-btn').forEach(btn => {
                btn.classList.toggle('active', btn.dataset.layout === currentLayout);
            });
        }

        function updateGridLayoutView() {
            const activeLayout = getActiveLayoutState();
            if (!activeLayout || !activeLayout.layout) return;
            const { layout } = activeLayout;
            const totalVisibleCells = layout.cols * layout.rows;
            const cellWidth = 100 / layout.cols;
            const cellHeight = 100 / layout.rows;
            gridCells.forEach((cell, i) => {
                if (i < totalVisibleCells) {
                    cell.style.display = 'flex';
                    cell.style.top = `${Math.floor(i / layout.cols) * cellHeight}%`;
                    cell.style.left = `${i % layout.cols * cellWidth}%`;
                    cell.style.width = `${cellWidth}%`;
                    cell.style.height = `${cellHeight}%`;
                } else {
                    const cellState = getGridState()[i];
                    if (cellState) {
                        const uniqueId = `stream-${cellState.camera.id}_${cellState.streamId}_${i}`;
                        if (localPlayers[uniqueId]) destroyPlayer(uniqueId);
                    }
                    cell.style.display = 'none';
                }
            });
            updateActiveLayoutButton();
        }
        
        async function destroyPlayer(id) {
            const playerData = localPlayers[id];
            if (!playerData) return;
            if (currentAudioPlayer && currentAudioPlayer.player === playerData.player) {
                currentAudioPlayer = null;
            }
            console.log(`[Grid] Destroying stream: ${id}`);
            if (playerData.cell) {
                playerData.cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                playerData.cell.classList.remove('active');
                playerData.cell.draggable = false;
            }
            await window.api.stopVideoStream(id);
            if (playerData.player) {
                 try { playerData.player.destroy(); } catch (e) {}
            }
            delete localPlayers[id];
        }

        function attachControlEvents(cellElement, cellIndex) {
            const controls = cellElement.querySelector('.cell-controls');
            if (!controls) return;
            const activeLayout = getActiveLayoutState();
            if (!activeLayout) return;
            const cellState = activeLayout.gridState[cellIndex];
            if (!cellState) return;
            controls.querySelector('.fullscreen-btn').onclick = (e) => { e.stopPropagation(); toggleFullscreen(cellIndex); };
            const streamSwitchBtn = controls.querySelector('.stream-switch-btn');
            if (streamSwitchBtn) {
                streamSwitchBtn.innerHTML = cellState.streamId === 0 ? '<i class="material-icons">hd</i>' : '<i class="material-icons">sd</i>';
                streamSwitchBtn.onclick = (e) => {
                    e.stopPropagation();
                    const loadingOverlay = cellElement.querySelector('.loading-overlay');
                    if(loadingOverlay) {
                        loadingOverlay.classList.remove('hidden');
                    }
                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    if (newGrid[cellIndex]) {
                        newGrid[cellIndex].streamId = newGrid[cellIndex].streamId === 0 ? 1 : 0;
                        stateManager.updateGridState(newGrid);
                    }
                };
            }
            controls.querySelector('.close-btn').onclick = (e) => {
                e.stopPropagation();
                const newGrid = getGridState().map(g => g ? { ...g } : null);
                newGrid[cellIndex] = null;
                stateManager.updateGridState(newGrid);
            };
            const audioBtn = controls.querySelector('.audio-btn');
            const uniqueId = `stream-${cellState.camera.id}_${cellState.streamId}_${cellIndex}`;
            const player = localPlayers[uniqueId]?.player;
            if (player) {
                audioBtn.onclick = (e) => {
                    e.stopPropagation();
                    if (currentAudioPlayer && currentAudioPlayer.player === player) {
                        player.volume = 0;
                        audioBtn.classList.remove('active');
                        audioBtn.innerHTML = '<i class="material-icons">volume_off</i>';
                        currentAudioPlayer = null;
                    } else {
                        if (currentAudioPlayer) {
                            currentAudioPlayer.player.volume = 0;
                            if (currentAudioPlayer.button) {
                                currentAudioPlayer.button.classList.remove('active');
                                currentAudioPlayer.button.innerHTML = '<i class="material-icons">volume_off</i>';
                            }
                        }
                        player.volume = 1;
                        audioBtn.classList.add('active');
                        audioBtn.innerHTML = '<i class="material-icons">volume_up</i>';
                        currentAudioPlayer = { player: player, button: audioBtn };
                    }
                };
            }
            const recordBtn = controls.querySelector('.record-btn');
            const camera = stateManager.state.cameras.find(c => c.id === cellState.camera.id);
            if (recordBtn && camera) {
                recordBtn.onclick = (e) => {
                    e.stopPropagation();
                    window.api.toggleRecording(camera);
                };
            }
        }

        async function render() {
            if (isRendering) {
                console.warn('[Grid] Render call skipped, another render is in progress.');
                return;
            }
            isRendering = true;
            console.log('[Grid] Starting render...');
            try {
                updateGridLayoutView();
                const activeLayout = getActiveLayoutState();
                if (!activeLayout) {
                    for (const id in localPlayers) await destroyPlayer(id);
                    return;
                }
                const { gridState } = activeLayout;
                const { cameras, recordingStates } = stateManager.state;
                const desiredStreams = new Set();
                const streamsToCreate = [];
                if (gridState) {
                    gridState.forEach((cellState, i) => {
                        if (cellState) {
                            const uniqueId = `stream-${cellState.camera.id}_${cellState.streamId}_${i}`;
                            desiredStreams.add(uniqueId);
                            if (!localPlayers[uniqueId]) {
                                streamsToCreate.push({ cellState, index: i, uniqueId });
                            }
                        }
                    });
                }
                for (const id in localPlayers) {
                    if (!desiredStreams.has(id)) {
                        await destroyPlayer(id);
                    }
                }
                for (const { cellState, index, uniqueId } of streamsToCreate) {
                    const camera = cameras.find(c => c.id === cellState.camera.id);
                    if (!camera) continue;
                    const cellElement = gridCells[index];
                    const template = document.getElementById('grid-cell-content-template');
                    if (!template) {
                        console.error('CRITICAL: grid-cell-content-template not found!');
                        continue;
                    }
                    const content = template.content.cloneNode(true);
                    cellElement.innerHTML = '';
                    cellElement.appendChild(content);
                    App.i18n.applyTranslationsToDOM(cellElement);
                    const loadingOverlay = cellElement.querySelector('.loading-overlay');
                    if (loadingOverlay) loadingOverlay.classList.remove('hidden');
                    const nameDiv = cellElement.querySelector('.cell-name');
                    const qualityLabel = cellState.streamId === 0 ? 'HD' : 'SD';
                    if (nameDiv) nameDiv.textContent = `${camera.name} (${qualityLabel}`;
                    const statsDiv = cellElement.querySelector('.cell-stats');
                    if (statsDiv) statsDiv.id = `stats-${uniqueId}`;
                    const result = await window.api.startVideoStream({ credentials: camera, streamId: cellState.streamId, uniqueStreamIdentifier: uniqueId });
                    if (result.success) {
                        const videoCanvas = cellElement.querySelector('.video-canvas');
                        const player = new JSMpeg.Player(`ws://localhost:${result.wsPort}`, { canvas: videoCanvas, autoplay: true, audio: true, volume: 0, disableWebAssembly: true, onPlay: () => { if (loadingOverlay) loadingOverlay.classList.add('hidden'); } });
                        localPlayers[uniqueId] = { player, cell: cellElement };
                    } else {
                        handleStreamDeath({ uniqueStreamIdentifier: uniqueId, error: result.error || App.i18n.t('unknown_error') });
                    }
                    attachControlEvents(cellElement, index);
                    cellElement.classList.add('active');
                    const currentUser = stateManager.state.currentUser;
                    cellElement.draggable = currentUser?.role === 'admin' || currentUser?.permissions?.manage_layout;
                    if (cellElement._dragStartHandler) cellElement.removeEventListener('dragstart', cellElement._dragStartHandler);
                    cellElement._dragStartHandler = (e) => { e.dataTransfer.setData("application/x-grid-cell-index", index.toString()); e.dataTransfer.effectAllowed = 'move'; };
                    cellElement.addEventListener('dragstart', cellElement._dragStartHandler);
                }
                for (const id in localPlayers) {
                    const { cell } = localPlayers[id];
                    const cameraId = id.split('_')[0].split('-')[1];
                    if (cell) {
                        const recordBtn = cell.querySelector('.record-btn');
                        if (recordBtn) recordBtn.classList.toggle('recording', !!recordingStates[cameraId]);
                    }
                }
                const occupiedCellIndexes = new Set(Object.values(localPlayers).map(p => parseInt(p.cell.dataset.cellId, 10)));
                gridCells.forEach((cell, i) => {
                    if (!occupiedCellIndexes.has(i) && cell.classList.contains('active')) {
                        cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                        cell.classList.remove('active');
                        cell.draggable = false;
                    }
                });
            } finally {
                console.log('[Grid] Render finished.');
                isRendering = false;
            }
        }

        function toggleFullscreen(cellIndex) {
            const cell = gridCells[cellIndex];
            if (!cell) return;
            const newGrid = getGridState().map(g => g ? { ...g } : null);
            const cellState = newGrid[cellIndex];
            if (!cellState) return;
            const isCurrentlyFullscreen = cell.classList.contains('fullscreen');
            const fsBtnIcon = cell.querySelector('.fullscreen-btn i');
            if (isCurrentlyFullscreen) {
                cellState.streamId = 1; 
                gridContainer.classList.remove('fullscreen-mode');
                cell.classList.remove('fullscreen');
                if (fsBtnIcon) fsBtnIcon.textContent = 'fullscreen';
                fullscreenCellIndex = null;
            } else {
                if (fullscreenCellIndex !== null) {
                    const oldFullscreenCell = gridCells[fullscreenCellIndex];
                    if (oldFullscreenCell && newGrid[fullscreenCellIndex]) {
                        newGrid[fullscreenCellIndex].streamId = 1;
                        oldFullscreenCell.classList.remove('fullscreen');
                        const oldFsBtnIcon = oldFullscreenCell.querySelector('.fullscreen-btn i');
                        if (oldFsBtnIcon) oldFsBtnIcon.textContent = 'fullscreen';
                    }
                }
                cellState.streamId = 0;
                fullscreenCellIndex = cellIndex;
                gridContainer.classList.add('fullscreen-mode');
                cell.classList.add('fullscreen');
                if (fsBtnIcon) fsBtnIcon.textContent = 'fullscreen_exit';
            }
            stateManager.updateGridState(newGrid);
        }

        function init() {
            for (let i = 0; i < MAX_GRID_SIZE; i++) {
                const cell = document.createElement('div');
                cell.className = 'grid-cell';
                cell.dataset.cellId = i;
                cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                cell.ondblclick = () => toggleFullscreen(i);
                cell.addEventListener('dragover', (e) => { e.preventDefault(); cell.classList.add('drag-over'); });
                cell.addEventListener('dragleave', () => { cell.classList.remove('drag-over'); });
                cell.addEventListener('drop', (e) => {
                    e.preventDefault();
                    cell.classList.remove('drag-over');
                    const currentUser = App.stateManager.state.currentUser;
                    if (currentUser?.role !== 'admin' && !currentUser?.permissions?.manage_layout) return;
                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    const targetIndex = i;
                    const cameraIdStr = e.dataTransfer.getData('application/x-camera-id');
                    if (cameraIdStr) {
                        const cameraId = parseInt(cameraIdStr, 10);
                        if (!isNaN(cameraId)) {
                            newGrid[targetIndex] = { camera: { id: cameraId }, streamId: 1 };
                            stateManager.updateGridState(newGrid);
                        }
                        return;
                    }
                    const sourceCellIndexStr = e.dataTransfer.getData("application/x-grid-cell-index");
                    if (sourceCellIndexStr !== "") {
                        const sourceIdx = parseInt(sourceCellIndexStr, 10);
                        [newGrid[targetIndex], newGrid[sourceIdx]] = [newGrid[sourceIdx], newGrid[targetIndex]];
                        stateManager.updateGridState(newGrid);
                    }
                });
                gridContainer.appendChild(cell);
                gridCells.push(cell);
            }
            initializeLayoutControls();
            window.addEventListener('keydown', (e) => { if (e.key === 'Escape' && fullscreenCellIndex !== null) { toggleFullscreen(fullscreenCellIndex); } });
            window.addEventListener('language-changed', updatePlaceholdersLanguage);
            startRenderLoop();
        }
        
        async function handleStreamDeath({ uniqueStreamIdentifier, error }) {
            console.log(`[Grid] Stream ${uniqueStreamIdentifier} died. Error: ${error}`);
            const playerInfo = localPlayers[uniqueStreamIdentifier];
            if (playerInfo) {
                delete localPlayers[uniqueStreamIdentifier]; 
                if (playerInfo.cell) {
                    playerInfo.cell.classList.add('error-state');
                    const loadingOverlay = playerInfo.cell.querySelector('.loading-overlay');
                    if (loadingOverlay) loadingOverlay.classList.add('hidden');
                    const errorOverlay = playerInfo.cell.querySelector('.error-overlay');
                    if (errorOverlay) {
                        const errorMessageEl = errorOverlay.querySelector('.error-message');
                        const retryBtn = errorOverlay.querySelector('.retry-button');
                        const closeBtn = errorOverlay.querySelector('.close-on-error-btn');
                        if (!retryBtn || !closeBtn) {
                            console.error("Retry or Close button not found in error overlay template!");
                            return;
                        }
                        retryBtn.textContent = App.t('retry_button') || 'Повторить';
                        closeBtn.textContent = App.t('close_button') || 'Закрыть';
                        let simpleError = error;
                        if (error.includes('Connection refused')) simpleError = 'Connection refused. (Камера не отвечает)';
                        else if (error.includes('Invalid data')) simpleError = 'Invalid data. (Неверный путь к потоку?)';
                        else if (error.includes('401 Unauthorized')) simpleError = '401 Unauthorized. (Неверный логин/пароль)';
                        errorMessageEl.textContent = simpleError;
                        errorOverlay.classList.remove('hidden');
                        const newRetryBtn = retryBtn.cloneNode(true);
                        retryBtn.parentNode.replaceChild(newRetryBtn, retryBtn);
                        const newCloseBtn = closeBtn.cloneNode(true);
                        closeBtn.parentNode.replaceChild(newCloseBtn, closeBtn);
                        newRetryBtn.onclick = () => { console.log(`[Grid] Retrying stream ${uniqueStreamIdentifier}...`); render(); };
                        newCloseBtn.onclick = () => { console.log(`[Grid] Closing errored cell for stream ${uniqueStreamIdentifier}...`); const cellIndex = parseInt(playerInfo.cell.dataset.cellId, 10); const newGrid = getGridState().map(g => g ? { ...g } : null); newGrid[cellIndex] = null; stateManager.updateGridState(newGrid); };
                    }
                }
            }
        }
        
        async function restartStreamsForCamera(cameraId) {
            console.log(`[Grid] Restarting all streams for camera ID: ${cameraId}`);
            const streamsToRestart = [];
            for (const id in localPlayers) {
                if (id.startsWith(`stream-${cameraId}_`)) {
                    streamsToRestart.push(id);
                }
            }
            for (const id of streamsToRestart) {
                await destroyPlayer(id);
            }
            setTimeout(() => render(), 100);
        }

        function updateStatsText(statsDiv) {
            if (!statsDiv) return;
            const codec = statsDiv.dataset.codec || '';
            const resolution = statsDiv.dataset.resolution || '';
            const fps = statsDiv.dataset.fps || '0';
            const bitrate = statsDiv.dataset.bitrate || '0';
            const staticInfo = [codec, resolution].filter(Boolean).join(', ');
            const dynamicInfo = `${Math.round(fps)}fps, ${Math.round(bitrate)}kbps`;
            statsDiv.textContent = [staticInfo, dynamicInfo].filter(Boolean).join(' | ');
        }
        
        function updateStreamInfo({ uniqueStreamIdentifier, codec, resolution }) {
            const statsDiv = document.getElementById(`stats-${uniqueStreamIdentifier}`);
            if (statsDiv) {
                statsDiv.dataset.codec = codec.toUpperCase();
                statsDiv.dataset.resolution = resolution;
                updateStatsText(statsDiv);
            }
        }
        
        function updateStreamStats({ uniqueStreamIdentifier, fps, bitrate }) {
            const statsDiv = document.getElementById(`stats-${uniqueStreamIdentifier}`);
            if (statsDiv) {
                statsDiv.dataset.fps = fps;
                statsDiv.dataset.bitrate = bitrate;
                updateStatsText(statsDiv);
            }
        }

        function handleAnalyticsUpdate({ cameraId, result }) {
            if (result && result.status === 'objects_detected' && result.objects) {
                analyticsState[cameraId] = {
                    objects: result.objects,
                    frame_width: result.frame_width,
                    frame_height: result.frame_height,
                    timestamp: Date.now()
                };
            }
        }

        return {
            init,
            render,
            getGridState,
            updateGridLayoutView,
            updatePlaceholdersLanguage,
            handleStreamDeath,
            restartStreamsForCamera,
            updateStreamStats,
            updateStreamInfo,
            handleAnalyticsUpdate
        };
    };
})(window);
// --- END OF FILE js/grid-manager.js ---