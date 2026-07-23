# OpenIPC Dashboard Roadmap

Последнее обновление: 2026-07-23.

Текущий стабильный релиз: `v0.2.8`.

Текущий фокус разработки: 🟡 `P12 Sites / Fleet Management / safe group operations`.

## Обозначения

- ✅ Сделано и вошло в релиз.
- 🟡 В работе / ближайший активный фокус.
- 🔜 Следующая очередь.
- 🧊 Backlog / будущая возможность.
- ⛔ Не делаем без дополнительного решения или подтверждения.

## Краткое состояние проекта

OpenIPC Dashboard после `v0.2.7` объединяет desktop VMS, OpenIPC/Majestic control center и защищённый Web companion на одном backend:

- поиск OpenIPC/ONVIF/RTSP камер в сети;
- live preview с HD/SD режимами;
- единый status-layer камер;
- Majestic schema-driven настройки;
- OpenIPC firmware status/network/time/logs/backup/reboot/update;
- firmware upgrade через `/ws/upgrade`;
- live firmware logs;
- in-app updater приложения через GitHub Releases;
- более компактный Dashboard UI и sidebar;
- WebRTC/MJPEG Web monitor, запись, snapshots, PTZ, архив и responsive RU/EN интерфейс;
- browser-safe Settings/Users/Logs/Diagnostics и device safe-action workflows;
- автономный `--server-only` режим с RBAC, session security и API v1;
- релизные Windows/Linux сборки через GitHub Actions.

## Актуализация 2026-07-23 — после `v0.2.8`

Этапы P2–P11, полный цикл P6 Web/Desktop parity и production hardening завершены. Следующая
задача проекта — перенести уже проверенные security/release/contracts границы на управление
десятками камер и несколькими площадками.

Ближайшие продуктовые принципы:

- продолжать подтверждать стабильность `v0.2.8` на реальных desktop/Web установках;
- сокращать стоимость релиза: воспроизводимые пакеты, checksums, package smoke и быстрый CI;
- разрешать удалённый Web-доступ только через явно спроектированные TLS/VPN/proxy границы;
- не дублировать domain logic между QML, Web и будущими интеграциями;
- массовые операции над камерами начинать с inventory, preview/diff и dry-run;
- строить Incident Center поверх существующих Health/Analytics/Archive/Audit данных;
- повышать число потоков только вместе с измеримым resource budget и benchmark matrix;
- расширять публичный API только как versioned contract с scoped credentials и audit.

Порядок новых продуктовых волн:

1. ✅ P11 — production hardening, release engineering, Web deployment и UI/runtime debt.
2. 🟡 P12 — Sites / Fleet Management и безопасные групповые операции.
3. 🔜 P13 — Incident Center, уведомления и операторские workflows.
4. 🧊 P14 — media scale, adaptive quality и multi-monitor.
5. 🧊 P15 — versioned integration ecosystem и внешние automation adapters.

## Актуализация 2026-07-04

Свежий обзор локального проекта и `OpenIPC/viewer` показывает, что Dashboard должен сохранять позицию не просто видеопросмотрщика, а OpenIPC-native desktop control center.

Что у нас сильнее и должно оставаться продуктовым ядром:

- глубокое OpenIPC/Majestic управление: firmware status/network/time/logs/backup/reboot/update и `/ws/upgrade`;
- schema-safe Majestic editor с diff, redaction, reload и rollback-health;
- discovery с OpenIPC mDNS, ONVIF, Majestic/WebUI fingerprinting, RTSP probe и Dahua SDK merge;
- строгие stream/reconnect/preview-budget политики с unit-тестами;
- in-app updater, Windows installer и Linux AppImage уже в текущей ветке релизов.

Что берём из подхода `OpenIPC/viewer` как инженерный ориентир:

- более явные границы между domain/services/UI вместо роста одного большого контроллера;
- small View/ViewModel style для крупных экранов, чтобы новые функции не раздували legacy QML;
- smoke/integration проверки пользовательских сценариев, а не только backend-политик;
- аккуратную публичную документацию первого запуска, диагностики и troubleshooting.

Главный ближайший риск: backend уже достаточно крепкий, но QML runtime пока проверяется слабее, чем C++ policy-слой. Поэтому P2.2 выполняется до Health Center v2 и Discovery v2.

## ✅ Release `v0.2.6`

Статус: опубликован 2026-07-12 как stable release.

В релиз вошли накопленные этапы P3/P4/P5/P7 и стабилизация сборок:

- Health Center v2 с профилями проверки, историей запусков и рекомендациями.
- Discovery/onboarding v2 с сохранением результатов поиска, профилями добавления и проверкой credentials.
- Majestic/OpenIPC advanced v3: firmware/control center, safer update flow, capabilities/action gates, manifest/checksum inspection.
- Analytics/Modules evolution: модули, evidence-снимки/клипы, архив, event feed и экспорт.
- Dashboard polish: sidebar, карточки камер, сворачивание блока действий, исправления модальных окон.
- Release pipeline: Windows installer и Linux AppImage успешно собираются и публикуются через GitHub Actions.

Открытый долг после релиза:

- приложение должно иметь работающий application log viewer, а не только firmware/live logs камеры;
- крупные QML/C++ файлы нужно дробить дальше без изменения поведения;
- CI workflow на `main` требует отдельной стабилизации после релизного workflow.

## ✅ Hotfix `v0.2.5.2`

- Исправлено исчезновение иконок в Metro sidebar на части пользовательских сборок.
- Sidebar-иконки больше не зависят от `QtQuick.Shapes` / `qmlshapesplugin`.
- Добавлен fallback через встроенный `MaterialIcons-Regular.ttf` из `qrc`.
- Старые SVG-path параметры оставлены для совместимости, но sidebar tiles теперь используют стабильные Material Icons ligature names.
- Проверено:
  - `cmake --build build_release --config Release --parallel`;
  - `ctest --test-dir build_release --output-on-failure`;
  - `git diff --check`.

## ✅ Закрыто в `v0.2.5`

### Release / CI

- Опубликован stable release `v0.2.5`.
- GitHub Release создаётся по тегу `v*`.
- `RELEASE_NOTES.md` используется как тело GitHub Release.
- Windows installer и Linux AppImage собираются через GitHub Actions.
- В release workflow добавлен Qt WebSockets.
- Pre-release теги вида `v*-pre*`, `v*-rc*`, `v*-beta*`, `v*-alpha*` помечаются как GitHub pre-release.
- Финальный `v0.2.5` опубликован как stable release.

### OpenIPC Control Center / Firmware

- Добавлен единый OpenIPC Control Center внутри окна Majestic/Firmware.
- Реализованы firmware read/write операции:
  - status;
  - network load/save/reset/Wi-Fi scan;
  - time load/save/NTP sync/set from PC;
  - logs;
  - firmware backup;
  - reboot;
  - update info;
  - firmware archive upload.
- Подготовлен и проверен firmware upgrade через `/ws/upgrade`.
- На реальной OpenIPC-камере подтверждён native firmware update из приложения с подробным логом процесса.
- Добавлены фазы upgrade/recovery:
  - flashing/rebooting;
  - probing;
  - waiting;
  - validating;
  - online;
  - degraded;
  - failed.
- После возврата камеры обновляются status/network/time/update-info/metrics.
- После возврата камеры запускаются Majestic API и RTSP main/sub probes.

