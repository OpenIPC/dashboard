# 📦 Файлы для Модуля ANPR (License Plate Detection)

## Список файлов для загрузки на GitHub Release

### 🐍 Python Скрипты

#### 1. `anpr_ocr.py` (Основной скрипт OCR)
- **Расположение**: `src-tauri/python_src/anpr_ocr.py`
- **Размер**: 19 KB (0.02 MB)
- **Назначение**: Основной Python скрипт для OCR распознавания номеров
- **Используется**: При режиме `use_python_ocr = true`
- **Требуется для**: Windows (основной режим)

#### 2. `crnn_ocr_model_best.pth` (PyTorch модель для Python OCR)
- **Расположение**: `src-tauri/python_src/anpr/crnn_ocr_model_best.pth`
- **Размер**: 34.88 MB (33.26 MB)
- **Назначение**: PyTorch модель CRNN для Python OCR
- **Используется**: Вместе с `anpr_ocr.py`
- **Требуется для**: Windows (только если используется Python OCR)

---

### 🤖 ONNX Модели (уже загружаются)

#### 3. `anpr_yolov8.onnx` (YOLO детекция номеров)
- **Расположение**: `artifacts/anpr/anpr_yolov8.onnx`
- **Размер**: 12.25 MB (11.68 MB)
- **Назначение**: YOLOv8 модель для детекции номерных знаков
- **Используется**: Всегда (основная детекция)
- **Уже настроено**: ✅ URL в `LICENSE_PLATE_DETECTOR_MODEL_RESOURCE`

#### 4. `anpr_crnn.onnx` (CRNN OCR модель)
- **Расположение**: `artifacts/anpr/anpr_crnn.onnx`
- **Размер**: 34.87 MB (33.26 MB)
- **Назначение**: CRNN модель для Rust ONNX OCR (fallback)
- **Используется**: Когда Python OCR недоступен
- **Уже настроено**: ✅ URL в `LICENSE_PLATE_OCR_MODEL_RESOURCE`

---

## 📊 Итоговая Таблица

| # | Файл | Размер | Где Сейчас | Где Загружать | Нужен для Загрузки |
|---|------|--------|------------|---------------|-------------------|
| 1 | `anpr_ocr.py` | 19 KB | `src-tauri/python_src/` | GitHub Repo/Release | ✅ ДА |
| 2 | `crnn_ocr_model_best.pth` | 33.26 MB | `src-tauri/python_src/anpr/` | GitHub Release | ✅ ДА |
| 3 | `anpr_yolov8.onnx` | 11.68 MB | `artifacts/anpr/` | Уже загружено | ❌ НЕТ |
| 4 | `anpr_crnn.onnx` | 33.26 MB | `artifacts/anpr/` | Уже загружено | ❌ НЕТ |

---

## 🎯 Что Нужно Сделать

### Вариант 1: GitHub Release (Рекомендуется)

Создать GitHub Release с тегом например `anpr-resources-v1.0.0` и загрузить:

```
anpr-resources-v1.0.0/
├── anpr_ocr.py (19 KB)
└── crnn_ocr_model_best.pth (33.26 MB)
```

**URL для кода:**
```
https://github.com/OpenIPC/dashboard/releases/download/anpr-resources-v1.0.0/anpr_ocr.py
https://github.com/OpenIPC/dashboard/releases/download/anpr-resources-v1.0.0/crnn_ocr_model_best.pth
```

### Вариант 2: GitHub Raw (Только для мелких файлов)

Для `anpr_ocr.py` (19 KB) можно использовать GitHub Raw:
```
https://raw.githubusercontent.com/OpenIPC/dashboard/main/src-tauri/python_src/anpr_ocr.py
```

Для `crnn_ocr_model_best.pth` (33 MB) - **только через Release!** (GitHub Raw лимит 100 MB, но медленно)

---

## 📝 Обновление Кода После Загрузки

После того как вы загрузите файлы на GitHub и дадите мне ссылки, я обновлю:

### 1. `src-tauri/src/analytics.rs`

**Сейчас:**
```rust
const LICENSE_PLATE_PYTHON_INFERENCE_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://raw.githubusercontent.com/OpenIPC/dashboard/main/src-tauri/python_src/inference.py",
        file_name: "inference.py",
        sha256: None,
    },
);
```

**Будет:**
```rust
const LICENSE_PLATE_PYTHON_OCR_SCRIPT_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/OpenIPC/dashboard/releases/download/anpr-resources-v1.0.0/anpr_ocr.py",
        file_name: "anpr_ocr.py",
        sha256: Some("..."), // Можно добавить SHA256
    },
);

const LICENSE_PLATE_PYTHON_MODEL_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/OpenIPC/dashboard/releases/download/anpr-resources-v1.0.0/crnn_ocr_model_best.pth",
        file_name: "crnn_ocr_model_best.pth",
        sha256: Some("..."), // Можно добавить SHA256
    },
);
```

