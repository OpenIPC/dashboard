"""License Plate analytics runtime.

Loads YOLO detector + CRNN OCR ONNX models via ONNX Runtime.
Processes frames from RTSP, returns JSON with detections.
"""

from __future__ import annotations

import base64
import json
import os
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import cv2
import numpy as np
import onnxruntime as ort

# Try to import EasyOCR for better Russian plate recognition
try:
    import easyocr
    EASYOCR_AVAILABLE = True
except ImportError:
    EASYOCR_AVAILABLE = False

APPLICATION_ROOT = Path(getattr(sys, "_MEIPASS", Path(__file__).resolve().parent))


@dataclass
class RuntimeConfig:
    confidence_threshold: float = 0.5  # Lowered for more detections (including angled plates)
    iou_threshold: float = 0.45
    track_buffer: int = 15
    max_results: int = 5
    min_plate_area: int = 800  # Lowered from 1000 to catch smaller plates
    min_plate_height: int = 25  # Lowered from 30
    min_aspect_ratio: float = 1.8  # Lowered from 2.0 for angled plates
    max_aspect_ratio: float = 8.0  # Increased from 7.0

    @classmethod
    def from_base64(cls, payload: Optional[str]) -> "RuntimeConfig":
        if not payload:
            return cls()
        try:
            decoded = base64.b64decode(payload).decode("utf-8")
            data = json.loads(decoded)
        except Exception:
            return cls()
        return cls(
            confidence_threshold=float(data.get("confidence", 0.5)),
            iou_threshold=float(data.get("iou", 0.45)),
            track_buffer=int(data.get("track_buffer", 15)),
            min_plate_area=int(data.get("min_plate_area", 800)),
            min_plate_height=int(data.get("min_plate_height", 25)),
            min_aspect_ratio=float(data.get("min_aspect_ratio", 1.8)),
            max_aspect_ratio=float(data.get("max_aspect_ratio", 8.0)),
        )


