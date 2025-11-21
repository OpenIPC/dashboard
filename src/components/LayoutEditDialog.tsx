import React, { useEffect, useMemo, useState } from 'react';
import {
  Box,
  Button,
  Dialog,
  DialogContent,
  Divider,
  IconButton,
  InputAdornment,
  List,
  ListItemButton,
  ListItemText,
  TextField,
  Tooltip,
  Typography
} from '@mui/material';
import {
  Close as CloseIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  CheckCircle as CheckCircleIcon,
  RadioButtonUnchecked as RadioButtonUncheckedIcon,
  Save as SaveIcon
} from '@mui/icons-material';
import { useLocalization } from '../hooks/useLocalization';
import type { LayoutTab } from '../types';

interface LayoutEditDialogProps {
  open: boolean;
  layouts: LayoutTab[];
  activeLayoutId: string | null;
  onClose: () => void;
  onRename: (layoutId: string, nextName: string) => void;
  onDelete: (layoutId: string) => void;
  onActivate: (layoutId: string) => void;
}

const LayoutEditDialog: React.FC<LayoutEditDialogProps> = ({
  open,
  layouts,
  activeLayoutId,
  onClose,
  onRename,
  onDelete,
  onActivate
}) => {
  const { t } = useLocalization();
  const [selectedId, setSelectedId] = useState<string | null>(null);
  const [draftName, setDraftName] = useState('');
  const [touched, setTouched] = useState(false);

  const sortedLayouts = useMemo(() => (
    [...layouts].sort((a, b) => a.name.localeCompare(b.name, undefined, { sensitivity: 'base' }))
  ), [layouts]);

  useEffect(() => {
    if (open) {
      const initialId = activeLayoutId || sortedLayouts[0]?.id || null;
      setSelectedId(initialId);
      const initialName = sortedLayouts.find(l => l.id === initialId)?.name || '';
      setDraftName(initialName);
      setTouched(false);
    }
  }, [open, activeLayoutId, sortedLayouts]);

  const handleSelect = (layoutId: string) => {
    setSelectedId(layoutId);
    const layout = layouts.find(l => l.id === layoutId);
    setDraftName(layout?.name || '');
    setTouched(false);
  };

  const handleRename = () => {
    const trimmed = draftName.trim();
    if (!selectedId || !trimmed) {
      setTouched(true);
      return;
    }
    onRename(selectedId, trimmed);
    setTouched(false);
  };

  const handleDelete = () => {
    if (!selectedId) return;
    if (!confirm(t('layout_manage_delete_confirm'))) {
      return;
    }
    onDelete(selectedId);
  };

  const handleActivate = () => {
    if (!selectedId) return;
    onActivate(selectedId);
  };

  const selectedLayout = layouts.find(layout => layout.id === selectedId) || null;
  const showError = touched && !draftName.trim();

  return (
    <Dialog
      open={open}
      onClose={onClose}
      maxWidth="md"
      fullWidth
      PaperProps={{
        sx: {
          background: 'linear-gradient(135deg, #1f262f 0%, #12161d 100%)',
          border: '1px solid rgba(69, 126, 209, 0.45)',
          boxShadow: '0 18px 48px rgba(0,0,0,0.55)',
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
          borderBottom: '1px solid rgba(69, 126, 209, 0.35)',
          bgcolor: 'rgba(23, 30, 41, 0.9)'
        }}
      >
        <Box>
          <Typography variant="subtitle2" sx={{ color: 'rgba(160, 190, 230, 0.75)', textTransform: 'uppercase', letterSpacing: 1 }}>
            {t('layout_manage_title')}
          </Typography>
          <Typography variant="h6" sx={{ color: '#f0f6ff', fontWeight: 600 }}>
            {t('layout_manage_subtitle')}
          </Typography>
        </Box>
        <IconButton
          onClick={onClose}
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

      <DialogContent sx={{ px: 0, py: 0 }}>
        {layouts.length === 0 ? (
          <Box sx={{ py: 8, textAlign: 'center' }}>
            <Typography variant="body1" sx={{ color: 'rgba(190, 205, 225, 0.75)' }}>
              {t('layout_manage_empty')}
            </Typography>
          </Box>
        ) : (
          <Box sx={{ display: 'flex', minHeight: 400 }}>
            <Box sx={{ width: '40%', borderRight: '1px solid rgba(69, 126, 209, 0.25)', bgcolor: 'rgba(15,20,29,0.65)' }}>
              <Box sx={{ px: 3, py: 2 }}>
                <Typography variant="body2" sx={{ color: 'rgba(170, 195, 220, 0.65)', mb: 2 }}>
                  {t('layout_manage_list_label')}
                </Typography>
                <List disablePadding sx={{ overflowY: 'auto', maxHeight: 420 }}>
                  {sortedLayouts.map(layout => {
                    const isActive = layout.id === activeLayoutId;
                    const isSelected = layout.id === selectedId;
                    return (
                      <ListItemButton
                        key={layout.id}
                        selected={isSelected}
                        onClick={() => handleSelect(layout.id)}
                        sx={{
                          mb: 0.5,
                          borderRadius: 1,
                          bgcolor: isSelected ? 'rgba(64, 120, 200, 0.35)' : 'transparent',
                          '&:hover': {
                            bgcolor: 'rgba(64, 120, 200, 0.25)'
                          }
                        }}
                      >
                        <ListItemText
                          primary={layout.name}
                          primaryTypographyProps={{
                            variant: 'body2',
                            sx: {
                              color: '#f5f7fb',
                              fontWeight: isSelected ? 600 : 500
                            }
                          }}
                          secondary={new Date(layout.template.updatedAt).toLocaleString()}
                          secondaryTypographyProps={{
                            variant: 'caption',
                            sx: { color: 'rgba(200, 212, 230, 0.45)' }
                          }}
                        />
                        <Tooltip title={isActive ? t('layout_manage_active') : t('layout_manage_make_active')}>
                          <Box sx={{ display: 'flex', alignItems: 'center', color: isActive ? '#6bdc79' : 'rgba(160,180,205,0.5)' }}>
                            {isActive ? <CheckCircleIcon fontSize="small" /> : <RadioButtonUncheckedIcon fontSize="small" />}
                          </Box>
                        </Tooltip>
                      </ListItemButton>
                    );
                  })}
                </List>
              </Box>
            </Box>

            <Box sx={{ flex: 1, px: 4, py: 4 }}>
              {selectedLayout ? (
                <Box sx={{ display: 'flex', flexDirection: 'column', gap: 3 }}>
                  <Box>
                    <Typography variant="subtitle2" sx={{ color: 'rgba(160, 190, 230, 0.7)', letterSpacing: 0.5, mb: 1 }}>
                      {t('layout_manage_details_title')}
                    </Typography>
                    <TextField
                      value={draftName}
                      onChange={(event) => setDraftName(event.target.value)}
                      onBlur={() => setTouched(true)}
                      label={t('layout_manage_name_label')}
                      fullWidth
                      variant="filled"
                      error={showError}
                      helperText={showError ? t('layout_name_required') : ' '}
                      InputProps={{
                        startAdornment: (
                          <InputAdornment position="start">
                            <EditIcon fontSize="small" sx={{ color: 'rgba(140,170,210,0.6)' }} />
                          </InputAdornment>
                        ),
                        sx: {
                          bgcolor: 'rgba(20, 27, 36, 0.9)',
                          borderRadius: 1,
                          input: {
                            color: '#f3f8ff',
                            fontWeight: 500
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

                  <Box>
                    <Typography variant="body2" sx={{ color: 'rgba(190, 205, 225, 0.6)' }}>
                      {t('layout_manage_meta_cells', { count: selectedLayout.template.gridSize })}
                    </Typography>
                    <Typography variant="body2" sx={{ color: 'rgba(190, 205, 225, 0.6)' }}>
                      {t('layout_manage_meta_cameras', { count: selectedLayout.template.cameraAssignments.length })}
                    </Typography>
                    <Typography variant="caption" sx={{ color: 'rgba(180, 195, 215, 0.45)' }}>
                      {t('layout_manage_meta_updated', { value: new Date(selectedLayout.template.updatedAt).toLocaleString() })}
                    </Typography>
                  </Box>

                  <Divider sx={{ borderColor: 'rgba(69, 126, 209, 0.25)' }} />

                  <Box sx={{ display: 'flex', justifyContent: 'flex-end', gap: 2 }}>
                    <Tooltip title={t('layout_manage_make_active')}>
                      <span>
                        <Button
                          variant="outlined"
                          onClick={handleActivate}
                          disabled={selectedLayout.id === activeLayoutId}
                          startIcon={<CheckCircleIcon />}
                          sx={{
                            borderColor: 'rgba(69, 126, 209, 0.45)',
                            color: '#8fb7ff',
                            '&:hover': {
                              borderColor: 'rgba(105, 156, 235, 0.85)',
                              background: 'rgba(79, 132, 220, 0.12)'
                            }
                          }}
                        >
                          {t('layout_manage_apply')}
                        </Button>
                      </span>
                    </Tooltip>
                    <Button
                      variant="contained"
                      color="primary"
                      startIcon={<SaveIcon />}
                      onClick={handleRename}
                      sx={{
                        textTransform: 'none',
                        fontWeight: 600,
                        background: 'linear-gradient(180deg, #4fa4ff 0%, #2d6aff 100%)',
                        '&:hover': {
                          background: 'linear-gradient(180deg, #5fb2ff 0%, #2f72ff 100%)'
                        }
                      }}
                    >
                      {t('layout_manage_save')}
                    </Button>
                    <Tooltip title={t('layout_manage_delete')}>
                      <span>
                        <Button
                          variant="outlined"
                          color="error"
                          startIcon={<DeleteIcon />}
                          onClick={handleDelete}
                          disabled={layouts.length <= 1}
                          sx={{
                            textTransform: 'none',
                            borderColor: 'rgba(255, 96, 96, 0.45)',
                            color: 'rgba(255, 128, 128, 0.85)',
                            '&:hover': {
                              borderColor: 'rgba(255, 128, 128, 0.85)',
                              background: 'rgba(255, 100, 100, 0.12)'
                            }
                          }}
                        >
                          {t('layout_manage_delete')}
                        </Button>
                      </span>
                    </Tooltip>
                  </Box>
                </Box>
              ) : (
                <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'center', height: '100%' }}>
                  <Typography variant="body2" sx={{ color: 'rgba(190, 205, 225, 0.6)' }}>
                    {t('layout_manage_select_prompt')}
                  </Typography>
                </Box>
              )}
            </Box>
          </Box>
        )}
      </DialogContent>
    </Dialog>
  );
};

export default LayoutEditDialog;
