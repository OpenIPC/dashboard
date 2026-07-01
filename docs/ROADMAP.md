# OpenIPC Dashboard Roadmap

Актуальный план развития после релиза `0.2.4`, интеграции Majestic/OpenIPC WebUI идей, разборки Dashboard и доработок Health Center.

Последнее обновление: 2026-07-01.

## Уже сделано

### Dashboard, сетка и общий UI

- Крупные части `DashboardView.qml` вынесены в отдельные QML-компоненты.
- Редактор раскладок вынесен в `LayoutEditorDialog.qml`.
- Сайдбар, top bar, status bar, toast, empty hints, drag proxy, layout toolbar и повторяющиеся кнопки/бейджи приведены к общему UI-набору.
- Исправлены проблемы раскладки Grid после дробления Dashboard.
- Исправлено мигание камеры в списке устройств при hover по action-кнопкам.
- Подпись видеопотока приведена к единому виду: codec, resolution, bitrate, FPS.
- Начата UI friendliness wave 1:
  - в сайдбар добавлена дружелюбная summary-карточка “Панель управления”;
  - основные действия визуально сгруппированы, “Поиск камер” стал главным CTA;
  - перегруженная сетка быстрых действий заменена на более читаемые command-кнопки `SidebarCommandButton.qml`;
  - редкие действия были сгруппированы отдельно, затем переведены в компактную Metro-сетку без скрытия;
  - UI wave 1.1: сайдбар переведён в более строгий Viewer-like стиль — компактная шапка, плотный блок действий, приглушённые кнопки, статистика и live-preview свернуты в короткие строки;
  - UI wave 1.2: верхняя summary-панель убрана с первого экрана сайдбара, действия подняты наверх и оформлены как Metro-like плитки с центрированными иконками/подписями;
  - UI wave 1.3: действия сайдбара ужаты в 3-колоночную Metro-сетку без блока “Ещё”, а кнопки создания/редактирования раскладок встроены в toolbar раскладок;
  - строка устройства вынесена в `DeviceListItem.qml` и стала стабильной карточкой с зарезервированной зоной quick actions;
  - заголовки групп вынесены в `SidebarSectionHeader.qml`;
  - empty state Dashboard получил короткий сценарий быстрого старта;
  - новые строки локализованы RU/EN.

### Единый статус камер

- Effective status вынесен в общий C++-слой.
- Dashboard, ячейки, Sidebar и Health Center используют единые правила online/offline/attention.
- Offline/ошибки потока приоритетнее старого optimistic online из списка камер.
- Добавлены unit-тесты для сценариев online/offline/attention/stream/auth.

### Health Center

- Фильтры: все, с проблемами, offline, online, в сетке, не в сетке, auth, stream.
- Массовая перепроверка камер.
- История последних проверок в текущей сессии.
- Экспорт диагностического отчёта в файл.
- Health Center использует тот же слой статусов, что Dashboard и Sidebar.

### Поиск камер

- Реализован поиск OpenIPC-камер в сети.
- Используются OpenIPC/mDNS, WS-Discovery/ONVIF, RTSP probe, HTTP/Majestic probe.
- Добавлены быстрый/глубокий сценарии поиска.
- Progress bar показывает процент и этап поиска.
- После завершения поиска progress bar корректно скрывается.

### Live preview и потоки

- Исправлена работа SD/substream для OpenIPC/Majestic.
- HD/fullscreen и SD/preview разведены корректнее.
- Добавлена индикация live-preview budget: active, pause, limit.
- Статусы preview и статус камеры синхронизированы с общим status-layer.

### Majestic API

- Настройки Majestic строятся по schema камеры.
- Настройки сгруппированы по понятным разделам вместо общей вкладки “Все настройки”.
- Добавлены локализованные названия и подсказки RU/EN для известных Majestic-полей.
- Apply работает через schema-safe diff без отправки `null`.
- Критичные настройки явно помечаются как требующие reload pipeline.
- После apply поддержан reload pipeline по штатному endpoint.
- Добавлен safe rollback v1:
  - перед критичным apply сохраняется rollback snapshot;
  - после применения показывается карточка rollback;
  - откат выполняется обратным diff-запросом.
- Backup/restore UI v1:
  - backup загружается как restore-кандидат;
  - показывается число отличий;
  - есть preview restore, apply backup diff, clear restore.
