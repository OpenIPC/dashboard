# OpenIPC Dashboard v0.2.5.1

Hotfix release for the Dashboard sidebar icon rendering issue reported by users after `v0.2.5`.

## Русский

### Исправлено

- Исправлено исчезновение иконок на плитках Metro sidebar у части пользователей.
- Sidebar-иконки больше не зависят от `QtQuick.Shapes` / `qmlshapesplugin`.
- Добавлен устойчивый fallback через встроенный `MaterialIcons-Regular.ttf`, упакованный в `qrc`.
- Старые SVG-path параметры оставлены для совместимости, но плитки сайдбара теперь используют стабильные Material Icons ligature names.
- Добавлен unit-тест, подтверждающий, что `0.2.5.1` корректно определяется как обновление после `0.2.5`.

### Внутренние изменения

- Зафиксирован текущий legacy `qmllint` baseline для дальнейшей аккуратной чистки QML.
- Выполнена безопасная чистка части `qmllint` предупреждений в QML-компонентах.
- C++ типы, используемые из QML, переведены на Qt QML type registration macros.

### Проверено

- `cmake --build build_release --config Release --parallel`
- `ctest --test-dir build_release --output-on-failure`
- `git diff --check`

## English

### Fixed

- Fixed missing icons in Metro sidebar tiles on some user installations.
- Sidebar icons no longer depend on `QtQuick.Shapes` / `qmlshapesplugin`.
- Added a robust fallback through the bundled `MaterialIcons-Regular.ttf` resource.
- Legacy SVG path properties are kept for compatibility, while sidebar tiles now use stable Material Icons ligature names.
- Added a unit test to ensure `0.2.5.1` is correctly detected as newer than `0.2.5`.

### Internal

- Documented the current legacy `qmllint` baseline for gradual QML cleanup.
- Safely reduced a subset of QML lint warnings.
- Moved QML-facing C++ types to Qt QML type registration macros.

### Verified

- `cmake --build build_release --config Release --parallel`
- `ctest --test-dir build_release --output-on-failure`
- `git diff --check`