### Firmware upgrade safety

- Uploaded archive проверяется до загрузки:
  - файл существует;
  - расширение `.tgz/.tar.gz/.gz`;
  - размер в допустимом диапазоне `0..128 MB`.
- Checklist показывает SoC/Flash/firmware variant.
- Имя uploaded archive проверяется на очевидные NOR/NAND и lite/ultimate mismatch.
- Если update page отдаёт checksum/signature, они попадают в safety checklist.
- Start update блокируется без подтверждения стабильного питания/сети.
- Dangerous options `reset` и `force` требуют отдельного подтверждения.

### Majestic API

- Настройки Majestic строятся по schema конкретной камеры.
- Убрана общая вкладка “Все настройки”; настройки сгруппированы по понятным разделам:
  - изображение;
  - видео и аудио;
  - события;
  - запись;
  - сеть и интеграции;
  - система.
- Добавлены локализованные RU/EN названия и подсказки для известных Majestic-параметров.
- Apply работает через schema-safe diff.
- Исправлена ошибка отправки `null` в config patch.
- Критичные настройки помечаются как требующие reload pipeline.
- После apply поддержан штатный reload pipeline.
- Safe rollback v2:
  - rollback snapshot перед критичным apply;
  - health-watch после apply/reload;
  - rollback banner, если API или поток не восстановились;
  - auto-rollback только при явном разрешении пользователя и доступном API.

### Firmware backup / restore

- Добавлена отдельная карточка `OpenIPC backup / restore` во вкладке Tools.
- Majestic JSON backup визуально отделён от полного OpenIPC firmware/overlay backup.
- Restore полного backup не угадывается через непроверенный endpoint.
- Для full restore приложение открывает штатный WebUI restore после жёсткого подтверждения.

### Live firmware logs

- `/ws/logs` работает через WebSocket в release-сборках.
- Если Qt WebSockets недоступен, используется polling fallback.
- Добавлены:
  - start/stop;
  - pause/resume;
  - clear;
  - source filters `all/majestic/kernel`;
  - severity highlighting для error/warn/majestic/kernel;
  - экспорт логов в `.log/.txt`;
  - настройка OpenIPC syslog ring-buffer.

### Application updater

- Добавлен GitHub Releases update-checker.
- Существующая кнопка “Проверить обновления” в настройках подключена к реальному checker.
- При обнаружении новой версии показывается модальное окно с release notes.
- Добавлены действия:
  - открыть релиз;
  - пропустить версию;
  - напомнить позже;
  - скачать и установить.
- Добавлена загрузка обновления прямо из приложения:
  - progress bar;
  - отмена;
  - выбор совместимого asset под текущую платформу.
- Updater больше не цепляет старую несовместимую ветку релизов `2.9.0`.
- Windows:
  - скачивается `OpenIPC-Dashboard-Installer.exe`;
  - запускается installer handoff;
  - временный установщик удаляется после завершения.
- Linux AppImage:
  - скачивается `OpenIPC-Dashboard-Linux.AppImage`;
  - после выхода приложения выполняется замена текущего AppImage;
  - временный файл удаляется;
  - приложение перезапускается.

### Dashboard, Grid и sidebar UI

- Крупные части `DashboardView.qml` вынесены в отдельные QML-компоненты.
- Редактор раскладок вынесен в `LayoutEditorDialog.qml`.
- Исправлены проблемы Grid после дробления Dashboard.
- Исправлено мигание камеры в списке устройств при hover по action-кнопкам.
- Подпись видеопотока приведена к единому виду: codec, resolution, bitrate, FPS.
- Sidebar переработан в более строгий Viewer-like / Metro-like стиль:
  - компактная сетка действий;
  - ровные иконки и подписи;
  - меньше визуального шума;
  - layout actions встроены в toolbar раскладок.
- Строка устройства вынесена в `DeviceListItem.qml`.
- Заголовки групп вынесены в `SidebarSectionHeader.qml`.
- Empty state Dashboard получил короткий сценарий быстрого старта.
- Новые UI-строки локализованы RU/EN.

### Поиск камер

- Реализован поиск OpenIPC-камер в сети.
- Используются:
  - OpenIPC/mDNS;
  - WS-Discovery/ONVIF;
  - RTSP probe;
  - HTTP/Majestic probe.
- Добавлены быстрый и глубокий сценарии поиска.
- Progress bar показывает процент и этап поиска.
- После завершения поиска progress bar корректно скрывается.

### Live preview и потоки

- Исправлена работа SD/substream для OpenIPC/Majestic.
- HD/fullscreen и SD/preview разведены корректнее.
- Добавлена индикация live-preview budget: active, pause, limit.
- Статусы preview и статус камеры синхронизированы через общий status-layer.

### Единый статус камер

- Effective status вынесен в общий C++ слой.
- Dashboard, ячейки, Sidebar и Health Center используют единые правила online/offline/attention.
- Offline/ошибки потока имеют приоритет над старым optimistic online из списка камер.
- Добавлены unit-тесты для online/offline/attention/stream/auth сценариев.

### Health Center v1

- Добавлены фильтры:
  - все;
  - с проблемами;
  - offline;
  - online;
  - в сетке;
  - не в сетке;
  - auth;
  - stream.
- Добавлена массовая перепроверка камер.
- Добавлена история проверок текущей сессии.
- Добавлен экспорт диагностического отчёта.
- Health Center использует общий status-layer.

### Linux

- Исправлен fallback отображения RAM:
  - `/proc/self/status` / `VmRSS`;
  - `/proc/self/statm`;
  - `getrusage(RUSAGE_SELF)`.
- Закрыт сценарий, когда CPU отображался, а RAM оставалась `0 MB`.
- Исправлена Linux CI-зависимость для GStreamer dev packages.

## ✅ P2 — QML debt cleanup и UI foundation

Цель P2: снизить технический долг в QML, закрепить правило маленьких компонентов и подготовить стабильную базу для следующих крупных функций.

### P2.1 — legacy `qmllint` cleanup

Статус: ✅ закрыто как baseline + safe cleanup.

Сделано:

- Снят актуальный полный legacy `qmllint` baseline.
- Baseline зафиксирован в [`docs/QML_LINT_BASELINE.md`](QML_LINT_BASELINE.md).
- Полный `qmllint` уменьшен примерно с `1322` до `1184` предупреждений.
- C++ типы, используемые из QML, переведены на Qt QML type registration macros:
  - `VideoPlayer`;
  - `RemoteFsModel`;
  - `AnalyticsModel`;
  - `AnalyticsEngine`;
  - `SshClient`;
  - `CamexController`.
- Для сгенерированной QML-регистрации добавлены include paths:
  - `src/backend`;
  - `src/backend/analytics`;
  - `src/backend/gst`.
- Из `main.cpp` убрана ручная регистрация этих QML-типов, singleton `SystemController` оставлен.
- Исправлены безопасные предупреждения в:
  - `Main.qml`;
  - `SettingsDialog.qml`;
  - `FileManagerDialog.qml`;
  - `StyledScrollBar.qml`.
- Проверки:
  - `cmake --build build_release --config Release --parallel` — ✅;
  - `ctest --test-dir build_release --output-on-failure` — ✅ `14/14`;
  - `appOpenIPC-Dashboard_qmllint` — ожидаемо ❌ на задокументированном legacy baseline.

