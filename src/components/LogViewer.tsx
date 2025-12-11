import React, { useState, useEffect, useMemo } from 'react';
import {
  Box,
  Paper,
  Typography,
  TextField,
  Select,
  MenuItem,
  Button,
  IconButton,
  Chip,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  FormControl,
  InputLabel,
  Tooltip,
  Alert,
  Snackbar,
} from '@mui/material';
import DeleteIcon from '@mui/icons-material/Delete';
import DownloadIcon from '@mui/icons-material/Download';
import RefreshIcon from '@mui/icons-material/Refresh';
import SearchIcon from '@mui/icons-material/Search';
import TableSortLabel from '@mui/material/TableSortLabel';
import type { AlertColor } from '@mui/material/Alert';
import { useLogger } from '../contexts/LoggerContext';
import type { LogLevel, LogCategory } from '../services/logger';
import { useLocalization } from '../hooks/useLocalization';

const LOG_LEVEL_COLORS: Record<LogLevel, string> = {
  info: '#2196f3',
  warn: '#ff9800',
  error: '#f44336',
  debug: '#9e9e9e',
};

interface LogViewerProps {
  variant?: 'full' | 'compact';
  showTitle?: boolean;
}

const LogViewer: React.FC<LogViewerProps> = ({ variant = 'full', showTitle = true }) => {
  const { logs, clearLogs, exportLogs, getFilteredLogs, stats } = useLogger();
  const { t, currentLanguage } = useLocalization();
  const isCompact = variant === 'compact';
  const [levelFilter, setLevelFilter] = useState<LogLevel | ''>('');
  const [categoryFilter, setCategoryFilter] = useState<LogCategory | ''>('');
  const [searchText, setSearchText] = useState('');
  const [autoScroll, setAutoScroll] = useState(true);
  const [refreshTick, setRefreshTick] = useState(0);
  const [sortField, setSortField] = useState<'time' | 'level' | 'category'>('time');
  const [sortDirection, setSortDirection] = useState<'asc' | 'desc'>('desc');
  const [snackbar, setSnackbar] = useState<{ open: boolean; message: string; severity: AlertColor } | null>(null);
  const tableEndRef = React.useRef<HTMLDivElement>(null);

  const levelLabels: Record<LogLevel, string> = {
    info: t('log_viewer.levels.info'),
    warn: t('log_viewer.levels.warn'),
    error: t('log_viewer.levels.error'),
    debug: t('log_viewer.levels.debug'),
  };

  const categoryLabels: Record<LogCategory, string> = {
    app: t('log_viewer.categories.app'),
    camera: t('log_viewer.categories.camera'),
    stream: t('log_viewer.categories.stream'),
    ptz: t('log_viewer.categories.ptz'),
    network: t('log_viewer.categories.network'),
    analytics: t('log_viewer.categories.analytics'),
    auth: t('log_viewer.categories.auth'),
    system: t('log_viewer.categories.system'),
  };

  const levelOptions: LogLevel[] = ['info', 'warn', 'error', 'debug'];
  const categoryOptions: LogCategory[] = ['app', 'camera', 'stream', 'ptz', 'network', 'analytics', 'auth', 'system'];

  // Фильтрация логов
  const filteredLogs = useMemo(() => {
    return getFilteredLogs({
      level: levelFilter || undefined,
      category: categoryFilter || undefined,
      search: searchText || undefined,
    });
  }, [logs, getFilteredLogs, levelFilter, categoryFilter, searchText, refreshTick]);

  const sortedLogs = useMemo(() => {
    const data = [...filteredLogs];
    data.sort((a, b) => {
      switch (sortField) {
        case 'level':
          return sortDirection === 'asc'
            ? a.level.localeCompare(b.level)
            : b.level.localeCompare(a.level);
        case 'category':
          return sortDirection === 'asc'
            ? a.category.localeCompare(b.category)
            : b.category.localeCompare(a.category);
        case 'time':
        default:
          return sortDirection === 'asc'
            ? a.timestamp - b.timestamp
            : b.timestamp - a.timestamp;
      }
    });
    return data;
  }, [filteredLogs, sortField, sortDirection]);

  // Автоскролл к последнему логу
  useEffect(() => {
    if (autoScroll && tableEndRef.current) {
      tableEndRef.current.scrollIntoView({ behavior: 'smooth' });
    }
  }, [autoScroll, sortedLogs]);

  const handleExport = () => {
    const content = exportLogs();
    const blob = new Blob([content], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `app-logs-${new Date().toISOString()}.txt`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    setSnackbar({ open: true, message: t('log_viewer.export_success'), severity: 'success' });
  };

  const handleClear = () => {
    if (confirm(t('log_viewer.clear_confirm'))) {
      clearLogs();
    }
  };

  const handleRefresh = () => {
    setRefreshTick(prev => prev + 1);
    setSnackbar({ open: true, message: t('log_viewer.refresh_success'), severity: 'info' });
  };

  const handleSortChange = (field: 'time' | 'level' | 'category') => {
    if (sortField === field) {
      setSortDirection(prev => (prev === 'asc' ? 'desc' : 'asc'));
    } else {
      setSortField(field);
      setSortDirection(field === 'time' ? 'desc' : 'asc');
    }
  };

  const formatTimestamp = (timestamp: number) => {
    const date = new Date(timestamp);
    const locale = currentLanguage === 'ru' ? 'ru-RU' : 'en-US';
    return date.toLocaleTimeString(locale, {
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit',
      fractionalSecondDigits: 3,
    });
  };

  const formatDetails = (details: unknown) => {
    if (!details) return '';
    try {
      return JSON.stringify(details, null, 2);
    } catch {
      return String(details);
    }
  };

  const selectMenuProps = { disablePortal: true };

  const renderSortableHeader = (field: 'time' | 'level' | 'category', label: string) => (
    <TableSortLabel
      active={sortField === field}
      direction={sortField === field ? sortDirection : 'asc'}
      onClick={() => handleSortChange(field)}
      title={t('log_viewer.sort_hint')}
      sx={{ color: '#fff', '& .MuiTableSortLabel-icon': { color: '#fff !important' } }}
    >
      {label}
    </TableSortLabel>
  );

  const handleSnackbarClose = () => setSnackbar(null);

  return (
    <Box sx={{ p: isCompact ? 1.5 : 2, height: '100%', display: 'flex', flexDirection: 'column', gap: isCompact ? 1.5 : 2 }}>
      {showTitle && (
        <Typography variant={isCompact ? 'h5' : 'h4'} gutterBottom sx={{ color: '#fff', mb: isCompact ? 0.5 : 0 }}>
          {t('log_viewer.title')}
        </Typography>
      )}

      {/* Статистика */}
      <Paper sx={{ p: isCompact ? 1.5 : 2, backgroundColor: '#1e1e1e', color: '#fff' }}>
        <Box sx={{ display: 'flex', gap: isCompact ? 1 : 2, flexWrap: 'wrap', alignItems: 'center' }}>
          <Typography variant="body2">
            {t('log_viewer.total_entries_label')}{' '}
            <strong>{stats.total}</strong>
          </Typography>
          <Chip
            label={`${levelLabels.info}: ${stats.byLevel.info}`}
            size="small"
            sx={{ bgcolor: LOG_LEVEL_COLORS.info, color: '#fff' }}
          />
          <Chip
            label={`${levelLabels.warn}: ${stats.byLevel.warn}`}
            size="small"
            sx={{ bgcolor: LOG_LEVEL_COLORS.warn, color: '#fff' }}
          />
          <Chip
            label={`${levelLabels.error}: ${stats.byLevel.error}`}
            size="small"
            sx={{ bgcolor: LOG_LEVEL_COLORS.error, color: '#fff' }}
          />
          <Chip
            label={`${levelLabels.debug}: ${stats.byLevel.debug}`}
            size="small"
            sx={{ bgcolor: LOG_LEVEL_COLORS.debug, color: '#fff' }}
          />
        </Box>
      </Paper>

      {/* Панель фильтров */}
      <Paper sx={{ p: isCompact ? 1.5 : 2, backgroundColor: '#1e1e1e', color: '#fff' }}>
        <Box sx={{ display: 'flex', gap: isCompact ? 1 : 2, flexWrap: 'wrap', alignItems: 'center' }}>
          <TextField
            label={t('log_viewer.search_label')}
            variant="outlined"
            size="small"
            value={searchText}
            onChange={(e) => setSearchText(e.target.value)}
            InputProps={{
              endAdornment: <SearchIcon sx={{ color: '#888' }} />,
            }}
            sx={{
              flex: 1,
              minWidth: '200px',
              input: { color: '#fff' },
              label: { color: '#ccc' },
              fieldset: { borderColor: '#555' },
            }}
          />

          <FormControl size="small" sx={{ minWidth: 120 }}>
            <InputLabel sx={{ color: '#ccc' }}>{t('log_viewer.level_filter_label')}</InputLabel>
            <Select
              value={levelFilter}
              onChange={(e) => setLevelFilter(e.target.value as LogLevel | '')}
              label={t('log_viewer.level_filter_label')}
              sx={{ color: '#fff', fieldset: { borderColor: '#555' } }}
              MenuProps={selectMenuProps}
            >
              <MenuItem value="">{t('log_viewer.all_levels')}</MenuItem>
              {levelOptions.map(level => (
                <MenuItem key={level} value={level}>
                  {levelLabels[level]}
                </MenuItem>
              ))}
            </Select>
          </FormControl>

          <FormControl size="small" sx={{ minWidth: 120 }}>
            <InputLabel sx={{ color: '#ccc' }}>{t('log_viewer.category_filter_label')}</InputLabel>
            <Select
              value={categoryFilter}
              onChange={(e) => setCategoryFilter(e.target.value as LogCategory | '')}
              label={t('log_viewer.category_filter_label')}
              sx={{ color: '#fff', fieldset: { borderColor: '#555' } }}
              MenuProps={selectMenuProps}
            >
              <MenuItem value="">{t('log_viewer.all_categories')}</MenuItem>
              {categoryOptions.map(category => (
                <MenuItem key={category} value={category}>
                  {categoryLabels[category]}
                </MenuItem>
              ))}
            </Select>
          </FormControl>

          <Tooltip title={t('log_viewer.autoscroll_tooltip')}>
            <Button
              variant={autoScroll ? 'contained' : 'outlined'}
              size="small"
              onClick={() => setAutoScroll(!autoScroll)}
            >
              {t('log_viewer.autoscroll_button')}
            </Button>
          </Tooltip>

          <Tooltip title={t('log_viewer.refresh_tooltip')}>
            <IconButton size="small" onClick={handleRefresh}>
              <RefreshIcon sx={{ color: '#fff' }} />
            </IconButton>
          </Tooltip>

          <Tooltip title={t('log_viewer.export_tooltip')}>
            <IconButton size="small" onClick={handleExport}>
              <DownloadIcon sx={{ color: '#fff' }} />
            </IconButton>
          </Tooltip>

          <Tooltip title={t('log_viewer.clear_tooltip')}>
            <IconButton size="small" onClick={handleClear}>
              <DeleteIcon sx={{ color: '#f44336' }} />
            </IconButton>
          </Tooltip>
        </Box>
      </Paper>

      {/* Таблица логов */}
      <Paper sx={{ flex: 1, backgroundColor: '#1e1e1e', color: '#fff', overflow: 'hidden', display: 'flex', flexDirection: 'column' }}>
        {sortedLogs.length === 0 ? (
          <Box sx={{ p: 3, textAlign: 'center' }}>
            <Alert severity="info">{t('log_viewer.empty_state')}</Alert>
          </Box>
        ) : (
          <TableContainer sx={{ flex: 1, overflow: 'auto' }}>
            <Table stickyHeader size="small">
              <TableHead>
                <TableRow>
                  <TableCell sx={{ bgcolor: '#2a2f33', color: '#fff', fontWeight: 'bold', width: '100px', fontSize: isCompact ? '0.75rem' : '0.85rem' }}>
                    {renderSortableHeader('time', t('log_viewer.table.time'))}
                  </TableCell>
                  <TableCell sx={{ bgcolor: '#2a2f33', color: '#fff', fontWeight: 'bold', width: '80px', fontSize: isCompact ? '0.75rem' : '0.85rem' }}>
                    {renderSortableHeader('level', t('log_viewer.table.level'))}
                  </TableCell>
                  <TableCell sx={{ bgcolor: '#2a2f33', color: '#fff', fontWeight: 'bold', width: '100px', fontSize: isCompact ? '0.75rem' : '0.85rem' }}>
                    {renderSortableHeader('category', t('log_viewer.table.category'))}
                  </TableCell>
                  <TableCell sx={{ bgcolor: '#2a2f33', color: '#fff', fontWeight: 'bold', fontSize: isCompact ? '0.75rem' : '0.85rem' }}>
                    {t('log_viewer.table.message')}
                  </TableCell>
                </TableRow>
              </TableHead>
              <TableBody>
                {sortedLogs.map((log) => (
                  <TableRow
                    key={log.id}
                    sx={{
                      '&:hover': { bgcolor: 'rgba(255,255,255,0.05)' },
                      bgcolor: log.level === 'error' ? 'rgba(244,67,54,0.1)' : 'transparent',
                    }}
                  >
                    <TableCell sx={{ color: '#aaa', fontSize: isCompact ? '0.7rem' : '0.75rem', fontFamily: 'monospace', whiteSpace: 'nowrap' }}>
                      {formatTimestamp(log.timestamp)}
                    </TableCell>
                    <TableCell>
                      <Chip
                        label={levelLabels[log.level]}
                        size="small"
                        sx={{
                          bgcolor: LOG_LEVEL_COLORS[log.level],
                          color: '#fff',
                          fontWeight: 'bold',
                          fontSize: isCompact ? '0.65rem' : '0.7rem',
                        }}
                      />
                    </TableCell>
                    <TableCell sx={{ color: '#ccc', fontSize: isCompact ? '0.75rem' : '0.85rem' }}>
                      {categoryLabels[log.category]}
                    </TableCell>
                    <TableCell sx={{ color: '#fff' }}>
                      <Box>
                        <Typography variant="body2" sx={{ wordBreak: 'break-word', fontSize: isCompact ? '0.85rem' : '1rem' }}>
                          {log.message}
                        </Typography>
                        {Boolean(log.details) && (
                          <Box
                            sx={{
                              mt: 1,
                              p: 1,
                              bgcolor: 'rgba(0,0,0,0.3)',
                              borderRadius: 1,
                              fontFamily: 'monospace',
                              fontSize: isCompact ? '0.7rem' : '0.75rem',
                              color: '#ddd',
                              maxHeight: '200px',
                              overflow: 'auto',
                            }}
                          >
                            <pre style={{ margin: 0, whiteSpace: 'pre-wrap' }}>
                              {formatDetails(log.details)}
                            </pre>
                          </Box>
                        )}
                        {log.stack && (
                          <Box
                            sx={{
                              mt: 1,
                              p: 1,
                              bgcolor: 'rgba(244,67,54,0.1)',
                              borderRadius: 1,
                              fontFamily: 'monospace',
                              fontSize: isCompact ? '0.65rem' : '0.7rem',
                              color: '#f44336',
                              maxHeight: '150px',
                              overflow: 'auto',
                            }}
                          >
                            <pre style={{ margin: 0, whiteSpace: 'pre-wrap' }}>{log.stack}</pre>
                          </Box>
                        )}
                      </Box>
                    </TableCell>
                  </TableRow>
                ))}
                <TableRow>
                  <TableCell colSpan={4} ref={tableEndRef} sx={{ height: 0, p: 0, border: 0 }} />
                </TableRow>
              </TableBody>
            </Table>
          </TableContainer>
        )}
      </Paper>
      {snackbar && (
        <Snackbar
          open={snackbar.open}
          autoHideDuration={4000}
          onClose={handleSnackbarClose}
          anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
        >
          <Alert severity={snackbar.severity} onClose={handleSnackbarClose} sx={{ width: '100%' }}>
            {snackbar.message}
          </Alert>
        </Snackbar>
      )}
    </Box>
  );
};

export default LogViewer;
