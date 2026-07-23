# OpenIPC Dashboard v0.2.8

OpenIPC Dashboard v0.2.8 completes the P11 production-hardening cycle: safer upgrades, verifiable release artifacts, managed Web deployment profiles, a modular browser client and stronger desktop/runtime gates.

## Русский

### Production и обновление

- Добавлена compatibility matrix обновления с v0.2.6.1 и v0.2.7 с сохранением камер, групп, layouts, archive path, Web-настроек и credential references.
- Версионированное состояние и legacy user hashes покрыты migration-тестами; очистка пользовательского профиля после обновления не требуется.
- Новый server-only lifecycle smoke проверяет readiness, конфликт порта, остановку и повторный запуск на том же состоянии.
- Добавлена воспроизводимая форма runtime-отчёта и расширенная startup/diagnostic telemetry: версия, profile, TLS/WebRTC/WebSocket, uptime и time-to-ready.

### Secure Web deployment

- Появились явные deployment profiles: `localhost`, trusted `lan`, `vpn` и `reverse_proxy`.
- Reverse proxy требует HTTPS external base URL и точный список доверенных proxy IP; произвольные `Forwarded` / `X-Forwarded-*` headers не считаются доверенными.
- Добавлены `/api/v1/health/live` и `/api/v1/health/ready`, public/local/WebSocket URLs и понятная диагностика конфликта портов.
- Secure cookies автоматически обязательны за валидным HTTPS proxy.
- Для нового headless-host добавлен одноразовый `--initialize-admin <name>` с паролем из локального файла, а не из command line.
- Добавлен `OPENIPC_DATA_ROOT` для изолированного server-only профиля без пересечения users/state/logs с desktop-профилем.
- `--server-only` больше не останавливается после изменения безопасных Web-настроек, даже если desktop auto-start отключён.
- Добавлены пример systemd unit, Windows Scheduled Task helper, Nginx guidance, backup/restore boundaries и отдельный threat model.
- Администратор видит origin/peer, idle/absolute TTL активных сессий и задаёт причину их отзыва; raw session tokens не раскрываются.
- Browser import больше не может изменить listener, deployment profile или trusted proxy list собственного сервера.
- Public STUN/TURN/cloud relay остаются выключенными до отдельной проверки credentials, coturn interoperability и relay-only сценариев.

### Web UI и browser quality

- Большой `src/web/app.js` разделён на `core.js`, `monitor.js`, `devices.js`, `admin.js` и компактный coordinator.
- Добавлен static contract, запрещающий дублирование global functions, потерю Qt resources/routes и повторный рост одного модуля.
- Release browser gate проверяет Edge, Chromium, Firefox и WebKit: admin/viewer login, RBAC, CSRF rejection, settings update, archive negative path, keyboard focus return, accessible control names и отсутствие horizontal overflow.
- Desktop/mobile browser snapshots сохраняются как CI artifacts; WebRTC codec limitations явно документированы, MJPEG остаётся fallback.
- Health и Analytics parity описана честно: основные сценарии работают, desktop-only history/zone/file details остаются `Partial`.

### Desktop QML и Updater

- Исправлено переполнение окна Updater: progress, release notes и action buttons теперь всегда укладываются в доступную ширину.
- Camera Search и Updater добавлены в focused smoke/lint gates.
- QML smoke выполняется при 100%, 125%, 150% и 200% scaling.
- Устранены старые `qmllint` предупреждения кнопок Camera Search через явные component IDs.

### Release engineering

- Windows/Linux release builds используют контролируемый parallelism, Qt/GStreamer cache и warnings-as-errors.
- Внешний `linuxdeployqt` загружается с bounded retries, size validation и закреплённой SHA-256 суммой.
- До публикации запускаются полный CTest, package-level server smoke установленного Installer и извлечённого AppImage, а также browser matrix.
- К релизу прикладываются `SHA256SUMS.txt`, release metadata и CycloneDX 1.5 SBOM/third-party manifest.
- Release baseline: 29 unit/contract tests, 4 QML scale smoke, targeted lint, Web asset contract и server lifecycle — 36 CTest gates.

