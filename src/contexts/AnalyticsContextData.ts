import { createContext } from 'react';
import type {
  AnalyticsModuleStatus,
  AnalyticsDetectionResponse,
  AnalyticsProcessFrameRequest,
  UpdateAnalyticsModuleConfigOptions,
} from '../services/analytics';

export interface AnalyticsDetectionEvent extends AnalyticsDetectionResponse {
  id: string;
  receivedAt: string;
}

export interface AnalyticsContextValue {
  modules: AnalyticsModuleStatus[];
  isLoadingModules: boolean;
  moduleOperationId: string | null;
  processingModuleIds: string[];
  lastError: string | null;
  lastUpdatedAt: string | null;
  detections: AnalyticsDetectionEvent[];
  refreshModules: () => Promise<AnalyticsModuleStatus[]>;
  toggleModule: (moduleId: string, enabled: boolean, pendingMessage?: string) => Promise<boolean>;
  processFrame: (request: AnalyticsProcessFrameRequest) => Promise<AnalyticsDetectionResponse | null>;
  updateModuleSnapshotsDir: (
    moduleId: string,
    snapshotsDir?: string,
  ) => Promise<AnalyticsModuleStatus | null>;
  updateModuleConfig: (
    moduleId: string,
    options: UpdateAnalyticsModuleConfigOptions,
  ) => Promise<AnalyticsModuleStatus | null>;
  clearDetections: () => void;
}

export const AnalyticsContext = createContext<AnalyticsContextValue | undefined>(undefined);
