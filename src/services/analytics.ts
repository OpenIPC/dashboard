import { invoke } from '@tauri-apps/api/core';
import { isTauriAvailable } from '../utils/tauri';

export type AnalyticsModuleState = 'disabled' | 'ready' | 'error' | string;

export type FaceSnapshotMode = 'disabled' | 'standard' | 'anonymized' | 'encrypted';

interface RawAnalyticsModuleConfig {
  snapshotsDir: string | null;
  faceSnapshotsMode?: FaceSnapshotMode | null;
  faceSnapshotKeyConfigured?: boolean | null;
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
  faceSnapshotsMode?: FaceSnapshotMode;
  faceSnapshotKeyConfigured?: boolean;
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
  trackId?: string | null;
  dwellMs?: number | null;
  firstSeenAt?: string | null;
  lastSeenAt?: string | null;
  eventType?: 'entered' | 'updated' | null;
  zone?: string | null;
}

export interface AnalyticsDetectionBox {
  id: string;
  label: string;
  confidence: number;
  bounds: AnalyticsBoundingBox;
  color: string;
  trackId?: string;
  dwellMs?: number;
  firstSeenAt?: string;
  lastSeenAt?: string;
  eventType?: 'entered' | 'updated';
  zone?: string;
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

export interface AnalyticsSnapshotListItem {
  id: string;
  moduleId: string;
  cameraId?: string;
  detectionId: string;
  capturedAt: string;
  confidence: number;
  bounds: AnalyticsBoundingBox;
  frameWidth: number;
  frameHeight: number;
  imageFile: string;
  metadataFile: string;
  folderPath: string;
  imagePath: string;
  metadataPath: string;
  imageSize?: number;
  metadataSize?: number;
  imageAvailable: boolean;
  encrypted: boolean;
}

export interface AnalyticsSnapshotListResponse {
  total: number;
  hasMore: boolean;
  items: AnalyticsSnapshotListItem[];
}

export interface AnalyticsSnapshotListOptions {
  moduleId?: string;
  cameraId?: string;
  limit?: number;
  offset?: number;
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
  const faceSnapshotsMode = raw.faceSnapshotsMode ?? undefined;
  const faceSnapshotKeyConfigured = raw.faceSnapshotKeyConfigured ?? undefined;

  const next: AnalyticsModuleConfig = {};
  if (snapshotsDir) {
    next.snapshotsDir = snapshotsDir;
  }
  if (faceSnapshotsMode) {
    next.faceSnapshotsMode = faceSnapshotsMode;
  }
  if (typeof faceSnapshotKeyConfigured === 'boolean') {
    next.faceSnapshotKeyConfigured = faceSnapshotKeyConfigured;
  }

