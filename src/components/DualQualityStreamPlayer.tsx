/**
 * DualQualityStreamPlayer - умный компонент для мгновенного переключения между SD и HD
 * Держит оба WebRTC соединения активными и переключает только видимость
 */

import React, { useRef, useEffect } from 'react';
import { Box } from '@mui/material';
import VideoStreamPlayer from './VideoStreamPlayer';

interface DualQualityStreamPlayerProps {
  streamName: string;
  activeQuality: 'sd' | 'hd';
  cellMuted: boolean;
  volume: number;
  onStatsUpdateSD?: (stats: any) => void;
  onStatsUpdateHD?: (stats: any) => void;
  onVideoRefSD?: (ref: HTMLVideoElement | null) => void;
  onVideoRefHD?: (ref: HTMLVideoElement | null) => void;
  go2rtcSettings: {
    showMonitor: boolean;
    enableSnapshot: boolean;
    enable2WayAudio: boolean;
    enableAdaptiveBitrate: boolean;
  };
  streamOptSettings: {
    enableFastStart: boolean;
  };
  cellIndex: number;
}

const DualQualityStreamPlayer: React.FC<DualQualityStreamPlayerProps> = ({
  streamName,
  activeQuality,
  cellMuted,
  volume,
  onStatsUpdateSD,
  onStatsUpdateHD,
  onVideoRefSD,
  onVideoRefHD,
  go2rtcSettings,
  streamOptSettings,
  cellIndex,
}) => {
  const sdVisible = activeQuality === 'sd';
  const hdVisible = activeQuality === 'hd';

  // Используем стабильные ключи - они НЕ должны меняться!
  const sdKey = `${streamName}-cell-${cellIndex}-sd-permanent`;
  const hdKey = `${streamName}-cell-${cellIndex}-hd-permanent`;

  return (
    <>
      {/* SD Stream - всегда активен */}
      <Box
        sx={{
          position: 'absolute',
          inset: 0,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          opacity: sdVisible ? 1 : 0,
          pointerEvents: sdVisible ? 'auto' : 'none',
          transition: 'opacity 160ms ease-out',
          visibility: 'visible', // КРИТИЧНО: всегда видим для браузера
          zIndex: sdVisible ? 2 : 1,
        }}
      >
        <VideoStreamPlayer
          key={sdKey}
          streamName={streamName}
          statsDisplayName={`${streamName} (SD)`}
          useHdQuality={false}
          controls={false}
          autoPlay
          muted={sdVisible ? cellMuted : true}
          width="100%"
          height="100%"
          objectFit="contain"
          style={{ borderRadius: 2 }}
          onStatsUpdate={onStatsUpdateSD}
          onVideoRef={onVideoRefSD}
          showMonitor={go2rtcSettings.showMonitor}
          enableSnapshot={go2rtcSettings.enableSnapshot}
          enable2WayAudio={go2rtcSettings.enable2WayAudio}
          enableAdaptiveBitrate={go2rtcSettings.enableAdaptiveBitrate}
          fastStart={streamOptSettings.enableFastStart}
          showWebRTCStats={true}
          webrtcStatsUpdateInterval={500}
          isPaused={false} // КРИТИЧНО: никогда не останавливать
          volume={volume}
        />
      </Box>

      {/* HD Stream - всегда активен */}
      <Box
        sx={{
          position: 'absolute',
          inset: 0,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          opacity: hdVisible ? 1 : 0,
          pointerEvents: hdVisible ? 'auto' : 'none',
          transition: 'opacity 160ms ease-out',
          visibility: 'visible', // КРИТИЧНО: всегда видим для браузера
          zIndex: hdVisible ? 2 : 1,
        }}
      >
        <VideoStreamPlayer
          key={hdKey}
          streamName={streamName}
          statsDisplayName={`${streamName} (HD)`}
          useHdQuality={true}
          controls={false}
          autoPlay
          muted={hdVisible ? cellMuted : true}
          width="100%"
          height="100%"
          objectFit="contain"
          style={{ borderRadius: 2 }}
          onStatsUpdate={onStatsUpdateHD}
          onVideoRef={onVideoRefHD}
          showMonitor={go2rtcSettings.showMonitor}
          enableSnapshot={go2rtcSettings.enableSnapshot}
          enable2WayAudio={go2rtcSettings.enable2WayAudio}
          enableAdaptiveBitrate={go2rtcSettings.enableAdaptiveBitrate}
          fastStart={streamOptSettings.enableFastStart}
          showWebRTCStats={true}
          webrtcStatsUpdateInterval={500}
          isPaused={false} // КРИТИЧНО: никогда не останавливать
          volume={volume}
        />
      </Box>
    </>
  );
};

// Мемоизация для предотвращения пересоздания компонента при изменении колбеков
export default React.memo(DualQualityStreamPlayer, (prevProps, nextProps) => {
  // Проверяем только критичные пропсы, которые должны вызывать перерисовку
  return (
    prevProps.streamName === nextProps.streamName &&
    prevProps.activeQuality === nextProps.activeQuality &&
    prevProps.cellMuted === nextProps.cellMuted &&
    prevProps.volume === nextProps.volume &&
    prevProps.cellIndex === nextProps.cellIndex &&
    prevProps.go2rtcSettings.showMonitor === nextProps.go2rtcSettings.showMonitor &&
    prevProps.go2rtcSettings.enableSnapshot === nextProps.go2rtcSettings.enableSnapshot &&
    prevProps.go2rtcSettings.enable2WayAudio === nextProps.go2rtcSettings.enable2WayAudio &&
    prevProps.go2rtcSettings.enableAdaptiveBitrate === nextProps.go2rtcSettings.enableAdaptiveBitrate &&
    prevProps.streamOptSettings.enableFastStart === nextProps.streamOptSettings.enableFastStart
    // ИГНОРИРУЕМ колбеки (onVideoRefSD, onVideoRefHD, onStatsUpdateSD, onStatsUpdateHD) - они не влияют на рендер
  );
});
