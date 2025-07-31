# export_model.py
from ultralytics import YOLO

# Загружаем стандартную модель YOLOv8n
model = YOLO("yolov8n.pt")

# Экспортируем её в формат ONNX
# opset=12 - хорошая версия для совместимости
# simplify=True - оптимизирует граф модели
model.export(format="onnx", opset=12, simplify=True)

print("Модель успешно экспортирована в 'yolov8n.onnx'")