class ONNXDetector:
    def __init__(self, model_path: Path, providers: List[str]):
        self.session = ort.InferenceSession(str(model_path), providers=providers)
        self.input_name = self.session.get_inputs()[0].name
        output = self.session.get_outputs()[0]
        self.output_names = [output.name]
        _, _, self.input_height, self.input_width = self.session.get_inputs()[0].shape

    def preprocess(self, frame: np.ndarray):
        h, w = frame.shape[:2]
        scale = min(self.input_width / w, self.input_height / h)
        nw, nh = int(w * scale), int(h * scale)
        resized = cv2.resize(frame, (nw, nh))
        canvas = np.full((self.input_height, self.input_width, 3), 114, dtype=np.uint8)
        top = (self.input_height - nh) // 2
        left = (self.input_width - nw) // 2
        canvas[top:top + nh, left:left + nw] = resized
        image = canvas[:, :, ::-1].astype(np.float32) / 255.0
        image = np.transpose(image, (2, 0, 1))[None]
        return image, scale, left, top

    def infer(self, frame: np.ndarray) -> np.ndarray:
        blob, scale, pad_x, pad_y = self.preprocess(frame)
        preds = self.session.run(self.output_names, {self.input_name: blob})[0]
        return preds, scale, pad_x, pad_y

    def postprocess(
        self,
        preds: np.ndarray,
        scale: float,
        pad_x: int,
        pad_y: int,
        frame_shape: Tuple[int, int],
        confidence_th: float,
        iou_th: float,
        max_results: int,
        min_area: int = 1000,
        min_height: int = 30,
        min_aspect: float = 2.0,
        max_aspect: float = 7.0,
    ) -> List[Dict[str, Any]]:
        h, w = frame_shape

        if preds.ndim == 3:
            preds = np.squeeze(preds, axis=0)
        elif preds.ndim == 2:
            preds = preds
        else:
            preds = preds.reshape((-1, preds.shape[-1]))

        if preds.shape[0] == 0:
            return []

        # YOLOv8 ONNX export sometimes yields shape (N, 85) where columns: cx,cy,w,h, conf, class scores
        # When using Ultralytics, objectness already applied and columns 4+ contain class confidences.
        if preds.shape[1] >= 85:
            boxes = preds[:, :4]
            scores = preds[:, 4:]
            class_scores = scores.max(axis=1)
            class_ids = scores.argmax(axis=1)
        elif preds.shape[1] == 6:
            # already filtered layout [cx, cy, w, h, conf, class]
            boxes = preds[:, :4]
            class_scores = preds[:, 4]
            class_ids = preds[:, 5].astype(np.int32)
        else:
            return []

        mask = class_scores >= confidence_th
        boxes = boxes[mask]
        class_scores = class_scores[mask]
        class_ids = class_ids[mask]

        if boxes.shape[0] == 0:
            return []

        # Convert from xywh to xyxy, undo padding/scale
        cx, cy, bw, bh = boxes[:, 0], boxes[:, 1], boxes[:, 2], boxes[:, 3]
        x1 = (cx - bw / 2 - pad_x) / scale
        y1 = (cy - bh / 2 - pad_y) / scale
        x2 = (cx + bw / 2 - pad_x) / scale
        y2 = (cy + bh / 2 - pad_y) / scale

        x1 = np.clip(x1, 0, w - 1)
        y1 = np.clip(y1, 0, h - 1)
        x2 = np.clip(x2, 0, w - 1)
        y2 = np.clip(y2, 0, h - 1)

        boxes_xyxy = np.stack([x1, y1, x2, y2], axis=1)

        # NMS
        idxs = self.nms(boxes_xyxy, class_scores, iou_th)
        idxs = idxs[:max_results]

        results: List[Dict[str, Any]] = []
        for i in idxs:
            x1_int, y1_int, x2_int, y2_int = int(boxes_xyxy[i][0]), int(boxes_xyxy[i][1]), int(boxes_xyxy[i][2]), int(boxes_xyxy[i][3])
            
            # Filter by minimum area
            width = x2_int - x1_int
            height = y2_int - y1_int
            area = width * height
            
            if area < min_area:
                continue
            
            if height < min_height:
                continue
            
            # Filter by aspect ratio (Russian plates: 2.0-7.0)
            aspect_ratio = width / max(height, 1)
            if aspect_ratio < min_aspect or aspect_ratio > max_aspect:
                continue
            
            results.append(
                {
                    "bbox": [x1_int, y1_int, x2_int, y2_int],
                    "confidence": float(class_scores[i]),
                    "class_id": int(class_ids[i]),
                }
            )
        return results

    @staticmethod
    def nms(boxes: np.ndarray, scores: np.ndarray, iou_th: float) -> List[int]:
        x1 = boxes[:, 0]
        y1 = boxes[:, 1]
        x2 = boxes[:, 2]
        y2 = boxes[:, 3]

        areas = (x2 - x1 + 1) * (y2 - y1 + 1)
        order = scores.argsort()[::-1]

        keep: List[int] = []
        while order.size > 0:
            i = order[0]
            keep.append(int(i))
            if order.size == 1:
                break
            xx1 = np.maximum(x1[i], x1[order[1:]])
            yy1 = np.maximum(y1[i], y1[order[1:]])
            xx2 = np.minimum(x2[i], x2[order[1:]])
            yy2 = np.minimum(y2[i], y2[order[1:]])

            w = np.maximum(0.0, xx2 - xx1 + 1)
            h = np.maximum(0.0, yy2 - yy1 + 1)
            inter = w * h
            union = areas[i] + areas[order[1:]] - inter
            iou = inter / np.maximum(union, 1e-6)

            inds = np.where(iou <= iou_th)[0]
            order = order[inds + 1]

        return keep


