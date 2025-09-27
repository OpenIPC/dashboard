import cv2 as cv
import os
from pathlib import Path
from plate_recognizer import recognize_plate_from_rtsp, ORT_AVAILABLE

# find sample image
candidates = [Path(__file__).parent.parent / 'public' / 'sample.jpg',
              Path(__file__).parent.parent / 'frontend' / 'assets' / 'icon.png']
img_path = None
for c in candidates:
    if c.exists():
        img_path = c
        break

if img_path is None:
    print('No sample image found; place an image at public/sample.jpg or frontend/assets/icon.png')
    exit(1)

# We will use cv.VideoCapture on an image by opening it directly (works on OpenCV builds) or just run model on image
img = cv.imread(str(img_path))
if img is None:
    print('Failed to read sample image')
    exit(1)

# Save temp video file alternative isn't ideal; instead, directly call model path via modified small helper
from lpd_yunet_ort import LPD_YuNetORT
from lpd_yunet import LPD_YuNet

# Try ORT path if available
use_ort = ORT_AVAILABLE
print('ORT available:', ORT_AVAILABLE)

if use_ort:
    model = LPD_YuNetORT(str(Path(__file__).parent / 'license_plate_detection.onnx'), inputSize=[320,240])
else:
    model = LPD_YuNet(str(Path(__file__).parent / 'license_plate_detection.onnx'))

img_resized = cv.resize(img, (320,240))

dets = model.infer(img_resized)
print('Detections shape:', dets.shape)
print(dets)
