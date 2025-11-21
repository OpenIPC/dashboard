/**
 * StreamMonitor Component
 * Real-time monitoring and statistics display for video streams
 */

import React, { useEffect, useState } from 'react';
import { Box, Typography, Chip, LinearProgress, Tooltip } from '@mui/material';
import { 
  SignalCellularAlt as SignalIcon,
  Speed as SpeedIcon,
  Person as PersonIcon,
  Videocam as VideocamIcon,
  AudioFile as AudioIcon,
} from '@mui/icons-material';
import { getGo2RtcService } from '../services/go2rtc';
import type { Go2RtcStreamInfo } from '../global';

interface StreamMonitorProps {
  streamName: string;
  updateInterval?: number;
  compact?: boolean;
  showDetails?: boolean;
}

const StreamMonitor: React.FC<StreamMonitorProps> = ({
  streamName,
  updateInterval = 2000,
  compact = false,
  showDetails = true,
}) => {
  const [streamInfo, setStreamInfo] = useState<Go2RtcStreamInfo | null>(null);
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    const service = getGo2RtcService();
    setIsLoading(true);

    const cleanup = service.startMonitoring(
      streamName,
      (info) => {
        setStreamInfo(info);
        setIsLoading(false);
      },
      updateInterval
    );

    return () => {
      cleanup();
    };
  }, [streamName, updateInterval]);

  if (isLoading) {
    return (
      <Box sx={{ width: '100%', p: 1 }}>
        <LinearProgress />
      </Box>
    );
  }

  if (!streamInfo) {
    return (
      <Box sx={{ p: 1 }}>
        <Typography variant="caption" color="error">
          No stream data
        </Typography>
      </Box>
    );
  }

  const getSignalStrength = () => {
    if (!streamInfo.online) return 0;
    const bitrate = streamInfo.bitrateKbps;
    if (bitrate > 3000) return 100;
    if (bitrate > 1500) return 75;
    if (bitrate > 500) return 50;
    if (bitrate > 100) return 25;
    return 10;
  };

  const getSignalColor = () => {
    const strength = getSignalStrength();
    if (strength >= 75) return 'success';
    if (strength >= 50) return 'warning';
    return 'error';
  };

  if (compact) {
    return (
      <Box
        sx={{
          display: 'flex',
          alignItems: 'center',
          gap: 1,
          px: 1,
          py: 0.5,
          backgroundColor: 'rgba(0, 0, 0, 0.5)',
          borderRadius: 1,
        }}
      >
        <Chip
          icon={<SignalIcon />}
          label={streamInfo.online ? 'Live' : 'Offline'}
          color={streamInfo.online ? 'success' : 'default'}
          size="small"
          sx={{ fontSize: '0.7rem' }}
        />
        {streamInfo.online && (
          <>
            <Tooltip title="Bitrate">
              <Chip
                icon={<SpeedIcon />}
                label={`${Math.round(streamInfo.bitrateKbps / 100) / 10} Mbps`}
                size="small"
                sx={{ fontSize: '0.7rem' }}
              />
            </Tooltip>
            {streamInfo.consumerCount > 0 && (
              <Tooltip title="Viewers">
                <Chip
                  icon={<PersonIcon />}
                  label={streamInfo.consumerCount}
                  size="small"
                  sx={{ fontSize: '0.7rem' }}
                />
              </Tooltip>
            )}
          </>
        )}
      </Box>
    );
  }

  return (
    <Box
      sx={{
        p: 1.5,
        backgroundColor: 'rgba(0, 0, 0, 0.6)',
        borderRadius: 1,
        border: '1px solid rgba(255, 255, 255, 0.1)',
      }}
    >
      <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 1 }}>
        <SignalIcon color={getSignalColor()} />
        <Typography variant="body2" sx={{ fontWeight: 600 }}>
          {streamInfo.online ? 'Live Stream' : 'Offline'}
        </Typography>
      </Box>

      {streamInfo.online && showDetails && (
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 0.5 }}>
          <Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
            <Typography variant="caption" color="text.secondary">
              Bitrate:
            </Typography>
            <Typography variant="caption" sx={{ fontWeight: 500 }}>
              {(streamInfo.bitrateKbps / 1024).toFixed(2)} Mbps
            </Typography>
          </Box>

          {streamInfo.videoCodec && (
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
              <Typography variant="caption" color="text.secondary">
                <VideocamIcon sx={{ fontSize: 12, mr: 0.5, verticalAlign: 'middle' }} />
                Video:
              </Typography>
              <Typography variant="caption" sx={{ fontWeight: 500 }}>
                {streamInfo.videoCodec.toUpperCase()}
                {streamInfo.resolution && ` @ ${streamInfo.resolution}`}
                {streamInfo.fps && ` ${streamInfo.fps}fps`}
              </Typography>
            </Box>
          )}

          {streamInfo.audioCodec && (
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
              <Typography variant="caption" color="text.secondary">
                <AudioIcon sx={{ fontSize: 12, mr: 0.5, verticalAlign: 'middle' }} />
                Audio:
              </Typography>
              <Typography variant="caption" sx={{ fontWeight: 500 }}>
                {streamInfo.audioCodec.toUpperCase()}
              </Typography>
            </Box>
          )}

          {streamInfo.consumerCount > 0 && (
            <Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
              <Typography variant="caption" color="text.secondary">
                Viewers:
              </Typography>
              <Typography variant="caption" sx={{ fontWeight: 500 }}>
                {streamInfo.consumerCount}
              </Typography>
            </Box>
          )}

          {streamInfo.latencyMs !== undefined && streamInfo.latencyMs > 0 && (
            <Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
              <Typography variant="caption" color="text.secondary">
                Latency:
              </Typography>
              <Typography
                variant="caption"
                sx={{ fontWeight: 500 }}
                color={streamInfo.latencyMs > 500 ? 'warning.main' : 'success.main'}
              >
                {streamInfo.latencyMs}ms
              </Typography>
            </Box>
          )}

          {/* Signal strength bar */}
          <Box sx={{ mt: 0.5 }}>
            <LinearProgress
              variant="determinate"
              value={getSignalStrength()}
              color={getSignalColor()}
              sx={{ height: 4, borderRadius: 2 }}
            />
          </Box>
        </Box>
      )}
    </Box>
  );
};

export default StreamMonitor;
