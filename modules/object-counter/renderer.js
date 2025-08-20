// Файл: /modules/object-counter/renderer.js
// Этот скрипт будет динамически загружен в рендер, если модуль включен.

(function() {

    /**
     * Создает, обновляет или удаляет HTML-элемент для отображения счетчика
     * внутри ячейки с видео.
     * @param {HTMLElement} cellElement - DOM-элемент ячейки (.grid-cell).
     * @param {object} counts - Объект со счетчиками, например { person: 1, car: 2 }.
     */
    function updateCounterUI(cellElement, counts) {
        let counterEl = cellElement.querySelector('.object-counter-display');

        // Если объект counts пустой (объектов в кадре нет), удаляем элемент счетчика, если он существует.
        if (Object.keys(counts).length === 0) {
            if (counterEl) {
                counterEl.remove();
            }
            return;
        }

        // Если объекты есть, но элемента для счётчика ещё нет, создаём его.
        if (!counterEl) {
            const newCounterEl = document.createElement('div');
            // Присваиваем класс, стили для которого определены в main.css
            newCounterEl.className = 'object-counter-display';
            
            const videoWrapper = cellElement.querySelector('.video-wrapper');
            if (videoWrapper) {
                videoWrapper.appendChild(newCounterEl);
                // Обновляем ссылку, чтобы дальнейший код работал с новым элементом
                counterEl = newCounterEl;
            } else {
                // На всякий случай, если структура DOM изменится
                console.error('[Module: ObjectCounter] Could not find .video-wrapper to append the counter.');
                return;
            }
        }

        // Форматируем текст для отображения, например: "person: 2"
        const text = Object.entries(counts)
    .map(([label, count]) => `${App.t('object_' + label) || label}: ${count}`) // Используем перевод
    .join(' | ');
            
        // Обновляем текст в существующем или только что созданном элементе.
        if (counterEl) {
            counterEl.textContent = text;
        }
    }

    // Убеждаемся, что API от preload.js уже загружено и готово к работе.
    if (window.api && typeof window.api.on === 'function') {
        
        // Подписываемся на событие обновления данных от main-процесса.
        window.api.on('module-object-counter-update', ({ cameraId, counts }) => {
            if (!window.App || !window.App.gridManager) {
                return; // Выходим, если глобальный объект App или gridManager еще не инициализирован
            }

            const gridState = window.App.gridManager.getGridState();
            
            // Ищем все ячейки, в которых отображается нужная камера.
            gridState.forEach((cell, cellIndex) => {
                if (cell && cell.camera.id === cameraId) {
                    const gridCells = document.querySelectorAll('.grid-cell');
                    const cellElement = gridCells[cellIndex];
                    if (cellElement) {
                        // Вызываем функцию для обновления UI счётчика в найденной ячейке.
                        updateCounterUI(cellElement, counts);
                    }
                }
            });
        });

        // Подписываемся на событие "приборки", которое вызывается при деактивации модуля.
        window.api.on('module-object-counter-cleanup', () => {
            const allCounters = document.querySelectorAll('.object-counter-display');
            allCounters.forEach(counter => counter.remove());
        });

    } else {
        console.error('[Module: ObjectCounter] Failed to subscribe to events: window.api.on is not defined.');
    }

})();