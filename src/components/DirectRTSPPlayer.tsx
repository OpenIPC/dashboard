import React, { useEffect, useRef, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
import Hls from 'hls.js';
import { isHlsErrorData } from '../utils/hls';

interface DirectRTSPPlayerProps {
  src: string;
  width?: string | number;
  height?: string | number;
  autoPlay?: boolean;
  muted?: boolean;
  controls?: boolean;
  className?: string;
  style?: React.CSSProperties;
  onError?: (error: Error) => void;
}

const DirectRTSPPlayer: React.FC<DirectRTSPPlayerProps> = ({
  src,
  width = '100%',
  height = 'auto',
  autoPlay = true,
  muted = true,
  controls = true,
  className = '',
  style = {},
  onError
}) => {
  const videoRef = useRef<HTMLVideoElement>(null);
  const hlsRef = useRef<Hls | null>(null);
  const [isLoading, setIsLoading] = useState<boolean>(false);
  
  useEffect(() => {
    if (!src) return;
    
    let isActive = true;
    const videoElement = videoRef.current;
    
    const setupStream = async () => {
      try {
        console.log('Setting up direct RTSP stream:', src);
        setIsLoading(true);
        
        // Если хоть какой-то HLS был запущен ранее, очищаем
        if (hlsRef.current) {
          console.log('Cleaning up existing HLS player');
          hlsRef.current.destroy();
          hlsRef.current = null;
        }
        
        // Проверяем RTSP URL на наличие проблем с форматированием (например, двойной @)
        let fixedUrl = src;
        
        // Используем Rust-функцию для исправления URL с двойным @ и специальными символами
        try {
          // Проверяем, содержит ли URL специальные символы, которые могут нуждаться в обработке
          const needsSpecialHandling = /[@%: ]/.test(src);
          if (needsSpecialHandling) {
            console.log('URL contains special characters, using backend to fix encoding issues');
          }
          
          // Передаем URL через sdp параметр для обработки в backend
          fixedUrl = await invoke('play_direct_rtsp', { sdp: src }) as string;
          
          // Маскируем пароль в логах для безопасности
          const maskedUrl = fixedUrl.replace(/(rtsp:\/\/[^:]+:)([^@]+)(@.*)/i, '$1****$3');
          console.log('Fixed RTSP URL:', maskedUrl);
        } catch (error) {
          console.error('Error fixing RTSP URL:', error);
          // Продолжаем с исходным URL, если произошла ошибка
          fixedUrl = src;
        }
        
        if (videoRef.current) {
          console.log('Setting video source to:', fixedUrl);
          videoRef.current.src = fixedUrl;
          
          if (autoPlay) {
            try {
              console.log('Attempting direct RTSP playback');
              await videoRef.current.play();
              console.log('RTSP direct playback successful!');
            } catch (error) {
              const playError = error as Error;
              console.error('Failed to play RTSP directly:', playError);
              
              // Check for permission errors that might indicate authentication issues
              const errorMsg = playError.message || String(playError);
              if (errorMsg.toLowerCase().includes('permission') || 
                  errorMsg.toLowerCase().includes('unauthorized') ||
                  errorMsg.toLowerCase().includes('authentication') ||
                  errorMsg.toLowerCase().includes('denied')) {
                console.warn('Possible authentication issue with RTSP stream');
              }
              
              // Если браузер не поддерживает RTSP напрямую, пробуем через FFmpeg
              try {
                console.log('Switching to FFmpeg HLS conversion');
                // Запускаем FFmpeg внутри Tauri для конвертации потока в HLS
                console.log('Calling play_recording with:', fixedUrl);
                const hlsUrl = await invoke('play_recording', { filePath: fixedUrl });
                console.log('Converted stream URL:', hlsUrl);
                
                // Проверяем, что компонент все еще смонтирован
                if (!isActive || !videoRef.current) return;
                
                if (Hls.isSupported()) {
                  console.log('HLS.js is supported, initializing player');
                  
                  // Используем HLS.js для воспроизведения HLS потока
                  const hls = new Hls({
                    debug: true,
                    fragLoadingTimeOut: 20000,
                    manifestLoadingTimeOut: 20000,
                    levelLoadingTimeOut: 20000
                  });
                  
                  hls.attachMedia(videoRef.current);
                  hls.on(Hls.Events.MEDIA_ATTACHED, () => {
                    console.log('HLS media attached, loading source:', hlsUrl);
                    hls.loadSource(hlsUrl as string);
                  });
                  
                  hls.on(Hls.Events.ERROR, (_event, data) => {
                    console.error('HLS error:', data);
                    if (!isHlsErrorData(data) || !data.fatal) {
                      return;
                    }

                    switch (data.type) {
                        case Hls.ErrorTypes.NETWORK_ERROR:
                          console.error('Fatal network error');
                          hls.startLoad(); // try to recover
                          break;
                        case Hls.ErrorTypes.MEDIA_ERROR:
                          console.error('Fatal media error');
                          hls.recoverMediaError(); // try to recover
                          break;
                        default:
                          if (onError) onError(new Error(`HLS fatal error: ${data.type}`));
                          hls.destroy();
                          break;
                    }
                  });
                  
                  hls.on(Hls.Events.MANIFEST_PARSED, (_event, parsedData) => {
                    console.log('HLS manifest parsed, playing...', parsedData);
                    
                    if (videoRef.current) {
                      console.log('Starting video playback');
                      videoRef.current.play().catch(e => {
                        console.error('Failed to play HLS stream:', e);
                        const errorMsg = e.message || String(e);
                        
                        // Check for common authentication errors
                        if (errorMsg.toLowerCase().includes('unauthorized') || 
                            errorMsg.toLowerCase().includes('authentication') ||
                            errorMsg.toLowerCase().includes('401')) {
                          if (onError) onError(new Error('Authentication failed. Please check your camera username and password.'));
                        } else {
                          if (onError) onError(new Error(`Failed to play HLS stream: ${errorMsg}`));
                        }
                      });
                    }
                  });
                  
                  // Сохраняем ссылку для последующей очистки
                  hlsRef.current = hls;
                } else if (videoRef.current.canPlayType('application/vnd.apple.mpegurl')) {
                  // Встроенная поддержка HLS (Safari)
                  videoRef.current.src = hlsUrl as string;
                  videoRef.current.play().catch(e => {
                    console.error('Failed to play HLS stream in native player:', e);
                    const errorMsg = e.message || String(e);
                    
                    // Check for common authentication errors
                    if (errorMsg.toLowerCase().includes('unauthorized') || 
                        errorMsg.toLowerCase().includes('authentication') ||
                        errorMsg.toLowerCase().includes('401')) {
                      if (onError) onError(new Error('Authentication failed. Please check your camera username and password.'));
                    } else {
                      if (onError) onError(new Error(`Failed to play HLS stream: ${errorMsg}`));
                    }
                  });
                } else {
                  console.error('HLS не поддерживается в этом браузере');
                  if (onError) onError(new Error('HLS not supported'));
                }
              } catch (ffmpegError) {
                console.error('Failed to convert RTSP stream:', ffmpegError);
                if (onError) onError(new Error('Failed to convert RTSP stream'));
              }
            }
          }
        }
      } catch (error) {
        console.error('Error setting up RTSP stream:', error);
        if (onError && isActive) onError(error as Error);
      } finally {
        if (isActive) {
          setIsLoading(false);
        }
      }
    };
    
    setupStream();
    
    return () => {
      isActive = false;
      // Очищаем HLS при размонтировании
      const hlsInstance = hlsRef.current;
      if (hlsInstance) {
        hlsInstance.destroy();
        hlsRef.current = null;
      }

      if (videoElement) {
        videoElement.pause();
        videoElement.src = '';
        videoElement.load();
      }
    };
  }, [src, autoPlay, onError]);
  
  return (
    <>
      {isLoading && (
        <div style={{
          position: 'absolute',
          top: '50%',
          left: '50%',
          transform: 'translate(-50%, -50%)',
          zIndex: 10,
          color: 'white',
          background: 'rgba(0,0,0,0.7)',
          padding: '8px 16px',
          borderRadius: '4px'
        }}>
          Подключение к камере...
        </div>
      )}
      <video
        ref={videoRef}
        width={width}
        height={height}
        controls={controls}
        muted={muted}
        className={className}
        style={{ backgroundColor: '#000', ...style }}
      />
    </>
  );
};

export default DirectRTSPPlayer;