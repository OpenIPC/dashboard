import React from 'react';

interface CellInfoProps {
  cameraName: string;
  streamId: number;
  stats?: {
    codec?: string;
    resolution?: string;
    bitrate?: string;
  };
}

const CellInfo: React.FC<CellInfoProps> = ({ cameraName, streamId, stats }) => {
  const qualityLabel = streamId === 0 ? 'HD' : 'SD';
  
  return (
    <>
      {/* Статистика потока - слева внизу */}
      {stats && (
        <div 
          className="cell-stats"
          style={{
            position: 'absolute',
            bottom: '0px',
            left: '0px',
            color: 'white',
            backgroundColor: 'rgba(0,0,0,0.8)',
            padding: '4px 8px',
            borderRadius: '4px',
            fontSize: '11px',
            fontFamily: 'monospace',
            border: '1px solid rgba(255,255,255,0.2)'
          }}
        >
          {stats.codec && stats.bitrate && `${stats.codec} | ${stats.bitrate}`}
          {stats.resolution && ` | ${stats.resolution}`}
        </div>
      )}

      {/* Название камеры и качество - справа внизу */}
      <div 
        className="cell-name"
        style={{
          position: 'absolute',
          bottom: '0px',
          right: '0px',
          color: 'white',
          backgroundColor: 'rgba(0,0,0,0.8)',
          padding: '4px 8px',
          borderRadius: '4px',
          fontSize: '12px',
          fontWeight: 'bold',
          border: '1px solid rgba(255,255,255,0.2)'
        }}
      >
        {cameraName} ({qualityLabel})
      </div>
    </>
  );
};

export default CellInfo;