import React, { useEffect, useRef } from 'react';

interface VideoJSPlayerProps {
  src: string;
  poster?: string;
  onReady?: (element: HTMLVideoElement) => void;
  onTimeUpdate?: (currentTime: number) => void;
  onLoadedMetadata?: (duration: number) => void;
  onSeeked?: () => void;
  onError?: (error: unknown) => void;
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
  const callbacksRef = useRef({ onReady, onTimeUpdate, onLoadedMetadata, onSeeked, onError });

  useEffect(() => {
    callbacksRef.current = { onReady, onTimeUpdate, onLoadedMetadata, onSeeked, onError };
  }, [onReady, onTimeUpdate, onLoadedMetadata, onSeeked, onError]);

  useEffect(() => {
    const video = videoRef.current;
    if (!video) {
      return;
    }

    const handleLoadedMetadata = () => {
      const duration = video.duration;
      const audioTracks = (video as unknown as { audioTracks?: { length: number } }).audioTracks;
      const hasAudioTrack = typeof audioTracks?.length === 'number' ? audioTracks.length > 0 : undefined;
      const audioInfo = {
        volume: video.volume,
        muted: video.muted,
        defaultMuted: video.defaultMuted,
        audioTracks: audioTracks?.length ?? 'unknown',
        mozHasAudio: (video as unknown as { mozHasAudio?: boolean }).mozHasAudio,
        webkitAudioDecodedByteCount: (video as unknown as { webkitAudioDecodedByteCount?: number }).webkitAudioDecodedByteCount,
      };
      console.log('NativeVideoPlayer: Metadata loaded, duration:', duration, 'audioInfo:', audioInfo);
      if (!Number.isNaN(duration)) {
        callbacksRef.current.onLoadedMetadata?.(duration);
      }
      if (hasAudioTrack === false) {
        console.warn('NativeVideoPlayer: No audio tracks detected in source.');
      }
    };

    const handleCanPlay = () => {
      console.log('NativeVideoPlayer: Can play');
      const playPromise = video.play();
      if (playPromise && typeof playPromise.catch === 'function') {
        playPromise.catch((error: unknown) => {
          console.warn('NativeVideoPlayer: Autoplay failed, waiting for manual play.', error);
        });
      }
    };

    const handlePlaying = () => {
      console.log('NativeVideoPlayer: Playing');
    };

    const handleTimeUpdate = () => {
      callbacksRef.current.onTimeUpdate?.(video.currentTime);
    };

    const handleSeeked = () => {
      console.log('NativeVideoPlayer: Seeked to:', video.currentTime);
      callbacksRef.current.onSeeked?.();
    };

    const handleError = () => {
      const error = video.error;
      console.error('NativeVideoPlayer: Playback error:', error);
      callbacksRef.current.onError?.(error);
    };

    video.addEventListener('loadedmetadata', handleLoadedMetadata);
    video.addEventListener('canplay', handleCanPlay);
    video.addEventListener('playing', handlePlaying);
    video.addEventListener('timeupdate', handleTimeUpdate);
    video.addEventListener('seeked', handleSeeked);
    video.addEventListener('error', handleError);

    callbacksRef.current.onReady?.(video);

    return () => {
      console.log('NativeVideoPlayer: Cleanup');
      video.pause();
      video.removeAttribute('src');
      video.load();
      video.removeEventListener('loadedmetadata', handleLoadedMetadata);
      video.removeEventListener('canplay', handleCanPlay);
      video.removeEventListener('playing', handlePlaying);
      video.removeEventListener('timeupdate', handleTimeUpdate);
      video.removeEventListener('seeked', handleSeeked);
      video.removeEventListener('error', handleError);
    };
  }, []);

  useEffect(() => {
    const video = videoRef.current;
    if (!video || !src) {
      return;
    }

    console.log('NativeVideoPlayer: Updating source to:', src);
    video.src = src;
    if (poster) {
      video.poster = poster;
    } else {
      video.removeAttribute('poster');
    }
    video.load();
  }, [src, poster]);

  return (
    <div
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
        style={{
          width: '100%',
          height: '100%',
          display: 'block'
        }}
        preload="auto"
        controls
        playsInline
      />
    </div>
  );
};

export default VideoJSPlayer;