import React, { useState, useEffect } from 'react';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Box,
  Button,
  TextField,
  Typography,
  Chip,
  Card,
  CardContent,
  CardActions,
  Alert,
  IconButton
} from '@mui/material';
import {
  Close as CloseIcon,
  Delete as DeleteIcon,
  Save as SaveIcon,
  Visibility as PreviewIcon,
  Add as AddIcon
} from '@mui/icons-material';
import { useLocalization } from '../contexts/LocalizationContext';
import type { LayoutTemplatePreview, Camera } from '../types';

interface LayoutManagerDialogProps {
  open: boolean;
  onClose: () => void;
  onSaveTemplate: (name: string, description?: string) => Promise<void>;
  onLoadTemplate: (templateId: string) => Promise<void>;
  onDeleteTemplate: (templateId: string) => Promise<void>;
  savedTemplates: LayoutTemplatePreview[];
  currentGridSize: number;
  currentCameraAssignments: { cellIndex: number; camera: Camera | null }[];
}

const LayoutManagerDialog: React.FC<LayoutManagerDialogProps> = ({
  open,
  onClose,
  onSaveTemplate,
  onLoadTemplate,
  onDeleteTemplate,
  savedTemplates,
  currentGridSize,
  currentCameraAssignments
}) => {
  const { t } = useLocalization();
  const [activeTab, setActiveTab] = useState<'save' | 'load'>('save');
  const [templateName, setTemplateName] = useState('');
  const [templateDescription, setTemplateDescription] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  // Сброс состояния при открытии диалога
  useEffect(() => {
    if (open) {
      setTemplateName('');
      setTemplateDescription('');
      setError(null);
      setActiveTab('save');
    }
  }, [open]);

  // Подсчет активных камер в текущей раскладке
  const activeCameraCount = currentCameraAssignments.filter(assignment => assignment.camera !== null).length;
  const activeCameraNames = currentCameraAssignments
    .filter(assignment => assignment.camera !== null)
    .slice(0, 3)
    .map(assignment => assignment.camera!.name);

  const handleSaveTemplate = async () => {
    if (!templateName.trim()) {
      setError(t('template_name_required'));
      return;
    }

    setIsLoading(true);
    setError(null);

    try {
      await onSaveTemplate(templateName.trim(), templateDescription.trim() || undefined);
      setTemplateName('');
      setTemplateDescription('');
      onClose();
    } catch (err) {
      setError(err instanceof Error ? err.message : t('save_template_error'));
    } finally {
      setIsLoading(false);
    }
  };

  const handleLoadTemplate = async (templateId: string) => {
    setIsLoading(true);
    setError(null);

    try {
      await onLoadTemplate(templateId);
      onClose();
    } catch (err) {
      setError(err instanceof Error ? err.message : t('load_template_error'));
    } finally {
      setIsLoading(false);
    }
  };

  const handleDeleteTemplate = async (templateId: string) => {
    if (!confirm(t('confirm_delete_template'))) {
      return;
    }

    setIsLoading(true);
    setError(null);

    try {
      await onDeleteTemplate(templateId);
    } catch (err) {
      setError(err instanceof Error ? err.message : t('delete_template_error'));
    } finally {
      setIsLoading(false);
    }
  };

  const renderSaveTab = () => (
    <Box sx={{ p: 2 }}>
      <Typography variant="h6" gutterBottom>
        {t('save_current_layout')}
      </Typography>
      
      {/* Превью текущей раскладки */}
      <Card sx={{ mb: 3, bgcolor: 'rgba(25, 118, 210, 0.05)' }}>
        <CardContent>
          <Typography variant="subtitle2" color="primary" gutterBottom>
            {t('current_layout_preview')}
          </Typography>
          <Box sx={{ display: 'flex', gap: 1, mb: 1 }}>
            <Chip 
              size="small" 
              label={`${currentGridSize} ${t('cells')}`} 
              color="primary" 
              variant="outlined" 
            />
            <Chip 
              size="small" 
              label={`${activeCameraCount} ${t('cameras')}`} 
              color="secondary" 
              variant="outlined" 
            />
          </Box>
          {activeCameraNames.length > 0 && (
            <Typography variant="body2" color="text.secondary">
              {t('cameras')}: {activeCameraNames.join(', ')}
              {activeCameraCount > 3 && ` +${activeCameraCount - 3} ${t('more')}`}
            </Typography>
          )}
        </CardContent>
      </Card>

      {/* Форма сохранения */}
      <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
        <TextField
          label={t('template_name')}
          value={templateName}
          onChange={(e) => setTemplateName(e.target.value)}
          required
          fullWidth
          error={!!error && !templateName.trim()}
          helperText={!templateName.trim() ? t('template_name_required') : ''}
        />
        
        <TextField
          label={t('template_description')}
          value={templateDescription}
          onChange={(e) => setTemplateDescription(e.target.value)}
          multiline
          rows={2}
          fullWidth
          placeholder={t('template_description_placeholder')}
        />
      </Box>

      {error && (
        <Alert severity="error" sx={{ mt: 2 }}>
          {error}
        </Alert>
      )}
    </Box>
  );

  const renderLoadTab = () => (
    <Box sx={{ p: 2 }}>
      <Typography variant="h6" gutterBottom>
        {t('load_saved_layout')}
      </Typography>

      {savedTemplates.length === 0 ? (
        <Box sx={{ textAlign: 'center', py: 4 }}>
          <Typography variant="body1" color="text.secondary">
            {t('no_saved_templates')}
          </Typography>
          <Button 
            startIcon={<AddIcon />}
            onClick={() => setActiveTab('save')}
            sx={{ mt: 2 }}
          >
            {t('save_first_template')}
          </Button>
        </Box>
      ) : (
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
          {savedTemplates.map((template) => (
            <Card key={template.id}>
                <CardContent>
                  <Typography variant="subtitle1" gutterBottom>
                    {template.name}
                  </Typography>
                  {template.description && (
                    <Typography variant="body2" color="text.secondary" gutterBottom>
                      {template.description}
                    </Typography>
                  )}
                  <Box sx={{ display: 'flex', gap: 1, mt: 1 }}>
                    <Chip size="small" label={`${template.gridSize} ${t('cells')}`} />
                    <Chip size="small" label={`${template.cameraCount} ${t('cameras')}`} />
                  </Box>
                  {template.previewCameras.length > 0 && (
                    <Typography variant="caption" color="text.secondary" sx={{ mt: 1, display: 'block' }}>
                      {template.previewCameras.join(', ')}
                    </Typography>
                  )}
                  <Typography variant="caption" color="text.secondary" sx={{ mt: 1, display: 'block' }}>
                    {new Date(template.createdAt).toLocaleDateString()}
                  </Typography>
                </CardContent>
                <CardActions>
                  <Button 
                    size="small" 
                    startIcon={<PreviewIcon />}
                    onClick={() => handleLoadTemplate(template.id)}
                    disabled={isLoading}
                  >
                    {t('load')}
                  </Button>
                  <IconButton 
                    size="small" 
                    onClick={() => handleDeleteTemplate(template.id)}
                    disabled={isLoading}
                  >
                    <DeleteIcon />
                  </IconButton>
                </CardActions>
              </Card>
            ))}
        </Box>
      )}

      {error && (
        <Alert severity="error" sx={{ mt: 2 }}>
          {error}
        </Alert>
      )}
    </Box>
  );

  return (
    <Dialog open={open} onClose={onClose} maxWidth="md" fullWidth>
      <DialogTitle>
        <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
          {t('layout_templates')}
          <IconButton onClick={onClose}>
            <CloseIcon />
          </IconButton>
        </Box>
      </DialogTitle>

      {/* Вкладки */}
      <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
        <Box sx={{ display: 'flex' }}>
          <Button
            onClick={() => setActiveTab('save')}
            sx={{ 
              borderRadius: 0, 
              borderBottom: activeTab === 'save' ? 2 : 0,
              borderColor: 'primary.main'
            }}
          >
            {t('save_layout')}
          </Button>
          <Button
            onClick={() => setActiveTab('load')}
            sx={{ 
              borderRadius: 0, 
              borderBottom: activeTab === 'load' ? 2 : 0,
              borderColor: 'primary.main'
            }}
          >
            {t('load_layout')} ({savedTemplates.length})
          </Button>
        </Box>
      </Box>

      <DialogContent sx={{ p: 0, minHeight: 400 }}>
        {activeTab === 'save' ? renderSaveTab() : renderLoadTab()}
      </DialogContent>

      {activeTab === 'save' && (
        <DialogActions>
          <Button onClick={onClose}>
            {t('cancel')}
          </Button>
          <Button 
            onClick={handleSaveTemplate}
            variant="contained"
            startIcon={<SaveIcon />}
            disabled={isLoading || !templateName.trim()}
          >
            {isLoading ? t('saving') : t('save_template')}
          </Button>
        </DialogActions>
      )}
    </Dialog>
  );
};

export default LayoutManagerDialog;