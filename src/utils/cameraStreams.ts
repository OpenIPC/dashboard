import { invoke } from '@tauri-apps/api/core';
import type { Camera } from '../types';

interface CameraStreamPaths {
  hdPath: string;
  sdPath: string;
}

export interface CameraRtspUrls {
  hdUrl: string;
  sdUrl: string;
}

const ensureLeadingPath = (value: string): string => {
  if (!value) {
    return '';
  }

  const trimmed = value.trim();
  if (!trimmed) {
    return '';
  }

  const lower = trimmed.toLowerCase();
  if (lower.startsWith('rtsp://') || lower.startsWith('http://') || lower.startsWith('https://')) {
    return trimmed;
  }

  return trimmed.startsWith('/') ? trimmed : `/${trimmed}`;
};

const deriveSdPathFromHd = (hdPath: string): string => {
  const normalized = ensureLeadingPath(hdPath || '/stream=0') || '/stream=0';

  if (normalized.toLowerCase().startsWith('rtsp://')) {
    return normalized;
  }

  const candidates = [
    normalized.replace(/stream=0/gi, 'stream=1'),
    normalized.replace(/stream0/gi, 'stream1'),
    normalized.replace(/(channels?\/)(101)/gi, '$1102'),
    normalized.replace(/_0\b/gi, '_1'),
    normalized.replace(/main/gi, 'sub'),
  ];

  for (const candidate of candidates) {
    if (candidate !== normalized) {
      return ensureLeadingPath(candidate);
    }
  }

  return '/stream=1';
};

export const resolveCameraStreamPaths = (
  camera: Camera,
  hdFallback?: string,
): CameraStreamPaths => {
  const hdCandidate = (camera.path_hd && camera.path_hd.trim()) || hdFallback || '/stream=0';
  const hdPath = ensureLeadingPath(hdCandidate);
  const sdPath = camera.path_sd && camera.path_sd.trim()
    ? ensureLeadingPath(camera.path_sd.trim())
    : deriveSdPathFromHd(hdPath);

  return { hdPath, sdPath };
};

export const buildCameraRtspUrls = async (camera: Camera): Promise<CameraRtspUrls> => {
  if (camera.streamUrl && camera.streamUrl.startsWith('rtsp://')) {
    try {
      const baseUrl = new URL(camera.streamUrl);
      const { hdPath, sdPath } = resolveCameraStreamPaths(camera, baseUrl.pathname);

      const absolutize = (path: string, fallback: string): string => {
        const trimmed = path?.trim() ?? '';
        if (!trimmed) {
          return fallback;
        }

        if (/^[a-z][a-z0-9+.-]*:\/\//i.test(trimmed)) {
          return trimmed;
        }

        const next = new URL(baseUrl.toString());
        next.pathname = trimmed.startsWith('/') ? trimmed : `/${trimmed}`;
        return next.toString();
      };

      const hdUrl = absolutize(hdPath, camera.streamUrl);
      const sdUrl = absolutize(sdPath, absolutize(deriveSdPathFromHd(baseUrl.pathname), camera.streamUrl));

      return { hdUrl, sdUrl };
    } catch (error) {
      console.warn('cameraStreams: Failed to parse streamUrl, falling back to manual construction', error);
    }
  }

  let password = '';
  if (camera.pass_enc && camera.pass_enc.trim() !== '') {
    password = await invoke<string>('decrypt_password', { enc: camera.pass_enc });
  } else {
    password = camera.pass || '';
  }

  const username = camera.user ? encodeURIComponent(camera.user) : '';
  const encodedPassword = password ? encodeURIComponent(password) : '';

  let authPart = '';
  if (username && encodedPassword) {
    authPart = `${username}:${encodedPassword}@`;
  } else if (username) {
    authPart = `${username}@`;
  } else if (encodedPassword) {
    authPart = `:${encodedPassword}@`;
  }

  const portPart = camera.port ? `:${camera.port}` : '';
  const base = `rtsp://${authPart}${camera.ip}${portPart}`;
  const { hdPath, sdPath } = resolveCameraStreamPaths(camera);

  const buildUrl = (path: string, fallback: string): string => {
    if (!path) {
      return `${base}${fallback}`;
    }
    if (path.toLowerCase().startsWith('rtsp://')) {
      return path;
    }
    return `${base}${path}`;
  };

  return {
    hdUrl: buildUrl(hdPath, '/stream=0'),
    sdUrl: buildUrl(sdPath, deriveSdPathFromHd('/stream=0')),
  };
};
