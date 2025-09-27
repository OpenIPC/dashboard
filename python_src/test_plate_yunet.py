import sys
try:
    with open("log.txt", "a", encoding="utf-8") as f:
        f.write("runner started\n")
except Exception as e:
    print(f"[runner] Failed to write log.txt: {e}")
    import sys; sys.stdout.flush()
print('=== test_plate_yunet.py: script entry ===', file=sys.stderr)
sys.stderr.flush()


import cv2 as cv
import numpy as np
import os
import glob
import argparse
import json
import signal
import time
# Ensure this script's directory is on sys.path so imports work when launched
# from project root (prevents ModuleNotFoundError for sibling modules).
script_dir = os.path.dirname(__file__)
if script_dir and script_dir not in sys.path:
    sys.path.insert(0, script_dir)
from lpd_yunet import LPD_YuNet
import easyocr

# Global placeholders for lazy initialization to avoid heavy startup memory use
READER = None
OCR_ENGINE = 'easyocr'  # Only EasyOCR with GPU
DEBUG_MODE = False

def detect_gpu_availability():
    """Detect GPU availability for EasyOCR"""
    gpu_available = False
    
    # Check EasyOCR GPU - try to initialize with GPU and see if it works
    try:
        # Try to create a small EasyOCR instance to test GPU
        import easyocr
        test_reader = easyocr.Reader(['en'], gpu=True)
        gpu_available = True
        print('[runner] EasyOCR GPU available', file=sys.stderr)
        # Clean up
        del test_reader
    except Exception as e:
        print(f'[runner] EasyOCR GPU not available ({e}), using CPU', file=sys.stderr)
    
    return gpu_available


# Значение по умолчанию, может быть переопределено через аргумент
ALLOWLIST = 'АБВЕКМНОРСТУХ0123456789'
# Patch model constants and OUT_DIR/IMG_DIR defaults (moved here so they are
# available during initialization inside main()). This prevents NameError and
# premature exit when model is created before these constants were defined.
try:
    from plate_recognizer import CONF_THRESHOLD, NMS_THRESHOLD, TOP_K, KEEP_TOP_K
except ImportError:
    CONF_THRESHOLD = 0.3
    NMS_THRESHOLD = 0.3
    TOP_K = 5000
    KEEP_TOP_K = 750
