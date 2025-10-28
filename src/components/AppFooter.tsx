import React, { useEffect, useMemo, useState } from 'react';
import { Box, IconButton, Tooltip, Typography } from '@mui/material';
import { invoke } from '@tauri-apps/api/core';
import { useLocalization } from '../contexts/LocalizationContext';
import QueryStatsRoundedIcon from '@mui/icons-material/QueryStatsRounded';
import AppStatus from './AppStatus';

interface AppResourceUsage {
  cpuUsage: number;
  memoryBytes: number;
  timestamp: number;
}

const POLL_INTERVAL = 3000;

const formatBytes = (bytes: number, fractionDigits = 1) => {
  if (!Number.isFinite(bytes) || bytes <= 0) {
    return '0 B';
  }
  const units = ['B', 'KB', 'MB', 'GB', 'TB'];
  const exponent = Math.min(Math.floor(Math.log(bytes) / Math.log(1024)), units.length - 1);
  const value = bytes / Math.pow(1024, exponent);
  return `${value.toFixed(fractionDigits)} ${units[exponent]}`;
};

const AppFooter: React.FC = () => {
  const { t } = useLocalization();
  const [usage, setUsage] = useState<AppResourceUsage | null>(null);
  const [errorKey, setErrorKey] = useState<string | null>(null);
  const [statusOpen, setStatusOpen] = useState(false);

  useEffect(() => {
    let isMounted = true;

    const fetchUsage = async () => {
      try {
        const stats = await invoke<AppResourceUsage>('get_app_resource_usage');
        console.debug('[AppFooter] Resource stats', stats);
        if (isMounted) {
          setUsage(stats);
          setErrorKey(null);
        }
      } catch (err) {
        console.error('Failed to fetch resource usage', err);
        if (isMounted) {
          setErrorKey('resource_monitor_unavailable');
          setUsage(null);
        }
      }
    };

    fetchUsage();
    const interval = setInterval(fetchUsage, POLL_INTERVAL);

    return () => {
      isMounted = false;
      clearInterval(interval);
    };
  }, []);

  const cpuDisplay = useMemo(() => {
    if (!usage || typeof usage.cpuUsage !== 'number' || Number.isNaN(usage.cpuUsage)) {
      return '--';
    }
    const value = Number.isFinite(usage.cpuUsage) ? usage.cpuUsage : 0;
    return `${value.toFixed(1)}%`;
  }, [usage]);

  const memoryDisplay = useMemo(() => {
    if (!usage || typeof usage.memoryBytes !== 'number') {
      return '--';
    }

    return formatBytes(usage.memoryBytes, 1);
  }, [usage]);

  return (
    <>
      <Box
        component="footer"
        sx={{
          bgcolor: '#1b1f24',
          borderTop: '1px solid rgba(255,255,255,0.08)',
          color: 'rgba(255,255,255,0.7)',
          px: 2,
          py: 1,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          fontSize: '0.85rem',
        }}
      >
        <Box sx={{ display: 'flex', gap: 3, alignItems: 'center' }}>
          <Box sx={{ display: 'flex', gap: 0.75, alignItems: 'baseline' }}>
            <Typography variant="body2" component="span" sx={{ fontWeight: 600, color: '#e0e5ec' }}>
              {t('status_cpu')}:
            </Typography>
            <Typography variant="body2" component="span">{cpuDisplay}</Typography>
          </Box>

          <Box sx={{ display: 'flex', gap: 0.75, alignItems: 'baseline' }}>
            <Typography variant="body2" component="span" sx={{ fontWeight: 600, color: '#e0e5ec' }}>
              {t('status_ram')}:
            </Typography>
            <Typography variant="body2" component="span">{memoryDisplay}</Typography>
          </Box>
        </Box>

        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1.25 }}>
          <Tooltip title={t('stream_status')}>
            <IconButton
              size="small"
              onClick={() => setStatusOpen(true)}
              sx={{ color: '#e0e5ec' }}
            >
              <QueryStatsRoundedIcon fontSize="small" />
            </IconButton>
          </Tooltip>
          <Typography variant="body2" component="span" sx={{ fontSize: '0.75rem', color: errorKey ? '#f48fb1' : 'rgba(255,255,255,0.5)' }}>
            {errorKey ? t(errorKey) : usage ? new Date(usage.timestamp).toLocaleTimeString() : ''}
          </Typography>
        </Box>
      </Box>
      <AppStatus open={statusOpen} onClose={() => setStatusOpen(false)} />
    </>
  );
};

export default AppFooter;
