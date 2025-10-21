// --- START OF FILE js/archive-manager.js ---
// Ensure Hls is available globally for archive-manager
if (typeof window.Hls === 'undefined') {
    const script = document.createElement('script');
    script.src = 'https://cdn.jsdelivr.net/npm/hls.js@latest';
    script.onload = () => { window.Hls = window.Hls || Hls; };
    document.head.appendChild(script);
}
(function(window) {
    'use strict';
    window.AppModules = window.AppModules || {};

    window.AppModules.createArchiveManager = function(App) {
        let archiveView, mainView, backBtn, cameraNameEl, datePickerEl, videoPlayer,
            placeholder, timelineWrapper, timelineCanvas, timelineCtx, timelineLabelsEl,
            eventListEl, filtersContainer, playPauseBtn, speedBtn, timeDisplay,
            clipStartBtn, clipExportBtn;

        const DAY_IN_SECONDS = 86400;
        // Убираем нестабильные скорости 8x и 16x для HLS-воспроизведения,
        // чтобы повысить надежность работы архива.
        const PLAYBACK_SPEEDS = [1, 2, 4];
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
        const DEFAULT_EVENT_DURATION = 30;
        const MIN_EVENT_DURATION = 1;
        let currentCamera = null, calendarInstance = null, recordingsForDay = [], allCameraEventsForDay = [], activeFilters = new Set();
        let hls = null, hlsConversionActive = false, currentHlsSource = null;
        let isPlaying = false, currentSpeedIndex = 0, currentTime = 0, animationFrameId = null;
        let isSelecting = false, selectionStartTime = 0, zoomLevel = 1, viewStartSeconds = 0, seekerTime = -1, mouseTime = -1;
        let targetZoomLevel = 1, targetViewStartSeconds = 0;
        let isDragging = false, lastMouseX = 0;
        // Preview elements and thumbnails (hoisted so multiple functions can use them)
        let timelinePreview, timelinePreviewImg, timelinePreviewTime;
        let timelineThumbnails = [];

        // Preview move handler (hoisted)
        function handleTimelinePreviewMove(e) {
            if (!recordingsForDay.length || !timelineThumbnails.length) return;
            const rect = timelineWrapper.getBoundingClientRect();
            const mouseX = e.clientX - rect.left;
            const totalVisibleSeconds = DAY_IN_SECONDS / zoomLevel;
            const hoverTime = viewStartSeconds + (mouseX / rect.width) * totalVisibleSeconds;
            // Найти ближайшее превью
            let best = timelineThumbnails[0];
            let minDiff = Math.abs(hoverTime - best.time);
            for (const thumb of timelineThumbnails) {
                const diff = Math.abs(hoverTime - thumb.time);
                if (diff < minDiff) { best = thumb; minDiff = diff; }
            }
            if (timelinePreviewImg) timelinePreviewImg.src = best.url;
            if (timelinePreviewTime) timelinePreviewTime.textContent = formatTime(Math.floor(best.time));
            // Позиционирование превью
            let left = mouseX - 60; // 120px ширина превью
            left = Math.max(0, Math.min(left, rect.width - 120));
            if (timelinePreview) {
                timelinePreview.style.left = left + 'px';
                timelinePreview.style.top = '-80px';
                timelinePreview.style.display = 'block';
            }
        }

        function hideTimelinePreview() {
            if (typeof timelinePreview !== 'undefined' && timelinePreview) timelinePreview.style.display = 'none';
        }

        // --- Загрузка реальных превью с сервера (hoisted) ---
        async function loadRealThumbnails() {
            timelineThumbnails = [];
            // Берём первый блок записи за день (или ближайший)
            if (!recordingsForDay.length) return;
            // Можно доработать: генерировать превью для всех блоков, сейчас — для первого
            const rec = recordingsForDay[0];
            try {
                const result = await window.api.getArchiveThumbnails({
                    sourceFilename: rec.name,
                    interval: 600, // 10 минут
                    count: 10
                });
                if (result.success && Array.isArray(result.thumbnails)) {
                    timelineThumbnails = result.thumbnails;
                } else {
                    // fallback: одна заглушка
                    console.warn('[Archive] getArchiveThumbnails returned no thumbnails, using placeholder');
                    // small inline placeholder (SVG) to avoid external network requests
                    const svgPlaceholder = 'data:image/svg+xml;utf8,' + encodeURIComponent('<svg xmlns="http://www.w3.org/2000/svg" width="120" height="68"><rect width="100%" height="100%" fill="#222"/><text x="50%" y="50%" fill="#888" font-size="12" font-family="Arial" dominant-baseline="middle" text-anchor="middle">No Preview</text></svg>');
                    timelineThumbnails = [{ time: 0, url: svgPlaceholder }];
                }
            } catch (e) {
                console.error('[Archive] Error while loading thumbnails:', e);
                const svgError = 'data:image/svg+xml;utf8,' + encodeURIComponent('<svg xmlns="http://www.w3.org/2000/svg" width="120" height="68"><rect width="100%" height="100%" fill="#222"/><text x="50%" y="50%" fill="#f66" font-size="12" font-family="Arial" dominant-baseline="middle" text-anchor="middle">Error</text></svg>');
                timelineThumbnails = [{ time: 0, url: svgError }];
            }
        }

        const formatTime = (totalSeconds) => {
            const h = Math.floor(totalSeconds / 3600).toString().padStart(2, '0');
            const m = Math.floor((totalSeconds % 3600) / 60).toString().padStart(2, '0');
            const s = Math.floor(totalSeconds % 60).toString().padStart(2, '0');
            return `${h}:${m}:${s}`;
        };

        const createLocalDateFromString = (timeString) => {
            if (!timeString) return new Date(NaN);
            const [datePart, timePartRaw] = timeString.split('T');
            if (!datePart) return new Date(NaN);
            const [yearStr, monthStr, dayStr] = datePart.split('-');
            const year = Number(yearStr || 0);
            const month = Number(monthStr || 1);
            const day = Number(dayStr || 1);
            // timePart may contain timezone suffix (Z) or milliseconds, so split by ':' and strip non-numeric
            const timePart = (timePartRaw || '00:00:00').split(':');
            const hour = Number(timePart[0] || 0);
            const minute = Number(timePart[1] || 0);
            let second = 0;
            if (timePart[2]) {
                // remove trailing Z or milliseconds
                const secMatch = timePart[2].match(/^([0-9]+(?:\.[0-9]+)?)/);
                second = secMatch ? Math.floor(Number(secMatch[1])) : 0;
            }
            return new Date(year, month - 1, day, hour, minute, second);
        };

        const coercePositiveNumber = (value, fallback = 0) => {
            const num = Number(value);
            if (!Number.isFinite(num) || num < 0) return fallback;
            return num;
        };

        function normalizeRecordings(recordings) {
            const startOfDay = getStartOfDay();
            const dayStartTime = startOfDay.getTime();
            return recordings
                .map((rec) => {
                    const recStartDate = createLocalDateFromString(rec.startTimeString);
                    if (!recStartDate || Number.isNaN(recStartDate.getTime())) {
                        return null;
                    }
                    const rawStartSeconds = (recStartDate.getTime() - dayStartTime) / 1000;
                    const durationCandidates = [rec.duration, rec.durationSeconds, rec.length, rec.totalDuration];
                    let rawDurationSeconds = 0;
                    for (const candidate of durationCandidates) {
                        const value = coercePositiveNumber(candidate);
                        if (value > 0) {
                            rawDurationSeconds = value;
                            break;
                        }
                    }
                    const rawEndSeconds = rawStartSeconds + rawDurationSeconds;
                    let startSeconds = Math.max(0, rawStartSeconds);
                    let endSeconds = Math.min(DAY_IN_SECONDS, rawEndSeconds);
                    if (!Number.isFinite(startSeconds) || !Number.isFinite(endSeconds)) {
                        return null;
                    }
                    const durationSeconds = endSeconds - startSeconds;
                    if (durationSeconds <= 0) {
                        return null;
                    }
                    return {
                        ...rec,
                        rawStartSeconds,
                        rawDurationSeconds,
                        startSeconds,
                        endSeconds,
                        durationSeconds
                    };
                })
                .filter(Boolean)
                .sort((a, b) => a.startSeconds - b.startSeconds);
        }

        function normalizeEvents(events, normalizedRecordings) {
            const startOfDay = getStartOfDay();
            const dayStartTime = startOfDay.getTime();
            return events
                .map((event) => {
                    const eventDate = new Date(event.timestamp * 1000);
                    if (!eventDate || Number.isNaN(eventDate.getTime())) {
                        return null;
                    }
                    const rawStartSeconds = (eventDate.getTime() - dayStartTime) / 1000;
                    let rawDurationSeconds = coercePositiveNumber(event.duration, DEFAULT_EVENT_DURATION);
                    if (rawDurationSeconds === 0) {
                        rawDurationSeconds = DEFAULT_EVENT_DURATION;
                    }
                    const preliminaryStart = Math.max(0, rawStartSeconds);
                    let preliminaryEnd = Math.min(DAY_IN_SECONDS, rawStartSeconds + rawDurationSeconds);
                    let startSeconds = preliminaryStart;
                    let endSeconds = preliminaryEnd;
                    let durationSeconds = Math.max(endSeconds - startSeconds, MIN_EVENT_DURATION);
                    let hasRecording = false;
                    for (const rec of normalizedRecordings) {
                        const overlaps = preliminaryEnd > rec.startSeconds && preliminaryStart < rec.endSeconds;
                        if (overlaps) {
                            hasRecording = true;
                            startSeconds = Math.max(preliminaryStart, rec.startSeconds);
                            endSeconds = Math.min(preliminaryEnd, rec.endSeconds);
                            durationSeconds = Math.max(endSeconds - startSeconds, MIN_EVENT_DURATION);
                            break;
                        }
                    }
                    return {
                        ...event,
                        rawStartSeconds,
                        rawDurationSeconds,
                        startSeconds,
                        endSeconds,
                        durationSeconds,
                        hasRecording
                    };
                })
                .filter((event) => event && event.durationSeconds > 0 && event.endSeconds > 0 && event.startSeconds < DAY_IN_SECONDS)
                .sort((a, b) => a.startSeconds - b.startSeconds);
        }
        
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
            currentTime = Math.max(0, Math.min(timeInSeconds, DAY_IN_SECONDS));
            seekerTime = currentTime;
            const targetBlock = recordingsForDay.find(rec => currentTime >= rec.startSeconds && currentTime < rec.endSeconds);
            if (!targetBlock) {
                App.modalHandler.showToast(App.t('archive_no_recordings_for_time'), true);
                return;
            }
            const blockStart = Number.isFinite(targetBlock.rawStartSeconds) ? targetBlock.rawStartSeconds : targetBlock.startSeconds;
            const blockDuration = Number.isFinite(targetBlock.rawDurationSeconds) && targetBlock.rawDurationSeconds > 0
                ? targetBlock.rawDurationSeconds
                : targetBlock.durationSeconds;
            let seekInFile = currentTime - blockStart;
            seekInFile = Math.max(0, Math.min(seekInFile, blockDuration));
            if (!hlsConversionActive || currentHlsSource !== targetBlock.name) {
                hlsConversionActive = true;
                currentHlsSource = targetBlock.name;
                placeholder.textContent = 'Подготовка видео...';
                placeholder.classList.remove('hidden');
                videoPlayer.classList.add('hidden');
                try {
                    const result = await window.api.prepareArchiveForHls({ filename: targetBlock.name, startTime: seekInFile });
                    if (!result.success) throw new Error(result.error);
                    hls.loadSource(result.url);
                    hls.attachMedia(videoPlayer);
                    hls.once(Hls.Events.LEVEL_LOADED, function() {
                        if (startPlaying) {
                            play();
                        }
                    });
                } catch (error) {
                    console.error('HLS preparation failed:', error);
                    App.modalHandler.showToast(`Ошибка подготовки HLS: ${error.message}`, true);
                    placeholder.textContent = 'Ошибка загрузки HLS';
                    hlsConversionActive = false;
                    currentHlsSource = null;
                }
            } else {
                videoPlayer.currentTime = seekInFile;
                if (startPlaying) {
                    play();
                }
            }
        }

        function updateTimelineAnimation() {
            const zoomDiff = targetZoomLevel - zoomLevel;
            const viewDiff = targetViewStartSeconds - viewStartSeconds;
            if (Math.abs(zoomDiff) < 0.001 && Math.abs(viewDiff) < 0.01) {
                zoomLevel = targetZoomLevel;
                viewStartSeconds = targetViewStartSeconds;
                return;
            }
            zoomLevel += zoomDiff * 0.2;
            viewStartSeconds += viewDiff * 0.2;
            syncUI();
        }

        function updateLoop() {
            updateTimelineAnimation();
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
                const { startSeconds, endSeconds, durationSeconds } = rec;
                if (endSeconds < viewStartSeconds || startSeconds > viewStartSeconds + totalVisibleSeconds) return;
                const x = ((startSeconds - viewStartSeconds) / totalVisibleSeconds) * canvasWidth;
                const w = (durationSeconds / totalVisibleSeconds) * canvasWidth;
                const isHovered = mouseTime >= startSeconds && mouseTime < endSeconds;
                timelineCtx.fillStyle = isHovered ? COLORS.recordingHover : COLORS.recording;
                timelineCtx.fillRect(x, canvasHeight * 0.25, Math.max(1, w), canvasHeight * 0.5);
            });
            allCameraEventsForDay.forEach(event => {
                const { startSeconds, endSeconds, durationSeconds } = event;
                if (endSeconds < viewStartSeconds || startSeconds > viewStartSeconds + totalVisibleSeconds) return;
                const eventObjects = Array.isArray(event.objects) ? event.objects : [];
                if (activeFilters.size > 0 && !eventObjects.some(obj => activeFilters.has(obj))) return;
                const mainObjectType = eventObjects[0];
                const isOutsideRecording = !event.hasRecording;
                if (isOutsideRecording) { timelineCtx.globalAlpha = 0.45; }
                timelineCtx.fillStyle = (mainObjectType === 'person') ? COLORS.eventPerson : (mainObjectType === 'car' ? COLORS.eventCar : COLORS.eventDefault);
                const x = ((startSeconds - viewStartSeconds) / totalVisibleSeconds) * canvasWidth;
                const w = (durationSeconds / totalVisibleSeconds) * canvasWidth;
                timelineCtx.fillRect(x, 0, Math.max(1, w), canvasHeight);
                if (isOutsideRecording) { timelineCtx.globalAlpha = 1; }
            });
            if (isSelecting) {
                const start = Math.min(selectionStartTime, currentTime);
                const end = Math.max(selectionStartTime, currentTime);
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
        
        // Note: single click seeks; drag starts only after mouse moves beyond a small threshold
        let mouseDown = false;
        let dragStarted = false;
        const DRAG_THRESHOLD_PX = 6;

        function handleTimelineMouseDown(e) {
            if (e.button === 0) {
                mouseDown = true;
                dragStarted = false;
                lastMouseX = e.clientX;
                const clickTime = getTimeFromMouseEvent(e);
                // Immediately seek to clicked time (this will prepare HLS starting from inside the recording if applicable)
                seek(clickTime, true);
            }
        }

        function handleTimelineMouseMove(e) {
            mouseTime = getTimeFromMouseEvent(e);
            if (mouseDown && !dragStarted) {
                const move = Math.abs(e.clientX - lastMouseX);
                if (move >= DRAG_THRESHOLD_PX) {
                    // start dragging
                    dragStarted = true;
                    isDragging = true;
                    timelineWrapper.classList.add('grabbing');
                }
            }
            if (isDragging) {
                const deltaX = e.clientX - lastMouseX;
                lastMouseX = e.clientX;
                const totalVisibleSecondsNow = DAY_IN_SECONDS / zoomLevel;
                const secondsPerPixel = totalVisibleSecondsNow / timelineWrapper.clientWidth;
                const newViewStart = viewStartSeconds - deltaX * secondsPerPixel;
                const maxViewStart = DAY_IN_SECONDS - totalVisibleSecondsNow;
                viewStartSeconds = Math.max(0, Math.min(newViewStart, maxViewStart < 0 ? 0 : maxViewStart));
                targetViewStartSeconds = viewStartSeconds;
                syncUI();
            } else {
                drawTimeline();
            }
        }

        function handleTimelineMouseUp(e) {
            if (isDragging) {
                isDragging = false;
                timelineWrapper.classList.remove('grabbing');
            }
            // reset mouse flags
            mouseDown = false;
            dragStarted = false;
        }

        function handleTimelineWheel(e) {
            e.preventDefault();
            const timeAtCursor = getTimeFromMouseEvent(e);
            const zoomFactor = e.deltaY < 0 ? 1.5 : 1 / 1.5;
            targetZoomLevel = Math.max(MIN_ZOOM, Math.min(targetZoomLevel * zoomFactor, MAX_ZOOM));
            const totalVisibleSecondsAtTarget = DAY_IN_SECONDS / targetZoomLevel;
            const rect = timelineWrapper.getBoundingClientRect();
            const mouseOffsetRatio = (e.clientX - rect.left) / rect.width;
            const newViewStart = timeAtCursor - (mouseOffsetRatio * totalVisibleSecondsAtTarget);
            const maxViewStart = DAY_IN_SECONDS - totalVisibleSecondsAtTarget;
            targetViewStartSeconds = Math.max(0, Math.min(newViewStart, maxViewStart < 0 ? 0 : maxViewStart));
        }
        
    async function init() {
            archiveView = document.getElementById('archive-view');
            mainView = document.getElementById('main-view');
            backBtn = document.getElementById('archive-back-btn');
            cameraNameEl = document.getElementById('archive-camera-name');
            datePickerEl = document.getElementById('archive-date-picker');
            videoPlayer = document.getElementById('archive-video-player');
            placeholder = document.getElementById('archive-video-placeholder');
            timelineWrapper = document.getElementById('timeline-wrapper');
            timelineCanvas = document.getElementById('timeline-canvas');
            timelineCtx = timelineCanvas.getContext('2d');
            timelineLabelsEl = document.getElementById('timeline-labels');
            // --- Preview elements (bind to hoisted variables) ---
            timelinePreview = document.getElementById('timeline-preview');
            timelinePreviewImg = document.getElementById('timeline-preview-img');
            timelinePreviewTime = document.getElementById('timeline-preview-time');
            eventListEl = document.getElementById('event-list');
            filtersContainer = document.getElementById('archive-filters');
            playPauseBtn = document.getElementById('ac-play-pause-btn');
            speedBtn = document.getElementById('ac-speed-btn');
            timeDisplay = document.getElementById('ac-time-display');
            clipStartBtn = document.getElementById('ac-clip-start-btn');
            clipExportBtn = document.getElementById('ac-clip-export-btn');

            backBtn.addEventListener('click', closeArchive);
            playPauseBtn.addEventListener('click', togglePlayPause);
            speedBtn.addEventListener('click', changeSpeed);
            clipStartBtn.addEventListener('click', () => {
                isSelecting = !isSelecting;
                if (isSelecting) {
                    selectionStartTime = currentTime;
                    clipStartBtn.classList.add('active');
                    clipExportBtn.disabled = false;
                    clipExportBtn.innerHTML = `<i class="material-icons">save</i>`;
                    clipExportBtn.title = "Закончить выделение и экспортировать";
                } else {
                    resetSelection();
                }
                drawTimeline();
            });
            clipExportBtn.addEventListener('click', async () => {
                if (isSelecting) {
                    const endTime = currentTime;
                    await handleExport(selectionStartTime, endTime);
                }
            });
            timelineWrapper.addEventListener('mousedown', handleTimelineMouseDown);
            document.addEventListener('mousemove', handleTimelineMouseMove);
            document.addEventListener('mouseup', handleTimelineMouseUp);
            timelineWrapper.addEventListener('mouseleave', () => {
                if (!isDragging) { mouseTime = -1; }
                hideTimelinePreview();
            });
            timelineWrapper.addEventListener('wheel', handleTimelineWheel, { passive: false });

            // Listen for recordings updates from main process and refresh archive if it concerns current camera
            if (window.api && typeof window.api.onRecordingsUpdated === 'function') {
                window.api.onRecordingsUpdated((payload) => {
                    try {
                        if (!currentCamera) return;
                        if (!payload || typeof payload.cameraId === 'undefined') return;
                        if (Number(payload.cameraId) === Number(currentCamera.id)) {
                            // reload data for currently selected date
                            console.log('[Archive] recordings-updated event received, reloading archive for current camera.');
                            loadDataForSelectedDate().catch(err => console.error('[Archive] reload after recordings-updated failed:', err));
                        }
                    } catch (e) { console.warn('[Archive] Error handling recordings-updated event:', e); }
                });
            }
            // add preview mousemove after functions are defined
            timelineWrapper.addEventListener('mousemove', handleTimelinePreviewMove);
            videoPlayer.addEventListener('timeupdate', () => {
                placeholder.classList.add('hidden');
                videoPlayer.classList.remove('hidden');
                const currentBlock = recordingsForDay.find(rec => rec.name === currentHlsSource);
                if (currentBlock) {
                    const blockStart = Number.isFinite(currentBlock.rawStartSeconds) ? currentBlock.rawStartSeconds : currentBlock.startSeconds;
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
                        hlsConversionActive = false;
                        currentHlsSource = null;
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
            await loadRealThumbnails(); // загружаем превью при каждом открытии архива
            await loadDataForSelectedDate();
        }

        function closeArchive() {
            archiveView.classList.add('hidden');
            mainView.classList.remove('hidden');
            currentCamera = null;
            if (calendarInstance) { calendarInstance.destroy(); calendarInstance = null; }
            if (hlsConversionActive) {
                window.api.stopVideoStream('hls-conversion'); 
                hlsConversionActive = false;
                currentHlsSource = null;
            }
            resetPlayer();
        }

        async function loadDataForSelectedDate() {
            if (!currentCamera) return;
            zoomLevel = 1;
            viewStartSeconds = 0;
            targetZoomLevel = 1;
            targetViewStartSeconds = 0;
            const date = datePickerEl.value;
            if (hlsConversionActive) {
                window.api.stopVideoStream('hls-conversion');
                hlsConversionActive = false;
                currentHlsSource = null;
            }
            recordingsForDay = [];
            allCameraEventsForDay = [];
            eventListEl.innerHTML = `<li>${App.t('loading_text')}</li>`;
            filtersContainer.innerHTML = '';
            try {
                const [recordings, events] = await Promise.all([
                    // Request by cameraId to avoid issues when display name differs from stored config name
                    window.api.getRecordingsForDate({ cameraId: currentCamera.id, date }),
                    window.api.getEventsForDate({ date })
                ]);
                recordingsForDay = normalizeRecordings(Array.isArray(recordings) ? recordings : []);
                console.log('[Archive] recordingsForDay normalized:', JSON.stringify(recordingsForDay, null, 2));
                const cameraEvents = (Array.isArray(events) ? events : []).filter(event => event.cameraId === currentCamera.id);
                allCameraEventsForDay = normalizeEvents(cameraEvents, recordingsForDay).sort((a, b) => b.timestamp - a.timestamp);
                // После загрузки записей генерируем превьюы для таймлайна
                try {
                    await loadRealThumbnails();
                } catch (e) {
                    console.error('[Archive] loadRealThumbnails failed:', e);
                }
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
            hlsConversionActive = false;
            currentHlsSource = null;
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
            zoomLevel = 1;
            viewStartSeconds = 0;
            targetZoomLevel = 1;
            targetViewStartSeconds = 0;
            currentTime = 0;
            seekerTime = -1;
            recordingsForDay = [];
            allCameraEventsForDay = [];
            activeFilters.clear();
            timeDisplay.textContent = formatTime(0);
            speedBtn.textContent = '1.0x';
            currentSpeedIndex = 0;
        }

        async function handleExport(start, end) {
            const selectionStart = Math.max(0, Math.min(start, end));
            const selectionEnd = Math.min(DAY_IN_SECONDS, Math.max(start, end));
            if (!Number.isFinite(selectionStart) || !Number.isFinite(selectionEnd) || selectionEnd - selectionStart <= 0) {
                App.modalHandler.showToast("Ошибка: выберите непустой диапазон для экспорта.", true);
                resetSelection();
                return;
            }
            clipExportBtn.disabled = true;
            clipExportBtn.innerHTML = `...`;
            clipStartBtn.disabled = true;

            const segments = [];
            const totalDuration = selectionEnd - selectionStart;
            recordingsForDay.forEach(rec => {
                const overlapStart = Math.max(selectionStart, rec.startSeconds);
                const overlapEnd = Math.min(selectionEnd, rec.endSeconds);
                if (overlapEnd <= overlapStart) {
                    return;
                }
                const blockStartSeconds = Number.isFinite(rec.rawStartSeconds) ? rec.rawStartSeconds : rec.startSeconds;
                const blockDurationSeconds = Number.isFinite(rec.rawDurationSeconds) && rec.rawDurationSeconds > 0
                    ? rec.rawDurationSeconds
                    : rec.durationSeconds;
                let startTimeInFile = overlapStart - blockStartSeconds;
                startTimeInFile = Math.max(0, Math.min(startTimeInFile, blockDurationSeconds));
                let durationInFile = overlapEnd - overlapStart;
                durationInFile = Math.max(0, Math.min(durationInFile, blockDurationSeconds - startTimeInFile));
                if (durationInFile > 0) {
                    segments.push({
                        sourceFilename: rec.name,
                        startTime: startTimeInFile,
                        duration: durationInFile
                    });
                }
            });

            if (!segments.length) {
                App.modalHandler.showToast(App.t('archive_export_single_file_error'), true);
                resetSelection();
                return;
            }

            if (segments.length > 1 && (!window.api || typeof window.api.exportArchiveClipBatch !== 'function')) {
                App.modalHandler.showToast('Экспорт нескольких блоков пока недоступен: обновите приложение.', true);
                resetSelection();
                return;
            }

            try {
                let result;
                if (segments.length === 1) {
                    const [segment] = segments;
                    result = await window.api.exportArchiveClip({
                        sourceFilename: segment.sourceFilename,
                        startTime: segment.startTime,
                        duration: segment.duration
                    });
                } else {
                    const batchPayload = {
                        cameraId: currentCamera ? currentCamera.id : undefined,
                        totalDuration,
                        segments
                    };
                    result = await window.api.exportArchiveClipBatch(batchPayload);
                }
                if (result && result.success) {
                    App.modalHandler.showToast(App.t('archive_export_success'));
                } else {
                    const errorMessage = result && result.error ? result.error : 'unknown';
                    App.modalHandler.showToast(`${App.t('archive_export_error')}: ${errorMessage}`, true);
                }
            } catch (error) {
                console.error('[Archive] export failed:', error);
                const message = error && error.message ? error.message : 'unknown error';
                App.modalHandler.showToast(`${App.t('archive_export_error')}: ${message}`, true);
            }

            resetSelection();
        }

        function applyFiltersAndRender() {
            let filteredEvents = allCameraEventsForDay;
            if (activeFilters.size > 0) {
                filteredEvents = allCameraEventsForDay.filter(event => {
                    const eventObjects = Array.isArray(event.objects) ? event.objects : [];
                    return eventObjects.some(obj => activeFilters.has(obj));
                });
            }
            renderEventList(filteredEvents);
            drawTimeline();
        }

        function renderFilters() {
            const allObjectTypes = new Set(allCameraEventsForDay.flatMap(e => Array.isArray(e.objects) ? e.objects : []).filter(Boolean));
            if (allObjectTypes.size === 0) {
                filtersContainer.innerHTML = '';
                return;
            }
            let filtersHTML = `<h3>${App.t('filters_title')}:</h3>`;
            allObjectTypes.forEach(type => {
                const label = App.t(`object_${type}`) || type;
                filtersHTML += `<div class="form-check-inline"><input type="checkbox" id="filter-${type}" data-type="${type}" class="form-check-input event-filter-cb"><label for="filter-${type}">${label}</label></div>`;
            });
            filtersContainer.innerHTML = filtersHTML;
            filtersContainer.querySelectorAll('.event-filter-cb').forEach(checkbox => {
                checkbox.addEventListener('change', () => {
                    if (checkbox.checked) {
                        activeFilters.add(checkbox.dataset.type);
                    } else {
                        activeFilters.delete(checkbox.dataset.type);
                    }
                    applyFiltersAndRender();
                });
            });
        }

        function renderEventList(events) {
            if (events.length === 0) {
                eventListEl.innerHTML = `<li style="color: var(--text-secondary); cursor: default;">${App.t('events_not_found')}</li>`;
                return;
            }
            let listHTML = '';
            events.forEach(event => {
                const timeString = formatTime(Math.floor(event.startSeconds));
                const eventObjects = Array.isArray(event.objects) ? event.objects : [];
                const objectsString = eventObjects.map(o => App.t(`object_${o}`) || o).join(', ');
                const noRecordingAttr = event.hasRecording ? '' : ' data-no-recording="1"';
                const noRecordingTitle = event.hasRecording ? '' : ` title="${App.t('archive_no_recordings_for_time')}"`;
                listHTML += `<li data-start-seconds="${event.startSeconds}" data-end-seconds="${event.endSeconds}" data-timestamp="${event.timestamp}"${noRecordingAttr}${noRecordingTitle}><span class="event-time">${timeString}</span><span class="event-objects">${objectsString}</span></li>`;
            });
            eventListEl.innerHTML = listHTML;
            eventListEl.querySelectorAll('li').forEach(item => {
                item.addEventListener('click', () => {
                    const startSeconds = Number(item.dataset.startSeconds);
                    const endSeconds = Number(item.dataset.endSeconds);
                    if (!Number.isFinite(startSeconds)) return;
                    const duration = Number.isFinite(endSeconds) ? endSeconds - startSeconds : 0;
                    const focusTime = startSeconds + Math.max(duration / 2, 0);
                    seek(focusTime, true);
                });
            });
        }

        function resetSelection() {
            isSelecting = false;
            selectionStartTime = 0;
            clipStartBtn.classList.remove('active');
            clipStartBtn.disabled = false;
            clipExportBtn.disabled = true;
            clipExportBtn.innerHTML = `<i class="material-icons">save</i>`;
            clipExportBtn.title = "Экспорт клипа";
            drawTimeline();
        }
        
        // VVVVVV --- НАЧАЛО НОВОГО БЛОКА --- VVVVVV
        /**
         * Вызывается из renderer.js при получении события о новой записи от MediaMTX.
         * Проверяет, открыт ли архив для нужной камеры, и если да - обновляет данные.
         * @param {string} cameraName - Имя камеры, для которой создана новая запись.
         */
        function refreshDataIfVisible(cameraName) {
            if (!archiveView.classList.contains('hidden') && currentCamera && currentCamera.name === cameraName) {
                console.log(`[Archive] Refreshing data for visible camera: ${cameraName} due to webhook event.`);
                // Эта функция уже умеет перезагружать все данные для текущей даты и камеры
                loadDataForSelectedDate();
            }
        }
        // ^^^^^^ --- КОНЕЦ НОВОГО БЛОКА --- ^^^^^^

        return { 
            init: () => init().catch(console.error),
            openArchiveForCamera,
            // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЯ --- VVVVVV
            refreshDataIfVisible // "Экспортируем" новую функцию, чтобы renderer.js мог ее вызвать
            // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
        };
   };
})(window);
// --- END OF FILE js/archive-manager.js ---