import React, { useCallback, useEffect, useRef, useState } from 'react';
import { Box } from '@mui/material';
import type { ObjectCounterLine, ObjectCounterZone, ObjectCounterPoint, AnalyticsDetectionBox } from '../../services/analytics';

interface AnalyticsOverlayProps {
  videoElement: HTMLVideoElement | null;
  lines: ObjectCounterLine[];
  zones: ObjectCounterZone[];
  detections?: AnalyticsDetectionBox[];
}

interface CanvasMetrics {
  width: number;
  height: number;
  offsetX: number;
  offsetY: number;
  scaleX: number;
  scaleY: number;
}

interface InterpolatedDetection extends AnalyticsDetectionBox {
  interpolatedBounds: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
}

const AnalyticsOverlay: React.FC<AnalyticsOverlayProps> = ({
  videoElement,
  lines,
  zones,
  detections = [],
}) => {
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const [metrics, setMetrics] = useState<CanvasMetrics | null>(null);
  const devicePixelRatioRef = useRef(typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1);
  
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
    const interpolationDuration = 100; 

    const animate = () => {
      const elapsed = Date.now() - startTime;
      const progress = Math.min(elapsed / interpolationDuration, 1);

      const interpolated = detections.map(currentDet => {
        const prevDet = prevDetectionsRef.current.get(currentDet.id);
        
        if (!prevDet) {
          return {
            ...currentDet,
            interpolatedBounds: { ...currentDet.bounds },
          };
        }

        if (progress < 1) {
          return {
            ...currentDet,
            interpolatedBounds: {
              x: prevDet.bounds.x + (currentDet.bounds.x - prevDet.bounds.x) * progress,
              y: prevDet.bounds.y + (currentDet.bounds.y - prevDet.bounds.y) * progress,
              width: prevDet.bounds.width + (currentDet.bounds.width - prevDet.bounds.width) * progress,
              height: prevDet.bounds.height + (currentDet.bounds.height - prevDet.bounds.height) * progress,
            },
          };
        }
        return {
          ...currentDet,
          interpolatedBounds: { ...currentDet.bounds },
        };
      });

      setInterpolatedDetections(interpolated);

      if (progress < 1) {
        animationFrameRef.current = requestAnimationFrame(animate);
      } else {
        prevDetectionsRef.current = newDetectionsMap;
      }
    };

    animationFrameRef.current = requestAnimationFrame(animate);

    return () => {
      if (animationFrameRef.current !== null) {
        cancelAnimationFrame(animationFrameRef.current);
      }
    };
  }, [detections]);

  const updateMetrics = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas || !videoElement) {
      setMetrics(null);
      return;
    }

    const canvasRect = canvas.getBoundingClientRect();
    // Use videoWidth/videoHeight to determine aspect ratio if available
    // But getBoundingClientRect gives the rendered size which is what we need relative to the viewport
    // However, if object-fit is contain, the video might be smaller than the element rect.
    // We need to calculate the actual video display rect within the video element.
    
    const videoRect = videoElement.getBoundingClientRect();
    if (!videoRect.width || !videoRect.height || !videoElement.videoWidth || !videoElement.videoHeight) {
      setMetrics(null);
      return;
    }

    // Calculate actual video dimensions inside the element (handling object-fit: contain)
    const videoRatio = videoElement.videoWidth / videoElement.videoHeight;
    const elementRatio = videoRect.width / videoRect.height;
    
    let renderWidth = videoRect.width;
    let renderHeight = videoRect.height;
    let renderLeft = videoRect.left;
    let renderTop = videoRect.top;

    if (videoRatio > elementRatio) {
      // Video is wider than element (black bars top/bottom)
      renderHeight = renderWidth / videoRatio;
      renderTop = videoRect.top + (videoRect.height - renderHeight) / 2;
    } else {
      // Video is taller than element (black bars left/right)
      renderWidth = renderHeight * videoRatio;
      renderLeft = videoRect.left + (videoRect.width - renderWidth) / 2;
    }

    const offsetX = renderLeft - canvasRect.left;
    const offsetY = renderTop - canvasRect.top;

    setMetrics({
      width: canvasRect.width,
      height: canvasRect.height,
      offsetX,
      offsetY,
      scaleX: renderWidth,
      scaleY: renderHeight,
    });
  }, [videoElement]);

  useEffect(() => {
    updateMetrics();
    if (!videoElement) {
      return;
    }
    const handleResize = () => updateMetrics();
    window.addEventListener('resize', handleResize);

    // Listen for video metadata loading to ensure we have correct dimensions
    videoElement.addEventListener('loadedmetadata', handleResize);
    videoElement.addEventListener('resize', handleResize); // Video element resize event

    const resizeObserver = new ResizeObserver(() => updateMetrics());
    resizeObserver.observe(videoElement);

    return () => {
      window.removeEventListener('resize', handleResize);
      videoElement.removeEventListener('loadedmetadata', handleResize);
      videoElement.removeEventListener('resize', handleResize);
      resizeObserver.disconnect();
    };
  }, [updateMetrics, videoElement]);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) {
      return;
    }
    if (!metrics) {
      canvas.width = 0;
      canvas.height = 0;
      return;
    }
    const ratio = (typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1);
    devicePixelRatioRef.current = ratio;
    canvas.width = Math.max(1, Math.round(metrics.width * ratio));
    canvas.height = Math.max(1, Math.round(metrics.height * ratio));
    canvas.style.width = `${metrics.width}px`;
    canvas.style.height = `${metrics.height}px`;
  }, [metrics]);

  const toCanvasCoords = useCallback(
    (point: ObjectCounterPoint | null): [number, number] | null => {
      if (!metrics || !point) {
        return null;
      }
      const x = metrics.offsetX + point.x * metrics.scaleX;
      const y = metrics.offsetY + point.y * metrics.scaleY;
      return [x, y];
    },
    [metrics],
  );

  const renderLine = useCallback((ctx: CanvasRenderingContext2D, line: ObjectCounterLine) => {
    const start = toCanvasCoords(line.start);
    const end = toCanvasCoords(line.end);
    if (!start || !end) {
      return;
    }

    // Draw Line Shadow/Outline
    ctx.lineWidth = 6;
    ctx.strokeStyle = 'rgba(0, 0, 0, 0.5)';
    ctx.lineCap = 'round';
    ctx.beginPath();
    ctx.moveTo(start[0], start[1]);
    ctx.lineTo(end[0], end[1]);
    ctx.stroke();

    // Draw Line
    ctx.lineWidth = 3;
    ctx.strokeStyle = '#22d3ee'; // Cyan
    ctx.beginPath();
    ctx.moveTo(start[0], start[1]);
    ctx.lineTo(end[0], end[1]);
    ctx.stroke();

    // Draw Direction Arrow (at midpoint)
    const midX = (start[0] + end[0]) / 2;
    const midY = (start[1] + end[1]) / 2;
    
    const dx = end[0] - start[0];
    const dy = end[1] - start[1];
    const len = Math.sqrt(dx * dx + dy * dy);

    if (len > 0) {
      // Normal vector (perpendicular to line)
      // If line is A->B, normal is (-dy, dx) [90 deg counter-clockwise] or (dy, -dx) [90 deg clockwise]
      // Let's define "Forward" as 90 deg clockwise (Right hand side)
      const nx = -dy / len;
      const ny = dx / len;
      const arrowLen = 30;

      const drawArrow = (factor: number, label: string) => {
        const tipX = midX + nx * arrowLen * factor;
        const tipY = midY + ny * arrowLen * factor;

        ctx.beginPath();
        ctx.moveTo(midX, midY);
        ctx.lineTo(tipX, tipY);
        ctx.lineWidth = 3;
        ctx.strokeStyle = '#22d3ee';
        ctx.stroke();

        // Arrowhead
        const headLen = 10;
        const angle = Math.atan2(tipY - midY, tipX - midX);
        ctx.beginPath();
        ctx.moveTo(tipX, tipY);
        ctx.lineTo(tipX - headLen * Math.cos(angle - Math.PI / 6), tipY - headLen * Math.sin(angle - Math.PI / 6));
        ctx.lineTo(tipX - headLen * Math.cos(angle + Math.PI / 6), tipY - headLen * Math.sin(angle + Math.PI / 6));
        ctx.closePath();
        ctx.fillStyle = '#22d3ee';
        ctx.fill();

        // Label
        ctx.font = 'bold 12px sans-serif';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        const labelX = tipX + (nx * factor * 15);
        const labelY = tipY + (ny * factor * 15);
        
        // Label background
        const textMetrics = ctx.measureText(label);
        const bgPad = 4;
        ctx.fillStyle = 'rgba(0, 0, 0, 0.7)';
        ctx.fillRect(
          labelX - textMetrics.width / 2 - bgPad, 
          labelY - 8 - bgPad, 
          textMetrics.width + bgPad * 2, 
          16 + bgPad * 2
        );

        ctx.fillStyle = '#fff';
        ctx.fillText(label, labelX, labelY);
      };

      // Draw arrows based on direction
      // We use "A" and "B" to denote sides relative to the line direction
      if (line.direction === 'forward') {
        drawArrow(1, 'IN');
      } else if (line.direction === 'backward') {
        drawArrow(-1, 'IN');
      } else {
        // Bidirectional
        drawArrow(1, 'A');
        drawArrow(-1, 'B');
      }
    }

    // Draw Endpoints
    [start, end].forEach((point, idx) => {
      ctx.beginPath();
      ctx.arc(point[0], point[1], 6, 0, Math.PI * 2);
      ctx.fillStyle = idx === 0 ? '#0ea5e9' : '#0ea5e9'; // Start/End same color for now
      ctx.fill();
      ctx.strokeStyle = '#fff';
      ctx.lineWidth = 2;
      ctx.stroke();
      
      // Label A/B
      ctx.fillStyle = '#fff';
      ctx.font = 'bold 10px sans-serif';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText(idx === 0 ? 'A' : 'B', point[0], point[1]);
    });

    // Draw Label
    if (line.name) {
      ctx.font = 'bold 14px sans-serif';
      ctx.fillStyle = '#fff';
      ctx.strokeStyle = 'rgba(0,0,0,0.8)';
      ctx.lineWidth = 3;
      ctx.textAlign = 'center';
      ctx.strokeText(line.name, midX, midY - 15);
      ctx.fillText(line.name, midX, midY - 15);
    }
  }, [toCanvasCoords]);

  const renderZone = useCallback((ctx: CanvasRenderingContext2D, zone: ObjectCounterZone) => {
    if (!zone.polygon.length) {
      return;
    }
    const points = zone.polygon
      .map(point => toCanvasCoords(point))
      .filter((point): point is [number, number] => Boolean(point));
    if (points.length < 3) {
      return;
    }

    ctx.lineWidth = 2;
    ctx.strokeStyle = '#f97316'; // Orange
    ctx.fillStyle = 'rgba(249, 115, 22, 0.2)';
    ctx.lineJoin = 'round';
    ctx.lineCap = 'round';
    
    ctx.beginPath();
    ctx.moveTo(points[0][0], points[0][1]);
    points.slice(1).forEach(([x, y]) => ctx.lineTo(x, y));
    ctx.closePath();
    ctx.fill();
    
    // Shadow for outline
    ctx.save();
    ctx.shadowColor = 'rgba(0,0,0,0.5)';
    ctx.shadowBlur = 4;
    ctx.stroke();
    ctx.restore();

    // Draw Label (at centroid)
    if (zone.name) {
      const centerX = points.reduce((sum, p) => sum + p[0], 0) / points.length;
      const centerY = points.reduce((sum, p) => sum + p[1], 0) / points.length;
      
      ctx.font = 'bold 14px sans-serif';
      ctx.fillStyle = '#fff';
      ctx.strokeStyle = 'rgba(0,0,0,0.8)';
      ctx.lineWidth = 3;
      ctx.textAlign = 'center';
      ctx.strokeText(zone.name, centerX, centerY);
      ctx.fillText(zone.name, centerX, centerY);
    }
  }, [toCanvasCoords]);

  const renderDetection = useCallback((ctx: CanvasRenderingContext2D, detection: InterpolatedDetection) => {
    if (!metrics) return;
    
    const { x, y, width, height } = detection.interpolatedBounds;
    
    // Convert normalized coordinates to canvas coordinates
    const canvasX = metrics.offsetX + x * metrics.scaleX;
    const canvasY = metrics.offsetY + y * metrics.scaleY;
    const canvasW = width * metrics.scaleX;
    const canvasH = height * metrics.scaleY;

    // Draw Bounding Box
    ctx.lineWidth = 2;
    ctx.strokeStyle = '#00ff00'; // Green
    ctx.strokeRect(canvasX, canvasY, canvasW, canvasH);

    // Draw Label
    if (detection.label) {
      const text = `${detection.label} ${(detection.confidence * 100).toFixed(0)}%`;
      ctx.font = '12px sans-serif';
      const textMetrics = ctx.measureText(text);
      
      ctx.fillStyle = '#00ff00';
      ctx.fillRect(canvasX, canvasY - 16, textMetrics.width + 4, 16);
      
      ctx.fillStyle = '#000';
      ctx.fillText(text, canvasX + 2, canvasY - 4);
    }
  }, [metrics]);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || !metrics) {
      return;
    }
    const ctx = canvas.getContext('2d');
    if (!ctx) {
      return;
    }
    const ratio = devicePixelRatioRef.current;
    ctx.save();
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.restore();
    ctx.save();
    ctx.scale(ratio, ratio);

    lines.forEach(line => renderLine(ctx, line));
    zones.forEach(zone => renderZone(ctx, zone));
    interpolatedDetections.forEach(det => renderDetection(ctx, det));

    ctx.restore();
  }, [lines, zones, interpolatedDetections, metrics, renderLine, renderZone, renderDetection]);

  return (
    <Box
      sx={{
        position: 'absolute',
        inset: 0,
        pointerEvents: 'none', // Allow clicks to pass through to video controls if any
        zIndex: 10,
      }}
    >
      <canvas
        ref={canvasRef}
        style={{
          position: 'absolute',
          inset: 0,
          width: '100%',
          height: '100%',
        }}
      />
    </Box>
  );
};

export default AnalyticsOverlay;
