/**
 * Optimized Stream Hook
 * Быстрое переключение между SD/HD с кэшированием WebRTC соединений
 */

import { useEffect, useRef, useCallback } from 'react';
import { streamPrewarmingService } from '../services/streamPrewarming';

interface StreamConnection {
  pc: RTCPeerConnection;
  quality: 'sd' | 'hd';
  lastUsed: number;
  isActive: boolean;
}

interface OptimizedStreamOptions {
  baseName: string;
  quality: 'sd' | 'hd';
  videoElement: HTMLVideoElement | null;
  enableCaching?: boolean;
  onConnectionChange?: (method: string) => void;
  onError?: (error: string) => void;
}

// Глобальный кэш соединений (shared между всеми компонентами)
const connectionCache = new Map<string, StreamConnection>();
const CACHE_CLEANUP_INTERVAL = 60000; // 1 минута
const MAX_CACHE_AGE = 300000; // 5 минут

// Периодическая очистка кэша
setInterval(() => {
  const now = Date.now();
  for (const [key, conn] of connectionCache.entries()) {
    if (!conn.isActive && now - conn.lastUsed > MAX_CACHE_AGE) {
      console.log(`[OptimizedStream] Cleaning up cached connection: ${key}`);
      conn.pc.close();
      connectionCache.delete(key);
    }
  }
}, CACHE_CLEANUP_INTERVAL);

/**
 * Хук для оптимизированной работы с видеопотоками
 */
