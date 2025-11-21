import { createContext } from 'react';
import type { Camera, CameraGroup, DashboardState } from '../types';

export type StreamingProvider = 'go2rtc';

export interface AppStateSettings {
  language: string;
  recordingsFolder: string;
  screenshotsFolder: string;
  hardwareAcceleration: string;
  analyticsProvider: string;
  enableNotifications: boolean;
  qscale: number;
  fps: number;
  analytics_resize_width: number;
  analytics_frame_skip: number;
  analytics_record_duration: number;
  anpr_detection_confidence: number;
  anpr_crop_expansion: number;
  anpr_crnn_confidence: number;
  anpr_python_confidence: number;
  anpr_enable_python: boolean;
}

export type CameraHealthStatus = 'online' | 'lagging' | 'offline';

export interface CameraStatusEntry {
  status: CameraHealthStatus;
  lastUpdated: number | null;
  bitrateKbps?: number;
  frameRate?: number;
}

export interface AppStateContextType {
  cameras: Camera[];
  setCameras: (cameras: Camera[]) => void;
  addCamera: (camera: Camera) => Promise<void>;
  updateCamera: (camera: Camera) => Promise<void>;
  removeCamera: (cameraId: number) => Promise<void>;
  groups: CameraGroup[];
  setGroups: (groups: CameraGroup[]) => void;
  addGroup: (group: CameraGroup) => Promise<void>;
  updateGroup: (group: CameraGroup) => Promise<void>;
  removeGroup: (groupId: number) => Promise<void>;
  settings: AppStateSettings;
  updateSettings: (newSettings: Partial<AppStateSettings>) => Promise<void>;
  streamingProvider: StreamingProvider;
  ensureStreamingBackendStarted: () => Promise<void>;
  isLoading: boolean;
  loadAppState: () => Promise<void>;
  saveAppState: () => Promise<void>;
  dashboardState: DashboardState;
  updateDashboardState: (
    updater: DashboardState | ((prev: DashboardState) => DashboardState)
  ) => void;
  cameraStatuses: Record<number, CameraStatusEntry>;
  updateCameraStatus: (cameraId: number, status: CameraStatusEntry) => void;
}

export const AppStateContext = createContext<AppStateContextType | undefined>(undefined);

