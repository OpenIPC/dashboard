import React from 'react';
import { useLocalization } from '../contexts/LocalizationContext';

interface LoadingOverlayProps {
  isVisible: boolean;
  message?: string;
}

export const LoadingOverlay: React.FC<LoadingOverlayProps> = ({ 
  isVisible, 
  message 
}) => {
  const { t } = useLocalization();
  
  if (!isVisible) return null;

  return (
    <div className="loading-overlay">
      <div className="spinner"></div>
      <div className="loading-message">{message || t('loading_text')}</div>
    </div>
  );
};

interface ErrorOverlayProps {
  isVisible: boolean;
  message: string;
  onRetry: () => void;
  onClose: () => void;
}

export const ErrorOverlay: React.FC<ErrorOverlayProps> = ({ 
  isVisible, 
  message, 
  onRetry, 
  onClose 
}) => {
  const { t } = useLocalization();
  
  if (!isVisible) return null;

  return (
    <div className="error-overlay">
      <i className="material-icons error-icon">error_outline</i>
      <p className="error-message">{message}</p>
      <div className="error-buttons">
        <button className="retry-button" onClick={onRetry}>
          {t('retry_button')}
        </button>
        <button className="close-on-error-btn" onClick={onClose}>
          {t('close_button')}
        </button>
      </div>
    </div>
  );
};