# WebRTC Stats Dashboard

## Обзор

WebRTC Stats Dashboard — это система мониторинга и диагностики WebRTC соединений в реальном времени. Она позволяет отслеживать качество потока, выявлять проблемы с сетью и анализировать производительность видеоконференций.

## Компоненты

### 1. WebRTCStatsCollector (src/services/webrtcStats.ts)

Сервис для сбора статистики из `RTCPeerConnection`.

#### Основные методы:

```typescript
const collector = new WebRTCStatsCollector();

// Установить соединение для мониторинга
collector.setPeerConnection(pc);

// Начать сбор статистики (интервал в мс)
collector.start(1000);

// Получить последние данные
const latestStats = collector.getLatest();

// Получить историю
const history = collector.getHistory();

// Получить средние значения за период (в секундах)
const avgStats = collector.getAverageStats(30);

// Получить качество соединения
const quality = collector.getConnectionQuality();
// Возвращает: 'excellent' | 'good' | 'fair' | 'poor' | 'unknown'

// Остановить сбор
collector.stop();

// Очистить ресурсы
collector.dispose();
```

#### Собираемая статистика:

**Видео:**
- Кодек (H264, H265, VP8, VP9, AV1)
- Разрешение (ширина x высота)
- Частота кадров (FPS)
- Битрейт (kbps)
- Пакеты (получено/потеряно)
- Потери пакетов (%)
- Jitter (мс)
- Кадры (декодировано/пропущено)
- Процент пропущенных кадров (%)

**Аудио:**
- Кодек (opus, pcmu, pcma, g722, aac)
- Битрейт (kbps)
- Пакеты (получено/потеряно)
- Потери пакетов (%)
- Jitter (мс)
- Уровень аудио (0-1)

**Сеть:**
- RTT (Round Trip Time, мс)
- Протокол (udp/tcp)
- Типы кандидатов (host/srflx/relay)
- Локальный/удалённый адрес
- Объём отправленных/полученных данных

#### Критерии качества:

**Excellent (Отличное):**
- Потери пакетов < 1%
- Jitter < 30 мс
- RTT < 50 мс

**Good (Хорошее):**
- Потери пакетов < 3%
- Jitter < 50 мс
- RTT < 100 мс

**Fair (Среднее):**
- Потери пакетов < 5%
- Jitter < 100 мс
- RTT < 200 мс

**Poor (Плохое):**
- Превышены пороги Fair

### 2. WebRTCStatsPanel (src/components/WebRTCStatsPanel.tsx)

Компактная панель отображения статистики в углу видеоплеера.

#### Пропсы:

```typescript
interface WebRTCStatsPanelProps {
  stats: WebRTCStats | null;
  compact?: boolean; // По умолчанию false
  onDetailsClick?: () => void; // Обработчик клика для открытия детального диалога
}
```

#### Режимы отображения:

**Compact (compact=true):**
- Иконка с индикатором качества
- Tooltip с ключевыми метриками при наведении
- Клик открывает детальный диалог

**Full (compact=false):**
- Детальная панель с разделами
- Видео, аудио, сеть
- Цветовая кодировка метрик

### 3. WebRTCStatsDialog (src/components/WebRTCStatsDialog.tsx)

Модальное окно с полной статистикой и графиками.

#### Пропсы:

```typescript
interface WebRTCStatsDialogProps {
  open: boolean;
  onClose: () => void;
  streamName: string;
  currentStats: WebRTCStats | null;
  history: WebRTCStats[];
}
```

#### Функции:

- **Состояние соединения:** Connection, ICE Connection, ICE Gathering, Signaling
- **Видео статистика:** Кодек, разрешение, FPS, битрейт, потери пакетов, jitter, кадры
- **Аудио статистика:** Кодек, битрейт, потери пакетов, jitter, уровень
- **Сетевая статистика:** RTT, протокол, кандидаты, адреса, объём данных
- **Графики (recharts):**
  - Битрейт и FPS во времени
  - Потери пакетов и Jitter во времени
  - RTT во времени

## Интеграция

### В VideoStreamPlayer

```typescript
<VideoStreamPlayer
  streamName="camera_01"
  showWebRTCStats={true}
  webrtcStatsUpdateInterval={1000} // Интервал обновления в мс
/>
```

### Автоматическое включение

В Dashboard.tsx статистика включена по умолчанию:

```typescript
<VideoStreamPlayer
  // ... другие пропсы
  showWebRTCStats={true}
/>
```

## Локализация

Все тексты локализованы в `public/locales/en.json` и `public/locales/ru.json`:

```json
{
  "webrtc_stats": {
    "title": "WebRTC Statistics",
    "connection_state": "Connection state",
    "video": "Video",
    "audio": "Audio",
    "network": "Network",
    // ... и т.д.
  }
}
```

## Использование

### Базовый пример

