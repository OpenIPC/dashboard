// --- START OF FILE js/grid-manager.js ---

// js/grid-manager.js (Полная версия с переключением HD/SD)

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

        let restartAttempts = {};

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

            const { layout, gridState } = activeLayout;
            const totalVisibleCells = layout.cols * layout.rows;
            const cellWidth = 100 / layout.cols;
            const cellHeight = 100 / layout.rows;

            gridCells.forEach((cell, i) => {
                if (i < totalVisibleCells) {
                    const row = Math.floor(i / layout.cols);
                    const col = i % layout.cols;
                    cell.style.display = 'flex';
                    cell.style.top = `${row * cellHeight}%`;
                    cell.style.left = `${col * cellWidth}%`;
                    cell.style.width = `${cellWidth}%`;
                    cell.style.height = `${cellHeight}%`;
                } else {
                    const cellState = (gridState && gridState[i]) ? gridState[i] : null;
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
            
            // VVVVVV --- ИЗМЕНЕНИЕ: УПРОЩАЕМ ЛОГИКУ КНОПКИ HD/SD --- VVVVVV
            const streamSwitchBtn = controls.querySelector('.stream-switch-btn');
            if (streamSwitchBtn) {
                streamSwitchBtn.innerHTML = cellState.streamId === 0 ? '<i class="material-icons">hd</i>' : '<i class="material-icons">sd</i>';
                streamSwitchBtn.onclick = (e) => {
                    e.stopPropagation();
                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    if (newGrid[cellIndex]) {
                        newGrid[cellIndex].streamId = newGrid[cellIndex].streamId === 0 ? 1 : 0;
                        stateManager.updateGridState(newGrid);
                    }
                };
            }
            // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
            
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
                    App.toggleRecording(camera);
                };
            }
        }

        async function render() {
            updateGridLayoutView();

            const activeLayout = getActiveLayoutState();
            if (!activeLayout) {
                for (const id in localPlayers) await destroyPlayer(id);
                return;
            }

            const { gridState } = activeLayout;
            const { cameras, recordingStates } = stateManager.state;
            const desiredStreams = new Set();
            if (gridState) {
                gridState.forEach((cell, index) => {
                    if (cell) desiredStreams.add(`stream-${cell.camera.id}_${cell.streamId}_${index}`);
                });
            }

            for (const id in localPlayers) {
                if (!desiredStreams.has(id)) await destroyPlayer(id);
            }

            const occupiedCells = new Set();

            if (gridState) {
                for (let i = 0; i < gridState.length; i++) {
                    const cellState = gridState[i];
                    if (!cellState) continue;

                    const cellElement = gridCells[i];
                    occupiedCells.add(cellElement);

                    const camera = cameras.find(c => c.id === cellState.camera.id);
                    if (!camera) continue;

                    const uniqueStreamIdentifier = `stream-${camera.id}_${cellState.streamId}_${i}`;

                    if (!localPlayers[uniqueStreamIdentifier]) {
                        localPlayers[uniqueStreamIdentifier] = { player: null, cell: cellElement };

                        const template = document.getElementById('grid-cell-content-template');
                        const content = template.content.cloneNode(true);
                        
                        const nameDiv = content.querySelector('.cell-name');
                        const statsDiv = content.querySelector('.cell-stats');
                        
                        // VVVVVV --- ИЗМЕНЕНИЕ: ОБНОВЛЯЕМ НАЗВАНИЕ В ЯЧЕЙКЕ --- VVVVVV
                        const qualityLabel = cellState.streamId === 0 ? 'HD' : 'SD';
                        nameDiv.textContent = `${camera.name} (${qualityLabel})`;
                        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
                        
                        statsDiv.id = `stats-${uniqueStreamIdentifier}`;

                        cellElement.innerHTML = '';
                        cellElement.appendChild(content);
                        cellElement.querySelector('.video-wrapper').innerHTML += `<span>${App.i18n.t('connecting')}</span>`;

                        const result = await window.api.startVideoStream({ 
                            credentials: camera, 
                            streamId: cellState.streamId,
                            uniqueStreamIdentifier: uniqueStreamIdentifier 
                        });

                        if (!localPlayers[uniqueStreamIdentifier]) {
                            await window.api.stopVideoStream(uniqueStreamIdentifier);
                            continue;
                        }

                        if (result.success) {
                            if (restartAttempts[uniqueStreamIdentifier]) {
                                delete restartAttempts[uniqueStreamIdentifier];
                            }
                            const videoCanvas = cellElement.querySelector('.video-canvas');
                            const connectingSpan = cellElement.querySelector('span');
                            if(connectingSpan) connectingSpan.remove();

                            const player = new JSMpeg.Player(`ws://localhost:${result.wsPort}`, {
                                canvas: videoCanvas,
                                autoplay: true,
                                audio: true,
                                volume: 0,
                                disableWebAssembly: true
                            });
                            localPlayers[uniqueStreamIdentifier].player = player;
                        } else {
                            const videoWrapper = cellElement.querySelector('.video-wrapper');
                            if (videoWrapper) {
                                videoWrapper.innerHTML = `<span>${App.i18n.t('error')}: ${result.error || App.i18n.t('unknown_error')}</span>`;
                            }
                            delete localPlayers[uniqueStreamIdentifier];
                        }
                    }

                    attachControlEvents(cellElement, i);
                    
                    const recordBtn = cellElement.querySelector('.record-btn');
                    if (recordBtn) recordBtn.classList.toggle('recording', !!recordingStates[camera.id]);

                    cellElement.classList.add('active');
                    const currentUser = stateManager.state.currentUser;
                    cellElement.draggable = currentUser?.role === 'admin' || currentUser?.permissions?.manage_layout;

                    if (cellElement._dragStartHandler) cellElement.removeEventListener('dragstart', cellElement._dragStartHandler);
                    cellElement._dragStartHandler = (e) => {
                        e.dataTransfer.setData("application/x-grid-cell-index", i.toString());
                        e.dataTransfer.effectAllowed = 'move';
                    };
                    cellElement.addEventListener('dragstart', cellElement._dragStartHandler);
                }
            }

            gridCells.forEach(cell => {
                if (!occupiedCells.has(cell)) {
                    if (cell.classList.contains('active')) {
                        cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                        cell.classList.remove('active');
                        cell.draggable = false;
                    }
                }
            });
        }

        // VVVVVV --- ИЗМЕНЕНИЕ: ГЛАВНАЯ ЛОГИКА ПЕРЕКЛЮЧЕНИЯ ПОТОКА --- VVVVVV
        function toggleFullscreen(cellIndex) {
            const cell = gridCells[cellIndex];
            if (!cell) return;

            const newGrid = getGridState().map(g => g ? { ...g } : null);
            const cellState = newGrid[cellIndex];
            if (!cellState) return;

            const isCurrentlyFullscreen = cell.classList.contains('fullscreen');
            const fsBtnIcon = cell.querySelector('.fullscreen-btn i');

            if (isCurrentlyFullscreen) {
                // Выходим из полноэкранного режима, возвращаем SD поток
                cellState.streamId = 1; // SD поток
                gridContainer.classList.remove('fullscreen-mode');
                cell.classList.remove('fullscreen');
                if (fsBtnIcon) fsBtnIcon.textContent = 'fullscreen';
                fullscreenCellIndex = null;
            } else {
                // Входим в полноэкранный режим, включаем HD поток
                if (fullscreenCellIndex !== null) {
                    // Если другая ячейка была в fullscreen, сворачиваем ее и возвращаем в SD
                    const oldFullscreenCell = gridCells[fullscreenCellIndex];
                    if (oldFullscreenCell && newGrid[fullscreenCellIndex]) {
                        newGrid[fullscreenCellIndex].streamId = 1; // SD поток
                        oldFullscreenCell.classList.remove('fullscreen');
                        const oldFsBtnIcon = oldFullscreenCell.querySelector('.fullscreen-btn i');
                        if (oldFsBtnIcon) oldFsBtnIcon.textContent = 'fullscreen';
                    }
                }
                
                cellState.streamId = 0; // HD поток
                fullscreenCellIndex = cellIndex;
                gridContainer.classList.add('fullscreen-mode');
                cell.classList.add('fullscreen');
                if (fsBtnIcon) fsBtnIcon.textContent = 'fullscreen_exit';
            }

            // Сохраняем новое состояние сетки. Это вызовет перерисовку.
            stateManager.updateGridState(newGrid);
        }
        // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

        function init() {
            for (let i = 0; i < MAX_GRID_SIZE; i++) {
                const cell = document.createElement('div');
                cell.className = 'grid-cell';
                cell.dataset.cellId = i;
                cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                cell.ondblclick = () => toggleFullscreen(i);
                cell.addEventListener('dragover', (e) => {
                    e.preventDefault();
                    cell.classList.add('drag-over');
                });
                cell.addEventListener('dragleave', () => {
                    cell.classList.remove('drag-over');
                });
                
                cell.addEventListener('drop', (e) => {
                    e.preventDefault();
                    cell.classList.remove('drag-over');

                    const currentUser = App.stateManager.state.currentUser;
                    if (currentUser?.role !== 'admin' && !currentUser?.permissions?.manage_layout) {
                        return;
                    }

                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    const targetIndex = i;

                    const cameraIdStr = e.dataTransfer.getData('application/x-camera-id');
                    if (cameraIdStr) {
                        const cameraId = parseInt(cameraIdStr, 10);
                        if (!isNaN(cameraId)) {
                            // VVVVVV --- ИЗМЕНЕНИЕ: ДОБАВЛЯЕМ КАМЕРУ СРАЗУ В SD --- VVVVVV
                            newGrid[targetIndex] = { camera: { id: cameraId }, streamId: 1 }; // По умолчанию SD поток
                            // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
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
            window.addEventListener('keydown', (e) => {
                if (e.key === 'Escape' && fullscreenCellIndex !== null) {
                    toggleFullscreen(fullscreenCellIndex);
                }
            });
            window.addEventListener('language-changed', updatePlaceholdersLanguage);

            window.api.onAnalyticsUpdate(({ cameraId, result }) => {
                const gridState = getGridState();
                gridState.forEach((cell, cellIndex) => {
                    if (cell && cell.camera.id === cameraId) {
                        const cellElement = gridCells[cellIndex];
                        if (!cellElement) return;

                        const videoWrapper = cellElement.querySelector('.video-wrapper');
                        if (!videoWrapper) return;
                        
                        const videoCanvas = videoWrapper.querySelector('.video-canvas');
                        const overlayCanvas = videoWrapper.querySelector('.overlay-canvas');
                        if (!videoCanvas || !overlayCanvas) return;

                        if (overlayCanvas.width !== videoCanvas.clientWidth || overlayCanvas.height !== videoCanvas.clientHeight) {
                            overlayCanvas.width = videoCanvas.clientWidth;
                            overlayCanvas.height = videoCanvas.clientHeight;
                        }
                        
                        const ctx = overlayCanvas.getContext('2d');
                        
                        setTimeout(() => {
                             ctx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
                        }, 1000);

                        ctx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);

                        if (result.status === 'objects_detected' && result.objects) {
                            const scaleX = overlayCanvas.width / result.frame_width;
                            const scaleY = overlayCanvas.height / result.frame_height;

                            result.objects.forEach(obj => {
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
                });
            });
        }

        async function handleStreamDeath(uniqueStreamIdentifier) {
            console.log(`[Grid] Stream ${uniqueStreamIdentifier} died. Cleaning up.`);
            if (localPlayers[uniqueStreamIdentifier]) {
                const playerInfo = localPlayers[uniqueStreamIdentifier];
                delete localPlayers[uniqueStreamIdentifier]; 
                
                restartAttempts[uniqueStreamIdentifier] = (restartAttempts[uniqueStreamIdentifier] || 0) + 1;
                
                const delay = Math.min(5000 * Math.pow(2, restartAttempts[uniqueStreamIdentifier] - 1), 60000);
                
                const reconnectingMessage = `${App.t('stream_died_reconnecting')} (попытка #${restartAttempts[uniqueStreamIdentifier]}, след. через ${delay / 1000}с)`;

                if (playerInfo.cell) {
                    playerInfo.cell.innerHTML = `<span><i class="material-icons">error_outline</i><br>${reconnectingMessage}</span>`;
                }
                
                console.log(`[Grid] Will attempt to restart stream ${uniqueStreamIdentifier} in ${delay / 1000}s.`);
                
                setTimeout(() => {
                    const currentState = getGridState();
                    const streamParts = uniqueStreamIdentifier.split('_');
                    const cellIndex = parseInt(streamParts[streamParts.length - 1], 10);
                    
                    const cellState = currentState[cellIndex];
                    const needsRestart = cellState && `stream-${cellState.camera.id}_${cellState.streamId}_${cellIndex}` === uniqueStreamIdentifier;
                    
                    if (needsRestart) {
                        console.log(`[Grid] Executing restart for stream ${uniqueStreamIdentifier}.`);
                        render();
                    } else {
                        console.log(`[Grid] Stream ${uniqueStreamIdentifier} no longer needed. Cancelling restart.`);
                        delete restartAttempts[uniqueStreamIdentifier];
                    }
                }, delay);
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
        
        function updateStreamStats({ uniqueStreamIdentifier, fps, bitrate }) {
            const statsDiv = document.getElementById(`stats-${uniqueStreamIdentifier}`);
            if (statsDiv) {
                statsDiv.textContent = `${Math.round(fps)}fps, ${Math.round(bitrate)}kbps`;
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
            updateStreamStats
        };
    };
})(window);

// --- END OF FILE js/grid-manager.js ---