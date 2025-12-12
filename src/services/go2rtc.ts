/**
 * go2rtc Enhanced Service
 * Provides comprehensive integration with go2rtc streaming server
 * Features: monitoring, multiple transports, snapshots, 2-way audio, adaptive bitrate
 */

import type {
  Go2RtcStreamStats,
  Go2RtcStreamInfo,
  Go2RtcTransportType,
  TwoWayAudioConfig,
  Go2RtcStreamFilter,
} from '../global';

export interface Go2RtcApiConfig {
  baseUrl: string;
  timeout?: number;
}

export interface SnapshotOptions {
  width?: number;
  height?: number;
  quality?: number;
}

export interface TransportPreference {
  primary: Go2RtcTransportType;
  fallbacks: Go2RtcTransportType[];
  autoSwitch: boolean;
}

/**
 * Main go2rtc service class
 */
export class Go2RtcService {
  private config: Go2RtcApiConfig;
  private statsCache: Map<string, Go2RtcStreamInfo> = new Map();
  private updateIntervals: Map<string, number> = new Map();

  constructor(config: Go2RtcApiConfig = { baseUrl: 'http://127.0.0.1:1984' }) {
    this.config = config;
  }

  /**
   * Get current streaming statistics for a stream
   */
  async getStreamStats(streamName: string): Promise<Go2RtcStreamStats | null> {
    try {
      const response = await fetch(`${this.config.baseUrl}/api/streams`, {
        signal: AbortSignal.timeout(this.config.timeout || 5000),
      });
      
      if (!response.ok) {
        console.warn(`[Go2RTC] Failed to fetch streams: ${response.status}`);
        return null;
      }

      const data = await response.json();
      return data[streamName] || null;
    } catch (error) {
      console.error('[Go2RTC] Error fetching stream stats:', error);
      return null;
    }
  }

  /**
   * Get processed stream information with calculated metrics
   */
  async getStreamInfo(streamName: string): Promise<Go2RtcStreamInfo | null> {
    const stats = await this.getStreamStats(streamName);
    if (!stats) return null;

    const info: Go2RtcStreamInfo = {
      streamName,
      online: stats.producers && stats.producers.length > 0,
      consumerCount: stats.consumers?.length || 0,
      bitrateKbps: 0,
      latencyMs: 0,
    };

    // Calculate bitrate from consumers
    if (stats.consumers) {
      let totalBytes = 0;
      stats.consumers.forEach(consumer => {
        if (consumer.send) totalBytes += consumer.send;
      });
      info.bitrateKbps = Math.round((totalBytes * 8) / 1024);
    }

    // Extract codec information
    if (stats.receivers && stats.receivers.length > 0) {
      const videoReceiver = stats.receivers.find(r => 
        r.codec?.toLowerCase().includes('h264') || 
        r.codec?.toLowerCase().includes('h265') ||
        r.codec?.toLowerCase().includes('vp')
      );
      if (videoReceiver) {
        info.videoCodec = videoReceiver.codec;
      }

      const audioReceiver = stats.receivers.find(r => 
        r.codec?.toLowerCase().includes('opus') || 
        r.codec?.toLowerCase().includes('pcm') ||
        r.codec?.toLowerCase().includes('aac')
      );
      if (audioReceiver) {
        info.audioCodec = audioReceiver.codec;
      }
    }

    this.statsCache.set(streamName, info);
    return info;
  }

  /**
   * Get snapshot/frame from stream
   */
  async getSnapshot(
    streamName: string,
    options?: SnapshotOptions
  ): Promise<Blob | null> {
    try {
      let url = `${this.config.baseUrl}/api/frame.jpeg?src=${encodeURIComponent(streamName)}`;
      
      if (options?.width) url += `&width=${options.width}`;
      if (options?.height) url += `&height=${options.height}`;
      if (options?.quality) url += `&quality=${options.quality}`;

      const response = await fetch(url, {
        signal: AbortSignal.timeout(this.config.timeout || 10000),
      });

      if (!response.ok) {
        console.warn(`[Go2RTC] Snapshot failed: ${response.status}`);
        return null;
      }

      return await response.blob();
    } catch (error) {
      console.error('[Go2RTC] Error fetching snapshot:', error);
      return null;
    }
  }

