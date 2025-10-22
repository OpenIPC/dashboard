import cv2 as cv
import numpy as np
import os
import time
from lpd_yunet import LPD_YuNet
try:
    from lpd_yunet_ort import LPD_YuNetORT
    ORT_AVAILABLE = True
except Exception:
    LPD_YuNetORT = None
    ORT_AVAILABLE = False
try:
    import easyocr
    EASY_AVAILABLE = True
except ImportError:
    easyocr = None
    EASY_AVAILABLE = False

try:
    from paddleocr import PaddleOCR
    PADDLE_AVAILABLE = True
except ImportError:
    PADDLE_AVAILABLE = False

try:
    import pytesseract
    TESSERACT_AVAILABLE = True
except ImportError:
    pytesseract = None
    TESSERACT_AVAILABLE = False

MODEL_PATH = os.path.join(os.path.dirname(__file__), 'license_plate_detection.onnx')
CONF_THRESHOLD = 0.3
NMS_THRESHOLD = 0.3
TOP_K = 5000
KEEP_TOP_K = 750

# --- Вспомогательные функции (можно доработать при необходимости) ---
def enhance_plate_image(plate_img, scale_factor=3.0):
    try:
        if len(plate_img.shape) == 3:
            gray = cv.cvtColor(plate_img, cv.COLOR_BGR2GRAY)
        else:
            gray = plate_img.copy()
        blurred = cv.GaussianBlur(gray, (3, 3), 0)
        clahe = cv.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
        enhanced = clahe.apply(blurred)
        filtered = cv.bilateralFilter(enhanced, 9, 75, 75)
        kernel = cv.getStructuringElement(cv.MORPH_RECT, (2, 2))
        morphed = cv.morphologyEx(filtered, cv.MORPH_CLOSE, kernel)
        if scale_factor > 1.0:
            h, w = morphed.shape[:2]
            new_w = int(w * scale_factor)
            new_h = int(h * scale_factor)
            resized = cv.resize(morphed, (new_w, new_h), interpolation=cv.INTER_CUBIC)
        else:
            resized = morphed
        kernel_sharp = np.array([[-1,-1,-1],[-1,9,-1],[-1,-1,-1]])
        sharpened = cv.filter2D(resized, -1, kernel_sharp)
        if len(plate_img.shape) == 3:
            sharpened_bgr = cv.cvtColor(sharpened, cv.COLOR_GRAY2BGR)
            return sharpened_bgr
        return sharpened
    except Exception:
        return plate_img

ALLOWED_CHARS = 'АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'


def normalize_text(text):
    if not text:
        return ""
    cleaned = ''.join(c for c in text if c.isalnum() or c in ALLOWED_CHARS)
    return cleaned.upper()


def recognize_text_easyocr(img, reader):
    if not EASY_AVAILABLE or reader is None:
        return ""
    try:
        results = reader.readtext(img, detail=1, paragraph=False, allowlist=ALLOWED_CHARS)
        lines = [result[1] for result in results if len(result) > 2 and result[2] > 0.2]
        return normalize_text(' '.join(lines))
    except Exception:
        return ""


def recognize_text_paddle(img, reader):
    if not PADDLE_AVAILABLE or reader is None:
        return ""
    try:
        # PaddleOCR returns a list of detections per image
        ocr_results = reader.ocr(img, cls=True)
        texts = []
        for result in ocr_results:
            for entry in result:
                if len(entry) >= 2:
                    candidate = entry[1][0] if isinstance(entry[1], (list, tuple)) else entry[1]
                    texts.append(candidate)
        return normalize_text(' '.join(texts))
    except Exception:
        return ""


def recognize_text_tesseract(img):
    if not TESSERACT_AVAILABLE:
        return ""
    try:
        config = '--oem 3 --psm 7'
        text = pytesseract.image_to_string(img, config=config, lang='eng+rus')
        return normalize_text(text)
    except Exception:
        return ""


