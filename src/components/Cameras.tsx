import React, { useState, useEffect } from 'react';
import { List, ListItem, ListItemText, ListItemSecondaryAction, IconButton, Typography, Button, Paper, Box, CircularProgress } from '@mui/material';
import { PlayArrow, Stop, Add, Search, Delete, Videocam } from '@mui/icons-material';
import RTSPPlayer from './RTSPPlayer';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/api/event';
import { open } from '@tauri-apps/plugin-shell';
import AddCameraDialog from './AddCameraDialog';
import CameraSearchDialog from './CameraSearchDialog';
import { CameraContextMenu } from './CameraContextMenu';
import type { Camera } from '../types';
import TerminalComponent from './Terminal';
import FileManager from './FileManager';

const Cameras: React.FC = () => {
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
  
  // Состояние для контекстного меню
  const [contextMenuCamera, setContextMenuCamera] = useState<Camera | null>(null);
  const [contextMenuPosition, setContextMenuPosition] = useState<{ left: number; top: number } | null>(null);
  const [sshTerminalCamera, setSshTerminalCamera] = useState<Camera | null>(null);
  const [fileManagerCamera, setFileManagerCamera] = useState<Camera | null>(null);

  useEffect(() => {
    console.log('🎥 CAMERAS COMPONENT LOADED 🎥');
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
      
      // Закрываем WebRTC соединение при размонтировании компонента
      if ((window as any).__peerConnection) {
        (window as any).__peerConnection.close();
        (window as any).__peerConnection = null;
      }
      
      // Останавливаем MediaMTX при выходе
      if (isStreaming) {
        invoke('mediamtx_stop').catch(console.error);
      }
    };
  }, [isStreaming]);

  // Отладочный useEffect для контекстного меню
  useEffect(() => {
    const handleGlobalContextMenu = (e: MouseEvent) => {
      console.log('Global context menu triggered on:', e.target);
      // В Tauri отключаем стандартное контекстное меню глобально
      e.preventDefault();
      return false;
    };

    // Отключаем стандартное контекстное меню во всем документе
    document.addEventListener('contextmenu', handleGlobalContextMenu);
    
    // Также добавляем CSS для отключения стандартного меню
    const style = document.createElement('style');
    style.textContent = `
      * {
        -webkit-user-select: none;
        -webkit-app-region: no-drag;
      }
      
      body {
        -webkit-context-menu: none;
      }
    `;
    document.head.appendChild(style);
    
    return () => {
      document.removeEventListener('contextmenu', handleGlobalContextMenu);
      if (style.parentNode) {
        style.parentNode.removeChild(style);
      }
    };
  }, []);

  // Загрузка камер из бэкенда
  const loadCameras = async () => {
    console.log('📡 Loading cameras from backend...');
    try {
      const loaded = await invoke('load_cameras');
      const list = loaded as Camera[];
      console.log('📡 Loaded cameras:', list);
      setCameras(list);
      // Экспортируем в window, чтобы Dashboard DnD мог найти камеры
      (window as any).__VMS_CAMERAS = list;
      // После загрузки проверим статусы камер
      checkAllStatuses(list).catch(console.error);
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
        protocol: data.protocol || 'onvif',
        port: data.port,
        user: data.user,
        pass: data.pass,
        pass_enc: encryptedPass as string,
        path_hd: data.pathHd || data.streamUrl || '',
        path_sd: data.pathSd || data.streamUrl || '',
        status: 'offline',
        onvifPort: data.onvifPort || 80,
      };
      
      const updatedCameras = [...cameras, newCamera];
      setCameras(updatedCameras);
      (window as any).__VMS_CAMERAS = updatedCameras;
      await invoke('save_cameras', { cameras: updatedCameras });
      setAddDialogOpen(false);
      // Проверка статуса для новой камеры
      await checkStatusFor(newCamera);
    } catch (err) {
      console.error('Failed to add camera:', err);
    }
  };

  // Обработчик удаления камеры
  const handleDeleteCamera = async (ip: string) => {
    try {
      // Сначала останавливаем стрим, если он запущен для этой камеры
      const camera = cameras.find(c => c.ip === ip);
      if (camera && streamingCameraId === camera.id) {
        // Закрываем WebRTC соединение, если оно существует
        if ((window as any).__peerConnection) {
          (window as any).__peerConnection.close();
          (window as any).__peerConnection = null;
        }
        
        // Останавливаем MediaMTX
        try {
          await invoke('mediamtx_stop');
          setStreamingCameraId(null);
          setIsStreaming(false);
          setStreamUrl('');
          setSelectedCamera(null);
        } catch (err) {
          console.error('Failed to stop stream:', err);
        }
      }
      
      // Удаляем камеру из бэкенда
      await invoke('remove_camera', { ip });
      
      // Обновляем состояние
  const updatedCameras = cameras.filter(c => c.ip !== ip);
      setCameras(updatedCameras);
  (window as any).__VMS_CAMERAS = updatedCameras;
      
      // Сохраняем обновленный список камер
      await invoke('save_cameras', { cameras: updatedCameras });
    } catch (err) {
      console.error('Failed to remove camera:', err);
    }
  };

  // Обработчик для запуска стрима камеры
  const handleStartStream = async (camera: Camera) => {
    try {
      setIsConnecting(true);
      setConnectionError('');
      
      // Название пути для MediaMTX
  const cameraPathName = `camera_${camera.id}`;
      
      // Используем path_hd, который должен содержать полный RTSP URL
  const rtspUrl = camera.path_hd || camera.path_sd;
      console.log('Starting stream for', camera.name, 'with RTSP URL:', rtspUrl);
      
      // Проверяем, что путь камеры содержит корректный RTSP URL
      if (!rtspUrl || !rtspUrl.startsWith('rtsp://')) {
        console.error('Invalid RTSP URL:', rtspUrl);
        setConnectionError('Неправильный формат RTSP URL. URL должен начинаться с rtsp://');
        setIsConnecting(false);
        return;
      }
      
      console.log('Adding camera to MediaMTX...');
      // Добавляем камеру в MediaMTX
      const addResult = await invoke('add_camera_to_mediamtx', { 
        name: cameraPathName,
        url: rtspUrl
      });
      console.log('MediaMTX add camera result:', addResult);
      
      // Запускаем MediaMTX
      console.log('Starting MediaMTX...');
      const result = await invoke('mediamtx_start');
      console.log('MediaMTX start result:', result);
      
      // Проверяем статус потока через некоторое время
      setTimeout(async () => {
        try {
          console.log('Checking stream status...');
          const status = await invoke('check_stream_status', { name: cameraPathName });
          console.log('Stream status:', status);
        } catch (error) {
          console.error('Failed to check stream status:', error);
        }
      }, 3000);
      
      // Сохраняем информацию о выбранной камере
      setSelectedCamera(camera);
      setStreamingCameraId(camera.id);
      setIsStreaming(true);

      // В MediaMTX потоки доступны через порт RTSP 8554
      const streamPath = `rtsp://localhost:8554/${cameraPathName}`;
      console.log('Stream path:', streamPath);
      setStreamUrl(streamPath);
      
      // Выводим справочную информацию для отладки
      console.log('MediaMTX should be rebroadcasting the stream now.');
      console.log('Source RTSP URL:', camera.path_hd);
      console.log('Local RTSP URL:', streamPath);
      console.log('HLS URL should be available at:', `http://localhost:8888/${cameraPathName}`);
      
      // RTSPPlayer компонент сам обработает отображение потока
      setIsConnecting(false);
    } catch (err) {
      console.error('Failed to start stream:', err);
      setConnectionError(typeof err === 'object' ? (err as Error).message : String(err));
      setIsConnecting(false);
      setIsStreaming(false);
      setStreamingCameraId(null);
      setSelectedCamera(null);
    }
  };
  // Проверка HTTP-доступности/авторизации камеры
  const checkStatusFor = async (camera: Camera) => {
    try {
      const pass = await invoke('decrypt_password', { enc: camera.pass_enc }) as string;
      const ok = await invoke('check_camera_http', { ip: camera.ip, user: camera.user, pass, port: camera.onvifPort || 80 });
      const status = ok ? 'online' : 'offline';
      const updated = cameras.map(c => c.ip === camera.ip ? { ...c, status } : c);
      setCameras(updated);
      (window as any).__VMS_CAMERAS = updated;
      await invoke('save_cameras', { cameras: updated });
    } catch (e) {
      console.warn('Status check failed for', camera.ip, e);
    }
  };

  // Обработчики для контекстного меню
  const handleContextMenu = (event: React.MouseEvent, camera: Camera) => {
    console.log('=== Context menu triggered ===');
    console.log('Camera:', camera.name);
    console.log('Event:', event);
    
    // Полная остановка события
    event.preventDefault();
    event.stopPropagation();
    
    // Устанавливаем состояние
    setContextMenuCamera(camera);
    setContextMenuPosition({
      left: event.clientX,
      top: event.clientY,
    });
    
    console.log('Context menu state set:', {
      camera: camera.name,
      position: { left: event.clientX, top: event.clientY }
    });
    
    // Возвращаем false для дополнительной блокировки
    return false;
  };

  const closeContextMenu = () => {
    console.log('Closing context menu');
    setContextMenuCamera(null);
    setContextMenuPosition(null);
  };

  const handleArchive = (camera: Camera) => {
    // Переходим на страницу архива с выбранной камерой
    const archiveUrl = `/archive?camera=${encodeURIComponent(camera.name)}`;
    window.location.hash = archiveUrl;
  };

  const handleEdit = (camera: Camera) => {
    // Открываем диалог редактирования с данными камеры
    setInitialData({
      id: camera.id,
      name: camera.name,
      ip: camera.ip,
      port: camera.port,
      user: camera.user,
      pass: camera.pass || '', // Если пароль зашифрован, нужна функция расшифровки
      path_hd: camera.path_hd,
      path_sd: camera.path_sd,
      protocol: camera.protocol,
      onvifPort: camera.onvifPort,
    });
    setAddDialogOpen(true);
  };

  const handleDelete = (camera: Camera) => {
    handleDeleteCamera(camera.ip);
  };

  const resolveCameraWebUrl = (camera: Camera) => {
    const rawAddress = camera.ip?.trim();
    if (!rawAddress) {
      console.warn('Camera IP is missing, cannot resolve web interface URL');
      return null;
    }

    if (/^https?:\/\//i.test(rawAddress)) {
      return rawAddress;
    }

    const httpPort = camera.port && camera.port !== 0 && camera.port !== 554 ? camera.port : undefined;
    const fallbackPort = camera.onvifPort && camera.onvifPort !== 0 ? camera.onvifPort : undefined;
    const portToUse = fallbackPort ?? httpPort;
    return `http://${rawAddress}${portToUse ? `:${portToUse}` : ''}`;
  };

  const handleOpenInBrowser = (camera: Camera) => {
    const url = resolveCameraWebUrl(camera);
    if (!url) {
      return;
    }

    void open(url).catch((error: unknown) => {
      console.error('Failed to open camera web interface in external browser', error);
    });
  };

  const handleFileManager = (camera: Camera) => {
    setFileManagerCamera(camera);
  };

  const handleSSH = (camera: Camera) => {
    setSshTerminalCamera(camera);
  };

  const checkAllStatuses = async (list?: Camera[]) => {
    const arr = list || cameras;
    for (const cam of arr) {
      // не блокируем UI: микропаузой снижаем нагрузку
      // eslint-disable-next-line no-await-in-loop
      await checkStatusFor(cam);
    }
  };




  // Обработчик для остановки стрима
  const handleStopStream = async () => {
    try {
      // Останавливаем MediaMTX
      await invoke('mediamtx_stop');
      
      // Сбрасываем состояния
      setStreamingCameraId(null);
      setIsStreaming(false);
      setStreamUrl('');
      setSelectedCamera(null);
      setConnectionError('');
    } catch (err) {
      console.error('Failed to stop stream:', err);
    }
  };

  return (
    <div>
      <Typography variant="h4" gutterBottom>
        Камеры
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
              <div
                key={camera.id}
                onContextMenu={(e) => {
                  console.log('DIV onContextMenu fired');
                  handleContextMenu(e, camera);
                }}
                style={{ width: '100%' }}
              >
                <ListItem 
                  sx={{
                    bgcolor: streamingCameraId === camera.id ? 'rgba(25, 118, 210, 0.1)' : 'transparent',
                    borderRadius: 1,
                    mb: 1,
                    // Отключаем стандартное контекстное меню через CSS
                    '&, & *': {
                      userSelect: 'none',
                      '-webkit-user-select': 'none',
                      '-moz-user-select': 'none',
                      '-ms-user-select': 'none',
                    }
                  }}
                  onDoubleClick={() => {
                    const cameraForGrid: any = {
                      id: camera.id,
                      name: camera.name,
                      ip: camera.ip,
                      user: camera.user,
                      pass_enc: camera.pass_enc,
                      streamUrl: camera.path_hd || camera.path_sd || '',
                    };
                    (window as any).setCellCamera?.(cameraForGrid);
                  }}
                  draggable
                  onDragStart={e => {
                    e.dataTransfer.setData('application/x-camera-id', String(camera.id));
                    e.dataTransfer.effectAllowed = 'move';
                  }}
                >
                <ListItemText 
                  primary={camera.name} 
                  secondary={`IP: ${camera.ip} | Порт: ${camera.port} | Статус: ${camera.status}`}
                  sx={{ 
                    cursor: 'pointer',
                    // Отключаем стандартное меню
                    userSelect: 'none',
                    '-webkit-user-select': 'none',
                    '-moz-user-select': 'none',
                    '-ms-user-select': 'none',
                  }}
                />
                <ListItemSecondaryAction>
                  <IconButton 
                    edge="end" 
                    aria-label="play" 
                    onClick={() => handleStartStream(camera)}
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
              </div>
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
                
                {selectedCamera && streamUrl ? (
                  <RTSPPlayer
                    cameraName={selectedCamera.name}
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
                ) : null}
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

      {/* Контекстное меню для камер */}
      <CameraContextMenu
        camera={contextMenuCamera}
        anchorPosition={contextMenuPosition}
        onClose={closeContextMenu}
        onArchive={handleArchive}
        onEdit={handleEdit}
        onDelete={handleDelete}
        onOpenInBrowser={handleOpenInBrowser}
        onFileManager={handleFileManager}
        onSSH={handleSSH}
      />
      <FileManager
        open={Boolean(fileManagerCamera)}
        camera={fileManagerCamera}
        onClose={() => setFileManagerCamera(null)}
      />
      <TerminalComponent
        open={Boolean(sshTerminalCamera)}
        camera={sshTerminalCamera}
        onClose={() => setSshTerminalCamera(null)}
      />
    </div>
  );
};

export default Cameras;