### 2. Обновление `LICENSE_PLATE_RESOURCES`

```rust
#[cfg(target_os = "windows")]
const LICENSE_PLATE_RESOURCES: &[ModuleResourceSpec] = &[
    LICENSE_PLATE_DETECTOR_MODEL_RESOURCE,      // anpr_yolov8.onnx (уже есть)
    LICENSE_PLATE_OCR_MODEL_RESOURCE,           // anpr_crnn.onnx (уже есть)
    LICENSE_PLATE_PYTHON_OCR_SCRIPT_RESOURCE,   // anpr_ocr.py (НОВОЕ!)
    LICENSE_PLATE_PYTHON_MODEL_RESOURCE,        // crnn_ocr_model_best.pth (НОВОЕ!)
    ONNX_RUNTIME_RESOURCE,                      // onnxruntime.dll
];
```

### 3. Обновление поиска Python модели в `license_plate.rs`

Нужно будет изменить поиск модели:

**Сейчас:**
```rust
let model_path = script_path
    .parent()
    .map(|p| p.join("anpr").join(PYTHON_OCR_MODEL_FILE))
```

**Будет:**
```rust
// Сначала ищем в module_dir (загружено), потом в anpr/
let model_path = vec![
    Some(module_dir.join(PYTHON_OCR_MODEL_FILE)),
    script_path.parent().map(|p| p.join("anpr").join(PYTHON_OCR_MODEL_FILE)),
]
.into_iter()
.flatten()
.find(|p| p.exists())
.ok_or_else(|| "Python OCR model not found")?;
```

---

## ⚙️ Расчёт Размера Загрузки

### Без Python OCR (только ONNX):
```
anpr_yolov8.onnx:     11.68 MB
anpr_crnn.onnx:       33.26 MB
onnxruntime.dll:     143.00 MB
─────────────────────────────
Итого:               187.94 MB
```

### С Python OCR (полная установка):
```
anpr_yolov8.onnx:            11.68 MB
anpr_crnn.onnx:              33.26 MB
anpr_ocr.py:                  0.02 MB
crnn_ocr_model_best.pth:     33.26 MB
onnxruntime.dll:            143.00 MB
───────────────────────────────────
Итого:                      221.22 MB
```

**Разница**: +33.28 MB при использовании Python OCR

---

## 🔍 Проверка SHA256 (Опционально)

Если хотите добавить SHA256 проверку:

```powershell
# Windows PowerShell
Get-FileHash "src-tauri\python_src\anpr_ocr.py" -Algorithm SHA256
Get-FileHash "src-tauri\python_src\anpr\crnn_ocr_model_best.pth" -Algorithm SHA256
```

Или через Python:
```python
import hashlib

def sha256sum(filename):
    h = hashlib.sha256()
    with open(filename, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b''):
            h.update(chunk)
    return h.hexdigest()

print(sha256sum("src-tauri/python_src/anpr_ocr.py"))
print(sha256sum("src-tauri/python_src/anpr/crnn_ocr_model_best.pth"))
```

---

## 📤 Команды для Подготовки Файлов

```powershell
# Создать директорию для релиза
New-Item -ItemType Directory -Force -Path "release-anpr"

# Скопировать нужные файлы
Copy-Item "src-tauri\python_src\anpr_ocr.py" "release-anpr\"
Copy-Item "src-tauri\python_src\anpr\crnn_ocr_model_best.pth" "release-anpr\"

# Проверить содержимое
Get-ChildItem "release-anpr" -File | Select-Object Name, Length, @{Name='SizeMB';Expression={[math]::Round($_.Length/1MB, 2)}}
```

---

## 🎯 Следующие Шаги

1. **Вы**: Загружаете файлы на GitHub (Release или репозиторий)
2. **Вы**: Даёте мне ссылки на файлы
3. **Я**: Обновляю код с правильными URL
4. **Мы**: Тестируем загрузку в приложении

---

## 📌 Важные Замечания

- `anpr_crnn.onnx` и `anpr_yolov8.onnx` уже загружаются из существующих репозиториев - их **НЕ НУЖНО** загружать повторно
- Нужны только `anpr_ocr.py` и `crnn_ocr_model_best.pth`
- Рекомендуется использовать **GitHub Release** для больших файлов
- SHA256 checksums опциональны, но повышают безопасность
