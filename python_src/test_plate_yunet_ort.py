import cv2 as cv
import numpy as np
from pathlib import Path

from lpd_yunet_ort import LPD_YuNetORT


def load_image(path, size):
    img = cv.imread(str(path))
    if img is None:
        raise RuntimeError(f"Failed to read image {path}")
    img = cv.resize(img, (size[0], size[1]))
    return img


if __name__ == '__main__':
    model = Path(__file__).parent / 'license_plate_detection.onnx'
    sample = None
    # try to find a sample image in repo (public/images or frontend/assets)
    candidates = [Path(__file__).parent.parent / 'public' / 'sample.jpg',
                  Path(__file__).parent.parent / 'frontend' / 'assets' / 'icon.png']
    for c in candidates:
        if c.exists():
            sample = c
            break

    if sample is None:
        print('No sample image found in repo. Please provide a path to an image and re-run the script.')
        exit(1)

    detector = LPD_YuNetORT(str(model), inputSize=[320, 240])
    img = load_image(sample, [320, 240])
    dets = detector.infer(img)
    print('Detections shape:', dets.shape)
    print(dets)
