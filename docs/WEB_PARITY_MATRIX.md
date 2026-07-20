# Web/Desktop Parity Matrix

Матрица является рабочим контрактом P6.8. Статус меняется только вместе с кодом и проверкой
соответствующего сценария.

Обозначения: `Done` — готово и проверено; `Partial` — работает основной сценарий;
`Planned` — входит в P6.9–P6.16; `Web N/A` — нативная функция заменена browser-адаптацией.

| Сценарий | Desktop source | Backend boundary | Web surface | Статус | Этап |
|---|---|---|---|---|---|
| Login/logout/session expiry | `LoginView.qml` | `UserManager`, `DashboardWebSessionStore` | Login view | Done | P6.2 |
| RBAC Live/Playback/PTZ/Export/Settings/Users/Analytics | `Main.qml`, dialogs | `UserManager` | Все workspace | Done | P6.9, P6.11 |
| Monitor layout 1/4/9 | `DashboardGridPanel.qml`, `DashboardLayoutToolbar.qml` | Camera/grid state | Monitor grid | Done | P6.5 |
| Device sidebar/search/assignment | `DashboardSidebar.qml`, `DeviceListItem.qml` | `CameraModel` | Device sidebar | Done | P6.5 |
| Camera discovery | `CameraSearchDialog.qml` | `NetworkDiscoveryService` | Discovery dialog | Done | P6.7 |
| Add/edit/delete camera | `AddCameraDialog.qml`, context dialogs | `SystemController`, `CameraModel` | Camera dialog | Done | P6.7 |
| WebRTC/MJPEG live preview | `GridCell.qml` | GStreamer/WebRTC managers | Monitor cell | Done | P6.5 |
| Recording | `GridCell.qml`, `RecordingPulseIndicator.qml` | `DashboardWebRecordingManager`/catalog | Monitor controls | Done | P6.10 |
| Snapshot | `GridCellControlsOverlay.qml` | Preview/snapshot adapter | Monitor controls/download | Done | P6.10 |
| Audio mute/volume | `GridCellControlsOverlay.qml` | WebRTC audio track | Monitor controls | Done | P6.10 |
| Fullscreen | `GridCell.qml` | Browser Fullscreen API | Monitor cell | Done | P6.10 |
| PTZ feedback | `PtzControlPanel.qml` | `PtzController` | Monitor overlay | Done | P6.10 |
| Health overview/run/details | `CameraHealthDialog.qml`, `HealthDetailsDialog.qml` | `CameraHealthController` | Health workspace | Partial | P6.9, P6.15 |
| Analytics modules/events/evidence | `AnalyticsView.qml`, `analytics/*` | `AnalyticsEngine` | Analytics workspace | Partial | P6.9, P6.15 |
| Archive inventory/playback | `ArchiveView.qml`, `Archive*` | `RecordingFileCatalog` | Archive workspace | Done | P6.10 |
| Archive download/export | `ArchiveExportDialog.qml` | Opaque file ID/Range endpoint | Browser download | Done | P6.10, P6.14 |
| Application/player/recording settings | `SettingsDialog.qml`, `SettingsStreamingPage.qml` | `DashboardPresentation` allowlist | Admin workspace | Done | P6.11 |
| Web server settings | `SettingsWebServerPage.qml` | `DashboardWebServer` | Read-only browser state; endpoint changes desktop-only | Web N/A | P6.11, P6.14 |
| Users/permissions/password | `UserManagementDialog.qml`, `AddUserDialog.qml` | `UserManager` | Admin workspace | Done | P6.11 |
| Logs/filter/export/live tail | `LogView.qml` | `LogModel` | Diagnostics workspace | Done | P6.12 |
| Diagnostics/metrics | Settings/Health panels | controllers | Diagnostics workspace | Done | P6.12 |
| Camex command/config generation | `CamexView.qml` | `CamexController` | Device workspace | Done | P6.13 |
| Majestic config/metrics/backup | `Majestic*` | `MajesticClient` | Read/metrics + preview/diff/apply; file backup via Web config export | Done | P6.13, P6.14 |
| OpenIPC status/network/time/logs | `OpenIpc*Page.qml` | `OpenIpcFirmwareClient` | Control Center | Done | P6.13 |
| Firmware update/reboot | `OpenIpcUpdatePage.qml` | `OpenIpcFirmwareClient` | Confirmed/idempotent safe action flow | Done | P6.13 |
| SSH terminal | `SshTerminalDialog.qml` | bounded device API | General terminal intentionally excluded | Web N/A | P6.13, P6.14 |
| Native open/save dialogs | Multiple dialogs | canonical roots/opaque IDs | Upload/download | Web N/A | P6.14 |
| Keychain access | Settings/auth flows | backend only | configured/not configured | Web N/A | P6.14 |
| Tray/window chrome/global shortcuts | `Main.qml`, window controls | OS integration | Browser-native equivalents | Web N/A | P6.14 |
| RU/EN localization | `I18n.qml` | shared semantic keys | Web catalog | Done | P6.9, P6.15 |
| Theme/design tokens | `Theme.qml` | `DashboardPresentation` | CSS variables | Done | P6.9, P6.15 |
| Responsive/accessibility/cross-browser | QML scalable layout | presentation states | CSS/DOM | Done (Chromium baseline) | P6.15 |

## Эталонные состояния

Для каждого экрана проверяются: loading, empty, ready, disabled/forbidden, validation error,
backend error, offline/reconnect и session expired. Эталонные viewport: 1600×900, 1366×768,
1024×768, 768×1024 и 390×844. Темы: dark; локали: RU и EN.

## Capability contract

`GET /api/v1/presentation` возвращает versioned capability manifest, permission catalog и
design tokens. Web не выводит недоступное действие только по локальной проверке роли:
каждая операция повторно проверяется backend endpoint.

## Проверенный baseline 2026-07-19

- release build: Qt 6.4.2 / MinGW 12.2;
- `ctest -j2`: 30/30, включая обычный/scaled QML smoke и targeted QML lint;
- browser: Chromium-based Codex in-app browser;
- viewport 1600×900 и 390×844: login layout, RU/EN, autofocus и отсутствие horizontal overflow;
- server-only: public server/root `200`, CSP присутствует, защищённый dashboard без сессии `401`;
- authenticated operator/admin screenshots требуют тестовую учётную запись и реальные/fixture камеры и остаются release-check, а не хранятся с пользовательскими credentials в репозитории.
