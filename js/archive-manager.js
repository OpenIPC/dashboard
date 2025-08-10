// --- ФАЙЛ: archive-manager.js ---

(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createArchiveManager = function(App) {
        const mainView = document.getElementById('main-view');
        const archiveView = document.getElementById('archive-view');
        const archiveBackBtn = document.getElementById('archive-back-btn');
        const archiveCameraNameEl = document.getElementById('archive-camera-name');
        const archiveDatePicker = document.getElementById('archive-date-picker');
        const archiveVideoPlayer = document.getElementById('archive-video-player');
        const archiveVideoPlaceholder = document.getElementById('archive-video-placeholder');
        
        const timelineWrapper = document.getElementById('timeline-wrapper');
        const timelineCanvas = document.getElementById('timeline-canvas');
        const timelineCtx = timelineCanvas.getContext('2d');
        const timelineLabelsEl = document.getElementById('timeline-labels');
        
        const archiveExportBtn = document.getElementById('archive-export-btn');
        const filtersContainer = document.getElementById('archive-filters');
        const eventListEl = document.getElementById('event-list');

        const dayInSeconds = 24 * 60 * 60;
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
        let isSelecting = false;
        let selectionStartTime = 0;
        let selectionEndTime = 0;
        let zoomLevel = 1;
        let viewStartSeconds = 0;
        let timeOffsetSeconds = 0;
        let calendarInstance = null;
        let seekerTime = -1;
        let mouseTime = -1;

        let recordingsForDay = [];
        let allCameraEventsForDay = [];
        let activeFilters = new Set();

        function createLocalDateFromString(timeString) {
            // Преобразуем "2025-08-10T13-05-46" в "2025-08-10 13:05:46"
            // JS-парсер поймет это как локальное время
            const formattedString = timeString.replace('T', ' ').replace(/-/g, ':').replace(':', '-').replace(':', '-');
            return new Date(formattedString);
        }
        
        async function openArchiveForCamera(camera) {
            currentCamera = camera;
            mainView.classList.add('hidden');
            archiveView.classList.remove('hidden');
            archiveCameraNameEl.textContent = `${App.t('archive_title')}: ${camera.name}`;

            if (calendarInstance) calendarInstance.destroy();
            
            const activeDates = await window.api.getDatesWithActivity(camera.name);
            calendarInstance = flatpickr(archiveDatePicker, {
                defaultDate: "today",
                dateFormat: "Y-m-d",
                locale: App.stateManager.state.appSettings.language === 'ru' ? 'ru' : 'default',
                onChange: () => loadDataForSelectedDate(),
                onDayCreate: (dObj, dStr, fp, dayElem) => {
                    const date = dayElem.dateObj;
                    const dateString = `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')}`;
                    if (activeDates.includes(dateString)) {
                        dayElem.classList.add("has-activity");
                    }
                }
            });
            
            resetPlayer();
            await loadDataForSelectedDate();
        }

        function closeArchive() {
            archiveView.classList.add('hidden');
            mainView.classList.remove('hidden');
            currentCamera = null;
            if (calendarInstance) {
                calendarInstance.destroy();
                calendarInstance = null;
            }
            resetPlayer();
        }

        async function loadDataForSelectedDate() {
            if (!currentCamera) return;
            resetZoom();
            const date = archiveDatePicker.value;
            
            recordingsForDay = [];
            allCameraEventsForDay = [];
            eventListEl.innerHTML = '<li>Загрузка...</li>';
            filtersContainer.innerHTML = '';
            drawTimeline();

            const [recordings, events] = await Promise.all([
                window.api.getRecordingsForDate({ cameraName: currentCamera.name, date }),
                window.api.getEventsForDate({ date })
            ]);
            
            recordingsForDay = recordings;
            allCameraEventsForDay = events
                .filter(event => event.cameraId === currentCamera.id)
                .sort((a, b) => b.timestamp - a.timestamp);

            renderFilters();
            applyFiltersAndRender();
        }
        
        function drawTimeline() {
            const rect = timelineWrapper.getBoundingClientRect();
            if (rect.width === 0 || rect.height === 0) return;
            timelineCanvas.width = rect.width;
            timelineCanvas.height = rect.height;

            timelineCtx.fillStyle = COLORS.background;
            timelineCtx.fillRect(0, 0, timelineCanvas.width, timelineCanvas.height);

            const totalTimelineWidth = timelineCanvas.width * zoomLevel;

            recordingsForDay.forEach(rec => {
                const recDate = createLocalDateFromString(rec.startTimeString);
                const startOfDay = new Date(recDate);
                startOfDay.setHours(0, 0, 0, 0);
                
                const startTimeInSeconds = (recDate.getTime() - startOfDay.getTime()) / 1000;
                const durationInSeconds = 300;
                const endTimeInSeconds = startTimeInSeconds + durationInSeconds;

                if (endTimeInSeconds < viewStartSeconds || startTimeInSeconds > viewStartSeconds + (dayInSeconds / zoomLevel)) return;
                
                const x = ((startTimeInSeconds - viewStartSeconds) / dayInSeconds) * totalTimelineWidth;
                const w = (durationInSeconds / dayInSeconds) * totalTimelineWidth;

                const isHovered = mouseTime >= startTimeInSeconds && mouseTime < endTimeInSeconds;
                timelineCtx.fillStyle = isHovered ? COLORS.recordingHover : COLORS.recording;
                timelineCtx.fillRect(x, timelineCanvas.height * 0.1, w, timelineCanvas.height * 0.8);
            });

            allCameraEventsForDay.forEach(event => {
                const eventDate = new Date(event.timestamp * 1000);
                const startOfDay = new Date(eventDate);
                startOfDay.setHours(0, 0, 0, 0);
                const eventTimeInSeconds = (eventDate.getTime() - startOfDay.getTime()) / 1000;
                
                if (eventTimeInSeconds < viewStartSeconds || eventTimeInSeconds > viewStartSeconds + (dayInSeconds / zoomLevel)) return;
                
                const mainObjectType = event.objects?.[0];
                if (activeFilters.size > 0 && !event.objects.some(obj => activeFilters.has(obj))) {
                    timelineCtx.fillStyle = COLORS.eventDefault;
                } else {
                    timelineCtx.fillStyle = (mainObjectType === 'person') ? COLORS.eventPerson : (mainObjectType === 'car' ? COLORS.eventCar : COLORS.eventDefault);
                }

                const x = ((eventTimeInSeconds - viewStartSeconds) / dayInSeconds) * totalTimelineWidth;
                timelineCtx.beginPath();
                timelineCtx.arc(x, timelineCanvas.height / 2, 4, 0, 2 * Math.PI);
                timelineCtx.fill();
            });

            if (isSelecting || (selectionEndTime - selectionStartTime > 0)) {
                const start = Math.min(selectionStartTime, selectionEndTime);
                const end = Math.max(selectionStartTime, selectionEndTime);
                const x = ((start - viewStartSeconds) / dayInSeconds) * totalTimelineWidth;
                const w = ((end - start) / dayInSeconds) * totalTimelineWidth;
                timelineCtx.fillStyle = COLORS.selection;
                timelineCtx.fillRect(x, 0, w, timelineCanvas.height);
            }
            
            if (seekerTime >= 0) {
                const x = ((seekerTime - viewStartSeconds) / dayInSeconds) * totalTimelineWidth;
                timelineCtx.fillStyle = COLORS.seeker;
                timelineCtx.fillRect(x - 1, 0, 2, timelineCanvas.height);
            }
        }

        function updateTimelineView() {
            const scrollWidth = timelineCanvas.width * zoomLevel;
            timelineWrapper.scrollLeft = (viewStartSeconds / dayInSeconds) * scrollWidth;
            renderTimelineLabels();
            drawTimeline();
        }

        function renderTimelineLabels() {
            timelineLabelsEl.style.width = `${zoomLevel * 100}%`;
            timelineLabelsEl.innerHTML = '';
            
            let step;
            if (zoomLevel < 3) step = 3600;
            else if (zoomLevel < 12) step = 900;
            else if (zoomLevel < 48) step = 300;
            else step = 60;
            
            for (let s = 0; s < dayInSeconds; s += step) {
                const label = document.createElement('span');
                const hour = Math.floor(s / 3600);
                const minute = Math.floor((s % 3600) / 60);
                label.textContent = `${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}`;
                label.style.left = `${(s / dayInSeconds) * 100}%`;
                timelineLabelsEl.appendChild(label);
            }
        }
        
        function resetPlayer() {
            archiveVideoPlayer.pause();
            archiveVideoPlayer.removeAttribute('src');
            archiveVideoPlayer.load();
            archiveVideoPlayer.classList.add('hidden');
            archiveVideoPlaceholder.classList.remove('hidden');
            eventListEl.innerHTML = '';
            filtersContainer.innerHTML = '';
            resetSelection();
            resetZoom();
            timeOffsetSeconds = 0;
            seekerTime = -1;
            recordingsForDay = [];
            allCameraEventsForDay = [];
            activeFilters.clear();
            drawTimeline();
        }

        function seekToTime(timeInSeconds) {
            seekerTime = timeInSeconds;
            const targetBlock = recordingsForDay.find(rec => {
                const recDate = createLocalDateFromString(rec.startTimeString);
                const startOfDay = new Date(recDate);
                startOfDay.setHours(0, 0, 0, 0);
                const blockStart = (recDate.getTime() - startOfDay.getTime()) / 1000;
                return timeInSeconds >= blockStart && timeInSeconds < blockStart + 300;
            });

            if (targetBlock) {
                playRecording(targetBlock.name);
                const recDate = createLocalDateFromString(targetBlock.startTimeString);
                const startOfDay = new Date(recDate);
                startOfDay.setHours(0, 0, 0, 0);
                const blockStartSeconds = (recDate.getTime() - startOfDay.getTime()) / 1000;

                const seekTimeInFile = timeInSeconds - blockStartSeconds;
                
                const onCanPlay = () => {
                    archiveVideoPlayer.currentTime = seekTimeInFile;
                    archiveVideoPlayer.removeEventListener('canplay', onCanPlay);
                };
                archiveVideoPlayer.addEventListener('canplay', onCanPlay);
            } else {
                App.modalHandler.showToast("Нет записи для этого момента времени", true);
            }
            drawTimeline();
        }
        
        function playRecording(filename) {
            if (archiveVideoPlayer.src.includes(encodeURIComponent(filename))) return;
            archiveVideoPlaceholder.classList.add('hidden');
            archiveVideoPlayer.classList.remove('hidden');
            archiveVideoPlayer.src = `video-archive://${encodeURIComponent(filename)}`;
            archiveVideoPlayer.play();
        }

        function handleTimelineMouseDown(e) {
            const rect = timelineWrapper.getBoundingClientRect();
            const mouseX = e.clientX - rect.left;
            const clickTime = viewStartSeconds + (mouseX / timelineCanvas.width) * (dayInSeconds / zoomLevel);

            isSelecting = true;
            selectionStartTime = clickTime;
            selectionEndTime = clickTime;
            archiveExportBtn.disabled = true;
            drawTimeline();
        }

        function handleTimelineMouseMove(e) {
            const rect = timelineWrapper.getBoundingClientRect();
            const mouseX = e.clientX - rect.left;
            mouseTime = viewStartSeconds + (mouseX / timelineCanvas.width) * (dayInSeconds / zoomLevel);
            
            if (isSelecting) {
                selectionEndTime = mouseTime;
            }
            drawTimeline();
        }

        function handleTimelineMouseUp(e) {
            if (!isSelecting) return;
            isSelecting = false;
            
            if (Math.abs(selectionEndTime - selectionStartTime) > 1) {
                archiveExportBtn.disabled = false;
            } else {
                seekToTime(selectionStartTime);
                resetSelection();
            }
            drawTimeline();
        }
        
        function handleTimelineWheel(e) {
            e.preventDefault();
            const rect = timelineWrapper.getBoundingClientRect();
            const mouseX = e.clientX - rect.left;
            const timeAtCursor = viewStartSeconds + (mouseX / rect.width) * (dayInSeconds / zoomLevel);

            const zoomFactor = e.deltaY < 0 ? 1.5 : 1 / 1.5;
            zoomLevel = Math.max(MIN_ZOOM, Math.min(zoomLevel * zoomFactor, MAX_ZOOM));
            
            let newViewStartSeconds = timeAtCursor - (mouseX / rect.width) * (dayInSeconds / zoomLevel);
            const maxViewStartSeconds = dayInSeconds - (dayInSeconds / zoomLevel);
            viewStartSeconds = Math.max(0, Math.min(newViewStartSeconds, maxViewStartSeconds));
            
            updateTimelineView();
        }
        
        function handleTimelineScroll() {
            const scrollPercent = timelineWrapper.scrollLeft / (timelineWrapper.scrollWidth - timelineWrapper.clientWidth);
            const totalHiddenSeconds = dayInSeconds - (dayInSeconds / zoomLevel);
            viewStartSeconds = scrollPercent * totalHiddenSeconds;
            renderTimelineLabels();
            drawTimeline();
        }
        
        function onPlayerTimeUpdate() {
            if (archiveVideoPlayer.paused) return;
            const currentBlock = recordingsForDay.find(rec => archiveVideoPlayer.src.includes(encodeURIComponent(rec.name)));
            if (currentBlock) {
                const recDate = createLocalDateFromString(currentBlock.startTimeString);
                const startOfDay = new Date(recDate);
                startOfDay.setHours(0, 0, 0, 0);
                const blockStartSeconds = (recDate.getTime() - startOfDay.getTime()) / 1000;
                seekerTime = blockStartSeconds + archiveVideoPlayer.currentTime;
                drawTimeline();
            }
        }
        
        async function handleExport() {
            if (archiveExportBtn.disabled) return;
            archiveExportBtn.disabled = true;
            archiveExportBtn.textContent = App.t('saving_text');
        
            const start = Math.min(selectionStartTime, selectionEndTime);
            const end = Math.max(selectionStartTime, selectionEndTime);

            const sourceBlock = recordingsForDay.find(rec => {
                 const recDate = createLocalDateFromString(rec.startTimeString);
                 const startOfDay = new Date(recDate);
                 startOfDay.setHours(0, 0, 0, 0);
                 const blockStart = (recDate.getTime() - startOfDay.getTime()) / 1000;
                 return start >= blockStart && end < blockStart + 300;
            });
        
            if (!sourceBlock) {
                alert(App.t('archive_export_single_file_error'));
                resetSelection();
                return;
            }
        
            const recDate = createLocalDateFromString(sourceBlock.startTimeString);
            const startOfDay = new Date(recDate);
            startOfDay.setHours(0, 0, 0, 0);
            const blockStartSeconds = (recDate.getTime() - startOfDay.getTime()) / 1000;
        
            const startTimeInFile = start - blockStartSeconds;
            const duration = end - start;
        
            const result = await window.api.exportArchiveClip({
                sourceFilename: sourceBlock.name,
                startTime: startTimeInFile,
                duration: duration
            });
        
            if (result.success) {
                alert(App.t('archive_export_success'));
            } else {
                alert(`${App.t('archive_export_error')}: ${result.error}`);
            }
        
            resetSelection();
        }
        
        function applyFiltersAndRender() {
            let filteredEvents = allCameraEventsForDay;
            if (activeFilters.size > 0) {
                filteredEvents = allCameraEventsForDay.filter(event => 
                    event.objects.some(obj => activeFilters.has(obj))
                );
            }
            renderEventList(filteredEvents);
            drawTimeline();
        }
        
        function renderFilters() {
            const allObjectTypes = new Set(allCameraEventsForDay.flatMap(e => e.objects));
            if (allObjectTypes.size === 0) return;

            let filtersHTML = '<h3>Фильтры:</h3>';
            allObjectTypes.forEach(type => {
                filtersHTML += `<div class="form-check-inline"><input type="checkbox" id="filter-${type}" data-type="${type}" class="form-check-input event-filter-cb"><label for="filter-${type}">${type}</label></div>`;
            });
            filtersContainer.innerHTML = filtersHTML;

            filtersContainer.querySelectorAll('.event-filter-cb').forEach(checkbox => {
                checkbox.addEventListener('change', () => {
                    if (checkbox.checked) activeFilters.add(checkbox.dataset.type);
                    else activeFilters.delete(checkbox.dataset.type);
                    applyFiltersAndRender();
                });
            });
        }
        
        function renderEventList(events) {
            if (events.length === 0) {
                eventListEl.innerHTML = `<li style="color: var(--text-secondary); cursor: default;">Событий не найдено.</li>`;
                return;
            }

            let listHTML = '';
            events.forEach(event => {
                const eventDate = new Date(event.timestamp * 1000);
                const timeString = eventDate.toLocaleTimeString();
                const objectsString = event.objects.join(', ');
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
                        seekToTime(timeInSeconds);
                    }
                });
            });
        }
        
        function resetSelection() {
            selectionStartTime = 0;
            selectionEndTime = 0;
            archiveExportBtn.disabled = true;
            archiveExportBtn.textContent = App.t('archive_export_clip');
            drawTimeline();
        }

        function resetZoom() {
            zoomLevel = 1;
            viewStartSeconds = 0;
            updateTimelineView();
        }

        function init() {
            archiveBackBtn.addEventListener('click', closeArchive);
            archiveExportBtn.addEventListener('click', handleExport);
            
            timelineWrapper.addEventListener('mousedown', handleTimelineMouseDown);
            timelineWrapper.addEventListener('mousemove', handleTimelineMouseMove);
            timelineWrapper.addEventListener('mouseup', handleTimelineMouseUp);
            timelineWrapper.addEventListener('mouseleave', () => {
                 mouseTime = -1; 
                 if (isSelecting) handleTimelineMouseUp();
                 drawTimeline();
            });

            timelineWrapper.addEventListener('wheel', handleTimelineWheel, { passive: false });
            timelineWrapper.addEventListener('scroll', handleTimelineScroll);
            
            archiveVideoPlayer.addEventListener('timeupdate', onPlayerTimeUpdate);
        }

        return { 
            init,
            openArchiveForCamera
        };
    }
})(window);