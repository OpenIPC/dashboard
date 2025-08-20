// Файл: /modules/face-detector/main.js (ВЕРСИЯ С ФАЙЛОВОЙ СИСТЕМОЙ И КУЛДАУНОМ)

const fs = require('fs/promises');
const path = require('path');
const sharp = require('sharp');

let analyticsListener = null;

// Время в миллисекундах, которое должно пройти перед сохранением следующего снимка.
// 10000 = 10 секунд.
const SAVE_COOLDOWN_MS = 10000; 

// Объект для хранения времени последнего сохранения для каждой камеры
const lastSaveTimestamps = {};

function activate(api) {
  console.log('[Module: FaceDetector] Бэкенд-часть активирована.');

  analyticsListener = async ({ cameraId, result }) => {
    // 1. Проверяем наличие ключа 'frame_path'. Если его нет, выходим.
    if (!result.frame_path) {
      return;
    }

    // 2. Проверяем, существуют ли в кадре объекты с меткой 'person'
    const persons = result.objects.filter(obj => obj.label === 'person');
    if (persons.length === 0) {
      // Если людей нет, но временный файл был создан, удаляем его, чтобы не оставлять мусор
      try { await fs.unlink(result.frame_path); } catch (e) {}
      return;
    }

    // 3. Проверяем кулдаун
    const now = Date.now();
    const lastSave = lastSaveTimestamps[cameraId] || 0;

    if (now - lastSave < SAVE_COOLDOWN_MS) {
      // Если время еще не прошло, удаляем временный файл и выходим
      try { await fs.unlink(result.frame_path); } catch (e) {}
      return;
    }
    
    let facesDir;
    try {
        const settings = await api.configManager.getAppSettings();
        const moduleFolderName = path.basename(__dirname);
        const savePathKey = `module_${moduleFolderName}_savePath`;

        if (settings[savePathKey]) {
            facesDir = settings[savePathKey];
        } else {
            const dataPath = api.configManager.getDataPath();
            facesDir = path.join(dataPath, 'faces');
        }
        await fs.mkdir(facesDir, { recursive: true });
    } catch(e) {
        console.error(`[Module: FaceDetector] Critical error determining save path: ${e.message}`);
        try { await fs.unlink(result.frame_path); } catch (e) {} // Очистка
        return;
    }

    try {
      // 4. Читаем изображение из временного файла с помощью sharp
      const image = sharp(result.frame_path);

      // Сохраняем только первое найденное лицо, чтобы не создавать много файлов из одного кадра
      const person = persons[0]; 
      const { x, y, w, h } = person.box;

      if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > result.frame_width || y + h > result.frame_height) {
          console.warn('[Module: FaceDetector] Skipping invalid person box.');
          // Пропускаем, но не забываем удалить временный файл
          try { await fs.unlink(result.frame_path); } catch (e) {}
          return;
      }

      // Добавляем отступы
      const paddingX = Math.round(w * 0.3);
      const paddingY = Math.round(h * 0.3);
      let extract_x = x - paddingX;
      let extract_y = y - paddingY;
      let extract_w = w + (paddingX * 2);
      let extract_h = h + (paddingY * 2);
      if (extract_x < 0) { extract_x = 0; }
      if (extract_y < 0) { extract_y = 0; }
      if (extract_x + extract_w > result.frame_width) { extract_w = result.frame_width - extract_x; }
      if (extract_y + extract_h > result.frame_height) { extract_h = result.frame_height - extract_y; }
      
      const faceBuffer = await image
        .extract({ left: extract_x, top: extract_y, width: extract_w, height: extract_h })
        .toBuffer();

      const timestamp = new Date().toISOString().replace(/:/g, '-').slice(0, -5);
      const randomSuffix = Math.random().toString(36).substring(2, 7);
      const filename = `person_${cameraId}_${timestamp}_${randomSuffix}.jpg`;
      const filePath = path.join(facesDir, filename);
      
      await fs.writeFile(filePath, faceBuffer);
      console.log(`[Module: FaceDetector] Successfully saved person snapshot to: ${filePath}`);

      // 5. Обновляем время последнего сохранения
      lastSaveTimestamps[cameraId] = now;

    } catch (error) {
      console.error('[Module: FaceDetector] Error processing/saving snapshot:', error);
    } finally {
      // 6. В любом случае (успех или ошибка) удаляем временный файл
      try {
        await fs.unlink(result.frame_path);
      } catch (e) {
        // Игнорируем ошибку, если файла уже нет
      }
    }
  };

  api.on('analytics-update', analyticsListener);
}

function deactivate(api) {
  console.log('[Module: FaceDetector] Бэкенд-часть деактивирована.');
  if (analyticsListener) {
    api.off('analytics-update', analyticsListener);
  }
}

module.exports = { activate, deactivate };