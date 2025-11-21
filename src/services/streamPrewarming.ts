/**
 * Stream Prewarming Service
 * Автоматически "прогревает" потоки для мгновенного появления видео
 */

import { invoke } from '@tauri-apps/api/core';

interface PrewarmConfig {
  enabled: boolean;
  prewarmBothQualities: boolean; // Прогревать SD и HD одновременно
  keepAliveInterval: number; // Интервал поддержания активности (мс)
  maxConcurrentPrewarms: number; // Максимум одновременных прогревов
}

interface StreamInfo {
  baseName: string;
  quality: 'sd' | 'hd';
  lastAccess: number;
  prewarmHandle?: number;
}

class StreamPrewarmingService {
  private static instance: StreamPrewarmingService;
  private config: PrewarmConfig = {
    enabled: true,
    prewarmBothQualities: true,
    keepAliveInterval: 30000, // 30 секунд
    maxConcurrentPrewarms: 10,
  };

  private activeStreams = new Map<string, StreamInfo>();
  private prewarmQueue: string[] = [];
  private isProcessingQueue = false;
  private keepAliveIntervals = new Map<string, ReturnType<typeof setInterval>>();

  private constructor() {
    // Загрузка конфигурации из localStorage
    this.loadConfig();
  }

  static getInstance(): StreamPrewarmingService {
    if (!StreamPrewarmingService.instance) {
      StreamPrewarmingService.instance = new StreamPrewarmingService();
    }
    return StreamPrewarmingService.instance;
  }

  /**
   * Загрузить конфигурацию из localStorage
   */
  private loadConfig(): void {
    try {
      const saved = localStorage.getItem('streamPrewarmingConfig');
      if (saved) {
        const parsed = JSON.parse(saved);
        this.config = { ...this.config, ...parsed };
      }
    } catch (error) {
      console.warn('[StreamPrewarming] Failed to load config:', error);
    }
  }

  /**
   * Сохранить конфигурацию в localStorage
   */
  saveConfig(config: Partial<PrewarmConfig>): void {
    this.config = { ...this.config, ...config };
    try {
      localStorage.setItem('streamPrewarmingConfig', JSON.stringify(this.config));
    } catch (error) {
      console.warn('[StreamPrewarming] Failed to save config:', error);
    }
  }

  getConfig(): PrewarmConfig {
    return { ...this.config };
  }

  /**
   * Прогреть поток для камеры
   * @param baseName - базовое имя камеры (без суффикса _0/_1)
   * @param priority - приоритет (true = добавить в начало очереди)
   */
  async prewarmStream(baseName: string, priority = false): Promise<void> {
    if (!this.config.enabled) {
      return;
    }

    console.log(`[StreamPrewarming] Prewarming stream: ${baseName}`);

    // Определяем какие потоки нужно прогреть
    const streamsToPrewarm: Array<{ name: string; quality: 'sd' | 'hd' }> = [];
    
    if (this.config.prewarmBothQualities) {
      streamsToPrewarm.push(
        { name: `${baseName}_0`, quality: 'sd' },
        { name: `${baseName}_1`, quality: 'hd' }
      );
    } else {
      // По умолчанию прогреваем SD
      streamsToPrewarm.push({ name: `${baseName}_0`, quality: 'sd' });
    }

    for (const { name, quality } of streamsToPrewarm) {
      if (!this.activeStreams.has(name)) {
        if (priority) {
          this.prewarmQueue.unshift(name);
        } else {
          this.prewarmQueue.push(name);
        }
        
        this.activeStreams.set(name, {
          baseName,
          quality,
          lastAccess: Date.now(),
        });
      } else {
        // Обновляем время последнего доступа
        const info = this.activeStreams.get(name)!;
        info.lastAccess = Date.now();
      }
    }

    // Запускаем обработку очереди
    void this.processQueue();
  }

  /**
   * Обработка очереди прогрева
   */
  private async processQueue(): Promise<void> {
    if (this.isProcessingQueue || this.prewarmQueue.length === 0) {
      return;
    }

    this.isProcessingQueue = true;

    try {
      // Обрабатываем до maxConcurrentPrewarms потоков одновременно
      const activeCount = Array.from(this.activeStreams.values()).filter(
        s => s.prewarmHandle !== undefined
      ).length;

      const canProcess = Math.min(
        this.config.maxConcurrentPrewarms - activeCount,
        this.prewarmQueue.length
      );

      const promises: Promise<void>[] = [];
      
      for (let i = 0; i < canProcess; i++) {
        const streamName = this.prewarmQueue.shift();
        if (streamName) {
          promises.push(this.prewarmSingleStream(streamName));
        }
      }

      await Promise.all(promises);
    } finally {
      this.isProcessingQueue = false;
      
      // Если в очереди еще есть потоки, продолжаем обработку
      if (this.prewarmQueue.length > 0) {
        void this.processQueue();
      }
    }
  }

