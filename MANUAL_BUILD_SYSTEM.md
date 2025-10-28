# Система ручной сборки VMS Dashboard

## ✅ Что настроено:

### 🔧 Локальная сборка (остается как раньше):
```bash
npm run build-windows    # Windows MSI
npm run build-linux      # Linux DEB + AppImage  
npm run build-macos      # macOS DMG
npm run build-release    # Текущая платформа
```

### 🚀 Ручная сборка через GitHub Actions:

#### 1. Тестовая сборка (без коммитов):
1. Идите на https://github.com/OpenIPC/dashboard/actions
2. Выберите "Build and Test"
3. Нажмите "Run workflow"
4. Выберите платформу:
   - `all` - Все платформы
   - `windows` - Только Windows
   - `linux` - Только Linux  
   - `macos` - Только macOS

#### 2. Релизная сборка (автоматическая при тегах):
```bash
git tag v1.0.0
git push origin v1.0.0
```

## 🎯 Что изменилось:

### ❌ Больше НЕ запускается автоматически:
- При каждом push в main/develop
- При создании pull request

### ✅ Запускается ТОЛЬКО:
- **Вручную** через GitHub Actions UI
- **Автоматически** при создании version тегов (v1.0.0, v2.1.3, etc.)

## 🔄 Workflow состояния:

- **build.yml**: Только ручной запуск + выбор платформы
- **release.yml**: Автоматический релиз при тегах + ручной запуск

Теперь вы полностью контролируете, когда запускаются сборки! 🎉