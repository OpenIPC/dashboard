import React, { useCallback, useEffect, useMemo, useState } from 'react';
import { Box, IconButton, Paper, Slider, Typography, Stack, Grid, Divider } from '@mui/material';
import { alpha, styled } from '@mui/material/styles';
import {
  North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest,
  ZoomIn as ZoomInIcon,
  ZoomOut as ZoomOutIcon,
  Close as CloseIcon,
  FiberManualRecord as StopIcon,
  Add,
  Remove,
} from '@mui/icons-material';
import { invoke } from '@tauri-apps/api/core';
import type { Camera } from '../types';

interface PTZControlsProps {
  camera: Camera;
  onClose?: () => void;
  scale?: number;
}

const SPEED_CURVE_EXPONENT = 2.0;
const SPEED_MULTIPLIERS = {
  panTilt: 1.0,
  zoom: 0.75,
  focus: 0.55,
};

const clampValue = (value: number, min = 0, max = 1) => Math.min(max, Math.max(min, value));

const RoundPTZButton = styled('button')(({ theme }) => ({
  border: 'none',
  outline: 'none',
  borderRadius: '50%',
  width: 32,
  height: 32,
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
  cursor: 'pointer',
  font: 'inherit',
  lineHeight: 1,
  backgroundColor: 'rgba(255,255,255,0.08)',
  color: theme.palette.text.primary,
  transition: 'background-color 140ms ease, color 140ms ease, transform 140ms ease',
  willChange: 'background-color, color',
  padding: 0,
  '&:hover:not(:disabled), &:focus-visible:not(:disabled)': {
    backgroundColor: alpha(theme.palette.primary.main, 0.9),
    color: theme.palette.common.white,
  },
  '&:active:not(:disabled)': {
    backgroundColor: alpha(theme.palette.primary.dark, 0.95),
    transform: 'scale(0.94)',
  },
  '&:disabled': {
    cursor: 'default',
    color: theme.palette.text.disabled,
    backgroundColor: 'rgba(255,255,255,0.08)',
    opacity: 0.7,
  },
}));

const CircleStopButton = styled(RoundPTZButton)(({ theme }) => ({
  '&:hover:not(:disabled), &:focus-visible:not(:disabled)': {
    backgroundColor: alpha(theme.palette.error.main, 0.9),
    color: theme.palette.common.white,
  },
  '&:active:not(:disabled)': {
    backgroundColor: alpha(theme.palette.error.dark, 0.95),
  },
}));

const DPadButton = styled(RoundPTZButton)({
  width: '100%',
  height: '100%',
  maxWidth: 72,
  minHeight: 40,
  aspectRatio: '1 / 1',
  justifySelf: 'center',
});

