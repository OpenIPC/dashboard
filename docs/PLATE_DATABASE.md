# License Plate Database Feature

## Обзор

Реализована полная система базы данных для хранения и управления записями распознанных номерных знаков с полными снимками транспортных средств.

## Компоненты

### Backend (Rust/Tauri)

1. **База данных** (`src-tauri/src/database/`)
   - `mod.rs` - Основная структура БД с SQLite
   - `commands.rs` - Tauri команды для доступа к БД из frontend
   
2. **Структура данных** (`PlateRecord`):
   - `id` - Уникальный идентификатор записи
   - `camera_id` - ID камеры
   - `plate_number` - Номерной знак
   - `confidence` - Уровень уверенности распознавания (0.0-1.0)
   - `timestamp` - Дата и время фиксации (RFC3339)
   - `full_image_path` - Путь к полному снимку транспортного средства
   - `plate_crop_path` - Путь к снимку номерного знака
   - `vehicle_type` - Тип транспорта (опционально)
   - `direction` - Направление движения (опционально)
   - `notes` - Заметки пользователя

3. **Индексация**:
   - По номеру (`plate_number`)
   - По времени (`timestamp`)
   - По камере (`camera_id`)

4. **Операции БД**:
   - `insert_record` - Вставка новой записи
   - `get_records` - Получение записей с фильтрами (номер, камера, период, пагинация)
   - `get_record_by_id` - Получение записи по ID
   - `update_notes` - Обновление заметок
   - `delete_record` - Удаление записи
   - `get_statistics` - Статистика (всего, за сегодня, уникальных номеров)
   - `search_plate_history` - Поиск всех записей по номеру

5. **Интеграция с ANPR** (`src-tauri/src/analytics.rs`):
   - При каждом распознавании номера:
     - Сохраняется полный кадр транспортного средства (`*_full.jpg`)
     - Сохраняется обрезанный снимок номера (`*.jpg`)
     - Создаётся JSON метаданные (`*.json`)
     - Вставляется запись в SQLite базу данных
   - Автоматическое извлечение номера из detection label
   - Парсинг camera_id для числового представления

### Frontend (React/TypeScript)

1. **Компонент** (`src/components/PlateDatabase.tsx`):
   - Просмотр всех записей в таблице
   - Фильтры по номеру, камере, периоду
   - Поиск по истории конкретного номера
   - Пагинация результатов
   - Детальный просмотр записи с полным изображением
   - Редактирование заметок
   - Удаление записей
   - Статистика в реальном времени

2. **Стили** (`src/components/PlateDatabase.css`):
   - Темная тема
   - Адаптивная раскладка
   - Таблица с сортировкой
   - Модальное окно деталей
   - Превью изображений

3. **Интеграция**:
   - Роутинг `/plate-database`
   - Пункт меню "Plate Database" с иконкой Storage
   - Использование Tauri IPC для вызова backend команд

## Файловая структура снимков

```
E:\VMS\License Plates\
  ├─ {camera_id}\
  │  ├─ {YYYY}\
  │  │  ├─ {MM}\
  │  │  │  ├─ {DD}\
  │  │  │  │  ├─ {timestamp}_{label}_{detection_id}_{order}_full.jpg  # Полный кадр
  │  │  │  │  ├─ {timestamp}_{label}_{detection_id}_{order}.jpg        # Обрезанный номер
  │  │  │  │  └─ {timestamp}_{label}_{detection_id}_{order}.json       # Метаданные
```

## API (Tauri Commands)

```typescript
// Получить записи с фильтрами
get_plate_records(
  limit: i32,
  offset: i32,
  plate_filter?: string,
  camera_filter?: i32,
  date_from?: string,
  date_to?: string
): PlateRecord[]

// Получить запись по ID
get_plate_record_by_id(id: i64): PlateRecord | null

// Обновить заметки
update_plate_notes(id: i64, notes: string): void

// Удалить запись
delete_plate_record(id: i64): void

// Получить статистику
get_plate_statistics(): PlateStatistics

// Поиск по номеру
search_plate_history(plate_number: string): PlateRecord[]
```

## База данных

**Файл**: `{app_data_dir}/plate_records.db`

**Схема**:
```sql
CREATE TABLE plate_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    camera_id INTEGER NOT NULL,
    plate_number TEXT NOT NULL,
    confidence REAL NOT NULL,
    timestamp TEXT NOT NULL,
    full_image_path TEXT NOT NULL,
    plate_crop_path TEXT NOT NULL,
    vehicle_type TEXT,
    direction TEXT,
    notes TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_plate_number ON plate_records(plate_number);
CREATE INDEX idx_timestamp ON plate_records(timestamp);
CREATE INDEX idx_camera_id ON plate_records(camera_id);
```

## Использование

1. **Автоматическая запись**:
   - Когда система ANPR распознает номер, запись автоматически добавляется в БД
   - Сохраняются оба снимка (полный и обрезанный)

2. **Просмотр**:
   - Перейти в "Plate Database" через боковое меню
   - Используйте фильтры для поиска нужных записей
   - Кликните на запись для просмотра деталей

3. **Поиск**:
   - Введите номер в фильтр и нажмите "Search"
   - Или используйте фильтры по камере и дате

4. **Управление**:
   - Добавляйте заметки к записям
   - Удаляйте ненужные записи
   - Просматривайте статистику

## Производительность

- **Индексы** для быстрого поиска
- **Пагинация** для работы с большими объемами данных
- **Bundled SQLite** - не требует установки
- **Асинхронные операции** - не блокируют UI

## Будущие улучшения

- [ ] Экспорт данных (CSV, PDF)
- [ ] Распознавание типа транспорта
- [ ] Определение направления движения
- [ ] Поиск по диапазону времени суток
- [ ] Группировка по камерам
- [ ] Графики и аналитика
- [ ] Уведомления о повторных проездах
- [ ] Черные/белые списки номеров
