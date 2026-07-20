# OpenIPC Dashboard v0.2.7

OpenIPC Dashboard v0.2.7 delivers the P6 Web/Desktop parity milestone: a secure embedded Web companion and autonomous server mode with a responsive operator and administration interface.

## Русский

### Главное

- Реализован встроенный Web-сервер и автономный режим `--server-only`, использующие те же камеры, пользователей, настройки и backend, что и desktop-приложение.
- Web-интерфейс повторяет основной desktop workflow, поддерживает русский и английский языки и адаптируется к desktop, планшетам и телефонам.
- Добавлены защищённый HTTP API v1, live-обновления через WebSocket и общий presentation layer, исключающий расхождение данных и форматирования между QML и Web.
- В настройках desktop появился отдельный раздел Web с адресами доступа, состоянием сервера, счётчиками клиентов и сессий, LAN bind, портами и параметрами безопасности.

### Монитор и камеры

- Доступны раскладки 1/4/9, назначение камер в ячейки, список устройств и компактные сворачиваемые контролы камеры для мобильных экранов.
- Реализованы поиск камер, ручное добавление, редактирование и удаление устройств непосредственно из браузера.
- Live preview использует WebRTC: H.264 передаётся с низкой задержкой без перекодирования, для H.265 предусмотрено ограниченное преобразование в H.264, а MJPEG используется как автоматический fallback.
- Добавлены запись, snapshot, mute/volume, fullscreen и PTZ с понятными состояниями выполнения и ошибок.
- Архив поддерживает фильтрацию, browser playback, HTTP Range и безопасное скачивание по opaque file ID без раскрытия локальных путей.

### Администрирование и диагностика

- В Web доступны безопасные настройки приложения, управление пользователями и правами, смена паролей, отзыв активных сессий и защита последнего администратора.
- Реализованы фильтрация и live tail логов, очистка журналов, диагностические метрики и скачивание диагностического bundle с redaction чувствительных данных.
- Добавлены экспорт и импорт browser-safe конфигурации без передачи паролей камер и локальных секретов.
- Camex, Majestic и OpenIPC Control Center получили capability checks, preview/diff и подтверждаемые safe-action сценарии; опасные операции остаются ограниченными серверной проверкой и аудитом.

### Безопасность

- Web-вход не меняет desktop-сессию; используются 256-bit opaque tokens, на диске/в памяти хранятся только SHA-256 digest сессий.
- Добавлены RBAC, sliding session TTL, отзыв сессий при изменении пользователя, CSRF/Origin checks, login rate limiting и security headers CSP/frame/no-sniff/no-referrer.
- API не сериализует пароли камер, hashes/salts пользователей, OAuth secrets, RTSP URL с credentials и произвольные filesystem paths.
- Mutation endpoints проверяют права, Origin/CSRF, входные данные, idempotency и создают audit events.

### Интерфейс и стабильность

- Окно поиска камер стало компактнее: больше места отдано результатам, а граница списка изменяется перетаскиванием мышью.
- Плитка «Поиск камер» в desktop sidebar больше не имеет постоянного синего выделения и использует общие hover/pressed/focus эффекты.
- `--server-only` использует offscreen Qt platform; Windows deployment включает необходимый `qoffscreen` plugin.
- Windows/Linux packaging дополнен GStreamer WebRTC/ICE/DTLS/SRTP runtime-компонентами.

### Проверка релиза

- Полная Release-сборка Qt 6.4 / MinGW 12.2.
- 27 C++ unit/contract tests, включая presentation, HTTP protocol и Web session store.
- QML smoke-тесты в обычном режиме и при `QT_SCALE_FACTOR=1.5`.
- Targeted QML lint и проверка каталога локализации.
- Автономный `--server-only` smoke через `GET /api/v1/server`.
- Всего: 30 из 30 локальных тестов проходят.
- GitHub Actions публикует Windows installer и Linux AppImage только после успешной сборки обеих платформ.

### Ограничения

