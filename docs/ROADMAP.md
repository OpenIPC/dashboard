# OpenIPC Dashboard Roadmap

Последнее обновление: 2026-07-13.

Текущий стабильный релиз: `v0.2.6`.

Текущий фокус разработки: `P9 Archive/Recording evolution`.

## Обозначения

- ✅ Сделано и вошло в релиз.
- 🟡 В работе / ближайший активный фокус.
- 🔜 Следующая очередь.
- 🧊 Backlog / будущая возможность.
- ⛔ Не делаем без дополнительного решения или подтверждения.

## Краткое состояние проекта

OpenIPC Dashboard после `v0.2.6` уже умеет не только смотреть камеры, но и управлять OpenIPC/Majestic-устройствами как единый control center:

- поиск OpenIPC/ONVIF/RTSP камер в сети;
- live preview с HD/SD режимами;
- единый status-layer камер;
- Majestic schema-driven настройки;
- OpenIPC firmware status/network/time/logs/backup/reboot/update;
- firmware upgrade через `/ws/upgrade`;
- live firmware logs;
- in-app updater приложения через GitHub Releases;
- более компактный Dashboard UI и sidebar;
- релизные Windows/Linux сборки через GitHub Actions.

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

## 🧊 P6 — Web version / Server mode

Статус: backlog, архитектурно возможно, но это отдельный крупный этап.

Идея:

- На ПК запускается desktop/server edition OpenIPC Dashboard.
- Другие устройства в LAN/Internet подключаются к web UI через браузер.
- Web UI повторяет основные функции desktop-приложения.

Что потребуется:

- Локальный HTTP/WebSocket server внутри приложения или отдельный companion server.
- Auth/session модель.
- Role-based permissions.
- Streaming strategy:
  - прямые ссылки на камеры;
  - proxy;
  - transcoding только если действительно потребуется.
- API layer для Dashboard state.
- Безопасная работа через статический IP/VPN/reverse proxy.
- Отдельный security review.

Почему не P2/P3:

- Это не “просто страница”.
- Нужно проектировать безопасность, сеть, доступ к потокам и синхронизацию состояния.
- Начинать стоит после стабилизации desktop-core и Health/Discovery v2.

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

## 🟡 P9 — Archive / Recording evolution

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

1. Перейти к P9.2: дробление `ArchiveView.qml` вокруг реальных сценариев архива.
2. После стабилизации UI закрыть P9.3 export/clipping workflow.
3. Расширять targeted `qmllint` на новые/изменённые компоненты и постепенно сокращать legacy baseline отдельными небольшими PR.
4. Держать release workflow главным production gate: Windows installer, Linux AppImage, smoke и release assets.
