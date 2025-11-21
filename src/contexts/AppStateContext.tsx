import React, { useState, useEffect, useCallback, useRef } from 'react';
import type { ReactNode } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { isTauriAvailable } from '../utils/tauri';
import { buildCameraRtspUrls } from '../utils/cameraStreams';
import type {
  Camera,
  CameraGroup,
  DashboardState,
  DashboardCellState,
  LayoutCameraAssignment,
  StoredLayoutTemplatePreview,
  StoredLayoutTemplate,
  StoredLayoutTab,
  WindowCameraBridge,
} from '../types';
import { MAX_DASHBOARD_CELLS } from '../types';
import { AppStateContext } from './AppStateContextData';
import type {
  AppStateContextType,
  AppStateSettings,
  StreamingProvider,
  CameraStatusEntry,
} from './AppStateContextData';

const defaultSettings: AppStateSettings = {
  language: 'en',
  recordingsFolder: '',
  screenshotsFolder: '',
  hardwareAcceleration: 'Auto',
  analyticsProvider: 'Auto (GPU if available)',
  enableNotifications: true,
  qscale: 8,
  fps: 20,
  analytics_resize_width: 640,
  analytics_frame_skip: 5,
  analytics_record_duration: 30,
  anpr_detection_confidence: 0.5,
  anpr_crop_expansion: 1.2,
  anpr_crnn_confidence: 0.75,
  anpr_python_confidence: 0.90,
  anpr_enable_python: true,
};

const clampNumber = (value: number, min: number, max: number) => Math.min(Math.max(value, min), max);

const toClampedNumber = (raw: unknown, fallback: number, min: number, max: number) => {
  if (typeof raw === 'number' && Number.isFinite(raw)) {
    return clampNumber(raw, min, max);
  }
  const parsed = Number(raw);
  return Number.isFinite(parsed) ? clampNumber(parsed, min, max) : fallback;
};

const pickString = (source: Record<string, unknown>, keys: string[], fallback: string) => {
  for (const key of keys) {
    const value = source[key];
    if (typeof value === 'string' && value.trim().length > 0) {
      return value;
    }
  }
  return fallback;
};

const pickBoolean = (source: Record<string, unknown>, keys: string[], fallback: boolean) => {
  for (const key of keys) {
    const value = source[key];
    if (typeof value === 'boolean') {
      return value;
    }
  }
  return fallback;
};

const pickNumber = (source: Record<string, unknown>, keys: string[], fallback: number, min: number, max: number) => {
  for (const key of keys) {
    if (key in source) {
      return toClampedNumber(source[key], fallback, min, max);
    }
  }
  return fallback;
};

const normalizeAppSettings = (input?: Record<string, unknown> | null): AppStateSettings => {
  const base = { ...defaultSettings };
  if (!input || typeof input !== 'object') {
    return base;
  }

  const source = input as Record<string, unknown>;

  const language = pickString(source, ['language'], base.language);
  const recordingsFolder = pickString(source, ['recordingsFolder', 'recordingsPath'], base.recordingsFolder);
  const screenshotsFolder = pickString(source, ['screenshotsFolder', 'screenshotsPath'], base.screenshotsFolder);
  const hardwareAcceleration = pickString(source, ['hardwareAcceleration', 'hwAccel'], base.hardwareAcceleration);
  const analyticsProvider = pickString(source, ['analyticsProvider', 'analytics_provider'], base.analyticsProvider);
  const enableNotifications = pickBoolean(source, ['enableNotifications', 'notifications_enabled'], base.enableNotifications);
  const qscale = pickNumber(source, ['qscale', 'qScale'], base.qscale, 2, 31);
  const fps = pickNumber(source, ['fps'], base.fps, 5, 60);

  const analytics_resize_width = pickNumber(source, ['analytics_resize_width'], base.analytics_resize_width, 0, 3840);
  const analytics_frame_skip = pickNumber(source, ['analytics_frame_skip'], base.analytics_frame_skip, 0, 60);
  const analytics_record_duration = pickNumber(source, ['analytics_record_duration'], base.analytics_record_duration, 5, 300);
  
  const anpr_detection_confidence = pickNumber(source, ['anpr_detection_confidence'], base.anpr_detection_confidence, 0.1, 1.0);
  const anpr_crop_expansion = pickNumber(source, ['anpr_crop_expansion'], base.anpr_crop_expansion, 1.0, 2.0);
  const anpr_crnn_confidence = pickNumber(source, ['anpr_crnn_confidence'], base.anpr_crnn_confidence, 0.1, 1.0);
  const anpr_python_confidence = pickNumber(source, ['anpr_python_confidence'], base.anpr_python_confidence, 0.1, 1.0);
  const anpr_enable_python = pickBoolean(source, ['anpr_enable_python'], base.anpr_enable_python);

  return {
    language,
    recordingsFolder,
    screenshotsFolder,
    hardwareAcceleration,
    analyticsProvider,
    enableNotifications,
    qscale,
    fps,
    analytics_resize_width,
    analytics_frame_skip,
    analytics_record_duration,
    anpr_detection_confidence,
    anpr_crop_expansion,
    anpr_crnn_confidence,
    anpr_python_confidence,
    anpr_enable_python,
  };
};

