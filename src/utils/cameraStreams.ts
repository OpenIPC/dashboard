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

const ensureLeadingSlash = (path: string): string => {
  if (!path) {
    return '';
  }
  return path.startsWith('/') ? path : `/${path}`;
};

const deriveSdPathFromHd = (hdPath: string): string => {
  const normalized = ensureLeadingSlash(hdPath || '/stream=0') || '/stream=0';
  const candidates = [
    normalized.replace(/stream=0/gi, 'stream=1'),
    normalized.replace(/stream0/gi, 'stream1'),
    normalized.replace(/(channels?\/)(101)/gi, '$1102'),
    normalized.replace(/_0\b/gi, '_1'),
    normalized.replace(/main/gi, 'sub'),
  ];

  for (const candidate of candidates) {
    if (candidate !== normalized) {
      return ensureLeadingSlash(candidate);
    }
  }

  return '/stream=1';
};

export const resolveCameraStreamPaths = (
  camera: Camera,
  hdFallback?: string,
): CameraStreamPaths => {
  const hdCandidate = (camera.path_hd && camera.path_hd.trim()) || hdFallback || '/stream=0';
  const hdPath = ensureLeadingSlash(hdCandidate);
  const sdPath = camera.path_sd && camera.path_sd.trim()
    ? ensureLeadingSlash(camera.path_sd.trim())
    : deriveSdPathFromHd(hdPath);

  return { hdPath, sdPath };
};

export const buildCameraRtspUrls = async (camera: Camera): Promise<CameraRtspUrls> => {
  if (camera.streamUrl && camera.streamUrl.startsWith('rtsp://')) {
    try {
      const baseUrl = new URL(camera.streamUrl);
      const { hdPath, sdPath } = resolveCameraStreamPaths(camera, baseUrl.pathname);
      const hdUrl = new URL(camera.streamUrl);
      hdUrl.pathname = hdPath;
      const sdUrl = new URL(camera.streamUrl);
      sdUrl.pathname = sdPath;
      return { hdUrl: hdUrl.toString(), sdUrl: sdUrl.toString() };
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

  return {
    hdUrl: `${base}${hdPath}`,
    sdUrl: `${base}${sdPath}`,
  };
};