Осталось в дальнейших P2.x:

- Чистить legacy-предупреждения по крупным файлам малыми безопасными порциями:
  - `GridCell.qml`;
  - `ArchiveView.qml`;
  - `MajesticControlDialog.qml`;
  - `analytics/ImageViewerWindow.qml`;
  - `SettingsDialog.qml`;
  - `FileManagerDialog.qml`.
- Новые QML-компоненты не должны добавлять предупреждения сверх baseline.

### P2.2 — QML smoke-проверки

Статус: ✅ завершено.

Реализовано:

- добавлен CLI-режим `--smoke-qml`;
- добавлен `SmokeHarness.qml`, который создаёт основные QML поверхности без реальной камеры;
- добавлен CTest `qml_smoke`;
- smoke отключает реальные camera health probes через `OPENIPC_SMOKE_QML=1`, чтобы не трогать LAN при UI-проверке;
- CI запускает `qml_smoke` отдельным шагом после unit-тестов, чтобы runtime-ошибки QML были видны отдельно.

Smoke-run покрывает:

- Dashboard;
- Grid Cell без реального RTSP-потока;
- компактный Grid Cell 160×90 для базовой проверки малых плиток;
- Settings;
- Camera Search;
- Majestic/OpenIPC Control Center;
- Health Center.

Готовность к закрытию:

- Есть повторяемая команда smoke-проверки.
- Команда задокументирована.
- Smoke-проверка не требует реальной камеры для базового запуска UI.

### P2.3 — дальнейшая декомпозиция крупных QML/controller файлов

Статус: ✅ закрыто для текущего baseline.

Сделано:

- из `GridCell.qml` вынесен `StreamStatsBadge.qml`, общий badge/formatter статистики preview/fullscreen потоков.
- из `GridCell.qml` вынесены `RecordingPulseIndicator.qml` и `StreamQualityBadge.qml` для дальнейшей разгрузки stream-cell UI.
- из `GridCell.qml` вынесен `GridCellControlsOverlay.qml`: верхняя панель live/audio/record/snapshot/analytics теперь живёт отдельным UI-компонентом.
- `AppUpdateDialog.qml` превращён в тонкую window-оболочку; вся визуальная часть и action bar вынесены в `AppUpdateContent.qml`.
- из `SettingsDialog.qml` вынесены `SettingsUpdatePanel.qml`, `SettingsAboutPage.qml`, `SettingsOAuthDialog.qml`, `SettingsFooterBar.qml`, `SettingsSaveNotification.qml`, `SettingsAnalyticsPerformancePanel.qml`, `SettingsEvidencePanel.qml` и общий `SettingsSpinBox.qml`.
- повторяющиеся confirmation-dialogs в `MajesticControlDialog.qml` заменены общим `MajesticConfirmDialog.qml`.

Итог:

- самые рискованные раздутые UI-области разделены на компоненты без изменения backend-контрактов;
- `AppUpdateDialog.qml` теперь отвечает за состояние update-flow, а не за всю верстку;
- `SettingsDialog.qml` стал меньше примерно на четверть и получил отдельные панели для update/about/OAuth/footer/analytics evidence;
- `MajesticControlDialog.qml` оставлен главным orchestration-controller, но однотипный нижний confirm-layer больше не дублируется.

Правило:

- Новый функционал не добавляем в уже раздутые QML-файлы, если он требует отдельного состояния, нескольких UI-блоков или приближается к 200-300 строкам.
- Сразу создаём отдельный QML-компонент и, если нужно, отдельный C++/QML controller/model.

Что не тащим в P2.3:

- глубокий распил `MajesticControlDialog.qml` по API-domain controllers: это отдельный архитектурный шаг, потому что файл держит таймеры, request ownership, rollback/watchdog и firmware-update state machine;
- перенос старых streaming-настроек из `SettingsDialog.qml` в отдельную страницу: безопаснее делать вместе с P2.4-polish, чтобы сразу проверить layout и локализацию;
- крупные старые панели Dashboard/Analytics — только если начнём их активно расширять.

### P2.4 — UI polish после sidebar wave

Статус: ✅ закрыто 2026-07-09.

Что сделано:

- Sidebar получил адаптивную ширину `264-320 px`; collapsed/expanded состояния и минимальный Dashboard `960x540` покрыты smoke.
- Ручки открытия/закрытия sidebar, layout-toolbar, вкладки и основные controls получили keyboard focus и видимое focus-состояние.
- Layout-toolbar отделён от window controls и получил горизонтальный overflow для большого числа раскладок.
- `SettingsDialog` получил безопасный минимум `560x480`, гибкие вкладки, перенос длинных RU/EN строк и `Ctrl+S`/`Esc`.
- Streaming/video-настройки вынесены из `SettingsDialog.qml` в адаптивный `SettingsStreamingPage.qml`.
- Analytics, evidence и update-панели перестраиваются на узкой ширине без переполнения.
- Tooltips добавлены для icon-only, disabled и оконных controls; состояния disabled/loading/error визуально разведены.
- Smoke-матрица проверяет Dashboard, Settings, streaming-page и layout-toolbar на обычных и минимальных размерах.
- Добавлен `qml_smoke_scaled` с `QT_SCALE_FACTOR=1.5`; он запускается в Windows/Linux CI.

## ✅ P3 — Health Center v2

Цель: превратить Health Center из диагностического окна в полноценный инструмент обслуживания камер.

Статус: ✅ закрыто 2026-07-09.

Реализовано:

- Отдельный `CameraHealthController` выполняет проверки с ограниченным параллелизмом, сообщает прогресс и не блокирует UI.
- История до 30 завершённых запусков сохраняется в транзакционном `state.sqlite3` и восстанавливается при следующем запуске.
- Health-профили:
  - быстрый — RTSP main и snapshot;
  - глубокий — полный набор RTSP/OpenIPC/Majestic;
  - OpenIPC/Majestic — WebUI прошивки и API;
  - RTSP-only — main/sub endpoints.
- Глубокие probes:
  - Majestic config/schema;
  - firmware status;
  - `/metrics`;
  - `/ws/logs` readiness;
  - RTSP main/sub;
  - snapshot/JPEG endpoint.
- Рекомендации различают:
  - неверные credentials;
  - недоступен RTSP;
  - firmware/WebUI отвечает, но stream не идёт;
  - камера online, но не в раскладке;
  - firmware WebUI доступен, но Majestic недоступен.
- Расширенный отчёт включает:
  - status;
  - каждый probe и его длительность;
  - последние логи;
  - версии firmware/Majestic;
  - IP, HTTP/RTSP-порты и stream URL без credentials.
- UI получил selector профиля, живой прогресс, постоянную историю, выбор запуска для экспорта и отдельное окно probe-доказательств.
- Добавлены unit-тесты policy/controller, локальные RTSP/HTTP test servers и QML smoke при обычном и `150%` масштабе.

Готовность к закрытию:

- ✅ Health Center сохраняет историю.
- ✅ Есть 4 профиля проверки.
- ✅ Отчёт и рекомендации показывают, какой endpoint сломан и что проверять дальше.

## ✅ P4 — Camera onboarding / Discovery v2

Цель: сделать добавление камер ещё проще и надёжнее.

