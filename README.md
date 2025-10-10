# VMS Dashboard - Professional Video Management System

<div align="center">

![VMS Dashboard](https://img.shields.io/badge/VMS-Dashboard-blue?style=for-the-badge)
![Tauri](https://img.shields.io/badge/Tauri-2.0-orange?style=for-the-badge)
![React](https://img.shields.io/badge/React-18-blue?style=for-the-badge)
![Rust](https://img.shields.io/badge/Rust-Latest-red?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Современная нативная система управления видеонаблюдением для профессионалов**

[Скачать релиз](https://github.com/OpenIPC/dashboard/releases) • [Документация](./docs/) • [Сообщить об ошибке](https://github.com/OpenIPC/dashboard/issues)

</div>

---

## 🚀 Ключевые особенности

### 📹 **Управление камерами**
- **Автообнаружение ONVIF камер** в локальной сети
- **Множественные протоколы**: ONVIF, RTSP, OpenIPC, Hikvision, Dahua
- **Два режима подключения**: прямой RTSP и MediaMTX интеграция
- **Группировка и организация** камер
- **Безопасное хранение** учетных данных

### 🖥️ **Профессиональный интерфейс мониторинга**
- **Мультисеточные макеты**: от 1 до 64 камер одновременно
- **Настраиваемые шаблоны** с системой вкладок
- **Drag & Drop** управление камерами
- **Полноэкранный режим** и детальный просмотр
- **Индивидуальные настройки** для каждой ячейки

### 🛡️ **Безопасность и производительность**
- **Нативное приложение** без браузерных ограничений
- **Локальная обработка** данных без облачных сервисов
- **Шифрование паролей** AES-256
- **Низкое потребление ресурсов** благодаря Rust/Tauri

---

## 📥 Быстрый старт

### Установка
1. Скачайте последний [релиз](https://github.com/OpenIPC/dashboard/releases) для вашей ОС
2. Установите приложение (Windows: `.msi`, macOS: `.dmg`, Linux: `.deb`/`.AppImage`)
3. Запустите VMS Dashboard

### Первая настройка
1. **Добавьте камеры**: используйте автопоиск ONVIF или добавьте вручную
2. **Создайте макет**: перетащите камеры в сетку мониторинга
3. **Настройте запись**: укажите расписание и параметры архива
4. **Начните мониторинг**: наслаждайтесь профессиональным видеонаблюдением!

---

## 🔧 Разработка

### Системные требования
- Node.js 18+
- Rust 1.70+
- Platform-specific dependencies (см. [Tauri Prerequisites](https://tauri.app/v1/guides/getting-started/prerequisites))

### Локальная сборка
```bash
# Клонирование репозитория
git clone https://github.com/OpenIPC/dashboard.git
cd dashboard

# Установка зависимостей
npm install

# Запуск в режиме разработки
npm run tauri

# Сборка релиза
npm run tauri-build
```

### Структура проекта
```
dashboard/
├── src/                    # React frontend
│   ├── components/         # UI компоненты
│   ├── contexts/           # React контексты
│   ├── services/           # API сервисы
│   └── types/              # TypeScript типы
├── src-tauri/              # Rust backend
│   ├── src/                # Основной код
│   ├── mediamtx/           # MediaMTX интеграция
│   └── capabilities/       # Tauri разрешения
└── docs/                   # Документация
```

## 📊 Системные требования

### Минимальные
- **ОС**: Windows 10, macOS 10.15, Ubuntu 18.04
- **RAM**: 4 GB
- **CPU**: Dual-core 2.0 GHz
- **GPU**: Поддержка H.264 декодирования
- **Сеть**: 100 Mbps для локальных камер

### Рекомендуемые
- **RAM**: 8+ GB
- **CPU**: Quad-core 3.0+ GHz
- **GPU**: Дискретная с аппаратным декодированием
- **Сеть**: Gigabit Ethernet
- **Хранилище**: SSD для записей

---

## 🤝 Вклад в проект

Мы приветствуем вклад сообщества! Пожалуйста:

1. 🍴 Форкните репозиторий
2. 🌿 Создайте feature-ветку (`git checkout -b feature/amazing-feature`)
3. 💾 Зафиксируйте изменения (`git commit -m 'Add amazing feature'`)
4. 📤 Отправьте в ветку (`git push origin feature/amazing-feature`)
5. 🔄 Создайте Pull Request

### Типы вкладов
- 🐛 **Bug fixes** - исправление ошибок
- ✨ **Features** - новая функциональность
- 📝 **Documentation** - улучшение документации
- 🌍 **Translations** - переводы интерфейса
- 🧪 **Testing** - написание тестов


## 🆘 Поддержка

- 📋 [Issues](https://github.com/OpenIPC/dashboard/issues) - сообщения об ошибках и предложения
- 💬 [Discussions](https://github.com/OpenIPC/dashboard/discussions) - общие вопросы и обсуждения
- 📖 [Wiki](https://github.com/OpenIPC/dashboard/wiki) - детальная документация
- 📧 Email: support@openipc.org

---

## 🙏 Благодарности

- [OpenIPC](https://openipc.org/) за поддержку проекта
- [Tauri](https://tauri.app/) за кроссплатформенный фреймворк
- [MediaMTX](https://github.com/bluenviron/mediamtx) за потоковый сервер
- Сообществу за тестирование и обратную связь

---

<div align="center">

**⭐ Поставьте звездочку, если проект оказался полезным!**

[⬆ Наверх](#vms-dashboard---professional-video-management-system)

</div>
