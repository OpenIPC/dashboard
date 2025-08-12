# --- ФАЙЛ: python_src/analytics.py (финальная версия с выбором провайдера) ---

import sys
import os
import json
import time
import base64
import threading

if getattr(sys, 'frozen', False):
    application_path = sys._MEIPASS
else:
    application_path = os.path.dirname(os.path.abspath(__file__))

try:
    import cv2
    import numpy as np
    import onnxruntime as ort
except Exception as e:
    error_message = f"Fatal error during library import: {str(e)}"
    print(json.dumps({"status": "error", "message": error_message}), flush=True)
    sys.exit(1)

# --- Классы и функции-хелперы (без изменений) ---
COCO_CLASSES = {
    0: 'person', 1: 'bicycle', 2: 'car', 3: 'motorcycle', 4: 'airplane', 5: 'bus', 6: 'train', 7: 'truck',
    8: 'boat', 9: 'traffic light', 10: 'fire hydrant', 11: 'stop sign', 12: 'parking meter', 13: 'bench',
    14: 'bird', 15: 'cat', 16: 'dog', 17: 'horse', 18: 'sheep', 19: 'cow', 20: 'elephant', 21: 'bear',
    22: 'zebra', 23: 'giraffe', 24: 'backpack', 25: 'umbrella', 26: 'handbag', 27: 'tie', 28: 'suitcase',
    29: 'frisbee', 30: 'skis', 31: 'snowboard', 32: 'sports ball', 33: 'kite', 34: 'baseball bat',
    35: 'baseball glove', 36: 'skateboard', 37: 'surfboard', 38: 'tennis racket', 39: 'bottle',
    40: 'wine glass', 41: 'cup', 42: 'fork', 43: 'knife', 44: 'spoon', 45: 'bowl', 46: 'banana',
    47: 'apple', 48: 'sandwich', 49: 'orange', 50: 'broccoli', 51: 'carrot', 52: 'hot dog', 53: 'pizza',
    54: 'donut', 55: 'cake', 56: 'chair', 57: 'couch', 58: 'potted plant', 59: 'bed', 60: 'dining table',
    61: 'toilet', 62: 'tv', 63: 'laptop', 64: 'mouse', 65: 'remote', 66: 'keyboard', 67: 'cell phone',
    68: 'microwave', 69: 'oven', 70: 'toaster', 71: 'sink', 72: 'refrigerator', 73: 'book', 74: 'clock',
    75: 'vase', 76: 'scissors', 77: 'teddy bear', 78: 'hair drier', 79: 'toothbrush'
}

class FrameGrabber:
    def __init__(self, src=0):
        self.stream = cv2.VideoCapture(src)
        if not self.stream.isOpened():
            raise IOError("Cannot open video stream")
        self.ret, self.frame = self.stream.read()
        self.stopped = False
        self.thread = threading.Thread(target=self.update, args=())
        self.thread.daemon = True
    def start(self):
        self.stopped = False
        self.thread.start()
    def update(self):
        while not self.stopped:
            ret, frame = self.stream.read()
            if not ret:
                self.stop()
                break
            self.ret, self.frame = ret, frame
    def read(self):
        return self.ret, self.frame
    def stop(self):
        self.stopped = True
        if self.thread.is_alive():
            self.thread.join(timeout=1.0)
        self.stream.release()

