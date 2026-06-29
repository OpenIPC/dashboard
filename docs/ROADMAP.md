# OpenIPC Dashboard Roadmap

Этот список закрепляет текущий план развития приложения после анализа OpenIPC Viewer и Majestic WebUI.

## Текущий статус

- Пункт 1 закрыт для текущего этапа: общий UI-набор расширен, повторяющиеся dashboard-кнопки и статусные бейджи вынесены в компоненты, Dashboard/Health используют единый визуальный статус. Глубокое физическое дробление `DashboardView.qml` на более мелкие файлы остаётся как техническая чистка без изменения поведения.
- Пункт 2 закрыт: effective status, online/offline, attention reason и search text живут в едином C++-слое, Dashboard/Sidebar/Health Center используют одни и те же правила, добавлены unit-тесты.

## 1. Dashboard и структура QML

- Вынести крупные части `DashboardView.qml` в отдельные компоненты.
- Вынести редактор раскладок в отдельный `LayoutEditorDialog.qml`.
- Вынести список устройств и групп в отдельные компоненты.
- Вынести повторяющиеся кнопки, карточки, бейджи и диалоги в единый UI-набор.
- Убрать implicit `model`/`index` в делегатах через `required property`.
- Убрать старые `qmllint info`: `parent.*`, layout `width/height`, deferred `contentItem`, неявные обращения.

## 2. Единый слой статусов камер

- Держать вычисление effective status в одном C++-слое.
- Добавить unit-тесты на сценарии online/offline/attention.
- Гарантировать совпадение статусов в ячейках, сайдбаре и Health Center.
- Сделать offline/ошибки потока приоритетнее устаревшего optimistic online из списка камер.
- Отображать status reason одинаково во всех местах UI: RTSP timeout, HTTP unavailable, auth error, stream stalled.

## 3. Health Center

- Фильтры: offline, с ошибками, в сетке, не в сетке.
- Массовая перепроверка offline-камер.
- Экспорт диагностического отчёта в файл.
- История последних проверок.

## 4. Majestic API / OpenIPC

- Завершить локализации и подсказки для всех настроек Majestic.
- Добавить тесты на diff patch без `null`.
- Улучшить apply-flow: проверка → сохранение → reload/restart → переподключение.
- Явно показывать настройки, требующие reload/restart.
- Улучшить backup/restore UI.
- Сделать страницу endpoints/capabilities в нормальном виде.
- Добавить безопасный rollback, если камера не вернулась после применения.

## 5. Поиск камер

- Поддерживать mDNS/OpenIPC, WS-Discovery/ONVIF, RTSP probe, HTTP/Majestic probe.
- Добавить быстрый и глубокий режим поиска.
- Добавить подробный лог этапов поиска.
- Убирать progress bar после завершения во всех edge-case.

## 6. Live preview

- Добавить tooltip для `Active / Pause / Limit`.
- Добавить preview budget modes: auto, economy, maximum.
- Улучшить приоритеты preview: активная, выбранная, тревожная, offline.
- Показывать причину паузы preview.

## 7. UX и единый визуальный язык

- Единый стиль кнопок, карточек, статусов и бейджей.
- Единые empty states и подсказки.
- Причесать диалоги под один визуальный язык.

## 8. Тесты и качество

- Расширить unit-тесты для статусов, Majestic patch, discovery parser, stream health, layout templates.
- Добавить QML smoke-сценарии, где это возможно.
- Постепенно снижать количество `qmllint info`.

## 9. Viewer-inspired функции

- Более удобная панель устройств.
- Улучшенное управление раскладками.
- Быстрые действия по камере из контекстного меню.
- Более сильный слой health/diagnostics.
- Более понятный onboarding: нашёл → проверил → добавил → показал.
