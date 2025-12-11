import React, { useCallback, useEffect, useMemo, useState } from 'react';
import {
  Alert,
  Box,
  Button,
  Chip,
  CircularProgress,
  Divider,
  FormControl,
  InputLabel,
  List,
  ListItem,
  ListItemText,
  MenuItem,
  Paper,
  Select,
  Stack,
  ToggleButton,
  ToggleButtonGroup,
  Tooltip,
  Typography,
} from '@mui/material';
import type { SelectChangeEvent } from '@mui/material';
import { invoke } from '@tauri-apps/api/core';
import FolderOpenIcon from '@mui/icons-material/FolderOpen';
import { useLocalization } from '../../hooks/useLocalization';
import { useAnalytics } from '../../hooks/useAnalytics';
import { useAppState } from '../../hooks/useAppState';
import RegionDesignerDialog from './designer/RegionDesignerDialog';
import type { RegionDesignerMode } from './designer/RegionDesignerTypes';
import {
  deleteObjectCounterLine,
  deleteObjectCounterZone,
  getObjectCounterAggregates,
  getObjectCounterEvents,
  getObjectCounterLines,
  getObjectCounterTopCameras,
  getObjectCounterZones,
  upsertObjectCounterLine,
  upsertObjectCounterZone,
  type ObjectCounterAggregate,
  type ObjectCounterEvent,
  type ObjectCounterLine,
  type ObjectCounterLineInput,
  type ObjectCounterTopCamera,
  type ObjectCounterZone,
  type ObjectCounterZoneInput,
} from '../../services/analytics';
import type { Camera } from '../../types';
import { ResponsiveContainer, AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip as ChartTooltip } from 'recharts';
import VideoStreamPlayer from '../VideoStreamPlayer';
import AnalyticsOverlay from './AnalyticsOverlay';

const PAGE_SIZE = 20;
const TIME_RANGE_OPTIONS = [60, 360, 1440]; // minutes: 1h, 6h, 24h
const DEFAULT_ZONE_POLYGON = [
  { x: 0.2, y: 0.2 },
  { x: 0.8, y: 0.2 },
  { x: 0.8, y: 0.8 },
  { x: 0.2, y: 0.8 },
];

const ImageWithFallback = ({ src, alt, fallback }: { src: string, alt: string, fallback: React.ReactNode }) => {
  const [error, setError] = useState(false);
  
  useEffect(() => {
      setError(false);
  }, [src]);

  if (error || !src) {
      return <>{fallback}</>;
  }

  return (
      <Box 
        component="img" 
        src={src} 
        alt={alt} 
        sx={{ width: '100%', borderRadius: 1, objectFit: 'cover', maxHeight: 100 }} 
        onError={() => setError(true)} 
      />
  );
};