class ONNXRnnRecognizer:
    def __init__(self, model_path: Path, providers: List[str]):
        self.session = ort.InferenceSession(str(model_path), providers=providers)
        self.input_name = self.session.get_inputs()[0].name
        self.output_name = self.session.get_outputs()[0].name
        # CRITICAL: ANPR-System CRNN model alphabet is LATIN characters!
        # Model was trained on dataset with Latin equivalents of Cyrillic letters
        # This matches the original Config.OCR_ALPHABET from inference.py
        self.alphabet = "0123456789ABCEHKMOPTXY"

    def preprocess(self, plate: np.ndarray) -> np.ndarray:
        """Enhanced preprocessing for better OCR accuracy."""
        # Convert to grayscale
        gray = cv2.cvtColor(plate, cv2.COLOR_BGR2GRAY)
        
        # Apply CLAHE (Contrast Limited Adaptive Histogram Equalization)
        clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8, 8))
        enhanced = clahe.apply(gray)
        
        # Bilateral filter to reduce noise while keeping edges sharp
        denoised = cv2.bilateralFilter(enhanced, 9, 75, 75)
        
        # Try adaptive thresholding for better text separation
        # This helps with varying lighting conditions
        binary = cv2.adaptiveThreshold(
            denoised, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
            cv2.THRESH_BINARY, 11, 2
        )
        
        # Morphological operations to clean up
        kernel = np.ones((2, 2), np.uint8)
        morph = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, kernel)
        morph = cv2.morphologyEx(morph, cv2.MORPH_OPEN, kernel)
        
        # Resize to model input size
        resized = cv2.resize(morph, (128, 32))
        
        # Normalize
        normalized = (resized / 255.0 - 0.5) / 0.5
        return normalized[np.newaxis, np.newaxis, :]

    def decode(self, logits: np.ndarray) -> str:
        # Expected logits shape: (seq_len, batch, num_classes)
        if logits.ndim == 3:
            logits = logits
        elif logits.ndim == 2:
            logits = logits[:, None, :]
        else:
            logits = logits.reshape(logits.shape[0], 1, -1)

        indices = logits.argmax(axis=2)
        last = 0
        chars = []
        for idx in indices[:, 0]:
            idx = int(idx)
            if idx != 0 and idx != last:
                if 0 < idx <= len(self.alphabet):
                    chars.append(self.alphabet[idx - 1])
            last = idx
        return "".join(chars)

    def recognize(self, plate: np.ndarray) -> str:
        """Recognize with fallback strategy."""
        # Try with adaptive thresholding first
        try:
            blob = self.preprocess(plate)
            logits = self.session.run([self.output_name], {self.input_name: blob})[0]
            text = self.decode(logits)
            
            # If result is too short or empty, try without binarization
            if len(text) < 6:
                blob_simple = self.preprocess_simple(plate)
                logits_simple = self.session.run([self.output_name], {self.input_name: blob_simple})[0]
                text_simple = self.decode(logits_simple)
                
                # Return longer result
                if len(text_simple) > len(text):
                    return text_simple
            
            return text
        except Exception:
            # Fallback to simple preprocessing
            blob = self.preprocess_simple(plate)
            logits = self.session.run([self.output_name], {self.input_name: blob})[0]
            return self.decode(logits)
    
    def preprocess_simple(self, plate: np.ndarray) -> np.ndarray:
        """Simpler preprocessing without binarization (fallback)."""
        gray = cv2.cvtColor(plate, cv2.COLOR_BGR2GRAY)
        
        # CLAHE only
        clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8, 8))
        enhanced = clahe.apply(gray)
        
        # Resize
        resized = cv2.resize(enhanced, (128, 32))
        
        # Normalize
        normalized = (resized / 255.0 - 0.5) / 0.5
        return normalized[np.newaxis, np.newaxis, :]


