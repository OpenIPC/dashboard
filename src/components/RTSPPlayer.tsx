import React, { useEffect, useRef, useState } from 'react';
import Hls from 'hls.js';
import { isHlsErrorData } from '../utils/hls';

interface RTSPPlayerProps {
  src: string;
  cameraName: string; // Add cameraName to props
  width?: string | number;
  height?: string | number;
  autoPlay?: boolean;
  muted?: boolean;
  controls?: boolean;
  className?: string;
  style?: React.CSSProperties;
  onError?: (error: Error) => void;
}

const RTSPPlayer: React.FC<RTSPPlayerProps> = ({
  src,
  cameraName,
  width = '100%',
  height = 'auto',
  autoPlay = true,
  muted = true,
  controls = true,
  className = '',
  style = {},
  onError,
}) => {
  const videoRef = useRef<HTMLVideoElement>(null);
  const hlsRef = useRef<Hls | null>(null);
  const [hlsUrl, setHlsUrl] = useState<string | null>(null);

  useEffect(() => {
    const setupStream = async () => {
      if (!src || !cameraName) return;

      try {
        const trimmedSource = src.trim();
        if (!trimmedSource) {
          throw new Error('Stream key is required for go2rtc playback');
        }

        const apiBases = ['http://127.0.0.1:1984', 'http://localhost:1984'];
        const encoded = encodeURIComponent(trimmedSource);

        const urls = apiBases.map(base => {
          const normalizedBase = base.endsWith('/') ? base.slice(0, -1) : base;
          return `${normalizedBase}/api/hls/${encoded}/index.m3u8`;
        });

        setHlsUrl(urls[0]);
        console.log(`go2rtc HLS endpoints prepared: ${urls.join(', ')}`);

      } catch (error) {
        console.error('Failed to setup go2rtc stream:', error);
        if (onError) {
          onError(error as Error);
        }
      }
    };

    setupStream();

    // Cleanup on component unmount
    return () => {
      if (hlsRef.current) {
        hlsRef.current.destroy();
        hlsRef.current = null;
      }
  // go2rtc streams are configured via config files; no dynamic cleanup needed here.
    };
  }, [src, cameraName, onError]);

  useEffect(() => {
    if (hlsUrl && videoRef.current) {
      const video = videoRef.current;

      const initializeHls = () => {
        if (Hls.isSupported()) {
          console.log('HLS.js is supported, setting up player for:', hlsUrl);
          if (hlsRef.current) {
            hlsRef.current.destroy();
          }
          const hls = new Hls({
            // Retry on errors
            fragLoadingMaxRetry: 6,
            manifestLoadingMaxRetry: 4,
            levelLoadingMaxRetry: 4,
          });
          hlsRef.current = hls;
          hls.loadSource(hlsUrl);
          hls.attachMedia(video);
          hls.on(Hls.Events.MANIFEST_PARSED, () => {
            console.log('HLS manifest parsed, attempting to play.');
            if (autoPlay) {
              video.play().catch(e => console.error('HLS autoplay failed:', e));
            }
          });
          hls.on(Hls.Events.ERROR, (_event, data) => {
            if (!isHlsErrorData(data) || !data.fatal) {
              return;
            }

            console.error('HLS fatal error:', data);
            switch (data.type) {
              case Hls.ErrorTypes.NETWORK_ERROR:
                console.error('HLS network error, trying to recover...');
                hls.startLoad();
                break;
              case Hls.ErrorTypes.MEDIA_ERROR:
                console.error('HLS media error, trying to recover...');
                hls.recoverMediaError();
                break;
              default:
                console.error('Unrecoverable HLS error, destroying HLS.');
                hls.destroy();
                break;
            }
          });
        } else if (video.canPlayType('application/vnd.apple.mpegurl')) {
          console.log('Native HLS support detected.');
          video.src = hlsUrl;
          video.addEventListener('loadedmetadata', () => {
            if (autoPlay) {
              video.play().catch(e => console.error('Native HLS autoplay failed:', e));
            }
          });
        } else {
          console.error('HLS is not supported in this browser.');
        }
      };
      
  // Add a short delay before initializing HLS to give go2rtc time to spin up segments
      const timer = setTimeout(initializeHls, 2000); // 2-second delay

      return () => clearTimeout(timer);
    }
  }, [hlsUrl, autoPlay]);

  return (
    <video
      ref={videoRef}
      width={width}
      height={height}
      autoPlay={autoPlay}
      muted={muted}
      controls={controls}
      className={className}
      style={{ backgroundColor: 'black', ...style }}
      onError={(e: React.SyntheticEvent<HTMLVideoElement, Event>) => {
        console.error('Video element error:', e);
        const videoElement = e.target as HTMLVideoElement;
        if (onError) {
          const mediaError = videoElement.error;
          if (mediaError) {
            onError(new Error(`Video Error: code ${mediaError.code} - ${mediaError.message || 'No message'}`));
          } else {
            onError(new Error('Unknown video error'));
          }
        }
      }}
    />
  );
};

export default RTSPPlayer;