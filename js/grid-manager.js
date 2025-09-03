// --- START OF FILE js/grid-manager.js ---
(function(window) {
    window.AppModules = window.AppModules || {};

    window.AppModules.createGridManager = function(App) {
        const stateManager = App.stateManager;
        const gridContainer = document.getElementById('grid-container');
        const layoutControls = document.getElementById('layout-controls');
        const MAX_GRID_SIZE = 64;

        let gridCells = [];
        let fullscreenCellIndex = null;
        let isRendering = false;
        const analyticsState = {};
        let animationFrameId = null;

        const peerConnections = {};
        const streamStatsHistory = {};
        let statsUpdateInterval = null;

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
                    // Для каждой ячейки храним два PeerConnection и два MediaStream: SD и HD
                    const mediaStreams = {};    // { cellIndex: { 0: streamHD, 1: streamSD } }
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
                        const videoPlayer = cellElement.querySelector('.video-player');
                        const overlayCanvas = cellElement.querySelector('.overlay-canvas');
                        if (!videoPlayer || !overlayCanvas) continue;
                        if (overlayCanvas.width !== videoPlayer.clientWidth || overlayCanvas.height !== videoPlayer.clientHeight) {
                            overlayCanvas.width = videoPlayer.clientWidth;
                            overlayCanvas.height = videoPlayer.clientHeight;
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
                const video = cell.querySelector('.video-player');
                if (!video || !video.srcObject) {
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
                    stopPlayerForCell(i);
                    cell.style.display = 'none';
                }
            });
            updateActiveLayoutButton();
        }
        
        function stopPlayerForCell(cellIndex) {
            const pc = peerConnections[cellIndex];
            if (pc) {
                pc.close();
                delete peerConnections[cellIndex];
            }
            const cell = gridCells[cellIndex];
            if (cell) {
                const video = cell.querySelector('.video-player');
                if (video && video.srcObject) {
                    video.srcObject.getTracks().forEach(track => track.stop());
                    video.srcObject = null;
                    delete video.dataset.streamPath;
                }
            }
        }

        function attachAllEvents(cellElement, cellIndex) {
            const cellState = getGridState()[cellIndex];
            const currentUser = App.stateManager.state.currentUser;

            cellElement.ondblclick = () => toggleFullscreen(cellIndex);
            cellElement.ondragover = (e) => { e.preventDefault(); cellElement.classList.add('drag-over'); };
            cellElement.ondragleave = () => { cellElement.classList.remove('drag-over'); };
            cellElement.ondrop = (e) => {
                e.preventDefault();
                cellElement.classList.remove('drag-over');
                if (currentUser?.role !== 'admin' && !currentUser?.permissions?.manage_layout) return;
                
                const newGrid = getGridState().map(g => g ? { ...g } : null);
                const cameraIdStr = e.dataTransfer.getData('application/x-camera-id');
                const sourceCellIdStr = e.dataTransfer.getData('application/x-source-cell-id');

                if (cameraIdStr) {
                    const cameraId = parseInt(cameraIdStr, 10);
                    if (isNaN(cameraId)) return;

                    if (sourceCellIdStr) {
                        const sourceCellId = parseInt(sourceCellIdStr, 10);
                        if (sourceCellId !== cellIndex) {
                            const sourceContent = newGrid[sourceCellId];
                            const targetContent = newGrid[cellIndex];
                            newGrid[cellIndex] = sourceContent;
                            newGrid[sourceCellId] = targetContent;
                        }
                    } else {
                        newGrid[cellIndex] = { camera: { id: cameraId }, streamId: 1 };
                    }
                    stateManager.updateGridState(newGrid);
                }
            };

            cellElement.oncontextmenu = (e) => {
                if (cellState && cellState.camera) {
                    e.preventDefault();
                    const cameraId = cellState.camera.id;
                    const menuItems = {
                        open_in_browser: `🌐  ${App.i18n.t('context_open_in_browser')}`,
                        files: `🗂️  ${App.i18n.t('context_file_manager')}`,
                        ssh: `💻  ${App.i18n.t('context_ssh')}`,
                        archive: `🗄️  ${App.i18n.t('archive_title')}`
                    };

                    if (currentUser.role === 'admin' || currentUser.permissions?.edit_cameras) {
                        menuItems.edit = `✏️  ${App.i18n.t('context_edit')}`;
                    }
                    if (currentUser.role === 'admin' || currentUser.permissions?.delete_cameras) {
                        menuItems.delete = `🗑️  ${App.i18n.t('context_delete')}`;
                    }
                    
                    window.api.showCameraContextMenu({ cameraId, labels: menuItems });
                }
            };

            // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ --- VVVVVV
            // Переносим обработчики кнопок внутрь, чтобы они всегда были актуальны
            const controls = cellElement.querySelector('.cell-controls');
            if (!controls) return;

            const streamSwitchBtn = controls.querySelector('.stream-switch-btn');
            if (streamSwitchBtn) {
                streamSwitchBtn.onclick = (e) => {
                    e.stopPropagation();
                    const currentGridState = getGridState()[cellIndex];
                    if (!currentGridState) return;

                    const newStreamId = currentGridState.streamId === 0 ? 1 : 0;
                    
                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    if (newGrid[cellIndex]) {
                        newGrid[cellIndex].streamId = newStreamId;
                        stateManager.updateGridState(newGrid);
                    }
                };
            }

            const pauseBtn = controls.querySelector('.pause-btn');
            if (pauseBtn) {
                pauseBtn.onclick = (e) => {
                    e.stopPropagation();
                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    if (!newGrid[cellIndex]) return;
                    
                    // Инвертируем состояние паузы
                    if (newGrid[cellIndex].paused) {
                        delete newGrid[cellIndex].paused;
                    } else {
                        newGrid[cellIndex].paused = true;
                    }
                    stateManager.updateGridState(newGrid);
                };
            }
            // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^

            if (!cellState) return;

            const screenshotBtn = controls.querySelector('.screenshot-btn');
            if (screenshotBtn) {
                screenshotBtn.onclick = async (e) => {
                    e.stopPropagation();
                    const video = cellElement.querySelector('.video-player');
                    if (!video || cellElement.classList.contains('paused-state') || !video.srcObject) {
                        App.modalHandler.showToast('Нельзя сделать скриншот, когда видео на паузе или не загружено.', true);
                        return;
                    }
                    const canvas = document.createElement('canvas');
                    canvas.width = video.videoWidth;
                    canvas.height = video.videoHeight;
                    const ctx = canvas.getContext('2d');
                    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);
                    
                    const dataUrl = canvas.toDataURL('image/jpeg');
                    const cameraName = cellState.camera.name || 'screenshot';

                    try {
                        const result = await window.api.saveScreenshot({ dataUrl, cameraName });
                        if (result.success) {
                            App.modalHandler.showToast(`Скриншот сохранен: ${result.path}`);
                        } else {
                            App.modalHandler.showToast(`Ошибка сохранения: ${result.error}`, true);
                        }
                    } catch (err) {
                        console.error("Ошибка вызова API сохранения скриншота:", err);
                        App.modalHandler.showToast(`Критическая ошибка: ${err.message}`, true);
                    }
                };
            }

            controls.querySelector('.fullscreen-btn').onclick = (e) => { e.stopPropagation(); toggleFullscreen(cellIndex); };

            controls.querySelector('.close-btn').onclick = (e) => {
                e.stopPropagation();
                const newGrid = getGridState().map(g => g ? { ...g } : null);
                newGrid[cellIndex] = null;
                stateManager.updateGridState(newGrid);
            };

            const audioBtn = controls.querySelector('.audio-btn');
            if (audioBtn) {
                audioBtn.onclick = (e) => {
                    e.stopPropagation();
                    const video = cellElement.querySelector('.video-player');
                    if (video) {
                        video.muted = !video.muted;
                        audioBtn.innerHTML = video.muted ? '<i class="material-icons">volume_off</i>' : '<i class="material-icons">volume_up</i>';
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

            const ptzButtons = cellElement.querySelectorAll('.ptz-btn');
            const sendPtzCommand = (command, action) => {
                window.api.ptzControl({ cameraId: cellState.camera.id, command, action });
            };
            ptzButtons.forEach(btn => {
                const command = btn.dataset.command;
                if (!command) return;
                btn.addEventListener('mousedown', (e) => { e.stopPropagation(); sendPtzCommand(command, 'start'); });
                btn.addEventListener('mouseup', (e) => { e.stopPropagation(); sendPtzCommand('stop', 'stop'); });
                btn.addEventListener('mouseleave', (e) => { if (e.buttons !== 1) { sendPtzCommand('stop', 'stop'); }});
            });
        }

        function createVideoPlayer(cell, streamUrl) {
            // Remove old video if exists
            const oldVideo = cell.querySelector('.video-player');
            if (oldVideo) cell.removeChild(oldVideo);
            const video = document.createElement('video');
            video.className = 'video-player';
            video.setAttribute('playsinline', '');
            video.setAttribute('controls', '');
            video.setAttribute('autoplay', '');
            video.style.width = '100%';
            video.style.height = '100%';
            // Detect device and stream type
            if (/\.m3u8($|\?)/.test(streamUrl)) {
                // HLS stream
                window.HlsLoader.attachHlsStream(video, streamUrl);
            } else {
                // Fallback: direct src
                video.src = streamUrl;
            }
            cell.appendChild(video);
            return video;
        }

        function getPreferredStreamUrl(cameraId) {
            // Проверяем доступность HLS потока
            const hlsUrl = 'http://127.0.0.1:8888/' + cameraId + '/hls.m3u8';
            // Проверяем доступность WebRTC (WHEP)
            const whepUrl = 'http://127.0.0.1:8889/' + cameraId + '/whep';
            // Для мобильных устройств и ТВ пробуем HLS, иначе WebRTC
            const isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini|SmartTV|GoogleTV/i.test(navigator.userAgent);
            // Попробуем HLS, если не доступен — fallback на WHEP
            return fetch(hlsUrl, { method: 'HEAD' })
                .then(resp => resp.ok ? hlsUrl : whepUrl)
                .catch(() => whepUrl);
        }

        async function attachStreamToCell(cell, cameraId) {
            cell.innerHTML = '';
            const streamUrl = await getPreferredStreamUrl(cameraId);
            createVideoPlayer(cell, streamUrl);
        }

        async function render() {
            if (isRendering) {
                return;
            }
            isRendering = true;
            try {
                updateGridLayoutView();
                const activeLayout = getActiveLayoutState();
                if (!activeLayout) {
                    gridCells.forEach((cell, i) => stopPlayerForCell(i));
                    isRendering = false;
                    return;
                }

                const { gridState } = activeLayout;
                const { cameras, recordingStates } = stateManager.state;

                for (let i = 0; i < gridCells.length; i++) {
                    const cellElement = gridCells[i];
                    const cellState = gridState[i];
                    
                    cellElement.setAttribute('draggable', 'false');
                    cellElement.ondragstart = null;

                    // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ: НОВАЯ ЛОГИКА РЕНДЕРА --- VVVVVV
                    if (cellState) {
                        const camera = cameras.find(c => c.id === cellState.camera.id);
                        if (!camera) {
                            stopPlayerForCell(i);
                            continue;
                        }

                        // Если в ячейке нет контента, создаем его
                        if (!cellElement.querySelector('.video-wrapper')) {
                            const template = document.getElementById('grid-cell-content-template');
                            const content = template.content.cloneNode(true);
                            cellElement.innerHTML = '';
                            cellElement.appendChild(content);
                            App.i18n.applyTranslationsToDOM(cellElement);
                        }

                        cellElement.setAttribute('draggable', 'true');
                        cellElement.ondragstart = (e) => {
                            e.stopPropagation();
                            e.dataTransfer.setData('application/x-camera-id', camera.id.toString());
                            e.dataTransfer.setData('application/x-source-cell-id', i.toString());
                        };

                        const videoElToUse = cellElement.querySelector('.video-player');
                        const currentStream = videoElToUse ? videoElToUse.dataset.streamPath : null;

                        if (cellState.paused) {
                            stopPlayerForCell(i);
                            cellElement.classList.add('paused-state');
                        } else {
                            cellElement.classList.remove('paused-state');
                            const streamId = cellState.streamId === 0 ? 0 : 1;
                            const streamPath = `cam${camera.id}_${streamId}`;

                            if (currentStream !== streamPath) {
                                videoElToUse.dataset.streamPath = streamPath;
                                startWebRTCPlayer(videoElToUse, streamPath, i, cellElement);
                            }
                            cellElement.classList.add('active');
                        }

                        // Обновляем UI элементы
                        const nameDiv = cellElement.querySelector('.cell-name');
                        if (nameDiv) {
                            const qualityLabel = (cellState.streamId === 0) ? 'HD' : 'SD';
                            nameDiv.textContent = `${camera.name} (${qualityLabel})`;
                        }
                        const streamSwitchBtn = cellElement.querySelector('.stream-switch-btn');
                        if(streamSwitchBtn) streamSwitchBtn.innerHTML = (cellState.streamId === 0) ? '<i class="material-icons">hd</i>' : '<i class="material-icons">sd</i>';
                        
                        const recordBtn = cellElement.querySelector('.record-btn');
                        if (recordBtn) recordBtn.classList.toggle('recording', !!recordingStates[camera.id]);

                        const pauseBtn = cellElement.querySelector('.pause-btn i');
                        if (pauseBtn) pauseBtn.textContent = cellState.paused ? 'play_arrow' : 'pause';

                    } else { // Если cellState пустой
                        stopPlayerForCell(i);
                        cellElement.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                        cellElement.classList.remove('active', 'error-state', 'paused-state');
                    }
                    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^
                    
                    attachAllEvents(cellElement, i);
                }
            } finally {
                isRendering = false;
            }
        }
        
        async function startWebRTCPlayer(videoElement, streamPath, cellIndex, cellElement) {
            const loadingOverlay = cellElement.querySelector('.loading-overlay');
            const errorOverlay = cellElement.querySelector('.error-overlay');
            
            loadingOverlay.classList.remove('hidden');
            errorOverlay.classList.add('hidden');
            cellElement.classList.remove('error-state');

            try {
                // Очищаем старый поток и PeerConnection
                if (videoElement.srcObject) {
                    videoElement.srcObject.getTracks().forEach(track => track.stop());
                    videoElement.srcObject = null;
                }
                if (peerConnections[cellIndex]) {
                    try { peerConnections[cellIndex].close(); } catch(e) {}
                    peerConnections[cellIndex] = null;
                }

                const pc = new RTCPeerConnection();
                peerConnections[cellIndex] = pc;

                pc.ontrack = (event) => {
                    if (!videoElement.srcObject) {
                        videoElement.srcObject = new MediaStream();
                    }
                    videoElement.srcObject.addTrack(event.track);
                };

                pc.addTransceiver('video', { 'direction': 'recvonly' });
                pc.addTransceiver('audio', { 'direction': 'recvonly' });

                await pc.setLocalDescription(await pc.createOffer());

                const response = await fetch(`http://127.0.0.1:8889/${streamPath}/whep`, {
                    method: 'POST',
                    body: pc.localDescription.sdp,
                    headers: { 'Content-Type': 'application/sdp' }
                });

                if (!response.ok) {
                    const errorText = await response.text();
                    // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ: Улучшенная обработка ошибок --- VVVVVV
                    let friendlyError = `WHEP: ${response.status} ${response.statusText}`;
                    try {
                        const errJson = JSON.parse(errorText);
                        if (errJson.error && errJson.error.includes('no one is publishing to path')) {
                            friendlyError = App.t('error_connection_refused');
                        } else if (errJson.error) {
                            friendlyError = errJson.error;
                        }
                    } catch(e) { /* ignore json parse error */ }
                    throw new Error(friendlyError);
                    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^
                }

                const answer = await response.text();
                await pc.setRemoteDescription({ type: 'answer', sdp: answer });

                videoElement.onplaying = () => {
                    loadingOverlay.classList.add('hidden');
                };

            } catch (e) {
                console.error(`[WebRTC] Failed to start player for ${streamPath}:`, e);
                errorOverlay.classList.remove('hidden');
                // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ: Отображаем понятную ошибку --- VVVVVV
                errorOverlay.querySelector('.error-message').textContent = e.message;
                // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^
                cellElement.classList.add('error-state');
                stopPlayerForCell(cellIndex);
            } finally {
                loadingOverlay.classList.add('hidden');
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

        function updateAllStats(data) {
            if (!data || !Array.isArray(data.items)) {
                return;
            }
        
            const gridState = getGridState();
            const now = Date.now();
        
            gridState.forEach((cellState, i) => {
                const cellElement = gridCells[i];
                if (!cellElement) return;
                const statsDiv = cellElement.querySelector('.cell-stats');
                if (!statsDiv) return;
        
                if (cellState && !cellState.paused) {
                    const streamId = cellState.streamId === 0 ? 0 : 1;
                    const pathName = `cam${cellState.camera.id}_${streamId}`;
                    
                    const pathData = data.items.find(item => item.name === pathName);
        
                    if (!pathData) {
                        return;
                    }
                    
                    let bitrate = '...';

                    if (pathData.bytesReceived) {
                        const history = streamStatsHistory[pathName];

                        if (history) {
                            const timeDiffSeconds = (now - history.lastTime) / 1000;
                            const byteDiff = pathData.bytesReceived - history.lastBytes;

                            if (timeDiffSeconds > 0 && byteDiff >= 0) {
                                const bytesPerSecond = byteDiff / timeDiffSeconds;
                                bitrate = `${Math.round((bytesPerSecond * 8) / 1000)}kbps`;
                            }
                        }

                        streamStatsHistory[pathName] = {
                            lastBytes: pathData.bytesReceived,
                            lastTime: now
                        };
                    }
                    
                    const videoTrackName = pathData.tracks?.find(t => !t.startsWith('G7') && !t.startsWith('Opus'));
                    const codec = videoTrackName || '...';
                    
                    const resolution = ''; 

                    let statsText = `${codec} | ${bitrate}`;
                    if (resolution) {
                        statsText = `${codec} | ${resolution} | ${bitrate}`;
                    }
                    
                    statsDiv.textContent = statsText;
                    statsDiv.style.display = 'block';
        
                } else {
                    statsDiv.style.display = 'none';
                }
            });
        }

        function init() {
            for (let i = 0; i < MAX_GRID_SIZE; i++) {
                const cell = document.createElement('div');
                cell.className = 'grid-cell';
                cell.dataset.cellId = i;
                cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                
                attachAllEvents(cell, i);
                
                gridContainer.appendChild(cell);
                gridCells.push(cell);
            }
            initializeLayoutControls();
            window.addEventListener('keydown', (e) => { if (e.key === 'Escape' && fullscreenCellIndex !== null) { toggleFullscreen(fullscreenCellIndex); } });
            window.addEventListener('language-changed', updatePlaceholdersLanguage);
            startRenderLoop();

            // Only register Electron API event if available
            if (window.api && typeof window.api.onMediamtxStatsUpdate === 'function') {
                window.api.onMediamtxStatsUpdate((statsData) => {
                    updateAllStats(statsData);
                });
            }
        }
        
        function handleAnalyticsUpdate(data) {
            const { cameraId, result } = data;
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
            handleAnalyticsUpdate
        };
    };
})(window);

// Replace direct srcObject assignment with adaptive stream attachment
// Example: when you need to show a stream in a cell
// attachStreamToCell(cell, streamUrl);
// Remove or refactor old srcObject logic for mobile compatibility