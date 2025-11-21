# 📦 Готовые Файлы для Загрузки на GitHub

## ✅ Файлы Подготовлены

Папка: `E:\dashboard\release-anpr\`

### 1️⃣ anpr_ocr.py
- **Размер**: 0.02 MB (19 KB)
- **SHA256**: `AF4492A2A5993E50E49F639B10B1F37C23C27F57DCA47EBB496070AF683B01B5`
- **Назначение**: Python скрипт для OCR распознавания номеров
- **Путь**: `release-anpr\anpr_ocr.py`

### 2️⃣ crnn_ocr_model_best.pth
- **Размер**: 33.26 MB
- **SHA256**: `D591089F47354FF586CFE8B01D42C81E3BA564ED46CF36666D38AE49FCFA2177`
- **Назначение**: PyTorch модель CRNN для Python OCR
- **Путь**: `release-anpr\crnn_ocr_model_best.pth`

---

## 📤 Инструкция по Загрузке на GitHub

### Вариант А: GitHub Release (Рекомендуется)

1. Перейдите на: https://github.com/OpenIPC/dashboard/releases/new
2. **Tag version**: `anpr-resources-v1.0.0`
3. **Release title**: `ANPR Resources v1.0.0`
4. **Description**:
   ```markdown
   # ANPR Module Resources
   
   Python OCR script and PyTorch model for License Plate Recognition module.
   
   ## Files:
   - `anpr_ocr.py` (19 KB) - Python OCR script
   - `crnn_ocr_model_best.pth` (33.26 MB) - CRNN PyTorch model
   
   ## SHA256 Checksums:
   - anpr_ocr.py: `AF4492A2A5993E50E49F639B10B1F37C23C27F57DCA47EBB496070AF683B01B5`
   - crnn_ocr_model_best.pth: `D591089F47354FF586CFE8B01D42C81E3BA564ED46CF36666D38AE49FCFA2177`
   ```
5. **Attach files**: Загрузите оба файла из `release-anpr\`
6. **Publish release**

### Вариант Б: Отдельный Репозиторий

Если размер слишком большой для основного репозитория, создайте:
`OpenIPC/dashboard-anpr-resources`

---

## 🔗 URL После Загрузки

После создания release, URL будут:

```
https://github.com/OpenIPC/dashboard/releases/download/anpr-resources-v1.0.0/anpr_ocr.py
https://github.com/OpenIPC/dashboard/releases/download/anpr-resources-v1.0.0/crnn_ocr_model_best.pth
```

---

## ✏️ Код для Обновления

После загрузки, дайте мне эти URL, и я обновлю код так:

### src-tauri/src/analytics.rs

```rust
// Python OCR script
const LICENSE_PLATE_PYTHON_OCR_SCRIPT_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/OpenIPC/dashboard/releases/download/anpr-resources-v1.0.0/anpr_ocr.py",
        file_name: "anpr_ocr.py",
        sha256: Some("af4492a2a5993e50e49f639b10b1f37c23c27f57dca47ebb496070af683b01b5"),
    },
);

// Python PyTorch model
const LICENSE_PLATE_PYTHON_MODEL_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/OpenIPC/dashboard/releases/download/anpr-resources-v1.0.0/crnn_ocr_model_best.pth",
        file_name: "crnn_ocr_model_best.pth",
        sha256: Some("d591089f47354ff586cfe8b01d42c81e3ba564ed46cf36666d38ae49fcfa2177"),
    },
);

#[cfg(target_os = "windows")]
const LICENSE_PLATE_RESOURCES: &[ModuleResourceSpec] = &[
    LICENSE_PLATE_DETECTOR_MODEL_RESOURCE,      // anpr_yolov8.onnx
    LICENSE_PLATE_OCR_MODEL_RESOURCE,           // anpr_crnn.onnx
    LICENSE_PLATE_PYTHON_OCR_SCRIPT_RESOURCE,   // anpr_ocr.py ← НОВОЕ
    LICENSE_PLATE_PYTHON_MODEL_RESOURCE,        // crnn_ocr_model_best.pth ← НОВОЕ
    ONNX_RUNTIME_RESOURCE,                      // onnxruntime.dll
];
```

---

## 🎯 Что Дальше?

1. ✅ Файлы подготовлены в `release-anpr\`
2. ⏳ **ВЫ**: Загружаете на GitHub Release
3. ⏳ **ВЫ**: Даёте мне URL (или просто имя тега релиза)
4. ⏳ **Я**: Обновляю код с правильными URL и SHA256
5. ⏳ Тестируем загрузку в приложении

---

## 📊 Итоговая Статистика

### Экономия места в дистрибутиве:
```
Раньше в bundle:
- python_src/anpr_ocr.py: 19 KB
- (остальные файлы уже загружались)
────────────────────────────────
Экономия: ~19 KB
```

### Общая загрузка при первой активации модуля:
```
ONNX модели:                178.94 MB (уже загружались)
+ Python OCR:                33.28 MB (НОВОЕ)
────────────────────────────────────────────
Итого:                      212.22 MB
```

---

## 📝 Заметки

- Файлы в `release-anpr\` готовы к загрузке
- SHA256 вычислены и указаны выше
- После загрузки не забудьте дать мне ссылки!
- Можете загружать как Release или в отдельный репозиторий
