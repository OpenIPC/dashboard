import React, { useCallback, useEffect, useMemo, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  Button,
  Box,
  List,
  ListItemButton,
  ListItemIcon,
  ListItemText,
  CircularProgress,
  Typography,
  Breadcrumbs,
  Link,
  Snackbar,
  Alert,
  IconButton,
  Tooltip,
} from '@mui/material';
import {
  Folder as FolderIcon,
  InsertDriveFile as FileIcon,
  CloudDownload as DownloadIcon,
  CloudUpload as UploadIcon,
  ArrowUpward as UpIcon,
  Refresh as RefreshIcon,
  Home as HomeIcon,
  Computer as ComputerIcon,
  Cloud as CloudIcon,
  Close as CloseIcon,
  Delete as DeleteIcon,
  FolderOpen as FolderOpenIcon,
} from '@mui/icons-material';
import { homeDir, dirname, join } from '@tauri-apps/api/path';
import type { Camera } from '../types';
import { useLocalization } from '../contexts/LocalizationContext';

interface FileManagerProps {
  open: boolean;
  camera: Camera | null;
  onClose: () => void;
}

interface RemoteEntry {
  name: string;
  isDir: boolean;
  size: number;
  modified?: number | null;
  permissions?: number | null;
}

interface LocalEntry {
  name: string;
  path: string;
  isDir: boolean;
  size: number;
  modified?: number;
}

interface LocalFsEntryResponse {
  name: string;
  path: string;
  isDir: boolean;
  size?: number;
  modified?: number | null;
}

type ToastState = {
  message: string;
  severity: 'success' | 'error' | 'info';
};

type TransportMode = 'scp' | 'sftp';

const LOCAL_DRAG_TYPE = 'application/x-local-entry';
const REMOTE_DRAG_TYPE = 'application/x-remote-entry';

const sanitizeRemotePath = (value: string): string =>
  `/${value.replace(/^\/+/g, '').replace(/\/+$/g, '')}`;

const joinRemotePath = (base: string, name: string): string => {
  if (!name) {
    return base || '/';
  }

  const normalizedBase = !base || base === '/' ? '/' : sanitizeRemotePath(base);
  if (normalizedBase === '/') {
    return `/${name.replace(/^\/+/, '')}`;
  }

  return `${normalizedBase.replace(/\/+$/g, '')}/${name.replace(/^\/+/, '')}`;
};

const getParentPath = (path: string): string => {
  const normalized = path.replace(/\/+$/g, '');
  if (!normalized || normalized === '/') {
    return '/';
  }
  const parts = normalized.split('/').filter(Boolean);
  parts.pop();
  return parts.length === 0 ? '/' : `/${parts.join('/')}`;
};

const formatSize = (bytes: number): string => {
  if (!Number.isFinite(bytes) || bytes <= 0) {
    return '0 B';
  }
  const units = ['B', 'KB', 'MB', 'GB', 'TB'];
  const order = Math.min(Math.floor(Math.log(bytes) / Math.log(1024)), units.length - 1);
  const value = bytes / 1024 ** order;
  const fractionDigits = order === 0 ? 0 : value >= 10 ? 0 : 1;
  return `${value.toFixed(fractionDigits)} ${units[order]}`;
};

const formatDate = (timestamp?: number | null): string => {
  if (!timestamp || Number.isNaN(timestamp)) {
    return '';
  }
  const date = new Date(timestamp * 1000);
  if (Number.isNaN(date.getTime())) {
    return '';
  }
  return date.toLocaleString();
};

const getErrorMessage = (err: unknown): string => {
  if (!err) {
    return 'Unknown error';
  }
  if (typeof err === 'string') {
    return err;
  }
  if (err instanceof Error) {
    return err.message;
  }
  try {
    return JSON.stringify(err);
  } catch {
    return String(err);
  }
};

