// Централизованная система логирования для нативного приложения
export type LogLevel = 'info' | 'warn' | 'error' | 'debug';
export type LogCategory = 'app' | 'camera' | 'stream' | 'ptz' | 'network' | 'analytics' | 'auth' | 'system';

export interface LogEntry {
  id: string;
  timestamp: number;
  level: LogLevel;
  category: LogCategory;
  message: string;
  details?: unknown;
  stack?: string;
}

type LogListener = (entry: LogEntry) => void;

class LoggerService {
  private logs: LogEntry[] = [];
  private maxLogs = 1000;
  private listeners: Set<LogListener> = new Set();
  private logIdCounter = 0;
  private readonly originalConsole = {
    log: console.log.bind(console),
    warn: console.warn.bind(console),
    error: console.error.bind(console),
    debug: console.debug.bind(console),
  };

  constructor() {
    // Перехватываем console методы
    console.log = (...args: unknown[]) => {
      this.originalConsole.log(...args);
      this.log('info', 'system', args.map(this.serializeArg).join(' '));
    };

    console.warn = (...args: unknown[]) => {
      this.originalConsole.warn(...args);
      this.log('warn', 'system', args.map(this.serializeArg).join(' '));
    };

    console.error = (...args: unknown[]) => {
      this.originalConsole.error(...args);
      this.log('error', 'system', args.map(this.serializeArg).join(' '), args[0]);
    };

    console.debug = (...args: unknown[]) => {
      this.originalConsole.debug(...args);
      this.log('debug', 'system', args.map(this.serializeArg).join(' '));
    };

    // Загружаем логи из localStorage при инициализации
    this.loadFromStorage();
  }

  private serializeArg = (arg: unknown): string => {
    if (typeof arg === 'string') {
      return arg;
    }
    try {
      return JSON.stringify(arg);
    } catch {
      return String(arg);
    }
  };

  private generateId(): string {
    return `log_${Date.now()}_${this.logIdCounter++}`;
  }

  /**
   * Основной метод логирования
   */
  log(level: LogLevel, category: LogCategory, message: string, details?: unknown): void {
    const entry: LogEntry = {
      id: this.generateId(),
      timestamp: Date.now(),
      level,
      category,
      message,
      details,
      stack: level === 'error' && details instanceof Error ? details.stack : undefined,
    };

    this.logs.push(entry);

    // Ограничиваем размер массива логов
    if (this.logs.length > this.maxLogs) {
      this.logs.shift();
    }

    // Сохраняем в localStorage
    this.saveToStorage();

    // Уведомляем слушателей
    this.notifyListeners(entry);

    // В продакшене можно добавить отправку на сервер
    if (import.meta.env.PROD && level === 'error') {
      this.sendToBackend(entry).catch(() => {
        // Игнорируем ошибки отправки
      });
    }
  }

  /**
   * Сокращённые методы для удобства
   */
  info(category: LogCategory, message: string, details?: unknown): void {
    this.log('info', category, message, details);
  }

  warn(category: LogCategory, message: string, details?: unknown): void {
    this.log('warn', category, message, details);
  }

  error(category: LogCategory, message: string, details?: unknown): void {
    this.log('error', category, message, details);
  }

  debug(category: LogCategory, message: string, details?: unknown): void {
    this.log('debug', category, message, details);
  }

  /**
   * Получить все логи или отфильтрованные
   */
  getLogs(filter?: { level?: LogLevel; category?: LogCategory; search?: string }): LogEntry[] {
    let filtered = [...this.logs];

    if (filter?.level) {
      filtered = filtered.filter(log => log.level === filter.level);
    }

    if (filter?.category) {
      filtered = filtered.filter(log => log.category === filter.category);
    }

    if (filter?.search) {
      const searchLower = filter.search.toLowerCase();
      filtered = filtered.filter(log =>
        log.message.toLowerCase().includes(searchLower) ||
        log.category.toLowerCase().includes(searchLower)
      );
    }

    return filtered;
  }

  /**
   * Очистить все логи
   */
  clear(): void {
    this.logs = [];
    this.saveToStorage();
    this.notifyListeners({
      id: this.generateId(),
      timestamp: Date.now(),
      level: 'info',
      category: 'system',
      message: 'Logs cleared',
    });
  }

  /**
   * Экспорт логов в текстовый файл
   */
  exportToFile(): string {
    const content = this.logs
      .map(log => {
        const date = new Date(log.timestamp).toISOString();
        const details = log.details ? `\n  Details: ${JSON.stringify(log.details, null, 2)}` : '';
        const stack = log.stack ? `\n  Stack: ${log.stack}` : '';
        return `[${date}] [${log.level.toUpperCase()}] [${log.category}] ${log.message}${details}${stack}`;
      })
      .join('\n\n');

    return content;
  }

  /**
   * Подписка на новые логи
   */
  subscribe(listener: LogListener): () => void {
    this.listeners.add(listener);
    return () => {
      this.listeners.delete(listener);
    };
  }

  /**
   * Уведомление слушателей о новом логе
   */
  private notifyListeners(entry: LogEntry): void {
    this.listeners.forEach(listener => {
      try {
        listener(entry);
      } catch (error) {
        // Избегаем рекурсии при ошибке в слушателе
        this.originalConsole.warn('Error in log listener:', error);
      }
    });
  }

  /**
   * Сохранение в localStorage
   */
  private saveToStorage(): void {
    try {
      // Сохраняем только последние 500 логов для экономии места
      const toSave = this.logs.slice(-500);
      localStorage.setItem('app_logs', JSON.stringify(toSave));
    } catch (error) {
      // Если localStorage переполнен, очищаем старые логи
      this.originalConsole.warn('Failed to save logs to storage:', error);
    }
  }

  /**
   * Загрузка из localStorage
   */
  private loadFromStorage(): void {
    try {
      const saved = localStorage.getItem('app_logs');
      if (saved) {
        this.logs = JSON.parse(saved);
      }
    } catch (error) {
      this.originalConsole.warn('Failed to load logs from storage:', error);
      this.logs = [];
    }
  }

  /**
   * Отправка критических ошибок на backend
   */
  private async sendToBackend(entry: LogEntry): Promise<void> {
    try {
      // Проверяем доступность Tauri API
      if (window.__TAURI__) {
        const { invoke } = await import('@tauri-apps/api/core');
        await invoke('log_error', {
          level: entry.level,
          category: entry.category,
          message: entry.message,
          details: JSON.stringify(entry.details),
          timestamp: entry.timestamp,
        });
      }
    } catch {
      // Игнорируем ошибки отправки
    }
  }

  /**
   * Получить статистику логов
   */
  getStats(): { total: number; byLevel: Record<LogLevel, number>; byCategory: Record<LogCategory, number> } {
    const byLevel: Record<LogLevel, number> = { info: 0, warn: 0, error: 0, debug: 0 };
    const byCategory: Record<LogCategory, number> = {
      app: 0,
      camera: 0,
      stream: 0,
      ptz: 0,
      network: 0,
      analytics: 0,
      auth: 0,
      system: 0,
    };

    this.logs.forEach(log => {
      byLevel[log.level]++;
      byCategory[log.category]++;
    });

    return {
      total: this.logs.length,
      byLevel,
      byCategory,
    };
  }
}

// Singleton instance
export const logger = new LoggerService();
