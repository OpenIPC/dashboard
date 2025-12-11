import React, { useState } from 'react';
import './GridControls.css';
import { useLocalization } from '../hooks/useLocalization';

interface CellControlsProps {
  isFullscreen: boolean;
  isRecording: boolean;
  isRecordingPending?: boolean;
  isMuted: boolean;
  volume?: number;
  streamId: number;
  streamName?: string;
  enableSnapshot?: boolean;
  onStreamSwitch: () => void;
  onAudio: () => void;
  onVolumeChange?: (value: number) => void;
  onRecord: () => void;
  onClose: () => void;
  onSnapshot?: () => void;
  onPTZ?: () => void;
  isPTZActive?: boolean;
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
  volume = 1,
  streamId,
  streamName,
  enableSnapshot = false,
  onStreamSwitch,
  onAudio,
  onVolumeChange,
  onRecord,
  onClose,
  onSnapshot,
  onPTZ,
  isPTZActive = false,
  moduleToggles = [],
}) => {
  const { t } = useLocalization();
  const [audioHover, setAudioHover] = useState(false);
  const sliderVolume = Math.min(1, Math.max(0, Number.isFinite(volume) ? volume : 1));
  const sliderVisible = Boolean(!isMuted && onVolumeChange && audioHover);
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

      {/* Группа контролов звука (кнопка + ползунок) */}
      <div
        className={`audio-control-group${sliderVisible ? ' show-slider' : ''}`}
        onMouseEnter={() => setAudioHover(true)}
        onMouseLeave={() => setAudioHover(false)}
      >
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

        {!isMuted && onVolumeChange && (
          <div
            className="volume-slider"
            title={t('volume') || 'Volume'}
            onClick={event => event.stopPropagation()}
            onMouseDown={event => event.stopPropagation()}
            onTouchStart={event => event.stopPropagation()}
          >
            <input
              type="range"
              min={0}
              max={1}
              step={0.05}
              value={sliderVolume}
              aria-label={t('volume') || 'Volume'}
              onChange={event => onVolumeChange(Number(event.target.value))}
            />
          </div>
        )}
      </div>

      {/* Кнопка PTZ */}
      {onPTZ && (
        <button 
          className={`icon-button ptz-btn ${isPTZActive ? 'active' : ''}`}
          title={t('toggle_ptz') || 'PTZ Control'}
          onClick={(e) => {
            e.stopPropagation();
            onPTZ();
          }}
        >
          <i className="material-icons">open_with</i>
        </button>
      )}

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