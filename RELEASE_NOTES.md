# OpenIPC Dashboard v0.2.6

OpenIPC Dashboard v0.2.6 is a large desktop-control release focused on OpenIPC/Majestic operations, Health Center v2, Discovery v2, analytics evidence, and a broad UI stability pass.

## Русский

### Главное

- Добавлен Health Center v2: профили проверок, история запусков, подробные probe-отчеты и рекомендации по проблемным камерам.
- Улучшен поиск и добавление камер: OpenIPC/ONVIF/RTSP discovery, выбор сетевого интерфейса, сохранение последней сессии, проверка credentials и массовое добавление найденных устройств.
- Расширен OpenIPC / Majestic Control Center: безопасные action gates, проверка firmware-архивов, checksum/manifest-инспекция, визуальный diff restore и более понятные вкладки status/network/time/update/tools/endpoints.
- Развита аналитика: управление AI-модулями, проверка артефактов, правила, события, evidence-снимки/клипы, архив и экспорт JSON/CSV.
- Проведена финальная полировка интерфейса: исправлены перекрытия, адаптивность окон, карточки камер, sidebar, вкладки, настройки и smoke-проверки QML.

### Добавлено

- `CameraHealthController` и `CameraHealthPolicy` для фоновых проверок камер с ограниченным параллелизмом.
- Health-профили: быстрый, глубокий, OpenIPC/Majestic и RTSP-only.
- История Health Center до 30 завершенных запусков с сохранением в SQLite.
- Детальное окно health-доказательств: endpoint, длительность probe, версии firmware/Majestic, порты и последние сообщения.
- Расширенные карточки камер в sidebar: IP, RTSP-порт, температура, online/offline/attention-статус.
- Сворачивание верхнего блока sidebar с действиями и live-preview, чтобы оставить больше места списку камер.
- QML smoke-режим `--smoke-qml`, `SmokeHarness.qml`, `qml_smoke` и `qml_smoke_scaled` для проверки UI без реальных камер.
- Новые Metro-компоненты для checkbox/menu/slider/window buttons и переиспользуемые UI-блоки настроек.
- Safe Actions panel для OpenIPC Control Center с явными причинами блокировки опасных операций.
- Firmware manifest/checksum panel: SHA-256, sidecar checksum-файлы, signature-файлы и блокировка update при mismatch.
- Majestic restore diff panel с группировкой изменений: live, reload-required, critical и secret-sensitive.
- Analytics Modules Center v2: инвентаризация модулей, статус модели, размер, backend, кадры, детекции, задержка и очистка временных хвостов.
- Analytics Overview v2 с summary по evidence, рекомендациями и статусом event store/modules/rules/assignments.
- Evidence-хранилище аналитики: сохранение снимков и клипов, локальный архив по датам, открытие папок снимков/клипов.
- Экспорт ленты событий аналитики в JSON и CSV с учетом текущих фильтров.

### Улучшено

- `SettingsDialog` разбит на отдельные страницы update/about/OAuth/streaming/analytics/evidence/footer.
- Streaming-настройки вынесены в отдельную адаптивную страницу.
- `GridCell.qml`, `AppUpdateDialog.qml` и часть Control Center разнесены на меньшие QML-компоненты.
- Окна Settings, Health Center, Analytics и Control Center стали устойчивее к разным размерам и масштабу Windows.
- Layout toolbar отделен от window controls и больше не уезжает при большом числе раскладок.
- Вкладки Analytics и OpenIPC Control Center выровнены и больше не прячут кнопки/поля.
- Окно Health Center получило корректную шапку, рамку, отступы и список камер без перекрытий.
- Control Center больше не перекрывает поля нижней информационной плашкой.
- Поиск камер показывает больше диагностических причин: уже добавлена, проверка credentials, discovery evidence, profile и network confidence.
- Архив аналитики показывает сохраненные снимки, даты, фильтры, preview и быстрые действия.
- Качество evidence-снимков улучшено за счет более аккуратного сохранения и отображения кадров.
- README и документация обновлены под OpenIPC-native desktop control center.

### Исправлено

- Исправлен crash при открытии окна аналитики на камере с включенными AI-модулями.
- Исправлено отсутствие сохранения evidence-снимков и пустой архив аналитики.
- Исправлено окно настроек, где пункты могли не отображаться.
- Исправлены перекрытия в Health Center: шапка больше не закрывает кнопки, верхняя рамка отображается корректно.
- Исправлены перекрытия и наезды элементов в OpenIPC Control Center.
- Исправлено отображение IP и температуры в карточке камеры sidebar.
- Исправлены смещения кнопок layout toolbar и проблемы с обрезанием вкладок.
- Исправлены layout-проблемы в Analytics, Settings, Camera Search, User Management и диалогах добавления/редактирования.
- Улучшены disabled/loading/error-состояния кнопок и полей, чтобы интерфейс не выглядел сломанным во время операций.

### Безопасность и ограничения

- Опасные OpenIPC-действия остаются под явными gates и не выполняются автоматически.
- Автоматический full restore OpenIPC backup и firmware rollback не включены в этот релиз.
- P6 Web/server mode остается в backlog.
- Интеграция `ipctool` намеренно не входит в v0.2.6.

### Проверка релиза

