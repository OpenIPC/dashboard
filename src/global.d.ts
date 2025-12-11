import type { WindowCameraBridge } from './types';

// go2rtc enhanced types
export type Go2RtcTransportType = 'webrtc' | 'mse' | 'hls' | 'rtsp' | 'rtmp' | 'mp4' | 'mjpeg' | 'homekit' | 'webtorrent';

export interface Go2RtcStreamStats {
  name: string;
  producers: Array<{
    type: string;
    url?: string;
    state?: string;
    time?: number;
    remoteAddr?: string;
  }>;
  consumers: Array<{
    type: string;
    remoteAddr?: string;
    state?: string;
    time?: number;
    send?: number;
    recv?: number;
  }>;
  receivers?: Array<{
    codec: string;
    state?: string;
    bytes?: number;
    senders?: number;
  }>;
  senders?: Array<{
    codec: string;
    state?: string;
    bytes?: number;
  }>;
}

export interface Go2RtcStreamInfo {
  streamName: string;
  online: boolean;
  consumerCount: number;
  bitrateKbps: number;
  videoCodec?: string;
  audioCodec?: string;
  resolution?: string;
  fps?: number;
  latencyMs?: number;
}

export interface TwoWayAudioConfig {
  enabled: boolean;
  codec: 'opus' | 'pcma' | 'pcmu';
  sampleRate: number;
  channels: number;
  pushToTalk?: boolean;
  echoCancellation?: boolean;
  noiseSuppression?: boolean;
  autoGainControl?: boolean;
}

export interface Go2RtcStreamFilter {
  rotate?: 0 | 90 | 180 | 270;
  width?: number;
  height?: number;
  crop?: { x: number; y: number; width: number; height: number };
  overlay?: {
    text?: string;
    position?: 'top-left' | 'top-right' | 'bottom-left' | 'bottom-right';
    fontSize?: number;
    color?: string;
  };
}

export interface AdaptiveStreamConfig {
  enabled: boolean;
  minBitrateKbps: number;
  maxBitrateKbps: number;
  targetLatencyMs: number;
  autoSwitchQuality: boolean;
}

declare global {
  interface Window {
    __VMS_CAMERAS?: WindowCameraBridge['__VMS_CAMERAS'];
    __peerConnection?: WindowCameraBridge['__peerConnection'];
    setCellCamera?: WindowCameraBridge['setCellCamera'];
    // go2rtc enhanced state
    __GO2RTC_STATS?: Map<string, Go2RtcStreamInfo>;
    __GO2RTC_TRANSPORT?: Map<string, Go2RtcTransportType>;
    __TAURI__?: unknown;
  }
}

export {};
declare module '@tauri-apps/api/window' {
  export const appWindow: {
    minimize: () => Promise<void>;
    toggleMaximize: () => Promise<void>;
    close: () => Promise<void>;
  };
}

declare module 'hls.js' {
  export default class Hls {
    static isSupported(): boolean;
    static Events: Record<string, string>;
    static ErrorTypes: Record<string, string>;
    constructor(config?: unknown);
    loadSource(source: string): void;
    attachMedia(media: HTMLMediaElement): void;
    on(event: string, handler: (...args: unknown[]) => void): void;
    off(event: string, handler: (...args: unknown[]) => void): void;
    startLoad(): void;
    recoverMediaError(): void;
    destroy(): void;
  }
}
