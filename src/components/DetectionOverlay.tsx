import React, { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import type { AnalyticsDetectionBox } from '../services/analytics';

interface DetectionOverlayProps {
  detections: AnalyticsDetectionBox[];
  frameWidth: number;
  frameHeight: number;
  videoElement?: HTMLVideoElement | null;
  visible?: boolean;
}

interface LayoutMetrics {
  offsetX: number;
  offsetY: number;
  scaleX: number;
  scaleY: number;
  containerWidth: number;
  containerHeight: number;
}

interface InterpolatedDetection extends AnalyticsDetectionBox {
  interpolatedBounds: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
}

const clamp = (value: number, min: number, max: number): number => {
  if (Number.isNaN(value)) {
    return min;
  }
  if (value < min) {
    return min;
  }
  if (value > max) {
    return max;
  }
  return value;
};

const DetectionOverlay: React.FC<DetectionOverlayProps> = ({
  detections,
  frameWidth,
  frameHeight,
  videoElement,
  visible = true,
}) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const [metrics, setMetrics] = useState<LayoutMetrics | null>(null);
  const [interpolatedDetections, setInterpolatedDetections] = useState<InterpolatedDetection[]>([]);
  const prevDetectionsRef = useRef<Map<string, AnalyticsDetectionBox>>(new Map());
  const animationFrameRef = useRef<number | null>(null);
  const lastUpdateTimeRef = useRef<number>(Date.now());

  // Smooth interpolation between detection updates
  useEffect(() => {
    if (detections.length === 0) {
      setInterpolatedDetections([]);
      prevDetectionsRef.current.clear();
      if (animationFrameRef.current !== null) {
        cancelAnimationFrame(animationFrameRef.current);
        animationFrameRef.current = null;
      }
      return;
    }

    const now = Date.now();
    const newDetectionsMap = new Map<string, AnalyticsDetectionBox>();
    detections.forEach(det => newDetectionsMap.set(det.id, det));

    const startTime = now;
    lastUpdateTimeRef.current = now;
    const interpolationDuration = 100; // Very fast transition for immediate tracking
    const extrapolationDuration = 3000; // Continue predicting for 3 seconds

    const animate = () => {
      const elapsed = Date.now() - startTime;
      const progress = Math.min(elapsed / interpolationDuration, 1);
      const easeProgress = 1 - Math.pow(1 - progress, 2); // Ease-out quadratic for smoother motion

      const interpolated = detections.map(currentDet => {
        const prevDet = prevDetectionsRef.current.get(currentDet.id);
        
        if (!prevDet) {
          return {
            ...currentDet,
            interpolatedBounds: { ...currentDet.bounds },
          };
        }

        // Calculate velocity
        const velocityX = currentDet.bounds.x - prevDet.bounds.x;
        const velocityY = currentDet.bounds.y - prevDet.bounds.y;
        const velocityWidth = currentDet.bounds.width - prevDet.bounds.width;
        const velocityHeight = currentDet.bounds.height - prevDet.bounds.height;

        if (progress < 1) {
          // Interpolate smoothly to new position
          return {
            ...currentDet,
            interpolatedBounds: {
              x: prevDet.bounds.x + velocityX * easeProgress,
              y: prevDet.bounds.y + velocityY * easeProgress,
              width: prevDet.bounds.width + velocityWidth * easeProgress,
              height: prevDet.bounds.height + velocityHeight * easeProgress,
            },
          };
        } else {
          // Extrapolate beyond current position to predict movement
          const extraElapsed = elapsed - interpolationDuration;
          const extraProgress = Math.min(extraElapsed / extrapolationDuration, 1);
          const dampingFactor = 1 - extraProgress * 0.5; // Gradually slow down extrapolation
          
          return {
            ...currentDet,
            interpolatedBounds: {
              x: currentDet.bounds.x + velocityX * extraProgress * dampingFactor,
              y: currentDet.bounds.y + velocityY * extraProgress * dampingFactor,
              width: currentDet.bounds.width + velocityWidth * extraProgress * dampingFactor * 0.3,
              height: currentDet.bounds.height + velocityHeight * extraProgress * dampingFactor * 0.3,
            },
          };
        }
      });

      setInterpolatedDetections(interpolated);

      // Continue animation during interpolation and extrapolation
      if (elapsed < interpolationDuration + extrapolationDuration) {
        animationFrameRef.current = requestAnimationFrame(animate);
      } else {
        animationFrameRef.current = null;
        prevDetectionsRef.current = newDetectionsMap;
      }
    };

    if (animationFrameRef.current !== null) {
      cancelAnimationFrame(animationFrameRef.current);
    }
    
    animate();

    return () => {
      if (animationFrameRef.current !== null) {
        cancelAnimationFrame(animationFrameRef.current);
        animationFrameRef.current = null;
      }
    };
  }, [detections]);

  const updateMetrics = useCallback(() => {
    const container = containerRef.current;
    
    if (!container || !videoElement || frameWidth <= 0 || frameHeight <= 0) {
      setMetrics(null);
      return;
    }

    if (typeof window === 'undefined') {
      return;
    }

    const containerRect = container.getBoundingClientRect();
    const containerWidth = containerRect.width;
    const containerHeight = containerRect.height;
    if (containerWidth === 0 || containerHeight === 0) {
      return;
    }

    const naturalWidth = videoElement.videoWidth || frameWidth;
    const naturalHeight = videoElement.videoHeight || frameHeight;
    if (naturalWidth <= 0 || naturalHeight <= 0) {
      setMetrics(null);
      return;
    }

  // Align overlay with video taking object-fit scaling into account.
  const computedStyle = window.getComputedStyle(videoElement);
    const objectFit = computedStyle?.objectFit || 'fill';

    const ratioX = containerWidth / naturalWidth;
    const ratioY = containerHeight / naturalHeight;

    let scaleFactor: number;
    switch (objectFit) {
      case 'cover':
        scaleFactor = Math.max(ratioX, ratioY);
        break;
      case 'contain':
        scaleFactor = Math.min(ratioX, ratioY);
        break;
      case 'scale-down':
        scaleFactor = Math.min(1, Math.min(ratioX, ratioY));
        break;
      case 'none':
        scaleFactor = 1;
        break;
      case 'fill':
      default:
        scaleFactor = ratioX;
        break;
    }

    if (!Number.isFinite(scaleFactor) || scaleFactor <= 0) {
      setMetrics(null);
      return;
    }

    const displayWidth = naturalWidth * scaleFactor;
    const displayHeight = naturalHeight * scaleFactor;

    const offsetX = (containerWidth - displayWidth) / 2;
    const offsetY = (containerHeight - displayHeight) / 2;

    // Scale from detection coordinates (frameWidth/frameHeight) to display pixels
    // Detection coordinates are relative to the frame size from backend
    setMetrics({
      offsetX,
      offsetY,
      scaleX: displayWidth / frameWidth,
      scaleY: displayHeight / frameHeight,
      containerWidth,
      containerHeight,
    });
  }, [videoElement, frameWidth, frameHeight]);

  useEffect(() => {
    updateMetrics();
  }, [updateMetrics, detections]);

  useEffect(() => {
    updateMetrics();

    if (!videoElement) {
      return;
    }

    const handleLoadedMetadata = () => {
      updateMetrics();
    };

    videoElement.addEventListener('loadedmetadata', handleLoadedMetadata);
    videoElement.addEventListener('loadeddata', handleLoadedMetadata);

    let resizeObserver: ResizeObserver | null = null;
    const handleWindowResize = () => {
      updateMetrics();
    };

    const hasWindow = typeof window !== 'undefined';

    if (typeof ResizeObserver !== 'undefined') {
      resizeObserver = new ResizeObserver(() => {
        updateMetrics();
      });

      if (containerRef.current) {
        resizeObserver.observe(containerRef.current);
      }

      resizeObserver.observe(videoElement);
    } else if (hasWindow) {
      window.addEventListener('resize', handleWindowResize);
    }

    return () => {
      videoElement.removeEventListener('loadedmetadata', handleLoadedMetadata);
      videoElement.removeEventListener('loadeddata', handleLoadedMetadata);

      if (resizeObserver) {
        resizeObserver.disconnect();
      } else if (hasWindow) {
        window.removeEventListener('resize', handleWindowResize);
      }
    };
  }, [videoElement, updateMetrics]);

  const overlayDetections = useMemo(() => {
    if (!metrics || interpolatedDetections.length === 0) {
      return [] as InterpolatedDetection[];
    }
    return interpolatedDetections;
  }, [interpolatedDetections, metrics]);

  const isVisible = Boolean(visible && metrics && overlayDetections.length > 0);

  return (
    <div
      ref={containerRef}
      style={{
        position: 'absolute',
        inset: 0,
        pointerEvents: 'none',
        zIndex: 12,
        opacity: isVisible ? 1 : 0,
        transition: 'opacity 120ms linear',
      }}
    >
      {isVisible && metrics &&
        overlayDetections.map(det => {
          const color = det.color || '#ff7f50';
          // Use interpolated bounds for smooth animation
          const bounds = det.interpolatedBounds;
          const widthPx = bounds.width * metrics.scaleX;
          const heightPx = bounds.height * metrics.scaleY;
          const rawLeft = metrics.offsetX + bounds.x * metrics.scaleX;
          const rawTop = metrics.offsetY + bounds.y * metrics.scaleY;
          const maxLeft = Math.max(metrics.containerWidth - widthPx, 0);
          const maxTop = Math.max(metrics.containerHeight - heightPx, 0);
          const left = clamp(rawLeft, 0, maxLeft);
          const top = clamp(rawTop, 0, maxTop);
          const width = Math.min(widthPx, Math.max(metrics.containerWidth - left, 0));
          const height = Math.min(heightPx, Math.max(metrics.containerHeight - top, 0));
          const labelParts: string[] = [];
          if (det.label) {
            labelParts.push(det.label);
          }
          if (Number.isFinite(det.confidence)) {
            labelParts.push(`${Math.round(det.confidence * 100)}%`);
          }
          if (det.zone) {
            labelParts.push(`[${det.zone}]`);
          }
          const labelText = labelParts.join(' ');

          return (
            <div
              key={det.id}
              style={{
                position: 'absolute',
                left,
                top,
                width,
                height,
                border: `2px solid ${color}`,
                boxShadow: '0 0 12px rgba(0, 0, 0, 0.35)',
                borderRadius: 4,
                backgroundColor: 'rgba(0, 0, 0, 0.08)',
                pointerEvents: 'none',
              }}
            >
              {labelText && (
                <div
                  style={{
                    position: 'absolute',
                    left: 0,
                    top: 0,
                    transform: 'translateY(-100%)',
                    padding: '2px 6px',
                    borderRadius: 4,
                    backgroundColor: 'rgba(0, 0, 0, 0.7)',
                    color: '#fff',
                    fontSize: 11,
                    fontWeight: 500,
                    letterSpacing: 0.4,
                    textTransform: 'uppercase',
                    pointerEvents: 'none',
                  }}
                >
                  {labelText}
                </div>
              )}
            </div>
          );
        })}
    </div>
  );
};

export default DetectionOverlay;
