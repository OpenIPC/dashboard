import { createContext } from 'react';

export type SupportedLanguage = 'en' | 'ru';

export interface TranslationValue {
  [key: string]: string | TranslationValue;
}

export interface Translations {
  [key: string]: TranslationValue;
}

export interface LocalizationContextType {
  currentLanguage: SupportedLanguage;
  setLanguage: (language: SupportedLanguage) => void;
  t: (key: string, params?: Record<string, string | number>) => string;
  translations: Translations;
}

export const LocalizationContext = createContext<LocalizationContextType | undefined>(undefined);
