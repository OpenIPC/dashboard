import React from 'react';
import './GridControls.css';
import { useLocalization } from '../contexts/LocalizationContext';

interface CellControlsProps {
  isFullscreen: boolean;
  isRecording: boolean;
  isRecordingPending?: boolean;
  isMuted: boolean;
  streamId: number;
  onStreamSwitch: () => void;
  onAudio: () => void;
  onRecord: () => void;
  onClose: () => void;
  onArchive?: () => void;
}

const CellControls: React.FC<CellControlsProps> = ({
  isFullscreen,
  isRecording,
  isRecordingPending = false,
  isMuted,
  streamId,
  onStreamSwitch,
  onAudio,
  onRecord,
  onClose,
  onArchive,
}) => {
  const { t } = useLocalization();
  
  return (
    <div className="cell-controls">
      {/* Кнопка переключения качества */}
      <button 
        className="icon-button stream-switch-btn" 
        title={t('switch_quality')}
        onClick={(e) => {
          e.stopPropagation();
          onStreamSwitch();
        }}
      >
        <i className="material-icons">
          {streamId === 0 ? 'hd' : 'sd'}
        </i>
      </button>

      {/* Кнопка звука */}
      <button 
        className="icon-button audio-btn" 
        title={isMuted ? t('enable_audio') : t('disable_audio')}
        onClick={(e) => {
          e.stopPropagation();
          onAudio();
        }}
      >
        <i className="material-icons">
          {isMuted ? 'volume_off' : 'volume_up'}
        </i>
      </button>

      {/* Кнопка записи */}
      <button 
        className={`icon-button record-btn ${isRecording ? 'recording' : ''}`}
        title={isRecording ? t('stop_recording') : t('start_recording')}
        disabled={isRecordingPending}
        onClick={(e) => {
          e.stopPropagation();
          onRecord();
        }}
        aria-busy={isRecordingPending}
      >
        <i className="material-icons">fiber_manual_record</i>
      </button>

      {/* Кнопка архива */}
      {onArchive && (
        <button 
          className="icon-button archive-btn" 
          title={t('open_archive')}
          onClick={(e) => {
            e.stopPropagation();
            onArchive();
          }}
        >
          <i className="material-icons">video_library</i>
        </button>
      )}

      {/* Кнопка полноэкранного режима */}
      {/* Кнопка закрытия (скрывается в полноэкранном режиме) */}
      {!isFullscreen && (
        <button 
          className="icon-button close-btn" 
          title={t('close')}
          onClick={(e) => {
            e.stopPropagation();
            onClose();
          }}
        >
          <i className="material-icons">close</i>
        </button>
      )}
    </div>
  );
};

export default CellControls;