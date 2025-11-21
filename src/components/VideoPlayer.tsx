import React, { useEffect, useRef } from 'react';
import Hls, { type ErrorData } from 'hls.js';

interface VideoPlayerProps {
  src: string;
  autoPlay?: boolean;
  controls?: boolean;
  muted?: boolean;
  width?: string | number;
  height?: string | number;
  className?: string;
  style?: React.CSSProperties;
}

const VideoPlayer: React.FC<VideoPlayerProps> = ({
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

  useEffect(() => {
    if (!src) return;
    
    let hls: Hls | null = null;
    const video = videoRef.current;

    if (!video) return;

    const initPlayer = () => {
      if (Hls.isSupported()) {
        hls = new Hls({
          enableWorker: true,
          lowLatencyMode: true,
          backBufferLength: 60
        });
        
        hls.loadSource(src);
        hls.attachMedia(video);
        
        hls.on(Hls.Events.MANIFEST_PARSED, () => {
          if (autoPlay) {
            video.play().catch(err => {
              console.error('Error playing video:', err);
            });
          }
        });
        
        hls.on(Hls.Events.ERROR, (_event, payload) => {
          const data = payload as ErrorData;
          if (data?.fatal) {
            switch (data.type) {
              case Hls.ErrorTypes.NETWORK_ERROR:
                console.error('Network error', data);
                hls?.startLoad();
                break;
              case Hls.ErrorTypes.MEDIA_ERROR:
                console.error('Media error', data);
                hls?.recoverMediaError();
                break;
              default:
                console.error('Unrecoverable error', data);
                hls?.destroy();
                break;
            }
          }
        });
      } else if (video.canPlayType('application/vnd.apple.mpegurl')) {
        // For Safari, which has native HLS support
        video.src = src;
        video.addEventListener('loadedmetadata', () => {
          if (autoPlay) {
            video.play().catch(err => {
              console.error('Error playing video:', err);
            });
          }
        });
      }
    };

    initPlayer();

    return () => {
      if (hls) {
        hls.destroy();
      }
      if (video) {
        video.removeAttribute('src');
        video.load();
      }
    };
  }, [src, autoPlay]);

  return (
    <video
      ref={videoRef}
      controls={controls}
      muted={muted}
      width={width}
      height={height}
      className={className}
      style={{ backgroundColor: '#000', ...style }}
    />
  );
};

export default VideoPlayer;