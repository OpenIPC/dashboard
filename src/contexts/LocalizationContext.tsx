import React, { useState, useEffect } from 'react';
import type { ReactNode } from 'react';
import {
  LocalizationContext,
  type LocalizationContextType,
  type SupportedLanguage,
  type Translations,
  type TranslationValue,
} from './LocalizationContextData';
import enTranslations from '../locales/en.json';
import ruTranslations from '../locales/ru.json';

const isTranslationObject = (
  value: TranslationValue,
): value is Exclude<TranslationValue, string> => typeof value === 'object' && value !== null;

const STORAGE_KEY = 'vms_dashboard_language';
const STATIC_TRANSLATIONS: Record<SupportedLanguage, Translations> = {
  en: enTranslations,
  ru: ruTranslations,
};

export const LocalizationProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [currentLanguage, setCurrentLanguage] = useState<SupportedLanguage>(() => {
    const saved = localStorage.getItem(STORAGE_KEY);
    return (saved as SupportedLanguage) || 'en';
  });
  
  const [translations, setTranslations] = useState<Translations>(
    STATIC_TRANSLATIONS[currentLanguage] ?? STATIC_TRANSLATIONS.en,
  );

  // Load translations
  useEffect(() => {
    const nextTranslations = STATIC_TRANSLATIONS[currentLanguage];
    if (nextTranslations) {
      setTranslations(nextTranslations);
    } else {
      console.warn(`Missing translations for ${currentLanguage}, falling back to English.`);
      setTranslations(STATIC_TRANSLATIONS.en);
    }
  }, [currentLanguage]);

  const setLanguage = (language: SupportedLanguage) => {
    setCurrentLanguage(language);
    localStorage.setItem(STORAGE_KEY, language);
  };

  // Translation function with parameter substitution
  const t = (key: string, params?: Record<string, string | number>): string => {
    const keys = key.split('.');
    let current: TranslationValue = translations;

    for (const k of keys) {
      if (!isTranslationObject(current)) {
        return `[${key}]`;
      }

      const next: TranslationValue | undefined = current[k];
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