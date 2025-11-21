# Экспорт моделей ANPR в ONNX

Заготовка Python-скрипта лежит в `tools/anpr/export_models.py`. Чтобы выполнить экспорт:

1. Склонируйте репозиторий Runoi/ANPR-System во внешнюю директорию, например `external/anpr-system`.
2. Создайте Python-окружение:
   ```powershell
   python -m venv .venv
   .\.venv\Scripts\Activate.ps1
   pip install torch torchvision ultralytics
   ```
3. Запустите скрипт экспорта с указанием путей к моделям и выходной директории для ONNX-файлов:
   ```powershell
   python tools/anpr/export_models.py --repo-path external/anpr-system --out-dir artifacts/anpr
   ```
4. В результате появятся два файла:
   - `artifacts/anpr/anpr_yolov8.onnx`
   - `artifacts/anpr/anpr_crnn.onnx`

Эти ONNX-модели будут использоваться в ONNX Runtime рантайме модуля `License Plate`.
