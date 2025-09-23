#!/usr/bin/env python3
"""
Alternative OCR implementation using PaddleOCR for better Russian text recognition
"""

import cv2 as cv
import numpy as np
import os
import sys
from pathlib import Path

try:
    from paddleocr import PaddleOCR
    PADDLE_AVAILABLE = True
except ImportError:
    PADDLE_AVAILABLE = False

class PaddleOCRProcessor:
    def __init__(self):
        if not PADDLE_AVAILABLE:
            print("[WARNING] PaddleOCR not available. Install with: pip install paddlepaddle paddleocr")
            return

        # Initialize PaddleOCR with Russian language support
        self.ocr = PaddleOCR(
            use_angle_cls=True,
            lang='en',  # Use English as base, we'll handle Russian separately
            show_log=False,
            use_gpu=False  # Set to True if GPU available
        )

    def recognize_text(self, image_path):
        """Recognize text using PaddleOCR with Russian optimizations."""
        if not PADDLE_AVAILABLE:
            return ""

        try:
            # Read image
            img = cv.imread(image_path)
            if img is None:
                return ""

            # Preprocessing for better recognition
            processed_img = self.preprocess_image(img)

            # Run OCR
            results = self.ocr.ocr(processed_img, cls=True)

            if not results or not results[0]:
                return ""

            # Extract text from results
            texts = []
            for line in results[0]:
                if len(line) >= 2:
                    text = line[1][0]  # Text content
                    confidence = line[1][1]  # Confidence score

                    if confidence > 0.5:  # Filter by confidence
                        texts.append(text)

            # Combine and clean results
            combined_text = ' '.join(texts)
            cleaned_text = self.postprocess_text(combined_text)

            print(f"[PaddleOCR] Result: '{cleaned_text}'")
            return cleaned_text

        except Exception as e:
            print(f"[WARNING] PaddleOCR failed: {e}")
            return ""

    def preprocess_image(self, img):
        """Advanced image preprocessing for OCR."""
        # Convert to grayscale
        if len(img.shape) == 3:
            gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
        else:
            gray = img.copy()

        # Enhance contrast
        clahe = cv.createCLAHE(clipLimit=3.0, tileGridSize=(8,8))
        enhanced = clahe.apply(gray)

        # Denoise
        denoised = cv.fastNlMeansDenoising(enhanced)

        # Morphological operations
        kernel = cv.getStructuringElement(cv.MORPH_RECT, (2,2))
        morphed = cv.morphologyEx(denoised, cv.MORPH_CLOSE, kernel)

        # Resize for better OCR
        height, width = morphed.shape
        if width < 300:
            scale_factor = 300 / width
            new_width = int(width * scale_factor)
            new_height = int(height * scale_factor)
            resized = cv.resize(morphed, (new_width, new_height), interpolation=cv.INTER_CUBIC)
        else:
            resized = morphed

        return resized

    def postprocess_text(self, text):
        """Post-process recognized text for Russian license plates."""
        if not text:
            return ""

        # Clean up text
        cleaned = ''.join(c for c in text if c.isalnum() or c in 'АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ').upper()

        # Common OCR error corrections for Russian
        corrections = {
            '0': 'O', '1': 'I', '8': 'B',
            'З': '3', 'О': 'O', 'А': 'A',
            'В': 'B', 'С': 'C', 'Н': 'H',
            'М': 'M', 'Т': 'T', 'Р': 'P'
        }

        for wrong, correct in corrections.items():
            cleaned = cleaned.replace(wrong, correct)

        # Try to extract license plate pattern
        import re
        pattern = r'[АВЕКМНОРСТУХABEKMHOPCTYX]\d{3}[АВЕКМНОРСТУХABEKMHOPCTYX]{2}\d{3}[АВЕКМНОРСТУХABEKMHOPCTYX]{0,2}'
        match = re.search(pattern, cleaned)
        if match:
            return match.group()

        return cleaned

def test_paddle_ocr():
    """Test PaddleOCR on license plate images."""
    processor = PaddleOCRProcessor()

    plates_dir = r"E:\VMS\License Plates"
    if not os.path.exists(plates_dir):
        print(f"Directory {plates_dir} not found")
        return

    image_files = [f for f in os.listdir(plates_dir) if f.endswith('.jpg')][:3]

    print("Testing PaddleOCR on license plate images:")

    for image_file in image_files:
        image_path = os.path.join(plates_dir, image_file)
        result = processor.recognize_text(image_path)
        print(f"{image_file}: {result}")

if __name__ == "__main__":
    test_paddle_ocr()