# VMS Dashboard - Build Guide

This guide explains how to build VMS Dashboard for all supported platforms (Windows, Linux, macOS) with integrated MediaMTX binaries.

## Overview

VMS Dashboard uses a multi-platform build system that:
- Automatically downloads MediaMTX binaries for target platforms
- Includes the correct binary in each build
- Creates native installers for each platform
- Supports cross-platform building (with limitations)

## Prerequisites

### Required Tools
- **Node.js 18+** - [Download](https://nodejs.org/)
- **Rust 1.70+** - [Install via rustup](https://rustup.rs/)
- **Python 3.6+** - [Download](https://python.org/downloads/)

### Platform-Specific Requirements
- **Windows**: Microsoft C++ Build Tools or Visual Studio
- **Linux**: Build essentials, pkg-config, libssl-dev
- **macOS**: Xcode Command Line Tools

See [Tauri Prerequisites](https://tauri.app/v1/guides/getting-started/prerequisites) for detailed setup.

## Quick Start

```bash
# Clone and setup
git clone https://github.com/OpenIPC/dashboard.git
cd dashboard
npm install

# Download MediaMTX binaries (required)
npm run download-mediamtx

# (Optional) Download bundled GStreamer runtime for Linux AppImage builds
npm run download-gstreamer

# Build for current platform
npm run build-release
```

## Build Commands

### NPM Scripts
```bash
# Download MediaMTX binaries for all platforms
npm run download-mediamtx

# Download the optional GStreamer runtime bundle (Linux packaging)
npm run download-gstreamer

# Build release for current platform
npm run build-release

# Build debug version
npm run build-debug

# Build for specific platforms (requires platform binaries)
npm run build-windows
npm run build-linux
npm run build-macos

# Development mode
npm run tauri
```

### Manual Build Scripts

#### Windows (PowerShell)
```powershell
# Basic build
.\tools\build.ps1

# Build for specific platform
.\tools\build.ps1 -Platform windows
.\tools\build.ps1 -Platform linux
.\tools\build.ps1 -Platform macos

# Debug build
.\tools\build.ps1 -Debug

# Download binaries only
.\tools\build.ps1 -DownloadOnly

# Show help
.\tools\build.ps1 -Help
```

#### Linux/macOS (Bash)
```bash
# Make script executable (first time)
chmod +x tools/build.sh

# Basic build
./tools/build.sh

# Build for specific platform
./tools/build.sh --platform windows
./tools/build.sh --platform linux
./tools/build.sh --platform macos

# Debug build
./tools/build.sh --debug

# Download binaries only
./tools/build.sh --download-only

# Show help
./tools/build.sh --help
```

#### Python (Cross-platform)
```bash
# Basic build
python tools/build.py

# Platform-specific builds
python tools/build.py --platform windows
python tools/build.py --platform linux
python tools/build.py --platform macos

# Debug build
python tools/build.py --debug

# Download only
python tools/build.py --download-only
```

## Build Outputs

### Windows
- **Location**: `src-tauri/target/release/bundle/msi/`
- **Format**: `.msi` installer
- **Includes**: MediaMTX executable, application files
- **Installation**: Double-click to install

### Linux
- **Location**: `src-tauri/target/release/bundle/`
- **Formats**: 
  - `.deb` package (Debian/Ubuntu)
  - `.AppImage` (Universal Linux)
- **Installation**: 
  - DEB: `sudo dpkg -i package.deb`
  - AppImage: `chmod +x app.AppImage && ./app.AppImage`

### macOS
- **Location**: `src-tauri/target/release/bundle/dmg/`
- **Format**: `.dmg` disk image
- **Includes**: Application bundle with MediaMTX
- **Installation**: Drag to Applications folder

## Cross-Platform Building

### Limitations
- **Native compilation**: Each platform builds best on its native OS
- **Cross-compilation**: Possible but requires additional setup
- **Rust targets**: May need to install additional Rust targets

### Installing Rust Targets
```bash
# For cross-compilation (optional)
rustup target add x86_64-pc-windows-gnu     # Windows from Linux
rustup target add x86_64-unknown-linux-gnu  # Linux from other platforms
rustup target add x86_64-apple-darwin       # macOS Intel
rustup target add aarch64-apple-darwin      # macOS Apple Silicon
```

## MediaMTX Integration

### Automatic Download
The build system automatically downloads MediaMTX binaries:
- **Windows**: `mediamtx.exe` from latest GitHub release
- **Linux**: `mediamtx` (x86_64)
- **macOS**: `mediamtx` (x86_64)

### Manual Download
```bash
# Download all platform binaries
npm run download-mediamtx

# Or use Python script directly
python tools/download-mediamtx.py
```

### Binary Locations
```
src-tauri/binaries/
├── windows/mediamtx.exe
├── linux/mediamtx
├── macos/mediamtx
└── mediamtx[.exe]  # Current platform binary
```

## GStreamer Runtime (Linux)

Linux AppImage builds can embed a GStreamer runtime so the application ships
with its own plugin registry and `gst-plugin-scanner`. The repository includes
`tools/download-gstreamer-runtime.py`, which tries to download the official
bundle and stage it under `src-tauri/resources/gstreamer` for packaging.

```bash
# Download or refresh the runtime bundle
npm run download-gstreamer

# Force a fresh download after changing mirrors or local archive
python tools/download-gstreamer-runtime.py --force
```

Because the upstream project no longer publishes redistributable Linux
binaries, the helper script may mark the download as skipped. In that case the
build succeeds and the AppImage falls back to the target system's GStreamer
installation. You can provide your own bundle by setting one of the following
before invoking the download script:

- `GSTREAMER_BUNDLE_URL` – semicolon-separated list of direct URLs to try first
- `GSTREAMER_BUNDLE_ARCHIVE` – path to a local `gstreamer-1.0-x86_64-*.tar.*`

Remove `src-tauri/resources/gstreamer/.download-skipped` or rerun the script
with `--force` after supplying a custom archive to retry bundling.

## Troubleshooting

### Common Issues

#### Missing MediaMTX Binary
```
Error: MediaMTX binary not found for windows
```
**Solution**: Run `npm run download-mediamtx`

#### Build Prerequisites Missing
```
Error: Microsoft C++ Build Tools not found
```
**Solution**: Install platform prerequisites (see Tauri docs)

#### Permission Errors (Linux/macOS)
```
Error: Permission denied
```
**Solution**: 
```bash
chmod +x tools/build.sh
chmod +x src-tauri/binaries/linux/mediamtx
```

#### Node.js Version Issues
```
Error: Node.js version not supported
```
**Solution**: Upgrade to Node.js 18+ using [Node Version Manager](https://github.com/nvm-sh/nvm)

### Debug Mode
For troubleshooting, use debug builds:
```bash
npm run build-debug
# or
python tools/build.py --debug
```

Debug builds:
- Include debug symbols
- Faster compilation
- Larger file size
- More verbose logging

### Build Clean
If builds fail, try cleaning:
```bash
# Clean Rust build artifacts
cargo clean

# Clean Node modules and reinstall
rm -rf node_modules package-lock.json
npm install

# Re-download MediaMTX binaries
npm run download-mediamtx
```

## CI/CD Integration

### GitHub Actions
The project includes GitHub Actions workflows:
- **build.yml**: Automated builds on push
- **release.yml**: Release builds with artifacts
- **prepare-release.yml**: Pre-release preparation

### Local Release Preparation
```bash
# Prepare release builds for all platforms
python tools/build.py --platform windows
python tools/build.py --platform linux  
python tools/build.py --platform macos

# Create release archives
tar -czf vms-dashboard-windows.tar.gz src-tauri/target/release/bundle/msi/
tar -czf vms-dashboard-linux.tar.gz src-tauri/target/release/bundle/deb/ src-tauri/target/release/bundle/appimage/
tar -czf vms-dashboard-macos.tar.gz src-tauri/target/release/bundle/dmg/
```

## Performance Tips

### Build Speed
- Use debug builds for development
- Enable Rust parallel compilation: `export CARGO_BUILD_JOBS=$(nproc)`
- Use SSD for build directory
- Increase RAM allocation for Node.js: `export NODE_OPTIONS="--max-old-space-size=8192"`

### Binary Size
- Release builds are automatically optimized
- Strip debug symbols in production
- Use UPX compression for smaller binaries (optional)

### Development Workflow
```bash
# Fast development cycle
npm run tauri        # Development mode with hot reload
npm run build-debug  # Quick debug build
npm run build-release # Final optimized build
```

## Support

For build-related issues:
1. Check [Prerequisites](https://tauri.app/v1/guides/getting-started/prerequisites)
2. Review this build guide
3. Open an [issue](https://github.com/OpenIPC/dashboard/issues) with:
   - Operating system and version
   - Node.js, Rust, Python versions
   - Complete error output
   - Steps to reproduce