const mergeNormalizedIntoBackend = (
  normalized: AppStateSettings,
  previous: Record<string, unknown>,
): Record<string, unknown> => ({
  ...previous,
  language: normalized.language,
  recordingsFolder: normalized.recordingsFolder,
  recordingsPath: normalized.recordingsFolder,
  screenshotsFolder: normalized.screenshotsFolder,
  screenshotsPath: normalized.screenshotsFolder,
  hardwareAcceleration: normalized.hardwareAcceleration,
  hwAccel: normalized.hardwareAcceleration,
  analyticsProvider: normalized.analyticsProvider,
  analytics_provider: normalized.analyticsProvider,
  enableNotifications: normalized.enableNotifications,
  notifications_enabled: normalized.enableNotifications,
  qscale: normalized.qscale,
  fps: normalized.fps,
  analytics_resize_width: normalized.analytics_resize_width,
  analytics_frame_skip: normalized.analytics_frame_skip,
  analytics_record_duration: normalized.analytics_record_duration,
  anpr_detection_confidence: normalized.anpr_detection_confidence,
  anpr_crop_expansion: normalized.anpr_crop_expansion,
  anpr_crnn_confidence: normalized.anpr_crnn_confidence,
  anpr_python_confidence: normalized.anpr_python_confidence,
  anpr_enable_python: normalized.anpr_enable_python,
});
const STREAMING_PROVIDER: StreamingProvider = 'go2rtc';

const ensureGo2RtcStreamingProvider = (target: Record<string, unknown> | null | undefined) => {
  if (!target || typeof target !== 'object') {
    return;
  }

  const container = target as Record<string, unknown>;
  const existing = container.streaming;
  const record: Record<string, unknown> =
    existing && typeof existing === 'object'
      ? { ...(existing as Record<string, unknown>) }
      : {};

  record.provider = STREAMING_PROVIDER;

  if (typeof record.enableOnDemand !== 'boolean') {
    record.enableOnDemand = true;
  }

  if (typeof record.restartOnConfigChange !== 'boolean') {
    record.restartOnConfigChange = true;
  }

  container.streaming = record;
};

const createDefaultDashboardCellState = (): DashboardCellState => ({
  cameraId: null,
  quality: 'sd',
  muted: true,
  paused: false,
});

const createDefaultDashboardState = (): DashboardState => ({
  gridSize: 4,
  cellStates: Array.from({ length: MAX_DASHBOARD_CELLS }, () => createDefaultDashboardCellState()),
  savedTemplatePreviews: [],
  layoutTemplates: [],
  layoutTabs: [],
  activeLayoutTabId: null,
});

