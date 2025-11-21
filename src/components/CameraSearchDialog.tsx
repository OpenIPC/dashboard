import React from 'react';
import { Dialog, DialogTitle, DialogContent, Box, Typography, Button, Paper, Stack } from '@mui/material';
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

const CameraSearchDialog: React.FC<{
  open: boolean;
  onClose: () => void;
  foundCameras: DiscoveredCamera[];
  onAddSelected?: (cam: DiscoveredCamera) => void;
  isDiscovering?: boolean;
  discoveryProgress?: string;
}> = ({ open, onClose, foundCameras, onAddSelected, isDiscovering = false, discoveryProgress = '' }) => {
  const { t } = useLocalization();
  const [selectedIp, setSelectedIp] = React.useState<string | null>(null);

  React.useEffect(() => {
    setSelectedIp(null);
  }, [open, foundCameras]);

  const selectedCamera = foundCameras.find(cam => cam.ip === selectedIp) || null;

  return (
    <Dialog open={open} onClose={onClose} maxWidth="sm" PaperProps={{
      sx: {
        background: '#393e43',
        borderRadius: 3,
        boxShadow: 8,
        minWidth: 400,
        color: '#fff',
        p: 0
      }
    }}>
      <DialogTitle sx={{ pb: 0, pt: 2, px: 3, fontWeight: 700, fontSize: 22, color: '#fff', textAlign: 'center' }}>
        {t('found_cameras')}
      </DialogTitle>
      <DialogContent sx={{ pt: 0, px: 3, pb: 3 }}>
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mb: 2 }}>
          {discoveryProgress && (
            <Typography variant="caption" color="#9fa6ad" textAlign="center">
              {discoveryProgress}
            </Typography>
          )}
          {foundCameras.length === 0 ? (
            <Box sx={{ textAlign: 'center', mt: 2 }}>
              <Typography variant="body2" color="#bbb">
                {isDiscovering ? t('searching') : t('nothing_found')}
              </Typography>
              {!isDiscovering && (
                <Typography variant="caption" color="#888" sx={{ display: 'block', mt: 1 }}>
                  {t('network_help_text')}
                </Typography>
              )}
            </Box>
          ) : foundCameras.map(cam => (
            <Paper
              key={cam.ip}
              sx={{
                bgcolor: selectedIp === cam.ip ? '#1976d2' : '#23272b',
                color: selectedIp === cam.ip ? '#fff' : '#fff',
                p: 2,
                borderRadius: 2,
                boxShadow: 0,
                cursor: 'pointer',
                border: selectedIp === cam.ip ? '2px solid #1976d2' : '2px solid transparent',
                transition: 'border 0.2s',
              }}
              onClick={() => setSelectedIp(cam.ip)}
            >
              <Typography fontWeight={700}>{cam.name}</Typography>
              <Typography variant="body2" color="#bbb">{cam.ip}</Typography>
            </Paper>
          ))}
        </Box>
        <Stack direction="row" justifyContent="space-between" alignItems="center">
          <Button
            variant="text"
            size="small"
            sx={{ color: '#ddd' }}
            disabled={isDiscovering}
            onClick={async () => { 
              try { 
                await invoke('discover_cameras'); 
              } catch (error) {
                console.error('Failed to restart discovery:', error);
              }
            }}
          >
            {t('repeat_search')}
          </Button>
          <Box sx={{ display: 'flex', justifyContent: 'flex-end', gap: 2 }}>
            <Button
            variant="outlined"
            sx={{ color: '#fff', borderColor: '#50545a', bgcolor: '#50545a' }}
            disabled={!selectedCamera}
            onClick={() => selectedCamera && onAddSelected && onAddSelected(selectedCamera)}
          >
            {t('add_selected')}
            </Button>
            <Button variant="contained" sx={{ bgcolor: '#50545a', color: '#fff' }} onClick={onClose}>
              {t('close')}
            </Button>
          </Box>
        </Stack>
      </DialogContent>
    </Dialog>
  );
};

export default CameraSearchDialog;
