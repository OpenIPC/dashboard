# ANPR Integration Guide

## Overview

The ANPR (Automatic Number Plate Recognition) module now uses a **hybrid approach**:
- **YOLO v8** for license plate detection (Rust + ONNX)
- **Python subprocess** for OCR with perspective correction (PyTorch CRNN)
- **Rust ONNX fallback** if Python is unavailable

This design provides the best accuracy while maintaining production readiness.

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Rust (Tauri Backend)                 │
│                                                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │  LicensePlateEngine                              │  │
│  │                                                  │  │
│  │  1. YoloDetector (ONNX)                         │  │
│  │     └─> Detects plate bounding boxes            │  │
│  │                                                  │  │
│  │  2. CrnnRecognizer                              │  │
│  │     ├─> recognize_with_python() [PRIMARY]      │  │
│  │     │   └─> Calls Python subprocess             │  │
│  │     │       └─> Perspective correction          │  │
│  │     │       └─> CRNN PyTorch model              │  │
│  │     │                                           │  │
│  │     └─> recognize() [FALLBACK]                 │  │
│  │         └─> Rust ONNX Runtime                  │  │
│  │         └─> Simple preprocessing               │  │
│  └──────────────────────────────────────────────────┘  │
│                                                         │
│  Cache: 5sec TTL, IOU + center distance matching       │
└─────────────────────────────────────────────────────────┘
                            │
                            ▼
              ┌─────────────────────────┐
              │  Python OCR Subprocess   │
              │  (anpr_ocr.py)           │
              │                          │
              │  1. Load CRNN model      │
              │  2. Perspective correct  │
              │  3. Binarization         │
              │  4. CTC decode           │
              │  5. Transliteration      │
              └─────────────────────────┘
```

---

## Setup

### 1. Install Python Dependencies

```bash
cd dashboard
pip install -r requirements.txt
```

**Required packages:**
- `torch>=2.0.0` - PyTorch for CRNN model
- `torchvision>=0.15.0` - Image transformations
- `opencv-python>=4.8.0` - Perspective correction
- `numpy>=1.24.0` - Array operations

### 2. Prepare Models

Place the following models in `artifacts/anpr/`:

```
artifacts/anpr/
├── anpr_yolov8.onnx          # YOLO detection (already present)
├── anpr_crnn.onnx            # Rust fallback OCR (already present)
└── crnn_ocr_model_best.pth   # Python PyTorch OCR ⚠️ NEW
```

**Download PyTorch CRNN model:**
```bash
# Copy from external/anpr-system or download from training repo
cp external/anpr-system/models/ocr_crnn/crnn_ocr_model_best.pth artifacts/anpr/
```

### 3. Verify Installation

Test Python OCR script manually:

```bash
# Save a test plate crop image as test_plate.jpg
python src-tauri/python_src/anpr_ocr.py \
    --model artifacts/anpr/crnn_ocr_model_best.pth \
    --image test_plate.jpg \
    --json
```

Expected output:
```json
{
  "success": true,
  "latin": "T1466",
  "cyrillic": "Т1466",
  "text": "Т1466"
}
```

---

## Configuration

### Toggle Python vs Rust OCR

Edit `src-tauri/src/analytics/license_plate.rs`:

```rust
const USE_PYTHON_OCR: bool = true;  // true = Python, false = Rust ONNX
```

### Performance Tuning

- **Cache TTL**: Adjust `RECOGNITION_CACHE_TTL` (default: 5 seconds)
- **Slow frame threshold**: `SLOW_FRAME_LOG_THRESHOLD` (default: 80ms)
- **Python timeout**: Currently no timeout (TODO: add timeout to subprocess)

---

## How It Works

### Detection Flow

1. **Frame arrives** → YOLO detector finds plate bounding boxes
2. **Check cache** → If plate seen recently (5s), reuse label
3. **Crop ROI** → Extract plate region from frame
4. **OCR** → Choose Python or Rust:
   - **Python** (default):
     - Save crop to temp file
     - Call `python anpr_ocr.py --model ... --image ...`
     - Parse JSON response
     - Fallback to Rust if Python fails
   - **Rust** (fallback):
     - Try 3 preprocessing strategies (minimal, video, print)
     - Run ONNX CRNN inference
     - Pick longest recognized text
5. **Transliteration** → Convert Latin → Cyrillic
6. **Update cache** → Store result for 5 seconds

### Python OCR Pipeline (`anpr_ocr.py`)

```python
Input: Plate crop image
  ↓
