#!/usr/bin/env python3
"""
Quick test script for ANPR OCR integration
Usage: python test_anpr_integration.py
"""
import sys
import os
from pathlib import Path

def test_imports():
    """Test if all required packages are installed"""
    print("🧪 Testing Python imports...")
    try:
        import torch
        import cv2
        import numpy as np
        from torchvision import transforms
        print(f"  ✅ torch {torch.__version__}")
        print(f"  ✅ cv2 {cv2.__version__}")
        print(f"  ✅ numpy {np.__version__}")
        return True
    except ImportError as e:
        print(f"  ❌ Missing dependency: {e}")
        print("\n💡 Install dependencies:")
        print("   pip install -r requirements.txt")
        return False

def test_model_exists():
    """Check if CRNN model exists"""
    print("\n🧪 Checking CRNN model...")
    model_path = Path("artifacts/anpr/crnn_ocr_model_best.pth")
    if model_path.exists():
        size_mb = model_path.stat().st_size / (1024 * 1024)
        print(f"  ✅ Model found: {model_path} ({size_mb:.2f} MB)")
        return True
    else:
        print(f"  ❌ Model not found: {model_path}")
        print("\n💡 Copy model from external repo:")
        print("   cp external/anpr-system/models/ocr_crnn/crnn_ocr_model_best.pth artifacts/anpr/")
        return False

def test_script_exists():
    """Check if anpr_ocr.py exists"""
    print("\n🧪 Checking OCR script...")
    script_path = Path("src-tauri/python_src/anpr_ocr.py")
    if script_path.exists():
        lines = len(script_path.read_text().splitlines())
        print(f"  ✅ Script found: {script_path} ({lines} lines)")
        return True
    else:
        print(f"  ❌ Script not found: {script_path}")
        return False

def test_ocr_script():
    """Test running OCR script with help flag"""
    print("\n🧪 Testing OCR script execution...")
    import subprocess
    try:
        result = subprocess.run(
            ["python", "src-tauri/python_src/anpr_ocr.py", "--help"],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode == 0:
            print("  ✅ Script executes successfully")
            return True
        else:
            print(f"  ❌ Script failed: {result.stderr}")
            return False
    except Exception as e:
        print(f"  ❌ Execution error: {e}")
        return False

def test_sample_image():
    """Test OCR on a sample image if available"""
    print("\n🧪 Looking for test images...")
    
    # Check if we have test images from external repo
    test_images = [
        Path("external/anpr-system/test_images"),
        Path("external/anpr-system/examples"),
    ]
    
    found_image = None
    for test_dir in test_images:
        if test_dir.exists():
            images = list(test_dir.glob("*.jpg")) + list(test_dir.glob("*.png"))
            if images:
                found_image = images[0]
                break
    
    if not found_image:
        print("  ⚠️  No test images found (optional)")
        return None
    
    print(f"  📸 Found test image: {found_image}")
    print("\n🧪 Running OCR on test image...")
    
    import subprocess
    try:
        result = subprocess.run([
            "python", "src-tauri/python_src/anpr_ocr.py",
            "--model", "artifacts/anpr/crnn_ocr_model_best.pth",
            "--image", str(found_image),
            "--json"
        ], capture_output=True, text=True, timeout=30)
        
        if result.returncode == 0:
            import json
            response = json.loads(result.stdout)
            if response.get("success"):
                print(f"  ✅ OCR successful!")
                print(f"     Latin:    {response.get('latin', 'N/A')}")
                print(f"     Cyrillic: {response.get('cyrillic', 'N/A')}")
                return True
            else:
                print(f"  ❌ OCR failed: {response.get('error', 'Unknown')}")
                return False
        else:
            print(f"  ❌ Script error: {result.stderr}")
            return False
    except Exception as e:
        print(f"  ❌ Test failed: {e}")
        return False

def main():
    print("=" * 60)
    print("🚗 ANPR Integration Test Suite")
    print("=" * 60)
    
    results = []
    
    # Run all tests
    results.append(("Imports", test_imports()))
    results.append(("Model", test_model_exists()))
    results.append(("Script", test_script_exists()))
    results.append(("Execution", test_ocr_script()))
    
    sample_result = test_sample_image()
    if sample_result is not None:
        results.append(("Sample OCR", sample_result))
    
    # Summary
    print("\n" + "=" * 60)
    print("📊 Test Summary")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for test_name, result in results:
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"  {status}  {test_name}")
    
    print(f"\n🎯 Result: {passed}/{total} tests passed")
    
    if passed == total:
        print("\n✅ All tests passed! ANPR integration is ready.")
        print("\n🚀 Next steps:")
        print("   1. Build Rust backend: cd src-tauri && cargo build")
        print("   2. Run application: npm run tauri dev")
        print("   3. Test with real video stream")
        return 0
    else:
        print("\n⚠️  Some tests failed. Fix issues above before proceeding.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