import os
OUT_DIR = os.path.join(os.path.dirname(__file__), 'test_images')
IMG_DIR = OUT_DIR
def main():
    import os
    print('Script started, args:', sys.argv, file=sys.stderr)
    sys.stderr.flush()
    print('[runner] Parsing arguments...', file=sys.stderr)
    sys.stderr.flush()
    parser = argparse.ArgumentParser(description='LPD-YuNet test: video/camera by default, images only with --images')
    parser.add_argument('--video', type=str, default=None, help='Путь к видеофайлу или rtsp-потоку. Если не указано — используется webcam.')
    parser.add_argument('--camera', type=int, default=0, help='Номер камеры для захвата (по умолчанию 0)')
    parser.add_argument('--images', action='store_true', help='Обрабатывать только изображения из test_images (только с этим флагом)')
    parser.add_argument('--frame-skip', type=int, default=2, help='Обрабатывать каждый N-й кадр (default: 2)')
    parser.add_argument('--resize-width', type=int, default=0, help='Изменить ширину кадра перед обработкой (0 = не менять, рекомендуется 0 для HD)')
    parser.add_argument('--save-dir', type=str, default=None, help='Папка для сохранения найденных номеров (по умолчанию test_images)')
    parser.add_argument('--min-score', type=float, default=0.65, help='Minimum confidence score for detections (default: 0.65, relaxed)')
    parser.add_argument('--min-area', type=int, default=1000, help='Minimum area for license plate (default: 4000, working)')
    parser.add_argument('--min-height', type=int, default=40, help='Minimum height for license plate crop (default: 40, working)')
    parser.add_argument('--min-aspect', type=float, default=1.2, help='Minimum aspect ratio (default: 1.2, relaxed)')
    parser.add_argument('--max-aspect', type=float, default=7.5, help='Maximum aspect ratio (default: 7.5, relaxed)')
    parser.add_argument('--allowlist', type=str, default='АБВЕКМНОРСТУХ0123456789', help='Разрешённые символы для распознавания номеров')
    parser.add_argument('--enable-position-filter', action='store_true', help='Enable y_min position filter (default: off)')
    parser.add_argument('--backend', type=str, default=None, help='Video backend to use (e.g., FFMPEG, DSHOW, MSMF, ANY). Default: FFMPEG for video, ANY for camera')
    parser.add_argument('--max-frames', type=int, default=0, help='Maximum number of frames to process (0 = unlimited)')
    parser.add_argument('--use-ort', action='store_true', help='Use ONNX Runtime (DirectML if available) for inference')
    parser.add_argument('--ocr-engine', type=str, default='auto', help="OCR engine to use: 'auto', 'easyocr', 'tesseract', 'paddle'")
    parser.add_argument('--debug', action='store_true', help='Enable debug mode (write debug files, verbose)')
    args = parser.parse_args()
    print('[runner] Args:', args, file=sys.stderr)
    sys.stderr.flush()
    # Set filter parameters
    min_score = args.min_score
    min_area = args.min_area
    min_height = args.min_height
    min_aspect = args.min_aspect
    max_aspect = args.max_aspect
    disable_position_filter = not args.enable_position_filter
    global ALLOWLIST
    # Fix allowlist encoding - use hardcoded Cyrillic allowlist
    ALLOWLIST = 'АБВЕКМНОРСТУХ0123456789'
    print(f'[runner] Filters: score>={min_score}, area>={min_area}, height>={min_height}, aspect in [{min_aspect}, {max_aspect}], allowlist={ALLOWLIST}, position_filter={not disable_position_filter}', file=sys.stderr)
    sys.stderr.flush()
    sys.stdout.flush()
    # Set backend
    backends = {'FFMPEG': cv.CAP_FFMPEG, 'DSHOW': cv.CAP_DSHOW, 'MSMF': cv.CAP_MSMF, 'ANY': cv.CAP_ANY}
    if args.backend:
        backend_id = backends.get(args.backend.upper(), cv.CAP_ANY)
    else:
        backend_id = cv.CAP_FFMPEG if args.video else cv.CAP_ANY
    # For RTSP, prefer GSTREAMER if available
    if args.video and 'rtsp' in args.video.lower():
        backend_id = cv.CAP_GSTREAMER
    print(f'[runner] Using backend: {args.backend or ("GSTREAMER" if args.video and "rtsp" in args.video.lower() else ("FFMPEG" if args.video else "ANY"))} ({backend_id})', file=sys.stderr)
    sys.stderr.flush()
    print('[runner] Checking model and image dir...')
    sys.stdout.flush()
    MODEL_PATH_LOCAL = os.path.join(os.path.dirname(__file__), 'license_plate_detection.onnx')
    if not os.path.exists(MODEL_PATH_LOCAL):
        print(f"Model not found: {MODEL_PATH_LOCAL}", file=sys.stderr)
        sys.stderr.flush()
        # Don't call sys.exit here — return from main so caller can handle process lifecycle
        return
    # Для видео-режима image dir не обязателен
    print('[runner] Model OK')
    sys.stdout.flush()
    print('[runner] Loading LPD_YuNet model...')
    sys.stdout.flush()
    try:
        # If requested, try to use ONNX Runtime wrapper (prefer DirectML provider)
        if args.use_ort:
            try:
                import onnxruntime as _ort
                try:
                    print('[runner] onnxruntime available providers:', _ort.get_available_providers(), file=sys.stderr)
                except Exception:
                    pass
                from lpd_yunet_ort import LPD_YuNetORT
                model = LPD_YuNetORT(
                    MODEL_PATH_LOCAL,
                    inputSize=[320, 240],
                    confThreshold=CONF_THRESHOLD,
                    nmsThreshold=NMS_THRESHOLD,
                    topK=TOP_K,
                    keepTopK=KEEP_TOP_K,
                    prefer_dml=True
                )
                # If the ORT wrapper exposes the session, print the providers actually used by the session
                try:
                    sess_providers = None
                    if hasattr(model, 'session'):
                        sess_providers = model.session.get_providers()
                    elif hasattr(model, '_session'):
                        sess_providers = model._session.get_providers()
                    if sess_providers is not None:
                        print('[runner] ORT session providers in use:', sess_providers, file=sys.stderr)
                except Exception:
                    pass
                print('[runner] Loaded ONNX Runtime (ORT) YuNet model')
            except Exception as e:
                print('[runner] Failed to load ORT model, falling back to OpenCV DNN:', e, file=sys.stderr)
                # Fallthrough to OpenCV DNN below
                args.use_ort = False

        if not args.use_ort:
            # Try to use DirectML (GPU) if available in OpenCV DNN, fallback to CPU
            try:
                dml_target = cv.dnn.DNN_TARGET_DML
                print('[runner] Trying DNN_TARGET_DML (DirectML/GPU)', file=sys.stderr)
            except AttributeError:
                dml_target = cv.dnn.DNN_TARGET_CPU
                print('[runner] DNN_TARGET_DML not available, using CPU', file=sys.stderr)
            model = LPD_YuNet(
                modelPath=MODEL_PATH_LOCAL,
                confThreshold=CONF_THRESHOLD,
                nmsThreshold=NMS_THRESHOLD,
                topK=TOP_K,
                keepTopK=KEEP_TOP_K,
                backendId=cv.dnn.DNN_BACKEND_OPENCV,
                targetId=dml_target
            )
            print('[runner] OpenCV DNN model loaded successfully')
    except Exception as e:
        print(f"[FATAL] Failed to load model: {e}")
        import traceback
        traceback.print_exc(file=sys.stdout)
        # Return instead of exiting to allow caller to observe logs
        return
    sys.stdout.flush()

    # Set OCR engine and debug mode (affects lazy init and behavior)
    global OCR_ENGINE, DEBUG_MODE
    OCR_ENGINE = 'easyocr'  # Only EasyOCR
    DEBUG_MODE = bool(args.debug)
    print(f'[runner] OCR engine: {OCR_ENGINE}, debug_mode={DEBUG_MODE}', file=sys.stderr)
    sys.stderr.flush()

    # Write an initialization heartbeat so parent can observe that init completed
    try:
        import tempfile as _temp
        hb_init = os.path.join(_temp.gettempdir(), f'dashboard_testplate_init_{os.getpid()}.txt')
        with open(hb_init, 'a', encoding='utf-8') as _f:
            _f.write(f"inited:{int(time.time())}\n")
        print(f"[DEBUG] Wrote init heartbeat to {hb_init}", file=sys.stderr)
        sys.stderr.flush()
    except Exception as _e:
        print(f"[DEBUG] Failed to write init heartbeat: {_e}", file=sys.stderr)
        sys.stderr.flush()
    frame_skip = max(1, args.frame_skip)
    resize_width = args.resize_width
    # Simple diagnostics: allow external observation of signals
    def _handle_signal(sig, frame):
        try:
            print(f"[DEBUG] Received signal: {sig}")
            sys.stdout.flush()
        except Exception:
            pass

    try:
        signal.signal(signal.SIGINT, _handle_signal)
        signal.signal(signal.SIGTERM, _handle_signal)
    except Exception:
        # signal handling may be limited on some Windows/Python builds
        pass
    # Определяем папку для сохранения
    # If no save-dir provided, create a per-process temp subdirectory to avoid collisions
    if args.save_dir:
        save_dir = args.save_dir
    else:
        import tempfile
        pid = os.getpid()
        system_temp = tempfile.gettempdir()
        save_dir = os.path.join(system_temp, f"dashboard_testplate_{pid}")
    if not os.path.exists(save_dir):
        try:
            os.makedirs(save_dir, exist_ok=True)
        except Exception:
            # fallback to OUT_DIR if temp dir creation fails
            save_dir = OUT_DIR
            os.makedirs(save_dir, exist_ok=True)
    if args.images:
        print('[runner] Images mode')
        sys.stdout.flush()
        img_files = []
        for ext in ('*.jpg', '*.jpeg', '*.png'):
            img_files.extend(glob.glob(os.path.join(IMG_DIR, ext)))
        img_files = [f for f in img_files if not os.path.basename(f).startswith('result_yunet')]
        if not img_files:
            print(f"No images found in {IMG_DIR}")
            return
        recognized = []
        for img_path in img_files:
            img = cv.imread(img_path)
            if img is None:
                print(f"Failed to read {img_path}")
                continue
            h, w, _ = img.shape
            # For license plate images, skip detection and directly apply OCR
            if os.path.basename(img_path).startswith('plate_yunet_'):
                # Enhance image for better OCR quality (use lighter enhancement)
                enhanced_img = enhance_plate_image(img, scale_factor=1.5)
                # Run OCR on in-memory image to avoid disk IO
                text = recognize_text(enhanced_img)
                print(f"{os.path.basename(img_path)}: OCR text: '{text}'")
                # Save only when recognized or when debugging
                saved_path = None
                if text and text.strip():
                    temp_path = img_path.replace('.jpg', '_enhanced.jpg')
                    cv.imwrite(temp_path, enhanced_img)
                    saved_path = temp_path
                elif DEBUG_MODE:
                    temp_path = img_path.replace('.jpg', '_enhanced.jpg')
                    cv.imwrite(temp_path, enhanced_img)
                    saved_path = temp_path
                recognized.append({"path": saved_path or img_path, "score": 1.0, "text": text})  # score 1.0 for direct OCR
                continue
            # Set minimum input size for the model
            min_w, min_h = 320, 240
            input_w = max(w, min_w)
            input_h = max(h, min_h)
            model.setInputSize([input_w, input_h])
            # Resize image to match input size
            img_resized = cv.resize(img, (input_w, input_h))
            results = model.infer(img_resized)
            print(f"{os.path.basename(img_path)}: {results.shape[0]} license plates detected.")
            img_vis = visualize(img, results)
            out_path = os.path.join(save_dir, f'result_yunet_{os.path.splitext(os.path.basename(img_path))[0]}.jpg')
            cv.imwrite(out_path, img_vis)
            print(f"Result image saved to: {out_path}")
            filtered_count = 0
            for i, det in enumerate(results):
                bbox = det[:-1].astype(np.int32)
                score = det[-1]
                pts = np.array([[bbox[0], bbox[1]], [bbox[2], bbox[3]], [bbox[4], bbox[5]], [bbox[6], bbox[7]]], dtype=np.int32)
                x_min, y_min = np.min(pts, axis=0)
                x_max, y_max = np.max(pts, axis=0)
                x_min, y_min = max(0, x_min), max(0, y_min)
                x_max, y_max = min(w-1, x_max), min(h-1, y_max)
                width = x_max - x_min
                height = y_max - y_min
                area = width * height
                aspect = width / height if height > 0 else 0
                if score < min_score:
                    print(f"Skip {i}: score {score:.2f} < {min_score}")
                    continue
                if area < min_area:
                    print(f"Skip {i}: area {area} < {min_area}")
                    continue
                if not (min_aspect < aspect < max_aspect):
                    print(f"Skip {i}: aspect {aspect:.2f} not in [{min_aspect}, {max_aspect}]")
                    continue
                if not disable_position_filter and y_min < h // 4:
                    print(f"Skip {i}: y_min {y_min} < {h // 4} (верх кадра)")
                    continue
                plate_img = img[y_min:y_max, x_min:x_max]
                # Enhance plate image for better quality (adaptive scale)
                # Use smaller scale to reduce memory/CPU; increase only for very small crops
                try:
                    h_plate = plate_img.shape[0] if plate_img is not None else 0
                except Exception:
                    h_plate = 0
                if h_plate and h_plate < 50:
                    sf = 2.0
                else:
                    sf = 1.5
                plate_img = enhance_plate_image(plate_img, scale_factor=sf)
                plate_path = os.path.join(save_dir, f'plate_yunet_{os.path.splitext(os.path.basename(img_path))[0]}_{filtered_count+1}.jpg')
                try:
                    # Run OCR in-memory first to avoid unnecessary IO
                    text = recognize_text(plate_img)
                    # Save plate image only if recognized or debugging is enabled
                    saved = False
                    if text and text.strip():
                        cv.imwrite(plate_path, plate_img)
                        saved = True
                    elif DEBUG_MODE:
                        cv.imwrite(plate_path, plate_img)
                        saved = True
                    if saved:
                        print(f"Plate saved to: {plate_path} (score={score:.2f}, area={area}, aspect={aspect:.2f}, text='{text}')")
                    else:
                        print(f"Plate (not saved) (score={score:.2f}, area={area}, aspect={aspect:.2f}, text='{text}')")
                except Exception as e:
                    print(f"Failed to save plate image: {e}")
                    text = ""
                filtered_count += 1
        if recognized:
            # Выводим JSON-строку для интеграции с Node.js
            print(json.dumps({"recognized": recognized}))
            sys.stdout.flush()
        return

    # --- VIDEO MODE: always run if not images mode ---
    print('[runner] Entering video mode (video/camera/RTSP)', file=sys.stderr)
    sys.stderr.flush()
    # Determine video source
    video_source = args.video if args.video else args.camera
    print(f'[runner] Video source: {video_source}', file=sys.stderr)
    sys.stderr.flush()
    # Try to open video capture
    cap, used_backend = try_open_capture(video_source, backend_id)
    if cap is None or not cap.isOpened():
        print(f"[FATAL] Failed to open video source: {video_source}", file=sys.stderr)
        sys.stderr.flush()
        # Do not exit the whole process here; return to allow caller to handle restarts
        return
    print(f"[runner] VideoCapture opened successfully with backend {used_backend}", file=sys.stderr)
    sys.stderr.flush()
    frame_count = 0
    processed_count = 0
    # Проверяем разрешение потока (только HD)
    hd_checked = False
    while True:
        ret, frame = cap.read()
        if not ret:
            print(f"[runner] End of stream or failed to read frame at count {frame_count}", file=sys.stderr)
            break
        frame_count += 1
        if not hd_checked:
            h0, w0 = frame.shape[:2]
            print(f"[runner] Stream resolution: width={w0}, height={h0}", file=sys.stderr)
            hd_checked = True
        if frame_count % frame_skip != 0:
            continue
        # Не уменьшаем кадр по ширине по умолчанию
        if resize_width > 0:
            h, w = frame.shape[:2]
            scale = resize_width / w
            frame = cv.resize(frame, (resize_width, int(h * scale)))
        # Set model input size
        h, w = frame.shape[:2]
        min_w, min_h = 320, 240
        input_w = max(w, min_w)
        input_h = max(h, min_h)
        model.setInputSize([input_w, input_h])
        frame_resized = cv.resize(frame, (input_w, input_h))
        results = model.infer(frame_resized)
        print(f"[runner] Frame {frame_count}: {results.shape[0]} license plates detected.", file=sys.stderr)
        filtered_count = 0
        for i, det in enumerate(results):
            bbox = det[:-1].astype(np.int32)
            score = det[-1]
            # Model returns coordinates in resized input space (input_w x input_h).
            # Scale them back to original frame coordinates before cropping.
            pts = np.array([[bbox[0], bbox[1]], [bbox[2], bbox[3]], [bbox[4], bbox[5]], [bbox[6], bbox[7]]], dtype=np.float32)
            # original frame size (w,h) and model input size
            orig_h, orig_w = frame.shape[:2]
            # Determine the actual model input size used for inference.
            # The model object may have a fixed input size (e.g., ORT wrapper)
            # stored in model.input_size; fall back to the local input_w/input_h.
            try:
                m_input = getattr(model, 'input_size', None)
                if m_input is None:
                    model_w, model_h = float(input_w), float(input_h)
                else:
                    # handle numpy arrays
                    try:
                        model_w = float(m_input[0])
                        model_h = float(m_input[1])
                    except Exception:
                        model_w, model_h = float(input_w), float(input_h)
            except Exception:
                model_w, model_h = float(input_w), float(input_h)

            scale_x = float(orig_w) / model_w
            scale_y = float(orig_h) / model_h
            pts[:, 0] = np.clip(np.round(pts[:, 0] * scale_x), 0, orig_w - 1)
            pts[:, 1] = np.clip(np.round(pts[:, 1] * scale_y), 0, orig_h - 1)
            pts = pts.astype(np.int32)
            x_min, y_min = np.min(pts, axis=0)
            x_max, y_max = np.max(pts, axis=0)
            x_min, y_min = max(0, x_min), max(0, y_min)
            x_max, y_max = min(w-1, x_max), min(h-1, y_max)
            width = x_max - x_min
            height = y_max - y_min
            area = width * height
            aspect = width / height if height > 0 else 0
            print(f"[runner] BBOX {i}: x_min={x_min}, y_min={y_min}, x_max={x_max}, y_max={y_max}, width={width}, height={height}, area={area}, aspect={aspect:.2f}, score={score:.2f}", file=sys.stderr)
            if score < min_score:
                print(f"[runner] Skip {i}: score {score:.2f} < {min_score}", file=sys.stderr)
                continue
            if area < min_area:
                print(f"[runner] Skip {i}: area {area} < {min_area}", file=sys.stderr)
                continue
            if height < min_height:
                print(f"[runner] Skip {i}: height {height} < {min_height}", file=sys.stderr)
                continue
            if not (min_aspect < aspect < max_aspect):
                print(f"[runner] Skip {i}: aspect {aspect:.2f} not in [{min_aspect}, {max_aspect}]", file=sys.stderr)
                continue
            if not disable_position_filter and y_min < h // 4:
                print(f"[runner] Skip {i}: y_min {y_min} < {h // 4} (верх кадра)", file=sys.stderr)
                continue
            # Use perspective transform to extract an upright plate crop from the detected quad.
            # This yields a more consistent crop for OCR than the axis-aligned bbox.
            pad_x_ratio = 0.20  # 20% width padding
            pad_y_ratio = 0.35  # 35% height padding
            try:
                # Ensure the quad points are ordered consistently: top-left, top-right, bottom-right, bottom-left
                def order_quad(quad):
                    # quad: numpy array shape (4,2)
                    q = quad.reshape((4,2)).astype(np.float32)
                    s = q.sum(axis=1)
                    diff = np.diff(q, axis=1).reshape(4)
                    rect = np.zeros((4,2), dtype=np.float32)
                    rect[0] = q[np.argmin(s)]      # top-left has smallest sum
                    rect[2] = q[np.argmax(s)]      # bottom-right has largest sum
                    rect[1] = q[np.argmin(diff)]   # top-right has smallest difference (x - y)
                    rect[3] = q[np.argmax(diff)]   # bottom-left has largest difference
                    return rect

                src_pts = order_quad(pts.astype(np.float32))
                # Estimate quad size using ordered points
                # width: distance between top-left and top-right, and between bottom-left and bottom-right
                widthA = np.linalg.norm(src_pts[0] - src_pts[1])
                widthB = np.linalg.norm(src_pts[3] - src_pts[2])
                maxWidth = int(max(widthA, widthB))
                # height: distance between top-left and bottom-left, and between top-right and bottom-right
                heightA = np.linalg.norm(src_pts[0] - src_pts[3])
                heightB = np.linalg.norm(src_pts[1] - src_pts[2])
                maxHeight = int(max(heightA, heightB))

                if maxWidth <= 0 or maxHeight <= 0:
                    raise ValueError('Invalid quad size')

                pad_x_px = int(pad_x_ratio * maxWidth)
                pad_y_px = int(pad_y_ratio * maxHeight)

                dst_w = maxWidth + 2 * pad_x_px
                dst_h = maxHeight + 2 * pad_y_px

                dst_pts = np.array([
                    [pad_x_px, pad_y_px],
                    [pad_x_px + maxWidth - 1, pad_y_px],
                    [pad_x_px + maxWidth - 1, pad_y_px + maxHeight - 1],
                    [pad_x_px, pad_y_px + maxHeight - 1]
                ], dtype=np.float32)

                M = cv.getPerspectiveTransform(src_pts, dst_pts)
                plate_img = cv.warpPerspective(frame, M, (dst_w, dst_h), flags=cv.INTER_CUBIC, borderMode=cv.BORDER_REPLICATE)
                # If the resulting crop is portrait (height > width) but license plates are expected to be landscape,
                # rotate the image by 90 degrees so text is upright.
                try:
                    if plate_img is not None:
                        ph, pw = plate_img.shape[:2]
                        # If plate is taller than wide, rotate clockwise to make it landscape
                        if pw < ph:
                            plate_img = cv.rotate(plate_img, cv.ROTATE_90_CLOCKWISE)
                except Exception:
                    pass
                print(f"[runner] Plate crop (perspective) shape: {plate_img.shape if plate_img is not None else None}", file=sys.stderr)
            except Exception as e:
                # Fallback to axis-aligned crop
                pad_x = int(0.20 * width)  # 20% width
                pad_y = int(0.35 * height) # 35% height
                x_min_pad = max(0, x_min - pad_x)
                x_max_pad = min(w-1, x_max + pad_x)
                y_min_pad = max(0, y_min - pad_y)
                y_max_pad = min(h-1, y_max + pad_y)
                plate_img = frame[y_min_pad:y_max_pad, x_min_pad:x_max_pad]
                print(f"[runner] Plate crop shape (fallback axis-aligned): {plate_img.shape if plate_img is not None else None}", file=sys.stderr)
            # Adaptive enhancement: smaller scale to lower CPU/Memory usage
            try:
                h_plate = plate_img.shape[0] if plate_img is not None else 0
            except Exception:
                h_plate = 0
            sf = 2.0 if h_plate and h_plate < 50 else 1.5
            plate_img = enhance_plate_image(plate_img, scale_factor=sf)
            plate_path = os.path.join(save_dir, f'plate_yunet_frame_{frame_count}_{filtered_count+1}.jpg')
            try:
                cv.imwrite(plate_path, plate_img)
                # Run OCR in-memory first to avoid disk IO and to allow flip-checks
                try:
                    text = recognize_text(plate_img)
                except Exception:
                    # Fallback to path-based OCR
                    text = recognize_text(plate_path)

                # Heuristic: if OCR produced nothing or contains very few allowed characters,
                # try horizontal flip (mirror) and re-run OCR — sometimes perspective warp or
                # camera orientation produces mirrored crops.
                def allowed_fraction(s):
                    if not s:
                        return 0.0
                    cnt = sum(1 for ch in s if ch.upper() in ALLOWLIST)
                    return cnt / max(1, len(s))

                best_text = text or ""
                best_score = allowed_fraction(best_text)
                try:
                    if best_score < 0.5:
                        flipped = cv.flip(plate_img, 1)
                        flipped_text = recognize_text(flipped)
                        flipped_score = allowed_fraction(flipped_text)
                        if flipped_score > best_score and flipped_text:
                            # prefer flipped result
                            best_text = flipped_text
                            best_score = flipped_score
                            # overwrite saved image with corrected (flipped) crop so files shown to user are readable
                            try:
                                cv.imwrite(plate_path, flipped)
                            except Exception:
                                pass
                except Exception:
                    pass

                text = best_text
                print(f"[runner] Plate saved to: {plate_path} (score={score:.2f}, area={area}, height={height}, aspect={aspect:.2f}, text='{text}')", file=sys.stderr)
                # Если распознано хоть что-то, сразу отправить JSON для Node.js
                if text and text.strip():
                    # Попробуем получить camera_id из аргументов (если есть)
                    camera_id = None
                    if hasattr(args, 'video') and args.video:
                        # Попробуем извлечь id из rtsp url, если есть
                        import re
                        m = re.search(r'cam(\d+)', args.video)
                        if m:
                            camera_id = m.group(1)
                    elif hasattr(args, 'camera'):
                        camera_id = args.camera
                    result_json = json.dumps({
                        "recognized": [{
                            "path": plate_path,
                            "score": float(score),
                            "text": text,
                            "camera_id": camera_id,
                            "timestamp": time.strftime('%Y-%m-%dT%H:%M:%S')
                        }]
                    }, ensure_ascii=False)
                    print(result_json)
                    print("")  # Для надёжности, чтобы каждая запись была на отдельной строке
                    sys.stdout.flush()
            except Exception as e:
                print(f"[runner] Failed to save plate image: {e}", file=sys.stderr)
                text = ""
            # Save bbox visualization for diagnostics only when debug mode is enabled
            try:
                if DEBUG_MODE:
                    vis_frame = frame.copy()
                    cv.polylines(vis_frame, [pts], isClosed=True, color=(0,255,0), thickness=2)
                    vis_path = os.path.join(save_dir, f'bbox_yunet_frame_{frame_count}_{filtered_count+1}.jpg')
                    cv.imwrite(vis_path, vis_frame)
                    print(f"[runner] BBox visualization saved to: {vis_path}", file=sys.stderr)
            except Exception as e:
                if DEBUG_MODE:
                    print(f"[runner] Failed to save bbox visualization: {e}", file=sys.stderr)
            filtered_count += 1
        # Сохраняем только если найден хотя бы один номер
        if filtered_count > 0:
            processed_count += 1
        if args.max_frames > 0 and processed_count >= args.max_frames:
            print(f"[runner] Reached max_frames={args.max_frames}, stopping.", file=sys.stderr)
            break
    cap.release()
    print(f"[runner] Video mode finished. Processed {processed_count} frames with detected plates.", file=sys.stderr)
    sys.stderr.flush()
    cap.release()
    print(f"[runner] Video mode finished. Processed {processed_count} frames with detected plates.", file=sys.stderr)
    sys.stderr.flush()


