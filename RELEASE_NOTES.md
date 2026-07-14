# OpenIPC Dashboard v0.2.6.1

OpenIPC Dashboard v0.2.6.1 is a maintenance and reliability release that completes P8 polish/observability, P9 archive and recording evolution, and P10 analytics/reliability work while fixing the latest startup and navigation regressions reported after v0.2.6.

## Русский

### Главное

- Полностью завершён P8: рабочее логирование приложения, дальнейшая декомпозиция backend/QML, QML runtime hardening и очистка CI.
- Полностью завершён P9: единый каталог записей, обновлённый архив, экспорт клипов, контроль хранилища и телеметрия записи.
- Полностью завершён P10: пользовательские зоны аналитики, надёжная доставка evidence, безопасное завершение и восстановление записей, дальнейшая декомпозиция крупных компонентов.
- Исправлены критические ошибки запуска приложения и открытия Camex.
- Улучшена навигация с клавиатуры в окне входа: `Tab`, `Shift+Tab` и `Enter` позволяют пройти форму без мыши.

### Архив и запись

- Добавлен единый каталог ручных и событийных записей с поддержкой MP4, MKV, AVI и MOV.
- Сохранена совместимость со старыми именами файлов; новый формат использует миллисекунды и устойчив к быстрым повторным запускам записи.
- Архив показывает реальный путь, источник, размер, длительность, камеру и группировку по датам.
- Добавлены быстрые периоды, точный календарный диапазон, фильтр `Все / Ручные / События` и сортировка по времени.
- Интерфейс архива разделён на самостоятельные панели поиска, результатов, воспроизведения, экспорта и хранилища.
- Добавлены прогресс экспорта, понятные ошибки ffmpeg и быстрое открытие каталога результата.
- Реализованы подсчёт занятого места, безопасный dry-run очистки и retention-политики по возрасту и размеру.
- Добавлено логирование старта, остановки, ошибки и ротации ручных и событийных записей.
- Пользователь может задавать длительность сегмента записи от 5 до 60 минут с шагом 5 минут.
- Панели периода и хранилища сделаны компактными и сворачиваемыми, чтобы список записей занимал основную высоту sidebar.

### Логи, настройки и интерфейс

- Кнопка `Логи` теперь открывает реальный журнал приложения: сообщения пишутся в `app.log`, отображаются в `LogView`, фильтруются и экспортируются.
- При открытии журнала загружается хвост существующего файла; добавлены пустое состояние и ручное обновление.
- Проведён аудит настроек: значения связаны с backend-параметрами и корректно восстанавливаются после перезапуска.
- Все пользовательские checkbox-компоненты приведены к единому MetroUI-стилю без наложения системного индикатора.
- Переключатели evidence в настройках аналитики получили понятные подписи и подсказки на русском и английском языках.
- В архиве текстовые переключатели заменены компактными иконками календаря и меню.
- Исправлены размеры и null-safe bindings окна логов, обнаруженные расширенными smoke-тестами.

### Аналитика и надёжность

- Редактор правил поддерживает пользовательские полигональные зоны из 3-8 точек и сохраняет их вместе с правилом.
- Совпадение детекций с зонами вынесено в отдельный тестируемый модуль с поддержкой старых пресетов зон.
- Доставка снимков и клипов получила повторные попытки с увеличивающейся задержкой, таймауты, счётчики и видимый последний статус.
- Локальные и NAS-копии сначала пишутся во временный `.part`-файл и публикуются атомарным переименованием.
- Экспорт и запись защищены от незавершённых файлов; архив находит и безопасно очищает устаревшие `.part`, `.tmp` и `.previous` артефакты.
- Удалён неиспользуемый legacy `AnalyticsModel`; панели зон и загрузки evidence вынесены в самостоятельные QML-компоненты.

### Исправления стабильности

- Исправлен crash при запуске приложения после обновления настроек аналитики.
- Исправлен crash при нажатии кнопки `Camex` в sidebar.
- Исправлены некорректные QML-связи и жизненный цикл модальных окон Camex.
- Улучшена последовательность фокуса формы входа для существующего пользователя и первого запуска.
- Windows QML smoke-тесты запускают GUI-процесс контролируемо, сохраняют stdout/stderr и корректно завершаются по таймауту.

