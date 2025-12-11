import React, { useState, useRef, useEffect, useCallback } from 'react';
import { Box } from '@mui/material';

interface VirtualJoystickProps {
  onMove: (x: number, y: number) => void;
  onStop: () => void;
  size?: number;
  stickSize?: number;
}

const VirtualJoystick: React.FC<VirtualJoystickProps> = ({ 
  onMove, 
  onStop, 
  size = 120, 
  stickSize = 50 
}) => {
  const [position, setPosition] = useState({ x: 0, y: 0 });
  const [isDragging, setIsDragging] = useState(false);
  const containerRef = useRef<HTMLDivElement>(null);
  const lastSendRef = useRef<number>(0);
  const moveRef = useRef({ x: 0, y: 0 });

  const radius = size / 2;
  const maxDist = radius - stickSize / 2;

  const handleStart = (clientX: number, clientY: number) => {
    setIsDragging(true);
    updatePosition(clientX, clientY);
  };

  const handleEnd = () => {
    setIsDragging(false);
    setPosition({ x: 0, y: 0 });
    moveRef.current = { x: 0, y: 0 };
    onStop();
  };

  const updatePosition = (clientX: number, clientY: number) => {
    if (!containerRef.current) return;

    const rect = containerRef.current.getBoundingClientRect();
    const centerX = rect.left + radius;
    const centerY = rect.top + radius;

    let dx = clientX - centerX;
    let dy = clientY - centerY;
    const distance = Math.sqrt(dx * dx + dy * dy);

    // Normalize if outside max distance
    if (distance > maxDist) {
      const angle = Math.atan2(dy, dx);
      dx = Math.cos(angle) * maxDist;
      dy = Math.sin(angle) * maxDist;
    }

    setPosition({ x: dx, y: dy });

    // Calculate normalized velocity (-1.0 to 1.0)
    const vx = dx / maxDist;
    const vy = -dy / maxDist; // Invert Y because screen Y is down, but PTZ Up is positive

    moveRef.current = { x: vx, y: vy };
    
    // Throttle updates to avoid flooding
    const now = Date.now();
    if (now - lastSendRef.current > 100) { // 100ms throttle
      onMove(vx, vy);
      lastSendRef.current = now;
    }
  };

  useEffect(() => {
    const handleWindowMove = (e: MouseEvent | TouchEvent) => {
      if (!isDragging) return;
      
      let clientX, clientY;
      if ('touches' in e) {
        clientX = e.touches[0].clientX;
        clientY = e.touches[0].clientY;
      } else {
        clientX = (e as MouseEvent).clientX;
        clientY = (e as MouseEvent).clientY;
      }
      
      updatePosition(clientX, clientY);
    };

    const handleWindowUp = () => {
      if (isDragging) {
        handleEnd();
      }
    };

    if (isDragging) {
      window.addEventListener('mousemove', handleWindowMove);
      window.addEventListener('mouseup', handleWindowUp);
      window.addEventListener('touchmove', handleWindowMove);
      window.addEventListener('touchend', handleWindowUp);
    }

    return () => {
      window.removeEventListener('mousemove', handleWindowMove);
      window.removeEventListener('mouseup', handleWindowUp);
      window.removeEventListener('touchmove', handleWindowMove);
      window.removeEventListener('touchend', handleWindowUp);
    };
  }, [isDragging]);

  return (
    <Box
      ref={containerRef}
      onMouseDown={(e) => handleStart(e.clientX, e.clientY)}
      onTouchStart={(e) => handleStart(e.touches[0].clientX, e.touches[0].clientY)}
      sx={{
        width: size,
        height: size,
        borderRadius: '50%',
        bgcolor: 'rgba(255, 255, 255, 0.1)',
        border: '2px solid rgba(255, 255, 255, 0.3)',
        position: 'relative',
        touchAction: 'none',
        cursor: 'pointer',
        backdropFilter: 'blur(4px)',
        boxShadow: '0 4px 30px rgba(0, 0, 0, 0.1)',
      }}
    >
      {/* Stick */}
      <Box
        sx={{
          width: stickSize,
          height: stickSize,
          borderRadius: '50%',
          bgcolor: isDragging ? 'primary.main' : 'rgba(255, 255, 255, 0.8)',
          position: 'absolute',
          top: '50%',
          left: '50%',
          marginTop: `-${stickSize / 2}px`,
          marginLeft: `-${stickSize / 2}px`,
          transform: `translate(${position.x}px, ${position.y}px)`,
          transition: isDragging ? 'none' : 'all 0.2s ease-out',
          boxShadow: '0 2px 10px rgba(0,0,0,0.3)',
          pointerEvents: 'none', // Let events pass to container
        }}
      />
    </Box>
  );
};

export default VirtualJoystick;
