import React, { useEffect, useState, useRef, useCallback } from 'react';
import { useNavigate } from 'react-router-dom';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/api/event';
import type { UnlistenFn } from '@tauri-apps/api/event';
import { open } from '@tauri-apps/plugin-shell';
import {
  Typography,
  Paper,
  List,
  ListItem,
  ListItemText,
  Collapse,
  IconButton,
  Box,
  Tooltip,
  ListItemButton,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Button,
  TextField,
  Slide,
  CircularProgress
} from '@mui/material';
import ExpandLess from '@mui/icons-material/ExpandLess';
import ExpandMore from '@mui/icons-material/ExpandMore';
import AddIcon from '@mui/icons-material/Add';
import SettingsOutlinedIcon from '@mui/icons-material/SettingsOutlined';
import SearchIcon from '@mui/icons-material/Search';
import CreateNewFolderIcon from '@mui/icons-material/CreateNewFolder';
import DeleteOutlineIcon from '@mui/icons-material/DeleteOutline';
import EditIcon from '@mui/icons-material/Edit';
import ArchiveOutlinedIcon from '@mui/icons-material/ArchiveOutlined';
import StorageIcon from '@mui/icons-material/Storage';
import QueryStatsRoundedIcon from '@mui/icons-material/QueryStatsRounded';
import LogoutIcon from '@mui/icons-material/Logout';
import ManageAccountsIcon from '@mui/icons-material/ManageAccounts';
import ArticleIcon from '@mui/icons-material/Article';
import type { Camera, CameraGroup, CameraFormDraft, CameraFormValues } from '../types';
import AddCameraDialog from './AddCameraDialog';
import BatchAddCameraDialog from './BatchAddCameraDialog';
import CameraSearchDialog from './CameraSearchDialog';
import type { DiscoveredCamera } from './CameraSearchDialog';
import SettingsModal from './SettingsModal';
import UserDialog from './UserDialog';
import TerminalComponent from './Terminal';
import FileManager from './FileManager';
import ArchiveImproved from './ArchiveImproved';
import AnalyticsModal from './AnalyticsModal';
import { useLocalization } from '../hooks/useLocalization';
import { useAppState } from '../hooks/useAppState';
import { useAuth } from '../hooks/useAuth';
import { useCameraContextMenu } from '../hooks/useCameraContextMenu';
import { CAMERA_STATUS_COLORS, resolveCameraStatusLabel } from '../utils/cameraStatus';
import { useToast } from '../hooks/useToast';
import { Toast } from './Toast';
import { useLoggerUi } from '../contexts/LoggerUiContext';

type DiscoveryEventPayload = DiscoveredCamera;

interface DiscoveryProgressPayload {
  scanned?: number;
  total?: number;
}

interface DiscoveryFinishedPayload {
  found?: number;
  status?: string;
}

