// --- START OF FILE modules/object-counter/renderer.js ---

(function() {
    'use strict';
    console.log('[Module: ObjectCounter] Renderer script loaded.');

    const counters = {}; // Хранилище для DOM-элементов счетчиков

    function updateCounterDisplay(cameraId, counts) {
        let counterEl = counters[cameraId];

        // Ищем ячейку, в которой сейчас эта камера
        const gridState = window.App.gridManager.getGridState();
        const cellInfo = gridState.map((state, index) => ({ state, index })).find(item => item.state && item.state.camera.id === cameraId);
        
        if (!cellInfo) {
            // Если камера не отображается, удаляем её счетчик
            if (counterEl) {
                counterEl.remove();
                delete counters[cameraId];
            }
            return;
        }

        const cellIndex = cellInfo.index;
        const gridCell = document.querySelector(`.grid-cell[data-cell-id='${cellIndex}']`);
        
        if (!gridCell) return;

        // Если счетчика еще нет, создаем его
        if (!counterEl) {
            counterEl = document.createElement('div');
            counterEl.className = 'object-counter-display';
            gridCell.appendChild(counterEl);
            counters[cameraId] = counterEl;
        } else if (counterEl.parentElement !== gridCell) {
            // Если камера переместилась в другую ячейку, перемещаем и счетчик
            gridCell.appendChild(counterEl);
        }

        const countEntries = Object.entries(counts);
        
        if (countEntries.length === 0) {
            counterEl.style.display = 'none';
        } else {
            counterEl.style.display = 'block';
            counterEl.innerHTML = countEntries.map(([label, count]) => {
                const translatedLabel = window.App.t('object_' + label) || label;
                return `${translatedLabel}: ${count}`;
            }).join('<br>');
        }

        // Устанавливаем таймер на скрытие счетчика, если новых данных не будет
        if (counterEl.hideTimeout) {
            clearTimeout(counterEl.hideTimeout);
        }
        counterEl.hideTimeout = setTimeout(() => {
            if (counterEl) {
                counterEl.style.display = 'none';
            }
        }, 2000); // Скрывать через 2 секунды неактивности
    }

    function processAnalyticsData(cameraId, objects) {
        const counts = {};
        objects.forEach(obj => {
            counts[obj.label] = (counts[obj.label] || 0) + 1;
        });
        updateCounterDisplay(cameraId, counts);
    }
    
    // Подписываемся на централизованное событие аналитики от renderer.js
    window.addEventListener('app-analytics-update', (event) => {
        const { cameraId, result } = event.detail;
        if (result && result.status === 'objects_detected' && result.objects) {
            processAnalyticsData(cameraId, result.objects);
        }
    });

    // Очистка при смене раскладки или закрытии камеры
    // (Пока простой вариант - просто скрывать все)
    window.addEventListener('state-changed', () => {
         Object.values(counters).forEach(el => el.style.display = 'none');
    });

})();
// --- END OF FILE modules/object-counter/renderer.js ---