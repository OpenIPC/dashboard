"""
Download Microsoft.ML.OnnxRuntime.DirectML NuGet package and extract DLLs
"""
import os
import sys
import urllib.request
import zipfile
import shutil
from pathlib import Path
import xml.etree.ElementTree as ET

# Get latest version from NuGet API
def get_latest_version(package_name):
    """Query NuGet API for latest version"""
    api_url = f"https://api.nuget.org/v3-flatcontainer/{package_name.lower()}/index.json"
    try:
        with urllib.request.urlopen(api_url) as response:
            import json
            data = json.loads(response.read())
            versions = data.get('versions', [])
            if versions:
                return versions[-1]  # Last version is latest
    except Exception as e:
        print(f"Error getting version: {e}")
    return None

PACKAGE_NAME = "Microsoft.ML.OnnxRuntime.DirectML"
VERSION = get_latest_version(PACKAGE_NAME) or "1.20.1"
DOWNLOAD_URL = f"https://www.nuget.org/api/v2/package/{PACKAGE_NAME}/{VERSION}"

def main():
    print("=== ONNX Runtime DirectML NuGet Downloader ===\n")
    
    # Create binaries directory
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    binaries_dir = project_root / "src-tauri" / "binaries"
    binaries_dir.mkdir(parents=True, exist_ok=True)
    
    nupkg_path = binaries_dir / f"{PACKAGE_NAME}.{VERSION}.nupkg"
    extract_dir = binaries_dir / "nuget_temp"
    
    print(f"1. Downloading {PACKAGE_NAME} version {VERSION}...")
    print(f"   URL: {DOWNLOAD_URL}")
    
    try:
        with urllib.request.urlopen(DOWNLOAD_URL) as response:
            total_size = int(response.headers.get('content-length', 0))
            print(f"   Size: {total_size / 1024 / 1024:.1f} MB")
            
            with open(nupkg_path, 'wb') as f:
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
    
    print("2. Extracting NuGet package...")
    try:
        if extract_dir.exists():
            shutil.rmtree(extract_dir)
        
        # NuGet package is just a ZIP file
        with zipfile.ZipFile(nupkg_path, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)
        
        print("   ✓ Extraction complete\n")
    except Exception as e:
        print(f"   ✗ Extraction failed: {e}")
        return 1
    
    print("3. Looking for DLL files in runtimes/win-x64/native...")
    dll_source = extract_dir / "runtimes" / "win-x64" / "native"
    
    if not dll_source.exists():
        # Try alternative paths
        print("   Looking in alternative paths...")
        for path in extract_dir.rglob("*.dll"):
            print(f"   Found: {path.relative_to(extract_dir)}")
            dll_source = path.parent
            break
    
    if not dll_source or not dll_source.exists():
        print("   ✗ No DLL directory found")
        return 1
    
    print(f"   Found DLLs in: {dll_source.relative_to(extract_dir)}\n")
    
    print("4. Copying DLL files...")
    dll_count = 0
    
    for dll_path in dll_source.glob("*.dll"):
        dest_path = binaries_dir / dll_path.name
        shutil.copy2(dll_path, dest_path)
        size_mb = dll_path.stat().st_size / 1024 / 1024
        print(f"   ✓ {dll_path.name} ({size_mb:.1f} MB)")
        dll_count += 1
    
    if dll_count == 0:
        print("   ✗ No DLL files found")
        return 1
    
    print(f"\n   Total: {dll_count} files copied\n")
    
    print("5. Cleaning up...")
    nupkg_path.unlink()
    shutil.rmtree(extract_dir)
    print("   ✓ Temporary files removed\n")
    
    print("=== Download Complete ===")
    print(f"\nDLL files are in: {binaries_dir}")
    print(f"\nFiles downloaded:")
    for dll in binaries_dir.glob("*.dll"):
        print(f"  - {dll.name}")
    
    print("\nNext steps:")
    print("  1. Replace system DLL: Copy onnxruntime.dll to C:\\Windows\\System32")
    print("  2. Or rebuild app: npm run tauri build")
    print("  3. DirectML should now work!\n")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