const DevicePanel: React.FC = () => {
  const { t } = useLocalization();
  const navigate = useNavigate();
  const {
    cameras,
    groups,
    addCamera,
    updateCamera,
    removeCamera,
    addGroup,
    updateGroup,
    removeGroup,
    isLoading,
    cameraStatuses,
    updateCameraStatus,
  } = useAppState();
  const { user, logout, hasPermission } = useAuth();
  const {
    openCameraContextMenu,
    registerDefaultCameraContextMenuHandlers,
  } = useCameraContextMenu();
  const { toast, showToast, hideToast } = useToast();
  const { openViewer } = useLoggerUi();

  const [addOpen, setAddOpen] = useState(false);
  const [searchOpen, setSearchOpen] = useState(false);
  const [settingsOpen, setSettingsOpen] = useState(false);
  const [userOpen, setUserOpen] = useState(false);
  const [analyticsOpen, setAnalyticsOpen] = useState(false);
  const [foundCameras, setFoundCameras] = useState<DiscoveredCamera[]>([]);
  const foundCamerasRef = useRef<DiscoveredCamera[]>(foundCameras);
  const [isDiscovering, setIsDiscovering] = useState(false);
  const [discoveryProgress, setDiscoveryProgress] = useState('');
  const [addDialogData, setAddDialogData] = useState<CameraFormDraft | null>(null);
  const [showArchive, setShowArchive] = useState(false);
  const [sshTerminalCamera, setSshTerminalCamera] = useState<Camera | null>(null);
  const [fileManagerCamera, setFileManagerCamera] = useState<Camera | null>(null);
  const [batchAddOpen, setBatchAddOpen] = useState(false);
  const [selectedBatchCameras, setSelectedBatchCameras] = useState<DiscoveredCamera[]>([]);

  const cameraStatusesRef = useRef(cameraStatuses);
  useEffect(() => {
    cameraStatusesRef.current = cameraStatuses;
  }, [cameraStatuses]);

  useEffect(() => {
    const checkAllCameras = async () => {
      for (const camera of cameras) {
        try {
          let pass = camera.pass;
          if (camera.pass_enc) {
             pass = await invoke<string>('decrypt_password', { enc: camera.pass_enc });
          }

          const ok = await invoke<boolean>('check_camera_http', {
            ip: camera.ip,
            user: camera.user,
            pass,
            port: camera.onvifPort || 80,
          });

          const currentStatus = cameraStatusesRef.current[camera.id];
          
          if (ok) {
             // If currently offline or undefined, mark as online
             if (!currentStatus || currentStatus.status === 'offline') {
                 updateCameraStatus(camera.id, {
                     status: 'online',
                     lastUpdated: Date.now()
                 });
             }
          } else {
             // If failed, mark as offline
             if (!currentStatus || currentStatus.status !== 'offline') {
                 updateCameraStatus(camera.id, {
                     status: 'offline',
                     lastUpdated: Date.now()
                 });
             }
          }
        } catch (e) {
          console.error('Failed to check camera status', camera.id, e);
        }
      }
    };

    const interval = setInterval(checkAllCameras, 30000); // Check every 30s
    checkAllCameras(); // Initial check

    return () => clearInterval(interval);
  }, [cameras, updateCameraStatus]);

  const handleOpenCameraInBrowser = useCallback((camera: Camera) => {
    const rawAddress = camera.ip?.trim();
    if (!rawAddress) {
      console.warn('Camera IP is missing, cannot open web interface');
      return;
    }

    const hasProtocol = /^https?:\/\//i.test(rawAddress);
    const defaultPort = camera.port && camera.port !== 0 && camera.port !== 554 ? camera.port : undefined;
    const onvifPort = camera.onvifPort && camera.onvifPort !== 0 ? camera.onvifPort : undefined;

    const portToUse = hasProtocol ? undefined : onvifPort ?? defaultPort;
    const formattedAddress = hasProtocol
      ? rawAddress
      : `http://${rawAddress}${portToUse ? `:${portToUse}` : ''}`;

    void open(formattedAddress).catch((error: unknown) => {
      console.error('Failed to open camera web interface in external browser', error);
    });
  }, []);

  const launchFileManager = useCallback((camera: Camera) => {
    setFileManagerCamera(camera);
  }, []);

  const openSshTerminal = useCallback((camera: Camera) => {
    setSshTerminalCamera(camera);
  }, []);

  const canEditCameras = hasPermission('edit_cameras');
  const canManageLayout = hasPermission('manage_layout');
  const canAccessSettings = hasPermission('access_settings');
  const canViewArchive = hasPermission('view_archive');
  const canManageUsers = user?.role === 'admin';
  const canDeleteCameras = hasPermission('delete_cameras');

  const mapCameraToFormDraft = useCallback((camera: Camera): CameraFormDraft => ({
    id: camera.id,
    name: camera.name,
    ip: camera.ip,
    protocol: camera.protocol,
    port: camera.port,
    user: camera.user,
    pass: camera.pass,
    pathHd: camera.path_hd,
    pathSd: camera.path_sd,
    onvifPort: camera.onvifPort ?? 80,
    streamUrl: camera.streamUrl ?? camera.path_hd ?? camera.path_sd ?? '',
    groupId: camera.groupId ?? null,
  }), []);

  useEffect(() => {
    if (!canViewArchive && showArchive) {
      setShowArchive(false);
    }
  }, [canViewArchive, showArchive]);

  const discoveryTimeoutRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  const clearDiscoveryTimer = useCallback(() => {
    if (discoveryTimeoutRef.current) {
      clearTimeout(discoveryTimeoutRef.current);
      discoveryTimeoutRef.current = null;
    }
  }, []);

  const scheduleDiscoveryTimeout = useCallback(() => {
    clearDiscoveryTimer();
    discoveryTimeoutRef.current = setTimeout(() => {
      setIsDiscovering(false);
      setDiscoveryProgress('');
      discoveryTimeoutRef.current = null;
    }, 15000);
  }, [clearDiscoveryTimer]);
  
  useEffect(() => {
    foundCamerasRef.current = foundCameras;
  }, [foundCameras]);

  // Состояние для групп
  const [expandedGroups, setExpandedGroups] = useState<Set<number | null>>(new Set([null]));
  const [addGroupOpen, setAddGroupOpen] = useState(false);
  const [newGroupName, setNewGroupName] = useState('');
  const [editingGroup, setEditingGroup] = useState<CameraGroup | null>(null);
  
  // Состояние для контекстного меню
  const nextId = useRef(1);

  // Обновляем nextId при изменении камер
  useEffect(() => {
    if (cameras.length > 0) {
      nextId.current = Math.max(...cameras.map(c => c.id)) + 1;
    }
  }, [cameras]);

  // Подписываемся на события поиска камер
  useEffect(() => {
    let isActive = true;
  const unlistenFns: UnlistenFn[] = [];

    const registerListeners = async () => {
      try {
        console.log('Setting up device discovery listeners...');

        const unlistenDeviceFound = await listen<DiscoveryEventPayload>('device-found', event => {
          const payload = event.payload;
          if (!payload?.ip) {
            return;
          }

          const detectedPort = typeof payload.detectedPort === 'number' ? payload.detectedPort : undefined;
          const suggestedRtspPort = typeof payload.port === 'number'
            ? payload.port
            : detectedPort && [554, 8554, 7447].includes(detectedPort)
              ? detectedPort
              : undefined;
          const suggestedOnvifPort = typeof payload.onvifPort === 'number'
            ? payload.onvifPort
            : detectedPort && ![554, 8554, 7447].includes(detectedPort)
              ? detectedPort
              : undefined;

          const normalizedCamera: DiscoveredCamera = {
            name: payload.name || `Camera ${payload.ip}`,
            ip: payload.ip,
            protocol: payload.protocol || 'onvif',
            detectedPort,
            onvifPort: suggestedOnvifPort,
            port: suggestedRtspPort
          };

          setFoundCameras(prev => {
            if (prev.some(cam => cam.ip === normalizedCamera.ip)) {
              return prev;
            }
            const updated = [...prev, normalizedCamera];
            setDiscoveryProgress(`Найдено камер: ${updated.length}`);
            return updated;
          });
        });

        if (!isActive) {
          unlistenDeviceFound();
          return;
        }
        unlistenFns.push(unlistenDeviceFound);

        const unlistenProgress = await listen<DiscoveryProgressPayload>('device-discovery-progress', event => {
          const payload = event.payload ?? {};
          const scanned = typeof payload.scanned === 'number' ? payload.scanned : undefined;
          const total = typeof payload.total === 'number' ? payload.total : undefined;
          const found = foundCamerasRef.current.length;

          if (scanned !== undefined && total !== undefined) {
            const suffix = found > 0 ? ` • найдено: ${found}` : '';
            setDiscoveryProgress(`Сканирование: ${scanned}/${total}${suffix}`);
          } else if (found === 0) {
            setDiscoveryProgress('Сканирование сети...');
          }
        });

        if (!isActive) {
          unlistenProgress();
          return;
        }
        unlistenFns.push(unlistenProgress);

        const unlistenFinished = await listen<DiscoveryFinishedPayload>('device-discovery-finished', event => {
          clearDiscoveryTimer();
          setIsDiscovering(false);

          const payload = event.payload ?? {};
          const found = typeof payload.found === 'number' ? payload.found : foundCamerasRef.current.length;

          if (found > 0) {
            setDiscoveryProgress(`Найдено камер: ${found}`);
          } else if (payload?.status === 'no-targets') {
            setDiscoveryProgress('Сети для сканирования не найдены');
          } else if (payload?.status === 'error') {
            setDiscoveryProgress('Ошибка при поиске камер');
          } else {
            setDiscoveryProgress('Камеры не найдены');
          }
        });

        if (!isActive) {
          unlistenFinished();
          return;
        }
        unlistenFns.push(unlistenFinished);

        console.log('Device discovery listeners ready');
      } catch (error) {
        console.error('Failed to register discovery listeners', error);
      }
    };

    registerListeners();

    return () => {
      isActive = false;
      unlistenFns.forEach(unlisten => unlisten());
      clearDiscoveryTimer();
    };
  }, [clearDiscoveryTimer]);

  const discoverCameras = useCallback(async (interfaces?: string[]) => {
    try {
      console.log('Starting camera discovery...', interfaces);
      setFoundCameras([]);
      setIsDiscovering(true);
      setDiscoveryProgress(t('camera_search_scanning'));

      scheduleDiscoveryTimeout();

      const payload = interfaces && interfaces.length > 0
        ? { request: { interfaces } }
        : { request: null };

      await invoke('discover_cameras', payload);
      console.log('Camera discovery started successfully');
    } catch (error) {
      console.error('Failed to start camera discovery:', error);
      setFoundCameras([]);
      clearDiscoveryTimer();
      setIsDiscovering(false);
      setDiscoveryProgress('');
    }
  }, [clearDiscoveryTimer, scheduleDiscoveryTimeout, t]);

  // Функции для работы с группами
  const handleCreateGroup = async () => {
    if (!newGroupName.trim()) return;
    
    const newGroup: CameraGroup = {
      id: Date.now(),
      name: newGroupName.trim(),
      cameraIds: [],
      createdAt: new Date()
    };
    
    await addGroup(newGroup);
    setNewGroupName('');
    setAddGroupOpen(false);
  };

  const handleRenameGroup = async (group: CameraGroup, newName: string) => {
    if (!newName.trim()) return;
    
    const updatedGroup = { ...group, name: newName.trim() };
    await updateGroup(updatedGroup);
  };

  const handleDeleteGroup = async (groupId: number) => {
    await removeGroup(groupId);
  };

  const handleMoveCameraToGroup = useCallback(async (camera: Camera, groupId: number | null) => {
    const updatedCamera = { ...camera, groupId };
    await updateCamera(updatedCamera);
  }, [updateCamera]);

  useEffect(() => {
    registerDefaultCameraContextMenuHandlers({
      onArchive: () => {
        if (canViewArchive) {
          setShowArchive(true);
        } else {
          alert(t('permission_denied'));
        }
      },
      onEdit: (camera: Camera) => {
        if (canEditCameras) {
          setAddDialogData(mapCameraToFormDraft(camera));
          setAddOpen(true);
        } else {
          alert(t('permission_denied'));
        }
      },
      onDelete: (camera: Camera) => {
        if (canDeleteCameras) {
          void removeCamera(camera.id);
        } else {
          alert(t('permission_denied'));
        }
      },
      onOpenInBrowser: (camera: Camera) => {
        handleOpenCameraInBrowser(camera);
      },
      onFileManager: (camera: Camera) => {
        launchFileManager(camera);
      },
      onSSH: (camera: Camera) => {
        openSshTerminal(camera);
      },
      onMoveToGroup: (camera: Camera, targetGroupId: number | null) => {
        if (canManageLayout || canEditCameras) {
          void handleMoveCameraToGroup(camera, targetGroupId);
        } else {
          alert(t('permission_denied'));
        }
      },
    });

    return () => {
      registerDefaultCameraContextMenuHandlers({});
    };
  }, [
    canViewArchive,
    canEditCameras,
    canDeleteCameras,
    canManageLayout,
    handleOpenCameraInBrowser,
    launchFileManager,
    mapCameraToFormDraft,
    openSshTerminal,
    registerDefaultCameraContextMenuHandlers,
    removeCamera,
    t,
    handleMoveCameraToGroup,
  ]);

  // Остальные функции
  const toggleGroup = (groupId: number | null) => {
    const newExpanded = new Set(expandedGroups);
    if (newExpanded.has(groupId)) {
      newExpanded.delete(groupId);
    } else {
      newExpanded.add(groupId);
    }
    setExpandedGroups(newExpanded);
  };

  const handleAddCamera = async (formData: CameraFormValues) => {
    const existingId = typeof formData.id === 'number' ? formData.id : undefined;

    if (existingId !== undefined) {
      const currentCamera = cameras.find(cam => cam.id === existingId);
      if (!currentCamera) {
        console.warn('Attempted to update camera that does not exist', existingId);
        return;
      }

      const updatedCamera: Camera = {
        ...currentCamera,
        name: formData.name ?? currentCamera.name,
        ip: formData.ip ?? currentCamera.ip,
        protocol: formData.protocol ?? currentCamera.protocol,
        port: formData.port ?? currentCamera.port,
        user: formData.user ?? currentCamera.user,
        pass: formData.pass ?? currentCamera.pass,
        path_hd: formData.pathHd ?? currentCamera.path_hd,
        path_sd: formData.pathSd ?? currentCamera.path_sd,
        onvifPort: formData.onvifPort ?? currentCamera.onvifPort,
        streamUrl: formData.streamUrl ?? currentCamera.streamUrl,
        groupId: formData.groupId ?? currentCamera.groupId ?? null,
      };

      await updateCamera(updatedCamera);
      return;
    }

    const camera: Camera = {
      id: nextId.current++,
      name: formData.name || 'Camera',
      ip: formData.ip,
      protocol: formData.protocol || 'onvif',
      port: formData.port || 554,
      user: formData.user || 'admin',
      pass: formData.pass || '',
      path_hd: formData.pathHd || formData.streamUrl || '',
      path_sd: formData.pathSd || formData.streamUrl || '',
      status: 'offline',
      onvifPort: formData.onvifPort,
      groupId: formData.groupId ?? null,
      streamUrl: formData.streamUrl ?? formData.pathHd ?? formData.pathSd ?? '',
    };
    
    await addCamera(camera);
  };

  const handleDeleteCamera = async (cameraIp: string) => {
    const cameraToDelete = cameras.find(c => c.ip === cameraIp);
    if (cameraToDelete) {
      await removeCamera(cameraToDelete.id);
    }
  };

  const showContextMenu = (event: React.MouseEvent, camera: Camera) => {
    event.preventDefault();
    event.stopPropagation();

    openCameraContextMenu({
      camera,
      anchorPosition: { left: event.clientX, top: event.clientY },
      groups,
    });
  };

  // Рендеринг группы камер
  const renderCameraGroup = (group: CameraGroup | null) => {
    const groupCameras = cameras.filter(c => c.groupId === (group?.id || null));
    const isExpanded = expandedGroups.has(group?.id || null);
    const groupName = group?.name || 'Без группы';
    
    if (groupCameras.length === 0 && group !== null) return null;

    return (
      <Box key={group?.id || 'ungrouped'} sx={{ mb: 1 }}>
        <ListItemButton
          onClick={() => toggleGroup(group?.id || null)}
          onContextMenu={(e) => {
            if (group) {
              e.preventDefault();
              // Здесь можно добавить контекстное меню для группы
            }
          }}
          sx={{ 
            bgcolor: '#3b4146', 
            borderRadius: 1, 
            mb: 0.5,
            py: 1,
            '&:hover': {
              bgcolor: '#4a5056'
            }
          }}
        >
          <ListItemText primary={groupName} />
          {isExpanded ? <ExpandLess /> : <ExpandMore />}
          {group && (
            <Box sx={{ display: 'flex', gap: 0.5 }}>
              <IconButton
                size="small"
                onClick={(e) => {
                  e.stopPropagation();
                  setEditingGroup(group);
                  setNewGroupName(group.name);
                  setAddGroupOpen(true);
                }}
                sx={{ color: 'rgba(255,255,255,0.7)' }}
              >
                <EditIcon fontSize="small" />
              </IconButton>
              <IconButton
                size="small"
                onClick={(e) => {
                  e.stopPropagation();
                  if (window.confirm(`Удалить группу "${group.name}"?`)) {
                    handleDeleteGroup(group.id);
                  }
                }}
                sx={{ color: 'rgba(255,255,255,0.7)' }}
              >
                <DeleteOutlineIcon fontSize="small" />
              </IconButton>
            </Box>
          )}
        </ListItemButton>
        
        <Collapse in={isExpanded} timeout="auto" unmountOnExit>
          <List sx={{ pl: 2 }}>
            {groupCameras.map((camera) => {
              const statusEntry = cameraStatuses[camera.id];
              const statusKey = statusEntry?.status ?? 'offline';
              const statusColor = CAMERA_STATUS_COLORS[statusKey] ?? CAMERA_STATUS_COLORS.offline;
              const statusLabel = resolveCameraStatusLabel(statusKey, t);
              const statusTooltip = statusLabel;

              return (
                <ListItem
                  key={camera.id}
                  draggable
                  onDragStart={(e) => {
                    e.dataTransfer.setData('application/x-camera-id', camera.id.toString());
                    e.dataTransfer.effectAllowed = 'move';
                  }}
                  onContextMenu={(e) => showContextMenu(e, camera)}
                  onDoubleClick={() => {
                    // Добавление камеры в сетку при двойном клике
                    const cameraForGrid: Camera = {
                      ...camera,
                      streamUrl: camera.streamUrl ?? camera.path_hd ?? camera.path_sd ?? '',
                    };
                    window.setCellCamera?.(cameraForGrid);
                  }}
                  sx={{ 
                    cursor: 'grab',
                    borderRadius: 1,
                    mb: 0.5,
                    bgcolor: 'rgba(255,255,255,0.05)',
                    display: 'flex',
                    alignItems: 'center',
                    gap: 1,
                    '&:hover': {
                      bgcolor: 'rgba(255,255,255,0.1)'
                    }
                  }}
                >
                  <Tooltip title={statusTooltip} placement="top">
                    <Box
                      sx={{
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'center',
                        p: 1,
                        mr: 0.5,
                      }}
                    >
                      <Box
                        sx={{
                          width: 10,
                          height: 10,
                          borderRadius: '50%',
                          backgroundColor: statusColor,
                          boxShadow: statusKey === 'online' ? `0 0 8px ${statusColor}` : 'none',
                        }}
                      />
                    </Box>
                  </Tooltip>
                  <Box sx={{ flexGrow: 1 }}>
                    <ListItemText 
                      primary={camera.name} 
                      secondary={camera.ip}
                      sx={{
                        '& .MuiListItemText-primary': {
                          fontSize: '0.9rem'
                        },
                        '& .MuiListItemText-secondary': {
                          fontSize: '0.8rem',
                          color: 'rgba(255,255,255,0.6)'
                        }
                      }}
                    />
                  </Box>
                  <IconButton
                    edge="end"
                    size="small"
                    onClick={() => handleDeleteCamera(camera.ip)}
                    sx={{ color: 'rgba(255,255,255,0.7)' }}
                  >
                    ×
                  </IconButton>
                </ListItem>
              );
            })}
          </List>
        </Collapse>
      </Box>
    );
  };

  // Drag & Drop для перемещения между группами
  const handleDragOver = (e: React.DragEvent) => {
    e.preventDefault();
    e.dataTransfer.dropEffect = 'move';
  };

  const handleDrop = (e: React.DragEvent, groupId: number | null) => {
    e.preventDefault();
    const cameraId = parseInt(e.dataTransfer.getData('application/x-camera-id'));
    const camera = cameras.find(c => c.id === cameraId);
    
    if (camera && camera.groupId !== groupId) {
      handleMoveCameraToGroup(camera, groupId);
    }
  };

  return (
    <Paper sx={{ width: '100%', bgcolor: '#2a2f33', color: '#fff', borderRadius: 2, boxShadow: 0, p: 2, height: '100%' }}>
      {isLoading ? (
        <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100%' }}>
          <Typography variant="body2" color="text.secondary">
            {t('loading_text')}...
          </Typography>
        </Box>
      ) : (
        <>
          {/* Панель инструментов */}
          <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', mb: 1 }}>
            <Box sx={{ display: 'flex', gap: 1, alignItems: 'center', justifyContent: 'flex-start' }}>
              {canEditCameras && (
                <Tooltip title={t('search_cameras')}>
                  <span>
                    <IconButton
                      size="small"
                      sx={{
                        color: isDiscovering ? 'rgba(25, 118, 210, 0.8)' : 'rgba(255,255,255,0.7)',
                        '&:disabled': { color: 'rgba(255,255,255,0.3)' },
                      }}
                      disabled={isDiscovering}
                      onClick={() => {
                        setSearchOpen(true);
                      }}
                    >
                      {isDiscovering ? <CircularProgress size={20} color="inherit" /> : <SearchIcon />}
                    </IconButton>
                  </span>
                </Tooltip>
              )}
              {canEditCameras && (
                <Tooltip title={t('create_group')}>
                  <span>
                    <IconButton
                      size="small"
                      sx={{ color: 'rgba(255,255,255,0.7)' }}
                      onClick={() => setAddGroupOpen(true)}
                    >
                      <CreateNewFolderIcon />
                    </IconButton>
                  </span>
                </Tooltip>
              )}
              {canEditCameras && (
                <Tooltip title={t('add_camera')}>
                  <span>
                    <IconButton
                      size="small"
                      sx={{ color: 'rgba(255,255,255,0.7)' }}
                      onClick={() => setAddOpen(true)}
                    >
                      <AddIcon />
                    </IconButton>
                  </span>
                </Tooltip>
              )}
              {canViewArchive && (
                <Tooltip title={t('analytics_unified_window')}>
                  <span>
                    <IconButton
                      size="small"
                      sx={{ color: 'rgba(255,255,255,0.7)' }}
                      onClick={() => setAnalyticsOpen(true)}
                    >
                      <QueryStatsRoundedIcon />
                    </IconButton>
                  </span>
                </Tooltip>
              )}
              {canAccessSettings && (
                <Tooltip title={t('settings_tooltip')}>
                  <span>
                    <IconButton
                      size="small"
                      onClick={() => setSettingsOpen(true)}
                      sx={{ color: 'rgba(255,255,255,0.7)' }}
                    >
                      <SettingsOutlinedIcon />
                    </IconButton>
                  </span>
                </Tooltip>
              )}
              <Tooltip title={t('log_viewer.tooltip')}>
                <span>
                  <IconButton
                    size="small"
                    sx={{ color: 'rgba(255,255,255,0.7)' }}
                    onClick={openViewer}
                  >
                    <ArticleIcon />
                  </IconButton>
                </span>
              </Tooltip>
            </Box>
            <Box sx={{ display: 'flex', alignItems: 'center', gap: 0.5 }}>
              {canManageUsers && (
                <Tooltip title={t('user_management_tooltip')}>
                  <span>
                    <IconButton
                      size="small"
                      sx={{ color: 'rgba(255,255,255,0.7)' }}
                      onClick={() => setUserOpen(true)}
                    >
                      <ManageAccountsIcon fontSize="small" />
                    </IconButton>
                  </span>
                </Tooltip>
              )}
              <Tooltip title={t('logout_tooltip')}>
                <span>
                  <IconButton
                    size="small"
                    sx={{ color: 'rgba(255,255,255,0.7)' }}
                    onClick={() => {
                      void logout();
                    }}
                  >
                    <LogoutIcon />
                  </IconButton>
                </span>
              </Tooltip>
            </Box>
          </Box>

      <Typography variant="subtitle1" sx={{ fontWeight: 800, pl: 1, mb: 1, textAlign: 'left' }}>
        {t('devices')}
      </Typography>

      {/* Список групп и камер */}
      <Box 
        sx={{ height: 'calc(100% - 80px)', overflowY: 'auto' }}
        onDragOver={(e) => e.preventDefault()}
      >
        {/* Рендерим группы */}
        {groups.map(group => (
          <Box
            key={group.id}
            onDragOver={handleDragOver}
            onDrop={(e) => handleDrop(e, group.id)}
          >
            {renderCameraGroup(group)}
          </Box>
        ))}
        
        {/* Рендерим камеры без группы */}
        <Box
          onDragOver={handleDragOver}
          onDrop={(e) => handleDrop(e, null)}
        >
          {renderCameraGroup(null)}
        </Box>
      </Box>

      {/* Диалоги */}
      <AddCameraDialog
        open={addOpen}
        onClose={() => { setAddOpen(false); setAddDialogData(null); }}
        onSave={handleAddCamera}
        initialData={addDialogData}
      />
      
      <CameraSearchDialog 
        open={searchOpen}
        onClose={() => setSearchOpen(false)}
        foundCameras={foundCameras}
        isDiscovering={isDiscovering}
        discoveryProgress={discoveryProgress}
        onStartDiscovery={filters => discoverCameras(filters?.interfaces)}
        onAddSelected={(camerasData) => {
          if (camerasData.length === 1) {
            const cameraData = camerasData[0];
            setAddDialogData({
              name: cameraData.name,
              ip: cameraData.ip,
              protocol: cameraData.protocol ?? 'onvif',
              port: cameraData.port ?? cameraData.detectedPort ?? 554,
              onvifPort: cameraData.onvifPort ?? cameraData.detectedPort ?? 80,
            });
            setAddOpen(true);
          } else {
            setSelectedBatchCameras(camerasData);
            setBatchAddOpen(true);
          }
          setSearchOpen(false);
        }}
      />

      <BatchAddCameraDialog
        open={batchAddOpen}
        onClose={() => setBatchAddOpen(false)}
        count={selectedBatchCameras.length}
        onAdd={async (credentials, onProgress) => {
          let tempId = nextId.current;
          let addedCount = 0;
          for (const cam of selectedBatchCameras) {
            const camera: Camera = {
              id: tempId++,
              name: cam.name || 'Camera',
              ip: cam.ip,
              protocol: cam.protocol || 'onvif',
              port: cam.port || cam.detectedPort || 554,
              user: credentials.user,
              pass: credentials.pass,
              path_hd: '',
              path_sd: '',
              status: 'offline',
              onvifPort: cam.onvifPort || cam.detectedPort || 80,
              groupId: null,
              streamUrl: '',
            };
            await addCamera(camera);
            addedCount++;
            if (onProgress) {
              onProgress(addedCount);
            }
          }
          setBatchAddOpen(false);
          setSelectedBatchCameras([]);
          showToast(t('batch_add_success', { count: addedCount }), 'success');
        }}
      />
      
      <Toast
        open={toast.open}
        message={toast.message}
        severity={toast.severity}
        onClose={hideToast}
      />
      
      <SettingsModal 
        isOpen={settingsOpen}
        onClose={() => setSettingsOpen(false)} 
      />
      

      {/* Диалог создания/редактирования группы */}
      <Dialog 
        open={addGroupOpen} 
        onClose={() => { setAddGroupOpen(false); setEditingGroup(null); setNewGroupName(''); }}
        TransitionComponent={Slide}
        slotProps={{
          transition: {
            direction: 'up',
          }
        }}
        PaperProps={{
          sx: {
            borderRadius: 3,
            background: 'linear-gradient(145deg, #2c3137 0%, #3a4047 100%)',
            border: '1px solid rgba(255,255,255,0.1)',
            boxShadow: '0 20px 40px rgba(0,0,0,0.4), 0 0 0 1px rgba(255,255,255,0.05)',
            minWidth: 400,
            maxWidth: 500,
            backdropFilter: 'blur(10px)',
          }
        }}
        BackdropProps={{
          sx: {
            backgroundColor: 'rgba(0,0,0,0.6)',
            backdropFilter: 'blur(4px)',
          }
        }}
      >
        <DialogTitle sx={{ 
          pb: 2, 
          pt: 3, 
          px: 3, 
          fontWeight: 600, 
          fontSize: '1.4rem', 
          color: '#fff',
          background: 'linear-gradient(90deg, #1976d2, #42a5f5)',
          WebkitBackgroundClip: 'text',
          WebkitTextFillColor: 'transparent',
          borderBottom: '1px solid rgba(255,255,255,0.1)',
          mb: 2
        }}>
          {editingGroup ? t('edit_group') : t('create_group')}
        </DialogTitle>
        
        <DialogContent sx={{ px: 3, pb: 3 }}>
          <TextField
            autoFocus
            margin="none"
            label={t('group_name')}
            fullWidth
            variant="outlined"
            value={newGroupName}
            onChange={(e) => setNewGroupName(e.target.value)}
            onKeyDown={(e) => {
              if (e.key === 'Enter') {
                if (editingGroup) {
                  handleRenameGroup(editingGroup, newGroupName);
                  setEditingGroup(null);
                } else {
                  handleCreateGroup();
                }
                setAddGroupOpen(false);
                setNewGroupName('');
              } else if (e.key === 'Escape') {
                setAddGroupOpen(false);
                setEditingGroup(null);
                setNewGroupName('');
              }
            }}
            sx={{
              '& .MuiOutlinedInput-root': {
                backgroundColor: 'rgba(255,255,255,0.05)',
                borderRadius: 2,
                '& fieldset': {
                  borderColor: 'rgba(255,255,255,0.2)',
                  borderWidth: 1,
                },
                '&:hover fieldset': {
                  borderColor: 'rgba(66,165,245,0.5)',
                },
                '&.Mui-focused fieldset': {
                  borderColor: '#42a5f5',
                  borderWidth: 2,
                },
                '& input': {
                  color: '#fff',
                  fontSize: '1.1rem',
                  padding: '12px 14px',
                }
              },
              '& .MuiInputLabel-root': {
                color: 'rgba(255,255,255,0.7)',
                '&.Mui-focused': {
                  color: '#42a5f5',
                }
              }
            }}
            placeholder={t('group_name_placeholder')}
          />
        </DialogContent>
        
        <DialogActions sx={{ 
          px: 3, 
          pb: 3, 
          pt: 1,
          gap: 1,
          justifyContent: 'flex-end'
        }}>
          <Button 
            onClick={() => { setAddGroupOpen(false); setEditingGroup(null); setNewGroupName(''); }}
            sx={{ 
              borderRadius: 2,
              textTransform: 'none',
              fontSize: '0.95rem',
              fontWeight: 500,
              px: 3,
              py: 1,
              color: 'rgba(255,255,255,0.7)',
              border: '1px solid rgba(255,255,255,0.2)',
              '&:hover': {
                backgroundColor: 'rgba(255,255,255,0.05)',
                borderColor: 'rgba(255,255,255,0.3)',
                color: '#fff'
              },
              transition: 'all 0.2s ease-in-out'
            }}
          >
            {t('cancel')}
          </Button>
          
          <Button 
            onClick={() => {
              if (editingGroup) {
                handleRenameGroup(editingGroup, newGroupName);
                setEditingGroup(null);
              } else {
                handleCreateGroup();
              }
              setAddGroupOpen(false);
              setNewGroupName('');
            }}
            disabled={!newGroupName.trim()}
            sx={{
              borderRadius: 2,
              textTransform: 'none',
              fontSize: '0.95rem',
              fontWeight: 600,
              px: 3,
              py: 1,
              background: 'linear-gradient(45deg, #1976d2, #42a5f5)',
              color: '#fff',
              border: 'none',
              position: 'relative',
              overflow: 'hidden',
              '&:hover': {
                background: 'linear-gradient(45deg, #1565c0, #1976d2)',
                transform: 'translateY(-1px)',
                boxShadow: '0 4px 12px rgba(25,118,210,0.4)',
              },
              '&:active': {
                transform: 'translateY(0)',
              },
              '&:disabled': {
                background: 'rgba(255,255,255,0.1)',
                color: 'rgba(255,255,255,0.3)',
                transform: 'none',
                boxShadow: 'none',
              },
              '&::before': {
                content: '""',
                position: 'absolute',
                top: 0,
                left: '-100%',
                width: '100%',
                height: '100%',
                background: 'linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent)',
                transition: 'left 0.5s ease-in-out',
              },
              '&:hover::before': {
                left: '100%',
              },
              transition: 'all 0.2s ease-in-out'
            }}
          >
            {editingGroup ? t('save') : t('create')}
          </Button>
        </DialogActions>
      </Dialog>
      {/* Архив камеры */}
      {showArchive && (
        <ArchiveImproved
          cameras={cameras}
          onClose={() => setShowArchive(false)}
        />
      )}
      {canManageUsers && (
        <UserDialog isOpen={userOpen} onClose={() => setUserOpen(false)} />
      )}
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
      <AnalyticsModal
        open={analyticsOpen}
        onClose={() => setAnalyticsOpen(false)}
      />
        </>
      )}
    </Paper>
  );
};

export default DevicePanel;