import React, { useEffect, useMemo, useState } from 'react';
import {
  Box,
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Divider,
  FormControl,
  FormControlLabel,
  InputLabel,
  MenuItem,
  Select,
  Stack,
  Switch,
  TextField,
  ToggleButton,
  ToggleButtonGroup,
  Typography,
} from '@mui/material';
import type { SelectChangeEvent } from '@mui/material';
import VideoStreamPlayer from '../../VideoStreamPlayer';
import { useLocalization } from '../../../hooks/useLocalization';
import { useAppState } from '../../../hooks/useAppState';
import type { Camera } from '../../../types';
import type {
  ObjectCounterLineInput,
  ObjectCounterZoneInput,
  ObjectCounterPoint,
} from '../../../services/analytics';
import RegionDrawingCanvas, {
  type RegionLineDraft,
  type RegionZoneDraft,
} from './RegionDrawingCanvas';
import type { RegionDesignerMode } from './RegionDesignerTypes';

interface RegionDesignerDialogProps {
  open: boolean;
  mode: RegionDesignerMode;
  camera: Camera | null;
  cameras: Camera[];
  lineDraft?: ObjectCounterLineInput | null;
  zoneDraft?: ObjectCounterZoneInput | null;
  isSaving?: boolean;
  errorMessage?: string | null;
  onClose: () => void;
  onSaveLine: (payload: ObjectCounterLineInput) => void;
  onSaveZone: (payload: ObjectCounterZoneInput) => void;
  onDraftChange?: (next: ObjectCounterLineInput | ObjectCounterZoneInput) => void;
  onCameraChange?: (cameraId: number) => void;
}

const CENTER_POINT: ObjectCounterPoint = { x: 0.5, y: 0.5 };

const defaultLine: ObjectCounterLineInput = {
  cameraId: 0,
  name: '',
  start: { ...CENTER_POINT },
  end: { ...CENTER_POINT },
  direction: 'bidirectional',
  enabled: true,
};

const defaultZone: ObjectCounterZoneInput = {
  cameraId: 0,
  name: '',
  polygon: [],
  zoneType: 'intrusion',
  enabled: true,
};

const clamp01 = (value: number) => Math.max(0, Math.min(1, value));

const normalizePoint = (point: ObjectCounterPoint) => ({
  x: clamp01(point.x ?? 0),
  y: clamp01(point.y ?? 0),
});

const normalizeLine = (line: ObjectCounterLineInput, cameraId: number): ObjectCounterLineInput => ({
  ...defaultLine,
  ...line,
  cameraId,
  start: normalizePoint(line.start ?? defaultLine.start),
  end: normalizePoint(line.end ?? defaultLine.end),
});

const normalizeZone = (zone: ObjectCounterZoneInput, cameraId: number): ObjectCounterZoneInput => ({
  ...defaultZone,
  ...zone,
  cameraId,
  polygon: (zone.polygon?.length ? zone.polygon : defaultZone.polygon).map(normalizePoint),
});

const normalizeLineDraftForCanvas = (line: ObjectCounterLineInput): RegionLineDraft => ({
  id: line.id,
  start: { ...line.start },
  end: { ...line.end },
  direction: line.direction as 'bidirectional' | 'incoming' | 'outgoing' | undefined,
});

const normalizeZoneDraftForCanvas = (zone: ObjectCounterZoneInput): RegionZoneDraft => ({
  id: zone.id,
  polygon: zone.polygon.map(point => ({ ...point })),
});

