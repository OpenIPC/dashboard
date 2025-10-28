import React, { useState, useRef, useEffect, useCallback } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { buildCameraRtspUrls } from '../utils/cameraStreams';
// appLocalDataDir removed; MediaMTX will be used for streaming
import { Box, Dialog, Typography } from '@mui/material';
import { useLocalization } from '../contexts/LocalizationContext';
import { useAppState } from '../contexts/AppStateContext';
import LayoutTabs from './LayoutTabs';
import LayoutTemplateDialog from './LayoutTemplateDialog';
import VideoStreamPlayer from './VideoStreamPlayer';
import CellControls from './CellControls';
import { useCameraContextMenu } from '../contexts/CameraContextMenuContext';
import type { CameraContextMenuHandlers } from '../contexts/CameraContextMenuContext';
import type {
  LayoutTemplate,
  LayoutTemplatePreview,
  LayoutCameraAssignment,
  Camera,
  LayoutTab,
  StreamQuality,
  StoredLayoutTemplate,
  StoredLayoutTemplatePreview,
  StoredLayoutTab,
} from '../types';
import { MAX_DASHBOARD_CELLS } from '../types';

// КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Глобальная инициализация аудио контекста
let globalAudioInitialized = false;

const initializeGlobalAudio = async (): Promise<void> => {
  if (globalAudioInitialized) return;
  
  try {
    if (typeof window !== 'undefined' && window.AudioContext) {
      const audioContext = new AudioContext();
      if (audioContext.state === 'suspended') {
        await audioContext.resume();
      }
      await audioContext.close();
      globalAudioInitialized = true;
      console.log('[GlobalAudio] ✓ Global audio initialized successfully');
    }
  } catch (error) {
    console.error('[GlobalAudio] Failed to initialize audio context:', error);
  }
};

// Инициализируем при загрузке модуля
if (typeof window !== 'undefined') {
  document.addEventListener('click', initializeGlobalAudio, { once: true });
  document.addEventListener('touchstart', initializeGlobalAudio, { once: true });
  document.addEventListener('keydown', initializeGlobalAudio, { once: true });
}

const MAX_CELLS = MAX_DASHBOARD_CELLS;
const GRID_PRESETS = [1, 4, 6, 8, 9, 12, 16, 20, 25, 32, 36, 49, 64].filter(size => size <= MAX_CELLS);

interface StreamInfo {
  cameraId: number;
  baseName: string;
  quality: StreamQuality;
}

interface StreamStatEntry {
  codec?: string;
  resolution?: string;
  bitrateKbps?: number;
  frameRate?: number;
  width?: number;
  height?: number;
}

