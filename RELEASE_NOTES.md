# OpenIPC Dashboard v0.2.5

`v0.2.5` is a stable release focused on turning the experimental OpenIPC / Majestic work from `0.2.4` and the `0.2.5-pre.*` line into a practical, safer control center for real cameras.

The main theme of this release is control from one place: camera discovery, live preview, Majestic configuration, OpenIPC firmware tools, firmware logs, firmware upgrade and application self-update are now tied together inside Dashboard.

## Главное

- Добавлен единый OpenIPC Control Center внутри окна Majestic/Firmware.
- Реализованы реальные firmware read/write операции: status, network, time, logs, backup, reboot, update info и firmware upload.
- В release-сборках включён Qt WebSockets, что открывает полноценную работу с `/ws/logs` и `/ws/upgrade`.
- Подтверждён firmware update реальной OpenIPC-камеры прямо из приложения через `/ws/upgrade` с подробным логом процесса.
- Усилен safety-layer обновления прошивки: совместимость, размер архива, checksum/signature где доступны, подтверждения питания/сети и dangerous options.
- Улучшен post-upgrade контроль возврата камеры: polling status endpoint, Majestic API probe, RTSP main/sub probe и итоговый health summary.
- Доработан Majestic rollback v2 для критичных настроек.
- Улучшены live firmware logs: WebSocket/polling режимы, фильтры, подсветка severity, экспорт в файл и настройка ring-buffer.
- Добавлен in-app updater приложения: проверка GitHub Releases, release notes popup, загрузка с прогрессом, запуск установки и очистка временных файлов.
- Завершено дробление `MajesticControlDialog.qml` на отдельные QML page/panel компоненты.
- Улучшен интерфейс Dashboard: Metro-like sidebar, компактные действия, layout toolbar, стабильный device list, исправления Grid.
- Исправлен fallback отображения RAM на Linux.

## OpenIPC Control Center / Firmware

- Добавлены отдельные страницы управления:
  - Status;
  - Network;
  - Time;
  - Update;
  - Tools;
  - Logs.
- Firmware-клиент умеет читать и применять настройки сети и времени.
- Добавлены NTP sync, установка времени с ПК и reset формы.
- Добавлены firmware backup и безопасный переход к штатному restore в WebUI после жёсткого подтверждения.
- Добавлены reboot-команды с подтверждением.
- Добавлен просмотр update information с SoC, flash, firmware variant, kernel/rootfs и дополнительными данными камеры.
- Добавлена подготовка и запуск update flow для GitHub firmware source и uploaded archive.

## Firmware upgrade safety

- Uploaded archive проверяется до загрузки:
  - файл существует;
  - расширение `.tgz`, `.tar.gz` или `.gz`;
  - размер находится в допустимом диапазоне `0..128 MB`.
- Checklist сравнивает доступные признаки SoC, flash и firmware variant.
- Имя uploaded archive проверяется на очевидные NOR/NAND и lite/ultimate mismatch.
- Если update page отдаёт checksum/signature, они показываются в safety checklist.
- Update блокируется без подтверждения стабильного питания и сети.
- Опции `reset` и `force` требуют отдельного явного подтверждения.
- После flashing/rebooting приложение переходит в фазы:
  - probing;
  - waiting;
  - validating;
  - online;
  - degraded;
  - failed.
- После возврата камеры автоматически обновляются status, network, time, update info и metrics.
- Дополнительно запускаются Majestic API и RTSP main/sub проверки.

## Majestic API

- Настройки Majestic строятся по schema конкретной камеры.
- Вкладка “Все настройки” заменена понятными разделами:
  - Изображение;
  - Видео и аудио;
  - События;
  - Запись;
  - Сеть и интеграции;
  - Система.
- Известные поля получили RU/EN названия и подсказки.
- Apply использует schema-safe diff и больше не отправляет `null` в patch.
- Критичные настройки явно помечаются как требующие reload pipeline.
- После apply/reload запускается наблюдение за состоянием API и потока.
- Rollback snapshot сохраняется перед критичным apply.
- Если API или поток не восстановились, показывается rollback banner.
- Auto-rollback возможен только если пользователь явно включил его и API камеры остаётся доступен.
- Majestic JSON backup визуально отделён от полного OpenIPC firmware/overlay backup.

## Live logs

- `/ws/logs` используется в WebSocket-enabled сборках.
- Если Qt WebSockets недоступен, сохраняется polling fallback.
- Добавлены start/stop, pause/resume, clear и фильтрация.
- Добавлены source filters:
  - all;
  - majestic;
  - kernel.
- Добавлена подсветка error/warn/majestic/kernel строк.
- Добавлен экспорт текущих или фильтрованных логов в `.log/.txt`.
- Добавлена настройка OpenIPC syslog ring-buffer через firmware-client.

## Application updater

