/**
 * Enhanced Video Stream Hook
 * Advanced video streaming with multiple transport support and adaptive bitrate
 */

import { useState, useEffect, useRef, useCallback } from 'react';
import { getGo2RtcService } from '../services/go2rtc';
import type {
  Go2RtcTransportType,
  Go2RtcStreamInfo,
  TwoWayAudioConfig,
  Go2RtcStreamFilter,
} from '../global';
import Hls from 'hls.js';

interface UseEnhancedVideoStreamOptions {
  streamName: string;
  preferredTransport?: Go2RtcTransportType;
  enableAdaptiveBitrate?: boolean;
  enableMonitoring?: boolean;
  audioConfig?: TwoWayAudioConfig;
  filters?: Go2RtcStreamFilter;
  onTransportChange?: (transport: Go2RtcTransportType) => void;
  onStatsUpdate?: (stats: Go2RtcStreamInfo) => void;
  onError?: (error: Error) => void;
}

interface UseEnhancedVideoStreamResult {
  videoRef: React.RefObject<HTMLVideoElement | null>;
  currentTransport: Go2RtcTransportType | null;
  streamInfo: Go2RtcStreamInfo | null;
  isLoading: boolean;
  error: Error | null;
  switchTransport: (transport: Go2RtcTransportType) => Promise<void>;
  takeSnapshot: () => Promise<Blob | null>;
  reconnect: () => Promise<void>;
}

