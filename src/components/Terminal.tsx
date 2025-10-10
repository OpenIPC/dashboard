import React, { useState } from 'react';
import { Typography, Box, Button, Dialog, DialogTitle, DialogContent, TextField, DialogActions } from '@mui/material';
import { Terminal } from '@mui/icons-material';

const TerminalComponent: React.FC = () => {
  const [open, setOpen] = useState(false);
  const [command, setCommand] = useState('');
  const [output, setOutput] = useState('');

  const handleExecute = () => {
    // Mock SSH execution
    setOutput(`Executing: ${command}\nMock output: Command executed successfully.`);
  };

  return (
    <Box>
      <Typography variant="h4" gutterBottom>
        SSH Terminal
      </Typography>
      <Button variant="contained" startIcon={<Terminal />} onClick={() => setOpen(true)}>
        Open Terminal
      </Button>
      <Dialog open={open} onClose={() => setOpen(false)} maxWidth="md" fullWidth>
        <DialogTitle>SSH Terminal</DialogTitle>
        <DialogContent>
          <TextField
            label="Command"
            fullWidth
            value={command}
            onChange={(e) => setCommand(e.target.value)}
            onKeyPress={(e) => e.key === 'Enter' && handleExecute()}
          />
          <Box mt={2} sx={{ backgroundColor: 'black', color: 'white', p: 2, height: '300px', overflow: 'auto' }}>
            <pre>{output}</pre>
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleExecute}>Execute</Button>
          <Button onClick={() => setOpen(false)}>Close</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default TerminalComponent;