- C++ unit tests для health policy/controller, camera model, discovery, onboarding, Majestic/OpenIPC clients и analytics artifact verifier.
- QML smoke tests: обычный запуск и масштабирование `QT_SCALE_FACTOR=1.5`.
- Ручная проверка на реальной OpenIPC-камере: discovery, live preview, sidebar, Health Center, Control Center, AI-модули, события, evidence-снимки и архив.
- Рекомендуемые команды перед публикацией тега:

```bash
cmake -S . -B build_release -DBUILD_TESTING=ON
cmake --build build_release --target appOpenIPC-Dashboard -j 2
ctest --test-dir build_release --output-on-failure
git diff --check
```

## English

### Highlights

- Added Health Center v2 with scan profiles, persisted run history, detailed probe reports and actionable camera recommendations.
- Improved camera onboarding and discovery with OpenIPC/ONVIF/RTSP probing, network-interface selection, persisted discovery sessions, credential validation and bulk add.
- Expanded the OpenIPC / Majestic Control Center with safe action gates, firmware archive inspection, checksum/manifest validation, restore diff preview and clearer status/network/time/update/tools/endpoints pages.
- Evolved analytics into a working modules/events/evidence center with model management, artifact checks, rules, event feed, snapshots/clips, archive browsing and JSON/CSV export.
- Completed a broad UI polish pass across settings, health, analytics, sidebar, camera cards, tabs, dialogs and QML smoke coverage.

### Added

- `CameraHealthController` and `CameraHealthPolicy` for bounded background camera checks.
- Health profiles: quick, deep, OpenIPC/Majestic and RTSP-only.
- Health Center history for up to 30 completed runs stored in SQLite.
- Detailed health evidence view with endpoint status, probe duration, firmware/Majestic versions, ports and recent messages.
- Rich sidebar camera cards with IP address, RTSP port, temperature and online/offline/attention state.
- Collapsible sidebar action/live-preview block so the device list can use the full sidebar height.
- QML smoke mode: `--smoke-qml`, `SmokeHarness.qml`, `qml_smoke` and `qml_smoke_scaled`.
- Reusable Metro controls for checkboxes, menus, sliders and window buttons.
- OpenIPC Safe Actions panel with explicit gates and blocking reasons for risky operations.
- Firmware manifest/checksum panel with SHA-256, sidecar checksum files, signature detection and update blocking on mismatch.
- Majestic restore diff panel with live, reload-required, critical and secret-sensitive change grouping.
- Analytics Modules Center v2 with module inventory, model status, size, backend, frames, detections, latency and safe cleanup of temporary artifacts.
- Analytics Overview v2 with evidence summary, recommendations and event store/modules/rules/assignments status.
- Analytics evidence storage for snapshots and clips, date-based archive browsing and folder shortcuts.
- JSON/CSV export for the filtered analytics event feed.

### Improved

- Split `SettingsDialog` into focused update/about/OAuth/streaming/analytics/evidence/footer components.
- Moved streaming settings into a responsive dedicated page.
- Split `GridCell.qml`, `AppUpdateDialog.qml` and several Control Center surfaces into smaller QML components.
- Made Settings, Health Center, Analytics and Control Center more robust across different window sizes and Windows scaling factors.
- Separated the layout toolbar from window controls and added overflow behavior for many layouts.
- Cleaned up Analytics and Control Center tabs so controls and fields no longer hide behind headers.
- Health Center now has a stable header, border, spacing and camera list layout.
- Removed the Control Center footer strip that could overlap fields and action buttons.
- Camera discovery now reports clearer reasons: already added, credential validation, discovery evidence, profile and network confidence.
- Analytics archive now shows saved snapshots, dates, filters, previews and quick actions.
- Improved evidence snapshot handling and preview quality.
- Updated README and docs around the OpenIPC-native desktop control center direction.

### Fixed

- Fixed a crash when opening Analytics while AI modules were enabled on a camera.
- Fixed missing evidence snapshot persistence and an empty analytics archive.
- Fixed the Settings dialog content disappearing in the modal window.
- Fixed Health Center header overlap and missing top border.
- Fixed overlapping controls and fields in OpenIPC Control Center.
- Fixed IP address and temperature display in the sidebar camera card.
- Fixed layout toolbar button drift and clipped tabs.
- Fixed layout issues in Analytics, Settings, Camera Search, User Management and add/edit dialogs.
- Improved disabled/loading/error states for buttons and fields during long-running operations.

### Safety And Scope

- Risky OpenIPC actions remain gated and are not executed implicitly.
- Automatic full OpenIPC backup restore and firmware rollback are not enabled in this release.
- P6 Web/server mode remains in the backlog.
- `ipctool` integration is intentionally not included in v0.2.6.

### Release Validation

- C++ unit tests cover health policy/controller, camera model, discovery, onboarding, Majestic/OpenIPC clients and analytics artifact verification.
- QML smoke tests cover normal startup and `QT_SCALE_FACTOR=1.5`.
- Real OpenIPC camera smoke testing covered discovery, live preview, sidebar, Health Center, Control Center, AI modules, events, evidence snapshots and archive browsing.
- Recommended commands before publishing the tag:

```bash
cmake -S . -B build_release -DBUILD_TESTING=ON
cmake --build build_release --target appOpenIPC-Dashboard -j 2
ctest --test-dir build_release --output-on-failure
git diff --check
```
