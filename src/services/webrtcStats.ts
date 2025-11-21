/**
 * WebRTC Statistics Service
 * Собирает детальную статистику WebRTC соединений для диагностики
 */

export interface WebRTCStats {
  // Connection info
  connectionState: RTCPeerConnectionState;
  iceConnectionState: RTCIceConnectionState;
  iceGatheringState: RTCIceGatheringState;
  signalingState: RTCSignalingState;
  
  // Video stats
  video: {
    codec: string;
    resolution: { width: number; height: number };
    frameRate: number;
    bitrate: number; // kbps
    packetsReceived: number;
    packetsLost: number;
    packetLossRate: number; // %
    jitter: number; // ms
    totalBytesReceived: number;
    framesDecoded: number;
    framesDropped: number;
    frameDropRate: number; // %
  } | null;
  
  // Audio stats
  audio: {
    codec: string;
    bitrate: number; // kbps
    packetsReceived: number;
    packetsLost: number;
    packetLossRate: number; // %
    jitter: number; // ms
    totalBytesReceived: number;
    audioLevel: number; // 0-1
  } | null;
  
  // Network stats
  network: {
    currentRoundTripTime: number; // ms
    availableOutgoingBitrate: number; // bps
    totalRoundTripTime: number;
    responseCount: number;
    
    // Candidate pair info
    localCandidateType: string;
    remoteCandidateType: string;
    localAddress: string;
    remoteAddress: string;
    protocol: string;
    
    // Throughput
    bytesSent: number;
    bytesReceived: number;
    packetsSent: number;
    packetsReceived: number;
  } | null;
  
  // Timing
  timestamp: number;
}

export interface WebRTCStatsHistory {
  stats: WebRTCStats[];
  maxLength: number;
}

/**
 * Сервис для сбора и анализа WebRTC статистики
 */
export class WebRTCStatsCollector {
  private pc: RTCPeerConnection | null = null;
  private intervalId: ReturnType<typeof setInterval> | null = null;
  private history: WebRTCStats[] = [];
  private readonly maxHistoryLength: number = 60; // Хранить последние 60 записей
  private previousStats: RTCStatsReport | null = null;
  private previousTimestamp: number = 0;

  constructor(peerConnection: RTCPeerConnection | null = null, maxHistoryLength: number = 60) {
    this.pc = peerConnection;
    this.maxHistoryLength = maxHistoryLength;
  }

  /**
   * Установить RTCPeerConnection для мониторинга
   */
  setPeerConnection(pc: RTCPeerConnection | null): void {
    this.pc = pc;
    this.history = [];
    this.previousStats = null;
    this.previousTimestamp = 0;
  }

  /**
   * Начать сбор статистики
   */
  start(intervalMs: number = 1000): void {
    this.stop();
    
    if (!this.pc) {
      console.warn('[WebRTCStatsCollector] No PeerConnection set');
      return;
    }

    this.intervalId = setInterval(async () => {
      const stats = await this.collect();
      if (stats) {
        this.history.push(stats);
        
        // Ограничиваем размер истории
        if (this.history.length > this.maxHistoryLength) {
          this.history.shift();
        }
      }
    }, intervalMs);
  }

  /**
   * Остановить сбор статистики
   */
  stop(): void {
    if (this.intervalId) {
      clearInterval(this.intervalId);
      this.intervalId = null;
    }
  }

  /**
   * Собрать текущую статистику
   */
  async collect(): Promise<WebRTCStats | null> {
    if (!this.pc) {
      return null;
    }

    try {
      const statsReport = await this.pc.getStats();
      const timestamp = Date.now();
      const timeDelta = this.previousTimestamp > 0 ? (timestamp - this.previousTimestamp) / 1000 : 1;

      const stats: WebRTCStats = {
        connectionState: this.pc.connectionState,
        iceConnectionState: this.pc.iceConnectionState,
        iceGatheringState: this.pc.iceGatheringState,
        signalingState: this.pc.signalingState,
        video: null,
        audio: null,
        network: null,
        timestamp,
      };

      // Debug: track what report types we see
      const reportTypes = new Set<string>();
      const inboundReports: any[] = [];
      const allReportsDebug: any[] = [];

      // Обрабатываем все статистические записи
      statsReport.forEach((report) => {
        reportTypes.add(report.type);
        
        // Collect all report types for debugging
        allReportsDebug.push({
          type: report.type,
          kind: report.kind,
          mediaType: (report as any).mediaType,
          id: report.id
        });
        
        if (report.type === 'inbound-rtp') {
          inboundReports.push({ kind: report.kind, mediaType: (report as any).mediaType });
          // Видео или аудио статистика
          if (report.kind === 'video') {
            stats.video = this.extractVideoStats(report, timeDelta);
          } else if (report.kind === 'audio') {
            stats.audio = this.extractAudioStats(report, timeDelta);
          }
        } else if (report.type === 'candidate-pair' && report.state === 'succeeded') {
          // Сетевая статистика (активная пара кандидатов)
          stats.network = this.extractNetworkStats(report, statsReport);
        }
      });

      // Debug logging only if no video stats collected
      if (!stats.video && this.pc.connectionState === 'connected') {
        console.log('[WebRTCStatsCollector] CONNECTED but no video stats!', 
          'Report types:', Array.from(reportTypes),
          'All reports:', allReportsDebug
        );
      }

      this.previousStats = statsReport;
      this.previousTimestamp = timestamp;

      return stats;
    } catch (error) {
      console.error('[WebRTCStatsCollector] Failed to collect stats:', error);
      return null;
    }
  }