class EasyOCRRecognizer:
    """OCR using EasyOCR for better Russian plate recognition."""
    def __init__(self):
        if not EASYOCR_AVAILABLE:
            raise ImportError("EasyOCR not available. Install with: pip install easyocr")
        # Initialize reader with Russian and English languages
        # gpu=False for CPU inference
        # recog_network='standard' for better accuracy
        self.reader = easyocr.Reader(
            ['ru', 'en'], 
            gpu=False, 
            verbose=False,
            recog_network='standard'
        )
    
    def preprocess(self, plate: np.ndarray) -> np.ndarray:
        """Enhanced preprocessing for better OCR accuracy."""
        # Convert to grayscale
        gray = cv2.cvtColor(plate, cv2.COLOR_BGR2GRAY)
        
        # Apply CLAHE for better contrast
        clahe = cv2.createCLAHE(clipLimit=4.0, tileGridSize=(8, 8))
        enhanced = clahe.apply(gray)
        
        # Sharpen the image
        kernel_sharpen = np.array([[-1,-1,-1],
                                   [-1, 9,-1],
                                   [-1,-1,-1]])
        sharpened = cv2.filter2D(enhanced, -1, kernel_sharpen)
        
        # Denoise
        denoised = cv2.fastNlMeansDenoising(sharpened, None, 10, 7, 21)
        
        return denoised
    
    def recognize(self, plate: np.ndarray) -> str:
        """Recognize text from plate image using EasyOCR."""
        preprocessed = self.preprocess(plate)
        
        # EasyOCR returns list of (bbox, text, confidence)
        # allowlist for Russian plates: АВЕКМНОРСТУХ + digits
        # But EasyOCR doesn't support Cyrillic in allowlist well, so we filter after
        results = self.reader.readtext(
            preprocessed, 
            detail=1,
            paragraph=False,
            min_size=10,
            contrast_ths=0.1,
            adjust_contrast=0.5,
            text_threshold=0.5,
            low_text=0.3,
            link_threshold=0.4
        )
        
        if not results:
            return ""
        
        # Get text with highest confidence
        best_result = max(results, key=lambda x: x[2])
        text = best_result[1]
        
        # Remove spaces and convert to uppercase
        text = text.replace(" ", "").replace("-", "").upper()
        
        return text


def select_providers(preference: str) -> List[str]:
    available = ort.get_available_providers()
    if preference == "dml" and "DmlExecutionProvider" in available:
        return ["DmlExecutionProvider", "CPUExecutionProvider"]
    if preference == "cuda" and "CUDAExecutionProvider" in available:
        return ["CUDAExecutionProvider", "CPUExecutionProvider"]
    return ["CPUExecutionProvider"]


def enlarge_and_crop(frame: np.ndarray, bbox: List[int], margin: float = 0.1) -> np.ndarray:
    h, w = frame.shape[:2]
    x1, y1, x2, y2 = bbox
    bw = x2 - x1
    bh = y2 - y1
    dx = int(bw * margin)
    dy = int(bh * margin)
    x1 = max(0, x1 - dx)
    y1 = max(0, y1 - dy)
    x2 = min(w - 1, x2 + dx)
    y2 = min(h - 1, y2 + dy)
    
    cropped = frame[y1:y2, x1:x2].copy()
    
    # Check if crop is too small or distorted
    if cropped.size == 0:
        return cropped
    
    crop_h, crop_w = cropped.shape[:2]
    if crop_h < 20 or crop_w < 40:
        return cropped  # Too small
    
    # Upscale if plate is too small (helps OCR)
    if crop_h < 64 or crop_w < 128:
        scale = max(64 / crop_h, 128 / crop_w)
        new_w = int(crop_w * scale)
        new_h = int(crop_h * scale)
        cropped = cv2.resize(cropped, (new_w, new_h), interpolation=cv2.INTER_CUBIC)
    
    return cropped


