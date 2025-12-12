import React, { useEffect, useRef, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
import Hls from 'hls.js';
import type { ErrorData } from 'hls.js';
import { performRtspHandshake, resolveStreamSource } from '../services/rtsp';
import { isTauriAvailable } from '../utils/tauri';
import StreamMonitor from './StreamMonitor';
import { WebRTCStatsCollector } from '../services/webrtcStats';
import { useWebRTCStatsContext } from '../contexts/WebRTCStatsContext';

// Stream optimization settings interface
interface StreamOptimizationSettings {
  enableLowLatency?: boolean;
  playoutDelayHint?: number;
  jitterBufferTarget?: number;
  enableLatencyMonitoring?: boolean;
  maxBufferedLatency?: number;
  latencyCheckInterval?: number;
}

// КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Глобальная инициализация аудио контекста
let globalAudioInitialized = false;

const initializeGlobalAudio = async (): Promise<void> => {
  if (globalAudioInitialized) return;
  
  try {
    if (typeof window !== 'undefined' && window.AudioContext) {
      const audioContext = new AudioContext();
      if (audioContext.state === 'suspended') {
        await audioContext.resume();
      }
      await audioContext.close();
      globalAudioInitialized = true;
      console.log('[GlobalAudio] ✓ Global audio initialized successfully');
    }
  } catch (error) {
    console.error('[GlobalAudio] Failed to initialize audio context:', error);
  }
};

// Инициализируем при загрузке модуля
if (typeof window !== 'undefined') {
  document.addEventListener('click', initializeGlobalAudio, { once: true });
  document.addEventListener('touchstart', initializeGlobalAudio, { once: true });
  document.addEventListener('keydown', initializeGlobalAudio, { once: true });
}

interface VideoStreamPlayerProps {
  streamName: string;
  useHdQuality?: boolean;
  useExactName?: boolean;
  onError?: (error: Error) => void;
  onVideoRef?: (ref: HTMLVideoElement | null) => void;
  controls?: boolean;
  autoPlay?: boolean;
  muted?: boolean;
  debug?: boolean;
  width?: string | number;
  height?: string | number;
  style?: React.CSSProperties;
  className?: string;
  objectFit?: React.CSSProperties['objectFit'];
  isPaused?: boolean;
  onStatsUpdate?: (stats: StreamPlaybackStats) => void;
  // Enhanced go2rtc features
  showMonitor?: boolean;
  monitorCompact?: boolean;
  enableSnapshot?: boolean;
  enable2WayAudio?: boolean;
  enableAdaptiveBitrate?: boolean;
  // Performance optimizations
  fastStart?: boolean; // Быстрый старт с минимальными задержками
  // WebRTC Stats Dashboard
  showWebRTCStats?: boolean; // Показывать панель статистики WebRTC
  webrtcStatsUpdateInterval?: number; // Интервал обновления статистики (мс)
  statsDisplayName?: string; // Отображаемое имя для WebRTC статистики (если отличается от streamName)
  volume?: number;
}

interface StreamPlaybackStats {
  bitrateKbps?: number;
  frameRate?: number;
  width?: number;
  height?: number;
  codec?: string;
}

const sanitizeName = (name: string) => name.replace(/ /g, '_').toLowerCase();
type StreamingBackend = 'go2rtc';

interface WhepEndpointDescriptor {
  base: string;
  provider: StreamingBackend;
}

const FALLBACK_WHEP_BASES: Record<StreamingBackend, string[]> = {
  go2rtc: ['http://127.0.0.1:1984'],
};

const buildFallbackDescriptors = (provider: StreamingBackend): WhepEndpointDescriptor[] =>
  FALLBACK_WHEP_BASES[provider].map(base => ({
    base,
    provider,
  }));

const areDescriptorListsEqual = (
  left: WhepEndpointDescriptor[] | undefined,
  right: WhepEndpointDescriptor[] | undefined,
): boolean => {
  if (left === right) {
    return true;
  }
  if (!left || !right || left.length !== right.length) {
    return false;
  }
  for (let index = 0; index < left.length; index += 1) {
    const a = left[index];
    const b = right[index];
    if (!a || !b || a.provider !== b.provider || a.base !== b.base) {
      return false;
    }
  }
  return true;
};

// Ensures the local SDP already contains host ICE candidates before we contact go2rtc.
const waitForIceGatheringComplete = async (
  pc: RTCPeerConnection,
  timeoutMs: number,
  logger?: (message: string) => void,
) => {
  if (pc.iceGatheringState === 'complete') {
    logger?.('ICE gathering already complete');
    return;
  }

  await new Promise<void>(resolve => {
    let settled = false;
    let timerId: number | null = null;
    const nativeAdd = typeof pc.addEventListener === 'function' ? pc.addEventListener.bind(pc) : null;
    const nativeRemove = typeof pc.removeEventListener === 'function' ? pc.removeEventListener.bind(pc) : null;
    const previousHandler = pc.onicegatheringstatechange;

    const handler = () => {
      logger?.(`ICE gathering state -> ${pc.iceGatheringState}`);
      if (pc.iceGatheringState === 'complete') {
        finish();
      }
    };

    const finish = () => {
      if (settled) {
        return;
      }
      settled = true;
      if (timerId !== null) {
        clearTimeout(timerId);
      }
      if (nativeRemove && nativeAdd) {
        nativeRemove('icegatheringstatechange', handler);
      } else {
        pc.onicegatheringstatechange = previousHandler ?? null;
      }
      resolve();
    };

    if (nativeAdd && nativeRemove) {
      nativeAdd('icegatheringstatechange', handler);
    } else {
      pc.onicegatheringstatechange = event => {
        previousHandler?.call(pc, event);
        handler();
      };
    }

    timerId = window.setTimeout(() => {
      logger?.(`ICE gathering timeout after ${timeoutMs}ms (state=${pc.iceGatheringState})`);
      finish();
    }, Math.max(200, timeoutMs));
  });
};

const isStreamingBackend = (value: unknown): value is StreamingBackend =>
  value === 'go2rtc';

const readStreamingProviderFallback = (): StreamingBackend => {
  if (typeof window === 'undefined') {
    return 'go2rtc';
  }

  try {
    const raw = window.localStorage.getItem('appSettings');
    if (raw) {
      const parsed = JSON.parse(raw) as { streaming?: { provider?: unknown } };
      if (parsed?.streaming?.provider === 'go2rtc') {
        return 'go2rtc';
      }
    }
  } catch {
    // Ignore parsing errors and fall back to go2rtc defaults.
  }

  return 'go2rtc';
};

type SdpAudioProfile = {
  codec: string;
  sampleRate?: number;
  channels?: number;
};

const parseSdpAudioProfile = (sdp?: string): SdpAudioProfile | null => {
  if (!sdp) {
    return null;
  }

  const lines = sdp.split(/\r?\n/);
  let inAudio = false;

  for (const rawLine of lines) {
    const line = rawLine.trim();
    if (!line) {
      continue;
    }

    if (line.startsWith('m=')) {
      inAudio = line.startsWith('m=audio');
      continue;
    }

    if (!inAudio || !line.startsWith('a=rtpmap:')) {
      continue;
    }

    const payload = line.substring('a=rtpmap:'.length).trim();
    const spaceIdx = payload.indexOf(' ');
    if (spaceIdx === -1) {
      continue;
    }

    const encoding = payload.substring(spaceIdx + 1).trim();
    if (!encoding) {
      continue;
    }

    const parts = encoding.split('/');
    let codec = parts[0]?.toLowerCase();
    if (!codec) {
      continue;
    }

    codec = (() => {
      switch (codec) {
        case 'mpeg4-generic':
        case 'mp4a-latm':
        case 'mp4a':
          return 'aac';
        default:
          return codec;
      }
    })();

    const rateValue = parts[1] ? parseInt(parts[1], 10) : NaN;
    const channelsValue = parts[2] ? parseInt(parts[2], 10) : NaN;

    return {
      codec,
      sampleRate: Number.isNaN(rateValue) ? undefined : rateValue,
      channels: Number.isNaN(channelsValue) ? undefined : channelsValue,
    };
  }

  return null;
};

const isOpusSampleRateSupported = (rate?: number): boolean => {
  if (rate === undefined) {
    return true;
  }
  return rate >= 8000 && rate <= 48000;
};

const isWebRtcAudioProfileSupported = (profile: SdpAudioProfile | null): boolean => {
  if (!profile) {
    return true;
  }

  const { codec, sampleRate } = profile;

  switch (codec) {
    case 'opus':
      return isOpusSampleRateSupported(sampleRate);
    case 'pcma':
    case 'pcmu':
    case 'g711':
    case 'g711a':
    case 'g711u':
    case 'g722':
      return true;
    default:
      return false;
  }
};

const isAudioProfileTranscodable = (profile: SdpAudioProfile | null): boolean => {
  if (!profile) {
    return false;
  }

  switch (profile.codec) {
    case 'aac':
    case 'mpeg4-generic':
    case 'mp4a-latm':
    case 'mp4a':
    case 'mp3':
    case 'mpa':
      return true;
    default:
      return false;
  }
};

const describeAudioProfile = (profile: SdpAudioProfile | null): string => {
  if (!profile) {
    return 'unknown';
  }

  const parts: string[] = [profile.codec.toUpperCase()];
  if (profile.sampleRate) {
    parts.push(`${profile.sampleRate}Hz`);
  }
  if (profile.channels) {
    parts.push(`${profile.channels}ch`);
  }
  return parts.join(' ');
};

const VideoStreamPlayer: React.FC<VideoStreamPlayerProps> = ({
  streamName,
  useHdQuality = false,
  useExactName = false,
  onError,
  onVideoRef,
  controls = true,
  autoPlay = true,
  muted = true,
  debug = false,
  width = '100%',
  height = '100%',
  style = {},
  className = '',
  objectFit = 'cover',
  isPaused = false,
  onStatsUpdate,
  // Enhanced go2rtc features
  showMonitor = false,
  monitorCompact = true,
  // Performance optimizations
  fastStart = true, // По умолчанию включено
  // WebRTC Stats Dashboard
  showWebRTCStats = false,
  webrtcStatsUpdateInterval = 1000,
  statsDisplayName, // Отображаемое имя для статистики
  volume: volumeProp = 1,
}) => {
  const videoRef = useRef<HTMLVideoElement>(null);
  const hlsRef = useRef<Hls | null>(null);
  const pcRef = useRef<RTCPeerConnection | null>(null);
  const audioRef = useRef<HTMLAudioElement | null>(null);
  const audioStreamRef = useRef<MediaStream | null>(null);
  const playbackStreamRef = useRef<MediaStream | null>(null);
  const [isConnecting, setIsConnecting] = useState(true);
  const [errorMessage, setErrorMessage] = useState<string | null>(null);
  const [connectionMethod, setConnectionMethod] = useState<string>('');
  const initialProvider = React.useMemo(() => readStreamingProviderFallback(), []);
  const [whepTargets, setWhepTargets] = useState<WhepEndpointDescriptor[]>(() => buildFallbackDescriptors(initialProvider));
  const [activeStreamingProvider, setActiveStreamingProvider] = useState<StreamingBackend>(initialProvider);
  const statsIntervalRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const webrtcStatsIntervalRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const lastStatsRef = useRef<{ timestamp: number; bytesReceived: number; framesDecoded?: number } | null>(null);
  const statsCallbackRef = useRef<VideoStreamPlayerProps['onStatsUpdate']>(undefined);
  const isPausedRef = useRef<boolean>(false);
  const activeStartAttemptRef = useRef<number | null>(null);
  const startAttemptCounterRef = useRef<number>(0);
  const retryTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const failureCountRef = useRef<number>(0);
  const overlayTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const [showConnectingOverlay, setShowConnectingOverlay] = useState(false);
  const debugRef = useRef(debug);
  const mutedRef = useRef(muted);
  const onVideoRefRef = useRef(onVideoRef);
  
  // Stream optimization settings
  const [streamOptSettings, setStreamOptSettings] = useState<StreamOptimizationSettings>({
    enableLowLatency: true,
    playoutDelayHint: 0,
    jitterBufferTarget: 0,
    enableLatencyMonitoring: true,
    maxBufferedLatency: 1.0,
    latencyCheckInterval: 2,
  });
  
  // WebRTC Stats Dashboard state
  const [forceH264, setForceH264] = useState(false);
  const volumeRef = useRef(Math.max(0, Math.min(1, volumeProp)));

  // Mount/unmount logging for debugging
  const componentId = useRef(`${streamName}-${useHdQuality ? 'HD' : 'SD'}-${Date.now()}`).current;
  useEffect(() => {
    console.log(`[VideoStreamPlayer][${componentId}] ✅ MOUNTED - streamName=${streamName}, quality=${useHdQuality ? 'HD' : 'SD'}`);
    return () => {
      console.log(`[VideoStreamPlayer][${componentId}] ❌ UNMOUNTED`);
    };
  }, [componentId, streamName, useHdQuality]);

  useEffect(() => {
    const clamped = Math.max(0, Math.min(1, volumeProp ?? 1));
    volumeRef.current = clamped;
    const effectiveVolume = mutedRef.current ? 0 : clamped;

    if (audioRef.current) {
      audioRef.current.volume = effectiveVolume;
    }

    if (videoRef.current) {
      videoRef.current.volume = effectiveVolume;
    }
  }, [volumeProp, muted]);

  const webrtcStatsCollectorRef = useRef<WebRTCStatsCollector | null>(null);
  const webrtcStatsContext = useWebRTCStatsContext();
  
  // Load stream optimization settings from backend
  useEffect(() => {
    const loadStreamOptSettings = async () => {
      try {
        const settings = await invoke<any>('get_app_settings');
        if (settings?.streamOptimization) {
          setStreamOptSettings({
            enableLowLatency: settings.streamOptimization.enableLowLatency ?? true,
            playoutDelayHint: settings.streamOptimization.playoutDelayHint ?? 0,
            jitterBufferTarget: settings.streamOptimization.jitterBufferTarget ?? 0,
            enableLatencyMonitoring: settings.streamOptimization.enableLatencyMonitoring ?? true,
            maxBufferedLatency: settings.streamOptimization.maxBufferedLatency ?? 1.0,
            latencyCheckInterval: settings.streamOptimization.latencyCheckInterval ?? 2,
          });
        }
      } catch (err) {
        console.warn('[VideoStreamPlayer] Failed to load stream optimization settings:', err);
      }
    };
    
    void loadStreamOptSettings();
  }, []);
  
  const applyAudioTrackState = (enabled: boolean) => {
    const updateStreamTracks = (stream: MediaStream | null) => {
      if (!stream) {
        return;
      }
      stream.getAudioTracks().forEach(track => {
        if (track.enabled !== enabled) {
          track.enabled = enabled;
        }
      });
    };

    updateStreamTracks(audioStreamRef.current);
    updateStreamTracks(playbackStreamRef.current);

    const audioEl = audioRef.current;
    if (audioEl) {
      if (enabled && audioStreamRef.current) {
        audioEl.srcObject = audioStreamRef.current;
        audioEl.muted = mutedRef.current;
        audioEl.volume = mutedRef.current ? 0 : volumeRef.current;
      } else {
        audioEl.pause();
        audioEl.srcObject = null;
      }
    }
  };

  useEffect(() => {
    debugRef.current = debug;
  }, [debug]);

  useEffect(() => {
    mutedRef.current = muted;
  }, [muted]);

  // Monitor and reduce latency for live streams
  useEffect(() => {
    if (!streamOptSettings.enableLatencyMonitoring) return;
    
    const videoEl = videoRef.current;
    if (!videoEl) return;

    const checkLatency = () => {
      if (!videoEl.srcObject || videoEl.paused) return;

      // For live WebRTC streams, check if buffered time is too far ahead
      if (videoEl.buffered.length > 0) {
        const bufferedEnd = videoEl.buffered.end(videoEl.buffered.length - 1);
        const currentTime = videoEl.currentTime;
        const latency = bufferedEnd - currentTime;

        // If latency > configured threshold, skip to live edge
        if (latency > streamOptSettings.maxBufferedLatency!) {
          console.log(`[Latency] Detected ${latency.toFixed(2)}s lag (max ${streamOptSettings.maxBufferedLatency}s), jumping to live edge`);
          videoEl.currentTime = bufferedEnd - 0.1; // Leave small buffer
        }
      }
    };

    // Check at configured interval (convert seconds to ms)
    const latencyCheckInterval = window.setInterval(checkLatency, streamOptSettings.latencyCheckInterval! * 1000);

    return () => {
      if (latencyCheckInterval) {
        clearInterval(latencyCheckInterval);
      }
    };
  }, [streamOptSettings]);

  useEffect(() => {
    onVideoRefRef.current = onVideoRef;
  }, [onVideoRef]);

  useEffect(() => {
    statsCallbackRef.current = onStatsUpdate;
  }, [onStatsUpdate]);

  useEffect(() => {
    isPausedRef.current = isPaused;

    const videoEl = videoRef.current;
    if (!videoEl) {
      return;
    }

    const stream = videoEl.srcObject instanceof MediaStream ? videoEl.srcObject : null;

    if (isPaused) {
      if (stream) {
        stream.getVideoTracks().forEach(track => {
          if (track.enabled) {
            track.enabled = false;
          }
        });
        stream.getAudioTracks().forEach(track => {
          if (track.enabled) {
            track.enabled = false;
          }
        });
      }

      if (!videoEl.paused) {
        try {
          videoEl.pause();
        } catch (err) {
          console.log(`[Playback:${streamName}] Pause request failed:`, err);
        }
      }

      console.log(`[Playback:${streamName}] Stream paused by controls`);
    } else {
      if (stream) {
        stream.getVideoTracks().forEach(track => {
          if (!track.enabled) {
            track.enabled = true;
          }
        });
        stream.getAudioTracks().forEach(track => {
          if (!track.enabled) {
            track.enabled = true;
          }
        });
      }

      const attemptResume = async () => {
        const hasMediaStream = videoEl.srcObject instanceof MediaStream;
        const hasSource = hasMediaStream || (typeof videoEl.src === 'string' && videoEl.src.length > 0);

        if (!hasSource) {
          console.log(`[Playback:${streamName}] Resume skipped: no media source attached yet`);
          return;
        }

        try {
          await videoEl.play();
          console.log(`[Playback:${streamName}] Playback resumed`);
        } catch (resumeError) {
          console.log(`[Playback:${streamName}] Resume play failed, retrying muted:`, resumeError);
          const priorMuted = videoEl.muted;
          videoEl.muted = true;
          try {
            await videoEl.play();
            console.log(`[Playback:${streamName}] Resume fallback succeeded with muted=true`);
          } catch (fallbackError) {
            console.warn(`[Playback:${streamName}] Resume fallback failed:`, fallbackError);
          } finally {
            videoEl.muted = priorMuted;
          }
        }
      };

      void attemptResume();
    }
  }, [isPaused, streamName]);

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Принудительная инициализация аудио при монтировании
  useEffect(() => {
    const forceAudioInit = async () => {
      await initializeGlobalAudio();
    };
    forceAudioInit();
  }, []);

  useEffect(() => {
    const loadWhepDescriptors = async () => {
      try {
        const response = await invoke<WhepEndpointDescriptor[]>('get_whep_endpoints');
        if (Array.isArray(response) && response.length > 0) {
          const normalized = response
            .map(entry => {
              const provider = isStreamingBackend(entry?.provider) ? entry.provider : 'go2rtc';
              const base = typeof entry?.base === 'string' ? entry.base.trim().replace(/\/$/, '') : '';
              if (!base) {
                return null;
              }
              return { base, provider } as WhepEndpointDescriptor;
            })
            .filter((entry): entry is WhepEndpointDescriptor => Boolean(entry));

          if (normalized.length > 0) {
            const deduped = normalized.reduce<WhepEndpointDescriptor[]>((acc, entry) => {
              const exists = acc.some(item => item.base === entry.base && item.provider === entry.provider);
              if (!exists) {
                acc.push(entry);
              }
              return acc;
            }, []);

            console.log('[VideoStreamPlayer] Loaded WHEP endpoints:', deduped);
            const go2rtcDescriptors = deduped.filter(item => item.provider === 'go2rtc');
            const preferredProvider: StreamingBackend = go2rtcDescriptors.length > 0 ? 'go2rtc' : deduped[0].provider;
            const orderedDescriptors = go2rtcDescriptors.length > 0
              ? [...go2rtcDescriptors, ...deduped.filter(item => item.provider !== 'go2rtc')]
              : deduped;
            setActiveStreamingProvider(preferredProvider);
            setWhepTargets(previous => (areDescriptorListsEqual(previous, orderedDescriptors) ? previous : orderedDescriptors));
            return;
          }
        }

        console.warn('[VideoStreamPlayer] Empty WHEP endpoint list returned, using defaults');
      } catch (error) {
        console.warn('[VideoStreamPlayer] Failed to load WHEP endpoints, using defaults', error);
      }

  const fallbackProvider = readStreamingProviderFallback();
  setActiveStreamingProvider(fallbackProvider);
  const fallbackDescriptors = buildFallbackDescriptors(fallbackProvider);
  setWhepTargets(previous => (areDescriptorListsEqual(previous, fallbackDescriptors) ? previous : fallbackDescriptors));
    };

    void loadWhepDescriptors();
  }, []);

  useEffect(() => {
    if (!audioRef.current && typeof window !== 'undefined') {
      const audioEl = document.createElement('audio');
      audioEl.autoplay = true;
      audioEl.hidden = true;
      audioEl.setAttribute('playsinline', 'true');
      audioEl.muted = muted;
      audioEl.volume = muted ? 0 : volumeRef.current;
      audioEl.addEventListener('playing', () => {
        console.info(`[VideoStreamPlayer] Hidden audio element playing for ${streamName}`);
      });
      audioEl.addEventListener('pause', () => {
        console.info(`[VideoStreamPlayer] Hidden audio element paused for ${streamName}`);
      });
      document.body.appendChild(audioEl);
      audioRef.current = audioEl;
      console.log(`[VideoStreamPlayer] 🔊 Created audio element for ${streamName}`);
    }

    return () => {
      if (audioRef.current) {
        console.log(`[VideoStreamPlayer] 🔇 Removing audio element for ${streamName}`);
        
        // Останавливаем все треки аудио элемента
        if (audioRef.current.srcObject instanceof MediaStream) {
          audioRef.current.srcObject.getTracks().forEach(track => {
            track.stop();
            console.log(`[VideoStreamPlayer] Stopped audio element track: ${track.id}`);
          });
        }
        
        audioRef.current.pause();
        audioRef.current.srcObject = null;
        audioRef.current.src = '';
        audioRef.current.load();
        audioRef.current.remove();
        audioRef.current = null;
      }
    };
  }, [streamName]);

  useEffect(() => {
    const element = videoRef.current;
    if (!element) {
      return;
    }

    element.muted = muted;
    element.defaultMuted = muted;
    element.volume = muted ? 0 : volumeRef.current;

    if (audioRef.current) {
      audioRef.current.muted = muted;
      audioRef.current.volume = muted ? 0 : volumeRef.current;
      // Don't remove srcObject when muting - just pause and mute
      // This allows quick unmute without re-establishing audio connection
      if (muted) {
        audioRef.current.pause();
      } else if (audioStreamRef.current) {
        // Restore srcObject if it was cleared before
        if (audioRef.current.srcObject !== audioStreamRef.current) {
          audioRef.current.srcObject = audioStreamRef.current;
        }
        // Try to play when unmuting
        audioRef.current.play().catch(() => undefined);
      }
    }

    applyAudioTrackState(!muted);

    console.info(
      `[VideoStreamPlayer] mute state for ${streamName} -> ${muted ? 'muted' : 'unmuted'}`,
    );

    if (!muted) {
      const resume = async () => {
        try {
          await element.play();
          element.muted = false;
          element.volume = volumeRef.current;
          if (audioRef.current) {
            audioRef.current.muted = false;
            audioRef.current.volume = volumeRef.current;
            await audioRef.current.play().catch(() => undefined);
          }
        } catch (err) {
          console.warn(
            `[VideoStreamPlayer] Playback retry after unmute failed: ${err instanceof Error ? err.message : String(err)}`,
          );
        }
      };
      void resume();
    }

  }, [muted, streamName]);

  useEffect(() => {
  console.log(`🎬 [VideoStreamPlayer] Starting component for stream: ${streamName}, HD: ${useHdQuality}, exact: ${useExactName}`);
  let isActive = true;

  failureCountRef.current = 0;
  if (retryTimerRef.current) {
    clearTimeout(retryTimerRef.current);
    retryTimerRef.current = null;
  }

    const prepareForAttempt = () => {
      if (!isActive) {
        return;
      }
      setIsConnecting(true);
      setErrorMessage(null);
      setConnectionMethod('');
      setShowConnectingOverlay(false);
      if (overlayTimerRef.current) {
        clearTimeout(overlayTimerRef.current);
        overlayTimerRef.current = null;
      }
      // Оптимизация: уменьшаем задержку для fastStart
      overlayTimerRef.current = setTimeout(() => {
        setShowConnectingOverlay(true);
      }, fastStart ? 50 : 180);
    };

    const clearStatsInterval = () => {
      if (statsIntervalRef.current) {
        clearInterval(statsIntervalRef.current);
        statsIntervalRef.current = null;
      }
      lastStatsRef.current = null;
    };

    const reportStatsUpdate = (stats: StreamPlaybackStats) => {
      if (statsCallbackRef.current) {
        statsCallbackRef.current(stats);
      }
    };

    const normalizeVariantName = (base: string, suffix: '_0' | '_1'): string => {
      if (base.endsWith(suffix)) {
        return base;
      }
      if (base.endsWith(suffix === '_0' ? '_1' : '_0')) {
        return `${base.slice(0, -2)}${suffix}`;
      }
      return `${base}${suffix}`;
    };

    let activeStreamName = streamName;
    let alternateStreamName: string | null = null;

    if (!useExactName) {
      if (useHdQuality) {
        activeStreamName = normalizeVariantName(streamName, '_0');
        alternateStreamName = normalizeVariantName(streamName, '_1');
      } else {
        activeStreamName = normalizeVariantName(streamName, '_1');
        alternateStreamName = normalizeVariantName(streamName, '_0');
      }
    }

    console.log(`[VideoStreamPlayer] Initializing: streamName="${streamName}", useHdQuality=${useHdQuality}, activeStreamName="${activeStreamName}", alternateStreamName="${alternateStreamName}"`);

    const name = sanitizeName(activeStreamName);
    const alternateName = alternateStreamName ? sanitizeName(alternateStreamName) : null;
    const encodedName = encodeURIComponent(name);
    const descriptors = whepTargets.length > 0
      ? whepTargets
      : buildFallbackDescriptors(activeStreamingProvider);

    const prefersGo2rtc = descriptors.some(descriptor => descriptor.provider === 'go2rtc');
    const descriptorSource = (() => {
      if (prefersGo2rtc) {
        const onlyGo2rtc = descriptors.filter(descriptor => descriptor.provider === 'go2rtc');
        if (onlyGo2rtc.length > 0) {
          return onlyGo2rtc;
        }
        return buildFallbackDescriptors('go2rtc');
      }
      if (descriptors.length > 0) {
        return descriptors;
      }
      return buildFallbackDescriptors(activeStreamingProvider);
    })();

    const whepEndpointSet = new Set<string>();
    const go2rtcBases = descriptorSource
      .filter(descriptor => descriptor.provider === 'go2rtc')
      .map(descriptor => descriptor.base);
    
    // КРИТИЧНО: Используем только базовое имя стрима без модификаторов
    // Это позволяет всем ячейкам использовать ОДИН producer в Go2RTC
    // вместо создания отдельного producer для каждой ячейки
    const go2rtcWarmCandidates = new Set<string>([
      name,
    ]);

    if (alternateName) {
      go2rtcWarmCandidates.add(alternateName);
    }

    descriptorSource.forEach(descriptor => {
      const normalizedBase = descriptor.base.endsWith('/')
        ? descriptor.base.slice(0, -1)
        : descriptor.base;

      if (!normalizedBase) {
        return;
      }

      // КРИТИЧНО: Используем только базовое имя стрима для Go2RTC
      // Без #audio=opus и других модификаторов, чтобы избежать множественных RTSP соединений
      const querySources = descriptor.provider === 'go2rtc'
        ? [encodedName]
        : [encodedName];

      querySources.forEach(srcValue => {
        [
          `${normalizedBase}/api/webrtc?src=${srcValue}`,
          `${normalizedBase}/api/webrtc?src=${srcValue}&dst=whep`,
          `${normalizedBase}/api/webrtc?dst=whep&src=${srcValue}`,
        ].forEach(url => whepEndpointSet.add(url));
      });

      if (descriptor.provider !== 'go2rtc') {
        [
          `${normalizedBase}/whep/${encodedName}`,
          `${normalizedBase}/${encodedName}/whep`,
        ].forEach(url => whepEndpointSet.add(url));
      }
    });

    if (whepEndpointSet.size === 0) {
      buildFallbackDescriptors(prefersGo2rtc ? 'go2rtc' : activeStreamingProvider).forEach(descriptor => {
        const normalizedBase = descriptor.base.endsWith('/')
          ? descriptor.base.slice(0, -1)
          : descriptor.base;

        if (!normalizedBase) {
          return;
        }

        [
          `${normalizedBase}/whep/${encodedName}`,
          `${normalizedBase}/${encodedName}/whep`,
          `${normalizedBase}/api/webrtc?src=${encodedName}`,
          `${normalizedBase}/api/webrtc?src=${encodedName}&dst=whep`,
          `${normalizedBase}/api/webrtc?dst=whep&src=${encodedName}`,
        ].forEach(url => whepEndpointSet.add(url));
      });
    }

    const whepEndpoints = Array.from(whepEndpointSet);

    // Always log for debugging
    console.log(`[Player:${name}] === STARTING STREAM ===`);
    console.log(`[Player:${name}] Original streamName: ${streamName}`);
    console.log(`[Player:${name}] Final activeStreamName: ${activeStreamName}`);
    console.log(`[Player:${name}] Streaming provider: ${descriptorSource[0]?.provider ?? 'unknown'}`);
    console.log(`[Player:${name}] WHEP endpoints:`, whepEndpoints);

    const logDebug = (msg: string) => {
      if (!debugRef.current) {
        return;
      }
      console.log(`[Player:${name}]`, msg);
    };
    
    logDebug(`=== STARTING STREAM ===`);
    logDebug(`Original streamName: ${streamName}`);
    logDebug(`Final activeStreamName: ${activeStreamName}`);
    logDebug(`Streaming provider: ${descriptorSource[0]?.provider ?? 'unknown'}`);
    logDebug(`WHEP endpoints: ${whepEndpoints.join(', ')}`);

    const cleanup = () => {
      console.log(`[VideoStreamPlayer] 🧹 CLEANUP called for ${streamName}, statsDisplayName=${statsDisplayName}`);
      
      if (retryTimerRef.current) {
        clearTimeout(retryTimerRef.current);
        retryTimerRef.current = null;
      }
      if (overlayTimerRef.current) {
        clearTimeout(overlayTimerRef.current);
        overlayTimerRef.current = null;
      }
      activeStartAttemptRef.current = null;
      clearStatsInterval();
      
      // Clear WebRTC stats interval (separate from player stats)
      if (webrtcStatsIntervalRef.current) {
        clearInterval(webrtcStatsIntervalRef.current);
        webrtcStatsIntervalRef.current = null;
      }
      
      if (statsCallbackRef.current) {
        statsCallbackRef.current({});
      }
      
      // Stop WebRTC Stats Collector
      if (webrtcStatsCollectorRef.current) {
        console.log(`[VideoStreamPlayer] Stopping WebRTC Stats Collector for ${statsDisplayName || streamName}`);
        webrtcStatsCollectorRef.current.stop();
        webrtcStatsCollectorRef.current.dispose();
        webrtcStatsCollectorRef.current = null;
      }
      
      // Unregister from global context - use statsDisplayName if provided
      const displayName = statsDisplayName || streamName;
      webrtcStatsContext.unregisterStream(displayName);
      
      // КРИТИЧНО: Сначала останавливаем все MediaStream треки
      if (audioStreamRef.current) {
        console.log(`[VideoStreamPlayer] 🔇 Stopping audio stream tracks for ${streamName}`);
        audioStreamRef.current.getTracks().forEach(track => {
          track.stop();
          console.log(`[VideoStreamPlayer] Stopped audio track: ${track.id}`);
        });
        audioStreamRef.current = null;
      }
      
      if (playbackStreamRef.current) {
        console.log(`[VideoStreamPlayer] 🎬 Stopping playback stream tracks for ${streamName}`);
        playbackStreamRef.current.getTracks().forEach(track => {
          track.stop();
          console.log(`[VideoStreamPlayer] Stopped playback track: ${track.kind} ${track.id}`);
        });
        playbackStreamRef.current = null;
      }
      
      if (videoRef.current) {
        console.log(`[VideoStreamPlayer] 📹 Cleaning up video element for ${streamName}`);
        videoRef.current.pause();
        
        if (videoRef.current.srcObject && videoRef.current.srcObject instanceof MediaStream) {
          const stream = videoRef.current.srcObject;
          stream.getTracks().forEach(track => {
            track.stop();
            console.log(`[VideoStreamPlayer] Stopped video track: ${track.kind} ${track.id}`);
          });
        }
        
        videoRef.current.srcObject = null;
        videoRef.current.src = '';
        videoRef.current.load();
      }
      
      // Закрываем HLS
      if (hlsRef.current) {
        console.log(`[VideoStreamPlayer] Destroying HLS for ${streamName}`);
        hlsRef.current.destroy();
        hlsRef.current = null;
      }
      
      // КРИТИЧНО: Закрываем WebRTC соединение после остановки треков
      if (pcRef.current) {
        console.log(`[VideoStreamPlayer] 🔌 Closing RTCPeerConnection for ${streamName}, state=${pcRef.current.connectionState}`);
        
        // Закрываем все receivers
        pcRef.current.getReceivers().forEach(receiver => {
          const track = receiver.track;
          if (track) {
            track.stop();
            console.log(`[VideoStreamPlayer] Stopped receiver track: ${track.kind} ${track.id}`);
          }
        });
        
        // Закрываем все senders (на всякий случай)
        pcRef.current.getSenders().forEach(sender => {
          const track = sender.track;
          if (track) {
            track.stop();
            console.log(`[VideoStreamPlayer] Stopped sender track: ${track.kind} ${track.id}`);
          }
        });
        
        pcRef.current.close();
        console.log(`[VideoStreamPlayer] RTCPeerConnection closed for ${streamName}`);
        pcRef.current = null;
      }
      
      // Очищаем аудио элемент
      if (audioRef.current) {
        console.log(`[VideoStreamPlayer] 🔊 Cleaning up audio element for ${streamName}`);
        audioRef.current.pause();
        audioRef.current.srcObject = null;
        audioRef.current.src = '';
      }
      
      onVideoRefRef.current?.(null);
      setShowConnectingOverlay(false);
    };

    const hideConnectingOverlay = () => {
      if (overlayTimerRef.current) {
        clearTimeout(overlayTimerRef.current);
        overlayTimerRef.current = null;
      }
      setShowConnectingOverlay(false);
    };

    const scheduleRetry = (delayMs: number) => {
      if (!isActive || retryTimerRef.current) {
        return;
      }

      const safeDelay = Math.max(300, delayMs);
      retryTimerRef.current = setTimeout(() => {
        retryTimerRef.current = null;
        if (!isActive) {
          return;
        }
        void startStream();
      }, safeDelay);
    };

    const restartAfterDisconnect = (reason: string) => {
      if (!isActive) {
        return;
      }
      console.warn(`[VideoStreamPlayer] Connection lost for ${name}: ${reason}. Restarting...`);
      cleanup();
      scheduleRetry(800);
    };

    const startStatsMonitoring = () => {
      if (!pcRef.current || !statsCallbackRef.current) {
        return;
      }

      type InboundVideoStats = {
        type?: string;
        isRemote?: boolean;
        kind?: string;
        mediaType?: string;
        timestamp?: number;
        bytesReceived?: number;
        framesDecoded?: number;
        codecId?: string;
        frameWidth?: number;
        frameHeight?: number;
      };

      const isInboundVideoReport = (report: unknown): report is InboundVideoStats => {
        if (!report || typeof report !== 'object') {
          return false;
        }
        const candidate = report as InboundVideoStats;
        if (candidate.type !== 'inbound-rtp') {
          return false;
        }
        if (candidate.isRemote) {
          return false;
        }
        if (candidate.kind) {
          return candidate.kind === 'video';
        }
        return candidate.mediaType === 'video';
      };

      clearStatsInterval();

      statsIntervalRef.current = setInterval(async () => {
        const pc = pcRef.current;
        if (!pc || !statsCallbackRef.current) {
          return;
        }

        try {
          const statsReport = await pc.getStats();
          let inboundVideo: InboundVideoStats | null = null;

          statsReport.forEach(report => {
            if (isInboundVideoReport(report)) {
              inboundVideo = report;
            }
          });

          if (!inboundVideo) {
            return;
          }

          const videoStats = inboundVideo as InboundVideoStats;
          const now = videoStats.timestamp ?? performance.now();
          const bytesReceived = videoStats.bytesReceived ?? 0;
          const prev = lastStatsRef.current;
          let bitrateKbps: number | undefined;
          let frameRate: number | undefined;

          if (prev) {
            const deltaTime = (now - prev.timestamp) / 1000;
            if (deltaTime > 0) {
              const deltaBytes = bytesReceived - prev.bytesReceived;
              if (deltaBytes >= 0) {
                bitrateKbps = (deltaBytes * 8) / deltaTime / 1000;
              }

              if (
                typeof videoStats.framesDecoded === 'number' &&
                typeof prev.framesDecoded === 'number'
              ) {
                const deltaFrames = videoStats.framesDecoded - prev.framesDecoded;
                if (deltaFrames >= 0) {
                  frameRate = deltaFrames / deltaTime;
                }
              }
            }
          }

          lastStatsRef.current = {
            timestamp: now,
            bytesReceived,
            framesDecoded:
              typeof videoStats.framesDecoded === 'number'
                ? videoStats.framesDecoded
                : undefined,
          };

          let codec: string | undefined;
          if (videoStats.codecId) {
            const codecReport = statsReport.get(videoStats.codecId);
            if (codecReport && 'mimeType' in codecReport) {
              const mime = String(codecReport.mimeType);
              const parts = mime.split('/');
              codec = parts.length > 1 ? parts[1].toUpperCase() : mime.toUpperCase();
              if (codec === 'H264') {
                codec = 'H.264';
              } else if (codec === 'H265' || codec === 'H.265') {
                codec = 'H.265';
              }
            }
          }

          let width: number | undefined;
          let height: number | undefined;
          if (videoRef.current) {
            width = videoRef.current.videoWidth || undefined;
            height = videoRef.current.videoHeight || undefined;
          }

          reportStatsUpdate({
            bitrateKbps: bitrateKbps !== undefined ? Math.max(0, Math.round(bitrateKbps)) : undefined,
            frameRate: frameRate !== undefined ? Math.max(0, Math.round(frameRate)) : undefined,
            width,
            height,
            codec,
          });
        } catch (statsError) {
          console.warn(`[Stats:${name}] Failed to poll playback stats`, statsError);
        }
      }, 1000);
    };

    const startWebRTC = async () => {
      if (!videoRef.current) return false;
      clearStatsInterval();

      const startHlsFallback = async (
        rtspUrl: string,
        profile: SdpAudioProfile | null,
        options: { signalError?: boolean } = {},
      ): Promise<boolean> => {
        if (!videoRef.current) {
          return false;
        }

        const { signalError = true } = options;

        try {
          logDebug(
            `[HLS] Starting fallback for audio ${describeAudioProfile(profile)} via ${rtspUrl}`,
          );
          const hlsUrlRaw = await invoke<string>('play_recording', { filePath: rtspUrl });
          const hlsUrl = String(hlsUrlRaw);

          if (!isActive || !videoRef.current) {
            return false;
          }

          const videoEl = videoRef.current;

          if (pcRef.current) {
            pcRef.current.close();
            pcRef.current = null;
          }

          if (hlsRef.current) {
            hlsRef.current.destroy();
            hlsRef.current = null;
          }

          videoEl.pause();
          videoEl.srcObject = null;
          videoEl.src = '';

          reportStatsUpdate({});

          const finalizeReadyState = () => {
            if (!isActive) {
              return;
            }
            setConnectionMethod('HLS');
            hideConnectingOverlay();
            setIsConnecting(false);
            setErrorMessage(null);
          };

          if (Hls.isSupported()) {
            const hls = new Hls({ lowLatencyMode: true, enableWorker: true, backBufferLength: 60 });
            hlsRef.current = hls;

            hls.on(Hls.Events.ERROR, (_event, data) => {
              const errorData = data as ErrorData;
              if (errorData.fatal) {
                logDebug(`[HLS] Fatal error: ${errorData.type}`);
                if (onError) {
                  onError(new Error(`HLS error: ${errorData.type}`));
                }
              } else {
                logDebug(`[HLS] Non-fatal error: ${errorData.type}`);
              }
            });

            hls.on(Hls.Events.MANIFEST_PARSED, () => {
              const element = videoRef.current;
              if (!element) {
                return;
              }
              element
                .play()
                .then(() => {
                  if (!mutedRef.current) {
                    element.muted = false;
                  }
                })
                .catch(err => {
                  logDebug(
                    `[HLS] Autoplay failed: ${err instanceof Error ? err.message : String(err)}`,
                  );
                });
            });

            hls.attachMedia(videoEl);
            hls.loadSource(hlsUrl);
            videoEl.muted = true;
            finalizeReadyState();
          } else if (videoEl.canPlayType('application/vnd.apple.mpegurl')) {
            videoEl.src = hlsUrl;
            videoEl.muted = true;
            finalizeReadyState();
            videoEl
              .play()
              .then(() => {
                if (!mutedRef.current) {
                  videoEl.muted = false;
                }
              })
              .catch(err => {
                logDebug(
                  `[HLS] Native autoplay failed: ${err instanceof Error ? err.message : String(err)}`,
                );
              });
          } else {
            throw new Error('HLS playback is not supported in this environment');
          }

          if (!mutedRef.current) {
            setTimeout(() => {
              if (videoRef.current) {
                videoRef.current.muted = false;
              }
            }, 400);
          }

          return true;
        } catch (error) {
          const message = error instanceof Error ? error.message : String(error);
          logDebug(`[HLS] Fallback failed: ${message}`);
          if (signalError) {
            setErrorMessage('Не удалось запустить HLS fallback');
            if (onError) {
              onError(error instanceof Error ? error : new Error(message));
            }
          }
          return false;
        }
      };

      let uniqueFallbackRtspUrls: string[] = [];
      let handshakeAudioProfile: SdpAudioProfile | null = null;

      try {
        logDebug('Starting WebRTC connection attempt');

        // Принудительная инициализация AudioContext перед созданием WebRTC соединения
        await initializeGlobalAudio();

        const fallbackRtspUrlCandidates: string[] = [
          `rtsp://127.0.0.1:8554/${name}`,
          `rtsp://localhost:8554/${name}`,
        ];

        if (isTauriAvailable()) {
          try {
            const resolvedSource = await resolveStreamSource(name);
            if (resolvedSource) {
              if (debugRef.current) {
                logDebug(`[RTSP] Resolved source for ${name}: ${resolvedSource}`);
              }
              fallbackRtspUrlCandidates.push(resolvedSource);
            }
          } catch (resolveError) {
            if (debugRef.current) {
              const message = resolveError instanceof Error ? resolveError.message : String(resolveError);
              logDebug(`[RTSP] Failed to resolve source for ${name}: ${message}`);
            }
          }
        }

  uniqueFallbackRtspUrls = Array.from(new Set(fallbackRtspUrlCandidates));

        type FallbackAttemptResult = 'none' | 'succeeded' | 'failed';
        const attemptFallbackForProfile = async (
          profile: SdpAudioProfile | null,
          reasonLabel: string,
          options: { signalError?: boolean; allowTranscode?: boolean } = {},
        ): Promise<FallbackAttemptResult> => {
          const { signalError: shouldSignalError = true, allowTranscode = false } = options;

          if (!profile) {
            logDebug(`[${reasonLabel}] No SDP audio profile available for compatibility check`);
            return 'none';
          }

          const supported = isWebRtcAudioProfileSupported(profile);
          if (supported) {
            logDebug(`[${reasonLabel}] Audio profile ${describeAudioProfile(profile)} is WebRTC-compatible`);
            return 'none';
          }

          if (allowTranscode && isAudioProfileTranscodable(profile)) {
            logDebug(
              `[${reasonLabel}] Audio profile ${describeAudioProfile(profile)} not natively supported, relying on go2rtc audio bridging`,
            );
            return 'none';
          }

          if (uniqueFallbackRtspUrls.length === 0) {
            logDebug(
              `[${reasonLabel}] Audio profile ${describeAudioProfile(profile)} unsupported, but no RTSP fallback URL is available`,
            );
            return 'none';
          }

          const profileDescription = describeAudioProfile(profile);
          for (let index = 0; index < uniqueFallbackRtspUrls.length; index += 1) {
            const fallbackUrl = uniqueFallbackRtspUrls[index];
            const isLastCandidate = index === uniqueFallbackRtspUrls.length - 1;
            console.warn(
              `[VideoStreamPlayer] ${reasonLabel} audio profile ${profileDescription} unsupported for WebRTC, switching to HLS via ${fallbackUrl}`,
            );
            logDebug(
              `[${reasonLabel}] Audio profile ${profileDescription} is not WebRTC-compatible, attempting HLS via ${fallbackUrl}`,
            );
            const fallbackSucceeded = await startHlsFallback(fallbackUrl, profile, {
              signalError: isLastCandidate && shouldSignalError,
            });
            if (fallbackSucceeded) {
              return 'succeeded';
            }
            console.error(`[VideoStreamPlayer] ${reasonLabel} HLS fallback failed for ${fallbackUrl}`);
            logDebug(`[${reasonLabel}] HLS fallback failed for ${fallbackUrl}`);
          }

          return 'failed';
        };

        let handshakeLog: string[] = [];
        let handshakeAttempted = false;
        const shouldRunRtspHandshake = !prefersGo2rtc && isTauriAvailable() && uniqueFallbackRtspUrls.length > 0;

        if (!shouldRunRtspHandshake && prefersGo2rtc && debugRef.current) {
          logDebug('Skipping RTSP probe for go2rtc provider to minimise start latency');
        }

        if (shouldRunRtspHandshake) {
          const handshakeTarget = uniqueFallbackRtspUrls[0];
          const transportPreference = useHdQuality ? 'tcp' : 'udp';
          try {
            handshakeAttempted = true;
            const handshake = await performRtspHandshake({
              url: handshakeTarget,
              transport: transportPreference,
              includeAudio: true,
              timeoutMs: 1800,
            });

            if (handshake?.sdp) {
              handshakeAudioProfile = parseSdpAudioProfile(handshake.sdp);
            }
            if (handshake?.log?.length) {
              handshakeLog = handshake.log;
            }
          } catch (probeError) {
            if (debugRef.current) {
              const message = probeError instanceof Error ? probeError.message : String(probeError);
              logDebug(`[RTSP] Handshake probe failed: ${message}`);
            }
          }
        }

        if (handshakeAudioProfile) {
          const profileDescription = describeAudioProfile(handshakeAudioProfile);
          console.info(`[VideoStreamPlayer] RTSP handshake audio profile: ${profileDescription}`);
          logDebug(`[RTSP] Handshake audio profile: ${profileDescription}`);
          if (handshakeLog.length > 0 && debugRef.current) {
            logDebug(`[RTSP] Handshake log:\n${handshakeLog.join('\n')}`);
          }
          const handshakeFallback = await attemptFallbackForProfile(handshakeAudioProfile, 'RTSP', {
            allowTranscode: true,
          });
          if (handshakeFallback === 'succeeded') {
            return true;
          }
          if (handshakeFallback === 'failed') {
            setErrorMessage('Не удалось переключиться на HLS для аудио');
            return false;
          }
        } else if (handshakeAttempted) {
          console.info('[VideoStreamPlayer] RTSP handshake completed without SDP audio profile; proceeding with WebRTC audio.');
          if (debugRef.current && handshakeLog.length > 0) {
            logDebug(`[RTSP] Handshake log (no SDP audio):\n${handshakeLog.join('\n')}`);
          }
        }

        try {
          await invoke('start_go2rtc');
        } catch {
          // ignore
        }

        console.log(`🚀 [WebRTC:${name}] ===== CREATING PEER CONNECTION =====`);
        const pcConfig: RTCConfiguration = {
          iceServers: [],
          bundlePolicy: 'max-bundle',
          iceTransportPolicy: 'all',
          // Optimize for low latency
          rtcpMuxPolicy: 'require',
        };
        const pc = new RTCPeerConnection(pcConfig);
        pcRef.current = pc;
        
        // Set low-latency hints on video track when it arrives (if enabled in settings)
        if (streamOptSettings.enableLowLatency) {
          pc.addEventListener('track', (event) => {
            if (event.track.kind === 'video') {
              const receiver = event.receiver;
              // Try to minimize jitter buffer and playout delay
              if ('playoutDelayHint' in receiver) {
                try {
                  (receiver as any).playoutDelayHint = streamOptSettings.playoutDelayHint! / 1000.0; // Convert ms to seconds
                  console.log(`[WebRTC Low-Latency] Set playoutDelayHint = ${streamOptSettings.playoutDelayHint}ms`);
                } catch (e) {
                  console.log('[WebRTC] Could not set playoutDelayHint:', e);
                }
              }
              if ('jitterBufferTarget' in receiver) {
                try {
                  (receiver as any).jitterBufferTarget = streamOptSettings.jitterBufferTarget; // Already in ms
                  console.log(`[WebRTC Low-Latency] Set jitterBufferTarget = ${streamOptSettings.jitterBufferTarget}ms`);
                } catch (e) {
                  console.log('[WebRTC] Could not set jitterBufferTarget:', e);
                }
              }
            }
          });
        }
        
        console.log(`[WebRTC Stats Check] showWebRTCStats=${showWebRTCStats}, streamName=${streamName}, statsDisplayName=${statsDisplayName}`);
        
        // Initialize WebRTC Stats Collector
        if (showWebRTCStats) {
          console.log(`[WebRTC Stats] Initializing collector for ${statsDisplayName || streamName}`);
          if (!webrtcStatsCollectorRef.current) {
            webrtcStatsCollectorRef.current = new WebRTCStatsCollector();
          }
          webrtcStatsCollectorRef.current.setPeerConnection(pc);
          webrtcStatsCollectorRef.current.start(webrtcStatsUpdateInterval);
          
          // Collect initial stats immediately (async, non-blocking)
          void webrtcStatsCollectorRef.current.collect();
          
          console.log(`[WebRTC Stats] Starting interval for ${statsDisplayName || streamName}, interval=${webrtcStatsUpdateInterval}ms`);
          
          let tickCount = 0;
          
          // Update stats in UI
          const updateInterval = setInterval(() => {
            tickCount++;
            const latestStats = webrtcStatsCollectorRef.current?.getLatest();
            const quality = webrtcStatsCollectorRef.current?.getConnectionQuality() || 'unknown';
            
            console.log(`[WebRTC Stats] Interval tick #${tickCount} for ${statsDisplayName || streamName}: hasStats=${!!latestStats}, quality=${quality}`);
            
            if (latestStats) {
              // Register stats in global context - use statsDisplayName if provided
              const displayName = statsDisplayName || streamName;
              console.log(`[WebRTC Stats] Registering: ${displayName}`, latestStats);
              webrtcStatsContext.registerStream(displayName, latestStats, quality);
              
              // Also call onStatsUpdate callback if provided
              if (statsCallbackRef.current && latestStats.video) {
                // Normalize codec name
                let codec = latestStats.video.codec;
                if (codec === 'H264') {
                  codec = 'H.264';
                } else if (codec === 'H265' || codec === 'H.265') {
                  codec = 'H.265';
                }

                // Convert WebRTCStats to StreamPlaybackStats
                const playbackStats: StreamPlaybackStats = {
                  codec,
                  width: latestStats.video.resolution.width,
                  height: latestStats.video.resolution.height,
                  frameRate: Math.round(latestStats.video.frameRate),
                  bitrateKbps: Math.round(latestStats.video.bitrate),
                };
                statsCallbackRef.current(playbackStats);
              }
            }
          }, webrtcStatsUpdateInterval);
          
          // Store WebRTC stats interval ref for cleanup (separate from player stats interval)
          if (webrtcStatsIntervalRef.current) {
            clearInterval(webrtcStatsIntervalRef.current);
          }
          webrtcStatsIntervalRef.current = updateInterval;
        }
        
        const videoTransceiver = pc.addTransceiver('video', { direction: 'recvonly' });
        pc.addTransceiver('audio', { direction: 'recvonly' });

        try {
          const videoCapabilities = RTCRtpReceiver.getCapabilities?.('video');
          const codecs = videoCapabilities?.codecs ?? [];
          const hevcCodecs = codecs.filter(codec => {
            const mime = codec.mimeType?.toLowerCase?.() ?? '';
            return mime === 'video/h265' || mime === 'video/hevc';
          });

          if (hevcCodecs.length > 0) {
            const remainingCodecs = codecs.filter(codec => !hevcCodecs.includes(codec));
            videoTransceiver.setCodecPreferences?.([...hevcCodecs, ...remainingCodecs]);
            logDebug(`Applied HEVC codec preference (${hevcCodecs.length} variants)`);
          } else {
            logDebug('HEVC codec not available in browser capabilities');
          }
        } catch (prefError) {
          logDebug(`Failed to set codec preferences: ${prefError instanceof Error ? prefError.message : String(prefError)}`);
        }
        
        pc.onicecandidate = event => {
          if (event.candidate) {
            logDebug(`[ICE] New candidate: ${event.candidate.candidate}`);
          } else {
            logDebug('[ICE] Candidate gathering complete');
          }
        };

        pc.oniceconnectionstatechange = () => {
          const state = pc.iceConnectionState;
          logDebug(`ICE connection state: ${state}`);
          if (state === 'failed') {
            console.error(`[VideoStreamPlayer] ICE connection failed for ${name}`);
            // Dump candidates if possible
          }
          if (isActive && (state === 'connected' || state === 'completed')) {
            setConnectionMethod('WebRTC');
            hideConnectingOverlay();
            setIsConnecting(false);
            setErrorMessage(null);
            return;
          }
          if (!isActive || pcRef.current !== pc) {
            return;
          }
          if (state === 'failed' || state === 'disconnected') {
            clearStatsInterval();
            reportStatsUpdate({});
            restartAfterDisconnect(`ice=${state}`);
          }
        };
        
        pc.onconnectionstatechange = () => {
          const state = pc.connectionState;
          logDebug(`Connection state: ${state}`);
          if (isActive && state === 'connected') {
            setConnectionMethod('WebRTC');
            hideConnectingOverlay();
            setIsConnecting(false);
            setErrorMessage(null);
          }
          if (!isActive || pcRef.current !== pc) {
            return;
          }
          if (state === 'failed' || state === 'disconnected') {
            clearStatsInterval();
            reportStatsUpdate({});
            restartAfterDisconnect(`connection=${state}`);
            return;
          }
          if (state === 'closed') {
            clearStatsInterval();
            reportStatsUpdate({});
          }
        };

        const attemptResumeAudio = () => {
          const element = videoRef.current;
          if (!element || mutedRef.current || isPausedRef.current) {
            return;
          }
          element.muted = false;
          element.defaultMuted = false;
          element.volume = volumeRef.current;
          void element.play().catch(err => {
            logDebug(
              `Audio resume play() failed: ${err instanceof Error ? err.message : String(err)}`,
            );
          });
          if (audioRef.current) {
            audioRef.current.muted = false;
            audioRef.current.volume = volumeRef.current;
            const playAttempt = audioRef.current.play();
            if (playAttempt && typeof playAttempt.catch === 'function') {
              void playAttempt.catch(err => {
                console.warn(
                  `[VideoStreamPlayer] Hidden audio element resume play() failed for ${streamName}: ${err instanceof Error ? err.message : String(err)}`,
                );
              });
            }
          }
        };

        pc.ontrack = event => {
          if (!videoRef.current || !isActive) {
            console.log(`[VideoStreamPlayer] ⚠️ Ignoring ontrack event for ${streamName} (isActive=${isActive}, hasVideoRef=${!!videoRef.current})`);
            return;
          }

          console.log(`[VideoStreamPlayer] 📡 ontrack fired: kind=${event.track.kind}, id=${event.track.id}, readyState=${event.track.readyState}`);

          let incomingStream = event.streams[0] ?? null;
          let playbackTrackReference: MediaStreamTrack | null = null;

          if (!incomingStream) {
            // Некоторые версии go2rtc не добавляют MediaStream в список event.streams,
            // поэтому создаем совместимый контейнер вручную.
            if (!playbackStreamRef.current) {
              playbackStreamRef.current = new MediaStream();
            }
            incomingStream = playbackStreamRef.current;
            if (!incomingStream.getTracks().includes(event.track)) {
              incomingStream.addTrack(event.track);
              console.debug(
                `[VideoStreamPlayer] Attached track ${event.track.kind} via manual MediaStream for ${streamName}`,
              );
            }
            playbackTrackReference = event.track;
          } else if (!playbackStreamRef.current) {
            playbackStreamRef.current = incomingStream;
          }
          
          // Добавляем обработчик onended для видео трека
          if (event.track.kind === 'video') {
            event.track.onended = () => {
              console.info(`[VideoStreamPlayer] 🛑 Video track ended for ${streamName}`);
            };
          }

          if (event.track.kind === 'audio') {
            // Allow audio for the currently active (visible) stream
            // muted state controls whether audio should play
            const allowAudio = !mutedRef.current;
            event.track.enabled = allowAudio;
            
            console.info(`[VideoStreamPlayer] 🔊 Audio track received for ${streamName} (quality=${useHdQuality ? 'HD' : 'SD'}, muted=${mutedRef.current}, allowAudio=${allowAudio})`);
            
            // Try to unmute the track explicitly if browser muted it
            // This is required because some browsers auto-mute WebRTC tracks
            if (event.track.muted && allowAudio) {
              console.warn(`[VideoStreamPlayer] ⚠️ Audio track is muted by browser, attempting to unmute...`);
              // Note: We can't directly unmute MediaStreamTrack, but setting enabled=true helps
              // The browser will unmute it after user interaction
            }
            
            console.info(
              `[VideoStreamPlayer] 🔊 Audio track attached for ${streamName} (muted=${event.track.muted}, enabled=${event.track.enabled}, readyState=${event.track.readyState})`,
            );
            event.track.onmute = () => {
              console.info(`[VideoStreamPlayer] Audio track muted for ${streamName}`);
            };
            event.track.onunmute = () => {
              console.info(`[VideoStreamPlayer] Audio track unmuted for ${streamName}`);
              if (!mutedRef.current) {
                attemptResumeAudio();
              }
            };
            event.track.onended = () => {
              console.info(`[VideoStreamPlayer] 🛑 Audio track ended for ${streamName}`);
              if (audioStreamRef.current) {
                audioStreamRef.current.getTracks().forEach(track => track.stop());
                audioStreamRef.current = null;
              }
              if (audioRef.current) {
                audioRef.current.pause();
                audioRef.current.srcObject = null;
              }
              if (playbackStreamRef.current && playbackTrackReference) {
                try {
                  playbackStreamRef.current.removeTrack(playbackTrackReference);
                } catch (removeError) {
                  console.debug(
                    `[VideoStreamPlayer] Failed to remove playback track for ${streamName}: ${removeError instanceof Error ? removeError.message : String(removeError)}`,
                  );
                }
              }
            };
            
            const audioEl = audioRef.current;
            if (audioEl) {
              // КРИТИЧНО: Останавливаем предыдущие аудио треки перед добавлением нового
              if (audioStreamRef.current) {
                console.log(`[VideoStreamPlayer] 🔄 Replacing existing audio stream for ${streamName}`);
                audioStreamRef.current.getTracks().forEach(track => {
                  track.stop();
                  console.log(`[VideoStreamPlayer] Stopped old audio track: ${track.id}`);
                });
                audioStreamRef.current = null;
              }
              
              const clonedTrack = event.track.clone();
              clonedTrack.enabled = allowAudio;
              const audioOnlyStream = new MediaStream([clonedTrack]);
              audioStreamRef.current = audioOnlyStream;
              if (!playbackStreamRef.current) {
                playbackStreamRef.current = new MediaStream();
              }
              if (!playbackStreamRef.current.getAudioTracks().includes(clonedTrack)) {
                playbackStreamRef.current.addTrack(clonedTrack);
              }
              playbackTrackReference = clonedTrack;
              if (mutedRef.current) {
                console.log(`[VideoStreamPlayer] 🔇 Audio muted for ${streamName}, pausing audio element`);
                audioEl.pause();
                audioEl.srcObject = null;
              } else {
                console.log(`[VideoStreamPlayer] 🔊 Setting audio srcObject and playing for ${streamName}`);
                audioEl.srcObject = audioOnlyStream;
                audioEl.muted = false;
                audioEl.volume = volumeRef.current;
                const playAttempt = audioEl.play();
                if (playAttempt && typeof playAttempt.catch === 'function') {
                  void playAttempt.catch(err => {
                    console.warn(
                      `[VideoStreamPlayer] Hidden audio element play() failed for ${streamName}: ${err instanceof Error ? err.message : String(err)}`,
                    );
                  });
                }
              }
            }
            // Try to resume audio even if track is initially muted by browser
            // The browser may unmute it after user interaction
            if (!mutedRef.current) {
              attemptResumeAudio();
            }
          }

          const element = videoRef.current;
          
          // КРИТИЧНО: Очищаем старый srcObject перед установкой нового
          if (element.srcObject !== incomingStream) {
            if (element.srcObject && element.srcObject instanceof MediaStream) {
              const oldStream = element.srcObject;
              console.log(`[VideoStreamPlayer] 🔄 Replacing video srcObject for ${streamName}`);
              oldStream.getTracks().forEach(track => {
                track.stop();
                console.log(`[VideoStreamPlayer] Stopped old video srcObject track: ${track.kind} ${track.id}`);
              });
            }
            element.srcObject = incomingStream;
            console.log(`[VideoStreamPlayer] ✅ Set new video srcObject for ${streamName}`);
          }

          if (event.track.kind === 'video') {
            console.info(`[VideoStreamPlayer] 📹 Video track attached for ${streamName} (readyState=${event.track.readyState})`);
            startStatsMonitoring();
            setConnectionMethod('WebRTC');
            hideConnectingOverlay();
            setIsConnecting(false);
            setErrorMessage(null);
          }

          const wantsAudio = !mutedRef.current;
          element.muted = true;

          const ensurePlayback = async () => {
            try {
              await element.play();
              if (wantsAudio) {
                element.muted = false;
                element.defaultMuted = false;
                element.volume = volumeRef.current;
              }
            } catch (playError) {
              if (wantsAudio) {
                logDebug(`Video playback failed (will remain muted): ${playError instanceof Error ? playError.message : String(playError)}`);
              }
            }

            if (isPausedRef.current) {
              try {
                if (!element.paused) {
                  await element.pause();
                }
              } catch (pauseError) {
                logDebug(`Pause after stream attach failed: ${pauseError instanceof Error ? pauseError.message : String(pauseError)}`);
              }
              incomingStream.getTracks().forEach(track => {
                track.enabled = false;
              });
            }
          };

          void ensurePlayback();
        };

        // Request both audio and video in the offer
        const offer = await pc.createOffer({
          offerToReceiveAudio: true,
          offerToReceiveVideo: true
        });
        await pc.setLocalDescription(offer);
        // Оптимизация: для fastStart используем агрессивный timeout
        await waitForIceGatheringComplete(
          pc, 
          fastStart ? 500 : 1200, 
          msg => logDebug(`[ICE] ${msg}`)
        );

        const localDescription = pc.localDescription ?? offer;
        if (!localDescription?.sdp) throw new Error('Failed to create SDP offer');
        if (debugRef.current) {
          logDebug(`Offer SDP preview includes HEVC: ${/H265|HEVC/i.test(localDescription.sdp)}`);
        }
        
        logDebug(`SDP offer created (${localDescription.sdp.length} bytes)`);

        // Check for HEVC support (Hardware decoding)
        // This helps the backend decide whether to transcode to H.264
        const isHevcSupported = (() => {
          try {
            if (typeof MediaSource !== 'undefined' && typeof MediaSource.isTypeSupported === 'function') {
              return MediaSource.isTypeSupported('video/mp4; codecs="hev1.1.6.L93.B0"') || 
                     MediaSource.isTypeSupported('video/mp4; codecs="hvc1.1.6.L93.B0"');
            }
            return false;
          } catch (e) {
            return false;
          }
        })();

        logDebug(`Client HEVC support detected: ${isHevcSupported}`);

        // Try WHEP via Tauri command
        try {
          const whepPayload: Record<string, unknown> = {
            path: name,
            offerSdp: localDescription.sdp,
            hevcSupported: isHevcSupported && !forceH264,
          };

          const answer = await invoke('whep_play', whepPayload);
          const answerSdp = String(answer);
          logDebug(`Received SDP answer from Tauri (${answerSdp.length} bytes)`);
          const answerProfile = parseSdpAudioProfile(answerSdp);
          if (answerProfile) {
            console.info(`[VideoStreamPlayer] WebRTC answer audio profile: ${describeAudioProfile(answerProfile)}`);
          } else {
            console.info('[VideoStreamPlayer] WebRTC answer does not include audio profile information');
          }
          const answerFallback = await attemptFallbackForProfile(answerProfile, 'WebRTC');
          if (answerFallback === 'succeeded') {
            return true;
          }
          if (answerFallback === 'failed') {
            setErrorMessage('Не удалось переключиться на HLS для аудио');
            return false;
          }
          await pc.setRemoteDescription({ type: 'answer', sdp: answerSdp });
          pc.getReceivers().forEach(receiver => {
            const track = receiver.track ?? null;
            const kind = track?.kind ?? 'unknown';
            const state = track?.readyState ?? 'absent';
            console.info(
              `[VideoStreamPlayer] Receiver ${kind} track state=${state} muted=${track?.muted ?? 'n/a'}`,
            );

            // Watchdog for H.265 fallback
            if (kind === 'video' && track && isHevcSupported && !forceH264) {
              const checkMuted = () => {
                if (isActive && track.muted && !forceH264) {
                  console.warn(`[VideoStreamPlayer] Video track still muted after 3s, forcing H.264 fallback for ${name}`);
                  setForceH264(true);
                }
              };

              if (track.muted) {
                console.log(`[VideoStreamPlayer] Video track is muted, starting fallback timer for ${name}`);
                setTimeout(checkMuted, 3000);
              }
              
              track.onmute = () => {
                console.log(`[VideoStreamPlayer] Video track muted event for ${name}`);
                setTimeout(checkMuted, 3000);
              };
            }
          });
          return true;
        } catch (err) {
          logDebug(`Tauri whep_play failed: ${err instanceof Error ? err.message : JSON.stringify(err)}`);
        }

        // Direct fetch attempts as fallback
        let lastFetchError: unknown = null;
        for (const endpoint of whepEndpoints) {
          try {
            logDebug(`Trying direct WHEP endpoint: ${endpoint}`);
            const resp = await fetch(endpoint, {
              method: 'POST',
              headers: { 'Content-Type': 'application/sdp' },
              body: localDescription.sdp
            });

            if (resp.status === 200 || resp.status === 201) {
              const answerSdp = await resp.text();
              logDebug(`Got SDP answer via ${endpoint} (${answerSdp.length} bytes)`);
              const answerProfile = parseSdpAudioProfile(answerSdp);
              if (answerProfile) {
                console.info(`[VideoStreamPlayer] WebRTC answer audio profile: ${describeAudioProfile(answerProfile)}`);
              } else {
                console.info('[VideoStreamPlayer] WebRTC answer does not include audio profile information');
              }
              const answerFallback = await attemptFallbackForProfile(
                answerProfile,
                'WebRTC',
              );
              if (answerFallback === 'succeeded') {
                return true;
              }
              if (answerFallback === 'failed') {
                setErrorMessage('Не удалось переключиться на HLS для аудио');
                return false;
              }
              await pc.setRemoteDescription({ type: 'answer', sdp: answerSdp });
              pc.getReceivers().forEach(receiver => {
                const track = receiver.track ?? null;
                const kind = track?.kind ?? 'unknown';
                const state = track?.readyState ?? 'absent';
                console.info(
                  `[VideoStreamPlayer] Receiver ${kind} track state=${state} muted=${track?.muted ?? 'n/a'}`,
                );
              });
              return true;
            }

            const errorBody = await resp.text();
            lastFetchError = new Error(`Endpoint ${endpoint} returned status ${resp.status}: ${errorBody}`);
            logDebug(`Endpoint ${endpoint} returned ${resp.status}: ${errorBody}`);
          } catch (fetchError) {
            lastFetchError = fetchError;
            logDebug(`Endpoint ${endpoint} failed: ${fetchError instanceof Error ? fetchError.message : String(fetchError)}`);
          }
        }

        throw lastFetchError instanceof Error
          ? lastFetchError
          : new Error(`All WebRTC endpoints failed`);
      } catch (e) {
        console.warn(`[VideoStreamPlayer] WebRTC start failed for ${name}: ${e instanceof Error ? e.message : String(e)}`);
        if (pcRef.current) {
          pcRef.current.close();
          pcRef.current = null;
        }

        if (uniqueFallbackRtspUrls.length > 0) {
          for (let index = 0; index < uniqueFallbackRtspUrls.length; index += 1) {
            const fallbackUrl = uniqueFallbackRtspUrls[index];
            const fallbackSucceeded = await startHlsFallback(
              fallbackUrl,
              handshakeAudioProfile,
              { signalError: index === uniqueFallbackRtspUrls.length - 1 }
            );

            if (fallbackSucceeded) {
              logDebug(`[WebRTC] Switched to HLS fallback via ${fallbackUrl}`);
              return true;
            }
          }
        }
      }
      return false;
    };

    async function startStream(): Promise<void> {
      if (!isActive) return;

      console.log(`[VideoStreamPlayer] 🔄 startStream() called for ${name} (${useHdQuality ? 'HD' : 'SD'})`);

      if (activeStartAttemptRef.current !== null) {
        if (debugRef.current) {
          logDebug('Start request ignored: previous attempt still in progress');
        }
        return;
      }

      const nextAttemptId = startAttemptCounterRef.current + 1;
      startAttemptCounterRef.current = nextAttemptId;
      activeStartAttemptRef.current = nextAttemptId;

      prepareForAttempt();

      if (onVideoRefRef.current && videoRef.current) {
        onVideoRefRef.current(videoRef.current);
      }

      const warmupGo2rtc = async () => {
        if (!prefersGo2rtc || go2rtcBases.length === 0) {
          return;
        }

        const warmBases = go2rtcBases.slice(0, 2);
        const warmSources = Array.from(go2rtcWarmCandidates).slice(0, 4);

        await Promise.allSettled(
          warmBases.flatMap(base =>
            warmSources.map((candidate: string) => {
              const controller = new AbortController();
              const timeoutId = setTimeout(() => controller.abort(), 900);
              const url = `${base}/api/streams?src=${encodeURIComponent(candidate)}`;
              return fetch(url, {
                method: 'GET',
                signal: controller.signal,
                cache: 'no-store',
              })
                .catch(() => undefined)
                .finally(() => clearTimeout(timeoutId));
            }),
          ),
        );
      };

      try {
        await warmupGo2rtc();
        const webrtcSuccess = await startWebRTC();
        if (webrtcSuccess) {
          failureCountRef.current = 0;
          return;
        }

        if (!isActive) {
          return;
        }

        failureCountRef.current += 1;
        const attemptNumber = failureCountRef.current;
        const retryDelay = Math.min(5000, 700 + attemptNumber * 600);

        console.warn(`[VideoStreamPlayer] WebRTC start failed for ${name} (attempt ${attemptNumber}), retrying in ${retryDelay}ms`);

        if (attemptNumber >= 5) {
          setErrorMessage('Не удалось подключиться к потоку');
          hideConnectingOverlay();
          setIsConnecting(false);
          return;
        }

        scheduleRetry(retryDelay);
      } finally {
        if (activeStartAttemptRef.current === nextAttemptId) {
          activeStartAttemptRef.current = null;
        }
      }
    }

    void startStream();

    return () => {
      isActive = false;
      cleanup();
    };
  }, [streamName, useHdQuality, useExactName, whepTargets, activeStreamingProvider, forceH264]);

  return (
    <div style={{ position: 'relative', width, height, ...style }} className={className}>
      <video
        ref={videoRef}
        controls={controls}
        muted={muted}
        autoPlay={autoPlay}
        preload={fastStart ? 'auto' : 'metadata'}
        playsInline
        disablePictureInPicture
        style={{ width: '100%', height: '100%', objectFit, backgroundColor: '#000', display: 'block' }}
        // Low-latency optimizations
        onLoadedMetadata={(e) => {
          const videoEl = e.target as HTMLVideoElement;
          // Try to minimize buffering
          try {
            // Set a very small playback buffer for live streams
            if ('setLatencyHint' in videoEl) {
              (videoEl as any).setLatencyHint(0.0);
            }
          } catch (err) {
            console.log('[Video] Could not set latency hint:', err);
          }
        }}
        onClick={() => {
          const videoEl = videoRef.current;
          if (videoEl) {
            void videoEl.play().catch(() => {
              /* ignore */
            });
            if (!mutedRef.current) {
              videoEl.muted = false;
            }
          }
          const audioEl = audioRef.current;
          if (audioEl) {
            audioEl.muted = false;
            audioEl.volume = volumeRef.current;
            const playAttempt = audioEl.play();
            if (playAttempt && typeof playAttempt.catch === 'function') {
              void playAttempt.catch(err => {
                console.warn(
                  `[VideoStreamPlayer] Hidden audio element resume play() failed for ${streamName}: ${err instanceof Error ? err.message : String(err)}`,
                );
              });
            }
          }
        }}
        onError={(e) => {
          const videoEl = e.target as HTMLVideoElement;
          const mediaErr = videoEl.error;
          const hasStream = videoEl.srcObject instanceof MediaStream;
          const hasUrlSource = typeof videoEl.src === 'string' && videoEl.src.length > 0;

          if (!hasStream && !hasUrlSource) {
            console.log(`Ignoring transient video error for ${streamName}: no source attached yet`);
            return;
          }
          
          if (
            mediaErr?.code === mediaErr?.MEDIA_ERR_SRC_NOT_SUPPORTED &&
            !hasStream &&
            (!hasUrlSource || mediaErr?.message?.includes('Empty src attribute'))
          ) {
            console.log(`Ignoring empty src error for ${streamName}`);
            return;
          }

          if (connectionMethod === 'WebRTC' || hasStream) {
            console.log(`Ignoring video error during active stream: ${mediaErr?.message}`);
            return;
          }
          
          const msg = mediaErr?.message || `code ${mediaErr?.code ?? 'n/a'}`;
          console.error(`Video error for ${streamName}:`, msg);
          setErrorMessage(`Video error: ${msg}`);
          onError?.(new Error(`Video error: ${msg}`));
        }}
      />

      {isConnecting && showConnectingOverlay && (
        <div style={{
          position: 'absolute', inset: 0, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center',
          backgroundColor: 'rgba(0,0,0,0.5)', color: '#fff', fontSize: 14, zIndex: 2
        }}>
          {errorMessage ?? 'Подключение к потоку...'}
          {debug && connectionMethod && (
            <div style={{ fontSize: 10, marginTop: 5 }}>Метод: {connectionMethod}</div>
          )}
        </div>
      )}

      {!isConnecting && errorMessage && !videoRef.current?.srcObject && (
        <div style={{
          position: 'absolute', inset: 0, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center',
          backgroundColor: 'rgba(0,0,0,0.7)', color: 'red', padding: 10, textAlign: 'center', fontSize: 14, zIndex: 2
        }}>
          Ошибка воспроизведения потока
          <div style={{ fontSize: 10, marginTop: 5 }}>{errorMessage}</div>
        </div>
      )}

      {/* Enhanced go2rtc features overlay - только Stream Monitor */}
      {showMonitor && (
        <>
          {/* Stream Monitor - top right */}
          <div style={{ position: 'absolute', top: 8, right: 8, zIndex: 3 }}>
            <StreamMonitor 
              streamName={streamName} 
              compact={monitorCompact !== false}
              updateInterval={2000}
            />
          </div>
        </>
      )}

    </div>
  );
};

export default VideoStreamPlayer;