  /**
   * Извлечь видео статистику
   */
  private extractVideoStats(report: any, timeDelta: number): WebRTCStats['video'] {
    const bytesReceived = report.bytesReceived || 0;
    const packetsReceived = report.packetsReceived || 0;
    const packetsLost = report.packetsLost || 0;
    const framesDecoded = report.framesDecoded || 0;
    const framesDropped = report.framesDropped || 0;

    // Вычисляем битрейт
    let bitrate = 0;
    if (this.previousStats && timeDelta > 0) {
      const prevReport = Array.from(this.previousStats.values()).find(
        (r: any) => r.type === 'inbound-rtp' && r.kind === 'video' && r.ssrc === report.ssrc
      );
      
      if (prevReport) {
        const bytesDelta = bytesReceived - (prevReport.bytesReceived || 0);
        bitrate = (bytesDelta * 8) / timeDelta / 1000; // kbps
      }
    }

    const totalPackets = packetsReceived + packetsLost;
    const packetLossRate = totalPackets > 0 ? (packetsLost / totalPackets) * 100 : 0;
    const frameDropRate = framesDecoded > 0 ? (framesDropped / framesDecoded) * 100 : 0;

    return {
      codec: this.getCodecName(report.codecId),
      resolution: {
        width: report.frameWidth || 0,
        height: report.frameHeight || 0,
      },
      frameRate: report.framesPerSecond || 0,
      bitrate: Math.round(bitrate),
      packetsReceived,
      packetsLost,
      packetLossRate: Math.round(packetLossRate * 100) / 100,
      jitter: Math.round((report.jitter || 0) * 1000 * 100) / 100, // секунды -> мс
      totalBytesReceived: bytesReceived,
      framesDecoded,
      framesDropped,
      frameDropRate: Math.round(frameDropRate * 100) / 100,
    };
  }

  /**
   * Извлечь аудио статистику
   */
  private extractAudioStats(report: any, timeDelta: number): WebRTCStats['audio'] {
    const bytesReceived = report.bytesReceived || 0;
    const packetsReceived = report.packetsReceived || 0;
    const packetsLost = report.packetsLost || 0;

    // Вычисляем битрейт
    let bitrate = 0;
    if (this.previousStats && timeDelta > 0) {
      const prevReport = Array.from(this.previousStats.values()).find(
        (r: any) => r.type === 'inbound-rtp' && r.kind === 'audio' && r.ssrc === report.ssrc
      );
      
      if (prevReport) {
        const bytesDelta = bytesReceived - (prevReport.bytesReceived || 0);
        bitrate = (bytesDelta * 8) / timeDelta / 1000; // kbps
      }
    }

    const totalPackets = packetsReceived + packetsLost;
    const packetLossRate = totalPackets > 0 ? (packetsLost / totalPackets) * 100 : 0;

    return {
      codec: this.getCodecName(report.codecId),
      bitrate: Math.round(bitrate),
      packetsReceived,
      packetsLost,
      packetLossRate: Math.round(packetLossRate * 100) / 100,
      jitter: Math.round((report.jitter || 0) * 1000 * 100) / 100, // секунды -> мс
      totalBytesReceived: bytesReceived,
      audioLevel: report.audioLevel || 0,
    };
  }