- Приложение проверяет новые версии в GitHub Releases репозитория `OpenIPC/dashboard`.
- Кнопка “Проверить обновления” в настройках подключена к реальному update-checker.
- При наличии новой версии показывается модальное окно с release notes.
- Можно открыть релиз, пропустить версию, напомнить позже или скачать обновление.
- Добавлена загрузка обновления прямо из приложения:
  - прогресс загрузки;
  - отмена;
  - выбор совместимого asset под текущую платформу;
  - защита от старой несовместимой ветки релизов `2.9.0`.
- Windows:
  - скачивается `OpenIPC-Dashboard-Installer.exe`;
  - запускается installer handoff;
  - временный установщик удаляется после завершения.
- Linux AppImage:
  - скачивается `OpenIPC-Dashboard-Linux.AppImage`;
  - после выхода приложения выполняется замена текущего AppImage;
  - временный файл удаляется;
  - приложение перезапускается.

## Camera discovery and live preview

- Улучшен поиск OpenIPC-камер в сети.
- Используются OpenIPC/mDNS, WS-Discovery/ONVIF, RTSP probe и HTTP/Majestic probe.
- Добавлен быстрый и глубокий сценарии поиска.
- Progress bar поиска показывает этап и процент.
- После завершения поиска progress bar скрывается.
- Исправлена работа SD/substream для OpenIPC/Majestic.
- HD/fullscreen и SD/preview разведены корректнее.
- Подпись видеопотока приведена к единому виду: codec, resolution, bitrate, FPS.
- Статусы preview и камеры синхронизированы через общий status-layer.

## Dashboard UI

- Крупные части `DashboardView.qml` вынесены в отдельные QML-компоненты.
- Редактор раскладок вынесен в `LayoutEditorDialog.qml`.
- Исправлены проблемы Grid после дробления Dashboard.
- Исправлено мигание камеры в списке устройств при hover по action-кнопкам.
- Sidebar переработан в более строгий Viewer-like / Metro-like стиль:
  - компактная сетка действий;
  - ровные иконки и подписи;
  - меньше визуального шума;
  - layout actions встроены в toolbar раскладок.
- Device list item вынесен в стабильную карточку с зарезервированной зоной quick actions.
- Empty state Dashboard получил короткий сценарий быстрого старта.
- Новые строки локализованы RU/EN.

## QML architecture

- Завершена P2-декомпозиция `MajesticControlDialog.qml`.
- В отдельные компоненты вынесены:
  - `MajesticOverviewPage.qml`;
  - `MajesticSettingsPage.qml`;
  - `MajesticEndpointsPage.qml`;
  - `MajesticRawJsonPage.qml`;
  - `MajesticMetricsPage.qml`;
  - `OpenIpcStatusPage.qml`;
  - `OpenIpcNetworkPage.qml`;
  - `OpenIpcTimePage.qml`;
  - `OpenIpcUpdatePage.qml`;
  - `OpenIpcToolsPage.qml`;
  - `OpenIpcLogsPanel.qml`;
  - `OpenIpcFirmwareBackupPanel.qml`;
  - `MajesticBackupRestorePanel.qml`;
  - `MajesticRollbackBanner.qml`.
- Корневой Majestic dialog оставлен controller-слоем для state, API calls, timers, connections и подтверждений.
- Новые извлечённые QML-компоненты проходят targeted `qmllint`.

## Linux

- Исправлен fallback RAM в status bar:
  - основной источник: `/proc/self/status` / `VmRSS`;
  - fallback: `/proc/self/statm`;
  - fallback: `getrusage(RUSAGE_SELF)`.
- Это закрывает сценарий, где CPU отображался, а RAM оставалась `0 MB`.
- Linux CI dependency issue с `libgstreamer1.0-dev` закрыт добавлением нужной зависимости.

## CI / Release

- Release workflow собирает Windows installer и Linux AppImage.
- В CI/release workflow добавлен Qt WebSockets module.
- Теги `v*-pre*`, `v*-rc*`, `v*-beta*`, `v*-alpha*` автоматически публикуются как GitHub pre-release.
- Финальный тег `v0.2.5` публикуется как stable release.

## Проверка

Перед публикацией локально проверено:

- Release build: успешно.
- `ctest`: 14/14 успешно.
- Targeted `qmllint` для новых/извлечённых Majestic/OpenIPC компонентов: успешно.
- `git diff --check`: без ошибок whitespace.
- На реальной OpenIPC-камере подтверждён firmware update через `/ws/upgrade`.

## Важно

- Firmware upgrade остаётся опасной операцией: перед обновлением делайте backup, проверяйте модель/SoC/flash и обеспечьте стабильное питание.
- Full restore полного OpenIPC backup намеренно не автоматизирован через непроверенный endpoint: приложение открывает штатный WebUI restore после подтверждения.
- В старых QML-файлах ещё остаются legacy `qmllint` warnings. Новые компоненты держим чистыми и продолжаем постепенно снижать старый шум.
