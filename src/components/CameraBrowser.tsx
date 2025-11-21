import React, { useState, useEffect } from 'react';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  IconButton,
  Box,
  Typography,
  Toolbar,
  TextField,
  Button,
  CircularProgress,
  Alert
} from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import RefreshIcon from '@mui/icons-material/Refresh';
import OpenInNewIcon from '@mui/icons-material/OpenInNew';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
import HomeIcon from '@mui/icons-material/Home';
import { useLocalization } from '../hooks/useLocalization';
import type { Camera } from '../types';

interface CameraBrowserProps {
  camera: Camera | null;
  open: boolean;
  onClose: () => void;
}

const CameraBrowser: React.FC<CameraBrowserProps> = ({ camera, open, onClose }) => {
  const { t } = useLocalization();
  const [currentUrl, setCurrentUrl] = useState<string>('');
  const [inputUrl, setInputUrl] = useState<string>('');
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string>('');
  const [iframeKey, setIframeKey] = useState<number>(0);

  // Инициализация URL при открытии
  useEffect(() => {
    if (camera && open) {
      // Для веб-интерфейса используем стандартный HTTP порт 80, игнорируем RTSP порт
      const baseUrl = `http://${camera.ip}`;
      setCurrentUrl(baseUrl);
      setInputUrl(baseUrl);
      setIframeKey(prev => prev + 1);
      setError('');
    }
  }, [camera, open]);

  const handleRefresh = () => {
    setLoading(true);
    setError('');
    setIframeKey(prev => prev + 1);
  };

  const handleNavigate = () => {
    if (inputUrl) {
      setCurrentUrl(inputUrl);
      setIframeKey(prev => prev + 1);
      setError('');
    }
  };

  const handleOpenExternal = () => {
    if (currentUrl) {
      window.open(currentUrl, '_blank');
    }
  };

  const handleBack = () => {
    // Простая реализация "назад" - возврат к базовому URL
    if (camera) {
      const baseUrl = `http://${camera.ip}`; // Используем только IP без порта
      setCurrentUrl(baseUrl);
      setInputUrl(baseUrl);
      setIframeKey(prev => prev + 1);
    }
  };

  const handleIframeLoad = () => {
    setLoading(false);
  };

  const handleIframeError = () => {
    setLoading(false);
    setError(`Не удалось загрузить страницу: ${currentUrl}`);
  };

  if (!camera) return null;

  return (
    <Dialog
      open={open}
      onClose={onClose}
      maxWidth="lg"
      fullWidth
      PaperProps={{
        sx: {
          height: '90vh',
          bgcolor: '#2a2f33',
          color: '#fff',
        }
      }}
    >
      <DialogTitle sx={{ m: 0, p: 2, bgcolor: '#1e2125', color: '#fff' }}>
        <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
          <Typography variant="h6" component="div">
            {camera.name} - {t('browser_web_interface')}
          </Typography>
          <IconButton
            aria-label="close"
            onClick={onClose}
            sx={{ color: '#fff' }}
          >
            <CloseIcon />
          </IconButton>
        </Box>
      </DialogTitle>

      {/* Панель навигации */}
      <Toolbar sx={{ bgcolor: '#23272b', borderBottom: '1px solid #3b4146' }}>
        <IconButton onClick={handleBack} sx={{ color: '#fff', mr: 1 }}>
          <ArrowBackIcon />
        </IconButton>
        <IconButton onClick={handleRefresh} sx={{ color: '#fff', mr: 1 }}>
          <RefreshIcon />
        </IconButton>
        <IconButton onClick={() => {
          if (camera) {
            const baseUrl = `http://${camera.ip}`; // Используем только IP без порта
            setCurrentUrl(baseUrl);
            setInputUrl(baseUrl);
            setIframeKey(prev => prev + 1);
          }
        }} sx={{ color: '#fff', mr: 2 }}>
          <HomeIcon />
        </IconButton>
        
        <TextField
          value={inputUrl}
          onChange={(e) => setInputUrl(e.target.value)}
          onKeyPress={(e) => {
            if (e.key === 'Enter') {
              handleNavigate();
            }
          }}
          size="small"
          fullWidth
          sx={{
            mr: 1,
            '& .MuiOutlinedInput-root': {
              bgcolor: '#3b4146',
              color: '#fff',
              '& fieldset': {
                borderColor: '#50545a',
              },
              '&:hover fieldset': {
                borderColor: '#1976d2',
              },
              '&.Mui-focused fieldset': {
                borderColor: '#1976d2',
              },
            },
            '& .MuiInputBase-input': {
              color: '#fff',
            },
          }}
          placeholder={t('browser_enter_url')}
        />
        
        <Button
          onClick={handleNavigate}
          variant="contained"
          size="small"
          sx={{ mr: 1, minWidth: 'auto' }}
        >
          {t('browser_navigate')}
        </Button>
        
        <IconButton onClick={handleOpenExternal} sx={{ color: '#fff' }}>
          <OpenInNewIcon />
        </IconButton>
      </Toolbar>

      <DialogContent sx={{ p: 0, position: 'relative', height: '100%', bgcolor: '#2a2f33' }}>
        {/* Индикатор загрузки */}
        {loading && (
          <Box
            sx={{
              position: 'absolute',
              top: '50%',
              left: '50%',
              transform: 'translate(-50%, -50%)',
              zIndex: 1000,
            }}
          >
            <CircularProgress />
          </Box>
        )}

        {/* Сообщение об ошибке */}
        {error && (
          <Box sx={{ p: 2 }}>
            <Alert 
              severity="error" 
              sx={{ 
                bgcolor: '#d32f2f', 
                color: '#fff',
                '& .MuiAlert-icon': {
                  color: '#fff'
                }
              }}
            >
              {error}
              <Box sx={{ mt: 1 }}>
                <Button 
                  onClick={handleRefresh} 
                  size="small" 
                  sx={{ color: '#fff', borderColor: '#fff' }}
                  variant="outlined"
                >
                  {t('browser_try_again')}
                </Button>
              </Box>
            </Alert>
          </Box>
        )}

        {/* Iframe для отображения веб-страницы камеры */}
        {currentUrl && !error && (
          <iframe
            key={iframeKey}
            src={currentUrl}
            style={{
              width: '100%',
              height: '100%',
              border: 'none',
              backgroundColor: '#ffffff',
            }}
            title={`${camera.name} Web Interface`}
            onLoad={handleIframeLoad}
            onError={handleIframeError}
            sandbox="allow-same-origin allow-scripts allow-forms allow-popups allow-pointer-lock"
          />
        )}
      </DialogContent>
    </Dialog>
  );
};

export default CameraBrowser;