def normalize_russian_plate(text: str) -> str:
    """
    Normalize OCR output to match Russian plate format: A###AA## or A###AA###
    
    Handles both ONNX CRNN (Latin output) and EasyOCR (mixed Cyrillic/Latin).
    Converts Latin -> Cyrillic and validates format.
    
    Valid formats:
    - A123BC45 (8 chars): 1 letter + 3 digits + 2 letters + 2 digits
    - A123BC456 (9 chars): 1 letter + 3 digits + 2 letters + 3 digits
    """
    if not text or len(text) < 6:
        return text
    
    # Mapping for Russian plates: Latin to Cyrillic
    lat_to_cyr = {
        'A': 'А', 'B': 'В', 'E': 'Е', 'C': 'С',
        'H': 'Н', 'K': 'К', 'M': 'М', 'O': 'О',
        'P': 'Р', 'T': 'Т', 'X': 'Х', 'Y': 'У',
    }
    
    # Valid Cyrillic letters for Russian plates
    valid_cyr = set('АВЕКМНОРСТУХ')
    
    # Remove any non-alphanumeric characters
    text = ''.join(c for c in text.upper() if c.isalnum())
    
    chars = list(text)
    normalized = []
    
    for i, ch in enumerate(chars):
        # Convert Latin to Cyrillic if possible
        ch_cyr = lat_to_cyr.get(ch, ch)
        
        # Position 0: MUST be letter
        if i == 0:
            if ch_cyr in valid_cyr:
                normalized.append(ch_cyr)
            elif ch_cyr.isalpha():
                # Try to find closest Cyrillic
                normalized.append(lat_to_cyr.get(ch_cyr, ch_cyr))
            elif ch.isdigit():
                # Number mistaken for letter: 0->О, 8->В
                if ch == '0': normalized.append('О')
                elif ch == '8': normalized.append('В')
                else: normalized.append(ch_cyr)  # Keep as is, will fail validation
            else:
                normalized.append(ch_cyr)
        
        # Positions 1-3: MUST be digits
        elif 1 <= i <= 3:
            if ch.isdigit():
                normalized.append(ch)
            elif ch_cyr in valid_cyr or ch_cyr.isalpha():
                # Letter mistaken for digit
                if ch_cyr in ['О', 'O']: normalized.append('0')
                elif ch_cyr in ['З', 'Z']: normalized.append('3')
                elif ch_cyr in ['Б']: normalized.append('6')
                elif ch_cyr in ['І', 'I']: normalized.append('1')
                elif ch_cyr in ['Т', 'T']: normalized.append('7')
                elif ch_cyr in ['С', 'C']: normalized.append('0')
                else: normalized.append(ch)  # Keep original
            else:
                normalized.append(ch)
        
        # Positions 4-5: MUST be letters
        elif 4 <= i <= 5:
            if ch_cyr in valid_cyr:
                normalized.append(ch_cyr)
            elif ch_cyr.isalpha():
                normalized.append(lat_to_cyr.get(ch_cyr, ch_cyr))
            elif ch.isdigit():
                # Digit mistaken for letter
                if ch == '0': normalized.append('О')
                elif ch == '3': normalized.append('Е')
                elif ch == '8': normalized.append('В')
                elif ch == '6': normalized.append('Б')
                elif ch == '1': normalized.append('І')
                else: normalized.append(ch_cyr)
            else:
                normalized.append(ch_cyr)
        
        # Positions 6+: MUST be digits (region code)
        elif i >= 6:
            if ch.isdigit():
                normalized.append(ch)
            elif ch_cyr in valid_cyr or ch_cyr.isalpha():
                # Letter mistaken for digit
                if ch_cyr in ['О', 'O']: normalized.append('0')
                elif ch_cyr in ['З', 'Z']: normalized.append('3')
                elif ch_cyr in ['Б']: normalized.append('6')
                elif ch_cyr in ['І', 'I']: normalized.append('1')
                elif ch_cyr in ['Т', 'T']: normalized.append('7')
                elif ch_cyr in ['С', 'C']: normalized.append('0')
                else: normalized.append(ch)
            else:
                normalized.append(ch)
        else:
            normalized.append(ch_cyr)
    
    result = ''.join(normalized)
    
    # Validate: must be 8 or 9 characters
    if len(result) < 8:
        return text  # Return original if too short
    
    return result[:9]  # Truncate if too long