def recognize_text(img, easy_reader=None, paddle_reader=None):
    # Prefer PaddleOCR on Linux because it does not require PyTorch-sized dependencies
    text = recognize_text_paddle(img, paddle_reader)
    if text:
        return text
    text = recognize_text_easyocr(img, easy_reader)
    if text:
        return text
    return recognize_text_tesseract(img)

def recognize_plate_from_rtsp(rtsp_url, frame_skip=2, min_score=0.65, min_area=2000, min_height=40, min_aspect=1.2, max_aspect=7.5, max_frames=10, use_ort=False):
    """
    Recognize plates from RTSP stream.
    If `use_ort=True` and ONNX Runtime wrapper is available, it will be used (attempts to use DirectML provider).
    Otherwise falls back to OpenCV-based `LPD_YuNet`.
    """
    if use_ort and ORT_AVAILABLE:
        model = LPD_YuNetORT(
            modelPath=MODEL_PATH,
            inputSize=[320, 240],
            confThreshold=CONF_THRESHOLD,
            nmsThreshold=NMS_THRESHOLD,
            topK=TOP_K,
            keepTopK=KEEP_TOP_K,
            prefer_dml=True
        )
    else:
        model = LPD_YuNet(
            modelPath=MODEL_PATH,
            confThreshold=CONF_THRESHOLD,
            nmsThreshold=NMS_THRESHOLD,
            topK=TOP_K,
            keepTopK=KEEP_TOP_K,
            backendId=cv.dnn.DNN_BACKEND_OPENCV,
            targetId=cv.dnn.DNN_TARGET_CPU
        )
    easyocr_reader = easyocr.Reader(['en', 'ru']) if EASY_AVAILABLE else None
    paddle_reader = PaddleOCR(lang='en', use_angle_cls=True, show_log=False, use_gpu=False) if PADDLE_AVAILABLE else None
    cap = cv.VideoCapture(rtsp_url)
    frame_count = 0
    results = []
    while cap.isOpened() and frame_count < max_frames:
        ret, frame = cap.read()
        if not ret:
            break
        frame_count += 1
        if frame_count % frame_skip != 0:
            continue
        h, w = frame.shape[:2]
        min_w, min_h = 320, 240
        input_w = max(w, min_w)
        input_h = max(h, min_h)
        model.setInputSize([input_w, input_h])
        frame_resized = cv.resize(frame, (input_w, input_h))
        detections = model.infer(frame_resized)
        for det in detections:
            bbox = det[:-1].astype(np.float32)
            score = det[-1]
            pts = np.array([[bbox[0], bbox[1]], [bbox[2], bbox[3]], [bbox[4], bbox[5]], [bbox[6], bbox[7]]], dtype=np.float32)
            # Map coordinates from model input space back to original frame size
            orig_h, orig_w = frame.shape[:2]
            input_w = input_w if 'input_w' in locals() else frame.shape[1]
            input_h = input_h if 'input_h' in locals() else frame.shape[0]
            scale_x = float(orig_w) / float(input_w)
            scale_y = float(orig_h) / float(input_h)
            pts[:, 0] = np.clip(np.round(pts[:, 0] * scale_x), 0, orig_w - 1)
            pts[:, 1] = np.clip(np.round(pts[:, 1] * scale_y), 0, orig_h - 1)
            pts = pts.astype(np.int32)
            x_min, y_min = np.min(pts, axis=0)
            x_max, y_max = np.max(pts, axis=0)
            width = x_max - x_min
            height = y_max - y_min
            area = width * height
            aspect = width / height if height > 0 else 0
            if score < min_score or area < min_area or height < min_height or not (min_aspect < aspect < max_aspect):
                continue
            plate_img = frame[y_min:y_max, x_min:x_max]
            plate_img = enhance_plate_image(plate_img)
            text = recognize_text(plate_img, easyocr_reader, paddle_reader)
            results.append({
                "score": float(score),
                "text": text,
                "bbox": [int(x_min), int(y_min), int(x_max), int(y_max)]
            })
    cap.release()
    return results