# --- Proper try_open_capture function ---
def try_open_capture(source, backend_preference=None):
    """Try to open a video/camera source with multiple backends for robustness."""
    import cv2 as cv
    backends = [cv.CAP_FFMPEG, cv.CAP_MSMF, cv.CAP_DSHOW, cv.CAP_ANY]
    if backend_preference is not None:
        backends = [backend_preference] + [b for b in backends if b != backend_preference]
    for backend in backends:
        try:
            cap = cv.VideoCapture(source, backend)
            if cap is not None and cap.isOpened():
                print(f"[DEBUG] CAP_PROP_FRAME_WIDTH: {cap.get(cv.CAP_PROP_FRAME_WIDTH)}", file=sys.stderr)
                print(f"[DEBUG] CAP_PROP_FRAME_HEIGHT: {cap.get(cv.CAP_PROP_FRAME_HEIGHT)}", file=sys.stderr)
                print(f"[DEBUG] CAP_PROP_FPS: {cap.get(cv.CAP_PROP_FPS)}", file=sys.stderr)
                sys.stderr.flush()
                return cap, backend
            else:
                print(f"[DEBUG] cap.isOpened() is False for backend {backend}", file=sys.stderr)
                sys.stderr.flush()
        except Exception as e:
            print(f"[DEBUG] Exception with backend {backend}: {e}", file=sys.stderr)
            sys.stderr.flush()
    print(f"[DEBUG] All backends failed to open source {source}", file=sys.stderr)
    sys.stderr.flush()
    return None, None

