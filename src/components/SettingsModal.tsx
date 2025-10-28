import React, { useState, useEffect } from 'react';
import { open as openDialog, save } from '@tauri-apps/plugin-dialog';
import { open as openExternal } from '@tauri-apps/plugin-shell';
import { invoke } from '@tauri-apps/api/core';
import { getVersion } from '@tauri-apps/api/app';
import { useLocalization } from '../contexts/LocalizationContext';
import type { SupportedLanguage } from '../contexts/LocalizationContext';
import { Toast, useToast } from './Toast';
import './SettingsModal.css';

interface SettingsModalProps {
  isOpen: boolean;
  onClose: () => void;
}

interface StreamingBackendSettings {
  provider: 'mediamtx';
  enableOnDemand: boolean;
  restartOnConfigChange: boolean;
}

const DEFAULT_STREAMING_SETTINGS: StreamingBackendSettings = {
  provider: 'mediamtx',
  enableOnDemand: true,
  restartOnConfigChange: true,
};

const createDefaultStreamingSettings = (): StreamingBackendSettings => ({
  ...DEFAULT_STREAMING_SETTINGS,
});

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
  enabledModules: string[];
  streaming: StreamingBackendSettings;
}

interface ModuleInfo {
  id: string;
  name: string;
  version: string;
  description: string;
  author: string;
}

