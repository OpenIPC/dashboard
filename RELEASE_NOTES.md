# OpenIPC Dashboard v0.2.5-pre.1

This is a pre-release build focused on stabilizing the large OpenIPC / Majestic Control Center update after `0.2.4`.

The main goal of this pre-release is to ship GitHub Actions artifacts built with Qt WebSockets enabled, so `/ws/logs` and the prepared `/ws/upgrade` firmware flow can be tested on real OpenIPC cameras.

## Highlights

- Enabled GitHub release builds with Qt WebSockets support.
- Added GitHub release workflow handling for prerelease tags such as `v0.2.5-pre.1`.
- Added an in-app GitHub Releases update checker.
- Added an update notification dialog with release notes from the GitHub Release page.
- Completed the P2 split of `MajesticControlDialog.qml` into focused page/panel QML components.
- Kept the main Majestic dialog as a controller layer for state, API calls, timers, connections and confirmations.
- Fixed Linux process RAM reporting fallback so the status bar no longer stays at `0 MB` when `/proc/self/status` parsing is unavailable.
- Preserved the OpenIPC Control Center / Firmware workflow added after `0.2.4`: status, network, time, logs, backup, reboot, update information and firmware upload.
- Preserved firmware WebSocket upgrade preparation: GitHub firmware source flow, uploaded archive flow, progress output and reboot/polling state.

## Majestic / OpenIPC Control Center

- Majestic pages are now split into reusable components:
  - overview;
  - schema-driven settings;
  - endpoints/capabilities;
  - raw JSON;
  - metrics.
- OpenIPC firmware pages are now split into reusable components:
  - status;
  - network;
  - time;
  - update;
  - tools;
  - live logs panel.
- Backup/restore and rollback panels are now separate components.
- New extracted QML components pass `qmllint`.

## Firmware and WebSockets

- Qt WebSockets are installed in CI/release workflows.
- The app can expose whether WebSockets are available in the current build.
- `/ws/logs` can use WebSocket mode when available, with polling fallback otherwise.
- `/ws/upgrade` support is prepared for GitHub firmware and uploaded archive flows.
- Firmware flashing is still guarded by safety checks and confirmations.

## Linux fix

- Fixed process RAM reporting on Linux:
  - primary source: `/proc/self/status` / `VmRSS`;
  - fallback: `/proc/self/statm`;
  - fallback: `getrusage(RUSAGE_SELF)`.
- This targets the issue where CPU usage was shown but RAM stayed at zero in the bottom status bar on some Linux systems.

## Application updates

- The app can periodically check `OpenIPC/dashboard` GitHub Releases.
- The existing “Check for updates” button in application settings now uses the real checker.
- When a newer version is found, the app shows a modal dialog with release notes from GitHub.
- Users can open the release page, skip the detected version, or be reminded later.
- Pre-release versions such as `0.2.5-pre.1` are detected and marked separately.

## Validation

Local validation before publishing:

- Release app build: passed.
- `ctest`: 13/13 passed.
- New extracted Majestic/OpenIPC QML components: `qmllint` passed.
- Diff whitespace check: passed.

## Known limitations

- This is a pre-release: firmware upgrade through `/ws/upgrade` should be tested carefully on real cameras only after backup and with stable power/network.
- Some firmware upgrade safety hardening is still planned for the next P1 pass: stronger archive compatibility checks, size/checksum validation where available and deeper post-upgrade health probes.
- Some old `qmllint` warnings still remain in the root controller dialog and will be cleaned gradually without moving UI blocks back into the main file.

---

## Русский

OpenIPC Dashboard `v0.2.5-pre.1` — предварительный релиз для стабилизации большого обновления OpenIPC / Majestic Control Center после `0.2.4`.

Главная цель этой pre-release сборки — получить артефакты GitHub Actions с включённым Qt WebSockets, чтобы можно было проверить `/ws/logs` и подготовленный `/ws/upgrade` firmware flow на реальных OpenIPC-камерах.

## Главное

- В release-сборках GitHub Actions включена поддержка Qt WebSockets.
- Release workflow теперь помечает теги вида `v0.2.5-pre.1` как GitHub pre-release.
- Добавлена проверка новых версий приложения через GitHub Releases.
- Добавлено всплывающее окно обновления с release notes со страницы GitHub Release.
- Завершено P2-дробление `MajesticControlDialog.qml` на отдельные QML-страницы и панели.
- Основной Majestic-диалог оставлен controller-слоем: состояние, API-вызовы, таймеры, connections и подтверждающие диалоги.
- Исправлен fallback отображения RAM на Linux: нижний status bar больше не должен показывать `0 MB`, если парсинг `/proc/self/status` недоступен.
- Сохранён OpenIPC Control Center / Firmware workflow: status, network, time, logs, backup, reboot, update information и firmware upload.
- Сохранена подготовка WebSocket firmware upgrade: GitHub firmware source flow, uploaded archive flow, progress output и состояние reboot/polling.

## Majestic / OpenIPC Control Center

- Majestic UI вынесен в отдельные компоненты:
  - overview;
  - schema-driven settings;
  - endpoints/capabilities;
  - raw JSON;
  - metrics.
- OpenIPC firmware UI вынесен в отдельные компоненты:
  - status;
  - network;
  - time;
  - update;
  - tools;
  - live logs panel.
- Backup/restore и rollback вынесены в отдельные панели.
- Новые вынесенные QML-компоненты проходят `qmllint`.

## Firmware и WebSockets

- CI/release workflows устанавливают Qt WebSockets.
- Приложение умеет показывать, доступна ли WebSocket-сборка.
- `/ws/logs` использует WebSocket-режим, если он доступен, и polling fallback, если WebSockets отсутствуют.
- `/ws/upgrade` подготовлен для GitHub firmware flow и uploaded archive flow.
- Firmware flashing остаётся под safety checks и подтверждениями.

## Исправление Linux RAM

- Исправлено определение RAM процесса на Linux:
  - основной источник: `/proc/self/status` / `VmRSS`;
  - fallback: `/proc/self/statm`;
  - fallback: `getrusage(RUSAGE_SELF)`.
- Это закрывает проблему, когда CPU отображался, а RAM в нижнем status bar оставалась нулевой.

## Обновления приложения

- Приложение периодически проверяет релизы `OpenIPC/dashboard` на GitHub.
- Уже существующая кнопка “Проверить обновления” в настройках теперь подключена к настоящей проверке.
- При обнаружении новой версии показывается модальное окно с release notes из GitHub Release.
- Пользователь может открыть страницу релиза, пропустить найденную версию или отложить напоминание.
- Pre-release версии вроде `0.2.5-pre.1` определяются и помечаются отдельно.

## Проверка

Локальная проверка перед публикацией:

- Release-сборка приложения: успешно.
- `ctest`: 13/13 успешно.
- Новые вынесенные Majestic/OpenIPC QML-компоненты: `qmllint` успешно.
- Проверка diff на пробелы/конфликты: успешно.

## Известные ограничения

- Это предварительный релиз: firmware upgrade через `/ws/upgrade` нужно тестировать осторожно, только после backup и при стабильном питании/сети.
- Часть усиления firmware upgrade safety layer остаётся на следующий P1-проход: строгая совместимость архива, размер/checksum где доступны и более глубокие health-probes после прошивки.
- В корневом controller-диалоге ещё остаются старые предупреждения `qmllint`; они будут чиститься постепенно, без возврата UI-блоков обратно в главный файл.
