import { invoke } from '@tauri-apps/api/core';
import { isTauriAvailable } from '../utils/tauri';

export type RtspTransportPreference = 'tcp' | 'udp';

export interface RtspHandshakeOptions {
  url: string;
  username?: string;
  password?: string;
  transport?: RtspTransportPreference;
  includeAudio?: boolean;
  timeoutMs?: number;
}

interface RawRtspHandshakeTrack {
  control_uri: string;
  response_headers: Record<string, string>;
}

interface RawRtspHandshakeResponse {
  base_uri: string;
  session?: string;
  sdp?: string;
  video: RawRtspHandshakeTrack;
  audio?: RawRtspHandshakeTrack;
  log: string[];
}

export interface RtspHandshakeTrack {
  controlUri: string;
  responseHeaders: Record<string, string>;
}

export interface RtspHandshakeResponse {
  baseUri: string;
  session?: string;
  sdp?: string;
  video: RtspHandshakeTrack;
  audio?: RtspHandshakeTrack;
  log: string[];
}

const normalizeTrack = (track: RawRtspHandshakeTrack): RtspHandshakeTrack => ({
  controlUri: track.control_uri,
  responseHeaders: track.response_headers,
});

export const performRtspHandshake = async (
  options: RtspHandshakeOptions,
): Promise<RtspHandshakeResponse | null> => {
  if (!isTauriAvailable()) {
    console.warn('[RTSP] Tauri runtime unavailable, skipping handshake');
    return null;
  }

  const transport = options.transport ?? 'tcp';
  const includeAudio = options.includeAudio ?? true;
  const timeoutMs = options.timeoutMs ?? 3000;

  try {
    const raw = await invoke<RawRtspHandshakeResponse>('rtsp_handshake', {
      request: {
        url: options.url,
        username: options.username,
        password: options.password,
        transport,
        include_audio: includeAudio,
        timeout_ms: timeoutMs,
      },
    });

    const response: RtspHandshakeResponse = {
      baseUri: raw.base_uri,
      session: raw.session,
      sdp: raw.sdp,
      video: normalizeTrack(raw.video),
      audio: raw.audio ? normalizeTrack(raw.audio) : undefined,
      log: raw.log,
    };

    return response;
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error);
    if (import.meta.env.DEV) {
      console.debug('[RTSP] Handshake attempt failed (non-fatal):', message);
    }
    return null;
  }
};

export const resolveStreamSource = async (path: string): Promise<string | null> => {
  if (!isTauriAvailable()) {
    return null;
  }

  try {
    const result = await invoke<string | null>('resolve_stream_source', { path });
    if (typeof result === 'string' && result.length > 0) {
      return result;
    }
    return null;
  } catch (error) {
    console.warn('[RTSP] Failed to resolve stream source:', error);
    return null;
  }
};
