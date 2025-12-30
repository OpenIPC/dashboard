pragma Singleton
import QtQml

QtObject {
    id: i18n

    // Current language: "ru" or "en"
    property string language: "ru"

    // Map original strings (mostly Russian) to English
    readonly property var trMapEn: ({
        "Логи": "Logs",
        "Очистить": "Clear",
        "Скачать": "Download",
        "Сохранить логи": "Save logs",
        "Текстовые файлы (*.txt)": "Text files (*.txt)",
        "Архив": "Archive",
        "Открыть в браузере": "Open in browser",
        "Файловый менеджер": "File manager",
        "SSH Терминал": "SSH Terminal",
        "Группы": "Groups",
        "Редактировать камеру": "Edit camera",
        "Удалить камеру": "Delete camera",
        "Введите команду...": "Enter command...",
        "Инфо": "Info",
        "Предупреждение": "Warning",
        "Отладка": "Debug",
        "Настройки успешно сохранены": "Settings saved successfully",
        "Выбор папки для записей": "Select recordings folder",
        "Выбор папки для снимков": "Select screenshots folder",
        "Сохранить конфигурацию": "Save configuration",
        "Импортировать конфигурацию": "Import configuration",
        "Все файлы (*)": "All files (*)",
        "Проверка обновлений...": "Checking for updates...",
        "Доступно обновление": "Update available",
        "Загрузка обновления...": "Downloading update...",
        "Обновление скачано — готово к установке": "Update downloaded - ready to install",
        "Установка обновления...": "Installing update...",
        "Обновление установлено": "Update installed",
        "Установлена последняя версия": "Latest version installed",
        "Ошибка": "Error",
        "Нажмите \"Проверить обновления\"": "Click \"Check for updates\"",
        "Dashboard for OpenIPC": "Dashboard for OpenIPC",
        "Раскладка": "Layout",
        "Новая раскладка": "New layout",
        "Редактирование": "Editing",
        "Редактор шаблонов": "Layout editor",
        "Название раскладки": "Layout name",
        "Название": "Name",
        "Предустановки": "Presets",
        "Поиск": "Search",
        "Начало": "Start",
        "Конец": "End",
        "Канал": "Channel",
        "Имя файла": "Filename",
        "Размер": "Size",
        "Скачивание...": "Downloading...",
        "Ошибка скачивания": "Download error",
        "Записей не найдено": "No recordings found",
        "Поиск...": "Searching...",
        "Выберите камеру": "Select camera",
        "%1 ячеек": "%1 cells",
        "Закрыть": "Close",
        "Сохранить": "Save",
        "Новая раскладка ": "New layout ",
        "Язык": "Language",
        "Папка записей": "Recordings folder",
        "Папка снимков": "Screenshots folder",
        "Выберите папку": "Choose folder",
        "Папка": "Folder",
        "Приложение": "Application",
        "Без группы": "No group",
        "Устройства": "Devices",
        "Найденные камеры": "Discovered cameras",
        "Найдено устройств: ": "Devices found: ",
        "Найдено камер: ": "Cameras found: ",
        "Сетевой интерфейс": "Network interface",
        "ОБНОВИТЬ ИНТЕРФЕЙСЫ": "REFRESH INTERFACES",
        "Выберите адаптер, который нужно сканировать.": "Select an adapter to scan.",
        "Устройство": "Device",
        "Сеть": "Network",
        "Порты": "Ports",
        "Протокол": "Protocol",
        "Нажмите \"Сканировать\", чтобы найти камеры.\nПроверьте, что сеть помечена как «Частная»...": "Click \"Scan\" to find cameras.\nMake sure the network is marked as Private...",
        "СКАНИРОВАТЬ": "SCAN",
        "ДОБАВИТЬ ВЫБРАННЫЕ (": "ADD SELECTED (",
        "Добавить новую камеру": "Add new camera",
        "Название": "Name",
        "IP / Host": "IP / Host",
        "RTSP порт": "RTSP port",
        "ONVIF порт": "ONVIF port",
        "Логин": "Login",
        "Аналитика": "Analytics",
        "Назад": "Back",
        "Лица": "Faces",
        "Объекты": "Objects",
        "Номера": "Plates",
        "Face Snapshots": "Face Snapshots",
        "Object Counter": "Object Counter",
        "License Plates": "License Plates",
        "Загрузка...": "Downloading...",
        "Пароль": "Password",
        "Канал": "Channel",
        "HD профиль": "HD profile",
        "SD профиль": "SD profile",
        "Ввести RTSP URL вручную": "Enter RTSP URL manually",
        "Стандартный формат URL": "Standard URL format",
        "RTSP HD URL": "RTSP HD URL",
        "RTSP SD URL": "RTSP SD URL",
        "ОТМЕНА": "CANCEL",
        "СОХРАНИТЬ": "SAVE",
        "УДАЛИТЬ": "DELETE",
        "Удаление камеры": "Delete camera",
        "Вы действительно хотите удалить эту камеру?": "Are you sure you want to delete this camera?",
        "Приложение": "Application",
        "Папка записей": "Recordings folder",
        "Папка снимков": "Screenshots folder",
        "Папка": "Folder",
        "Выберите папку": "Choose folder",
        "Язык": "Language",
        "Трансляция": "Streaming",
        "Аналитика": "Analytics",
        "Модули": "Modules",
        "О программе": "About",
        "Общие": "General",
        "Настройки — ": "Settings — ",
        "Поиск камер...": "Search cameras...",
        "Поиск": "Search",
        "Камера": "Camera",
        "ЦП": "CPU",
        "ОЗУ": "RAM",
        "ОБНОВИТЬ ИНТЕРФЕЙСЫ": "REFRESH INTERFACES",
        "ВСЕ": "ALL",
        "Найдено устройств: ": "Devices found: ",
        "Найдено камер: ": "Cameras found: ",
        "Добавить группу": "Add group",
        "Добавить камеру": "Add camera",
        "Аналитика": "Analytics",
        "Настройки": "Settings",
        "Логи": "Logs",
        "Пользователь": "User",
        "Выход": "Logout",
        "Пожалуйста, войдите для продолжения": "Please log in to continue",
        "Запомнить меня": "Remember me",
        "Обновить": "Refresh",
        "Выберите запись для просмотра": "Select a recording to view",
        "Записи": "Recordings",
        "Нет записей": "No recordings",
        "ВОЙТИ": "LOGIN",
        "Данные по умолчанию: логин admin, пароль admin.": "Default credentials: login admin, password admin.",
        "Неверный логин или пароль": "Invalid username or password",
        "Найденные камеры": "Discovered cameras",
        "Сетевой интерфейс": "Network interface",
        "Выберите адаптер, который нужно сканировать.": "Select an adapter to scan.",
        "Устройство": "Device",
        "Сеть": "Network",
        "Порты": "Ports",
        "Протокол": "Protocol",
        "СКАНИРОВАТЬ": "SCAN",
        "ДОБАВИТЬ ВЫБРАННЫЕ (": "ADD SELECTED (",
        "Аппаратное ускорение": "Hardware acceleration",
        "Авто": "Auto",
        "Без ускорения": "No acceleration",
        "Авто выберет доступное ускорение (NVIDIA/Intel) или CPU": "Auto will choose available acceleration (NVIDIA/Intel) or CPU",
        "Уведомления": "Notifications",
        "Включить уведомления": "Enable notifications",
        "Управление конфигурацией": "Configuration management",
        "Экспорт конфигурации": "Export configuration",
        "Импорт конфигурации": "Import configuration",
        "Выбор папки для записей": "Select recordings folder",
        "Выбор папки для снимков": "Select screenshots folder",
        "Сохранить конфигурацию": "Save configuration",
        "Импортировать конфигурацию": "Import configuration",
        "Все файлы (*)": "All files (*)",
        "Ошибка": "Error",
        "Ошибка обновления": "Update error",
        "Нажмите \"Проверить обновления\"": "Click \"Check for updates\""
        ,"Обновления": "Updates"
        ,"Статус": "Status"
        ,"Действие": "Action"
        ,"Проверить обновления": "Check for updates"
        ,"Прогресс": "Progress"
        ,"Ошибка": "Error"
        ,"Установка": "Installation"
        ,"Установить и перезапустить": "Install and restart"
        ,"Позже": "Later"
        ,"Проверка обновлений...": "Checking for updates..."
        ,"Доступно обновление": "Update available"
        ,"Загрузка обновления...": "Downloading update..."
        ,"Обновление скачано — готово к установке": "Update downloaded — ready to install"
        ,"Установка обновления...": "Installing update..."
        ,"Обновление установлено": "Update installed"
        ,"Установлена последняя версия": "Latest version installed"
        ,"Трансляция (MDK)": "Streaming (MDK)"
        ,"Предпочтительный поток": "Preferred stream"
        ,"Режим кадра": "Frame mode"
        ,"Обрезать по краям": "Crop to fill"
        ,"Сохранять пропорции": "Fit"
        ,"Растянуть": "Stretch"
        ,"Отображать статистику": "Show stats"
        ,"Показывать codec/res/bitrate/fps": "Show codec/res/bitrate/fps"
        ,"Автовоспроизведение": "Autoplay"
        ,"Запускать поток сразу после загрузки": "Start stream on load"
        ,"Доступные опции основаны на текущей версии MDK-плеера (FFmpeg, без HW-декодеров). Дополнительные настройки можно будет добавить позже.": "Options reflect the current MDK player build (FFmpeg, no HW decoders). More settings can be added later.",
        
        // User Management
        "Управление пользователями": "User Management",
        "Пользователи": "Users",
        "Права": "Permissions",
        "Права доступа": "Access rights",
        "Настройка прав доступа": "Access rights settings",
        "Просмотр (Live)": "Live View",
        "Архив (Playback)": "Playback",
        "Управление PTZ": "PTZ Control",
        "Экспорт": "Export",
        "Настройки системы": "System settings",
        "Пользователь: ": "User: ",
        "Все интерфейсы": "All interfaces",
        "Администратор": "Administrator",
        "Оператор": "Operator",
        "Сменить пароль": "Change password",
        "Удалить пользователя": "Delete user",
        "Добавить пользователя": "Add user",
        "Добавить нового пользователя": "Add new user",
        "Роль": "Role",
        "Сменить пароль для": "Change password for",
        "Старый пароль": "Old password",
        "Новый пароль": "New password",
        "Подтвердите пароль": "Confirm password",
        "Пароли не совпадают": "Passwords do not match",
        "Неверный старый пароль": "Incorrect old password",
        "Создать новую группу": "Create new group",
        "Название группы": "Group name",
        "Создать": "Create",
        "Отмена": "Cancel",
        "Пожалуйста, войдите для\nпродолжения": "Please log in to\ncontinue",
        "Пакетное добавление": "Batch Add",
        "Введите логин и пароль для выбранных камер:": "Enter login and password for selected cameras:",
        "Добавить": "Add"
    })

    // Optional reverse translations for English source strings when showing Russian
    readonly property var trMapRu: ({
        "No Signal": "Нет сигнала",
        "Loading...": "Загрузка...",
        "Layout": "Раскладка",
        "Face Detector": "Детектор лиц",
        "Object Counter": "Счетчик объектов",
        "License Plate": "Распознавание номеров",
        "Detects faces in video stream": "Обнаруживает лица в видеопотоке",
        "Counts people and vehicles": "Считает людей и транспорт",
        "Recognizes license plates": "Распознает номерные знаки",
        "Snapshots directory": "Каталог снимков",
        "License plate snapshots": "Снимки номеров",
        "Object snapshots": "Снимки объектов",
        "Choose...": "Выбрать...",
        "Use default": "По умолчанию",
        "Face snapshot mode": "Режим снимков лиц",
        "Disabled": "Отключено",
        "Standard": "Стандартный",
        "Anonymized": "Анонимный",
        "Encrypted": "Зашифрованный",
        "Face snapshots are not captured.": "Снимки лиц не сохраняются.",
        "Faces are saved as-is without additional processing.": "Лица сохраняются как есть без обработки.",
        "Snapshots are blurred before being stored.": "Снимки размываются перед сохранением.",
        "Snapshots are encrypted with your key and stored as .bin files.": "Снимки шифруются вашим ключом и сохраняются как .bin файлы.",
        "Encryption requires a 64-character hexadecimal key.": "Для шифрования требуется 64-значный шестнадцатеричный ключ.",
        "Snapshot encryption key": "Ключ шифрования снимков",
        "64 hex characters": "64 hex символа",
        "Save": "Сохранить",
        "Reset key": "Сбросить ключ",
        "Key configured. Saving a new key will replace it.": "Ключ настроен. Сохранение нового ключа заменит его.",
        "No key configured. Provide one to enable encryption.": "Ключ не настроен. Укажите его для включения шифрования.",
        "Ready": "Готов",
        "Downloading...": "Загрузка...",
        "Error: ": "Ошибка: ",
        "Author: ": "Автор: ",
        "Default directory": "Каталог по умолчанию",
        "Choose snapshots directory": "Выберите каталог снимков"
    })

    function t(text, params) {
        var lang = language || "ru";
        var value = text;
        if (lang === "en") {
            value = trMapEn[text] || text;
        } else if (lang === "ru") {
            value = trMapRu[text] || text;
        }
        if (params && params.length) {
            for (var i = 0; i < params.length; ++i) {
                value = value.replace("%" + (i + 1), params[i]);
            }
        }
        return value;
    }
}
