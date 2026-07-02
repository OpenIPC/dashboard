# OpenIPC Dashboard Roadmap

Последнее обновление: 2026-07-02.

Текущий стабильный релиз: `v0.2.5.2`.

Текущий фокус разработки: `P2 — качество QML, smoke-проверки и архитектурная полировка`.

## Обозначения

- ✅ Сделано и вошло в релиз.
- 🟡 В работе / ближайший активный фокус.
- 🔜 Следующая очередь.
- 🧊 Backlog / будущая возможность.
- ⛔ Не делаем без дополнительного решения или подтверждения.

## Краткое состояние проекта

OpenIPC Dashboard после `v0.2.5.2` уже умеет не только смотреть камеры, но и управлять OpenIPC/Majestic-устройствами как единый control center:

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

## 🟡 P2 — текущий активный этап

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

Статус: 🔜 следующий шаг P2.

Что нужно сделать:

- Подготовить минимальный smoke-run основных QML окон:
  - Dashboard;
  - Settings;
  - Camera Search;
  - Majestic/OpenIPC Control Center;
  - Health Center.
- Проверить, можно ли стабильно запускать smoke-тесты в CI/headless окружении.
- Если headless нестабилен, оставить локальный smoke script и CI-friendly targeted `qmllint`.

Готовность к закрытию:

- Есть повторяемая команда smoke-проверки.
- Команда задокументирована.
- Smoke-проверка не требует реальной камеры для базового запуска UI.

### P2.3 — дальнейшая декомпозиция крупных QML/controller файлов

Статус: 🔜 после P2.1/P2.2.

Правило:

- Новый функционал не добавляем в уже раздутые QML-файлы, если он требует отдельного состояния, нескольких UI-блоков или приближается к 200-300 строкам.
- Сразу создаём отдельный QML-компонент и, если нужно, отдельный C++/QML controller/model.

Кандидаты на дальнейшую декомпозицию:

- `SettingsDialog.qml`;
- `AppUpdateDialog.qml`;
- оставшиеся controller-блоки в `MajesticControlDialog.qml`;
- крупные старые панели Dashboard/Analytics, если начнём их активно расширять.

### P2.4 — UI polish после sidebar wave

Статус: 🔜 после технической чистки.

Что проверить:

- collapsed/expanded sidebar на разных размерах окна;
- выравнивание иконок и подписей;
- длинные RU/EN строки;
- keyboard focus;
- tooltips;
- визуальное состояние disabled/loading/error;
- мелкие DPI/масштабирование Windows/Linux.

## 🔜 P3 — Health Center v2

Цель: превратить Health Center из диагностического окна в полноценный инструмент обслуживания камер.

Планируемые функции:

- Персистентная история проверок между запусками.
- Health-профили:
  - быстрый;
  - глубокий;
  - OpenIPC/Majestic;
  - RTSP-only.
- Deeper probes:
  - Majestic config/schema;
  - firmware status;
  - `/metrics`;
  - `/ws/logs` readiness;
  - RTSP main/sub;
  - snapshot/JPEG endpoint.
- Рекомендации по исправлению:
  - неверные credentials;
  - недоступен RTSP;
  - Majestic API отвечает, но stream не идёт;
  - камера online, но не в раскладке;
  - firmware WebUI доступен, но Majestic недоступен.
- Экспорт расширенного отчёта:
  - status;
  - probes;
  - последние логи;
  - версии firmware/Majestic;
  - сетевые данные.

Готовность к закрытию:

- Health Center сохраняет историю.
- Есть минимум 3 профиля проверки.
- Отчёт помогает пользователю понять, что именно сломалось.

## 🔜 P4 — Camera onboarding / Discovery v2

Цель: сделать добавление камер ещё проще и надёжнее.

Планируемые функции:

- Улучшенное обнаружение дублей.
- Массовое добавление найденных камер.
- Проверка credentials перед добавлением.
- Выбор stream profile при добавлении:
  - OpenIPC/Majestic;
  - ONVIF;
  - RTSP manual.
- Более понятные причины “почему камера найдена, но не добавлена”.
- Сохранение последнего результата поиска.
- Улучшение работы с несколькими сетевыми интерфейсами.

## 🔜 P5 — Majestic/OpenIPC advanced v3

Цель: приблизить управление к уровню WebUI камеры, но сохранить безопасность desktop-приложения.

Планируемые функции:

- Дальнейшее расширение capabilities/endpoints view.
- Более глубокая проверка firmware image metadata, если формат архива позволяет.
- Поддержка checksum sidecar/download, если OpenIPC источники отдают такие данные.
- Улучшенная visual diff-модель для Majestic restore.
- Более явное разделение live-параметров и reload-required параметров.
- Возможный wizard для безопасного изменения критичных video/network настроек.

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

## 🧊 P7 — Analytics / Modules evolution

Планируемые направления:

- Улучшение UX модулей аналитики.
- Более понятное управление моделями/артефактами.
- Диагностика модулей.
- Визуальная история событий.
- Улучшенный экспорт событий и snapshot-данных.

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

1. P2.2: подготовить минимальный QML smoke-run.
2. P2.3: продолжить точечную чистку legacy QML по baseline.
3. P2.3: вынести следующий крупный старый UI/controller блок, если он мешает чистке.
4. P2.4: довести последние UI-polish мелочи после sidebar/layout wave.
5. После закрытия P2 перейти к P3 Health Center v2.
