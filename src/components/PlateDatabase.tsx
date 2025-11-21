import React, { useState, useEffect, useCallback } from 'react';
import { useNavigate } from 'react-router-dom';
import { invoke } from '@tauri-apps/api/core';
import { convertFileSrc } from '@tauri-apps/api/core';
import { IconButton } from '@mui/material';
import ArrowBack from '@mui/icons-material/ArrowBack';
import { useLocalization } from '../hooks/useLocalization';
import './PlateDatabase.css';

interface PlateRecord {
  id: number;
  camera_id: number;
  plate_number: string;
  confidence: number;
  timestamp: string;
  full_image_path: string;
  plate_crop_path: string;
  vehicle_type?: string;
  direction?: string;
  notes?: string;
}

interface PlateStatistics {
  total_records: number;
  today_count: number;
  unique_plates: number;
}

export default function PlateDatabase() {
  const navigate = useNavigate();
  const { t } = useLocalization();
  const [records, setRecords] = useState<PlateRecord[]>([]);
  const [statistics, setStatistics] = useState<PlateStatistics | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [selectedRecord, setSelectedRecord] = useState<PlateRecord | null>(null);
  
  // Filters
  const [plateFilter, setPlateFilter] = useState('');
  const [cameraFilter, setCameraFilter] = useState('');
  const [dateFrom, setDateFrom] = useState('');
  const [dateTo, setDateTo] = useState('');
  const [page, setPage] = useState(0);
  const [limit] = useState(50);

  // Load statistics
  const loadStatistics = useCallback(async () => {
    try {
      const stats: PlateStatistics = await invoke('get_plate_statistics');
      setStatistics(stats);
    } catch (err) {
      console.error('Failed to load statistics:', err);
    }
  }, []);

  // Load records
  const loadRecords = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const offset = page * limit;
      const cameraId = cameraFilter ? parseInt(cameraFilter) : null;
      
      const data: PlateRecord[] = await invoke('get_plate_records', {
        limit,
        offset,
        plateFilter: plateFilter || null,
        cameraFilter: cameraId,
        dateFrom: dateFrom || null,
        dateTo: dateTo || null,
      });
      
      setRecords(data);
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    } finally {
      setLoading(false);
    }
  }, [page, limit, plateFilter, cameraFilter, dateFrom, dateTo]);

  // Initial load
  useEffect(() => {
    loadStatistics();
    loadRecords();
  }, [loadStatistics, loadRecords]);

  // Search by plate number
  const handleSearch = async () => {
    if (!plateFilter.trim()) {
      loadRecords();
      return;
    }

    setLoading(true);
    setError(null);
    try {
      const data: PlateRecord[] = await invoke('search_plate_history', {
        plateNumber: plateFilter.trim(),
      });
      setRecords(data);
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    } finally {
      setLoading(false);
    }
  };

  // Update notes
  const handleUpdateNotes = async (id: number, notes: string) => {
    try {
      await invoke('update_plate_notes', { id, notes });
      // Reload records to show updated notes
      await loadRecords();
      if (selectedRecord?.id === id) {
        setSelectedRecord({ ...selectedRecord, notes });
      }
    } catch (err) {
      alert(`Failed to update notes: ${err}`);
    }
  };

  // Delete record
  const handleDelete = async (id: number) => {
    if (!confirm(t('plate_delete_confirm'))) {
      return;
    }

    try {
      await invoke('delete_plate_record', { id });
      await loadRecords();
      await loadStatistics();
      if (selectedRecord?.id === id) {
        setSelectedRecord(null);
      }
    } catch (err) {
      alert(`Failed to delete record: ${err}`);
    }
  };

  // Clear filters
  const handleClearFilters = () => {
    setPlateFilter('');
    setCameraFilter('');
    setDateFrom('');
    setDateTo('');
    setPage(0);
  };

  // Format timestamp
  const formatTimestamp = (timestamp: string) => {
    return new Date(timestamp).toLocaleString();
  };

  // Image loading with cache
  const [imageUrls, setImageUrls] = useState<Map<string, string>>(new Map());

  const loadImage = async (path: string) => {
    if (!path || imageUrls.has(path)) return;

    try {
      const bytes = await invoke<number[]>('read_plate_image', { path });
      const blob = new Blob([new Uint8Array(bytes)], { type: 'image/jpeg' });
      const url = URL.createObjectURL(blob);
      
      setImageUrls(prev => new Map(prev).set(path, url));
    } catch (err) {
      console.error('Failed to load image:', path, err);
      setImageUrls(prev => new Map(prev).set(path, ''));
    }
  };

  // Load images for visible records
  useEffect(() => {
    records.forEach(record => {
      loadImage(record.plate_crop_path);
    });
  }, [records]);

  // Load detail images
  useEffect(() => {
    if (selectedRecord) {
      loadImage(selectedRecord.full_image_path);
      loadImage(selectedRecord.plate_crop_path);
    }
  }, [selectedRecord]);

  return (
    <div className="plate-database">
      <div className="plate-database-header">
        <div className="header-left">
          <IconButton
            onClick={() => navigate('/')}
            sx={{ 
              color: '#fff',
              backgroundColor: '#23272f',
              border: '1px solid #404040',
              boxShadow: 1,
              width: 36,
              height: 36,
              borderRadius: '10px',
              transition: 'background 0.2s',
              '&:hover': {
                backgroundColor: '#31343c',
                borderColor: '#4caf50',
                color: '#4caf50'
              }
            }}
            size="medium"
            title={t('backToMain')}
          >
            <ArrowBack fontSize="medium" />
          </IconButton>
          <h1>{t('plate_database')}</h1>
        </div>
        {statistics && (
          <div className="statistics">
            <div className="stat-item">
              <span className="stat-label">{t('plate_total_records')}:</span>
              <span className="stat-value">{statistics.total_records}</span>
            </div>
            <div className="stat-item">
              <span className="stat-label">{t('plate_today')}:</span>
              <span className="stat-value">{statistics.today_count}</span>
            </div>
            <div className="stat-item">
              <span className="stat-label">{t('plate_unique_plates')}:</span>
              <span className="stat-value">{statistics.unique_plates}</span>
            </div>
          </div>
        )}
      </div>

      <div className="filters">
        <div className="filter-group">
          <input
            type="text"
            placeholder={t('plate_number')}
            value={plateFilter}
            onChange={(e) => setPlateFilter(e.target.value)}
            onKeyPress={(e) => e.key === 'Enter' && handleSearch()}
          />
          <input
            type="text"
            placeholder={t('plate_camera')}
            value={cameraFilter}
            onChange={(e) => setCameraFilter(e.target.value)}
          />
          <input
            type="date"
            placeholder={t('plate_date_from')}
            value={dateFrom}
            onChange={(e) => setDateFrom(e.target.value)}
          />
          <input
            type="date"
            placeholder={t('plate_date_to')}
            value={dateTo}
            onChange={(e) => setDateTo(e.target.value)}
          />
        </div>
        <div className="filter-actions">
          <button onClick={handleSearch}>{t('plate_search')}</button>
          <button onClick={() => { handleClearFilters(); loadRecords(); }}>{t('plate_clear')}</button>
          <button onClick={loadRecords}>{t('plate_refresh')}</button>
        </div>
      </div>

      {error && <div className="error-message">{error}</div>}

      <div className="plate-database-content">
        <div className="records-list">
          {loading ? (
            <div className="loading">{t('plate_loading')}</div>
          ) : records.length === 0 ? (
            <div className="no-records">{t('plate_no_records')}</div>
          ) : (
            <table className="records-table">
              <thead>
                <tr>
                  <th>{t('plate_timestamp')}</th>
                  <th>{t('plate_number')}</th>
                  <th>{t('plate_camera')}</th>
                  <th>{t('plate_confidence')}</th>
                  <th>{t('plate_preview')}</th>
                  <th>{t('plate_actions')}</th>
                </tr>
              </thead>
              <tbody>
                {records.map((record) => (
                  <tr
                    key={record.id}
                    className={selectedRecord?.id === record.id ? 'selected' : ''}
                    onClick={() => setSelectedRecord(record)}
                  >
                    <td>{formatTimestamp(record.timestamp)}</td>
                    <td className="plate-number">{record.plate_number}</td>
                    <td>Camera {record.camera_id}</td>
                    <td>{(record.confidence * 100).toFixed(1)}%</td>
                    <td>
                      {imageUrls.get(record.plate_crop_path) && (
                        <img
                          src={imageUrls.get(record.plate_crop_path)}
                          alt="Plate crop"
                          className="plate-thumbnail"
                        />
                      )}
                    </td>
                    <td>
                      <button
                        className="btn-small"
                        onClick={(e) => {
                          e.stopPropagation();
                          setSelectedRecord(record);
                        }}
                      >
                        {t('plate_view')}
                      </button>
                      <button
                        className="btn-small btn-danger"
                        onClick={(e) => {
                          e.stopPropagation();
                          handleDelete(record.id);
                        }}
                      >
                        {t('plate_delete')}
                      </button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          )}

          <div className="pagination">
            <button disabled={page === 0} onClick={() => setPage(page - 1)}>
              {t('plate_previous')}
            </button>
            <span>{t('plate_page')} {page + 1}</span>
            <button
              disabled={records.length < limit}
              onClick={() => setPage(page + 1)}
            >
              {t('plate_next')}
            </button>
          </div>
        </div>

        {selectedRecord && (
          <div className="record-details">
            <div className="details-header">
              <h2>{t('plate_record_details')}</h2>
              <button
                className="close-button"
                onClick={() => setSelectedRecord(null)}
              >
                ×
              </button>
            </div>

            <div className="details-content">
              <div className="image-section">
                <div className="image-container">
                  <h3>{t('plate_full_vehicle')}</h3>
                  {imageUrls.get(selectedRecord.full_image_path) ? (
                    <img
                      src={imageUrls.get(selectedRecord.full_image_path)}
                      alt="Full vehicle"
                      className="full-image"
                    />
                  ) : (
                    <div className="loading-placeholder">Loading...</div>
                  )}
                </div>
                <div className="image-container">
                  <h3>{t('plate_crop')}</h3>
                  {imageUrls.get(selectedRecord.plate_crop_path) ? (
                    <img
                      src={imageUrls.get(selectedRecord.plate_crop_path)}
                      alt="Plate crop"
                      className="crop-image"
                    />
                  ) : (
                    <div className="loading-placeholder">Loading...</div>
                  )}
                </div>
              </div>

              <div className="info-section">
                <div className="info-row">
                  <label>{t('plate_number')}:</label>
                  <span className="plate-number-large">{selectedRecord.plate_number}</span>
                </div>
                <div className="info-row">
                  <label>{t('plate_timestamp')}:</label>
                  <span>{formatTimestamp(selectedRecord.timestamp)}</span>
                </div>
                <div className="info-row">
                  <label>{t('plate_camera')}:</label>
                  <span>{t('plate_camera')} {selectedRecord.camera_id}</span>
                </div>
                <div className="info-row">
                  <label>{t('plate_confidence')}:</label>
                  <span>{(selectedRecord.confidence * 100).toFixed(2)}%</span>
                </div>
                {selectedRecord.vehicle_type && (
                  <div className="info-row">
                    <label>{t('plate_vehicle_type')}:</label>
                    <span>{selectedRecord.vehicle_type}</span>
                  </div>
                )}
                {selectedRecord.direction && (
                  <div className="info-row">
                    <label>{t('plate_direction')}:</label>
                    <span>{selectedRecord.direction}</span>
                  </div>
                )}
              </div>

              <div className="notes-section">
                <label>{t('plate_notes')}:</label>
                <textarea
                  value={selectedRecord.notes || ''}
                  onChange={(e) => {
                    setSelectedRecord({ ...selectedRecord, notes: e.target.value });
                  }}
                  placeholder={t('plate_notes_placeholder')}
                  rows={4}
                />
                <button
                  onClick={() => handleUpdateNotes(selectedRecord.id, selectedRecord.notes || '')}
                >
                  {t('plate_save_notes')}
                </button>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