```typescript
import { WebRTCStatsCollector } from './services/webrtcStats';
import WebRTCStatsPanel from './components/WebRTCStatsPanel';
import WebRTCStatsDialog from './components/WebRTCStatsDialog';

const MyComponent = () => {
  const [stats, setStats] = useState<WebRTCStats | null>(null);
  const [history, setHistory] = useState<WebRTCStats[]>([]);
  const [showDialog, setShowDialog] = useState(false);
  const collectorRef = useRef<WebRTCStatsCollector | null>(null);

  useEffect(() => {
    const pc = new RTCPeerConnection();
    const collector = new WebRTCStatsCollector();
    
    collector.setPeerConnection(pc);
    collector.start(1000);
    
    const interval = setInterval(() => {
      setStats(collector.getLatest());
      setHistory(collector.getHistory());
    }, 1000);
    
    collectorRef.current = collector;
    
    return () => {
      clearInterval(interval);
      collector.stop();
      collector.dispose();
    };
  }, []);

  return (
    <div>
      <WebRTCStatsPanel 
        stats={stats}
        compact={true}
        onDetailsClick={() => setShowDialog(true)}
      />
      
      <WebRTCStatsDialog
        open={showDialog}
        onClose={() => setShowDialog(false)}
        streamName="My Stream"
        currentStats={stats}
        history={history}
      />
    </div>
  );
};
```

## Диагностика проблем

### Высокие потери пакетов (> 3%)

**Причины:**
- Слабое Wi-Fi соединение
- Перегруженная сеть
- Проблемы на стороне сервера

**Решение:**
- Использовать Ethernet вместо Wi-Fi
- Проверить другие устройства в сети
- Уменьшить битрейт потока

### Высокий Jitter (> 50 мс)

**Причины:**
- Нестабильное соединение
- Переменная задержка в сети
- Проблемы с QoS

**Решение:**
- Настроить QoS на роутере
- Приоритизировать WebRTC трафик
- Использовать проводное соединение

### Высокий RTT (> 100 мс)

**Причины:**
- Географическая удалённость
- Плохой маршрут в сети
- Перегруженные каналы связи

**Решение:**
- Использовать TURN сервер ближе к пользователю
- Проверить маршрут traceroute
- Сменить провайдера

### Пропущенные кадры (> 1%)

**Причины:**
- Недостаточная производительность GPU/CPU
- Слишком высокое разрешение для устройства
- Проблемы с декодером

**Решение:**
- Снизить разрешение потока
- Закрыть другие приложения
- Обновить драйверы видеокарты

## Архитектура

```
┌─────────────────────────────────────────────────────────┐
│                  VideoStreamPlayer                       │
│  ┌───────────────────────────────────────────────────┐  │
│  │         RTCPeerConnection (WebRTC)                │  │
│  │                      │                            │  │
│  │                      ▼                            │  │
│  │         WebRTCStatsCollector                      │  │
│  │           │                                       │  │
│  │           ├─► getStats() every 1s                │  │
│  │           ├─► Parse video/audio/network stats    │  │
│  │           ├─► Calculate bitrate, packet loss     │  │
│  │           ├─► Store history (60 samples)         │  │
│  │           └─► Assess connection quality          │  │
│  └───────────────────────────────────────────────────┘  │
│                      │                                   │
│                      ▼                                   │
│  ┌───────────────────────────────────────────────────┐  │
│  │      WebRTCStatsPanel (bottom-left corner)        │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │ 🟢 Excellent  1500 kbps  30 FPS  12 ms RTT │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  │                   │ onClick                       │  │
│  │                   ▼                               │  │
│  │      WebRTCStatsDialog (Modal)                    │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │ Connection State: connected                 │  │  │
│  │  │ Video: H264 1920x1080 30fps 1500kbps       │  │  │
│  │  │ Audio: opus 64kbps                          │  │  │
│  │  │ Network: UDP RTT=12ms Loss=0.1%             │  │  │
│  │  │                                             │  │  │
│  │  │ [Bitrate/FPS Graph]                         │  │  │
│  │  │ [Packet Loss/Jitter Graph]                  │  │  │
│  │  │ [RTT Graph]                                 │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## Производительность

- **Сбор статистики:** ~2-5 мс каждую секунду
- **Обновление UI:** React рендеринг < 16 мс
- **История:** 60 образцов = ~1 минута (при интервале 1 с)
- **Память:** ~100 КБ на соединение

## Совместимость

- Chrome/Edge 89+
- Firefox 90+
- Safari 15+
- Opera 75+

## Зависимости

- **recharts** (^3.2.1) — для графиков
- **@mui/material** — для UI компонентов
- **@mui/icons-material** — для иконок

## Roadmap

- [ ] Экспорт статистики в CSV/JSON
- [ ] Алерты при деградации качества
- [ ] Сравнение нескольких потоков
- [ ] Статистика за сессию (мин/макс/средн)
- [ ] Интеграция с системой логирования
