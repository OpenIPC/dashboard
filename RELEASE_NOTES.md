# OpenIPC Dashboard v0.2.3

## English

OpenIPC Dashboard 0.2.3 is a major usability and analytics update. It makes camera monitoring more reliable, turns AI analytics into a manageable workflow, adds Camex remote-access tooling, and polishes the application across Windows and Linux.

### AI analytics

- Reworked the Analytics workspace into clear Overview, Cameras, Modules, Events, Rules, Archive, and Diagnostics sections.
- Added per-camera AI module assignment with visible readiness and activity states.
- Added full model lifecycle controls: installation, removal, clean reload, model folder access, backend status, file size, and diagnostics.
- Added real model download progress based on transferred bytes, plus HTTP timeouts, retries, redirect handling, and reliable GitHub downloads.
- Improved analytics performance with frame throttling, configurable target FPS, parallel job limits, and detailed runtime telemetry.
- Added persistent SQLite event storage, filtering, event details, evidence snapshots/clips, and clearer empty states.
- Stabilized the analytics rule editor and fixed a crash when creating rules.
- Improved rule configuration for triggers, confidence, duration, zones, snapshots, clips, and notifications.

### Camera monitoring

- Fixed camera online/offline status so an active video stream is reflected correctly in the device list.
- Improved camera discovery and moved network search into the main sidebar controls.
- Added clearer onboarding for an empty layout, with close and “do not show again” controls.
- Improved camera cells, stream overlays, device counters, and sidebar state persistence.

### Camex integration

- Added a dedicated Camex workspace for configuring remote access to OpenIPC cameras through UDP/TCP tunnels.
- Added guided server and camera setup, generated commands and configuration, connection checks, and links to Camex releases and documentation.
- Styled and localized Camex to match the rest of the application.

### Interface and localization

- Reworked analytics, settings, layout editing, archive, dialogs, and sidebar controls for a consistent dark interface.
- Improved responsive window resizing and restored window/sidebar state across launches.
- Fixed live language switching and expanded Russian and English localization across newly added and existing screens.
- Improved dropdown indicators, text contrast, scrollbars, spacing, control sizing, and empty states.

### Authentication and system fixes

- Added remembered credentials and automatic sign-in when “Remember me” is enabled.
- Improved password storage and compatibility with existing user data.
- Fixed Linux absolute-path handling for recordings, snapshots, and analytics evidence.
- Improved CPU and memory reporting on Linux.
- Added regression tests for authentication and cross-platform path handling.

### Build and release

- Improved Windows dependency deployment and GitHub Actions compatibility with current CMake, OpenSSL, GStreamer, and MinGW environments.
- The release includes a Windows installer and Linux AppImage.

## Русский

OpenIPC Dashboard 0.2.3 — крупное обновление удобства и видеоаналитики. Мониторинг камер стал надёжнее, работа с ИИ-модулями превратилась в понятный управляемый процесс, появилась интеграция Camex, а интерфейс Windows- и Linux-версий получил комплексную доработку.

### ИИ-аналитика

- Страница аналитики переработана и разделена на понятные вкладки: «Обзор», «Камеры», «Модули», «События», «Правила», «Архив» и «Диагностика».
- Добавлено назначение ИИ-модулей отдельным камерам с отображением готовности и активности.
- Реализовано полное управление моделями: установка, удаление, чистая перезагрузка, открытие папки моделей, статус backend, размер файла и диагностика.
- Добавлен реальный прогресс загрузки моделей по переданным байтам, тайм-ауты, повторные попытки, обработка перенаправлений и стабильное скачивание с GitHub.
- Улучшена производительность аналитики: ограничение частоты кадров, настройка целевого FPS, лимит параллельных задач и подробная телеметрия.
- Добавлено постоянное SQLite-хранилище событий, фильтрация, подробности события, снимки и клипы-доказательства, а также понятные пустые состояния.
- Исправлено падение приложения при добавлении правила и стабилизирован редактор правил.
- Улучшена настройка триггеров, порога уверенности, длительности, зон, снимков, клипов и уведомлений.

### Мониторинг камер

- Исправлен статус камер: активный видеопоток теперь корректно отображается как состояние «Онлайн» в списке устройств.
- Улучшен поиск камер в сети, кнопка поиска перенесена в основную панель управления.
- Добавлена понятная подсказка для пустой раскладки с закрытием и настройкой «Не показывать при следующем запуске».
- Улучшены ячейки камер, информационные оверлеи, счётчики устройств и сохранение состояния сайдбара.

### Интеграция Camex

- Добавлен отдельный раздел Camex для настройки удалённого доступа к OpenIPC-камерам через UDP/TCP-туннели.
- Реализованы пошаговая настройка сервера и камеры, генерация команд и конфигурации, проверка соединения и ссылки на релизы и документацию Camex.
- Интерфейс Camex приведён к общему стилю приложения и полностью локализован.

### Интерфейс и локализация

- Переработаны аналитика, настройки, редактор раскладок, архив, диалоги и элементы сайдбара в едином тёмном стиле.
- Улучшено адаптивное изменение размеров окна и восстановление состояния окна и сайдбара между запусками.
- Исправлено переключение языка без перезапуска, расширены русская и английская локализации новых и существующих экранов.
- Улучшены индикаторы выпадающих списков, контраст текста, полосы прокрутки, отступы, размеры элементов и пустые состояния.

### Авторизация и системные исправления

- Добавлено запоминание учётных данных и автоматический вход при включённой настройке «Запомнить меня».
- Улучшено хранение паролей и совместимость с существующими пользовательскими данными.
- Исправлена обработка абсолютных Linux-путей для записей, снимков и материалов аналитики.
- Улучшено отображение загрузки процессора и оперативной памяти в Linux.
- Добавлены регрессионные тесты авторизации и кроссплатформенной обработки путей.

### Сборка и релиз

- Улучшено развёртывание зависимостей Windows и совместимость GitHub Actions с актуальными версиями CMake, OpenSSL, GStreamer и MinGW.
- В релиз входят установщик для Windows и Linux AppImage.
