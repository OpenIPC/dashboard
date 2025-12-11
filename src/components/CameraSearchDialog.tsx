import React, { useCallback, useEffect, useMemo, useState } from 'react';
import {
  Box,
  Button,
  Chip,
  Checkbox,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Divider,
  FormControl,
  IconButton,
  InputLabel,
  LinearProgress,
  ListItemText,
  MenuItem,
  Select,
  Stack,
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableRow,
  Tooltip,
  Typography,
} from '@mui/material';
import type { SelectChangeEvent } from '@mui/material/Select';
import CloseIcon from '@mui/icons-material/Close';
import { invoke } from '@tauri-apps/api/core';
import { useLocalization } from '../hooks/useLocalization';

export interface DiscoveredCamera {
  name: string;
  ip: string;
  protocol?: string;
  port?: number;
  onvifPort?: number;
  detectedPort?: number;
}

export interface DiscoveryFilters {
  interfaces?: string[];
}

interface CameraSearchDialogProps {
  open: boolean;
  onClose: () => void;
  foundCameras: DiscoveredCamera[];
  onAddSelected?: (cams: DiscoveredCamera[]) => void;
  isDiscovering?: boolean;
  discoveryProgress?: string;
  onStartDiscovery?: (filters?: DiscoveryFilters) => Promise<void> | void;
}

interface NetworkInterfaceInfo {
  name: string;
  displayName?: string;
  ipv4: string;
  netmask: string;
  cidr: string;
  is_loopback: boolean;
}

type InterfaceSelection = 'all' | string;

