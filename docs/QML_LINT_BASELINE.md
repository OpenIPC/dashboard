# QML lint baseline

Дата актуализации: 2026-07-02.

Этот документ фиксирует текущее состояние legacy `qmllint` после релиза `v0.2.5` и после первой безопасной чистки P2.1.

## Текущее состояние

Команда проверки:

```powershell
cmake --build build_release --target appOpenIPC-Dashboard_qmllint --config Release
```

Результат на 2026-07-02:

- полный `qmllint` пока падает на legacy baseline;
- до чистки было около `1322` предупреждений;
- после P2.1 safe cleanup осталось `1184` предупреждения;
- обычная сборка `Release` проходит;
- unit-тесты проходят: `14/14`.

## Основные источники предупреждений

| Файл | Предупреждений | Комментарий |
| --- | ---: | --- |
| `src/ui/GridCell.qml` | 163 | Старый крупный экран видеоячейки, много unqualified access и unresolved C++ QML types в lint tooling. |
| `src/ui/analytics/ImageViewerWindow.qml` | 128 | Legacy analytics UI, требует отдельной типизации делегатов/контролов. |
| `src/ui/SettingsDialog.qml` | 126 | Часть безопасных `parent.*` уже исправлена, остались legacy-секции. |
| `src/ui/ArchiveView.qml` | 110 | Старый экран архива с видеоплеером и unqualified access. |
| `src/ui/MajesticControlDialog.qml` | 83 | Оставшийся крупный legacy-контейнер после дробления Majestic. |
| `src/ui/analytics/SnapshotBrowser.qml` | 55 | Legacy analytics browser. |
| `src/ui/DashboardView.qml` | 47 | Остаточные предупреждения после дробления Dashboard. |
| `src/ui/AnalyticsView.qml` | 41 | Legacy shell analytics-раздела. |
| `src/ui/CameraSearchDialog.qml` | 39 | Старый диалог поиска камер. |
| `src/ui/FileManagerDialog.qml` | 35 | Часть safe warnings исправлена, остаток требует отдельной ревизии старого FileManager. |

## Основные категории

- `Unqualified access`: 835.
- `Unresolved QML type`: 77.
- `Property "text" not found on type "QQuickItem"` и родственные `parent.*`: остаточный legacy-класс.
- `Cannot defer property assignment`: 20.
- Layout-managed `height` / `width`: остаточные места в старых компонентах.
- `qlonglong` в QML tooling: предупреждения типов Qt/C++ bridge.

## Что уже сделано в P2.1

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
2. `ArchiveView.qml`.
3. `MajesticControlDialog.qml`.
4. `analytics/ImageViewerWindow.qml`.
5. `SettingsDialog.qml` legacy-секции.
6. `FileManagerDialog.qml`.

Эти работы относятся к дальнейшим P2.x этапам и должны выполняться отдельными небольшими PR/коммитами, чтобы не сломать runtime-поведение UI.
