/**
 * GridCell - мемоизированный компонент ячейки дашборда
 * Предотвращает пересоздание VideoStreamPlayer при изменениях в других ячейках
 */

import React, { memo, useCallback } from 'react';
import { Box } from '@mui/material';
import DualQualityStreamPlayer from './DualQualityStreamPlayer';
import DetectionOverlay from './DetectionOverlay';
import type { StreamQuality } from '../types';
import type { AnalyticsDetectionBox } from '../services/analytics';

interface GridCellProps {
  streamBaseName: string;
  quality: StreamQuality;
  cellMuted: boolean;
  volume: number;
  cellIndex: number;
  onStatsUpdateSD: (stats: any) => void;
  onStatsUpdateHD: (stats: any) => void;
  onVideoRefSD: (ref: HTMLVideoElement | null) => void;
  onVideoRefHD: (ref: HTMLVideoElement | null) => void;
  go2rtcSettings: {
    showMonitor: boolean;
    enableSnapshot: boolean;
    enable2WayAudio: boolean;
    enableAdaptiveBitrate: boolean;
  };
  streamOptSettings: {
    enableFastStart: boolean;
  };
  detections: AnalyticsDetectionBox[];
  detectionFrameWidth: number;
  detectionFrameHeight: number;
  videoElement: HTMLVideoElement | null;
  hasDetections: boolean;
}

const GridCell: React.FC<GridCellProps> = memo(({
  streamBaseName,
  quality,
  cellMuted,
  volume,
  cellIndex,
  onStatsUpdateSD,
  onStatsUpdateHD,
  onVideoRefSD,
  onVideoRefHD,
  go2rtcSettings,
  streamOptSettings,
  detections,
  detectionFrameWidth,
  detectionFrameHeight,
  videoElement,
  hasDetections,
}) => {
  return (
    <Box sx={{ position: 'relative', width: '100%', height: '100%', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
      <Box sx={{ position: 'absolute', inset: 0, zIndex: 1 }}>
        <DualQualityStreamPlayer
          streamName={streamBaseName}
          activeQuality={quality}
          cellMuted={cellMuted}
          volume={volume}
          onStatsUpdateSD={onStatsUpdateSD}
          onStatsUpdateHD={onStatsUpdateHD}
          onVideoRefSD={onVideoRefSD}
          onVideoRefHD={onVideoRefHD}
          go2rtcSettings={go2rtcSettings}
          streamOptSettings={streamOptSettings}
          cellIndex={cellIndex}
        />
      </Box>
      <DetectionOverlay
        detections={detections}
        frameWidth={detectionFrameWidth}
        frameHeight={detectionFrameHeight}
        videoElement={videoElement}
        visible={hasDetections}
      />
    </Box>
  );
}, (prevProps, nextProps) => {
  // Глубокое сравнение только критичных пропсов
  const sameBasicProps = 
    prevProps.streamBaseName === nextProps.streamBaseName &&
    prevProps.quality === nextProps.quality &&
    prevProps.cellMuted === nextProps.cellMuted &&
    prevProps.volume === nextProps.volume &&
    prevProps.cellIndex === nextProps.cellIndex &&
    prevProps.go2rtcSettings.showMonitor === nextProps.go2rtcSettings.showMonitor &&
    prevProps.go2rtcSettings.enableSnapshot === nextProps.go2rtcSettings.enableSnapshot &&
    prevProps.go2rtcSettings.enable2WayAudio === nextProps.go2rtcSettings.enable2WayAudio &&
    prevProps.go2rtcSettings.enableAdaptiveBitrate === nextProps.go2rtcSettings.enableAdaptiveBitrate &&
    prevProps.streamOptSettings.enableFastStart === nextProps.streamOptSettings.enableFastStart;

  if (!sameBasicProps) {
    return false; // Props changed, re-render
  }

  // Compare detection props - CRITICAL for overlay rendering
  const sameDetections = 
    prevProps.hasDetections === nextProps.hasDetections &&
    prevProps.detections.length === nextProps.detections.length &&
    prevProps.detectionFrameWidth === nextProps.detectionFrameWidth &&
    prevProps.detectionFrameHeight === nextProps.detectionFrameHeight &&
    prevProps.videoElement === nextProps.videoElement;

  if (!sameDetections) {
    return false; // Detection props changed, re-render
  }

  // If detections exist, compare detection bounds (they might have same IDs but moved)
  if (prevProps.detections.length > 0) {
    for (let i = 0; i < prevProps.detections.length; i++) {
      const prev = prevProps.detections[i];
      const next = nextProps.detections[i];
      if (!next || prev.id !== next.id || 
          prev.bounds.x !== next.bounds.x ||
          prev.bounds.y !== next.bounds.y ||
          prev.bounds.width !== next.bounds.width ||
          prev.bounds.height !== next.bounds.height) {
        return false; // Detection moved, re-render
      }
    }
  }

  return true; // No changes, skip re-render
});

GridCell.displayName = 'GridCell';

export default GridCell;
