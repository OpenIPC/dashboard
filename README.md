# OpenIPC Dashboard

[![Latest release](https://img.shields.io/github/v/release/OpenIPC/dashboard)](https://github.com/OpenIPC/dashboard/releases/latest)
[![CI](https://github.com/OpenIPC/dashboard/actions/workflows/ci.yml/badge.svg)](https://github.com/OpenIPC/dashboard/actions/workflows/ci.yml)
[![Build and Release](https://github.com/OpenIPC/dashboard/actions/workflows/release.yml/badge.svg)](https://github.com/OpenIPC/dashboard/actions/workflows/release.yml)

OpenIPC Dashboard is a cross-platform VMS, analytics workspace and OpenIPC-native control center for OpenIPC and ONVIF cameras. It combines a Qt/QML desktop application with a secure embedded Web companion and autonomous server mode, all backed by the same C++ domain state and permission model.

The current stable release is [v0.2.7](https://github.com/OpenIPC/dashboard/releases/tag/v0.2.7).

## Download

- [Windows installer](https://github.com/OpenIPC/dashboard/releases/latest/download/OpenIPC-Dashboard-Installer.exe)
- [Linux AppImage](https://github.com/OpenIPC/dashboard/releases/latest/download/OpenIPC-Dashboard-Linux.AppImage)
- [Release notes](RELEASE_NOTES.md)

On Linux, make the AppImage executable before launching it:

```bash
chmod +x OpenIPC-Dashboard-Linux.AppImage
./OpenIPC-Dashboard-Linux.AppImage
```

## What It Does

### Monitor and media

- Low-latency RTSP playback with `Zero`, `Balanced` and `Smooth` buffer modes.
- Configurable TCP, UDP and HTTP transports, frame watchdogs, bounded reconnect and automatic HD-to-SD fallback.
- Platform-dependent hardware decoding through DXVA, D3D11, CUDA and Intel Quick Sync paths.
- Layouts, per-cell controls, PTZ, audio, snapshots, fullscreen and manual/event recording.
- Unified archive with filtering, playback, clip export, retention controls and safe recovery of incomplete files.

### Discovery and onboarding

- OpenIPC mDNS, ONVIF WS-Discovery, Majestic and legacy WebUI fingerprints.
- Bounded RTSP/HTTP subnet probing and Dahua SDK result merging.
- Confidence/evidence reporting, credential checks and camera profiles before addition.
- Manual add, edit, delete, groups and saved discovery results.

### OpenIPC and Majestic control

- Device status, capabilities, network, time, services, logs, backup, reboot and firmware workflows.
- Schema-driven Majestic configuration using the camera's own `config.schema.json`.
- Minimal nested patches, redacted preview/diff and explicit apply confirmation.
- Live ISP controls, metrics, pipeline reload, snapshots, day/night hardware control and PCM speaker upload.
- Firmware manifest/checksum inspection and guarded update flows, including `/ws/upgrade` where supported.

### Analytics and operations

- Analytics modules, custom polygon zones, event feed and evidence snapshots/clips.
- Shared manual and event recording coordination with EOS-safe finalization.
- Camera Health Center, application logs, diagnostics and exportable diagnostic bundles.
- Versioned SQLite state, legacy migration and observable/testable camera policies.

### Web companion

- Responsive RU/EN interface for desktop, tablet and mobile browsers.
- Authenticated layouts `1/4/9`, camera assignment, WebRTC live view and MJPEG fallback.
- Discovery, onboarding, recording, snapshots, audio, fullscreen, PTZ, archive and downloads.
- Browser-safe settings, users, permissions, sessions, logs and diagnostics.
- Camex, Majestic and OpenIPC actions with capability, preview/diff, confirmation and audit boundaries.
- Versioned REST API, WebSocket state updates, CSRF/Origin validation and role-based access control.

See the [Web server guide](docs/WEB_SERVER.md) and [Web/Desktop parity matrix](docs/WEB_PARITY_MATRIX.md) for deployment details and intentional browser adaptations.

## Quick Start

1. Install or unpack the latest release.
2. Start Dashboard and create the initial administrator account.
3. Open **Camera Search**, select the network interface and discover or add cameras.
4. Assign cameras to monitor cells and choose the required layout.
5. For browser access, open **Settings → Web**, enable the server and use one of the displayed access URLs.

Remote Web access is disabled by default. Keep the server on localhost unless LAN access is explicitly required. Do not expose the plain HTTP port directly to the Internet; use a trusted LAN, VPN or HTTPS reverse proxy.

### Autonomous server mode

The same executable can run without loading the desktop QML window:

```bash
appOpenIPC-Dashboard --server-only
```

Server-only mode uses the saved Web settings. Create the initial administrator once in the desktop application before relying on browser login.

## Security Model

- Desktop and camera credentials use the operating-system credential manager rather than settings files or credential-bearing stream URLs.
- Web sessions use opaque tokens, digest-only server storage, bounded lifetime and revocation after user security changes.
- The browser API does not serialize camera passwords, user hashes/salts, OAuth secrets or arbitrary local filesystem paths.
- State-changing requests enforce permissions, CSRF/Origin validation, input validation, rate limits and audit events.
- Archive and configuration downloads use opaque identifiers and canonical-root validation.
- Destructive firmware/restore operations are never assumed safe: use a backup, stable power and compatible recovery procedure.

See [Security](docs/SECURITY.md) and [Web server security](docs/WEB_SERVER.md#security-headers-and-data-boundaries).

## Supported Platforms

- Windows 10/11 x64. Official releases use Qt 6.4.2 with MinGW 12.2.
- Linux x86_64 through the official AppImage or a source build.
- Chromium-based browsers are the current Web UI release baseline; Firefox and WebKit compatibility remains part of ongoing validation.

For multiple high-bitrate streams, 8 GB or more RAM, hardware video decoding and Gigabit Ethernet are recommended. Actual capacity depends on codec, resolution, frame rate, analytics modules and GPU/driver support.

## Build From Source

### Dependencies

- CMake 3.16 or newer.
- C++17 compiler. Use MinGW 12.2 with the official Qt 6.4 Windows package.
- Qt 6.4+ modules: Quick, Core, Network, Concurrent, Multimedia, SQL, Test and WebSockets.
- GStreamer 1.x development/runtime packages, including SDP and WebRTC components.

### Configure and build

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON
cmake --build build --parallel
```

On Windows, point CMake at the selected Qt installation when it is not already discoverable:

```powershell
cmake -S . -B build `
  -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.4.2/mingw_64" `
  -DBUILD_TESTING=ON
cmake --build build --parallel
```

### Tests

```bash
ctest --test-dir build --output-on-failure
ctest --test-dir build -L unit --output-on-failure
ctest --test-dir build -R "qml_smoke|qml_lint_targeted" --output-on-failure
```

The `v0.2.7` release baseline contains 27 unit/contract tests, two QML smoke tests and targeted QML lint: 30 tests in total. CI validates Linux and Windows, including an RTSP smoke path on Linux. The release workflow produces both the Windows installer and Linux AppImage before publishing a stable GitHub Release.

## Repository Layout

| Path | Purpose |
|---|---|
| `src/backend/` | Domain services, camera state, discovery, recording, analytics and device control |
| `src/backend/presentation/` | Shared Desktop/Web presentation models and safe formatting |
| `src/backend/web/` | Embedded HTTP/WebSocket server, sessions, WebRTC and API v1 |
| `src/ui/` | Desktop Qt Quick/QML interface |
| `src/web/` | Embedded HTML/CSS/JavaScript Web client |
| `tests/` | Unit, contract, policy and persistence tests |
| `.github/workflows/` | Linux/Windows CI and release packaging |

## Documentation

- [Product Roadmap](docs/ROADMAP.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Security](docs/SECURITY.md)
- [Web server deployment](docs/WEB_SERVER.md)
- [Web/Desktop parity matrix](docs/WEB_PARITY_MATRIX.md)
- [Camera discovery](docs/DISCOVERY.md)
- [Majestic integration](docs/MAJESTIC.md)
- [Third-party AI models](docs/THIRD_PARTY_MODELS.md)
- [QML lint baseline](docs/QML_LINT_BASELINE.md)

## Next Direction

With the existing roadmap through P10 and Web/Desktop parity delivered, the next active cycle is P11: production hardening, release engineering, secure Web deployment and UI/runtime debt reduction. Later planned directions cover fleet/sites, incident operations, higher media scale and a versioned integration ecosystem.

See the [Roadmap](docs/ROADMAP.md) for scope, dependencies and release gates.
