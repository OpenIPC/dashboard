import React, { useState, useEffect } from 'react';
import { List, ListItem, ListItemText, ListItemSecondaryAction, IconButton, Typography, Button, Paper, Box, CircularProgress } from '@mui/material';
import { PlayArrow, Stop, Add, Search, Delete, Videocam } from '@mui/icons-material';
import DirectRTSPPlayer from './DirectRTSPPlayer';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/api/event';
import AddCameraDialog from './AddCameraDialog';
import CameraSearchDialog from './CameraSearchDialog';

interface Camera {
  id: number;
  name: string;
  ip: string;
  port: number;
  user: string;
  pass_enc: string;
  path_hd: string;
  path_sd: string;
  status: string;
  protocol: string;
  onvifPort?: number;
}

const DirectCameras: React.FC = () => {
  const [cameras, setCameras] = useState<Camera[]>([]);
  const [addDialogOpen, setAddDialogOpen] = useState(false);
  const [searchDialogOpen, setSearchDialogOpen] = useState(false);
  const [foundCameras, setFoundCameras] = useState<{name: string; ip: string; protocol?: string}[]>([]);
  const [streamingCameraId, setStreamingCameraId] = useState<number | null>(null);
  const [isStreaming, setIsStreaming] = useState<boolean>(false);
  const [initialData, setInitialData] = useState<any>(null);
  const [streamUrl, setStreamUrl] = useState<string>('');
  const [selectedCamera, setSelectedCamera] = useState<Camera | null>(null);
  const [isConnecting, setIsConnecting] = useState<boolean>(false);
  const [connectionError, setConnectionError] = useState<string>('');

  useEffect(() => {
    // Загрузка камер при монтировании компонента
    loadCameras();
    
    // Подписка на событие device-found для поиска камер
    const unlisten = listen('device-found', (event) => {
      const camera = event.payload as { ip: string; name: string; protocol: string };
      setFoundCameras(prev => {
        if (prev.find(c => c.ip === camera.ip)) return prev;
        return [...prev, camera];
      });
    });

    return () => {
      // Очистка при размонтировании
      unlisten.then(unlistenFn => unlistenFn());
    };
  }, []);

  // Загрузка камер из бэкенда
  const loadCameras = async () => {
    try {
      const loaded = await invoke('load_cameras');
      setCameras(loaded as Camera[]);
    } catch (err) {
      console.error('Failed to load cameras:', err);
    }
  };

  // Функция для запуска поиска камер
  const startDiscovery = async () => {
    setFoundCameras([]);
    setSearchDialogOpen(true);
    try {
      await invoke('discover_cameras');
    } catch (err) {
      console.error('Failed to start camera discovery:', err);
    }
  };

  // Обработчик добавления камеры
  const handleAddCamera = async (data: any) => {
    try {
      const encryptedPass = await invoke('encrypt_password', { password: data.pass });
      const newCamera: Camera = {
        id: cameras.length ? Math.max(...cameras.map(c => c.id)) + 1 : 1,
        name: data.name,
        ip: data.ip,
        port: data.port,
        user: data.user,
        pass_enc: encryptedPass as string,
        path_hd: data.streamUrl || '',
        path_sd: data.streamUrl || '',
        status: 'offline',
        protocol: data.protocol || 'onvif',
        onvifPort: data.onvifPort || 80,
      };
      
      const updatedCameras = [...cameras, newCamera];
      setCameras(updatedCameras);
      await invoke('save_cameras', { cameras: updatedCameras });
      setAddDialogOpen(false);
    } catch (err) {
      console.error('Failed to add camera:', err);
    }
  };

  // Обработчик удаления камеры
  const handleDeleteCamera = async (ip: string) => {
    try {
      // Останавливаем стрим если он запущен для этой камеры
      const camera = cameras.find(c => c.ip === ip);
      if (camera && streamingCameraId === camera.id) {
        setStreamingCameraId(null);
        setIsStreaming(false);
        setStreamUrl('');
        setSelectedCamera(null);
      }
      
      // Удаляем камеру из бэкенда
      await invoke('remove_camera', { ip });
      
      // Обновляем состояние
      const updatedCameras = cameras.filter(c => c.ip !== ip);
      setCameras(updatedCameras);
      
      // Сохраняем обновленный список камер
      await invoke('save_cameras', { cameras: updatedCameras });
    } catch (err) {
      console.error('Failed to remove camera:', err);
    }
  };

  // Обработчик для прямого воспроизведения RTSP
  const handleDirectPlay = async (camera: Camera) => {
    try {
      setIsConnecting(true);
      setConnectionError('');
      
      // Получаем декодированный пароль
      const password = await invoke('decrypt_password', { encrypted: camera.pass_enc }) as string;
      
      // Если у камеры уже есть путь path_hd, используем его
      let rtspUrl = camera.path_hd;
      
      // Если пути нет или он не начинается с rtsp://, пробуем получить URL через ONVIF
      if (!rtspUrl || !rtspUrl.startsWith('rtsp://')) {
        console.log('Attempting to get RTSP URL via ONVIF');
        
        try {
          // Получаем профили ONVIF камеры
          const profiles = await invoke('get_onvif_profiles', {
            ip: camera.ip,
            port: camera.onvifPort || 80,
            user: camera.user,
            pass: password
          }) as any[];
          
          console.log('ONVIF profiles:', profiles);
          
          if (profiles && profiles.length > 0) {
            // Берем первый профиль по умолчанию
            const profileToken = profiles[0].token;
            
            // Получаем RTSP URL для этого профиля
            rtspUrl = await invoke('get_stream_uri', {
              ip: camera.ip,
              port: camera.onvifPort || 80,
              user: camera.user,
              pass: password,
              profileToken: profileToken
            }) as string;
            
            console.log('ONVIF RTSP URL:', rtspUrl);
          } else {
            throw new Error('No profiles found via ONVIF');
          }
        } catch (onvifError) {
          console.error('Failed to get RTSP URL via ONVIF:', onvifError);
          
          // Если ONVIF не сработал, пробуем создать URL стандартного формата
          // Encode username and password to handle special characters
          const encodedUser = encodeURIComponent(camera.user);
          const encodedPass = encodeURIComponent(password);
          rtspUrl = `rtsp://${encodedUser}:${encodedPass}@${camera.ip}:${camera.port || 554}/stream=0`;
          console.log('Created standard format RTSP URL (credentials masked)');
        }
      }
      
      console.log('Final RTSP URL for direct playback:', rtspUrl.replace(/:[^:@]+@/, ':****@'));
      
      // Используем функцию для очистки URL от проблем с форматированием
      const fixedUrl = await invoke('play_direct_rtsp', { sdp: rtspUrl }) as string;
      
      // Сохраняем информацию о выбранной камере и URL
      setSelectedCamera(camera);
      setStreamingCameraId(camera.id);
      setIsStreaming(true);
      setStreamUrl(fixedUrl);
      setIsConnecting(false);
      
    } catch (err) {
      console.error('Failed to start direct RTSP stream:', err);
      setConnectionError(typeof err === 'object' ? (err as Error).message : String(err));
      setIsConnecting(false);
      setIsStreaming(false);
      setStreamingCameraId(null);
      setSelectedCamera(null);
    }
  };

  // Обработчик для остановки стрима
  const handleStopStream = () => {
    // Просто сбрасываем состояние
    setStreamingCameraId(null);
    setIsStreaming(false);
    setStreamUrl('');
    setSelectedCamera(null);
    setConnectionError('');
  };

  return (
    <div>
      <Typography variant="h4" gutterBottom>
        Прямое подключение к камерам
      </Typography>
      <div style={{ marginBottom: '20px', display: 'flex', gap: '10px' }}>
        <Button 
          variant="contained" 
          startIcon={<Add />} 
          onClick={() => setAddDialogOpen(true)}
        >
          Добавить камеру
        </Button>
        <Button 
          variant="outlined" 
          startIcon={<Search />} 
          onClick={startDiscovery}
        >
          Поиск камер
        </Button>
      </div>
      
      <Box sx={{ display: 'flex', flexDirection: { xs: 'column', md: 'row' }, gap: 2 }}>
        {/* Левая часть - список камер */}
        <Box sx={{ flex: 1, width: '100%' }}>
          <List>
            {cameras.map((camera) => (
              <ListItem 
                key={camera.id} 
                sx={{
                  bgcolor: streamingCameraId === camera.id ? 'rgba(25, 118, 210, 0.1)' : 'transparent',
                  borderRadius: 1,
                  mb: 1,
                }}
              >
                <ListItemText 
                  primary={camera.name} 
                  secondary={`IP: ${camera.ip} | Порт: ${camera.port}`} 
                />
                <ListItemSecondaryAction>
                  <IconButton 
                    edge="end" 
                    aria-label="play" 
                    onClick={() => handleDirectPlay(camera)}
                    disabled={isStreaming && streamingCameraId === camera.id}
                  >
                    <PlayArrow />
                  </IconButton>
                  <IconButton 
                    edge="end" 
                    aria-label="stop" 
                    onClick={handleStopStream}
                    disabled={!isStreaming || streamingCameraId !== camera.id}
                  >
                    <Stop />
                  </IconButton>
                  <IconButton 
                    edge="end" 
                    aria-label="delete" 
                    onClick={() => handleDeleteCamera(camera.ip)}
                  >
                    <Delete />
                  </IconButton>
                </ListItemSecondaryAction>
              </ListItem>
            ))}
            {cameras.length === 0 && (
              <Typography variant="body1" style={{ textAlign: 'center', marginTop: '20px' }}>
                Нет добавленных камер. Добавьте камеру вручную или с помощью поиска.
              </Typography>
            )}
          </List>
        </Box>
        
        {/* Правая часть - видеоплеер */}
        {isStreaming && streamUrl && (
          <Box sx={{ flex: 1, width: '100%' }}>
            <Paper 
              elevation={3} 
              sx={{ 
                p: 2, 
                bgcolor: '#1e1e1e', 
                borderRadius: 2, 
                overflow: 'hidden',
                height: '100%',
                minHeight: 400,
              }}
            >
              <Box sx={{ mb: 2, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <Typography variant="h6" sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                  <Videocam /> {selectedCamera?.name || 'Видеопоток'}
                </Typography>
                <Button variant="outlined" size="small" onClick={handleStopStream}>
                  Остановить
                </Button>
              </Box>
              <Box sx={{ width: '100%', height: 320, position: 'relative' }}>
                {isConnecting && (
                  <Box sx={{ 
                    position: 'absolute', 
                    display: 'flex', 
                    alignItems: 'center', 
                    justifyContent: 'center',
                    width: '100%', 
                    height: '100%', 
                    zIndex: 10,
                    bgcolor: 'rgba(0,0,0,0.7)',
                    borderRadius: '8px'
                  }}>
                    <Box sx={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 2 }}>
                      <CircularProgress size={40} />
                      <Typography>Подключение к потоку...</Typography>
                    </Box>
                  </Box>
                )}
                
                {connectionError && (
                  <Box sx={{ 
                    position: 'absolute', 
                    display: 'flex', 
                    alignItems: 'center', 
                    justifyContent: 'center',
                    width: '100%', 
                    height: '100%', 
                    zIndex: 10,
                    bgcolor: 'rgba(0,0,0,0.7)',
                    borderRadius: '8px'
                  }}>
                    <Typography color="error">Ошибка подключения: {connectionError}</Typography>
                  </Box>
                )}
                
                <DirectRTSPPlayer 
                  src={streamUrl}
                  autoPlay 
                  muted 
                  controls
                  width="100%"
                  height="100%"
                  style={{ 
                    borderRadius: '8px',
                    objectFit: 'contain' 
                  }}
                  onError={(error) => {
                    console.error('RTSP Player error:', error);
                    setConnectionError(`Ошибка воспроизведения: ${error.message}`);
                    setIsConnecting(false);
                  }}
                />
              </Box>
            </Paper>
          </Box>
        )}
      </Box>
      
      {/* Диалог добавления камеры */}
      <AddCameraDialog 
        open={addDialogOpen} 
        onClose={() => setAddDialogOpen(false)} 
        onSave={handleAddCamera}
        initialData={initialData} 
      />
      
      {/* Диалог поиска камер */}
      <CameraSearchDialog 
        open={searchDialogOpen} 
        onClose={() => setSearchDialogOpen(false)} 
        foundCameras={foundCameras}
        onAddSelected={(cam) => {
          // Используем данные найденной камеры для заполнения формы
          setInitialData({
            name: cam.name,
            ip: cam.ip,
            protocol: cam.protocol || 'onvif',
            port: 554,
            user: 'admin',
            pass: '',
            onvifPort: 80,
            streamUrl: ''
          });
          // Теперь передаем initialData в AddCameraDialog через состояние
          setAddDialogOpen(true);
          setSearchDialogOpen(false);
        }}
      />
    </div>
  );
};

export default DirectCameras;