import React, { useCallback, useMemo } from 'react';
import {
  Typography,
  Box,
  Button,
  Stack,
  Alert,
  CircularProgress,
  Paper,
  Chip,
  Divider,
  List,
  ListItem,
  ListItemText,
} from '@mui/material';
import { BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend } from 'recharts';
import { useAnalytics } from '../hooks/useAnalytics';

interface StateChartDatum {
  state: string;
  count: number;
}

interface ProgressChartDatum {
  name: string;
  progress: number;
}

const STATE_LABELS: Record<string, string> = {
  ready: 'Ready',
  disabled: 'Disabled',
  error: 'Error',
  loading: 'Loading',
  other: 'Other',
};

const formatPercentage = (value: number): string => `${Math.round(value * 100)}%`;

const Analytics: React.FC = () => {
  const {
    modules,
    isLoadingModules,
    lastError,
    lastUpdatedAt,
    refreshModules,
    detections,
    processingModuleIds,
    clearDetections,
  } = useAnalytics();

  const handleRefresh = useCallback(() => {
    void refreshModules();
  }, [refreshModules]);

  const modulesReady = modules.length > 0;

  const enabledCount = useMemo(
    () => modules.filter(module => module.enabled).length,
    [modules],
  );

  const errorCount = useMemo(
    () => modules.filter(module => module.state === 'error').length,
    [modules],
  );

  const stateChartData: StateChartDatum[] = useMemo(() => {
    if (!modulesReady) {
      return [];
    }

    const totals: Record<string, number> = {
      ready: 0,
      disabled: 0,
      error: 0,
      loading: 0,
      other: 0,
    };

    modules.forEach(module => {
      if (module.state === 'ready' || module.state === 'disabled' || module.state === 'error') {
        totals[module.state] += 1;
      } else if (module.state === 'loading') {
        totals.loading += 1;
      } else {
        totals.other += 1;
      }
    });

    return Object.entries(totals)
      .filter(([, count]) => count > 0)
      .map(([state, count]) => ({
        state: STATE_LABELS[state] ?? state,
        count,
      }));
  }, [modules, modulesReady]);

  const progressChartData: ProgressChartDatum[] = useMemo(() => {
    if (!modulesReady) {
      return [];
    }

    return modules.map(module => ({
      name: module.name,
      progress: module.progress ?? (module.enabled ? 1 : 0),
    }));
  }, [modules, modulesReady]);

  const moduleNameLookup = useMemo(() => {
    const map = new Map<string, string>();
    modules.forEach(module => {
      map.set(module.id, module.name);
    });
    return map;
  }, [modules]);

  const recentDetections = useMemo(() => detections.slice(0, 10), [detections]);

  const isProcessing = processingModuleIds.length > 0;

  const formattedUpdatedAt = useMemo(() => {
    if (!lastUpdatedAt) {
      return '—';
    }

    const timestamp = Date.parse(lastUpdatedAt);
    if (Number.isNaN(timestamp)) {
      return lastUpdatedAt;
    }

    return new Date(timestamp).toLocaleString();
  }, [lastUpdatedAt]);

  return (
    <Box display="flex" flexDirection="column" gap={3}>
      <Stack direction="row" alignItems="center" justifyContent="space-between">
        <Box>
          <Typography variant="h4" gutterBottom>
            Analytics
          </Typography>
          <Typography variant="body2" color="text.secondary">
            Last updated: {formattedUpdatedAt}
          </Typography>
        </Box>
        <Button variant="contained" onClick={handleRefresh} disabled={isLoadingModules}>
          {isLoadingModules ? 'Refreshing…' : 'Refresh'}
        </Button>
      </Stack>

      {lastError && (
        <Alert severity="warning">
          {lastError}
        </Alert>
      )}

      <Stack direction={{ xs: 'column', md: 'row' }} spacing={2}>
        <Paper sx={{ p: 2, flex: 1 }}>
          <Typography variant="subtitle2" color="text.secondary">
            Total Modules
          </Typography>
          <Typography variant="h5">{modules.length}</Typography>
        </Paper>
        <Paper sx={{ p: 2, flex: 1 }}>
          <Typography variant="subtitle2" color="text.secondary">
            Enabled
          </Typography>
          <Typography variant="h5">{enabledCount}</Typography>
        </Paper>
        <Paper sx={{ p: 2, flex: 1 }}>
          <Typography variant="subtitle2" color="text.secondary">
            Errors
          </Typography>
          <Typography variant="h5">{errorCount}</Typography>
        </Paper>
      </Stack>

      {isLoadingModules && !modulesReady ? (
        <Box display="flex" flexDirection="column" alignItems="center" justifyContent="center" py={6}>
          <CircularProgress />
          <Typography variant="body2" color="text.secondary" mt={2}>
            Loading module data…
          </Typography>
        </Box>
      ) : modulesReady ? (
        <Box display="flex" flexDirection="column" gap={3}>
          {isProcessing && (
            <Alert severity="info">
              Processing analytics for modules: {processingModuleIds.map(id => moduleNameLookup.get(id) ?? id).join(', ')}
            </Alert>
          )}

          {stateChartData.length > 0 && (
            <Box>
              <Typography variant="h6" gutterBottom>
                Module States
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <BarChart data={stateChartData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="state" />
                  <YAxis allowDecimals={false} />
                  <Tooltip />
                  <Legend />
                  <Bar dataKey="count" fill="#8884d8" name="Modules" />
                </BarChart>
              </ResponsiveContainer>
            </Box>
          )}

          {progressChartData.length > 0 && (
            <Box>
              <Typography variant="h6" gutterBottom>
                Module Progress
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <BarChart data={progressChartData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="name" interval={0} angle={-15} textAnchor="end" height={80} />
                  <YAxis tickFormatter={formatPercentage} domain={[0, 1]} />
                  <Tooltip formatter={value => formatPercentage(Number(value))} />
                  <Legend />
                  <Bar dataKey="progress" fill="#82ca9d" name="Progress" />
                </BarChart>
              </ResponsiveContainer>
            </Box>
          )}

          <Paper sx={{ p: 2 }}>
            <Stack direction="row" justifyContent="space-between" alignItems="center" mb={2}>
              <Box>
                <Typography variant="h6">Recent Detections</Typography>
                <Typography variant="body2" color="text.secondary">
                  Showing up to the 10 most recent detection events.
                </Typography>
              </Box>
              <Button variant="outlined" size="small" onClick={clearDetections} disabled={detections.length === 0}>
                Clear
              </Button>
            </Stack>

            {recentDetections.length === 0 ? (
              <Typography variant="body2" color="text.secondary">
                No detections recorded yet.
              </Typography>
            ) : (
              <List disablePadding>
                {recentDetections.map((event, index) => {
                  const moduleLabel = moduleNameLookup.get(event.moduleId) ?? event.moduleId;
                  const cameraLabel = event.cameraId ?? 'Unknown camera';
                  const detectionCount = event.detections.length;
                  const processedAt = new Date(event.processedAt).toLocaleString();
                  const receivedAt = new Date(event.receivedAt).toLocaleTimeString();

                  return (
                    <React.Fragment key={event.id}>
                      {index > 0 && <Divider component="li" />}
                      <ListItem alignItems="flex-start">
                        <ListItemText
                          primary={
                            <Stack direction="row" spacing={1} alignItems="center">
                              <Typography variant="subtitle1">{moduleLabel}</Typography>
                              <Chip label={`${detectionCount} detections`} size="small" color={detectionCount > 0 ? 'success' : 'default'} />
                            </Stack>
                          }
                          secondary={
                            <Stack spacing={0.5} mt={1}>
                              <Typography variant="body2" color="text.secondary">
                                Camera: {cameraLabel}
                              </Typography>
                              <Typography variant="body2" color="text.secondary">
                                Processed: {processedAt} • Received: {receivedAt}
                              </Typography>
                              {event.detections.slice(0, 3).map(detection => (
                                <Typography key={detection.id} variant="body2">
                                  • {detection.label} ({Math.round(detection.confidence * 100)}%)
                                </Typography>
                              ))}
                              {event.detections.length > 3 && (
                                <Typography variant="body2" color="text.secondary">
                                  +{event.detections.length - 3} more…
                                </Typography>
                              )}
                            </Stack>
                          }
                        />
                      </ListItem>
                    </React.Fragment>
                  );
                })}
              </List>
            )}
          </Paper>
        </Box>
      ) : (
        <Paper sx={{ p: 4, textAlign: 'center' }}>
          <Typography variant="subtitle1" gutterBottom>
            No analytics module data available yet.
          </Typography>
          <Typography variant="body2" color="text.secondary">
            Enable analytics modules in Settings → Modules to see analytics insights here.
          </Typography>
        </Paper>
      )}
    </Box>
  );
};

export default Analytics;