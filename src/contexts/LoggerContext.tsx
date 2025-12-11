import React, { createContext, useContext, useEffect, useState, useCallback } from 'react';
import { logger } from '../services/logger';
import type { LogEntry, LogLevel, LogCategory } from '../services/logger';

interface LoggerContextType {
  logs: LogEntry[];
  addLog: (level: LogLevel, category: LogCategory, message: string, details?: unknown) => void;
  clearLogs: () => void;
  exportLogs: () => string;
  getFilteredLogs: (filter?: { level?: LogLevel; category?: LogCategory; search?: string }) => LogEntry[];
  stats: { total: number; byLevel: Record<LogLevel, number>; byCategory: Record<LogCategory, number> };
}

const LoggerContext = createContext<LoggerContextType | undefined>(undefined);

export const LoggerProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [logs, setLogs] = useState<LogEntry[]>([]);
  const [stats, setStats] = useState(logger.getStats());

  // Подписываемся на новые логи
  useEffect(() => {
    // Загружаем существующие логи
    setLogs(logger.getLogs());

    // Подписываемся на обновления
    const unsubscribe = logger.subscribe((entry) => {
      setLogs(prev => [...prev, entry]);
      setStats(logger.getStats());
    });

    return unsubscribe;
  }, []);

  const addLog = useCallback((level: LogLevel, category: LogCategory, message: string, details?: unknown) => {
    logger.log(level, category, message, details);
  }, []);

  const clearLogs = useCallback(() => {
    logger.clear();
    setLogs([]);
    setStats(logger.getStats());
  }, []);

  const exportLogs = useCallback(() => {
    return logger.exportToFile();
  }, []);

  const getFilteredLogs = useCallback((filter?: { level?: LogLevel; category?: LogCategory; search?: string }) => {
    return logger.getLogs(filter);
  }, []);

  return (
    <LoggerContext.Provider value={{ logs, addLog, clearLogs, exportLogs, getFilteredLogs, stats }}>
      {children}
    </LoggerContext.Provider>
  );
};

export const useLogger = () => {
  const context = useContext(LoggerContext);
  if (!context) {
    throw new Error('useLogger must be used within LoggerProvider');
  }
  return context;
};