  /**
   * Извлечь сетевую статистику
   */
  private extractNetworkStats(report: any, statsReport: RTCStatsReport): WebRTCStats['network'] {
    // Получаем информацию о локальном и удаленном кандидатах
    const localCandidate = statsReport.get(report.localCandidateId);
    const remoteCandidate = statsReport.get(report.remoteCandidateId);

    return {
      currentRoundTripTime: Math.round((report.currentRoundTripTime || 0) * 1000 * 100) / 100, // с -> мс
      availableOutgoingBitrate: report.availableOutgoingBitrate || 0,
      totalRoundTripTime: report.totalRoundTripTime || 0,
      responseCount: report.responsesReceived || 0,
      
      localCandidateType: localCandidate?.candidateType || 'unknown',
      remoteCandidateType: remoteCandidate?.candidateType || 'unknown',
      localAddress: localCandidate ? `${localCandidate.address || ''}:${localCandidate.port || ''}` : '',
      remoteAddress: remoteCandidate ? `${remoteCandidate.address || ''}:${remoteCandidate.port || ''}` : '',
      protocol: localCandidate?.protocol || 'unknown',
      
      bytesSent: report.bytesSent || 0,
      bytesReceived: report.bytesReceived || 0,
      packetsSent: report.packetsSent || 0,
      packetsReceived: report.packetsReceived || 0,
    };
  }

  /**
   * Получить имя кодека по ID
   */
  private getCodecName(codecId: string | undefined): string {
    if (!codecId || !this.previousStats) {
      return 'unknown';
    }

    const codecReport = this.previousStats.get(codecId);
    if (codecReport && codecReport.mimeType) {
      // Извлекаем имя кодека из mimeType (например, "video/H264" -> "H264")
      const parts = codecReport.mimeType.split('/');
      return parts.length > 1 ? parts[1] : codecReport.mimeType;
    }

    return 'unknown';
  }

  /**
   * Получить историю статистики
   */
  getHistory(): WebRTCStats[] {
    return [...this.history];
  }

  /**
   * Получить последнюю статистику
   */
  getLatest(): WebRTCStats | null {
    return this.history.length > 0 ? this.history[this.history.length - 1] : null;
  }

  /**
   * Очистить историю
   */
  clearHistory(): void {
    this.history = [];
  }

  /**
   * Получить средние значения за указанный период
   */
  getAverageStats(periodSeconds: number = 10): Partial<WebRTCStats> | null {
    if (this.history.length === 0) {
      return null;
    }

    const now = Date.now();
    const periodMs = periodSeconds * 1000;
    const recentStats = this.history.filter(s => now - s.timestamp <= periodMs);

    if (recentStats.length === 0) {
      return null;
    }

    // Вычисляем средние значения
    const avgVideoBitrate = recentStats
      .filter(s => s.video)
      .reduce((sum, s) => sum + (s.video?.bitrate || 0), 0) / recentStats.length;

    const avgVideoPacketLoss = recentStats
      .filter(s => s.video)
      .reduce((sum, s) => sum + (s.video?.packetLossRate || 0), 0) / recentStats.length;

    const avgVideoJitter = recentStats
      .filter(s => s.video)
      .reduce((sum, s) => sum + (s.video?.jitter || 0), 0) / recentStats.length;

    const avgRtt = recentStats
      .filter(s => s.network)
      .reduce((sum, s) => sum + (s.network?.currentRoundTripTime || 0), 0) / recentStats.length;

    return {
      video: {
        bitrate: Math.round(avgVideoBitrate),
        packetLossRate: Math.round(avgVideoPacketLoss * 100) / 100,
        jitter: Math.round(avgVideoJitter * 100) / 100,
      } as any,
      network: {
        currentRoundTripTime: Math.round(avgRtt * 100) / 100,
      } as any,
    };
  }

  /**
   * Проверить качество соединения
   */
  getConnectionQuality(): 'excellent' | 'good' | 'fair' | 'poor' | 'unknown' {
    const latest = this.getLatest();
    if (!latest || !latest.video || !latest.network) {
      return 'unknown';
    }

    const { packetLossRate, jitter } = latest.video;
    const { currentRoundTripTime } = latest.network;

    // Критерии качества
    if (packetLossRate < 1 && jitter < 30 && currentRoundTripTime < 50) {
      return 'excellent';
    } else if (packetLossRate < 3 && jitter < 50 && currentRoundTripTime < 100) {
      return 'good';
    } else if (packetLossRate < 5 && jitter < 100 && currentRoundTripTime < 200) {
      return 'fair';
    } else {
      return 'poor';
    }
  }

  /**
   * Освободить ресурсы
   */
  dispose(): void {
    this.stop();
    this.pc = null;
    this.history = [];
    this.previousStats = null;
  }
}
