import React, { useState, useEffect, useCallback, useRef } from 'react';
import { List, ListItem, ListItemText, ListItemSecondaryAction, IconButton, Typography, Button, Paper, Box, CircularProgress } from '@mui/material';
import { PlayArrow, Stop, Add, Search, Delete, Videocam } from '@mui/icons-material';
import VideoStreamPlayer from './VideoStreamPlayer';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/api/event';
import type { UnlistenFn } from '@tauri-apps/api/event';
import { open } from '@tauri-apps/plugin-shell';
import AddCameraDialog from './AddCameraDialog';
import CameraSearchDialog, { type DiscoveredCamera } from './CameraSearchDialog';
import { CameraContextMenu } from './CameraContextMenu';
import type { Camera, CameraFormDraft, CameraFormValues } from '../types';
import TerminalComponent from './Terminal';
import FileManager from './FileManager';

type DiscoveryEventPayload = DiscoveredCamera;

interface DiscoveryProgressPayload {
  scanned?: number;
  total?: number;
}

interface DiscoveryFinishedPayload {
  found?: number;
  status?: string;
}

const Cameras: React.FC = () => {
  const [cameras, setCameras] = useState<Camera[]>([]);
  const [addDialogOpen, setAddDialogOpen] = useState(false);
  const [searchDialogOpen, setSearchDialogOpen] = useState(false);
  const [foundCameras, setFoundCameras] = useState<DiscoveredCamera[]>([]);
  const [streamingCameraId, setStreamingCameraId] = useState<number | null>(null);
  const [isStreaming, setIsStreaming] = useState<boolean>(false);
  const [initialData, setInitialData] = useState<CameraFormDraft | null>(null);
  const [streamBaseName, setStreamBaseName] = useState<string | null>(null);
  const [selectedCamera, setSelectedCamera] = useState<Camera | null>(null);
  const [isDiscovering, setIsDiscovering] = useState(false);
  const [discoveryProgress, setDiscoveryProgress] = useState('');
  const [go2rtcSettings, setGo2rtcSettings] = useState({
    showMonitor: false,
    enableSnapshot: true,
    enable2WayAudio: false,
    enableAdaptiveBitrate: true,
  });

  const foundCamerasRef = useRef<DiscoveredCamera[]>(foundCameras);
  useEffect(() => {
    foundCamerasRef.current = foundCameras;
  }, [foundCameras]);

  // Load go2rtc settings from localStorage
  useEffect(() => {
    try {
      const savedSettings = localStorage.getItem('go2rtcSettings');
      if (savedSettings) {
        setGo2rtcSettings(JSON.parse(savedSettings));
      }
    } catch (error) {
      console.error('Failed to load go2rtc settings:', error);
    }
  }, []);
  const [isConnecting, setIsConnecting] = useState<boolean>(false);
  const [connectionError, setConnectionError] = useState<string>('');
  
  // Состояние для контекстного меню
  const [contextMenuCamera, setContextMenuCamera] = useState<Camera | null>(null);
  const [contextMenuPosition, setContextMenuPosition] = useState<{ left: number; top: number } | null>(null);
  const [sshTerminalCamera, setSshTerminalCamera] = useState<Camera | null>(null);
  const [fileManagerCamera, setFileManagerCamera] = useState<Camera | null>(null);

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
  const checkStatusFor = useCallback(async (camera: Camera) => {
    try {
      const pass = await invoke<string>('decrypt_password', { enc: camera.pass_enc });
      const ok = await invoke<boolean>('check_camera_http', {
        ip: camera.ip,
        user: camera.user,
        pass,
        port: camera.onvifPort || 80,
      });
      const status = ok ? 'online' : 'offline';
      const updated = cameras.map(c => (c.ip === camera.ip ? { ...c, status } : c));
      setCameras(updated);
      window.__VMS_CAMERAS = updated;
      await invoke('save_cameras', { cameras: updated });
    } catch (error) {
      console.warn('Status check failed for', camera.ip, error);
    }
  }, [cameras]);

  const checkAllStatuses = useCallback(async (list?: Camera[]) => {
    const arr = list || cameras;
    for (const cam of arr) {
      await checkStatusFor(cam);
    }
  }, [cameras, checkStatusFor]);

  const loadCameras = useCallback(async () => {
    console.log('📡 Loading cameras from backend...');
    try {
      const list = await invoke<Camera[]>('load_cameras');
      console.log('📡 Loaded cameras:', list);
      setCameras(list);
      // Экспортируем в window, чтобы Dashboard DnD мог найти камеры
      window.__VMS_CAMERAS = list;
      // После загрузки проверим статусы камер
      checkAllStatuses(list).catch(console.error);
    } catch (err) {
      console.error('Failed to load cameras:', err);
    }
  }, [checkAllStatuses]);

  useEffect(() => {
    console.log('🎥 CAMERAS COMPONENT LOADED 🎥');
    loadCameras();

    const listenerPromises: Promise<UnlistenFn>[] = [];

    listenerPromises.push(
      listen<DiscoveryEventPayload>('device-found', event => {
        const camera = event.payload;
        setFoundCameras(prev => {
          if (prev.find(c => c.ip === camera.ip)) {
            return prev;
          }
          return [...prev, camera];
        });
      })
    );

    listenerPromises.push(
      listen<DiscoveryProgressPayload>('device-discovery-progress', event => {
        const payload = event.payload ?? {};
        const scanned = typeof payload.scanned === 'number' ? payload.scanned : undefined;
        const total = typeof payload.total === 'number' ? payload.total : undefined;

        if (scanned !== undefined && total !== undefined) {
          const foundCount = foundCamerasRef.current.length;
          const suffix = foundCount > 0 ? ` • найдено: ${foundCount}` : '';
          setDiscoveryProgress(`Сканирование: ${scanned}/${total}${suffix}`);
        } else {
          setDiscoveryProgress('Сканирование сети...');
        }
      })
    );

    listenerPromises.push(
      listen<DiscoveryFinishedPayload>('device-discovery-finished', event => {
        setIsDiscovering(false);
        const payload = event.payload ?? {};
        const found = typeof payload.found === 'number' ? payload.found : foundCamerasRef.current.length;

        if (found > 0) {
          setDiscoveryProgress(`Найдено камер: ${found}`);
        } else if (payload.status === 'no-targets') {
          setDiscoveryProgress('Сети для сканирования не найдены');
        } else if (payload.status === 'error') {
          setDiscoveryProgress('Ошибка при поиске камер');
        } else {
          setDiscoveryProgress('Камеры не найдены');
        }
      })
    );

    return () => {
      listenerPromises.forEach(promise => {
        promise
          .then(unlistenFn => unlistenFn())
          .catch(() => undefined);
      });

      if (window.__peerConnection) {
        window.__peerConnection.close();
        window.__peerConnection = null;
      }
    };
  }, [isStreaming, loadCameras]);

  const triggerDiscovery = useCallback(async (interfaces?: string[]) => {
    try {
      setFoundCameras([]);
      setIsDiscovering(true);
      setDiscoveryProgress('Сканирование сети...');

      const payload = interfaces && interfaces.length > 0
        ? { request: { interfaces } }
        : { request: null };

      await invoke('discover_cameras', payload);
    } catch (err) {
      console.error('Failed to start camera discovery:', err);
      setIsDiscovering(false);
      setDiscoveryProgress('');
    }
  }, []);

  // Обработчик добавления камеры
  const handleAddCamera = async (data: CameraFormValues) => {
    try {
      const encryptedPass = await invoke<string>('encrypt_password', { password: data.pass });
      const newCamera: Camera = {
        id: cameras.length ? Math.max(...cameras.map(c => c.id)) + 1 : 1,
        name: data.name,
        ip: data.ip,
        protocol: data.protocol || 'onvif',
        port: data.port,
        user: data.user,
        pass: data.pass,
        pass_enc: encryptedPass,
        path_hd: data.pathHd || data.streamUrl || '',
        path_sd: data.pathSd || data.streamUrl || '',
        status: 'offline',
        onvifPort: data.onvifPort || 80,
      };
      
      const updatedCameras = [...cameras, newCamera];
      setCameras(updatedCameras);
      window.__VMS_CAMERAS = updatedCameras;
      await invoke('save_cameras', { cameras: updatedCameras });
      setAddDialogOpen(false);
      setInitialData(null);
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
        if (window.__peerConnection) {
          window.__peerConnection.close();
          window.__peerConnection = null;
        }
        
        setStreamingCameraId(null);
        setIsStreaming(false);
        setStreamBaseName(null);
        setSelectedCamera(null);
      }
      
      // Удаляем камеру из бэкенда
      await invoke('remove_camera', { ip });
      
      // Обновляем состояние
      const updatedCameras = cameras.filter(c => c.ip !== ip);
      setCameras(updatedCameras);
      window.__VMS_CAMERAS = updatedCameras;
      
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
      
      const baseName = `cam${camera.id}`;
      const primaryUrl = camera.path_hd?.trim() || camera.path_sd?.trim() || '';
      const fallbackUrl = camera.path_sd?.trim() || camera.path_hd?.trim() || '';

      if (!primaryUrl.startsWith('rtsp://')) {
        console.error('Invalid RTSP URL:', primaryUrl);
        setConnectionError('Камера должна иметь корректный RTSP URL (rtsp://...).');
        setIsConnecting(false);
        return;
      }

      const hdUrl = primaryUrl;
      const sdUrl = fallbackUrl && fallbackUrl.startsWith('rtsp://') ? fallbackUrl : primaryUrl;

      console.log('Configuring go2rtc stream for', camera.name, {
        cameraId: camera.id,
        hdUrl,
        sdUrl,
      });

      await invoke('add_camera_streams', {
        cameraId: camera.id,
        hdUrl,
        sdUrl,
      });

      await invoke('start_go2rtc');

      setSelectedCamera(camera);
      setStreamingCameraId(camera.id);
      setStreamBaseName(baseName);
      setIsStreaming(true);
      setIsConnecting(false);
    } catch (err) {
      console.error('Failed to start stream:', err);
      setConnectionError(typeof err === 'object' ? (err as Error).message : String(err));
      setIsConnecting(false);
      setIsStreaming(false);
      setStreamingCameraId(null);
      setStreamBaseName(null);
      setSelectedCamera(null);
    }
  };
  // Проверка HTTP-доступности/авторизации камеры
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
      pass: camera.pass || '',
      pathHd: camera.path_hd,
      pathSd: camera.path_sd,
      protocol: camera.protocol,
      onvifPort: camera.onvifPort,
      streamUrl: camera.path_hd || camera.path_sd || '',
      groupId: camera.groupId ?? null,
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




  // Обработчик для остановки стрима
  const handleStopStream = () => {
    setStreamingCameraId(null);
    setIsStreaming(false);
    setStreamBaseName(null);
    setSelectedCamera(null);
    setConnectionError('');
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
          onClick={() => setSearchDialogOpen(true)}
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
                    const cameraForGrid: Camera = {
                      ...camera,
                      streamUrl: camera.streamUrl ?? camera.path_hd ?? camera.path_sd ?? '',
                    };
                    window.setCellCamera?.(cameraForGrid);
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
        {isStreaming && streamBaseName && (
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
                
                {selectedCamera && streamBaseName ? (
                  <VideoStreamPlayer
                    streamName={streamBaseName}
                    useHdQuality
                    autoPlay
                    muted
                    controls
                    width="100%"
                    height="100%"
                    objectFit="contain"
                    onError={error => {
                      console.error('VideoStreamPlayer error:', error);
                      setConnectionError(`Ошибка воспроизведения: ${error.message}`);
                      setIsConnecting(false);
                    }}
                    showMonitor={go2rtcSettings.showMonitor}
                    enableSnapshot={go2rtcSettings.enableSnapshot}
                    enable2WayAudio={go2rtcSettings.enable2WayAudio}
                    enableAdaptiveBitrate={go2rtcSettings.enableAdaptiveBitrate}
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
        isDiscovering={isDiscovering}
        discoveryProgress={discoveryProgress}
        onStartDiscovery={filters => triggerDiscovery(filters?.interfaces)}
        onAddSelected={(cams) => {
          if (cams.length === 0) {
            return;
          }
          const cam = cams[0];
          setInitialData({
            name: cam.name,
            ip: cam.ip,
            protocol: cam.protocol || 'onvif',
            port: cam.port ?? cam.detectedPort ?? 554,
            user: 'admin',
            pass: '',
            onvifPort: cam.onvifPort ?? cam.detectedPort ?? 80,
            streamUrl: ''
          });
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