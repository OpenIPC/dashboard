import React from 'react';
import './GridControls.css';
import { useLocalization } from '../hooks/useLocalization';

interface CellControlsProps {
  isFullscreen: boolean;
  isRecording: boolean;
  isRecordingPending?: boolean;
  isMuted: boolean;
  streamId: number;
  streamName?: string;
  enableSnapshot?: boolean;
  onStreamSwitch: () => void;
  onAudio: () => void;
  onRecord: () => void;
  onClose: () => void;
  onSnapshot?: () => void;
  moduleToggles?: ModuleToggleProps[];
}

interface ModuleToggleProps {
  moduleId: string;
  label: string;
  icon: string;
  tooltip: string;
  active: boolean;
  disabled: boolean;
  onToggle: () => void;
}

const CellControls: React.FC<CellControlsProps> = ({
  isFullscreen,
  isRecording,
  isRecordingPending = false,
  isMuted,
  streamId,
  streamName,
  enableSnapshot = false,
  onStreamSwitch,
  onAudio,
  onRecord,
  onClose,
  onSnapshot,
  moduleToggles = [],
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

      {/* Кнопки модулей аналитики */}
      {moduleToggles.map(toggle => (
        <button
          key={toggle.moduleId}
          className={`icon-button module-btn${toggle.active ? ' active' : ''}`}
          title={toggle.tooltip}
          disabled={toggle.disabled}
          aria-pressed={toggle.active}
          aria-label={toggle.label}
          onClick={(e) => {
            e.stopPropagation();
            toggle.onToggle();
          }}
        >
          <i className="material-icons">{toggle.icon}</i>
        </button>
      ))}

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

      {/* Кнопка снимка */}
      {enableSnapshot && onSnapshot && (
        <button 
          className="icon-button snapshot-btn" 
          title={t('take_snapshot') || 'Take Snapshot'}
          onClick={(e) => {
            e.stopPropagation();
            onSnapshot();
          }}
        >
          <i className="material-icons">photo_camera</i>
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