const normalizeDashboardState = (state: Partial<DashboardState> | null | undefined): DashboardState => {
  const base = createDefaultDashboardState();
  if (!state) {
    return base;
  }

  const gridSize = typeof state.gridSize === 'number' && state.gridSize > 0
    ? state.gridSize
    : base.gridSize;

  const inputCells = Array.isArray(state.cellStates) ? state.cellStates : [];
  const cellStates = Array.from({ length: MAX_DASHBOARD_CELLS }, (_, idx) => {
    const cell = inputCells[idx] as Partial<DashboardCellState> | undefined;
    if (!cell) {
      return createDefaultDashboardCellState();
    }

    const cameraId = typeof cell.cameraId === 'number' ? cell.cameraId : null;
  const quality: 'hd' | 'sd' = cell.quality === 'hd' ? 'hd' : 'sd';
    const muted = typeof cell.muted === 'boolean' ? cell.muted : true;
    const paused = typeof cell.paused === 'boolean' ? cell.paused : false;

    return { cameraId, quality, muted, paused };
  });

  const normalizeTemplatePreview = (preview: unknown): StoredLayoutTemplatePreview | null => {
    if (!preview || typeof preview !== 'object') {
      return null;
    }
    const data = preview as Record<string, unknown>;
    const createdAt = typeof data.createdAt === 'string' ? data.createdAt : new Date().toISOString();
    const previewCameras = Array.isArray(data.previewCameras)
      ? data.previewCameras.filter((value: unknown): value is string => typeof value === 'string')
      : [];

    const normalized: StoredLayoutTemplatePreview = {
      id: String(data.id ?? crypto.randomUUID?.() ?? Date.now().toString()),
      name: typeof data.name === 'string' ? data.name : 'Untitled layout',
      description: typeof data.description === 'string' ? data.description : undefined,
      gridSize: typeof data.gridSize === 'number' && data.gridSize > 0 ? (data.gridSize as number) : gridSize,
      cameraCount: typeof data.cameraCount === 'number' ? (data.cameraCount as number) : previewCameras.length,
      previewCameras,
      createdAt,
    };

    return normalized;
  };

  const normalizeLayoutTemplate = (template: unknown): StoredLayoutTemplate | null => {
    if (!template || typeof template !== 'object') {
      return null;
    }
    const data = template as Record<string, unknown>;

    const cameraAssignments: LayoutCameraAssignment[] = Array.isArray(data.cameraAssignments)
      ? (data.cameraAssignments as unknown[]).map((assignment): LayoutCameraAssignment => {
          if (!assignment || typeof assignment !== 'object') {
            return { cellIndex: 0, cameraId: null };
          }

          const record = assignment as Record<string, unknown>;
          const cellIndex = typeof record.cellIndex === 'number' ? record.cellIndex : 0;
          const cameraId = typeof record.cameraId === 'number' ? record.cameraId : null;
          return { cellIndex, cameraId };
        })
      : [];

    const normalized: StoredLayoutTemplate = {
      id: String(data.id ?? crypto.randomUUID?.() ?? Date.now().toString()),
      name: typeof data.name === 'string' ? (data.name as string) : 'Untitled layout',
      description: typeof data.description === 'string' ? (data.description as string) : undefined,
      gridSize: typeof data.gridSize === 'number' && data.gridSize > 0 ? (data.gridSize as number) : gridSize,
      cameraAssignments,
      createdAt: typeof data.createdAt === 'string' ? (data.createdAt as string) : new Date().toISOString(),
      updatedAt: typeof data.updatedAt === 'string' ? (data.updatedAt as string) : new Date().toISOString(),
    };

    return normalized;
  };

  const normalizeLayoutTab = (tab: unknown): StoredLayoutTab | null => {
    if (!tab || typeof tab !== 'object') {
      return null;
    }
    const data = tab as Record<string, unknown>;

    const templateId = typeof data.templateId === 'string'
      ? (data.templateId as string)
      : typeof (data.template as Record<string, unknown> | undefined)?.id === 'string'
        ? ((data.template as Record<string, unknown>).id as string)
        : '';
    if (!templateId) {
      return null;
    }

    const normalized: StoredLayoutTab = {
      id: String(data.id ?? crypto.randomUUID?.() ?? Date.now().toString()),
      name: typeof data.name === 'string' ? (data.name as string) : 'Layout',
      templateId,
    };

    return normalized;
  };

  const savedTemplatePreviews = Array.isArray(state.savedTemplatePreviews)
    ? state.savedTemplatePreviews
        .map(normalizeTemplatePreview)
        .filter((preview): preview is StoredLayoutTemplatePreview => Boolean(preview))
    : base.savedTemplatePreviews;

  const layoutTemplates = Array.isArray(state.layoutTemplates)
    ? state.layoutTemplates
        .map(normalizeLayoutTemplate)
        .filter((template): template is StoredLayoutTemplate => Boolean(template))
    : base.layoutTemplates;

  const layoutTabs = Array.isArray(state.layoutTabs)
    ? state.layoutTabs
        .map(normalizeLayoutTab)
        .filter((tab): tab is StoredLayoutTab => Boolean(tab))
    : base.layoutTabs;

  const activeLayoutTabId = typeof state.activeLayoutTabId === 'string' ? state.activeLayoutTabId : base.activeLayoutTabId;

  return { gridSize, cellStates, savedTemplatePreviews, layoutTemplates, layoutTabs, activeLayoutTabId };
};