def visualize(image, dets, line_color=(0, 255, 0)):
    output = image.copy()
    for det in dets:
        bbox = det[:-1].astype(np.int32)
        x1, y1, x2, y2, x3, y3, x4, y4 = bbox
        cv.line(output, (x1, y1), (x2, y2), line_color, 2)
        cv.line(output, (x2, y2), (x3, y3), line_color, 2)
        cv.line(output, (x3, y3), (x4, y4), line_color, 2)
        cv.line(output, (x4, y4), (x1, y1), line_color, 2)
    return output

def enhance_plate_image(plate_img, scale_factor=3.0):
    """Enhance plate image with advanced preprocessing for better OCR accuracy."""
    try:
        # Defensive: if crop failed or is empty, return it unchanged
        if plate_img is None:
            return plate_img
        if hasattr(plate_img, 'size') and plate_img.size == 0:
            return plate_img
        # Convert to grayscale if needed
        if len(plate_img.shape) == 3:
            gray = cv.cvtColor(plate_img, cv.COLOR_BGR2GRAY)
        else:
            gray = plate_img.copy()
        # Apply Gaussian blur to reduce noise
        blurred = cv.GaussianBlur(gray, (3, 3), 0)
        # Enhance contrast using CLAHE
        clahe = cv.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
        enhanced = clahe.apply(blurred)
        # Apply bilateral filter to reduce noise while keeping edges
        filtered = cv.bilateralFilter(enhanced, 9, 75, 75)
        # Apply morphological operations to clean up
        kernel = cv.getStructuringElement(cv.MORPH_RECT, (2, 2))
        morphed = cv.morphologyEx(filtered, cv.MORPH_CLOSE, kernel)
        # Resize with high-quality interpolation
        if scale_factor > 1.0:
            h, w = morphed.shape[:2]
            new_w = int(w * scale_factor)
            new_h = int(h * scale_factor)
            resized = cv.resize(morphed, (new_w, new_h), interpolation=cv.INTER_CUBIC)
        else:
            resized = morphed
        # Apply sharpening filter
        kernel_sharp = np.array([[-1,-1,-1], [-1, 9,-1], [-1,-1,-1]])
        sharpened = cv.filter2D(resized, -1, kernel_sharp)
        # Ensure final image is in correct format
        if len(plate_img.shape) == 3:
            # Convert back to BGR if original was color
            sharpened_bgr = cv.cvtColor(sharpened, cv.COLOR_GRAY2BGR)
            return sharpened_bgr
        return sharpened
    except Exception as e:
        print(f"[WARNING] Image enhancement failed: {e}")
        return plate_img