- Endpoints/capabilities v1:
  - summary по endpoints/capabilities;
  - RTSP main/sub/JPEG;
  - WS preview;
  - MJPEG, MP4, HLS;
  - audio endpoints;
  - Majestic API endpoints;
  - OpenIPC firmware endpoints `/ws/logs`, `/ws/upgrade`, `/upload`, status/network/time/update pages.

### OpenIPC Control Center / Firmware

- Добавлен единый OpenIPC Control Center в окне Majestic/Firmware.
- Реализованы read/write операции firmware-клиента:
  - status;
  - network load/save/reset/Wi‑Fi scan;
  - time load/save/NTP sync/set from PC;
  - logs;
  - firmware backup;
  - reboot;
  - update info;
  - firmware archive upload.
- Добавлены live firmware logs v1:
  - `/ws/logs`, если сборка имеет Qt WebSockets;
  - polling fallback, если Qt WebSockets отсутствует;
  - start/stop, pause/resume, clear, filter.
- Подготовлен firmware upgrade через `/ws/upgrade`:
  - optional Qt WebSockets в CMake;
  - `webSocketsAvailable`;
  - GitHub source flow;
  - uploaded `/tmp/firmware.tgz` flow;
  - progress output;
  - состояние flashing/rebooting;
  - CI/release устанавливают `qtwebsockets`.
- Добавлен firmware update safety-gate v1:
  - нормализация SoC/flash/firmware variant из update page;
  - блокировка update без `/ws/upgrade`, SoC/flash и выбранных kernel/rootfs;
  - отдельные опции kernel/rootfs/reset/force;
  - статусный checklist OK/WARN/BLOCK;
  - понятная причина блокировки update-кнопок.
- Добавлен базовый polling возврата камеры после `/ws/upgrade`:
  - после flashing/rebooting приложение ожидает доступность status endpoint;
  - показывает попытки возврата;
  - фиксирует успешное возвращение камеры или ошибку таймаута.
- Начата декомпозиция OpenIPC Update UI из `MajesticControlDialog.qml`:
  - `OpenIpcPageHeader.qml`;
  - `OpenIpcFirmwareStatusCardsGrid.qml`;
  - `OpenIpcInfoRowsCard.qml`;
  - `OpenIpcFirmwareQuickActionsCard.qml`;
  - `OpenIpcUpdateWarningCard.qml`;
  - `OpenIpcUpdateOptionsCard.qml`;
  - `OpenIpcUpgradeProgressPanel.qml`;
  - `OpenIpcFirmwareStatusGrid.qml`;
  - `OpenIpcUpdateChecklistGrid.qml`.
- Новые вынесенные OpenIPC QML-компоненты проверены отдельным `qmllint`.
- P2-декомпозиция `MajesticControlDialog.qml` завершена:
  - страницы вынесены в `MajesticOverviewPage.qml`, `MajesticSettingsPage.qml`, `OpenIpcStatusPage.qml`, `OpenIpcNetworkPage.qml`, `OpenIpcTimePage.qml`, `OpenIpcUpdatePage.qml`, `OpenIpcToolsPage.qml`, `MajesticEndpointsPage.qml`, `MajesticRawJsonPage.qml`, `MajesticMetricsPage.qml`;
  - повторяемые панели вынесены в `OpenIpcLogsPanel.qml`, `MajesticBackupRestorePanel.qml`, `MajesticRollbackBanner.qml`;
  - корневой диалог оставлен controller-слоем: состояние, API-вызовы, таймеры, Connections и подтверждающие диалоги;
  - новые компоненты подключены в QML module и проходят `qmllint` без предупреждений.

### Release / CI

- Подготовлены release notes для `0.2.4`.
- Исправлена Linux CI-зависимость, из-за которой падала установка GStreamer dev-пакетов.
- Release `0.2.4` собран.
- CI/release workflows обновлены для Qt WebSockets.
- Подготовлена pre-release линия `0.2.5-pre.1`:
  - `RELEASE_NOTES.md` обновлён под WebSockets-enabled GitHub artifacts;
  - release workflow помечает теги с `-pre/-rc/-beta/-alpha` как GitHub pre-release;
  - project version поднят до `0.2.5`.
