// Файл: /modules/object-counter/main.js

let analyticsListener = null;
const cleanupTimers = new Map();

/**
 * Функция, которая вызывается при активации модуля.
 * @param {object} api - Объект API для взаимодействия с ядром приложения.
 */
function activate(api) {
  console.log('[Module: ObjectCounter] Бэкенд-часть активирована.');
  
  analyticsListener = ({ cameraId, result }) => {
    if (cleanupTimers.has(cameraId)) {
      clearTimeout(cleanupTimers.get(cameraId));
    }

    const frameCounts = {};
    if (result.objects && result.objects.length > 0) {
        result.objects.forEach(obj => {
            frameCounts[obj.label] = (frameCounts[obj.label] || 0) + 1;
        });
    }
    
    api.sendToRenderer('module-object-counter-update', {
      cameraId,
      counts: frameCounts
    });

    const timerId = setTimeout(() => {
      api.sendToRenderer('module-object-counter-update', {
        cameraId,
        counts: {}
      });
      cleanupTimers.delete(cameraId);
    }, 1500);

    cleanupTimers.set(cameraId, timerId);
  };
  
  api.on('analytics-update', analyticsListener);
}

/**
 * Функция, которая вызывается при деактивации модуля.
 * @param {object} api - Объект API для взаимодействия с ядром приложения.
 */
function deactivate(api) {
  console.log('[Module: ObjectCounter] Бэкенд-часть деактивирована.');
  
  // VVVVVV --- ИЗМЕНЕНИЕ: ИСПОЛЬЗУЕМ api.off --- VVVVVV
  if (analyticsListener) {
    api.off('analytics-update', analyticsListener);
  }
  
  cleanupTimers.forEach(timerId => clearTimeout(timerId));
  cleanupTimers.clear();

  api.sendToRenderer('module-object-counter-cleanup');
}

module.exports = { activate, deactivate };