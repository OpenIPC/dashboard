/**
 * WebRTC Stats Context
 * Глобальное хранилище статистики WebRTC соединений для всех активных потоков
 */

import React, { createContext, useContext, useState, useCallback } from 'react';
import type { ReactNode } from 'react';
import type { WebRTCStats } from '../services/webrtcStats';

interface WebRTCStatsEntry {
  streamName: string;
  stats: WebRTCStats;
  quality: 'excellent' | 'good' | 'fair' | 'poor' | 'unknown';
  lastUpdated: number;
}

interface WebRTCStatsContextValue {
  getAllStats: () => WebRTCStatsEntry[]; // Функция-getter вместо прямого массива
  registerStream: (streamName: string, stats: WebRTCStats, quality: string) => void;
  unregisterStream: (streamName: string) => void;
  clearAll: () => void;
}

const WebRTCStatsContext = createContext<WebRTCStatsContextValue | undefined>(undefined);

export const WebRTCStatsProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  // КРИТИЧНО: Используем ref вместо state чтобы избежать re-render всего дерева компонентов
  // Stats обновляются каждые 500ms - это вызывало бы тысячи ненужных re-renders
  const allStatsRef = React.useRef<WebRTCStatsEntry[]>([]);
  
  // Getter для безопасного доступа к current stats
  const getAllStats = useCallback(() => allStatsRef.current, []);

  const registerStream = useCallback((streamName: string, stats: WebRTCStats, quality: string) => {
    const prev = allStatsRef.current;
    const existing = prev.findIndex(entry => entry.streamName === streamName);
    const newEntry: WebRTCStatsEntry = {
      streamName,
      stats,
      quality: quality as WebRTCStatsEntry['quality'],
      lastUpdated: Date.now(),
    };

    if (existing >= 0) {
      const updated = [...prev];
      updated[existing] = newEntry;
      allStatsRef.current = updated;
    } else {
      allStatsRef.current = [...prev, newEntry];
    }
  }, []);

  const unregisterStream = useCallback((streamName: string) => {
    allStatsRef.current = allStatsRef.current.filter(entry => entry.streamName !== streamName);
  }, []);

  const clearAll = useCallback(() => {
    allStatsRef.current = [];
  }, []);

  return (
    <WebRTCStatsContext.Provider value={{ getAllStats, registerStream, unregisterStream, clearAll }}>
      {children}
    </WebRTCStatsContext.Provider>
  );
};

export const useWebRTCStatsContext = () => {
  const context = useContext(WebRTCStatsContext);
  if (!context) {
    throw new Error('useWebRTCStatsContext must be used within WebRTCStatsProvider');
  }
  return context;
};
