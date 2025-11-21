import { invoke } from '@tauri-apps/api/core';
import { isTauriAvailable } from '../utils/tauri';

export type AnalyticsModuleState = 'disabled' | 'ready' | 'error' | string;

interface RawAnalyticsModuleConfig {
  snapshotsDir: string | null;
}

interface RawAnalyticsModuleStatus {
  id: string;
  name: string;
  version: string;
  description: string;
  enabled: boolean;
  state: AnalyticsModuleState;
  progress: number | null;
  message: string | null;
  lastActivatedAt: string | null;
  lastErrorAt: string | null;
  config?: RawAnalyticsModuleConfig | null;
}

export interface AnalyticsModuleStatus {
  id: string;
  name: string;
  version: string;
  description: string;
  enabled: boolean;
  state: AnalyticsModuleState;
  progress?: number;
  message?: string;
  lastActivatedAt?: string;
  lastErrorAt?: string;
  config?: AnalyticsModuleConfig;
}

export interface AnalyticsModuleConfig {
  snapshotsDir?: string;
}

interface RawBoundingBox {
  x: number;
  y: number;
  width: number;
  height: number;
}

export interface AnalyticsBoundingBox {
  x: number;
  y: number;
  width: number;
  height: number;
}

interface RawDetectionBox {
  id: string;
  label: string;
  confidence: number;
  bounds: RawBoundingBox;
  color: string;
}

export interface AnalyticsDetectionBox {
  id: string;
  label: string;
  confidence: number;
  bounds: AnalyticsBoundingBox;
  color: string;
}

interface RawDetectionResponse {
  moduleId: string;
  cameraId: string | null;
  detections: RawDetectionBox[];
  processedAt: string;
  frameWidth: number;
  frameHeight: number;
}

export interface AnalyticsDetectionResponse {
  moduleId: string;
  cameraId?: string;
  detections: AnalyticsDetectionBox[];
  processedAt: string;
  frameWidth: number;
  frameHeight: number;
}

export interface AnalyticsProcessFrameRequest {
  moduleId: string;
  cameraId?: string;
  frameBase64: string;
  frameWidth: number;
  frameHeight: number;
  options?: Record<string, any>;
}

const normalizeModuleConfig = (
  raw?: RawAnalyticsModuleConfig | null,
): AnalyticsModuleConfig | undefined => {
  if (!raw) {
    return undefined;
  }

  const snapshotsDir = raw.snapshotsDir ?? undefined;
  if (!snapshotsDir) {
    return undefined;
  }

  return { snapshotsDir };
};

const normalizeModuleStatus = (
  raw: RawAnalyticsModuleStatus,
): AnalyticsModuleStatus => ({
  id: raw.id,
  name: raw.name,
  version: raw.version,
  description: raw.description,
  enabled: raw.enabled,
  state: raw.state,
  progress: typeof raw.progress === 'number' ? raw.progress : undefined,
  message: raw.message ?? undefined,
  lastActivatedAt: raw.lastActivatedAt ?? undefined,
  lastErrorAt: raw.lastErrorAt ?? undefined,
  config: normalizeModuleConfig(raw.config),
});

const normalizeDetectionBox = (raw: RawDetectionBox): AnalyticsDetectionBox => ({
  id: raw.id,
  label: raw.label,
  confidence: raw.confidence,
  bounds: {
    x: raw.bounds.x,
    y: raw.bounds.y,
    width: raw.bounds.width,
    height: raw.bounds.height,
  },
  color: raw.color,
});

const normalizeDetectionResponse = (
  raw: RawDetectionResponse,
): AnalyticsDetectionResponse => ({
  moduleId: raw.moduleId,
  cameraId: raw.cameraId ?? undefined,
  detections: raw.detections.map(normalizeDetectionBox),
  processedAt: raw.processedAt,
  frameWidth: raw.frameWidth,
  frameHeight: raw.frameHeight,
});

export const listAnalyticsModules = async (): Promise<AnalyticsModuleStatus[]> => {
  if (!isTauriAvailable()) {
    return [];
  }

  try {
    const raw = await invoke<RawAnalyticsModuleStatus[]>('analytics_list_modules');
    return raw.map(normalizeModuleStatus);
  } catch (error) {
    console.warn('[Analytics] Failed to list modules:', error);
    return [];
  }
};

export const enableAnalyticsModule = async (moduleId: string): Promise<boolean> => {
  if (!isTauriAvailable()) {
    return false;
  }

  try {
    await invoke('analytics_enable_module', { moduleId });
    return true;
  } catch (error) {
    console.warn(`[Analytics] Failed to enable module ${moduleId}:`, error);
    return false;
  }
};

export const disableAnalyticsModule = async (moduleId: string): Promise<boolean> => {
  if (!isTauriAvailable()) {
    return false;
  }

  try {
    await invoke('analytics_disable_module', { moduleId });
    return true;
  } catch (error) {
    console.warn(`[Analytics] Failed to disable module ${moduleId}:`, error);
    return false;
  }
};

export const processAnalyticsFrame = async (
  request: AnalyticsProcessFrameRequest,
): Promise<AnalyticsDetectionResponse | null> => {
  if (!isTauriAvailable()) {
    return null;
  }

  try {
    const raw = await invoke<RawDetectionResponse>('analytics_process_frame', {
      payload: {
        module_id: request.moduleId,
        camera_id: request.cameraId ?? null,
        frame_base64: request.frameBase64,
        frame_width: request.frameWidth,
        frame_height: request.frameHeight,
        options: request.options ?? null,
      },
    });

    return normalizeDetectionResponse(raw);
  } catch (error) {
    console.warn('[Analytics] Failed to process frame:', error);
    return null;
  }
};

export const updateAnalyticsModuleSnapshotsDir = async (
  moduleId: string,
  snapshotsDir?: string,
): Promise<AnalyticsModuleStatus | null> => {
  if (!isTauriAvailable()) {
    return null;
  }

  try {
    const raw = await invoke<RawAnalyticsModuleStatus>('analytics_update_module_config', {
      payload: {
        module_id: moduleId,
        snapshots_dir: snapshotsDir ?? null,
      },
    });

    return normalizeModuleStatus(raw);
  } catch (error) {
    console.warn(`[Analytics] Failed to update snapshots dir for ${moduleId}:`, error);
    return null;
  }
};
