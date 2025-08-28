// --- START OF FILE js/grid-manager.js ---
(function(window) {
    window.AppModules = window.AppModules || {};

    window.AppModules.createGridManager = function(App) {
        const stateManager = App.stateManager;
        const gridContainer = document.getElementById('grid-container');
        const layoutControls = document.getElementById('layout-controls');
        const MAX_GRID_SIZE = 64;

        let localPlayers = {}; // Ключ: uniqueIdBase (stream-cameraId_cellIndex)
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
                        const videoCanvas = cellElement.querySelector('.video-canvas:not(.hidden)');
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
                        const uniqueIdBase = `stream-${cellState.camera.id}_${i}`;
                        if (localPlayers[uniqueIdBase]) {
                            destroyPlayerSet(uniqueIdBase);
                        }
                    }
                    cell.style.display = 'none';
                }
            });
            updateActiveLayoutButton();
        }
        
        async function destroyPlayerSet(uniqueIdBase) {
            const playerData = localPlayers[uniqueIdBase];
            if (!playerData) return;
            if (currentAudioPlayer && (currentAudioPlayer.player === playerData.hdPlayer || currentAudioPlayer.player === playerData.sdPlayer)) {
                currentAudioPlayer = null;
            }
            console.log(`[Grid] Destroying stream set: ${uniqueIdBase}`);
            if (playerData.cell) {
                playerData.cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                playerData.cell.classList.remove('active', 'error-state');
                playerData.cell.draggable = false;
            }
            await window.api.stopVideoStream(uniqueIdBase);
            if (playerData.hdPlayer) try { playerData.hdPlayer.destroy(); } catch (e) {}
            if (playerData.sdPlayer) try { playerData.sdPlayer.destroy(); } catch (e) {}
            delete localPlayers[uniqueIdBase];
        }

        function attachControlEvents(cellElement, cellIndex) {
            const controls = cellElement.querySelector('.cell-controls');
            if (!controls) return;
            const cellState = getGridState()[cellIndex];
            if (!cellState) return;
            controls.querySelector('.fullscreen-btn').onclick = (e) => { e.stopPropagation(); toggleFullscreen(cellIndex); };
            const streamSwitchBtn = controls.querySelector('.stream-switch-btn');
            if (streamSwitchBtn) {
                streamSwitchBtn.onclick = (e) => {
                    e.stopPropagation();
                    const newStreamId = cellState.streamId === 0 ? 1 : 0;
                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    if (newGrid[cellIndex]) {
                        newGrid[cellIndex].streamId = newStreamId;
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
            const uniqueIdBase = `stream-${cellState.camera.id}_${cellIndex}`;
            const playerData = localPlayers[uniqueIdBase];
            if (playerData) {
                audioBtn.onclick = (e) => {
                    e.stopPropagation();
                    const activePlayer = cellState.streamId === 0 ? playerData.hdPlayer : playerData.sdPlayer;
                    if (!activePlayer) return;
                    if (currentAudioPlayer && currentAudioPlayer.player === activePlayer) {
                        activePlayer.volume = 0;
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
                        activePlayer.volume = 1;
                        audioBtn.classList.add('active');
                        audioBtn.innerHTML = '<i class="material-icons">volume_up</i>';
                        currentAudioPlayer = { player: activePlayer, button: audioBtn };
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

        function attachPtzEvents(cellElement, cellIndex) {
            const cellState = getGridState()[cellIndex];
            if (!cellState || !cellState.camera) return;

            const cameraId = cellState.camera.id;
            const ptzButtons = cellElement.querySelectorAll('.ptz-btn');

            const sendPtzCommand = (command, action) => {
                console.log(`[PTZ] Sending command:`, { cameraId, command, action });
                window.api.ptzControl({ cameraId, command, action });
            };

            ptzButtons.forEach(btn => {
                const command = btn.dataset.command;
                if (!command) return;

                // Начать движение при нажатии
                btn.addEventListener('mousedown', (e) => {
                    e.stopPropagation();
                    sendPtzCommand(command, 'start');
                });
                // Остановить движение при отпускании кнопки
                btn.addEventListener('mouseup', (e) => {
                    e.stopPropagation();
                    sendPtzCommand('stop', 'stop');
                });
                // Остановить движение, если курсор ушел с кнопки
                btn.addEventListener('mouseleave', (e) => {
                    // Проверяем, что кнопка мыши больше не зажата
                    if (e.buttons !== 1) {
                         sendPtzCommand('stop', 'stop');
                    }
                });
            });
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
                    for (const id in localPlayers) await destroyPlayerSet(id);
                    isRendering = false;
                    return;
                }
                const { gridState } = activeLayout;
                const { cameras, recordingStates } = stateManager.state;
                const desiredStreams = new Set();
                const streamsToCreate = [];
                if (gridState) {
                    gridState.forEach((cellState, i) => {
                        if (cellState) {
                            const uniqueIdBase = `stream-${cellState.camera.id}_${i}`;
                            desiredStreams.add(uniqueIdBase);
                            if (!localPlayers[uniqueIdBase]) {
                                streamsToCreate.push({ cellState, index: i, uniqueIdBase });
                            }
                        }
                    });
                }
                for (const idBase in localPlayers) {
                    if (!desiredStreams.has(idBase)) {
                        await destroyPlayerSet(idBase);
                    }
                }
                for (const { cellState, index, uniqueIdBase } of streamsToCreate) {
                    const camera = cameras.find(c => c.id === cellState.camera.id);
                    if (!camera) continue;
                    const cellElement = gridCells[index];
                    const template = document.getElementById('grid-cell-content-template');
                    if (!template) { console.error('CRITICAL: grid-cell-content-template not found!'); continue; }
                    const content = template.content.cloneNode(true);
                    cellElement.innerHTML = '';
                    cellElement.appendChild(content);
                    App.i18n.applyTranslationsToDOM(cellElement);
                    const loadingOverlay = cellElement.querySelector('.loading-overlay');
                    if (loadingOverlay) loadingOverlay.classList.remove('hidden');
                    const statsDiv = cellElement.querySelector('.cell-stats');
                    if (statsDiv) statsDiv.id = `stats-${uniqueIdBase}`;
                    const result = await window.api.startVideoStream({ credentials: camera, uniqueStreamIdentifier: uniqueIdBase });
                    if (result.success && result.hdPort && result.sdPort) {
                        const hdCanvas = cellElement.querySelector('.video-canvas-hd');
                        const sdCanvas = cellElement.querySelector('.video-canvas-sd');
                        let loadedStreams = 0;
                        const onPlay = () => {
                            loadedStreams++;
                            if (loadedStreams >= 2 && loadingOverlay) {
                                loadingOverlay.classList.add('hidden');
                            }
                        };
                        const hdPlayer = new JSMpeg.Player(`ws://localhost:${result.hdPort}`, { canvas: hdCanvas, autoplay: true, audio: true, volume: 0, disableWebAssembly: true, onPlay });
                        const sdPlayer = new JSMpeg.Player(`ws://localhost:${result.sdPort}`, { canvas: sdCanvas, autoplay: true, audio: true, volume: 0, disableWebAssembly: true, onPlay });
                        // START: ИСПРАВЛЕНИЕ - Инициализируем раздельную статистику
                        localPlayers[uniqueIdBase] = { hdPlayer, sdPlayer, cell: cellElement, stats: { hd: {}, sd: {} } };
                        // END: ИСПРАВЛЕНИЕ
                    } else {
                        handleStreamDeath({ uniqueStreamIdentifier: uniqueIdBase, error: result.error || App.i18n.t('unknown_error') });
                    }
                    attachControlEvents(cellElement, index);
                    attachPtzEvents(cellElement, index);
                    cellElement.classList.add('active');
                }
                if (gridState) {
                    gridState.forEach((cellState, i) => {
                        if (cellState) {
                            const uniqueIdBase = `stream-${cellState.camera.id}_${i}`;
                            const playerData = localPlayers[uniqueIdBase];
                            if (!playerData) return;
                            const cellElement = playerData.cell;
                            const camera = cameras.find(c => c.id === cellState.camera.id);
                            const hdCanvas = cellElement.querySelector('.video-canvas-hd');
                            const sdCanvas = cellElement.querySelector('.video-canvas-sd');
                            if (hdCanvas && sdCanvas) {
                                hdCanvas.classList.toggle('hidden', cellState.streamId !== 0);
                                sdCanvas.classList.toggle('hidden', cellState.streamId !== 1);
                            }
                            const nameDiv = cellElement.querySelector('.cell-name');
                            if (nameDiv && camera) {
                                const qualityLabel = cellState.streamId === 0 ? 'HD' : 'SD';
                                nameDiv.textContent = `${camera.name} (${qualityLabel})`;
                            }
                            const streamSwitchBtn = cellElement.querySelector('.stream-switch-btn');
                            if(streamSwitchBtn) streamSwitchBtn.innerHTML = cellState.streamId === 0 ? '<i class="material-icons">hd</i>' : '<i class="material-icons">sd</i>';
                            const recordBtn = cellElement.querySelector('.record-btn');
                            if (recordBtn) recordBtn.classList.toggle('recording', !!recordingStates[camera.id]);
                            
                            // START: ИСПРАВЛЕНИЕ - Обновляем текст статистики при переключении
                            updateStatsText(playerData);
                            // END: ИСПРАВЛЕНИЕ
                        }
                    });
                }
                const occupiedCellIndexes = new Set();
                gridState.forEach((cellState, i) => { if (cellState) occupiedCellIndexes.add(i); });
                gridCells.forEach((cell, i) => { if (!occupiedCellIndexes.has(i)) { const playerKey = Object.keys(localPlayers).find(k => k.endsWith(`_${i}`)); if (playerKey) destroyPlayerSet(playerKey); else if(cell.classList.contains('active')) { cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`; cell.classList.remove('active'); cell.draggable = false; } } });
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
            const baseId = uniqueStreamIdentifier.substring(0, uniqueStreamIdentifier.lastIndexOf('_'));
            const playerInfo = localPlayers[baseId];

            if (playerInfo && playerInfo.cell) {
                // НЕ удаляем плеер, чтобы сохранить статистику второго потока
                // delete localPlayers[baseId]; 
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
                    newRetryBtn.onclick = () => { console.log(`[Grid] Retrying stream ${baseId}...`); render(); };
                    newCloseBtn.onclick = () => { console.log(`[Grid] Closing errored cell for stream ${baseId}...`); const cellIndex = parseInt(playerInfo.cell.dataset.cellId, 10); const newGrid = getGridState().map(g => g ? { ...g } : null); newGrid[cellIndex] = null; stateManager.updateGridState(newGrid); };
                }
            }
        }
        
        async function restartStreamsForCamera(cameraId) {
            console.log(`[Grid] Restarting all streams for camera ID: ${cameraId}`);
            const gridState = getGridState();
            const streamsToRestart = [];
            gridState.forEach((cellState, i) => {
                if (cellState && cellState.camera.id === cameraId) {
                    const uniqueIdBase = `stream-${cellState.camera.id}_${i}`;
                    if (localPlayers[uniqueIdBase]) {
                        streamsToRestart.push(uniqueIdBase);
                    }
                }
            });
            for (const idBase of streamsToRestart) {
                await destroyPlayerSet(idBase);
            }
            setTimeout(() => render(), 100);
        }

        // START: ИСПРАВЛЕНИЕ - Логика отображения статистики
        function updateStatsText(playerData) {
            if (!playerData || !playerData.cell || !playerData.stats) return;
            const statsDiv = playerData.cell.querySelector('.cell-stats');
            if (!statsDiv) return;

            const cellIndex = parseInt(playerData.cell.dataset.cellId, 10);
            const cellState = getGridState()[cellIndex];
            if (!cellState) {
                statsDiv.textContent = '';
                return;
            }

            const activeStreamType = cellState.streamId === 0 ? 'hd' : 'sd';
            const statsToDisplay = playerData.stats[activeStreamType] || {};

            const { codec = '', resolution = '', fps = 0, bitrate = 0 } = statsToDisplay;
            const staticInfo = [codec.toUpperCase(), resolution].filter(Boolean).join(', ');
            const dynamicInfo = `${Math.round(fps)}fps, ${Math.round(bitrate)}kbps`;
            statsDiv.textContent = [staticInfo, dynamicInfo].filter(Boolean).join(' | ');
        }
        
        function updateStreamInfo({ uniqueStreamIdentifier, codec, resolution }) {
            console.log(`[DEBUG STATS] updateStreamInfo called for ${uniqueStreamIdentifier}`);
            const baseId = uniqueStreamIdentifier.substring(0, uniqueStreamIdentifier.lastIndexOf('_'));
            const playerData = localPlayers[baseId];
            if (playerData) {
                const streamType = uniqueStreamIdentifier.endsWith('_0') ? 'hd' : 'sd';
                console.log(`[DEBUG STATS] Found playerData for baseId ${baseId}. Updating ${streamType} stats.`);
                playerData.stats[streamType].codec = codec;
                playerData.stats[streamType].resolution = resolution;
                updateStatsText(playerData);
            } else {
                console.log(`[DEBUG STATS] No playerData found for baseId ${baseId}.`);
            }
        }
        
        function updateStreamStats({ uniqueStreamIdentifier, fps, bitrate }) {
            console.log(`[DEBUG STATS] updateStreamStats called for ${uniqueStreamIdentifier}`);
            const baseId = uniqueStreamIdentifier.substring(0, uniqueStreamIdentifier.lastIndexOf('_'));
            const playerData = localPlayers[baseId];
            if (playerData) {
                const streamType = uniqueStreamIdentifier.endsWith('_0') ? 'hd' : 'sd';
                console.log(`[DEBUG STATS] Found playerData for baseId ${baseId}. Updating ${streamType} stats.`);
                playerData.stats[streamType].fps = fps;
                playerData.stats[streamType].bitrate = bitrate;
                updateStatsText(playerData);
            } else {
                console.log(`[DEBUG STATS] No playerData found for baseId ${baseId}.`);
            }
        }
        // END: ИСПРАВЛЕНИЕ

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