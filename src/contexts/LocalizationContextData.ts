import { createContext } from 'react';

export type SupportedLanguage = 'en' | 'ru';

export type TranslationValue = string | { [key: string]: TranslationValue };

export type Translations = { [key: string]: TranslationValue };

export interface LocalizationContextType {
  currentLanguage: SupportedLanguage;
  setLanguage: (language: SupportedLanguage) => void;
  t: (key: string, params?: Record<string, string | number>) => string;
  translations: Translations;
}

export const LocalizationContext = createContext<LocalizationContextType | undefined>(undefined);
