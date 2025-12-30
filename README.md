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

## Roadmap for Porting

1.  **Video Streaming**: Integrate `QtMultimedia` or `LibVLC` to replace the web-based player.
2.  **ONVIF Discovery**: Implement UDP broadcast discovery in `SystemController::scanNetwork`.
3.  **Sidecar Management**: Bundle `go2rtc` and `ffmpeg` binaries and manage them via `QProcess`.
