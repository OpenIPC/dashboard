import React from 'react';
import { Box, IconButton, Paper, Typography } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import { useLoggerUi } from '../contexts/LoggerUiContext';
import { useLocalization } from '../hooks/useLocalization';
import LogViewer from './LogViewer';

const LogViewerModal: React.FC = () => {
  const { isOpen, closeViewer } = useLoggerUi();
  const { t } = useLocalization();

  if (!isOpen) {
    return null;
  }

  return (
    <Box
      sx={{
        position: 'fixed',
        inset: 0,
        bgcolor: 'rgba(0, 0, 0, 0.65)',
        zIndex: 2200,
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        p: 2,
        backdropFilter: 'blur(4px)'
      }}
    >
      <Paper
        elevation={6}
        sx={{
          width: 'min(1200px, 95vw)',
          height: 'min(90vh, 820px)',
          bgcolor: '#181a1f',
          borderRadius: 2,
          border: '1px solid rgba(255, 255, 255, 0.08)',
          boxShadow: '0 30px 60px rgba(0,0,0,0.7)',
          display: 'flex',
          flexDirection: 'column',
        }}
      >
        <Box
          sx={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            px: 2,
            py: 1,
            borderBottom: '1px solid rgba(255,255,255,0.08)'
          }}
        >
          <Typography variant="h6" sx={{ color: '#fff', fontWeight: 600 }}>
            {t('log_viewer.title')}
          </Typography>
          <IconButton
            onClick={closeViewer}
            aria-label={t('close_button')}
            size="small"
            sx={{ color: '#fff' }}
          >
            <CloseIcon />
          </IconButton>
        </Box>
        <Box sx={{ flex: 1, minHeight: 0 }}>
          <LogViewer variant="compact" showTitle={false} />
        </Box>
      </Paper>
    </Box>
  );
};

export default LogViewerModal;
