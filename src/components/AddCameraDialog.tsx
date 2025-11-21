import React, { useState, useEffect, useCallback } from 'react';
import { Dialog, DialogTitle, DialogContent, Box, TextField, Button, IconButton, FormControlLabel, Checkbox, Typography, Tooltip } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import HelpOutlineIcon from '@mui/icons-material/HelpOutline';
import { useLocalization } from '../hooks/useLocalization';
import type { CameraFormDraft, CameraFormValues } from '../types';

interface AddCameraDialogProps {
  open: boolean;
  onClose: () => void;
  onSave?: (data: CameraFormValues) => void;
  initialData?: CameraFormDraft | null;
}

const DEFAULT_RTSP_PORT = 554;
const DEFAULT_ONVIF_PORT = 80;
const DEFAULT_USER = 'admin';

type RtspFormat = 'standard' | 'hikvision';

interface ParsedRtspUrl {
  format: RtspFormat;
  username?: string;
  password?: string;
  host?: string;
  port?: number;
  profile?: string;
}

const safeDecode = (value: string | undefined): string | undefined => {
  if (value === undefined) {
    return undefined;
  }
  try {
    return decodeURIComponent(value);
  } catch {
    return value;
  }
};

const parseRtspUrl = (url?: string): ParsedRtspUrl | null => {
  if (!url) {
    return null;
  }

  try {
    const parsed = new URL(url);
    if (parsed.protocol !== 'rtsp:') {
      return null;
    }

    const pathname = parsed.pathname || '';

    if (pathname.startsWith('/stream=')) {
      const profile = pathname.split('=')[1] ?? '0';
      return {
        format: 'standard',
        username: safeDecode(parsed.username) ?? undefined,
        password: safeDecode(parsed.password) ?? undefined,
        host: parsed.hostname || undefined,
        port: parsed.port ? Number(parsed.port) : undefined,
        profile,
      };
    }

    if (pathname.startsWith('/Streaming/Channels/10')) {
      const profile = pathname.replace('/Streaming/Channels/10', '') || '0';
      return {
        format: 'hikvision',
        username: safeDecode(parsed.username) ?? undefined,
        password: safeDecode(parsed.password) ?? undefined,
        host: parsed.hostname || undefined,
        port: parsed.port ? Number(parsed.port) : undefined,
        profile,
      };
    }

    return null;
  } catch {
    return null;
  }
};

