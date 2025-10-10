import React, { useState, useEffect } from 'react';
import { Dialog, DialogTitle, DialogContent, Box, TextField, Button, IconButton, FormControlLabel, Checkbox, Typography, Tooltip } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import HelpOutlineIcon from '@mui/icons-material/HelpOutline';
import { useLocalization } from '../contexts/LocalizationContext';

const AddCameraDialog: React.FC<{ open: boolean; onClose: () => void; onSave?: (data: any) => void; initialData?: any }> = ({ open, onClose, onSave, initialData }) => {
  const { t } = useLocalization();
  const [name, setName] = useState(initialData?.name || '');
  const [ip, setIp] = useState(initialData?.ip || '');
  const [port, setPort] = useState(initialData?.port || 554); // RTSP порт обычно 554
  const [user, setUser] = useState(initialData?.user || 'admin');
  const [pass, setPass] = useState(initialData?.pass || '');
  const [onvifPort, setOnvifPort] = useState(initialData?.onvifPort || 80);
  const [streamUrlHd, setStreamUrlHd] = useState(initialData?.pathHd || initialData?.streamUrl || '');
  const [streamUrlSd, setStreamUrlSd] = useState(initialData?.pathSd || initialData?.streamUrl || '');
  const [useManualUrl, setUseManualUrl] = useState(false);
  const [urlFormat, setUrlFormat] = useState('standard'); // 'standard' или 'hikvision'
  const [streamChannel, setStreamChannel] = useState('1');
  const [hdProfile, setHdProfile] = useState('0');
  const [sdProfile, setSdProfile] = useState('1');

  useEffect(() => {
    setName(initialData?.name || '');
    setIp(initialData?.ip || '');
    setPort(initialData?.port || 554);
    setUser(initialData?.user || 'admin');
    setPass(initialData?.pass || '');
    setOnvifPort(initialData?.onvifPort || 80);
  setStreamUrlHd(initialData?.pathHd || initialData?.streamUrl || '');
  setStreamUrlSd(initialData?.pathSd || initialData?.streamUrl || '');
    setUseManualUrl(!!initialData?.streamUrl);
  setStreamChannel('1');
  setHdProfile('0');
  setSdProfile('1');
    // По умолчанию используем стандартный формат URL (как в VLC)
    setUrlFormat('standard');
  }, [initialData, open]);
  
  // Генерирует стандартный RTSP URL
  const generateStandardUrl = (profile: string) => {
    if (!ip) return '';
    // Стандартный формат: rtsp://username:password@host:port/path
    // Если порт 554 (стандартный RTSP порт), то его можно опустить
    const portPart = port === 554 ? '' : `:${port}`;
    return `rtsp://${encodeURIComponent(user)}:${encodeURIComponent(pass)}@${ip}${portPart}/stream=${profile}`;
  };
  
  // Генерирует URL в стиле Hikvision
  const generateHikvisionUrl = (profile: string) => {
    if (!ip) return '';
    // Для Hikvision используем стандартную аутентификацию в URL
    const encodedPassword = pass.replace('@', '%40');
    return `rtsp://${encodeURIComponent(user)}:${encodedPassword}@${ip}:${port}/Streaming/Channels/10${profile}`;
  };
  
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
  }, [ip, port, user, pass, streamChannel, hdProfile, sdProfile, useManualUrl, urlFormat]);

  const handleSave = async () => {
    // Если URL не указан вручную, сгенерируем его
    let finalHdUrl = streamUrlHd;
    let finalSdUrl = streamUrlSd;
    if (!useManualUrl) {
      finalHdUrl = urlFormat === 'standard' ? generateStandardUrl(hdProfile) : generateHikvisionUrl(hdProfile);
      finalSdUrl = urlFormat === 'standard' ? generateStandardUrl(sdProfile) : generateHikvisionUrl(sdProfile);
    }
    
    let data: any = { 
      name, 
      ip, 
      protocol: 'onvif', 
      port, 
      user, 
      pass, 
      pathHd: finalHdUrl, 
      pathSd: finalSdUrl, 
      onvifPort, 
      streamUrl: finalHdUrl 
    };
    
    console.log('Saving camera with RTSP URLs:', { hd: finalHdUrl, sd: finalSdUrl });
    
    if (onSave) onSave(data);
    onClose();
  };

  return (
    <Dialog open={open} onClose={onClose} PaperProps={{ sx: { background: '#2f3438', color: '#fff', borderRadius: 2, minWidth: 520 } }}>
      <DialogTitle sx={{ px: 3, py: 1.5, bgcolor: 'transparent', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        <Box sx={{ fontWeight: 800 }}>{t('add_camera')}</Box>
        <IconButton onClick={onClose} sx={{ color: '#cfd6db', border: '1px solid rgba(255,255,255,0.04)', width: 36, height: 36, borderRadius: '50%' }}>
          <CloseIcon />
        </IconButton>
      </DialogTitle>
      <DialogContent sx={{ px: 3, pb: 3 }}>
        <Box component="form" sx={{ display: 'grid', gap: 2 }}>
          <TextField label={t('camera_name')} value={name} onChange={e => setName(e.target.value)} sx={{ input: { color: '#fff' }, bgcolor: '#2a2d2f', borderRadius: 1 }} />
          <TextField label={t('ip_host')} value={ip} onChange={e => setIp(e.target.value)} disabled={useManualUrl} sx={{ input: { color: '#fff' }, bgcolor: '#2a2d2f', borderRadius: 1 }} />
          
          {!useManualUrl && (
            <>
              <Box sx={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 2 }}>
                <TextField label={t('rtsp_port_label')} type="number" value={port} onChange={e => setPort(Number(e.target.value))} sx={{ input: { color: '#fff' } }} />
                <TextField label={t('onvif_port_label')} type="number" value={onvifPort} onChange={e => setOnvifPort(Number(e.target.value))} sx={{ input: { color: '#fff' } }} />
              </Box>
              <Box sx={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 2 }}>
                <TextField label={t('login')} value={user} onChange={e => setUser(e.target.value)} sx={{ input: { color: '#fff' } }} />
                <TextField label={t('password')} type="password" value={pass} onChange={e => setPass(e.target.value)} sx={{ input: { color: '#fff' } }} />
              </Box>
              <Box sx={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 2 }}>
                <TextField label={t('channel')} value={streamChannel} onChange={e => setStreamChannel(e.target.value)} sx={{ input: { color: '#fff' } }} />
                <TextField label={t('hd_profile')} value={hdProfile} onChange={e => setHdProfile(e.target.value)} sx={{ input: { color: '#fff' } }} />
              </Box>
              <Box sx={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 2 }}>
                <div />
                <TextField label={t('sd_profile')} value={sdProfile} onChange={e => setSdProfile(e.target.value)} sx={{ input: { color: '#fff' } }} />
              </Box>
            </>
          )}
          
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
          
          {!useManualUrl && (
            <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
              <FormControlLabel 
                control={
                  <Checkbox 
                    checked={urlFormat === 'standard'} 
                    onChange={e => setUrlFormat(e.target.checked ? 'standard' : 'hikvision')} 
                    sx={{ color: '#fff' }}
                  />
                } 
                label={t('standard_url_format')} 
              />
              <Tooltip title={t('standard_url_format')}>
                <HelpOutlineIcon fontSize="small" sx={{ color: '#aaa', cursor: 'pointer' }} />
              </Tooltip>
            </Box>
          )}
          
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
