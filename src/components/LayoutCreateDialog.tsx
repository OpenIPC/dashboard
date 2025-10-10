import React, { useEffect, useState } from 'react';
import {
  Box,
  Button,
  Dialog,
  DialogContent,
  IconButton,
  TextField,
  Typography
} from '@mui/material';
import { Close as CloseIcon, Check as CheckIcon } from '@mui/icons-material';
import { useLocalization } from '../contexts/LocalizationContext';

interface LayoutCreateDialogProps {
  open: boolean;
  defaultName: string;
  onCancel: () => void;
  onSubmit: (name: string) => void;
}

const LayoutCreateDialog: React.FC<LayoutCreateDialogProps> = ({
  open,
  defaultName,
  onCancel,
  onSubmit
}) => {
  const { t } = useLocalization();
  const [name, setName] = useState(defaultName);
  const [touched, setTouched] = useState(false);

  useEffect(() => {
    if (open) {
      setName(defaultName);
      setTouched(false);
    }
  }, [open, defaultName]);

  const handleSubmit = () => {
    const trimmed = name.trim();
    if (!trimmed) {
      setTouched(true);
      return;
    }
    onSubmit(trimmed);
  };

  const showError = touched && !name.trim();

  return (
    <Dialog
      open={open}
      onClose={onCancel}
      maxWidth="xs"
      fullWidth
      PaperProps={{
        sx: {
          background: 'linear-gradient(180deg, #2f3540 0%, #1f242d 100%)',
          border: '1px solid rgba(82, 182, 255, 0.35)',
          boxShadow: '0 16px 40px rgba(0, 0, 0, 0.45)',
          borderRadius: 2,
          overflow: 'hidden'
        }
      }}
    >
      <Box
        sx={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          px: 3,
          py: 2,
          bgcolor: 'rgba(29, 35, 44, 0.9)',
          borderBottom: '1px solid rgba(82,182,255,0.25)'
        }}
      >
        <Box>
          <Typography variant="subtitle2" sx={{ color: 'rgba(150, 190, 230, 0.75)', textTransform: 'uppercase', letterSpacing: 1 }}>
            {t('layout_create_title')}
          </Typography>
          <Typography variant="h6" sx={{ color: '#eef5ff', fontWeight: 600 }}>
            {t('layout_create_subtitle')}
          </Typography>
        </Box>
        <IconButton
          onClick={onCancel}
          size="small"
          sx={{
            bgcolor: 'rgba(255, 255, 255, 0.05)',
            color: 'rgba(255, 255, 255, 0.7)',
            '&:hover': {
              bgcolor: 'rgba(255, 255, 255, 0.12)',
              color: '#fff'
            }
          }}
        >
          <CloseIcon fontSize="small" />
        </IconButton>
      </Box>

      <DialogContent sx={{ px: 3, py: 4 }}>
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 3 }}>
          <Box>
            <Typography variant="body2" sx={{ color: 'rgba(200, 215, 235, 0.65)', mb: 1 }}>
              {t('layout_create_hint')}
            </Typography>
            <TextField
              value={name}
              onChange={(event) => setName(event.target.value)}
              onBlur={() => setTouched(true)}
              fullWidth
              autoFocus
              variant="filled"
              label={t('layout_name_label')}
              placeholder={t('layout_name_placeholder') || ''}
              error={showError}
              helperText={showError ? t('layout_name_required') : ' '}
              InputProps={{
                sx: {
                  bgcolor: 'rgba(22, 28, 36, 0.9)',
                  borderRadius: 1,
                  input: {
                    color: '#f3f8ff',
                    fontWeight: 500,
                    letterSpacing: 0.3
                  }
                }
              }}
              InputLabelProps={{
                sx: {
                  color: 'rgba(160, 190, 220, 0.7)',
                  '&.Mui-focused': {
                    color: '#6cb8ff'
                  }
                }
              }}
            />
          </Box>

          <Box sx={{ display: 'flex', justifyContent: 'flex-end', gap: 1.5 }}>
            <Button onClick={onCancel} color="inherit" sx={{ color: 'rgba(200,210,225,0.8)' }}>
              {t('cancel')}
            </Button>
            <Button
              variant="contained"
              startIcon={<CheckIcon />}
              onClick={handleSubmit}
              sx={{
                px: 3,
                py: 1,
                textTransform: 'none',
                fontWeight: 600,
                background: 'linear-gradient(180deg, #4fa4ff 0%, #2d6aff 100%)',
                boxShadow: '0 8px 24px rgba(77, 164, 255, 0.35)',
                '&:hover': {
                  background: 'linear-gradient(180deg, #5fb2ff 0%, #2f72ff 100%)'
                }
              }}
            >
              {t('layout_create_action')}
            </Button>
          </Box>
        </Box>
      </DialogContent>
    </Dialog>
  );
};

export default LayoutCreateDialog;
