import React, { useState } from 'react';
import { Dialog, DialogTitle, DialogContent, DialogActions, TextField, Button, Box, Typography, LinearProgress } from '@mui/material';
import { useLocalization } from '../hooks/useLocalization';

interface BatchAddCameraDialogProps {
  open: boolean;
  onClose: () => void;
  onAdd: (credentials: { user: string; pass: string }, onProgress?: (current: number) => void) => Promise<void>;
  count: number;
}

const BatchAddCameraDialog: React.FC<BatchAddCameraDialogProps> = ({ open, onClose, onAdd, count }) => {
  const { t } = useLocalization();
  const [user, setUser] = useState('admin');
  const [pass, setPass] = useState('');
  const [isAdding, setIsAdding] = useState(false);
  const [progress, setProgress] = useState(0);

  const handleAdd = async () => {
    setIsAdding(true);
    setProgress(0);
    try {
      await onAdd({ user, pass }, (current) => {
        setProgress(current);
      });
      onClose();
    } catch (error) {
      console.error('Failed to batch add cameras:', error);
    } finally {
      setIsAdding(false);
    }
  };

  return (
    <Dialog open={open} onClose={!isAdding ? onClose : undefined} maxWidth="xs" fullWidth PaperProps={{
      sx: {
        bgcolor: '#393e43',
        color: '#fff'
      }
    }}>
      <DialogTitle sx={{ color: '#fff' }}>{t('batch_add_cameras')}</DialogTitle>
      <DialogContent>
        <Typography variant="body2" sx={{ mb: 2, color: '#bbb' }}>
          {t('batch_add_description', { count })}
        </Typography>
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
          <TextField
            label={t('login')}
            value={user}
            onChange={(e) => setUser(e.target.value)}
            fullWidth
            size="small"
            variant="outlined"
            disabled={isAdding}
            InputLabelProps={{ sx: { color: '#aaa' } }}
            InputProps={{ sx: { color: '#fff', '& .MuiOutlinedInput-notchedOutline': { borderColor: '#555' } } }}
          />
          <TextField
            label={t('password')}
            type="password"
            value={pass}
            onChange={(e) => setPass(e.target.value)}
            fullWidth
            size="small"
            variant="outlined"
            disabled={isAdding}
            InputLabelProps={{ sx: { color: '#aaa' } }}
            InputProps={{ sx: { color: '#fff', '& .MuiOutlinedInput-notchedOutline': { borderColor: '#555' } } }}
          />
        </Box>
        {isAdding && (
          <Box sx={{ mt: 3 }}>
            <Typography variant="caption" sx={{ color: '#aaa', mb: 1, display: 'block' }}>
              {t('adding_cameras_progress', { current: progress, total: count })}
            </Typography>
            <LinearProgress variant="determinate" value={(progress / count) * 100} />
          </Box>
        )}
      </DialogContent>
      <DialogActions>
        <Button onClick={onClose} sx={{ color: '#aaa' }} disabled={isAdding}>{t('cancel')}</Button>
        <Button onClick={handleAdd} variant="contained" color="primary" disabled={isAdding}>
          {isAdding ? t('adding') : t('add_selected')}
        </Button>
      </DialogActions>
    </Dialog>
  );
};

export default BatchAddCameraDialog;
