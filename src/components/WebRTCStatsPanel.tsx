/**
 * WebRTC Stats Panel
 * Компактная панель для отображения статистики WebRTC соединения
 */

import React, { useEffect, useState } from 'react';
import { Box, Typography, Tooltip, IconButton } from '@mui/material';
import {
  SignalCellularAlt as SignalIcon,
  Warning as WarningIcon,
  Error as ErrorIcon,
  CheckCircle as CheckCircleIcon,
  InfoOutlined as InfoIcon,
} from '@mui/icons-material';
import type { WebRTCStats } from '../services/webrtcStats';

interface WebRTCStatsPanelProps {
  stats: WebRTCStats | null;
  compact?: boolean;
  onDetailsClick?: () => void;
}

const WebRTCStatsPanel: React.FC<WebRTCStatsPanelProps> = ({
  stats,
  compact = false,
  onDetailsClick,
}) => {
  const [quality, setQuality] = useState<'excellent' | 'good' | 'fair' | 'poor' | 'unknown'>('unknown');

  useEffect(() => {
    if (!stats || !stats.video || !stats.network) {
      setQuality('unknown');
      return;
    }

    const { packetLossRate, jitter } = stats.video;
    const { currentRoundTripTime } = stats.network;

    // Определяем качество соединения
    if (packetLossRate < 1 && jitter < 30 && currentRoundTripTime < 50) {
      setQuality('excellent');
    } else if (packetLossRate < 3 && jitter < 50 && currentRoundTripTime < 100) {
      setQuality('good');
    } else if (packetLossRate < 5 && jitter < 100 && currentRoundTripTime < 200) {
      setQuality('fair');
    } else {
      setQuality('poor');
    }
  }, [stats]);

  if (!stats) {
    return null;
  }

  // Цвета и иконки для качества
  const qualityConfig = {
    excellent: { color: '#4caf50', icon: CheckCircleIcon, label: 'Отлично' },
    good: { color: '#8bc34a', icon: SignalIcon, label: 'Хорошо' },
    fair: { color: '#ff9800', icon: WarningIcon, label: 'Удовлетворительно' },
    poor: { color: '#f44336', icon: ErrorIcon, label: 'Плохо' },
    unknown: { color: '#9e9e9e', icon: InfoIcon, label: 'Неизвестно' },
  };

  const config = qualityConfig[quality];
  const QualityIcon = config.icon;

  // Компактный режим - только индикатор качества
  if (compact) {
    return (
      <Tooltip 
        title={
          <Box sx={{ p: 1 }}>
            <Typography variant="body2" sx={{ fontWeight: 'bold', mb: 1 }}>
              {config.label}
            </Typography>
            {stats.video && (
              <>
                <Typography variant="caption" display="block">
                  Битрейт: {stats.video.bitrate} kbps
                </Typography>
                <Typography variant="caption" display="block">
                  Потери пакетов: {stats.video.packetLossRate}%
                </Typography>
                <Typography variant="caption" display="block">
                  Jitter: {stats.video.jitter} ms
                </Typography>
              </>
            )}
            {stats.network && (
              <Typography variant="caption" display="block">
                RTT: {stats.network.currentRoundTripTime} ms
              </Typography>
            )}
          </Box>
        }
      >
        <IconButton
          size="small"
          onClick={onDetailsClick}
          sx={{
            color: config.color,
            '&:hover': { backgroundColor: 'rgba(255,255,255,0.1)' },
          }}
        >
          <QualityIcon fontSize="small" />
        </IconButton>
      </Tooltip>
    );
  }

  // Полный режим
  return (
    <Box
      sx={{
        position: 'absolute',
        top: 8,
        left: 8,
        backgroundColor: 'rgba(0, 0, 0, 0.75)',
        backdropFilter: 'blur(10px)',
        borderRadius: 2,
        padding: 1.5,
        minWidth: 220,
        zIndex: 10,
        border: `1px solid ${config.color}`,
        boxShadow: `0 0 10px ${config.color}40`,
      }}
    >
      {/* Заголовок с индикатором качества */}
      <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 1.5 }}>
        <QualityIcon sx={{ fontSize: 20, color: config.color }} />
        <Typography variant="subtitle2" sx={{ color: config.color, fontWeight: 'bold' }}>
          {config.label}
        </Typography>
        {onDetailsClick && (
          <IconButton
            size="small"
            onClick={onDetailsClick}
            sx={{
              ml: 'auto',
              color: '#fff',
              '&:hover': { backgroundColor: 'rgba(255,255,255,0.1)' },
            }}
          >
            <InfoIcon fontSize="small" />
          </IconButton>
        )}
      </Box>

      {/* Состояние соединения */}
      <Box sx={{ mb: 1 }}>
        <Typography variant="caption" sx={{ color: '#aaa', display: 'block' }}>
          Соединение:
        </Typography>
        <Typography variant="body2" sx={{ color: '#fff' }}>
          {stats.connectionState} / {stats.iceConnectionState}
        </Typography>
      </Box>

      {/* Видео статистика */}
      {stats.video && (
        <Box sx={{ mb: 1 }}>
          <Typography variant="caption" sx={{ color: '#aaa', display: 'block' }}>
            Видео:
          </Typography>
          <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1 }}>
            <StatItem
              label="Битрейт"
              value={`${stats.video.bitrate} kbps`}
              color={stats.video.bitrate > 1000 ? '#4caf50' : stats.video.bitrate > 500 ? '#ff9800' : '#f44336'}
            />
            <StatItem
              label="FPS"
              value={stats.video.frameRate.toString()}
              color="#2196f3"
            />
            <StatItem
              label="Разрешение"
              value={`${stats.video.resolution.width}x${stats.video.resolution.height}`}
              color="#9c27b0"
            />
            <StatItem
              label="Кодек"
              value={stats.video.codec}
              color="#00bcd4"
            />
          </Box>
          <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1, mt: 0.5 }}>
            <StatItem
              label="Потери"
              value={`${stats.video.packetLossRate}%`}
              color={stats.video.packetLossRate < 1 ? '#4caf50' : stats.video.packetLossRate < 3 ? '#ff9800' : '#f44336'}
            />
            <StatItem
              label="Jitter"
              value={`${stats.video.jitter} ms`}
              color={stats.video.jitter < 30 ? '#4caf50' : stats.video.jitter < 50 ? '#ff9800' : '#f44336'}
            />
            {stats.video.frameDropRate > 0 && (
              <StatItem
                label="Dropped"
                value={`${stats.video.frameDropRate}%`}
                color="#f44336"
              />
            )}
          </Box>
        </Box>
      )}

      {/* Аудио статистика */}
      {stats.audio && (
        <Box sx={{ mb: 1 }}>
          <Typography variant="caption" sx={{ color: '#aaa', display: 'block' }}>
            Аудио:
          </Typography>
          <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1 }}>
            <StatItem
              label="Битрейт"
              value={`${stats.audio.bitrate} kbps`}
              color="#4caf50"
            />
            <StatItem
              label="Кодек"
              value={stats.audio.codec}
              color="#00bcd4"
            />
            <StatItem
              label="Потери"
              value={`${stats.audio.packetLossRate}%`}
              color={stats.audio.packetLossRate < 1 ? '#4caf50' : '#ff9800'}
            />
          </Box>
        </Box>
      )}

      {/* Сетевая статистика */}
      {stats.network && (
        <Box>
          <Typography variant="caption" sx={{ color: '#aaa', display: 'block' }}>
            Сеть:
          </Typography>
          <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1 }}>
            <StatItem
              label="RTT"
              value={`${stats.network.currentRoundTripTime} ms`}
              color={stats.network.currentRoundTripTime < 50 ? '#4caf50' : stats.network.currentRoundTripTime < 100 ? '#ff9800' : '#f44336'}
            />
            <StatItem
              label="Протокол"
              value={stats.network.protocol.toUpperCase()}
              color="#9c27b0"
            />
            <StatItem
              label="Тип"
              value={`${stats.network.localCandidateType}→${stats.network.remoteCandidateType}`}
              color="#00bcd4"
            />
          </Box>
        </Box>
      )}
    </Box>
  );
};

// Компонент для отображения одного параметра
const StatItem: React.FC<{ label: string; value: string; color: string }> = ({ label, value, color }) => (
  <Box
    sx={{
      display: 'inline-flex',
      flexDirection: 'column',
      backgroundColor: 'rgba(255, 255, 255, 0.05)',
      borderRadius: 1,
      padding: '4px 8px',
      borderLeft: `3px solid ${color}`,
    }}
  >
    <Typography variant="caption" sx={{ color: '#aaa', fontSize: 10, lineHeight: 1 }}>
      {label}
    </Typography>
    <Typography variant="body2" sx={{ color: '#fff', fontSize: 12, fontWeight: 'bold', lineHeight: 1.2 }}>
      {value}
    </Typography>
  </Box>
);

export default WebRTCStatsPanel;
