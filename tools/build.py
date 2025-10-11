#!/usr/bin/env python3
"""
Cross-platform build script for VMS Dashboard
Automatically prepares MediaMTX binaries, the Linux GStreamer runtime,
and builds for the selected target platform.
"""

import os
import sys
import platform
import subprocess
import shutil
from pathlib import Path

def get_current_platform():
    """Detect current platform"""
    system = platform.system().lower()
    if system == "windows":
        return "windows"
    elif system == "linux":
        return "linux"
    elif system == "darwin":
        return "macos"
    else:
        raise ValueError(f"Unsupported platform: {system}")

def prepare_mediamtx_binary(target_platform=None):
    """Prepare MediaMTX binary for target platform"""
    if target_platform is None:
        target_platform = get_current_platform()
    
    print(f"🔧 Preparing MediaMTX binary for {target_platform}...")
    
    # Source and destination paths
    if target_platform == "windows":
        src_binary = Path("src-tauri/binaries/windows/mediamtx.exe")
        dst_binary = Path("src-tauri/binaries/mediamtx.exe")
    else:
        src_binary = Path(f"src-tauri/binaries/{target_platform}/mediamtx")
        dst_binary = Path("src-tauri/binaries/mediamtx")
    
    # Check if source binary exists
    if not src_binary.exists():
        print(f"❌ Binary not found: {src_binary}")
        print("🔄 Running MediaMTX downloader...")
        
        # Run download script
        subprocess.run([sys.executable, "tools/download-mediamtx.py"], check=True)
        
        if not src_binary.exists():
            raise FileNotFoundError(f"Failed to download {target_platform} binary")
    
    # Copy binary to expected location
    dst_binary.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src_binary, dst_binary)
    
    # Make executable on Unix systems
    if target_platform in ["linux", "macos"]:
        os.chmod(dst_binary, 0o755)
    
    print(f"✅ MediaMTX binary ready: {dst_binary}")


def prepare_gstreamer_runtime(target_platform=None):
    """Ensure the bundled GStreamer runtime is available for Linux builds."""
    if target_platform is None:
        target_platform = get_current_platform()

    if target_platform != "linux":
        return

    scanner_path = Path("src-tauri/resources/gstreamer/libexec/gstreamer-1.0/gst-plugin-scanner")
    skip_marker = Path("src-tauri/resources/gstreamer/.download-skipped")

    if scanner_path.exists():
        print("✅ GStreamer runtime already prepared.")
        return

    if skip_marker.exists():
        print("ℹ️  GStreamer bundle download previously skipped; relying on system GStreamer.")
        return

    print("🔧 Preparing GStreamer runtime for Linux bundle...")
    subprocess.run([sys.executable, "tools/download-gstreamer-runtime.py"], check=True)

def run_build(target=None, debug=False):
    """Run Tauri build"""
    print(f"🚀 Building VMS Dashboard...")
    
    # Prepare build command
    if debug:
        cmd = ["npm", "run", "tauri", "--", "build", "--debug"]
    else:
        cmd = ["npm", "run", "tauri-build"]
    
    if target:
        if debug:
            cmd.extend(["--", "--target", target])
        else:
            cmd = ["npm", "run", "tauri", "--", "build", "--target", target]
    
    print(f"📋 Command: {' '.join(cmd)}")
    
    # Run build with shell=True on Windows
    import platform
    use_shell = platform.system().lower() == "windows"
    
    result = subprocess.run(cmd, shell=use_shell)
    
    if result.returncode == 0:
        print("🎉 Build completed successfully!")
    else:
        print("❌ Build failed!")
        sys.exit(1)

def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(description="VMS Dashboard Cross-Platform Builder")
    parser.add_argument("--platform", choices=["windows", "linux", "macos"], 
                      help="Target platform (auto-detected if not specified)")
    parser.add_argument("--target", help="Rust target triple")
    parser.add_argument("--debug", action="store_true", help="Build in debug mode")
    parser.add_argument("--download-only", action="store_true", 
                      help="Only download MediaMTX binaries")
    
    args = parser.parse_args()
    
    try:
        # Determine target platform
        target_platform = args.platform or get_current_platform()
        print(f"🎯 Target platform: {target_platform}")
        
        # Prepare MediaMTX binary
        prepare_mediamtx_binary(target_platform)
        prepare_gstreamer_runtime(target_platform)
        
        if args.download_only:
            print("✅ Download completed!")
            return
        
        # Run build
        run_build(args.target, args.debug)
        
    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()