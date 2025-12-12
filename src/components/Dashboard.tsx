import React, { useState, useRef, useEffect, useCallback, useMemo } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { buildCameraRtspUrls } from '../utils/cameraStreams';
// appLocalDataDir removed; go2rtc handles all streaming duties now
import { Box, Typography, IconButton } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import { useLocalization } from '../hooks/useLocalization';
import { useAppState } from '../hooks/useAppState';
import LayoutTabs from './LayoutTabs';
import LayoutTemplateDialog from './LayoutTemplateDialog';
import GridCell from './GridCell';
import CellControls from './CellControls';
import PTZControls from './PTZControls';
import { useCameraContextMenu } from '../hooks/useCameraContextMenu';
import { useAnalytics } from '../hooks/useAnalytics';
import { useToast } from '../hooks/useToast';
import { Toast } from './Toast';
import { streamPrewarmingService } from '../services/streamPrewarming';
import type { CameraContextMenuHandlers } from '../contexts/CameraContextMenuContextData';
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
import type {
  AnalyticsModuleStatus,
  AnalyticsDetectionResponse,
  AnalyticsDetectionBox,
} from '../services/analytics';
import type { CameraStatusEntry } from '../contexts/AppStateContextData';

// КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Глобальная инициализация аудио контекста
let globalAudioInitialized = false;

const initializeGlobalAudio = async (): Promise<void> => {
  if (globalAudioInitialized) {
    return;
  }

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
const ANALYSIS_WARMUP_RETRY_MS = 1000;
const CAMERA_STATUS_OFFLINE_TIMEOUT_MS = 8000;
const DEFAULT_CELL_VOLUME = 0.75;
const MIN_AUDIBLE_VOLUME = 0.01;

const createInitialVolumeArray = () => Array.from({ length: MAX_CELLS }, () => DEFAULT_CELL_VOLUME);
const clampVolume = (value?: number | null): number => {
  if (typeof value !== 'number' || Number.isNaN(value)) {
    return DEFAULT_CELL_VOLUME;
  }
  return Math.min(1, Math.max(0, value));
};

const MODULE_ICON_MAP: Record<string, string> = {
  'face-detector': 'tag_faces',
  'object-counter': 'analytics',
  'person-detector': 'directions_walk',
  'vehicle-detector': 'directions_car',
  'license-plate-detector': 'local_parking',
};

const resolveModuleIcon = (moduleId: string): string => {
  return MODULE_ICON_MAP[moduleId] ?? 'sensors';
};

type VideoRefScope = 'grid' | 'fullscreen' | 'hd';

interface VideoElementEntry {
  grid: HTMLVideoElement | null;
  fullscreen: HTMLVideoElement | null;
  hd: HTMLVideoElement | null;
}

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
  lastUpdated?: number;
}

interface CaptureContext {
  module: AnalyticsModuleStatus;
  camera: Camera;
  video: HTMLVideoElement;
  index: number;
}

interface CellDetectionState {
  cameraId: string;
  moduleId: string;
  detections: AnalyticsDetectionBox[];
  processedAt: string;
  frameWidth: number;
  frameHeight: number;
}

interface ProcessedFrameResult {
  response: AnalyticsDetectionResponse;
  frameWidth: number;
  frameHeight: number;
}