const ObjectCounterPanel: React.FC = () => {
  const { t } = useLocalization();
  const { modules, processFrame, detections } = useAnalytics();
  const { cameras } = useAppState();
  const [events, setEvents] = useState<ObjectCounterEvent[]>([]);
  const [aggregates, setAggregates] = useState<ObjectCounterAggregate[]>([]);
  const [topCameras, setTopCameras] = useState<ObjectCounterTopCamera[]>([]);
  const [lines, setLines] = useState<ObjectCounterLine[]>([]);
  const [zones, setZones] = useState<ObjectCounterZone[]>([]);
  const [designerOpen, setDesignerOpen] = useState(false);
  const [designerMode, setDesignerMode] = useState<RegionDesignerMode>('line');
  const [designerCamera, setDesignerCamera] = useState<Camera | null>(null);
  const [designerLineDraft, setDesignerLineDraft] = useState<ObjectCounterLineInput | null>(null);
  const [designerZoneDraft, setDesignerZoneDraft] = useState<ObjectCounterZoneInput | null>(null);
  const [designerSaving, setDesignerSaving] = useState(false);
  const [designerError, setDesignerError] = useState<string | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [cameraFilter, setCameraFilter] = useState<string>('');
  const [selectedTypes, setSelectedTypes] = useState<string[]>([]);
  const [timeRangeMinutes, setTimeRangeMinutes] = useState<number>(1440);
  const [page, setPage] = useState(0);
  const [hasMore, setHasMore] = useState(false);
  const [imageUrls, setImageUrls] = useState<Map<string, string>>(new Map());

  const cameraNameMap = useMemo(() => {
    const map = new Map<string, string>();
    cameras.forEach(camera => {
      map.set(String(camera.id), camera.name);
    });
    return map;
  }, [cameras]);

  const loadImage = useCallback(
    async (path: string) => {
      if (!path || imageUrls.has(path)) {
        return;
      }
      try {
        const bytes = await invoke<number[]>('read_object_image', { path });
        const blob = new Blob([new Uint8Array(bytes)], { type: 'image/jpeg' });
        const url = URL.createObjectURL(blob);
        setImageUrls(prev => new Map(prev).set(path, url));
      } catch (err) {
        console.error('Failed to load image:', path, err);
        setImageUrls(prev => new Map(prev).set(path, ''));
      }
    },
    [imageUrls],
  );

  useEffect(() => {
    events.forEach(event => {
      if (event.snapshotPath) {
        void loadImage(event.snapshotPath);
      }
    });
  }, [events, loadImage]);

  const defaultCameraId = useMemo(() => {
    if (cameraFilter) {
      const parsed = Number(cameraFilter);
      if (!Number.isNaN(parsed)) {
        return parsed;
      }
    }
    return cameras[0]?.id ?? null;
  }, [cameraFilter, cameras]);

  const [videoElement, setVideoElement] = useState<HTMLVideoElement | null>(null);

  const activeCamera = useMemo(() => {
    if (!cameraFilter) return null;
    return cameras.find(c => String(c.id) === cameraFilter) ?? null;
  }, [cameraFilter, cameras]);

  const activeCameraLines = useMemo(() => {
    if (!activeCamera) return [];
    return lines.filter(l => l.cameraId === activeCamera.id && l.enabled);
  }, [activeCamera, lines]);

  const activeCameraZones = useMemo(() => {
    if (!activeCamera) return [];
    return zones.filter(z => z.cameraId === activeCamera.id && z.enabled);
  }, [activeCamera, zones]);

  const activeCameraDetections = useMemo(() => {
    if (!activeCamera) return [];
    const latestEvent = detections.find(d => d.cameraId === String(activeCamera.id));
    return latestEvent ? latestEvent.detections : [];
  }, [activeCamera, detections]);

  const hasCameras = cameras.length > 0;

  const activeProcessing = useMemo(() => {
    const mod = modules.find(m => m.id === 'object-counter');
    return Boolean(mod?.enabled && mod?.state === 'ready');
  }, [modules]);

  // Frame processing loop for live analytics - DISABLED to prevent freezing
  /*
  useEffect(() => {
    if (!videoElement || !activeCamera || !activeProcessing) {
      return;
    }

    let timerId: number | null = null;
    let isProcessing = false;
    let mounted = true;

    const processLoop = async () => {
      if (!mounted) return;
      
      if (isProcessing) {
        timerId = window.setTimeout(processLoop, 50);
        return;
      }

      isProcessing = true;

      try {
        if (videoElement.paused || videoElement.ended || !videoElement.videoWidth) {
           // Video not ready, wait a bit
        } else {
           const width = videoElement.videoWidth;
           const height = videoElement.videoHeight;
           
           // Limit resolution for performance (e.g. max 640px width)
           const MAX_WIDTH = 640;
           let scale = 1;
           if (width > MAX_WIDTH) {
             scale = MAX_WIDTH / width;
           }
           
           const canvas = document.createElement('canvas');
           canvas.width = Math.round(width * scale);
           canvas.height = Math.round(height * scale);
           const ctx = canvas.getContext('2d');
           
           if (ctx) {
             ctx.drawImage(videoElement, 0, 0, canvas.width, canvas.height);
             const base64 = canvas.toDataURL('image/jpeg', 0.8).split(',')[1];
             
             if (base64) {
               await processFrame({
                 moduleId: 'object-counter',
                 cameraId: String(activeCamera.id),
                 frameBase64: base64,
                 frameWidth: canvas.width,
                 frameHeight: canvas.height,
                 options: {}
               });
             }
           }
        }
      } catch (e) {
        console.warn('[ObjectCounter] Frame processing error:', e);
      } finally {
        isProcessing = false;
        if (mounted) {
          // Target ~5 FPS (200ms)
          timerId = window.setTimeout(processLoop, 200);
        }
      }
    };

    processLoop();

    return () => {
      mounted = false;
      if (timerId) clearTimeout(timerId);
    };
  }, [videoElement, activeCamera, activeProcessing, processFrame]);
  */

  const availableTypes = useMemo(() => {
    const typeSet = new Set<string>();
    events.forEach(event => typeSet.add(event.objectType));
    return Array.from(typeSet).sort();
  }, [events]);

  const resolveCameraLabel = (cameraId?: string | number) => {
    if (!cameraId) {
      return t('snapshots_unknown_camera');
    }
    const key = String(cameraId);
    return cameraNameMap.get(key) ?? key;
  };

  const resolveDirectionLabel = (direction: string) => {
    switch (direction) {
      case 'forward':
        return t('analytics_object_counter_line_direction_forward');
      case 'backward':
        return t('analytics_object_counter_line_direction_backward');
      default:
        return t('analytics_object_counter_line_direction_bidirectional');
    }
  };

  const resolveZoneTypeLabel = (zoneType: string) => {
    switch (zoneType) {
      case 'loitering':
        return t('analytics_object_counter_zone_type_loitering');
      case 'presence':
        return t('analytics_object_counter_zone_type_presence');
      default:
        return t('analytics_object_counter_zone_type_intrusion');
    }
  };

  const resolveObjectLabel = (type: string) => {
    switch (type) {
      case 'person': return t('analytics_object_counter_object_person');
      case 'car': return t('analytics_object_counter_object_car');
      case 'truck': return t('analytics_object_counter_object_truck');
      case 'bus': return t('analytics_object_counter_object_bus');
      case 'bicycle': return t('analytics_object_counter_object_bicycle');
      case 'motorcycle': return t('analytics_object_counter_object_motorcycle');
      default: return type;
    }
  };

  const resolveTargetCamera = useCallback(
    (preferredId?: number | null): Camera | null => {
      if (!cameras.length) {
        return null;
      }
      if (preferredId) {
        const match = cameras.find(camera => camera.id === preferredId);
        if (match) {
          return match;
        }
      }
      const fallbackId = defaultCameraId ?? cameras[0]?.id;
      return cameras.find(camera => camera.id === fallbackId) ?? cameras[0] ?? null;
    },
    [cameras, defaultCameraId],
  );

  const handleDesignerCameraChange = useCallback(
    (cameraId: number) => {
      const nextCamera = cameras.find(camera => camera.id === cameraId);
      if (!nextCamera) {
        return;
      }
      setDesignerCamera(nextCamera);
      setDesignerLineDraft(prev => (prev ? { ...prev, cameraId: nextCamera.id } : prev));
      setDesignerZoneDraft(prev => (prev ? { ...prev, cameraId: nextCamera.id } : prev));
    },
    [cameras],
  );

  const openLineDesigner = (existing?: ObjectCounterLine) => {
    if (!hasCameras) {
      return;
    }
    const targetCamera = resolveTargetCamera(existing?.cameraId);
    if (!targetCamera) {
      return;
    }
    setDesignerMode('line');
    setDesignerCamera(targetCamera);
    setDesignerLineDraft({
      id: existing?.id,
      cameraId: targetCamera.id,
      name: existing?.name ?? '',
      start: existing?.start ?? { x: 0.1, y: 0.1 },
      end: existing?.end ?? { x: 0.9, y: 0.9 },
      direction: existing?.direction ?? 'bidirectional',
      objectType: existing?.objectType,
      enabled: existing?.enabled ?? true,
    });
    setDesignerZoneDraft(null);
    setDesignerError(null);
    setDesignerOpen(true);
  };

  const openZoneDesigner = (existing?: ObjectCounterZone) => {
    if (!hasCameras) {
      return;
    }
    const targetCamera = resolveTargetCamera(existing?.cameraId);
    if (!targetCamera) {
      return;
    }
    setDesignerMode('zone');
    setDesignerCamera(targetCamera);
    const polygonSource = existing?.polygon?.length ? existing.polygon : DEFAULT_ZONE_POLYGON;
    setDesignerZoneDraft({
      id: existing?.id,
      cameraId: targetCamera.id,
      name: existing?.name ?? '',
      polygon: polygonSource.map(point => ({ x: point.x, y: point.y })),
      zoneType: existing?.zoneType ?? 'intrusion',
      objectType: existing?.objectType,
      dwellThresholdMs: existing?.dwellThresholdMs,
      enabled: existing?.enabled ?? true,
    });
    setDesignerLineDraft(null);
    setDesignerError(null);
    setDesignerOpen(true);
  };

  const closeDesigner = () => {
    if (designerSaving) {
      return;
    }
    setDesignerOpen(false);
    setDesignerCamera(null);
    setDesignerLineDraft(null);
    setDesignerZoneDraft(null);
    setDesignerError(null);
  };

  const handleDesignerDraftChange = (draft: ObjectCounterLineInput | ObjectCounterZoneInput) => {
    if ('start' in draft && 'end' in draft) {
      setDesignerLineDraft(draft);
    } else {
      setDesignerZoneDraft(draft as ObjectCounterZoneInput);
    }
  };

  const handleDesignerSaveLine = async (draft: ObjectCounterLineInput) => {
    setDesignerSaving(true);
    setDesignerError(null);
    try {
      const result = await upsertObjectCounterLine(draft);
      if (!result) {
        throw new Error(t('analytics_object_counter_save_failed'));
      }
      setLines(prev => {
        const idx = prev.findIndex(item => item.id === result.id);
        if (idx >= 0) {
          const next = [...prev];
          next[idx] = result;
          return next;
        }
        return [...prev, result];
      });
      closeDesigner();
    } catch (err) {
      setDesignerError(err instanceof Error ? err.message : String(err));
    } finally {
      setDesignerSaving(false);
    }
  };

  const handleDesignerSaveZone = async (draft: ObjectCounterZoneInput) => {
    setDesignerSaving(true);
    setDesignerError(null);
    try {
      const result = await upsertObjectCounterZone(draft);
      if (!result) {
        throw new Error(t('analytics_object_counter_save_failed'));
      }
      setZones(prev => {
        const idx = prev.findIndex(item => item.id === result.id);
        if (idx >= 0) {
          const next = [...prev];
          next[idx] = result;
          return next;
        }
        return [...prev, result];
      });
      closeDesigner();
    } catch (err) {
      setDesignerError(err instanceof Error ? err.message : String(err));
    } finally {
      setDesignerSaving(false);
    }
  };

  const handleAddLine = () => openLineDesigner();
  const handleEditLine = (line: ObjectCounterLine) => openLineDesigner(line);
  const handleAddZone = () => openZoneDesigner();
  const handleEditZone = (zone: ObjectCounterZone) => openZoneDesigner(zone);

  const handleDeleteLine = async (lineId: number) => {
    if (!window.confirm(t('analytics_object_counter_line_delete_confirm'))) {
      return;
    }
    const success = await deleteObjectCounterLine(lineId);
    if (success) {
      setLines(prev => prev.filter(line => line.id !== lineId));
    } else {
      setError(t('analytics_object_counter_delete_failed'));
    }
  };

  const handleDeleteZone = async (zoneId: number) => {
    if (!window.confirm(t('analytics_object_counter_zone_delete_confirm'))) {
      return;
    }
    const success = await deleteObjectCounterZone(zoneId);
    if (success) {
      setZones(prev => prev.filter(zone => zone.id !== zoneId));
    } else {
      setError(t('analytics_object_counter_delete_failed'));
    }
  };

  const handleCameraChange = (event: SelectChangeEvent<string>) => {
    setCameraFilter(event.target.value);
    setPage(0);
  };

  const handleTypeToggle = (type: string) => {
    setSelectedTypes(prev => {
      if (prev.includes(type)) {
        return prev.filter(item => item !== type);
      }
      return [...prev, type];
    });
    setPage(0);
  };

  const handleTimeRangeChange = (_: React.MouseEvent<HTMLElement>, value: number | null) => {
    if (!value) {
      return;
    }
    setTimeRangeMinutes(value);
    setPage(0);
  };

  const bucketMinutes = useMemo(() => {
    if (timeRangeMinutes <= 120) {
      return 5;
    }
    if (timeRangeMinutes <= 720) {
      return 15;
    }
    return 30;
  }, [timeRangeMinutes]);

  const bucketCount = useMemo(() => {
    return Math.min(Math.ceil(timeRangeMinutes / bucketMinutes), 200);
  }, [timeRangeMinutes, bucketMinutes]);

  useEffect(() => {
    let cancelled = false;
    const cameraIdNumeric = cameraFilter ? Number(cameraFilter) : undefined;
    const objectTypesParam = selectedTypes.length ? selectedTypes : undefined;
    const dateTo = new Date();
    const dateFrom = new Date(dateTo.getTime() - timeRangeMinutes * 60 * 1000);

    const loadData = async () => {
      setLoading(true);
      setError(null);
      try {
        const [
          eventsResponse,
          aggregatesResponse,
          topCameraResponse,
          lineResponse,
          zoneResponse,
        ] = await Promise.all([
          getObjectCounterEvents({
            limit: PAGE_SIZE,
            offset: page * PAGE_SIZE,
            cameraId: cameraIdNumeric,
            objectTypes: objectTypesParam,
            dateFrom: dateFrom.toISOString(),
            dateTo: dateTo.toISOString(),
          }),
          getObjectCounterAggregates({
            bucketMinutes,
            buckets: bucketCount,
            cameraId: cameraIdNumeric,
            objectTypes: objectTypesParam,
          }),
          getObjectCounterTopCameras({
            limit: 5,
            minutes: timeRangeMinutes,
            objectTypes: objectTypesParam,
          }),
          getObjectCounterLines({ cameraId: cameraIdNumeric }),
          getObjectCounterZones({ cameraId: cameraIdNumeric }),
        ]);

        if (!cancelled) {
          setEvents(eventsResponse);
          setHasMore(eventsResponse.length === PAGE_SIZE);
          setAggregates(aggregatesResponse);
          setTopCameras(topCameraResponse);
          setLines(lineResponse);
          setZones(zoneResponse);
        }
      } catch (err) {
        if (!cancelled) {
          const message = err instanceof Error ? err.message : String(err);
          setError(message);
        }
      } finally {
        if (!cancelled) {
          setLoading(false);
        }
      }
    };

    void loadData();

    // Auto-refresh events every 5 seconds
    const intervalId = setInterval(() => {
      void loadData();
    }, 5000);

    return () => {
      cancelled = true;
      clearInterval(intervalId);
    };
  }, [cameraFilter, selectedTypes, timeRangeMinutes, page, bucketMinutes, bucketCount]);

  const totalCount = useMemo(() => aggregates.reduce((sum, entry) => sum + entry.totalCount, 0), [aggregates]);
  return (
    <>
      <Stack spacing={2} height="100%">
        <Alert severity="info">
          {t('analytics_object_counter_placeholder')}
        </Alert>

        {activeProcessing && (
          <Alert severity="success">
            {t('analytics_object_counter_processing')}
          </Alert>
        )}

        <Stack direction={{ xs: 'column', md: 'row' }} spacing={2} alignItems={{ xs: 'flex-start', md: 'center' }}>
        <FormControl
          size="small"
          sx={{ width: { xs: '100%', sm: 220 }, alignSelf: { sm: 'flex-start' } }}
        >
          <InputLabel>{t('snapshots_filter_camera')}</InputLabel>
          <Select
            label={t('snapshots_filter_camera')}
            value={cameraFilter}
            onChange={handleCameraChange}
          >
            <MenuItem value="">{t('snapshots_filter_all')}</MenuItem>
            {cameras.map(camera => (
              <MenuItem key={camera.id} value={String(camera.id)}>
                {camera.name}
              </MenuItem>
            ))}
          </Select>
        </FormControl>

        <Box>
          <Typography variant="caption" color="text.secondary">
            {t('analytics_object_tab_hint')}
          </Typography>
          <ToggleButtonGroup
            size="small"
            color="primary"
            exclusive
            value={timeRangeMinutes}
            onChange={handleTimeRangeChange}
            sx={{ mt: 0.5 }}
          >
            {TIME_RANGE_OPTIONS.map(option => (
              <ToggleButton key={option} value={option}>
                {option >= 60 ? `${Math.round(option / 60)}h` : `${option}m`}
              </ToggleButton>
            ))}
          </ToggleButtonGroup>
        </Box>

        <Box sx={{ flexGrow: 1 }} />

        <Stack direction="row" spacing={1} alignItems="center">
          <Tooltip title={t('snapshots_refresh')}>
            <span>
              <Button
                size="small"
                variant="outlined"
                onClick={() => {
                  setPage(0);
                }}
                disabled={loading}
              >
                {t('snapshots_refresh')}
              </Button>
            </span>
          </Tooltip>
          {loading && <CircularProgress size={18} />}
        </Stack>
      </Stack>

      {/* Tabs removed for compact view */}

      <Stack spacing={2}>
          <Stack direction={{ xs: 'column', lg: 'row' }} spacing={2}>
            <Box sx={{ flex: 2, minWidth: 0 }}>
              {activeCamera && (
                <Paper sx={{ p: 0, overflow: 'hidden', position: 'relative', bgcolor: '#000', height: 480 }}>
                  <VideoStreamPlayer
                    streamName={`cam${activeCamera.id}`}
                    useHdQuality={false}
                    showMonitor={false}
                    enableSnapshot={false}
                    enable2WayAudio={false}
                    controls={false}
                    autoPlay
                    muted
                    objectFit="contain"
                    height="100%"
                    width="100%"
                    onVideoRef={setVideoElement}
                  />
                  <AnalyticsOverlay
                    videoElement={videoElement}
                    lines={activeCameraLines}
                    zones={activeCameraZones}
                    detections={activeCameraDetections}
                  />
                  <Box
                    sx={{
                      position: 'absolute',
                      top: 16,
                      left: 16,
                      bgcolor: 'rgba(0,0,0,0.6)',
                      color: '#fff',
                      px: 1.5,
                      py: 0.5,
                      borderRadius: 1,
                      pointerEvents: 'none',
                    }}
                  >
                    <Typography variant="subtitle2">
                      Live Analytics: {activeCamera.name}
                    </Typography>
                  </Box>
                </Paper>
              )}
            </Box>

            <Paper sx={{ flex: 1, p: 2, maxHeight: 480, overflow: 'auto' }}>
               <Stack spacing={2}>
                  <Box>
                    <Typography variant="h6">{t('analytics_object_counter_regions_title')}</Typography>
                    <Typography variant="body2" color="text.secondary">
                      {t('analytics_object_counter_regions_hint')}
                    </Typography>
                  </Box>
                  
                  {!hasCameras && (
                    <Alert severity="warning">
                      {t('analytics_object_counter_regions_no_cameras')}
                    </Alert>
                  )}

                  <Box>
                      <Stack direction="row" justifyContent="space-between" alignItems="center" mb={1}>
                        <Typography variant="subtitle1">{t('analytics_object_counter_lines_title')}</Typography>
                        <Button variant="contained" size="small" onClick={handleAddLine} disabled={!hasCameras}>
                          {t('analytics_object_counter_line_add')}
                        </Button>
                      </Stack>
                      {lines.length === 0 ? (
                        <Typography variant="body2" color="text.secondary">
                          {t('analytics_object_counter_lines_empty')}
                        </Typography>
                      ) : (
                        <List dense>
                          {lines.map(line => (
                            <ListItem
                              key={line.id}
                              divider
                              secondaryAction={
                                <Stack direction="row" spacing={1}>
                                  <Button size="small" variant="outlined" onClick={() => handleEditLine(line)}>
                                    {t('analytics_object_counter_edit')}
                                  </Button>
                                  <Button
                                    size="small"
                                    variant="outlined"
                                    color="error"
                                    onClick={() => handleDeleteLine(line.id)}
                                  >
                                    {t('analytics_object_counter_delete')}
                                  </Button>
                                </Stack>
                              }
                            >
                              <ListItemText
                                primary={
                                  <Stack direction="row" spacing={1} alignItems="center">
                                    <Typography variant="subtitle2">{line.name || t('analytics_object_counter_line_unnamed')}</Typography>
                                    <Chip label={resolveDirectionLabel(line.direction)} size="small" variant="outlined" />
                                    <Chip
                                      label={line.enabled ? t('analytics_object_counter_enabled') : t('analytics_object_counter_disabled')}
                                      color={line.enabled ? 'success' : 'default'}
                                      size="small"
                                    />
                                  </Stack>
                                }
                                secondary={
                                  <Stack spacing={0.5}>
                                    <Typography variant="body2" color="text.secondary">
                                      {t('analytics_object_counter_line_summary', {
                                        startX: line.start.x.toFixed(2),
                                        startY: line.start.y.toFixed(2),
                                        endX: line.end.x.toFixed(2),
                                        endY: line.end.y.toFixed(2),
                                      })}
                                    </Typography>
                                    <Typography variant="body2" color="text.secondary">
                                      {line.objectType ? resolveObjectLabel(line.objectType) : t('analytics_object_counter_object_type_any')}
                                    </Typography>
                                    <Typography variant="body2" color="text.secondary">
                                      {resolveCameraLabel(line.cameraId)}
                                    </Typography>
                                  </Stack>
                                }
                              />
                            </ListItem>
                          ))}
                        </List>
                      )}
                  </Box>
                  <Divider />
                  <Box>
                      <Stack direction="row" justifyContent="space-between" alignItems="center" mb={1}>
                        <Typography variant="subtitle1">{t('analytics_object_counter_zones_title')}</Typography>
                        <Button variant="contained" size="small" onClick={handleAddZone} disabled={!hasCameras}>
                          {t('analytics_object_counter_zone_add')}
                        </Button>
                      </Stack>
                      {zones.length === 0 ? (
                        <Typography variant="body2" color="text.secondary">
                          {t('analytics_object_counter_zones_empty')}
                        </Typography>
                      ) : (
                        <List dense>
                          {zones.map(zone => (
                            <ListItem
                              key={zone.id}
                              divider
                              secondaryAction={
                                <Stack direction="row" spacing={1}>
                                  <Button size="small" variant="outlined" onClick={() => handleEditZone(zone)}>
                                    {t('analytics_object_counter_edit')}
                                  </Button>
                                  <Button
                                    size="small"
                                    variant="outlined"
                                    color="error"
                                    onClick={() => handleDeleteZone(zone.id)}
                                  >
                                    {t('analytics_object_counter_delete')}
                                  </Button>
                                </Stack>
                              }
                            >
                              <ListItemText
                                primary={
                                  <Stack direction="row" spacing={1} alignItems="center">
                                    <Typography variant="subtitle2">{zone.name || t('analytics_object_counter_zone_unnamed')}</Typography>
                                    <Chip label={resolveZoneTypeLabel(zone.zoneType)} size="small" variant="outlined" />
                                    <Chip
                                      label={zone.enabled ? t('analytics_object_counter_enabled') : t('analytics_object_counter_disabled')}
                                      color={zone.enabled ? 'success' : 'default'}
                                      size="small"
                                    />
                                  </Stack>
                                }
                                secondary={
                                  <Stack spacing={0.5}>
                                    <Typography variant="body2" color="text.secondary">
                                      {t('analytics_object_counter_zone_summary', { points: zone.polygon.length })}
                                    </Typography>
                                    {zone.dwellThresholdMs && (
                                      <Typography variant="body2" color="text.secondary">
                                        {t('analytics_object_counter_zone_dwell_hint', { value: zone.dwellThresholdMs })}
                                      </Typography>
                                    )}
                                    <Typography variant="body2" color="text.secondary">
                                      {zone.objectType ? resolveObjectLabel(zone.objectType) : t('analytics_object_counter_object_type_any')}
                                    </Typography>
                                    <Typography variant="body2" color="text.secondary">
                                      {resolveCameraLabel(zone.cameraId)}
                                    </Typography>
                                  </Stack>
                                }
                              />
                            </ListItem>
                          ))}
                        </List>
                      )}
                  </Box>
               </Stack>
            </Paper>
          </Stack>

          {availableTypes.length > 0 && (
        <Stack direction="row" spacing={1} flexWrap="wrap">
          {availableTypes.map(type => (
            <Chip
              key={type}
              label={type}
              size="small"
              variant={selectedTypes.includes(type) ? 'filled' : 'outlined'}
              color={selectedTypes.includes(type) ? 'primary' : 'default'}
              onClick={() => handleTypeToggle(type)}
            />
          ))}
        </Stack>
      )}

      {error && (
        <Alert severity="error" onClose={() => setError(null)}>
          {error}
        </Alert>
      )}

      <Stack direction={{ xs: 'column', lg: 'row' }} spacing={2}>
        <Paper sx={{ p: 2, flex: 2, minHeight: 260 }}>
          <Stack direction="row" justifyContent="space-between" alignItems="center" mb={2}>
            <Box>
              <Typography variant="h6">{t('analytics_object_counter_recent_events')}</Typography>
              <Typography variant="body2" color="text.secondary">
                {t('analytics_object_counter_event_count', { count: totalCount })}
              </Typography>
            </Box>
          </Stack>
          {aggregates.length === 0 ? (
            <Box sx={{ height: 220, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
              <Typography variant="body2" color="text.secondary">
                {loading ? t('plate_loading') : t('analytics_object_counter_empty')}
              </Typography>
            </Box>
          ) : (
            <Box sx={{ height: 220 }}>
              <ResponsiveContainer width="100%" height="100%">
                <AreaChart data={aggregates} margin={{ left: 0, right: 0, top: 10, bottom: 0 }}>
                  <defs>
                    <linearGradient id="objectCounterGradient" x1="0" y1="0" x2="0" y2="1">
                      <stop offset="0%" stopColor="#42a5f5" stopOpacity={0.4} />
                      <stop offset="80%" stopColor="#42a5f5" stopOpacity={0.05} />
                    </linearGradient>
                  </defs>
                  <CartesianGrid strokeDasharray="3 3" strokeOpacity={0.2} />
                  <XAxis dataKey="bucketStart" tickFormatter={value => new Date(value).toLocaleTimeString()} minTickGap={32} />
                  <YAxis allowDecimals={false} width={48} />
                  <ChartTooltip formatter={(value: number) => [`${value}`, t('analytics_object_counter_people', { count: value })]} labelFormatter={value => new Date(value).toLocaleString()} />
                  <Area type="monotone" dataKey="totalCount" stroke="#42a5f5" fill="url(#objectCounterGradient)" strokeWidth={2} />
                </AreaChart>
              </ResponsiveContainer>
            </Box>
          )}
        </Paper>

        <Paper sx={{ p: 2, flex: 1 }}>
          <Typography variant="h6" gutterBottom>
            {t('analytics_object_counter_recent_events')}
          </Typography>
          {topCameras.length === 0 ? (
            <Typography variant="body2" color="text.secondary">
              {t('analytics_object_counter_empty')}
            </Typography>
          ) : (
            <List>
              {topCameras.map(item => (
                <ListItem key={`${item.cameraId ?? 'unknown'}-${item.totalCount}`} disableGutters>
                  <ListItemText
                    primary={resolveCameraLabel(item.cameraId)}
                    secondary={t('analytics_object_counter_people', { count: item.totalCount })}
                  />
                </ListItem>
              ))}
            </List>
          )}
        </Paper>
      </Stack>
      <Paper sx={{ p: 2, flex: 1, minHeight: 360, display: 'flex', flexDirection: 'column', mt: 2 }}>
        <Stack direction="row" justifyContent="space-between" alignItems="center" mb={2}>
          <Box>
            <Typography variant="h6">{t('analytics_object_counter_recent_hint')}</Typography>
            <Typography variant="body2" color="text.secondary">
              {t('analytics_object_counter_processed_at', { time: new Date().toLocaleString() })}
            </Typography>
          </Box>
          <Stack direction="row" spacing={1}>
            <Button
              variant="outlined"
              size="small"
              startIcon={<FolderOpenIcon />}
              onClick={async () => {
                try {
                  const snapshotsDir = modules.find(m => m.id === 'object-counter')?.config?.snapshotsDir;
                  if (snapshotsDir) {
                    await invoke('local_reveal_path', { path: snapshotsDir });
                  } else {
                    // Fallback to default if not set
                    await invoke('local_reveal_path', { path: 'modules/object-counter' });
                  }
                } catch (e) {
                  console.error('Failed to open folder:', e);
                }
              }}
            >
              {t('open_folder')}
            </Button>
            <Chip label={t('analytics_object_counter_event_count', { count: events.length })} size="small" />
          </Stack>
        </Stack>

        {events.length === 0 ? (
          <Box sx={{ flex: 1, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
            <Typography variant="body2" color="text.secondary">
              {loading ? t('plate_loading') : t('analytics_object_counter_empty')}
            </Typography>
          </Box>
        ) : (
          <List sx={{ flex: 1, overflow: 'auto' }}>
            {events.map((event, index) => (
              <React.Fragment key={event.id}>
                {index > 0 && <Divider component="li" />}
                <ListItem alignItems="flex-start">
                  <Stack direction="row" spacing={2} width="100%">
                    <Box sx={{ width: 120, height: 90, flexShrink: 0, bgcolor: 'action.hover', borderRadius: 1, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
                      <ImageWithFallback
                        src={event.snapshotPath ? imageUrls.get(event.snapshotPath) || '' : ''}
                        alt={event.objectType}
                        fallback={<Typography variant="caption" color="text.secondary">No Image</Typography>}
                      />
                    </Box>
                    <ListItemText
                      primary={
                        <Stack direction="row" spacing={1} alignItems="center">
                          <Typography variant="subtitle1">
                            {resolveCameraLabel(event.cameraId)}
                          </Typography>
                          <Chip
                            label={t('analytics_object_counter_people', { count: event.count })}
                            color="primary"
                            size="small"
                          />
                          <Chip label={resolveObjectLabel(event.objectType)} size="small" variant="outlined" />
                        </Stack>
                      }
                      secondary={
                        <Stack spacing={0.5} mt={1}>
                          <Typography variant="body2" color="text.secondary">
                            {new Date(event.processedAt).toLocaleString()}
                          </Typography>
                          <Typography variant="body2">
                            {t('analytics_object_counter_people', { count: event.count })} •
                            {' '}
                            {Math.round(event.confidenceAvg * 100)}%
                            {event.zone && ` • ${event.zone}`}
                            {event.description && ` • ${event.description}`}
                          </Typography>
                          {event.dwellAvgMs && (
                            <Typography variant="body2" color="text.secondary">
                              dwell {Math.round(event.dwellAvgMs)} ms
                            </Typography>
                          )}
                        </Stack>
                      }
                    />
                  </Stack>
                </ListItem>
              </React.Fragment>
            ))}
          </List>
        )}

        <Stack direction="row" justifyContent="space-between" alignItems="center" mt={2}>
          <Button
            size="small"
            variant="outlined"
            onClick={() => setPage(prev => Math.max(0, prev - 1))}
            disabled={page === 0 || loading}
          >
            {t('plate_previous')}
          </Button>
          <Typography variant="body2">
            {t('snapshots_page_indicator', { current: page + 1, total: hasMore ? `${page + 2}+` : page + 1 })}
          </Typography>
          <Button
            size="small"
            variant="outlined"
            onClick={() => setPage(prev => prev + 1)}
            disabled={!hasMore || loading}
          >
            {t('plate_next')}
          </Button>
        </Stack>
      </Paper>
      </Stack>
      </Stack>

      <RegionDesignerDialog
        open={designerOpen}
        mode={designerMode}
        camera={designerCamera}
        cameras={cameras}
        lineDraft={designerLineDraft}
        zoneDraft={designerZoneDraft}
        isSaving={designerSaving}
        errorMessage={designerError}
        onClose={closeDesigner}
        onSaveLine={handleDesignerSaveLine}
        onSaveZone={handleDesignerSaveZone}
        onDraftChange={handleDesignerDraftChange}
        onCameraChange={handleDesignerCameraChange}
      />
    </>
  );
};

export default ObjectCounterPanel;