const dashboardStatesEqual = (a: DashboardState, b: DashboardState): boolean => {
  if (a === b) return true;
  if (a.gridSize !== b.gridSize) return false;
  const len = Math.max(a.cellStates.length, b.cellStates.length);
  for (let i = 0; i < len; i += 1) {
    const cellA = a.cellStates[i] ?? createDefaultDashboardCellState();
    const cellB = b.cellStates[i] ?? createDefaultDashboardCellState();
    if (
      cellA.cameraId !== cellB.cameraId ||
      cellA.quality !== cellB.quality ||
      cellA.muted !== cellB.muted ||
      cellA.paused !== cellB.paused
    ) {
      return false;
    }
  }

  const shallowEqual = (lhs: unknown, rhs: unknown) => JSON.stringify(lhs ?? null) === JSON.stringify(rhs ?? null);

  if (!shallowEqual(a.savedTemplatePreviews ?? [], b.savedTemplatePreviews ?? [])) {
    return false;
  }

  if (!shallowEqual(a.layoutTemplates ?? [], b.layoutTemplates ?? [])) {
    return false;
  }

  if (!shallowEqual(a.layoutTabs ?? [], b.layoutTabs ?? [])) {
    return false;
  }

  if ((a.activeLayoutTabId ?? null) !== (b.activeLayoutTabId ?? null)) {
    return false;
  }

  return true;
};

interface AppStateProviderProps {
  children: ReactNode;
}

