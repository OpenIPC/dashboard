# Platform Requirements for VMS Dashboard

This document details the system requirements and platform-specific considerations for building and running VMS Dashboard.

## Overview

VMS Dashboard is built using Tauri 2.0, which provides native performance across Windows, Linux, and macOS while maintaining a single codebase for the user interface.

## Supported Platforms

### Windows
- **Minimum Version**: Windows 10 (1903 or later)
- **Recommended**: Windows 11
- **Architecture**: x86_64 (64-bit)
- **Runtime**: WebView2 (automatically installed)

### Linux  
- **Distributions**: 
  - Ubuntu 18.04 LTS or later
  - Debian 10 or later
  - Fedora 32 or later
  - CentOS 8 or later
  - Arch Linux (current)
- **Architecture**: x86_64 (64-bit)
- **Desktop Environment**: Any modern DE (GNOME, KDE, XFCE, etc.)

### macOS
- **Minimum Version**: macOS 10.15 (Catalina)
- **Recommended**: macOS 12 (Monterey) or later
- **Architecture**: 
  - Intel x86_64
  - Apple Silicon (ARM64) - via Rosetta 2

## Hardware Requirements

### Minimum Requirements
| Component | Specification |
|-----------|---------------|
| **CPU** | Dual-core 2.0 GHz |
| **RAM** | 4 GB |
| **Storage** | 500 MB free space |
| **GPU** | H.264 hardware decoding support |
| **Network** | 100 Mbps Ethernet |

### Recommended Requirements
| Component | Specification |
|-----------|---------------|
| **CPU** | Quad-core 3.0 GHz+ |
| **RAM** | 8 GB+ |
| **Storage** | 2 GB free space (SSD recommended) |
| **GPU** | Discrete GPU with hardware decoding |
| **Network** | Gigabit Ethernet |

### Performance Considerations
- **Multiple Camera Streams**: 2 GB RAM per 16 concurrent streams
- **Recording**: Additional storage based on retention needs
- **High Resolution**: Hardware H.264/H.265 decoding recommended for 4K streams
- **Network Bandwidth**: 2-8 Mbps per camera depending on resolution/quality

## Development Requirements

### Build Environment

