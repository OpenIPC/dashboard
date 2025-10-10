import React from 'react';
import { Typography, Box, Paper, TextField, Button } from '@mui/material';

const Settings: React.FC = () => {
  return (
    <Box sx={{ p: 2 }}>
      <Typography variant="h4" gutterBottom sx={{ color: '#fff' }}>
        Settings
      </Typography>
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
    </Box>
  );
};

export default Settings;