### Ограничения

- Plain HTTP подходит только для доверенной management LAN; для недоверенных сетей используйте VPN или HTTPS reverse proxy.
- WebKit/Firefox/OS могут отличаться набором WebRTC codecs; при отсутствии совместимого пути используется MJPEG.
- Реальные time-to-first-frame, reconnect storm и GPU/codec budgets подтверждаются на конкретных камерах и входят в sanitized field qualification.
- General SSH terminal, native dialogs, keychain UI и desktop window integrations намеренно остаются desktop-only.

## English

### Production and upgrades

- Added a v0.2.6.1/v0.2.7 upgrade matrix covering cameras, groups, layouts, archive paths, Web settings and credential references.
- Versioned state and legacy user hashes now have explicit migration tests; upgrades do not require deleting the user profile.
- A server-only lifecycle smoke verifies readiness, port-conflict rejection, shutdown and recovery on the same state.
- Added a reproducible runtime issue form and bounded startup/diagnostic telemetry for version, profile, TLS/WebRTC/WebSocket, uptime and time-to-ready.

### Secure Web deployment

- Added explicit `localhost`, trusted `lan`, `vpn` and `reverse_proxy` deployment profiles.
- Reverse proxy mode requires an HTTPS external base URL and exact trusted proxy IPs; arbitrary forwarded headers are never identity evidence.
- Added liveness/readiness probes, public/local/WebSocket URLs, secure-cookie enforcement and actionable bind-conflict diagnostics.
- New headless hosts can bootstrap one administrator from a restricted local password file with `--initialize-admin`; the password is never a command-line value.
- Added `OPENIPC_DATA_ROOT` for an isolated server-only profile that does not share users, state or logs with the desktop profile.
- `--server-only` now remains online after browser-safe settings updates even when desktop auto-start is disabled.
- Added systemd, Windows Scheduled Task and Nginx guidance plus backup/restore and threat-model documentation.
- Session administration now includes origin/peer, idle/absolute TTL and a revocation reason without exposing tokens.
- Browser imports cannot move or expose their own listener/deployment/trusted-proxy settings.
- Public STUN/TURN and cloud relay remain disabled pending a separate credential and interoperability qualification.

### Web UI and browser quality

- Split the former 87 KiB `app.js` into state/API, monitor, device, administration and coordinator modules.
- Added a static module/resource/route contract.
- The release browser gate covers Edge, Chromium, Firefox and WebKit with admin/viewer RBAC, CSRF rejection, settings/archive contracts, keyboard focus, accessible names and desktop/mobile overflow checks.
- CI retains desktop/mobile browser snapshots. Browser codec limitations and MJPEG fallback are documented.
- Health and Analytics retain explicit `Partial` labels for their remaining desktop-only detail workflows.

### Desktop QML and Updater

- Fixed Updater horizontal overflow for download progress, release notes and action buttons.
- Camera Search and Updater are now part of focused smoke/lint gates.
- QML smoke runs at 100%, 125%, 150% and 200% scaling.
- Removed legacy Camera Search button lint warnings with explicit component references.

### Release engineering

- Windows/Linux release jobs use bounded parallelism, dependency caches and warnings-as-errors.
- The mutable upstream `linuxdeployqt` asset is protected by retry, size and pinned SHA-256 validation.
- Full CTest, installed/extracted package smoke and the browser matrix run before publishing.
- Releases include SHA-256 checksums, release metadata and a CycloneDX 1.5 SBOM/third-party manifest.
- Baseline: 29 unit/contract tests plus four QML scale smokes, targeted lint, Web asset contract and server lifecycle — 36 CTest gates.

### Known limitations

- Plain HTTP is for a trusted management LAN only; use a VPN or HTTPS reverse proxy elsewhere.
- WebRTC codec availability varies by browser/OS; MJPEG remains the compatibility fallback.
- Real-camera time-to-first-frame, reconnect-storm and GPU/codec budgets remain field qualification metrics.
- General SSH terminal, native dialogs, keychain UI and desktop window integrations remain desktop-only.
