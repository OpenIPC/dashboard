import React, { useCallback, useEffect, useMemo, useState } from 'react';
import type { ReactNode } from 'react';
import { listen, type UnlistenFn } from '@tauri-apps/api/event';
import {
  listAnalyticsModules,
  enableAnalyticsModule,
  disableAnalyticsModule,
  processAnalyticsFrame,
  updateAnalyticsModuleConfig,
  type AnalyticsModuleStatus,
  type AnalyticsProcessFrameRequest,
  type AnalyticsDetectionResponse,
} from '../services/analytics';
import { AnalyticsContext } from './AnalyticsContextData';
import type {
  AnalyticsContextValue,
  AnalyticsDetectionEvent,
} from './AnalyticsContextData';
import { isTauriAvailable } from '../utils/tauri';

const MIN_PROGRESS_HINT = 0.01;

const withProgressHint = (value?: number): number => {
  if (typeof value === 'number' && value > 0) {
    return value;
  }
  return MIN_PROGRESS_HINT;
};

const generateEventId = (): string => {
  if (typeof crypto !== 'undefined' && crypto.randomUUID) {
    return crypto.randomUUID();
  }
  return `${Date.now().toString(36)}-${Math.random().toString(36).slice(2, 10)}`;
};

interface AnalyticsProviderProps {
  children: ReactNode;
}

interface AnalyticsModuleProgressPayload {
  moduleId: string;
  progress: number;
  stage?: string | null;
}

