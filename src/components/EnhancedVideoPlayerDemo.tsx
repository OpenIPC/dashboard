/**
 * Enhanced Video Player Demo
 * Example showing all go2rtc enhanced features
 */

import React, { useState } from 'react';
import { Box, Paper, Typography, Button, ButtonGroup, Chip, Stack } from '@mui/material';
import { 
  Videocam as VideoIcon,
  Hd as HdIcon,
  Sd as SdIcon,
} from '@mui/icons-material';
import { useEnhancedVideoStream } from '../hooks/useEnhancedVideoStream';
import StreamMonitor from './StreamMonitor';
import TwoWayAudioControl from './TwoWayAudioControl';
import SnapshotButton from './SnapshotButton';
import type { Go2RtcTransportType } from '../global';

interface EnhancedVideoPlayerDemoProps {
  cameraId: number;
  cameraName: string;
  enableAudio?: boolean;
  enableMonitoring?: boolean;
}

const EnhancedVideoPlayerDemo: React.FC<EnhancedVideoPlayerDemoProps> = ({
  cameraId,
  cameraName,
  enableAudio = false,
  enableMonitoring = true,
}) => {
  const [quality, setQuality] = useState<'hd' | 'sd'>('hd');
  const streamName = `cam${cameraId}_${quality === 'hd' ? 0 : 1}`;

  const {
    videoRef,
    currentTransport,
    streamInfo,
    isLoading,
    error,
    switchTransport,
    takeSnapshot,
    reconnect,
  } = useEnhancedVideoStream({
    streamName,
    preferredTransport: 'webrtc',
    enableAdaptiveBitrate: true,
    enableMonitoring,
    audioConfig: enableAudio ? {
      enabled: true,
      codec: 'opus',
      sampleRate: 48000,
      channels: 1,
      echoCancellation: true,
      noiseSuppression: true,
      autoGainControl: true,
    } : undefined,
    onStatsUpdate: (stats) => {
      console.log('[Demo] Stream stats:', stats);
    },
    onTransportChange: (transport) => {
      console.log('[Demo] Transport changed:', transport);
    },
  });

  const handleQualityChange = (newQuality: 'hd' | 'sd') => {
    setQuality(newQuality);
  };

  const handleTransportChange = (transport: Go2RtcTransportType) => {
    switchTransport(transport);
  };

  const handleSnapshot = async () => {
    const blob = await takeSnapshot();
    if (blob) {
      console.log('[Demo] Snapshot captured:', blob.size, 'bytes');
    }
  };

  return (
    <Paper
      elevation={3}
      sx={{
        overflow: 'hidden',
        backgroundColor: '#000',
        position: 'relative',
      }}
    >
      {/* Video Container */}
      <Box
        sx={{
          position: 'relative',
          width: '100%',
          paddingBottom: '56.25%', // 16:9 aspect ratio
          backgroundColor: '#000',
        }}
      >
        <video
          ref={videoRef}
          autoPlay
          playsInline
          muted={!enableAudio}
          style={{
            position: 'absolute',
            top: 0,
            left: 0,
            width: '100%',
            height: '100%',
            objectFit: 'contain',
          }}
        />

        {/* Loading Overlay */}
        {isLoading && (
          <Box
            sx={{
              position: 'absolute',
              top: 0,
              left: 0,
              right: 0,
              bottom: 0,
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              backgroundColor: 'rgba(0, 0, 0, 0.7)',
            }}
          >
            <Typography color="white">Loading stream...</Typography>
          </Box>
        )}

        {/* Error Overlay */}
        {error && (
          <Box
            sx={{
              position: 'absolute',
              top: 0,
              left: 0,
              right: 0,
              bottom: 0,
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              backgroundColor: 'rgba(0, 0, 0, 0.7)',
            }}
          >
            <Box textAlign="center" p={2}>
              <Typography color="error" gutterBottom>
                Stream Error
              </Typography>
              <Typography variant="body2" color="white" gutterBottom>
                {error.message}
              </Typography>
              <Button variant="contained" onClick={reconnect} sx={{ mt: 1 }}>
                Reconnect
              </Button>
            </Box>
          </Box>
        )}

        {/* Top Controls */}
        <Box
          sx={{
            position: 'absolute',
            top: 8,
            left: 8,
            right: 8,
            display: 'flex',
            justifyContent: 'space-between',
            alignItems: 'flex-start',
          }}
        >
          {/* Camera Name */}
          <Chip
            icon={<VideoIcon />}
            label={cameraName}
            sx={{
              backgroundColor: 'rgba(0, 0, 0, 0.7)',
              color: 'white',
            }}
          />

          {/* Quality Switch */}
          <ButtonGroup size="small" variant="contained">
            <Button
              onClick={() => handleQualityChange('hd')}
              color={quality === 'hd' ? 'primary' : 'inherit'}
              startIcon={<HdIcon />}
            >
              HD
            </Button>
            <Button
              onClick={() => handleQualityChange('sd')}
              color={quality === 'sd' ? 'primary' : 'inherit'}
              startIcon={<SdIcon />}
            >
              SD
            </Button>
          </ButtonGroup>
        </Box>

        {/* Stream Monitor */}
        {enableMonitoring && (
          <Box
            sx={{
              position: 'absolute',
              top: 8,
              right: 8,
            }}
          >
            <StreamMonitor streamName={streamName} compact />
          </Box>
        )}

        {/* Bottom Controls */}
        <Box
          sx={{
            position: 'absolute',
            bottom: 8,
            left: 8,
            right: 8,
            display: 'flex',
            justifyContent: 'space-between',
            alignItems: 'flex-end',
          }}
        >
          {/* 2-Way Audio */}
          {enableAudio && (
            <TwoWayAudioControl
              streamName={streamName}
              enabled
              compact
            />
          )}

          {/* Right Controls */}
          <Stack direction="row" spacing={1}>
            {/* Snapshot Button */}
            <SnapshotButton
              streamName={streamName}
              width={1920}
              height={1080}
              quality={95}
              autoDownload
              size="small"
            />

            {/* Transport Info */}
            {currentTransport && (
              <Chip
                label={currentTransport.toUpperCase()}
                size="small"
                sx={{
                  backgroundColor: 'rgba(0, 0, 0, 0.7)',
                  color: 'white',
                }}
              />
            )}
          </Stack>
        </Box>
      </Box>

      {/* Detailed Monitor (Optional) */}
      {enableMonitoring && streamInfo && (
        <Box sx={{ p: 2, backgroundColor: 'background.paper' }}>
          <StreamMonitor streamName={streamName} showDetails />
          
          {/* Transport Controls */}
          <Box sx={{ mt: 2 }}>
            <Typography variant="caption" color="text.secondary" gutterBottom>
              Transport:
            </Typography>
            <ButtonGroup size="small" fullWidth>
              <Button
                onClick={() => handleTransportChange('webrtc')}
                variant={currentTransport === 'webrtc' ? 'contained' : 'outlined'}
              >
                WebRTC
              </Button>
              <Button
                onClick={() => handleTransportChange('hls')}
                variant={currentTransport === 'hls' ? 'contained' : 'outlined'}
              >
                HLS
              </Button>
              <Button
                onClick={() => handleTransportChange('mjpeg')}
                variant={currentTransport === 'mjpeg' ? 'contained' : 'outlined'}
              >
                MJPEG
              </Button>
            </ButtonGroup>
          </Box>
        </Box>
      )}
    </Paper>
  );
};

export default EnhancedVideoPlayerDemo;