- Добавлен update-checker приложения:
  - периодическая проверка `OpenIPC/dashboard` GitHub Releases;
  - подключение к существующей кнопке “Проверить обновления” в настройках;
  - модальное окно с release notes со страницы релиза;
  - действия “Открыть релиз”, “Напомнить позже”, “Пропустить эту версию”;
  - semver-сравнение stable/pre-release тегов.
- Исправлен Linux fallback для RAM в status bar приложения:
  - `/proc/self/status` / `VmRSS`;
  - fallback через `/proc/self/statm`;
  - fallback через `getrusage(RUSAGE_SELF)`.

## Что осталось сделать

### P0 — стабилизация текущего большого набора изменений

- Прогнать GitHub Actions после добавления `qtwebsockets` и убедиться, что Windows/Linux действительно собирают WebSocket-enabled build.
- Проверить на реальной OpenIPC-камере:
  - `/ws/logs`;
  - `/ws/upgrade` GitHub flow;
  - `/ws/upgrade` uploaded archive flow;
  - поведение при обрыве сети/закрытии WebSocket.
- Провести визуальную QA-проверку OpenIPC Control Center на RU/EN.
- Убедиться, что fallback без Qt WebSockets остаётся рабочим.
- Зафиксировать текущий большой набор изменений аккуратным коммитом после проверки и опубликовать тег `v0.2.5-pre.1`.

### P1 — Majestic/OpenIPC hardening

- Firmware upgrade safety layer:
  - усилить проверку SoC/flash/firmware variant до строгой совместимости archive/image;
  - проверка размера архива;
  - checksum/signature, если camera/update page отдаёт эти данные;
  - предупреждение по питанию/сети;
  - отдельное явное подтверждение dangerous options reset/force.
- Расширить polling возврата камеры после `/ws/upgrade`:
  - HTTP WebUI probe уже есть через status endpoint, добавить отдельный health summary;
  - Majestic API probe;
  - RTSP probe;
  - понятный статус “камера прошивается / перезагружается / вернулась / не вернулась”.
- Safe rollback v2 для Majestic:
  - post-apply health probe;
  - таймер наблюдения после reload pipeline;
  - предложение rollback, если поток/API не восстановились;
  - авто-rollback только если камера всё ещё доступна по API и пользователь разрешил.
- Firmware backup/restore:
  - улучшить карточку firmware backup;
  - добавить restore/upload backup только с жёсткими подтверждениями;
  - разделить Majestic config backup и полный OpenIPC flash/overlay backup визуально.
- Live logs v2:
  - экспорт live logs в файл;
  - severity highlighting;
  - фильтры all/majestic/kernel;
  - ring-buffer настройки.

### P2 — качество QML и дальнейшая полировка

- Правило для нового функционала: если фича требует отдельного состояния, нескольких визуальных блоков или приближается к ~200-300 строкам, сразу заводить отдельный QML-компонент и, при необходимости, отдельный C++/QML controller/model, а не раздувать главный файл.
- После крупных функциональных изменений не возвращать UI-блоки в `MajesticControlDialog.qml`, а расширять существующие page/panel-компоненты или создавать новые.
- Постепенно снижать старый шум `qmllint` в корневом controller-диалоге:
  - deferred `contentItem` warning;
  - старые unqualified access warnings внутри timers/connections;
  - типовые предупреждения старых helper-функций.
- Добавить QML smoke-проверки для основных диалогов, если получится стабильно запускать headless/CI.

### P3 — Health Center v2

- Персистентная история проверок между запусками.
- Отдельные health-профили:
  - быстрый;
  - глубокий;
  - OpenIPC/Majestic;
  - RTSP-only.
- Deeper probes:
  - Majestic config/schema;
  - firmware status;
  - `/metrics`;
  - `/ws/logs` readiness;
  - disk/overlay/memory/temp.
- Health report в более структурированном формате: JSON + human-readable TXT.

### P4 — Поиск камер v2

- Подробный live-log этапов discovery в UI.
- Повторная проверка найденных устройств перед добавлением.
- Умная дедупликация одного устройства по IP/MAC/hostname/ONVIF UUID.
- Настраиваемые диапазоны сканирования.
- Более понятная confidence-модель: почему камера считается OpenIPC/ONVIF/RTSP.

### P5 — Live preview v2

- Tooltip/пояснение для `Active / Pause / Limit`.
- Preview budget modes:
  - auto;
  - economy;
  - maximum.