  /**
   * Get snapshot as base64 data URL
   */
  async getSnapshotDataUrl(
    streamName: string,
    options?: SnapshotOptions
  ): Promise<string | null> {
    const blob = await this.getSnapshot(streamName, options);
    if (!blob) return null;

    return new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onloadend = () => resolve(reader.result as string);
      reader.onerror = reject;
      reader.readAsDataURL(blob);
    });
  }

  /**
   * Start monitoring stream with periodic updates
   */
  startMonitoring(
    streamName: string,
    callback: (info: Go2RtcStreamInfo) => void,
    intervalMs: number = 2000
  ): () => void {
    this.stopMonitoring(streamName);

    const update = async () => {
      const info = await this.getStreamInfo(streamName);
      if (info) callback(info);
    };

    update(); // Initial update
    const intervalId = window.setInterval(update, intervalMs);
    this.updateIntervals.set(streamName, intervalId);

    return () => this.stopMonitoring(streamName);
  }

  /**
   * Stop monitoring stream
   */
  stopMonitoring(streamName: string): void {
    const intervalId = this.updateIntervals.get(streamName);
    if (intervalId) {
      clearInterval(intervalId);
      this.updateIntervals.delete(streamName);
    }
  }

  /**
   * Get optimal transport for current conditions
   */
  async getOptimalTransport(
    streamName: string,
    preference?: TransportPreference
  ): Promise<Go2RtcTransportType> {
    const defaultPreference: TransportPreference = {
      primary: 'webrtc',
      fallbacks: ['mse', 'hls', 'mjpeg'],
      autoSwitch: true,
    };

    const pref = preference || defaultPreference;

    // Check if WebRTC is supported
    if (pref.primary === 'webrtc' && typeof RTCPeerConnection !== 'undefined') {
      return 'webrtc';
    }

    // Check MSE support
    if (pref.fallbacks.includes('mse') && typeof MediaSource !== 'undefined') {
      return 'mse';
    }

    // Check HLS support
    if (pref.fallbacks.includes('hls')) {
      const video = document.createElement('video');
      if (video.canPlayType('application/vnd.apple.mpegurl')) {
        return 'hls';
      }
    }

    // Fallback to MJPEG (always supported)
    return 'mjpeg';
  }

  /**
   * Build stream URL with filters
   */
  buildStreamUrl(
    streamName: string,
    transport: Go2RtcTransportType,
    filters?: Go2RtcStreamFilter
  ): string {
    let src = streamName;

    // Apply filters as URL fragments
    const filterParts: string[] = [];

    if (filters?.rotate) {
      filterParts.push(`rotate=${filters.rotate}`);
    }

    if (filters?.width || filters?.height) {
      const w = filters.width || 0;
      const h = filters.height || 0;
      filterParts.push(`width=${w}`, `height=${h}`);
    }

    if (filterParts.length > 0) {
      src += '#' + filterParts.join('#');
    }

    // Build URL based on transport
    switch (transport) {
      case 'webrtc':
        return `${this.config.baseUrl}/api/webrtc?src=${encodeURIComponent(src)}`;
      case 'mse':
        return `${this.config.baseUrl}/api/ws?src=${encodeURIComponent(src)}`;
      case 'hls':
        return `${this.config.baseUrl}/api/hls/${encodeURIComponent(streamName)}/index.m3u8`;
      case 'mjpeg':
        return `${this.config.baseUrl}/api/stream.mjpeg?src=${encodeURIComponent(src)}`;
      case 'mp4':
        return `${this.config.baseUrl}/api/stream.mp4?src=${encodeURIComponent(src)}`;
      default:
        return `${this.config.baseUrl}/api/webrtc?src=${encodeURIComponent(src)}`;
    }
  }

  /**
   * Create WebRTC connection with 2-way audio support
   */
  async createWebRTCConnection(
    streamName: string,
    audioConfig?: TwoWayAudioConfig
  ): Promise<RTCPeerConnection> {
    const pc = new RTCPeerConnection({
      iceServers: [{ urls: 'stun:stun.l.google.com:19302' }],
    });

    // Add transceivers for receiving
    pc.addTransceiver('video', { direction: 'recvonly' });
    
    if (audioConfig?.enabled) {
      // Enable 2-way audio
      const stream = await navigator.mediaDevices.getUserMedia({
        audio: {
          echoCancellation: audioConfig.echoCancellation ?? true,
          noiseSuppression: audioConfig.noiseSuppression ?? true,
          autoGainControl: audioConfig.autoGainControl ?? true,
          sampleRate: audioConfig.sampleRate || 48000,
          channelCount: audioConfig.channels || 1,
        },
      });

      const audioTrack = stream.getAudioTracks()[0];
      if (audioTrack) {
        pc.addTrack(audioTrack, stream);
        console.log('[Go2RTC] 2-way audio enabled');
      }
    } else {
      pc.addTransceiver('audio', { direction: 'recvonly' });
    }

    return pc;
  }

  /**
   * Perform WHEP handshake with 2-way audio support
   */
  async connectWebRTC(
    streamName: string,
    videoElement: HTMLVideoElement,
    audioConfig?: TwoWayAudioConfig
  ): Promise<RTCPeerConnection> {
    const pc = await this.createWebRTCConnection(streamName, audioConfig);

    pc.addEventListener('track', (event) => {
      if (event.streams && event.streams[0]) {
        videoElement.srcObject = event.streams[0];
        console.log('[Go2RTC] Received media stream');
      }
    });

    pc.addEventListener('connectionstatechange', () => {
      console.log(`[Go2RTC] Connection state: ${pc.connectionState}`);
      if (pc.connectionState === 'failed' || pc.connectionState === 'disconnected') {
        console.warn('[Go2RTC] Connection failed');
      }
    });

    // Create and set local description
    const offer = await pc.createOffer();
    await pc.setLocalDescription(offer);

    // Wait for ICE gathering
    await this.waitForIceGathering(pc);

    // Send offer to go2rtc
    const url = this.buildStreamUrl(streamName, 'webrtc');
    const response = await fetch(url, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/sdp',
        'Accept': 'application/sdp',
      },
      body: pc.localDescription?.sdp,
    });

    if (!response.ok) {
      throw new Error(`WHEP handshake failed: ${response.status}`);
    }

    const answerSdp = await response.text();
    await pc.setRemoteDescription({
      type: 'answer',
      sdp: answerSdp,
    });

    console.log('[Go2RTC] WebRTC connection established');
    return pc;
  }

  /**
   * Wait for ICE gathering to complete
   */
  private async waitForIceGathering(pc: RTCPeerConnection, timeoutMs: number = 3000): Promise<void> {
    if (pc.iceGatheringState === 'complete') return;

    return new Promise((resolve) => {
      const timeout = setTimeout(() => {
        pc.removeEventListener('icegatheringstatechange', handler);
        resolve();
      }, timeoutMs);

      const handler = () => {
        if (pc.iceGatheringState === 'complete') {
          clearTimeout(timeout);
          pc.removeEventListener('icegatheringstatechange', handler);
          resolve();
        }
      };

      pc.addEventListener('icegatheringstatechange', handler);
    });
  }

  /**
   * Get all available streams
   */
  async getAllStreams(): Promise<string[]> {
    try {
      const response = await fetch(`${this.config.baseUrl}/api/streams`, {
        signal: AbortSignal.timeout(this.config.timeout || 5000),
      });

      if (!response.ok) return [];

      const data = await response.json();
      return Object.keys(data);
    } catch (error) {
      console.error('[Go2RTC] Error fetching streams list:', error);
      return [];
    }
  }

  /**
   * Check if stream is online
   */
  async isStreamOnline(streamName: string): Promise<boolean> {
    const info = await this.getStreamInfo(streamName);
    return info?.online || false;
  }

  /**
   * Cleanup resources
   */
  dispose(): void {
    // Clear all monitoring intervals
    this.updateIntervals.forEach((id) => clearInterval(id));
    this.updateIntervals.clear();
    this.statsCache.clear();
  }
}

// Singleton instance
let go2rtcServiceInstance: Go2RtcService | null = null;

/**
 * Get or create go2rtc service instance
 */
export function getGo2RtcService(config?: Go2RtcApiConfig): Go2RtcService {
  if (!go2rtcServiceInstance) {
    go2rtcServiceInstance = new Go2RtcService(config);
  }
  return go2rtcServiceInstance;
}

/**
 * Reset service instance (useful for testing)
 */
export function resetGo2RtcService(): void {
  if (go2rtcServiceInstance) {
    go2rtcServiceInstance.dispose();
    go2rtcServiceInstance = null;
  }
}
