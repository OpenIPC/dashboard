# OpenIPC Dashboard v0.2.5.2

Emergency hotfix after the removed `v0.2.5.1` release. This release focuses on startup stability, updater/TLS reliability, sidebar icon rendering on clean Windows machines, and camera onboarding templates requested by users.

## Русский

### Исправлено

- Исправлен критический сбой запуска приложения: QML больше не падает на `VideoPlayer is not a type`.
- Восстановлена явная runtime-регистрация C++ типов, используемых из QML: `VideoPlayer`, `AnalyticsModel`, `AnalyticsEngine`, `SshClient`, `RemoteFsModel`, `CamexController`, `SystemController`.
- Исправлена упаковка Windows-релиза для updater/TLS: workflow теперь добавляет совместимые OpenSSL 1.1 DLL и запускает TLS self-test перед публикацией.
- Updater теперь показывает понятную ошибку, если TLS/OpenSSL на машине пользователя недоступен.
- Updater больше не предлагает старые несовместимые релизы другой архитектурной ветки, например `2.9.0`, как обновление для текущей `0.x` ветки.
- Исправлено отображение иконок в Metro sidebar tiles на чистых Windows-системах: SVG-path иконки теперь рендерятся напрямую, без зависимости от системного шрифта.
- Убран некритичный binding-loop warning в окне release notes updater-а.

### Добавлено

- Добавлены RTSP-шаблоны камер XM/Xiongmai и XM/Sofia.
- Добавлены RTSP-шаблоны Reolink, TP-Link и Uniview.
- Добавлен C++ helper для Sofia password hash, чтобы формировать корректные XM/Sofia RTSP URL прямо из мастера добавления камеры.
- Добавлен unit-тест, который защищает updater от выбора несовместимой legacy release line.

### Проверено

- `cmake --build build_release --config Release --parallel`
- `ctest --test-dir build_release --output-on-failure` — 14/14 passed
- `git diff --check`
- `appOpenIPC-Dashboard.exe --self-test-tls` — `EXIT=0`
- Smoke-запуск из `build_release`: приложение остаётся запущенным, QML root создаётся успешно.

## English

### Fixed

- Fixed a critical startup crash: QML no longer fails with `VideoPlayer is not a type`.
- Restored explicit runtime registration for QML-facing C++ types: `VideoPlayer`, `AnalyticsModel`, `AnalyticsEngine`, `SshClient`, `RemoteFsModel`, `CamexController`, `SystemController`.
- Fixed Windows release packaging for updater/TLS: the workflow now bundles compatible OpenSSL 1.1 DLLs and runs a TLS self-test before publishing.
- The updater now reports a clear error when TLS/OpenSSL is unavailable on the user machine.
- The updater no longer offers old incompatible releases from another architecture line, such as `2.9.0`, as updates for the current `0.x` line.
- Fixed Metro sidebar tile icons on clean Windows systems: SVG path icons are now rendered directly without relying on a system font.
- Removed a non-critical binding-loop warning in the updater release notes dialog.

### Added

- Added XM/Xiongmai and XM/Sofia RTSP templates.
- Added Reolink, TP-Link and Uniview RTSP templates.
- Added a C++ helper for Sofia password hashing so XM/Sofia RTSP URLs can be generated correctly in the add-camera wizard.
- Added a unit test that protects the updater from selecting an incompatible legacy release line.

### Verified

- `cmake --build build_release --config Release --parallel`
- `ctest --test-dir build_release --output-on-failure` — 14/14 passed
- `git diff --check`
- `appOpenIPC-Dashboard.exe --self-test-tls` — `EXIT=0`
- Smoke launch from `build_release`: the application stays running and the QML root object is created successfully.