- Улучшить приоритеты preview:
  - выбранная камера;
  - fullscreen;
  - тревожная;
  - недавно добавленная;
  - offline/ошибка.
- Показывать причину паузы preview прямо в ячейке.

### P6 — UX и полировка

- Причесать все крупные диалоги под единый визуальный язык Control Center.
- Добавить больше понятных empty states.
- Привести кнопки опасных операций к единому паттерну:
  - warning copy;
  - confirm dialog;
  - progress;
  - recovery hint.
- Улучшить onboarding: нашёл → проверил → добавил → показал → предложил health check.

### P7 — Тесты

- Unit-тесты для:
  - firmware update payload;
  - WebSocket availability fallback;
  - Majestic rollback patch;
  - backup restore diff;
  - endpoint list generation, если вынесем в C++/модель.
- Интеграционный smoke для firmware-client HTTP parsing.
- Регрессионные тесты discovery/parser/status policy.

### P8 / Future — Web Server + Web Client

Идея: добавить режим, в котором desktop-приложение на ПК работает как основной сервер, а другие устройства в локальной сети или через статический IP/домен открывают web-страницу с максимально полным повторением функционала Dashboard.

Базовая архитектура:

```text
Камеры OpenIPC / ONVIF / RTSP
        ↓
ПК-сервер с OpenIPC Dashboard
        ↓
Встроенный HTTP/WebSocket API server
        ↓
Web UI в браузере телефона / планшета / другого ПК
```

Ключевые принципы:

- Desktop-приложение остаётся главным backend-узлом:
  - хранит камеры, группы, пользователей и настройки;
  - выполняет discovery, health-check, Majestic/OpenIPC API, firmware operations;
  - управляет правами доступа и аудитом действий;
  - при необходимости проксирует или подготавливает видеопотоки для браузера.
- Web UI — отдельный клиент, повторяющий дизайн и сценарии desktop UI.
- QML-интерфейс нельзя просто “открыть в браузере” как есть; потребуется отдельный web frontend.
- Backend-логику C++ желательно переиспользовать через внутренний API-слой.

План реализации по этапам:

1. Встроенный HTTP/WebSocket сервер:
   - авторизация;
   - REST/WebSocket API для камер, статусов, групп, health, логов;
   - настройка bind address/port;
   - режим LAN-only по умолчанию.
2. Web Dashboard v1:
   - список камер;
   - online/offline/attention статусы;
   - grid layout;
   - базовый live-view;
   - быстрые действия по камере.
3. Видео для браузера:
   - использовать HLS/MJPEG/WebRTC, если камера/Majestic отдаёт подходящий поток;
   - добавить server-side proxy только там, где это безопасно и оправдано;
   - отдельно исследовать WebRTC gateway для низкой задержки.
4. Web OpenIPC/Majestic Control Center:
   - status/network/time/logs/backup/reboot/update;
   - Majestic settings;
   - endpoints/capabilities;
   - безопасные подтверждения опасных операций.
5. Пользователи, роли и безопасность:
   - HTTPS/TLS;
   - session/token auth;
   - роли viewer/operator/admin;
   - защита WebSocket/API;
   - audit log для опасных действий.
6. Удалённый доступ:
   - reverse proxy/VPN как рекомендуемый безопасный сценарий;
   - прямой доступ по статическому IP только после включения HTTPS и сильной авторизации;
   - документация по безопасной публикации наружу.

Основные сложности:

- Браузер не воспроизводит RTSP напрямую, поэтому потребуется HLS/MJPEG/WebRTC или проксирование.
- Полное повторение desktop-функционала — крупный milestone, а не маленькая доработка.
- Доступ через интернет требует отдельной модели безопасности.
- Нужно не дублировать бизнес-логику, а вынести её в общий backend/API-слой.

Ориентировочный масштаб: отдельное направление уровня `0.3.x`/`0.4.0`, после стабилизации текущего OpenIPC/Majestic/Firmware функционала.

## Ближайший рекомендуемый порядок

1. Проверить GitHub Actions с `qtwebsockets`.
2. Провести ручную проверку OpenIPC Control Center на реальной камере.
3. Закоммитить текущий большой пакет изменений.
4. Начать декомпозицию `MajesticControlDialog.qml`, потому что файл снова стал слишком крупным.
5. После декомпозиции добить `qmllint` warnings уже по новым маленьким компонентам.
