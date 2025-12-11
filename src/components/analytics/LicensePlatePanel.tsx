import React, { useState, useEffect, useCallback, useMemo } from 'react';
import {
  Alert,
  Box,
  Button,
  Chip,
  Divider,
  FormControl,
  IconButton,
  InputLabel,
  MenuItem,
  Paper,
  Select,
  Stack,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  TextField,
  Typography,
  Tab,
  Tabs,
} from '@mui/material';
import type { SelectChangeEvent } from '@mui/material';
import RefreshIcon from '@mui/icons-material/Refresh';
import DeleteIcon from '@mui/icons-material/DeleteOutline';
import NoteAltIcon from '@mui/icons-material/NoteAlt';
import CloseIcon from '@mui/icons-material/Close';
import { invoke } from '@tauri-apps/api/core';
import { useLocalization } from '../../hooks/useLocalization';
import { useAppState } from '../../hooks/useAppState';
import { useAnalytics } from '../../hooks/useAnalytics';
import VideoStreamPlayer from '../VideoStreamPlayer';
import AnalyticsOverlay from './AnalyticsOverlay';

interface PlateRecord {
  id: number;
  camera_id: number;
  plate_number: string;
  confidence: number;
  timestamp: string;
  full_image_path: string;
  plate_crop_path: string;
  vehicle_type?: string;
  direction?: string;
  notes?: string;
}

interface PlateStatistics {
  total_records: number;
  today_count: number;
  unique_plates: number;
}

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
        sx={{ width: '100%', borderRadius: 1 }} 
        onError={() => setError(true)} 
      />
  );
};

