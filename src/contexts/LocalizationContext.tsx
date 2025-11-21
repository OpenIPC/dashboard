import React, { useState, useEffect } from 'react';
import type { ReactNode } from 'react';
import {
  LocalizationContext,
  type LocalizationContextType,
  type SupportedLanguage,
  type Translations,
  type TranslationValue,
} from './LocalizationContextData';

const isTranslationValue = (value: TranslationValue | string): value is TranslationValue =>
  typeof value === 'object' && value !== null;

const STORAGE_KEY = 'vms_dashboard_language';

export const LocalizationProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [currentLanguage, setCurrentLanguage] = useState<SupportedLanguage>(() => {
    const saved = localStorage.getItem(STORAGE_KEY);
    return (saved as SupportedLanguage) || 'en';
  });
  
  const [translations, setTranslations] = useState<Translations>({});

  // Load translations
  useEffect(() => {
    const loadTranslations = async () => {
      try {
        const response = await fetch(`/locales/${currentLanguage}.json`);
        const translationData = await response.json();
        setTranslations(translationData);
      } catch (error) {
        console.error(`Failed to load translations for ${currentLanguage}:`, error);
        // Fallback to English if loading fails and not already English
        if (currentLanguage !== 'en') {
          try {
            const fallbackResponse = await fetch('/locales/en.json');
            const fallbackData = await fallbackResponse.json();
            setTranslations(fallbackData);
          } catch (fallbackError) {
            console.error('Failed to load fallback translations:', fallbackError);
          }
        }
      }
    };

    loadTranslations();
  }, [currentLanguage]);

  const setLanguage = (language: SupportedLanguage) => {
    setCurrentLanguage(language);
    localStorage.setItem(STORAGE_KEY, language);
  };

  // Translation function with parameter substitution
  const t = (key: string, params?: Record<string, string | number>): string => {
    const keys = key.split('.');
    let current: TranslationValue | string = translations;

    for (const k of keys) {
      if (!isTranslationValue(current)) {
        return `[${key}]`;
      }

  const next: string | TranslationValue | undefined = current[k];
      if (next === undefined) {
        // Return key if translation not found (development mode indicator)
        return `[${key}]`;
      }

      current = next;
    }

    if (typeof current !== 'string') {
      return `[${key}]`;
    }
    
    // Replace parameters in the format {{param}}
    if (params) {
      return current.replace(/\{\{(\w+)\}\}/g, (match, paramKey) => {
        return params[paramKey]?.toString() || match;
      });
    }
    
    return current;
  };

  const contextValue: LocalizationContextType = {
    currentLanguage,
    setLanguage,
    t,
    translations,
  };

  return (
    <LocalizationContext.Provider value={contextValue}>
      {children}
    </LocalizationContext.Provider>
  );
};

export default LocalizationContext;