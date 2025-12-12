import React, { useState, useEffect, useCallback, useMemo } from 'react';
import { open as openDialog, save, ask } from '@tauri-apps/plugin-dialog';
import { open as openExternal } from '@tauri-apps/plugin-shell';
import { check } from '@tauri-apps/plugin-updater';
import { relaunch } from '@tauri-apps/plugin-process';
import { invoke } from '@tauri-apps/api/core';
import { getVersion } from '@tauri-apps/api/app';
import { useLocalization } from '../hooks/useLocalization';
import type { SupportedLanguage } from '../contexts/LocalizationContextData';
import { Toast } from './Toast';
import { useToast } from '../hooks/useToast';
import { useAnalytics } from '../hooks/useAnalytics';
import type { AnalyticsModuleStatus, FaceSnapshotMode } from '../services/analytics';
import './SettingsModal.css';

interface SettingsModalProps {
  isOpen: boolean;
  onClose: () => void;
}

type StreamingProvider = 'go2rtc';

const STREAMING_PROVIDERS: StreamingProvider[] = ['go2rtc'];

const isStreamingProvider = (value: unknown): value is StreamingProvider =>
  typeof value === 'string' && STREAMING_PROVIDERS.includes(value as StreamingProvider);

interface StreamingBackendSettings {
  provider: StreamingProvider;
  enableOnDemand: boolean;
  restartOnConfigChange: boolean;
}

const DEFAULT_STREAMING_SETTINGS: StreamingBackendSettings = {
  provider: 'go2rtc',
  enableOnDemand: true,
  restartOnConfigChange: true,
};

const createDefaultStreamingSettings = (): StreamingBackendSettings => ({
  ...DEFAULT_STREAMING_SETTINGS,
});

interface Go2RtcEnhancedSettings {
  showMonitor: boolean;
  enableSnapshot: boolean;
  enable2WayAudio: boolean;
  enableAdaptiveBitrate: boolean;
}

const DEFAULT_GO2RTC_ENHANCED: Go2RtcEnhancedSettings = {
  showMonitor: false,
  enableSnapshot: true,
  enable2WayAudio: false,
  enableAdaptiveBitrate: true,
};

interface StreamOptimizationSettings {
  enableFastStart: boolean; // Быстрый старт с минимальными задержками
  enablePrewarming: boolean; // Прогрев потоков в фоне
  prewarmBothQualities: boolean; // Прогревать SD и HD одновременно
  enableConnectionCaching: boolean; // Кэширование WebRTC соединений
  maxCachedConnections: number; // Максимум кэшированных соединений
  keepAliveInterval: number; // Интервал keep-alive (секунды)
  
  // Мгновенное переключение качества
  keepHdStreamAlive: boolean; // Держать HD поток всегда активным для мгновенного переключения
  
  // Low-latency настройки WebRTC
  enableLowLatency: boolean; // Включить режим минимальной задержки
  playoutDelayHint: number; // Задержка воспроизведения (0-2000 мс)
  jitterBufferTarget: number; // Размер джиттер-буфера (0-1000 мс)
  
  // Автоматическая коррекция задержки
  enableLatencyMonitoring: boolean; // Мониторинг и коррекция задержки
  maxBufferedLatency: number; // Макс. задержка перед прыжком на live (секунды)
  latencyCheckInterval: number; // Интервал проверки задержки (секунды)
  
  // RTSP оптимизации
  rtspBufferSize: number; // Размер RTSP буфера (0 = минимум)
}

const DEFAULT_STREAM_OPTIMIZATION: StreamOptimizationSettings = {
  enableFastStart: true,
  enablePrewarming: true,
  prewarmBothQualities: true,
  enableConnectionCaching: true,
  maxCachedConnections: 10,
  keepAliveInterval: 30,
  
  // Мгновенное переключение включено по умолчанию
  keepHdStreamAlive: true,
  
  // Low-latency по умолчанию включено
  enableLowLatency: true,
  playoutDelayHint: 0, // Минимальная задержка
  jitterBufferTarget: 0, // Минимальный джиттер-буфер
  
  // Автоматическая коррекция включена
  enableLatencyMonitoring: true,
  maxBufferedLatency: 1.0, // 1 секунда
  latencyCheckInterval: 2, // Проверка каждые 2 секунды
  
  // RTSP с минимальным буфером
  rtspBufferSize: 0,
};

interface AppSettings {
  language: string;
  recordingsPath: string;
  screenshotsPath: string;
  hwAccel: string;
  notifications_enabled: boolean;
  qscale: number;
  fps: number;
  analytics_resize_width: number;
  analytics_frame_skip: number;
  analytics_record_duration: number;
  analytics_provider: string;
  anpr_detection_confidence: number;
  anpr_crop_expansion: number;
  anpr_crnn_confidence: number;
  anpr_python_confidence: number;
  anpr_enable_python: boolean;
  enabledModules: string[];
  streaming: StreamingBackendSettings;
  go2rtcEnhanced: Go2RtcEnhancedSettings;
  streamOptimization: StreamOptimizationSettings;
}

interface ModuleInfo {
  id: string;
  name: string;
  version: string;
  description: string;
  author: string;
}

const MODULE_METADATA: Record<string, ModuleInfo> = {
  'face-detector': {
    id: 'face-detector',
    name: 'Face Detector',
    version: '0.0.1',
    description: 'Обнаруживает и сохраняет лица людей из видеопотока.',
    author: 'Rinibr',
  },
  'license-plate-detector': {
    id: 'license-plate-detector',
    name: 'License Plate Detector',
    version: '0.0.1',
    description: 'Сохраняет обрезанные изображения обнаруженных номерных знаков и уведомляет интерфейс.',
    author: 'Rinibr',
  },
  'object-counter': {
    id: 'object-counter',
    name: 'Object Counter',
    version: '0.0.1',
    description: 'Отображает счетчик обнаруженных объектов в ячейке.',
    author: 'Rinibr',
  },
};

const FACE_SNAPSHOT_MODE_VALUES: FaceSnapshotMode[] = [
  'disabled',
  'standard',
  'anonymized',
  'encrypted',
];

interface FaceSnapshotModeCopy {
  label: string;
  description: string;
}

const FACE_SNAPSHOT_MODE_COPY: Record<FaceSnapshotMode, { en: FaceSnapshotModeCopy; ru: FaceSnapshotModeCopy }> = {
  disabled: {
    en: {
      label: 'Disabled',
      description: 'Face snapshots are not captured.',
    },
    ru: {
      label: 'Отключено',
      description: 'Снимки лиц не сохраняются.',
    },
  },
  standard: {
    en: {
      label: 'Standard',
      description: 'Faces are saved as-is without additional processing.',
    },
    ru: {
      label: 'Стандартный',
      description: 'Снимки сохраняются без дополнительной обработки.',
    },
  },
  anonymized: {
    en: {
      label: 'Anonymized',
      description: 'Snapshots are blurred before being stored.',
    },
    ru: {
      label: 'Анонимный',
      description: 'Перед сохранением лица размываются.',
    },
  },
  encrypted: {
    en: {
      label: 'Encrypted',
      description: 'Snapshots are encrypted with your key and stored as .bin files.',
    },
    ru: {
      label: 'Зашифрованный',
      description: 'Снимки шифруются вашим ключом и сохраняются как .bin.',
    },
  },
};

const DEFAULT_FACE_SNAPSHOT_MODE: FaceSnapshotMode = 'standard';

const isStringArray = (value: unknown): value is string[] =>
  Array.isArray(value) && value.every(item => typeof item === 'string');

