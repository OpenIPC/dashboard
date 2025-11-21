# License Plate Module Integration Plan

## Цели

- Использовать модели из проекта [Runoi/ANPR-System](https://github.com/Runoi/ANPR-System)
- Подготовить их к выполнению в ONNX Runtime (CPU / DirectML)
- Добавить отдельный модуль `license-plate` в Dashboard

## Экспорт моделей

Скрипт: `tools/anpr/export_models.py`

1. Подготовить локальную копию репозитория ANPR-System
2. Выполнить экспорт `YOLO -> anpr_yolov8.onnx`, `CRNN -> anpr_crnn.onnx`
3. Сохранить файлы в `artifacts/anpr`
4. Скопировать готовые ONNX модели в `src-tauri/python_src/anpr` для упаковки рантайма (`anpr_yolov8.onnx`, `anpr_crnn.onnx`)

## Рантайм

Промежуточный прототип лежит в `src-tauri/python_src/anpr/license_plate_runtime.py`. Он:
- Загружает ONNX модели через ONNX Runtime
- Подключается к RTSP
- Сейчас выводит заглушку `detections: []`

Дальше нужно:
1. Реализовать пост-обработку YOLO (bbox, confidence, фильтрация)
2. Реализовать cropping, препроцессинг, OCR, стабилизацию текста
3. Вернуть JSON интерфейс, совместимый с менеджером аналитики

## Следующие шаги

- Дописать пост-обработку / OCR в рантайме
- Создать PyInstaller spec и собрать бинарники
- Подготовить архив модуля + обновить `module-registry.json`
- Сделать UI/локализацию для параметров модуля