const CameraSearchDialog: React.FC<CameraSearchDialogProps> = ({
  open,
  onClose,
  foundCameras,
  onAddSelected,
  isDiscovering = false,
  discoveryProgress = '',
  onStartDiscovery,
}) => {
  const { t } = useLocalization();
  const [selectedIps, setSelectedIps] = useState<Set<string>>(new Set());
  const [interfaces, setInterfaces] = useState<NetworkInterfaceInfo[]>([]);
  const [interfacesLoading, setInterfacesLoading] = useState(false);
  const [interfacesError, setInterfacesError] = useState<string | null>(null);
  const [selectedInterfaceId, setSelectedInterfaceId] = useState<InterfaceSelection>('all');
  const [scanError, setScanError] = useState<string | null>(null);
  const selectedInterface = useMemo(
    () => (selectedInterfaceId === 'all' ? null : interfaces.find(iface => iface.name === selectedInterfaceId) ?? null),
    [selectedInterfaceId, interfaces]
  );

  const loadInterfaces = useCallback(async () => {
    setInterfacesLoading(true);
    setInterfacesError(null);
    try {
      const result = await invoke<NetworkInterfaceInfo[]>('list_network_interfaces');
      setInterfaces(result);
      setSelectedInterfaceId(prev => {
        if (result.length === 1) {
          return result[0].name;
        }
        if (prev === 'all') {
          return 'all';
        }
        return result.some(iface => iface.name === prev) ? prev : 'all';
      });
    } catch (error) {
      console.error('Failed to load network interfaces', error);
      setInterfacesError(t('camera_search_interfaces_error'));
    } finally {
      setInterfacesLoading(false);
    }
  }, [t]);

  const resetState = useCallback(() => {
    setSelectedIps(new Set());
    setScanError(null);
  }, []);

  useEffect(() => {
    if (open) {
      resetState();
      void loadInterfaces();
    }
  }, [open, resetState, loadInterfaces]);

  useEffect(() => {
    if (!open) {
      setSelectedInterfaceId('all');
    }
  }, [open]);

  useEffect(() => {
    setSelectedIps(new Set());
  }, [foundCameras]);

  const toggleSelection = (ip: string) => {
    setSelectedIps(prev => {
      const next = new Set(prev);
      if (next.has(ip)) {
        next.delete(ip);
      } else {
        next.add(ip);
      }
      return next;
    });
  };

  const handleSelectAll = () => {
    if (selectedIps.size === foundCameras.length) {
      setSelectedIps(new Set());
    } else {
      setSelectedIps(new Set(foundCameras.map(c => c.ip)));
    }
  };

  const selectedCameras = useMemo(
    () => foundCameras.filter(cam => selectedIps.has(cam.ip)),
    [foundCameras, selectedIps]
  );

  const getInterfaceFilters = useCallback((): string[] | undefined => {
    if (!selectedInterface) {
      return undefined;
    }
    return [selectedInterface.name, selectedInterface.ipv4];
  }, [selectedInterface]);

  const handleInterfaceChange = (event: SelectChangeEvent<string>) => {
    const value = event.target.value as InterfaceSelection;
    setSelectedInterfaceId(value);
  };

  const getInterfaceLabel = (iface?: NetworkInterfaceInfo | null) => {
    if (!iface) {
      return t('camera_search_all_interfaces');
    }
    const label = iface.displayName || iface.name;
    return `${label} • ${iface.ipv4}`;
  };

  const startScan = useCallback(async () => {
    const interfacesFilter = getInterfaceFilters();
    try {
      setScanError(null);
      if (onStartDiscovery) {
        await onStartDiscovery(
          interfacesFilter && interfacesFilter.length > 0
            ? { interfaces: interfacesFilter }
            : undefined
        );
      } else {
        const payload = interfacesFilter && interfacesFilter.length > 0
          ? { request: { interfaces: interfacesFilter } }
          : { request: null };
        await invoke('discover_cameras', payload);
      }
    } catch (error) {
      console.error('Failed to start discovery', error);
      const message =
        error instanceof Error
          ? error.message
          : typeof error === 'string'
            ? error
            : t('camera_search_scan_error');
      setScanError(message);
    }
  }, [getInterfaceFilters, onStartDiscovery, t]);

  const renderCameraTable = () => {
    if (foundCameras.length === 0) {
      return (
        <Box sx={{ textAlign: 'center', py: 4 }}>
          <Typography variant="body2" color="text.secondary">
            {isDiscovering ? t('searching') : t('camera_search_no_results_hint')}
          </Typography>
          {!isDiscovering && (
            <Typography variant="caption" color="text.secondary">
              {t('network_help_text')}
            </Typography>
          )}
        </Box>
      );
    }

    return (
      <Table size="small" sx={{ '& td, & th': { borderColor: 'rgba(255,255,255,0.08)' } }}>
        <TableHead>
          <TableRow>
            <TableCell padding="checkbox">
              <Checkbox
                size="small"
                indeterminate={selectedIps.size > 0 && selectedIps.size < foundCameras.length}
                checked={foundCameras.length > 0 && selectedIps.size === foundCameras.length}
                onChange={handleSelectAll}
              />
            </TableCell>
            <TableCell>{t('camera_search_table_device')}</TableCell>
            <TableCell>{t('camera_search_table_network')}</TableCell>
            <TableCell>{t('camera_search_table_ports')}</TableCell>
            <TableCell>{t('camera_search_table_protocol')}</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
          {foundCameras.map(cam => {
            const isSelected = selectedIps.has(cam.ip);
            return (
              <TableRow
                key={cam.ip}
                hover
                selected={isSelected}
                onClick={() => toggleSelection(cam.ip)}
                sx={{ cursor: 'pointer' }}
              >
                <TableCell padding="checkbox">
                  <Checkbox size="small" checked={isSelected} />
                </TableCell>
                <TableCell>
                  <Typography fontWeight={600}>{cam.name}</Typography>
                  <Typography variant="body2" color="text.secondary">
                    {cam.ip}
                  </Typography>
                </TableCell>
                <TableCell>
                  <Stack direction="row" spacing={1} alignItems="center">
                    {cam.detectedPort ? (
                      <Chip size="small" label={`TCP ${cam.detectedPort}`} variant="outlined" />
                    ) : (
                      <Chip size="small" label="TCP" variant="outlined" />
                    )}
                    {cam.onvifPort && cam.onvifPort !== cam.detectedPort && (
                      <Chip size="small" label={`ONVIF ${cam.onvifPort}`} color="primary" variant="outlined" />
                    )}
                  </Stack>
                </TableCell>
                <TableCell>
                  <Stack direction="row" spacing={1} alignItems="center">
                    <Chip
                      size="small"
                      label={`RTSP ${cam.port ?? 554}`}
                      color="success"
                      variant="outlined"
                    />
                    {cam.onvifPort && (
                      <Chip size="small" label={`ONVIF ${cam.onvifPort}`} color="info" variant="outlined" />
                    )}
                  </Stack>
                </TableCell>
                <TableCell>
                  <Chip size="small" label={cam.protocol ?? 'ONVIF'} />
                </TableCell>
              </TableRow>
            );
          })}
        </TableBody>
      </Table>
    );
  };

  return (
    <Dialog
      open={open}
      onClose={onClose}
      maxWidth="md"
      fullWidth
      PaperProps={{
        sx: {
          background: '#31363b',
          borderRadius: 3,
          boxShadow: 12,
          color: '#fff',
        },
      }}
    >
      <DialogTitle sx={{ fontWeight: 700, fontSize: 20, display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        {t('found_cameras')}
        <IconButton onClick={onClose} size="small" sx={{ color: 'rgba(255,255,255,0.7)' }}>
          <CloseIcon fontSize="small" />
        </IconButton>
      </DialogTitle>
      <DialogContent sx={{ display: 'flex', flexDirection: 'column', gap: 3 }}>
        <Box>
          <Stack spacing={1.5}>
            <Stack direction={{ xs: 'column', sm: 'row' }} justifyContent="space-between" alignItems={{ xs: 'flex-start', sm: 'center' }}>
              <Box>
                <Typography variant="subtitle2" fontWeight={600}>
                  {t('camera_search_interface_label')}
                </Typography>
                <Typography variant="caption" color="text.secondary">
                  {t('camera_search_interface_hint')}
                </Typography>
              </Box>
              <Stack direction="row" spacing={1} sx={{ mt: { xs: 1, sm: 0 } }}>
                <Button variant="text" size="small" onClick={() => loadInterfaces()} disabled={interfacesLoading}>
                  {t('camera_search_interface_refresh')}
                </Button>
                <Tooltip title={t('camera_search_select_all_hint')}>
                  <Button
                    variant="outlined"
                    size="small"
                    onClick={() => setSelectedInterfaceId('all')}
                    disabled={interfacesLoading}
                  >
                    {t('camera_search_all_interfaces_short')}
                  </Button>
                </Tooltip>
              </Stack>
            </Stack>
            <FormControl fullWidth size="small" disabled={interfacesLoading}>
              <InputLabel id="interface-select-label">
                {t('camera_search_interface_label')}
              </InputLabel>
              <Select
                labelId="interface-select-label"
                label={t('camera_search_interface_label')}
                value={selectedInterfaceId}
                onChange={handleInterfaceChange}
                renderValue={value => {
                  const typedValue = value as InterfaceSelection;
                  if (typedValue === 'all') {
                    return t('camera_search_all_interfaces');
                  }
                  const iface = interfaces.find(item => item.name === typedValue);
                  return iface ? getInterfaceLabel(iface) : typedValue;
                }}
              >
                <MenuItem value="all">
                  <ListItemText
                    primary={t('camera_search_all_interfaces')}
                    secondary={t('camera_search_all_interfaces_hint')}
                  />
                </MenuItem>
                {interfaces.map(iface => (
                  <MenuItem key={iface.name} value={iface.name} sx={{ alignItems: 'flex-start' }}>
                    <Stack direction="row" spacing={1} alignItems="center" sx={{ width: '100%' }}>
                      <ListItemText
                        primary={iface.displayName || iface.name}
                        secondary={`${iface.ipv4} • ${iface.cidr}`}
                      />
                      {iface.is_loopback && (
                        <Chip label={t('camera_search_loopback')} size="small" color="warning" />
                      )}
                    </Stack>
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
            {interfacesLoading && <LinearProgress />}
            {interfacesError && (
              <Typography color="error" variant="body2">
                {interfacesError}
              </Typography>
            )}
          </Stack>
        </Box>

        <Divider sx={{ opacity: 0.2 }} />

        <Box>
          <Stack direction="row" justifyContent="space-between" alignItems="center" sx={{ mb: 1 }}>
            <Typography variant="subtitle2" fontWeight={600}>
              {t('camera_search_results_title', { count: foundCameras.length })}
            </Typography>
            <Box>
              {discoveryProgress && (
                <Typography variant="caption" color="text.secondary">
                  {discoveryProgress}
                </Typography>
              )}
            </Box>
          </Stack>
          {scanError && (
            <Typography color="error" variant="body2" sx={{ mb: 1 }}>
              {scanError}
            </Typography>
          )}
          {isDiscovering && <LinearProgress sx={{ mb: 2 }} />}
          {renderCameraTable()}
        </Box>
      </DialogContent>
      <DialogActions sx={{ px: 3, pb: 3, justifyContent: 'space-between' }}>
        <Stack direction="row" spacing={1}>
          <Button variant="contained" onClick={() => startScan()} disabled={isDiscovering || interfacesLoading}>
            {isDiscovering ? t('camera_search_scanning') : t('camera_search_scan')}
          </Button>
        </Stack>
        <Stack direction="row" spacing={1}>
          <Button
            variant="contained"
            color="primary"
            disabled={selectedCameras.length === 0}
            onClick={() => onAddSelected && onAddSelected(selectedCameras)}
          >
            {t('add_selected')} ({selectedCameras.length})
          </Button>
        </Stack>
      </DialogActions>
    </Dialog>
  );
};

export default CameraSearchDialog;
