import React, { createContext, useContext, useState, useEffect } from 'react';
import type { ReactNode } from 'react';
import { invoke } from '@tauri-apps/api/core';
import type {
  Camera,
  CameraGroup,
  DashboardState,
  DashboardCellState,
  StoredLayoutTemplatePreview,
  StoredLayoutTemplate,
  StoredLayoutTab,
} from '../types';
import { MAX_DASHBOARD_CELLS } from '../types';

interface AppState {
  cameras: Camera[];
  groups: CameraGroup[];
  settings: {
    language: string;
    recordingsFolder: string;
    screenshotsFolder: string;
    hardwareAcceleration: string;
    analyticsProvider: string;
    enableNotifications: boolean;
  };
}

interface AppStateContextType {
  // Cameras
  cameras: Camera[];
  setCameras: (cameras: Camera[]) => void;
  addCamera: (camera: Camera) => Promise<void>;
  updateCamera: (camera: Camera) => Promise<void>;
  removeCamera: (cameraId: number) => Promise<void>;
  
  // Groups
  groups: CameraGroup[];
  setGroups: (groups: CameraGroup[]) => void;
  addGroup: (group: CameraGroup) => Promise<void>;
  updateGroup: (group: CameraGroup) => Promise<void>;
  removeGroup: (groupId: number) => Promise<void>;
  
  // Settings
  settings: AppState['settings'];
  updateSettings: (newSettings: Partial<AppState['settings']>) => Promise<void>;
  
  // State management
  isLoading: boolean;
  loadAppState: () => Promise<void>;
  saveAppState: () => Promise<void>;

  // Dashboard layout
  dashboardState: DashboardState;
  updateDashboardState: (
    updater: DashboardState | ((prev: DashboardState) => DashboardState)
  ) => void;
}

const defaultSettings: AppState['settings'] = {
  language: 'en',
  recordingsFolder: 'E:\\VMS',
  screenshotsFolder: 'E:\\VMS\\Screenshots',
  hardwareAcceleration: 'Auto',
  analyticsProvider: 'Auto (GPU if available)',
  enableNotifications: true,
};

const AppStateContext = createContext<AppStateContextType | undefined>(undefined);

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

    const cameraAssignments = Array.isArray(data.cameraAssignments)
      ? (data.cameraAssignments as unknown[])
          .map((assignment: any) => ({
            cellIndex: typeof assignment?.cellIndex === 'number' ? assignment.cellIndex : 0,
            cameraId: typeof assignment?.cameraId === 'number' ? assignment.cameraId : null,
          }))
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

export const useAppState = () => {
  const context = useContext(AppStateContext);
  if (!context) {
    throw new Error('useAppState must be used within an AppStateProvider');
  }
  return context;
};

interface AppStateProviderProps {
  children: ReactNode;
}

export const AppStateProvider: React.FC<AppStateProviderProps> = ({ children }) => {
  const [cameras, setCamerasState] = useState<Camera[]>([]);
  const [groups, setGroupsState] = useState<CameraGroup[]>([]);
  const [settings, setSettingsState] = useState<AppState['settings']>(defaultSettings);
  const [isLoading, setIsLoading] = useState(true);
  const [dashboardState, setDashboardState] = useState<DashboardState>(createDefaultDashboardState());

  // Загрузка состояния приложения при инициализации
  const loadAppState = async () => {
    setIsLoading(true);
    try {
      console.log('AppStateContext: Loading app state...');
      
      // Загружаем камеры
      const loadedCameras = await invoke<Camera[]>('load_cameras');
      console.log('AppStateContext: Loaded cameras:', loadedCameras);
      setCamerasState(loadedCameras || []);
      
      // Заполняем глобальную переменную для совместимости с Dashboard
      (window as any).__VMS_CAMERAS = loadedCameras || [];
      console.log('AppStateContext: Set global __VMS_CAMERAS:', (window as any).__VMS_CAMERAS.length, 'cameras');
      
      // Загружаем группы
      const loadedGroups = await invoke<CameraGroup[]>('load_groups');
      console.log('AppStateContext: Loaded groups:', loadedGroups);
      setGroupsState(loadedGroups || []);
      
      // Загружаем настройки (пока из localStorage, потом можно добавить в Rust)
      const savedSettings = localStorage.getItem('app_settings');
      if (savedSettings) {
        const parsedSettings = JSON.parse(savedSettings);
        setSettingsState({ ...defaultSettings, ...parsedSettings });
        console.log('AppStateContext: Loaded settings:', parsedSettings);
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
  };

  // Сохранение состояния приложения
  const saveAppState = async () => {
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
      
      // Сохраняем настройки
      localStorage.setItem('app_settings', JSON.stringify(settings));
      console.log('AppStateContext: Settings saved successfully');

  localStorage.setItem('dashboard_state', JSON.stringify(dashboardState));
  console.log('AppStateContext: Dashboard state saved successfully');
      
      console.log('AppStateContext: App state saved successfully');
    } catch (error) {
      console.error('AppStateContext: Failed to save app state:', error);
    }
  };

  // Автоматическое сохранение при изменении состояния
  useEffect(() => {
    if (!isLoading) {
      console.log('AppStateContext: Auto-saving due to state change');
      void saveAppState();
    }
  }, [cameras, groups, settings, dashboardState, isLoading]);

  // Инициализация при монтировании
  useEffect(() => {
    loadAppState();
  }, []);

  // Обновляем глобальную переменную при изменении cameras
  useEffect(() => {
    (window as any).__VMS_CAMERAS = cameras;
  }, [cameras]);

  // Функции для работы с камерами
  const setCameras = (newCameras: Camera[]) => {
    setCamerasState(newCameras);
  };

  const addCamera = async (camera: Camera) => {
    console.log('AppStateContext: Adding camera:', camera);
    const newCameras = [...cameras, camera];
    setCamerasState(newCameras);
    (window as any).__VMS_CAMERAS = newCameras;
    console.log('AppStateContext: Camera added, new cameras count:', newCameras.length);
  };

  const updateCamera = async (updatedCamera: Camera) => {
    const newCameras = cameras.map(cam => 
      cam.id === updatedCamera.id ? updatedCamera : cam
    );
    setCamerasState(newCameras);
    (window as any).__VMS_CAMERAS = newCameras;
  };

  const removeCamera = async (cameraId: number) => {
    const newCameras = cameras.filter(cam => cam.id !== cameraId);
    setCamerasState(newCameras);
    (window as any).__VMS_CAMERAS = newCameras;
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
  };

  // Функции для работы с настройками
  const updateSettings = async (newSettings: Partial<AppState['settings']>) => {
    const updatedSettings = { ...settings, ...newSettings };
    setSettingsState(updatedSettings);
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

  // Устанавливаем глобальную переменную для совместимости с Dashboard
  useEffect(() => {
    (window as any).__VMS_CAMERAS = cameras;
  }, [cameras]);

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
    
    // State management
    isLoading,
    loadAppState,
    saveAppState,

    // Dashboard layout
    dashboardState,
    updateDashboardState,
  };

  return (
    <AppStateContext.Provider value={contextValue}>
      {children}
    </AppStateContext.Provider>
  );
};