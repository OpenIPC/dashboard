#!/usr/bin/env python3
"""
MediaMTX binaries downloader for cross-platform builds
Downloads latest MediaMTX binaries for Windows, Linux and macOS
"""

import os
import sys

# Check for required dependencies
try:
    import requests
except ImportError:
    print("Error: 'requests' module not found.")
    print("Please install it with: pip install requests")
    print("Or: pip install -r requirements.txt")
    sys.exit(1)

import zipfile
import tarfile
import shutil
from pathlib import Path

# MediaMTX latest release URL
MEDIAMTX_API_URL = "https://api.github.com/repos/bluenviron/mediamtx/releases/latest"

# Platform mappings
PLATFORMS = {
    "windows": {
        "asset_pattern": "mediamtx_v.*_windows_amd64.zip",
        "binary_name": "mediamtx.exe",
        "extract_func": "extract_zip"
    },
    "linux": {
        "asset_pattern": "mediamtx_v.*_linux_amd64.tar.gz", 
        "binary_name": "mediamtx",
        "extract_func": "extract_tar"
    },
    "macos": {
        "asset_pattern": "mediamtx_v.*_darwin_amd64.tar.gz",
        "binary_name": "mediamtx", 
        "extract_func": "extract_tar"
    }
}

def get_latest_release():
    """Get latest MediaMTX release info"""
    print("🔍 Getting latest MediaMTX release info...")
    response = requests.get(MEDIAMTX_API_URL)
    response.raise_for_status()
    return response.json()

def download_file(url, destination):
    """Download file with progress"""
    print(f"⬇️  Downloading {os.path.basename(destination)}...")
    response = requests.get(url, stream=True)
    response.raise_for_status()
    
    total_size = int(response.headers.get('content-length', 0))
    downloaded = 0
    
    with open(destination, 'wb') as f:
        for chunk in response.iter_content(chunk_size=8192):
            if chunk:
                f.write(chunk)
                downloaded += len(chunk)
                if total_size > 0:
                    percent = (downloaded / total_size) * 100
                    print(f"\r   Progress: {percent:.1f}%", end='', flush=True)
    print()

def extract_zip(archive_path, extract_to):
    """Extract ZIP archive"""
    with zipfile.ZipFile(archive_path, 'r') as zip_ref:
        zip_ref.extractall(extract_to)

def extract_tar(archive_path, extract_to):
    """Extract TAR.GZ archive"""
    with tarfile.open(archive_path, 'r:gz') as tar_ref:
        tar_ref.extractall(extract_to)

def download_platform_binary(platform, asset_url, version):
    """Download and extract binary for specific platform"""
    print(f"\n📦 Processing {platform}...")
    
    # Create platform directory
    platform_dir = Path("src-tauri/binaries") / platform
    platform_dir.mkdir(parents=True, exist_ok=True)
    
    # Download archive
    archive_name = asset_url.split('/')[-1]
    archive_path = platform_dir / archive_name
    download_file(asset_url, archive_path)
    
    # Extract archive
    platform_config = PLATFORMS[platform]
    extract_func = globals()[platform_config["extract_func"]]
    extract_func(archive_path, platform_dir)
    
    # Find and copy binary
    binary_name = platform_config["binary_name"]
    
    # Look for binary in extracted files
    for root, dirs, files in os.walk(platform_dir):
        if binary_name in files:
            src_binary = Path(root) / binary_name
            dst_binary = platform_dir / binary_name
            
            if src_binary != dst_binary:
                shutil.copy2(src_binary, dst_binary)
                # Make executable on Unix systems
                if platform in ["linux", "macos"]:
                    os.chmod(dst_binary, 0o755)
            break
    
    # Clean up
    archive_path.unlink()
    
    # Remove extraction directories
    for item in platform_dir.iterdir():
        if item.is_dir():
            shutil.rmtree(item)
    
    print(f"✅ {platform} binary ready: {binary_name}")

def main():
    """Main function"""
    print("🚀 MediaMTX Cross-Platform Binary Downloader")
    print("=" * 50)
    
    try:
        # Get latest release
        release_data = get_latest_release()
        version = release_data["tag_name"]
        assets = release_data["assets"]
        
        print(f"📋 Latest version: {version}")
        print(f"📋 Found {len(assets)} assets")
        
        # Process each platform
        for platform, config in PLATFORMS.items():
            asset_pattern = config["asset_pattern"]
            
            # Find matching asset
            matching_asset = None
            for asset in assets:
                asset_name = asset["name"]
                # Simple pattern matching - check if the key parts match
                if platform == "windows" and "windows_amd64.zip" in asset_name:
                    matching_asset = asset
                    break
                elif platform == "linux" and "linux_amd64.tar.gz" in asset_name:
                    matching_asset = asset
                    break
                elif platform == "macos" and "darwin_amd64.tar.gz" in asset_name:
                    matching_asset = asset
                    break
            
            if matching_asset:
                download_platform_binary(platform, matching_asset["browser_download_url"], version)
            else:
                print(f"❌ No asset found for {platform} (looking for pattern in asset names)")
                # Debug: show available assets
                print(f"   Available assets:")
                for asset in assets[:3]:  # Show first 3 for debugging
                    print(f"   - {asset['name']}")
        
        print(f"\n🎉 All binaries downloaded successfully!")
        print(f"📁 Binaries location: src-tauri/binaries/")
        
    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()