const FileManager: React.FC<FileManagerProps> = ({ open, camera, onClose }) => {
  const { t } = useLocalization();
  const [remoteEntries, setRemoteEntries] = useState<RemoteEntry[]>([]);
  const [remotePath, setRemotePath] = useState<string>('/');
  const [remoteLoading, setRemoteLoading] = useState(false);
  const [remoteError, setRemoteError] = useState<string | null>(null);
  const [remoteSelected, setRemoteSelected] = useState<string | null>(null);

  const [localEntries, setLocalEntries] = useState<LocalEntry[]>([]);
  const [localPath, setLocalPath] = useState<string>('');
  const [localHome, setLocalHome] = useState<string>('');
  const [localLoading, setLocalLoading] = useState(false);
  const [localError, setLocalError] = useState<string | null>(null);
  const [localSelected, setLocalSelected] = useState<string | null>(null);

  const [transport, setTransport] = useState<TransportMode>('scp');
  const [transferBusy, setTransferBusy] = useState(false);
  const [toast, setToast] = useState<ToastState | null>(null);

  const remoteSegments = useMemo(() => {
    const crumbs: Array<{ label: string; path: string }> = [{ label: '/', path: '/' }];
    if (!remotePath || remotePath === '/') {
      return crumbs;
    }

    const normalized = remotePath.replace(/^\/+/g, '').replace(/\/+$/g, '');
    let current = '';

    for (const part of normalized.split('/')) {
      if (!part) {
        continue;
      }

      current = `${current}/${part}`;
      crumbs.push({ label: part, path: current });
    }

    return crumbs;
  }, [remotePath]);

  const localSegments = useMemo(() => {
    if (!localPath) {
      return [] as Array<{ label: string; path: string }>;
    }

    const crumbs: Array<{ label: string; path: string }> = [];
    const normalized = localPath.replace(/\\/g, '/');
    const parts = normalized.split('/').filter(Boolean);

    if (/^[A-Za-z]:/.test(localPath)) {
      const sep = '\\';
      if (parts.length > 0) {
        let current = `${parts[0]}${sep}`;
        crumbs.push({ label: parts[0], path: current });
        for (let index = 1; index < parts.length; index += 1) {
          const part = parts[index];
          if (!current.endsWith(sep)) {
            current = `${current}${sep}`;
          }
          current = `${current}${part}`;
          crumbs.push({ label: part, path: current });
        }
      }
      return crumbs;
    }

    crumbs.push({ label: '/', path: '/' });
    let current = '/';
    for (const part of parts) {
      current = current === '/' ? `/${part}` : `${current}/${part}`;
      crumbs.push({ label: part, path: current });
    }
    return crumbs;
  }, [localPath]);

  const switchTransport = useCallback(
    (mode: TransportMode, reason: 'fallback' | 'restored') => {
      setTransport((current) => {
        if (current === mode) {
          return current;
        }
        const severity = reason === 'restored' ? 'success' : 'info';
        setToast({
          message:
            reason === 'fallback'
              ? t('fileManager.toast.scpFallback')
              : t('fileManager.toast.scpRestored'),
          severity,
        });
        return mode;
      });
    },
    [t]
  );

  const withTransport = useCallback(
    async (handler: (mode: TransportMode) => Promise<unknown>) => {
      const preferred: TransportMode[] = transport === 'scp' ? ['scp', 'sftp'] : ['sftp', 'scp'];
      const errors: Partial<Record<TransportMode, unknown>> = {};

      for (const mode of preferred) {
        console.debug('[FileManager] transport attempt', { mode });
        try {
          const result = await handler(mode);
          if (mode !== transport) {
            switchTransport(mode, mode === 'scp' ? 'restored' : 'fallback');
          }
          console.debug('[FileManager] transport success', {
            mode,
            restored: mode !== transport,
          });
          return { mode, result };
        } catch (err) {
          console.warn('[FileManager] transport failure', { mode, err });
          errors[mode] = err;
        }
      }

      const messages: string[] = [];
      if (errors.scp) {
        messages.push(
          t('fileManager.errors.scp', { message: getErrorMessage(errors.scp) })
        );
      }
      if (errors.sftp) {
        messages.push(
          t('fileManager.errors.sftp', { message: getErrorMessage(errors.sftp) })
        );
      }

      const combined = messages.join(' | ');
      throw new Error(combined || 'SCP/SFTP operation failed');
    },
    [switchTransport, t, transport]
  );

  const loadRemote = useCallback(
    async (target: string) => {
      if (!camera) {
        setRemoteEntries([]);
        setRemotePath('/');
        return;
      }

      const normalizedTarget = !target || target === '/' ? '/' : sanitizeRemotePath(target);

      setRemoteLoading(true);
      setRemoteError(null);
      console.debug('[FileManager] loadRemote start', {
        camera: camera?.ip ?? null,
        target: normalizedTarget,
        preferredTransport: transport,
      });

      try {
        const { result } = await withTransport((mode) => {
          const command = mode === 'scp' ? 'camera_scp_list' : 'camera_sftp_list';
          console.debug('[FileManager] invoking remote list', {
            command,
            mode,
            target: normalizedTarget,
          });
          return invoke<RemoteEntry[]>(command, {
            host: camera.ip,
            path: normalizedTarget,
            username: camera.user || null,
            password_plain: camera.pass || null,
            password_enc: camera.pass_enc || null,
            port: camera.port ?? null,
          });
        });

        const rawEntries = Array.isArray(result) ? (result as RemoteEntry[]) : [];
        const mapped = rawEntries.map((entry) => ({
          name: entry.name,
          isDir: entry.isDir,
          size: Number(entry.size ?? 0),
          modified:
            typeof entry.modified === 'number'
              ? entry.modified
              : entry.modified === null || entry.modified === undefined
              ? null
              : Number(entry.modified) || null,
          permissions: typeof entry.permissions === 'number' ? entry.permissions : null,
        }));

        mapped.sort((a, b) => {
          if (a.isDir && !b.isDir) return -1;
          if (!a.isDir && b.isDir) return 1;
          return a.name.toLowerCase().localeCompare(b.name.toLowerCase());
        });

        setRemoteEntries(mapped);
        setRemotePath(normalizedTarget);
        setRemoteSelected(null);
        console.debug('[FileManager] loadRemote success', {
          target: normalizedTarget,
          entries: mapped.length,
        });
      } catch (err) {
        setRemoteEntries([]);
        setRemoteError(getErrorMessage(err));
        console.error('[FileManager] loadRemote error', {
          target: normalizedTarget,
          error: err,
        });
      } finally {
        setRemoteLoading(false);
        console.debug('[FileManager] loadRemote done', { target: normalizedTarget });
      }
    },
    [camera, withTransport]
  );

  const loadLocal = useCallback(async (target: string) => {
    if (!target) {
      return;
    }

    setLocalLoading(true);
    setLocalError(null);

    try {
      const response = await invoke<LocalFsEntryResponse[] | null>('local_fs_list', {
        path: target,
      });

      const mapped = (Array.isArray(response) ? response : []).map((entry) => {
        if (!entry) {
          return null;
        }

        const rawName = entry.name ?? '';
        const pathValue = entry.path ?? '';
        const fallbackName = pathValue.split(/[\\/]/).pop() ?? '';
        const name = rawName || fallbackName;
        if (!name || !pathValue) {
          return null;
        }

        const rawModified = entry.modified;
        let normalizedModified: number | undefined;

        if (typeof rawModified === 'number' && Number.isFinite(rawModified)) {
          normalizedModified = Math.floor(rawModified);
        } else if (typeof rawModified === 'string' && rawModified) {
          const numeric = Number(rawModified);
          if (Number.isFinite(numeric)) {
            normalizedModified = Math.floor(numeric);
          }
        }

        return {
          name,
          path: pathValue,
          isDir: Boolean(entry.isDir),
          size: Number(entry.size ?? 0),
          modified: normalizedModified,
        } as LocalEntry;
      });

      const filtered: LocalEntry[] = mapped.filter((item): item is LocalEntry => Boolean(item));

      filtered.sort((a: LocalEntry, b: LocalEntry) => {
        if (a.isDir && !b.isDir) return -1;
        if (!a.isDir && b.isDir) return 1;
        return a.name.toLowerCase().localeCompare(b.name.toLowerCase());
      });

      setLocalEntries(filtered);
      setLocalPath(target);
      setLocalSelected(null);
    } catch (err) {
      setLocalEntries([]);
      setLocalError(getErrorMessage(err));
    } finally {
      setLocalLoading(false);
    }
  }, []);

  useEffect(() => {
    if (!open) {
      return;
    }

    if (!camera) {
      setRemoteEntries([]);
      setRemotePath('/');
      setRemoteSelected(null);
      return;
    }

    setRemoteSelected(null);
    void loadRemote('/');
  }, [open, camera, loadRemote]);

  useEffect(() => {
    if (!open) {
      return;
    }

    let cancelled = false;

    (async () => {
      try {
        const home = await homeDir();
        if (cancelled) {
          return;
        }
        setLocalHome(home);
        await loadLocal(home);
      } catch (err) {
        if (!cancelled) {
          setLocalEntries([]);
          setLocalError(getErrorMessage(err));
        }
      }
    })();

    return () => {
      cancelled = true;
    };
  }, [open, loadLocal]);

  useEffect(() => {
    setTransport('scp');
  }, [camera?.id]);

  useEffect(() => {
    if (!open) {
      setToast(null);
    }
  }, [open]);

  const remoteSelectedEntry = useMemo(
    () => remoteEntries.find((entry) => entry.name === remoteSelected) ?? null,
    [remoteEntries, remoteSelected]
  );

  const localSelectedEntry = useMemo(
    () => localEntries.find((entry) => entry.name === localSelected) ?? null,
    [localEntries, localSelected]
  );

  const canDownload = Boolean(
    camera && localPath && remoteSelectedEntry && !remoteSelectedEntry.isDir && !transferBusy
  );
  const canUpload = Boolean(
    camera && remotePath && localSelectedEntry && !localSelectedEntry.isDir && !transferBusy
  );
  const canDeleteRemote = useMemo(() => {
    if (!camera || !remoteSelectedEntry || transferBusy) {
      return false;
    }
    const fullPath = joinRemotePath(remotePath, remoteSelectedEntry.name);
    return fullPath !== '/';
  }, [camera, remotePath, remoteSelectedEntry, transferBusy]);
  const canDeleteLocal = Boolean(localSelectedEntry && !transferBusy);
  const canRevealLocal = Boolean(localSelectedEntry && !transferBusy);

  const downloadFile = useCallback(
    async (remoteFullPath: string, localFullPath: string, displayName: string) => {
      if (!camera) {
        return;
      }

      setTransferBusy(true);
      try {
        await withTransport((mode) => {
          const command = mode === 'scp' ? 'camera_scp_download' : 'camera_sftp_download';
          return invoke<boolean>(command, {
            host: camera.ip,
            remotePath: remoteFullPath,
            localPath: localFullPath,
            username: camera.user || null,
            passwordPlain: camera.pass || null,
            passwordEnc: camera.pass_enc || null,
            port: camera.port ?? null,
          });
        });

        setToast({
          message: t('fileManager.downloadSuccess', { name: displayName }),
          severity: 'success',
        });

        const targetDir = localPath || (await dirname(localFullPath));
        if (targetDir) {
          await loadLocal(targetDir);
        }
      } catch (err) {
        setToast({
          message: getErrorMessage(err),
          severity: 'error',
        });
      } finally {
        setTransferBusy(false);
      }
    },
    [camera, loadLocal, localPath, t, withTransport]
  );

  const uploadFile = useCallback(
    async (localFullPath: string, fileName: string) => {
      if (!camera) {
        return;
      }

      const remoteFullPath = joinRemotePath(remotePath, fileName);

      setTransferBusy(true);
      try {
        await withTransport((mode) => {
          const command = mode === 'scp' ? 'camera_scp_upload' : 'camera_sftp_upload';
          return invoke<boolean>(command, {
            host: camera.ip,
            remotePath: remoteFullPath,
            localPath: localFullPath,
            username: camera.user || null,
            passwordPlain: camera.pass || null,
            passwordEnc: camera.pass_enc || null,
            port: camera.port ?? null,
          });
        });

        setToast({
          message: t('fileManager.uploadSuccess', { name: fileName }),
          severity: 'success',
        });

        await loadRemote(remotePath);
      } catch (err) {
        setToast({
          message: getErrorMessage(err),
          severity: 'error',
        });
      } finally {
        setTransferBusy(false);
      }
    },
    [camera, loadRemote, remotePath, t, withTransport]
  );

  const handleDownloadClick = useCallback(async () => {
    if (!remoteSelectedEntry || remoteSelectedEntry.isDir || !localPath) {
      return;
    }

    const remoteFullPath = joinRemotePath(remotePath, remoteSelectedEntry.name);
    const localFullPath = await join(localPath, remoteSelectedEntry.name);
    await downloadFile(remoteFullPath, localFullPath, remoteSelectedEntry.name);
  }, [downloadFile, localPath, remotePath, remoteSelectedEntry]);

  const handleUploadClick = useCallback(async () => {
    if (!localSelectedEntry || localSelectedEntry.isDir) {
      return;
    }

    await uploadFile(localSelectedEntry.path, localSelectedEntry.name);
  }, [localSelectedEntry, uploadFile]);

  const handleRemoteDelete = useCallback(async () => {
    if (!camera || !remoteSelectedEntry) {
      return;
    }

    const entry = remoteSelectedEntry;
    const remoteFullPath = joinRemotePath(remotePath, entry.name);
    if (!remoteFullPath || remoteFullPath === '/') {
      return;
    }

    setTransferBusy(true);
    try {
      await invoke<boolean>('camera_remote_delete', {
        host: camera.ip,
        remotePath: remoteFullPath,
        isDir: entry.isDir,
        username: camera.user || null,
        passwordPlain: camera.pass || null,
        passwordEnc: camera.pass_enc || null,
        port: camera.port ?? null,
      });

      setToast({
        message: t('fileManager.deleteRemoteSuccess', { name: entry.name }),
        severity: 'success',
      });

      await loadRemote(remotePath);
    } catch (err) {
      setToast({
        message: t('fileManager.deleteRemoteError', { message: getErrorMessage(err) }),
        severity: 'error',
      });
    } finally {
      setTransferBusy(false);
    }
  }, [camera, loadRemote, remotePath, remoteSelectedEntry, t]);

  const handleLocalDelete = useCallback(async () => {
    if (!localSelectedEntry) {
      return;
    }

    const entry = localSelectedEntry;
    setTransferBusy(true);
    try {
      await invoke<boolean>('local_fs_delete', {
        path: entry.path,
      });

      setToast({
        message: t('fileManager.deleteLocalSuccess', { name: entry.name }),
        severity: 'success',
      });

      const targetDir = localPath || (await dirname(entry.path));
      await loadLocal(targetDir);
    } catch (err) {
      setToast({
        message: t('fileManager.deleteLocalError', { message: getErrorMessage(err) }),
        severity: 'error',
      });
    } finally {
      setTransferBusy(false);
    }
  }, [dirname, loadLocal, localPath, localSelectedEntry, t]);

  const handleRevealLocal = useCallback(async () => {
    if (!localSelectedEntry) {
      return;
    }

    const entry = localSelectedEntry;
    try {
      await invoke<boolean>('local_reveal_path', {
        path: entry.path,
      });

      setToast({
        message: t('fileManager.revealSuccess', { name: entry.name }),
        severity: 'info',
      });
    } catch (err) {
      setToast({
        message: t('fileManager.revealError', { message: getErrorMessage(err) }),
        severity: 'error',
      });
    }
  }, [localSelectedEntry, t]);

  const handleRemoteDoubleClick = useCallback(
    (entry: RemoteEntry) => {
      if (entry.isDir) {
        const nextPath = joinRemotePath(remotePath, entry.name);
        void loadRemote(nextPath);
      }
    },
    [loadRemote, remotePath]
  );

  const handleLocalDoubleClick = useCallback(
    (entry: LocalEntry) => {
      if (entry.isDir) {
        void loadLocal(entry.path);
      }
    },
    [loadLocal]
  );

  const handleRemoteUp = useCallback(() => {
    const parent = getParentPath(remotePath);
    if (parent !== remotePath) {
      void loadRemote(parent);
    }
  }, [loadRemote, remotePath]);

  const handleLocalUp = useCallback(async () => {
    if (!localPath) {
      return;
    }

    try {
      const parent = await dirname(localPath);
      if (parent && parent !== localPath) {
        await loadLocal(parent);
      }
    } catch {
      // ignore
    }
  }, [localPath, loadLocal]);

  const handleRemoteDragStart = useCallback(
    (event: React.DragEvent<HTMLDivElement>, entry: RemoteEntry) => {
      if (transferBusy || entry.isDir) {
        event.preventDefault();
        return;
      }

      const remoteFullPath = joinRemotePath(remotePath, entry.name);
      event.dataTransfer.setData(
        REMOTE_DRAG_TYPE,
        JSON.stringify({ path: remoteFullPath, name: entry.name })
      );
      event.dataTransfer.effectAllowed = 'copy';
    },
    [remotePath, transferBusy]
  );

  const handleLocalDragStart = useCallback(
    (event: React.DragEvent<HTMLDivElement>, entry: LocalEntry) => {
      if (transferBusy || entry.isDir) {
        event.preventDefault();
        return;
      }

      event.dataTransfer.setData(
        LOCAL_DRAG_TYPE,
        JSON.stringify({ path: entry.path, name: entry.name })
      );
      event.dataTransfer.effectAllowed = 'copy';
    },
    [transferBusy]
  );

  const handleRemoteDragOver = useCallback((event: React.DragEvent<HTMLDivElement>) => {
    if (Array.from(event.dataTransfer.types).includes(LOCAL_DRAG_TYPE)) {
      event.preventDefault();
      event.dataTransfer.dropEffect = 'copy';
    }
  }, []);

  const handleLocalDragOver = useCallback((event: React.DragEvent<HTMLDivElement>) => {
    if (Array.from(event.dataTransfer.types).includes(REMOTE_DRAG_TYPE)) {
      event.preventDefault();
      event.dataTransfer.dropEffect = 'copy';
    }
  }, []);

  const handleRemoteDrop = useCallback(
    async (event: React.DragEvent<HTMLDivElement>) => {
      if (!Array.from(event.dataTransfer.types).includes(LOCAL_DRAG_TYPE)) {
        return;
      }

      event.preventDefault();

      if (transferBusy) {
        return;
      }

      const payload = event.dataTransfer.getData(LOCAL_DRAG_TYPE);
      if (!payload) {
        return;
      }

      try {
        const data = JSON.parse(payload) as { path: string; name: string; isDir?: boolean };
        if (data.isDir) {
          setToast({
            message: t('fileManager.transfer.folderUnsupported'),
            severity: 'info',
          });
          return;
        }
        await uploadFile(data.path, data.name);
      } catch (err) {
        setToast({
          message: getErrorMessage(err),
          severity: 'error',
        });
      }
    },
    [t, transferBusy, uploadFile]
  );

  const handleLocalDrop = useCallback(
    async (event: React.DragEvent<HTMLDivElement>) => {
      if (!Array.from(event.dataTransfer.types).includes(REMOTE_DRAG_TYPE)) {
        return;
      }

      event.preventDefault();

      if (transferBusy) {
        return;
      }

      if (!localPath) {
        setToast({
          message: t('fileManager.selectLocalFolderPrompt'),
          severity: 'info',
        });
        return;
      }

      const payload = event.dataTransfer.getData(REMOTE_DRAG_TYPE);
      if (!payload) {
        return;
      }

      try {
        const data = JSON.parse(payload) as { path: string; name: string; isDir?: boolean };
        if (data.isDir) {
          setToast({
            message: t('fileManager.transfer.folderUnsupported'),
            severity: 'info',
          });
          return;
        }

        const localFullPath = await join(localPath, data.name);
        await downloadFile(data.path, localFullPath, data.name);
      } catch (err) {
        setToast({
          message: getErrorMessage(err),
          severity: 'error',
        });
      }
    },
    [downloadFile, localPath, t, transferBusy]
  );

  const handleRemoteHome = useCallback(() => {
    void loadRemote('/');
  }, [loadRemote]);

  const handleLocalHome = useCallback(() => {
    if (localHome) {
      void loadLocal(localHome);
    }
  }, [localHome, loadLocal]);

  const handleToastClose = useCallback(
    (_event?: React.SyntheticEvent | Event, reason?: string) => {
      if (reason === 'clickaway') {
        return;
      }
      setToast(null);
    },
    []
  );

  const handleClose = useCallback(() => {
    if (!transferBusy) {
      onClose();
    }
  }, [onClose, transferBusy]);

  const dialogTitle = camera
    ? t('fileManager.titleWithCamera', { name: camera.name })
    : t('fileManager.title');
  const transportLabel = t('fileManager.transportMode', { mode: transport.toUpperCase() });

  return (
    <Dialog
      open={open}
      onClose={handleClose}
      fullWidth
      maxWidth="lg"
      keepMounted={false}
      PaperProps={{
        sx: {
          width: '100%',
          maxWidth: 1100,
          height: { xs: '120vh', md: '85vh' },
          maxHeight: '120vh',
          display: 'flex',
          flexDirection: 'column',
        },
      }}
    >
      <DialogTitle
        sx={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          gap: 1,
        }}
      >
        <Box display="flex" alignItems="center" gap={1} minWidth={0}>
          <Typography variant="h6" component="span" noWrap>
            {dialogTitle}
          </Typography>
        </Box>
        <IconButton size="small" onClick={handleClose}>
          <CloseIcon fontSize="small" />
        </IconButton>
      </DialogTitle>
      <DialogContent
        sx={{
          flex: 1,
          display: 'flex',
          flexDirection: 'column',
          p: 0,
          minWidth: 900,
          overflow: 'hidden',
        }}
      >
        <Box sx={{ display: 'flex', flexDirection: 'column', height: '100%' }}>
          <Box
            sx={{
              position: 'sticky',
              top: 0,
              zIndex: 2,
              backgroundColor: (theme) => theme.palette.background.paper,
              borderBottom: (theme) => `1px solid ${theme.palette.divider}`,
              px: 2,
              py: 1,
            }}
          >
            <Box
              sx={{
                display: 'grid',
                gridTemplateColumns: { xs: '1fr', md: 'repeat(2, minmax(0, 1fr))' },
                gap: 2,
              }}
            >
              <Box sx={{ minWidth: 0 }}>
                <Box display="flex" alignItems="center" gap={1} mb={1}>
                  <CloudIcon color="primary" fontSize="small" />
                  <Typography variant="subtitle1">{t('fileManager.remoteAccess')}</Typography>
                  <Typography variant="caption" color="text.secondary">
                    {transportLabel}
                  </Typography>
                  <Box flexGrow={1} />
                  <Tooltip title={t('fileManager.tooltip.root')}>
                    <span>
                      <IconButton
                        size="small"
                        onClick={handleRemoteHome}
                        disabled={!camera || remoteLoading || transferBusy}
                      >
                        <HomeIcon fontSize="small" />
                      </IconButton>
                    </span>
                  </Tooltip>
                  <Tooltip title={t('fileManager.tooltip.up')}>
                    <span>
                      <IconButton
                        size="small"
                        onClick={handleRemoteUp}
                        disabled={!camera || remoteLoading || transferBusy || remotePath === '/'}
                      >
                        <UpIcon fontSize="small" />
                      </IconButton>
                    </span>
                  </Tooltip>
                  <Tooltip title={t('fileManager.tooltip.refresh')}>
                    <span>
                      <IconButton
                        size="small"
                        onClick={() => void loadRemote(remotePath)}
                        disabled={!camera || remoteLoading}
                      >
                        <RefreshIcon fontSize="small" />
                      </IconButton>
                    </span>
                  </Tooltip>
                </Box>

                <Breadcrumbs separator=">" maxItems={4}>
                  {remoteSegments.map((segment, index) => {
                    const isLast = index === remoteSegments.length - 1;
                    if (isLast) {
                      return (
                        <Typography key={segment.path} variant="body2" color="text.primary">
                          {segment.label}
                        </Typography>
                      );
                    }
                    return (
                      <Link
                        key={segment.path}
                        component="button"
                        color="inherit"
                        underline="hover"
                        variant="body2"
                        onClick={() => void loadRemote(segment.path)}
                      >
                        {segment.label}
                      </Link>
                    );
                  })}
                </Breadcrumbs>
              </Box>

              <Box sx={{ minWidth: 0 }}>
                <Box display="flex" alignItems="center" gap={1} mb={1}>
                  <ComputerIcon color="primary" fontSize="small" />
                  <Typography variant="subtitle1">{t('fileManager.localComputer')}</Typography>
                  <Box flexGrow={1} />
                  <Tooltip title={t('fileManager.tooltip.home')}>
                    <span>
                      <IconButton
                        size="small"
                        onClick={handleLocalHome}
                        disabled={localLoading || transferBusy || !localHome}
                      >
                        <HomeIcon fontSize="small" />
                      </IconButton>
                    </span>
                  </Tooltip>
                  <Tooltip title={t('fileManager.tooltip.up')}>
                    <span>
                      <IconButton
                        size="small"
                        onClick={() => void handleLocalUp()}
                        disabled={localLoading || transferBusy || !localPath}
                      >
                        <UpIcon fontSize="small" />
                      </IconButton>
                    </span>
                  </Tooltip>
                  <Tooltip title={t('fileManager.tooltip.refresh')}>
                    <span>
                      <IconButton
                        size="small"
                        onClick={() => {
                          if (localPath) {
                            void loadLocal(localPath);
                          }
                        }}
                        disabled={localLoading || !localPath}
                      >
                        <RefreshIcon fontSize="small" />
                      </IconButton>
                    </span>
                  </Tooltip>
                </Box>

                {localSegments.length > 0 ? (
                  <Breadcrumbs separator=">" maxItems={4}>
                    {localSegments.map((segment, index) => {
                      const isLast = index === localSegments.length - 1;
                      if (isLast) {
                        return (
                          <Typography key={segment.path} variant="body2" color="text.primary">
                            {segment.label}
                          </Typography>
                        );
                      }
                      return (
                        <Link
                          key={segment.path}
                          component="button"
                          color="inherit"
                          underline="hover"
                          variant="body2"
                          onClick={() => void loadLocal(segment.path)}
                        >
                          {segment.label}
                        </Link>
                      );
                    })}
                  </Breadcrumbs>
                ) : (
                  <Typography variant="body2" color="text.secondary">
                    {localPath || t('fileManager.selectFolder')}
                  </Typography>
                )}
              </Box>
            </Box>
          </Box>

          <Box
            sx={{
              flex: 1,
              display: 'grid',
              gridTemplateColumns: { xs: '1fr', md: 'repeat(2, minmax(0, 1fr))' },
              gap: 2,
              px: 2,
              py: 2,
              overflow: 'hidden',
            }}
          >
            <Box display="flex" flexDirection="column" minHeight={0} sx={{ minWidth: 0 }}>
              <Box
                sx={{
                  flexGrow: 1,
                  border: (theme) => `1px solid ${theme.palette.divider}`,
                  borderRadius: 1,
                  overflow: 'auto',
                  position: 'relative',
                }}
                onDragOver={handleRemoteDragOver}
                onDrop={handleRemoteDrop}
              >
                {!camera ? (
                  <Box
                    height="100%"
                    display="flex"
                    alignItems="center"
                    justifyContent="center"
                    px={2}
                    textAlign="center"
                  >
                    <Typography variant="body2" color="text.secondary">
                      {t('fileManager.noCameraSelected')}
                    </Typography>
                  </Box>
                ) : remoteLoading ? (
                  <Box height="100%" display="flex" alignItems="center" justifyContent="center">
                    <CircularProgress size={32} />
                  </Box>
                ) : remoteError ? (
                  <Box p={2}>
                    <Alert severity="error">{remoteError}</Alert>
                  </Box>
                ) : remoteEntries.length === 0 ? (
                  <Box height="100%" display="flex" alignItems="center" justifyContent="center">
                    <Typography variant="body2" color="text.secondary">
                      {t('fileManager.emptyFolder')}
                    </Typography>
                  </Box>
                ) : (
                  <List dense disablePadding>
                    {remoteEntries.map((entry) => {
                      const secondaryParts: string[] = [];
                      if (entry.isDir) {
                        secondaryParts.push(t('fileManager.entryTypeFolder'));
                      } else {
                        secondaryParts.push(formatSize(entry.size));
                      }
                      const dateText = formatDate(entry.modified);
                      if (dateText) {
                        secondaryParts.push(dateText);
                      }
                      return (
                        <ListItemButton
                          key={joinRemotePath(remotePath, entry.name)}
                          component="div"
                          selected={remoteSelected === entry.name}
                          onClick={() => setRemoteSelected(entry.name)}
                          onDoubleClick={() => handleRemoteDoubleClick(entry)}
                          draggable={!entry.isDir && !transferBusy}
                          onDragStart={(event) => handleRemoteDragStart(event, entry)}
                          sx={{ borderRadius: 0.5, mx: 0.5, my: 0.25 }}
                        >
                          <ListItemIcon sx={{ minWidth: 32 }}>
                            {entry.isDir ? (
                              <FolderIcon fontSize="small" />
                            ) : (
                              <FileIcon fontSize="small" />
                            )}
                          </ListItemIcon>
                          <ListItemText
                            primary={entry.name}
                            primaryTypographyProps={{ noWrap: true }}
                            secondary={secondaryParts.join(' - ')}
                            secondaryTypographyProps={{ noWrap: true }}
                          />
                        </ListItemButton>
                      );
                    })}
                  </List>
                )}
              </Box>

              <Box display="flex" justifyContent="space-between" alignItems="center" mt={1}>
                {transferBusy ? (
                  <Typography variant="caption" color="text.secondary">
                    {t('fileManager.transferInProgress')}
                  </Typography>
                ) : (
                  <span />
                )}
                <Box display="flex" gap={1}>
                  <Button
                    variant="outlined"
                    color="error"
                    startIcon={<DeleteIcon />}
                    onClick={() => void handleRemoteDelete()}
                    disabled={!canDeleteRemote}
                  >
                    {t('fileManager.deleteAction')}
                  </Button>
                  <Button
                    variant="contained"
                    startIcon={<DownloadIcon />}
                    onClick={() => void handleDownloadClick()}
                    disabled={!canDownload}
                  >
                    {t('fileManager.downloadAction')}
                  </Button>
                </Box>
              </Box>
            </Box>

            <Box display="flex" flexDirection="column" minHeight={0} sx={{ minWidth: 0 }}>
              <Box
                sx={{
                  flexGrow: 1,
                  border: (theme) => `1px solid ${theme.palette.divider}`,
                  borderRadius: 1,
                  overflow: 'auto',
                  position: 'relative',
                }}
                onDragOver={handleLocalDragOver}
                onDrop={handleLocalDrop}
              >
                {localLoading ? (
                  <Box height="100%" display="flex" alignItems="center" justifyContent="center">
                    <CircularProgress size={32} />
                  </Box>
                ) : localError ? (
                  <Box p={2}>
                    <Alert severity="error">{localError}</Alert>
                  </Box>
                ) : localEntries.length === 0 ? (
                  <Box height="100%" display="flex" alignItems="center" justifyContent="center">
                    <Typography variant="body2" color="text.secondary">
                      {t('fileManager.emptyFolder')}
                    </Typography>
                  </Box>
                ) : (
                  <List dense disablePadding>
                    {localEntries.map((entry) => {
                      const secondaryParts: string[] = [];
                      if (entry.isDir) {
                        secondaryParts.push(t('fileManager.entryTypeFolder'));
                      } else {
                        secondaryParts.push(formatSize(entry.size));
                      }
                      if (entry.modified) {
                        const dateText = formatDate(entry.modified);
                        if (dateText) {
                          secondaryParts.push(dateText);
                        }
                      }
                      return (
                        <ListItemButton
                          key={entry.path}
                          component="div"
                          selected={localSelected === entry.name}
                          onClick={() => setLocalSelected(entry.name)}
                          onDoubleClick={() => handleLocalDoubleClick(entry)}
                          draggable={!entry.isDir && !transferBusy}
                          onDragStart={(event) => handleLocalDragStart(event, entry)}
                          sx={{ borderRadius: 0.5, mx: 0.5, my: 0.25 }}
                        >
                          <ListItemIcon sx={{ minWidth: 32 }}>
                            {entry.isDir ? (
                              <FolderIcon fontSize="small" />
                            ) : (
                              <FileIcon fontSize="small" />
                            )}
                          </ListItemIcon>
                          <ListItemText
                            primary={entry.name}
                            primaryTypographyProps={{ noWrap: true }}
                            secondary={secondaryParts.join(' - ')}
                            secondaryTypographyProps={{ noWrap: true }}
                          />
                        </ListItemButton>
                      );
                    })}
                  </List>
                )}
              </Box>

              <Box display="flex" justifyContent="flex-end" alignItems="center" mt={1}>
                <Box display="flex" gap={1}>
                  <Button
                    variant="outlined"
                    startIcon={<FolderOpenIcon />}
                    onClick={() => void handleRevealLocal()}
                    disabled={!canRevealLocal}
                  >
                    {t('fileManager.revealAction')}
                  </Button>
                  <Button
                    variant="outlined"
                    color="error"
                    startIcon={<DeleteIcon />}
                    onClick={() => void handleLocalDelete()}
                    disabled={!canDeleteLocal}
                  >
                    {t('fileManager.deleteAction')}
                  </Button>
                  <Button
                    variant="contained"
                    color="secondary"
                    startIcon={<UploadIcon />}
                    onClick={() => void handleUploadClick()}
                    disabled={!canUpload}
                  >
                    {t('fileManager.uploadAction')}
                  </Button>
                </Box>
              </Box>
            </Box>
          </Box>
        </Box>
      </DialogContent>

      <Snackbar
        open={Boolean(toast)}
        autoHideDuration={5000}
        onClose={handleToastClose}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'center' }}
      >
        {toast ? (
          <Alert onClose={handleToastClose} severity={toast.severity} sx={{ width: '100%' }}>
            {toast.message}
          </Alert>
        ) : undefined}
      </Snackbar>
    </Dialog>
  );
};

export default FileManager;