### Проверка релиза

- Полная Release-сборка приложения.
- 24 C++ unit-теста.
- QML smoke-тесты в обычном режиме и при `QT_SCALE_FACTOR=1.5`.
- Targeted QML lint и проверка каталога локализации.
- Всего: 27 из 27 локальных тестов проходят.
- GitHub Actions собирает Windows installer и Linux AppImage перед публикацией релиза.

### Ограничения

- Web/server mode P6 остаётся в backlog.
- Автоматический firmware rollback и непроверенный full restore не выполняются.
- Интеграция `ipctool` намеренно не включена.

## English

### Highlights

- Completed P8: application logging, further backend/QML decomposition, QML runtime hardening, and CI cleanup.
- Completed P9: unified recording catalog, Archive UI v2, clip export, storage management, and recording telemetry.
- Completed P10: custom analytics zones, reliable evidence delivery, safe recording finalization and recovery, and further component decomposition.
- Fixed critical application startup and Camex dialog crashes.
- Added complete keyboard navigation to the login form with `Tab`, `Shift+Tab`, and `Enter`.

### Archive And Recording

- Added a unified catalog for manual and analytics-triggered recordings with MP4, MKV, AVI, and MOV support.
- Preserved legacy filename compatibility while the new millisecond-based format prevents collisions during rapid recording restarts.
- Archive entries now expose the real path, source, size, duration, camera, and date grouping.
- Added quick periods, an exact calendar range, `All / Manual / Events` filtering, and time sorting.
- Split the archive UI into focused search, results, playback, export, and storage components.
- Added export progress, actionable ffmpeg errors, and a shortcut to the result directory.
- Added storage usage summaries, safe cleanup dry runs, and age/size retention policies.
- Added application-log telemetry for manual and event recording start, stop, failure, and segment rotation.
- Recording segment duration is configurable from 5 to 60 minutes in 5-minute steps.
- Period and storage panels are compact and collapsible so recordings retain most of the sidebar height.

### Logs, Settings, And UI

- The `Logs` action now opens a real application log viewer backed by `app.log`, with filtering, refresh, empty state, and export.
- Existing log tails are loaded at startup and whenever the viewer is opened.
- Audited settings bindings so values control their intended backend parameters and survive application restart.
- Standardized user-facing checkboxes on the MetroUI style without overlapping native indicators.
- Added localized labels and tooltips for analytics evidence switches.
- Replaced text archive toggles with compact calendar and menu icons.
- Fixed LogView sizing and null-safe bindings exposed by expanded smoke coverage.

### Analytics And Reliability

- Rules now support persisted custom polygon zones with 3-8 draggable points.
- Detection-to-zone matching is isolated in a tested component while retaining legacy zone presets.
- Snapshot and clip delivery now has bounded exponential retries, timeouts, counters, and a visible last status.
- Local and NAS uploads write to temporary `.part` files before atomic publication.
- Export and recording finalization protect completed targets; Archive can find and safely clean stale `.part`, `.tmp`, and `.previous` artifacts.
- Removed the unused legacy `AnalyticsModel` and extracted the zone editor and evidence upload settings into focused QML components.

### Stability Fixes

- Fixed an application startup crash introduced around analytics settings.
- Fixed a crash when opening Camex from the sidebar.
- Fixed invalid QML bindings and Camex modal lifecycle handling.
- Improved initial focus and focus order for existing-user login and first-run account setup.
- Windows QML smoke tests now launch the GUI process deterministically, capture stdout/stderr, and enforce a reliable timeout.

### Release Validation

- Full Release build.
- 24 C++ unit tests.
- QML smoke tests at the default scale and `QT_SCALE_FACTOR=1.5`.
- Targeted QML lint and localization catalog validation.
- All 27 local tests pass.
- GitHub Actions must complete the Windows installer and Linux AppImage jobs before publishing the release.

### Scope

- P6 Web/server mode remains in the backlog.
- Automatic firmware rollback and unverified full restore remain disabled.
- `ipctool` integration is intentionally excluded.
