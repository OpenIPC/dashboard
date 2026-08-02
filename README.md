# OpenIPC Dashboard

[![Latest release](https://img.shields.io/github/v/release/OpenIPC/dashboard)](https://github.com/OpenIPC/dashboard/releases/latest)
[![CI](https://github.com/OpenIPC/dashboard/actions/workflows/ci.yml/badge.svg)](https://github.com/OpenIPC/dashboard/actions/workflows/ci.yml)
[![Build and Release](https://github.com/OpenIPC/dashboard/actions/workflows/release.yml/badge.svg)](https://github.com/OpenIPC/dashboard/actions/workflows/release.yml)

OpenIPC Dashboard is a cross-platform VMS, analytics workspace and OpenIPC-native control center for OpenIPC and ONVIF cameras. It combines a Qt/QML desktop application with a secure embedded Web companion and autonomous server mode, all backed by the same C++ domain state and permission model.

The current stable release is [v0.2.8](https://github.com/OpenIPC/dashboard/releases/tag/v0.2.8).

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
- Multi-page layouts with preserved assignments, kiosk/page cycling, per-cell digital zoom,
  PTZ, audio, desktop push-to-talk, snapshots, fullscreen and manual/event recording.
- Per-user camera scope is shared by the Qt application and Web server; Talk remains a
  separate permission and does not follow automatically from Live View or PTZ.
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
- Native Qt Sites/Fleet workspace with `Site → Area`, groups and tags, version/health/last-seen
  inventory, saved filters and site-scoped operator access.
- Redacted configuration baselines and drift reports, plus guarded batch inventory, health,
  configuration-read and baseline-apply workflows with preflight, dry-run, maintenance windows,
  bounded concurrency, cancellation, per-device outcomes, backup and audit.
- Offline import/export of site definitions and credential-free inventory/diagnostic exports.
- Versioned SQLite state, legacy migration and observable/testable camera policies.

### Web companion

- Responsive RU/EN interface for desktop, tablet and mobile browsers.
- Authenticated paged layouts `1/4/9`: assignments may exceed the visible grid, with manual
  navigation, kiosk mode and automatic page cycling.
- WebRTC live view with MJPEG fallback, per-cell digital zoom/pan, recording, snapshots,
  audio, fullscreen and PTZ.
- Per-user camera scope enforced by the API and WebSocket paths, plus a separate browser
  push-to-talk permission for OpenIPC/Majestic cameras.
- Discovery, onboarding, recording, snapshots, audio, fullscreen, PTZ, archive and downloads.
- Browser-safe settings, users, permissions, sessions, logs and diagnostics.
- Camex, Majestic and OpenIPC actions with capability, preview/diff, confirmation and audit boundaries.
- Versioned REST API, WebSocket state updates, CSRF/Origin validation and role-based access control.
- Explicit localhost/LAN/VPN/reverse-proxy profiles, readiness probes and auditable session management.

See the [Web server guide](docs/WEB_SERVER.md), [secure deployment guide](docs/WEB_DEPLOYMENT.md) and [Web/Desktop parity matrix](docs/WEB_PARITY_MATRIX.md) for deployment details and intentional browser adaptations.

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

Server-only mode uses the saved Web settings. For a new headless host, the first administrator can
be created once from a restricted local password file:

```bash
OPENIPC_INITIAL_ADMIN_PASSWORD_FILE=/run/openipc-dashboard-admin \
  appOpenIPC-Dashboard --server-only --initialize-admin admin
```

Remove the password file after the successful first start. Subsequent starts use only
`--server-only`. See [Secure Web deployment](docs/WEB_DEPLOYMENT.md) for systemd, Windows scheduled
task, VPN and HTTPS reverse-proxy guidance.

For a dedicated service profile, set `OPENIPC_DATA_ROOT` to an absolute writable directory. This
keeps headless users, state, settings, analytics data and logs separate from the desktop profile.

## Security Model

- Desktop and camera credentials use the operating-system credential manager rather than settings files or credential-bearing stream URLs.
- Web sessions use opaque tokens, digest-only server storage, bounded lifetime and revocation after user security changes.
- The browser API does not serialize camera passwords, user hashes/salts, OAuth secrets or arbitrary local filesystem paths.
- State-changing requests enforce permissions, CSRF/Origin validation, input validation, rate limits and audit events.
- Camera-scoped users are filtered and authorized again on direct preview, WebRTC, recording,
  PTZ, analytics, health and archive paths; push-to-talk has its own permission bit.
- Archive and configuration downloads use opaque identifiers and canonical-root validation.
- Destructive firmware/restore operations are never assumed safe: use a backup, stable power and compatible recovery procedure.

See [Security](docs/SECURITY.md) and [Web server security](docs/WEB_SERVER.md#security-headers-and-data-boundaries).

## Supported Platforms

- Windows 10/11 x64. Official releases use Qt 6.4.2 with MinGW 12.2.
- Linux x86_64 through the official AppImage or a source build.
- Microsoft Edge, Chromium, Firefox and WebKit UI flows are exercised by the release browser gate. WebRTC codec availability remains browser/OS dependent and falls back to MJPEG.

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

The current tree contains 30 unit/contract tests, four QML scale smoke tests,
targeted QML lint, a static Web-module contract and a server lifecycle test: 38 CTest gates in
total. Release CI additionally validates authenticated admin/viewer browser paths in Edge,
Chromium, Firefox and WebKit, then smoke-tests the installed Windows package and extracted Linux
AppImage. Published assets include SHA-256 checksums, release metadata and a CycloneDX SBOM.

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
- [Secure Web deployment](docs/WEB_DEPLOYMENT.md)
- [Web/Desktop parity matrix](docs/WEB_PARITY_MATRIX.md)
- [Upgrade compatibility](docs/UPGRADE_COMPATIBILITY.md)
- [Camera discovery](docs/DISCOVERY.md)
- [Majestic integration](docs/MAJESTIC.md)
- [Sites and Fleet management](docs/FLEET_MANAGEMENT.md)
- [Incident event foundation](docs/INCIDENT_EVENTS.md)
- [Third-party AI models](docs/THIRD_PARTY_MODELS.md)
- [QML lint baseline](docs/QML_LINT_BASELINE.md)

## Next Direction

P12 Sites / Fleet Management is implemented in the main Qt application. P13.1 now provides the
normalized, credential-safe event foundation for Health, Analytics, Archive/Recording and Fleet
Audit. The next active package is P13.2: incident lifecycle and immutable operator timeline.
Later directions cover notification delivery, higher media scale and a versioned integration
ecosystem.

See the [Roadmap](docs/ROADMAP.md) for scope, dependencies and release gates.
