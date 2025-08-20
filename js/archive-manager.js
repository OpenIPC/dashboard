// --- START OF FILE js/archive-manager.js ---
(function(window) {
    'use strict';
    window.AppModules = window.AppModules || {};

    window.AppModules.createArchiveManager = function(App) {
        // --- DOM Элементы ---
        const archiveView = document.getElementById('archive-view');
        const mainView = document.getElementById('main-view');
        const backBtn = document.getElementById('archive-back-btn');
        const cameraNameEl = document.getElementById('archive-camera-name');
        const datePickerEl = document.getElementById('archive-date-picker');
        const videoPlayer = document.getElementById('archive-video-player');
        const placeholder = document.getElementById('archive-video-placeholder');
        const timelineWrapper = document.getElementById('timeline-wrapper');
        const timelineCanvas = document.getElementById('timeline-canvas');
        const timelineCtx = timelineCanvas.getContext('2d');
        const timelineLabelsEl = document.getElementById('timeline-labels');
        const eventListEl = document.getElementById('event-list');
        const filtersContainer = document.getElementById('archive-filters');
        const exportBtn = document.getElementById('archive-export-btn');
        const playPauseBtn = document.getElementById('ac-play-pause-btn');
        const speedBtn = document.getElementById('ac-speed-btn');
        const timeDisplay = document.getElementById('ac-time-display');

        // --- Константы и состояние ---
        const DAY_IN_SECONDS = 86400;
        const PLAYBACK_SPEEDS = [1, 2, 4, 8, 16];
        const MIN_ZOOM = 1;
        const MAX_ZOOM = 24 * 12;

        const COLORS = {
            background: '#2d333b',
            label: 'rgba(173, 181, 189, 0.7)',
            recording: 'rgba(13, 110, 253, 0.7)',
            recordingHover: 'rgba(13, 110, 253, 1)',
            selection: 'rgba(255, 255, 0, 0.4)',
            seeker: 'rgba(255, 255, 255, 0.9)',
            eventPerson: '#f85149',
            eventCar: '#ffc107',
            eventDefault: '#6c757d'
        };

        let currentCamera = null;
        let calendarInstance = null;
        let recordingsForDay = [];
        let allCameraEventsForDay = [];
        let activeFilters = new Set();
        
        let hls = null; 
        let isPlaying = false;
        let currentSpeedIndex = 0;
        let currentTime = 0;
        let animationFrameId = null;
        
        let isSelecting = false;
        let selectionStartTime = 0;
        let selectionEndTime = 0;
        let zoomLevel = 1;
        let viewStartSeconds = 0;
        let seekerTime = -1;
        let mouseTime = -1;
        
        let isDragging = false;
        let lastMouseX = 0;
        
        let pendingSeekTime = -1;

        // --- Утилиты ---
        const formatTime = (totalSeconds) => {
            const h = Math.floor(totalSeconds / 3600).toString().padStart(2, '0');
            const m = Math.floor((totalSeconds % 3600) / 60).toString().padStart(2, '0');
            const s = Math.floor(totalSeconds % 60).toString().padStart(2, '0');
            return `${h}:${m}:${s}`;
        };

        const createLocalDateFromString = (timeString) => {
            const [datePart, timePart] = timeString.split('T');
            const [year, month, day] = datePart.split('-');
            const [hour, minute, second] = timePart.split('-');
            return new Date(year, month - 1, day, hour, minute, second);
        };
        
        // --- Логика плеера ---
        function play() {
            if (isPlaying) return;
            isPlaying = true;
            playPauseBtn.innerHTML = `<i class="material-icons">pause</i>`;
            const playPromise = videoPlayer.play();
            if (playPromise !== undefined) {
                playPromise.catch(error => {
                    if (error.name !== 'AbortError') console.error("Play error:", error);
                });
            }
        }

        function pause() {
            if (!isPlaying) return;
            isPlaying = false;
            playPauseBtn.innerHTML = `<i class="material-icons">play_arrow</i>`;
            videoPlayer.pause();
        }

        function togglePlayPause() {
            if (videoPlayer.paused) {
                if (videoPlayer.ended) {
                    seek(currentTime, true);
                } else {
                    play();
                }
            } else {
                pause();
            }
        }
        
        function changeSpeed() {
            currentSpeedIndex = (currentSpeedIndex + 1) % PLAYBACK_SPEEDS.length;
            const newSpeed = PLAYBACK_SPEEDS[currentSpeedIndex];
            videoPlayer.playbackRate = newSpeed;
            speedBtn.textContent = `${newSpeed.toFixed(1)}x`;
        }
        
        function getStartOfDay() {
            if (!calendarInstance || !calendarInstance.selectedDates.length) {
                const today = new Date();
                today.setHours(0,0,0,0);
                return today;
            };
            const selectedDate = new Date(calendarInstance.selectedDates[0]);
            selectedDate.setHours(0, 0, 0, 0);
            return selectedDate;
        }

        async function seek(timeInSeconds, startPlaying = false) {
            pause();
            currentTime = Math.max(0, Math.min(timeInSeconds, DAY_IN_SECONDS));
            seekerTime = currentTime;
            
            const targetBlock = recordingsForDay.find(rec => {
                const start = (createLocalDateFromString(rec.startTimeString).getTime() - getStartOfDay().getTime()) / 1000;
                // VVVVVV --- ИЗМЕНЕНИЕ: УДАЛЯЕМ ЗАГЛУШКУ --- VVVVVV
                const end = start + rec.duration;
                // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
                return currentTime >= start && currentTime < end;
            });
        
            if (!targetBlock) {
                App.modalHandler.showToast(App.t('archive_no_recordings_for_time'), true);
                return;
            }

            placeholder.textContent = 'Подготовка видео...';
            placeholder.classList.remove('hidden');
            videoPlayer.classList.add('hidden');

            const blockStart = (createLocalDateFromString(targetBlock.startTimeString).getTime() - getStartOfDay().getTime()) / 1000;
            const seekInFile = currentTime - blockStart;
            
            pendingSeekTime = seekInFile;

            if (hls) {
                try {
                    const result = await window.api.prepareArchiveForHls(targetBlock.name);
                    if (!result.success) throw new Error(result.error);
                    
                    hls.loadSource(result.url);
                    hls.attachMedia(videoPlayer);
                    if (startPlaying) {
                       videoPlayer.addEventListener('canplay', play, { once: true });
                    }
                } catch (error) {
                    console.error('HLS preparation failed:', error);
                    App.modalHandler.showToast(`Ошибка подготовки HLS: ${error.message}`, true);
                    placeholder.textContent = 'Ошибка загрузки HLS';
                    pendingSeekTime = -1;
                }
            } else {
                const newSrc = `video-archive://${encodeURIComponent(targetBlock.name)}`;
                videoPlayer.src = newSrc;
                videoPlayer.load();
                if (startPlaying) videoPlayer.play();
            }
        }

        function updateLoop() {
            drawUI();
            requestAnimationFrame(updateLoop);
        }
        
        function drawUI() {
            drawTimeline();
            timeDisplay.textContent = formatTime(seekerTime >= 0 ? seekerTime : currentTime);
        }

        function drawTimeline() {
            const rect = timelineWrapper.getBoundingClientRect();
            if (rect.width === 0 || rect.height === 0) return;
            const dpr = window.devicePixelRatio || 1;
            const canvasWidth = rect.width;
            const canvasHeight = rect.height;
            timelineCanvas.width = canvasWidth * dpr;
            timelineCanvas.height = canvasHeight * dpr;
            timelineCtx.setTransform(1, 0, 0, 1, 0, 0);
            timelineCtx.scale(dpr, dpr);
            timelineCtx.fillStyle = COLORS.background;
            timelineCtx.fillRect(0, 0, canvasWidth, canvasHeight);
            const totalVisibleSeconds = DAY_IN_SECONDS / zoomLevel;
            
            recordingsForDay.forEach(rec => {
                const recDate = createLocalDateFromString(rec.startTimeString);
                const startOfDay = getStartOfDay();
                const startTimeInSeconds = (recDate.getTime() - startOfDay.getTime()) / 1000;
                // VVVVVV --- ИЗМЕНЕНИЕ: УДАЛЯЕМ ЗАГЛУШКУ --- VVVVVV
                const durationInSeconds = rec.duration;
                // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
                const endTimeInSeconds = startTimeInSeconds + durationInSeconds;
                if (endTimeInSeconds < viewStartSeconds || startTimeInSeconds > viewStartSeconds + totalVisibleSeconds) return;
                const x = ((startTimeInSeconds - viewStartSeconds) / totalVisibleSeconds) * canvasWidth;
                const w = (durationInSeconds / totalVisibleSeconds) * canvasWidth;
                const isHovered = mouseTime >= startTimeInSeconds && mouseTime < endTimeInSeconds;
                timelineCtx.fillStyle = isHovered ? COLORS.recordingHover : COLORS.recording;
                timelineCtx.fillRect(x, canvasHeight * 0.25, Math.max(1, w), canvasHeight * 0.5);
            });
            
            allCameraEventsForDay.forEach(event => {
                const eventDate = new Date(event.timestamp * 1000);
                const startOfDay = getStartOfDay();
                const startTimeInSeconds = (eventDate.getTime() - startOfDay.getTime()) / 1000;
                const durationInSeconds = event.duration || 30;
                const endTimeInSeconds = startTimeInSeconds + durationInSeconds;

                if (endTimeInSeconds < viewStartSeconds || startTimeInSeconds > viewStartSeconds + totalVisibleSeconds) return;
                
                if (activeFilters.size > 0 && !event.objects.some(obj => activeFilters.has(obj))) return;

                const mainObjectType = event.objects?.[0];
                timelineCtx.fillStyle = (mainObjectType === 'person') ? COLORS.eventPerson : (mainObjectType === 'car' ? COLORS.eventCar : COLORS.eventDefault);
                
                const x = ((startTimeInSeconds - viewStartSeconds) / totalVisibleSeconds) * canvasWidth;
                const w = (durationInSeconds / totalVisibleSeconds) * canvasWidth;

                timelineCtx.fillRect(x, 0, Math.max(1, w), canvasHeight);
            });

            if (isSelecting || (selectionEndTime - selectionStartTime > 0)) {
                const start = Math.min(selectionStartTime, selectionEndTime);
                const end = Math.max(selectionStartTime, selectionEndTime);
                const x = ((start - viewStartSeconds) / totalVisibleSeconds) * canvasWidth;
                const w = ((end - start) / totalVisibleSeconds) * canvasWidth;
                timelineCtx.fillStyle = COLORS.selection;
                timelineCtx.fillRect(x, 0, w, canvasHeight);
            }
            if (seekerTime >= viewStartSeconds && seekerTime < viewStartSeconds + totalVisibleSeconds) {
                const x = ((seekerTime - viewStartSeconds) / totalVisibleSeconds) * canvasWidth;
                timelineCtx.fillStyle = COLORS.seeker;
                timelineCtx.fillRect(x - 1, 0, 2, canvasHeight);
            }
        }

        function syncUI() {
            renderTimelineLabels();
            drawTimeline();
        }

        function renderTimelineLabels() {
            timelineLabelsEl.innerHTML = '';
            const totalVisibleSeconds = DAY_IN_SECONDS / zoomLevel;
            
            let step;
            if (zoomLevel <= 2) step = 3600 * 2; else if (zoomLevel <= 4) step = 3600; else if (zoomLevel <= 12) step = 1800; else if (zoomLevel <= 24) step = 900; else if (zoomLevel <= 48) step = 300; else if (zoomLevel <= 96) step = 180; else step = 60;
            const totalLabels = DAY_IN_SECONDS / step;
            if (totalLabels > 500) return;

            const firstVisibleSecond = Math.floor(viewStartSeconds / step) * step;
            const lastVisibleSecond = viewStartSeconds + totalVisibleSeconds;
            
            for (let s = firstVisibleSecond; s <= lastVisibleSecond; s += step) {
                 if (s >= DAY_IN_SECONDS) break;
                const label = document.createElement('span');
                const hour = Math.floor(s / 3600);
                const minute = Math.floor((s % 3600) / 60);
                label.textContent = `${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}`;
                
                const pixelPos = ((s - viewStartSeconds) / totalVisibleSeconds) * timelineWrapper.clientWidth;
                label.style.left = `${pixelPos}px`;
                timelineLabelsEl.appendChild(label);
            }
        }
        
        function getTimeFromMouseEvent(e) {
            const rect = timelineWrapper.getBoundingClientRect();
            const mouseX = e.clientX - rect.left;
            const totalVisibleSeconds = DAY_IN_SECONDS / zoomLevel;
            return viewStartSeconds + (mouseX / rect.width) * totalVisibleSeconds;
        }
        
        function handleTimelineMouseDown(e) {
            if (e.button === 0) {
                isSelecting = true;
                const clickTime = getTimeFromMouseEvent(e);
                selectionStartTime = clickTime;
                selectionEndTime = clickTime;
                exportBtn.disabled = true;
                isDragging = true;
                lastMouseX = e.clientX;
                timelineWrapper.classList.add('grabbing');
                drawTimeline();
            }
        }

        function handleTimelineMouseMove(e) {
            mouseTime = getTimeFromMouseEvent(e);
            if (isDragging) {
                if (Math.abs(e.clientX - lastMouseX) > 2) {
                    isSelecting = false;
                    selectionStartTime = 0;
                    selectionEndTime = 0;
                }
                const deltaX = e.clientX - lastMouseX;
                lastMouseX = e.clientX;
                const totalVisibleSeconds = DAY_IN_SECONDS / zoomLevel;
                const secondsPerPixel = totalVisibleSeconds / timelineWrapper.clientWidth;
                viewStartSeconds -= deltaX * secondsPerPixel;
                const maxViewStart = DAY_IN_SECONDS - totalVisibleSeconds;
                viewStartSeconds = Math.max(0, Math.min(viewStartSeconds, maxViewStart < 0 ? 0 : maxViewStart));
                syncUI();
            } else if (isSelecting) {
                selectionEndTime = getTimeFromMouseEvent(e);
                drawTimeline();
            } else {
                drawTimeline();
            }
        }

        function handleTimelineMouseUp(e) {
            if (isDragging) {
                isDragging = false;
                timelineWrapper.classList.remove('grabbing');
            }
            if (isSelecting) {
                isSelecting = false;
                if (Math.abs(selectionEndTime - selectionStartTime) < 1) {
                    seek(selectionStartTime, true);
                    resetSelection();
                } else {
                    exportBtn.disabled = false;
                }
            }
            drawTimeline();
        }

        function handleTimelineWheel(e) {
            e.preventDefault();
            const timeAtCursor = getTimeFromMouseEvent(e);
            const oldZoom = zoomLevel;
            const zoomFactor = e.deltaY < 0 ? 1.5 : 1 / 1.5;
            zoomLevel = Math.max(MIN_ZOOM, Math.min(zoomLevel * zoomFactor, MAX_ZOOM));
            if (oldZoom === zoomLevel) return;

            const totalVisibleSeconds = DAY_IN_SECONDS / zoomLevel;
            const rect = timelineWrapper.getBoundingClientRect();
            const mouseOffsetRatio = (e.clientX - rect.left) / rect.width;

            viewStartSeconds = timeAtCursor - (mouseOffsetRatio * totalVisibleSeconds);
            const maxViewStart = DAY_IN_SECONDS - totalVisibleSeconds;
            viewStartSeconds = Math.max(0, Math.min(viewStartSeconds, maxViewStart < 0 ? 0 : maxViewStart));
            syncUI();
        }
        
        function resetZoom() {
            zoomLevel = 1;
            viewStartSeconds = 0;
            syncUI();
        }

        function init() {
            backBtn.addEventListener('click', closeArchive);
            exportBtn.addEventListener('click', handleExport);
            playPauseBtn.addEventListener('click', togglePlayPause);
            speedBtn.addEventListener('click', changeSpeed);
            
            timelineWrapper.addEventListener('mousedown', handleTimelineMouseDown);
            document.addEventListener('mousemove', handleTimelineMouseMove);
            document.addEventListener('mouseup', handleTimelineMouseUp);
            
            timelineWrapper.addEventListener('mouseleave', () => {
                 if (!isDragging) {
                    mouseTime = -1;
                 }
            });
            timelineWrapper.addEventListener('wheel', handleTimelineWheel, { passive: false });
            
            videoPlayer.addEventListener('timeupdate', () => {
                placeholder.classList.add('hidden');
                videoPlayer.classList.remove('hidden');

                if (pendingSeekTime >= 0 && videoPlayer.seekable.length > 0 && videoPlayer.seekable.end(0) > pendingSeekTime) {
                    videoPlayer.currentTime = pendingSeekTime;
                    pendingSeekTime = -1;
                }

                const currentBlock = recordingsForDay.find(rec => {
                    const blockStart = (createLocalDateFromString(rec.startTimeString).getTime() - getStartOfDay().getTime()) / 1000;
                    const blockEnd = blockStart + rec.duration;
                    return currentTime >= blockStart && currentTime <= blockEnd;
                });

                if (currentBlock) {
                    const blockStart = (createLocalDateFromString(currentBlock.startTimeString).getTime() - getStartOfDay().getTime()) / 1000;
                    currentTime = blockStart + videoPlayer.currentTime;
                    seekerTime = currentTime;
                }
            });

            videoPlayer.addEventListener('play', () => { isPlaying = true; playPauseBtn.innerHTML = `<i class="material-icons">pause</i>`; });
            videoPlayer.addEventListener('pause', () => { isPlaying = false; playPauseBtn.innerHTML = `<i class="material-icons">play_arrow</i>`; });
            videoPlayer.addEventListener('ended', () => { isPlaying = false; playPauseBtn.innerHTML = `<i class="material-icons">replay</i>`; });

            window.addEventListener('resize', syncUI);
            
            updateLoop();

            if (Hls.isSupported()) {
                hls = new Hls();
                hls.on(Hls.Events.ERROR, function (event, data) {
                    if (data.fatal) {
                        console.error('HLS fatal error:', data);
                        placeholder.textContent = `Ошибка HLS: ${data.details}`;
                        placeholder.classList.remove('hidden');
                        videoPlayer.classList.add('hidden');
                        hls.destroy();
                        setTimeout(() => hls = new Hls(), 1000);
                    }
                });
            } else {
                console.warn("HLS is not supported in this browser. Falling back to direct video playback.");
            }
        }

        async function openArchiveForCamera(camera) {
            currentCamera = camera;
            mainView.classList.add('hidden');
            archiveView.classList.remove('hidden');
            cameraNameEl.textContent = `${App.t('archive_title')}: ${camera.name}`;
            if (calendarInstance) calendarInstance.destroy();
            const activeDates = await window.api.getDatesWithActivity(camera.name);
            calendarInstance = flatpickr(datePickerEl, {
                defaultDate: "today",
                dateFormat: "Y-m-d",
                locale: App.stateManager.state.appSettings.language === 'ru' ? 'ru' : 'default',
                onChange: () => loadDataForSelectedDate(),
                onDayCreate: (dObj, dStr, fp, dayElem) => {
                    const date = dayElem.dateObj;
                    const dateString = `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')}`;
                    if (activeDates.includes(dateString)) { dayElem.classList.add("has-activity"); }
                }
            });
            await resetPlayer();
            await loadDataForSelectedDate();
        }

        function closeArchive() {
            archiveView.classList.add('hidden');
            mainView.classList.remove('hidden');
            currentCamera = null;
            if (calendarInstance) { calendarInstance.destroy(); calendarInstance = null; }
            resetPlayer();
        }

        async function loadDataForSelectedDate() {
            if (!currentCamera) return;
            resetZoom();
            const date = datePickerEl.value;
            recordingsForDay = [];
            allCameraEventsForDay = [];
            eventListEl.innerHTML = `<li>${App.t('loading_text')}</li>`;
            filtersContainer.innerHTML = '';
            drawTimeline();
            try {
                const [recordings, events] = await Promise.all([
                    window.api.getRecordingsForDate({ cameraName: currentCamera.name, date }),
                    window.api.getEventsForDate({ date })
                ]);
                recordingsForDay = recordings;
                allCameraEventsForDay = events.filter(event => event.cameraId === currentCamera.id).sort((a, b) => b.timestamp - a.timestamp);
            } catch (err) {
                console.error('Error loading archive data:', err);
                App.modalHandler.showToast(App.t('archive_load_error'), true);
            }
            renderFilters();
            applyFiltersAndRender();
            syncUI();
        }

        async function resetPlayer() {
            pause();
            cancelAnimationFrame(animationFrameId);
            animationFrameId = null;

            if (hls) {
                hls.detachMedia();
            }
            videoPlayer.removeAttribute('src');
            videoPlayer.load();
            videoPlayer.classList.add('hidden');
            placeholder.textContent = App.t('archive_placeholder');
            placeholder.classList.remove('hidden');
            eventListEl.innerHTML = '';
            filtersContainer.innerHTML = '';
            resetSelection();
            resetZoom();
            currentTime = 0;
            seekerTime = -1;
            pendingSeekTime = -1;
            recordingsForDay = [];
            allCameraEventsForDay = [];
            activeFilters.clear();
            timeDisplay.textContent = formatTime(0);
            speedBtn.textContent = '1.0x';
            currentSpeedIndex = 0;

            if (!animationFrameId) {
                updateLoop();
            }
        }

        async function handleExport() {
            if (exportBtn.disabled) return;
            exportBtn.disabled = true;
            exportBtn.textContent = App.t('saving_text');
            const start = Math.min(selectionStartTime, selectionEndTime);
            const end = Math.max(selectionStartTime, selectionEndTime);
            const sourceBlock = recordingsForDay.find(rec => {
                const recDate = createLocalDateFromString(rec.startTimeString);
                const startOfDay = new Date(recDate);
                startOfDay.setHours(0, 0, 0, 0);
                const blockStart = (recDate.getTime() - startOfDay.getTime()) / 1000;
                return start >= blockStart && end < blockStart + rec.duration;
            });
            if (!sourceBlock) { App.modalHandler.showToast(App.t('archive_export_single_file_error'), true); resetSelection(); return; }
            const recDate = createLocalDateFromString(sourceBlock.startTimeString);
            const startOfDay = new Date(recDate);
            startOfDay.setHours(0, 0, 0, 0);
            const blockStartSeconds = (recDate.getTime() - startOfDay.getTime()) / 1000;
            const startTimeInFile = start - blockStartSeconds;
            const duration = end - start;
            const result = await window.api.exportArchiveClip({ sourceFilename: sourceBlock.name, startTime: startTimeInFile, duration: duration });
            if (result.success) { App.modalHandler.showToast(App.t('archive_export_success')); } else { App.modalHandler.showToast(`${App.t('archive_export_error')}: ${result.error}`, true); }
            resetSelection();
        }

        function applyFiltersAndRender() {
            let filteredEvents = allCameraEventsForDay;
            if (activeFilters.size > 0) { filteredEvents = allCameraEventsForDay.filter(event => event.objects.some(obj => activeFilters.has(obj))); }
            renderEventList(filteredEvents);
            drawTimeline();
        }

        function renderFilters() {
            const allObjectTypes = new Set(allCameraEventsForDay.flatMap(e => e.objects).filter(Boolean));
            if (allObjectTypes.size === 0) { filtersContainer.innerHTML = ''; return; }
            let filtersHTML = `<h3>${App.t('filters_title')}:</h3>`;
            allObjectTypes.forEach(type => {
                const label = App.t(`object_${type}`) || type;
                filtersHTML += `<div class="form-check-inline"><input type="checkbox" id="filter-${type}" data-type="${type}" class="form-check-input event-filter-cb"><label for="filter-${type}">${label}</label></div>`;
            });
            filtersContainer.innerHTML = filtersHTML;
            filtersContainer.querySelectorAll('.event-filter-cb').forEach(checkbox => {
                checkbox.addEventListener('change', () => {
                    if (checkbox.checked) activeFilters.add(checkbox.dataset.type); else activeFilters.delete(checkbox.dataset.type);
                    applyFiltersAndRender();
                });
            });
        }

        function renderEventList(events) {
            if (events.length === 0) { eventListEl.innerHTML = `<li style="color: var(--text-secondary); cursor: default;">${App.t('events_not_found')}</li>`; return; }
            let listHTML = '';
            events.forEach(event => {
                const eventDate = new Date(event.timestamp * 1000);
                const timeString = eventDate.toLocaleTimeString();
                const objectsString = event.objects.map(o => App.t(`object_${o}`) || o).join(', ');
                listHTML += `<li data-timestamp="${event.timestamp}"><span class="event-time">${timeString}</span><span class="event-objects">${objectsString}</span></li>`;
            });
            eventListEl.innerHTML = listHTML;
            eventListEl.querySelectorAll('li').forEach(item => {
                item.addEventListener('click', () => {
                    const timestamp = parseFloat(item.dataset.timestamp);
                    if (timestamp) {
                        const eventDate = new Date(timestamp * 1000);
                        const startOfDay = new Date(eventDate);
                        startOfDay.setHours(0, 0, 0, 0);
                        const timeInSeconds = (eventDate.getTime() - startOfDay.getTime()) / 1000;
                        seek(timeInSeconds, true);
                    }
                });
            });
        }

        function resetSelection() {
            selectionStartTime = 0;
            selectionEndTime = 0;
            exportBtn.disabled = true;
            exportBtn.textContent = App.t('archive_export_clip');
        }
        
        return { 
            init,
            openArchiveForCamera
        };
   };
})(window);
// --- END OF FILE js/archive-manager.js ---