Реализовано:

- Улучшенное обнаружение дублей по IP, serial/fingerprint/evidence и признаку уже добавленной камеры.
- Сохранение последнего результата поиска и восстановление discovery-сессии после перезапуска.
- Массовое добавление найденных камер через backend без ручной сборки URL в QML.
- Проверка credentials перед добавлением с прогрессом и понятным статусом по каждой найденной камере.
- Выбор onboarding profile при добавлении:
  - OpenIPC/Majestic;
  - ONVIF;
  - RTSP manual.
- Более понятные причины “почему камера найдена, но не добавлена”: validation status, discovery evidence, отметка “уже в списке”.
- Улучшение работы с несколькими сетевыми интерфейсами: интерфейс показывает IP/prefix, а последняя discovery-сессия хранит выбранный адаптер и профиль поиска.

Проверено:

- `cmake --build build_release --target appOpenIPC-Dashboard -j 2`;
- `ctest --test-dir build_release -R "camera_model_tests|network_discovery_service_tests|camera_onboarding_parser_tests|qml_smoke|state_store_tests" --output-on-failure`;
- `ctest --test-dir build_release --output-on-failure`.

## ✅ P5 — Majestic/OpenIPC advanced v3

Цель: приблизить управление к уровню WebUI камеры, но сохранить безопасность desktop-приложения.

Реализовано:

- Safe capabilities/actions matrix для OpenIPC Control Center: diff apply, pipeline reload, live ISP, network write, firmware update и restore теперь имеют явные gates и причины блокировки.
- Расширенный endpoints/capabilities view: endpoints получили статусы, риск-классы, подтверждение probe/capability и отдельный action-gates блок.
- Firmware archive manifest/checksum inspection:
  - расчёт локального SHA-256 перед upload;
  - проверка опубликованного checksum из update page;
  - поддержка sidecar `.sha256`, `.sha256sum`, `.sha256.txt`;
  - обнаружение signature-файлов `.sig`, `.asc`, `.minisig`;
  - checksum mismatch блокирует дальнейший upload/update.
- Firmware update checklist использует manifest/checksum state, а не только наличие текста checksum на странице камеры.
- Majestic restore получил visual diff summary: всего изменений, live, reload-required, critical и secret-sensitive параметры.
- Начат дальнейший split Control Center:
  - `OpenIpcSafeActionsPanel.qml`;
  - `OpenIpcFirmwareManifestPanel.qml`;
  - `MajesticRestoreDiffPanel.qml`.

Проверено:

- `cmake -S . -B build_release`;
- `cmake --build build_release --target appOpenIPC-Dashboard -j 2`;
- `ctest --test-dir build_release -R "qml_smoke|majestic_client_tests|openipc_firmware_client_tests|camera_model_tests" --output-on-failure`.

## ✅ P6 — Web version / Server mode

Цель: дать Dashboard безопасный браузерный companion и автономный server-only режим без дублирования desktop-состояния и секретов.

Статус: Web/Desktop parity выпущен в `v0.2.7` 2026-07-20. P6.1–P6.16 закрыты;
дальнейшее расширение Web API и поддержка STUN/TURN ведутся как отдельные этапы.

Контрольная точка P6:

- desktop и автономный `--server-only` используют один backend, настройки, камеры и пользователей;
- web-аутентификация, RBAC, HTTP API v1 и live-обновления через WebSocket работают;
- интерфейс монитора повторяет основной desktop workflow: раскладки 1/4/9, активная ячейка,
  список камер и рабочие окна Health, Analytics и Archive;
- WebRTC проверен на реальной OpenIPC-камере: H.264 passthrough обеспечивает нормальный
  исходный FPS и low-latency playback, MJPEG остается автоматическим fallback;
- Qt WebSockets и полный GStreamer WebRTC/ICE/DTLS/SRTP runtime включены в Windows/Linux packaging;
- полная MinGW release-сборка и 30 автоматических тестов проходят успешно.

Зафиксированные границы текущей версии:

- одновременно может работать только один экземпляр сервера на выбранных HTTP/WebSocket портах;
- используются host ICE candidates для localhost/LAN/VPN; настройка STUN/TURN пока отсутствует;
- общий SSH terminal и native window/tray/keychain интеграции намеренно остаются desktop-only;
- destructive firmware/restore операции требуют backup и ручной проверки на совместимой камере.

Реализовано:

- P6.1 Embedded server core:
  - HTTP/1.1 server на `QTcpServer` с лимитами заголовков/тела и отказом от chunked request bodies;
  - `--server-only` с offscreen Qt platform;
  - localhost-only по умолчанию, LAN bind включается только явно;
  - опциональный WebSocket server и debounce live-state updates при наличии Qt WebSockets.
- P6.2 Auth/session/security:
  - web-аутентификация через существующий `UserManager` без изменения desktop-сессии;
  - 256-bit opaque tokens, хранение только SHA-256 digest, sliding TTL;
  - HttpOnly/SameSite cookies, optional Secure cookie, Bearer API sessions;
  - RBAC для Live View, Playback, PTZ, Settings и Analytics;
  - CSRF/Origin checks, login rate limiting, invalidation при изменении пользователей/прав;
  - CSP, frame denial, no-sniff, no-referrer, same-origin/no-store policies.
- P6.3 Dashboard API v1:
  - server/session/dashboard/cameras;
  - Health status и запуск проверок;
  - Analytics modules/diagnostics/events;
  - Archive inventory и Range streaming;
  - PTZ move/stop.
- P6.4 Data boundaries:
  - API не отдает пароли, hashes/salts, OAuth secrets, credential-bearing RTSP URL и локальные пути;
  - archive playback использует SHA-256 file IDs и повторную canonical-root проверку;
  - записи отдаются bounded chunks без загрузки файла целиком в память.
- P6.5 Web UI:
  - операторское рабочее пространство в стиле desktop с сохраняемыми раскладками 1/4/9;
  - сворачиваемый блок действий, информативные карточки устройств и назначение камеры в активную ячейку;
  - Health, Analytics и Archive открываются едиными рабочими окнами поверх live-раскладки;
  - responsive рабочий интерфейс RU/EN;
  - login, summary, camera cards и authenticated preview endpoint;
  - Health, Analytics и Archive workspace;
  - WebSocket connection status и live dashboard refresh.
- P6.6 Desktop integration:
  - отдельная вкладка `Настройки > Web`;
  - enable/LAN/bind/HTTP/WebSocket/session/Secure cookie controls;
  - runtime status, session/client counters, access URLs и запуск в браузере.

Streaming strategy:

- live preview использует WebRTC через GStreamer `webrtcbin`; H.264 передаётся без перекодирования с исходным FPS;
- для H.265 применяется ограниченное low-latency перекодирование в H.264, а authenticated MJPEG relay остаётся автоматическим fallback для отдельной ячейки;
- RTSP credentials не попадают в browser API, peer pipelines и idle fallback pipelines закрываются автоматически;
- локальный архив отдается authenticated HTTP Range endpoint, поскольку браузер не имеет доступа к desktop filesystem.

### P6.7 Web parity evolution

Статус: P6.8–P6.16 реализованы и включены в `v0.2.7`. Кодовый hardening,
Windows release build, server-only smoke и 30/30 test suite завершены;
Windows installer и Linux AppImage публикуются только после успешного GitHub Actions CI.

