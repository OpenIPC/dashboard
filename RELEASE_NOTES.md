# OpenIPC Dashboard v0.2.4

OpenIPC Dashboard 0.2.4 is a major OpenIPC-focused release. It turns the app from a viewer-oriented dashboard into a much more complete camera control center: stronger discovery, Majestic API integration, firmware operations, better stream health handling, and a more modular UI foundation.

## Highlights

- Added a native OpenIPC / Majestic Control Center inspired by the camera WebUI.
- Added real OpenIPC firmware read/write operations for status, network, time, logs, backup, reboot, update info, and firmware archive upload.
- Reworked Majestic settings into schema-driven sections with localized labels, hints, safe diff review, and reload/restart indicators.
- Improved OpenIPC camera discovery with mDNS, ONVIF WS-Discovery, Majestic HTTP detection, RTSP probing, progress reporting, cancellation, and cross-protocol deduplication.
- Refactored large Dashboard QML areas into reusable components for grid, sidebar, top bar, status bar, dialogs, buttons, badges, and empty states.
- Improved camera status consistency between the video grid, sidebar, and health UI.
- Added stream health, reconnect, quality, and session policy layers with unit tests.
- Fixed SD/HD stream switching for preview/fullscreen OpenIPC streams.
- Added GitHub Actions CI coverage and expanded the test suite.

## OpenIPC / Majestic Control Center

- Reads Majestic configuration from `/api/v1/config.json`.
- Builds settings UI from `/api/v1/config.schema.json`.
- Preserves unknown camera-specific fields instead of rewriting the whole config blindly.
- Applies only a minimal validated patch through `/api/v1/config`.
- Fixes the previous `null` patch issue when saving changed Majestic settings.
- Adds live ISP controls for image tuning.
- Adds raw JSON inspection.
- Adds Prometheus metrics viewing.
- Adds endpoint/capability overview.
- Adds JPEG snapshot and backup helpers.
- Adds clear confirmation flow before applying changes.
- Redacts sensitive values in review flows where possible.

## Firmware operations

The app now talks to real OpenIPC WebUI/CGI endpoints for firmware-level control:

- Status / pulse data.
- Network read/write.
- DHCP/static network configuration.
- Wi-Fi scan.
- Network reset.
- Timezone and NTP server configuration.
- NTP sync and setting camera time from the PC.
- Syslog, Majestic log, and dmesg snapshots.
- Firmware backup download.
- Reboot with explicit confirmation.
- Firmware update information.
- Firmware archive upload to the camera.

Final GitHub firmware upgrade through `/ws/upgrade` is intentionally guarded for now because the bundled Qt tree does not include Qt WebSockets yet. The UI keeps a safe fallback to the camera WebUI for that final flashing step.

## Camera discovery

- Added OpenIPC mDNS discovery.
- Added Majestic HTTP fingerprinting.
- Added RTSP reachability probing.
- Improved ONVIF WS-Discovery probing.
- Added fast and deep scan modes.
- Added progress percentage during network scan.
- Automatically hides the progress bar after scan completion.
- Merges duplicate evidence by IP address.
- Shows detected ports, protocol hints, and confidence.

## Dashboard and UI

- Split large Dashboard pieces into reusable QML components.
- Added modular sidebar, top bar, status bar, grid panel, layout toolbar, toast, and empty state components.
- Added reusable Majestic controls.
- Improved layout grid behavior.
- Improved camera cell overlays and stream badges.
- Standardized stream overlay format: codec, resolution, bitrate, FPS.
- Improved context actions for OpenIPC/Majestic cameras.
- Expanded Russian and English localization for new controls.

## Stream health and camera status

- Added `CameraStatusPolicy`.
- Added `ReconnectPolicy`.
- Added `StreamHealthPolicy`.
- Added `StreamQualityPolicy`.
- Added `StreamSessionPolicy`.
- Offline and stream failure states now take priority over stale optimistic online states.
- Fixed sidebar/grid status mismatch.
- Improved reconnect behavior and stream diagnostics.

## Tests and quality

Added or expanded unit tests for:

- camera onboarding parsing;
- camera status policy;
- Majestic API client;
- OpenIPC firmware client;
- network discovery;
- stream health;
- stream quality;
- stream session logic;
- reconnect policy;
- state store;
- model artifact verification.

Local validation before release:

- Release app build: passed.
- `ctest`: 13/13 passed.
- Firmware client tests: passed.
- Diff whitespace check: passed.

## Known limitations

- Full firmware flashing through `/ws/upgrade` requires Qt WebSockets and is not enabled in this build.
- Live firmware logs over WebSocket are not implemented yet; log snapshots are available.
- Some deeper firmware administration features will continue to expand in future releases.

---

## Русский

OpenIPC Dashboard 0.2.4 — крупный релиз, сфокусированный на OpenIPC-камерах, Majestic API, firmware-операциях, поиске камер, стабильности видеопотоков и приведении интерфейса к более цельной архитектуре.

### Главное

- Добавлен OpenIPC / Majestic Control Center в стиле WebUI камеры.
- Подключены реальные firmware-операции: status, network, time, logs, backup, reboot, update info и upload firmware archive.
- Majestic-настройки теперь строятся из schema камеры, имеют подсказки, локализацию, safe diff review и отметки reload/restart.
- Улучшен поиск OpenIPC-камер: mDNS, ONVIF WS-Discovery, Majestic HTTP probe, RTSP probe, прогресс, отмена и дедупликация.
- Dashboard частично разобран на переиспользуемые QML-компоненты.
- Исправлена рассинхронизация статусов камеры между grid и sidebar.
- Добавлены policy-слои для stream health, reconnect, quality и session.
- Исправлена работа SD/HD потоков для preview/fullscreen.
- Добавлен GitHub Actions CI и расширены unit-тесты.

### Важно

Финальный firmware upgrade через `/ws/upgrade` пока защищён и не запускается напрямую, потому что текущий Qt bundle не содержит Qt WebSockets. Для этого шага оставлен безопасный переход в WebUI камеры.