def preprocess(img, input_width, input_height):
    img_height, img_width = img.shape[:2]
    ratio = min(input_width / img_width, input_height / img_height)
    new_width, new_height = int(img_width * ratio), int(img_height * ratio)
    resized_img = cv2.resize(img, (new_width, new_height), interpolation=cv2.INTER_LINEAR)
    padded_img = np.full((input_height, input_width, 3), 114, dtype=np.uint8)
    padded_img[(input_height - new_height) // 2 : (input_height - new_height) // 2 + new_height,
               (input_width - new_width) // 2 : (input_width - new_width) // 2 + new_width] = resized_img
    image_data = np.array(padded_img, dtype=np.float32) / 255.0
    image_data = np.transpose(image_data, (2, 0, 1))
    image_data = np.expand_dims(image_data, axis=0)
    return image_data, ratio, ((input_width - new_width) // 2, (input_height - new_height) // 2)

def postprocess(output, ratio, pad, confidence_threshold=0.5, iou_threshold=0.5):
    predictions = np.squeeze(output).T
    scores = np.max(predictions[:, 4:], axis=1)
    predictions = predictions[scores > confidence_threshold, :]
    scores = scores[scores > confidence_threshold]
    if predictions.shape[0] == 0:
        return []
    class_ids = np.argmax(predictions[:, 4:], axis=1)
    boxes = predictions[:, :4]
    x, y, w, h = boxes[:, 0], boxes[:, 1], boxes[:, 2], boxes[:, 3]
    x1, y1, x2, y2 = x - w / 2, y - h / 2, x + w / 2, y + h / 2
    x1, y1, x2, y2 = (x1 - pad[0]) / ratio, (y1 - pad[1]) / ratio, (x2 - pad[0]) / ratio, (y2 - pad[1]) / ratio
    indices = cv2.dnn.NMSBoxes(np.column_stack((x1, y1, x2 - x1, y2 - y1)), scores, confidence_threshold, iou_threshold)
    detections = []
    for i in indices:
        detections.append({
            'label': COCO_CLASSES[class_ids[i]],
            'confidence': float(scores[i]),
            'box': {'x': int(x1[i]), 'y': int(y1[i]), 'w': int(x2[i] - x1[i]), 'h': int(y2[i] - y1[i])}
        })
    return detections
# --- Конец хелперов ---

def run_analytics(rtsp_url, config_str, provider_choice='auto'):
    try:
        model_path = os.path.join(application_path, 'yolov8n.onnx')
        session = None
        
        # VVVVVV --- ИЗМЕНЕНИЕ: Логика выбора провайдера на основе аргумента --- VVVVVV
        available_providers = ort.get_available_providers()
        
        def try_provider(provider_name):
            nonlocal session
            if provider_name in available_providers:
                try:
                    session = ort.InferenceSession(model_path, providers=[provider_name, 'CPUExecutionProvider'])
                    print(json.dumps({"status": "info", "provider": provider_name}), flush=True)
                    return True
                except Exception as e:
                    print(json.dumps({"status": "info", "provider": provider_name, "error": f"{provider_name} failed: {str(e)}"}), flush=True)
                    session = None
            return False

        if provider_choice == 'dml':
            try_provider('DmlExecutionProvider')
        elif provider_choice == 'auto':
            # В режиме "Авто" для Windows пробуем DML, для остальных - сразу CPU
            if sys.platform == "win32":
                if not try_provider('DmlExecutionProvider'):
                    try_provider('CPUExecutionProvider')
            else:
                 try_provider('CPUExecutionProvider')
        
        # Если выбор был 'cpu' или все остальное провалилось, используем CPU
        if session is None:
            if not try_provider('CPUExecutionProvider'):
                 raise RuntimeError("Could not initialize any ONNX Runtime provider.")

        # ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^

        input_name = session.get_inputs()[0].name
        output_name = session.get_outputs()[0].name
        input_height = session.get_inputs()[0].shape[2]
        input_width = session.get_inputs()[0].shape[3]
    except Exception as e:
        print(json.dumps({"status": "error", "message": f"Failed to load ONNX model or provider: {str(e)}"}), flush=True)
        sys.exit(1)
        
    # --- Основной цикл (без изменений) ---
    config = {}
    if config_str:
        try:
            config_json = base64.b64decode(config_str).decode('utf-8')
            config = json.loads(config_json)
        except Exception:
            pass
    
    objects_to_detect = config.get('objects', None)
    confidence_threshold = config.get('confidence', 0.5)
    frame_skip = int(config.get('frame_skip', 5)) or 1
    frame_grabber = None
    
    while True: 
        try:
            if frame_grabber is None or frame_grabber.stopped:
                try:
                    frame_grabber = FrameGrabber(rtsp_url)
                    frame_grabber.start()
                    time.sleep(2) 
                except IOError as e:
                    print(json.dumps({"status": "error", "message": str(e)}), flush=True)
                    time.sleep(5)
                    continue
            frame_count = 0
            while not frame_grabber.stopped:
                ret, frame = frame_grabber.read()
                if not ret or frame is None:
                    time.sleep(0.5)
                    if frame_grabber.stopped:
                        break 
                    continue
                frame_count += 1
                if frame_count % frame_skip != 0:
                    continue
                input_tensor, ratio, pad = preprocess(frame, input_width, input_height)
                outputs = session.run([output_name], {input_name: input_tensor})
                detections = postprocess(outputs[0], ratio, pad, confidence_threshold)
                
                filtered_objects = [obj for obj in detections if obj['label'] in objects_to_detect] if objects_to_detect else detections

                if len(filtered_objects) > 0:
                    result = {
                        "status": "objects_detected",
                        "timestamp": time.time(),
                        "objects": filtered_objects
                    }
                    print(json.dumps(result), flush=True)
        
        except Exception as e:
            print(json.dumps({"status": "error", "message": f"Runtime error: {str(e)}"}), flush=True)
            if frame_grabber:
                frame_grabber.stop()
            frame_grabber = None
            time.sleep(5)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        rtsp_stream_url = sys.argv[1]
        config_arg = sys.argv[2] if len(sys.argv) > 2 else None
        # VVVVVV --- ИЗМЕНЕНИЕ: Читаем третий аргумент --- VVVVVV
        provider_arg = sys.argv[3] if len(sys.argv) > 3 else 'auto'
        run_analytics(rtsp_stream_url, config_arg, provider_arg)
        # ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^
    else:
        print(json.dumps({"status": "error", "message": "RTSP URL not provided"}), flush=True)
        sys.exit(1)