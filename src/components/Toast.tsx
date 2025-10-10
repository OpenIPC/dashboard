import React, { useState } from 'react';
import { Alert, Snackbar } from '@mui/material';

interface ToastProps {
  message: string;
  severity?: 'success' | 'error' | 'warning' | 'info';
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

interface ToastState {
  open: boolean;
  message: string;
  severity: 'success' | 'error' | 'warning' | 'info';
}

export const useToast = () => {
  const [toast, setToast] = useState<ToastState>({
    open: false,
    message: '',
    severity: 'success'
  });

  const showToast = (message: string, severity: ToastState['severity'] = 'success') => {
    setToast({
      open: true,
      message,
      severity
    });
  };

  const hideToast = () => {
    setToast(prev => ({ ...prev, open: false }));
  };

  return {
    toast,
    showToast,
    hideToast
  };
};