#!/usr/bin/env python3
"""
EasyOCR for license plates - command-line interface
Usage: python easyocr_cli.py <image_path>
"""

import sys
import os
from pathlib import Path

try:
    import easyocr
    import cv2
    import numpy as np
except ImportError as e:
    print(f"ERROR: Missing dependency: {e}", file=sys.stderr)
    print("Install with: pip install easyocr opencv-python", file=sys.stderr)
    sys.exit(1)

# Initialize EasyOCR reader (cache for future calls)
_READER_CACHE = None

def get_reader():
    """Get or create EasyOCR reader"""
    global _READER_CACHE
    if _READER_CACHE is None:
        _READER_CACHE = easyocr.Reader(['ru', 'en'], gpu=False)
    return _READER_CACHE

def preprocess_for_ocr(image_path):
    """Preprocess image for better OCR - minimal processing"""
    img = cv2.imread(str(image_path), cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise ValueError(f"Cannot read image: {image_path}")
    
    # Minimal preprocessing - only slight contrast enhancement
    img = cv2.convertScaleAbs(img, alpha=1.2, beta=10)
    
    return img

def recognize_plate(image_path):
    """Recognize license plate using EasyOCR"""
    try:
        # Preprocess
        processed = preprocess_for_ocr(image_path)
        
        # Get reader
        reader = get_reader()
        
        # Run OCR
        results = reader.readtext(
            processed,
            detail=0,  # Only text, no bounding boxes
            paragraph=False,
            allowlist='0123456789АВЕКМНОРСТУХ',
            width_ths=0.7,  # Merge close words
            height_ths=0.7
        )
        
        # Join all results
        text = ''.join(results).upper()
        
        # Clean result
        cleaned = ''.join(c for c in text if c.isalnum())
        
        return cleaned
        
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return ""

def main():
    if len(sys.argv) != 2:
        print("Usage: python easyocr_cli.py <image_path>", file=sys.stderr)
        sys.exit(1)
    
    image_path = Path(sys.argv[1])
    if not image_path.exists():
        print(f"ERROR: Image not found: {image_path}", file=sys.stderr)
        sys.exit(1)
    
    result = recognize_plate(image_path)
    print(result)  # Output to stdout
    sys.exit(0)

if __name__ == "__main__":
    main()
