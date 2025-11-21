"""
Download ONNX Runtime with DirectML support
"""
import os
import sys
import urllib.request
import zipfile
import shutil
from pathlib import Path

ONNX_VERSION = "1.20.1"
# Use DirectML version instead of GPU (CUDA) version
DOWNLOAD_URL = f"https://github.com/microsoft/onnxruntime/releases/download/v{ONNX_VERSION}/onnxruntime-win-x64-{ONNX_VERSION}.zip"

def main():
    print("=== ONNX Runtime DirectML Downloader ===\n")
    
    # Create binaries directory
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    binaries_dir = project_root / "src-tauri" / "binaries"
    binaries_dir.mkdir(parents=True, exist_ok=True)
    
    zip_path = binaries_dir / "onnxruntime.zip"
    extract_dir = binaries_dir / "onnxruntime"
    
    print(f"1. Downloading ONNX Runtime {ONNX_VERSION}...")
    print(f"   URL: {DOWNLOAD_URL}")
    
    try:
        with urllib.request.urlopen(DOWNLOAD_URL) as response:
            total_size = int(response.headers.get('content-length', 0))
            print(f"   Size: {total_size / 1024 / 1024:.1f} MB")
            
            with open(zip_path, 'wb') as f:
                downloaded = 0
                chunk_size = 8192
                while True:
                    chunk = response.read(chunk_size)
                    if not chunk:
                        break
                    f.write(chunk)
                    downloaded += len(chunk)
                    progress = (downloaded / total_size) * 100 if total_size > 0 else 0
                    print(f"\r   Progress: {progress:.1f}%", end='', flush=True)
        
        print("\n   ✓ Download complete\n")
    except Exception as e:
        print(f"\n   ✗ Download failed: {e}")
        return 1
    
    print("2. Extracting archive...")
    try:
        if extract_dir.exists():
            shutil.rmtree(extract_dir)
        
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)
        
        print("   ✓ Extraction complete\n")
    except Exception as e:
        print(f"   ✗ Extraction failed: {e}")
        return 1
    
    print("3. Copying DLL files...")
    dll_count = 0
    
    for dll_path in extract_dir.rglob("*.dll"):
        dest_path = binaries_dir / dll_path.name
        shutil.copy2(dll_path, dest_path)
        print(f"   ✓ {dll_path.name} ({dll_path.stat().st_size / 1024 / 1024:.1f} MB)")
        dll_count += 1
    
    if dll_count == 0:
        print("   ✗ No DLL files found")
        return 1
    
    print(f"\n   Total: {dll_count} files copied\n")
    
    print("4. Cleaning up...")
    zip_path.unlink()
    shutil.rmtree(extract_dir)
    print("   ✓ Temporary files removed\n")
    
    print("=== Download Complete ===")
    print(f"\nDLL files are in: {binaries_dir}")
    print("\nNext steps:")
    print("  1. The DLLs will be automatically bundled with the app")
    print("  2. Rebuild the application: npm run tauri build")
    print("  3. DirectML will be available on first run\n")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
