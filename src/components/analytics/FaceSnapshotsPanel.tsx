import React, { useCallback, useEffect, useMemo, useState } from 'react';
import {
  Alert,
  Box,
  Button,
  Chip,
  CircularProgress,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Divider,
  FormControl,
  IconButton,
  InputLabel,
  MenuItem,
  Paper,
  Select,
  Stack,
  Typography,
} from '@mui/material';
import type { SelectChangeEvent } from '@mui/material';
import RefreshIcon from '@mui/icons-material/Refresh';
import FolderOpenIcon from '@mui/icons-material/FolderOpen';
import ImageIcon from '@mui/icons-material/Image';
import ChevronLeftIcon from '@mui/icons-material/ChevronLeft';
import ChevronRightIcon from '@mui/icons-material/ChevronRight';
import CloseIcon from '@mui/icons-material/Close';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
import { convertFileSrc, invoke } from '@tauri-apps/api/core';
import { useNavigate } from 'react-router-dom';
import { useLocalization } from '../../hooks/useLocalization';
import { useAnalytics } from '../../hooks/useAnalytics';
import { useAppState } from '../../hooks/useAppState';
import { listAnalyticsSnapshots } from '../../services/analytics';
import type {
  AnalyticsSnapshotListItem,
  AnalyticsSnapshotListResponse,
} from '../../services/analytics';
import { isTauriAvailable } from '../../utils/tauri';

const PAGE_SIZE = 32;
const FACE_MODULE_ID = 'face-detector';

const normalizeFilePath = (value: string): string => {
  if (!value) {
    return value;
  }

  if (value.startsWith('\\\\?\\UNC\\')) {
    return `\\\\${value.slice('\\\\?\\UNC\\'.length)}`.replace(/\\/g, '/');
  }
  if (value.startsWith('\\\\?\\')) {
    return value.slice('\\\\?\\'.length).replace(/\\/g, '/');
  }
  if (value.startsWith('//?/UNC/')) {
    return `//${value.slice('//?/UNC/'.length)}`.replace(/\\/g, '/');
  }
  if (value.startsWith('//?/')) {
    return value.slice('//?/'.length).replace(/\\/g, '/');
  }

  return value.replace(/\\/g, '/');
};

type SnapshotMetaState = Pick<AnalyticsSnapshotListResponse, 'total' | 'hasMore'>;

export interface FaceSnapshotsPanelProps {
  variant?: 'standalone' | 'embedded';
  onBackClick?: () => void;
  moduleId?: string;
}