export function useEnhancedVideoStream(
  options: UseEnhancedVideoStreamOptions
): UseEnhancedVideoStreamResult {
  const {
    streamName,
    preferredTransport,
    enableAdaptiveBitrate = false,
    enableMonitoring = true,
    audioConfig,
    filters,
    onTransportChange,
    onStatsUpdate,
    onError,
  } = options;

  const videoRef = useRef<HTMLVideoElement>(null);
  const [currentTransport, setCurrentTransport] = useState<Go2RtcTransportType | null>(null);
  const [streamInfo, setStreamInfo] = useState<Go2RtcStreamInfo | null>(null);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState<Error | null>(null);

  const peerConnectionRef = useRef<RTCPeerConnection | null>(null);
  const hlsInstanceRef = useRef<Hls | null>(null);
  const monitorCleanupRef = useRef<(() => void) | null>(null);
  const lastBitrateCheckRef = useRef<number>(0);
  const consecutiveLowBitrateRef = useRef<number>(0);

  // Cleanup function
  const cleanup = useCallback(() => {
    // Stop monitoring
    if (monitorCleanupRef.current) {
      monitorCleanupRef.current();
      monitorCleanupRef.current = null;
    }

    // Close WebRTC connection
    if (peerConnectionRef.current) {
      peerConnectionRef.current.close();
      peerConnectionRef.current = null;
    }

    // Destroy HLS instance
    if (hlsInstanceRef.current) {
      hlsInstanceRef.current.destroy();
      hlsInstanceRef.current = null;
    }

    // Clear video element
    if (videoRef.current) {
      videoRef.current.srcObject = null;
      videoRef.current.src = '';
    }
  }, []);

  // WebRTC connection
  const connectWebRTC = useCallback(async () => {
    if (!videoRef.current) return;

    try {
      setIsLoading(true);
      const service = getGo2RtcService();
      
      const pc = await service.connectWebRTC(
        streamName,
        videoRef.current,
        audioConfig
      );

      peerConnectionRef.current = pc;
      setCurrentTransport('webrtc');
      setError(null);
      onTransportChange?.('webrtc');
      console.log('[EnhancedStream] WebRTC connected');
    } catch (err) {
      const error = err instanceof Error ? err : new Error(String(err));
      setError(error);
      onError?.(error);
      console.error('[EnhancedStream] WebRTC connection failed:', err);
      // Fallback to HLS
      await connectHLS();
    } finally {
      setIsLoading(false);
    }
  }, [streamName, audioConfig, onTransportChange, onError]);

  // HLS connection
  const connectHLS = useCallback(async () => {
    if (!videoRef.current) return;

    try {
      setIsLoading(true);
      const service = getGo2RtcService();
      const url = service.buildStreamUrl(streamName, 'hls', filters);

      if (Hls.isSupported()) {
        const hls = new Hls({
          enableWorker: true,
          lowLatencyMode: true,
          backBufferLength: 90,
        });

        hls.loadSource(url);
        hls.attachMedia(videoRef.current);

        hls.on(Hls.Events.ERROR, (event, data: any) => {
          if (data.fatal) {
            console.error('[EnhancedStream] HLS fatal error:', data);
            const error = new Error(`HLS error: ${data.details || 'unknown'}`);
            setError(error);
            onError?.(error);
            hls.destroy();
            hlsInstanceRef.current = null;
          }
        });

        hlsInstanceRef.current = hls;
        setCurrentTransport('hls');
        setError(null);
        onTransportChange?.('hls');
        console.log('[EnhancedStream] HLS connected');
      } else if (videoRef.current.canPlayType('application/vnd.apple.mpegurl')) {
        // Native HLS support (Safari)
        videoRef.current.src = url;
        setCurrentTransport('hls');
        setError(null);
        onTransportChange?.('hls');
        console.log('[EnhancedStream] Native HLS connected');
      } else {
        throw new Error('HLS not supported');
      }
    } catch (err) {
      const error = err instanceof Error ? err : new Error(String(err));
      setError(error);
      onError?.(error);
      console.error('[EnhancedStream] HLS connection failed:', err);
    } finally {
      setIsLoading(false);
    }
  }, [streamName, filters, onTransportChange, onError]);

  // MSE connection (for future implementation)
  const connectMSE = useCallback(async () => {
    console.warn('[EnhancedStream] MSE transport not yet implemented');
    // Fallback to HLS
    await connectHLS();
  }, [connectHLS]);

  // MJPEG connection
  const connectMJPEG = useCallback(async () => {
    if (!videoRef.current) return;

    try {
      setIsLoading(true);
      const service = getGo2RtcService();
      const url = service.buildStreamUrl(streamName, 'mjpeg', filters);
      
      videoRef.current.src = url;
      setCurrentTransport('mjpeg');
      setError(null);
      onTransportChange?.('mjpeg');
      console.log('[EnhancedStream] MJPEG connected');
    } catch (err) {
      const error = err instanceof Error ? err : new Error(String(err));
      setError(error);
      onError?.(error);
      console.error('[EnhancedStream] MJPEG connection failed:', err);
    } finally {
      setIsLoading(false);
    }
  }, [streamName, filters, onTransportChange, onError]);

  // Switch transport
  const switchTransport = useCallback(async (transport: Go2RtcTransportType) => {
    cleanup();

    switch (transport) {
      case 'webrtc':
        await connectWebRTC();
        break;
      case 'hls':
        await connectHLS();
        break;
      case 'mse':
        await connectMSE();
        break;
      case 'mjpeg':
        await connectMJPEG();
        break;
      default:
        console.warn('[EnhancedStream] Unsupported transport:', transport);
        await connectWebRTC();
    }
  }, [cleanup, connectWebRTC, connectHLS, connectMSE, connectMJPEG]);

  // Reconnect
  const reconnect = useCallback(async () => {
    if (currentTransport) {
      await switchTransport(currentTransport);
    } else {
      await connectWebRTC();
    }
  }, [currentTransport, switchTransport, connectWebRTC]);

  // Take snapshot
  const takeSnapshot = useCallback(async (): Promise<Blob | null> => {
    const service = getGo2RtcService();
    return await service.getSnapshot(streamName);
  }, [streamName]);

  // Adaptive bitrate logic
  useEffect(() => {
    if (!enableAdaptiveBitrate || !enableMonitoring || !streamInfo) return;

    const now = Date.now();
    if (now - lastBitrateCheckRef.current < 5000) return; // Check every 5 seconds
    lastBitrateCheckRef.current = now;

    const bitrate = streamInfo.bitrateKbps;

    // If bitrate is consistently low, switch to lower quality transport
    if (bitrate < 500 && currentTransport === 'webrtc') {
      consecutiveLowBitrateRef.current++;
      
      if (consecutiveLowBitrateRef.current >= 3) {
        console.warn('[EnhancedStream] Low bitrate detected, switching to HLS');
        switchTransport('hls');
        consecutiveLowBitrateRef.current = 0;
      }
    } else {
      consecutiveLowBitrateRef.current = 0;
    }

    // If bitrate improves and we're on HLS, switch back to WebRTC
    if (bitrate > 1500 && currentTransport === 'hls') {
      console.log('[EnhancedStream] Bitrate improved, switching back to WebRTC');
      switchTransport('webrtc');
    }
  }, [streamInfo, currentTransport, enableAdaptiveBitrate, enableMonitoring, switchTransport]);

  // Start monitoring
  useEffect(() => {
    if (!enableMonitoring) return;

    const service = getGo2RtcService();
    const cleanup = service.startMonitoring(streamName, (info) => {
      setStreamInfo(info);
      onStatsUpdate?.(info);
    });

    monitorCleanupRef.current = cleanup;

    return () => {
      cleanup();
      monitorCleanupRef.current = null;
    };
  }, [streamName, enableMonitoring, onStatsUpdate]);

  // Initial connection
  useEffect(() => {
    const initConnection = async () => {
      if (preferredTransport) {
        await switchTransport(preferredTransport);
      } else {
        // Auto-select best transport
        const service = getGo2RtcService();
        const optimalTransport = await service.getOptimalTransport(streamName);
        await switchTransport(optimalTransport);
      }
    };

    initConnection();

    return () => {
      cleanup();
    };
  }, [streamName, preferredTransport]); // eslint-disable-line react-hooks/exhaustive-deps

  return {
    videoRef,
    currentTransport,
    streamInfo,
    isLoading,
    error,
    switchTransport,
    takeSnapshot,
    reconnect,
  };
}
