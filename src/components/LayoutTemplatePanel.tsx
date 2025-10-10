import React from 'react';
import {
  Box,
  Paper,
  IconButton,
  Typography,
  Stack,
  Chip,
  Button,
  Divider,
  List,
  ListItem,
  ListItemButton,
  ListItemText,
  Fade,
  TextField
} from '@mui/material';
import {
  Close as CloseIcon,
  Delete as DeleteIcon,
  GridView as GridViewIcon,
  PlaylistPlay as ManageIcon,
  Save as SaveIcon
} from '@mui/icons-material';
import { useLocalization } from '../contexts/LocalizationContext';
import type { LayoutTemplatePreview } from '../types';

const QUICK_LAYOUTS = [1, 4, 9, 16, 25, 32, 64];

interface LayoutTemplatePanelProps {
  open: boolean;
  onClose: () => void;
  templates: LayoutTemplatePreview[];
  currentGridSize: number;
  onSelectGrid: (size: number) => void;
  onSaveTemplate: (name: string) => void | Promise<void>;
  onManageTemplates: () => void;
  onLoadTemplate: (templateId: string) => Promise<void>;
  onDeleteTemplate: (templateId: string) => Promise<void>;
  draftName: string;
  onDraftNameChange: (value: string) => void;
  isSavingTemplate?: boolean;
}