const PTZControls: React.FC<PTZControlsProps> = ({ camera, onClose, scale = 1 }) => {
  const [speed, setSpeed] = useState<number>(0.5);
  const [resolvedPass, setResolvedPass] = useState('');
  const [passwordReady, setPasswordReady] = useState(false);

  useEffect(() => {
    let cancelled = false;
    setPasswordReady(false);
    setResolvedPass('');

    if (!camera) {
      setPasswordReady(true);
      return;
    }

    const resolvePassword = async () => {
      try {
        const pass = camera.pass_enc
          ? await invoke<string>('decrypt_password', { enc: camera.pass_enc })
          : camera.pass || '';

        if (!cancelled) {
          setResolvedPass(pass);
          setPasswordReady(true);
        }
      } catch (error) {
        console.error('Failed to resolve camera password:', error);
        if (!cancelled) {
          setResolvedPass('');
          setPasswordReady(true);
        }
      }
    };

    resolvePassword();

    return () => {
      cancelled = true;
    };
  }, [camera]);

  const controlsDisabled = useMemo(() => !camera || !passwordReady, [camera, passwordReady]);
  const curvedBaseSpeed = useMemo(() => Math.pow(speed, SPEED_CURVE_EXPONENT), [speed]);
  const getAxisSpeed = useCallback(
    (multiplier: number) => clampValue(curvedBaseSpeed * multiplier),
    [curvedBaseSpeed]
  );

  const handleMove = useCallback(async (x: number, y: number) => {
    if (!camera || controlsDisabled) return;
    
    try {
      const panTiltSpeed = getAxisSpeed(SPEED_MULTIPLIERS.panTilt);
      await invoke('ptz_move', {
        ip: camera.ip,
        port: camera.onvifPort || 80,
        user: camera.user,
        pass: resolvedPass,
        x: x * panTiltSpeed,
        y: y * panTiltSpeed,
        zoom: 0.0
      });
    } catch (error) {
      console.error('PTZ Move failed:', error);
    }
  }, [camera, controlsDisabled, getAxisSpeed, resolvedPass]);

  const handleZoom = useCallback(async (zoomDirection: number) => {
    if (!camera || controlsDisabled) return;
    
    try {
      const zoomSpeed = getAxisSpeed(SPEED_MULTIPLIERS.zoom);
      await invoke('ptz_move', {
        ip: camera.ip,
        port: camera.onvifPort || 80,
        user: camera.user,
        pass: resolvedPass,
        x: 0.0,
        y: 0.0,
        zoom: zoomDirection * zoomSpeed
      });
    } catch (error) {
      console.error('PTZ Zoom failed:', error);
    }
  }, [camera, controlsDisabled, getAxisSpeed, resolvedPass]);

  const handleFocus = useCallback(async (focusDirection: number) => {
    if (!camera || controlsDisabled) return;
    try {
      const focusSpeed = getAxisSpeed(SPEED_MULTIPLIERS.focus);
      await invoke('ptz_focus', {
        ip: camera.ip,
        port: camera.onvifPort || 80,
        user: camera.user,
        pass: resolvedPass,
        speed: focusDirection * focusSpeed
      });
    } catch (error) {
      console.error('PTZ Focus failed:', error);
    }
  }, [camera, controlsDisabled, getAxisSpeed, resolvedPass]);

  const handleStop = useCallback(async () => {
    if (!camera || controlsDisabled) return;
    
    try {
      // Stop both PTZ and Focus
      await invoke('ptz_stop', {
        ip: camera.ip,
        port: camera.onvifPort || 80,
        user: camera.user,
        pass: resolvedPass
      });
      
      await invoke('focus_stop', {
        ip: camera.ip,
        port: camera.onvifPort || 80,
        user: camera.user,
        pass: resolvedPass
      });
    } catch (error) {
      console.error('PTZ Stop failed:', error);
    }
  }, [camera, controlsDisabled, resolvedPass]);

  const triggerStop = useCallback(() => {
    void handleStop();
  }, [handleStop]);

  const DirectionButton = ({ icon: Icon, x, y }: { icon: any, x: number, y: number }) => (
    <DPadButton
      type="button"
      disabled={controlsDisabled}
      onMouseDown={() => handleMove(x, y)}
      onMouseUp={triggerStop}
      onMouseLeave={triggerStop}
    >
      <Icon sx={{ fontSize: 20 }} />
    </DPadButton>
  );

  return (
    <Paper 
      elevation={6}
      sx={{
        position: 'absolute',
        bottom: 10,
        right: 10,
        zIndex: 1000,
        bgcolor: 'background.paper',
        borderRadius: 1,
        p: 1,
        width: 240,
        border: '1px solid',
        borderColor: 'divider',
        pointerEvents: 'auto',
        transform: `scale(${scale})`,
        transformOrigin: 'bottom right',
      }}
      onClick={(e) => e.stopPropagation()}
    >
      {/* Header */}
      <Box 
        display="flex" 
        justifyContent="space-between" 
        alignItems="center" 
        mb={1} 
        px={1}
      >
        <Typography variant="subtitle2" fontWeight="bold">PTZ Control</Typography>
        <IconButton size="small" onClick={onClose} sx={{ p: 0.5 }}>
          <CloseIcon fontSize="small" />
        </IconButton>
      </Box>

      <Divider sx={{ mb: 2 }} />

      <Grid container spacing={2}>
        {/* D-Pad Section */}
        <Grid size={{ xs: 7 }}>
          <Box
            display="grid"
            gridTemplateColumns="repeat(3, minmax(40px, 1fr))"
            gridAutoRows="minmax(40px, 1fr)"
            gap={0.4}
            justifyItems="stretch"
            alignItems="stretch"
          >
            <DirectionButton icon={NorthWest} x={-1} y={1} />
            <DirectionButton icon={North} x={0} y={1} />
            <DirectionButton icon={NorthEast} x={1} y={1} />
            
            <DirectionButton icon={West} x={-1} y={0} />
            <CircleStopButton type="button" onClick={triggerStop} style={{ width: '100%', height: '100%' }}>
              <StopIcon fontSize="small" />
            </CircleStopButton>
            <DirectionButton icon={East} x={1} y={0} />
            
            <DirectionButton icon={SouthWest} x={-1} y={-1} />
            <DirectionButton icon={South} x={0} y={-1} />
            <DirectionButton icon={SouthEast} x={1} y={-1} />
          </Box>
        </Grid>

        {/* Zoom/Focus Section */}
        <Grid size={{ xs: 5 }}>
          <Stack spacing={1}>
            {/* Zoom */}
            <Box display="flex" alignItems="center" justifyContent="space-between">
              <Typography variant="caption" color="text.secondary">Zoom</Typography>
              <Box>
                <RoundPTZButton
                  type="button"
                  disabled={controlsDisabled}
                  onMouseDown={() => handleZoom(1)}
                  onMouseUp={triggerStop}
                  onMouseLeave={triggerStop}
                >
                  <ZoomInIcon fontSize="small" />
                </RoundPTZButton>
                <RoundPTZButton
                  type="button"
                  disabled={controlsDisabled}
                  onMouseDown={() => handleZoom(-1)}
                  onMouseUp={triggerStop}
                  onMouseLeave={triggerStop}
                >
                  <ZoomOutIcon fontSize="small" />
                </RoundPTZButton>
              </Box>
            </Box>

            {/* Focus */}
            <Box display="flex" alignItems="center" justifyContent="space-between">
              <Typography variant="caption" color="text.secondary">Focus</Typography>
              <Box>
                <RoundPTZButton
                  type="button"
                  disabled={controlsDisabled}
                  onMouseDown={() => handleFocus(1)}
                  onMouseUp={triggerStop}
                  onMouseLeave={triggerStop}
                >
                  <Add fontSize="small" />
                </RoundPTZButton>
                <RoundPTZButton
                  type="button"
                  disabled={controlsDisabled}
                  onMouseDown={() => handleFocus(-1)}
                  onMouseUp={triggerStop}
                  onMouseLeave={triggerStop}
                >
                  <Remove fontSize="small" />
                </RoundPTZButton>
              </Box>
            </Box>

          </Stack>
        </Grid>
      </Grid>

      {/* Speed Slider */}
      <Box mt={2} px={1}>
        <Box display="flex" justifyContent="space-between" mb={0.5}>
          <Typography variant="caption">Speed</Typography>
          <Typography variant="caption">{Math.round(speed * 10)}</Typography>
        </Box>
        <Slider
          size="small"
          value={speed}
          min={0.1}
          max={1.0}
          step={0.1}
          onChange={(_, val) => setSpeed(val as number)}
          sx={{ py: 1 }}
          disabled={controlsDisabled}
        />
      </Box>

      {!passwordReady && (
        <Box mt={1} px={1}>
          <Typography variant="caption" color="text.secondary">
            Подключаемся к камере…
          </Typography>
        </Box>
      )}
    </Paper>
  );
};
export default PTZControls;