export function useOptimizedStream(options: OptimizedStreamOptions) {
  const {
    baseName,
    quality,
    videoElement,
    enableCaching = true,
    onConnectionChange,
    onError,
  } = options;

  const currentConnectionRef = useRef<RTCPeerConnection | null>(null);
  const isConnectingRef = useRef(false);

  /**
   * Получить ключ для кэша
   */
  const getCacheKey = useCallback((name: string, qual: 'sd' | 'hd') => {
    return `${name}_${qual}`;
  }, []);

  /**
   * Получить имя потока с суффиксом качества
   */
  const getStreamName = useCallback((qual: 'sd' | 'hd') => {
    return `${baseName}${qual === 'hd' ? '_1' : '_0'}`;
  }, [baseName]);

  /**
   * Создать WebRTC соединение
   */
  const createWebRTCConnection = useCallback(
    async (qual: 'sd' | 'hd'): Promise<RTCPeerConnection> => {
      const streamName = getStreamName(qual);
      
      console.log(`[OptimizedStream] Creating WebRTC connection for ${streamName}`);

      const pc = new RTCPeerConnection({
        iceServers: [{ urls: 'stun:stun.l.google.com:19302' }],
        bundlePolicy: 'max-bundle',
        rtcpMuxPolicy: 'require',
      });

      // Настройка для минимальной задержки
      const transceivers = pc.getTransceivers();
      transceivers.forEach(transceiver => {
        const sender = transceiver.sender;
        const params = sender.getParameters();
        if (!params.encodings) {
          params.encodings = [{}];
        }
        params.encodings[0].priority = 'high';
        params.encodings[0].networkPriority = 'high';
        sender.setParameters(params);
      });

      // Добавляем transceiver для получения видео
      pc.addTransceiver('video', {
        direction: 'recvonly',
      });

      // Создаем offer
      const offer = await pc.createOffer();
      await pc.setLocalDescription(offer);

      // Отправляем offer на go2rtc
      const go2rtcUrl = `http://localhost:1984/api/webrtc?src=${streamName}`;
      const response = await fetch(go2rtcUrl, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          type: offer.type,
          sdp: offer.sdp,
        }),
      });

      if (!response.ok) {
        throw new Error(`WebRTC negotiation failed: ${response.status}`);
      }

      const answer = await response.json();
      await pc.setRemoteDescription(answer);

      // Ждем ICE connected
      await new Promise<void>((resolve, reject) => {
        const timeout = setTimeout(() => {
          reject(new Error('ICE connection timeout'));
        }, 10000);

        pc.oniceconnectionstatechange = () => {
          console.log(`[OptimizedStream] ICE state: ${pc.iceConnectionState}`);
          if (pc.iceConnectionState === 'connected' || pc.iceConnectionState === 'completed') {
            clearTimeout(timeout);
            resolve();
          } else if (pc.iceConnectionState === 'failed') {
            clearTimeout(timeout);
            reject(new Error('ICE connection failed'));
          }
        };
      });

      console.log(`[OptimizedStream] WebRTC connection established for ${streamName}`);
      return pc;
    },
    [baseName, getStreamName]
  );

  /**
   * Подключить поток к video элементу
   */
  const connectStream = useCallback(async () => {
    if (!videoElement || isConnectingRef.current) {
      return;
    }

    isConnectingRef.current = true;

    try {
      const streamName = getStreamName(quality);
      const cacheKey = getCacheKey(baseName, quality);

      // Отмечаем поток как используемый для prewarming
      streamPrewarmingService.touchStream(streamName);

      let pc: RTCPeerConnection;
      let fromCache = false;

      // Проверяем кэш
      if (enableCaching && connectionCache.has(cacheKey)) {
        const cached = connectionCache.get(cacheKey)!;
        
        // Проверяем, что соединение еще живо
        if (
          cached.pc.iceConnectionState === 'connected' ||
          cached.pc.iceConnectionState === 'completed'
        ) {
          console.log(`[OptimizedStream] Using cached connection: ${cacheKey}`);
          pc = cached.pc;
          cached.isActive = true;
          cached.lastUsed = Date.now();
          fromCache = true;
        } else {
          console.log(`[OptimizedStream] Cached connection dead, creating new: ${cacheKey}`);
          cached.pc.close();
          connectionCache.delete(cacheKey);
          pc = await createWebRTCConnection(quality);
        }
      } else {
        pc = await createWebRTCConnection(quality);
      }

      // Закрываем предыдущее соединение если оно было
      if (currentConnectionRef.current && currentConnectionRef.current !== pc) {
        const oldPc = currentConnectionRef.current;
        
        // Если кэширование включено, сохраняем в кэш
        if (enableCaching) {
          // Находим ключ старого соединения
          for (const [key, conn] of connectionCache.entries()) {
            if (conn.pc === oldPc) {
              conn.isActive = false;
              conn.lastUsed = Date.now();
              console.log(`[OptimizedStream] Moved connection to cache: ${key}`);
              break;
            }
          }
        } else {
          oldPc.close();
        }
      }

      currentConnectionRef.current = pc;

      // Добавляем в кэш если это новое соединение
      if (!fromCache && enableCaching) {
        connectionCache.set(cacheKey, {
          pc,
          quality,
          lastUsed: Date.now(),
          isActive: true,
        });
      }

      // Получаем MediaStream из PeerConnection
      const stream = new MediaStream();
      pc.getReceivers().forEach(receiver => {
        if (receiver.track) {
          stream.addTrack(receiver.track);
        }
      });

      // Привязываем к video элементу
      videoElement.srcObject = stream;
      
      // Настройки video элемента для минимальной задержки
      videoElement.autoplay = true;
      videoElement.playsInline = true;
      videoElement.muted = true;
      
      // Пытаемся воспроизвести
      try {
        await videoElement.play();
        onConnectionChange?.('WebRTC' + (fromCache ? ' (cached)' : ''));
      } catch (playError) {
        console.warn('[OptimizedStream] Autoplay failed:', playError);
      }

    } catch (error) {
      console.error('[OptimizedStream] Connection failed:', error);
      onError?.(error instanceof Error ? error.message : String(error));
    } finally {
      isConnectingRef.current = false;
    }
  }, [
    videoElement,
    quality,
    baseName,
    enableCaching,
    getCacheKey,
    getStreamName,
    createWebRTCConnection,
    onConnectionChange,
    onError,
  ]);

  /**
   * Отключить поток
   */
  const disconnectStream = useCallback(() => {
    if (currentConnectionRef.current) {
      const pc = currentConnectionRef.current;
      
      if (enableCaching) {
        // Переводим соединение в неактивное состояние
        for (const conn of connectionCache.values()) {
          if (conn.pc === pc) {
            conn.isActive = false;
            conn.lastUsed = Date.now();
            break;
          }
        }
      } else {
        pc.close();
      }
      
      currentConnectionRef.current = null;
    }

    if (videoElement) {
      videoElement.srcObject = null;
    }
  }, [videoElement, enableCaching]);

  /**
   * Прогреть оба потока (SD и HD) для быстрого переключения
   */
  const prewarmBothQualities = useCallback(async () => {
    if (!enableCaching) return;

    const qualities: Array<'sd' | 'hd'> = ['sd', 'hd'];
    
    for (const qual of qualities) {
      const cacheKey = getCacheKey(baseName, qual);
      
      // Пропускаем если уже в кэше
      if (connectionCache.has(cacheKey)) {
        continue;
      }

      try {
        const pc = await createWebRTCConnection(qual);
        connectionCache.set(cacheKey, {
          pc,
          quality: qual,
          lastUsed: Date.now(),
          isActive: false,
        });
        console.log(`[OptimizedStream] Prewarmed ${cacheKey}`);
      } catch (error) {
        console.warn(`[OptimizedStream] Failed to prewarm ${cacheKey}:`, error);
      }
    }
  }, [baseName, enableCaching, getCacheKey, createWebRTCConnection]);

  // Подключаемся при изменении качества или video элемента
  useEffect(() => {
    void connectStream();

    return () => {
      disconnectStream();
    };
  }, [connectStream, disconnectStream]);

  // Прогреваем оба качества при монтировании
  useEffect(() => {
    void prewarmBothQualities();
  }, [prewarmBothQualities]);

  return {
    reconnect: connectStream,
    disconnect: disconnectStream,
    prewarmBothQualities,
    cacheStats: {
      cacheSize: connectionCache.size,
      cacheKeys: Array.from(connectionCache.keys()),
    },
  };
}

/**
 * Очистить весь кэш соединений
 */
export function clearStreamCache(): void {
  console.log('[OptimizedStream] Clearing all cached connections');
  for (const conn of connectionCache.values()) {
    conn.pc.close();
  }
  connectionCache.clear();
}

/**
 * Получить статистику кэша
 */
export function getStreamCacheStats() {
  return {
    totalConnections: connectionCache.size,
    activeConnections: Array.from(connectionCache.values()).filter(c => c.isActive).length,
    connections: Array.from(connectionCache.entries()).map(([key, conn]) => ({
      key,
      quality: conn.quality,
      isActive: conn.isActive,
      lastUsed: conn.lastUsed,
      iceState: conn.pc.iceConnectionState,
      connectionState: conn.pc.connectionState,
    })),
  };
}