#### All Platforms
- **Node.js**: 18.0 or later ([Download](https://nodejs.org/))
- **Rust**: 1.70 or later ([Install via rustup](https://rustup.rs/))
- **Python**: 3.6 or later ([Download](https://python.org/downloads/))
- **Git**: Latest version ([Download](https://git-scm.com/))

#### Windows-Specific
- **Microsoft C++ Build Tools** or **Visual Studio 2019/2022**
  - Install via [Visual Studio Installer](https://visualstudio.microsoft.com/downloads/)
  - Required components:
    - MSVC v143 compiler
    - Windows 10/11 SDK
    - CMake tools for Visual Studio
- **WebView2**: Usually pre-installed on Windows 10/11

#### Linux-Specific
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y \
    build-essential \
    curl \
    wget \
    file \
    pkg-config \
    libssl-dev \
    libgtk-3-dev \
    libayatana-appindicator3-dev \
    librsvg2-dev \
    libwebkit2gtk-4.0-dev

# Fedora
sudo dnf install -y \
    gcc \
    gcc-c++ \
    make \
    pkg-config \
    openssl-devel \
    gtk3-devel \
    libayatana-appindicator-gtk3-devel \
    librsvg2-devel \
    webkit2gtk3-devel

# Arch Linux
sudo pacman -S --needed \
    base-devel \
    curl \
    wget \
    file \
    pkg-config \
    openssl \
    gtk3 \
    libayatana-appindicator \
    librsvg \
    webkit2gtk
```

#### macOS-Specific
- **Xcode Command Line Tools**:
  ```bash
  xcode-select --install
  ```
- **Homebrew** (recommended for dependencies):
  ```bash
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  ```

### Rust Targets for Cross-Compilation

#### Install Additional Targets
```bash
# Windows targets
rustup target add x86_64-pc-windows-msvc
rustup target add x86_64-pc-windows-gnu

# Linux targets  
rustup target add x86_64-unknown-linux-gnu
rustup target add aarch64-unknown-linux-gnu

# macOS targets
rustup target add x86_64-apple-darwin
rustup target add aarch64-apple-darwin
```

## Runtime Dependencies

### MediaMTX Integration
VMS Dashboard includes MediaMTX streaming server:
- **Version**: 1.15.1 (automatically downloaded)
- **Platforms**: Windows, Linux x86_64, macOS x86_64
- **Purpose**: RTSP proxy, format conversion, stream management

### Network Configuration
- **Firewall**: Allow inbound connections on configured ports
- **RTSP**: Default port 8554 (configurable)
- **HTTP API**: Default port 9997 (configurable)
- **Camera Discovery**: UDP multicast for ONVIF discovery

### Security Considerations
- **Antivirus**: May need to whitelist VMS Dashboard and MediaMTX
- **Windows Defender**: Automatic exclusions for signed binaries
- **Network Security**: Consider VPN access for remote camera management

## Platform-Specific Features

### Windows
- **Native Installer**: MSI package with automatic updates
- **Windows Service**: Optional service mode for background operation
- **Registry Integration**: Settings stored in Windows Registry
- **Start Menu**: Desktop shortcuts and Start Menu integration

### Linux
- **Package Formats**: 
  - DEB packages for Debian/Ubuntu
  - AppImage for universal compatibility
- **Desktop Integration**: .desktop files for application menus
- **Systemd**: Service files for daemon mode
- **Configuration**: XDG Base Directory specification

### macOS
- **DMG Installer**: Drag-and-drop installation
- **Application Bundle**: Standard .app package
- **Keychain**: Secure credential storage
- **Sandbox**: App Store compatibility (future)

## Performance Optimization

### Platform-Specific Optimizations

#### Windows
- **Hardware Acceleration**: DirectX/Direct3D for video rendering
- **Memory Management**: Large Address Aware for >2GB RAM usage
- **Process Priority**: Automatic priority adjustment for camera processing

#### Linux
- **GPU Acceleration**: VA-API/VDPAU hardware decoding
- **Memory**: Kernel buffer tuning for network intensive operations
- **Scheduling**: Real-time scheduling for critical video threads

#### macOS
- **Metal Rendering**: GPU-accelerated video display
- **Core Audio**: Low-latency audio processing
- **Power Management**: Efficient CPU usage on battery

### Resource Monitoring
```bash
# Monitor VMS Dashboard resource usage

# Windows
tasklist /fi "imagename eq dashboard.exe" /fo table

# Linux  
ps aux | grep dashboard
htop -p $(pgrep dashboard)

# macOS
ps aux | grep Dashboard
sudo fs_usage -w -f pathname | grep Dashboard
```

## Troubleshooting

### Common Issues

#### Build Failures
```bash
# Clear build cache
cargo clean
rm -rf node_modules package-lock.json
npm install

# Update Rust
rustup update

# Update Node.js
npm install -g npm@latest
```

#### Runtime Issues
- **Missing Dependencies**: Check platform-specific requirements
- **Permission Errors**: Run with appropriate privileges for network access
- **Firewall Blocking**: Configure firewall rules for RTSP/HTTP ports
- **GPU Issues**: Update graphics drivers, fallback to software decoding

#### Platform-Specific

##### Windows
- **WebView2 Missing**: Download from Microsoft
- **Visual C++ Runtime**: Install latest redistributable
- **Port Conflicts**: Check for conflicting applications on network ports

##### Linux
- **Missing Libraries**: Install development packages
- **AppImage Permissions**: `chmod +x *.AppImage`
- **Display Issues**: Check Wayland/X11 compatibility

##### macOS
- **Gatekeeper**: Allow app in Security & Privacy settings
- **Permissions**: Grant camera/microphone permissions if needed
- **Rosetta**: Install for Intel app compatibility on Apple Silicon

## Future Platform Support

### Planned Support
- **ARM64 Linux**: Native builds for Raspberry Pi 4+, ARM servers
- **Windows ARM**: Native builds for Windows on ARM devices
- **FreeBSD**: Community-requested Unix variant

### Considerations
- **Mobile Platforms**: iOS/Android via Tauri Mobile (future)
- **Web Assembly**: Browser-based deployment option
- **Embedded Systems**: Lightweight builds for OpenWrt/embedded Linux

## Validation

### Testing Matrix
| Platform | Version | Architecture | Status |
|----------|---------|--------------|--------|
| Windows 10 | 1903+ | x86_64 | ✅ Fully Supported |
| Windows 11 | All | x86_64 | ✅ Fully Supported |
| Ubuntu LTS | 18.04+ | x86_64 | ✅ Fully Supported |
| Debian | 10+ | x86_64 | ✅ Fully Supported |
| Fedora | 32+ | x86_64 | ✅ Supported |
| macOS | 10.15+ | x86_64 | ✅ Fully Supported |
| macOS | 11+ | ARM64 | 🔄 Via Rosetta 2 |

### Automated Testing
- **GitHub Actions**: Continuous testing on all supported platforms
- **Virtual Machines**: Automated testing on clean installations
- **Hardware Testing**: Physical device validation for camera compatibility

---

For the most up-to-date requirements and troubleshooting, see the [Build Guide](./build-guide.md) and [GitHub Issues](https://github.com/OpenIPC/dashboard/issues).