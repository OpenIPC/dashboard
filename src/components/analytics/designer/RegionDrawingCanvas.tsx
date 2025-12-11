import React, { useCallback, useEffect, useRef, useState } from 'react';
import { Box, Typography } from '@mui/material';
import { useLocalization } from '../../../hooks/useLocalization';
import type { RegionDesignerMode } from './RegionDesignerTypes';

export interface RegionPoint {
  x: number;
  y: number;
}

export interface RegionLineDraft {
  id?: number;
  start: RegionPoint;
  end: RegionPoint;
  direction?: 'bidirectional' | 'incoming' | 'outgoing';
}

export interface RegionZoneDraft {
  id?: number;
  polygon: RegionPoint[];
}

export interface CanvasMetrics {
  width: number;
  height: number;
  offsetX: number;
  offsetY: number;
  scaleX: number;
  scaleY: number;
}

interface RegionDrawingCanvasProps {
  mode: RegionDesignerMode;
  videoElement: HTMLVideoElement | null;
  lineDraft: RegionLineDraft | null;
  zoneDraft: RegionZoneDraft | null;
  onLineChange: (draft: RegionLineDraft | null) => void;
  onZoneChange: (draft: RegionZoneDraft | null) => void;
  onMetricsChange?: (metrics: CanvasMetrics | null) => void;
}

const clamp01 = (value: number) => Math.max(0, Math.min(1, value));

const pointsAlmostEqual = (a?: RegionPoint, b?: RegionPoint) => {
  if (!a || !b) {
    return false;
  }
  const dx = a.x - b.x;
  const dy = a.y - b.y;
  return Math.hypot(dx, dy) < 0.003;
};

const distanceToSegment = (p: [number, number], v: [number, number], w: [number, number]) => {
  const l2 = (w[0] - v[0]) ** 2 + (w[1] - v[1]) ** 2;
  if (l2 === 0) return Math.hypot(p[0] - v[0], p[1] - v[1]);
  let t = ((p[0] - v[0]) * (w[0] - v[0]) + (p[1] - v[1]) * (w[1] - v[1])) / l2;
  t = Math.max(0, Math.min(1, t));
  return Math.hypot(p[0] - (v[0] + t * (w[0] - v[0])), p[1] - (v[1] + t * (w[1] - v[1])));
};

const isPointInPolygon = (point: [number, number], polygon: [number, number][]) => {
  let inside = false;
  for (let i = 0, j = polygon.length - 1; i < polygon.length; j = i++) {
    const xi = polygon[i][0], yi = polygon[i][1];
    const xj = polygon[j][0], yj = polygon[j][1];
    const intersect = ((yi > point[1]) !== (yj > point[1]))
        && (point[0] < (xj - xi) * (point[1] - yi) / (yj - yi) + xi);
    if (intersect) inside = !inside;
  }
  return inside;
};

