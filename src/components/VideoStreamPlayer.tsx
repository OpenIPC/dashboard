import React, { useEffect, useRef, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
// @ts-ignore - hls.js не имеет типов
import Hls from 'hls.js';

// КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Глобальная инициализация аудио контекста
let globalAudioInitialized = false;

const initializeGlobalAudio = async (): Promise<void> => {
  if (globalAudioInitialized) return;
  
  try {
    if (typeof window !== 'undefined' && window.AudioContext) {
      const audioContext = new AudioContext();
      if (audioContext.state === 'suspended') {
        await audioContext.resume();
      }
      await audioContext.close();
      globalAudioInitialized = true;
      console.log('[GlobalAudio] ✓ Global audio initialized successfully');
    }
  } catch (error) {
    console.error('[GlobalAudio] Failed to initialize audio context:', error);
  }
};

// Инициализируем при загрузке модуля
if (typeof window !== 'undefined') {
  document.addEventListener('click', initializeGlobalAudio, { once: true });
  document.addEventListener('touchstart', initializeGlobalAudio, { once: true });
  document.addEventListener('keydown', initializeGlobalAudio, { once: true });
}

interface VideoStreamPlayerProps {
  streamName: string;
  useHdQuality?: boolean;
  useExactName?: boolean;
  onError?: (error: Error) => void;
  onVideoRef?: (ref: HTMLVideoElement | null) => void;
  controls?: boolean;
  autoPlay?: boolean;
  muted?: boolean;
  debug?: boolean;
  width?: string | number;
  height?: string | number;
  style?: React.CSSProperties;
  className?: string;
  objectFit?: React.CSSProperties['objectFit'];
  isPaused?: boolean;
  onStatsUpdate?: (stats: StreamPlaybackStats) => void;
}

interface StreamPlaybackStats {
  bitrateKbps?: number;
  frameRate?: number;
  width?: number;
  height?: number;
  codec?: string;
}

const sanitizeName = (name: string) => name.replace(/ /g, '_').toLowerCase();
const DEFAULT_WHEP_BASES = ['http://127.0.0.1:8889', 'http://127.0.0.1:9997'];

const VideoStreamPlayer: React.FC<VideoStreamPlayerProps> = ({
  streamName,
  useHdQuality = false,
  useExactName = false,
  onError,
  onVideoRef,
  controls = true,
  autoPlay = true,
  muted = true,
  debug = false,
  width = '100%',
  height = '100%',
  style = {},
  className = '',
  objectFit = 'cover',
  isPaused = false,
  onStatsUpdate,
}) => {
  const videoRef = useRef<HTMLVideoElement>(null);
  const hlsRef = useRef<Hls | null>(null);
  const pcRef = useRef<RTCPeerConnection | null>(null);
  const [isConnecting, setIsConnecting] = useState(true);
  const [errorMessage, setErrorMessage] = useState<string | null>(null);
  const [connectionMethod, setConnectionMethod] = useState<string>('');
  const [whepBases, setWhepBases] = useState<string[]>([]);
  const statsIntervalRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const lastStatsRef = useRef<{ timestamp: number; bytesReceived: number; framesDecoded?: number } | null>(null);
  const statsCallbackRef = useRef<VideoStreamPlayerProps['onStatsUpdate']>(undefined);
  const isPausedRef = useRef<boolean>(false);

  useEffect(() => {
    statsCallbackRef.current = onStatsUpdate;
  }, [onStatsUpdate]);

  useEffect(() => {
    isPausedRef.current = isPaused;

    const videoEl = videoRef.current;
    if (!videoEl) {
      return;
    }

    const stream = videoEl.srcObject instanceof MediaStream ? videoEl.srcObject : null;

    if (isPaused) {
      if (stream) {
        stream.getVideoTracks().forEach(track => {
          if (track.enabled) {
            track.enabled = false;
          }
        });
        stream.getAudioTracks().forEach(track => {
          if (track.enabled) {
            track.enabled = false;
          }
        });
      }

      if (!videoEl.paused) {
        try {
          videoEl.pause();
        } catch (err) {
          console.log(`[Playback:${streamName}] Pause request failed:`, err);
        }
      }

      console.log(`[Playback:${streamName}] Stream paused by controls`);
    } else {
      if (stream) {
        stream.getVideoTracks().forEach(track => {
          if (!track.enabled) {
            track.enabled = true;
          }
        });
        stream.getAudioTracks().forEach(track => {
          if (!track.enabled) {
            track.enabled = true;
          }
        });
      }

      const attemptResume = async () => {
        try {
          await videoEl.play();
          console.log(`[Playback:${streamName}] Playback resumed`);
        } catch (resumeError) {
          console.log(`[Playback:${streamName}] Resume play failed, retrying muted:`, resumeError);
          const priorMuted = videoEl.muted;
          videoEl.muted = true;
          try {
            await videoEl.play();
            console.log(`[Playback:${streamName}] Resume fallback succeeded with muted=true`);
          } catch (fallbackError) {
            console.warn(`[Playback:${streamName}] Resume fallback failed:`, fallbackError);
          } finally {
            videoEl.muted = priorMuted;
          }
        }
      };

      void attemptResume();
    }
  }, [isPaused, streamName]);

  // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Принудительная инициализация аудио при монтировании
  useEffect(() => {
    const forceAudioInit = async () => {
      await initializeGlobalAudio();
    };
    forceAudioInit();
  }, []);

  useEffect(() => {
    const loadWhepBases = async () => {
      try {
        const bases = await invoke<string[]>('get_whep_endpoints');
        if (Array.isArray(bases) && bases.length > 0) {
          const normalized = Array.from(new Set(
            bases.map(base => base.trim().replace(/\/$/, ''))
          ));
          console.log('[VideoStreamPlayer] Loaded WHEP bases:', normalized);
          setWhepBases(normalized);
        } else {
          console.warn('[VideoStreamPlayer] Empty WHEP base list returned, using defaults');
          setWhepBases([...DEFAULT_WHEP_BASES]);
        }
      } catch (error) {
  console.warn('[VideoStreamPlayer] Failed to load WHEP endpoints, using defaults', error);
  setWhepBases([...DEFAULT_WHEP_BASES]);
      }
    };

    loadWhepBases();
  }, []);

  // ИСПРАВЛЕННЫЙ эффект для управления аудио при изменении muted состояния
  useEffect(() => {
    console.log(`🔊 [AudioControl:${streamName}] Effect triggered: muted=${muted}, quality=${useHdQuality ? 'HD' : 'SD'}`);
    
    if (videoRef.current) {
      console.log(`[AudioControl:${streamName}] Setting video.muted = ${muted} for ${useHdQuality ? 'HD' : 'SD'} stream`);
      
      // ДИАГНОСТИКА: Проверяем состояние видео элемента
      console.log(`[AudioControl:${streamName}] 🔍 VIDEO ELEMENT STATE: paused=${videoRef.current.paused}, volume=${videoRef.current.volume}, muted=${videoRef.current.muted}, readyState=${videoRef.current.readyState}`);
      
      videoRef.current.muted = muted;
      
      // ИСПРАВЛЕНИЕ: Для WebRTC потоков НЕ отключаем треки, только управляем через video.muted
      if (videoRef.current.srcObject && videoRef.current.srcObject instanceof MediaStream) {
        const stream = videoRef.current.srcObject;
        const audioTracks = stream.getAudioTracks();
        
        console.log(`[AudioControl:${streamName}] Found ${audioTracks.length} audio tracks in ${useHdQuality ? 'HD' : 'SD'} stream`);
        
        // ДИАГНОСТИКА: Подробная информация о каждом треке
        audioTracks.forEach((track, index) => {
          console.log(`[AudioControl:${streamName}] 🔍 TRACK ${index} DETAILS: enabled=${track.enabled}, readyState=${track.readyState}, muted=${track.muted || 'N/A'}, kind=${track.kind}, id=${track.id.substring(0, 8)}...`);
        });
        
        // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Оставляем все аудио треки включенными для WebRTC
        audioTracks.forEach((track, index) => {
          if (!track.enabled) {
            track.enabled = true;
            console.log(`[AudioControl:${streamName}] ⚡ FIXED: Re-enabling disabled audio track ${index} in ${useHdQuality ? 'HD' : 'SD'} stream`);
          }
          console.log(`[AudioControl:${streamName}] Audio track ${index} (${useHdQuality ? 'HD' : 'SD'}): enabled=${track.enabled}, video.muted=${muted}`);
        });
        
        // НОВОЕ ИСПРАВЛЕНИЕ: Особая обработка для SD потоков
        if (!useHdQuality && audioTracks.length > 0) {
          console.log(`[AudioControl:${streamName}] 🎯 Special SD stream audio initialization`);
          
          // Принудительно активируем аудио для SD потока
          setTimeout(() => {
            audioTracks.forEach((track, index) => {
              const wasEnabled = track.enabled;
              track.enabled = true;
              console.log(`[AudioControl:${streamName}] ⚡ SD FIX: Audio track ${index} - was enabled: ${wasEnabled}, now enabled: ${track.enabled}, readyState: ${track.readyState}, muted: ${track.muted || 'N/A'}`);
            });
            
            if (videoRef.current && !muted) {
              const wasMuted = videoRef.current.muted;
              videoRef.current.muted = false;
              console.log(`[AudioControl:${streamName}] ⚡ SD FIX: Video element - was muted: ${wasMuted}, now muted: ${videoRef.current.muted}, volume: ${videoRef.current.volume}`);
            }

            // Дополнительная диагностика через 1 секунду
            setTimeout(() => {
              console.log(`[AudioControl:${streamName}] 🔍 SD POST-CHECK: Audio tracks status after 1 second:`);
              audioTracks.forEach((track, index) => {
                console.log(`  Track ${index}: enabled=${track.enabled}, readyState=${track.readyState}, kind=${track.kind}`);
              });
              
              if (videoRef.current) {
                console.log(`[AudioControl:${streamName}] 🔍 SD POST-CHECK: Video element - muted=${videoRef.current.muted}, volume=${videoRef.current.volume}, paused=${videoRef.current.paused}`);
              }
            }, 1000);
          }, 200);
        }
      } else {
        console.log(`[AudioControl:${streamName}] ❌ NO STREAM: Video element has no srcObject`);
      }
    } else {
      console.log(`[AudioControl:${streamName}] ❌ NO VIDEO ELEMENT: videoRef.current is null`);
    }
  }, [muted, streamName, useHdQuality]);

  useEffect(() => {
    console.log(`🎬 [VideoStreamPlayer] Starting component for stream: ${streamName}, HD: ${useHdQuality}, exact: ${useExactName}`);
    let isActive = true;

    if (onVideoRef && videoRef.current) {
      onVideoRef(videoRef.current);
    }

    const clearStatsInterval = () => {
      if (statsIntervalRef.current) {
        clearInterval(statsIntervalRef.current);
        statsIntervalRef.current = null;
      }
      lastStatsRef.current = null;
    };

    const reportStatsUpdate = (stats: StreamPlaybackStats) => {
      if (statsCallbackRef.current) {
        statsCallbackRef.current(stats);
      }
    };

    let activeStreamName = streamName;
    
    if (!useExactName) {
      if (useHdQuality) {
        activeStreamName = streamName.replace('_1', '_0');
        if (!activeStreamName.endsWith('_0')) {
          activeStreamName = `${streamName}_0`;
        }
      } else {
        activeStreamName = streamName.replace('_0', '_1');  
        if (!activeStreamName.endsWith('_1')) {
          activeStreamName = `${streamName}_1`;
        }
      }
    }

    const name = sanitizeName(activeStreamName);
    const encodedName = encodeURIComponent(name);
    const baseCandidates = whepBases.length > 0 ? whepBases : DEFAULT_WHEP_BASES;
    const whepEndpoints = baseCandidates.flatMap(base => {
      const normalized = base.endsWith('/') ? base.slice(0, -1) : base;
      return [
        `${normalized}/whep/${encodedName}`,
        `${normalized}/${encodedName}/whep`
      ];
    });

    const logDebug = (msg: string) => {
      console.log(`[Player:${name}]`, msg);
    };
    
    logDebug(`=== STARTING STREAM ===`);
    logDebug(`Original streamName: ${streamName}`);
    logDebug(`Final activeStreamName: ${activeStreamName}`);
    logDebug(`WHEP endpoints: ${whepEndpoints.join(', ')}`);

    const cleanup = () => {
      clearStatsInterval();
      if (statsCallbackRef.current) {
        statsCallbackRef.current({});
      }
      if (hlsRef.current) {
        hlsRef.current.destroy();
        hlsRef.current = null;
      }
      if (pcRef.current) {
        pcRef.current.close();
        pcRef.current = null;
      }
      if (videoRef.current) {
        videoRef.current.pause();
        
        if (videoRef.current.srcObject && videoRef.current.srcObject instanceof MediaStream) {
          const stream = videoRef.current.srcObject;
          stream.getTracks().forEach(track => {
            track.stop();
            logDebug(`Stopped track: ${track.kind}`);
          });
        }
        
        videoRef.current.src = '';
        videoRef.current.srcObject = null;
        videoRef.current.load();
      }
      if (onVideoRef) {
        onVideoRef(null);
      }
    };

    const startStatsMonitoring = () => {
      if (!pcRef.current || !onStatsUpdate) return;

      clearStatsInterval();

      statsIntervalRef.current = setInterval(async () => {
        const pc = pcRef.current;
        if (!pc) return;

        try {
          const statsReport = await pc.getStats();
          let inboundVideo: any = null;

          statsReport.forEach((report: any) => {
            if (report.type === 'inbound-rtp' && !report.isRemote && (report.kind === 'video' || report.mediaType === 'video')) {
              inboundVideo = report;
            }
          });

          if (!inboundVideo) {
            return;
          }

          const now = inboundVideo.timestamp ?? performance.now();
          const bytesReceived = inboundVideo.bytesReceived ?? 0;
          const prev = lastStatsRef.current;
          let bitrateKbps: number | undefined;
          let frameRate: number | undefined;

          if (prev) {
            const deltaTime = (now - prev.timestamp) / 1000;
            if (deltaTime > 0) {
              const deltaBytes = bytesReceived - prev.bytesReceived;
              if (deltaBytes >= 0) {
                bitrateKbps = (deltaBytes * 8) / deltaTime / 1000;
              }

              if (inboundVideo.framesDecoded !== undefined && prev.framesDecoded !== undefined) {
                const deltaFrames = inboundVideo.framesDecoded - prev.framesDecoded;
                if (deltaFrames >= 0) {
                  frameRate = deltaFrames / deltaTime;
                }
              }
            }
          }

          lastStatsRef.current = {
            timestamp: now,
            bytesReceived,
            framesDecoded: inboundVideo.framesDecoded,
          };

          let codec: string | undefined;
          if (inboundVideo.codecId) {
            const codecReport = statsReport.get(inboundVideo.codecId as string);
            if (codecReport && 'mimeType' in codecReport && typeof codecReport.mimeType === 'string') {
              const mime = codecReport.mimeType as string;
              const parts = mime.split('/');
              codec = parts.length > 1 ? parts[1].toUpperCase() : mime.toUpperCase();
              if (codec === 'H264') {
                codec = 'H.264';
              } else if (codec === 'H265' || codec === 'H.265') {
                codec = 'H.265';
              }
            }
          }

          let width: number | undefined;
          let height: number | undefined;
          if (videoRef.current) {
            width = videoRef.current.videoWidth || undefined;
            height = videoRef.current.videoHeight || undefined;
          }

          reportStatsUpdate({
            bitrateKbps: bitrateKbps !== undefined ? Math.max(0, Math.round(bitrateKbps)) : undefined,
            frameRate: frameRate !== undefined ? Math.max(0, Math.round(frameRate)) : undefined,
            width,
            height,
            codec,
          });
        } catch (statsError) {
          console.warn(`[Stats:${name}] Failed to poll playback stats`, statsError);
        }
      }, 1000);
    };

    const startWebRTC = async () => {
      if (!videoRef.current) return false;
      clearStatsInterval();
      
      try {
        logDebug('Starting WebRTC connection attempt');
        
        // Принудительная инициализация AudioContext перед созданием WebRTC соединения
        await initializeGlobalAudio();
        
        try { 
          await invoke('mediamtx_start'); 
        } catch (_e) { 
          // ignore 
        }

        console.log(`🚀 [WebRTC:${name}] ===== CREATING PEER CONNECTION =====`);
        const pc = new RTCPeerConnection({ 
          iceServers: [{ urls: 'stun:stun.l.google.com:19302' }] 
        });
        pcRef.current = pc;
        pc.addTransceiver('video', { direction: 'recvonly' });
        pc.addTransceiver('audio', { direction: 'recvonly' });
        
        pc.oniceconnectionstatechange = () => {
          logDebug(`ICE connection state: ${pc.iceConnectionState}`);
        };
        
        pc.onconnectionstatechange = () => {
          logDebug(`Connection state: ${pc.connectionState}`);
          if (pc.connectionState === 'failed' || pc.connectionState === 'disconnected' || pc.connectionState === 'closed') {
            clearStatsInterval();
            reportStatsUpdate({});
          }
        };

        pc.ontrack = (ev) => {
          if (!videoRef.current || !isActive) return;
          const stream = ev.streams[0];
          if (stream) {
            logDebug(`Received track: ${ev.track.kind}`);
            
            const videoTracks = stream.getVideoTracks();
            const audioTracks = stream.getAudioTracks();
            logDebug(`Stream has ${videoTracks.length} video tracks and ${audioTracks.length} audio tracks`);
            console.log(`[WebRTC:${name}] 📡 TRACK RECEIVED: ${ev.track.kind} track, stream has ${videoTracks.length} video, ${audioTracks.length} audio tracks, muted=${muted}, quality=${useHdQuality ? 'HD' : 'SD'}`);
            
            // ВАЖНО: Проверяем каждый трек отдельно
            if (ev.track.kind === 'audio') {
              console.log(`[WebRTC:${name}] 🎵 AUDIO TRACK DETAILS: enabled=${ev.track.enabled}, readyState=${ev.track.readyState}, muted=${ev.track.muted || 'N/A'}, id=${ev.track.id}`);
              // Принудительно активируем сразу при получении
              ev.track.enabled = true;
              console.log(`[WebRTC:${name}] 🎵 AUDIO TRACK FORCE ENABLED immediately on receive`);
            }
            
            // ОТЛАДКА: Выводим в терминал Tauri
            if (typeof window !== 'undefined' && (window as any).__TAURI__) {
              console.log(`[TAURI DEBUG] WebRTC stream: ${name}, audio tracks: ${audioTracks.length}, quality: ${useHdQuality ? 'HD' : 'SD'}`);
            }
            
            if (audioTracks.length > 0) {
              console.log(`[WebRTC:${name}] ✓ Found ${audioTracks.length} audio tracks for ${useHdQuality ? 'HD' : 'SD'} quality`);
              
              audioTracks.forEach((track, index) => {
                console.log(`[WebRTC:${name}] Audio track ${index} initial state: enabled=${track.enabled}, muted=${track.muted}, readyState=${track.readyState}`);
                
                // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Принудительно активируем все аудио треки
                track.enabled = true;
                console.log(`[WebRTC:${name}] Audio track ${index} FORCE ENABLED for ${useHdQuality ? 'HD' : 'SD'} WebRTC compatibility`);
                
                track.onended = () => console.log(`[WebRTC:${name}] Audio track ${index} ended`);
                track.onmute = () => console.log(`[WebRTC:${name}] Audio track ${index} muted`);
                track.onunmute = () => console.log(`[WebRTC:${name}] Audio track ${index} unmuted`);
              });
              
              // НОВОЕ ИСПРАВЛЕНИЕ: Специальная обработка для SD потоков
              if (!useHdQuality) {
                console.log(`[WebRTC:${name}] 🎯 SD STREAM DETECTED - Applying special audio initialization`);
                
                // Дополнительные попытки активации аудио для SD
                const activateSDAudio = () => {
                  audioTracks.forEach((track, index) => {
                    if (!track.enabled) {
                      track.enabled = true;
                      console.log(`[WebRTC:${name}] ⚡ SD SPECIAL: Re-enabled audio track ${index}`);
                    }
                  });
                };
                
                // Множественные попытки с разными задержками
                setTimeout(activateSDAudio, 50);
                setTimeout(activateSDAudio, 150);
                setTimeout(activateSDAudio, 300);
                setTimeout(activateSDAudio, 500);
              }
            } else {
              console.error(`[WebRTC:${name}] ❌ AUDIO INITIALIZATION FAILED - No audio tracks in stream for ${useHdQuality ? 'HD' : 'SD'} quality`);
            }
            
            videoRef.current.srcObject = stream;
            if (ev.track.kind === 'video') {
              startStatsMonitoring();
            }
            
            // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: ВСЕГДА начинаем с muted=true для обхода autoplay блокировки
            videoRef.current.muted = true;
            console.log(`[WebRTC:${name}] Set video.muted = true for initial playback (original muted=${muted})`);
            
            // НОВОЕ ИСПРАВЛЕНИЕ: Добавляем обработчики для разблокировки аудио при взаимодействии пользователя
            const enableAudioOnInteraction = () => {
              if (videoRef.current?.srcObject instanceof MediaStream) {
                const stream = videoRef.current.srcObject;
                const audioTracks = stream.getAudioTracks();
                console.log(`[WebRTC:${name}] 🎯 User interaction detected - checking ${audioTracks.length} audio tracks`);
                
                audioTracks.forEach((track, index) => {
                  if (!track.enabled) {
                    track.enabled = true;
                    console.log(`[WebRTC:${name}] ⚡ INTERACTION FIX: Audio track ${index} re-enabled`);
                  }
                });
                
                // Устанавливаем правильное muted состояние
                if (videoRef.current && !muted) {
                  videoRef.current.muted = false;
                  console.log(`[WebRTC:${name}] ⚡ INTERACTION FIX: Video unmuted after user interaction`);
                }
              }
            };
            
            // Добавляем обработчики взаимодействия
            videoRef.current.addEventListener('click', enableAudioOnInteraction);
            videoRef.current.addEventListener('play', enableAudioOnInteraction);
            
            videoRef.current.play()
              .then(() => {
                logDebug('Video playback started successfully');
                setConnectionMethod('WebRTC');
                setIsConnecting(false);
                setErrorMessage(null);
                
                // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Восстанавливаем правильное muted состояние после успешного воспроизведения
                const restoreAudioForSD = () => {
                  if (videoRef.current && !muted) {
                    videoRef.current.muted = false;
                    console.log(`[WebRTC:${name}] ⚡ UNMUTED video after successful autoplay (user wants unmuted) - ${useHdQuality ? 'HD' : 'SD'} stream`);
                  }
                  
                  // СПЕЦИАЛЬНОЕ ИСПРАВЛЕНИЕ ДЛЯ SD: Дополнительная активация аудио
                  if (!useHdQuality && videoRef.current?.srcObject instanceof MediaStream) {
                    const stream = videoRef.current.srcObject;
                    const audioTracks = stream.getAudioTracks();
                    console.log(`[WebRTC:${name}] 🎯 SD POST-PLAYBACK: Checking ${audioTracks.length} audio tracks`);
                    
                    audioTracks.forEach((track, index) => {
                      if (!track.enabled) {
                        track.enabled = true;
                        console.log(`[WebRTC:${name}] ⚡ SD POST-PLAYBACK: Re-enabled audio track ${index}`);
                      }
                    });
                  }
                };
                
                setTimeout(restoreAudioForSD, 100);
                
                // ДОПОЛНИТЕЛЬНАЯ ЗАДЕРЖКА ДЛЯ SD ПОТОКОВ
                if (!useHdQuality) {
                  setTimeout(restoreAudioForSD, 300);
                  setTimeout(restoreAudioForSD, 600);
                }
                
                // Проверяем аудио треки после начала воспроизведения
                if (videoRef.current?.srcObject instanceof MediaStream) {
                  const finalStream = videoRef.current.srcObject;
                  const finalAudioTracks = finalStream.getAudioTracks();
                  console.log(`[WebRTC:${name}] ✓ Playback started. Final stream has ${finalAudioTracks.length} audio tracks`);
                  
                  finalAudioTracks.forEach((track, index) => {
                    if (!track.enabled) {
                      track.enabled = true;
                      console.log(`[WebRTC:${name}] ⚡ FIXED: Re-enabled audio track ${index} after playback start`);
                    }
                  });
                }
                
                // Дополнительная проверка через задержку
                setTimeout(() => {
                  if (videoRef.current?.srcObject instanceof MediaStream) {
                    const delayedCheckStream = videoRef.current.srcObject;
                    const delayedAudioTracks = delayedCheckStream.getAudioTracks();
                    console.log(`[WebRTC:${name}] 🔍 Delayed check (500ms): ${delayedAudioTracks.length} audio tracks found`);
                    
                    delayedAudioTracks.forEach((track, index) => {
                      if (!track.enabled) {
                        track.enabled = true;
                        console.log(`[WebRTC:${name}] ⚡ Delayed fix: Audio track ${index} re-enabled`);
                      }
                    });
                  }
                }, 500);

                if (isPausedRef.current) {
                  if (!videoRef.current?.paused) {
                    try {
                      videoRef.current?.pause();
                    } catch (err) {
                      console.log(`[WebRTC:${name}] Pause after start failed:`, err);
                    }
                  }
                  if (videoRef.current?.srcObject instanceof MediaStream) {
                    const pausedStream = videoRef.current.srcObject;
                    pausedStream.getTracks().forEach(track => {
                      if (track.enabled) {
                        track.enabled = false;
                      }
                    });
                  }
                  console.log(`[WebRTC:${name}] Stream started while paused - keeping media tracks disabled`);
                }
              })
              .catch((playError) => {
                console.log(`[WebRTC:${name}] Initial play() failed:`, playError);
                
                // Попытка fallback с muted=true
                if (videoRef.current) {
                  videoRef.current.muted = true;
                  videoRef.current.play().then(() => {
                    console.log(`[WebRTC:${name}] ✓ Fallback play succeeded with muted=true`);
                    setConnectionMethod('WebRTC');
                    setIsConnecting(false);
                    setErrorMessage(null);
                    
                    if (!muted) {
                      setTimeout(() => {
                        if (videoRef.current) {
                          videoRef.current.muted = false;
                          console.log(`[WebRTC:${name}] ⚡ Unmuted video after fallback playback`);
                        }
                      }, 500);
                    }
                  }).catch((fallbackError) => {
                    console.log(`[WebRTC:${name}] Fallback play also failed:`, fallbackError);
                    logDebug(`Video playback failed: ${playError.message}`);
                  });
                }
              });
          }
        };

        const offer = await pc.createOffer();
        await pc.setLocalDescription(offer);
        if (!offer.sdp) throw new Error('Failed to create SDP offer');
        
        logDebug(`SDP offer created (${offer.sdp.length} bytes)`);

        // Try WHEP via Tauri command
        try {
          const answer = await invoke('whep_play', { path: name, offerSdp: offer.sdp });
          const answerSdp = String(answer);
          logDebug(`Received SDP answer from Tauri (${answerSdp.length} bytes)`);
          await pc.setRemoteDescription({ type: 'answer', sdp: answerSdp });
          return true;
        } catch (err) {
          logDebug(`Tauri whep_play failed: ${err instanceof Error ? err.message : JSON.stringify(err)}`);
        }

        // Direct fetch attempts as fallback
        let lastFetchError: unknown = null;
        for (const endpoint of whepEndpoints) {
          try {
            logDebug(`Trying direct WHEP endpoint: ${endpoint}`);
            const resp = await fetch(endpoint, {
              method: 'POST',
              headers: { 'Content-Type': 'application/sdp' },
              body: offer.sdp
            });

            if (resp.status === 200 || resp.status === 201) {
              const answerSdp = await resp.text();
              logDebug(`Got SDP answer via ${endpoint} (${answerSdp.length} bytes)`);
              await pc.setRemoteDescription({ type: 'answer', sdp: answerSdp });
              return true;
            }

            const errorBody = await resp.text();
            lastFetchError = new Error(`Endpoint ${endpoint} returned status ${resp.status}: ${errorBody}`);
            logDebug(`Endpoint ${endpoint} returned ${resp.status}: ${errorBody}`);
          } catch (fetchError) {
            lastFetchError = fetchError;
            logDebug(`Endpoint ${endpoint} failed: ${fetchError instanceof Error ? fetchError.message : String(fetchError)}`);
          }
        }

        throw lastFetchError instanceof Error
          ? lastFetchError
          : new Error(`All WebRTC endpoints failed`);
      } catch (e) {
        logDebug(`WebRTC start failed: ${e instanceof Error ? e.message : String(e)}`);
        if (pcRef.current) {
          pcRef.current.close();
          pcRef.current = null;
        }
      }
      return false;
    };

    const startStream = async () => {
      if (!isActive) return;

      const webrtcSuccess = await startWebRTC();
      if (webrtcSuccess) return;

      setErrorMessage('Failed to connect to stream');
      setIsConnecting(false);
    };

    startStream();

    return () => {
      isActive = false;
      cleanup();
    };
  }, [streamName, useHdQuality, useExactName, whepBases]);

  return (
    <div style={{ position: 'relative', width, height, ...style }} className={className}>
      <video
        ref={videoRef}
        controls={controls}
        muted={muted}
        autoPlay={autoPlay}
        style={{ width: '100%', height: '100%', objectFit, backgroundColor: '#000' }}
        onClick={() => {
          // ИСПРАВЛЕНИЕ: Обработчик клика для разблокировки аудио
          if (videoRef.current) {
            console.log(`[VideoClick:${streamName}] User clicked video, ensuring audio tracks are properly enabled`);
            
            videoRef.current.play().catch(error => {
              console.log(`[VideoClick:${streamName}] Play after click failed:`, error);
            });
            
            if (videoRef.current.srcObject instanceof MediaStream) {
              const stream = videoRef.current.srcObject;
              const audioTracks = stream.getAudioTracks();
              
              console.log(`[VideoClick:${streamName}] Checking ${audioTracks.length} audio tracks after click`);
              audioTracks.forEach((track, index) => {
                if (!track.enabled) {
                  track.enabled = true;
                  console.log(`[VideoClick:${streamName}] ⚡ Re-enabled audio track ${index} after user click`);
                }
              });
            }
          }
        }}
        onError={(e) => {
          const videoEl = e.target as HTMLVideoElement;
          const mediaErr = videoEl.error;
          
          if (connectionMethod === 'WebRTC' || videoEl.srcObject) {
            console.log(`Ignoring video error during active stream: ${mediaErr?.message}`);
            return;
          }
          
          const msg = mediaErr?.message || `code ${mediaErr?.code ?? 'n/a'}`;
          console.error(`Video error for ${streamName}:`, msg);
          setErrorMessage(`Video error: ${msg}`);
          onError?.(new Error(`Video error: ${msg}`));
        }}
      />

      {isConnecting && (
        <div style={{
          position: 'absolute', inset: 0, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center',
          backgroundColor: 'rgba(0,0,0,0.5)', color: '#fff', fontSize: 14, zIndex: 2
        }}>
          Подключение к потоку...
          {debug && connectionMethod && (
            <div style={{ fontSize: 10, marginTop: 5 }}>Метод: {connectionMethod}</div>
          )}
        </div>
      )}

      {!isConnecting && errorMessage && !videoRef.current?.srcObject && (
        <div style={{
          position: 'absolute', inset: 0, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center',
          backgroundColor: 'rgba(0,0,0,0.7)', color: 'red', padding: 10, textAlign: 'center', fontSize: 14, zIndex: 2
        }}>
          Ошибка воспроизведения потока
          <div style={{ fontSize: 10, marginTop: 5 }}>{errorMessage}</div>
        </div>
      )}
    </div>
  );
};

export default VideoStreamPlayer;