const SettingsModal: React.FC<SettingsModalProps> = ({ isOpen, onClose }) => {
  const { currentLanguage, setLanguage, t } = useLocalization();
  const { toast, showToast, hideToast } = useToast();
  const {
    modules,
    isLoadingModules,
    moduleOperationId,
    refreshModules,
    toggleModule,
    updateModuleSnapshotsDir,
    updateModuleConfig,
  } = useAnalytics();
  const [activeTab, setActiveTab] = useState('tab-general');
  const [appVersion, setAppVersion] = useState('Loading...');
  const [settings, setSettings] = useState<AppSettings>({
    language: currentLanguage,
    recordingsPath: 'C:\\Users\\vavol\\Videos\\OpenIPC-VMS',
    screenshotsPath: 'C:\\Users\\vavol\\Videos\\OpenIPC-VMS\\Screenshots',
    hwAccel: 'auto',
    notifications_enabled: true,
    qscale: 8,
    fps: 20,
    analytics_resize_width: 640,
    analytics_frame_skip: 5,
    analytics_record_duration: 30,
    analytics_provider: 'auto',
    anpr_detection_confidence: 0.5,
    anpr_crop_expansion: 1.2,
    anpr_crnn_confidence: 0.75,
    anpr_python_confidence: 0.90,
    anpr_enable_python: true,
    enabledModules: [],
    streaming: createDefaultStreamingSettings(),
    go2rtcEnhanced: DEFAULT_GO2RTC_ENHANCED,
    streamOptimization: DEFAULT_STREAM_OPTIMIZATION,
  });
  const [faceSnapshotKeyInputs, setFaceSnapshotKeyInputs] = useState<Record<string, string>>({});
  const [faceSnapshotKeyErrors, setFaceSnapshotKeyErrors] = useState<Record<string, string | null>>({});
  const [faceSnapshotKeyBusy, setFaceSnapshotKeyBusy] = useState<Record<string, boolean>>({});

  // Safe settings with guaranteed defaults - prevents crashes when accessing nested properties
  const safeSettings = useMemo(() => ({
    ...settings,
    go2rtcEnhanced: settings.go2rtcEnhanced ?? DEFAULT_GO2RTC_ENHANCED,
    streamOptimization: settings.streamOptimization ?? DEFAULT_STREAM_OPTIMIZATION,
  }), [settings]);
  const languageKey = currentLanguage === 'ru' ? 'ru' : 'en';

  const normalizeStreamingSettings = useCallback((input?: unknown): StreamingBackendSettings => {
    const base = createDefaultStreamingSettings();
    if (!input || typeof input !== 'object' || input === null) {
      return base;
    }

    const source = input as Record<string, unknown>;

    return {
      provider:
        isStreamingProvider(source.provider) ? source.provider : base.provider,
      enableOnDemand:
        typeof source.enableOnDemand === 'boolean'
          ? source.enableOnDemand
          : base.enableOnDemand,
      restartOnConfigChange:
        typeof source.restartOnConfigChange === 'boolean'
          ? source.restartOnConfigChange
          : base.restartOnConfigChange,
    };
  }, []);

  const mergeAppSettings = useCallback(
    (prev: AppSettings, incoming?: Partial<AppSettings> | null): AppSettings => {
      if (!incoming) {
        return {
          ...prev,
          language: currentLanguage,
          streaming: normalizeStreamingSettings(prev.streaming),
          go2rtcEnhanced: prev.go2rtcEnhanced ?? DEFAULT_GO2RTC_ENHANCED,
          streamOptimization: prev.streamOptimization ?? DEFAULT_STREAM_OPTIMIZATION,
        };
      }

  const incomingSafe: Partial<AppSettings> = incoming ?? {};
  const { streaming, enabledModules, go2rtcEnhanced, streamOptimization, ...rest } = incomingSafe;

      const resolvedModules = isStringArray(enabledModules)
        ? enabledModules
        : prev.enabledModules;

      const next = {
        ...prev,
        ...rest,
        streaming: normalizeStreamingSettings(streaming ?? prev.streaming),
        enabledModules: resolvedModules,
        language: currentLanguage,
        go2rtcEnhanced: go2rtcEnhanced ? { ...DEFAULT_GO2RTC_ENHANCED, ...go2rtcEnhanced } : (prev.go2rtcEnhanced ?? DEFAULT_GO2RTC_ENHANCED),
        streamOptimization: streamOptimization ? { ...DEFAULT_STREAM_OPTIMIZATION, ...streamOptimization } : (prev.streamOptimization ?? DEFAULT_STREAM_OPTIMIZATION),
      } as AppSettings;

      return next;
    },
    [currentLanguage, normalizeStreamingSettings],
  );

  const prepareStreamingForSave = (
    streaming: StreamingBackendSettings
  ): StreamingBackendSettings => ({
    provider: isStreamingProvider(streaming.provider)
      ? streaming.provider
      : DEFAULT_STREAMING_SETTINGS.provider,
    enableOnDemand: Boolean(streaming.enableOnDemand),
    restartOnConfigChange: Boolean(streaming.restartOnConfigChange),
  });
  const [updateStatus, setUpdateStatus] = useState('idle');

  useEffect(() => {
    // Load app version
    const loadAppVersion = async () => {
      try {
        const version = await getVersion();
        setAppVersion(version);
      } catch (error) {
        console.error('Failed to get app version:', error);
        setAppVersion('Unknown');
      }
    };

    // Load settings from Tauri backend
    const loadSettings = async () => {
      try {
        const savedSettings = await invoke<Partial<AppSettings> | null>('get_app_settings');
        const merged = mergeAppSettings(settings, savedSettings);
        
        // Ensure go2rtcEnhanced has default values if not present
        if (!merged.go2rtcEnhanced) {
          merged.go2rtcEnhanced = DEFAULT_GO2RTC_ENHANCED;
        }
        
        // Ensure streamOptimization has default values if not present
        if (!merged.streamOptimization) {
          merged.streamOptimization = DEFAULT_STREAM_OPTIMIZATION;
        }
        
        // Load ANPR config from backend
        try {
          const anprConfig = await invoke<{
            detection_confidence: number;
            crop_expansion_factor: number;
            crnn_confidence_threshold: number;
            python_confidence_threshold: number;
            enable_python_ocr: boolean;
          }>('get_anpr_config');
          
          merged.anpr_detection_confidence = anprConfig.detection_confidence;
          merged.anpr_crop_expansion = anprConfig.crop_expansion_factor;
          merged.anpr_crnn_confidence = anprConfig.crnn_confidence_threshold;
          merged.anpr_python_confidence = anprConfig.python_confidence_threshold;
          merged.anpr_enable_python = anprConfig.enable_python_ocr;
        } catch (e) {
          console.error('Failed to load ANPR config:', e);
        }
        
        // Load go2rtc settings from localStorage if available (overrides backend)
        const go2rtcSettingsStr = localStorage.getItem('go2rtcSettings');
        if (go2rtcSettingsStr) {
          try {
            const go2rtcSettings = JSON.parse(go2rtcSettingsStr);
            merged.go2rtcEnhanced = {
              ...DEFAULT_GO2RTC_ENHANCED,
              ...go2rtcSettings
            };
          } catch (e) {
            console.error('Failed to parse go2rtc settings:', e);
          }
        }
        
        // Load stream optimization settings from localStorage
        const streamOptSettingsStr = localStorage.getItem('streamOptimizationSettings');
        if (streamOptSettingsStr) {
          try {
            const streamOptSettings = JSON.parse(streamOptSettingsStr);
            merged.streamOptimization = {
              ...DEFAULT_STREAM_OPTIMIZATION,
              ...streamOptSettings
            };
          } catch (e) {
            console.error('Failed to parse stream optimization settings:', e);
          }
        }
        
        setSettings(merged);
      } catch (error) {
        console.error('Failed to load settings:', error);
        // Fallback to localStorage
        const localSettings = localStorage.getItem('appSettings');
        if (localSettings) {
          try {
            const parsed = JSON.parse(localSettings) as Partial<AppSettings>;
            const merged = mergeAppSettings(settings, parsed);
            
            // Ensure go2rtcEnhanced has default values if not present
            if (!merged.go2rtcEnhanced) {
              merged.go2rtcEnhanced = DEFAULT_GO2RTC_ENHANCED;
            }
            
            // Ensure streamOptimization has default values if not present
            if (!merged.streamOptimization) {
              merged.streamOptimization = DEFAULT_STREAM_OPTIMIZATION;
            }
            
            // Load go2rtc settings from localStorage if available
            const go2rtcSettingsStr = localStorage.getItem('go2rtcSettings');
            if (go2rtcSettingsStr) {
              try {
                const go2rtcSettings = JSON.parse(go2rtcSettingsStr);
                merged.go2rtcEnhanced = {
                  ...DEFAULT_GO2RTC_ENHANCED,
                  ...go2rtcSettings
                };
              } catch (e) {
                console.error('Failed to parse go2rtc settings:', e);
              }
            }
            
            // Load stream optimization settings from localStorage
            const streamOptSettingsStr = localStorage.getItem('streamOptimizationSettings');
            if (streamOptSettingsStr) {
              try {
                const streamOptSettings = JSON.parse(streamOptSettingsStr);
                merged.streamOptimization = {
                  ...DEFAULT_STREAM_OPTIMIZATION,
                  ...streamOptSettings
                };
              } catch (e) {
                console.error('Failed to parse stream optimization settings:', e);
              }
            }
            
            setSettings(merged);
          } catch (parseError) {
            console.error('Failed to parse local settings:', parseError);
          }
        } else {
          // If no saved settings, use current language and defaults
          setSettings(prev => ({
            ...prev,
            language: currentLanguage,
            streaming: prev.streaming ?? createDefaultStreamingSettings(),
            go2rtcEnhanced: prev.go2rtcEnhanced ?? DEFAULT_GO2RTC_ENHANCED,
            streamOptimization: prev.streamOptimization ?? DEFAULT_STREAM_OPTIMIZATION
          }));
        }
      }
    };
    
    if (isOpen) {
      loadAppVersion();
      loadSettings();
      void refreshModules();
    }
  }, [isOpen, currentLanguage, refreshModules, mergeAppSettings]);

  useEffect(() => {
    if (isOpen && activeTab === 'tab-modules') {
      void refreshModules();
    }
  }, [isOpen, activeTab, refreshModules]);

  useEffect(() => {
    const enabledFromModules = modules
      .filter(module => module.enabled)
      .map(module => module.id)
      .sort();

    setSettings(prev => {
      const current = [...prev.enabledModules].sort();
      if (
        current.length === enabledFromModules.length &&
        current.every((value, index) => value === enabledFromModules[index])
      ) {
        return prev;
      }

      return {
        ...prev,
        enabledModules: enabledFromModules,
      };
    });
  }, [modules, setSettings]);

  const modulesReady = modules.length > 0;
  const modulesToRender: AnalyticsModuleStatus[] = modulesReady
    ? modules
    : Object.values(MODULE_METADATA).map(meta => ({
        id: meta.id,
        name: meta.name,
        version: meta.version,
        description: meta.description,
        enabled: settings.enabledModules.includes(meta.id),
        state: settings.enabledModules.includes(meta.id) ? 'ready' : 'disabled',
        progress: undefined,
        message: undefined,
        lastActivatedAt: undefined,
        lastErrorAt: undefined,
        config: undefined,
      }));

  const handleTabClick = (tabId: string) => {
    setActiveTab(tabId);
  };

  const handleSettingChange = <K extends keyof AppSettings>(key: K, value: AppSettings[K]) => {
    if (key === 'streaming') {
      console.warn('Use handleStreamingSettingChange for streaming updates');
      return;
    }
    setSettings(prev => ({
      ...prev,
      [key]: value,
    }));
    
    // Special handling for language change - don't apply immediately
    // We'll apply it only when saving
  };

  const handleSaveSettings = async () => {
    try {
      const payload: AppSettings = {
        ...settings,
        streaming: prepareStreamingForSave(settings.streaming)
      };

      // Prepare the success message BEFORE changing language
      const successMessage = payload.language !== currentLanguage 
        ? (payload.language === 'en' ? 'Settings saved successfully!' : 'Настройки успешно сохранены!')
        : t('settings_saved_success');
      
      // Save settings via Tauri backend
      await invoke('save_app_settings', { settings: payload });
      
      // Save ANPR config separately
      try {
        await invoke('save_anpr_config', {
          config: {
            detection_confidence: payload.anpr_detection_confidence,
            crop_expansion_factor: payload.anpr_crop_expansion,
            crnn_confidence_threshold: payload.anpr_crnn_confidence,
            python_confidence_threshold: payload.anpr_python_confidence,
            enable_python_ocr: payload.anpr_enable_python,
          }
        });
      } catch (e) {
        console.error('Failed to save ANPR config:', e);
      }
      
      // Also save to localStorage as backup
      localStorage.setItem('appSettings', JSON.stringify(payload));
      
      // Save go2rtc settings separately for VideoStreamPlayer components
      localStorage.setItem('go2rtcSettings', JSON.stringify(payload.go2rtcEnhanced));
      
      // Save stream optimization settings separately
      localStorage.setItem('streamOptimizationSettings', JSON.stringify(payload.streamOptimization));

      // Update local state with sanitized payload
      setSettings(payload);
      
      // Apply language change only when saving
      if (payload.language !== currentLanguage) {
        setLanguage(payload.language as SupportedLanguage);
        
        // Wait a bit for language change to apply
        setTimeout(() => {
          showToast(successMessage, 'success');
          
          // Close modal after showing toast
          setTimeout(() => {
            onClose();
          }, 1500);
        }, 100);
      } else {
        // Show success toast immediately if no language change
        showToast(successMessage, 'success');
        
        // Close modal after a brief delay to show the toast
        setTimeout(() => {
          onClose();
        }, 1500);
      }
      
    } catch (error) {
      console.error('Failed to save settings:', error);
      
      // Fallback to localStorage only
      const fallbackPayload: AppSettings = {
        ...settings,
        streaming: prepareStreamingForSave(settings.streaming)
      };
      localStorage.setItem('appSettings', JSON.stringify(fallbackPayload));
      setSettings(fallbackPayload);
      
      // Prepare error message in correct language
      const errorMessage = settings.language !== currentLanguage 
        ? (settings.language === 'en' ? 'Settings saved locally (Tauri save failed)' : 'Настройки сохранены локально (ошибка сохранения Tauri)')
        : 'Settings saved locally (Tauri save failed)';
      
      // Apply language change even on error
      if (settings.language !== currentLanguage) {
        setLanguage(settings.language as SupportedLanguage);
        
        // Wait a bit for language change to apply
        setTimeout(() => {
          showToast(errorMessage, 'warning');
          
          // Close modal after showing toast
          setTimeout(() => {
            onClose();
          }, 1500);
        }, 100);
      } else {
        // Show error toast immediately if no language change
        showToast(errorMessage, 'warning');
        
        // Close modal after a brief delay to show the toast
        setTimeout(() => {
          onClose();
        }, 1500);
      }
    }
  };

  const selectDirectory = async (settingKey: keyof AppSettings) => {
    console.log('selectDirectory called with:', settingKey);
    
    try {
      const tauriWindow =
        typeof window !== 'undefined'
          ? (window as Window & { __TAURI__?: unknown; __TAURI_INTERNALS__?: unknown })
          : null;

      // Check if we're in Tauri environment using a more reliable method
      console.log('Checking Tauri environment...');
      console.log('window.__TAURI__:', tauriWindow?.__TAURI__);
      console.log('window.__TAURI_INTERNALS__:', tauriWindow?.__TAURI_INTERNALS__);
      console.log('User agent:', navigator.userAgent);

      // Check if we're in Tauri environment
      const isTauri = Boolean(tauriWindow?.__TAURI__ ?? tauriWindow?.__TAURI_INTERNALS__);

      console.log('Is Tauri detected:', isTauri);

      if (isTauri) {
        console.log('Attempting to use Tauri dialog API...');

        const selected = await openDialog({
          directory: true,
          multiple: false,
          title: settingKey === 'recordingsPath'
            ? t('settings_recordings_folder')
            : t('settings_screenshots_folder')
        });

        console.log('Dialog result:', selected);

        if (selected && typeof selected === 'string') {
          console.log('Setting path:', selected);
          handleSettingChange(settingKey, selected);

          // Show success toast
          const toast = document.getElementById('app-toast');
          if (toast) {
            toast.textContent = t('settings_saved_success');
            toast.className = 'toast-notification show';
            setTimeout(() => {
              toast.className = 'toast-notification';
            }, 3000);
          }
        } else {
          console.log('Dialog was cancelled or returned invalid result');
        }
      } else {
        console.log('Tauri not available, using fallback...');
        // Fallback for browser environment
        const path = prompt(`${settingKey === 'recordingsPath' ? t('settings_recordings_folder') : t('settings_screenshots_folder')}:`);
        if (path) {
          handleSettingChange(settingKey, path);
          
          const toast = document.getElementById('app-toast');
          if (toast) {
            toast.textContent = t('settings_saved_success');
            toast.className = 'toast-notification show';
            setTimeout(() => {
              toast.className = 'toast-notification';
            }, 3000);
          }
        }
      }
    } catch (error) {
      console.error('Error in selectDirectory:', error);
      console.error('Error type:', typeof error);
      console.error('Error details:', error);
      
      // Fallback to prompt if Tauri dialog fails
      const path = prompt(`${settingKey === 'recordingsPath' ? t('settings_recordings_folder') : t('settings_screenshots_folder')}:`);
      if (path) {
        handleSettingChange(settingKey, path);
        
        const toast = document.getElementById('app-toast');
        if (toast) {
          toast.textContent = t('settings_saved_success');
          toast.className = 'toast-notification show';
          setTimeout(() => {
            toast.className = 'toast-notification';
          }, 3000);
        }
      } else {
        // Show error toast
        const toast = document.getElementById('app-toast');
        if (toast) {
          toast.textContent = t('error') + ': ' + String(error);
          toast.className = 'toast-notification show error';
          setTimeout(() => {
            toast.className = 'toast-notification';
          }, 3000);
        }
      }
    }
  };

  const handleModuleToggle = async (moduleId: string, enabled: boolean) => {
    const pendingMessage = enabled
      ? currentLanguage === 'ru'
        ? 'Загрузка…'
        : 'Preparing…'
      : undefined;

    try {
      const success = await toggleModule(moduleId, enabled, pendingMessage);

      if (!success) {
        const errorMessage = currentLanguage === 'ru'
          ? 'Не удалось обновить модуль'
          : 'Failed to update module';
        showToast(errorMessage, 'error');
        return;
      }

      if (!modulesReady) {
        setSettings(prev => {
          const nextEnabled = enabled
            ? Array.from(new Set([...prev.enabledModules, moduleId]))
            : prev.enabledModules.filter(id => id !== moduleId);
          return {
            ...prev,
            enabledModules: nextEnabled,
          };
        });
      }

      const successMessage = enabled
        ? currentLanguage === 'ru'
          ? 'Модуль включён'
          : 'Module enabled'
        : currentLanguage === 'ru'
          ? 'Модуль отключён'
          : 'Module disabled';
      showToast(successMessage, 'success');
    } catch (error) {
      console.error('Failed to toggle module state:', error);
      const errorMessage = currentLanguage === 'ru'
        ? 'Не удалось обновить модуль'
        : 'Failed to update module';
      showToast(errorMessage, 'error');
    }
  };

  const pickSnapshotsDirectory = useCallback(
    async (initialPath?: string): Promise<string | undefined> => {
      const tauriWindow =
        typeof window !== 'undefined'
          ? (window as Window & { __TAURI__?: unknown; __TAURI_INTERNALS__?: unknown })
          : null;
      const isTauri = Boolean(tauriWindow?.__TAURI__ ?? tauriWindow?.__TAURI_INTERNALS__);

      const promptLabel =
        currentLanguage === 'ru'
          ? 'Выберите каталог для сохранения снимков'
          : 'Choose folder for snapshots';

      if (!isTauri) {
        const fallback = prompt(promptLabel, initialPath ?? '');
        return fallback && fallback.trim().length > 0 ? fallback.trim() : undefined;
      }

      try {
        const result = await openDialog({
          directory: true,
          defaultPath: initialPath && initialPath.length > 0 ? initialPath : undefined,
        });

        if (!result || Array.isArray(result)) {
          return undefined;
        }

        return result;
      } catch (error) {
        console.error('Failed to open snapshots directory dialog:', error);
        const message =
          currentLanguage === 'ru'
            ? 'Не удалось открыть диалог выбора каталога'
            : 'Unable to open folder dialog';
        showToast(message, 'error');
        return undefined;
      }
    },
    [currentLanguage, showToast],
  );

  const handleSnapshotsDirectoryUpdate = useCallback(
    async (moduleId: string, initialPath?: string) => {
      const selected = await pickSnapshotsDirectory(initialPath);
      if (!selected) {
        return;
      }

      const updated = await updateModuleSnapshotsDir(moduleId, selected);
      if (updated) {
        const message =
          currentLanguage === 'ru'
            ? 'Каталог для снимков обновлён'
            : 'Snapshots directory updated';
        showToast(message, 'success');
      } else {
        const message =
          currentLanguage === 'ru'
            ? 'Не удалось обновить каталог для снимков'
            : 'Failed to update snapshots directory';
        showToast(message, 'error');
      }
    },
    [pickSnapshotsDirectory, updateModuleSnapshotsDir, currentLanguage, showToast],
  );

  const handleSnapshotsDirectoryReset = useCallback(
    async (moduleId: string) => {
      const updated = await updateModuleSnapshotsDir(moduleId, undefined);
      if (updated) {
        const message =
          currentLanguage === 'ru'
            ? 'Каталог для снимков сброшен на значение по умолчанию'
            : 'Snapshots directory reset to default';
        showToast(message, 'success');
      } else {
        const message =
          currentLanguage === 'ru'
            ? 'Не удалось сбросить каталог для снимков'
            : 'Failed to reset snapshots directory';
        showToast(message, 'error');
      }
    },
    [updateModuleSnapshotsDir, currentLanguage, showToast],
  );

  const resolveFaceSnapshotMode = useCallback(
    (value?: FaceSnapshotMode): FaceSnapshotMode => {
      if (!value) {
        return DEFAULT_FACE_SNAPSHOT_MODE;
      }
      return FACE_SNAPSHOT_MODE_VALUES.includes(value) ? value : DEFAULT_FACE_SNAPSHOT_MODE;
    },
    [],
  );

  const validateFaceSnapshotKey = useCallback(
    (input: string): string | null => {
      const trimmed = input.trim();
      if (trimmed.length === 0) {
        return currentLanguage === 'ru'
          ? 'Введите ключ из 64 шестнадцатеричных символов'
          : 'Enter a 64-character hexadecimal key';
      }
      if (!/^[0-9a-fA-F]{64}$/.test(trimmed)) {
        return currentLanguage === 'ru'
          ? 'Ключ должен содержать 64 шестнадцатеричных символа'
          : 'The key must contain 64 hexadecimal characters';
      }
      return null;
    },
    [currentLanguage],
  );

  const handleFaceSnapshotModeChange = useCallback(
    async (moduleId: string, mode: FaceSnapshotMode) => {
      const updated = await updateModuleConfig(moduleId, { faceSnapshotsMode: mode });
      if (updated) {
        const message =
          currentLanguage === 'ru'
            ? 'Режим снимков лиц обновлён'
            : 'Face snapshot mode updated';
        showToast(message, 'success');
      } else {
        const message =
          currentLanguage === 'ru'
            ? 'Не удалось обновить режим снимков'
            : 'Failed to update face snapshot mode';
        showToast(message, 'error');
      }
    },
    [updateModuleConfig, currentLanguage, showToast],
  );

  const handleFaceSnapshotKeyInputChange = useCallback((moduleId: string, value: string) => {
    setFaceSnapshotKeyInputs(prev => ({
      ...prev,
      [moduleId]: value,
    }));
    setFaceSnapshotKeyErrors(prev => ({
      ...prev,
      [moduleId]: null,
    }));
  }, []);

  const handleFaceSnapshotKeySave = useCallback(
    async (moduleId: string) => {
      const input = (faceSnapshotKeyInputs[moduleId] ?? '').trim();
      const validationError = validateFaceSnapshotKey(input);
      if (validationError) {
        setFaceSnapshotKeyErrors(prev => ({
          ...prev,
          [moduleId]: validationError,
        }));
        return;
      }

      setFaceSnapshotKeyBusy(prev => ({
        ...prev,
        [moduleId]: true,
      }));

      try {
        const updated = await updateModuleConfig(moduleId, {
          faceSnapshotKeyHex: input.toLowerCase(),
        });

        if (updated) {
          setFaceSnapshotKeyInputs(prev => ({
            ...prev,
            [moduleId]: '',
          }));
          setFaceSnapshotKeyErrors(prev => ({
            ...prev,
            [moduleId]: null,
          }));
          const message =
            currentLanguage === 'ru'
              ? 'Ключ шифрования сохранён'
              : 'Encryption key saved';
          showToast(message, 'success');
        } else {
          const message =
            currentLanguage === 'ru'
              ? 'Не удалось сохранить ключ шифрования'
              : 'Failed to save encryption key';
          showToast(message, 'error');
        }
      } catch (error) {
        console.error('Failed to save face snapshot key:', error);
        const message =
          currentLanguage === 'ru'
            ? 'Не удалось сохранить ключ шифрования'
            : 'Failed to save encryption key';
        showToast(message, 'error');
      } finally {
        setFaceSnapshotKeyBusy(prev => ({
          ...prev,
          [moduleId]: false,
        }));
      }
    },
    [
      faceSnapshotKeyInputs,
      updateModuleConfig,
      validateFaceSnapshotKey,
      currentLanguage,
      showToast,
    ],
  );

  const handleFaceSnapshotKeyReset = useCallback(
    async (moduleId: string) => {
      const promptMessage =
        currentLanguage === 'ru'
          ? 'Сбросить ключ шифрования? Снимки больше не будут зашифрованы до ввода нового ключа.'
          : 'Reset the encryption key? Snapshots will not be encrypted until a new key is provided.';

      const confirmed = typeof window === 'undefined' ? true : window.confirm(promptMessage);
      if (!confirmed) {
        return;
      }

      setFaceSnapshotKeyBusy(prev => ({
        ...prev,
        [moduleId]: true,
      }));

      try {
        const updated = await updateModuleConfig(moduleId, {
          resetFaceSnapshotKey: true,
        });

        if (updated) {
          setFaceSnapshotKeyInputs(prev => ({
            ...prev,
            [moduleId]: '',
          }));
          setFaceSnapshotKeyErrors(prev => ({
            ...prev,
            [moduleId]: null,
          }));
          const message =
            currentLanguage === 'ru'
              ? 'Ключ шифрования сброшен'
              : 'Encryption key reset';
          showToast(message, 'success');
        } else {
          const message =
            currentLanguage === 'ru'
              ? 'Не удалось сбросить ключ шифрования'
              : 'Failed to reset encryption key';
          showToast(message, 'error');
        }
      } catch (error) {
        console.error('Failed to reset face snapshot key:', error);
        const message =
          currentLanguage === 'ru'
            ? 'Не удалось сбросить ключ шифрования'
            : 'Failed to reset encryption key';
        showToast(message, 'error');
      } finally {
        setFaceSnapshotKeyBusy(prev => ({
          ...prev,
          [moduleId]: false,
        }));
      }
    },
    [updateModuleConfig, currentLanguage, showToast],
  );

  const handleExportConfig = async () => {
    try {
      // Check if we're in Tauri environment
      const tauriWindow =
        typeof window !== 'undefined'
          ? (window as Window & { __TAURI__?: unknown; __TAURI_INTERNALS__?: unknown })
          : null;
      const isTauri = Boolean(tauriWindow?.__TAURI__ ?? tauriWindow?.__TAURI_INTERNALS__);
      
      const config = JSON.stringify(settings, null, 2);
      
      if (isTauri) {
        console.log('Attempting to use Tauri save dialog...');
        
        const filePath = await save({
          defaultPath: 'vms-config.json',
          filters: [{
            name: 'JSON',
            extensions: ['json']
          }],
          title: t('settings_export_config')
        });
        
        if (filePath) {
          await invoke('save_config_file', { 
            filePath: filePath,
            content: config 
          });
          
          const toast = document.getElementById('app-toast');
          if (toast) {
            toast.textContent = t('settings_export_success');
            toast.className = 'toast-notification show';
            setTimeout(() => {
              toast.className = 'toast-notification';
            }, 3000);
          }
        }
      } else {
        console.log('Tauri not available, using browser download...');
        // Fallback for browser environment
        const blob = new Blob([config], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'vms-config.json';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        const toast = document.getElementById('app-toast');
        if (toast) {
          toast.textContent = 'Конфигурация экспортирована';
          toast.className = 'toast-notification show';
          setTimeout(() => {
            toast.className = 'toast-notification';
          }, 3000);
        }
      }
    } catch (error) {
      console.error('Export failed:', error);
      const toast = document.getElementById('app-toast');
      if (toast) {
        toast.textContent = 'Ошибка при экспорте конфигурации';
        toast.className = 'toast-notification show error';
        setTimeout(() => {
          toast.className = 'toast-notification';
        }, 3000);
      }
    }
  };

  const handleImportConfig = () => {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = event => {
      const file = (event.target as HTMLInputElement).files?.[0];
      if (file) {
        const reader = new FileReader();
        reader.onload = loadEvent => {
          try {
            const rawContent = loadEvent.target?.result;
            if (typeof rawContent !== 'string') {
              throw new Error('Configuration payload is not a string');
            }

            const config = JSON.parse(rawContent) as Partial<AppSettings>;
            setSettings(prev => mergeAppSettings(prev, config));
            
            const toast = document.getElementById('app-toast');
            if (toast) {
              toast.textContent = 'Конфигурация импортирована';
              toast.className = 'toast-notification show';
              setTimeout(() => {
                toast.className = 'toast-notification';
              }, 3000);
            }
          } catch (importError) {
            console.error('Failed to import configuration:', importError);
            alert('Ошибка при импорте конфигурации');
          }
        };
        reader.readAsText(file);
      }
    };
    input.click();
  };

  const handleCheckForUpdates = async () => {
    setUpdateStatus('checking');
    try {
      const update = await check();
      if (update?.available) {
        setUpdateStatus('available');
        
        const yes = await ask(
          `New version ${update.version} is available!\n\n${update.body || 'No release notes.'}\n\nDo you want to install it now?`, 
          { title: 'Update Available', kind: 'info', okLabel: 'Install & Restart', cancelLabel: 'Later' }
        );
        
        if (yes) {
          await update.downloadAndInstall();
          await relaunch();
        }
      } else {
        setUpdateStatus('latest');
        const toast = document.getElementById('app-toast');
        if (toast) {
          toast.textContent = t('update_latest');
          toast.className = 'toast-notification show';
          setTimeout(() => {
            toast.className = 'toast-notification';
          }, 3000);
        }
      }
    } catch (error) {
      console.error('Update check failed:', error);
      setUpdateStatus('error');
      const toast = document.getElementById('app-toast');
      if (toast) {
        toast.textContent = 'Update check failed';
        toast.className = 'toast-notification show';
        setTimeout(() => {
          toast.className = 'toast-notification';
        }, 3000);
      }
    }
  };

  const openDonationPage = () => {
    void openExternal('https://opencollective.com/openipc/projects/openipc-dashboard/donate?interval=oneTime&amount=20&contributeAs=me');
  };

  const reportIssue = () => {
    void openExternal('https://github.com/OpenIPC/dashboard/issues');
  };

  if (!isOpen) return null;

  return (
    <div className="modal-backdrop">
      <div className="modal-content large">
        <span className="modal-close-btn" onClick={onClose}>&times;</span>
        <h2 id="settings-modal-title">{t('general_settings_title')}</h2>
        
        <div className="modal-body">
          <div className="tabs">
            <button 
              className={`tab-button general-tab ${activeTab === 'tab-general' ? 'active' : ''}`}
              onClick={() => handleTabClick('tab-general')}
            >
              {t('settings_tab_general')}
            </button>
            <button 
              className={`tab-button general-tab ${activeTab === 'tab-streaming' ? 'active' : ''}`}
              onClick={() => handleTabClick('tab-streaming')}
            >
              {t('settings_tab_streaming')}
            </button>
            <button 
              className={`tab-button general-tab ${activeTab === 'tab-analytics' ? 'active' : ''}`}
              onClick={() => handleTabClick('tab-analytics')}
            >
              {t('settings_tab_analytics')}
            </button>
            <button 
              className={`tab-button general-tab ${activeTab === 'tab-modules' ? 'active' : ''}`}
              onClick={() => handleTabClick('tab-modules')}
            >
              {t('settings_tab_modules')}
            </button>
            <button 
              className={`tab-button general-tab ${activeTab === 'tab-about' ? 'active' : ''}`}
              onClick={() => handleTabClick('tab-about')}
            >
              {t('settings_tab_about')}
            </button>
          </div>

          {/* General Tab */}
          {activeTab === 'tab-general' && (
            <div className="tab-content active">
              <h3>{t('settings_app_header')}</h3>
              <div className="form-grid simple with-button">
                <span>{t('settings_language')}</span>
                <select 
                  id="app-settings-language"
                  value={settings.language}
                  onChange={(e) => handleSettingChange('language', e.target.value)}
                >
                  <option value="en">English</option>
                  <option value="ru">Русский</option>
                </select>

                <span>{t('settings_recordings_folder')}</span>
                <div className="form-input-wrapper">
                  <input 
                    type="text" 
                    id="app-settings-recordings-path"
                    value={settings.recordingsPath}
                    readOnly
                    placeholder={t('settings_recordings_folder') + '...'}
                  />
                  <button 
                    id="select-rec-path-btn"
                    onClick={() => selectDirectory('recordingsPath')}
                    style={{ padding: '0 10px', minWidth: '40px', height: '35px' }}
                  >
                    <i className="material-icons" style={{ fontSize: '20px' }}>folder_open</i>
                  </button>
                </div>

                <span>{t('settings_screenshots_folder')}</span>
                <div className="form-input-wrapper">
                  <input 
                    type="text" 
                    id="app-settings-screenshots-path"
                    value={settings.screenshotsPath}
                    readOnly
                    placeholder={t('settings_screenshots_folder') + '...'}
                  />
                  <button 
                    id="select-screenshots-path-btn"
                    onClick={() => selectDirectory('screenshotsPath')}
                    style={{ padding: '0 10px', minWidth: '40px', height: '35px' }}
                  >
                    <i className="material-icons" style={{ fontSize: '20px' }}>folder_open</i>
                  </button>
                </div>

                <span>{t('settings_hw_accel')}</span>
                <div style={{ display: 'flex', flexDirection: 'column' }}>
                  <select 
                    id="app-settings-hw-accel"
                    value={settings.hwAccel}
                    onChange={(e) => handleSettingChange('hwAccel', e.target.value)}
                    style={{ width: '100%' }}
                  >
                    <option value="auto">{t('settings_hw_accel_auto')}</option>
                    <option value="nvidia">{t('settings_hw_accel_nvidia')}</option>
                    <option value="intel">{t('settings_hw_accel_intel')}</option>
                    <option value="none">{t('settings_hw_accel_none')}</option>
                  </select>
                  <span style={{ fontSize: '11px', color: '#888', marginTop: '4px', lineHeight: '1.2' }}>
                    {t('settings_hw_accel_hint')}
                  </span>
                </div>
              </div>

              <div className="form-grid simple with-button">
                <span>{t('settings_notifications_label')}</span>
                <div className="form-check-inline">
                  <input 
                    type="checkbox" 
                    id="app-settings-notifications-enabled"
                    checked={settings.notifications_enabled}
                    onChange={(e) => handleSettingChange('notifications_enabled', e.target.checked)}
                    className="form-check-input"
                  />
                  <label htmlFor="app-settings-notifications-enabled">
                    {t('settings_notifications_enable_label')}
                  </label>
                </div>
              </div>

              <div>
                <h3>{t('settings_config_management_header')}</h3>
                <div className="config-buttons-container">
                  <button id="export-config-btn" onClick={handleExportConfig}>
                    {t('settings_config_export_btn')}
                  </button>
                  <button 
                    id="import-config-btn" 
                    onClick={handleImportConfig}
                    style={{ backgroundColor: 'var(--danger-color)' }}
                  >
                    {t('settings_config_import_btn')}
                  </button>
                </div>
              </div>

              <h3>{t('update_header')}</h3>
              <div className="form-grid simple">
                <span>{t('update_status_label')}</span>
                <span id="update-status-text" style={{ fontStyle: 'italic' }}>
                  {updateStatus === 'checking' ? t('update_checking') : 
                   updateStatus === 'latest' ? t('update_latest') :
                   t('update_check_prompt')}
                </span>
                <span>{t('update_action_label')}</span>
                <button 
                  id="check-for-updates-btn"
                  onClick={handleCheckForUpdates}
                  disabled={updateStatus === 'checking'}
                  style={{ width: '200px', justifySelf: 'start' }}
                >
                  {t('update_check_button')}
                </button>
              </div>
            </div>
          )}

          {/* Streaming Tab */}
          {activeTab === 'tab-streaming' && (
            <div className="tab-content active">
              <h3>{t('streaming_settings_header')}</h3>
              <p style={{ color: '#666', fontSize: '13px' }}>
                {t('streaming_settings_desc')}
              </p>

              {/* 
                  Legacy Streaming Settings (Hidden as they are not currently functional in Direct Copy mode)
                  These settings (qscale, fps, onDemand) are preserved in state for future transcoding support
                  but hidden from UI to avoid user confusion.
              */}
              {/* 
              <div className="form-grid simple">
                <span>{t('streaming_on_demand_label')}</span>
                <div className="form-check-inline">
                  <input
                    type="checkbox"
                    id="app-settings-streaming-on-demand"
                    checked={settings.streaming.enableOnDemand}
                    onChange={e =>
                      handleStreamingSettingChange('enableOnDemand', e.target.checked)
                    }
                    className="form-check-input"
                  />
                  <label htmlFor="app-settings-streaming-on-demand">
                    {t('streaming_on_demand_help')}
                  </label>
                </div>

                <span>{t('streaming_restart_on_change_label')}</span>
                <div className="form-check-inline">
                  <input
                    type="checkbox"
                    id="app-settings-streaming-restart"
                    checked={settings.streaming.restartOnConfigChange}
                    onChange={e =>
                      handleStreamingSettingChange(
                        'restartOnConfigChange',
                        e.target.checked
                      )
                    }
                    className="form-check-input"
                  />
                  <label htmlFor="app-settings-streaming-restart">
                    {t('streaming_restart_on_change_help')}
                  </label>
                </div>
              </div>

              <div className="form-grid simple">
                <label htmlFor="app-settings-qscale">{t('streaming_qscale_label')}</label>
                <div className="form-input-wrapper">
                  <input 
                    type="range" 
                    id="app-settings-qscale"
                    min="2" 
                    max="31" 
                    step="1"
                    value={settings.qscale}
                    onChange={(e) => handleSettingChange('qscale', parseInt(e.target.value))}
                    style={{ flexGrow: 1 }}
                  />
                  <span className="range-value" id="app-settings-qscale-value" style={{ minWidth: '30px', textAlign: 'center' }}>
                    {settings.qscale}
                  </span>
                </div>

                <label htmlFor="app-settings-fps">{t('streaming_fps_label')}</label>
                <input 
                  type="number" 
                  id="app-settings-fps"
                  min="5" 
                  max="30" 
                  step="1"
                  value={settings.fps}
                  onChange={(e) => handleSettingChange('fps', parseInt(e.target.value))}
                  style={{ width: '100px', justifySelf: 'start' }}
                />
              </div>
              */}

              {/* go2rtc Enhanced Features Section */}
              <div style={{ marginTop: '30px', padding: '20px', backgroundColor: '#252525', borderRadius: '8px', border: '1px solid #333' }}>
                <h4 style={{ marginTop: 0, marginBottom: '10px', display: 'flex', alignItems: 'center', gap: '8px' }}>
                  <span>⭐</span>
                  <span>{t('go2rtc_enhanced_header')}</span>
                </h4>
                <p style={{ color: '#888', fontSize: '13px', marginBottom: '20px' }}>
                  {t('go2rtc_enhanced_desc')}
                </p>

                <div className="form-grid simple">
                  <span>{t('go2rtc_monitor_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="go2rtc-show-monitor"
                      checked={safeSettings.go2rtcEnhanced.showMonitor}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          go2rtcEnhanced: { ...prev.go2rtcEnhanced, showMonitor: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="go2rtc-show-monitor" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('go2rtc_monitor_help')}
                    </label>
                  </div>

                  <span>{t('go2rtc_snapshot_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="go2rtc-enable-snapshot"
                      checked={safeSettings.go2rtcEnhanced.enableSnapshot}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          go2rtcEnhanced: { ...prev.go2rtcEnhanced, enableSnapshot: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="go2rtc-enable-snapshot" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('go2rtc_snapshot_help')}
                    </label>
                  </div>

                  <span>{t('go2rtc_2way_audio_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="go2rtc-enable-2way-audio"
                      checked={safeSettings.go2rtcEnhanced.enable2WayAudio}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          go2rtcEnhanced: { ...prev.go2rtcEnhanced, enable2WayAudio: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="go2rtc-enable-2way-audio" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('go2rtc_2way_audio_help')}
                    </label>
                  </div>

                  <span>{t('go2rtc_adaptive_bitrate_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="go2rtc-adaptive-bitrate"
                      checked={safeSettings.go2rtcEnhanced.enableAdaptiveBitrate}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          go2rtcEnhanced: { ...prev.go2rtcEnhanced, enableAdaptiveBitrate: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="go2rtc-adaptive-bitrate" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('go2rtc_adaptive_bitrate_help')}
                    </label>
                  </div>
                </div>

                <div style={{ marginTop: '15px', padding: '12px', backgroundColor: '#1a3a52', borderRadius: '4px', fontSize: '12px', color: '#a8d5ff' }}>
                  <strong>ℹ️ Info:</strong> {t('go2rtc_info_note')}
                </div>
              </div>

              {/* Stream Optimization Section */}
              <div style={{ marginTop: '30px', padding: '20px', backgroundColor: '#252525', borderRadius: '8px', border: '1px solid #333' }}>
                <h4 style={{ marginTop: 0, marginBottom: '10px', display: 'flex', alignItems: 'center', gap: '8px' }}>
                  <span>🚀</span>
                  <span>{t('stream_optimization_header')}</span>
                </h4>
                <p style={{ color: '#888', fontSize: '13px', marginBottom: '20px' }}>
                  {t('stream_optimization_desc')}
                </p>

                <div className="form-grid simple">
                  <span>{t('stream_fast_start_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="stream-fast-start"
                      checked={safeSettings.streamOptimization.enableFastStart}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...(prev.streamOptimization ?? DEFAULT_STREAM_OPTIMIZATION), enableFastStart: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="stream-fast-start" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('stream_fast_start_help')}
                    </label>
                  </div>

                  <span>{t('stream_prewarming_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="stream-prewarming"
                      checked={safeSettings.streamOptimization.enablePrewarming}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, enablePrewarming: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="stream-prewarming" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('stream_prewarming_help')}
                    </label>
                  </div>

                  <span>{t('stream_prewarm_both_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="stream-prewarm-both"
                      checked={safeSettings.streamOptimization.prewarmBothQualities}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, prewarmBothQualities: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                      disabled={!safeSettings.streamOptimization.enablePrewarming}
                    />
                    <label htmlFor="stream-prewarm-both" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('stream_prewarm_both_help')}
                    </label>
                  </div>

                  <span>{t('stream_caching_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="stream-caching"
                      checked={safeSettings.streamOptimization.enableConnectionCaching}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, enableConnectionCaching: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="stream-caching" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('stream_caching_help')}
                    </label>
                  </div>

                  <span>{t('stream_keep_hd_alive_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="stream-keep-hd-alive"
                      checked={safeSettings.streamOptimization.keepHdStreamAlive}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, keepHdStreamAlive: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="stream-keep-hd-alive" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('stream_keep_hd_alive_help')}
                    </label>
                  </div>

                  <span>{t('stream_keepalive_label')}</span>
                  <div style={{ display: 'flex', flexDirection: 'column' }}>
                    <input
                      type="number"
                      id="stream-keepalive"
                      value={safeSettings.streamOptimization.keepAliveInterval}
                      onChange={e => {
                        const value = parseInt(e.target.value, 10);
                        if (!isNaN(value) && value >= 10 && value <= 300) {
                          setSettings(prev => ({
                            ...prev,
                            streamOptimization: { ...prev.streamOptimization, keepAliveInterval: value }
                          }));
                        }
                      }}
                      min="10"
                      max="300"
                      disabled={!safeSettings.streamOptimization.enablePrewarming}
                      style={{ width: '100px' }}
                    />
                    <label htmlFor="stream-keepalive" style={{ color: '#aaa', fontSize: '12px', marginTop: '4px' }}>
                      {t('stream_keepalive_help')}
                    </label>
                  </div>
                </div>

                <div style={{ marginTop: '15px', padding: '12px', backgroundColor: '#2d4a2d', borderRadius: '4px', fontSize: '12px', color: '#a8ffa8' }}>
                  <strong>✓ Рекомендуется:</strong> {t('stream_optimization_recommendation')}
                </div>
              </div>

              {/* Low-Latency Settings Section */}
              <div style={{ marginTop: '30px', padding: '20px', backgroundColor: '#252525', borderRadius: '8px', border: '1px solid #ff6b35' }}>
                <h4 style={{ marginTop: 0, marginBottom: '10px', display: 'flex', alignItems: 'center', gap: '8px' }}>
                  <span>⚡</span>
                  <span>{t('low_latency_header')}</span>
                </h4>
                <p style={{ color: '#888', fontSize: '13px', marginBottom: '20px' }}>
                  {t('low_latency_desc')}
                </p>

                <div className="form-grid simple">
                  <span>{t('low_latency_enable_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="low-latency-enable"
                      checked={safeSettings.streamOptimization.enableLowLatency}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, enableLowLatency: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="low-latency-enable" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('low_latency_enable_help')}
                    </label>
                  </div>

                  <span>{t('low_latency_playout_delay_label')}</span>
                  <div className="form-input-wrapper">
                    <input 
                      type="range" 
                      id="playout-delay"
                      min="0" 
                      max="2000" 
                      step="100"
                      value={safeSettings.streamOptimization.playoutDelayHint}
                      onChange={(e) => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, playoutDelayHint: parseInt(e.target.value) }
                        }));
                      }}
                      disabled={!safeSettings.streamOptimization.enableLowLatency}
                      style={{ flexGrow: 1 }}
                    />
                    <span className="range-value" style={{ minWidth: '60px', textAlign: 'center' }}>
                      {safeSettings.streamOptimization.playoutDelayHint} мс
                    </span>
                  </div>
                  <div style={{ gridColumn: '2', fontSize: '11px', color: '#888', marginTop: '-5px', marginBottom: '5px' }}>
                    {t('low_latency_playout_delay_hint')}
                  </div>

                  <span>{t('low_latency_jitter_buffer_label')}</span>
                  <div className="form-input-wrapper">
                    <input 
                      type="range" 
                      id="jitter-buffer"
                      min="0" 
                      max="1000" 
                      step="50"
                      value={safeSettings.streamOptimization.jitterBufferTarget}
                      onChange={(e) => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, jitterBufferTarget: parseInt(e.target.value) }
                        }));
                      }}
                      disabled={!safeSettings.streamOptimization.enableLowLatency}
                      style={{ flexGrow: 1 }}
                    />
                    <span className="range-value" style={{ minWidth: '60px', textAlign: 'center' }}>
                      {safeSettings.streamOptimization.jitterBufferTarget} мс
                    </span>
                  </div>
                  <div style={{ gridColumn: '2', fontSize: '11px', color: '#888', marginTop: '-5px', marginBottom: '5px' }}>
                    {t('low_latency_jitter_buffer_hint')}
                  </div>
                </div>

                <h5 style={{ marginTop: '25px', marginBottom: '15px', color: '#ff9a76', fontSize: '14px' }}>
                  {t('low_latency_monitoring_header')}
                </h5>

                <div className="form-grid simple">
                  <span>{t('low_latency_monitoring_enable_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="latency-monitoring-enable"
                      checked={safeSettings.streamOptimization.enableLatencyMonitoring}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, enableLatencyMonitoring: e.target.checked }
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="latency-monitoring-enable" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('low_latency_monitoring_enable_help')}
                    </label>
                  </div>

                  <span>{t('low_latency_max_buffered_label')}</span>
                  <div className="form-input-wrapper">
                    <input 
                      type="range" 
                      id="max-buffered-latency"
                      min="0.5" 
                      max="5.0" 
                      step="0.5"
                      value={safeSettings.streamOptimization.maxBufferedLatency}
                      onChange={(e) => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, maxBufferedLatency: parseFloat(e.target.value) }
                        }));
                      }}
                      disabled={!safeSettings.streamOptimization.enableLatencyMonitoring}
                      style={{ flexGrow: 1 }}
                    />
                    <span className="range-value" style={{ minWidth: '60px', textAlign: 'center' }}>
                      {safeSettings.streamOptimization.maxBufferedLatency.toFixed(1)} с
                    </span>
                  </div>
                  <div style={{ gridColumn: '2', fontSize: '11px', color: '#888', marginTop: '-5px', marginBottom: '5px' }}>
                    {t('low_latency_max_buffered_hint')}
                  </div>

                  <span>{t('low_latency_check_interval_label')}</span>
                  <div className="form-input-wrapper">
                    <input 
                      type="range" 
                      id="latency-check-interval"
                      min="1" 
                      max="10" 
                      step="1"
                      value={safeSettings.streamOptimization.latencyCheckInterval}
                      onChange={(e) => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, latencyCheckInterval: parseInt(e.target.value) }
                        }));
                      }}
                      disabled={!safeSettings.streamOptimization.enableLatencyMonitoring}
                      style={{ flexGrow: 1 }}
                    />
                    <span className="range-value" style={{ minWidth: '60px', textAlign: 'center' }}>
                      {safeSettings.streamOptimization.latencyCheckInterval} с
                    </span>
                  </div>
                  <div style={{ gridColumn: '2', fontSize: '11px', color: '#888', marginTop: '-5px', marginBottom: '5px' }}>
                    {t('low_latency_check_interval_hint')}
                  </div>
                </div>

                <h5 style={{ marginTop: '25px', marginBottom: '15px', color: '#ff9a76', fontSize: '14px' }}>
                  {t('low_latency_rtsp_header')}
                </h5>

                <div className="form-grid simple">
                  <span>{t('low_latency_rtsp_buffer_label')}</span>
                  <div className="form-input-wrapper">
                    <input 
                      type="range" 
                      id="rtsp-buffer-size"
                      min="0" 
                      max="10" 
                      step="1"
                      value={safeSettings.streamOptimization.rtspBufferSize}
                      onChange={(e) => {
                        setSettings(prev => ({
                          ...prev,
                          streamOptimization: { ...prev.streamOptimization, rtspBufferSize: parseInt(e.target.value) }
                        }));
                      }}
                      style={{ flexGrow: 1 }}
                    />
                    <span className="range-value" style={{ minWidth: '60px', textAlign: 'center' }}>
                      {settings.streamOptimization.rtspBufferSize === 0 ? t('minimum') : settings.streamOptimization.rtspBufferSize}
                    </span>
                  </div>
                </div>

                <div style={{ marginTop: '15px', padding: '12px', backgroundColor: '#4a2d2d', borderRadius: '4px', fontSize: '12px', color: '#ffb3a8' }}>
                  <strong>⚠️ {t('warning_label')}</strong> {t('low_latency_warning')}
                </div>
              </div>
            </div>
          )}

          {/* Analytics Tab */}
          {activeTab === 'tab-analytics' && (
            <div className="tab-content active intellect-only">
              <h3>{t('global_analytics_settings_header')}</h3>
              <p style={{ color: '#666', fontSize: '13px' }}>
                {t('global_analytics_settings_desc')}
              </p>

              {/* GPU/CPU Provider Info */}
              <div style={{ marginBottom: '20px', padding: '15px', backgroundColor: '#1a3a52', borderRadius: '6px', border: '1px solid #2563eb' }}>
                <div style={{ display: 'flex', alignItems: 'center', gap: '10px', marginBottom: '8px' }}>
                  <span style={{ fontSize: '20px' }}>⚡</span>
                  <strong style={{ color: '#60a5fa' }}>{currentLanguage === 'ru' ? 'Ускорение вычислений' : 'Compute Acceleration'}</strong>
                </div>
                <p style={{ margin: '8px 0', fontSize: '13px', color: '#93c5fd' }}>
                  {currentLanguage === 'ru' 
                    ? 'DirectML использует GPU для значительного ускорения распознавания и снижения нагрузки на CPU. Рекомендуется для систем с дискретной видеокартой.'
                    : 'DirectML uses GPU for significant acceleration of recognition and reduces CPU load. Recommended for systems with discrete graphics card.'}
                </p>
                <div style={{ marginTop: '10px', padding: '10px', backgroundColor: '#0f2a3f', borderRadius: '4px', fontSize: '12px' }}>
                  <div style={{ color: '#94a3b8' }}>
                    <strong>{currentLanguage === 'ru' ? 'Режимы работы:' : 'Operation modes:'}</strong>
                  </div>
                  <ul style={{ margin: '8px 0', paddingLeft: '20px', color: '#cbd5e1' }}>
                    <li><strong>Авто / Auto:</strong> {currentLanguage === 'ru' ? 'Автоматически использует GPU (DirectML), если доступен' : 'Automatically uses GPU (DirectML) if available'}</li>
                    <li><strong>GPU (DirectML):</strong> {currentLanguage === 'ru' ? 'Принудительно использует GPU (Windows 10+)' : 'Forces GPU usage (Windows 10+)'}</li>
                    <li><strong>CPU:</strong> {currentLanguage === 'ru' ? 'Только процессор (для совместимости или отладки)' : 'CPU only (for compatibility or debugging)'}</li>
                  </ul>
                </div>
              </div>

              <div className="form-grid simple">
                <label htmlFor="app-settings-analytics-provider">{t('settings_analytics_provider')}</label>
                <select
                  id="app-settings-analytics-provider"
                  value={settings.analytics_provider}
                  onChange={(e) => handleSettingChange('analytics_provider', e.target.value)}
                >
                  <option value="auto">{t('analytics_provider_auto')}</option>
                  <option value="dml">{t('analytics_provider_dml')}</option>
                  <option value="cpu">{t('analytics_provider_cpu')}</option>
                </select>

                <label htmlFor="app-settings-analytics-resize-width">{t('analytics_resize_width_label')}</label>
                <input 
                  type="number" 
                  id="app-settings-analytics-resize-width"
                  min="0" 
                  max="1920" 
                  step="10"
                  value={settings.analytics_resize_width}
                  onChange={(e) => handleSettingChange('analytics_resize_width', parseInt(e.target.value))}
                  style={{ width: '100px', justifySelf: 'start' }}
                />

                <label htmlFor="app-settings-analytics-frame-skip">{t('analytics_frame_skip_label')}</label>
                <input 
                  type="number" 
                  id="app-settings-analytics-frame-skip"
                  min="0" 
                  max="30" 
                  step="1"
                  value={settings.analytics_frame_skip}
                  onChange={(e) => handleSettingChange('analytics_frame_skip', parseInt(e.target.value))}
                  style={{ width: '100px', justifySelf: 'start' }}
                />

                <label htmlFor="app-settings-analytics-record-duration">{t('analytics_record_duration_label')}</label>
                <input 
                  type="number" 
                  id="app-settings-analytics-record-duration"
                  min="5" 
                  max="300" 
                  step="5"
                  value={settings.analytics_record_duration}
                  onChange={(e) => handleSettingChange('analytics_record_duration', parseInt(e.target.value))}
                  style={{ width: '100px', justifySelf: 'start' }}
                />
              </div>

              {/* ANPR Settings Section */}
              <div style={{ marginTop: '30px', padding: '20px', backgroundColor: '#252525', borderRadius: '8px', border: '1px solid #333' }}>
                <h4 style={{ marginTop: 0, marginBottom: '10px', display: 'flex', alignItems: 'center', gap: '8px' }}>
                  <span>🚗</span>
                  <span>{t('anpr_settings_header')}</span>
                </h4>
                <p style={{ color: '#888', fontSize: '13px', marginBottom: '20px' }}>
                  {t('anpr_settings_desc')}
                </p>

                <div className="form-grid simple">
                  <label htmlFor="anpr-detection-confidence">{t('anpr_detection_confidence_label')}</label>
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
                    <input 
                      type="number" 
                      id="anpr-detection-confidence"
                      min="0.1" 
                      max="1.0" 
                      step="0.05"
                      value={settings.anpr_detection_confidence}
                      onChange={(e) => handleSettingChange('anpr_detection_confidence', parseFloat(e.target.value))}
                      style={{ width: '100px' }}
                    />
                    <label htmlFor="anpr-detection-confidence" style={{ color: '#aaa', fontSize: '11px', lineHeight: '1.3' }}>
                      {t('anpr_detection_confidence_help')}
                    </label>
                  </div>

                  <label htmlFor="anpr-crop-expansion">{t('anpr_crop_expansion_label')}</label>
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
                    <input 
                      type="number" 
                      id="anpr-crop-expansion"
                      min="1.0" 
                      max="2.0" 
                      step="0.1"
                      value={settings.anpr_crop_expansion}
                      onChange={(e) => handleSettingChange('anpr_crop_expansion', parseFloat(e.target.value))}
                      style={{ width: '100px' }}
                    />
                    <label htmlFor="anpr-crop-expansion" style={{ color: '#aaa', fontSize: '11px', lineHeight: '1.3' }}>
                      {t('anpr_crop_expansion_help')}
                    </label>
                  </div>

                  <label htmlFor="anpr-crnn-confidence">{t('anpr_crnn_confidence_label')}</label>
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
                    <input 
                      type="number" 
                      id="anpr-crnn-confidence"
                      min="0.5" 
                      max="1.0" 
                      step="0.05"
                      value={settings.anpr_crnn_confidence}
                      onChange={(e) => handleSettingChange('anpr_crnn_confidence', parseFloat(e.target.value))}
                      style={{ width: '100px' }}
                    />
                    <label htmlFor="anpr-crnn-confidence" style={{ color: '#aaa', fontSize: '11px', lineHeight: '1.3' }}>
                      {t('anpr_crnn_confidence_help')}
                    </label>
                  </div>

                  <label htmlFor="anpr-python-confidence">{t('anpr_python_confidence_label')}</label>
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
                    <input 
                      type="number" 
                      id="anpr-python-confidence"
                      min="0.5" 
                      max="1.0" 
                      step="0.05"
                      value={settings.anpr_python_confidence}
                      onChange={(e) => handleSettingChange('anpr_python_confidence', parseFloat(e.target.value))}
                      style={{ width: '100px' }}
                    />
                    <label htmlFor="anpr-python-confidence" style={{ color: '#aaa', fontSize: '11px', lineHeight: '1.3' }}>
                      {t('anpr_python_confidence_help')}
                    </label>
                  </div>

                  <span>{t('anpr_enable_python_label')}</span>
                  <div className="form-check-inline">
                    <input
                      type="checkbox"
                      id="anpr-enable-python"
                      checked={settings.anpr_enable_python}
                      onChange={e => {
                        setSettings(prev => ({
                          ...prev,
                          anpr_enable_python: e.target.checked
                        }));
                      }}
                      className="form-check-input"
                    />
                    <label htmlFor="anpr-enable-python" style={{ color: '#aaa', fontSize: '12px' }}>
                      {t('anpr_enable_python_help')}
                    </label>
                  </div>
                </div>

                <div style={{ marginTop: '15px', padding: '12px', backgroundColor: '#2d4a2d', borderRadius: '4px', fontSize: '12px', color: '#a8ffa8' }}>
                  <strong>💡 {currentLanguage === 'ru' ? 'Совет' : 'Tip'}:</strong> {currentLanguage === 'ru' 
                    ? 'Для дальних камер увеличьте расширение обрезки до 1.4-1.5. Для камер под углом включите Python OCR.'
                    : 'For distant cameras, increase crop expansion to 1.4-1.5. For angled cameras, enable Python OCR.'}
                </div>
              </div>
            </div>
          )}

          {/* Modules Tab */}
          {activeTab === 'tab-modules' && (
            <div className="tab-content active intellect-only">
              <h3>{t('modules_management_header')}</h3>
              <p style={{ color: '#6c757d', fontSize: '13px' }}>
                {t('modules_description')}
              </p>
              {isLoadingModules && (
                <p style={{ color: '#6c757d', fontSize: '13px' }}>
                  {currentLanguage === 'ru' ? 'Обновляем статус модулей…' : 'Refreshing module status…'}
                </p>
              )}
              {!modulesReady && !isLoadingModules && (
                <p style={{ color: '#6c757d', fontSize: '13px' }}>
                  {currentLanguage === 'ru'
                    ? 'Информация о модулях загружается из бэкенда…'
                    : 'Module information is loading from backend…'}
                </p>
              )}
              <div id="modules-list" style={{ marginTop: '15px' }}>
                {modulesToRender.map(module => {
                  const meta = MODULE_METADATA[module.id];
                  const baseName = meta?.name ?? module.name;
                  const baseDescription = meta?.description ?? module.description;
                  const baseAuthor = meta?.author ?? '—';

                  const nameKey = `module_${module.id}_name`;
                  const translatedName = t(nameKey);
                  const displayName = translatedName === nameKey ? baseName : translatedName;

                  const descriptionKey = `module_${module.id}_description`;
                  const translatedDescription = t(descriptionKey);
                  const descriptionText = translatedDescription === descriptionKey ? baseDescription : translatedDescription;

                  const authorKey = `module_${module.id}_author`;
                  const translatedAuthor = t(authorKey);
                  const authorText = translatedAuthor === authorKey ? baseAuthor : translatedAuthor;

                  const statusColor =
                    module.state === 'error'
                      ? '#dc3545'
                      : module.state === 'ready'
                        ? '#28a745'
                        : '#6c757d';

                  const statusLabel = (() => {
                    if (module.state === 'ready') {
                      return currentLanguage === 'ru' ? 'Готов' : 'Ready';
                    }
                    if (module.state === 'loading') {
                      return module.message ?? (currentLanguage === 'ru' ? 'Загрузка…' : 'Loading…');
                    }
                    if (module.state === 'error') {
                      return module.message ?? (currentLanguage === 'ru' ? 'Ошибка' : 'Error');
                    }
                    return currentLanguage === 'ru' ? 'Отключён' : 'Disabled';
                  })();

                  const progressValue = Math.max(0, Math.min(1, module.progress ?? 0));
                  const progressPercent = Math.min(100, Math.max(0, Math.round(progressValue * 100)));
                  const isBusy = moduleOperationId === module.id;
                  const isDisabled = !modulesReady || isLoadingModules || isBusy;
                  const lastActivatedLabel = module.lastActivatedAt && modulesReady
                    ? (currentLanguage === 'ru'
                        ? `Активирован: ${new Date(module.lastActivatedAt).toLocaleString()}`
                        : `Activated: ${new Date(module.lastActivatedAt).toLocaleString()}`)
                    : null;
                  const lastErrorLabel = module.lastErrorAt && modulesReady
                    ? (currentLanguage === 'ru'
                        ? `Последняя ошибка: ${new Date(module.lastErrorAt).toLocaleString()}`
                        : `Last error: ${new Date(module.lastErrorAt).toLocaleString()}`)
                    : null;
                  const snapshotsDir = module.config?.snapshotsDir;
                  const snapshotsDisplay = snapshotsDir ?? (currentLanguage === 'ru'
                    ? 'Каталог по умолчанию'
                    : 'Default directory');
                  const isFaceDetector = module.id === 'face-detector';
                  const faceSnapshotMode = isFaceDetector
                    ? resolveFaceSnapshotMode(module.config?.faceSnapshotsMode)
                    : DEFAULT_FACE_SNAPSHOT_MODE;
                  const faceSnapshotModeCopy = FACE_SNAPSHOT_MODE_COPY[faceSnapshotMode][languageKey];
                  const faceSnapshotKeyConfigured = Boolean(module.config?.faceSnapshotKeyConfigured);
                  const faceSnapshotKeyValue = faceSnapshotKeyInputs[module.id] ?? '';
                  const faceSnapshotKeyError = faceSnapshotKeyErrors[module.id] ?? null;
                  const faceSnapshotKeyIsBusy = faceSnapshotKeyBusy[module.id] ?? false;

                  return (
                    <div key={module.id} className="form-check-inline" style={{ 
                      marginBottom: '15px', 
                      borderBottom: '1px solid #eee', 
                      paddingBottom: '10px', 
                      width: '100%', 
                      alignItems: 'flex-start',
                      flexDirection: 'column'
                    }}>
                      <div style={{ display: 'flex', alignItems: 'center', width: '100%' }}>
                        <input 
                          type="checkbox" 
                          id={`module-${module.id}`}
                          data-id={module.id}
                          className="form-check-input module-checkbox"
                          checked={module.enabled}
                          disabled={isDisabled}
                          onChange={async (e) => {
                            await handleModuleToggle(module.id, e.target.checked);
                          }}
                          style={{ marginTop: '5px' }}
                        />
                        <div style={{ flexGrow: 1, marginLeft: '10px' }}>
                          <label htmlFor={`module-${module.id}`} style={{ fontWeight: 'bold', fontSize: '1.1em' }}>
                            {displayName} <span style={{ fontSize: '0.8em', color: '#666' }}>v{module.version}</span>
                          </label>
                          <p style={{ margin: '5px 0 0 0', fontSize: '0.9em', color: '#d8dee9' }}>
                            {descriptionText}
                          </p>
                          <p style={{ margin: '5px 0 0 0', fontSize: '0.8em', color: '#b5c1d6' }}>
                            {t('author_prefix')}: {authorText}
                          </p>
                          <div style={{ marginTop: '8px', width: '100%' }}>
                            {module.state === 'loading' && (
                              <div className="module-progress-bar">
                                <div
                                  className={`module-progress-fill${isBusy ? ' is-loading' : ''}`}
                                  style={{ width: `${progressPercent}%` }}
                                />
                              </div>
                            )}
                            <div style={{ fontSize: '0.8em', color: statusColor }}>
                              {statusLabel}
                            </div>
                            {lastActivatedLabel && (
                              <div style={{ fontSize: '0.75em', color: '#6c757d', marginTop: '2px' }}>
                                {lastActivatedLabel}
                              </div>
                            )}
                            {module.state === 'error' && lastErrorLabel && (
                              <div style={{ fontSize: '0.75em', color: '#dc3545', marginTop: '2px' }}>
                                {lastErrorLabel}
                              </div>
                            )}
                            {['face-detector', 'license-plate-detector', 'object-counter'].includes(module.id) && (
                              <div className="module-config-block">
                                <div className="module-config-label">
                                  {module.id === 'license-plate-detector'
                                    ? currentLanguage === 'ru'
                                      ? 'Каталог снимков номеров'
                                      : 'License plate snapshots'
                                    : module.id === 'object-counter'
                                      ? currentLanguage === 'ru'
                                        ? 'Каталог снимков объектов'
                                        : 'Object snapshots'
                                      : currentLanguage === 'ru'
                                        ? 'Каталог для снимков лиц'
                                        : 'Snapshots directory'}
                                </div>
                                <div className="module-config-controls">
                                  <span
                                    className="module-config-value"
                                    title={snapshotsDisplay}
                                  >
                                    {snapshotsDisplay}
                                  </span>
                                  <button
                                    type="button"
                                    className="module-config-button primary"
                                    onClick={() => handleSnapshotsDirectoryUpdate(module.id, snapshotsDir)}
                                    disabled={isDisabled}
                                  >
                                    {currentLanguage === 'ru' ? 'Выбрать…' : 'Choose…'}
                                  </button>
                                  <button
                                    type="button"
                                    className="module-config-button secondary"
                                    onClick={() => handleSnapshotsDirectoryReset(module.id)}
                                    disabled={isDisabled}
                                  >
                                    {currentLanguage === 'ru' ? 'По умолчанию' : 'Use default'}
                                  </button>
                                </div>
                              </div>
                            )}
                            {isFaceDetector && (
                              <div className="module-config-block">
                                <div className="module-config-label">
                                  {currentLanguage === 'ru'
                                    ? 'Режим сохранения снимков'
                                    : 'Face snapshot mode'}
                                </div>
                                <div
                                  className="module-config-controls"
                                  style={{ flexDirection: 'column', alignItems: 'flex-start', gap: '8px' }}
                                >
                                  <select
                                    value={faceSnapshotMode}
                                    disabled={isDisabled}
                                    onChange={event =>
                                      handleFaceSnapshotModeChange(
                                        module.id,
                                        event.target.value as FaceSnapshotMode,
                                      )
                                    }
                                    style={{ width: '100%', maxWidth: '320px' }}
                                  >
                                    {FACE_SNAPSHOT_MODE_VALUES.map(mode => {
                                      const copy = FACE_SNAPSHOT_MODE_COPY[mode][languageKey];
                                      return (
                                        <option key={mode} value={mode}>
                                          {copy.label}
                                        </option>
                                      );
                                    })}
                                  </select>
                                  <span className="module-config-hint" style={{ fontSize: '0.85em' }}>
                                    {faceSnapshotModeCopy.description}
                                  </span>
                                  {faceSnapshotMode === 'encrypted' && (
                                    <span className="module-config-hint" style={{ fontSize: '0.85em', color: '#f0ad4e' }}>
                                      {currentLanguage === 'ru'
                                        ? 'Для шифрования требуется ключ из 64 шестнадцатеричных символов.'
                                        : 'Encryption requires a 64-character hexadecimal key.'}
                                    </span>
                                  )}
                                </div>
                              </div>
                            )}
                            {isFaceDetector && (
                              <div className="module-config-block">
                                <div className="module-config-label">
                                  {currentLanguage === 'ru'
                                    ? 'Ключ для шифрования снимков'
                                    : 'Snapshot encryption key'}
                                </div>
                                <div
                                  className="module-config-controls"
                                  style={{ flexDirection: 'column', alignItems: 'flex-start', gap: '6px', width: '100%' }}
                                >
                                  <div
                                    style={{
                                      display: 'flex',
                                      flexWrap: 'wrap',
                                      gap: '8px',
                                      width: '100%',
                                      alignItems: 'center',
                                    }}
                                  >
                                    <input
                                      type="text"
                                      value={faceSnapshotKeyValue}
                                      maxLength={64}
                                      spellCheck={false}
                                      autoComplete="off"
                                      placeholder={currentLanguage === 'ru'
                                        ? '64 символа в hex-формате'
                                        : '64 hex characters'}
                                      onChange={event =>
                                        handleFaceSnapshotKeyInputChange(module.id, event.target.value)
                                      }
                                      style={{
                                        flex: '1 1 240px',
                                        minWidth: '200px',
                                        padding: '6px 8px',
                                      }}
                                      disabled={isDisabled || faceSnapshotKeyIsBusy}
                                    />
                                    <button
                                      type="button"
                                      className="module-config-button primary"
                                      onClick={() => handleFaceSnapshotKeySave(module.id)}
                                      disabled={isDisabled || faceSnapshotKeyIsBusy}
                                    >
                                      {currentLanguage === 'ru' ? 'Сохранить' : 'Save'}
                                    </button>
                                    <button
                                      type="button"
                                      className="module-config-button secondary"
                                      onClick={() => handleFaceSnapshotKeyReset(module.id)}
                                      disabled={isDisabled || faceSnapshotKeyIsBusy || !faceSnapshotKeyConfigured}
                                    >
                                      {currentLanguage === 'ru' ? 'Сбросить ключ' : 'Reset key'}
                                    </button>
                                  </div>
                                  <div
                                    className="module-config-hint"
                                    style={{
                                      fontSize: '0.8em',
                                      color: faceSnapshotKeyConfigured ? '#28a745' : '#b5c1d6',
                                    }}
                                  >
                                    {faceSnapshotKeyConfigured
                                      ? currentLanguage === 'ru'
                                        ? 'Ключ установлен. Новый ключ заменит текущий.'
                                        : 'Key configured. Saving a new key will replace it.'
                                      : currentLanguage === 'ru'
                                        ? 'Ключ не задан. Укажите ключ, чтобы включить шифрование.'
                                        : 'No key configured. Provide one to enable encryption.'}
                                  </div>
                                  {faceSnapshotKeyError && (
                                    <div style={{ color: '#dc3545', fontSize: '0.8em' }}>
                                      {faceSnapshotKeyError}
                                    </div>
                                  )}
                                </div>
                              </div>
                            )}
                          </div>
                        </div>
                      </div>
                    </div>
                  );
                })}
              </div>
            </div>
          )}

          {/* About Tab */}
          {activeTab === 'tab-about' && (
            <div className="tab-content active intellect-only">
              <div className="about-container">
                <div id="about-logo-container">
                  {/* <img src="" alt="App Logo" className="about-logo" style={{ display: 'none' }} /> */}
                </div>
                <h3>{t('app_title')}</h3>
                <p><span>{t('about_version')}</span>: <span id="app-version">{appVersion}</span></p>
                <p>
                  {t('about_description')}
                </p>
                <p>
                  {t('about_author')}
                </p>
                <div className="donation-section">
                  <p>
                    {t('about_donation_prompt')}
                  </p>
                  <button 
                    id="donate-btn" 
                    className="donation-button"
                    onClick={openDonationPage}
                  >
                    <i className="material-icons">volunteer_activism</i>
                    <span>{t('about_donation_button')}</span>
                  </button>
                </div>
              </div>
            </div>
          )}
        </div>

        <div className="modal-footer">
          <div style={{ display: 'flex', width: '100%', justifyContent: 'space-between' }}>
            <button 
              id="report-issue-btn"
              style={{ backgroundColor: '#6c757d' }}
              onClick={reportIssue}
            >
              {t('report_issue_button')}
            </button>
            <button id="save-settings-btn" onClick={handleSaveSettings}>
              {t('save')}
            </button>
          </div>
        </div>
      </div>
      <Toast
        message={toast.message}
        severity={toast.severity}
        open={toast.open}
        onClose={hideToast}
      />
    </div>
  );
};

export default SettingsModal;