const LicensePlatePanel: React.FC = () => {
  const { t } = useLocalization();
  const { cameras } = useAppState();
  const { modules, processFrame, detections } = useAnalytics();
  const [records, setRecords] = useState<PlateRecord[]>([]);
  const [statistics, setStatistics] = useState<PlateStatistics | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [selectedRecord, setSelectedRecord] = useState<PlateRecord | null>(null);
  const [plateFilter, setPlateFilter] = useState('');
  const [cameraFilter, setCameraFilter] = useState('');
  const [dateFrom, setDateFrom] = useState('');
  const [dateTo, setDateTo] = useState('');
  const [page, setPage] = useState(0);
  const [limit] = useState(50);
  const [imageUrls, setImageUrls] = useState<Map<string, string>>(new Map());
  const [activeTab, setActiveTab] = useState(0);
  const [videoElement, setVideoElement] = useState<HTMLVideoElement | null>(null);

  const cameraNameMap = useMemo(() => {
    const map = new Map<number, string>();
    cameras.forEach(camera => {
      map.set(camera.id, camera.name);
    });
    return map;
  }, [cameras]);

  const activeCamera = useMemo(() => {
    if (!cameraFilter) return cameras[0] ?? null;
    return cameras.find(c => String(c.id) === cameraFilter) ?? null;
  }, [cameraFilter, cameras]);

  const activeCameraDetections = useMemo(() => {
    if (!activeCamera) return [];
    const latestEvent = detections.find(d => d.cameraId === String(activeCamera.id));
    return latestEvent ? latestEvent.detections : [];
  }, [activeCamera, detections]);

  const activeProcessing = useMemo(() => {
    const mod = modules.find(m => m.id === 'license-plate-detector');
    return Boolean(mod?.enabled && (mod?.state === 'ready' || mod?.state === 'running'));
  }, [modules]);

  // Frame processing loop for live analytics
  useEffect(() => {
    if (!videoElement || !activeCamera || !activeProcessing || activeTab !== 0) {
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
           
           // Limit resolution for performance (e.g. max 1280px width for HD analysis)
           const MAX_WIDTH = 1280;
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
                 moduleId: 'license-plate-detector',
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
        console.warn('[LicensePlate] Frame processing error:', e);
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
  }, [videoElement, activeCamera, activeProcessing, processFrame, activeTab]);

  const resolveCameraName = (cameraId: number) => {
    return cameraNameMap.get(cameraId) ?? cameraId;
  };

  const loadStatistics = useCallback(async () => {
    try {
      const stats: PlateStatistics = await invoke('get_plate_statistics');
      setStatistics(stats);
    } catch (err) {
      console.error('Failed to load statistics:', err);
    }
  }, []);

  const loadRecords = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const offset = page * limit;
      const cameraId = cameraFilter ? parseInt(cameraFilter, 10) : null;
      const data: PlateRecord[] = await invoke('get_plate_records', {
        limit,
        offset,
        plateFilter: plateFilter || null,
        cameraFilter: cameraId,
        dateFrom: dateFrom || null,
        dateTo: dateTo || null,
      });
      setRecords(data);
      if (data.length === 0) {
        setSelectedRecord(null);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    } finally {
      setLoading(false);
    }
  }, [page, limit, plateFilter, cameraFilter, dateFrom, dateTo]);

  useEffect(() => {
    void loadStatistics();
    void loadRecords();
  }, [loadStatistics, loadRecords]);

  const handleSearch = async () => {
    if (!plateFilter.trim()) {
      await loadRecords();
      return;
    }
    setLoading(true);
    setError(null);
    try {
      const data: PlateRecord[] = await invoke('search_plate_history', {
        plateNumber: plateFilter.trim(),
      });
      setRecords(data);
      setPage(0);
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    } finally {
      setLoading(false);
    }
  };

  const handleUpdateNotes = async (record: PlateRecord) => {
    try {
      await invoke('update_plate_notes', { id: record.id, notes: record.notes || '' });
      await loadRecords();
      setSelectedRecord(prev => (prev && prev.id === record.id ? { ...prev, notes: record.notes } : prev));
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    }
  };

  const handleDelete = async (id: number) => {
    if (!window.confirm(t('plate_delete_confirm'))) {
      return;
    }
    try {
      await invoke('delete_plate_record', { id });
      await loadRecords();
      await loadStatistics();
      if (selectedRecord?.id === id) {
        setSelectedRecord(null);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    }
  };

  const handleClearFilters = () => {
    setPlateFilter('');
    setCameraFilter('');
    setDateFrom('');
    setDateTo('');
    setPage(0);
  };

  const handleCameraFilterChange = (event: SelectChangeEvent<string>) => {
    setCameraFilter(event.target.value);
  };

  const formatTimestamp = (timestamp: string) => {
    const value = new Date(timestamp);
    return Number.isNaN(value.getTime()) ? timestamp : value.toLocaleString();
  };

  const loadImage = useCallback(
    async (path: string) => {
      if (!path || imageUrls.has(path)) {
        return;
      }
      try {
        const bytes = await invoke<number[]>('read_plate_image', { path });
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
    records.forEach(record => {
      void loadImage(record.plate_crop_path);
    });
  }, [records, loadImage]);

  useEffect(() => {
    if (selectedRecord) {
      void loadImage(selectedRecord.full_image_path);
      void loadImage(selectedRecord.plate_crop_path);
    }
  }, [selectedRecord, loadImage]);

  useEffect(() => {
    return () => {
      imageUrls.forEach(url => {
        if (url) {
          URL.revokeObjectURL(url);
        }
      });
    };
  }, [imageUrls]);

  const selectedImageSources = useMemo(() => {
    if (!selectedRecord) {
      return { full: '', crop: '' };
    }
    return {
      full: imageUrls.get(selectedRecord.full_image_path) ?? '',
      crop: imageUrls.get(selectedRecord.plate_crop_path) ?? '',
    };
  }, [selectedRecord, imageUrls]);

  return (
    <Stack spacing={1} sx={{ height: '100%', overflow: 'hidden' }}>
      <Paper sx={{ p: 1, display: 'flex', alignItems: 'center', justifyContent: 'space-between', flexShrink: 0 }}>
        <Stack direction="row" spacing={2} alignItems="center">
          <Typography variant="subtitle1" fontWeight="bold">{t('analytics_plate_title')}</Typography>
          {statistics && (
            <Stack direction="row" spacing={1}>
              <Chip label={`${t('plate_stat_total')}: ${statistics.total_records}`} size="small" variant="outlined" />
              <Chip label={`${t('plate_stat_today')}: ${statistics.today_count}`} size="small" color="primary" />
            </Stack>
          )}
        </Stack>
        <Stack direction="row" spacing={1} alignItems="center">
          {activeTab === 0 && (
             <FormControl size="small" sx={{ minWidth: 200 }}>
                <Select
                  value={cameraFilter}
                  displayEmpty
                  onChange={(e: SelectChangeEvent) => setCameraFilter(e.target.value)}
                  sx={{ height: 32 }}
                >
                  <MenuItem value="">
                    <em>{t('all_cameras')}</em>
                  </MenuItem>
                  {cameras.map(camera => (
                    <MenuItem key={camera.id} value={String(camera.id)}>
                      {camera.name}
                    </MenuItem>
                  ))}
                </Select>
             </FormControl>
           )}
          <IconButton size="small" onClick={() => { loadRecords(); loadStatistics(); }}>
            <RefreshIcon fontSize="small" />
          </IconButton>
        </Stack>
      </Paper>

      <Tabs 
        value={activeTab} 
        onChange={(_, v) => setActiveTab(v)} 
        sx={{ minHeight: 36, borderBottom: 1, borderColor: 'divider', flexShrink: 0, '& .MuiTab-root': { minHeight: 36, py: 0 } }}
      >
        <Tab label={t('analytics_live_view')} />
        <Tab label={t('analytics_history')} />
      </Tabs>

      {activeTab === 0 && (
        <Stack direction="row" spacing={1} sx={{ flex: 1, minHeight: 0, overflow: 'hidden' }}>
          <Box sx={{ flex: 1, display: 'flex', flexDirection: 'column', bgcolor: '#000', position: 'relative', borderRadius: 1, overflow: 'hidden' }}>
            {activeCamera ? (
                <>
                <VideoStreamPlayer
                  streamName={`cam${activeCamera.id}`}
                  useHdQuality={true}
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
                  lines={[]}
                  zones={[]}
                  detections={activeCameraDetections}
                />
                {!activeProcessing && (
                  <Box
                    sx={{
                      position: 'absolute',
                      inset: 0,
                      display: 'flex',
                      alignItems: 'center',
                      justifyContent: 'center',
                      bgcolor: 'rgba(0,0,0,0.7)',
                      zIndex: 10,
                    }}
                  >
                    <Typography variant="h6" color="error">
                      {t('analytics_module_not_active')}
                    </Typography>
                  </Box>
                )}
                </>
            ) : (
                <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'center', height: '100%', color: 'white' }}>
                    <Typography>{t('select_camera')}</Typography>
                </Box>
            )}
          </Box>
          
          <Paper sx={{ width: 250, display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>
            <Box sx={{ p: 1, borderBottom: 1, borderColor: 'divider', flexShrink: 0 }}>
              <Typography variant="caption" fontWeight="bold">{t('analytics_recent_detections')}</Typography>
            </Box>
            <Box sx={{ flex: 1, overflow: 'auto', p: 0 }}>
              <Table size="small" padding="none">
                <TableBody>
                  {activeCameraDetections.slice(0, 20).map((det, idx) => (
                    <TableRow key={det.id || idx}>
                      <TableCell sx={{ px: 1, py: 0.5 }}>
                        <Typography variant="body2" fontWeight="bold">{det.label}</Typography>
                        <Typography variant="caption" color="text.secondary">
                          {(det.confidence * 100).toFixed(0)}%
                        </Typography>
                      </TableCell>
                      <TableCell align="right" sx={{ px: 1, py: 0.5 }}>
                        <Typography variant="caption" color="text.secondary">
                          {new Date().toLocaleTimeString()}
                        </Typography>
                      </TableCell>
                    </TableRow>
                  ))}
                  {activeCameraDetections.length === 0 && (
                    <TableRow>
                      <TableCell colSpan={2} align="center" sx={{ py: 4, color: 'text.secondary' }}>
                        {t('no_data')}
                      </TableCell>
                    </TableRow>
                  )}
                </TableBody>
              </Table>
            </Box>
          </Paper>
        </Stack>
      )}

      {activeTab === 1 && (
      <Stack direction="row" spacing={1} sx={{ flex: 1, minHeight: 0, overflow: 'hidden' }}>
        <Paper sx={{ width: 250, p: 1, display: 'flex', flexDirection: 'column', gap: 1, overflow: 'auto' }}>
          <Typography variant="caption" fontWeight="bold">{t('filters')}</Typography>
          <TextField
            label={t('plate_number')}
            size="small"
            fullWidth
            value={plateFilter}
            onChange={e => setPlateFilter(e.target.value)}
          />
          <FormControl fullWidth size="small">
            <InputLabel>{t('camera')}</InputLabel>
            <Select
              value={cameraFilter}
              label={t('camera')}
              onChange={(e: SelectChangeEvent) => setCameraFilter(e.target.value)}
            >
              <MenuItem value="">
                <em>{t('all_cameras')}</em>
              </MenuItem>
              {cameras.map(camera => (
                <MenuItem key={camera.id} value={String(camera.id)}>
                  {camera.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
          <TextField
            label={t('date_from')}
            type="date"
            size="small"
            fullWidth
            InputLabelProps={{ shrink: true }}
            value={dateFrom}
            onChange={e => setDateFrom(e.target.value)}
          />
          <TextField
            label={t('date_to')}
            type="date"
            size="small"
            fullWidth
            InputLabelProps={{ shrink: true }}
            value={dateTo}
            onChange={e => setDateTo(e.target.value)}
          />
          <Button variant="contained" size="small" startIcon={<RefreshIcon />} onClick={loadRecords} disabled={loading}>
            {t('apply')}
          </Button>
        </Paper>

        <Paper sx={{ flex: 1, display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>
          <TableContainer sx={{ flex: 1, overflow: 'auto' }}>
            <Table stickyHeader size="small" padding="checkbox">
              <TableHead>
                <TableRow>
                  <TableCell>{t('plate_number')}</TableCell>
                  <TableCell>{t('camera')}</TableCell>
                  <TableCell>{t('timestamp')}</TableCell>
                  <TableCell>{t('plate_confidence')}</TableCell>
                  <TableCell align="right">{t('actions')}</TableCell>
                </TableRow>
              </TableHead>
              <TableBody>
                {loading ? (
                  <TableRow>
                    <TableCell colSpan={5} align="center" sx={{ py: 3 }}>
                      {t('loading')}...
                    </TableCell>
                  </TableRow>
                ) : records.length === 0 ? (
                  <TableRow>
                    <TableCell colSpan={5} align="center" sx={{ py: 3 }}>
                      {t('no_records')}
                    </TableCell>
                  </TableRow>
                ) : (
                  records.map(record => (
                    <TableRow
                      key={record.id}
                      hover
                      selected={selectedRecord?.id === record.id}
                      onClick={() => setSelectedRecord(record)}
                      sx={{ cursor: 'pointer' }}
                    >
                      <TableCell sx={{ py: 0.5 }}>
                        <Typography variant="body2" fontWeight="bold">
                          {record.plate_number}
                        </Typography>
                      </TableCell>
                      <TableCell sx={{ py: 0.5 }}>{resolveCameraName(record.camera_id)}</TableCell>
                      <TableCell sx={{ py: 0.5 }}>{formatTimestamp(record.timestamp)}</TableCell>
                      <TableCell sx={{ py: 0.5 }}>
                        <Chip
                          label={`${(record.confidence * 100).toFixed(0)}%`}
                          size="small"
                          color={record.confidence > 0.8 ? 'success' : 'warning'}
                          variant="outlined"
                          sx={{ height: 20, fontSize: '0.7rem' }}
                        />
                      </TableCell>
                      <TableCell align="right" sx={{ py: 0.5 }}>
                        <IconButton size="small" onClick={e => { e.stopPropagation(); handleDelete(record.id); }}>
                          <DeleteIcon fontSize="small" />
                        </IconButton>
                      </TableCell>
                    </TableRow>
                  ))
                )}
              </TableBody>
            </Table>
          </TableContainer>
          <Stack direction="row" spacing={1} justifyContent="flex-end" sx={{ p: 0.5, borderTop: 1, borderColor: 'divider' }}>
            <Button size="small" variant="outlined" disabled={page === 0} onClick={() => setPage(prev => Math.max(0, prev - 1))}>
              {t('plate_previous')}
            </Button>
            <Button
              size="small"
              variant="contained"
              disabled={records.length < limit}
              onClick={() => setPage(prev => prev + 1)}
            >
              {t('plate_next')}
            </Button>
          </Stack>
        </Paper>

        <Paper sx={{ width: 280, p: 1, display: 'flex', flexDirection: 'column', overflow: 'auto' }}>
          {!selectedRecord ? (
            <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'center', height: '100%', color: 'text.secondary' }}>
              <Typography variant="body2">{t('select_record_details')}</Typography>
            </Box>
          ) : (
            <Stack spacing={1}>
              <Stack direction="row" justifyContent="space-between" alignItems="center">
                <Typography variant="subtitle2" fontWeight="bold">
                  {t('details')}
                </Typography>
                <IconButton size="small" onClick={() => setSelectedRecord(null)}>
                  <CloseIcon fontSize="small" />
                </IconButton>
              </Stack>
              <Divider />
              
              <Stack direction="column" spacing={1}>
                <Box sx={{ flex: 1 }}>
                  <Typography variant="caption" gutterBottom>
                    {t('plate_full_vehicle')}
                  </Typography>
                  <ImageWithFallback
                    src={selectedImageSources.full}
                    alt="Full vehicle"
                    fallback={
                      <Box sx={{ height: 120, bgcolor: 'background.default', borderRadius: 1, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
                        <Typography variant="caption" color="text.secondary">
                          {t('snapshots_image_missing')}
                        </Typography>
                      </Box>
                    }
                  />
                </Box>
                <Box sx={{ width: '100%' }}>
                  <Typography variant="caption" gutterBottom>
                    {t('plate_crop')}
                  </Typography>
                  <ImageWithFallback
                    src={selectedImageSources.crop}
                    alt="Plate crop"
                    fallback={
                      <Box sx={{ height: 80, bgcolor: 'background.default', borderRadius: 1, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
                        <Typography variant="caption" color="text.secondary">
                          {t('snapshots_image_missing')}
                        </Typography>
                      </Box>
                    }
                  />
                </Box>
              </Stack>

              <Stack spacing={0.5}>
                <Typography variant="caption">{t('plate_number')}</Typography>
                <Typography variant="h6">{selectedRecord.plate_number}</Typography>
                <Typography variant="caption" color="text.secondary">
                  {formatTimestamp(selectedRecord.timestamp)}
                </Typography>
                <Typography variant="caption">
                  {t('plate_confidence')}: {(selectedRecord.confidence * 100).toFixed(2)}%
                </Typography>
              </Stack>

              <Stack spacing={1}>
                <Typography variant="caption">{t('plate_notes')}</Typography>
                <TextField
                  multiline
                  minRows={2}
                  size="small"
                  value={selectedRecord.notes ?? ''}
                  onChange={event => setSelectedRecord({ ...selectedRecord, notes: event.target.value })}
                />
                <Button
                  variant="contained"
                  size="small"
                  startIcon={<NoteAltIcon />}
                  onClick={() => selectedRecord && void handleUpdateNotes(selectedRecord)}
                >
                  {t('plate_save_notes')}
                </Button>
              </Stack>
            </Stack>
          )}
        </Paper>
      </Stack>
      )}
    </Stack>
  );
};

export default LicensePlatePanel;
