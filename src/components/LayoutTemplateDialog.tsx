import React, { useEffect, useMemo, useState } from 'react';
import {
  Box,
  Button,
  Dialog,
  DialogContent,
  IconButton,
  TextField,
  Tooltip,
  Typography
} from '@mui/material';
import {
  Close as CloseIcon,
  Save as SaveIcon
} from '@mui/icons-material';
import { useLocalization } from '../contexts/LocalizationContext';

interface LayoutTemplateDialogProps {
  open: boolean;
  mode: 'create' | 'manage';
  layoutName: string;
  gridSize: number;
  availableGridSizes: number[];
  onClose: () => void;
  onSave: (name: string) => void;
  onGridSelect: (gridSize: number) => void;
}

const calculatePreviewGrid = (cellCount: number) => {
  if (cellCount <= 0) {
    return { cols: 1, rows: 1 };
  }

  if (cellCount === 1) return { cols: 1, rows: 1 };
  if (cellCount === 4) return { cols: 2, rows: 2 };
  if (cellCount === 6) return { cols: 3, rows: 2 };
  if (cellCount === 8) return { cols: 4, rows: 2 };
  if (cellCount === 9) return { cols: 3, rows: 3 };
  if (cellCount === 12) return { cols: 4, rows: 3 };
  if (cellCount === 16) return { cols: 4, rows: 4 };
  if (cellCount === 20) return { cols: 5, rows: 4 };
  if (cellCount === 25) return { cols: 5, rows: 5 };
  if (cellCount === 32) return { cols: 8, rows: 4 };
  if (cellCount === 36) return { cols: 6, rows: 6 };
  if (cellCount === 49) return { cols: 7, rows: 7 };
  if (cellCount === 64) return { cols: 8, rows: 8 };

  const sqrt = Math.sqrt(cellCount);
  const cols = Math.ceil(sqrt);
  const rows = Math.ceil(cellCount / cols);
  return { cols, rows };
};