const SettingsModal: React.FC<SettingsModalProps> = ({ isOpen, onClose }) => {
  const { currentLanguage, setLanguage, t } = useLocalization();
  const { toast, showToast, hideToast } = useToast();
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
    enabledModules: [],
    streaming: createDefaultStreamingSettings()
  });

  const normalizeStreamingSettings = (input?: unknown): StreamingBackendSettings => {
    const base = createDefaultStreamingSettings();
    if (!input || typeof input !== 'object') {
      return base;
    }

    const source = input as Record<string, unknown>;

    return {
      provider: 'mediamtx',
      enableOnDemand:
        typeof source.enableOnDemand === 'boolean'
          ? source.enableOnDemand
          : base.enableOnDemand,
      restartOnConfigChange:
        typeof source.restartOnConfigChange === 'boolean'
          ? source.restartOnConfigChange
          : base.restartOnConfigChange,
    };
  };

  const handleStreamingSettingChange = (
    key: keyof StreamingBackendSettings,
    value: unknown
  ) => {
    setSettings(prev => {
      const nextStreaming: StreamingBackendSettings = {
        ...prev.streaming,
      };

      switch (key) {
        case 'provider':
          nextStreaming.provider = 'mediamtx';
          break;
        case 'enableOnDemand':
          nextStreaming.enableOnDemand = Boolean(value);
          break;
        case 'restartOnConfigChange':
          nextStreaming.restartOnConfigChange = Boolean(value);
          break;
        default:
          break;
      }

      return {
        ...prev,
        streaming: nextStreaming
      };
    });
  };
  const prepareStreamingForSave = (
    streaming: StreamingBackendSettings
  ): StreamingBackendSettings => ({
    provider: 'mediamtx',
    enableOnDemand: Boolean(streaming.enableOnDemand),
    restartOnConfigChange: Boolean(streaming.restartOnConfigChange),
  });
  const [updateStatus, setUpdateStatus] = useState('idle');
  const [availableModules] = useState<ModuleInfo[]>([
    {
      id: 'face-detector',
      name: 'Face Detector',
      version: '0.0.1',
      description: 'Обнаруживает и сохраняет лица людей из видеопотока.',
      author: 'Rinibr'
    },
    {
      id: 'license-plate-detector',
      name: 'License Plate Detector',
      version: '0.0.1',
      description: 'Сохраняет обрезанные изображения обнаруженных номерных знаков и уведомляет интерфейс.',
      author: 'OpenIPC'
    },
    {
      id: 'object-counter',
      name: 'Object Counter',
      version: '0.0.1',
      description: 'Отображает счетчик обнаруженных объектов в ячейке.',
      author: 'Rinibr'
    }
  ]);

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
        const savedSettings = await invoke('get_app_settings');
        const saved = savedSettings as Record<string, unknown>;
        setSettings(prev => {
          const mergedStreaming = normalizeStreamingSettings(saved['streaming'] ?? prev.streaming);
          const merged = {
            ...prev,
            ...(saved as any),
            language: currentLanguage,
            streaming: mergedStreaming
          } as AppSettings;
          return merged;
        });
      } catch (error) {
        console.error('Failed to load settings:', error);
        // Fallback to localStorage
        const localSettings = localStorage.getItem('appSettings');
        if (localSettings) {
          const parsed = JSON.parse(localSettings) as Record<string, unknown>;
          setSettings(prev => {
            const mergedStreaming = normalizeStreamingSettings(parsed['streaming'] ?? prev.streaming);
            const merged = {
              ...prev,
              ...(parsed as any),
              language: currentLanguage,
              streaming: mergedStreaming
            } as AppSettings;
            return merged;
          });
        } else {
          // If no saved settings, use current language
          setSettings(prev => ({
            ...prev,
            language: currentLanguage,
            streaming: prev.streaming ?? createDefaultStreamingSettings()
          }));
        }
      }
    };
    
    if (isOpen) {
      loadAppVersion();
      loadSettings();
    }
  }, [isOpen]);

  const handleTabClick = (tabId: string) => {
    setActiveTab(tabId);
  };

  const handleSettingChange = (key: keyof AppSettings, value: any) => {
    if (key === 'streaming') {
      console.warn('Use handleStreamingSettingChange for streaming updates');
      return;
    }
    setSettings(prev => ({ ...prev, [key]: value }));
    
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
      
      // Also save to localStorage as backup
      localStorage.setItem('appSettings', JSON.stringify(payload));

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
      // Check if we're in Tauri environment using a more reliable method
      console.log('Checking Tauri environment...');
      console.log('window.__TAURI__:', typeof window !== 'undefined' && (window as any).__TAURI__);
      console.log('window.__TAURI_INTERNALS__:', typeof window !== 'undefined' && (window as any).__TAURI_INTERNALS__);
      console.log('User agent:', navigator.userAgent);
      
      // Check if we're in Tauri environment
      const isTauri = typeof window !== 'undefined' && 
                     ((window as any).__TAURI__ || (window as any).__TAURI_INTERNALS__);
      
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

  const handleModuleToggle = (moduleId: string, enabled: boolean) => {
    const newEnabledModules = enabled 
      ? [...settings.enabledModules, moduleId]
      : settings.enabledModules.filter(id => id !== moduleId);
    handleSettingChange('enabledModules', newEnabledModules);
  };

  const handleExportConfig = async () => {
    try {
      // Check if we're in Tauri environment
      const isTauri = typeof window !== 'undefined' && 
                     ((window as any).__TAURI__ || (window as any).__TAURI_INTERNALS__);
      
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
    input.onchange = (e) => {
      const file = (e.target as HTMLInputElement).files?.[0];
      if (file) {
        const reader = new FileReader();
        reader.onload = (e) => {
          try {
            const config = JSON.parse(e.target?.result as string);
            setSettings(prev => ({ ...prev, ...config }));
            
            const toast = document.getElementById('app-toast');
            if (toast) {
              toast.textContent = 'Конфигурация импортирована';
              toast.className = 'toast-notification show';
              setTimeout(() => {
                toast.className = 'toast-notification';
              }, 3000);
            }
          } catch (error) {
            alert('Ошибка при импорте конфигурации');
          }
        };
        reader.readAsText(file);
      }
    };
    input.click();
  };

  const handleCheckForUpdates = () => {
    setUpdateStatus('checking');
    // Simulate update check
    setTimeout(() => {
      setUpdateStatus('latest');
      const toast = document.getElementById('app-toast');
      if (toast) {
        toast.textContent = 'У вас установлена последняя версия';
        toast.className = 'toast-notification show';
        setTimeout(() => {
          toast.className = 'toast-notification';
        }, 3000);
      }
    }, 2000);
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
                <select 
                  id="app-settings-hw-accel"
                  value={settings.hwAccel}
                  onChange={(e) => handleSettingChange('hwAccel', e.target.value)}
                >
                  <option value="auto">{t('settings_hw_accel_auto')}</option>
                  <option value="nvidia">{t('settings_hw_accel_nvidia')}</option>
                  <option value="intel">{t('settings_hw_accel_intel')}</option>
                  <option value="none">{t('settings_hw_accel_none')}</option>
                </select>
              </div>

              <div className="intellect-only">
                <div className="form-grid simple with-button">
                  <span>{t('settings_analytics_provider')}</span>
                  <select 
                    id="app-settings-analytics-provider"
                    value={settings.analytics_provider}
                    onChange={(e) => handleSettingChange('analytics_provider', e.target.value)}
                  >
                    <option value="auto">{t('analytics_provider_auto')}</option>
                    <option value="dml">{t('analytics_provider_dml')}</option>
                    <option value="cpu">{t('analytics_provider_cpu')}</option>
                  </select>
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

              <div className="form-grid simple">
                <span>{t('streaming_provider_label')}</span>
                <span style={{ fontWeight: 500 }}>{t('streaming_provider_mediamtx')}</span>

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
            </div>
          )}

          {/* Analytics Tab */}
          {activeTab === 'tab-analytics' && (
            <div className="tab-content active intellect-only">
              <h3>{t('global_analytics_settings_header')}</h3>
              <p style={{ color: '#666', fontSize: '13px' }}>
                {t('global_analytics_settings_desc')}
              </p>
              <div className="form-grid simple">
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
            </div>
          )}

          {/* Modules Tab */}
          {activeTab === 'tab-modules' && (
            <div className="tab-content active intellect-only">
              <h3>{t('modules_management_header')}</h3>
              <p style={{ color: '#6c757d', fontSize: '13px' }}>
                {t('modules_description')}
              </p>
              <div id="modules-list" style={{ marginTop: '15px' }}>
                {availableModules.map(module => (
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
                        checked={settings.enabledModules.includes(module.id)}
                        onChange={(e) => handleModuleToggle(module.id, e.target.checked)}
                        style={{ marginTop: '5px' }}
                      />
                      <div style={{ flexGrow: 1, marginLeft: '10px' }}>
                        <label htmlFor={`module-${module.id}`} style={{ fontWeight: 'bold', fontSize: '1.1em' }}>
                          {module.name} <span style={{ fontSize: '0.8em', color: '#666' }}>v{module.version}</span>
                        </label>
                        <p style={{ margin: '5px 0 0 0', fontSize: '0.9em', color: '#333' }}>
                          {t(`module_${module.id}_description`)}
                        </p>
                        <p style={{ margin: '5px 0 0 0', fontSize: '0.8em', color: '#888' }}>
                          {t('author_prefix')}: {t(`module_${module.id}_author`)}
                        </p>
                      </div>
                    </div>
                  </div>
                ))}
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