# OpenIPC Dashboard

Desktop VMS and analytics client for OpenIPC and ONVIF cameras, built with C++17, Qt 6, QML, GStreamer and ONNX Runtime.

The project is designed around resilient RTSP playback, secure credential handling, transactional state storage and observable/testable camera operations. See [Architecture](docs/ARCHITECTURE.md), [Security](docs/SECURITY.md), [Web server](docs/WEB_SERVER.md) and [third-party AI models](docs/THIRD_PARTY_MODELS.md).

## Prerequisites

*   **C++ Compiler**: MSVC (Visual Studio 2019+) or MinGW.
*   **CMake**: Version 3.16 or higher.
*   **Qt 6.4+** with Quick, Network, Multimedia, SQL and Test.
*   **GStreamer 1.x** development and runtime packages.

## Project Structure

*   `src/main.cpp`: Application entry point.
*   `src/ui/Main.qml`: Main user interface (QML).
*   `src/backend/`: C++ classes handling business logic (replacing Rust).
*   `CMakeLists.txt`: Build configuration.

## How to Build

1.  Open this folder in **Visual Studio Code** or **Qt Creator**.
2.  Configure the project using CMake.
    *   If using VS Code, ensure the `CMake Tools` extension is installed.
    *   Select your Kit (e.g., `Visual Studio Community 2022 Release - amd64`).
3.  Build the project.

### Command Line Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"
cmake --build .
```

### Tests

```bash
cmake -S . -B build -DBUILD_TESTING=ON -DOPENIPC_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
ctest --test-dir build -L unit --output-on-failure
ctest --test-dir build -R qml_smoke --output-on-failure
```

Pull requests are built and tested on Linux and Windows. CI runs C++ unit tests and the `qml_smoke` UI bootstrap as separate gates. Linux CI also publishes a generated H.264 stream through a pinned MediaMTX container and consumes it with GStreamer as an RTSP smoke test.

## Features

*   **Low Latency Streaming**: Optimized for FPV and real-time monitoring with configurable buffer modes (Zero, Balanced, Smooth).
*   **Hardware Acceleration**: Extensive support for GPU decoding including DXVA, D3D11, CUDA, and QuickSync to minimize CPU usage.
*   **Protocol Control**: Ability to force RTSP via TCP, UDP, or HTTP to suit different network conditions.
*   **Resilient Streams**: Frame watchdog, bounded exponential reconnect, authentication-failure blocking and automatic HD-to-SD fallback.
*   **Coordinated Recording**: Manual and event clips share one per-camera recorder with EOS-safe MP4 finalization and buffered evidence fallback.
*   **Secure Secrets**: Login and camera credentials are stored by the operating-system credential manager, not in settings or stream URLs.
*   **Transactional Storage**: Versioned SQLite state with automatic migration from legacy `state.json`.
*   **Verified AI Models**: Pinned downloads, SHA-256 and size checks, atomic replacement, source and license metadata.
*   **OpenIPC / Majestic Control Center**: Runtime schema discovery, full configuration editing, diff preview, per-field reset, live ISP controls, metrics, pipeline reload, backups, snapshots, day/night hardware control and PCM speaker upload.
*   **Multi-layer Camera Discovery**: OpenIPC mDNS, ONVIF WS-Discovery, Majestic and legacy WebUI fingerprints, bounded RTSP/HTTP subnet probing and Dahua SDK results are merged by IP with confidence and evidence.
*   **Video Tools**: Horizontal video mirroring for HUD or teleprompter applications.
*   **Modern Interface**: Clean and responsive dashboard built on Qt 6 and QML.
*   **Web Companion**: Embedded localhost/LAN server with session authentication, role-based REST API, live state updates and protected archive playback.
*   **Customizable**: Settings specifically exposed for tuning player performance.

### Majestic camera control

Right-click an OpenIPC camera and choose **OpenIPC / Majestic**. The control center reads both `/api/v1/config.json` and the camera's own `/api/v1/config.schema.json`, so the available video, image, audio, OSD, motion, recording and outgoing-stream settings follow the installed Majestic build instead of a fixed desktop-side list.

Configuration writes are never implicit. Dashboard builds a minimal nested patch, shows a redacted diff, and only then posts it to `/api/v1/config`. Legacy builds without a schema remain readable but are not offered schema-safe writes. See [Majestic integration](docs/MAJESTIC.md) for API coverage and operational notes.

Camera search uses the OpenIPC firmware's native mDNS marker in addition to ONVIF and bounded active probing. Normal mode scans the local `/24`; deep mode can cover up to `/20`. See [camera discovery](docs/DISCOVERY.md) for protocol coverage and network-boundary limitations.

## System Requirements

### Minimum
*   **OS**: Windows 10 (64-bit)
*   **Processor**: Intel Core i3 (6th Gen) / AMD Ryzen 3 or equivalent
*   **RAM**: 4 GB
*   **Graphics**: GPU with DirectX 11 support
*   **Network**: 100 Mbps Ethernet or 5GHz Wi-Fi

### Recommended
*   **OS**: Windows 10/11 (64-bit)
*   **Processor**: Intel Core i5 / AMD Ryzen 5 or better
*   **RAM**: 8 GB or higher
*   **Graphics**: Dedicated NVIDIA (CUDA) or Intel (QuickSync) GPU for multi-stream hardware decoding
*   **Network**: Gigabit Ethernet usually recommended for multiple high-bitrate streams
