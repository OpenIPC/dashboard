// js/archive-manager.js (Финальная версия с вашими функциями и новыми улучшениями)

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
        const timelineRecordingsEl = document.getElementById('timeline-recordings');
        const timelineWrapper = document.getElementById('timeline-wrapper');
        const archiveExportBtn = document.getElementById('archive-export-btn');
        
        // Новые элементы для фильтров и списка событий
        const filtersContainer = document.getElementById('archive-filters');
        const eventListEl = document.getElementById('event-list');

        const dayInSeconds = 24 * 60 * 60;
        const MIN_ZOOM = 1;
        const MAX_ZOOM = 24 * 12; // 5-минутный интервал

        // Словарь для цветов маркеров
        const OBJECT_COLORS = {
            person: '#f85149', // Красный
            car: '#ffc107',    // Желтый
            bicycle: '#17a2b8', // Голубой
            dog: '#fd7e14',    // Оранжевый
            cat: '#6f42c1',    // Фиолетовый
            default: '#6c757d' // Серый для остальных
        };

        let currentCamera = null;
        let isSelecting = false;
        let selectionStartPercent = 0;
        let selectionEndPercent = 0;
        let zoomLevel = 1;
        let viewStartSeconds = 0;
        let timeOffsetSeconds = 0;

        let allCameraEvents = []; // Хранилище всех событий за выбранный день
        let activeFilters = new Set(); // Хранилище активных фильтров

        async function openArchiveForCamera(camera) {
            currentCamera = camera;
            mainView.classList.add('hidden');
            archiveView.classList.remove('hidden');

            archiveCameraNameEl.textContent = `${App.t('archive_title')}: ${camera.name}`;
            archiveDatePicker.valueAsDate = new Date();
            
            resetPlayer();
            
            try {
                const timeResult = await window.api.getCameraTime(camera);
                if (timeResult.success && (timeResult.cameraTimestamp || timeResult.systemTime)) {
                    const cameraTimestamp = timeResult.cameraTimestamp || timeResult.systemTime;
                    const localTimestamp = Math.floor(Date.now() / 1000);
                    timeOffsetSeconds = cameraTimestamp - localTimestamp;
                    console.log(`[Archive] Time sync success. Offset: ${timeOffsetSeconds} seconds.`);
                } else {
                    throw new Error(timeResult.error || 'timestamp not found in camera response');
                }
            } catch (e) {
                timeOffsetSeconds = 0;
                console.warn(`[Archive] Time sync failed: ${e.message}. Using file-based time.`);
                if (App.modalHandler && App.modalHandler.showToast) {
                    App.modalHandler.showToast('Ошибка синхронизации времени с камерой', true);
                }
            }
            
            await loadRecordingsForDate();
        }

        function closeArchive() {
            archiveView.classList.add('hidden');
            mainView.classList.remove('hidden');
            currentCamera = null;
            resetPlayer();
        }
        
        function resetZoom() {
            zoomLevel = 1;
            viewStartSeconds = 0;
            timelineWrapper.scrollLeft = 0;
        }

        async function loadRecordingsForDate() {
            if (!currentCamera) return;
            resetZoom();
            const date = archiveDatePicker.value;
            timelineRecordingsEl.innerHTML = '<div>Loading...</div>';
            eventListEl.innerHTML = '';
            filtersContainer.innerHTML = '';

            const [recordings, events] = await Promise.all([
                window.api.getRecordingsForDate({ cameraName: currentCamera.name, date }),
                window.api.getEventsForDate({ date })
            ]);
            
            allCameraEvents = events
                .filter(event => event.cameraId === currentCamera.id)
                .sort((a, b) => b.timestamp - a.timestamp);

            renderFilters();
            applyFiltersAndRender(recordings);
        }

        function renderFilters() {
            const allObjectTypes = new Set(allCameraEvents.flatMap(e => e.objects));
            if (allObjectTypes.size === 0) {
                filtersContainer.innerHTML = '';
                return;
            }

            let filtersHTML = '<h3>Фильтры:</h3>';
            allObjectTypes.forEach(type => {
                const isChecked = activeFilters.has(type) ? 'checked' : '';
                filtersHTML += `
                    <div class="form-check-inline">
                        <input type="checkbox" id="filter-${type}" data-type="${type}" class="form-check-input event-filter-cb" ${isChecked}>
                        <label for="filter-${type}">${type}</label>
                    </div>
                `;
            });
            filtersContainer.innerHTML = filtersHTML;

            filtersContainer.querySelectorAll('.event-filter-cb').forEach(checkbox => {
                checkbox.addEventListener('change', () => {
                    if (checkbox.checked) {
                        activeFilters.add(checkbox.dataset.type);
                    } else {
                        activeFilters.delete(checkbox.dataset.type);
                    }
                    loadRecordingsForDate();
                });
            });
        }

        function applyFiltersAndRender(recordings) {
            let filteredEvents = allCameraEvents;
            if (activeFilters.size > 0) {
                filteredEvents = allCameraEvents.filter(event => 
                    event.objects.some(obj => activeFilters.has(obj))
                );
            }
            
            renderTimeline(recordings, filteredEvents);
            renderEventList(filteredEvents);
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
                listHTML += `
                    <li data-timestamp="${event.timestamp}">
                        <span class="event-time">${timeString}</span>
                        <span class="event-objects">${objectsString}</span>
                    </li>
                `;
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

        function renderTimeline(recordings, events = []) {
            const timelineContent = document.createDocumentFragment();
            const labelsContainer = document.createElement('div');
            labelsContainer.id = 'timeline-labels';
            const selectionEl = document.createElement('div');
            selectionEl.id = 'timeline-selection';

            timelineContent.appendChild(labelsContainer);
            timelineContent.appendChild(selectionEl);

            resetSelection();

            if (recordings.length === 0 && events.length === 0) {
                const noRecEl = document.createElement('div');
                noRecEl.style.cssText = "text-align:center; width:100%; color: var(--text-secondary);";
                noRecEl.textContent = App.t('archive_no_recordings');
                timelineContent.appendChild(noRecEl);
            } else {
                recordings.forEach(rec => {
                    const recDate = new Date(rec.startTime);
                    const startOfDay = new Date(recDate);
                    startOfDay.setHours(0, 0, 0, 0);
                    const fileStartTimeInSeconds = (recDate.getTime() - startOfDay.getTime()) / 1000;
                    const actualStartTimeInSeconds = fileStartTimeInSeconds + timeOffsetSeconds;
                    const durationInSeconds = 300;
                    const leftPercent = (actualStartTimeInSeconds / dayInSeconds) * 100;
                    const widthPercent = (durationInSeconds / dayInSeconds) * 100;
                    if (leftPercent < -1 || leftPercent > 101) return;

                    const block = document.createElement('div');
                    block.className = 'timeline-block';
                    block.style.left = `${leftPercent}%`;
                    block.style.width = `${widthPercent}%`;
                    block.dataset.filename = rec.name;
                    block.dataset.startTimeSec = actualStartTimeInSeconds;
                    
                    block.addEventListener('click', (e) => {
                        e.stopPropagation();
                        // Клик по блоку теперь тоже перематывает видео
                        const rect = timelineWrapper.getBoundingClientRect();
                        const positionInBlock = e.clientX - e.target.getBoundingClientRect().left;
                        const blockWidthPx = e.target.offsetWidth;
                        const clickPercentInBlock = positionInBlock / blockWidthPx;
                        const timeInSeconds = actualStartTimeInSeconds + (durationInSeconds * clickPercentInBlock);
                        seekToTime(timeInSeconds);
                    });
                    timelineContent.appendChild(block);
                });

                events.forEach(event => {
                    const eventDate = new Date(event.timestamp * 1000);
                    const startOfDay = new Date(eventDate);
                    startOfDay.setHours(0, 0, 0, 0);
                    const eventTimeInSeconds = (eventDate.getTime() - startOfDay.getTime()) / 1000;
                    const leftPercent = (eventTimeInSeconds / dayInSeconds) * 100;
                    if (leftPercent < 0 || leftPercent > 100) return;

                    const marker = document.createElement('div');
                    marker.className = 'timeline-event-marker';
                    marker.style.left = `${leftPercent}%`;

                    if (event.objects && event.objects.length > 0) {
                        const mainObjectType = event.objects[0];
                        marker.style.backgroundColor = OBJECT_COLORS[mainObjectType] || OBJECT_COLORS.default;
                        marker.title = `Событие: ${event.objects.join(', ')} @ ${eventDate.toLocaleTimeString()}`;
                    }
                    timelineContent.appendChild(marker);
                });
            }

            timelineRecordingsEl.innerHTML = '';
            timelineRecordingsEl.appendChild(timelineContent);
            updateTimelineView();
        }
        
        function updateTimelineView() {
            timelineRecordingsEl.style.width = `${zoomLevel * 100}%`;
            timelineWrapper.scrollLeft = viewStartSeconds / dayInSeconds * timelineRecordingsEl.offsetWidth;
            renderTimelineLabels();
            updateSelectionView();
        }
        
        function renderTimelineLabels() {
            // ... (Код из вашего файла остается, но добавляем стили маркеров) ...
            const styleId = 'timeline-label-styles';
            let styleEl = document.getElementById(styleId);
            if (!styleEl) {
                styleEl = document.createElement('style'); styleEl.id = styleId; document.head.appendChild(styleEl);
            }
            styleEl.textContent = `
                .timeline-label { position: absolute; top: 0; height: 100%; border-left: 1px solid var(--border-color); }
                .timeline-label.minor { border-color: #444; }
                .timeline-label.major { border-color: #777; }
                .timeline-label.major::after { content: attr(data-time); position: absolute; top: 5px; left: 5px; color: var(--text-secondary); font-size: 12px; }
                .timeline-event-marker {
                    position: absolute; top: 50%; transform: translate(-50%, -50%);
                    width: 8px; height: 8px; border-radius: 50%; border: 1px solid white;
                    z-index: 10; pointer-events: none;
                }
            `;
        }

        function seekToTime(timeInSeconds) {
            const blocks = timelineRecordingsEl.querySelectorAll('.timeline-block');
            let targetBlock = null;
            for (const block of blocks) {
                const blockStart = parseFloat(block.dataset.startTimeSec);
                const blockEnd = blockStart + 300;
                if (timeInSeconds >= blockStart && timeInSeconds <= blockEnd) {
                    targetBlock = block;
                    break;
                }
            }

            if (targetBlock) {
                playRecording(targetBlock.dataset.filename);
                document.querySelectorAll('.timeline-block.selected').forEach(b => b.classList.remove('selected'));
                targetBlock.classList.add('selected');

                const seekTimeInFile = timeInSeconds - parseFloat(targetBlock.dataset.startTimeSec);
                
                const onCanPlay = () => {
                    archiveVideoPlayer.currentTime = seekTimeInFile;
                    archiveVideoPlayer.removeEventListener('canplay', onCanPlay);
                };
                archiveVideoPlayer.addEventListener('canplay', onCanPlay);
            } else {
                App.modalHandler.showToast("Нет записи для этого момента времени", true);
            }
        }

        function playRecording(filename) {
            if (archiveVideoPlayer.src.includes(encodeURIComponent(filename))) {
                // Если видео уже проигрывается, не перезагружаем его
                return;
            }
            archiveVideoPlaceholder.classList.add('hidden');
            archiveVideoPlayer.classList.remove('hidden');
            archiveVideoPlayer.src = `video-archive://${encodeURIComponent(filename)}`;
            archiveVideoPlayer.play();
        }
        
        function resetPlayer() {
            archiveVideoPlayer.pause();
            archiveVideoPlayer.removeAttribute('src');
            archiveVideoPlayer.load();
            archiveVideoPlayer.classList.add('hidden');
            archiveVideoPlaceholder.classList.remove('hidden');
            timelineRecordingsEl.innerHTML = '';
            eventListEl.innerHTML = '';
            filtersContainer.innerHTML = '';
            document.querySelectorAll('.timeline-block.selected').forEach(b => b.classList.remove('selected'));
            resetSelection();
            resetZoom();
            timeOffsetSeconds = 0;
            allCameraEvents = [];
            activeFilters.clear();
        }

        function resetSelection() {
            const selectionEl = document.getElementById('timeline-selection');
            if (selectionEl) selectionEl.style.display = 'none';
            archiveExportBtn.disabled = true;
            archiveExportBtn.textContent = App.t('archive_export_clip');
            selectionStartPercent = 0;
            selectionEndPercent = 0;
        }

        function updateSelectionView() {
            const selectionEl = document.getElementById('timeline-selection');
            if (!selectionEl) return;
            const start = Math.min(selectionStartPercent, selectionEndPercent);
            const end = Math.max(selectionStartPercent, selectionEndPercent);
            if (end - start <= 0) {
                selectionEl.style.display = 'none';
                return;
            }
            selectionEl.style.left = `${start}%`;
            selectionEl.style.width = `${end - start}%`;
            selectionEl.style.display = 'block';
        }

        function handleTimelineMouseDown(e) {
            // ВАША ЛОГИКА ВЫДЕЛЕНИЯ СОХРАНЕНА
            isSelecting = true;
            const rect = timelineWrapper.getBoundingClientRect();
            const positionInScrolledContent = timelineWrapper.scrollLeft + e.clientX - rect.left;
            const totalContentWidth = timelineRecordingsEl.offsetWidth;
            selectionStartPercent = (positionInScrolledContent / totalContentWidth) * 100;
            selectionEndPercent = selectionStartPercent;
            updateSelectionView();
            archiveExportBtn.disabled = true;
        }

        function handleTimelineMouseMove(e) {
            // ВАША ЛОГИКА ВЫДЕЛЕНИЯ СОХРАНЕНА
            if (!isSelecting) return;
            const rect = timelineWrapper.getBoundingClientRect();
            const positionInScrolledContent = timelineWrapper.scrollLeft + e.clientX - rect.left;
            const totalContentWidth = timelineRecordingsEl.offsetWidth;
            let currentPercent = (positionInScrolledContent / totalContentWidth) * 100;
            selectionEndPercent = Math.max(0, Math.min(100, currentPercent));
            updateSelectionView();
        }
        
        function handleTimelineMouseUp(e) {
            // ВАША ЛОГИКА ВЫДЕЛЕНИЯ СОХРАНЕНА
            if (!isSelecting) return;
            isSelecting = false;
            if (Math.abs(selectionEndPercent - selectionStartPercent) > 0.1) {
                archiveExportBtn.disabled = false;
            } else {
                resetSelection();
            }
        }

        function handleTimelineWheel(e) {
            e.preventDefault();
            const rect = timelineWrapper.getBoundingClientRect();
            const mouseX = e.clientX - rect.left;
            const timeAtCursor = viewStartSeconds + (mouseX / rect.width) * (dayInSeconds / zoomLevel);
            const zoomFactor = 1.25;
            let newZoomLevel = e.deltaY < 0 ? zoomLevel * zoomFactor : zoomLevel / zoomFactor;
            zoomLevel = Math.max(MIN_ZOOM, Math.min(newZoomLevel, MAX_ZOOM));
            let newViewStartSeconds = timeAtCursor - (mouseX / rect.width) * (dayInSeconds / zoomLevel);
            const maxViewStartSeconds = dayInSeconds - (dayInSeconds / zoomLevel);
            viewStartSeconds = Math.max(0, Math.min(newViewStartSeconds, maxViewStartSeconds));
            updateTimelineView();
        }
        
        function handleTimelineScroll(e) {
            const scrollLeft = e.target.scrollLeft;
            const scrollWidth = e.target.scrollWidth;
            const clientWidth = e.target.clientWidth;
            if (scrollWidth <= clientWidth) { viewStartSeconds = 0; }
            else {
                const scrollableWidth = scrollWidth - clientWidth;
                const scrollPercentage = scrollLeft / scrollableWidth;
                const totalHiddenSeconds = dayInSeconds - (dayInSeconds / zoomLevel);
                viewStartSeconds = scrollPercentage * totalHiddenSeconds;
            }
            renderTimelineLabels();
        }
        
        async function handleExport() {
            if (archiveExportBtn.disabled) return;
            archiveExportBtn.disabled = true;
            archiveExportBtn.textContent = App.t('saving_text');
        
            const start = Math.min(selectionStartPercent, selectionEndPercent);
            const end = Math.max(selectionStartPercent, selectionEndPercent);
            
            const selectionStartSeconds = (start / 100) * dayInSeconds;
            const selectionEndSeconds = (end / 100) * dayInSeconds;
        
            let sourceBlock = null;
            const blocks = timelineRecordingsEl.querySelectorAll('.timeline-block');
            for (const block of blocks) {
                const blockStart = parseFloat(block.dataset.startTimeSec);
                const blockEnd = blockStart + 300; // Длительность 5 минут
        
                if (selectionStartSeconds >= blockStart && selectionEndSeconds <= blockEnd) {
                    sourceBlock = block;
                    break;
                }
            }
        
            if (!sourceBlock) {
                alert(App.t('archive_export_single_file_error'));
                resetSelection();
                return;
            }
        
            const blockStartSeconds = parseFloat(sourceBlock.dataset.startTimeSec);
        
            const startTimeInFile = (selectionStartSeconds - blockStartSeconds);
            const duration = selectionEndSeconds - selectionStartSeconds;
        
            const result = await window.api.exportArchiveClip({
                sourceFilename: sourceBlock.dataset.filename,
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

        function init() {
            archiveBackBtn.addEventListener('click', closeArchive);
            archiveDatePicker.addEventListener('change', loadRecordingsForDate);
            timelineWrapper.addEventListener('mousedown', handleTimelineMouseDown);
            window.addEventListener('mousemove', handleTimelineMouseMove);
            window.addEventListener('mouseup', handleTimelineMouseUp);
            archiveExportBtn.addEventListener('click', handleExport);
            timelineWrapper.addEventListener('wheel', handleTimelineWheel, { passive: false });
            timelineWrapper.addEventListener('scroll', handleTimelineScroll);
        }

        return { 
            init,
            openArchiveForCamera
        };
    }
})(window);