/**
 * SnapshotButton Component
 * Quick snapshot/screenshot functionality using go2rtc frame API
 */

import React, { useState } from 'react';
import { IconButton, Tooltip, CircularProgress, Snackbar, Alert } from '@mui/material';
import { CameraAlt as CameraIcon, Download as DownloadIcon } from '@mui/icons-material';
import { invoke } from '@tauri-apps/api/core';
import { isTauriAvailable } from '../utils/tauri';

interface SnapshotButtonProps {
  streamName: string;
  width?: number;
  height?: number;
  quality?: number;
  autoDownload?: boolean;
  filename?: string;
  onSnapshot?: (blob: Blob) => void;
  size?: 'small' | 'medium' | 'large';
}

const SnapshotButton: React.FC<SnapshotButtonProps> = ({
  streamName,
  width,
  height,
  quality = 90,
  autoDownload = true,
  filename,
  onSnapshot,
  size = 'medium',
}) => {
  const [isCapturing, setIsCapturing] = useState(false);
  const [showSuccess, setShowSuccess] = useState(false);
  const [showError, setShowError] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');

  const handleSnapshot = async () => {
    if (isCapturing) return;

    setIsCapturing(true);
    setShowError(false);

    try {
      if (isTauriAvailable()) {
        // Use Tauri backend for snapshot
        const imageData = await invoke<number[]>('get_go2rtc_snapshot', {
          streamName,
          width,
          height,
          quality,
        });

        const blob = new Blob([new Uint8Array(imageData)], { type: 'image/jpeg' });
        
        if (onSnapshot) {
          onSnapshot(blob);
        }

        if (autoDownload) {
          await downloadSnapshot(blob);
        }

        setShowSuccess(true);
      } else {
        // Browser-only fallback
        console.warn('[Snapshot] Tauri not available, using browser fallback');
        setErrorMessage('Snapshot feature requires Tauri environment');
        setShowError(true);
      }
    } catch (error) {
      console.error('[Snapshot] Failed to capture:', error);
      setErrorMessage(error instanceof Error ? error.message : 'Failed to capture snapshot');
      setShowError(true);
    } finally {
      setIsCapturing(false);
    }
  };

  const downloadSnapshot = async (blob: Blob): Promise<void> => {
    try {
      const timestamp = new Date().toISOString().replace(/[:.]/g, '-').split('T');
      const date = timestamp[0];
      const time = timestamp[1]?.split('-').slice(0, 3).join('-') || '00-00-00';
      const defaultFilename = `${streamName}_${date}_${time}.jpg`;
      const finalFilename = filename || defaultFilename;

      if (isTauriAvailable()) {
        // Use Tauri save_screenshot command
        const arrayBuffer = await blob.arrayBuffer();
        const uint8Array = new Uint8Array(arrayBuffer);
        const dataArray = Array.from(uint8Array);

        await invoke('save_screenshot', {
          filename: finalFilename,
          data: dataArray,
        });
      } else {
        // Browser download fallback
        const url = URL.createObjectURL(blob);
        const link = document.createElement('a');
        link.href = url;
        link.download = finalFilename;
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
        URL.revokeObjectURL(url);
      }
    } catch (error) {
      console.error('[Snapshot] Download failed:', error);
      throw error;
    }
  };

  return (
    <>
      <Tooltip title="Take Snapshot" arrow>
        <IconButton
          onClick={handleSnapshot}
          disabled={isCapturing}
          size={size}
          sx={{
            backgroundColor: 'rgba(33, 150, 243, 0.9)',
            color: 'white',
            '&:hover': {
              backgroundColor: 'rgba(25, 118, 210, 1)',
            },
            '&:disabled': {
              backgroundColor: 'rgba(0, 0, 0, 0.3)',
            },
            boxShadow: '0 2px 8px rgba(0,0,0,0.3)',
          }}
        >
          {isCapturing ? (
            <CircularProgress size={size === 'small' ? 16 : 24} sx={{ color: 'white' }} />
          ) : autoDownload ? (
            <DownloadIcon fontSize={size} />
          ) : (
            <CameraIcon fontSize={size} />
          )}
        </IconButton>
      </Tooltip>

      <Snackbar
        open={showSuccess}
        autoHideDuration={3000}
        onClose={() => setShowSuccess(false)}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
      >
        <Alert severity="success" onClose={() => setShowSuccess(false)}>
          Snapshot saved successfully
        </Alert>
      </Snackbar>

      <Snackbar
        open={showError}
        autoHideDuration={5000}
        onClose={() => setShowError(false)}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
      >
        <Alert severity="error" onClose={() => setShowError(false)}>
          {errorMessage}
        </Alert>
      </Snackbar>
    </>
  );
};

export default SnapshotButton;
