import React from 'react';
import { Alert, Snackbar } from '@mui/material';
import type { ToastSeverity } from '../hooks/useToast';

interface ToastProps {
  message: string;
  severity?: ToastSeverity;
  open: boolean;
  onClose: () => void;
  autoHideDuration?: number;
}

export const Toast: React.FC<ToastProps> = ({ 
  message, 
  severity = 'success', 
  open, 
  onClose, 
  autoHideDuration = 3000 
}) => {
  return (
    <Snackbar
      open={open}
      autoHideDuration={autoHideDuration}
      onClose={onClose}
      anchorOrigin={{ vertical: 'top', horizontal: 'right' }}
    >
      <Alert 
        onClose={onClose} 
        severity={severity} 
        sx={{ width: '100%' }}
      >
        {message}
      </Alert>
    </Snackbar>
  );
};