def recognize_text(image_or_path):
    """Lazy-initialized OCR pipeline using only EasyOCR with GPU.

    Initializes EasyOCR on first call.
    Returns the recognized text or empty string.
    """
    global READER
    results = []

    def init_easyocr():
        global READER
        if READER is None:
            gpu_available = detect_gpu_availability()
            try:
                READER = easyocr.Reader(['en', 'ru'], gpu=gpu_available)
                if gpu_available:
                    print('[runner] EasyOCR initialized with GPU support', file=sys.stderr)
                else:
                    print('[runner] EasyOCR initialized with CPU', file=sys.stderr)
            except Exception as e:
                print(f"[WARNING] easyocr init failed: {e}", file=sys.stderr)
                READER = None

    # Only use EasyOCR
    init_easyocr()
    if READER:
        try:
            # Accept either numpy array (image) or filesystem path
            is_array = hasattr(image_or_path, 'ndim')
            if is_array:
                easyocr_result = READER.readtext(image_or_path, detail=1, paragraph=False, allowlist=ALLOWLIST)
            else:
                easyocr_result = READER.readtext(image_or_path, detail=1, paragraph=False, allowlist=ALLOWLIST)
            if easyocr_result:
                text1 = ' '.join([result[1] for result in easyocr_result if result[2] > 0.2])
                if text1:
                    results.append(text1)
        except Exception as e:
            print(f"[WARNING] EasyOCR recognition failed: {e}", file=sys.stderr)

    if not results:
        return ""
    return max(results, key=len)


if __name__ == '__main__':
    import traceback
    try:
        main()
    except SystemExit as e:
        try:
            print(f"[FATAL] SystemExit called with code={e.code}", file=sys.stderr)
            traceback.print_exc(file=sys.stderr)
            sys.stderr.flush()
        except Exception:
            pass
        # Give caller a moment to read logs
        try:
            time.sleep(2)
        except Exception:
            pass
        raise
    except Exception as e:
        try:
            print(f"[FATAL] Unhandled exception in main: {e}", file=sys.stderr)
            traceback.print_exc(file=sys.stderr)
            sys.stderr.flush()
        except Exception:
            pass
        try:
            time.sleep(2)
        except Exception:
            pass
        # exit with non-zero to indicate failure
        try:
            sys.exit(1)
        except Exception:
            pass