const LayoutTemplateDialog: React.FC<LayoutTemplateDialogProps> = ({
  open,
  mode,
  layoutName,
  gridSize,
  availableGridSizes,
  onClose,
  onSave,
  onGridSelect
}) => {
  const { t } = useLocalization();
  const [draftName, setDraftName] = useState(layoutName);
  const [touched, setTouched] = useState(false);
  const [selectedGrid, setSelectedGrid] = useState(gridSize);

  const gridOptions = useMemo(() => {
    const allowed = [1, 4, 9, 16, 32, 64];
    const combined = new Set<number>(allowed);
    availableGridSizes.forEach((size) => {
      if (allowed.includes(size)) {
        combined.add(size);
      }
    });
    combined.add(gridSize);
    return Array.from(combined).sort((a, b) => a - b);
  }, [availableGridSizes, gridSize]);

  useEffect(() => {
    if (open) {
      setDraftName(layoutName);
      setSelectedGrid(gridSize);
      setTouched(false);
    }
  }, [open, layoutName, gridSize]);

  const showError = touched && !draftName.trim();

  const headerSubtitle = useMemo(() => (
    mode === 'create'
      ? t('layout_editor_mode_create')
      : t('layout_editor_mode_manage')
  ), [mode, t]);

  const handleSave = () => {
    if (!draftName.trim()) {
      setTouched(true);
      return;
    }
    onSave(draftName.trim());
  };

  const handleGridClick = (size: number) => {
    setSelectedGrid(size);
    onGridSelect(size);
  };

  const handleKeyDown: React.KeyboardEventHandler<HTMLInputElement> = (event) => {
    if (event.key === 'Enter') {
      event.preventDefault();
      handleSave();
    }
  };

  return (
    <Dialog
      open={open}
      onClose={onClose}
      PaperProps={{
        sx: {
          width: 380,
          maxWidth: '85vw',
          background: 'linear-gradient(135deg, #2b3442 0%, #1b2029 100%)',
          borderRadius: 1.2,
          border: '1px solid rgba(99, 156, 221, 0.35)',
          boxShadow: '0 20px 36px rgba(0, 0, 0, 0.55)'
        }
      }}
    >
      <Box
        sx={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          px: 1.75,
          py: 1.25,
          borderBottom: '1px solid rgba(99, 156, 221, 0.35)',
          bgcolor: 'rgba(19, 24, 32, 0.9)'
        }}
      >
        <Box>
          <Typography variant="subtitle2" sx={{ color: 'rgba(150, 190, 230, 0.7)', letterSpacing: 1, textTransform: 'uppercase' }}>
            {headerSubtitle}
          </Typography>
          <Typography variant="h6" sx={{ color: '#f0f6ff', fontWeight: 600 }}>
            {t('layout_editor_title')}
          </Typography>
        </Box>
        <IconButton
          onClick={onClose}
          size="small"
          sx={{
            color: 'rgba(255,255,255,0.65)',
            bgcolor: 'rgba(255, 255, 255, 0.06)',
            '&:hover': {
              color: '#fff',
              bgcolor: 'rgba(255, 255, 255, 0.14)'
            }
          }}
        >
          <CloseIcon fontSize="small" />
        </IconButton>
      </Box>

      <DialogContent sx={{ px: 1.75, py: 2, display: 'flex', flexDirection: 'column', gap: 1.75 }}>
        <Box>
          <Typography variant="body2" sx={{ color: 'rgba(195, 210, 230, 0.6)', mb: 1.25, fontSize: '0.8rem' }}>
            {t('layout_editor_hint')}
          </Typography>
          <TextField
            fullWidth
            value={draftName}
            onChange={(event) => setDraftName(event.target.value)}
            onBlur={() => setTouched(true)}
            onKeyDown={handleKeyDown}
            variant="filled"
            label={t('layout_editor_name_label')}
            placeholder={t('layout_editor_name_placeholder') || ''}
            autoFocus
            error={showError}
            helperText={showError ? t('layout_name_required') : ' '}
            InputProps={{
              sx: {
                bgcolor: 'rgba(15, 22, 32, 0.85)',
                borderRadius: 1,
                input: {
                  color: '#f5f9ff',
                  fontWeight: 500,
                  fontSize: '0.95rem'
                }
              }
            }}
            InputLabelProps={{
              sx: {
                color: 'rgba(165, 190, 220, 0.7)',
                '&.Mui-focused': {
                  color: '#7fc1ff'
                }
              }
            }}
          />
        </Box>

        <Box>
          <Typography variant="subtitle2" sx={{ color: 'rgba(170, 195, 225, 0.7)', mb: 0.75, fontSize: '0.82rem' }}>
            {t('layout_editor_layouts_label')}
          </Typography>
          <Box
            sx={{
              display: 'grid',
              gridTemplateColumns: 'repeat(auto-fit, minmax(72px, 1fr))',
              gap: 1
            }}
          >
            {gridOptions.map((size) => {
              const { cols, rows } = calculatePreviewGrid(size);
              const totalCells = cols * rows;
              const cells = Array.from({ length: totalCells });
              const isSelected = selectedGrid === size;

              return (
                <Tooltip key={`grid-option-${size}`} title={t('layout_editor_grid_option', { count: size })}>
                  <Box
                    component="button"
                    type="button"
                    onClick={() => handleGridClick(size)}
                    sx={{
                      all: 'unset',
                      cursor: 'pointer',
                      display: 'flex',
                      flexDirection: 'column',
                      alignItems: 'center',
                      gap: 0.35,
                      borderRadius: 1,
                      border: isSelected ? '2px solid rgba(127, 193, 255, 0.95)' : '1px solid rgba(140,160,190,0.35)',
                      boxShadow: isSelected ? '0 0 12px rgba(127, 193, 255, 0.35)' : '0 6px 16px rgba(0,0,0,0.25)',
                      background: isSelected ? 'linear-gradient(180deg, rgba(85,150,235,0.28) 0%, rgba(32,62,110,0.28) 100%)' : 'rgba(18, 24, 34, 0.85)',
                      padding: 0.65,
                      transition: 'all 140ms ease'
                    }}
                  >
                    <Box
                      sx={{
                        width: '100%',
                        aspectRatio: '4 / 3',
                        display: 'grid',
                        gridTemplateColumns: `repeat(${cols}, 1fr)`,
                        gridTemplateRows: `repeat(${rows}, 1fr)`,
                        gap: 0.3
                      }}
                    >
                      {cells.map((_, idx) => (
                        <Box
                          // eslint-disable-next-line react/no-array-index-key
                          key={`grid-${size}-${idx}`}
                          sx={{
                            borderRadius: 0.6,
                            border: '1px solid rgba(140,160,190,0.35)',
                            bgcolor: 'rgba(200, 215, 240, 0.08)'
                          }}
                        />
                      ))}
                    </Box>
                    <Typography variant="caption" sx={{ color: 'rgba(200, 215, 235, 0.8)', fontWeight: isSelected ? 600 : 500, fontSize: '0.72rem' }}>
                      {t('layout_editor_cells_label', { count: size })}
                    </Typography>
                  </Box>
                </Tooltip>
              );
            })}
          </Box>
        </Box>
      </DialogContent>

      <Box
        sx={{
          display: 'flex',
          justifyContent: 'flex-end',
          gap: 1.25,
          px: 1.75,
          pb: 2
        }}
      >
        <Button onClick={onClose} color="inherit" sx={{ color: 'rgba(200,210,225,0.8)' }}>
          {t('layout_editor_cancel')}
        </Button>
        <Button
          variant="contained"
          startIcon={<SaveIcon />}
          onClick={handleSave}
          sx={{
            px: 2,
            py: 0.85,
            textTransform: 'none',
            fontWeight: 600,
            background: 'linear-gradient(180deg, #4fa4ff 0%, #2d6aff 100%)',
            boxShadow: '0 10px 24px rgba(79, 164, 255, 0.35)',
            '&:hover': {
              background: 'linear-gradient(180deg, #5fb2ff 0%, #2f72ff 100%)'
            }
          }}
        >
          {t('layout_editor_save')}
        </Button>
      </Box>
    </Dialog>
  );
};

export default LayoutTemplateDialog;