const RegionDesignerDialog: React.FC<RegionDesignerDialogProps> = ({
  open,
  mode,
  camera,
  cameras,
  lineDraft,
  zoneDraft,
  isSaving = false,
  errorMessage,
  onClose,
  onSaveLine,
  onSaveZone,
  onDraftChange,
  onCameraChange,
}) => {
  const { t } = useLocalization();
  const { ensureStreamingBackendStarted, prewarmCameraStreams } = useAppState();
  const [activeLineDraft, setActiveLineDraft] = useState<ObjectCounterLineInput | null>(null);
  const [activeZoneDraft, setActiveZoneDraft] = useState<ObjectCounterZoneInput | null>(null);
  const [quality, setQuality] = useState<'sd' | 'hd'>('sd');
  const [videoElement, setVideoElement] = useState<HTMLVideoElement | null>(null);
  const [isPreparingStream, setIsPreparingStream] = useState(false);
  const [prewarmError, setPrewarmError] = useState<string | null>(null);

  const streamName = useMemo(() => {
    if (!camera?.id) {
      return null;
    }
    return `cam${camera.id}`;
  }, [camera?.id]);

  useEffect(() => {
    if (!open) {
      setVideoElement(null);
    }
  }, [open]);

  useEffect(() => {
    if (!open || !camera) {
      return;
    }
    let cancelled = false;
    const prepare = async () => {
      setIsPreparingStream(true);
      setPrewarmError(null);
      try {
        await ensureStreamingBackendStarted();
        await prewarmCameraStreams(camera);
      } catch (err) {
        if (!cancelled) {
          setPrewarmError(err instanceof Error ? err.message : String(err));
        }
      } finally {
        if (!cancelled) {
          setIsPreparingStream(false);
        }
      }
    };
    void prepare();
    return () => {
      cancelled = true;
    };
  }, [camera, ensureStreamingBackendStarted, open, prewarmCameraStreams]);

  useEffect(() => {
    if (!camera) {
      setActiveLineDraft(null);
      setActiveZoneDraft(null);
      return;
    }
    // Only update state from props if the ID changes or we don't have a draft yet.
    // This prevents overwriting local state with prop updates during editing (dragging).
    if (mode === 'line') {
      if (!activeLineDraft || activeLineDraft.id !== lineDraft?.id) {
        const normalized = normalizeLine(lineDraft ?? defaultLine, camera.id);
        setActiveLineDraft(normalized);
        onDraftChange?.(normalized);
      }
    } else {
      if (!activeZoneDraft || activeZoneDraft.id !== zoneDraft?.id) {
        const normalized = normalizeZone(zoneDraft ?? defaultZone, camera.id);
        setActiveZoneDraft(normalized);
        onDraftChange?.(normalized);
      }
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [camera, mode, lineDraft?.id, zoneDraft?.id]);

  const handleQualityToggle = (_: React.MouseEvent<HTMLElement>, next: 'sd' | 'hd' | null) => {
    if (next) {
      setQuality(next);
    }
  };

  const handleLineFieldChange = (patch: Partial<ObjectCounterLineInput>) => {
    setActiveLineDraft(prev => {
      if (!prev) {
        return prev;
      }
      const next = { ...prev, ...patch };
      onDraftChange?.(next);
      return next;
    });
  };

  const handleZoneFieldChange = (patch: Partial<ObjectCounterZoneInput>) => {
    setActiveZoneDraft(prev => {
      if (!prev) {
        return prev;
      }
      const next = { ...prev, ...patch };
      onDraftChange?.(next);
      return next;
    });
  };

  const handleCameraSelect = (event: SelectChangeEvent<string>) => {
    const numeric = Number(event.target.value);
    if (!Number.isFinite(numeric)) {
      return;
    }
    if (mode === 'line' && activeLineDraft) {
      handleLineFieldChange({ cameraId: numeric });
    } else if (mode === 'zone' && activeZoneDraft) {
      handleZoneFieldChange({ cameraId: numeric });
    }
    onCameraChange?.(numeric);
  };

  const handleLineCanvasChange = (draft: RegionLineDraft | null) => {
    if (!draft || !activeLineDraft) {
      return;
    }
    const next: ObjectCounterLineInput = {
      ...activeLineDraft,
      start: { ...draft.start },
      end: { ...draft.end },
    };
    setActiveLineDraft(next);
    onDraftChange?.(next);
  };

  const handleZoneCanvasChange = (draft: RegionZoneDraft | null) => {
    if (!draft || !activeZoneDraft) {
      return;
    }
    const next: ObjectCounterZoneInput = {
      ...activeZoneDraft,
      polygon: draft.polygon.map((point: ObjectCounterPoint) => ({ ...point })),
    };
    setActiveZoneDraft(next);
    onDraftChange?.(next);
  };

  const saveLine = () => {
    if (!activeLineDraft) {
      return;
    }
    onSaveLine(activeLineDraft);
  };

  const saveZone = () => {
    if (!activeZoneDraft) {
      return;
    }
    onSaveZone(activeZoneDraft);
  };

  const isLineValid = Boolean(activeLineDraft?.name?.trim());
  const isZoneValid = Boolean(activeZoneDraft?.name?.trim() && activeZoneDraft?.polygon?.length >= 3);

  return (
    <Dialog open={open} onClose={onClose} fullScreen maxWidth="lg">
      <DialogTitle>
        {mode === 'line'
          ? t('analytics_object_counter_line_dialog_create')
          : t('analytics_object_counter_zone_dialog_create')}
      </DialogTitle>
      <DialogContent dividers sx={{ p: 0 }}>
        <Stack direction={{ xs: 'column', md: 'row' }} sx={{ height: '100%' }}>
          <Box sx={{ flex: 2, position: 'relative', backgroundColor: '#000', minHeight: 300 }}>
            {streamName && (
              <VideoStreamPlayer
                streamName={streamName}
                useHdQuality={quality === 'hd'}
                showMonitor={false}
                enableSnapshot={false}
                enable2WayAudio={false}
                controls={false}
                autoPlay
                muted
                objectFit="contain"
                height="100%"
                width="100%"
                onVideoRef={ref => {
                  setVideoElement(ref);
                }}
              />
            )}
            <RegionDrawingCanvas
              mode={mode}
              videoElement={videoElement}
              lineDraft={activeLineDraft ? normalizeLineDraftForCanvas(activeLineDraft) : null}
              zoneDraft={activeZoneDraft ? normalizeZoneDraftForCanvas(activeZoneDraft) : null}
              onLineChange={handleLineCanvasChange}
              onZoneChange={handleZoneCanvasChange}
            />
            <Box
              sx={{
                position: 'absolute',
                top: 16,
                right: 16,
                zIndex: 20,
                backgroundColor: 'rgba(0,0,0,0.55)',
                borderRadius: 2,
                p: 0.5,
              }}
            >
              <ToggleButtonGroup
                size="small"
                color="primary"
                exclusive
                value={quality}
                onChange={handleQualityToggle}
              >
                <ToggleButton value="sd">SD</ToggleButton>
                <ToggleButton value="hd">HD</ToggleButton>
              </ToggleButtonGroup>
            </Box>
            {isPreparingStream && (
              <Box
                sx={{
                  position: 'absolute',
                  inset: 0,
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  bgcolor: 'rgba(0,0,0,0.6)',
                  color: '#fff',
                }}
              >
                <Typography variant="body1">{t('analytics_object_counter_preparing_stream')}</Typography>
              </Box>
            )}
            {prewarmError && (
              <Box
                sx={{
                  position: 'absolute',
                  bottom: 16,
                  left: 16,
                  right: 16,
                  bgcolor: 'rgba(50,50,50,0.9)',
                  p: 2,
                  borderRadius: 1,
                }}
              >
                <Typography variant="body2" color="error">
                  {prewarmError}
                </Typography>
              </Box>
            )}
          </Box>
          <Divider flexItem orientation="vertical" />
          <Box sx={{ flex: 1, p: 3 }}>
            <Stack spacing={2}>
              <Typography variant="h6">
                {t('analytics_object_counter_regions_title')}
              </Typography>
              {mode === 'line' && activeLineDraft && (
                <>
                  <FormControl size="small" fullWidth>
                    <InputLabel>{t('snapshots_filter_camera')}</InputLabel>
                    <Select
                      label={t('snapshots_filter_camera')}
                      value={String(activeLineDraft.cameraId)}
                      onChange={handleCameraSelect}
                    >
                      {cameras.map(item => (
                        <MenuItem key={item.id} value={String(item.id)}>
                          {item.name}
                        </MenuItem>
                      ))}
                    </Select>
                  </FormControl>
                  <TextField
                    label={t('analytics_object_counter_line_name')}
                    value={activeLineDraft.name}
                    onChange={event => handleLineFieldChange({ name: event.target.value })}
                    fullWidth
                  />
                  <FormControl fullWidth size="small">
                    <InputLabel>{t('analytics_object_counter_line_direction')}</InputLabel>
                    <Select
                      label={t('analytics_object_counter_line_direction')}
                      value={activeLineDraft.direction}
                      onChange={(event: SelectChangeEvent<string>) =>
                        handleLineFieldChange({ direction: event.target.value })
                      }
                    >
                      <MenuItem value="bidirectional">{t('analytics_object_counter_line_direction_bidirectional')}</MenuItem>
                      <MenuItem value="forward">{t('analytics_object_counter_line_direction_forward')}</MenuItem>
                      <MenuItem value="backward">{t('analytics_object_counter_line_direction_backward')}</MenuItem>
                    </Select>
                  </FormControl>
                  <TextField
                    label={t('analytics_object_counter_line_object_type')}
                    value={activeLineDraft.objectType ?? ''}
                    onChange={event =>
                      handleLineFieldChange({ objectType: event.target.value.trim() || undefined })
                    }
                    placeholder={t('analytics_object_counter_object_type_any')}
                    fullWidth
                  />
                  <FormControlLabel
                    control={
                      <Switch
                        checked={activeLineDraft.enabled}
                        onChange={(_, checked) => handleLineFieldChange({ enabled: checked })}
                      />
                    }
                    label={t('analytics_object_counter_line_enabled')}
                  />
                </>
              )}
              {mode === 'zone' && activeZoneDraft && (
                <>
                  <FormControl size="small" fullWidth>
                    <InputLabel>{t('snapshots_filter_camera')}</InputLabel>
                    <Select
                      label={t('snapshots_filter_camera')}
                      value={String(activeZoneDraft.cameraId)}
                      onChange={handleCameraSelect}
                    >
                      {cameras.map(item => (
                        <MenuItem key={item.id} value={String(item.id)}>
                          {item.name}
                        </MenuItem>
                      ))}
                    </Select>
                  </FormControl>
                  <TextField
                    label={t('analytics_object_counter_zone_name')}
                    value={activeZoneDraft.name}
                    onChange={event => handleZoneFieldChange({ name: event.target.value })}
                    fullWidth
                  />
                  <FormControl fullWidth size="small">
                    <InputLabel>{t('analytics_object_counter_zone_type')}</InputLabel>
                    <Select
                      label={t('analytics_object_counter_zone_type')}
                      value={activeZoneDraft.zoneType}
                      onChange={(event: SelectChangeEvent<string>) =>
                        handleZoneFieldChange({ zoneType: event.target.value })
                      }
                    >
                      <MenuItem value="intrusion">{t('analytics_object_counter_zone_type_intrusion')}</MenuItem>
                      <MenuItem value="loitering">{t('analytics_object_counter_zone_type_loitering')}</MenuItem>
                      <MenuItem value="presence">{t('analytics_object_counter_zone_type_presence')}</MenuItem>
                    </Select>
                  </FormControl>
                  <TextField
                    label={t('analytics_object_counter_zone_object_type')}
                    value={activeZoneDraft.objectType ?? ''}
                    onChange={event =>
                      handleZoneFieldChange({ objectType: event.target.value.trim() || undefined })
                    }
                    placeholder={t('analytics_object_counter_object_type_any')}
                    fullWidth
                  />
                  <TextField
                    label={t('analytics_object_counter_zone_dwell')}
                    type="number"
                    value={activeZoneDraft.dwellThresholdMs ?? ''}
                    onChange={event =>
                      handleZoneFieldChange({
                        dwellThresholdMs: event.target.value ? Number(event.target.value) : undefined,
                      })
                    }
                    helperText={t('analytics_object_counter_zone_dwell_hint', { value: activeZoneDraft.dwellThresholdMs ?? 0 })}
                    fullWidth
                  />
                  <FormControlLabel
                    control={
                      <Switch
                        checked={activeZoneDraft.enabled}
                        onChange={(_, checked) => handleZoneFieldChange({ enabled: checked })}
                      />
                    }
                    label={t('analytics_object_counter_zone_enabled')}
                  />
                </>
              )}
            </Stack>
          </Box>
        </Stack>
      </DialogContent>
      <DialogActions>
        {errorMessage && (
          <Typography variant="body2" color="error" sx={{ flexGrow: 1 }}>
            {errorMessage}
          </Typography>
        )}
        <Button onClick={onClose}>{t('cancel')}</Button>
        {mode === 'line' ? (
          <Button onClick={saveLine} variant="contained" disabled={!isLineValid || isSaving}>
            {t('save')}
          </Button>
        ) : (
          <Button onClick={saveZone} variant="contained" disabled={!isZoneValid || isSaving}>
            {t('save')}
          </Button>
        )}
      </DialogActions>
    </Dialog>
  );
};

export default RegionDesignerDialog;