const LayoutTemplatePanel: React.FC<LayoutTemplatePanelProps> = ({
  open,
  onClose,
  templates,
  currentGridSize,
  onSelectGrid,
  onSaveTemplate,
  onManageTemplates,
  onLoadTemplate,
  onDeleteTemplate,
  draftName,
  onDraftNameChange,
  isSavingTemplate = false
}) => {
  const { t } = useLocalization();

  const handleDeleteTemplate = async (templateId: string) => {
    const confirmed = window.confirm(t('confirm_delete_template'));
    if (confirmed) {
      try {
        await onDeleteTemplate(templateId);
      } catch (error) {
        console.error('Failed to delete layout template:', error);
      }
    }
  };

  if (!open) {
    return null;
  }

  return (
    <Fade in={open}>
      <Box
        sx={{
          position: 'absolute',
          top: 60,
          left: 16,
          zIndex: 40,
          pointerEvents: 'auto'
        }}
      >
        <Paper
          elevation={6}
          sx={{
            width: 320,
            bgcolor: 'rgba(32,35,40,0.98)',
            borderRadius: 2,
            border: '1px solid rgba(255,255,255,0.08)',
            boxShadow: '0 18px 40px rgba(0,0,0,0.45)',
            display: 'flex',
            flexDirection: 'column',
            maxHeight: 480
          }}
        >
          <Box sx={{ px: 2.5, py: 2, pb: 1.5, display: 'flex', alignItems: 'flex-start', justifyContent: 'space-between', gap: 1 }}>
            <Box>
              <Typography variant="subtitle1" sx={{ color: '#fff', fontWeight: 600 }}>
                {t('layout_templates_panel_title')}
              </Typography>
              <Typography variant="caption" sx={{ color: 'rgba(255,255,255,0.6)' }}>
                {t('layout_templates_panel_subtitle')}
              </Typography>
            </Box>
            <IconButton
              size="small"
              onClick={onClose}
              sx={{
                color: 'rgba(255,255,255,0.6)',
                '&:hover': {
                  color: '#fff',
                  bgcolor: 'rgba(255,255,255,0.1)'
                }
              }}
            >
              <CloseIcon fontSize="small" />
            </IconButton>
          </Box>

          <Box sx={{ px: 2.5, pb: 2 }}>
            <Typography variant="overline" sx={{ color: 'rgba(255,255,255,0.5)' }}>
              {t('layout_templates_quick_presets')}
            </Typography>
            <Stack direction="row" flexWrap="wrap" gap={1} sx={{ mt: 1 }}>
              {QUICK_LAYOUTS.map(size => (
                <Button
                  key={`quick-layout-${size}`}
                  size="small"
                  variant={currentGridSize === size ? 'contained' : 'outlined'}
                  onClick={() => onSelectGrid(size)}
                  startIcon={<GridViewIcon fontSize="inherit" />}
                  sx={{
                    minWidth: 84,
                    fontSize: '0.75rem',
                    px: 1.2,
                    borderColor: currentGridSize === size ? 'transparent' : 'rgba(255,255,255,0.12)',
                    color: currentGridSize === size ? '#fff' : 'rgba(255,255,255,0.75)',
                    bgcolor: currentGridSize === size ? 'primary.main' : 'transparent',
                    '&:hover': {
                      borderColor: 'rgba(255,255,255,0.3)',
                      bgcolor: currentGridSize === size ? 'primary.dark' : 'rgba(255,255,255,0.05)'
                    }
                  }}
                >
                  {size}
                </Button>
              ))}
            </Stack>
            <TextField
              fullWidth
              size="small"
              label={t('layout_templates_name_label')}
              placeholder={t('layout_templates_name_placeholder')}
              value={draftName}
              onChange={(event) => onDraftNameChange(event.target.value)}
              autoFocus
              sx={{
                mt: 2,
                '& .MuiOutlinedInput-root': {
                  backgroundColor: 'rgba(21,23,28,0.9)',
                  color: '#fff',
                  '& fieldset': {
                    borderColor: 'rgba(255,255,255,0.12)'
                  },
                  '&:hover fieldset': {
                    borderColor: 'rgba(255,255,255,0.3)'
                  },
                  '&.Mui-focused fieldset': {
                    borderColor: 'primary.main'
                  }
                },
                '& .MuiInputLabel-root': {
                  color: 'rgba(255,255,255,0.65)'
                },
                '& .MuiInputLabel-root.Mui-focused': {
                  color: '#fff'
                }
              }}
              inputProps={{
                maxLength: 60
              }}
            />
          </Box>

          <Divider sx={{ borderColor: 'rgba(255,255,255,0.08)' }} />

          <Box sx={{ flex: 1, overflowY: 'auto', px: 1.5 }}>
            <List dense disablePadding>
              {templates.length === 0 ? (
                <ListItem sx={{ py: 3, justifyContent: 'center' }}>
                  <Typography variant="body2" sx={{ color: 'rgba(255,255,255,0.5)', textAlign: 'center' }}>
                    {t('layout_templates_empty_state')}
                  </Typography>
                </ListItem>
              ) : (
                templates.map(template => (
                  <ListItem
                    key={template.id}
                    secondaryAction={
                      <IconButton
                        edge="end"
                        size="small"
                        onClick={() => { void handleDeleteTemplate(template.id); }}
                        sx={{
                          color: 'rgba(255,255,255,0.45)',
                          '&:hover': {
                            color: '#ff6f61',
                            bgcolor: 'rgba(255,111,97,0.08)'
                          }
                        }}
                      >
                        <DeleteIcon fontSize="inherit" />
                      </IconButton>
                    }
                    disablePadding
                    sx={{
                      mb: 0.5,
                      borderRadius: 1.5,
                      '&:hover': {
                        backgroundColor: 'rgba(255,255,255,0.08)'
                      }
                    }}
                  >
                    <ListItemButton
                      onClick={() => { void onLoadTemplate(template.id); }}
                      sx={{
                        alignItems: 'flex-start',
                        py: 1.2,
                        borderRadius: 1.5
                      }}
                    >
                      <Stack spacing={0.5} sx={{ flex: 1 }}>
                        <ListItemText
                          primary={
                            <Typography variant="subtitle2" sx={{ color: '#fff', fontWeight: 600 }}>
                              {template.name}
                            </Typography>
                          }
                          secondary={
                            template.description ? (
                              <Typography variant="caption" sx={{ color: 'rgba(255,255,255,0.6)' }}>
                                {template.description}
                              </Typography>
                            ) : undefined
                          }
                        />
                        <Stack direction="row" spacing={1} alignItems="center">
                          <Chip
                            size="small"
                            label={t('layout_templates_cells_label', { count: template.gridSize })}
                            sx={{
                              bgcolor: 'rgba(25,118,210,0.12)',
                              color: 'rgba(187, 222, 251, 0.95)'
                            }}
                          />
                          <Chip
                            size="small"
                            label={t('layout_templates_camera_label', { count: template.cameraCount })}
                            sx={{
                              bgcolor: 'rgba(129,199,132,0.14)',
                              color: 'rgba(200,230,201,0.95)'
                            }}
                          />
                        </Stack>
                        {template.previewCameras?.length > 0 && (
                          <Typography variant="caption" sx={{ color: 'rgba(255,255,255,0.45)' }}>
                            {template.previewCameras.join(', ')}
                          </Typography>
                        )}
                      </Stack>
                    </ListItemButton>
                  </ListItem>
                ))
              )}
            </List>
          </Box>

          <Divider sx={{ borderColor: 'rgba(255,255,255,0.08)' }} />

          <Stack direction="row" spacing={1} sx={{ p: 1.5 }}>
            <Button
              fullWidth
              variant="contained"
              startIcon={<SaveIcon fontSize="small" />}
              onClick={() => { void onSaveTemplate(draftName.trim()); }}
              disabled={!draftName.trim() || isSavingTemplate}
            >
              {isSavingTemplate ? t('saving') : t('layout_templates_save_button')}
            </Button>
            <Button
              fullWidth
              variant="outlined"
              startIcon={<ManageIcon fontSize="small" />}
              onClick={onManageTemplates}
              sx={{
                borderColor: 'rgba(255,255,255,0.12)',
                color: 'rgba(255,255,255,0.8)',
                '&:hover': {
                  borderColor: 'rgba(255,255,255,0.4)'
                }
              }}
            >
              {t('layout_templates_manage_button')}
            </Button>
          </Stack>
        </Paper>
      </Box>
    </Fade>
  );
};

export default LayoutTemplatePanel;