const RegionDrawingCanvas: React.FC<RegionDrawingCanvasProps> = ({
  mode,
  videoElement,
  lineDraft,
  zoneDraft,
  onLineChange,
  onZoneChange,
  onMetricsChange,
}) => {
  const { t } = useLocalization();
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const [isDragging, setIsDragging] = useState(false);
  const [dragIndex, setDragIndex] = useState<number | null>(null);
  const [dragTarget, setDragTarget] = useState<'start' | 'end' | 'point' | 'whole-line' | 'whole-zone' | null>(null);
  const [hoverTarget, setHoverTarget] = useState<'start' | 'end' | 'point' | 'whole-line' | 'whole-zone' | null>(null);
  const [hoverIndex, setHoverIndex] = useState<number | null>(null);
  const [dragStartMouse, setDragStartMouse] = useState<RegionPoint | null>(null);
  const [initialLineDraft, setInitialLineDraft] = useState<RegionLineDraft | null>(null);
  const [initialZoneDraft, setInitialZoneDraft] = useState<RegionZoneDraft | null>(null);
  const [metrics, setMetrics] = useState<CanvasMetrics | null>(null);
  const devicePixelRatioRef = useRef(typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1);

  const updateMetrics = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas || !videoElement) {
      setMetrics(null);
      onMetricsChange?.(null);
      return;
    }

    const canvasRect = canvas.getBoundingClientRect();
    const videoRect = videoElement.getBoundingClientRect();
    if (!videoRect.width || !videoRect.height) {
      setMetrics(null);
      onMetricsChange?.(null);
      return;
    }

    const offsetX = videoRect.left - canvasRect.left;
    const offsetY = videoRect.top - canvasRect.top;

    const nextMetrics: CanvasMetrics = {
      width: canvasRect.width,
      height: canvasRect.height,
      offsetX,
      offsetY,
      scaleX: videoRect.width,
      scaleY: videoRect.height,
    };
    setMetrics(nextMetrics);
    onMetricsChange?.(nextMetrics);
  }, [onMetricsChange, videoElement]);

  useEffect(() => {
    updateMetrics();
    if (!videoElement) {
      return;
    }
    const handleResize = () => updateMetrics();
    window.addEventListener('resize', handleResize);

    const resizeObserver = new ResizeObserver(() => updateMetrics());
    resizeObserver.observe(videoElement);

    return () => {
      window.removeEventListener('resize', handleResize);
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
    (point: RegionPoint | null): [number, number] | null => {
      if (!metrics || !point) {
        return null;
      }
      const x = metrics.offsetX + point.x * metrics.scaleX;
      const y = metrics.offsetY + point.y * metrics.scaleY;
      return [x, y];
    },
    [metrics],
  );

  const fromCanvasCoords = useCallback(
    (x: number, y: number): RegionPoint | null => {
      if (!metrics || !metrics.scaleX || !metrics.scaleY) {
        return null;
      }
      return {
        x: clamp01((x - metrics.offsetX) / metrics.scaleX),
        y: clamp01((y - metrics.offsetY) / metrics.scaleY),
      };
    },
    [metrics],
  );

  const renderLine = useCallback((ctx: CanvasRenderingContext2D, draft: RegionLineDraft) => {
    if (pointsAlmostEqual(draft.start, draft.end)) {
      return;
    }
    const start = toCanvasCoords(draft.start);
    const end = toCanvasCoords(draft.end);
    if (!start || !end) {
      return;
    }

    const isHovered = hoverTarget === 'whole-line';
    const isStartHovered = hoverTarget === 'start';
    const isEndHovered = hoverTarget === 'end';

    // Draw Line
    ctx.lineWidth = isHovered ? 5 : 3;
    ctx.strokeStyle = isHovered ? '#67e8f9' : '#22d3ee';
    ctx.beginPath();
    ctx.moveTo(start[0], start[1]);
    ctx.lineTo(end[0], end[1]);
    ctx.stroke();

    // Draw Direction Arrow (at midpoint)
    const midX = (start[0] + end[0]) / 2;
    const midY = (start[1] + end[1]) / 2;
    const angle = Math.atan2(end[1] - start[1], end[0] - start[0]);
    
    ctx.save();
    ctx.translate(midX, midY);
    ctx.rotate(angle);
    
    // Draw main arrow (A -> B)
    ctx.beginPath();
    ctx.moveTo(-6, -6);
    ctx.lineTo(8, 0);
    ctx.lineTo(-6, 6);
    ctx.fillStyle = isHovered ? '#67e8f9' : '#22d3ee';
    ctx.fill();

    // If bidirectional, draw reverse arrow
    if (draft.direction === 'bidirectional') {
      ctx.beginPath();
      ctx.moveTo(6, -6);
      ctx.lineTo(-8, 0);
      ctx.lineTo(6, 6);
      ctx.fillStyle = isHovered ? '#67e8f9' : '#22d3ee';
      ctx.fill();
    }
    ctx.restore();

    // Draw Endpoints and Labels
    const drawPoint = (x: number, y: number, label: string, hovered: boolean) => {
      ctx.beginPath();
      ctx.arc(x, y, hovered ? 8 : 6, 0, Math.PI * 2);
      ctx.fillStyle = hovered ? '#38bdf8' : '#0ea5e9';
      ctx.fill();
      ctx.strokeStyle = '#fff';
      ctx.lineWidth = 2;
      ctx.stroke();

      // Label background
      ctx.fillStyle = 'rgba(0, 0, 0, 0.7)';
      ctx.beginPath();
      ctx.arc(x, y - 20, 10, 0, Math.PI * 2);
      ctx.fill();

      // Label text
      ctx.font = 'bold 12px sans-serif';
      ctx.fillStyle = '#fff';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText(label, x, y - 20);
    };

    drawPoint(start[0], start[1], 'A', isStartHovered);
    drawPoint(end[0], end[1], 'B', isEndHovered);
  }, [toCanvasCoords, hoverTarget]);

  const renderPolygon = useCallback((ctx: CanvasRenderingContext2D, draft: RegionZoneDraft) => {
    if (!draft.polygon.length) {
      return;
    }
    const points = draft.polygon
      .map(point => toCanvasCoords(point))
      .filter((point): point is [number, number] => Boolean(point));
    if (points.length < 3) {
      // Draw points even if not a full polygon yet
      points.forEach((p, idx) => {
        const isPointHovered = hoverTarget === 'point' && hoverIndex === idx;
        ctx.beginPath();
        ctx.arc(p[0], p[1], isPointHovered ? 7 : 5, 0, Math.PI * 2);
        ctx.fillStyle = isPointHovered ? '#fb923c' : '#f97316';
        ctx.fill();
      });
      return;
    }

    const isZoneHovered = hoverTarget === 'whole-zone';

    ctx.lineWidth = isZoneHovered ? 3 : 2;
    ctx.strokeStyle = isZoneHovered ? '#fb923c' : '#f97316';
    ctx.fillStyle = isZoneHovered ? 'rgba(249, 115, 22, 0.35)' : 'rgba(249, 115, 22, 0.25)';
    
    ctx.beginPath();
    ctx.moveTo(points[0][0], points[0][1]);
    points.slice(1).forEach(([x, y]) => ctx.lineTo(x, y));
    ctx.closePath();
    ctx.fill();
    ctx.stroke();

    // Draw vertices
    points.forEach(([x, y], idx) => {
      const isPointHovered = hoverTarget === 'point' && hoverIndex === idx;
      ctx.beginPath();
      ctx.arc(x, y, isPointHovered ? 7 : 5, 0, Math.PI * 2);
      ctx.fillStyle = isPointHovered ? '#fb923c' : '#f97316';
      ctx.fill();
      ctx.strokeStyle = '#fff';
      ctx.lineWidth = 1;
      ctx.stroke();
    });
  }, [toCanvasCoords, hoverTarget, hoverIndex]);

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

    if (mode === 'line' && lineDraft) {
      renderLine(ctx, lineDraft);
    } else if (mode === 'zone' && zoneDraft) {
      renderPolygon(ctx, zoneDraft);
    }
    ctx.restore();
  }, [lineDraft, mode, renderLine, renderPolygon, zoneDraft, metrics]);

  const handlePointerDown = (event: React.PointerEvent<HTMLCanvasElement>) => {
    if (event.button !== 0) {
      // Allow secondary clicks only for context menu suppression
      if (event.button === 2) {
        event.preventDefault();
      }
      return;
    }
    if (!metrics) {
      return;
    }
    const canvas = canvasRef.current;
    if (!canvas) {
      return;
    }
    if (mode === 'line' && !lineDraft) {
      return;
    }
    event.preventDefault();
    event.currentTarget.setPointerCapture(event.pointerId);
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;

    setIsDragging(true);

    if (mode === 'line' && lineDraft) {
      const isNewLine = !lineDraft.id;
      const needsPlacement = pointsAlmostEqual(lineDraft.start, lineDraft.end);
      const start = toCanvasCoords(lineDraft.start);
      const end = toCanvasCoords(lineDraft.end);
      if (isNewLine && needsPlacement) {
        const normalized = fromCanvasCoords(x, y);
        if (normalized) {
          onLineChange({ ...lineDraft, start: normalized, end: normalized });
          setDragTarget('end');
        }
        return;
      }
      if (start && Math.hypot(start[0] - x, start[1] - y) < 15) {
        setDragTarget('start');
      } else if (end && Math.hypot(end[0] - x, end[1] - y) < 15) {
        setDragTarget('end');
      } else if (start && end && distanceToSegment([x, y], start, end) < 10) {
        setDragTarget('whole-line');
        const normalized = fromCanvasCoords(x, y);
        if (normalized) {
          setDragStartMouse(normalized);
          setInitialLineDraft({ ...lineDraft, start: { ...lineDraft.start }, end: { ...lineDraft.end } });
        }
      } else {
        const normalized = fromCanvasCoords(x, y);
        if (normalized) {
          const shouldResetStart = !lineDraft.id && !pointsAlmostEqual(lineDraft.start, lineDraft.end);
          const nextLine: RegionLineDraft = shouldResetStart
            ? { ...lineDraft, start: normalized, end: normalized }
            : { ...lineDraft, end: normalized };
          onLineChange(nextLine);
          setDragTarget('end');
        }
      }
    } else if (mode === 'zone') {
      const workingDraft = zoneDraft ?? { polygon: [] };
      const candidates = workingDraft.polygon.map(point => toCanvasCoords(point));
      const index = candidates.findIndex(point => point && Math.hypot(point[0] - x, point[1] - y) < 15);
      if (index >= 0) {
        setDragIndex(index);
        setDragTarget('point');
      } else {
        const polygonPoints = candidates.filter((p): p is [number, number] => p !== null);
        if (polygonPoints.length >= 3 && isPointInPolygon([x, y], polygonPoints)) {
          setDragTarget('whole-zone');
          const normalized = fromCanvasCoords(x, y);
          if (normalized) {
            setDragStartMouse(normalized);
            setInitialZoneDraft({ ...workingDraft, polygon: workingDraft.polygon.map(p => ({ ...p })) });
          }
        } else {
          const normalized = fromCanvasCoords(x, y);
          if (normalized) {
            const nextPolygon = [...workingDraft.polygon, normalized];
            onZoneChange({ ...workingDraft, polygon: nextPolygon });
            setDragIndex(nextPolygon.length - 1);
            setDragTarget('point');
          }
        }
      }
    }
  };

  const handlePointerMove = (event: React.PointerEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas || !metrics) {
      return;
    }
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;
    const normalized = fromCanvasCoords(x, y);
    if (!normalized) {
      return;
    }

    if (isDragging) {
      if (mode === 'line' && lineDraft && dragTarget) {
        if (dragTarget === 'start') {
          onLineChange({ ...lineDraft, start: normalized });
        } else if (dragTarget === 'end') {
          onLineChange({ ...lineDraft, end: normalized });
        } else if (dragTarget === 'whole-line' && dragStartMouse && initialLineDraft) {
          const dx = normalized.x - dragStartMouse.x;
          const dy = normalized.y - dragStartMouse.y;
          const newStart = { x: clamp01(initialLineDraft.start.x + dx), y: clamp01(initialLineDraft.start.y + dy) };
          const newEnd = { x: clamp01(initialLineDraft.end.x + dx), y: clamp01(initialLineDraft.end.y + dy) };
          onLineChange({ ...lineDraft, start: newStart, end: newEnd });
        }
      } else if (mode === 'zone' && zoneDraft && dragTarget) {
        if (dragTarget === 'point' && dragIndex !== null) {
          const nextPolygon = zoneDraft.polygon.map((point, index) => (index === dragIndex ? normalized : point));
          onZoneChange({ ...zoneDraft, polygon: nextPolygon });
        } else if (dragTarget === 'whole-zone' && dragStartMouse && initialZoneDraft) {
          const dx = normalized.x - dragStartMouse.x;
          const dy = normalized.y - dragStartMouse.y;
          const nextPolygon = initialZoneDraft.polygon.map(point => ({
            x: clamp01(point.x + dx),
            y: clamp01(point.y + dy),
          }));
          onZoneChange({ ...zoneDraft, polygon: nextPolygon });
        }
      }
    } else {
      // Hover logic
      let nextHoverTarget: typeof hoverTarget = null;
      let nextHoverIndex: number | null = null;

      if (mode === 'line' && lineDraft) {
        const start = toCanvasCoords(lineDraft.start);
        const end = toCanvasCoords(lineDraft.end);
        if (start && Math.hypot(start[0] - x, start[1] - y) < 15) {
          nextHoverTarget = 'start';
        } else if (end && Math.hypot(end[0] - x, end[1] - y) < 15) {
          nextHoverTarget = 'end';
        } else if (start && end && distanceToSegment([x, y], start, end) < 10) {
          nextHoverTarget = 'whole-line';
        }
      } else if (mode === 'zone' && zoneDraft) {
        const candidates = zoneDraft.polygon.map(point => toCanvasCoords(point));
        const index = candidates.findIndex(point => point && Math.hypot(point[0] - x, point[1] - y) < 15);
        if (index >= 0) {
          nextHoverTarget = 'point';
          nextHoverIndex = index;
        } else {
          const polygonPoints = candidates.filter((p): p is [number, number] => p !== null);
          if (polygonPoints.length >= 3 && isPointInPolygon([x, y], polygonPoints)) {
            nextHoverTarget = 'whole-zone';
          }
        }
      }

      if (nextHoverTarget !== hoverTarget || nextHoverIndex !== hoverIndex) {
        setHoverTarget(nextHoverTarget);
        setHoverIndex(nextHoverIndex);
      }
    }
  };

  const handlePointerUp = (event?: React.PointerEvent<HTMLCanvasElement>) => {
    if (event && event.currentTarget.hasPointerCapture?.(event.pointerId)) {
      event.currentTarget.releasePointerCapture(event.pointerId);
    }
    setIsDragging(false);
    setDragTarget(null);
    setDragIndex(null);
    setDragStartMouse(null);
    setInitialLineDraft(null);
    setInitialZoneDraft(null);
  };

  const getCursor = () => {
    if (isDragging) return 'grabbing';
    if (hoverTarget === 'start' || hoverTarget === 'end' || hoverTarget === 'point') return 'grab';
    if (hoverTarget === 'whole-line' || hoverTarget === 'whole-zone') return 'move';
    return 'crosshair';
  };

  return (
    <Box
      sx={{
        position: 'absolute',
        inset: 0,
        zIndex: 10,
      }}
    >
      <canvas
        ref={canvasRef}
        width={1920}
        height={1080}
        style={{
          position: 'absolute',
          inset: 0,
          width: '100%',
          height: '100%',
          pointerEvents: 'auto',
          cursor: getCursor(),
          touchAction: 'none',
        }}
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerUp}
        onPointerCancel={handlePointerUp}
        onContextMenu={event => event.preventDefault()}
      />
      {(mode === 'line' && lineDraft) && (
        <Box
          sx={{
            position: 'absolute',
            top: 24,
            left: 24,
            bgcolor: 'rgba(0,0,0,0.7)',
            color: '#fff',
            borderRadius: 1,
            px: 2,
            py: 1.5,
            maxWidth: 320,
            pointerEvents: 'none',
          }}
        >
          <Typography variant="subtitle2" sx={{ fontWeight: 'bold', mb: 0.5 }}>
            {t('analytics_designer_line_crossing_setup')}
          </Typography>
          <Typography variant="body2" sx={{ opacity: 0.9 }}>
            • <span dangerouslySetInnerHTML={{ __html: t('analytics_designer_line_instruction_1').replace('A', '<b>A</b>').replace('B', '<b>B</b>') }} />
          </Typography>
          <Typography variant="body2" sx={{ opacity: 0.9 }}>
            • <span dangerouslySetInnerHTML={{ __html: t('analytics_designer_line_instruction_2').replace('A → B', '<b>A → B</b>') }} />
          </Typography>
          <Typography variant="body2" sx={{ opacity: 0.9 }}>
            • {t('analytics_designer_line_instruction_3')}
          </Typography>
        </Box>
      )}
      {(mode === 'zone' && (!zoneDraft || zoneDraft.polygon.length < 3)) && (
        <Box
          sx={{
            position: 'absolute',
            top: 24,
            left: 24,
            bgcolor: 'rgba(0,0,0,0.7)',
            color: '#fff',
            borderRadius: 1,
            px: 2,
            py: 1.5,
            maxWidth: 320,
            pointerEvents: 'none',
          }}
        >
          <Typography variant="subtitle2" sx={{ fontWeight: 'bold', mb: 0.5 }}>
            {t('analytics_designer_zone_setup')}
          </Typography>
          <Typography variant="body2" sx={{ opacity: 0.9 }}>
            • {t('analytics_designer_zone_instruction_1')}
          </Typography>
          <Typography variant="body2" sx={{ opacity: 0.9 }}>
            • {t('analytics_designer_zone_instruction_2')}
          </Typography>
          <Typography variant="body2" sx={{ opacity: 0.9 }}>
            • {t('analytics_designer_zone_instruction_3')}
          </Typography>
        </Box>
      )}
    </Box>
  );
};

export default RegionDrawingCanvas;
