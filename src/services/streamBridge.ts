import { invoke } from '@tauri-apps/api/core';
import type { StreamPathStatus } from '../types';

/**
 * Fetch current MediaMTX path status list. Used as a foundation for a unified stream bridge dashboard.
 */
export async function fetchStreamPathStatuses(): Promise<StreamPathStatus[]> {
  try {
    const result = await invoke<StreamPathStatus[]>('list_mediamtx_paths');
    return result;
  } catch (error) {
    console.error('[streamBridge] Failed to fetch stream path statuses', error);
    if (error instanceof Error) {
      throw error;
    }

    throw new Error(
      typeof error === 'string' && error.trim().length > 0
        ? error
        : 'Не удалось получить статус потоков'
    );
  }
}