const Dashboard: React.FC = () => {
  const { t } = useLocalization();
  const {
    cameras: appCameras,
    groups,
    isLoading: appStateLoading,
    dashboardState,
    updateDashboardState,
    streamingProvider,
    ensureStreamingBackendStarted,
    updateCameraStatus,
    settings,
  } = useAppState();
  const {
    openCameraContextMenu,
    getDefaultCameraContextMenuHandlers,
  } = useCameraContextMenu();

  // Load go2rtc enhanced settings from localStorage
  const [go2rtcSettings, setGo2rtcSettings] = useState({
    showMonitor: false,
    enableSnapshot: true,
    enable2WayAudio: false,
    enableAdaptiveBitrate: true,
  });

  // Load stream optimization settings
  const [streamOptSettings, setStreamOptSettings] = useState({
    enableFastStart: true,
    enablePrewarming: true,
    prewarmBothQualities: true,
    enableConnectionCaching: true,
    maxCachedConnections: 10,
    keepAliveInterval: 30,
  });

  useEffect(() => {
    const saved = localStorage.getItem('go2rtcSettings');
    if (saved) {
      try {
        setGo2rtcSettings(JSON.parse(saved));
      } catch (e) {
        console.error('Failed to load go2rtc settings:', e);
      }
    }

    const optSaved = localStorage.getItem('streamOptimizationSettings');
    if (optSaved) {
      try {
        const parsed = JSON.parse(optSaved);
        setStreamOptSettings(parsed);
        
        // Apply settings to prewarming service
        streamPrewarmingService.saveConfig({
          enabled: parsed.enablePrewarming,
          prewarmBothQualities: parsed.prewarmBothQualities,
          keepAliveInterval: parsed.keepAliveInterval * 1000, // Convert to ms
          maxConcurrentPrewarms: parsed.maxCachedConnections,
        });
      } catch (e) {
        console.error('Failed to load stream optimization settings:', e);
      }
    }
  }, []);

  const [gridSize, setGridSize] = useState<number>(4);
  const [cellCameras, setCellCameras] = useState<(Camera | null)[]>(() => Array.from({ length: MAX_CELLS }, () => null));
  const [cellStreams, setCellStreams] = useState<(StreamInfo | null)[]>(() => Array.from({ length: MAX_CELLS }, () => null));
  const cellStreamsRef = useRef<(StreamInfo | null)[]>(cellStreams); // Ref для избежания зависимости в useMemo
  const [hoveredCell, setHoveredCellState] = useState<number | null>(null);
  const [fullscreenCell, setFullscreenCell] = useState<number | null>(null);
  
  // Новые состояния для управления ячейками
  const [cellPaused, setCellPaused] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => false));
  const [cellMuted, setCellMuted] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => true));
  const [cellVolume, setCellVolume] = useState<number[]>(createInitialVolumeArray);
  const [cellRecording, setCellRecording] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => false));
  const [recordingPending, setRecordingPending] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => false));
  const [cellPTZActive, setCellPTZActive] = useState<boolean[]>(() => Array.from({ length: MAX_CELLS }, () => false));
  
  // Состояние для статистики потоков
  const streamStatsRef = React.useRef<Record<string, StreamStatEntry>>({});
  const deriveCameraStatus = useCallback(
    (cameraId: number): CameraStatusEntry => {
      const baseName = `cam${cameraId}`;
      const statKeys = [`${baseName}_0`, `${baseName}_1`];
      const entries = statKeys
        .map(key => streamStatsRef.current[key])
        .filter((entry): entry is StreamStatEntry => Boolean(entry))
        .sort((a, b) => (b.lastUpdated ?? 0) - (a.lastUpdated ?? 0));

      const latest = entries[0];
      const lastUpdated = latest?.lastUpdated ?? null;
      const now = Date.now();

      if (!lastUpdated || now - lastUpdated > CAMERA_STATUS_OFFLINE_TIMEOUT_MS) {
        return { status: 'offline', lastUpdated };
      }

      const frameRate = latest?.frameRate;
      const bitrateKbps = latest?.bitrateKbps;
      
      // User requested to ignore latency for status indication
      // const lagging = ...

      return {
        status: 'online',
        lastUpdated,
        frameRate,
        bitrateKbps,
      };
    },
    [],
  );

  const refreshCameraStatus = useCallback(
    (cameraId: number) => {
      if (typeof cameraId !== 'number' || cameraId <= 0) {
        return;
      }
      updateCameraStatus(cameraId, deriveCameraStatus(cameraId));
    },
    [deriveCameraStatus, updateCameraStatus],
  );
  const [cellDetections, setCellDetections] = useState<(CellDetectionState | null)[]>(
    () => Array.from({ length: MAX_CELLS }, () => null),
  );
  const cellCamerasRef = useRef<(Camera | null)[]>(cellCameras);
  const hoveredCellRef = useRef<number | null>(hoveredCell);
  const restoringLayoutRef = useRef(false);
  const hasRestoredLayoutRef = useRef(false);
  const dragSourceCellRef = useRef<number | null>(null);
  const dragCameraIdRef = useRef<number | null>(null);
  const pointerDragState = useRef<{ sourceIndex: number | null; cameraId: number | null; active: boolean }>({
    sourceIndex: null,
    cameraId: null,
    active: false,
  });
  const pointerUpHandlerRef = useRef<((event: MouseEvent) => void) | null>(null);
  const pointerMoveHandlerRef = useRef<((event: MouseEvent) => void) | null>(null);
  const pointerKeyHandlerRef = useRef<((event: KeyboardEvent) => void) | null>(null);
  const pointerListenersAttachedRef = useRef(false);
  const videoElementRefs = useRef<VideoElementEntry[]>(
    Array.from({ length: MAX_CELLS }, () => ({ grid: null, fullscreen: null, hd: null }))
  );
  const [frameCapturePending, setFrameCapturePending] = useState<boolean[]>(
    () => Array.from({ length: MAX_CELLS }, () => false)
  );
  const [frameAnalysisActive, setFrameAnalysisActive] = useState<boolean[]>(
    () => Array.from({ length: MAX_CELLS }, () => false)
  );
  const frameAnalysisActiveRef = useRef<boolean[]>(Array.from({ length: MAX_CELLS }, () => false));
  const analysisLoopTimersRef = useRef<(number | null)[]>(Array.from({ length: MAX_CELLS }, () => null));
  const analysisModuleRef = useRef<(string | null)[]>(Array.from({ length: MAX_CELLS }, () => null));
  const { toast, showToast, hideToast } = useToast();
  const { modules: analyticsModules, processFrame, processingModuleIds, detections: analyticsDetections } = useAnalytics();
  const readyAnalyticsModules = useMemo(
    () => analyticsModules.filter(module => module.enabled && module.state === 'ready'),
    [analyticsModules],
  );
  const visibleAnalyticsModules = useMemo(
    () => analyticsModules.filter(module => module.enabled),
    [analyticsModules],
  );
  const hasReadyAnalyticsModule = readyAnalyticsModules.length > 0;
  const processingModuleSet = useMemo(() => new Set(processingModuleIds), [processingModuleIds]);

  const getLocalizedModuleNameById = useCallback(
    (moduleId: string, fallback?: string) => {
      const key = `module_${moduleId}_name`;
      const translated = t(key);
      return translated === key ? fallback ?? moduleId : translated;
    },
    [t],
  );

  const getLocalizedModuleName = useCallback(
    (module: AnalyticsModuleStatus) => getLocalizedModuleNameById(module.id, module.name),
    [getLocalizedModuleNameById],
  );


  useEffect(() => {
    cellCamerasRef.current = cellCameras;
  }, [cellCameras]);

  useEffect(() => {
    cellStreamsRef.current = cellStreams;
  }, [cellStreams]);

  useEffect(() => {
    setCellDetections(prev => {
      let changed = false;
      const next = prev.map((entry, idx) => {
        const camera = cellCameras[idx];
        if (!camera) {
          if (entry) {
            changed = true;
          }
          return null;
        }

        const cameraId = String(camera.id);
        if (entry && entry.cameraId === cameraId) {
          return entry;
        }

        if (entry) {
          changed = true;
        }
        return null;
      });

      return changed ? next : prev;
    });
  }, [cellCameras]);

  useEffect(() => {
    hoveredCellRef.current = hoveredCell;
  }, [hoveredCell]);

  useEffect(() => {
    frameAnalysisActiveRef.current = frameAnalysisActive;
  }, [frameAnalysisActive]);

  // Track last processed detection event ID to avoid reprocessing
  const lastProcessedDetectionKeyRef = useRef<string | null>(null);

  // Process incoming detection events from analytics context
  useEffect(() => {
    if (!analyticsDetections || analyticsDetections.length === 0) {
      return;
    }

    const latestDetection = analyticsDetections[0];
    
    // Create unique key from processedAt + moduleId + cameraId to avoid reprocessing same detection
    const detectionKey = `${latestDetection.processedAt}_${latestDetection.moduleId}_${latestDetection.cameraId || 'unknown'}`;
    
    // Skip if we already processed this exact detection
    if (lastProcessedDetectionKeyRef.current === detectionKey) {
      return;
    }
    
    lastProcessedDetectionKeyRef.current = detectionKey;

    // Find which cell is analyzing this camera with this module
    const targetCellIndex = cellCamerasRef.current.findIndex((camera, idx) => {
      if (!camera) return false;
      const cameraId = String(camera.id);
      const activeModuleId = analysisModuleRef.current[idx];
      const isAnalyzing = frameAnalysisActiveRef.current[idx];
      
      return (
        isAnalyzing &&
        activeModuleId === latestDetection.moduleId &&
        (latestDetection.cameraId === cameraId || latestDetection.cameraId === undefined)
      );
    });

    if (targetCellIndex === -1) {
      return;
    }

    // Update cell detections with new detection data
    setCellDetections(prev => {
      const next = [...prev];

      const newDetectionState = {
        cameraId: latestDetection.cameraId || String(cellCamerasRef.current[targetCellIndex]?.id || ''),
        moduleId: latestDetection.moduleId,
        detections: latestDetection.detections,
        processedAt: latestDetection.processedAt,
        frameWidth: latestDetection.frameWidth,
        frameHeight: latestDetection.frameHeight,
      };

      next[targetCellIndex] = newDetectionState;

      return next;
    });
  }, [analyticsDetections]);  // ✅ Only analyticsDetections dependency!


  const setFrameCapturePendingState = useCallback((index: number, value: boolean) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }

    setFrameCapturePending(prev => {
      if (prev[index] === value) {
        return prev;
      }
      const updated = [...prev];
      updated[index] = value;
      return updated;
    });
  }, []);

  const setFrameAnalysisActiveState = useCallback((index: number, value: boolean) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }

    frameAnalysisActiveRef.current[index] = value;
    setFrameAnalysisActive(prev => {
      if (prev[index] === value) {
        return prev;
      }
      const updated = [...prev];
      updated[index] = value;
      return updated;
    });
  }, []);

  const clearAnalysisTimer = useCallback((index: number) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }

    const timer = analysisLoopTimersRef.current[index];
    if (timer != null) {
      clearTimeout(timer);
      analysisLoopTimersRef.current[index] = null;
    }
  }, []);

  const stopFrameAnalysis = useCallback(
    (index: number, options?: { silent?: boolean }) => {
      if (index < 0 || index >= MAX_CELLS) {
        return;
      }

      const activeModuleId = analysisModuleRef.current[index];
      const activeModule = activeModuleId
        ? analyticsModules.find(module => module.id === activeModuleId)
        : null;
      const localizedModuleName = activeModuleId
        ? getLocalizedModuleNameById(activeModuleId, activeModule?.name ?? activeModuleId)
        : null;

      setCellDetections(prev => {
        if (!prev[index]) {
          return prev;
        }
        const updated = [...prev];
        updated[index] = null;
        return updated;
      });

      if (!frameAnalysisActiveRef.current[index]) {
        return;
      }

      clearAnalysisTimer(index);
      analysisModuleRef.current[index] = null;
      setFrameAnalysisActiveState(index, false);
      setFrameCapturePendingState(index, false);

      if (!options?.silent && localizedModuleName) {
        showToast(t('module_toggle_stopped', { module: localizedModuleName }), 'info');
      }
    },
    [
      analyticsModules,
      clearAnalysisTimer,
      getLocalizedModuleNameById,
      setCellDetections,
      setFrameAnalysisActiveState,
      setFrameCapturePendingState,
      showToast,
      t,
    ],
  );


  const updateVideoElementRef = useCallback(
    (index: number, element: HTMLVideoElement | null, scope: VideoRefScope) => {
      if (index < 0 || index >= MAX_CELLS) {
        return;
      }

      const entry = videoElementRefs.current[index];
      if (!entry) {
        videoElementRefs.current[index] = { grid: null, fullscreen: null, hd: null };
      }
      videoElementRefs.current[index][scope] = element;
    },
    [],
  );

  const isVideoElementReady = useCallback((element: HTMLVideoElement | null) => {
    if (!element) {
      return false;
    }

    if (element.videoWidth > 0 && element.videoHeight > 0) {
      return true;
    }

    return element.readyState >= 2;
  }, []);

  const getCapturePrerequisites = useCallback(
    (
      index: number,
      moduleId: string,
      options: { suppressToast?: boolean } = {},
    ): CaptureContext | null => {
      const { suppressToast = false } = options;

      if (index < 0 || index >= MAX_CELLS) {
        return null;
      }

      const knownModule = analyticsModules.find(module => module.id === moduleId) ?? null;
      const moduleName = getLocalizedModuleNameById(moduleId, knownModule?.name ?? moduleId);

      if (!hasReadyAnalyticsModule) {
        if (!suppressToast) {
          showToast(t('module_toggle_disabled', { module: moduleName }), 'warning');
        }
        return null;
      }

      const targetModule = readyAnalyticsModules.find(module => module.id === moduleId);
      if (!targetModule) {
        if (!suppressToast) {
          showToast(t('module_toggle_disabled', { module: moduleName }), 'warning');
        }
        return null;
      }

      if (processingModuleSet.has(moduleId)) {
        if (!suppressToast) {
          showToast(t('module_toggle_busy', { module: moduleName }), 'info');
        }
        return null;
      }

      const camera = cellCamerasRef.current[index];
      if (!camera) {
        if (!suppressToast) {
          showToast(t('module_toggle_no_video', { module: moduleName }), 'warning');
        }
        return null;
      }

      const entry = videoElementRefs.current[index];
      const videoEl = entry?.hd ?? null;
      if (!videoEl || !isVideoElementReady(videoEl)) {
        if (!suppressToast) {
          showToast(t('module_toggle_hd_warmup', { module: moduleName }), 'info');
        }
        return null;
      }

      return {
        module: targetModule,
        camera,
        video: videoEl,
        index,
      };
    },
    [
  analyticsModules,
      hasReadyAnalyticsModule,
      isVideoElementReady,
      processingModuleSet,
      readyAnalyticsModules,
      getLocalizedModuleNameById,
      showToast,
      t,
    ],
  );

  const captureAndProcessFrame = useCallback(
    async (context: CaptureContext): Promise<ProcessedFrameResult | null> => {
      try {
        const { video, module, camera } = context;
        let width = video.videoWidth || video.clientWidth;
        let height = video.videoHeight || video.clientHeight;

        if (!width || !height) {
          return null;
        }

        const forceFullResolution = module.id === 'face-detector';
        // Apply resize if configured (skip for face-detector to preserve snapshot quality)
        const resizeWidth = settings.analytics_resize_width;
        if (!forceFullResolution && resizeWidth > 0 && width > resizeWidth) {
          const scale = resizeWidth / width;
          width = resizeWidth;
          height = Math.round(height * scale);
        }

        const canvas = document.createElement('canvas');
        canvas.width = width;
        canvas.height = height;
        const ctx = canvas.getContext('2d');
        if (!ctx) {
          return null;
        }

        ctx.drawImage(video, 0, 0, width, height);
        const dataUrl = canvas.toDataURL('image/jpeg', 0.92);
        const base64 = dataUrl.split(',')[1];
        if (!base64) {
          return null;
        }

        const options: Record<string, any> = {};
        if (settings.anpr_detection_confidence > 0) {
          options.confidence_threshold = settings.anpr_detection_confidence;
        }

        const response = await processFrame({
          moduleId: module.id,
          cameraId: String(camera.id),
          frameBase64: base64,
          frameWidth: width,
          frameHeight: height,
          options,
        });

        if (!response) {
          return null;
        }

        return {
          response,
          frameWidth: width,
          frameHeight: height,
        };
      } catch (error) {
        console.error('[Dashboard] Failed to capture analytics frame:', error);
        return null;
      }
    },
    [processFrame],
  );

  const runAnalysisIteration = useCallback(
    async (
      index: number,
      options: { initial?: boolean; showSuccessToast?: boolean } = {},
    ) => {
      clearAnalysisTimer(index);

      if (!frameAnalysisActiveRef.current[index]) {
        return;
      }

      const activeModuleId = analysisModuleRef.current[index];
      if (!activeModuleId) {
        stopFrameAnalysis(index, { silent: true });
        return;
      }

      const context = getCapturePrerequisites(index, activeModuleId, {
        suppressToast: !options.initial,
      });

      if (!context) {
        if (!frameAnalysisActiveRef.current[index]) {
          stopFrameAnalysis(index, { silent: true });
          return;
        }

        const timer = window.setTimeout(() => {
          void runAnalysisIteration(index, { initial: false });
        }, ANALYSIS_WARMUP_RETRY_MS);
        analysisLoopTimersRef.current[index] = timer;
        return;
      }

      setFrameCapturePendingState(index, true);

      const result = await captureAndProcessFrame(context);

      setFrameCapturePendingState(index, false);

      if (!result) {
        const moduleName = getLocalizedModuleName(context.module);
        stopFrameAnalysis(index, { silent: true });
        showToast(t('module_toggle_error', { module: moduleName }), 'error');
        return;
      }

      const { response, frameWidth, frameHeight } = result;
      const cameraId = response.cameraId ?? String(context.camera.id);

      setCellDetections(prev => {
        const next = [...prev];

        if (!response.detections.length || !frameWidth || !frameHeight) {
          if (next[index]) {
            next[index] = null;
            return next;
          }
          return prev;
        }

        next[index] = {
          cameraId,
          moduleId: response.moduleId,
          detections: response.detections,
          processedAt: response.processedAt,
          frameWidth,
          frameHeight,
        };

        return next;
      });

      if (options.showSuccessToast) {
        const moduleName = getLocalizedModuleName(context.module);
        showToast(t('module_toggle_started', { module: moduleName }), 'success');
      }

      if (frameAnalysisActiveRef.current[index]) {
        const fps = settings.fps || 20;
        const skip = settings.analytics_frame_skip ?? 5;
        // Calculate delay: skip frames * frame duration. Min 50ms to avoid UI freeze.
        const delay = Math.max(50, (skip * 1000) / fps);

        const timer = window.setTimeout(() => {
          void runAnalysisIteration(index, { initial: false });
        }, delay);
        analysisLoopTimersRef.current[index] = timer;
      }
    },
    [
      captureAndProcessFrame,
      clearAnalysisTimer,
      getCapturePrerequisites,
  getLocalizedModuleName,
      setCellDetections,
      setFrameCapturePendingState,
      showToast,
      stopFrameAnalysis,
      t,
      settings,
    ],
  );

  useEffect(() => {
    if (hasReadyAnalyticsModule) {
      return;
    }

    frameAnalysisActiveRef.current.forEach((active, idx) => {
      if (active) {
        stopFrameAnalysis(idx, { silent: true });
      }
    });
  }, [hasReadyAnalyticsModule, stopFrameAnalysis]);

  useEffect(() => {
    frameAnalysisActiveRef.current.forEach((active, idx) => {
      if (active && !cellCameras[idx]) {
        stopFrameAnalysis(idx, { silent: true });
      }
    });
  }, [cellCameras, stopFrameAnalysis]);

  useEffect(() => () => {
    analysisLoopTimersRef.current.forEach(timer => {
      if (timer != null) {
        clearTimeout(timer);
      }
    });
  }, []);

  const isModuleButtonDisabled = useCallback(
    (index: number, moduleId: string) => {
      if (index < 0 || index >= MAX_CELLS) {
        return true;
      }

      const isActive =
        frameAnalysisActive[index] && analysisModuleRef.current[index] === moduleId;
      if (isActive) {
        return false;
      }

      if (!hasReadyAnalyticsModule) {
        return true;
      }

      if (!readyAnalyticsModules.some(module => module.id === moduleId)) {
        return true;
      }

      if (processingModuleSet.has(moduleId)) {
        return true;
      }

      if (frameCapturePending[index]) {
        return true;
      }

      const camera = cellCamerasRef.current[index];
      if (!camera) {
        return true;
      }

      const entry = videoElementRefs.current[index];
      const hdVideo = entry?.hd ?? null;
      if (!hdVideo || !isVideoElementReady(hdVideo)) {
        return false;
      }

      return false;
    },
    [
      frameAnalysisActive,
      frameCapturePending,
      hasReadyAnalyticsModule,
      isVideoElementReady,
      processingModuleSet,
      readyAnalyticsModules,
    ],
  );

  const getModuleButtonTooltip = useCallback(
    (index: number, module: AnalyticsModuleStatus, moduleName: string) => {
      if (index < 0 || index >= MAX_CELLS) {
        return t('module_toggle_no_video', { module: moduleName });
      }

      const isActive =
        frameAnalysisActive[index] && analysisModuleRef.current[index] === module.id;
      if (isActive) {
        return frameCapturePending[index]
          ? t('module_toggle_running', { module: moduleName })
          : t('module_toggle_stop', { module: moduleName });
      }

      if (!hasReadyAnalyticsModule) {
        return t('module_toggle_disabled', { module: moduleName });
      }

      if (processingModuleSet.has(module.id)) {
        return t('module_toggle_busy', { module: moduleName });
      }

      const camera = cellCamerasRef.current[index];
      if (!camera) {
        return t('module_toggle_no_video', { module: moduleName });
      }

      const entry = videoElementRefs.current[index];
      const hdVideo = entry?.hd ?? null;
      if (!hdVideo || !isVideoElementReady(hdVideo)) {
        return t('module_toggle_hd_warmup', { module: moduleName });
      }

      if (frameCapturePending[index]) {
        return t('module_toggle_busy', { module: moduleName });
      }

      return t('module_toggle_start', { module: moduleName });
    },
    [
      frameAnalysisActive,
      frameCapturePending,
      hasReadyAnalyticsModule,
      isVideoElementReady,
      processingModuleSet,
      t,
    ],
  );

  const handleModuleToggle = useCallback(
    (index: number, moduleId: string) => {
      if (index < 0 || index >= MAX_CELLS) {
        return;
      }

      const activeModule = analysisModuleRef.current[index];
      const isActive = frameAnalysisActiveRef.current[index] && activeModule === moduleId;
      if (isActive) {
        stopFrameAnalysis(index);
        return;
      }

      const knownModule =
        analyticsModules.find(module => module.id === moduleId) ?? null;
      const localizedName = getLocalizedModuleNameById(moduleId, knownModule?.name ?? moduleId);

      if (!hasReadyAnalyticsModule) {
        showToast(t('module_toggle_disabled', { module: localizedName }), 'warning');
        return;
      }

      const targetModule = readyAnalyticsModules.find(module => module.id === moduleId);
      if (!targetModule) {
        showToast(t('module_toggle_disabled', { module: localizedName }), 'warning');
        return;
      }

      if (processingModuleSet.has(moduleId) || frameCapturePending[index]) {
        showToast(t('module_toggle_busy', { module: localizedName }), 'info');
        return;
      }

      const camera = cellCamerasRef.current[index];
      if (!camera) {
        showToast(t('module_toggle_no_video', { module: localizedName }), 'warning');
        return;
      }

      const entry = videoElementRefs.current[index];
      const hdVideo = entry?.hd ?? null;

      if (!hdVideo || !isVideoElementReady(hdVideo)) {
        showToast(t('module_toggle_hd_warmup', { module: localizedName }), 'info');
        analysisModuleRef.current[index] = moduleId;
        setFrameAnalysisActiveState(index, true);
        void runAnalysisIteration(index, { initial: true, showSuccessToast: true });
        return;
      }

      analysisModuleRef.current[index] = moduleId;
      setFrameAnalysisActiveState(index, true);

      void runAnalysisIteration(index, { initial: true, showSuccessToast: true });
    },
    [
      analyticsModules,
      frameCapturePending,
      getLocalizedModuleNameById,
      hasReadyAnalyticsModule,
      isVideoElementReady,
      processingModuleSet,
      readyAnalyticsModules,
      runAnalysisIteration,
      setFrameAnalysisActiveState,
      showToast,
      stopFrameAnalysis,
      t,
    ],
  );

  const updateHoveredCell = useCallback((index: number | null) => {
    hoveredCellRef.current = index;
    setHoveredCellState(index);
  }, []);

  const resetDragState = useCallback(() => {
    dragSourceCellRef.current = null;
    dragCameraIdRef.current = null;
    pointerDragState.current = { sourceIndex: null, cameraId: null, active: false };

    if (pointerListenersAttachedRef.current) {
      if (pointerUpHandlerRef.current) {
        document.removeEventListener('mouseup', pointerUpHandlerRef.current);
        pointerUpHandlerRef.current = null;
      }
      if (pointerMoveHandlerRef.current) {
        document.removeEventListener('mousemove', pointerMoveHandlerRef.current);
        pointerMoveHandlerRef.current = null;
      }
      if (pointerKeyHandlerRef.current) {
        document.removeEventListener('keydown', pointerKeyHandlerRef.current);
        pointerKeyHandlerRef.current = null;
      }
      pointerListenersAttachedRef.current = false;
    }

    if (typeof document !== 'undefined') {
      document.querySelectorAll('.grid-cell.drag-over').forEach(element => {
        element.classList.remove('drag-over');
      });
    }

    updateHoveredCell(null);
  }, [updateHoveredCell]);

  useEffect(() => {
    return () => {
      resetDragState();
    };
  }, [resetDragState]);

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
      const nextMuted = !updated[index];
      updated[index] = nextMuted;

      if (!nextMuted) {
        setCellVolume(prevVolume => {
          const current = prevVolume[index];
          if (current > MIN_AUDIBLE_VOLUME) {
            return prevVolume;
          }
          const nextVolume = [...prevVolume];
          nextVolume[index] = DEFAULT_CELL_VOLUME;
          return nextVolume;
        });
        void initializeGlobalAudio();
      }

      return updated;
    });
  };

  const toggleCellPTZ = (index: number) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }
    setCellPTZActive(prev => {
      const updated = [...prev];
      updated[index] = !updated[index];
      return updated;
    });
  };

  const handleCellVolumeChange = useCallback((index: number, value: number) => {
    if (index < 0 || index >= MAX_CELLS) {
      return;
    }
    const clamped = clampVolume(value);

    setCellVolume(prev => {
      if (prev[index] === clamped) {
        return prev;
      }
      const next = [...prev];
      next[index] = clamped;
      return next;
    });

    if (clamped <= MIN_AUDIBLE_VOLUME) {
      setCellMuted(prev => {
        if (prev[index]) {
          return prev;
        }
        const next = [...prev];
        next[index] = true;
        return next;
      });
    } else {
      setCellMuted(prev => {
        if (!prev[index]) {
          return prev;
        }
        const next = [...prev];
        next[index] = false;
        return next;
      });
      void initializeGlobalAudio();
    }
  }, []);

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
    const streamId = stream.quality === 'hd' ? 0 : 1;
    const statsKey = `${stream.baseName}_${streamId}`;

    const prev = streamStatsRef.current;
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

    const now = Date.now();
    if (nextEntry.lastUpdated !== now) {
      nextEntry.lastUpdated = now;
      changed = true;
    }

    if (changed) {
      streamStatsRef.current = {
        ...prev,
        [statsKey]: nextEntry,
      };
      refreshCameraStatus(stream.cameraId);
    }
  }, [refreshCameraStatus]);

  const enterFullscreen = (index: number) => {
    if (!cellCameras[index]) {
      return;
    }
    console.log('[Dashboard] Entering fullscreen for cell', index);
    // Switch cell to HD
    setCellQuality(index, 'hd');
    // Set fullscreen state (will trigger position:fixed on GridCell)
    setFullscreenCell(index);
  };

  const exitFullscreen = () => {
    setFullscreenCell(prevIndex => {
      if (prevIndex !== null) {
        // Return cell to SD quality after exiting fullscreen
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
    console.log('Dashboard: assigning camera to cell', {
      cameraId: camera.id,
      cellIndex,
      streamingProvider,
    });

    stopFrameAnalysis(cellIndex, { silent: true });

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
      console.log('Dashboard: Registering camera streams', {
        cameraId: camera.id,
        hdUrl,
        sdUrl,
        streamingProvider,
      });
      console.log('Dashboard: Calling add_camera_streams...');
      
      // Add timeout to detect if invoke hangs
      const addStreamsPromise = invoke('add_camera_streams', {
        cameraId: camera.id,
        hdUrl,
        sdUrl,
      });
      
      const timeoutPromise = new Promise((_, reject) => 
        setTimeout(() => reject(new Error('add_camera_streams timeout after 10s')), 10000)
      );
      
      await Promise.race([addStreamsPromise, timeoutPromise]);
      console.log('Dashboard: add_camera_streams completed for camera', camera.id);
    } catch (error) {
      console.error('Dashboard: Failed to configure streaming backend for camera', camera.id, error);
      throw error;
    }

    console.log('Dashboard: After try-catch, calling ensureStreamingBackendStarted...');
    await ensureStreamingBackendStarted();
    console.log('Dashboard: ensureStreamingBackendStarted completed');

    const baseName = `cam${camera.id}`;
    console.log('Dashboard: baseName =', baseName);

    // Prewarm stream if enabled
    if (streamOptSettings.enablePrewarming) {
      console.log(`[Dashboard] Prewarming stream for camera: ${baseName}`);
      void streamPrewarmingService.prewarmStream(baseName, true); // Priority = true for user action
    }

    setCellStreams(prev => {
      const next = [...prev];
      const nextQuality: StreamQuality = fullscreenCell === cellIndex ? 'hd' : 'sd';
      next[cellIndex] = { cameraId: camera.id, baseName, quality: nextQuality };
      console.log(`[Dashboard] Updated cellStreams: cellIndex=${cellIndex}, baseName=${baseName}, quality=${nextQuality}`);
      return next;
    });
  }, [fullscreenCell, stopFrameAnalysis, ensureStreamingBackendStarted, streamOptSettings.enablePrewarming]);

  useEffect(() => {
    if (appStateLoading || restoringLayoutRef.current || hasRestoredLayoutRef.current) {
      return;
    }

    restoringLayoutRef.current = true;

    const { gridSize: savedGridSize, cellStates } = dashboardState;

    setGridSize(savedGridSize);
    setCellPaused(cellStates.map(cell => cell?.paused ?? false));
    setCellMuted(cellStates.map(cell => cell?.muted ?? true));
    setCellVolume(Array.from({ length: MAX_CELLS }, (_, idx) => clampVolume(cellStates[idx]?.volume)));
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
      const volume = clampVolume(cellVolume[idx]);
      return { cameraId, quality, muted, paused, volume };
    });

    updateDashboardState(prev => ({
      ...prev,
      gridSize,
      cellStates,
    }));
  }, [gridSize, cellCameras, cellStreams, cellMuted, cellPaused, cellVolume, appStateLoading, updateDashboardState]);

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
          updateHoveredCell(targetCell);
        }
      } catch (error) {
        console.error('Dashboard: Ошибка при добавлении камеры в ячейку через двойной клик', error);
      }
    };

    window.setCellCamera = handler;

    return () => {
      if (window.setCellCamera === handler) {
        window.setCellCamera = undefined;
      }
    };
  }, [assignCameraToCell, updateHoveredCell]);

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

  const swapCellContents = useCallback((sourceIndex: number, targetIndex: number) => {
    if (sourceIndex === targetIndex) {
      return;
    }

    setCellCameras(prev => {
      const next = [...prev];
      [next[sourceIndex], next[targetIndex]] = [next[targetIndex], next[sourceIndex]];
      return next;
    });

    setCellStreams(prev => {
      const next = [...prev];
      [next[sourceIndex], next[targetIndex]] = [next[targetIndex], next[sourceIndex]];
      return next;
    });

    setCellPaused(prev => {
      const next = [...prev];
      [next[sourceIndex], next[targetIndex]] = [next[targetIndex], next[sourceIndex]];
      return next;
    });

    setCellMuted(prev => {
      const next = [...prev];
      [next[sourceIndex], next[targetIndex]] = [next[targetIndex], next[sourceIndex]];
      return next;
    });

    setCellRecording(prev => {
      const next = [...prev];
      [next[sourceIndex], next[targetIndex]] = [next[targetIndex], next[sourceIndex]];
      return next;
    });

    setRecordingPending(prev => {
      const next = [...prev];
      [next[sourceIndex], next[targetIndex]] = [next[targetIndex], next[sourceIndex]];
      return next;
    });

    setFullscreenCell(prev => {
      if (prev === sourceIndex) {
        return targetIndex;
      }
      if (prev === targetIndex) {
        return sourceIndex;
      }
      return prev;
    });
  }, [setCellCameras, setCellStreams, setCellPaused, setCellMuted, setCellRecording, setRecordingPending, setFullscreenCell]);

  const finalizeDragFallback = useCallback(() => {
    const sourceIndex = dragSourceCellRef.current;
    const targetIndex = hoveredCellRef.current;

    const isValidIndex = (value: number | null): value is number =>
      typeof value === 'number' && value >= 0 && value < MAX_CELLS;

    if (
      isValidIndex(sourceIndex) &&
      isValidIndex(targetIndex) &&
      sourceIndex !== targetIndex &&
      cellCamerasRef.current[sourceIndex]
    ) {
      swapCellContents(sourceIndex, targetIndex);
    }

    resetDragState();
  }, [resetDragState, swapCellContents]);

  const finalizePointerDrag = useCallback((targetIndex: number | null) => {
    const { active, sourceIndex } = pointerDragState.current;
    if (!active || sourceIndex === null) {
      resetDragState();
      return;
    }

    if (
      typeof targetIndex === 'number' &&
      targetIndex >= 0 &&
      targetIndex < MAX_CELLS &&
      targetIndex !== sourceIndex &&
      cellCamerasRef.current[sourceIndex]
    ) {
      swapCellContents(sourceIndex, targetIndex);
    }

    if (typeof document !== 'undefined') {
      document.querySelectorAll('.grid-cell.drag-over').forEach(element => {
        element.classList.remove('drag-over');
      });
    }

    resetDragState();
  }, [resetDragState, swapCellContents]);

  const attachPointerListeners = useCallback(() => {
    if (pointerListenersAttachedRef.current) {
      return;
    }

    const resolveCellIndexFromEvent = (event: MouseEvent): number | null => {
      if (!(event.target instanceof HTMLElement)) {
        return null;
      }

      const cellElement = event.target.closest('[data-cell-index]') as HTMLElement | null;
      if (!cellElement) {
        return null;
      }

      const attr = cellElement.getAttribute('data-cell-index');
      if (!attr) {
        return null;
      }

      const parsed = Number.parseInt(attr, 10);
      return Number.isNaN(parsed) ? null : parsed;
    };

    const pointerMoveHandler = (event: MouseEvent) => {
      if (!pointerDragState.current.active) {
        return;
      }

      if (typeof document === 'undefined') {
        return;
      }

      const element = document.elementFromPoint(event.clientX, event.clientY);
      const cellElement = element instanceof HTMLElement ? element.closest('[data-cell-index]') : null;

      let targetIndex: number | null = null;
      if (cellElement instanceof HTMLElement) {
        const attr = cellElement.getAttribute('data-cell-index');
        if (attr) {
          const parsed = Number.parseInt(attr, 10);
          if (!Number.isNaN(parsed)) {
            targetIndex = parsed;
          }
        }
      }

      if (targetIndex !== null && targetIndex !== hoveredCellRef.current) {
        if (hoveredCellRef.current !== null) {
          const previous = document.querySelector(`[data-cell-index="${hoveredCellRef.current}"]`);
          previous?.classList.remove('drag-over');
        }
        updateHoveredCell(targetIndex);
        (cellElement as HTMLElement | null)?.classList.add('drag-over');
        return;
      }

      if (targetIndex === null && hoveredCellRef.current !== null) {
        const previous = document.querySelector(`[data-cell-index="${hoveredCellRef.current}"]`);
        previous?.classList.remove('drag-over');
        updateHoveredCell(null);
      }
    };

    const pointerUpHandler = (event: MouseEvent) => {
      if (!pointerDragState.current.active) {
        return;
      }

      let targetIndex = resolveCellIndexFromEvent(event);

      if (targetIndex === null && hoveredCellRef.current !== null) {
        targetIndex = hoveredCellRef.current;
      }

      finalizePointerDrag(targetIndex);
    };

    const keyHandler = (event: KeyboardEvent) => {
      if (event.key === 'Escape' && pointerDragState.current.active) {
        event.preventDefault();
        resetDragState();
      }
    };

    pointerUpHandlerRef.current = pointerUpHandler;
    pointerMoveHandlerRef.current = pointerMoveHandler;
    pointerKeyHandlerRef.current = keyHandler;

    document.addEventListener('mouseup', pointerUpHandler);
    document.addEventListener('mousemove', pointerMoveHandler);
    document.addEventListener('keydown', keyHandler);

    pointerListenersAttachedRef.current = true;
  }, [finalizePointerDrag, resetDragState, updateHoveredCell]);

  // Helper function for taking snapshots
  const takeSnapshot = async (streamBaseName: string, quality: string) => {
    try {
      // Build full stream name with quality suffix
      const streamSuffix = quality === 'hd' ? '_0' : '_1';
      const fullStreamName = `${streamBaseName}${streamSuffix}`;
      
      // Get snapshot from go2rtc
      const imageData = await invoke<number[]>('get_go2rtc_snapshot', {
        streamName: fullStreamName,
      });

      // Convert to base64
      const uint8Array = new Uint8Array(imageData);
      let binary = '';
      for (let i = 0; i < uint8Array.length; i++) {
        binary += String.fromCharCode(uint8Array[i]);
      }
      const base64Data = btoa(binary);

      // Get screenshots directory from settings
      let screenshotsDir: string | null = null;
      try {
        const settings = await invoke<any>('get_app_settings');
        if (settings?.screenshotsPath) {
          screenshotsDir = settings.screenshotsPath;
        }
      } catch (e) {
        console.warn('[Snapshot] Could not load settings, using default directory');
      }

      // Save screenshot
      const savedPath = await invoke<string>('save_screenshot', {
        args: {
          imageData: `data:image/jpeg;base64,${base64Data}`,
          directory: screenshotsDir,
          cameraName: streamBaseName,
          streamName: streamBaseName,
          quality: quality,
        }
      });

      console.log(`[Snapshot] Saved: ${savedPath}`);
      showToast(t('snapshot_saved') || 'Snapshot saved successfully', 'success');
    } catch (error) {
      console.error('[Snapshot] Failed:', error);
      showToast(t('snapshot_failed') || 'Failed to save snapshot', 'error');
    }
  };

  // Создаём стабильные коллбеки для GridCell через useMemo чтобы избежать ремаунтов
  // КРИТИЧНО: Используем ref для cellStreams чтобы НЕ пересоздавать callbacks при каждом setCellStreams
  const gridCellCallbacks = React.useMemo(() => {
    const callbacks: Array<{
      onStatsUpdateSD: (stats: any) => void;
      onStatsUpdateHD: (stats: any) => void;
      onVideoRefSD: (ref: HTMLVideoElement | null) => void;
      onVideoRefHD: (ref: HTMLVideoElement | null) => void;
    }> = [];

    for (let idx = 0; idx < MAX_CELLS; idx++) {
      callbacks.push({
        // Separate callback for SD stream - always uses 'sd' quality
        onStatsUpdateSD: (stats: any) => {
          const streamInfo = cellStreamsRef.current[idx];
          if (streamInfo) {
            // Force SD quality for this callback
            applyStreamStatsUpdate({ ...streamInfo, quality: 'sd' }, stats);
          }
        },
        // Separate callback for HD stream - always uses 'hd' quality
        onStatsUpdateHD: (stats: any) => {
          const streamInfo = cellStreamsRef.current[idx];
          if (streamInfo) {
            // Force HD quality for this callback
            applyStreamStatsUpdate({ ...streamInfo, quality: 'hd' }, stats);
          }
        },
        onVideoRefSD: (ref: HTMLVideoElement | null) => {
          const streamInfo = cellStreamsRef.current[idx]; // Читаем из ref
          // Update grid ref if SD is the active quality, otherwise still register for fallback
          if (streamInfo && streamInfo.quality === 'sd') {
            updateVideoElementRef(idx, ref, 'grid');
          } else if (ref) {
            // Still register SD video element even if not active quality (for fallback)
            const entry = videoElementRefs.current[idx];
            if (entry && !entry.grid && !entry.hd) {
              updateVideoElementRef(idx, ref, 'grid');
            }
          }
        },
        onVideoRefHD: (ref: HTMLVideoElement | null) => {
          const streamInfo = cellStreamsRef.current[idx]; // Читаем из ref
          // ALWAYS update hd ref
          updateVideoElementRef(idx, ref, 'hd');
          // Update grid ref if HD is the active quality
          if (streamInfo && streamInfo.quality === 'hd') {
            updateVideoElementRef(idx, ref, 'grid');
          }
        },
      });
    }
    return callbacks;
  }, [applyStreamStatsUpdate]); // ТОЛЬКО applyStreamStatsUpdate в зависимостях!

  useEffect(() => {
    const intervalId = window.setInterval(() => {
      // Only update status for cameras that are currently in the grid
      const activeCameraIds = new Set(
        cellCamerasRef.current
          .filter((c): c is Camera => c !== null)
          .map(c => c.id)
      );
      activeCameraIds.forEach(cameraId => refreshCameraStatus(cameraId));
    }, 5000);
    return () => {
      window.clearInterval(intervalId);
    };
  }, [refreshCameraStatus]);

  const gridCells = Array.from({ length: cols * rows }).map((_, idx) => {
    const cam = cellCameras[idx];
    const streamInfo = cellStreams[idx];
    const moduleToggles = visibleAnalyticsModules.map(module => {
      const displayName = getLocalizedModuleName(module);
      return {
        moduleId: module.id,
        label: displayName,
        icon: resolveModuleIcon(module.id),
        tooltip: getModuleButtonTooltip(idx, module, displayName),
        active: frameAnalysisActive[idx] && analysisModuleRef.current[idx] === module.id,
        disabled: isModuleButtonDisabled(idx, module.id),
        onToggle: () => handleModuleToggle(idx, module.id),
      };
    });

    const handlePointerMouseDown = (e: React.MouseEvent) => {
      if (e.button !== 0 || !cam) {
        return;
      }

      const targetEl = e.target as HTMLElement | null;
      if (targetEl?.closest('[data-prevent-drag]')) {
        return;
      }

      e.preventDefault();
      if (window.getSelection) {
        const selection = window.getSelection();
        selection?.removeAllRanges();
      }

      pointerDragState.current = {
        sourceIndex: idx,
        cameraId: cam.id,
        active: true,
      };

      dragSourceCellRef.current = idx;
      dragCameraIdRef.current = cam.id;

      updateHoveredCell(idx);
      attachPointerListeners();
      e.currentTarget.classList.add('drag-over');
    };

    const handlePointerMouseUp = (e: React.MouseEvent) => {
      if (!pointerDragState.current.active) {
        return;
      }

      finalizePointerDrag(idx);
      e.currentTarget.classList.remove('drag-over');
    };

    const handleDrop = async (e: React.DragEvent) => {
      e.preventDefault();
      e.stopPropagation();

      try {
        const payloadRaw =
          e.dataTransfer.getData('application/x-camera-drag') ||
          e.dataTransfer.getData('text/plain');

        let sharedPayload: { type?: string; cameraId?: number; sourceIndex?: number } | null = null;
        if (payloadRaw) {
          try {
            sharedPayload = JSON.parse(payloadRaw);
          } catch {
            sharedPayload = null;
          }
        }

        const sourceCellStr =
          e.dataTransfer.getData('application/x-source-cell') ||
          e.dataTransfer.getData('application/x-source-cell-id');
        let sourceIndex: number | undefined;
        if (sourceCellStr) {
          const parsed = Number.parseInt(sourceCellStr, 10);
          if (!Number.isNaN(parsed)) {
            sourceIndex = parsed;
          }
        } else if (sharedPayload?.type === 'dashboard-camera-drag' && typeof sharedPayload.sourceIndex === 'number') {
          sourceIndex = sharedPayload.sourceIndex;
        } else if (dragSourceCellRef.current !== null) {
          sourceIndex = dragSourceCellRef.current;
        }

        if (sourceIndex !== undefined && sourceIndex >= 0 && sourceIndex < MAX_CELLS && sourceIndex !== idx) {
          if (cellCameras[sourceIndex]) {
            swapCellContents(sourceIndex, idx);
            return;
          }
        }

        const cameraIdStr = e.dataTransfer.getData('application/x-camera-id');
        let cameraId: number | undefined;
        if (cameraIdStr) {
          const parsed = Number.parseInt(cameraIdStr, 10);
          if (!Number.isNaN(parsed)) {
            cameraId = parsed;
          }
        } else if (sharedPayload?.type === 'dashboard-camera-drag' && typeof sharedPayload.cameraId === 'number') {
          cameraId = sharedPayload.cameraId;
        } else if (dragCameraIdRef.current !== null) {
          cameraId = dragCameraIdRef.current;
        }

        if (cameraId === undefined) {
          return;
        }

        const targetCamera = appCameras.find(c => c && c.id === cameraId);

        if (!targetCamera) {
          console.error('Dashboard: Camera not found in state:', cameraId);
          return;
        }

        await assignCameraToCell(targetCamera, idx);
      } catch (error) {
        console.error(`Dashboard: Failed to handle drop on cell ${idx}`, error);
      } finally {
        resetDragState();
      }
    };

    const handleDragStart = (e: React.DragEvent) => {
      if (!cam) {
        e.preventDefault();
        return;
      }

      const target = e.target as HTMLElement | null;
      if (target?.closest('[data-prevent-drag]')) {
        e.preventDefault();
        return;
      }

      e.stopPropagation();
      e.dataTransfer.effectAllowed = 'move';

      dragSourceCellRef.current = idx;
      dragCameraIdRef.current = cam.id;
      updateHoveredCell(idx);

      const payload = JSON.stringify({
        type: 'dashboard-camera-drag',
        cameraId: cam.id,
        sourceIndex: idx,
      });

      const sourceIndexStr = idx.toString();
      const cameraIdStr = cam.id.toString();

      e.dataTransfer.setData('application/x-camera-drag', payload);
      e.dataTransfer.setData('application/x-camera-id', cameraIdStr);
      e.dataTransfer.setData('application/x-source-cell', sourceIndexStr);
      e.dataTransfer.setData('application/x-source-cell-id', sourceIndexStr);
      e.dataTransfer.setData('text/plain', payload);

      const dragImage = (target?.closest('.grid-cell') ?? e.currentTarget) as HTMLElement;
      if (dragImage) {
        try {
          const rect = dragImage.getBoundingClientRect();
          e.dataTransfer.setDragImage(dragImage, rect.width / 2, rect.height / 2);
        } catch {
          // WebView may throw if drag images are unsupported; ignore.
        }
      }
    };

    const handleDragEnd = () => {
      finalizeDragFallback();
    };

    const handleClearCell = async () => {
      if (cellRecording[idx] && streamInfo) {
        await toggleRecordingForCell(idx);
      }

      if (fullscreenCell === idx) {
        exitFullscreen();
      }

      stopFrameAnalysis(idx, { silent: true });
      setFrameCapturePendingState(idx, false);
      updateVideoElementRef(idx, null, 'grid');
      updateVideoElementRef(idx, null, 'fullscreen');
    updateVideoElementRef(idx, null, 'hd');

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
      setCellDetections(prev => {
        if (!prev[idx]) {
          return prev;
        }
        const copy = [...prev];
        copy[idx] = null;
        return copy;
      });
      setCellPaused(prev => {
        const copy = [...prev];
        copy[idx] = false;
        return copy;
      });
      setCellVolume(prev => {
        const copy = [...prev];
        copy[idx] = DEFAULT_CELL_VOLUME;
        return copy;
      });
      setCellMuted(prev => {
        const copy = [...prev];
        copy[idx] = true;
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

    const handleCellSnapshot = () => {
      if (streamInfo) {
        void takeSnapshot(streamInfo.baseName, streamInfo.quality);
      }
    };

    const detectionState = cellDetections[idx];
    const videoEntry = videoElementRefs.current[idx];
    // When in fullscreen, prefer fullscreen video element; otherwise use grid/hd
    const isInFullscreen = fullscreenCell === idx;
    const gridVideoElement = isInFullscreen
      ? (videoEntry?.fullscreen ?? videoEntry?.hd ?? videoEntry?.grid ?? null)
      : (videoEntry?.grid ?? videoEntry?.hd ?? videoEntry?.fullscreen ?? null);
    const hasDetections = !!(detectionState && detectionState.detections.length > 0);

    return (
      <Box
        key={`cell-${idx}`}
        className={`grid-cell ${cellPaused[idx] ? 'paused-state' : ''} ${cellRecording[idx] ? 'recording' : ''} ${fullscreenCell === idx ? 'fullscreen' : ''}`}
        draggable={!!cam}
        onDragStart={handleDragStart}
        onDragEnd={handleDragEnd}
        onMouseDown={handlePointerMouseDown}
        onMouseUp={handlePointerMouseUp}
        data-cell-index={idx}
        tabIndex={0}
        sx={{
          position: 'relative',
          overflow: 'hidden',
          width: '100%',
          height: '100%',
          maxHeight: '100%',
          minHeight: 0,
          border: '1px solid #31353a',
          borderRadius: gridSize > 32 ? 1 : 2,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          backgroundColor: '#36393f',
          cursor: cam ? 'grab' : 'default',
          '&.drag-over': { boxShadow: '0 0 0 2px #1976d2' }
        }}
        onMouseEnter={(event) => {
          updateHoveredCell(idx);
          if (pointerDragState.current.active) {
            event.currentTarget.classList.add('drag-over');
          }
        }}
        onMouseLeave={(event) => {
          event.currentTarget.classList.remove('drag-over');
          if (!pointerDragState.current.active) {
            updateHoveredCell(null);
          }
        }}
        onDragEnter={e => {
          e.preventDefault();
          updateHoveredCell(idx);
          e.currentTarget.classList.add('drag-over');
        }}
        onDragOver={e => {
          e.preventDefault();
          e.stopPropagation();
          e.dataTransfer.dropEffect = 'move';
          if (hoveredCellRef.current !== idx) {
            updateHoveredCell(idx);
          }
        }}
        onDragLeave={e => {
          const nextTarget = e.relatedTarget as Node | null;
          if (nextTarget && e.currentTarget.contains(nextTarget)) {
            return;
          }
          e.currentTarget.classList.remove('drag-over');
          if (!pointerDragState.current.active && hoveredCellRef.current === idx) {
            updateHoveredCell(null);
          }
        }}
        onDrop={e => {
          e.currentTarget.classList.remove('drag-over');
          handleDrop(e);
        }}
        onDoubleClick={() => {
          if (cellCameras[idx]) {
            // Switch to HD quality before opening fullscreen
            // This ensures DualQualityStreamPlayer shows HD stream immediately
            setCellQuality(idx, 'hd');
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
                {/* GridCell - use position:fixed when fullscreen to cover screen without remounting */}
                <Box 
                  sx={{ 
                    // When fullscreen: break out of grid cell and cover entire screen
                    position: fullscreenCell === idx ? 'fixed' : 'absolute',
                    inset: 0,
                    // When fullscreen: highest z-index to be above everything
                    zIndex: fullscreenCell === idx ? 9999 : 'auto',
                    // When fullscreen: dark background
                    backgroundColor: fullscreenCell === idx ? '#000' : 'transparent',
                  }}
                  // Double-click to exit fullscreen
                  onDoubleClick={(e) => {
                    if (fullscreenCell === idx) {
                      e.stopPropagation();
                      console.log('[Dashboard] Exiting fullscreen via double-click');
                      exitFullscreen();
                    }
                  }}
                >
                  {/* Close button when fullscreen */}
                  {fullscreenCell === idx && (
                    <>
                      <IconButton
                        onClick={(e) => {
                          e.stopPropagation();
                          console.log('[Dashboard] Exiting fullscreen via close button');
                          exitFullscreen();
                        }}
                        sx={{
                          position: 'absolute',
                          top: 16,
                          right: 16,
                          zIndex: 10000,
                          color: '#fff',
                          backgroundColor: 'rgba(0,0,0,0.5)',
                          '&:hover': { backgroundColor: 'rgba(0,0,0,0.8)' }
                        }}
                      >
                        <CloseIcon />
                      </IconButton>
                      
                      {/* Fullscreen controls */}
                      <Box 
                        data-prevent-drag
                        sx={{
                          position: 'absolute',
                          top: 16,
                          left: 16,
                          zIndex: 10000
                        }}
                      >
                        <CellControls
                          isFullscreen={true}
                          isRecording={cellRecording[idx]}
                          isRecordingPending={recordingPending[idx]}
                          isMuted={cellMuted[idx]}
                          volume={cellVolume[idx]}
                          isPTZActive={cellPTZActive[idx]}
                          streamId={streamInfo?.quality === 'hd' ? 0 : 1}
                          streamName={streamInfo?.baseName}
                          enableSnapshot={go2rtcSettings.enableSnapshot}
                          onStreamSwitch={handleCellStreamSwitch}
                          onAudio={() => toggleCellMuted(idx)}
                          onVolumeChange={(value) => handleCellVolumeChange(idx, value)}
                          onRecord={() => { void toggleRecordingForCell(idx); }}
                          onSnapshot={async () => {
                            if (streamInfo?.baseName) {
                              await takeSnapshot(streamInfo.baseName, streamInfo.quality);
                            }
                          }}
                          onPTZ={() => toggleCellPTZ(idx)}
                          onClose={() => exitFullscreen()}
                          moduleToggles={moduleToggles}
                        />
                      </Box>

                      {/* PTZ Controls Overlay (Fullscreen) */}
                      {cellPTZActive[idx] && cam && (
                        <Box
                          sx={{
                            position: 'absolute',
                            inset: 0,
                            zIndex: 10001, // Higher than controls
                            pointerEvents: 'none',
                          }}
                        >
                          <PTZControls 
                            camera={cam} 
                            onClose={() => toggleCellPTZ(idx)}
                            scale={1}
                          />
                        </Box>
                      )}
                      
                      {/* Camera name overlay in fullscreen */}
                      <div
                        className="cell-name"
                        style={{
                          position: 'absolute',
                          bottom: 32,
                          right: 32,
                          backgroundColor: 'rgba(0, 0, 0, 0.75)',
                          color: 'white',
                          padding: '8px 16px',
                          borderRadius: 8,
                          fontSize: 18,
                          fontWeight: 500,
                          zIndex: 10000,
                          whiteSpace: 'nowrap'
                        }}
                      >
                        {cam?.name || `Camera ${cam?.id}`} · {streamInfo?.quality.toUpperCase()}
                      </div>
                      
                      {/* Stats overlay in fullscreen */}
                      <div
                        className="cell-stats"
                        style={{
                          position: 'absolute',
                          bottom: 32,
                          left: 32,
                          backgroundColor: 'rgba(0, 0, 0, 0.75)',
                          color: 'white',
                          padding: '6px 12px',
                          borderRadius: 8,
                          fontSize: 14,
                          fontFamily: 'monospace',
                          zIndex: 10000,
                          whiteSpace: 'nowrap'
                        }}
                      >
                        {(() => {
                          // Read current stream info from state, not closure variable
                          const currentStreamInfo = cellStreams[idx];
                          if (!currentStreamInfo) {
                            return `${t('connecting_stream') || 'Подключение...'}`;
                          }
                          const streamId = currentStreamInfo.quality === 'hd' ? 0 : 1;
                          const statsKey = `${currentStreamInfo.baseName}_${streamId}`;
                          const stats = streamStatsRef.current[statsKey];
                          
                          // Debug logging
                          if (fullscreenCell === idx) {
                            console.log('[Dashboard] Fullscreen stats lookup:', {
                              idx,
                              statsKey,
                              stats,
                              allStatsKeys: Object.keys(streamStatsRef.current),
                              currentStreamInfo
                            });
                          }
                          
                          if (stats) {
                            const segments: string[] = [];
                            if (stats.codec) segments.push(stats.codec);
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
                            return segments.length > 0 ? segments.join(' | ') : currentStreamInfo.quality.toUpperCase();
                          }
                          return `${currentStreamInfo.quality.toUpperCase()} | ${t('connecting_stream') || 'Подключение...'}`;
                        })()}
                      </div>
                    </>
                  )}
                  <GridCell
                    streamBaseName={cellStreams[idx]?.baseName || streamInfo.baseName}
                    quality={cellStreams[idx]?.quality || streamInfo.quality}
                    cellMuted={cellMuted[idx]}
                    volume={cellVolume[idx]}
                    cellIndex={idx}
                    onStatsUpdateSD={gridCellCallbacks[idx].onStatsUpdateSD}
                    onStatsUpdateHD={gridCellCallbacks[idx].onStatsUpdateHD}
                    onVideoRefSD={gridCellCallbacks[idx].onVideoRefSD}
                    onVideoRefHD={gridCellCallbacks[idx].onVideoRefHD}
                    go2rtcSettings={go2rtcSettings}
                    streamOptSettings={streamOptSettings}
                    detections={detectionState?.detections ?? []}
                    detectionFrameWidth={detectionState?.frameWidth ?? 0}
                    detectionFrameHeight={detectionState?.frameHeight ?? 0}
                    videoElement={gridVideoElement}
                    hasDetections={hasDetections}
                  />
                </Box>
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
                    zIndex: 10,
                    whiteSpace: 'nowrap',
                    maxWidth: 'fit-content'
                  }}
                >
                  {(() => {
                    const streamId = streamInfo.quality === 'hd' ? 0 : 1;
                    const statsKey = `${streamInfo.baseName}_${streamId}`;
                    const stats = streamStatsRef.current[statsKey];
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
            <Box data-prevent-drag>
              <CellControls
                isFullscreen={fullscreenCell === idx}
                isRecording={cellRecording[idx]}
                isRecordingPending={recordingPending[idx]}
                isMuted={cellMuted[idx]}
                volume={cellVolume[idx]}
                isPTZActive={cellPTZActive[idx]}
                streamId={streamInfo?.quality === 'hd' ? 0 : 1}
                streamName={streamInfo?.baseName}
                enableSnapshot={go2rtcSettings.enableSnapshot}
                onStreamSwitch={handleCellStreamSwitch}
                onAudio={handleCellAudio}
                onVolumeChange={(value) => handleCellVolumeChange(idx, value)}
                onRecord={handleCellRecord}
                onSnapshot={handleCellSnapshot}
                onPTZ={() => toggleCellPTZ(idx)}
                onClose={() => { void handleClearCell(); }}
                moduleToggles={moduleToggles}
              />
            </Box>

            {/* PTZ Controls Overlay */}
            {cellPTZActive[idx] && cam && (
              <Box
                sx={{
                  position: 'absolute',
                  inset: 0,
                  zIndex: 100,
                  pointerEvents: 'none',
                }}
              >
                <PTZControls 
                  camera={cam} 
                  onClose={() => toggleCellPTZ(idx)}
                  scale={gridSize === 1 ? 1 : (gridSize <= 4 ? 0.8 : 0.6)}
                />
              </Box>
            )}
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

  const dialogLayout = templateDialog.layoutId
    ? layoutTabs.find(tab => tab.id === templateDialog.layoutId) ?? null
    : null;
  const isTemplateDialogOpen = templateDialog.open && !!dialogLayout;
  const dialogGridSize = dialogLayout?.template.gridSize ?? gridSize;
  const dialogLayoutName = dialogLayout?.name ?? generateDefaultLayoutName();

  // go2rtc is the sole streaming provider; no readiness polling is required here.

  return (
    <>
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
          gridTemplateColumns: `repeat(${cols}, minmax(0, 1fr))`, 
          gridTemplateRows: `repeat(${rows}, minmax(0, 1fr))`, 
          gap: gridSize > 25 ? 1 : 2, // Меньшие отступы для больших сеток
          p: 0, 
          minHeight: 0,
          width: '100%',
          height: '100%',
          '& > div': {
            minWidth: 0,
            minHeight: 0,
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

      {/* Dialog removed - using position:fixed for fullscreen instead */}
      </Box>
      <Toast
        message={toast.message}
        severity={toast.severity}
        open={toast.open}
        onClose={hideToast}
      />
    </>
  );
};

export default Dashboard;