Preprocessing:
  ├─> Grayscale conversion
  ├─> Gaussian blur (5x5)
  ├─> Otsu binarization
  ├─> Find contours
  └─> 4-point perspective transform  # ⭐ KEY FEATURE
      └─> Corrects tilted/skewed plates
  ↓
CRNN Model:
  ├─> Resize to 128×32
  ├─> Normalize [-1, 1]: (x/255 - 0.5) / 0.5
  ├─> PyTorch CRNN forward pass
  └─> CTC decode (blank removal + deduplication)
  ↓
Output: "T1466" (Latin)
  ↓
Transliteration:
  A→А, B→В, E→Е, C→С, H→Н, K→К, M→М, O→О, P→Р, T→Т, X→Х, Y→У
  ↓
Output: "Т1466" (Cyrillic)
```

---

## Known Issues & Limitations

### Current Status

✅ **Working:**
- YOLO detection (88%+ confidence)
- Python subprocess integration
- Perspective correction in Python
- CTC decode
- Transliteration
- Caching (5sec TTL)

❌ **Known Problems:**
- Model accuracy: "T1466" instead of "Т207ОВ125" (training data issue)
- No timeout on Python subprocess (can hang indefinitely)
- Temp file cleanup on crash (cleanup is best-effort)

### Performance

- **Python OCR**: ~50-150ms per plate (includes subprocess overhead)
- **Rust OCR**: ~5-10ms per plate (ONNX Runtime)
- **YOLO detection**: ~20-30ms per frame

**Typical frame processing**: 80-200ms (1-3 detections per frame)

---

## Troubleshooting

### Python OCR not working

1. **Check Python is in PATH:**
   ```bash
   python --version
   # Should show Python 3.8+
   ```

2. **Verify dependencies:**
   ```bash
   python -c "import torch, cv2, numpy; print('OK')"
   ```

3. **Test script manually:**
   ```bash
   python src-tauri/python_src/anpr_ocr.py --help
   ```

4. **Check logs:**
   - Rust will print: `"Python OCR failed: <error>"`
   - Python prints to stderr: `⚠️ Preprocessing failed: ...`

### Fallback to Rust OCR

If Python fails, Rust automatically uses ONNX:
```
[INFO] Python OCR failed, using Rust fallback
[INFO] OCR strategy 'minimal': 'T146' (score=4)
[INFO] OCR strategy 'video': 'T1466' (score=5)  ← Best
[INFO] OCR strategy 'print': 'T14' (score=3)
```

### Model not found errors

```
Error: Python OCR model not found: artifacts/anpr/crnn_ocr_model_best.pth
```

**Solution:** Copy model from `external/anpr-system/`:
```bash
cp external/anpr-system/models/ocr_crnn/crnn_ocr_model_best.pth artifacts/anpr/
```

---

## Future Improvements

### High Priority
- [ ] Add timeout to Python subprocess (avoid hangs)
- [ ] Implement voting/stabilization (track plates across frames)
- [ ] Better model training data (fix "T1466" vs "Т207ОВ125")

### Medium Priority
- [ ] Use Python sidecar instead of subprocess (persistent process)
- [ ] Add confidence scores to OCR results
- [ ] Support multiple OCR models (e.g., EasyOCR, PaddleOCR)

### Low Priority
- [ ] Port perspective correction to Rust (imageproc + opencv-rust)
- [ ] ONNX quantization for faster inference
- [ ] GPU acceleration (CUDA/DirectML)

---

## Development

### Testing Python OCR

```python
# Test with a sample image
python src-tauri/python_src/anpr_ocr.py \
    --model artifacts/anpr/crnn_ocr_model_best.pth \
    --image path/to/plate.jpg \
    --json | jq .
```

### Disabling Python OCR

Set `USE_PYTHON_OCR = false` in `license_plate.rs` and recompile:

```bash
cd src-tauri
cargo build --release
```

### Benchmarking

```bash
# Time Python OCR
time python src-tauri/python_src/anpr_ocr.py --model ... --image ...

# Compare with Rust ONNX (check logs for timing)
```

---

## Credits

- **YOLO detection**: Ultralytics YOLOv8
- **CRNN model**: Based on Runoi/ANPR-System
- **Perspective correction**: OpenCV `getPerspectiveTransform`
- **CTC decode**: Custom implementation in Python + Rust

---

**Last updated**: 2025-11-11
**Status**: ✅ Ready for testing
