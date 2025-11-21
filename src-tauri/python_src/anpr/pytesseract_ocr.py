#!/usr/bin/env python3
"""
Pytesseract OCR for license plates - command-line interface
Usage: python pytesseract_ocr.py <image_path>
"""

import sys
import os
from pathlib import Path

try:
    import pytesseract
    from PIL import Image
    import cv2
    import numpy as np
except ImportError as e:
    print(f"ERROR: Missing dependency: {e}", file=sys.stderr)
    print("Install with: pip install pytesseract pillow opencv-python", file=sys.stderr)
    sys.exit(1)

# Configure Tesseract path for Windows
pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'

# Russian license plate characters: digits + Cyrillic letters
# АВЕКМНОРСТУХ - Cyrillic letters used on Russian plates
PLATE_WHITELIST = "0123456789АВЕКМНОРСТУХ"

def preprocess_for_ocr(image_path):
    """Preprocess image for better OCR - optimized for license plates"""
    # Read image
    img = cv2.imread(str(image_path))
    if img is None:
        raise ValueError(f"Cannot read image: {image_path}")
    
    # Convert to grayscale
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    
    # Slightly increase contrast
    gray = cv2.convertScaleAbs(gray, alpha=1.3, beta=10)
    
    # Slight blur to reduce noise
    gray = cv2.GaussianBlur(gray, (3, 3), 0)
    
    return gray

def recognize_plate(image_path):
    """Recognize license plate text using Pytesseract"""
    try:
        # Preprocess
        processed = preprocess_for_ocr(image_path)
        
        # Convert to PIL Image
        pil_img = Image.fromarray(processed)
        
        # Configure Tesseract for single-line Russian text
        config = (
            '--psm 7 '  # Single text line
            '--oem 3 '  # LSTM + Legacy engine
            f'-c tessedit_char_whitelist={PLATE_WHITELIST} '
            '-c language_model_penalty_non_dict_word=0.8 '
            '-c language_model_penalty_non_freq_dict_word=0.8'
        )
        
        # Run OCR with Russian language
        text = pytesseract.image_to_string(
            pil_img,
            lang='rus',
            config=config
        )
        
        # Clean result - keep only alphanumeric
        cleaned = ''.join(c for c in text if c.isalnum()).upper()
        
        return cleaned
        
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return ""

def main():
    if len(sys.argv) != 2:
        print("Usage: python pytesseract_ocr.py <image_path>", file=sys.stderr)
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