  /**
   * Прогреть один поток
   */
  private async prewarmSingleStream(streamName: string): Promise<void> {
    try {
      const info = this.activeStreams.get(streamName);
      if (!info) return;

      console.log(`[StreamPrewarming] Starting prewarm for: ${streamName}`);

      // Проверяем, что поток онлайн
      const isOnline = await invoke<boolean>('check_go2rtc_stream_online', {
        streamName,
      });

      if (!isOnline) {
        console.warn(`[StreamPrewarming] Stream ${streamName} is not online`);
        this.activeStreams.delete(streamName);
        return;
      }

      // Делаем HEAD запрос к потоку для его активации в go2rtc
      // go2rtc начнет получать данные с камеры
      try {
        const response = await fetch(`http://localhost:1984/api/stream.html?src=${streamName}`, {
          method: 'HEAD',
        });
        
        if (response.ok) {
          console.log(`[StreamPrewarming] Prewarmed successfully: ${streamName}`);
          
          // Устанавливаем keep-alive для поддержания активности
          this.setupKeepAlive(streamName);
        }
      } catch (fetchError) {
        console.warn(`[StreamPrewarming] Failed to prewarm ${streamName}:`, fetchError);
      }

    } catch (error) {
      console.error(`[StreamPrewarming] Error prewarming ${streamName}:`, error);
      this.activeStreams.delete(streamName);
    }
  }

  /**
   * Установить keep-alive для потока
   */
  private setupKeepAlive(streamName: string): void {
    // Удаляем старый интервал если есть
    const existingInterval = this.keepAliveIntervals.get(streamName);
    if (existingInterval) {
      clearInterval(existingInterval);
    }

    // Создаем новый интервал
    const interval = setInterval(async () => {
      const info = this.activeStreams.get(streamName);
      if (!info) {
        this.stopKeepAlive(streamName);
        return;
      }

      // Проверяем, не устарел ли поток (не использовался > 5 минут)
      const now = Date.now();
      const inactiveTime = now - info.lastAccess;
      const maxInactiveTime = 5 * 60 * 1000; // 5 минут

      if (inactiveTime > maxInactiveTime) {
        console.log(`[StreamPrewarming] Stream ${streamName} inactive for ${Math.round(inactiveTime / 1000)}s, stopping keep-alive`);
        this.stopStream(streamName);
        return;
      }

      // Ping потока для поддержания активности
      try {
        await fetch(`http://localhost:1984/api/stream.html?src=${streamName}`, {
          method: 'HEAD',
        });
      } catch (error) {
        console.warn(`[StreamPrewarming] Keep-alive failed for ${streamName}:`, error);
      }
    }, this.config.keepAliveInterval);

    this.keepAliveIntervals.set(streamName, interval);
  }

  /**
   * Остановить keep-alive для потока
   */
  private stopKeepAlive(streamName: string): void {
    const interval = this.keepAliveIntervals.get(streamName);
    if (interval) {
      clearInterval(interval);
      this.keepAliveIntervals.delete(streamName);
    }
  }

  /**
   * Отметить поток как активно используемый
   */
  touchStream(streamName: string): void {
    const info = this.activeStreams.get(streamName);
    if (info) {
      info.lastAccess = Date.now();
    }
  }

  /**
   * Остановить прогрев потока
   */
  stopStream(streamName: string): void {
    console.log(`[StreamPrewarming] Stopping stream: ${streamName}`);
    this.stopKeepAlive(streamName);
    this.activeStreams.delete(streamName);
  }

  /**
   * Остановить все потоки
   */
  stopAll(): void {
    console.log('[StreamPrewarming] Stopping all streams');
    
    for (const streamName of this.activeStreams.keys()) {
      this.stopKeepAlive(streamName);
    }
    
    this.activeStreams.clear();
    this.prewarmQueue = [];
  }

  /**
   * Получить информацию об активных потоках
   */
  getActiveStreams(): Map<string, StreamInfo> {
    return new Map(this.activeStreams);
  }

  /**
   * Получить статус прогрева
   */
  getStatus() {
    return {
      enabled: this.config.enabled,
      activeStreams: this.activeStreams.size,
      queueLength: this.prewarmQueue.length,
      streams: Array.from(this.activeStreams.entries()).map(([name, info]) => ({
        name,
        baseName: info.baseName,
        quality: info.quality,
        lastAccess: info.lastAccess,
        inactiveDuration: Date.now() - info.lastAccess,
      })),
    };
  }
}

export const streamPrewarmingService = StreamPrewarmingService.getInstance();
export type { PrewarmConfig, StreamInfo };
