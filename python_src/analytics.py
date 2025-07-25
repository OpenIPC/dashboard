# analytics.py (версия с устранением задержки и оптимизациями)
import cv2
import sys
import json
import time
import base64
import threading # <--- Импортируем модуль для работы с потоками

try:
    from ultralytics import YOLO
except ImportError:
    print(json.dumps({
        "status": "error", 
        "message": "Ultralytics YOLO library not found. Please run 'pip install ultralytics'."
    }), flush=True)
    sys.exit(1)

# Класс для чтения видеопотока в отдельном потоке
class FrameGrabber:
    """
    Класс для захвата кадров из видеопотока в отдельном потоке,
    чтобы избежать накопления задержки в буфере cv2.VideoCapture.
    """
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
            self.ret = ret
            self.frame = frame

    def read(self):
        return self.ret, self.frame

    def stop(self):
        self.stopped = True
        if self.thread.is_alive():
            self.thread.join(timeout=1.0)
        self.stream.release()


def run_analytics(rtsp_url, config_str):
    try:
        model = YOLO("yolov8n.pt")
    except Exception as e:
        print(json.dumps({"status": "error", "message": f"Failed to load YOLOv8 model: {e}"}), flush=True)
        sys.exit(1)

    config = {}
    if config_str:
        try:
            config_json = base64.b64decode(config_str).decode('utf-8')
            config = json.loads(config_json)
        except Exception as e:
            print(json.dumps({"status": "error", "message": f"Invalid config provided: {e}"}), flush=True)
    
    roi = config.get('roi')
    objects_to_detect = config.get('objects', None)
    confidence_threshold = config.get('confidence', 0.5)
    frame_skip = int(config.get('frame_skip', 5))
    if frame_skip < 1:
        frame_skip = 1
    resize_width = int(config.get('resize_width', 640))

    # Используем наш новый класс FrameGrabber
    try:
        frame_grabber = FrameGrabber(rtsp_url)
        frame_grabber.start()
        time.sleep(2) # Даем время на подключение и заполнение первого кадра
    except IOError as e:
        print(json.dumps({"status": "error", "message": str(e)}), flush=True)
        sys.exit(1)
        
    frame_count = 0

    # Основной цикл и блок finally для корректного завершения
    try:
        while not frame_grabber.stopped:
            ret, frame = frame_grabber.read()
            if not ret or frame is None:
                # Поток мог завершиться, даем ему немного времени и проверяем снова
                time.sleep(0.5)
                if frame_grabber.stopped:
                    break
                continue
            
            frame_count += 1
            if frame_count % frame_skip != 0:
                # VVV ИЗМЕНЕНИЕ: Замена sleep на continue VVV
                # Это более эффективно, так как не вносит искусственную задержку.
                # Цикл просто перейдет к следующей итерации.
                continue
                # ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^

            original_height, original_width = frame.shape[:2]
            
            scale_x, scale_y = 1.0, 1.0
            if resize_width > 0 and original_width > resize_width:
                scale_x = original_width / resize_width
                new_height = int(original_height / scale_x)
                scale_y = original_height / new_height
                frame_to_process = cv2.resize(frame, (resize_width, new_height), interpolation=cv2.INTER_AREA)
            else:
                frame_to_process = frame

            results = model(frame_to_process, verbose=False, conf=confidence_threshold)
            
            detected_objects = []
            for box in results[0].boxes:
                class_id = int(box.cls[0])
                label = model.names[class_id]
                
                x1, y1, x2, y2 = box.xyxy[0]
                x, y, w, h = int(x1), int(y1), int(x2 - x1), int(y2 - y1)
                
                detected_objects.append({
                    'label': label,
                    'confidence': float(box.conf[0]),
                    'box': {
                        'x': int(x * scale_x),
                        'y': int(y * scale_y),
                        'w': int(w * scale_x),
                        'h': int(h * scale_y)
                    }
                })

            filtered_objects = []
            for obj in detected_objects:
                if objects_to_detect and obj['label'] not in objects_to_detect:
                    continue

                if roi:
                    box = obj['box']
                    obj_center_x = box['x'] + box['w'] / 2
                    obj_center_y = box['y'] + box['h'] / 2
                    
                    roi_x1 = roi['x'] * original_width
                    roi_y1 = roi['y'] * original_height
                    roi_x2 = (roi['x'] + roi['w']) * original_width
                    roi_y2 = (roi['y'] + roi['h']) * original_height
                    
                    if not (roi_x1 < obj_center_x < roi_x2 and roi_y1 < obj_center_y < roi_y2):
                        continue
                
                filtered_objects.append(obj)
                
            if len(filtered_objects) > 0:
                result = {
                    "status": "objects_detected",
                    "timestamp": time.time(),
                    "objects": filtered_objects
                }
                print(json.dumps(result), flush=True)
                
    finally:
        print(json.dumps({"status": "info", "message": "Analytics process stopping."}), flush=True)
        frame_grabber.stop()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        rtsp_stream_url = sys.argv[1]
        config_arg = sys.argv[2] if len(sys.argv) > 2 else None
        run_analytics(rtsp_stream_url, config_arg)
    else:
        print(json.dumps({"status": "error", "message": "RTSP URL not provided"}), flush=True)
        sys.exit(1)