def main():
    if len(sys.argv) < 2:
        print(json.dumps({"status": "error", "message": "RTSP URL not provided"}))
        return

    rtsp_url = sys.argv[1]
    encoded_config = sys.argv[2] if len(sys.argv) > 2 else None
    provider_choice = sys.argv[3] if len(sys.argv) > 3 else "cpu"

    config = RuntimeConfig.from_base64(encoded_config)
    providers = select_providers(provider_choice)

    yolo_path = APPLICATION_ROOT / "anpr_yolov8.onnx"
    ocr_path = APPLICATION_ROOT / "anpr_crnn.onnx"

    detector = ONNXDetector(yolo_path, providers)
    
    # ALWAYS prefer EasyOCR for better Russian recognition
    recognizer = None
    
    if EASYOCR_AVAILABLE:
        try:
            recognizer = EasyOCRRecognizer()
            sys.stderr.write("[ANPR] ✅ Using EasyOCR for Russian plate recognition\n")
            sys.stderr.flush()
        except Exception as e:
            sys.stderr.write(f"[ANPR] ⚠️ Failed to initialize EasyOCR: {e}\n")
            sys.stderr.flush()
    
    # Fallback to ONNX CRNN only if EasyOCR failed
    if recognizer is None:
        try:
            recognizer = ONNXRnnRecognizer(ocr_path, providers)
            sys.stderr.write("[ANPR] ⚠️ Using ONNX CRNN (less accurate for Russian plates)\n")
            sys.stderr.flush()
        except Exception as e:
            print(json.dumps({"status": "error", "message": f"Failed to initialize OCR: {e}"}))
            return

    cap = cv2.VideoCapture(rtsp_url)
    if not cap.isOpened():
        print(json.dumps({"status": "error", "message": "Cannot open stream"}))
        return

    while True:
        ok, frame = cap.read()
        if not ok:
            time.sleep(0.5)
            continue

        preds, scale, pad_x, pad_y = detector.infer(frame)
        detections = detector.postprocess(
            preds,
            scale,
            pad_x,
            pad_y,
            frame.shape[:2],
            config.confidence_threshold,
            config.iou_threshold,
            config.max_results,
            config.min_plate_area,
            config.min_plate_height,
            config.min_aspect_ratio,
            config.max_aspect_ratio,
        )

        plates: List[Dict[str, Any]] = []
        for det in detections:
            # Увеличен margin до 0.2 для лучшего захвата контекста номера
            roi = enlarge_and_crop(frame, det["bbox"], margin=0.2)
            if roi.size == 0:
                continue
            
            # Save cropped plate for debugging (optional)
            # cv2.imwrite(f"plate_{int(time.time()*1000)}.jpg", roi)
            
            try:
                text = recognizer.recognize(roi)
                
                # Log raw OCR output for debugging
                if text:
                    sys.stderr.write(f"[OCR] Raw: '{text}'\n")
                    sys.stderr.flush()
                
                # Normalize and validate Russian plate format
                text = normalize_russian_plate(text)
                
                # Additional validation: must be 8-9 chars
                if text and (len(text) < 7 or len(text) > 10):
                    sys.stderr.write(f"[OCR] Rejected (invalid length {len(text)}): '{text}'\n")
                    sys.stderr.flush()
                    text = ""
                
                if text:
                    sys.stderr.write(f"[OCR] Final: '{text}'\n")
                    sys.stderr.flush()
                    
            except Exception as exc:
                text = ""
                sys.stderr.write(f"[OCR] Error: {exc}\n")
                sys.stderr.flush()

            plates.append(
                {
                    "bbox": {
                        "x1": det["bbox"][0],
                        "y1": det["bbox"][1],
                        "x2": det["bbox"][2],
                        "y2": det["bbox"][3],
                    },
                    "confidence": det["confidence"],
                    "text": text,
                }
            )

        result = {
            "status": "license_plate",
            "timestamp": time.time(),
            "plates": plates,
        }
        print(json.dumps(result), flush=True)


if __name__ == "__main__":
    main()