export const AnalyticsProvider: React.FC<AnalyticsProviderProps> = ({ children }) => {
  const [modules, setModules] = useState<AnalyticsModuleStatus[]>([]);
  const [isLoadingModules, setIsLoadingModules] = useState<boolean>(false);
  const [moduleOperationId, setModuleOperationId] = useState<string | null>(null);
  const [processingModuleIds, setProcessingModuleIds] = useState<string[]>([]);
  const [lastError, setLastError] = useState<string | null>(null);
  const [lastUpdatedAt, setLastUpdatedAt] = useState<string | null>(null);
  const [detections, setDetections] = useState<AnalyticsDetectionEvent[]>([]);

  const refreshModules = useCallback(async (): Promise<AnalyticsModuleStatus[]> => {
    setIsLoadingModules(true);
    try {
      const result = await listAnalyticsModules();
      setModules(result);
      setLastUpdatedAt(new Date().toISOString());
      setLastError(null);
      return result;
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error);
      console.warn('[Analytics] refreshModules failed:', message);
      setLastError(message);
      setModules([]);
      return [];
    } finally {
      setIsLoadingModules(false);
    }
  }, []);

  const toggleModule = useCallback(
    async (moduleId: string, enabled: boolean, pendingMessage?: string): Promise<boolean> => {
      setModuleOperationId(moduleId);
      setLastError(null);

      setModules(prev => {
        if (prev.length === 0) {
          return prev;
        }

        return prev.map(module => {
          if (module.id !== moduleId) {
            return module;
          }

          if (enabled) {
            return {
              ...module,
              enabled: true,
              state: 'loading',
              progress: withProgressHint(module.progress),
              message: pendingMessage ?? module.message,
            };
          }

          return {
            ...module,
            enabled: false,
            state: 'disabled',
            progress: undefined,
            message: undefined,
          };
        });
      });

      try {
        const success = enabled
          ? await enableAnalyticsModule(moduleId)
          : await disableAnalyticsModule(moduleId);

        if (!success) {
          throw new Error('Analytics backend reported failure');
        }

        await refreshModules();
        return true;
      } catch (error) {
        const message = error instanceof Error ? error.message : String(error);
        console.warn('[Analytics] toggleModule failed:', message);
        setLastError(message);
        await refreshModules();
        return false;
      } finally {
        setModuleOperationId(null);
      }
    },
    [refreshModules],
  );

  const processFrame = useCallback(
    async (
      request: AnalyticsProcessFrameRequest,
    ): Promise<AnalyticsDetectionResponse | null> => {
      const moduleId = request.moduleId;

      setProcessingModuleIds(prev =>
        prev.includes(moduleId) ? prev : [...prev, moduleId],
      );
      setLastError(null);

      try {
        const response = await processAnalyticsFrame(request);
        if (!response) {
          setLastError('Analytics frame processing failed');
          return null;
        }

        const event: AnalyticsDetectionEvent = {
          id: generateEventId(),
          receivedAt: new Date().toISOString(),
          ...response,
        };

        setDetections(prev => [event, ...prev].slice(0, 50));
        return response;
      } catch (error) {
        const message = error instanceof Error ? error.message : String(error);
        console.warn('[Analytics] processFrame failed:', message);
        setLastError(message);
        return null;
      } finally {
        setProcessingModuleIds(prev => prev.filter(id => id !== moduleId));
      }
    },
    [],
  );

  const clearDetections = useCallback(() => {
    setDetections([]);
  }, []);

  const updateModuleConfig = useCallback(
    async (moduleId: string, options: Parameters<typeof updateAnalyticsModuleConfig>[1]) => {
      try {
        const updated = await updateAnalyticsModuleConfig(moduleId, options);
        if (updated) {
          setModules(prev => {
            const exists = prev.some(module => module.id === updated.id);
            if (!exists) {
              return prev;
            }
            return prev.map(module => (module.id === updated.id ? updated : module));
          });
          setLastError(null);
          return updated;
        }

        const message = 'Failed to update module configuration';
        setLastError(message);
        await refreshModules();
        return null;
      } catch (error) {
        const message = error instanceof Error ? error.message : String(error);
        console.warn('[Analytics] updateModuleConfig failed:', message);
        setLastError(message);
        await refreshModules();
        return null;
      }
    },
    [refreshModules],
  );

  const updateModuleSnapshotsDir = useCallback(
    async (moduleId: string, snapshotsDir?: string) => {
      try {
        return await updateModuleConfig(moduleId, { snapshotsDir: snapshotsDir ?? null });
      } catch (error) {
        console.warn('[Analytics] updateModuleSnapshotsDir failed:', error);
        return null;
      }
    },
    [updateModuleConfig],
  );

  // Listen to analytics module progress events
  useEffect(() => {
    if (!isTauriAvailable()) {
      return;
    }

    let unlisten: UnlistenFn | null = null;

    const setup = async () => {
      try {
        unlisten = await listen<AnalyticsModuleProgressPayload>(
          'analytics-module-progress',
          event => {
            const payload = event.payload;
            if (!payload) {
              return;
            }

            const clamped = Number.isFinite(payload.progress)
              ? Math.max(0, Math.min(1, payload.progress))
              : undefined;

            setModules(prev => {
              if (prev.length === 0) {
                return prev;
              }

              let changed = false;
              const next = prev.map(module => {
                if (module.id !== payload.moduleId) {
                  return module;
                }

                changed = true;
                return {
                  ...module,
                  enabled: true,
                  state: 'loading',
                  progress: clamped ?? module.progress ?? 0,
                  message: payload.stage ?? module.message,
                };
              });

              return changed ? next : prev;
            });
          },
        );
      } catch (error) {
        console.warn('[Analytics] failed to subscribe to progress events:', error);
      }
    };

    void setup();

    return () => {
      if (unlisten) {
        unlisten();
        unlisten = null;
      }
    };
  }, []);

  // Listen to analytics detection events
  useEffect(() => {
    if (!isTauriAvailable()) {
      return;
    }

    let unlisten: UnlistenFn | null = null;

    const setup = async () => {
      try {
        unlisten = await listen<AnalyticsDetectionResponse>(
          'analytics-detection',
          event => {
            const payload = event.payload;
            if (!payload) {
              return;
            }

            const detectionEvent: AnalyticsDetectionEvent = {
              id: generateEventId(),
              receivedAt: new Date().toISOString(),
              ...payload,
            };

            setDetections(prev => [detectionEvent, ...prev].slice(0, 50));
          },
        );
      } catch (error) {
        console.warn('[Analytics] failed to subscribe to detection events:', error);
      }
    };

    void setup();

    return () => {
      if (unlisten) {
        unlisten();
        unlisten = null;
      }
    };
  }, []);

  useEffect(() => {
    void refreshModules();
  }, [refreshModules]);

  const contextValue = useMemo<AnalyticsContextValue>(
    () => ({
      modules,
      isLoadingModules,
      moduleOperationId,
      processingModuleIds,
      lastError,
      lastUpdatedAt,
      detections,
      refreshModules,
      toggleModule,
      processFrame,
      clearDetections,
      updateModuleConfig,
      updateModuleSnapshotsDir,
    }),
    [
      modules,
      isLoadingModules,
      moduleOperationId,
      processingModuleIds,
      lastError,
      lastUpdatedAt,
      detections,
      refreshModules,
      toggleModule,
      processFrame,
      clearDetections,
      updateModuleConfig,
      updateModuleSnapshotsDir,
    ],
  );

  return <AnalyticsContext.Provider value={contextValue}>{children}</AnalyticsContext.Provider>;
};
