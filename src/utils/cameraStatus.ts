import type { CameraHealthStatus } from '../contexts/AppStateContextData';

export const CAMERA_STATUS_COLORS: Record<CameraHealthStatus, string> = {
  online: '#4caf50',
  lagging: '#ffb74d',
  offline: '#f44336',
};

const CAMERA_STATUS_DEFAULT_LABELS: Record<CameraHealthStatus, string> = {
  online: 'Online',
  lagging: 'Lagging',
  offline: 'Offline',
};

export const resolveCameraStatusLabel = (
  status: CameraHealthStatus,
  translate?: (key: string) => string,
): string => {
  if (typeof translate === 'function') {
    const key = `camera_status_${status}`;
    const localized = translate(key);
    if (localized && localized !== key) {
      return localized;
    }
  }
  return CAMERA_STATUS_DEFAULT_LABELS[status];
};