  return Object.keys(next).length > 0 ? next : undefined;
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
  trackId: raw.trackId ?? undefined,
  dwellMs: raw.dwellMs ?? undefined,
  firstSeenAt: raw.firstSeenAt ?? undefined,
  lastSeenAt: raw.lastSeenAt ?? undefined,
  eventType: raw.eventType ?? undefined,
  zone: raw.zone ?? undefined,
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
  return updateAnalyticsModuleConfig(moduleId, { snapshotsDir: snapshotsDir ?? null });
};

export interface UpdateAnalyticsModuleConfigOptions {
  snapshotsDir?: string | null;
  faceSnapshotsMode?: FaceSnapshotMode;
  faceSnapshotKeyHex?: string;
  resetFaceSnapshotKey?: boolean;
}

export const updateAnalyticsModuleConfig = async (
  moduleId: string,
  options: UpdateAnalyticsModuleConfigOptions,
): Promise<AnalyticsModuleStatus | null> => {
  if (!isTauriAvailable()) {
    return null;
  }

  const payload: Record<string, unknown> = {
    module_id: moduleId,
  };

  if (Object.prototype.hasOwnProperty.call(options, 'snapshotsDir')) {
    payload.snapshots_dir = options.snapshotsDir ?? null;
  }
  if (options.faceSnapshotsMode) {
    payload.face_snapshots_mode = options.faceSnapshotsMode;
  }
  if (options.faceSnapshotKeyHex) {
    payload.face_snapshot_key_hex = options.faceSnapshotKeyHex;
  }
  if (options.resetFaceSnapshotKey) {
    payload.reset_face_snapshot_key = true;
  }

  try {
    const raw = await invoke<RawAnalyticsModuleStatus>('analytics_update_module_config', {
      payload,
    });
    return normalizeModuleStatus(raw);
  } catch (error) {
    console.warn(`[Analytics] Failed to update module config for ${moduleId}:`, error);
    return null;
  }
};

export const listAnalyticsSnapshots = async (
  options?: AnalyticsSnapshotListOptions,
): Promise<AnalyticsSnapshotListResponse> => {
  if (!isTauriAvailable()) {
    return { total: 0, hasMore: false, items: [] };
  }

  const payload: Record<string, unknown> = {};
  if (options?.moduleId) {
    payload.moduleId = options.moduleId;
  }
  if (options?.cameraId) {
    payload.cameraId = options.cameraId;
  }
  if (typeof options?.limit === 'number') {
    payload.limit = options.limit;
  }
  if (typeof options?.offset === 'number') {
    payload.offset = options.offset;
  }

  try {
    const args = Object.keys(payload).length > 0 ? { payload } : {};
    return await invoke<AnalyticsSnapshotListResponse>('analytics_list_snapshots', args);
  } catch (error) {
    console.warn('[Analytics] Failed to list snapshots:', error);
    throw error instanceof Error ? error : new Error(String(error));
  }
};

// Object Counter analytics data services

interface RawObjectCounterEvent {
  id: number;
  camera_id?: number | null;
  module_id: string;
  processed_at: string;
  object_type: string;
  count: number;
  confidence_avg: number;
  dwell_avg_ms?: number | null;
  width_avg?: number | null;
  height_avg?: number | null;
  zone?: string | null;
  snapshot_path?: string | null;
  description?: string | null;
}

interface RawObjectCounterAggregate {
  bucket_start: string;
  total_count: number;
}

interface RawObjectCounterTopCamera {
  camera_id?: number | null;
  total_count: number;
}

export interface ObjectCounterEvent {
  id: number;
  cameraId?: number;
  moduleId: string;
  processedAt: string;
  objectType: string;
  count: number;
  confidenceAvg: number;
  dwellAvgMs?: number;
  widthAvg?: number;
  heightAvg?: number;
  zone?: string;
  snapshotPath?: string;
  description?: string;
}

export interface ObjectCounterAggregate {
  bucketStart: string;
  totalCount: number;
}

export interface ObjectCounterTopCamera {
  cameraId?: number;
  totalCount: number;
}

const normalizeObjectCounterEvent = (raw: RawObjectCounterEvent): ObjectCounterEvent => ({
  id: raw.id,
  cameraId: raw.camera_id ?? undefined,
  moduleId: raw.module_id,
  processedAt: raw.processed_at,
  objectType: raw.object_type,
  count: raw.count,
  confidenceAvg: raw.confidence_avg,
  dwellAvgMs: raw.dwell_avg_ms ?? undefined,
  widthAvg: raw.width_avg ?? undefined,
  heightAvg: raw.height_avg ?? undefined,
  zone: raw.zone ?? undefined,
  snapshotPath: raw.snapshot_path ?? undefined,
  description: raw.description ?? undefined,
});

const normalizeObjectCounterAggregate = (
  raw: RawObjectCounterAggregate,
): ObjectCounterAggregate => ({
  bucketStart: raw.bucket_start,
  totalCount: raw.total_count,
});

const normalizeObjectCounterTopCamera = (
  raw: RawObjectCounterTopCamera,
): ObjectCounterTopCamera => ({
  cameraId: raw.camera_id ?? undefined,
  totalCount: raw.total_count,
});

interface ObjectCounterEventsOptions {
  limit?: number;
  offset?: number;
  cameraId?: number;
  objectTypes?: string[];
  dateFrom?: string;
  dateTo?: string;
}

export const getObjectCounterEvents = async (
  options?: ObjectCounterEventsOptions,
): Promise<ObjectCounterEvent[]> => {
  if (!isTauriAvailable()) {
    return [];
  }

  const payload = {
    limit: options?.limit ?? 20,
    offset: options?.offset ?? 0,
    camera_filter: options?.cameraId ?? null,
    object_types: options?.objectTypes ?? null,
    date_from: options?.dateFrom ?? null,
    date_to: options?.dateTo ?? null,
  };

  try {
    const raw = await invoke<RawObjectCounterEvent[]>('get_object_counter_events', payload);
    return raw.map(normalizeObjectCounterEvent);
  } catch (error) {
    console.warn('[Analytics] Failed to fetch object counter events:', error);
    return [];
  }
};

interface ObjectCounterAggregateOptions {
  bucketMinutes: number;
  buckets: number;
  cameraId?: number;
  objectTypes?: string[];
}

export const getObjectCounterAggregates = async (
  options: ObjectCounterAggregateOptions,
): Promise<ObjectCounterAggregate[]> => {
  if (!isTauriAvailable()) {
    return [];
  }

  const payload = {
    bucket_minutes: options.bucketMinutes,
    buckets: options.buckets,
    camera_filter: options.cameraId ?? null,
    object_types: options.objectTypes ?? null,
  };

  try {
    const raw = await invoke<RawObjectCounterAggregate[]>(
      'get_object_counter_aggregates',
      payload,
    );
    return raw.map(normalizeObjectCounterAggregate);
  } catch (error) {
    console.warn('[Analytics] Failed to fetch object counter aggregates:', error);
    return [];
  }
};

interface ObjectCounterTopOptions {
  limit?: number;
  minutes: number;
  objectTypes?: string[];
}

export const getObjectCounterTopCameras = async (
  options: ObjectCounterTopOptions,
): Promise<ObjectCounterTopCamera[]> => {
  if (!isTauriAvailable()) {
    return [];
  }

  const payload = {
    limit: options.limit ?? 5,
    minutes: options.minutes,
    object_types: options.objectTypes ?? null,
  };

  try {
    const raw = await invoke<RawObjectCounterTopCamera[]>(
      'get_object_counter_top_cameras',
      payload,
    );
    return raw.map(normalizeObjectCounterTopCamera);
  } catch (error) {
    console.warn('[Analytics] Failed to fetch object counter top cameras:', error);
    return [];
  }
};

export interface ObjectCounterPoint {
  x: number;
  y: number;
}

export interface ObjectCounterLine {
  id: number;
  cameraId: number;
  name: string;
  start: ObjectCounterPoint;
  end: ObjectCounterPoint;
  direction: string;
  objectType?: string;
  enabled: boolean;
}

export interface ObjectCounterLineInput {
  id?: number;
  cameraId: number;
  name: string;
  start: ObjectCounterPoint;
  end: ObjectCounterPoint;
  direction: string;
  objectType?: string;
  enabled: boolean;
}

interface RawObjectCounterLine {
  id: number;
  camera_id: number;
  name: string;
  start_x: number;
  start_y: number;
  end_x: number;
  end_y: number;
  direction: string;
  object_type?: string | null;
  enabled: boolean;
}

export interface ObjectCounterZone {
  id: number;
  cameraId: number;
  name: string;
  polygon: ObjectCounterPoint[];
  zoneType: string;
  objectType?: string;
  dwellThresholdMs?: number;
  enabled: boolean;
}

export interface ObjectCounterZoneInput {
  id?: number;
  cameraId: number;
  name: string;
  polygon: ObjectCounterPoint[];
  zoneType: string;
  objectType?: string;
  dwellThresholdMs?: number;
  enabled: boolean;
}

interface RawObjectCounterZone {
  id: number;
  camera_id: number;
  name: string;
  polygon: string;
  zone_type: string;
  object_type?: string | null;
  dwell_threshold_ms?: number | null;
  enabled: boolean;
}

const normalizeObjectCounterLine = (raw: RawObjectCounterLine): ObjectCounterLine => ({
  id: raw.id,
  cameraId: raw.camera_id,
  name: raw.name,
  start: { x: raw.start_x, y: raw.start_y },
  end: { x: raw.end_x, y: raw.end_y },
  direction: raw.direction,
  objectType: raw.object_type ?? undefined,
  enabled: Boolean(raw.enabled),
});

const parsePolygon = (serialized: string): ObjectCounterPoint[] => {
  try {
    const parsed = JSON.parse(serialized);
    if (Array.isArray(parsed)) {
      return parsed
        .map(point => {
          if (Array.isArray(point) && point.length >= 2) {
            const [x, y] = point;
            return { x: Number(x) || 0, y: Number(y) || 0 };
          }
          if (point && typeof point === 'object' && 'x' in point && 'y' in point) {
            const coord = point as { x: number; y: number };
            return { x: Number(coord.x) || 0, y: Number(coord.y) || 0 };
          }
          return undefined;
        })
        .filter((value): value is ObjectCounterPoint => Boolean(value));
    }
  } catch (error) {
    console.warn('[Analytics] Failed to parse zone polygon', error);
  }
  return [];
};

const normalizeObjectCounterZone = (raw: RawObjectCounterZone): ObjectCounterZone => ({
  id: raw.id,
  cameraId: raw.camera_id,
  name: raw.name,
  polygon: parsePolygon(raw.polygon),
  zoneType: raw.zone_type,
  objectType: raw.object_type ?? undefined,
  dwellThresholdMs: raw.dwell_threshold_ms ?? undefined,
  enabled: Boolean(raw.enabled),
});

const serializeLinePayload = (input: ObjectCounterLineInput) => ({
  id: input.id ?? null,
  camera_id: input.cameraId,
  name: input.name,
  start_x: input.start.x,
  start_y: input.start.y,
  end_x: input.end.x,
  end_y: input.end.y,
  direction: input.direction,
  object_type: input.objectType ?? null,
  enabled: input.enabled,
});

const serializeZonePayload = (input: ObjectCounterZoneInput) => ({
  id: input.id ?? null,
  camera_id: input.cameraId,
  name: input.name,
  polygon: JSON.stringify(input.polygon ?? []),
  zone_type: input.zoneType,
  object_type: input.objectType ?? null,
  dwell_threshold_ms: input.dwellThresholdMs ?? null,
  enabled: input.enabled,
});

interface ObjectCounterLineQueryOptions {
  cameraId?: number;
}

export const getObjectCounterLines = async (
  options?: ObjectCounterLineQueryOptions,
): Promise<ObjectCounterLine[]> => {
  if (!isTauriAvailable()) {
    return [];
  }

  try {
    const raw = await invoke<RawObjectCounterLine[]>(
      'get_object_counter_lines',
      { camera_id: options?.cameraId ?? null },
    );
    return raw.map(normalizeObjectCounterLine);
  } catch (error) {
    console.warn('[Analytics] Failed to fetch object counter lines:', error);
    return [];
  }
};

export const upsertObjectCounterLine = async (
  input: ObjectCounterLineInput,
): Promise<ObjectCounterLine | null> => {
  if (!isTauriAvailable()) {
    return null;
  }

  try {
    const raw = await invoke<RawObjectCounterLine>(
      'upsert_object_counter_line',
      { payload: serializeLinePayload(input) },
    );
    return normalizeObjectCounterLine(raw);
  } catch (error) {
    console.warn('[Analytics] Failed to upsert object counter line:', error);
    return null;
  }
};

export const deleteObjectCounterLine = async (id: number): Promise<boolean> => {
  if (!isTauriAvailable()) {
    return false;
  }

  try {
    await invoke('delete_object_counter_line', { id });
    return true;
  } catch (error) {
    console.warn('[Analytics] Failed to delete object counter line:', error);
    return false;
  }
};

interface ObjectCounterZoneQueryOptions {
  cameraId?: number;
}

export const getObjectCounterZones = async (
  options?: ObjectCounterZoneQueryOptions,
): Promise<ObjectCounterZone[]> => {
  if (!isTauriAvailable()) {
    return [];
  }

  try {
    const raw = await invoke<RawObjectCounterZone[]>(
      'get_object_counter_zones',
      { camera_id: options?.cameraId ?? null },
    );
    return raw.map(normalizeObjectCounterZone);
  } catch (error) {
    console.warn('[Analytics] Failed to fetch object counter zones:', error);
    return [];
  }
};

export const upsertObjectCounterZone = async (
  input: ObjectCounterZoneInput,
): Promise<ObjectCounterZone | null> => {
  if (!isTauriAvailable()) {
    return null;
  }

  try {
    const raw = await invoke<RawObjectCounterZone>(
      'upsert_object_counter_zone',
      { payload: serializeZonePayload(input) },
    );
    return normalizeObjectCounterZone(raw);
  } catch (error) {
    console.warn('[Analytics] Failed to upsert object counter zone:', error);
    return null;
  }
};

export const deleteObjectCounterZone = async (id: number): Promise<boolean> => {
  if (!isTauriAvailable()) {
    return false;
  }

  try {
    await invoke('delete_object_counter_zone', { id });
    return true;
  } catch (error) {
    console.warn('[Analytics] Failed to delete object counter zone:', error);
    return false;
  }
};