const AddCameraDialog: React.FC<AddCameraDialogProps> = ({ open, onClose, onSave, initialData }) => {
  const { t } = useLocalization();
  const isEditing = Boolean(initialData?.id);
  const [name, setName] = useState(initialData?.name ?? '');
  const [ip, setIp] = useState(initialData?.ip ?? '');
  const [port, setPort] = useState(initialData?.port ?? DEFAULT_RTSP_PORT);
  const [user, setUser] = useState(initialData?.user ?? DEFAULT_USER);
  const [pass, setPass] = useState(initialData?.pass ?? '');
  const [onvifPort, setOnvifPort] = useState(initialData?.onvifPort ?? DEFAULT_ONVIF_PORT);
  const [streamUrlHd, setStreamUrlHd] = useState(initialData?.pathHd ?? initialData?.streamUrl ?? '');
  const [streamUrlSd, setStreamUrlSd] = useState(initialData?.pathSd ?? initialData?.streamUrl ?? '');
  const [useManualUrl, setUseManualUrl] = useState(false);
  const [urlFormat, setUrlFormat] = useState<RtspFormat>('standard');
  const [streamChannel, setStreamChannel] = useState('1');
  const [hdProfile, setHdProfile] = useState('0');
  const [sdProfile, setSdProfile] = useState('1');

  useEffect(() => {
    setName(initialData?.name ?? '');
    setIp(initialData?.ip ?? '');
    setPort(initialData?.port ?? DEFAULT_RTSP_PORT);
    setUser(initialData?.user ?? DEFAULT_USER);
    setPass(initialData?.pass ?? '');
    setOnvifPort(initialData?.onvifPort ?? DEFAULT_ONVIF_PORT);
    const initialHdUrl = initialData?.pathHd ?? initialData?.streamUrl ?? '';
    const initialSdUrl = initialData?.pathSd ?? '';
    const parsedHd = parseRtspUrl(initialHdUrl);
    const parsedSd = parseRtspUrl(initialSdUrl);
    const detected = parsedHd ?? parsedSd;

    if (detected?.host && !initialData?.ip) {
      setIp(detected.host);
    }

    if (detected?.port && (!initialData?.port || Number.isNaN(initialData.port))) {
      setPort(detected.port);
    }

    if (detected?.username && !initialData?.user) {
      setUser(detected.username);
    }

    if (detected?.password && !initialData?.pass) {
      setPass(detected.password);
    }

    setHdProfile(parsedHd?.profile ?? '0');

    if (parsedSd?.profile) {
      setSdProfile(parsedSd.profile);
    } else if (parsedHd?.profile && parsedHd.profile !== '0') {
      setSdProfile(parsedHd.profile);
    } else {
      setSdProfile('1');
    }

    setStreamUrlHd(initialHdUrl);
    setStreamUrlSd(initialSdUrl);
    setStreamChannel('1');

    if (detected) {
      setUrlFormat(detected.format);
    } else {
      setUrlFormat('standard');
    }

    const hasInitialUrls = Boolean(initialHdUrl || initialSdUrl);
    const shouldUseManual = Boolean(initialData) && hasInitialUrls && !detected;
    setUseManualUrl(shouldUseManual);
  }, [initialData, open]);
  
  // Генерирует стандартный RTSP URL
  const generateStandardUrl = useCallback((profile: string) => {
    if (!ip) return '';
    // Стандартный формат: rtsp://username:password@host:port/path
    // Если порт 554 (стандартный RTSP порт), то его можно опустить
    const portPart = port === 554 ? '' : `:${port}`;
    const encodedUser = encodeURIComponent(user);
    const encodedPass = encodeURIComponent(pass);
    return `rtsp://${encodedUser}:${encodedPass}@${ip}${portPart}/stream=${profile}`;
  }, [ip, pass, port, user]);
  
  // Генерирует URL в стиле Hikvision
  const generateHikvisionUrl = useCallback((profile: string) => {
    if (!ip) return '';
    // Для Hikvision используем стандартную аутентификацию в URL
    const encodedUser = encodeURIComponent(user);
    const encodedPassword = encodeURIComponent(pass);
    return `rtsp://${encodedUser}:${encodedPassword}@${ip}:${port}/Streaming/Channels/10${profile}`;
  }, [ip, pass, port, user]);
  
  // Обновляет RTSP URL при изменении параметров
  useEffect(() => {
    if (!useManualUrl) {
      if (urlFormat === 'standard') {
        setStreamUrlHd(generateStandardUrl(hdProfile));
        setStreamUrlSd(generateStandardUrl(sdProfile));
      } else {
        setStreamUrlHd(generateHikvisionUrl(hdProfile));
        setStreamUrlSd(generateHikvisionUrl(sdProfile));
      }
    }
  }, [generateHikvisionUrl, generateStandardUrl, hdProfile, sdProfile, streamChannel, useManualUrl, urlFormat]);

  const handleSave = async () => {
    // Если URL не указан вручную, сгенерируем его
    let finalHdUrl = streamUrlHd;
    let finalSdUrl = streamUrlSd;
    if (!useManualUrl) {
      finalHdUrl = urlFormat === 'standard' ? generateStandardUrl(hdProfile) : generateHikvisionUrl(hdProfile);
      finalSdUrl = urlFormat === 'standard' ? generateStandardUrl(sdProfile) : generateHikvisionUrl(sdProfile);
    }
    
    const baseData: CameraFormValues = {
      id: typeof initialData?.id === 'number' ? initialData.id : undefined,
      name,
      ip,
      protocol: initialData?.protocol ?? 'onvif',
      port,
      user,
      pass,
      pathHd: finalHdUrl,
      pathSd: finalSdUrl,
      onvifPort,
      streamUrl: finalHdUrl,
      groupId: initialData?.groupId ?? null,
    };
    
    console.log('Saving camera with RTSP URLs:', { hd: finalHdUrl, sd: finalSdUrl });
    
    onSave?.(baseData);
    onClose();
  };

  return (
    <Dialog open={open} onClose={onClose} PaperProps={{ sx: { background: '#2f3438', color: '#fff', borderRadius: 2, minWidth: 520 } }}>
      <DialogTitle sx={{ px: 3, py: 1.5, bgcolor: 'transparent', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        <Box sx={{ fontWeight: 800 }}>{t(isEditing ? 'edit_camera_title' : 'add_camera_title')}</Box>
        <IconButton onClick={onClose} sx={{ color: '#cfd6db', border: '1px solid rgba(255,255,255,0.04)', width: 36, height: 36, borderRadius: '50%' }}>
          <CloseIcon />
        </IconButton>
      </DialogTitle>
      <DialogContent sx={{ px: 3, pb: 3 }}>
        <Box component="form" sx={{ display: 'grid', gap: 2 }}>
          <TextField label={t('camera_name')} value={name} onChange={e => setName(e.target.value)} sx={{ input: { color: '#fff' }, bgcolor: '#2a2d2f', borderRadius: 1 }} />
          <TextField label={t('ip_host')} value={ip} onChange={e => setIp(e.target.value)} disabled={useManualUrl} sx={{ input: { color: '#fff' }, bgcolor: '#2a2d2f', borderRadius: 1 }} />
          
          <Box sx={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 2 }}>
            <TextField
              label={t('rtsp_port_label')}
              type="number"
              value={port}
              onChange={e => setPort(Number(e.target.value))}
              disabled={useManualUrl}
              sx={{ input: { color: '#fff' } }}
            />
            <TextField
              label={t('onvif_port_label')}
              type="number"
              value={onvifPort}
              onChange={e => setOnvifPort(Number(e.target.value))}
              disabled={useManualUrl}
              sx={{ input: { color: '#fff' } }}
            />
          </Box>
          <Box sx={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 2 }}>
            <TextField
              label={t('login')}
              value={user}
              onChange={e => setUser(e.target.value)}
              disabled={useManualUrl}
              sx={{ input: { color: '#fff' } }}
            />
            <TextField
              label={t('password')}
              type="password"
              value={pass}
              onChange={e => setPass(e.target.value)}
              disabled={useManualUrl}
              sx={{ input: { color: '#fff' } }}
            />
          </Box>
          <Box sx={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 2 }}>
            <TextField
              label={t('channel')}
              value={streamChannel}
              onChange={e => setStreamChannel(e.target.value)}
              disabled={useManualUrl}
              sx={{ input: { color: '#fff' } }}
            />
            <TextField
              label={t('hd_profile')}
              value={hdProfile}
              onChange={e => setHdProfile(e.target.value)}
              disabled={useManualUrl}
              sx={{ input: { color: '#fff' } }}
            />
          </Box>
          <Box sx={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 2 }}>
            <div />
            <TextField
              label={t('sd_profile')}
              value={sdProfile}
              onChange={e => setSdProfile(e.target.value)}
              disabled={useManualUrl}
              sx={{ input: { color: '#fff' } }}
            />
          </Box>
          
          <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
            <FormControlLabel 
              control={
                <Checkbox 
                  checked={useManualUrl} 
                  onChange={e => setUseManualUrl(e.target.checked)} 
                  sx={{ color: '#fff' }}
                />
              } 
              label={t('manual_rtsp_url')} 
            />
            <Tooltip title={t('manual_rtsp_url')}>
              <HelpOutlineIcon fontSize="small" sx={{ color: '#aaa', cursor: 'pointer' }} />
            </Tooltip>
          </Box>
          
          <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
            <FormControlLabel 
              control={
                <Checkbox 
                  checked={urlFormat === 'standard'} 
                  onChange={e => setUrlFormat(e.target.checked ? 'standard' : 'hikvision')} 
                  disabled={useManualUrl}
                  sx={{ color: '#fff' }}
                />
              } 
              label={t('standard_url_format')} 
              disabled={useManualUrl}
            />
            <Tooltip title={t('standard_url_format')}>
              <HelpOutlineIcon fontSize="small" sx={{ color: '#aaa', cursor: 'pointer' }} />
            </Tooltip>
          </Box>
          
          <TextField 
            label={t('rtsp_hd_url')} 
            value={streamUrlHd} 
            onChange={e => setStreamUrlHd(e.target.value)} 
            disabled={!useManualUrl} 
            sx={{ input: { color: '#fff' } }} 
            fullWidth 
            multiline 
            rows={2}
          />
          <TextField 
            label={t('rtsp_sd_url')} 
            value={streamUrlSd} 
            onChange={e => setStreamUrlSd(e.target.value)} 
            disabled={!useManualUrl} 
            sx={{ input: { color: '#fff' }, mt: 1 }} 
            fullWidth 
            multiline 
            rows={2}
          />
          
          {!useManualUrl && (streamUrlHd || streamUrlSd) && (
            <Typography variant="caption" sx={{ color: '#aaa' }}>
              {t('generated_urls', { hd: streamUrlHd, sd: streamUrlSd })}
            </Typography>
          )}

          <Box sx={{ display: 'flex', gap: 2, mt: 1, justifyContent: 'flex-end' }}>
            <Button variant="contained" onClick={handleSave} sx={{ bgcolor: '#50606a', color: '#fff', px: 3 }}>{t('save')}</Button>
            <Button variant="outlined" onClick={onClose} sx={{ color: '#cfe3ff', borderColor: 'rgba(255,255,255,0.06)' }}>{t('cancel')}</Button>
          </Box>
        </Box>
      </DialogContent>
    </Dialog>
  );
};

export default AddCameraDialog;
