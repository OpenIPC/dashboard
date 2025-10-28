import React, { useEffect, useRef, useState } from 'react';
import videojs from 'video.js';
import 'video.js/dist/video-js.css';

interface VideoJSPlayerProps {
  src: string;
  poster?: string;
  onReady?: (player: any) => void;
  onTimeUpdate?: (currentTime: number) => void;
  onLoadedMetadata?: (duration: number) => void;
  onSeeked?: () => void;
  onError?: (error: any) => void;
}

const VideoJSPlayer: React.FC<VideoJSPlayerProps> = ({
  src,
  poster = undefined,
  onReady,
  onTimeUpdate,
  onLoadedMetadata,
  onSeeked,
  onError
}) => {
  const videoRef = useRef<HTMLVideoElement>(null);
  const playerRef = useRef<any>(null);
  const [isPlayerReady, setIsPlayerReady] = useState(false);
  const containerRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    console.log('VideoJSPlayer: Component mounted with src:', src);
    
    if (!src) {
      console.log('VideoJSPlayer: No src provided');
      return;
    }

    // Добавляем задержку для обеспечения того, что DOM готов
    const initPlayer = () => {
      if (!videoRef.current || !containerRef.current) {
        console.log('VideoJSPlayer: Refs not ready');
        return;
      }

      // Проверяем что элемент в DOM
      if (!document.contains(videoRef.current)) {
        console.log('VideoJSPlayer: Element not in DOM yet, retrying...');
        setTimeout(initPlayer, 100);
        return;
      }

      // Убеждаемся что player ещё не создан
      if (playerRef.current) {
        console.log('VideoJSPlayer: Player already exists');
        return;
      }

      console.log('VideoJSPlayer: Initializing Video.js player...');

      try {
        const player = videojs(videoRef.current, {
          controls: true,
          responsive: false,
          fluid: false,
          fill: true,
          preload: 'metadata',
          autoplay: false,
          muted: false,
          poster: poster,
          playbackRates: [0.5, 1, 1.25, 1.5, 2],
          techOrder: ['html5'],
          html5: {
            vhs: {
              overrideNative: true
            }
          }
        });

        playerRef.current = player;
        console.log('VideoJSPlayer: Player created successfully');

        player.ready(() => {
          console.log('VideoJSPlayer: Player ready');
          setIsPlayerReady(true);
          
          // Устанавливаем источник
          player.src({
            src: src,
            type: 'video/mp4'
          });
          
          console.log('VideoJSPlayer: Source set to:', src);
          
          if (onReady) {
            onReady(player);
          }
        });

        // Event listeners
        player.on('loadstart', () => {
          console.log('VideoJSPlayer: Load start');
        });

        player.on('loadeddata', () => {
          console.log('VideoJSPlayer: Data loaded');
        });

        player.on('loadedmetadata', () => {
          const duration = player.duration();
          console.log('VideoJSPlayer: Metadata loaded, duration:', duration);
          if (onLoadedMetadata && typeof duration === 'number') {
            onLoadedMetadata(duration);
          }
        });

        player.on('canplay', () => {
          console.log('VideoJSPlayer: Can play');
        });

        player.on('canplaythrough', () => {
          console.log('VideoJSPlayer: Can play through');
        });

        player.on('timeupdate', () => {
          if (onTimeUpdate) {
            const currentTime = player.currentTime();
            if (typeof currentTime === 'number') {
              onTimeUpdate(currentTime);
            }
          }
        });

        player.on('seeked', () => {
          console.log('VideoJSPlayer: Seeked to:', player.currentTime());
          if (onSeeked) {
            onSeeked();
          }
        });

        player.on('error', (e: any) => {
          console.error('VideoJSPlayer: Error occurred:', e);
          const error = player.error();
          if (error) {
            console.error('VideoJSPlayer: Error details:', error);
          }
          if (onError) {
            onError(e);
          }
        });

      } catch (error) {
        console.error('VideoJSPlayer: Failed to create player:', error);
      }
    };

    // Запускаем инициализацию с задержкой
    const timeoutId = setTimeout(initPlayer, 200);

    return () => {
      clearTimeout(timeoutId);
      console.log('VideoJSPlayer: Cleanup called');
      
      if (playerRef.current) {
        console.log('VideoJSPlayer: Disposing player');
        try {
          if (!playerRef.current.isDisposed()) {
            playerRef.current.dispose();
          }
        } catch (error) {
          console.error('VideoJSPlayer: Error disposing player:', error);
        }
        playerRef.current = null;
      }
      
      setIsPlayerReady(false);
    };
  }, [src]);

  // Обновляем callback'и когда они изменяются
  useEffect(() => {
    if (playerRef.current && isPlayerReady) {
      console.log('VideoJSPlayer: Updating callbacks');
      // Здесь можно обновить callback'и если нужно
    }
  }, [onReady, onTimeUpdate, onLoadedMetadata, onSeeked, onError, isPlayerReady]);

  return (
    <div 
      ref={containerRef}
      style={{ 
        width: '100%', 
        height: '100%',
        position: 'relative',
        display: 'block',
        backgroundColor: '#000',
        overflow: 'hidden'
      }}
    >
      <video
        ref={videoRef}
        className="video-js vjs-default-skin"
        style={{ 
          width: '100%', 
          height: '100%',
          display: 'block'
        }}
        preload="metadata"
      />
    </div>
  );
};

export default VideoJSPlayer;