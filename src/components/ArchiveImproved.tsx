import React, { useState, useEffect, useRef, useCallback, useMemo } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { useLocalization } from '../hooks/useLocalization';
import VideoJSPlayer from './VideoJSPlayer';
import EnsureHttpServer from './EnsureHttpServer';
import { 
  Box, 
  Typography, 
  IconButton, 
  TextField
} from '@mui/material';
import {
  ArrowBack
} from '@mui/icons-material';

interface Camera {
  id?: number;
  name?: string;
  ip?: string;
  streamUrl?: string;
  protocol?: string;
  port?: number;
  user?: string;
  pass_enc?: string;
  path_hd?: string;
  path_sd?: string;
  status?: string;
}

interface Recording {
  filename: string;
  start_time: string;
  end_time?: string;
  size: number;
  duration?: number;
}

interface Event {
  id: string;
  timestamp: string;
  event_type: string;
  camera_id: string;
  description: string;
}

interface ArchiveImprovedProps {
  cameras: Camera[];
  onClose: () => void;
}

interface TimelineSegment {
  start: number;
  end: number;
  recording: Recording;
  color: string;
}

const ZOOM_LEVELS = [
  { name: '24h', seconds: 86400 }, // Весь день
  { name: '12h', seconds: 43200 }, // 12 часов
  { name: '6h', seconds: 21600 },  // 6 часов
  { name: '3h', seconds: 10800 },  // 3 часа
  { name: '1h', seconds: 3600 },   // 1 час
  { name: '30m', seconds: 1800 },  // 30 минут
  { name: '10m', seconds: 600 }    // 10 минут
];

// Константы убраны - теперь используются в Video.js

