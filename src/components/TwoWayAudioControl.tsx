/**
 * TwoWayAudioControl Component
 * Enables bidirectional audio communication for intercoms and interactive cameras
 */

import React, { useState, useRef, useEffect } from 'react';
import { Box, IconButton, Tooltip, Typography, Slider, Switch, FormControlLabel } from '@mui/material';
import {
  Mic as MicIcon,
  MicOff as MicOffIcon,
  VolumeUp as VolumeUpIcon,
  VolumeOff as VolumeOffIcon,
  Settings as SettingsIcon,
} from '@mui/icons-material';
import type { TwoWayAudioConfig } from '../global';

interface TwoWayAudioControlProps {
  streamName: string;
  enabled?: boolean;
  pushToTalk?: boolean;
  onAudioStateChange?: (active: boolean) => void;
  compact?: boolean;
}

const TwoWayAudioControl: React.FC<TwoWayAudioControlProps> = ({
  streamName,
  enabled = false,
  pushToTalk = false,
  onAudioStateChange,
  compact = false,
}) => {
  const [isActive, setIsActive] = useState(false);
  const [isMuted, setIsMuted] = useState(true);
  const [volume, setVolume] = useState(80);
  const [showSettings, setShowSettings] = useState(false);
  const [config, setConfig] = useState<TwoWayAudioConfig>({
    enabled: true,
    codec: 'opus',
    sampleRate: 48000,
    channels: 1,
    pushToTalk: pushToTalk,
    echoCancellation: true,
    noiseSuppression: true,
    autoGainControl: true,
  });

  const audioContextRef = useRef<AudioContext | null>(null);
  const mediaStreamRef = useRef<MediaStream | null>(null);
  const gainNodeRef = useRef<GainNode | null>(null);

  useEffect(() => {
    return () => {
      stopAudio();
    };
  }, []);

  const startAudio = async () => {
    try {
      const stream = await navigator.mediaDevices.getUserMedia({
        audio: {
          echoCancellation: config.echoCancellation,
          noiseSuppression: config.noiseSuppression,
          autoGainControl: config.autoGainControl,
          sampleRate: config.sampleRate,
          channelCount: config.channels,
        },
      });

      mediaStreamRef.current = stream;

      // Create audio context for volume control
      const audioContext = new AudioContext();
      audioContextRef.current = audioContext;

      const source = audioContext.createMediaStreamSource(stream);
      const gainNode = audioContext.createGain();
      gainNodeRef.current = gainNode;
      
      gainNode.gain.value = volume / 100;
      source.connect(gainNode);

      setIsActive(true);
      setIsMuted(false);
      onAudioStateChange?.(true);

      console.log('[TwoWayAudio] Started for stream:', streamName);
    } catch (error) {
      console.error('[TwoWayAudio] Failed to start audio:', error);
      alert('Failed to access microphone. Please check permissions.');
    }
  };

  const stopAudio = () => {
    if (mediaStreamRef.current) {
      mediaStreamRef.current.getTracks().forEach(track => track.stop());
      mediaStreamRef.current = null;
    }

    if (audioContextRef.current) {
      audioContextRef.current.close();
      audioContextRef.current = null;
    }

    setIsActive(false);
    setIsMuted(true);
    onAudioStateChange?.(false);

    console.log('[TwoWayAudio] Stopped');
  };

  const toggleMute = () => {
    if (mediaStreamRef.current) {
      mediaStreamRef.current.getAudioTracks().forEach(track => {
        track.enabled = isMuted;
      });
      setIsMuted(!isMuted);
    }
  };

  const handleVolumeChange = (_event: Event, newValue: number | number[]) => {
    const vol = Array.isArray(newValue) ? newValue[0] : newValue;
    setVolume(vol);
    if (gainNodeRef.current) {
      gainNodeRef.current.gain.value = vol / 100;
    }
  };

  const handleMouseDown = () => {
    if (config.pushToTalk && !isActive) {
      startAudio();
    }
  };

  const handleMouseUp = () => {
    if (config.pushToTalk && isActive) {
      stopAudio();
    }
  };

  if (!enabled) {
    return null;
  }

  if (compact) {
    return (
      <Box
        sx={{
          display: 'flex',
          alignItems: 'center',
          gap: 0.5,
          px: 1,
          py: 0.5,
          backgroundColor: 'rgba(0, 0, 0, 0.6)',
          borderRadius: 1,
        }}
      >
        <Tooltip title={config.pushToTalk ? 'Push to Talk' : (isActive ? 'Stop Audio' : 'Start Audio')}>
          <IconButton
            size="small"
            color={isActive ? 'error' : 'default'}
            onMouseDown={handleMouseDown}
            onMouseUp={handleMouseUp}
            onMouseLeave={handleMouseUp}
            onClick={() => {
              if (!config.pushToTalk) {
                if (isActive) stopAudio();
                else startAudio();
              }
            }}
          >
            {isActive ? <MicIcon fontSize="small" /> : <MicOffIcon fontSize="small" />}
          </IconButton>
        </Tooltip>

        {isActive && (
          <Tooltip title={isMuted ? 'Unmute' : 'Mute'}>
            <IconButton size="small" onClick={toggleMute}>
              {isMuted ? <VolumeOffIcon fontSize="small" /> : <VolumeUpIcon fontSize="small" />}
            </IconButton>
          </Tooltip>
        )}
      </Box>
    );
  }

  return (
    <Box
      sx={{
        p: 2,
        backgroundColor: 'rgba(0, 0, 0, 0.7)',
        borderRadius: 1,
        border: '1px solid rgba(255, 255, 255, 0.1)',
      }}
    >
      <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', mb: 2 }}>
        <Typography variant="subtitle2" sx={{ fontWeight: 600 }}>
          Two-Way Audio
        </Typography>
        <IconButton size="small" onClick={() => setShowSettings(!showSettings)}>
          <SettingsIcon fontSize="small" />
        </IconButton>
      </Box>

      <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
        {/* Main control */}
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
          <Tooltip title={config.pushToTalk ? 'Hold to Talk' : (isActive ? 'Stop' : 'Start')}>
            <IconButton
              size="large"
              color={isActive ? 'error' : 'primary'}
              onMouseDown={handleMouseDown}
              onMouseUp={handleMouseUp}
              onMouseLeave={handleMouseUp}
              onClick={() => {
                if (!config.pushToTalk) {
                  if (isActive) stopAudio();
                  else startAudio();
                }
              }}
              sx={{
                border: '2px solid',
                borderColor: isActive ? 'error.main' : 'primary.main',
              }}
            >
              {isActive ? <MicIcon /> : <MicOffIcon />}
            </IconButton>
          </Tooltip>

          <Box sx={{ flex: 1 }}>
            <Typography variant="caption" color="text.secondary">
              {config.pushToTalk
                ? 'Hold button to talk'
                : isActive
                ? 'Audio active'
                : 'Click to start audio'}
            </Typography>
          </Box>

          {isActive && (
            <Tooltip title={isMuted ? 'Unmute' : 'Mute'}>
              <IconButton onClick={toggleMute} color={isMuted ? 'default' : 'primary'}>
                {isMuted ? <VolumeOffIcon /> : <VolumeUpIcon />}
              </IconButton>
            </Tooltip>
          )}
        </Box>

        {/* Volume control */}
        {isActive && !isMuted && (
          <Box>
            <Typography variant="caption" color="text.secondary" gutterBottom>
              Volume: {volume}%
            </Typography>
            <Slider
              value={volume}
              onChange={handleVolumeChange}
              min={0}
              max={100}
              size="small"
            />
          </Box>
        )}

        {/* Settings */}
        {showSettings && (
          <Box
            sx={{
              p: 1.5,
              backgroundColor: 'rgba(255, 255, 255, 0.05)',
              borderRadius: 1,
              display: 'flex',
              flexDirection: 'column',
              gap: 1,
            }}
          >
            <FormControlLabel
              control={
                <Switch
                  checked={config.pushToTalk || false}
                  onChange={(e) => setConfig({ ...config, pushToTalk: e.target.checked })}
                  size="small"
                />
              }
              label={<Typography variant="caption">Push to Talk</Typography>}
            />
            <FormControlLabel
              control={
                <Switch
                  checked={config.echoCancellation || false}
                  onChange={(e) => setConfig({ ...config, echoCancellation: e.target.checked })}
                  size="small"
                />
              }
              label={<Typography variant="caption">Echo Cancellation</Typography>}
            />
            <FormControlLabel
              control={
                <Switch
                  checked={config.noiseSuppression || false}
                  onChange={(e) => setConfig({ ...config, noiseSuppression: e.target.checked })}
                  size="small"
                />
              }
              label={<Typography variant="caption">Noise Suppression</Typography>}
            />
            <FormControlLabel
              control={
                <Switch
                  checked={config.autoGainControl || false}
                  onChange={(e) => setConfig({ ...config, autoGainControl: e.target.checked })}
                  size="small"
                />
              }
              label={<Typography variant="caption">Auto Gain Control</Typography>}
            />
          </Box>
        )}
      </Box>
    </Box>
  );
};

export default TwoWayAudioControl;