- Сервер по умолчанию слушает только localhost; доступ из LAN включается явно и должен защищаться доверенной сетью, VPN или HTTPS reverse proxy.
- WebRTC использует host ICE candidates для localhost/LAN/VPN; встроенной настройки STUN/TURN пока нет.
- Общий SSH terminal намеренно не переносится в браузер; native window/tray/keychain функции остаются desktop-only.
- Destructive firmware/restore операции требуют backup, стабильного питания и ручной проверки на совместимой камере.

## English

### Highlights

- Added an embedded Web server and autonomous `--server-only` mode sharing the same cameras, users, settings and backend as the desktop application.
- The responsive RU/EN Web interface follows the desktop operator workflow across desktop, tablet and mobile viewports.
- Added a protected HTTP API v1, live WebSocket updates and a shared presentation layer so QML and Web expose consistent state and formatting.
- Desktop Settings now includes a Web page with access URLs, runtime status, client/session counters, LAN binding, ports and security controls.

### Monitor And Cameras

- Added 1/4/9 layouts, camera-to-cell assignment, the device sidebar and compact collapsible camera controls for mobile screens.
- Camera discovery, manual onboarding, editing and deletion are available from the browser.
- Live preview uses WebRTC with low-latency H.264 passthrough, bounded H.265-to-H.264 conversion and automatic MJPEG fallback.
- Added recording, snapshots, mute/volume, fullscreen and PTZ with visible busy and error feedback.
- Archive supports filtering, browser playback, HTTP Range and safe downloads through opaque file IDs without exposing local paths.

### Administration And Diagnostics

- Browser-safe settings, user and permission management, password changes, session revocation and last-administrator protection are available in Web.
- Added filtered logs, live tail, log clearing, diagnostic metrics and redacted diagnostic bundle downloads.
- Added browser-safe configuration export/import without camera passwords or local secrets.
- Camex, Majestic and OpenIPC Control Center use capability checks, preview/diff and confirmed safe-action workflows with server-side validation and audit events.

### Security

- Web authentication is isolated from the desktop session and uses 256-bit opaque tokens with SHA-256 session digests only.
- Added RBAC, sliding session TTL, revocation after user security changes, CSRF/Origin validation, login rate limiting and CSP/frame/no-sniff/no-referrer headers.
- The API never serializes camera passwords, user hashes/salts, OAuth secrets, credential-bearing RTSP URLs or arbitrary filesystem paths.
- Mutation endpoints enforce permissions, Origin/CSRF validation, input validation, idempotency and audit logging.

### UI And Reliability

- Compacted the camera discovery dialog, expanded result space and added a draggable result-pane divider.
- Removed the permanent blue primary state from the desktop Camera Search tile so it shares the standard hover/pressed/focus effects.
- `--server-only` uses the Qt offscreen platform; Windows deployments include the required `qoffscreen` plugin.
- Windows and Linux packaging now includes the GStreamer WebRTC/ICE/DTLS/SRTP runtime components.

### Release Validation

- Full Qt 6.4 / MinGW 12.2 Release build.
- 27 C++ unit and contract tests, including presentation, HTTP protocol and Web session storage.
- QML smoke tests at the default scale and `QT_SCALE_FACTOR=1.5`.
- Targeted QML lint and localization catalog validation.
- Autonomous `--server-only` smoke through `GET /api/v1/server`.
- All 30 local tests pass.
- GitHub Actions publishes the Windows installer and Linux AppImage only after both platform builds succeed.

### Known Limitations

- The server listens on localhost by default. LAN access must be explicitly enabled and protected by a trusted network, VPN or HTTPS reverse proxy.
- WebRTC currently uses host ICE candidates for localhost/LAN/VPN; built-in STUN/TURN configuration is not available yet.
- A general SSH terminal is intentionally excluded from Web; native window, tray and keychain integrations remain desktop-only.
- Destructive firmware and restore operations require a backup, stable power and manual validation on compatible camera hardware.