const ArchiveImproved: React.FC<ArchiveImprovedProps> = ({ cameras, onClose }) => {
  const { t } = useLocalization();
  
  // Stable callbacks for EnsureHttpServer to prevent infinite re-mounting
  const handleServerStarted = useCallback(() => {
    console.log('HTTP Server started for archive playback');
  }, []);
  
  const handleServerError = useCallback((error: string) => {
    console.error('HTTP Server error:', error);
  }, []);
  
  const [selectedRecording, setSelectedRecording] = useState<Recording | null>(null);
  const [selectedDate, setSelectedDate] = useState<string>(
    new Date().toISOString().split('T')[0]
  );
  const [selectedCamera, setSelectedCamera] = useState<Camera | null>(null);
  const [recordings, setRecordings] = useState<Recording[]>([]);
  const [isLoadingRecordings, setIsLoadingRecordings] = useState(false);
  const [events, setEvents] = useState<Event[]>([]);
  
  // Video player state - используем только необходимые для VideoJSPlayer
  const [, setCurrentTime] = useState(0);
  const [, setDuration] = useState(0);
  const [videoSource, setVideoSource] = useState<string>('');
  const [isVideoLoading, setIsVideoLoading] = useState(false);
  
  // Timeline state - DISABLED (timeline removed)
  const [zoomLevel] = useState(0); // Index in ZOOM_LEVELS
  const [viewStartTime] = useState(0); // Start time of visible area in seconds from midnight
  const [timelineSegments, setTimelineSegments] = useState<TimelineSegment[]>([]);
  const [selectedTimelineTime] = useState(0);
  
  // Selection state удалено - используется Video.js для управления видео
  
  // Refs
  const videoRef = useRef<HTMLVideoElement>(null);
  const timelineCanvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);

  // Cleanup blob URLs when component unmounts
  useEffect(() => {
    const sourceToRevoke = videoSource;
    return () => {
      if (sourceToRevoke && sourceToRevoke.startsWith('blob:')) {
        URL.revokeObjectURL(sourceToRevoke);
      }
    };
  }, [videoSource]);

  // Cleanup old blob URL when video source changes
  useEffect(() => {
    const previousSource = videoRef.current?.src;
    if (previousSource && previousSource.startsWith('blob:') && previousSource !== videoSource) {
      URL.revokeObjectURL(previousSource);
    }
  }, [videoSource]);

  // Load recordings and events
  const loadArchiveData = useCallback(async () => {
    if (!selectedCamera) return;

    setIsLoadingRecordings(true);
    try {
      console.log(`Loading archive data for camera ${selectedCamera.name} on ${selectedDate}`);
      
      // Load recordings from Tauri backend
      const recordingsData = await invoke<Recording[]>('get_recordings_for_date', {
        cameraName: selectedCamera.name || 'ONVIF Camera',
        date: selectedDate
      });
      setRecordings(recordingsData);
      
      // Generate timeline segments from recordings
      generateTimelineSegments(recordingsData);
      
      // Also load events if needed
      const eventsData = await invoke<Event[]>('get_events_for_date', {
        date: selectedDate
      });
      setEvents(eventsData);
      
    } catch (error) {
      console.error('Failed to load archive data:', error);
      setRecordings([]);
      setTimelineSegments([]);
      setEvents([]);
    } finally {
      setIsLoadingRecordings(false);
    }
  }, [selectedCamera, selectedDate]);

  // Initialize with first camera if available
  useEffect(() => {
    if (cameras.length > 0 && !selectedCamera) {
      setSelectedCamera(cameras[0]);
    }
  }, [cameras, selectedCamera]);

  // Load archive data when camera or date changes
  useEffect(() => {
    if (selectedCamera) {
      loadArchiveData();
    }
  }, [loadArchiveData, selectedCamera, selectedDate]);

  // Play selected recording
  const playRecording = async (recording: Recording) => {
    console.log('=== ArchiveImproved: playRecording START ===');
    console.log('ArchiveImproved: Recording to play:', recording);
    console.log('ArchiveImproved: Recording filename:', recording.filename);
    console.log('ArchiveImproved: Selected camera:', selectedCamera);
    
    if (!recording.filename || !selectedCamera) {
      console.log('ArchiveImproved: Missing required data, aborting');
      return;
    }
    
    console.log('ArchiveImproved: Setting isVideoLoading to true');
    setIsVideoLoading(true);
    
    try {
      console.log('ArchiveImproved: Checking for existing blob URL...');
      // Очищаем предыдущий blob URL если он есть
      if (videoSource && videoSource.startsWith('blob:')) {
        console.log('ArchiveImproved: Revoking previous blob URL:', videoSource);
        URL.revokeObjectURL(videoSource);
      }
      
      // Video.js отлично работает с HTTP Range requests,
      // поэтому всегда используем HTTP streaming
  const httpUrl = `http://127.0.0.1:8080/${encodeURIComponent(recording.filename)}`;
      console.log('ArchiveImproved: Generated HTTP URL:', httpUrl);
      console.log('ArchiveImproved: Starting HTTP availability check...');
      
      // Проверим доступность URL
      try {
        const response = await fetch(httpUrl, { method: 'HEAD' });
        console.log('ArchiveImproved: HTTP check response status:', response.status);
        console.log('ArchiveImproved: HTTP check Accept-Ranges header:', response.headers.get('Accept-Ranges'));
        console.log('ArchiveImproved: HTTP check Content-Length header:', response.headers.get('Content-Length'));
        console.log('ArchiveImproved: HTTP check Content-Type header:', response.headers.get('Content-Type'));
        
        if (!response.ok) {
          throw new Error(`HTTP server returned ${response.status}`);
        }
        
        console.log('ArchiveImproved: HTTP check successful!');
      } catch (fetchError) {
        console.error('ArchiveImproved: Failed to check HTTP URL:', fetchError);
        throw new Error('HTTP server is not responding');
      }
      
      console.log('ArchiveImproved: Setting videoSource to:', httpUrl);
      setVideoSource(httpUrl);
      console.log('ArchiveImproved: Setting selectedRecording...');
      setSelectedRecording(recording);
      console.log('ArchiveImproved: Setting isVideoLoading to false to show VideoJSPlayer...');
      setIsVideoLoading(false);
      console.log('ArchiveImproved: playRecording completed successfully');
      
    } catch (error) {
      console.error('ArchiveImproved: Failed to prepare recording for playback:', error);
      setIsVideoLoading(false);
      alert(`Failed to load video: ${error}`);
    }
  };

  // Handle seeking удалено - используется Video.js

  // Handle recording selection
  const handleRecordingSelect = (recording: Recording) => {
    playRecording(recording);
  };

  // Generate timeline segments from recordings
  const generateTimelineSegments = (recordings: Recording[]) => {
    const segments: TimelineSegment[] = recordings.map((recording, index) => {
      const startTime = new Date(recording.start_time);
      const startOfDay = new Date(startTime);
      startOfDay.setHours(0, 0, 0, 0);
      
      const startSeconds = (startTime.getTime() - startOfDay.getTime()) / 1000;
      const endSeconds = startSeconds + (recording.duration || 180);
      
      return {
        start: startSeconds,
        end: endSeconds,
        recording,
        color: `hsl(${(index * 60) % 360}, 70%, 50%)`
      };
    });
    
    setTimelineSegments(segments);
  };

  // Format time удалено - используется Video.js

  // Format timeline time display
  const selectedDateBase = useMemo(() => new Date(selectedDate), [selectedDate]);

  const formatTimelineTime = useCallback((seconds: number): string => {
    const date = new Date(selectedDateBase);
    date.setSeconds(seconds);
    return date.toLocaleTimeString('ru-RU', { 
      hour: '2-digit', 
      minute: '2-digit',
      second: ZOOM_LEVELS[zoomLevel].seconds <= 600 ? '2-digit' : undefined
    });
  }, [selectedDateBase, zoomLevel]);

  // Draw timeline
  const drawTimeline = useCallback(() => {
    const canvas = timelineCanvasRef.current;
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    
    // Get display dimensions
    const rect = canvas.getBoundingClientRect();
    const displayWidth = rect.width;
    const displayHeight = 100;
    
    const currentZoom = ZOOM_LEVELS[zoomLevel];
    const pixelsPerSecond = displayWidth / currentZoom.seconds;
    
    // Clear canvas
    ctx.clearRect(0, 0, displayWidth, displayHeight);
    
    // Draw background
    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, displayWidth, displayHeight);
    
    // Draw time grid
    ctx.strokeStyle = '#444';
    ctx.lineWidth = 1;
    ctx.fillStyle = '#ccc';
    ctx.font = 'bold 12px Arial';
    ctx.textAlign = 'left';
    ctx.textBaseline = 'top';
    
    // Calculate grid interval based on zoom level
    let gridInterval: number;
    if (currentZoom.seconds <= 600) { // 10 minutes or less
      gridInterval = 60; // 1 minute
    } else if (currentZoom.seconds <= 3600) { // 1 hour or less
      gridInterval = 300; // 5 minutes
    } else if (currentZoom.seconds <= 10800) { // 3 hours or less
      gridInterval = 900; // 15 minutes
    } else if (currentZoom.seconds <= 21600) { // 6 hours or less
      gridInterval = 1800; // 30 minutes
    } else {
      gridInterval = 3600; // 1 hour
    }
    
    // Draw grid lines and labels
    for (let time = Math.floor(viewStartTime / gridInterval) * gridInterval; 
         time <= viewStartTime + currentZoom.seconds; 
         time += gridInterval) {
      
      if (time < 0 || time >= 86400) continue; // Skip times outside the day
      
      const x = (time - viewStartTime) * pixelsPerSecond;
      
      if (x >= 0 && x <= displayWidth) {
        // Draw grid line
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, displayHeight);
        ctx.stroke();
        
        // Draw time label
        const timeLabel = formatTimelineTime(time);
        const textMetrics = ctx.measureText(timeLabel);
        if (x + textMetrics.width < displayWidth) {
          ctx.fillText(timeLabel, x + 2, 15);
        }
      }
    }
    
    // Draw recordings
    timelineSegments.forEach(segment => {
      const startX = (segment.start - viewStartTime) * pixelsPerSecond;
      const endX = (segment.end - viewStartTime) * pixelsPerSecond;
      
      if (endX >= 0 && startX <= displayWidth) {
        const x = Math.max(0, startX);
        const w = Math.min(displayWidth - x, endX - startX);
        
        // Draw recording segment
        ctx.fillStyle = segment.color;
        ctx.fillRect(x, displayHeight - 40, w, 30);
        
        // Draw border
        ctx.strokeStyle = '#fff';
        ctx.lineWidth = 1;
        ctx.strokeRect(x, displayHeight - 40, w, 30);
        
        // Draw recording info if there's space
        if (w > 50) {
          ctx.fillStyle = '#fff';
          ctx.font = '10px Arial';
          const filename = segment.recording.filename.split('_').slice(-2).join('_');
          ctx.fillText(filename, x + 2, displayHeight - 45);
        }
      }
    });
    
    // Draw events
    events.forEach(event => {
      const eventTime = new Date(event.timestamp);
      const startOfDay = new Date(eventTime);
      startOfDay.setHours(0, 0, 0, 0);
      const eventSeconds = (eventTime.getTime() - startOfDay.getTime()) / 1000;
      
      const x = (eventSeconds - viewStartTime) * pixelsPerSecond;
      
      if (x >= 0 && x <= displayWidth) {
        // Draw event marker
        ctx.fillStyle = '#ff4444';
        ctx.fillRect(x - 1, 20, 2, displayHeight - 60);
        
        // Draw event info if zoomed in enough
        if (currentZoom.seconds <= 3600) {
          ctx.fillStyle = '#ff4444';
          ctx.font = '9px Arial';
          ctx.save();
          ctx.translate(x + 3, 35);
          ctx.rotate(-Math.PI / 4);
          ctx.fillText(event.event_type, 0, 0);
          ctx.restore();
        }
      }
    });
    
    // Draw current playback position
    if (selectedTimelineTime >= viewStartTime && selectedTimelineTime <= viewStartTime + currentZoom.seconds) {
      const x = (selectedTimelineTime - viewStartTime) * pixelsPerSecond;
      ctx.strokeStyle = '#00ff00';
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, displayHeight);
      ctx.stroke();
    }
    
    // Selection drawing удалено - теперь используется Video.js
    
  }, [zoomLevel, viewStartTime, timelineSegments, events, selectedTimelineTime, formatTimelineTime]);

  // Video control handlers удалены - теперь управляется Video.js

  // Zoom controls - DISABLED (timeline removed)
  /*
  const zoomIn = () => {
    if (zoomLevel < ZOOM_LEVELS.length - 1) {
      setZoomLevel(zoomLevel + 1);
      const newZoom = ZOOM_LEVELS[zoomLevel + 1];
      const newViewStart = selectedTimelineTime - newZoom.seconds / 2;
      const clampedViewStart = Math.max(0, Math.min(86400 - newZoom.seconds, newViewStart));
      setViewStartTime(clampedViewStart);
    }
  };

  const zoomOut = () => {
    if (zoomLevel > 0) {
      setZoomLevel(zoomLevel - 1);
      const newZoom = ZOOM_LEVELS[zoomLevel - 1];
      const newViewStart = selectedTimelineTime - newZoom.seconds / 2;
      const clampedViewStart = Math.max(0, Math.min(86400 - newZoom.seconds, newViewStart));
      setViewStartTime(clampedViewStart);
    }
  };
  */

  // Effects
  useEffect(() => {
    loadArchiveData();
  }, [loadArchiveData]);

  useEffect(() => {
    drawTimeline();
  }, [drawTimeline]);

  // Setup high-DPI canvas
  useEffect(() => {
    const canvas = timelineCanvasRef.current;
    if (!canvas) return;

    const resizeCanvas = () => {
      const rect = canvas.getBoundingClientRect();
      const devicePixelRatio = window.devicePixelRatio || 1;
      
      // Set actual size in memory (scaled up for high DPI)
      canvas.width = rect.width * devicePixelRatio;
      canvas.height = 100 * devicePixelRatio;
      
      // Scale the canvas back down using CSS
      canvas.style.width = rect.width + 'px';
      canvas.style.height = '100px';
      
      // Scale the drawing context so everything draws at the correct size
      const ctx = canvas.getContext('2d');
      if (ctx) {
        ctx.scale(devicePixelRatio, devicePixelRatio);
      }
      
      // Redraw after resize
      drawTimeline();
    };

    // Initial setup
    resizeCanvas();
    
    // Listen for window resize
    window.addEventListener('resize', resizeCanvas);
    
    return () => {
      window.removeEventListener('resize', resizeCanvas);
    };
  }, [drawTimeline]);

  // Fullscreen handling удалено - используется Video.js

  if (!selectedCamera) {
    return null;
  }

  return (
    <EnsureHttpServer 
      onServerStarted={handleServerStarted}
      onServerError={handleServerError}
    >
      <Box 
        ref={containerRef}
        sx={{ 
          position: 'fixed', 
          top: 0, 
          left: 0, 
          right: 0, 
          bottom: 0, 
          bgcolor: '#1a1a1a', 
          color: '#fff',
          zIndex: 1000,
          display: 'flex',
          flexDirection: 'column'
        }}
      >
      {/* Header */}
      <Box sx={{ 
        height: '60px',
        backgroundColor: '#2d2d2d',
        display: 'flex', 
        alignItems: 'center', 
        padding: '0 16px', 
        borderBottom: '1px solid #404040',
        gap: 2
      }}>
          <Box sx={{ display: 'flex', alignItems: 'center', gap: 2, position: 'relative', left: 16 }}>
            <IconButton 
              onClick={() => {
                // Очищаем blob URL перед закрытием
                if (videoSource && videoSource.startsWith('blob:')) {
                  URL.revokeObjectURL(videoSource);
                }
                onClose();
              }} 
              sx={{ 
                color: '#fff',
                backgroundColor: '#23272f',
                border: '1px solid #404040',
                boxShadow: 1,
                width: 36,
                height: 36,
                borderRadius: '10px',
                transition: 'background 0.2s',
                marginLeft: '264px',
                '&:hover': {
                  backgroundColor: '#31343c',
                  borderColor: '#4caf50',
                  color: '#4caf50'
                }
              }}
              size="medium"
              title={t('backToMain')}
            >
              <ArrowBack fontSize="medium" />
            </IconButton>
            <Typography variant="h6" sx={{ ml: 2, color: '#fff', fontWeight: 500 }}>
              {t('archive')}: {selectedCamera.name || t('onvifCamera')}
            </Typography>
          </Box>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 2, fontSize: '0.8rem', color: '#ccc' }}>
          {/* Надписи shortcuts убраны по просьбе пользователя */}
        </Box>
      </Box>

      {/* Main Content */}
      <Box sx={{ flex: 1, display: 'flex', flexDirection: 'row' }}> {/* Меняем direction на row */}
        {/* Left Panel */}
        <Box sx={{
          width: '280px',
          backgroundColor: '#252525',
          borderRight: '1px solid #404040',
          display: 'flex',
          flexDirection: 'column',
          position: 'absolute',
          left: 0,
          top: 0,
          bottom: 0,
          zIndex: 10
        }}>
          {/* Date Selector */}
          <Box sx={{ p: 2 }}>
            <Typography variant="subtitle2" sx={{ mb: 1, display: 'flex', alignItems: 'center' }}>
              📅 {t('selectDate')}
            </Typography>
            <TextField
              type="date"
              value={selectedDate}
              onChange={(e) => setSelectedDate(e.target.value)}
              size="small"
              fullWidth
              sx={{
                '& .MuiInputBase-input': { color: 'white' },
                '& .MuiOutlinedInput-root': {
                  backgroundColor: '#3a3a3a',
                  '& fieldset': { borderColor: '#555' },
                  '&:hover fieldset': { borderColor: '#777' },
                  '&.Mui-focused fieldset': { borderColor: '#4caf50' }
                }
              }}
            />
          </Box>

          {/* Cameras */}
          <Box sx={{ p: 2 }}>
            <Typography variant="subtitle2" sx={{ mb: 1 }}>
              📹 {t('cameras')}
            </Typography>
            {cameras.map((camera) => (
              <Box 
                key={camera.id}
                onClick={() => setSelectedCamera(camera)}
                sx={{
                  p: 1.5,
                  mb: 1,
                  backgroundColor: selectedCamera?.id === camera.id ? '#4caf50' : '#3a3a3a',
                  borderRadius: 1,
                  display: 'flex',
                  alignItems: 'center',
                  gap: 1,
                  cursor: 'pointer',
                  '&:hover': {
                    backgroundColor: selectedCamera?.id === camera.id ? '#4caf50' : '#505050'
                  }
                }}
              >
                <Box sx={{
                  width: 8,
                  height: 8,
                  borderRadius: '50%',
                  backgroundColor: selectedCamera?.id === camera.id ? '#fff' : '#4caf50'
                }} />
                <Box>
                  <Typography variant="body2" sx={{ fontWeight: 500 }}>
                    {camera.name || 'ONVIF Camera'}
                  </Typography>
                  <Typography variant="caption" sx={{ color: '#ccc' }}>
                    {camera.ip || '192.168.0.157'}
                  </Typography>
                </Box>
              </Box>
            ))}
          </Box>

          {/* Recordings */}
          <Box sx={{ flex: 1, p: 2 }}>
            <Typography variant="subtitle2" sx={{ mb: 1 }}>
              📼 {t('recordings')} ({recordings.length})
            </Typography>
            
            {isLoadingRecordings ? (
              <Box sx={{ textAlign: 'center', p: 2 }}>
                <Typography variant="body2" sx={{ color: '#ccc' }}>
                  {t('loadingRecordings')}
                </Typography>
              </Box>
            ) : recordings.length === 0 ? (
              <Box sx={{ textAlign: 'center', p: 2 }}>
                <Typography variant="body2" sx={{ color: '#ccc' }}>
                  {t('noRecordingsFound')}
                </Typography>
              </Box>
            ) : (
              recordings.map((recording) => (
                <Box
                  key={recording.filename}
                  onClick={() => handleRecordingSelect(recording)}
                  sx={{
                    p: 1.5,
                    mb: 1,
                    backgroundColor: selectedRecording?.filename === recording.filename ? '#4caf50' : '#3a3a3a',
                    borderRadius: 1,
                    cursor: 'pointer',
                    '&:hover': {
                      backgroundColor: selectedRecording?.filename === recording.filename ? '#4caf50' : '#4a4a4a'
                    }
                  }}
                >
                  <Typography variant="body2" sx={{ 
                    fontWeight: 500,
                    overflow: 'hidden',
                    textOverflow: 'ellipsis',
                    whiteSpace: 'nowrap',
                    maxWidth: '200px'
                  }}>
                    {recording.filename}
                  </Typography>
                  <Typography variant="caption" sx={{ color: '#ccc', display: 'block' }}>
                    {recording.start_time}
                  </Typography>
                  <Typography variant="caption" sx={{ color: '#999', fontSize: '0.7rem' }}>
                    {t('duration')}: {recording.duration ? 
                      typeof recording.duration === 'number' ? 
                        `${Math.floor(recording.duration / 60)}:${String(recording.duration % 60).padStart(2, '0')}` :
                        recording.duration 
                      : t('unknown')}
                  </Typography>
                  <Typography variant="caption" sx={{ color: '#999', fontSize: '0.7rem', display: 'block' }}>
                    {t('size')}: {(recording.size / (1024 * 1024)).toFixed(2)} MB
                  </Typography>
                </Box>
              ))
            )}

            {/* Total info */}
            <Box sx={{ mt: 2, p: 1, backgroundColor: '#2d2d2d', borderRadius: 1 }}>
              <Typography variant="caption" sx={{ color: '#999' }}>
                {t('total')}: {recordings.length} {t('recordings').toLowerCase()}
              </Typography>
              <br />
              <Typography variant="caption" sx={{ color: '#999' }}>
                {t('size')}: {(recordings.reduce((total, r) => total + r.size, 0) / (1024 * 1024)).toFixed(2)} MB
              </Typography>
            </Box>
          </Box>
        </Box>

        {/* Video Area */}
        <Box sx={{ 
          flex: 1, 
          display: 'flex', 
          flexDirection: 'column',
          position: 'relative',
          overflow: 'hidden',
         height: 'calc(100vh - 64px)', // Вычитаем высоту верхней панели
         marginLeft: '280px' // Ширина боковой панели
        }}>
          {/* Video Player */}
          <Box sx={{ 
            flex: 1, 
            bgcolor: '#000', 
            position: 'relative',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            width: '100%',
            height: '100%' // Полная высота
          }}>
            {selectedRecording ? (
              isVideoLoading ? (
                <Box sx={{
                  width: '100%',
                  height: '100%',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  color: '#fff'
                }}>
                  Loading video...
                </Box>
              ) : (
                <VideoJSPlayer
                  src={videoSource}
                  onReady={() => {
                    console.log('Video.js player ready');
                    setIsVideoLoading(false);
                  }}
                  onTimeUpdate={(currentTime) => {
                    setCurrentTime(currentTime);
                  }}
                  onLoadedMetadata={(duration) => {
                    setDuration(duration);
                    setIsVideoLoading(false);
                  }}
                  onSeeked={() => {
                    console.log('Video seeked successfully');
                  }}
                  onError={(error) => {
                    console.error('Video.js playback error:', error);
                    setIsVideoLoading(false);
                  }}
                />
              )
            ) : (
              <Typography variant="h6" sx={{ color: '#666' }}>
                {t('selectRecordingToPlay')}
              </Typography>
            )}
            
            {/* Video Controls Overlay - ОТКЛЮЧЕНО: Video.js уже предоставляет все необходимые элементы управления */}
            {/* Используем встроенные Video.js элементы управления вместо дублирующих overlay */}
          </Box>
        </Box>
      </Box>
    </Box>
    </EnsureHttpServer>
  );
};

export default ArchiveImproved;