# OpenIPC Dashboard (C++ Rewrite)

This is a C++ / Qt 6 rewrite of the [OpenIPC Dashboard](https://github.com/OpenIPC/dashboard).

## Prerequisites

*   **C++ Compiler**: MSVC (Visual Studio 2019+) or MinGW.
*   **CMake**: Version 3.16 or higher.
*   **Qt 6**: Installed via the Qt Online Installer. Make sure to install the `Qt 6.x` component for your compiler (e.g., MSVC 2019 64-bit).

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

## Features

*   **Low Latency Streaming**: Optimized for FPV and real-time monitoring with configurable buffer modes (Zero, Balanced, Smooth).
*   **Hardware Acceleration**: Extensive support for GPU decoding including DXVA, D3D11, CUDA, and QuickSync to minimize CPU usage.
*   **Protocol Control**: Ability to force RTSP via TCP, UDP, or HTTP to suit different network conditions.
*   **Video Tools**: Horizontal video mirroring for HUD or teleprompter applications.
*   **Modern Interface**: Clean and responsive dashboard built on Qt 6 and QML.
*   **Customizable**: Settings specifically exposed for tuning player performance.

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
