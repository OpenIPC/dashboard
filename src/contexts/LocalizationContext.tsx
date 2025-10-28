import React, { createContext, useContext, useState, useEffect } from 'react';
import type { ReactNode } from 'react';

export type SupportedLanguage = 'en' | 'ru';

interface TranslationValue {
  [key: string]: string | TranslationValue;
}

interface Translations {
  [key: string]: TranslationValue;
}

interface LocalizationContextType {
  currentLanguage: SupportedLanguage;
  setLanguage: (language: SupportedLanguage) => void;
  t: (key: string, params?: Record<string, string | number>) => string;
  translations: Translations;
}

const LocalizationContext = createContext<LocalizationContextType | undefined>(undefined);

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
    let value: any = translations;
    
    for (const k of keys) {
      if (value && typeof value === 'object' && k in value) {
        value = value[k];
      } else {
        // Return key if translation not found (development mode indicator)
        return `[${key}]`;
      }
    }
    
    if (typeof value !== 'string') {
      return `[${key}]`;
    }
    
    // Replace parameters in the format {{param}}
    if (params) {
      return value.replace(/\{\{(\w+)\}\}/g, (match, paramKey) => {
        return params[paramKey]?.toString() || match;
      });
    }
    
    return value;
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

export const useLocalization = (): LocalizationContextType => {
  const context = useContext(LocalizationContext);
  if (!context) {
    throw new Error('useLocalization must be used within a LocalizationProvider');
  }
  return context;
};

export default LocalizationContext;