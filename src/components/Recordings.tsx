import React, { useState } from 'react';
import { 
  Typography, 
  List, 
  ListItem, 
  ListItemText, 
  ListItemSecondaryAction, 
  IconButton, 
  Dialog, 
  DialogTitle, 
  DialogContent, 
  DialogActions, 
  Button,
  Box,
  Divider,
  Snackbar,
  Alert
} from '@mui/material';
import { PlayArrow, Close } from '@mui/icons-material';
import { invoke } from '@tauri-apps/api/core';
import HlsVideoPlayerWithServer from './HlsVideoPlayer';

interface Recording {
  id: number;
  name: string;
  date: string;
  duration: string;
  rtspUrl: string;
}

const Recordings: React.FC = () => {
  // Mock data - in a real app, this would come from the backend
  const recordings: Recording[] = [
    { 
      id: 1, 
      name: 'Front Door Camera', 
      date: '2023-10-01', 
      duration: '10:30', 
      rtspUrl: 'rtsp://localhost:8554/front-door' 
    },
    { 
      id: 2, 
      name: 'Backyard Camera', 
      date: '2023-10-02', 
      duration: '15:45', 
      rtspUrl: 'rtsp://localhost:8554/backyard' 
    },
  ];

  const [selectedRecording, setSelectedRecording] = useState<Recording | null>(null);
  const [playbackUrl, setPlaybackUrl] = useState<string | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [snackbarOpen, setSnackbarOpen] = useState(false);

  const handlePlay = async (recording: Recording) => {
    setSelectedRecording(recording);
    setIsLoading(true);
    setError(null);
    
    try {
      // Call the Tauri backend to convert RTSP to HLS
      const hlsUrl = await invoke<string>('play_recording', { 
        filePath: recording.rtspUrl 
      });
      
      console.log('Playback URL:', hlsUrl);
      setPlaybackUrl(hlsUrl);
    } catch (err) {
      console.error('Error starting playback:', err);
      setError(err instanceof Error ? err.message : 'Failed to start playback');
      setSnackbarOpen(true);
    } finally {
      setIsLoading(false);
    }
  };

  const handleClose = () => {
    setSelectedRecording(null);
    setPlaybackUrl(null);
  };

  const handleSnackbarClose = () => {
    setSnackbarOpen(false);
  };
  
  return (
    <div>
      <Typography variant="h4" gutterBottom>
        Recordings
      </Typography>
      
      <List>
        {recordings.map((recording) => (
          <ListItem key={recording.id}>
            <ListItemText 
              primary={recording.name} 
              secondary={`Date: ${recording.date} | Duration: ${recording.duration}`} 
            />
            <ListItemSecondaryAction>
              <IconButton 
                edge="end" 
                aria-label="play" 
                onClick={() => handlePlay(recording)}
              >
                <PlayArrow />
              </IconButton>
            </ListItemSecondaryAction>
          </ListItem>
        ))}
      </List>
      
      {/* Playback Dialog */}
      <Dialog 
        open={!!selectedRecording && !!playbackUrl} 
        onClose={handleClose}
        maxWidth="lg"
        fullWidth
      >
        {selectedRecording && (
          <>
            <DialogTitle>
              <Box display="flex" justifyContent="space-between" alignItems="center">
                <Typography variant="h6">
                  {selectedRecording.name} - {selectedRecording.date}
                </Typography>
                <IconButton edge="end" onClick={handleClose}>
                  <Close />
                </IconButton>
              </Box>
            </DialogTitle>
            <Divider />
            <DialogContent>
              {isLoading ? (
                <Box display="flex" justifyContent="center" alignItems="center" height="400px">
                  <Typography>Loading video stream...</Typography>
                </Box>
              ) : playbackUrl ? (
                <HlsVideoPlayerWithServer 
                  src={playbackUrl} 
                  controls={true}
                  autoPlay={true}
                  muted={false}
                  height="400px"
                />
              ) : error ? (
                <Box 
                  display="flex" 
                  justifyContent="center" 
                  alignItems="center" 
                  height="400px"
                  flexDirection="column"
                  sx={{ backgroundColor: 'error.main', color: 'error.contrastText', p: 2, borderRadius: 1 }}
                >
                  <Typography>Error loading video: {error}</Typography>
                  <Button 
                    variant="contained" 
                    color="secondary" 
                    sx={{ mt: 2 }}
                    onClick={() => handlePlay(selectedRecording)}
                  >
                    Retry
                  </Button>
                </Box>
              ) : null}
            </DialogContent>
            <DialogActions>
              <Button onClick={handleClose} color="primary">Close</Button>
            </DialogActions>
          </>
        )}
      </Dialog>
      
      {/* Error snackbar */}
      <Snackbar 
        open={snackbarOpen} 
        autoHideDuration={6000} 
        onClose={handleSnackbarClose}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'center' }}
      >
        <Alert onClose={handleSnackbarClose} severity="error">
          {error || 'An error occurred during playback'}
        </Alert>
      </Snackbar>
    </div>
  );
};

export default Recordings;