Детальный план возобновления и критерии полного паритета:
[`WEB_PARITY_ROADMAP.md`](WEB_PARITY_ROADMAP.md).

- [x] Desktop-like monitor workspace, раскладки 1/4/9 и sidebar устройств.
- [x] Browser-compatible WebRTC live preview без раскрытия credentials камеры, с MJPEG fallback.
- [x] Health, Analytics и Archive поверх рабочего монитора.
- [x] Camera discovery, onboarding и edit dialogs.
- [x] P6.8 — инвентаризация экранов, parity matrix, baseline screenshots и capability contract.
- [x] P6.9 — shared presentation models, formatting, localization и design tokens.
- [x] P6.10 — recording, snapshots, audio, fullscreen, PTZ feedback и Archive parity.
- [x] P6.11 — Settings, users, permissions и session administration.
- [x] P6.12 — logs, diagnostics, live tail, redaction и diagnostic bundles.
- [x] P6.13 — Camex, Majestic и OpenIPC Control Center с safe-action workflow.
- [x] P6.14 — browser-адаптации файловых, keychain, shortcut и native-only функций.
- [x] P6.15 — visual parity, responsive polish, accessibility и Chromium screenshot baseline.
- [x] P6.16 — production hardening, full-suite и автоматизированные release gates.

Проверки этапа:

- полная сборка `appOpenIPC-Dashboard` на Qt 6.4/MinGW;
- `dashboard_http_protocol_tests`;
- `dashboard_web_session_store_tests`;
- `qml_smoke` и targeted QML lint;
- живой `--server-only` smoke через `GET /api/v1/server`;
- `git diff --check`.

Эксплуатация и security review: `docs/WEB_SERVER.md`.

## ✅ P7 — Analytics / Modules evolution

Цель: сделать аналитику не просто набором вкладок, а понятным рабочим центром для модулей, артефактов, событий и evidence-данных.

Реализовано:

- Modules Center v2:
  - инвентаризация AI-модулей через `AnalyticsEngine::moduleInventory()`;
  - карточки модулей показывают статус артефакта, размер модели, storage footprint и временные хвосты;
  - ручная SHA-256/size проверка модели;
  - безопасная очистка `.part/.tmp/.download/.previous` без удаления установленной модели;
  - быстрый переход к source repository модуля.
- Diagnostics API:
  - `verifyModuleArtifact`;
  - `cleanupModuleArtifacts`;
  - `analyticsRecommendations`;
  - `analyticsEvidenceSummary`;
  - `getCameraAnalyticsDiagnostics`.
- Analytics Overview v2:
  - compact summary по evidence-артефактам;
  - счетчики файлов/снимков/клипов/занятого места;
  - рекомендации по состоянию event store, evidence, modules, assignments и rules.
- Events export:
  - экспорт текущей ленты с учетом фильтров;
  - JSON/CSV;
  - дефолтный путь `Documents/OpenIPC/Analytics`.
- Event feed UX:
  - быстрые кнопки экспорта;
  - статус результата экспорта прямо в панели событий.

Проверено:

- `cmake --build build_release --target appOpenIPC-Dashboard -j 2`;
- `ctest --test-dir build_release -R "qml_smoke|model_artifact_verifier_tests|camera_model_tests" --output-on-failure`;
- `git diff --check`.

## ✅ P8 — Polish / Refactor / Observability

Цель: снизить технический долг после быстрого роста P3-P7, сделать приложение проще сопровождать и стабильнее диагностировать без больших продуктовых рисков.

Принцип этапа: дробим только там, где есть понятная граница ответственности, и каждый шаг закрываем сборкой/тестами. Поведение приложения должно оставаться прежним или становиться очевидно надежнее.

Статус: ✅ закрыто 2026-07-13.

### P8.1 Application logging

Статус: ✅ закрыто в текущей ветке 2026-07-12.

Задачи:

- включить application log handler по умолчанию;
- писать события в `app.log` и одновременно отдавать их в `LogModel`;
- загружать хвост существующего `app.log` при старте и при открытии окна логов;
- добавить пустое состояние, обновление, экспорт и фильтры в `LogView`;
- покрыть `LogModel` unit-тестами.

Что это даст:

- кнопка “Логи” станет реально полезной для диагностики;
- можно будет отличать проблемы QML/backend/network без запуска из терминала;
- будущие crash/bug reports будут проще воспроизводить.

### P8.2 Refactor map: крупные файлы

Статус: ✅ закрыто.

Уже сделано в текущей ветке:

- `MajesticApplyConfirmDialog.qml` вынесен из `MajesticControlDialog.qml` без изменения поведения apply-flow.
- `MajesticFileDialogs.qml` отделяет snapshot/config/PCM/firmware file pickers от Control Center и сохраняет прежние backend-вызовы.
- обработка crop/upscale/enhance для аналитических evidence вынесена из `AnalyticsEngine.cpp` в `AnalyticsEvidenceImageProcessor`.
- для алгоритма evidence image processing добавлены отдельные unit-тесты границ детекции, валидации и улучшения малых снимков.
- `AnalyticsEngineEvents.cpp` теперь отвечает за in-memory/SQLite event store, запросы, очистку и JSON/CSV export.
- `AnalyticsEngineDiagnostics.cpp` содержит module inventory, artifact health, evidence summary и рекомендации; основной `AnalyticsEngine.cpp` уменьшен примерно на 900 строк.
- `AnalyticsEngineUploads.cpp` вынес OAuth/keychain и очередь cloud/local/FTP upload; основной `AnalyticsEngine.cpp` уменьшен еще примерно на 480 строк.
- из `SystemController.cpp` вынесены camera groups, recording/export utilities и app settings/path handling; публичный QML API не изменён.
- `SystemControllerState.cpp` вынес state serialization, legacy JSON migration, SQLite state-store save/load и keychain password restore; основной `SystemController.cpp` уменьшен примерно на 500 строк.
- `I18n.qml` очищен от повторяющихся ключей в английском словаре с сохранением фактического runtime-поведения.

Принятые границы:

- `MajesticControlDialog.qml` остается координатором текущей camera transaction: `Connections`, timers и rollback/apply state пока не дробятся без отдельного state-proxy, чтобы не ломать управление камерой.
- дальнейшая декомпозиция Analytics runtime/inference и крупных QML-панелей переносится в обычную архитектурную эволюцию следующих продуктовых этапов, не в P8-hotspot.

Что это даст:

- меньше риска ломать соседние функции при точечных правках;
- быстрее review и тестирование;
- проще добавлять новые этапы вроде Archive/Recording v2.

### P8.3 QML/runtime hardening

Статус: ✅ закрыто.

Уже сделано:

- smoke matrix создаёт Settings, Analytics, Majestic Control Center и окно application logs;
- smoke запускается в обычном режиме и при `QT_SCALE_FACTOR=1.5`;
- `LogView` получил адаптивные размеры, явные model roles и безопасные ссылки на кнопки/модель;
- новый `MajesticFileDialogs.qml` проходит QML cache compilation, а весь набор компонентов — оба smoke-теста.
- добавлен воспроизводимый `qml_lint_targeted` test для изменяемых критичных компонентов;
- `I18n.qml` добавлен в targeted QML lint после чистки словаря;
- `i18n_catalog_tests` проверяет, что словари не содержат дублей и явных mojibake-маркеров;
- полный legacy `qmllint` baseline измерен и оставлен отдельной очередью, чтобы не смешивать массовые QML-правки с безопасным refactor.