export const AppStateProvider: React.FC<AppStateProviderProps> = ({ children }) => {
  const [cameras, setCamerasState] = useState<Camera[]>([]);
  const [groups, setGroupsState] = useState<CameraGroup[]>([]);
  const [settings, setSettingsState] = useState<AppStateSettings>(defaultSettings);
  const backendSettingsRef = useRef<Record<string, unknown>>({});
  const streamingProvider: StreamingProvider = STREAMING_PROVIDER;
  const [isLoading, setIsLoading] = useState(true);
  const [dashboardState, setDashboardState] = useState<DashboardState>(createDefaultDashboardState());
  const [cameraStatuses, setCameraStatuses] = useState<Record<number, CameraStatusEntry>>({});
  const streamingStartedRef = useRef(false);
  const streamingStartPromiseRef = useRef<Promise<void> | null>(null);
  const prewarmedCamerasRef = useRef<Set<number>>(new Set());

  const setGlobalCameras = useCallback((cameraList: Camera[]) => {
    if (typeof window === 'undefined') {
      return;
    }
    (window as WindowCameraBridge).__VMS_CAMERAS = cameraList;
  }, []);

  const ensureStreamingBackendStarted = useCallback(async () => {
    if (!isTauriAvailable()) {
      return;
    }

    if (streamingStartedRef.current) {
      return;
    }

    if (streamingStartPromiseRef.current) {
      await streamingStartPromiseRef.current;
      return;
    }

    const startPromise = (async () => {
      try {
        const result = await invoke<string>('start_go2rtc');
        streamingStartedRef.current = true;
        console.log('AppStateContext: go2rtc start result:', result);
      } catch (error) {
        console.warn('AppStateContext: Failed to start go2rtc backend:', error);
      } finally {
        streamingStartPromiseRef.current = null;
      }
    })();

    streamingStartPromiseRef.current = startPromise;
    await startPromise;
  }, []);

  const prewarmCameraStreams = useCallback(async (camera: Camera) => {
    if (!isTauriAvailable()) {
      return;
    }

    if (typeof camera?.id !== 'number') {
      return;
    }

    if (prewarmedCamerasRef.current.has(camera.id)) {
      return;
    }

    prewarmedCamerasRef.current.add(camera.id);

    await ensureStreamingBackendStarted();

    try {
      const { hdUrl, sdUrl } = await buildCameraRtspUrls(camera);
      await invoke('add_camera_streams', {
        cameraId: camera.id,
        hdUrl,
        sdUrl,
      });
      console.log('AppStateContext: Prepared go2rtc streams for camera', camera.id);
    } catch (error) {
      prewarmedCamerasRef.current.delete(camera.id);
      console.warn('AppStateContext: Failed to prepare go2rtc for camera', camera.id, error);
    }
  }, [ensureStreamingBackendStarted]);

  // Загрузка состояния приложения при инициализации
  const loadAppState = useCallback(async () => {
    setIsLoading(true);
    try {
      console.log('AppStateContext: Loading app state...');
      
      // Загружаем камеры
  const loadedCameras = await invoke<Camera[]>('load_cameras');
  console.log('AppStateContext: Loaded cameras:', loadedCameras);
  const normalizedCameras = loadedCameras ?? [];
  setCamerasState(normalizedCameras);
  setGlobalCameras(normalizedCameras);
  console.log('AppStateContext: Set global __VMS_CAMERAS:', normalizedCameras.length, 'cameras');
      
      // Загружаем группы
      const loadedGroups = await invoke<CameraGroup[]>('load_groups');
      console.log('AppStateContext: Loaded groups:', loadedGroups);
      setGroupsState(loadedGroups || []);
      
      // Загружаем настройки из backend и localStorage
      let settingsLoaded = false;

      if (isTauriAvailable()) {
        try {
          const backendSettings = await invoke<Record<string, unknown> | null>('get_app_settings');
          if (backendSettings) {
            backendSettingsRef.current = { ...backendSettings };

            const previousProvider = (() => {
              const streaming = (backendSettings as Record<string, unknown>).streaming;
              if (streaming && typeof streaming === 'object') {
                const provider = (streaming as Record<string, unknown>).provider;
                return typeof provider === 'string' ? provider : undefined;
              }
              return undefined;
            })();

            ensureGo2RtcStreamingProvider(backendSettingsRef.current);

            const updatedProvider = (() => {
              const streaming = backendSettingsRef.current?.streaming;
              if (streaming && typeof streaming === 'object') {
                const provider = (streaming as Record<string, unknown>).provider;
                return typeof provider === 'string' ? provider : undefined;
              }
              return undefined;
            })();

            if (previousProvider !== STREAMING_PROVIDER && updatedProvider === STREAMING_PROVIDER) {
              try {
                await invoke('save_app_settings', { settings: backendSettingsRef.current });
              } catch (saveError) {
                console.warn('AppStateContext: Failed to persist go2rtc streaming provider override:', saveError);
              }
            }

            const normalized = normalizeAppSettings(backendSettings);
            setSettingsState(normalized);
            settingsLoaded = true;

            try {
              localStorage.setItem('appSettings', JSON.stringify(backendSettingsRef.current));
              localStorage.setItem('app_settings', JSON.stringify(normalized));
            } catch (storageError) {
              console.warn('AppStateContext: Failed to persist appSettings to localStorage:', storageError);
            }
          }
        } catch (error) {
          console.warn('AppStateContext: Failed to load settings from backend:', error);
        }
      }

      if (!settingsLoaded) {
        const savedSettings =
          localStorage.getItem('appSettings') ?? localStorage.getItem('app_settings');
        if (savedSettings) {
          try {
            const parsedSettings = JSON.parse(savedSettings) as Record<string, unknown>;
            backendSettingsRef.current = mergeNormalizedIntoBackend(
              normalizeAppSettings(parsedSettings),
              backendSettingsRef.current,
            );
            ensureGo2RtcStreamingProvider(backendSettingsRef.current);
            const normalized = normalizeAppSettings(parsedSettings);
            setSettingsState(normalized);
            try {
              localStorage.setItem('app_settings', JSON.stringify(normalized));
            } catch (storageError) {
              console.warn('AppStateContext: Failed to persist normalized backup settings:', storageError);
            }
            settingsLoaded = true;
            console.log('AppStateContext: Loaded settings:', parsedSettings);
          } catch (error) {
            console.error('AppStateContext: Failed to parse settings:', error);
          }
        }
      } else {
        // Legacy backup key with normalized structure only
        const legacySettings = localStorage.getItem('app_settings');
        if (legacySettings) {
          try {
            const parsedLegacy = JSON.parse(legacySettings) as Record<string, unknown>;
            const normalizedLegacy = normalizeAppSettings(parsedLegacy);
            backendSettingsRef.current = mergeNormalizedIntoBackend(normalizedLegacy, backendSettingsRef.current);
            ensureGo2RtcStreamingProvider(backendSettingsRef.current);
            setSettingsState(normalizedLegacy);
            settingsLoaded = true;
          } catch (error) {
            console.error('AppStateContext: Failed to parse legacy normalized settings:', error);
          }
        }
      }

        const savedDashboardState = localStorage.getItem('dashboard_state');
        if (savedDashboardState) {
          try {
            const parsedDashboard = JSON.parse(savedDashboardState) as Partial<DashboardState>;
            const normalized = normalizeDashboardState(parsedDashboard);
            setDashboardState(normalized);
            console.log('AppStateContext: Loaded dashboard state');
          } catch (error) {
            console.error('AppStateContext: Failed to parse dashboard state:', error);
            setDashboardState(createDefaultDashboardState());
          }
        } else {
          setDashboardState(createDefaultDashboardState());
        }
      
      console.log('AppStateContext: App state loaded successfully:', { 
        cameras: loadedCameras?.length || 0, 
        groups: loadedGroups?.length || 0 
      });
    } catch (error) {
      console.error('AppStateContext: Failed to load app state:', error);
    } finally {
      setIsLoading(false);
    }
  }, [setGlobalCameras]);

  // Сохранение состояния приложения
  const saveAppState = useCallback(async () => {
    try {
      console.log('AppStateContext: Saving app state...', { 
        camerasCount: cameras.length, 
        groupsCount: groups.length 
      });
      
      // Сохраняем камеры
      await invoke('save_cameras', { cameras });
      console.log('AppStateContext: Cameras saved successfully');
      
      // Сохраняем группы
      await invoke('save_groups', { groups });
      console.log('AppStateContext: Groups saved successfully');
      
      // NOTE: Settings are NOT saved here to prevent overwriting with partial state
      // Settings are only saved through SettingsModal.handleSaveSettings()
      // This prevents losing fields like analytics_provider, anpr_*, go2rtcEnhanced, etc.
      console.log('AppStateContext: Settings NOT saved (managed by SettingsModal only)');

      localStorage.setItem('dashboard_state', JSON.stringify(dashboardState));
      console.log('AppStateContext: Dashboard state saved successfully');
      
      console.log('AppStateContext: App state saved successfully');
    } catch (error) {
      console.error('AppStateContext: Failed to save app state:', error);
    }
  }, [cameras, dashboardState, groups]); // Removed 'settings' from dependencies

  // Debounced auto-save to prevent excessive disk writes
  // КРИТИЧНО: Разделяем сохранение камер и дашборда для предотвращения избыточных записей
  useEffect(() => {
    if (isLoading) {
      return;
    }

    const timer = setTimeout(() => {
      console.log('AppStateContext: Auto-saving cameras due to cameras change (debounced)');
      // Сохраняем только камеры, не трогая остальное
      void invoke('save_cameras', { cameras }).catch((error) => {
        console.error('AppStateContext: Failed to save cameras:', error);
      });
    }, 5000); // 5 second debounce

    return () => clearTimeout(timer);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [cameras, isLoading]); // Только cameras, без dashboardState!

  // Отдельный эффект для сохранения dashboard state (только в localStorage)
  useEffect(() => {
    if (isLoading) {
      return;
    }

    const timer = setTimeout(() => {
      console.log('AppStateContext: Auto-saving dashboard state to localStorage (debounced)');
      localStorage.setItem('dashboard_state', JSON.stringify(dashboardState));
    }, 1000); // Быстрее, т.к. только localStorage

    return () => clearTimeout(timer);
  }, [dashboardState, isLoading]);

  // Отдельный эффект для сохранения groups
  useEffect(() => {
    if (isLoading) {
      return;
    }

    const timer = setTimeout(() => {
      console.log('AppStateContext: Auto-saving groups due to groups change (debounced)');
      void invoke('save_groups', { groups }).catch((error) => {
        console.error('AppStateContext: Failed to save groups:', error);
      });
    }, 5000);

    return () => clearTimeout(timer);
  }, [groups, isLoading]);

  // Инициализация при монтировании
  useEffect(() => {
    void loadAppState();
  }, [loadAppState]);

  useEffect(() => {
    if (!isTauriAvailable()) {
      return;
    }
    void ensureStreamingBackendStarted();
  }, [ensureStreamingBackendStarted]);

  useEffect(() => {
    if (!isTauriAvailable() || isLoading) {
      return;
    }

    let cancelled = false;

    const warmup = async () => {
      for (const camera of cameras) {
        if (cancelled) {
          break;
        }
        await prewarmCameraStreams(camera);
      }
    };

    void warmup();

    return () => {
      cancelled = true;
    };
  }, [cameras, isLoading, prewarmCameraStreams]);

  // Обновляем глобальную переменную при изменении cameras
  useEffect(() => {
    setGlobalCameras(cameras);
  }, [cameras, setGlobalCameras]);

  // Функции для работы с камерами
  const setCameras = (newCameras: Camera[]) => {
    setCamerasState(newCameras);
    setGlobalCameras(newCameras);
  };

  const addCamera = async (camera: Camera) => {
    console.log('AppStateContext: Adding camera:', camera);
    const newCameras = [...cameras, camera];
    setCamerasState(newCameras);
    setGlobalCameras(newCameras);
    console.log('AppStateContext: Camera added, new cameras count:', newCameras.length);
    await prewarmCameraStreams(camera);
  };

  const updateCamera = async (updatedCamera: Camera) => {
    const newCameras = cameras.map(cam => 
      cam.id === updatedCamera.id ? updatedCamera : cam
    );
    setCamerasState(newCameras);
    setGlobalCameras(newCameras);
    if (typeof updatedCamera.id === 'number') {
      prewarmedCamerasRef.current.delete(updatedCamera.id);
    }
    await prewarmCameraStreams(updatedCamera);
  };

  const removeCamera = async (cameraId: number) => {
    const newCameras = cameras.filter(cam => cam.id !== cameraId);
    setCamerasState(newCameras);
    setGlobalCameras(newCameras);
    prewarmedCamerasRef.current.delete(cameraId);
  };

  // Функции для работы с группами
  const setGroups = (newGroups: CameraGroup[]) => {
    setGroupsState(newGroups);
  };

  const addGroup = async (group: CameraGroup) => {
    const newGroups = [...groups, group];
    setGroupsState(newGroups);
  };

  const updateGroup = async (updatedGroup: CameraGroup) => {
    const newGroups = groups.map(group => 
      group.id === updatedGroup.id ? updatedGroup : group
    );
    setGroupsState(newGroups);
  };

  const removeGroup = async (groupId: number) => {
    const newGroups = groups.filter(group => group.id !== groupId);
    setGroupsState(newGroups);
    
    // Убираем камеры из удаленной группы
    const updatedCameras = cameras.map(camera => 
      camera.groupId === groupId ? { ...camera, groupId: undefined } : camera
    );
    setCamerasState(updatedCameras);
    setGlobalCameras(updatedCameras);
  };

  // Функции для работы с настройками
  const persistSettings = useCallback(async (normalized: AppStateSettings) => {
    const rawSettings = mergeNormalizedIntoBackend(normalized, backendSettingsRef.current);
    ensureGo2RtcStreamingProvider(rawSettings);
    backendSettingsRef.current = rawSettings;

    try {
      localStorage.setItem('appSettings', JSON.stringify(rawSettings));
      localStorage.setItem('app_settings', JSON.stringify(normalized));
    } catch (error) {
      console.warn('AppStateContext: Failed to cache settings locally:', error);
    }

    if (isTauriAvailable()) {
      try {
        await invoke('save_app_settings', { settings: rawSettings });
      } catch (error) {
        console.warn('AppStateContext: Failed to persist settings via Tauri:', error);
      }
    }
  }, []);

  const updateSettings = async (newSettings: Partial<AppStateSettings>) => {
    const updatedSettings = normalizeAppSettings({ ...settings, ...newSettings });
    setSettingsState(updatedSettings);
    await persistSettings(updatedSettings);
  };

  const updateDashboardState = (
    updater: DashboardState | ((prev: DashboardState) => DashboardState)
  ) => {
    setDashboardState(prev => {
      const nextRaw = typeof updater === 'function'
        ? (updater as (prev: DashboardState) => DashboardState)(prev)
        : updater;
      const next = normalizeDashboardState(nextRaw);
      return dashboardStatesEqual(prev, next) ? prev : next;
    });
  };

  const updateCameraStatus = useCallback((cameraId: number, status: CameraStatusEntry) => {
    if (typeof cameraId !== 'number') {
      return;
    }

    setCameraStatuses(prev => {
      const prevEntry = prev[cameraId];
      if (
        prevEntry &&
        prevEntry.status === status.status &&
        prevEntry.lastUpdated === status.lastUpdated &&
        prevEntry.bitrateKbps === status.bitrateKbps &&
        prevEntry.frameRate === status.frameRate
      ) {
        return prev;
      }

      return {
        ...prev,
        [cameraId]: { ...status },
      };
    });
  }, []);

  const contextValue: AppStateContextType = {
    // Cameras
    cameras,
    setCameras,
    addCamera,
    updateCamera,
    removeCamera,
    
    // Groups
    groups,
    setGroups,
    addGroup,
    updateGroup,
    removeGroup,
    
    // Settings
    settings,
    updateSettings,
    streamingProvider,
    ensureStreamingBackendStarted,
    
    // State management
    isLoading,
    loadAppState,
    saveAppState,

    // Dashboard layout
    dashboardState,
    updateDashboardState,

    // Camera statuses
    cameraStatuses,
    updateCameraStatus,
  };

  return (
    <AppStateContext.Provider value={contextValue}>
      {children}
    </AppStateContext.Provider>
  );
};