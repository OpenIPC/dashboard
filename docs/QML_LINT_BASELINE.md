# QML lint baseline

Дата актуализации: 2026-07-14.

Этот документ фиксирует текущее состояние legacy `qmllint` после релиза `v0.2.6.1` и завершения P10.

## Текущее состояние

Команда проверки:

```powershell
cmake --build build_release --target appOpenIPC-Dashboard_qmllint --config Release
```

Результат на 2026-07-14:

- полный `qmllint` пока падает на legacy baseline;
- до чистки было около `1322` предупреждений;
- после P10 осталось `1093` предупреждения (`-91` относительно предыдущего baseline);
- обычная сборка `Release` проходит;
- unit-тесты проходят: `24/24`;
- QML smoke проходит в обычном режиме и при `QT_SCALE_FACTOR=1.5`;
- отдельный `qml_lint_targeted` проходит без предупреждений для новых и существенно изменённых компонентов.

## Основные источники предупреждений

| Файл | Предупреждений | Комментарий |
| --- | ---: | --- |
| `src/ui/GridCell.qml` | 149 | Старый крупный экран видеоячейки, много unqualified access и unresolved C++ QML types в lint tooling. |
| `src/ui/analytics/ImageViewerWindow.qml` | 125 | Legacy analytics UI, требует отдельной типизации делегатов/контролов. |
| `src/ui/CameraSearchDialog.qml` | 69 | Диалог поиска камер и сложные delegate/model bindings. |
| `src/ui/MajesticControlDialog.qml` | 66 | Оставшийся крупный coordinator после безопасного дробления Majestic. |
| `src/ui/analytics/SnapshotBrowser.qml` | 53 | Legacy analytics browser. |
| `src/ui/DashboardView.qml` | 50 | Остаточные предупреждения shell после дробления Dashboard. |
| `src/ui/analytics/AnalyticsModulesPanel.qml` | 49 | Динамические module-card bindings и model roles. |
| `src/ui/ArchiveView.qml` | 42 | После декомпозиции P9 предупреждений заметно меньше, остался player/shell layer. |
| `src/ui/SettingsDialog.qml` | 41 | Coordinator настроек после выноса отдельных страниц и evidence upload panel. |
| `src/ui/AnalyticsView.qml` | 40 | Legacy shell analytics-раздела. |
| `src/ui/analytics/AnalyticsOverviewPanel.qml` | 38 | Динамические диагностические карты. |
| `src/ui/FileManagerDialog.qml` | 35 | Часть safe warnings исправлена, остаток требует отдельной ревизии старого FileManager. |

## Основные категории

- `Unqualified access` остается крупнейшим классом legacy-предупреждений.
- часть `Unresolved QML type` относится к динамическим C++/QML model roles и tooling Qt 6.4.
- `Property "text" not found on type "QQuickItem"` и родственные `parent.*`: остаточный legacy-класс.
- `Cannot defer property assignment`: 20.
- Layout-managed `height` / `width`: остаточные места в старых компонентах.
- `qlonglong` в QML tooling: предупреждения типов Qt/C++ bridge.

## Что уже сделано к завершению P10

- C++ типы, которые используются напрямую из QML, переведены на Qt QML type registration macros.
- Для target `appOpenIPC-Dashboard` добавлены include paths для сгенерированной QML-регистрации:
  - `src/backend`;
  - `src/backend/analytics`;
  - `src/backend/gst`.
- Ручная регистрация этих типов в `main.cpp` убрана, оставлен singleton `SystemController`.
- Исправлены безопасные `parent.*` предупреждения в:
  - `src/ui/Main.qml`;
  - `src/ui/SettingsDialog.qml`;
  - `src/ui/FileManagerDialog.qml`;
  - `src/ui/StyledScrollBar.qml`.
- Часть `height` внутри `ColumnLayout` / `GridLayout` заменена на `Layout.preferredHeight`.
- `ArchiveView`, Dashboard, Settings, Majestic Control Center и Analytics разделены на меньшие компоненты без изменения публичного QML API.
- `AnalyticsZoneEditor.qml`, `SettingsEvidenceUploadPanel.qml`, архивные P9-компоненты, `RulesPanel.qml` и `LogView.qml` включены в обязательный targeted lint.
- устранён рекурсивный runtime-warning `LogView` при закрытии окна на масштабе 150%, который мог переполнять application log.

## Правило на будущее

Новые QML-компоненты не должны добавлять предупреждения в baseline.

Для новых файлов и при существенной переработке существующих файлов:

1. давать корневым контролам явные `id`;
2. не обращаться из `background`, `contentItem`, `indicator`, `delegate` к `parent.*`, если можно обратиться к явному `id`;
3. внутри Layout использовать `Layout.preferredWidth` / `Layout.preferredHeight`, а не прямые `width` / `height`, если элемент управляется Layout;
4. не оставлять неявные singleton/global references без необходимости;
5. после правки запускать обычную сборку и unit-тесты, а полный `qmllint` сверять с этим baseline.

## Следующие кандидаты на чистку

1. `GridCell.qml`.
2. `analytics/ImageViewerWindow.qml`.
3. `CameraSearchDialog.qml`.
4. `MajesticControlDialog.qml` только после введения отдельного transaction state-proxy.
5. `analytics/SnapshotBrowser.qml`.
6. `DashboardView.qml` и `AnalyticsModulesPanel.qml` небольшими изолированными пакетами.

Эти работы относятся к дальнейшей архитектурной эволюции и должны выполняться отдельными небольшими PR/коммитами, чтобы не сломать runtime-поведение UI.
