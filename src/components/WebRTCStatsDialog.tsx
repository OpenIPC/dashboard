/**
 * WebRTC Stats Dialog
 * Детальный диалог с графиками и полной статистикой WebRTC соединения
 */

import React, { useMemo } from 'react';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  IconButton,
  Box,
  Typography,
  Paper,
  Chip,
} from '@mui/material';
import {
  Close as CloseIcon,
  SignalCellularAlt as SignalIcon,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend } from 'recharts';
import type { WebRTCStats } from '../services/webrtcStats';

interface WebRTCStatsDialogProps {
  open: boolean;
  onClose: () => void;
  streamName: string;
  currentStats: WebRTCStats | null;
  history: WebRTCStats[];
}

const WebRTCStatsDialog: React.FC<WebRTCStatsDialogProps> = ({
  open,
  onClose,
  streamName,
  currentStats,
  history,
}) => {
  // Подготовка данных для графиков
  const chartData = useMemo(() => {
    return history.map((stat, index) => ({
      index,
      time: new Date(stat.timestamp).toLocaleTimeString(),
      bitrate: stat.video?.bitrate || 0,
      packetLoss: stat.video?.packetLossRate || 0,
      jitter: stat.video?.jitter || 0,
      rtt: stat.network?.currentRoundTripTime || 0,
      frameRate: stat.video?.frameRate || 0,
      frameDrop: stat.video?.frameDropRate || 0,
    }));
  }, [history]);

  if (!currentStats) {
    return null;
  }

  // Определяем качество соединения
  const getQualityColor = () => {
    if (!currentStats.video || !currentStats.network) return '#9e9e9e';
    
    const { packetLossRate, jitter } = currentStats.video;
    const { currentRoundTripTime } = currentStats.network;

    if (packetLossRate < 1 && jitter < 30 && currentRoundTripTime < 50) {
      return '#4caf50'; // excellent
    } else if (packetLossRate < 3 && jitter < 50 && currentRoundTripTime < 100) {
      return '#8bc34a'; // good
    } else if (packetLossRate < 5 && jitter < 100 && currentRoundTripTime < 200) {
      return '#ff9800'; // fair
    } else {
      return '#f44336'; // poor
    }
  };

  const qualityColor = getQualityColor();

  return (
    <Dialog
      open={open}
      onClose={onClose}
      maxWidth="lg"
      fullWidth
      PaperProps={{
        sx: {
          backgroundColor: '#1e1e1e',
          backgroundImage: 'none',
          minHeight: '80vh',
        },
      }}
    >
      <DialogTitle
        sx={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          borderBottom: '1px solid #333',
          pb: 2,
        }}
      >
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
          <SignalIcon sx={{ color: qualityColor }} />
          <Box>
            <Typography variant="h6">WebRTC Статистика</Typography>
            <Typography variant="caption" sx={{ color: '#aaa' }}>
              {streamName}
            </Typography>
          </Box>
        </Box>
        <IconButton onClick={onClose} sx={{ color: '#fff' }}>
          <CloseIcon />
        </IconButton>
      </DialogTitle>

      <DialogContent sx={{ mt: 2 }}>
        {/* Текущее состояние */}
        <Paper sx={{ p: 2, mb: 3, backgroundColor: '#252525', backgroundImage: 'none' }}>
          <Typography variant="subtitle1" sx={{ mb: 2, fontWeight: 'bold' }}>
            Состояние соединения
          </Typography>
          <Box sx={{ display: 'flex', gap: 2, flexWrap: 'wrap' }}>
            <Box sx={{ flex: '1 1 200px' }}>
              <InfoChip label="Connection" value={currentStats.connectionState} color={qualityColor} />
            </Box>
            <Box sx={{ flex: '1 1 200px' }}>
              <InfoChip label="ICE Connection" value={currentStats.iceConnectionState} color={qualityColor} />
            </Box>
            <Box sx={{ flex: '1 1 200px' }}>
              <InfoChip label="ICE Gathering" value={currentStats.iceGatheringState} color="#2196f3" />
            </Box>
            <Box sx={{ flex: '1 1 200px' }}>
              <InfoChip label="Signaling" value={currentStats.signalingState} color="#9c27b0" />
            </Box>
          </Box>
        </Paper>

        {/* Видео статистика */}
        {currentStats.video && (
          <Paper sx={{ p: 2, mb: 3, backgroundColor: '#252525', backgroundImage: 'none' }}>
            <Typography variant="subtitle1" sx={{ mb: 2, fontWeight: 'bold' }}>
              📹 Видео
            </Typography>
            <Box sx={{ display: 'flex', gap: 2, flexWrap: 'wrap' }}>
              <Box sx={{ flex: '1 1 45%' }}>
                <DetailItem label="Кодек" value={currentStats.video.codec.toUpperCase()} />
                <DetailItem 
                  label="Разрешение" 
                  value={`${currentStats.video.resolution.width}x${currentStats.video.resolution.height}`}
                />
                <DetailItem label="Частота кадров" value={`${currentStats.video.frameRate} FPS`} />
                <DetailItem label="Битрейт" value={`${currentStats.video.bitrate} kbps`} />
              </Box>
              <Box sx={{ flex: '1 1 45%' }}>
                <DetailItem label="Пакетов получено" value={currentStats.video.packetsReceived.toLocaleString()} />
                <DetailItem label="Пакетов потеряно" value={currentStats.video.packetsLost.toLocaleString()} />
                <DetailItem label="Потери пакетов" value={`${currentStats.video.packetLossRate}%`} />
                <DetailItem label="Jitter" value={`${currentStats.video.jitter} ms`} />
              </Box>
              <Box sx={{ flex: '1 1 45%' }}>
                <DetailItem label="Кадров декодировано" value={currentStats.video.framesDecoded.toLocaleString()} />
                <DetailItem label="Кадров пропущено" value={currentStats.video.framesDropped.toLocaleString()} />
                <DetailItem label="Процент пропущенных" value={`${currentStats.video.frameDropRate}%`} />
              </Box>
              <Box sx={{ flex: '1 1 45%' }}>
                <DetailItem 
                  label="Всего получено" 
                  value={`${(currentStats.video.totalBytesReceived / 1024 / 1024).toFixed(2)} MB`}
                />
              </Box>
            </Box>
          </Paper>
        )}

        {/* Аудио статистика */}
        {currentStats.audio && (
          <Paper sx={{ p: 2, mb: 3, backgroundColor: '#252525', backgroundImage: 'none' }}>
            <Typography variant="subtitle1" sx={{ mb: 2, fontWeight: 'bold' }}>
              🔊 Аудио
            </Typography>
            <Box sx={{ display: 'flex', gap: 2, flexWrap: 'wrap' }}>
              <Box sx={{ flex: '1 1 45%' }}>
                <DetailItem label="Кодек" value={currentStats.audio.codec.toUpperCase()} />
                <DetailItem label="Битрейт" value={`${currentStats.audio.bitrate} kbps`} />
                <DetailItem label="Уровень аудио" value={`${(currentStats.audio.audioLevel * 100).toFixed(0)}%`} />
              </Box>
              <Box sx={{ flex: '1 1 45%' }}>
                <DetailItem label="Пакетов получено" value={currentStats.audio.packetsReceived.toLocaleString()} />
                <DetailItem label="Пакетов потеряно" value={currentStats.audio.packetsLost.toLocaleString()} />
                <DetailItem label="Потери пакетов" value={`${currentStats.audio.packetLossRate}%`} />
                <DetailItem label="Jitter" value={`${currentStats.audio.jitter} ms`} />
              </Box>
            </Box>
          </Paper>
        )}

        {/* Сетевая статистика */}
        {currentStats.network && (
          <Paper sx={{ p: 2, mb: 3, backgroundColor: '#252525', backgroundImage: 'none' }}>
            <Typography variant="subtitle1" sx={{ mb: 2, fontWeight: 'bold' }}>
              🌐 Сеть
            </Typography>
            <Box sx={{ display: 'flex', gap: 2, flexWrap: 'wrap' }}>
              <Box sx={{ flex: '1 1 45%' }}>
                <DetailItem label="Текущий RTT" value={`${currentStats.network.currentRoundTripTime} ms`} />
                <DetailItem label="Протокол" value={currentStats.network.protocol.toUpperCase()} />
                <DetailItem label="Локальный кандидат" value={currentStats.network.localCandidateType} />
                <DetailItem label="Удалённый кандидат" value={currentStats.network.remoteCandidateType} />
              </Box>
              <Box sx={{ flex: '1 1 45%' }}>
                <DetailItem label="Локальный адрес" value={currentStats.network.localAddress} />
                <DetailItem label="Удалённый адрес" value={currentStats.network.remoteAddress} />
                <DetailItem 
                  label="Отправлено" 
                  value={`${(currentStats.network.bytesSent / 1024).toFixed(0)} KB`}
                />
                <DetailItem 
                  label="Получено" 
                  value={`${(currentStats.network.bytesReceived / 1024).toFixed(0)} KB`}
                />
              </Box>
            </Box>
          </Paper>
        )}

        {/* Графики */}
        {chartData.length > 1 && (
          <>
            {/* График битрейта и FPS */}
            <Paper sx={{ p: 2, mb: 3, backgroundColor: '#252525', backgroundImage: 'none' }}>
              <Typography variant="subtitle1" sx={{ mb: 2, fontWeight: 'bold' }}>
                📊 Битрейт и FPS
              </Typography>
              <ResponsiveContainer width="100%" height={200}>
                <LineChart data={chartData}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#333" />
                  <XAxis dataKey="time" stroke="#aaa" tick={{ fontSize: 12 }} />
                  <YAxis yAxisId="left" stroke="#aaa" tick={{ fontSize: 12 }} />
                  <YAxis yAxisId="right" orientation="right" stroke="#aaa" tick={{ fontSize: 12 }} />
                  <Tooltip
                    contentStyle={{ backgroundColor: '#1e1e1e', border: '1px solid #333' }}
                    labelStyle={{ color: '#fff' }}
                  />
                  <Legend />
                  <Line yAxisId="left" type="monotone" dataKey="bitrate" stroke="#2196f3" name="Битрейт (kbps)" dot={false} />
                  <Line yAxisId="right" type="monotone" dataKey="frameRate" stroke="#4caf50" name="FPS" dot={false} />
                </LineChart>
              </ResponsiveContainer>
            </Paper>

            {/* График потерь пакетов и jitter */}
            <Paper sx={{ p: 2, mb: 3, backgroundColor: '#252525', backgroundImage: 'none' }}>
              <Typography variant="subtitle1" sx={{ mb: 2, fontWeight: 'bold' }}>
                📊 Потери пакетов и Jitter
              </Typography>
              <ResponsiveContainer width="100%" height={200}>
                <LineChart data={chartData}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#333" />
                  <XAxis dataKey="time" stroke="#aaa" tick={{ fontSize: 12 }} />
                  <YAxis yAxisId="left" stroke="#aaa" tick={{ fontSize: 12 }} />
                  <YAxis yAxisId="right" orientation="right" stroke="#aaa" tick={{ fontSize: 12 }} />
                  <Tooltip
                    contentStyle={{ backgroundColor: '#1e1e1e', border: '1px solid #333' }}
                    labelStyle={{ color: '#fff' }}
                  />
                  <Legend />
                  <Line yAxisId="left" type="monotone" dataKey="packetLoss" stroke="#f44336" name="Потери пакетов (%)" dot={false} />
                  <Line yAxisId="right" type="monotone" dataKey="jitter" stroke="#ff9800" name="Jitter (ms)" dot={false} />
                </LineChart>
              </ResponsiveContainer>
            </Paper>

            {/* График RTT */}
            <Paper sx={{ p: 2, mb: 3, backgroundColor: '#252525', backgroundImage: 'none' }}>
              <Typography variant="subtitle1" sx={{ mb: 2, fontWeight: 'bold' }}>
                📊 Round Trip Time (RTT)
              </Typography>
              <ResponsiveContainer width="100%" height={200}>
                <LineChart data={chartData}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#333" />
                  <XAxis dataKey="time" stroke="#aaa" tick={{ fontSize: 12 }} />
                  <YAxis stroke="#aaa" tick={{ fontSize: 12 }} />
                  <Tooltip
                    contentStyle={{ backgroundColor: '#1e1e1e', border: '1px solid #333' }}
                    labelStyle={{ color: '#fff' }}
                  />
                  <Legend />
                  <Line type="monotone" dataKey="rtt" stroke="#9c27b0" name="RTT (ms)" dot={false} />
                </LineChart>
              </ResponsiveContainer>
            </Paper>
          </>
        )}
      </DialogContent>
    </Dialog>
  );
};

// Компонент для отображения детальной информации
const DetailItem: React.FC<{ label: string; value: string | number }> = ({ label, value }) => (
  <Box sx={{ display: 'flex', justifyContent: 'space-between', py: 0.5 }}>
    <Typography variant="body2" sx={{ color: '#aaa' }}>
      {label}:
    </Typography>
    <Typography variant="body2" sx={{ color: '#fff', fontWeight: 'bold' }}>
      {value}
    </Typography>
  </Box>
);

// Компонент для отображения чипа с информацией
const InfoChip: React.FC<{ label: string; value: string; color: string }> = ({ label, value, color }) => (
  <Box>
    <Typography variant="caption" sx={{ color: '#aaa', display: 'block', mb: 0.5 }}>
      {label}
    </Typography>
    <Chip
      label={value}
      size="small"
      sx={{
        backgroundColor: `${color}20`,
        color: color,
        fontWeight: 'bold',
        borderRadius: 1,
      }}
    />
  </Box>
);

export default WebRTCStatsDialog;
