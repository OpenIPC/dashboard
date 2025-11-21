import React, { useEffect, useRef, useState } from 'react';
import Hls, { type ErrorData } from 'hls.js';
import EnsureHttpServer from './EnsureHttpServer';

interface HlsVideoPlayerProps {
  src: string;
  autoPlay?: boolean;
  controls?: boolean;
  muted?: boolean;
  width?: string | number;
  height?: string | number;
  className?: string;
  style?: React.CSSProperties;
}

const HlsVideoPlayer: React.FC<HlsVideoPlayerProps> = ({
  src,
  autoPlay = true,
  controls = true,
  muted = true,
  width = '100%',
  height = 'auto',
  className = '',
  style = {},
}) => {
  const videoRef = useRef<HTMLVideoElement>(null);
  const [error, setError] = useState<string | null>(null);
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    if (!src) return;
    
    let hls: Hls | null = null;
    const video = videoRef.current;

    if (!video) return;

    setIsLoading(true);

    const initPlayer = () => {
      if (Hls.isSupported()) {
        console.log('HLS is supported, initializing player with source:', src);
        
        hls = new Hls({
          enableWorker: true,
          lowLatencyMode: true,
          backBufferLength: 60,
          xhrSetup: (xhr: XMLHttpRequest) => {
            // Add custom headers if needed
            xhr.addEventListener('error', (event: ProgressEvent<EventTarget>) => {
              console.error('XHR Error:', event);
              setError(`Network error loading stream: ${src}`);
            });
          }
        });
        
        hls.loadSource(src);
        hls.attachMedia(video);
        
        hls.on(Hls.Events.MANIFEST_PARSED, () => {
          console.log('HLS manifest parsed, attempting to play');
          setIsLoading(false);
          if (autoPlay) {
            video.play().catch(err => {
              const message = err instanceof Error ? err.message : String(err);
              console.error('Error playing video:', err);
              setError(`Error playing video: ${message}`);
            });
          }
        });
        
        hls.on(Hls.Events.ERROR, (_event, payload) => {
          const data = payload as ErrorData;
          if (data?.fatal) {
            console.error('Fatal HLS error:', data.type, data.details);
            setError(`HLS Error: ${data.type} - ${data.details}`);
            
            switch (data.type) {
              case Hls.ErrorTypes.NETWORK_ERROR:
                console.log('Network error, trying to recover...');
                hls?.startLoad();
                break;
              case Hls.ErrorTypes.MEDIA_ERROR:
                console.log('Media error, trying to recover...');
                hls?.recoverMediaError();
                break;
              default:
                console.error('Fatal error, cannot recover');
                hls?.destroy();
                break;
            }
          } else if (data) {
            console.warn('Non-fatal HLS error:', data.type, data.details);
          }
        });
      } else if (video.canPlayType('application/vnd.apple.mpegurl')) {
        // For Safari which has native HLS support
        console.log('Using native HLS support');
        video.src = src;
        video.addEventListener('loadedmetadata', () => {
          setIsLoading(false);
          if (autoPlay) {
            video.play().catch(err => {
              const message = err instanceof Error ? err.message : String(err);
              console.error('Error playing video:', err);
              setError(`Error playing video: ${message}`);
            });
          }
        });
      } else {
        console.error('HLS is not supported in this browser');
        setError('HLS playback is not supported in this browser');
      }
    };

    initPlayer();

    return () => {
      if (hls) {
        hls.destroy();
      }
      if (video) {
        video.pause();
        video.src = '';
        video.load();
      }
    };
  }, [src, autoPlay]);

  return (
    <div className={`video-container ${className}`} style={{ position: 'relative', ...style }}>
      {isLoading && (
        <div className="video-loader" style={{ 
          position: 'absolute', 
          top: 0, 
          left: 0, 
          right: 0, 
          bottom: 0, 
          display: 'flex', 
          justifyContent: 'center', 
          alignItems: 'center',
          background: 'rgba(0,0,0,0.5)' 
        }}>
          <p>Loading video...</p>
        </div>
      )}
      
      {error && (
        <div className="video-error" style={{ 
          position: 'absolute', 
          top: 0, 
          left: 0, 
          right: 0, 
          bottom: 0, 
          display: 'flex', 
          flexDirection: 'column',
          justifyContent: 'center', 
          alignItems: 'center',
          background: 'rgba(0,0,0,0.7)',
          color: 'white',
          padding: '20px',
          textAlign: 'center'
        }}>
          <p>{error}</p>
          <button 
            onClick={() => window.location.reload()} 
            style={{ padding: '8px 16px', marginTop: '10px' }}
          >
            Retry
          </button>
        </div>
      )}
      
      <video
        ref={videoRef}
        controls={controls}
        muted={muted}
        width={width}
        height={height}
        style={{ width: '100%', height: '100%' }}
      />
    </div>
  );
};

// Wrap the video player with the HTTP server check
const HlsVideoPlayerWithServer: React.FC<HlsVideoPlayerProps> = (props) => {
  const [serverStatus, setServerStatus] = useState<'checking' | 'ready' | 'error'>('checking');
  
  const handleServerStarted = () => {
    console.log('HTTP Server started successfully');
    setServerStatus('ready');
  };
  
  const handleServerError = (error: string) => {
    console.error('HTTP Server error:', error);
    setServerStatus('error');
  };

  return (
    <EnsureHttpServer 
      onServerStarted={handleServerStarted} 
      onServerError={handleServerError}
    >
      {serverStatus === 'ready' ? (
        <HlsVideoPlayer {...props} />
      ) : serverStatus === 'error' ? (
        <div className="server-error" style={{ 
          padding: '20px', 
          background: 'rgba(255,0,0,0.1)', 
          border: '1px solid red',
          borderRadius: '4px',
          margin: '10px 0'
        }}>
          <h3>Streaming Server Error</h3>
          <p>The streaming server could not be started. Please try again later.</p>
        </div>
      ) : (
        <div className="server-loading" style={{ padding: '20px', textAlign: 'center' }}>
          <p>Initializing streaming server...</p>
        </div>
      )}
    </EnsureHttpServer>
  );
};

export default HlsVideoPlayerWithServer;