#!/usr/bin/env python3
"""Download ffmpeg essentials build for Windows from gyan.dev."""

from __future__ import annotations

import io
import os
import sys
import zipfile
from pathlib import Path

import requests

# Using gyan.dev essentials build (smaller than full build)
FFMPEG_VERSION = "7.1.1"
FFMPEG_URL = f"https://github.com/GyanD/codexffmpeg/releases/download/{FFMPEG_VERSION}/ffmpeg-{FFMPEG_VERSION}-essentials_build.zip"

BINARY_ROOT = Path("src-tauri") / "binaries"


def ensure_destination(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def download_ffmpeg() -> bytes:
    print(f"Downloading ffmpeg {FFMPEG_VERSION} from gyan.dev...")
    response = requests.get(FFMPEG_URL, timeout=300, stream=True)
    response.raise_for_status()
    
    total_size = int(response.headers.get('content-length', 0))
    downloaded = 0
    chunks = []
    
    for chunk in response.iter_content(chunk_size=8192):
        downloaded += len(chunk)
        chunks.append(chunk)
        if total_size > 0:
            progress = (downloaded / total_size) * 100
            print(f"\rProgress: {progress:.1f}% ({downloaded}/{total_size} bytes)", end='')
    
    print()  # New line after progress
    return b''.join(chunks)


def extract_ffmpeg_binaries(archive_bytes: bytes, dest_dir: Path) -> None:
    """Extract only ffmpeg.exe and ffplay.exe from the archive."""
    print("Extracting ffmpeg binaries...")
    
    with zipfile.ZipFile(io.BytesIO(archive_bytes)) as zf:
        # Find ffmpeg.exe and ffplay.exe in the archive
        for member in zf.namelist():
            if member.endswith('bin/ffmpeg.exe') or member.endswith('bin/ffplay.exe'):
                # Extract to binaries directory
                filename = Path(member).name
                target_path = dest_dir / filename
                
                with zf.open(member) as source:
                    ensure_destination(target_path)
                    with open(target_path, 'wb') as target:
                        target.write(source.read())
                
                print(f"  Extracted: {filename} -> {target_path}")
                
                # Make executable on Unix-like systems
                if os.name != 'nt':
                    target_path.chmod(0o755)


def main() -> int:
    try:
        # Determine destination
        dest_dir = BINARY_ROOT
        ffmpeg_path = dest_dir / "ffmpeg.exe"
        
        # Check if already exists
        if ffmpeg_path.exists():
            print(f"ffmpeg.exe already exists at {ffmpeg_path}")
            if os.environ.get("CI"):
                print("CI environment detected, skipping download.")
                return 0
            
            try:
                response = input("Download again? (y/N): ").strip().lower()
                if response != 'y':
                    print("Skipping download.")
                    return 0
            except EOFError:
                print("Non-interactive mode detected, skipping download.")
                return 0
        
        # Download
        archive_bytes = download_ffmpeg()
        print(f"Downloaded {len(archive_bytes)} bytes")
        
        # Extract
        extract_ffmpeg_binaries(archive_bytes, dest_dir)

        print(
            "\n[SUCCESS] ffmpeg binaries installed. Silent wrappers are now built automatically "
            "via cargo and bundled with the app."
        )
        return 0
        
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