const FaceSnapshotsPanel: React.FC<FaceSnapshotsPanelProps> = ({ variant = 'standalone', onBackClick, moduleId = FACE_MODULE_ID }) => {
  const { t } = useLocalization();
  const { modules } = useAnalytics();
  const { cameras } = useAppState();
  const navigate = useNavigate();

  const isStandalone = variant === 'standalone';

  const [snapshots, setSnapshots] = useState<AnalyticsSnapshotListItem[]>([]);
  const [meta, setMeta] = useState<SnapshotMetaState>({ total: 0, hasMore: false });
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [cameraFilter, setCameraFilter] = useState<string>('');
  const [page, setPage] = useState<number>(0);
  const [selectedSnapshot, setSelectedSnapshot] = useState<AnalyticsSnapshotListItem | null>(null);

  const cameraNameMap = useMemo(() => {
    const map = new Map<string, string>();
    cameras.forEach(camera => {
      map.set(String(camera.id), camera.name);
    });
    return map;
  }, [cameras]);

  const resolveCameraName = useCallback(
    (cameraId?: string) => {
      if (!cameraId) {
        return t('snapshots_unknown_camera');
      }
      return cameraNameMap.get(cameraId) ?? cameraId;
    },
    [cameraNameMap, t],
  );

  const resolveModuleLabel = useCallback(
    (moduleId: string) => {
      const translationKey = `module_${moduleId}_name`;
      const translated = t(translationKey);
      if (!translated.startsWith('[')) {
        return translated;
      }
      const found = modules.find(module => module.id === moduleId);
      return found?.name ?? moduleId;
    },
    [modules, t],
  );

  const handleBack = useCallback(() => {
    if (onBackClick) {
      onBackClick();
      return;
    }

    if (window.history.length > 1) {
      navigate(-1);
      return;
    }
    navigate('/');
  }, [navigate, onBackClick]);

  const loadSnapshots = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const response = await listAnalyticsSnapshots({
        moduleId: moduleId,
        cameraId: cameraFilter || undefined,
        limit: PAGE_SIZE,
        offset: page * PAGE_SIZE,
      });
      setSnapshots(response.items);
      setMeta({ total: response.total, hasMore: response.hasMore });
    } catch (err) {
      const message = err instanceof Error ? err.message : String(err);
      setSnapshots([]);
      setMeta({ total: 0, hasMore: false });
      setError(message);
    } finally {
      setLoading(false);
    }
  }, [cameraFilter, page, moduleId]);

  useEffect(() => {
    void loadSnapshots();
  }, [loadSnapshots]);

  const handleRefresh = () => {
    setPage(0);
    void loadSnapshots();
  };

  const handleReveal = useCallback(async (path: string) => {
    if (!isTauriAvailable()) {
      return;
    }
    try {
      await invoke('local_reveal_path', { path });
    } catch (err) {
      console.warn('Failed to reveal path', err);
    }
  }, []);

  const handleCameraChange = (event: SelectChangeEvent<string>) => {
    setCameraFilter(event.target.value);
    setPage(0);
  };

  const handlePrevPage = () => {
    setPage(prev => Math.max(0, prev - 1));
  };

  const handleNextPage = () => {
    if (meta.hasMore) {
      setPage(prev => prev + 1);
    }
  };

  const formatTimestamp = (value: string) => {
    const date = new Date(value);
    return Number.isNaN(date.getTime()) ? value : date.toLocaleString();
  };

  const formatBytes = (value?: number) => {
    if (!value && value !== 0) {
      return t('unknown');
    }
    if (value < 1024) {
      return `${value} B`;
    }
    if (value < 1024 * 1024) {
      return `${(value / 1024).toFixed(1)} KB`;
    }
    return `${(value / (1024 * 1024)).toFixed(1)} MB`;
  };

  const renderImage = (item: AnalyticsSnapshotListItem, variant: 'grid' | 'dialog') => {
    if (!item.imageAvailable || !isTauriAvailable()) {
      return (
        <Box
          sx={{
            width: '100%',
            height: variant === 'grid' ? 180 : 360,
            bgcolor: '#111',
            color: 'rgba(255,255,255,0.4)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            borderRadius: 1,
          }}
        >
          <Stack spacing={1} alignItems="center">
            <ImageIcon fontSize={variant === 'grid' ? 'large' : 'inherit'} />
            <Typography variant="body2">{t('snapshots_image_missing')}</Typography>
          </Stack>
        </Box>
      );
    }

    const src = convertFileSrc(normalizeFilePath(item.imagePath));
    return (
      <Box
        component="img"
        src={src}
        alt={item.id}
        sx={{
          width: '100%',
          height: variant === 'grid' ? 180 : 'auto',
          maxHeight: variant === 'grid' ? undefined : 480,
          objectFit: 'cover',
          borderRadius: 1,
          backgroundColor: '#000',
        }}
      />
    );
  };

  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, height: '100%' }}>
      {isStandalone && (
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
          <Stack direction="row" alignItems="center" spacing={1}>
            <Button
              variant="text"
              size="small"
              color="inherit"
              startIcon={<ArrowBackIcon />}
              onClick={handleBack}
            >
              {t('snapshots_back_button')}
            </Button>
            <Typography variant="h5">{t('snapshots_title')}</Typography>
            <Chip label={t('snapshots_total_count', { count: meta.total })} size="small" />
            {loading && <CircularProgress size={18} />}
            <Box sx={{ flexGrow: 1 }} />
            <Button
              variant="outlined"
              size="small"
              startIcon={<RefreshIcon />}
              onClick={handleRefresh}
              disabled={loading}
            >
              {t('snapshots_refresh')}
            </Button>
          </Stack>
          <Typography variant="body2" color="text.secondary">
            {t('snapshots_description')}
          </Typography>
        </Box>
      )}

      {!isStandalone && (
        <Stack direction="row" spacing={1} alignItems="center">
          <Chip label={t('snapshots_total_count', { count: meta.total })} size="small" />
          {loading && <CircularProgress size={18} />}
          <Box sx={{ flexGrow: 1 }} />
          <Button
            variant="outlined"
            size="small"
            startIcon={<RefreshIcon />}
            onClick={handleRefresh}
            disabled={loading}
          >
            {t('snapshots_refresh')}
          </Button>
        </Stack>
      )}

      <Stack direction={{ xs: 'column', md: 'row' }} spacing={2} alignItems={{ xs: 'stretch', md: 'center' }}>
        <FormControl size="small" sx={{ minWidth: 220 }}>
          <InputLabel>{t('snapshots_filter_camera')}</InputLabel>
          <Select
            label={t('snapshots_filter_camera')}
            value={cameraFilter}
            onChange={handleCameraChange}
          >
            <MenuItem value="">{t('snapshots_filter_all')}</MenuItem>
            {[...cameraNameMap.entries()].map(([value, label]) => (
              <MenuItem key={value} value={value}>
                {label}
              </MenuItem>
            ))}
          </Select>
        </FormControl>

        <Box sx={{ flexGrow: 1 }} />

        <Stack direction="row" spacing={1} alignItems="center">
          <IconButton onClick={handlePrevPage} disabled={page === 0 || loading}>
            <ChevronLeftIcon />
          </IconButton>
          <Typography variant="body2">
            {t('snapshots_page_indicator', {
              current: page + 1,
              total: meta.total > 0 ? Math.ceil(meta.total / PAGE_SIZE) : 1,
            })}
          </Typography>
          <IconButton onClick={handleNextPage} disabled={!meta.hasMore || loading}>
            <ChevronRightIcon />
          </IconButton>
        </Stack>
      </Stack>

      {error && (
        <Alert severity="error" onClose={() => setError(null)}>
          {t('snapshots_error')} {error}
        </Alert>
      )}

      {!loading && snapshots.length === 0 && !error && (
        <Paper sx={{ p: 4, textAlign: 'center', color: 'text.secondary' }}>
          <Typography variant="subtitle1">{t('snapshots_empty_state')}</Typography>
        </Paper>
      )}

      <Box sx={{ flexGrow: 1, overflow: 'auto' }}>
        <Box
          sx={{
            display: 'grid',
            gap: 2,
            gridTemplateColumns: {
              xs: '1fr',
              sm: 'repeat(2, 1fr)',
              md: 'repeat(3, 1fr)',
              lg: 'repeat(4, 1fr)',
            },
          }}
        >
          {snapshots.map(snapshot => {
            const cameraName = resolveCameraName(snapshot.cameraId);
            const moduleLabel = resolveModuleLabel(snapshot.moduleId);
            const confidence = Math.round(snapshot.confidence * 100);
            return (
              <Paper key={snapshot.id} sx={{ p: 2, display: 'flex', flexDirection: 'column', gap: 1.5 }}>
                <Box sx={{ position: 'relative' }}>
                  {renderImage(snapshot, 'grid')}
                  <Chip
                    label={`${confidence}%`}
                    size="small"
                    color={confidence >= 70 ? 'success' : confidence >= 40 ? 'warning' : 'default'}
                    sx={{ position: 'absolute', top: 8, left: 8 }}
                  />
                  {!snapshot.imageAvailable && (
                    <Chip
                      label={t('snapshots_image_missing')}
                      size="small"
                      color="default"
                      sx={{ position: 'absolute', top: 8, right: 8 }}
                    />
                  )}
                  {snapshot.encrypted && (
                    <Chip
                      label={t('snapshots_encrypted_badge')}
                      size="small"
                      color="info"
                      sx={{ position: 'absolute', bottom: 8, left: 8 }}
                    />
                  )}
                </Box>

                <Box>
                  <Typography variant="subtitle1" noWrap title={cameraName}>
                    {cameraName}
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    {formatTimestamp(snapshot.capturedAt)}
                  </Typography>
                </Box>

                <Box>
                  <Typography variant="body2">{moduleLabel}</Typography>
                  <Typography variant="caption" color="text.secondary">
                    {t('snapshots_resolution_label', {
                      width: snapshot.frameWidth,
                      height: snapshot.frameHeight,
                    })}
                  </Typography>
                </Box>

                <Stack direction="row" spacing={1} flexWrap="wrap">
                  <Button
                    size="small"
                    variant="contained"
                    startIcon={<ImageIcon />}
                    onClick={() => setSelectedSnapshot(snapshot)}
                  >
                    {t('snapshots_view_full')}
                  </Button>
                  <Button
                    size="small"
                    variant="outlined"
                    startIcon={<FolderOpenIcon />}
                    disabled={!isTauriAvailable()}
                    onClick={() => handleReveal(snapshot.folderPath)}
                  >
                    {t('snapshots_open_folder')}
                  </Button>
                </Stack>
              </Paper>
            );
          })}
        </Box>
      </Box>

      <Dialog
        open={Boolean(selectedSnapshot)}
        onClose={() => setSelectedSnapshot(null)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
          {t('snapshots_details_title')}
          <IconButton size="small" onClick={() => setSelectedSnapshot(null)}>
            <CloseIcon />
          </IconButton>
        </DialogTitle>
        <DialogContent dividers>
          {selectedSnapshot && (
            <Stack spacing={2}>
              {renderImage(selectedSnapshot, 'dialog')}
              <Divider />
              <Stack spacing={1}>
                <Typography variant="subtitle1">
                  {resolveCameraName(selectedSnapshot.cameraId)}
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  {formatTimestamp(selectedSnapshot.capturedAt)}
                </Typography>
                <Typography variant="body2">
                  {t('snapshots_module_label', { value: resolveModuleLabel(selectedSnapshot.moduleId) })}
                </Typography>
                <Typography variant="body2">
                  {t('snapshots_confidence_label', { value: Math.round(selectedSnapshot.confidence * 100) })}
                </Typography>
                <Typography variant="body2">
                  {t('snapshots_resolution_label', {
                    width: selectedSnapshot.frameWidth,
                    height: selectedSnapshot.frameHeight,
                  })}
                </Typography>
                <Typography variant="body2">
                  {t('snapshots_path_label', { value: selectedSnapshot.imagePath })}
                </Typography>
                <Typography variant="body2">
                  {t('snapshots_metadata_label', { value: selectedSnapshot.metadataPath })}
                </Typography>
                <Typography variant="body2">
                  {t('snapshots_bytes_label', {
                    image: formatBytes(selectedSnapshot.imageSize),
                    metadata: formatBytes(selectedSnapshot.metadataSize),
                  })}
                </Typography>
              </Stack>
            </Stack>
          )}
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setSelectedSnapshot(null)}>{t('close_button')}</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default FaceSnapshotsPanel;
