# Система локализации VMS Dashboard

## Обзор

В приложение VMS Dashboard интегрирована полноценная система локализации, поддерживающая английский (en) и русский (ru) языки. Система основана на React Context API и следует паттерну, использованному в OpenIPC Dashboard.

## Структура файлов

```
src/
├── contexts/
│   └── LocalizationContext.tsx     # Контекст и хуки локализации
└── components/
    ├── App.tsx                     # Оборачиваем в LocalizationProvider
    ├── SettingsModal.tsx           # Полностью локализован
    └── DevicePanel.tsx             # Локализованы tooltips и заголовки

public/
└── locales/
    ├── en.json                     # Английские переводы
    └── ru.json                     # Русские переводы
```

## Использование

### Подключение в компоненте

```tsx
import { useLocalization } from '../contexts/LocalizationContext';

const MyComponent: React.FC = () => {
  const { t, currentLanguage, setLanguage } = useLocalization();
  
  return (
    <div>
      <h1>{t('app_title')}</h1>
      <p>{t('about_description')}</p>
      <button onClick={() => setLanguage('en')}>English</button>
      <button onClick={() => setLanguage('ru')}>Русский</button>
    </div>
  );
};
```

### Переводы с параметрами

```tsx
// В файле переводов:
// "welcome_user": "Welcome, {{username}}!"

// В компоненте:
const message = t('welcome_user', { username: 'John' });
// Результат: "Welcome, John!"
```

## Функциональность

### Переключение языка
- Язык сохраняется в localStorage
- Автоматическая загрузка переводов при смене языка
- Переключение в настройках приложения (Settings → General → Language)

### Fallback механизм
- При ошибке загрузки языка автоматически загружается английский
- Недостающие ключи отображаются как `[ключ]` в режиме разработки

### Сохранение состояния
- Выбранный язык сохраняется между сессиями
- Восстановление языка при запуске приложения

## Локализованные компоненты

### SettingsModal
- **Вкладки**: Общие, Трансляция, Аналитика, Модули, О программе
- **Настройки**: Все поля, кнопки и описания
- **Переключатель языка**: Функциональный dropdown

### DevicePanel  
- **Tooltips**: Поиск камер, добавление камеры, настройки и т.д.
- **Заголовки**: "Устройства"

## Добавление новых переводов

1. Добавьте ключ в `public/locales/en.json`:
```json
{
  "new_feature_title": "New Feature"
}
```

2. Добавьте перевод в `public/locales/ru.json`:
```json
{
  "new_feature_title": "Новая функция"
}
```

3. Используйте в компоненте:
```tsx
<h2>{t('new_feature_title')}</h2>
```

## Расширение на другие языки

Для добавления нового языка:

1. Создайте файл `public/locales/[код_языка].json`
2. Добавьте тип в `SupportedLanguage`:
```tsx
export type SupportedLanguage = 'en' | 'ru' | 'fr'; // добавили французский
```

3. Обновите переключатель в SettingsModal:
```tsx
<option value="fr">Français</option>
```

## Заключение

Система локализации готова к использованию и легко расширяется. Все основные компоненты интерфейса локализованы, переключение языка работает в реальном времени, и настройки сохраняются между сессиями.