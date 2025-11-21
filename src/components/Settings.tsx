import React, { useState, useEffect } from 'react';
import { 
  Typography, 
  Box, 
  Paper, 
  TextField, 
  Button, 
  Switch, 
  FormControlLabel,
  Divider,
  Alert,
  Snackbar
} from '@mui/material';

interface Go2RtcSettings {
  showMonitor: boolean;
  enableSnapshot: boolean;
  enable2WayAudio: boolean;
  enableAdaptiveBitrate: boolean;
}

const Settings: React.FC = () => {
  const [go2rtcSettings, setGo2rtcSettings] = useState<Go2RtcSettings>({
    showMonitor: false,
    enableSnapshot: true,
    enable2WayAudio: false,
    enableAdaptiveBitrate: true,
  });
  const [showSuccess, setShowSuccess] = useState(false);

  // Load settings from localStorage
  useEffect(() => {
    const saved = localStorage.getItem('go2rtcSettings');
    if (saved) {
      try {
        setGo2rtcSettings(JSON.parse(saved));
      } catch (e) {
        console.error('Failed to load go2rtc settings:', e);
      }
    }
  }, []);

  const handleGo2RtcSettingChange = (key: keyof Go2RtcSettings, value: boolean) => {
    setGo2rtcSettings(prev => ({ ...prev, [key]: value }));
  };

  const saveGo2RtcSettings = () => {
    localStorage.setItem('go2rtcSettings', JSON.stringify(go2rtcSettings));
    setShowSuccess(true);
    // Reload page to apply settings
    setTimeout(() => window.location.reload(), 1500);
  };

  return (
    <Box sx={{ p: 2 }}>
      <Typography variant="h4" gutterBottom sx={{ color: '#fff' }}>
        Settings
      </Typography>

      {/* go2rtc Enhanced Settings */}
      <Paper sx={{ p: 3, mb: 3, backgroundColor: '#1e1e1e', color: '#fff' }}>
        <Typography variant="h6" gutterBottom sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
          ⭐ go2rtc Enhanced Features
        </Typography>
        <Typography variant="body2" sx={{ color: '#aaa', mb: 2 }}>
          Enable advanced streaming features for all cameras
        </Typography>
        
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
          <FormControlLabel
            control={
              <Switch
                checked={go2rtcSettings.showMonitor}
                onChange={(e) => handleGo2RtcSettingChange('showMonitor', e.target.checked)}
                color="primary"
              />
            }
            label={
              <Box>
                <Typography variant="body1">Real-Time Stream Monitor</Typography>
                <Typography variant="caption" sx={{ color: '#888' }}>
                  Display bitrate, codec, viewers, and latency info on each stream
                </Typography>
              </Box>
            }
          />

          <FormControlLabel
            control={
              <Switch
                checked={go2rtcSettings.enableSnapshot}
                onChange={(e) => handleGo2RtcSettingChange('enableSnapshot', e.target.checked)}
                color="primary"
              />
            }
            label={
              <Box>
                <Typography variant="body1">Quick Snapshots</Typography>
                <Typography variant="caption" sx={{ color: '#888' }}>
                  Add snapshot button to each stream for instant captures
                </Typography>
              </Box>
            }
          />

          <FormControlLabel
            control={
              <Switch
                checked={go2rtcSettings.enable2WayAudio}
                onChange={(e) => handleGo2RtcSettingChange('enable2WayAudio', e.target.checked)}
                color="primary"
              />
            }
            label={
              <Box>
                <Typography variant="body1">Two-Way Audio</Typography>
                <Typography variant="caption" sx={{ color: '#888' }}>
                  Enable bidirectional audio for intercoms and interactive cameras
                </Typography>
              </Box>
            }
          />

          <FormControlLabel
            control={
              <Switch
                checked={go2rtcSettings.enableAdaptiveBitrate}
                onChange={(e) => handleGo2RtcSettingChange('enableAdaptiveBitrate', e.target.checked)}
                color="primary"
              />
            }
            label={
              <Box>
                <Typography variant="body1">Adaptive Bitrate</Typography>
                <Typography variant="caption" sx={{ color: '#888' }}>
                  Automatically switch between HD/SD based on network conditions
                </Typography>
              </Box>
            }
          />

          <Alert severity="info" sx={{ mt: 1 }}>
            Changes will take effect after reloading the application
          </Alert>

          <Button 
            variant="contained" 
            color="primary" 
            onClick={saveGo2RtcSettings}
            sx={{ alignSelf: 'flex-start', mt: 1 }}
          >
            Save & Reload
          </Button>
        </Box>
      </Paper>

      <Divider sx={{ my: 3, borderColor: '#444' }} />

      {/* Original Settings */}
      <Paper sx={{ p: 2, backgroundColor: '#1e1e1e', color: '#fff' }}>
        <Typography variant="h6" gutterBottom>
          Majestic Configuration
        </Typography>
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
          <Box sx={{ display: 'flex', gap: 2, flexWrap: 'wrap' }}>
            <TextField
              label="Video Resolution"
              defaultValue="1920x1080"
              sx={{ flex: 1, minWidth: '200px', input: { color: '#fff' }, label: { color: '#ccc' }, fieldset: { borderColor: '#ccc' } }}
              variant="outlined"
            />
            <TextField
              label="Frame Rate"
              defaultValue="30"
              sx={{ flex: 1, minWidth: '200px', input: { color: '#fff' }, label: { color: '#ccc' }, fieldset: { borderColor: '#ccc' } }}
              variant="outlined"
            />
          </Box>
          <Button variant="contained" color="primary" sx={{ alignSelf: 'flex-start' }}>
            Save Settings
          </Button>
        </Box>
      </Paper>

      <Snackbar
        open={showSuccess}
        autoHideDuration={3000}
        onClose={() => setShowSuccess(false)}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
      >
        <Alert severity="success" onClose={() => setShowSuccess(false)}>
          Settings saved! Reloading...
        </Alert>
      </Snackbar>
    </Box>
  );
};

export default Settings;