### P8.4 CI cleanup

Статус: ✅ закрыто.

Уже сделано:

- production workflows переведены на Node 24-compatible поколения официальных `actions/*`;
- CI и release используют одинаковые Qt `6.4.2` modules и GStreamer `1.26.10` для Windows.
- Linux и Windows CI теперь запускают targeted QML lint после build/smoke.
- release workflow остается главным production gate; локально обязательны build + unit + smoke + lint перед пушем.

## ✅ P9 — Archive / Recording evolution

Цель: превратить локальный архив из базового списка файлов в надежный слой записей, который одинаково понимают ручная запись, архив, экспорт и будущие события.

Статус: ✅ закрыт 2026-07-13.

### P9.1 Recording catalog foundation

Статус: ✅ стартовый пакет закрыт 2026-07-13.

Задачи:

- вынести генерацию, разбор и метаданные файлов записи в отдельный backend-модуль;
- поддержать старый формат имен и новый формат с миллисекундами, чтобы не было коллизий при быстрых стартах записи;
- искать не только `.mp4`, но и `.mkv/.avi/.mov`;
- фильтровать по IP/санитизированному имени камеры и подпапкам;
- возвращать в UI настоящий `filePath` и `fileUrl`, а не пересобирать путь по имени файла;
- покрыть парсер и `ArchiveController::search` unit-тестами.

Что это даст:

- архив начнет стабильно открывать записи из пользовательского каталога и подпапок;
- ручная запись и архив будут использовать одну схему именования;
- следующие функции можно строить поверх каталога, не размазывая эвристику по QML и контроллерам.

Проверено:

- `cmake --build build_release --target appOpenIPC-Dashboard -j 2`;
- `ctest --test-dir build_release -L unit --output-on-failure`;
- `ctest --test-dir build_release -L smoke --output-on-failure`;
- `ctest --test-dir build_release -L lint --output-on-failure`;
- `git diff --check`.

### P9.2 Archive UI v2

Статус: ✅ закрыт 2026-07-13.

План:

- улучшить список записей: размер, длительность, источник, камера;
- сделать пустые состояния и ошибки поиска понятнее;
- добавить быстрые периоды, сортировку и группировку по дням без перегруза окна;
- отделить `ArchiveView.qml` на небольшие компоненты: sidebar filters, result list, player controls, export dialog.

Сделано:

- вынесен `ArchiveSearchSidebar.qml`: выбор камеры, период, быстрые диапазоны, запуск поиска и календарь;
- вынесен `ArchiveResultsList.qml`: секции по датам, источник записи, размер/длительность и пустые состояния;
- вынесен `ArchivePlaybackControls.qml`: таймлайн, play/stop, seek, громкость, скорость, fullscreen и маркеры нарезки;
- вынесен `ArchiveExportDialog.qml`: подготовка имени клипа и единая точка запуска `ArchiveController::exportVideo`;
- добавлены фильтр по источнику записи (`Все / Ручные / События`) и сортировка `Сначала новые / Сначала старые`;
- `ArchiveView.qml` больше не содержит встроенный список результатов, корневой календарь, панель плеера и `FileDialog` экспорта;
- пустой поиск больше не перебивает пользователя модальным окном — состояние показывается в списке;
- новые компоненты добавлены в targeted QML lint.

Закрыто следующими подпунктами P9:

- P9.3: прогресс/ошибка ffmpeg и более понятная финализация экспортируемого клипа;
- P9.4: политики хранения и контроль занятого места;
- P9.5: телеметрия надежности записи и связка с application log.

### P9.3 Export and clipping workflow

Статус: ✅ закрыт 2026-07-13.

Сделано:

- нормализовать экспорт через один backend-путь;
- явно показывать прогресс/ошибку ffmpeg;
- сохранять клипы рядом с архивом или в выбранный каталог;
- добавить unit/integration coverage для путей экспорта без запуска реального UI;
- добавить `ArchiveExportStatus.qml`: прогресс, финальный путь, ошибка и быстрое открытие папки результата;
- писать старт/успех/ошибку экспорта в application log.

### P9.4 Retention and storage management

Статус: ✅ закрыт 2026-07-13.

Сделано:

- подсчет занятого места по каталогу записей;
- политика хранения по дням/размеру;
- безопасная очистка только внутри выбранного каталога;
- предупреждения, если выбранный каталог небезопасен для очистки;
- добавить `ArchiveStoragePanel.qml`: размер архива, счетчики ручных/event-записей, dry-run план очистки и применение очистки;
- покрыть storage summary и cleanup unit-тестом.

### P9.5 Recording reliability telemetry

Статус: ✅ закрыт 2026-07-13.

Сделано:

- отображать статус текущей ручной записи отдельно от live-preview;
- фиксировать старт/стоп/ошибку записи в application log;
- связать analytics-triggered clips с тем же каталогом и UI-архивом;
- логировать старт/остановку event clips, ручной записи и ротацию сегментов;
- логировать предупреждения fallback-записи, если event clip не удалось переключить на buffered recording.

## ✅ P10 — Reliability / Analytics completion / Architecture

Цель: закрыть оставшиеся разрывы между интерфейсом и фактическим runtime-поведением аналитики, усилить сохранность записей и продолжить безопасную декомпозицию без изменения рабочих сценариев камер.

Статус: ✅ закрыт 2026-07-14.

### P10.1 Analytics rules and zones

Статус: ✅ закрыт.

Сделано:

- реализован настоящий полигональный редактор зоны правила с 3–8 перемещаемыми вершинами;
- правила сохраняют нормализованный `zonePolygon`, старые `full/center/left/right/top/bottom` пресеты полностью совместимы;
- backend проверяет центр детекции через отдельный `AnalyticsRuleZoneMatcher`;
- границы полигона считаются частью зоны, некорректная custom-зона безопасно не срабатывает;
- matcher покрыт unit-тестами для пресетов, polygon inside/outside/edge и нормализации координат;
- удалён неиспользуемый `AnalyticsModel` с mock-данными и заглушечным config API.

Что это даёт:

- пользователь задаёт реальную область интереса вместо декоративного выбора;
- rule engine и UI используют один сохраняемый формат;
- тестируемая геометрия больше не спрятана внутри крупного `AnalyticsEngine.cpp`.

### P10.2 Evidence upload reliability

Статус: ✅ закрыт.

Сделано:

- очередь выгрузки получила до трёх попыток с ограниченной экспоненциальной задержкой;
- HTTP-операции получили таймаут, FTP — connect/max-time ограничения;
- локальная/NAS-выгрузка выполняется через `.part` и финальное переименование;
- добавлены runtime-состояния `queued/uploading/retry_wait/success/failed`, счетчики успехов, ошибок и повторов;
- итог и повторы пишутся в application log и входят в `analyticsDiagnostics`;
- восстановлена пропавшая после прежнего refactor панель upload/OAuth в настройках аналитики;
- retry-policy вынесена в отдельный модуль и покрыта unit-тестами.

Что это даёт:

- временный сетевой сбой больше не теряет evidence молча;
- состояние очереди и последняя ошибка видны в интерфейсе;
- локальная копия не появляется в целевом каталоге под финальным именем до полного завершения записи.

### P10.3 Recording finalization and recovery

Статус: ✅ закрыт.

Сделано:

- экспорт `ffmpeg` пишет в `*.part.<container>` и публикует итог только после успешного завершения;
- существующий экспорт сохраняется как `.previous` на время замены и возвращается при ошибке финализации;
- ошибка `ffmpeg` удаляет неполный временный файл;
- архив обнаруживает `.part/.part.*/.tmp/.previous`, считает их размер и возраст;
- безопасная очистка удаляет только stale-файлы старше 15 минут и только внутри проверенного каталога записей;
- состояние незавершённых файлов и команда очистки доступны в сворачиваемой панели хранилища;
- recovery-path покрыт unit-тестом, подтверждающим сохранность обычной записи.

Что это даёт:

- повреждённый клип не маскируется под готовый экспорт;
- после сбоя можно увидеть и убрать хвосты без ручного поиска по диску;
- очистка не выходит за границы пользовательского каталога архива.

### P10.4 Architecture and QML hardening

Статус: ✅ закрыт.

Сделано:

- `AnalyticsZoneEditor`, `SettingsEvidenceUploadPanel`, zone matcher и retry policy вынесены в самостоятельные компоненты;
- удалён мёртвый QML type registration `AnalyticsModel`;
- targeted `qmllint` расширен на редактор правил, зону и upload panel;
- исправлен lifecycle-доступ делегата `LogView`, который на масштабе 150% мог рекурсивно генерировать предупреждения и переполнять application log;
- полный legacy QML baseline обновлён: `1093` предупреждения против прежних `1184`, новые P10-файлы проходят targeted lint без предупреждений.

Принятые границы:

- `MajesticControlDialog.qml` остаётся transaction coordinator до появления отдельного state-proxy;
- оставшийся legacy `qmllint` долг сокращается небольшими изолированными пакетами и не смешивается с функциональными изменениями камер;
- опасные автоматические firmware/cloud операции по-прежнему не включаются без отдельного security-дизайна.

### P10.5 Quality gates

Статус: ✅ закрыт.

Проверено:

- полная сборка `appOpenIPC-Dashboard`;
- `24/24` unit-тестов;
- QML smoke в обычном режиме и при `QT_SCALE_FACTOR=1.5`;
- targeted `qmllint` для новых и изменённых критичных компонентов;
- `git diff --check` и отсутствие новых TypeError/ReferenceError в smoke-логах.

## ✅ P11 — Production hardening / Web operations / Release quality

Цель: превратить функционально насыщенный `v0.2.7` в предсказуемую production-платформу,
которую можно безопасно обновлять, диагностировать и развёртывать как desktop-приложение
или автономный Web-сервер.

Целевой релиз: `v0.2.8`.

### P11.1 Field stabilization и upgrade compatibility

Статус: ✅ закрыт в `v0.2.8`.

Работы:

- собрать подтверждённые runtime-отчёты Windows/Linux и desktop/server-only после `v0.2.7`;
- завести воспроизводимую issue-форму с версией, platform, GPU, Qt/GStreamer и diagnostic bundle;
- проверить обновление с `v0.2.6.1` и `v0.2.7` с сохранением пользователей, камер, групп,
  Web-настроек, архива и credential references;
- добавить explicit migration tests для settings/state/session schema;
- проверить первый запуск, повторный запуск, портовые конфликты и восстановление после
  некорректного завершения desktop и `--server-only`;
- зафиксировать time-to-ready, time-to-first-frame, reconnect storm и shutdown budget;
- исключить crash/blocker и неконтролируемый рост логов до начала новых крупных функций.

Результат этапа:

- обновление не требует ручной очистки пользовательского состояния;
- diagnostic bundle достаточен для воспроизведения типовых runtime-проблем;
- подтверждённая compatibility matrix становится частью release notes.

### P11.2 Release engineering и supply-chain metadata

Статус: ✅ закрыт в `v0.2.8`.

Работы:

- перевести Windows CMake build на контролируемый `--parallel` и измерить выигрыш без OOM;
- кэшировать безопасно кэшируемые Qt/GStreamer/download зависимости между CI run;
- оставить bounded retries и integrity/size validation для внешних release assets;
- публиковать SHA-256 checksums Windows Installer и Linux AppImage;
- генерировать SBOM/third-party manifest с версиями Qt, GStreamer, OpenSSL, libssh и AI models;
- выполнять smoke уже упакованного Installer/AppImage, а не только build-directory executable;
- проверять `--server-only`, offscreen plugin, WebRTC runtime и TLS runtime из чистого package;
- унифицировать CI/release workflow, чтобы одинаковые build flags не расходились;
- добавить понятный failure summary и сохранять релевантные CMake/package logs.

Результат этапа:

- release assets воспроизводимы, проверяемы и сопровождаются checksums/metadata;
- transient dependency failure не требует ручного редактирования тега;
- среднее время release workflow заметно сокращено относительно `v0.2.7` baseline.

### P11.3 Secure Web deployment

Статус: ✅ закрыт в `v0.2.8`.

Работы:

- формализовать trusted reverse proxy и external base URL без доверия произвольным headers;
- добавить deployment profiles для localhost, trusted LAN, VPN и HTTPS reverse proxy;
- подготовить systemd unit и Windows service/headless guidance для `--server-only`;
- добавить health/readiness endpoints, структурированную startup diagnostics и port-conflict hints;
- спроектировать STUN/TURN configuration с write-only credentials и ICE diagnostics;
- проверить secure cookies, WebSocket upgrade и download URLs за TLS-терминирующим proxy;
- добавить session/audit dashboard: активные клиенты, origin, idle/absolute TTL и revoke reason;
- определить backup/restore Web server settings без экспорта secrets;
- провести отдельный threat-model review перед любым публичным remote-access сценарием.

Результат этапа:

- Web server разворачивается как управляемый локальный/VPN/proxy service;
- документация даёт проверяемый путь HTTPS deployment без прямого проброса HTTP-порта;
- STUN/TURN и trusted proxy не ослабляют существующие RBAC/CSRF/session границы.

Принятая граница: собственный cloud relay в P11 не создаётся.

### P11.4 Web UI maintainability и remaining parity

Статус: ✅ закрыт в `v0.2.8`; Health/Analytics сохраняют документированные `Partial`-границы.

Работы:

- разделить крупный `src/web/app.js` на state, API, monitor, admin и device modules;
- формализовать Web component states и design tokens без отдельной бизнес-логики в DOM;
- довести Health и Analytics details до документированного parity либо оставить явный Partial;
- добавить screenshot regression для desktop/tablet/mobile, RU/EN и основных error/empty states;
- проверить keyboard-only flow, focus return, screen-reader labels, contrast и reduced motion;
- расширить браузерную матрицу Chromium → Firefox → WebKit с известными codec limitations;
- добавить contract smoke для login/logout, role boundaries, settings update и archive download;
- удерживать mobile controls компактными без горизонтального overflow.

Результат этапа:

- Web UI изменяется пакетами без дальнейшего роста одного глобального скрипта;
- parity matrix подтверждается автоматическими contract/visual gates;
- browser-specific ограничения видны пользователю до запуска действия.

