import React, { createContext, useContext, useMemo, useState } from 'react';

interface LoggerUiContextValue {
  isOpen: boolean;
  openViewer: () => void;
  closeViewer: () => void;
  toggleViewer: () => void;
}

const LoggerUiContext = createContext<LoggerUiContextValue | undefined>(undefined);

export const LoggerUiProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [isOpen, setIsOpen] = useState(false);

  const value = useMemo<LoggerUiContextValue>(() => ({
    isOpen,
    openViewer: () => setIsOpen(true),
    closeViewer: () => setIsOpen(false),
    toggleViewer: () => setIsOpen(prev => !prev),
  }), [isOpen]);

  return <LoggerUiContext.Provider value={value}>{children}</LoggerUiContext.Provider>;
};

export const useLoggerUi = (): LoggerUiContextValue => {
  const ctx = useContext(LoggerUiContext);
  if (!ctx) {
    throw new Error('useLoggerUi must be used within LoggerUiProvider');
  }
  return ctx;
};