const Dashboard: React.FC = () => {
  const { t } = useLocalization();
  const {
    cameras: appCameras,
    groups,
    isLoading: appStateLoading,
    dashboardState,
    updateDashboardState,
  } = useAppState();
  const {
    openCameraContextMenu,
    getDefaultCameraContextMenuHandlers,
  } = useCameraContextMenu();
  const [gridSize, setGridSize] = useState<number>(4);
  const [cellCameras, setCellCameras] = useState<(Camera | null)[]>(() => Array.from({ length: MAX_CELLS }, () => null));
  const [cellStreams, setCellStreams] = useState<(StreamInfo | null)[]>(() => Array.from({ length: MAX_CELLS }, () => null));
  const [hoveredCell, setHoveredCell] = useState<number | null>(null);
  const [fullscreenCell, setFullscreenCell] = useState<number | null>(null);
  
  // Новые состояния для управления ячейками
  const [cellPaused, setCellPaused] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => false));
  const [cellMuted, setCellMuted] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => true));
  const [cellRecording, setCellRecording] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => false));
  const [recordingPending, setRecordingPending] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => false));
  
  // Состояние для статистики потоков
  const [streamStats, setStreamStats] = useState<Record<string, StreamStatEntry>>({});
  const cellCamerasRef = useRef<(Camera | null)[]>(cellCameras);
  const hoveredCellRef = useRef<number | null>(hoveredCell);
  const restoringLayoutRef = useRef(false);
  const hasRestoredLayoutRef = useRef(false);


  useEffect(() => {
    cellCamerasRef.current = cellCameras;
  }, [cellCameras]);

  useEffect(() => {
    hoveredCellRef.current = hoveredCell;
  }, [hoveredCell]);

  const setCellQuality = (index: number, quality: StreamQuality) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }
    setCellStreams(prev => {
      const current = prev[index];
      if (!current || current.quality === quality) {
        return prev;
      }
      const updated = [...prev];
      updated[index] = { ...current, quality };
      return updated;
    });
  };

  const toggleCellQuality = (index: number) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }
    setCellStreams(prev => {
      const current = prev[index];
      if (!current) {
        return prev;
      }
      const nextQuality: StreamQuality = current.quality === 'sd' ? 'hd' : 'sd';
      const updated = [...prev];
      updated[index] = { ...current, quality: nextQuality };
      return updated;
    });
  };

  const toggleCellMuted = (index: number) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }
    setCellMuted(prev => {
      const updated = [...prev];
      updated[index] = !updated[index];
      return updated;
    });
  };

  const setCellRecordingState = (index: number, value: boolean) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }
    setCellRecording(prev => {
      const updated = [...prev];
      updated[index] = value;
      return updated;
    });
  };

  const setRecordingPendingState = (index: number, value: boolean) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }
    setRecordingPending(prev => {
      const updated = [...prev];
      updated[index] = value;
      return updated;
    });
  };

  const getHdStreamKey = (stream: StreamInfo) => `${stream.baseName}_0`;

  const toggleRecordingForCell = async (index: number) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }

    if (recordingPending[index]) {
      console.warn('Dashboard: Recording request already in progress for cell', index);
      return;
    }

    const camera = cellCameras[index];
    const stream = cellStreams[index];

    if (!camera || !stream) {
      console.warn('Dashboard: Cannot toggle recording without camera and stream info', { index, camera, stream });
      return;
    }

    const streamKey = getHdStreamKey(stream);
    setRecordingPendingState(index, true);

    try {
      if (!cellRecording[index]) {
  const { hdUrl } = await buildCameraRtspUrls(camera);
        await invoke('start_recording', {
          args: {
            cameraId: camera.id,
            cameraName: camera.name || `Camera ${camera.id}`,
            streamPath: streamKey,
            quality: 'hd',
            durationSeconds: 600,
            rtspUrl: hdUrl,
          },
        });
        setCellRecordingState(index, true);
        console.log('[Dashboard] Recording started', { streamKey, cameraId: camera.id, cellIndex: index });
      } else {
        await invoke('stop_recording', {
          args: {
            streamPath: streamKey,
          },
        });
        setCellRecordingState(index, false);
        console.log('[Dashboard] Recording stopped', { streamKey, cameraId: camera.id, cellIndex: index });
      }
    } catch (error) {
      console.error('Dashboard: Failed to toggle recording', { streamKey, error });
    } finally {
      setRecordingPendingState(index, false);
    }
  };

  const openCellArchive = (index: number, camera?: Camera | null) => {
    if (index < 0 || index >= MAX_CELLS || !camera) return;
    try {
      console.log(`Opening archive for camera ${camera.id} in cell ${index}`);
    } catch (error) {
      console.error('Failed to open archive:', error);
    }
  };

  const applyStreamStatsUpdate = useCallback((stream: StreamInfo | null, metrics: Partial<StreamStatEntry>) => {
    if (!stream) return;
    const statsKey = `${stream.baseName}_${stream.quality}`;

    setStreamStats(prev => {
      const prevEntry = prev[statsKey] ?? {};
      const nextEntry: StreamStatEntry = { ...prevEntry };
      let changed = false;

      if (metrics.codec && metrics.codec !== prevEntry.codec) {
        nextEntry.codec = metrics.codec;
        changed = true;
      }

      if (metrics.bitrateKbps !== undefined && metrics.bitrateKbps !== prevEntry.bitrateKbps) {
        nextEntry.bitrateKbps = metrics.bitrateKbps;
        changed = true;
      }

      if (metrics.frameRate !== undefined && metrics.frameRate !== prevEntry.frameRate) {
        nextEntry.frameRate = metrics.frameRate;
        changed = true;
      }

      if (metrics.width !== undefined && metrics.height !== undefined) {
        if (metrics.width !== prevEntry.width || metrics.height !== prevEntry.height) {
          nextEntry.width = metrics.width;
          nextEntry.height = metrics.height;
          nextEntry.resolution = `${metrics.width}x${metrics.height}`;
          changed = true;
        }
      } else if (metrics.resolution && metrics.resolution !== prevEntry.resolution) {
        nextEntry.resolution = metrics.resolution;
        changed = true;
      }

      if (!changed) {
        return prev;
      }

      return {
        ...prev,
        [statsKey]: nextEntry,
      };
    });
  }, []);

  const enterFullscreen = (index: number) => {
    if (!cellCameras[index]) {
      return;
    }
    setCellQuality(index, 'hd');
    setFullscreenCell(index);
  };

  const exitFullscreen = () => {
    setFullscreenCell(prevIndex => {
      if (prevIndex !== null) {
        setCellQuality(prevIndex, 'sd');
      }
      return null;
    });
  };

  // Layout template management
  const [savedTemplates, setSavedTemplates] = useState<LayoutTemplatePreview[]>([]);
  const [templateStore, setTemplateStore] = useState<Record<string, LayoutTemplate>>({});
  const [layoutsRestored, setLayoutsRestored] = useState(false);
  const [templateDialog, setTemplateDialog] = useState<{ open: boolean; layoutId: string | null; mode: 'create' | 'manage' }>({
    open: false,
    layoutId: null,
    mode: 'manage'
  });

  const areAssignmentsEqual = useCallback((left: LayoutCameraAssignment[], right: LayoutCameraAssignment[]) => {
    if (left.length !== right.length) {
      return false;
    }

    for (let index = 0; index < left.length; index += 1) {
      const lhs = left[index];
      const rhs = right[index];
      if (lhs.cellIndex !== rhs.cellIndex || lhs.cameraId !== rhs.cameraId) {
        return false;
      }
    }

    return true;
  }, []);

  const areStringArraysEqual = useCallback((left: string[], right: string[]) => {
    if (left.length !== right.length) {
      return false;
    }

    for (let index = 0; index < left.length; index += 1) {
      if (left[index] !== right[index]) {
        return false;
      }
    }

    return true;
  }, []);

  // Layout tabs management
  const [layoutTabs, setLayoutTabs] = useState<LayoutTab[]>([]);
  const [activeTabId, setActiveTabId] = useState<string | null>(null);


  const createEmptyCameraArray = () => Array.from({ length: MAX_CELLS }, () => null as Camera | null);
  const createEmptyStreamArray = () => Array.from({ length: MAX_CELLS }, () => null as StreamInfo | null);

  const assignCameraToCell = useCallback(async (camera: Camera, cellIndex: number) => {
    console.log('Dashboard: assigning camera to cell', { cameraId: camera.id, cellIndex });

    setCellStreams(prev => {
      const next = [...prev];
      next[cellIndex] = null;
      return next;
    });

    setCellCameras(prev => {
      const next = [...prev];
      next[cellIndex] = camera;
      return next;
    });

    try {
  const { hdUrl, sdUrl } = await buildCameraRtspUrls(camera);
      console.log('Dashboard: Registering camera streams', { cameraId: camera.id, hdUrl, sdUrl });
      await invoke('add_camera_streams', {
        cameraId: camera.id,
        hdUrl,
        sdUrl,
      });
      console.log('Dashboard: add_camera_streams completed for camera', camera.id);
    } catch (error) {
      console.error('Dashboard: Failed to configure MediaMTX for camera', camera.id, error);
      throw error;
    }

    try {
      await invoke('mediamtx_start');
    } catch (error) {
      console.warn('Dashboard: mediamtx_start failed or already running', error);
    }

    const baseName = `cam${camera.id}`;
    const sdPath = `${baseName}_1`;

    void (async () => {
      const ready = await waitForMediaMtxPath(sdPath, 12, 1000);
      if (!ready) {
        console.warn(`Dashboard: Stream path ${sdPath} not confirmed ready after polling`);
      }
    })();

    setCellStreams(prev => {
      const next = [...prev];
      const nextQuality: StreamQuality = fullscreenCell === cellIndex ? 'hd' : 'sd';
      next[cellIndex] = { cameraId: camera.id, baseName, quality: nextQuality };
      return next;
    });
  }, [fullscreenCell]);

  useEffect(() => {
    if (appStateLoading || hasRestoredLayoutRef.current) {
      return;
    }

    restoringLayoutRef.current = true;

    const { gridSize: savedGridSize, cellStates } = dashboardState;

    setGridSize(savedGridSize);
    setCellPaused(cellStates.map(cell => cell?.paused ?? false));
    setCellMuted(cellStates.map(cell => cell?.muted ?? true));
    setCellRecording(Array.from({ length: MAX_CELLS }, () => false));
    setRecordingPending(Array.from({ length: MAX_CELLS }, () => false));
  setCellCameras(createEmptyCameraArray());
  setCellStreams(createEmptyStreamArray());

    const restoreAssignments = async () => {
      for (let idx = 0; idx < Math.min(MAX_CELLS, cellStates.length); idx += 1) {
        const cell = cellStates[idx];
        if (!cell || cell.cameraId == null) {
          continue;
        }

        const camera = appCameras.find(c => c.id === cell.cameraId);
        if (!camera) {
          console.warn('Dashboard: Failed to restore camera, not found in state', cell.cameraId);
          continue;
        }

        try {
          await assignCameraToCell(camera, idx);
          setCellStreams(prev => {
            const next = [...prev];
            const current = next[idx];
            if (current) {
              next[idx] = { ...current, quality: cell.quality };
            }
            return next;
          });
        } catch (error) {
          console.error('Dashboard: Failed to restore camera assignment', {
            cameraId: cell.cameraId,
            index: idx,
            error,
          });
        }
      }
    };

    restoreAssignments().finally(() => {
      restoringLayoutRef.current = false;
      hasRestoredLayoutRef.current = true;
    });
  }, [appStateLoading, dashboardState, appCameras, assignCameraToCell]);

  useEffect(() => {
    if (appStateLoading || restoringLayoutRef.current) {
      return;
    }

    const cellStates = Array.from({ length: MAX_CELLS }, (_, idx) => {
      const cameraId = cellCameras[idx]?.id ?? null;
      const quality: StreamQuality = cellStreams[idx]?.quality ?? 'sd';
      const muted = cellMuted[idx];
      const paused = cellPaused[idx];
      return { cameraId, quality, muted, paused };
    });

    updateDashboardState(prev => ({
      ...prev,
      gridSize,
      cellStates,
    }));
  }, [gridSize, cellCameras, cellStreams, cellMuted, cellPaused, appStateLoading, updateDashboardState]);

  useEffect(() => {
    if (!layoutsRestored || restoringLayoutRef.current) {
      return;
    }

    if (!activeTabId) {
      return;
    }

    const activeTab = layoutTabs.find(tab => tab.id === activeTabId);
    if (!activeTab) {
      return;
    }

    const assignments = cellCameras.reduce<LayoutCameraAssignment[]>((acc, camera, index) => {
      if (camera) {
        acc.push({ cellIndex: index, cameraId: camera.id });
      }
      return acc;
    }, []);

    const sameGrid = activeTab.template.gridSize === gridSize;
    const sameAssignments = areAssignmentsEqual(assignments, activeTab.template.cameraAssignments);

    if (sameGrid && sameAssignments) {
      return;
    }

    const updatedTemplate: LayoutTemplate = {
      ...activeTab.template,
      gridSize,
      cameraAssignments: assignments,
      updatedAt: new Date(),
    };

    setLayoutTabs(prevTabs =>
      prevTabs.map(tab =>
        tab.id === activeTabId
          ? { ...tab, template: updatedTemplate }
          : tab
      )
    );

    setTemplateStore(prevStore => ({
      ...prevStore,
      [updatedTemplate.id]: updatedTemplate,
    }));

    setSavedTemplates(prevPreviews => {
      const index = prevPreviews.findIndex(preview => preview.id === updatedTemplate.id);
      if (index === -1) {
        return prevPreviews;
      }

      const assignedCameraNames = assignments
        .map(assignment => {
          const camera = cellCameras[assignment.cellIndex];
          return camera?.name ?? '';
        })
        .filter(Boolean)
        .slice(0, 4);

      const existingPreview = prevPreviews[index];
      if (
        existingPreview.gridSize === updatedTemplate.gridSize &&
        existingPreview.cameraCount === assignments.length &&
        areStringArraysEqual(existingPreview.previewCameras, assignedCameraNames)
      ) {
        return prevPreviews;
      }

      const nextPreviews = [...prevPreviews];
      nextPreviews[index] = {
        ...existingPreview,
        gridSize: updatedTemplate.gridSize,
        cameraCount: assignments.length,
        previewCameras: assignedCameraNames,
      };
      return nextPreviews;
    });
  }, [
    cellCameras,
    gridSize,
    layoutTabs,
    activeTabId,
    layoutsRestored,
    areAssignmentsEqual,
    areStringArraysEqual,
  ]);

  useEffect(() => {
    const handler = async (camera: Camera) => {
      const preferredCell = hoveredCellRef.current;
      let targetCell: number | null = null;

      if (typeof preferredCell === 'number' && preferredCell >= 0 && preferredCell < MAX_CELLS) {
        if (cellCamerasRef.current[preferredCell] === null) {
          targetCell = preferredCell;
        }
      }

      if (targetCell === null) {
        const freeIndex = cellCamerasRef.current.findIndex(c => c === null);
        targetCell = freeIndex >= 0 ? freeIndex : null;
      }

      if (targetCell === null) {
        console.warn('Dashboard: Нет свободных ячеек для камеры', camera.id);
        return;
      }

      try {
        await assignCameraToCell(camera, targetCell);
        if (preferredCell === null) {
          setHoveredCell(targetCell);
        }
      } catch (error) {
        console.error('Dashboard: Ошибка при добавлении камеры в ячейку через двойной клик', error);
      }
    };

    (window as any).setCellCamera = handler;

    return () => {
      if ((window as any).setCellCamera === handler) {
        delete (window as any).setCellCamera;
      }
    };
  }, [assignCameraToCell]);

  const generateDefaultLayoutName = useCallback(() => {
    const fallbackLayoutName = 'Раскладка';
    const localizedLayout = t('default_layout');
    const baseName = localizedLayout && localizedLayout !== 'default_layout' ? localizedLayout : fallbackLayoutName;
    const existingNames = new Set(layoutTabs.map(tab => tab.name.toLowerCase()));
    let index = layoutTabs.length + 1;
    let candidate = `${baseName} ${index}`;

    while (existingNames.has(candidate.toLowerCase())) {
      index += 1;
      candidate = `${baseName} ${index}`;
    }

    return candidate;
  }, [layoutTabs, t]);

  const createNewTab = useCallback((name?: string, template?: LayoutTemplate): LayoutTab => {
    const tabId = Date.now().toString();
    const fallbackLayoutName = 'Раскладка';
    const fallbackUntitledName = 'Безымянная раскладка';

    let defaultName: string;
    if (name) {
      defaultName = name;
    } else {
      const localizedLayout = t('default_layout');
      const layoutName = localizedLayout && localizedLayout !== 'default_layout' ? localizedLayout : fallbackLayoutName;
      defaultName = `${layoutName} ${layoutTabs.length + 1}`;
    }

    return {
      id: tabId,
      name: defaultName,
      template: template || {
        id: tabId,
        name: name || t('untitled_layout') || fallbackUntitledName,
        gridSize: 4,
        cameraAssignments: [],
        createdAt: new Date(),
        updatedAt: new Date()
      },
      isActive: false
    };
  }, [t, layoutTabs.length]);

  const handleTabChange = async (tabId: string) => {
    const tab = layoutTabs.find(t => t.id === tabId);
    if (!tab) return;


    if (activeTabId) {
      const previousActiveTab = layoutTabs.find(t => t.id === activeTabId);
      if (previousActiveTab) {
        const updatedTemplate: LayoutTemplate = {
          ...previousActiveTab.template,
          gridSize,
          cameraAssignments: cellCameras
            .map((camera, index) => ({
              cellIndex: index,
              cameraId: camera ? camera.id : null
            }))
            .filter((assignment): assignment is LayoutCameraAssignment => assignment.cameraId !== null),
          updatedAt: new Date()
        };

        setLayoutTabs(prev => prev.map(t => (
          t.id === activeTabId
            ? { ...t, template: updatedTemplate }
            : t
        )));

        setTemplateStore(prev => ({
          ...prev,
          [updatedTemplate.id]: updatedTemplate,
        }));
      }
    }

    setActiveTabId(tabId);
    setGridSize(tab.template.gridSize);

    setCellCameras(createEmptyCameraArray());
    setCellStreams(createEmptyStreamArray());

    setLayoutTabs(prev => prev.map(t => ({
      ...t,
      isActive: t.id === tabId
    })));

    for (const assignment of tab.template.cameraAssignments) {
      if (assignment.cameraId && assignment.cellIndex < MAX_CELLS) {
  const camera = appCameras.find(c => c && c.id === assignment.cameraId);
        if (camera) {
          await assignCameraToCell(camera, assignment.cellIndex);
        }
      }
    }
  };

  const handleTabClose = async (tabId: string) => {
    const remainingTabs = layoutTabs.filter(t => t.id !== tabId);
    
    if (remainingTabs.length === 0) {
      // Create default tab if no tabs remain
      const defaultTab = createNewTab(t('default_layout') || 'Layout');
      const activeDefaultTab = { ...defaultTab, isActive: true };
      setLayoutTabs([activeDefaultTab]);
      setActiveTabId(activeDefaultTab.id);
      setGridSize(4);
      setCellCameras(createEmptyCameraArray());
      setCellStreams(createEmptyStreamArray());
      setTemplateStore(prev => ({
        ...prev,
        [activeDefaultTab.template.id]: activeDefaultTab.template,
      }));
    } else {
      setLayoutTabs(remainingTabs);
      
      if (activeTabId === tabId) {
        // Switch to first remaining tab
        const newActiveTab = remainingTabs[0];
        setActiveTabId(newActiveTab.id);
        await handleTabChange(newActiveTab.id);
      }
    }
  };

  const openCreateLayoutDialog = () => {
    const defaultName = generateDefaultLayoutName();
    const newTab = createNewTab(defaultName);

    setTemplateStore(prev => ({
      ...prev,
      [newTab.template.id]: newTab.template,
    }));

    setLayoutTabs(prevTabs => {
      const deactivated = prevTabs.map(tab => ({ ...tab, isActive: false }));
      return [...deactivated, { ...newTab, isActive: true }];
    });

    setActiveTabId(newTab.id);
    setGridSize(newTab.template.gridSize);
    setCellCameras(createEmptyCameraArray());
    setCellStreams(createEmptyStreamArray());

    setTemplateDialog({
      open: true,
      layoutId: newTab.id,
      mode: 'create'
    });
  };

  const openManageLayoutDialog = () => {
    const targetId = activeTabId || layoutTabs[0]?.id || null;
    if (!targetId) {
      return;
    }

    setTemplateDialog({
      open: true,
      layoutId: targetId,
      mode: 'manage'
    });
  };

  const closeTemplateDialog = () => {
    setTemplateDialog(prev => ({
      ...prev,
      open: false
    }));
  };

  const handleTemplateDialogSave = (name: string) => {
    const layoutId = templateDialog.layoutId;
    if (!layoutId) {
      closeTemplateDialog();
      return;
    }

    handleRenameLayout(layoutId, name);
    closeTemplateDialog();
  };

  const handleGridPresetSelect = (layoutId: string, nextGridSize: number) => {
    setLayoutTabs(prevTabs => prevTabs.map(tab => {
      if (tab.id !== layoutId) {
        return tab;
      }

      const filteredAssignments = tab.template.cameraAssignments.filter(assignment => assignment.cellIndex < nextGridSize);
      const updatedTemplate: LayoutTemplate = {
        ...tab.template,
        gridSize: nextGridSize,
        cameraAssignments: filteredAssignments,
        updatedAt: new Date()
      };

      return {
        ...tab,
        template: updatedTemplate
      };
    }));

    setTemplateStore(prevStore => {
      const existing = prevStore[layoutId];
      if (!existing) {
        return prevStore;
      }

      const filteredAssignments = existing.cameraAssignments.filter(assignment => assignment.cellIndex < nextGridSize);

      return {
        ...prevStore,
        [layoutId]: {
          ...existing,
          gridSize: nextGridSize,
          cameraAssignments: filteredAssignments,
          updatedAt: new Date()
        }
      };
    });

    setSavedTemplates(prevPreviews => prevPreviews.map(preview => {
      if (preview.id !== layoutId) {
        return preview;
      }

      const limitedCameraCount = Math.min(preview.cameraCount, nextGridSize);

      return {
        ...preview,
        gridSize: nextGridSize,
        cameraCount: limitedCameraCount
      };
    }));

    if (layoutId === activeTabId) {
      setGridSize(nextGridSize);

      setCellCameras(prev => {
        const next = [...prev];
        for (let index = nextGridSize; index < MAX_CELLS; index += 1) {
          next[index] = null;
        }
        return next;
      });

      setCellStreams(prev => {
        const next = [...prev];
        for (let index = nextGridSize; index < MAX_CELLS; index += 1) {
          next[index] = null;
        }
        return next;
      });

      setCellPaused(prev => {
        const next = [...prev];
        for (let index = nextGridSize; index < MAX_CELLS; index += 1) {
          next[index] = false;
        }
        return next;
      });

      setCellMuted(prev => {
        const next = [...prev];
        for (let index = nextGridSize; index < MAX_CELLS; index += 1) {
          next[index] = true;
        }
        return next;
      });

      setCellRecording(prev => {
        const next = [...prev];
        for (let index = nextGridSize; index < MAX_CELLS; index += 1) {
          next[index] = false;
        }
        return next;
      });

      setRecordingPending(prev => {
        const next = [...prev];
        for (let index = nextGridSize; index < MAX_CELLS; index += 1) {
          next[index] = false;
        }
        return next;
      });
    }
  };

  const handleRenameLayout = (layoutId: string, nextName: string) => {
    const trimmedName = nextName.trim();
    if (!trimmedName) {
      return;
    }

    let templateToUpdate: LayoutTemplate | null = null;

    setLayoutTabs(prevTabs => prevTabs.map(tab => {
      if (tab.id !== layoutId) {
        return tab;
      }

      const updatedTemplate: LayoutTemplate = {
        ...tab.template,
        name: trimmedName,
        updatedAt: new Date()
      };

      templateToUpdate = updatedTemplate;

      return {
        ...tab,
        name: trimmedName,
        template: updatedTemplate
      };
    }));

    if (templateToUpdate) {
      setTemplateStore(prevStore => ({
        ...prevStore,
        [templateToUpdate!.id]: templateToUpdate!
      }));

      setSavedTemplates(prevPreviews => prevPreviews.map(preview => (
        preview.id === templateToUpdate!.id
          ? { ...preview, name: trimmedName }
          : preview
      )));
    }
  };

  // Restore layout templates and tabs from persisted dashboard state (with legacy fallback)
  useEffect(() => {
    if (layoutsRestored || appStateLoading) {
      return;
    }

    let previewsSource: StoredLayoutTemplatePreview[] = Array.isArray(dashboardState.savedTemplatePreviews)
      ? dashboardState.savedTemplatePreviews
      : [];
    let templatesSource: StoredLayoutTemplate[] = Array.isArray(dashboardState.layoutTemplates)
      ? dashboardState.layoutTemplates
      : [];
    const storedTabs: StoredLayoutTab[] = Array.isArray(dashboardState.layoutTabs)
      ? dashboardState.layoutTabs
      : [];
    const activeStoredId = typeof dashboardState.activeLayoutTabId === 'string'
      ? dashboardState.activeLayoutTabId
      : null;

    if (previewsSource.length === 0) {
      const legacyRaw = localStorage.getItem('dashboard_layout_templates');
      if (legacyRaw) {
        try {
          const legacyParsed = JSON.parse(legacyRaw) as StoredLayoutTemplatePreview[];
          if (Array.isArray(legacyParsed)) {
            previewsSource = legacyParsed;
          }
        } catch (error) {
          console.warn('Dashboard: Failed to parse legacy template previews', error);
        }
      }
    }

    if (templatesSource.length === 0 && previewsSource.length > 0) {
      const restoredTemplates: StoredLayoutTemplate[] = [];
      for (const preview of previewsSource) {
        try {
          const rawTemplate = localStorage.getItem(`dashboard_layout_${preview.id}`);
          if (!rawTemplate) continue;
          const parsedTemplate = JSON.parse(rawTemplate) as StoredLayoutTemplate;
          restoredTemplates.push(parsedTemplate);
        } catch (error) {
          console.warn('Dashboard: Failed to parse legacy template', preview.id, error);
        }
      }
      if (restoredTemplates.length > 0) {
        templatesSource = restoredTemplates;
      }
    }

    const templateMap: Record<string, LayoutTemplate> = {};
    templatesSource.forEach(template => {
      const runtimeTemplate: LayoutTemplate = {
        ...template,
        cameraAssignments: Array.isArray(template.cameraAssignments)
          ? template.cameraAssignments.map(assignment => ({
              cellIndex: typeof assignment.cellIndex === 'number' ? assignment.cellIndex : 0,
              cameraId: typeof assignment.cameraId === 'number' ? assignment.cameraId : null,
            }))
          : [],
        createdAt: template.createdAt ? new Date(template.createdAt) : new Date(),
        updatedAt: template.updatedAt ? new Date(template.updatedAt) : new Date(),
      };
      templateMap[runtimeTemplate.id] = runtimeTemplate;
    });

    const restoredPreviews = previewsSource.map(preview => ({
      ...preview,
      previewCameras: Array.isArray(preview.previewCameras) ? preview.previewCameras : [],
      createdAt: preview.createdAt ? new Date(preview.createdAt) : new Date(),
    }));

    const resolvedTabs: LayoutTab[] = storedTabs
      .map(tab => {
        const template = templateMap[tab.templateId];
        if (!template) {
          return null;
        }
        return {
          id: tab.id,
          name: tab.name,
          template,
          isActive: false,
        };
      })
      .filter((tab): tab is LayoutTab => Boolean(tab));

    setSavedTemplates(restoredPreviews);
    setTemplateStore(templateMap);

    if (resolvedTabs.length > 0) {
      const hasStoredActive = activeStoredId && resolvedTabs.some(tab => tab.id === activeStoredId);
      const resolvedActiveId = hasStoredActive ? activeStoredId! : resolvedTabs[0].id;
      const updatedTabs = resolvedTabs.map(tab => ({
        ...tab,
        isActive: tab.id === resolvedActiveId,
      }));
      setLayoutTabs(updatedTabs);
      setActiveTabId(resolvedActiveId);
    }

    setLayoutsRestored(true);
  }, [layoutsRestored, appStateLoading, dashboardState]);

  useEffect(() => {
    if (appStateLoading || !layoutsRestored) {
      return;
    }

    const serializedPreviews = savedTemplates.map(preview => ({
      ...preview,
      createdAt: preview.createdAt instanceof Date
        ? preview.createdAt.toISOString()
        : new Date(preview.createdAt).toISOString(),
    }));

    const serializedTemplates = Object.values(templateStore).map(template => ({
      ...template,
      cameraAssignments: template.cameraAssignments.map(assignment => ({
        cellIndex: assignment.cellIndex,
        cameraId: typeof assignment.cameraId === 'number' ? assignment.cameraId : null,
      })),
      createdAt: template.createdAt.toISOString(),
      updatedAt: template.updatedAt.toISOString(),
    }));

    const serializedTabs = layoutTabs.map(tab => ({
      id: tab.id,
      name: tab.name,
      templateId: tab.template.id,
    }));

    updateDashboardState(prev => ({
      ...prev,
      savedTemplatePreviews: serializedPreviews,
      layoutTemplates: serializedTemplates,
      layoutTabs: serializedTabs,
      activeLayoutTabId: activeTabId,
    }));
  }, [savedTemplates, templateStore, layoutTabs, activeTabId, appStateLoading, layoutsRestored, updateDashboardState]);

  // Initialize default tab
  useEffect(() => {
    if (!layoutsRestored) {
      return;
    }

    if (layoutTabs.length === 0) {
      // Используем setTimeout чтобы дать время контексту локализации инициализироваться
      const timer = setTimeout(() => {
        const defaultTabName = t('default_layout') || 'Раскладка';
        const defaultTab = createNewTab(defaultTabName);
        // Делаем дефолтную вкладку активной
        const activeDefaultTab = { ...defaultTab, isActive: true };
        setLayoutTabs([activeDefaultTab]);
        setActiveTabId(defaultTab.id);
        setTemplateStore(prev => ({
          ...prev,
          [activeDefaultTab.template.id]: activeDefaultTab.template,
        }));
      }, 100);
      
      return () => clearTimeout(timer);
    }
  }, [createNewTab, t, layoutTabs.length, layoutsRestored]);

  // Update tab names when localization is ready
  useEffect(() => {
    if (layoutTabs.length > 0 && t && layoutTabs[0].name.includes('[default_layout]')) {
      setLayoutTabs(prev => prev.map((tab, index) => {
        if (index === 0 && tab.name.includes('[default_layout]')) {
          return {
            ...tab,
            name: t('default_layout') || 'Раскладка'
          };
        }
        return tab;
      }));
    }
  }, [t, layoutTabs]);

  // Логируем инициализацию
  React.useEffect(() => {
    console.log('Dashboard: Component initialized');
    console.log('Dashboard: Cameras available:', appCameras.length);
  }, [appCameras.length]);

  // React.useEffect(() => {
  //   const loadWhepBases = async () => {
  //     try {
  //       const bases = await invoke<string[]>('get_whep_endpoints');
  //       if (Array.isArray(bases) && bases.length > 0) {
  //         const normalized = Array.from(new Set(
  //           bases.map(base => base.trim().replace(/\/$/, ''))
  //         ));
  //         console.log('Dashboard: Loaded WHEP base endpoints:', normalized);
  //         setWhepBases(normalized);
  //       } else {
  //         console.warn('Dashboard: get_whep_endpoints returned empty list, using defaults');
  //         setWhepBases([...DEFAULT_WHEP_BASES]);
  //       }
  //     } catch (error) {
  // console.warn('Dashboard: Failed to load WHEP endpoints, using defaults', error);
  // setWhepBases([...DEFAULT_WHEP_BASES]);
  //     }
  //   };

  //   loadWhepBases();
  // }, []);

  // вычисляем оптимальную сетку с учетом соотношения сторон
  const calculateGridDimensions = (cellCount: number) => {
    if (cellCount === 1) return { cols: 1, rows: 1 };
    if (cellCount === 4) return { cols: 2, rows: 2 };
    if (cellCount === 9) return { cols: 3, rows: 3 };
    if (cellCount === 16) return { cols: 4, rows: 4 };
    if (cellCount === 25) return { cols: 5, rows: 5 };
    if (cellCount === 32) return { cols: 8, rows: 4 }; // 8x4 для лучшего соотношения
    if (cellCount === 64) return { cols: 8, rows: 8 };
    
    // Для других значений вычисляем оптимальное соотношение
    const sqrt = Math.sqrt(cellCount);
    const cols = Math.ceil(sqrt);
    const rows = Math.ceil(cellCount / cols);
    return { cols, rows };
  };

  const { cols, rows } = calculateGridDimensions(gridSize);
  const gridCells = Array.from({ length: cols * rows }).map((_, idx) => {
    const cam = cellCameras[idx];
    const streamInfo = cellStreams[idx];

    const handleDrop = async (e: React.DragEvent) => {
      e.preventDefault();
      e.stopPropagation();
      const cameraIdStr = e.dataTransfer.getData('application/x-camera-id');
      if (!cameraIdStr) return;

      const cameraId = Number.parseInt(cameraIdStr, 10);
      if (Number.isNaN(cameraId)) return;

      const targetCamera = appCameras.find(c => c && c.id === cameraId);

      if (!targetCamera) {
        console.error('Dashboard: Camera not found in state:', cameraId);
        return;
      }

      try {
        await assignCameraToCell(targetCamera, idx);
      } catch (error) {
        console.error(`Dashboard: Failed to assign camera ${cameraId} to cell ${idx}`, error);
      }
    };

    const handleClearCell = async () => {
      if (cellRecording[idx] && streamInfo) {
        await toggleRecordingForCell(idx);
      }

      if (fullscreenCell === idx) {
        exitFullscreen();
      }

      setCellCameras(prev => {
        const copy = [...prev];
        copy[idx] = null;
        return copy;
      });
      setCellStreams(prev => {
        const copy = [...prev];
        copy[idx] = null;
        return copy;
      });
      setCellPaused(prev => {
        const copy = [...prev];
        copy[idx] = false;
        return copy;
      });
      setCellRecordingState(idx, false);
      setRecordingPendingState(idx, false);
    };

    // Функции обработки событий для контроллов ячеек
    const handleCellStreamSwitch = () => {
      toggleCellQuality(idx);
    };

    const handleCellAudio = () => {
      toggleCellMuted(idx);
    };

    const handleCellRecord = () => {
      void toggleRecordingForCell(idx);
    };

    const handleCellArchive = () => {
      openCellArchive(idx, cam);
    };

    return (
      <Box
        key={`cell-${idx}`}
        className={`grid-cell ${cellPaused[idx] ? 'paused-state' : ''} ${cellRecording[idx] ? 'recording' : ''} ${fullscreenCell === idx ? 'fullscreen' : ''}`}
        tabIndex={0}
        sx={{
          position: 'relative',
          overflow: 'hidden',
          width: '100%',
          height: '100%',
          minHeight: gridSize > 32 ? 60 : gridSize > 16 ? 120 : gridSize > 9 ? 150 : 180,
          maxHeight: gridSize === 1 ? '100%' : gridSize === 4 ? 400 : gridSize === 9 ? 300 : gridSize <= 25 ? 200 : 150,
          border: '1px solid #31353a',
          borderRadius: gridSize > 32 ? 1 : 2,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          backgroundColor: '#36393f',
          cursor: 'move',
          aspectRatio: '16/9',
          '&.drag-over': { boxShadow: '0 0 0 2px #1976d2' }
        }}
        onMouseEnter={() => setHoveredCell(idx)}
        onMouseLeave={() => setHoveredCell(null)}
        onDragEnter={e => { e.preventDefault(); e.currentTarget.classList.add('drag-over'); }}
        onDragOver={e => { e.preventDefault(); e.stopPropagation(); e.dataTransfer.dropEffect = 'move'; }}
        onDragLeave={e => { e.currentTarget.classList.remove('drag-over'); }}
        onDrop={e => { e.currentTarget.classList.remove('drag-over'); handleDrop(e); }}
        onDoubleClick={() => {
          if (cellCameras[idx]) {
            enterFullscreen(idx);
          }
        }}
        onContextMenu={(event) => {
          if (!cam) {
            return;
          }

          event.preventDefault();
          event.stopPropagation();

          const defaultHandlers: CameraContextMenuHandlers = getDefaultCameraContextMenuHandlers();

          openCameraContextMenu({
            camera: cam,
            anchorPosition: { left: event.clientX, top: event.clientY },
            handlers: {
              onArchive: (camera) => {
                openCellArchive(idx, camera);
                defaultHandlers.onArchive?.(camera);
              },
            },
            groups,
          });
        }}
      >
        {cam ? (
          <>
            {streamInfo ? (
              <>
                <VideoStreamPlayer
                  key={`${streamInfo.baseName}-cell-${idx}-${streamInfo.quality}`}
                  streamName={streamInfo.baseName}
                  useHdQuality={streamInfo.quality === 'hd'}
                  controls={false}
                  autoPlay
                  muted={cellMuted[idx]}
                  width="100%"
                  height="100%"
                  style={{ borderRadius: 2 }}
                  onStatsUpdate={(stats) => applyStreamStatsUpdate(streamInfo, stats)}
                />
                {streamInfo.quality !== 'hd' && (
                  <VideoStreamPlayer
                    key={`${streamInfo.baseName}-cell-${idx}-hd-preload`}
                    streamName={streamInfo.baseName}
                    useHdQuality
                    controls={false}
                    autoPlay
                    muted
                    width={1}
                    height={1}
                    className="hd-preload-player"
                    style={{
                      position: 'absolute',
                      top: 0,
                      left: 0,
                      width: 1,
                      height: 1,
                      opacity: 0,
                      pointerEvents: 'none',
                    }}
                  />
                )}
                {/* Название камеры - правый нижний угол */}
                <div 
                  className="cell-name"
                  style={{ 
                    position: 'absolute', 
                    bottom: 8, 
                    right: 8, 
                    backgroundColor: 'rgba(0, 0, 0, 0.7)', 
                    color: 'white', 
                    padding: '4px 8px', 
                    borderRadius: 4, 
                    fontSize: 12, 
                    fontWeight: 500,
                    opacity: 1,
                    transition: 'opacity 0.3s ease-in-out',
                    zIndex: 10,
                    whiteSpace: 'nowrap',
                    maxWidth: 'fit-content'
                  }}
                >
                  {`${cam.name || `Camera ${cam.id}`}`} · {streamInfo.quality.toUpperCase()}
                </div>
                {/* Статистика потока - левый нижний угол */}
                <div 
                  className="cell-stats"
                  style={{ 
                    position: 'absolute', 
                    bottom: 8, 
                    left: 8, 
                    backgroundColor: 'rgba(0, 0, 0, 0.7)', 
                    color: 'white', 
                    padding: '2px 6px', 
                    borderRadius: 3, 
                    fontSize: 10, 
                    fontFamily: 'monospace',
                    opacity: 1,
                    transition: 'opacity 0.3s ease-in-out',
                    zIndex: 10,
                    whiteSpace: 'nowrap',
                    maxWidth: 'fit-content'
                  }}
                >
                  {(() => {
                    const statsKey = `${streamInfo.baseName}_${streamInfo.quality}`;
                    const stats = streamStats[statsKey];
                    if (stats) {
                      const segments: string[] = [];
                      if (stats.codec) {
                        segments.push(stats.codec);
                      }
                      if (stats.resolution) {
                        segments.push(stats.resolution);
                      } else if (stats.width && stats.height) {
                        segments.push(`${stats.width}x${stats.height}`);
                      }
                      if (stats.bitrateKbps !== undefined) {
                        segments.push(`${stats.bitrateKbps} kbps`);
                      }
                      if (stats.frameRate !== undefined) {
                        segments.push(`${stats.frameRate} fps`);
                      }

                      if (segments.length > 0) {
                        return segments.join(' | ');
                      }
                    }
                    return `${streamInfo.quality.toUpperCase()} | ${t('connecting_stream') || 'Подключение...'}`;
                  })()}
                </div>
              </>
            ) : (
              <Box sx={{ display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', color: '#bfc8d6' }}>
                <Typography variant="body2" sx={{ color: '#bfc8d6', fontSize: 12 }}>
                  {t('connecting_stream') || 'Подключение...'}
                </Typography>
              </Box>
            )}
            {/* Контроллы ячейки - всегда присутствуют, но скрыты через CSS */}
            <CellControls
              isFullscreen={fullscreenCell === idx}
              isRecording={cellRecording[idx]}
              isRecordingPending={recordingPending[idx]}
              isMuted={cellMuted[idx]}
              streamId={streamInfo?.quality === 'hd' ? 0 : 1}
              onStreamSwitch={handleCellStreamSwitch}
              onAudio={handleCellAudio}
              onRecord={handleCellRecord}
              onClose={() => { void handleClearCell(); }}
              onArchive={handleCellArchive}
            />
          </>
        ) : (
          <Box sx={{ 
            display: 'flex', 
            alignItems: 'center', 
            justifyContent: 'center', 
            color: '#bfc8d6',
            opacity: gridSize > 32 ? 0.6 : gridSize > 16 ? 0.8 : 1
          }}>
            <svg 
              width={gridSize > 32 ? "24" : gridSize > 16 ? "32" : "48"} 
              height={gridSize > 32 ? "24" : gridSize > 16 ? "32" : "48"} 
              viewBox="0 0 48 48" 
              fill="none" 
              xmlns="http://www.w3.org/2000/svg"
            >
              <rect x="8" y="12" width="32" height="24" rx="4" fill="#bfc8d6" fillOpacity="0.12" />
              <path d="M32 20C32 22.2091 30.2091 24 28 24C25.7909 24 24 22.2091 24 20C24 17.7909 25.7909 16 28 16C30.2091 16 32 17.7909 32 20Z" fill="#bfc8d6" fillOpacity="0.32" />
              <rect x="34" y="14" width="6" height="6" rx="2" fill="#bfc8d6" />
            </svg>
          </Box>
        )}
      </Box>
    );
  });

  const fullscreenIndex = fullscreenCell ?? -1;
  const fullscreenCamera = fullscreenIndex >= 0 ? cellCameras[fullscreenIndex] : null;
  const fullscreenStream = fullscreenIndex >= 0 ? cellStreams[fullscreenIndex] : null;
  const fullscreenMuted = fullscreenIndex >= 0 ? cellMuted[fullscreenIndex] : true;
  const fullscreenRecording = fullscreenIndex >= 0 ? cellRecording[fullscreenIndex] : false;
  const fullscreenRecordingPending = fullscreenIndex >= 0 ? recordingPending[fullscreenIndex] : false;
  const fullscreenStatsKey = fullscreenStream ? `${fullscreenStream.baseName}_${fullscreenStream.quality}` : null;
  const fullscreenStats = fullscreenStatsKey ? streamStats[fullscreenStatsKey] : undefined;

  const dialogLayout = templateDialog.layoutId
    ? layoutTabs.find(tab => tab.id === templateDialog.layoutId) ?? null
    : null;
  const isTemplateDialogOpen = templateDialog.open && !!dialogLayout;
  const dialogGridSize = dialogLayout?.template.gridSize ?? gridSize;
  const dialogLayoutName = dialogLayout?.name ?? generateDefaultLayoutName();

  // Polling-функция для ожидания появления потока camera_{idx} в MediaMTX
  async function waitForMediaMtxPath(pathName: string, maxAttempts = 5, intervalMs = 1000): Promise<boolean> {
    for (let attempt = 0; attempt < maxAttempts; attempt++) {
      try {
        // Используем Tauri команду вместо прямого fetch для избежания CORS
        const isReady = await invoke('check_mediamtx_path_ready', { pathName });
        console.log(`MediaMTX path ${pathName} readiness check (attempt ${attempt + 1}):`, isReady);
        
        if (isReady) {
          console.log(`MediaMTX: path ${pathName} ready (attempt ${attempt + 1})`);
          return true;
        }
      } catch (err) {
        console.warn(`MediaMTX polling error (attempt ${attempt + 1}):`, err);
      }
      await new Promise(r => setTimeout(r, intervalMs));
    }
    console.warn(`MediaMTX: path ${pathName} not ready after ${maxAttempts} attempts`);
    return false;
  }

  return (
    <Box sx={{ height: '100%', display: 'flex', flexDirection: 'column', bgcolor: '#23272b' }}>
      {/* Layout Tabs */}
      <LayoutTabs
        tabs={layoutTabs}
        activeTabId={activeTabId}
  onTabChange={(id) => { void handleTabChange(id); }}
  onTabClose={(id) => { void handleTabClose(id); }}
  onNewTab={openCreateLayoutDialog}
  onManageLayouts={openManageLayoutDialog}
      />
      
      {/* Camera Grid */}
      <Box sx={{ 
        flex: 1, 
        p: 0, 
        display: 'flex', 
        flexDirection: 'column', 
        bgcolor: '#23272b', 
        minHeight: 0,
        overflow: 'hidden', // Предотвращаем переполнение
        position: 'relative'
      }}>
        <Box sx={{ 
          flex: 1, 
          display: 'grid', 
          gridTemplateColumns: `repeat(${cols}, 1fr)`, 
          gridTemplateRows: `repeat(${rows}, 1fr)`, 
          gap: gridSize > 25 ? 1 : 2, // Меньшие отступы для больших сеток
          p: 0, 
          minHeight: 0,
          width: '100%',
          height: '100%',
          '& > div': {
            minWidth: gridSize > 32 ? '60px' : gridSize > 16 ? '80px' : '120px',
            minHeight: gridSize > 32 ? '45px' : gridSize > 16 ? '60px' : '90px',
            maxHeight: 'none'
          }
        }}>
          {gridCells}
        </Box>
      </Box>

      <LayoutTemplateDialog
        open={isTemplateDialogOpen}
        mode={templateDialog.mode}
        layoutName={dialogLayoutName}
        gridSize={dialogGridSize}
        availableGridSizes={GRID_PRESETS}
        onClose={closeTemplateDialog}
        onSave={handleTemplateDialogSave}
        onGridSelect={(size: number) => {
          if (templateDialog.layoutId) {
            handleGridPresetSelect(templateDialog.layoutId, size);
          }
        }}
      />

      <Dialog
        open={fullscreenCell !== null}
        onClose={exitFullscreen}
        fullScreen
        PaperProps={{
          sx: {
            backgroundColor: '#000',
          }
        }}
      >
        <Box sx={{ position: 'relative', width: '100vw', height: '100vh', backgroundColor: '#000', display: 'flex', flexDirection: 'column' }}>
          {fullscreenCamera && fullscreenStream ? (
            <>
              <Box
                className="fullscreen-dialog"
                sx={{ flex: 1, position: 'relative', display: 'flex', alignItems: 'center', justifyContent: 'center', padding: { xs: 1, md: 3 }, cursor: 'zoom-out' }}
                onDoubleClick={(event) => {
                  const target = event.target as HTMLElement | null;
                  if (target?.closest('.cell-controls')) {
                    return;
                  }
                  exitFullscreen();
                }}
              >
                <VideoStreamPlayer
                  key={`fullscreen-${fullscreenStream.baseName}-${fullscreenStream.quality}`}
                  streamName={fullscreenStream.baseName}
                  useHdQuality={fullscreenStream.quality === 'hd'}
                  controls={false}
                  autoPlay
                  muted={fullscreenMuted}
                  width="100%"
                  height="100%"
                  objectFit="contain"
                  style={{ maxWidth: '100%', maxHeight: '100%' }}
                  onStatsUpdate={(stats) => applyStreamStatsUpdate(fullscreenStream, stats)}
                />
                <div
                  className="cell-name"
                  style={{
                    opacity: 1,
                    bottom: 32,
                    right: 32,
                    fontSize: 18,
                    padding: '8px 16px',
                    borderRadius: 8,
                    backgroundColor: 'rgba(0, 0, 0, 0.75)'
                  }}
                >
                  {`${fullscreenCamera.name || `Camera ${fullscreenCamera.id}`}`} · {fullscreenStream.quality.toUpperCase()}
                </div>
                <div
                  className="cell-stats"
                  style={{
                    opacity: 1,
                    bottom: 32,
                    left: 32,
                    fontSize: 14,
                    padding: '6px 12px',
                    borderRadius: 8,
                    backgroundColor: 'rgba(0, 0, 0, 0.75)'
                  }}
                >
                  {fullscreenStats ? (() => {
                    const segments: string[] = [];
                    if (fullscreenStats.codec) {
                      segments.push(fullscreenStats.codec);
                    }
                    if (fullscreenStats.resolution) {
                      segments.push(fullscreenStats.resolution);
                    } else if (fullscreenStats.width && fullscreenStats.height) {
                      segments.push(`${fullscreenStats.width}x${fullscreenStats.height}`);
                    }
                    if (fullscreenStats.bitrateKbps !== undefined) {
                      segments.push(`${fullscreenStats.bitrateKbps} kbps`);
                    }
                    if (fullscreenStats.frameRate !== undefined) {
                      segments.push(`${fullscreenStats.frameRate} fps`);
                    }
                    return segments.length > 0
                      ? segments.join(' | ')
                      : `${fullscreenStream.quality.toUpperCase()} | ${t('connecting_stream') || 'Подключение...'}`;
                  })()
                    : `${fullscreenStream.quality.toUpperCase()} | ${t('connecting_stream') || 'Подключение...'}`}
                </div>
                <CellControls
                  isFullscreen
                  isRecording={fullscreenRecording}
                  isRecordingPending={fullscreenRecordingPending}
                  isMuted={fullscreenMuted}
                  streamId={fullscreenStream.quality === 'hd' ? 0 : 1}
                  onStreamSwitch={() => toggleCellQuality(fullscreenIndex)}
                  onAudio={() => toggleCellMuted(fullscreenIndex)}
                  onRecord={() => { void toggleRecordingForCell(fullscreenIndex); }}
                  onClose={() => {}}
                  onArchive={() => openCellArchive(fullscreenIndex, fullscreenCamera)}
                />
              </Box>
            </>
          ) : (
            <Box sx={{ flex: 1, display: 'flex', alignItems: 'center', justifyContent: 'center', color: '#fff' }}>
              <Typography variant="h6">{t('no_stream_selected') || 'Нет доступного HD-потока'}</Typography>
            </Box>
          )}
        </Box>
      </Dialog>
    </Box>
  );
};

export default Dashboard;