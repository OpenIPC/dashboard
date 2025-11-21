# VMS Dashboard - Professional Video Management System

<div align="center">

![VMS Dashboard](https://img.shields.io/badge/VMS-Dashboard-blue?style=for-the-badge)
![Tauri](https://img.shields.io/badge/Tauri-2.0-orange?style=for-the-badge)
![React](https://img.shields.io/badge/React-18-blue?style=for-the-badge)
![Rust](https://img.shields.io/badge/Rust-Latest-red?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Modern native video surveillance management system for professionals**

[Download Release](https://github.com/OpenIPC/dashboard/releases) • [Documentation](./docs/) • [Report Bug](https://github.com/OpenIPC/dashboard/issues)

</div>

---

## 🚀 Key Features

### 📹 **Camera Management**
- **Automatic ONVIF camera discovery** in local network
- **Multiple protocols support**: ONVIF, RTSP, OpenIPC, Hikvision, Dahua
- **Two connection modes**: direct RTSP and go2rtc integration
- **Camera grouping and organization**
- **Secure credential storage** with encryption

### 🎬 **Advanced go2rtc Integration** ⭐ NEW
- **Multiple transport protocols**: WebRTC (ultra-low latency), HLS, MSE, MJPEG with automatic fallback
- **Two-way audio support**: bidirectional communication for intercoms and interactive cameras
- **Real-time monitoring**: live bitrate, codec info, viewer count, and latency metrics
- **Adaptive bitrate streaming**: automatic quality switching based on network conditions
- **Quick snapshots**: instant frame capture via `/api/frame.jpeg` without full stream loading
- **Smart transport selection**: automatically chooses optimal protocol for best performance

### 🖥️ **Professional Monitoring Interface**
- **Multi-grid layouts**: from 1 to 64 cameras simultaneously
- **Customizable templates** with tab system
- **Drag & Drop** camera management
- **Fullscreen mode** and detailed viewing
- **Individual cell settings** (audio, pause, recording)
- **Stream statistics overlay**: real-time quality indicators

### 📊 **Archive & Analytics**
- **Interactive timeline** for recording navigation
- **Calendar interface** for search
- **Video clip export** functionality
- **Activity statistics and charts**
- **Event log** with filtering and search

### � **ANPR (License Plate Recognition)** ⭐ NEW
- **Automatic plate detection** using YOLOv8 AI model
- **OCR with perspective correction** for accurate text recognition
- **Cyrillic transliteration** for Russian license plates
- **Real-time processing** with smart caching
- **Hybrid approach**: Python subprocess (primary) + Rust ONNX (fallback)
- **Easy toggle** between Python/Rust OCR modes
- **GPU acceleration** via DirectML for 2-5x faster processing (Windows 10/11) ✨

### 🚀 **GPU Acceleration** ⭐ NEW
- **DirectML support** for NVIDIA, AMD, and Intel GPUs
- **Automatic GPU detection** with CPU fallback
- **60-90% CPU usage reduction** during ANPR analytics
- **2-5x faster inference** for YOLO detection and OCR
- **Easy configuration**: Auto/GPU/CPU selection in settings
- **Bundled ONNX Runtime 1.23.0** with DirectML (no system dependencies)
- **System requirements**: Windows 10 1903+ with DirectX 12 compatible GPU

### �🛡️ **Security & Performance**
- **Native application** without browser limitations
- **Local data processing** without cloud dependencies
- **AES-256 password encryption**
- **Low resource consumption** thanks to Rust/Tauri
- **Hardware acceleration** support for video processing

---

## 📥 Quick Start

### Installation
1. Download the latest [release](https://github.com/OpenIPC/dashboard/releases) for your OS
2. Install the application (Windows: `.msi`, macOS: `.dmg`, Linux: `.deb`/`.AppImage`)
3. Launch VMS Dashboard

### Initial Setup
1. **Add cameras**: use ONVIF auto-discovery or add manually
2. **Create layout**: drag cameras to monitoring grid
3. **Configure recording**: set schedule and archive parameters
4. **Start monitoring**: enjoy professional video surveillance!

---

## 🔧 Development

### System Requirements
- Node.js 18+
- Rust 1.70+
- Platform-specific dependencies (see [Tauri Prerequisites](https://tauri.app/v1/guides/getting-started/prerequisites))

### Local Build

#### Prerequisites
- **Node.js 18+** - [Download](https://nodejs.org/)
- **Rust 1.70+** - [Install](https://rustup.rs/)
- **Python 3.6+** - [Download](https://python.org/downloads/)
- Platform-specific dependencies (see [Tauri Prerequisites](https://tauri.app/v1/guides/getting-started/prerequisites))

#### Quick Start
```bash
# Clone repository
git clone https://github.com/OpenIPC/dashboard.git
cd dashboard

# Install dependencies
npm install

# Install Python dependencies (for ANPR)
pip install -r requirements.txt

# Run integration test (optional)
python test_anpr_integration.py

# Download go2rtc binaries (required for builds)
npm run download-go2rtc

# Download ffmpeg binaries (optional, bundled with installer)
python tools/download-ffmpeg.py

# (Optional) Download bundled GStreamer runtime for Linux AppImage builds
npm run download-gstreamer

# Run in development mode
npm run tauri

# Build release for current platform
npm run build-release
```

#### Cross-Platform Building
```bash
# Build for Windows (from any platform)
npm run build-windows

# Build for Linux (from any platform) 
npm run build-linux

# Build for macOS (from any platform)
npm run build-macos

# Build debug version
npm run build-debug

# Only download go2rtc binaries
npm run download-go2rtc

# Only download the optional GStreamer runtime bundle
npm run download-gstreamer
```

#### Alternative Build Methods
```bash
# Using PowerShell (Windows)
.\tools\build.ps1 --platform windows

# Using Bash (Linux/macOS)
./tools/build.sh --platform linux

# Using Python directly
python tools/build.py --platform macos --debug
```

#### Optional Linux GStreamer Bundle
- `npm run download-gstreamer` stages a redistributable runtime when one is available.
- If no bundle is found, Linux builds rely on the target system's GStreamer installation.
- Provide custom artifacts with `GSTREAMER_BUNDLE_URL` (URLs) or `GSTREAMER_BUNDLE_ARCHIVE` (local file) before rerunning the download script.

#### Build Outputs
- **Windows**: `.msi` installer in `src-tauri/target/release/bundle/msi/`
- **Linux**: `.deb` package and `.AppImage` in `src-tauri/target/release/bundle/`
- **macOS**: `.dmg` installer in `src-tauri/target/release/bundle/dmg/`

#### Release Building
For creating distributable releases:
```bash
# Prepare all platform binaries
npm run download-go2rtc

# Build release installers
npm run build-release

# For other platforms (requires platform-specific setup)
# npm run build-windows  # Windows MSI
# npm run build-linux    # Linux DEB + AppImage  
# npm run build-macos    # macOS DMG
```

See [Build Guide](./docs/build-guide.md) for detailed build instructions and [Release Process](./docs/release-process.md) for creating releases.

#### Manual GitHub Actions Build
You can trigger builds manually without committing code:

1. **Go to**: [GitHub Actions](https://github.com/OpenIPC/dashboard/actions)
2. **Select**: "Build and Test" workflow
3. **Click**: "Run workflow" 
4. **Choose platform**:
   - `all` - Build for all platforms (Windows, Linux, macOS)
   - `windows` - Build only for Windows  
   - `linux` - Build only for Linux
   - `macos` - Build only for macOS

This allows you to test builds on different platforms without automatic triggering on every commit.

#### Automatic Release Build
For official releases, push a version tag:
```bash
git tag v1.0.0
git push origin v1.0.0
```
This automatically builds and creates a GitHub release with installers for all platforms.

### Project Structure
```
dashboard/
├── src/                    # React frontend
│   ├── components/         # UI components
│   ├── contexts/           # React contexts
│   ├── services/           # API services
│   ├── hooks/              # React hooks (useEnhancedVideoStream)
│   └── types/              # TypeScript types
├── src-tauri/              # Rust backend
│   ├── src/                # Core code
│   ├── binaries/           # go2rtc runtime
│   └── capabilities/       # Tauri permissions
└── docs/                   # Documentation
    ├── go2rtc-enhanced-guide.md       # Complete go2rtc guide
    ├── GO2RTC_QUICK_START.md          # Quick start guide
    └── GO2RTC_IMPLEMENTATION_SUMMARY.md  # Implementation details
```

---

## 📚 Documentation

### User Guides
- [Getting Started](./docs/build-guide.md) - Installation and setup
- [go2rtc Enhanced Features](./docs/GO2RTC_QUICK_START.md) - **NEW** Advanced streaming capabilities
- [Camera Configuration](./docs/platform-requirements.md) - Camera setup guide
- [Analytics Setup](./docs/analytics-overview.md) - Video analytics configuration

### Developer Guides
- [Build Guide](./docs/build-guide.md) - Building from source
- [go2rtc API Reference](./docs/go2rtc-enhanced-guide.md) - Complete API documentation
- [Release Process](./docs/release-process.md) - Release workflow

### Feature Documentation
- **go2rtc Integration**:
  - Multiple transports (WebRTC, HLS, MSE, MJPEG)
  - Two-way audio for intercoms
  - Real-time stream monitoring
  - Adaptive bitrate streaming
  - Quick snapshots without full stream loading

---## 📊 System Requirements

### Minimum
- **OS**: Windows 10, macOS 10.15, Ubuntu 18.04
- **RAM**: 4 GB
- **CPU**: Dual-core 2.0 GHz
- **GPU**: H.264 decoding support
- **Network**: 100 Mbps for local cameras

### Recommended
- **RAM**: 8+ GB
- **CPU**: Quad-core 3.0+ GHz
- **GPU**: Discrete with hardware decoding
- **Network**: Gigabit Ethernet
- **Storage**: SSD for recordings

---

## 🤝 Contributing

We welcome community contributions! Please:

1. 🍴 Fork the repository
2. 🌿 Create a feature branch (`git checkout -b feature/amazing-feature`)
3. 💾 Commit your changes (`git commit -m 'Add amazing feature'`)
4. 📤 Push to the branch (`git push origin feature/amazing-feature`)
5. 🔄 Create a Pull Request

### Types of Contributions
- 🐛 **Bug fixes** - fixing issues
- ✨ **Features** - new functionality
- 📝 **Documentation** - improving docs
- 🌍 **Translations** - UI translations
- 🧪 **Testing** - writing tests


## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

## 🆘 Support

- 📋 [Issues](https://github.com/OpenIPC/dashboard/issues) - bug reports and feature requests
- 💬 [Discussions](https://github.com/OpenIPC/dashboard/discussions) - general questions and discussions
- 📖 [Wiki](https://github.com/OpenIPC/dashboard/wiki) - detailed documentation
-  Email: support@openipc.org

---

## 🙏 Acknowledgments

- [OpenIPC](https://openipc.org/) for project support
- [Tauri](https://tauri.app/) for cross-platform framework
- [go2rtc](https://github.com/AlexxIT/go2rtc) for streaming server
- Community for testing and feedback

---

<div align="center">

**⭐ Star this repo if you find it useful!**

[⬆ Back to top](#vms-dashboard---professional-video-management-system)

</div>