### P11.5 Desktop QML/runtime debt

Статус: ✅ release-пакет `v0.2.8` закрыт; дальнейшее сокращение legacy debt продолжается инкрементально.

Работы:

- продолжить сокращение legacy `qmllint` baseline небольшими измеримыми пакетами;
- вынести coordination/state из `MajesticControlDialog.qml` и следующих крупнейших QML файлов;
- добавить targeted smoke для Camera Search resize/compact layout и sidebar interaction states;
- унифицировать focus/keyboard/accessibility правила desktop dialogs;
- проверять scaling 100/125/150/200% и длинные RU/EN строки;
- сокращать прямые связи QML → крупный SystemController через presentation/adapters;
- запрещать новые lint/runtime warnings в изменяемых компонентах.

Результат этапа:

- новые функции не увеличивают legacy QML debt;
- критичные desktop workflows имеют focused smoke и lint gates;
- крупные dialogs постепенно становятся thin UI поверх тестируемых state objects.

### P11 release gates

- upgrade/migration matrix `v0.2.6.1 → v0.2.8` и `v0.2.7 → v0.2.8`;
- полный C++/contract/QML test suite на Windows и Linux;
- package-level smoke Windows Installer и Linux AppImage;
- desktop и `--server-only` startup/shutdown/recovery smoke;
- Web login/RBAC/CSRF/session/archive/device-action negative tests;
- browser baseline Chromium и documented Firefox/WebKit result;
- release assets, SHA-256 checksums, SBOM/manifest и актуальные release notes;
- отсутствие открытых Critical/High security defects и blocker regressions.

## 🟡 P12 — Sites / Fleet Management

Цель: управлять десятками камер и несколькими площадками без опасных неявных массовых
операций и без обязательного внешнего cloud service.

Работы:

- модель `Site / Area / Group / Tag` поверх существующих камер и групп;
- fleet inventory с firmware/Majestic versions, capabilities, health и last-seen;
- быстрые фильтры по site, tag, model, firmware drift, offline и maintenance state;
- сохранённые views и role-scoped доступ к отдельным площадкам;
- configuration baseline и drift report с redacted preview/diff;
- безопасные групповые read operations и экспорт inventory/diagnostics;
- batch mutation только через selection summary, compatibility preflight, dry-run и audit;
- maintenance windows, concurrency limits, progress, cancellation и per-device result;
- backup-before-change и recovery guidance для firmware/configuration waves;
- offline-friendly import/export site definitions без credentials.

Definition of Done:

- оператор видит состояние площадок и быстро локализует проблемную группу;
- администратор получает drift/compatibility report до любых изменений;
- массовая операция не скрывает частичный failure и не обходит per-device capability checks.

Принятая граница: массовое автоматическое firmware обновление не включается до отдельной
real-camera qualification matrix и подтверждённого recovery plan.

## 🔜 P13 — Incident Center / Notifications / Operator workflow

Цель: объединить Health, Analytics, Recording, Logs и Audit в управляемый жизненный цикл
инцидента, а не набор разрозненных событий.

Работы:

- единая нормализованная event schema и корреляция по camera/site/time window;
- состояния `new / acknowledged / investigating / resolved / false positive`;
- severity, owner, comments, bookmarks и immutable activity timeline;
- связь инцидента с recordings, snapshots, health runs, logs и device operations;
- фильтры, saved searches, retention и экспорт incident bundle;
- rule-based дедупликация, cooldown и suppression during maintenance;
- адаптеры Webhook/MQTT/Telegram/email с scoped secrets, retries и delivery audit;
- notification routing по site/severity/schedule без раскрытия camera credentials;
- операторские dashboard counters и SLA/time-to-ack metrics;
- desktop/Web parity для incident review и acknowledgement.

Definition of Done:

- одно событие не создаёт бесконтрольный notification storm;
- evidence и audit позволяют восстановить историю решения;
- failed delivery виден, повторяем и не теряется молча.

## 🧊 P14 — Media scale / Adaptive quality / Multi-monitor

Цель: увеличить практический предел одновременного наблюдения только вместе с измеримыми
CPU/GPU/network budgets и предсказуемой деградацией.

Исследовательские пакеты:

- layout 16/25 после подтверждения preview/decoder budget;
- adaptive HD/SD selection по cell size, focus, visibility и resource pressure;
- per-camera/per-cell codec, FPS, bitrate, latency, drops и reconnect telemetry;
- hardware decode/encode capability matrix Windows/Linux/GPU vendors;
- bounded WebRTC peer/transcode pools и fair resource scheduling;
- background-tab, hidden-window и multi-monitor lifecycle;
- optional low-bandwidth/site profile и operator-controlled quality caps;
- benchmark fixtures H.264/H.265, offline/reconnect storm и mixed-resolution walls.

Definition of Done:

- заявленный camera count подтверждён benchmark matrix, а не только успешным открытием;
- перегрузка снижает качество предсказуемо и не приводит к crash/OOM/reconnect storm;
- desktop и Web показывают причину degraded mode.

## 🧊 P15 — Versioned integration ecosystem

Цель: позволить внешней автоматизации безопасно использовать Dashboard без прямого доступа
к внутренним SQLite/settings и без загрузки непроверенного кода в процесс приложения.

Возможные работы:

- OpenAPI/JSON Schema для стабильной части `/api/v1` и compatibility policy;
- scoped service accounts/API tokens с expiry, rotation, revoke и audit;
- webhook subscriptions с signature verification, retry и delivery status;
- MQTT/Home Assistant/NVR adapters как внешние процессы поверх versioned contracts;
- documented examples для inventory, health, archive и incident APIs;
- import/export schemas с versioning и migration tools;
- provider interface для внешней аналитики/evidence без передачи camera credentials;
- deprecation window и contract tests для API clients.

Принятая граница: произвольные in-process plugins и unsigned native modules не входят в
первую версию ecosystem из-за ABI, supply-chain и security рисков.

## ⛔ Решения, которые пока не делаем автоматически

- Автоматический full restore OpenIPC backup через непроверенный endpoint.
- Автоматический firmware rollback после неудачной прошивки: для firmware это опасная зона и зависит от bootloader/разметки/конкретной камеры.
- Cloud relay / публичный удалённый доступ без отдельного security-дизайна.
- Web version без auth, permissions и threat model.

## Quality gates для следующих работ

Перед коммитом:

- `git diff --check`
- Release build или релевантная локальная сборка
- `ctest --test-dir build_release --output-on-failure`
- targeted `qmllint` для изменённых/новых QML компонентов

Перед релизом:

- Windows build в GitHub Actions
- Linux AppImage build в GitHub Actions
- Проверка release assets
- Проверка GitHub Release notes
- Smoke-проверка приложения
- Если затронут firmware upgrade — ручная проверка на реальной OpenIPC-камере только после backup и при стабильном питании

## Ближайший практический порядок работ

1. Утвердить data model P12 Sites/Fleet до реализации массовых операций.
2. Начать read-only inventory и фильтры site/area/tag без массовых mutations.
3. Спроектировать configuration drift report и dry-run contract до batch apply.
4. Вести QML/Web debt малыми пакетами без смешивания с firmware/device changes.
5. Держать release workflow главным production gate: Windows installer, Linux AppImage,
   package smoke, release notes и проверяемые assets.
