pub mod commands;

use chrono::Utc;
use rusqlite::{params, Connection, Result as SqliteResult};
use serde::{Deserialize, Serialize};
use std::path::Path;
use std::sync::Mutex;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PlateRecord {
    pub id: i64,
    pub camera_id: i32,
    pub plate_number: String,
    pub confidence: f32,
    pub timestamp: String, // ISO 8601 format
    pub full_image_path: String,
    pub plate_crop_path: String,
    pub vehicle_type: Option<String>,
    pub direction: Option<String>, // in/out
    pub notes: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PlateRecordInsert {
    pub camera_id: i32,
    pub plate_number: String,
    pub confidence: f32,
    pub timestamp: String,
    pub full_image_path: String,
    pub plate_crop_path: String,
    pub vehicle_type: Option<String>,
    pub direction: Option<String>,
}

pub struct PlateDatabase {
    conn: Mutex<Connection>,
}

impl PlateDatabase {
    pub fn new(db_path: &Path) -> SqliteResult<Self> {
        let conn = Connection::open(db_path)?;

        // Create tables if they don't exist
        conn.execute(
            "CREATE TABLE IF NOT EXISTS plate_records (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                camera_id INTEGER NOT NULL,
                plate_number TEXT NOT NULL,
                confidence REAL NOT NULL,
                timestamp TEXT NOT NULL,
                full_image_path TEXT NOT NULL,
                plate_crop_path TEXT NOT NULL,
                vehicle_type TEXT,
                direction TEXT,
                notes TEXT,
                created_at TEXT DEFAULT CURRENT_TIMESTAMP
            )",
            [],
        )?;

        // Create indexes for better query performance
        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_plate_number ON plate_records(plate_number)",
            [],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_timestamp ON plate_records(timestamp DESC)",
            [],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_camera_id ON plate_records(camera_id)",
            [],
        )?;

        Ok(Self {
            conn: Mutex::new(conn),
        })
    }

    pub fn insert_record(&self, record: PlateRecordInsert) -> SqliteResult<i64> {
        let conn = self.conn.lock().unwrap();

        conn.execute(
            "INSERT INTO plate_records 
             (camera_id, plate_number, confidence, timestamp, full_image_path, 
              plate_crop_path, vehicle_type, direction)
             VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8)",
            params![
                record.camera_id,
                record.plate_number,
                record.confidence,
                record.timestamp,
                record.full_image_path,
                record.plate_crop_path,
                record.vehicle_type,
                record.direction,
            ],
        )?;

        Ok(conn.last_insert_rowid())
    }

    pub fn get_records(
        &self,
        limit: i32,
        offset: i32,
        plate_filter: Option<&str>,
        camera_filter: Option<i32>,
        date_from: Option<&str>,
        date_to: Option<&str>,
    ) -> SqliteResult<Vec<PlateRecord>> {
        let conn = self.conn.lock().unwrap();

        let mut query = String::from(
            "SELECT id, camera_id, plate_number, confidence, timestamp, 
             full_image_path, plate_crop_path, vehicle_type, direction, notes
             FROM plate_records WHERE 1=1",
        );

        let mut params: Vec<Box<dyn rusqlite::ToSql>> = Vec::new();

        if let Some(plate) = plate_filter {
            query.push_str(" AND plate_number LIKE ?");
            params.push(Box::new(format!("%{}%", plate)));
        }

        if let Some(camera_id) = camera_filter {
            query.push_str(" AND camera_id = ?");
            params.push(Box::new(camera_id));
        }

        if let Some(from) = date_from {
            query.push_str(" AND timestamp >= ?");
            params.push(Box::new(from.to_string()));
        }

        if let Some(to) = date_to {
            query.push_str(" AND timestamp <= ?");
            params.push(Box::new(to.to_string()));
        }

        query.push_str(" ORDER BY timestamp DESC LIMIT ? OFFSET ?");
        params.push(Box::new(limit));
        params.push(Box::new(offset));

        let param_refs: Vec<&dyn rusqlite::ToSql> = params.iter().map(|p| p.as_ref()).collect();

        let mut stmt = conn.prepare(&query)?;
        let records = stmt.query_map(param_refs.as_slice(), |row| {
            Ok(PlateRecord {
                id: row.get(0)?,
                camera_id: row.get(1)?,
                plate_number: row.get(2)?,
                confidence: row.get(3)?,
                timestamp: row.get(4)?,
                full_image_path: row.get(5)?,
                plate_crop_path: row.get(6)?,
                vehicle_type: row.get(7)?,
                direction: row.get(8)?,
                notes: row.get(9)?,
            })
        })?;

        records.collect()
    }

    pub fn get_record_by_id(&self, id: i64) -> SqliteResult<Option<PlateRecord>> {
        let conn = self.conn.lock().unwrap();

        let mut stmt = conn.prepare(
            "SELECT id, camera_id, plate_number, confidence, timestamp,
             full_image_path, plate_crop_path, vehicle_type, direction, notes
             FROM plate_records WHERE id = ?",
        )?;

        let mut records = stmt.query_map(params![id], |row| {
            Ok(PlateRecord {
                id: row.get(0)?,
                camera_id: row.get(1)?,
                plate_number: row.get(2)?,
                confidence: row.get(3)?,
                timestamp: row.get(4)?,
                full_image_path: row.get(5)?,
                plate_crop_path: row.get(6)?,
                vehicle_type: row.get(7)?,
                direction: row.get(8)?,
                notes: row.get(9)?,
            })
        })?;

        records.next().transpose()
    }

    pub fn update_notes(&self, id: i64, notes: &str) -> SqliteResult<()> {
        let conn = self.conn.lock().unwrap();
        conn.execute(
            "UPDATE plate_records SET notes = ? WHERE id = ?",
            params![notes, id],
        )?;
        Ok(())
    }

    pub fn delete_record(&self, id: i64) -> SqliteResult<()> {
        let conn = self.conn.lock().unwrap();
        conn.execute("DELETE FROM plate_records WHERE id = ?", params![id])?;
        Ok(())
    }

    pub fn get_statistics(&self) -> SqliteResult<PlateStatistics> {
        let conn = self.conn.lock().unwrap();

        let total_records: i64 =
            conn.query_row("SELECT COUNT(*) FROM plate_records", [], |row| row.get(0))?;

        let today_records: i64 = conn.query_row(
            "SELECT COUNT(*) FROM plate_records 
             WHERE DATE(timestamp) = DATE('now')",
            [],
            |row| row.get(0),
        )?;

        let unique_plates: i64 = conn.query_row(
            "SELECT COUNT(DISTINCT plate_number) FROM plate_records",
            [],
            |row| row.get(0),
        )?;

        Ok(PlateStatistics {
            total_records,
            today_records,
            unique_plates,
        })
    }

    pub fn search_plate_history(&self, plate_number: &str) -> SqliteResult<Vec<PlateRecord>> {
        self.get_records(100, 0, Some(plate_number), None, None, None)
    }
}

// Object Counter database structures

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ObjectCounterEvent {
    pub id: i64,
    pub camera_id: Option<i32>,
    pub module_id: String,
    pub processed_at: String,
    pub object_type: String,
    pub count: i32,
    pub confidence_avg: f32,
    pub dwell_avg_ms: Option<f32>,
    pub width_avg: Option<f32>,
    pub height_avg: Option<f32>,
    pub zone: Option<String>,
    pub snapshot_path: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ObjectCounterAggregate {
    pub bucket_start: String,
    pub total_count: i32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ObjectCounterTopCamera {
    pub camera_id: Option<i32>,
    pub total_count: i32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ObjectCounterLine {
    pub id: i64,
    pub camera_id: i32,
    pub name: String,
    pub start_x: f32,
    pub start_y: f32,
    pub end_x: f32,
    pub end_y: f32,
    pub direction: String,
    pub object_type: Option<String>,
    pub enabled: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ObjectCounterLineInput {
    pub id: Option<i64>,
    pub camera_id: i32,
    pub name: String,
    pub start_x: f32,
    pub start_y: f32,
    pub end_x: f32,
    pub end_y: f32,
    pub direction: String,
    pub object_type: Option<String>,
    pub enabled: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ObjectCounterZone {
    pub id: i64,
    pub camera_id: i32,
    pub name: String,
    pub polygon: String,
    pub zone_type: String,
    pub object_type: Option<String>,
    pub dwell_threshold_ms: Option<i64>,
    pub enabled: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ObjectCounterZoneInput {
    pub id: Option<i64>,
    pub camera_id: i32,
    pub name: String,
    pub polygon: String,
    pub zone_type: String,
    pub object_type: Option<String>,
    pub dwell_threshold_ms: Option<i64>,
    pub enabled: bool,
}

pub struct ObjectCounterDatabase {
    conn: Mutex<Connection>,
}

impl ObjectCounterDatabase {
    pub fn new(db_path: &Path) -> SqliteResult<Self> {
        let conn = Connection::open(db_path)?;

        conn.execute(
            "CREATE TABLE IF NOT EXISTS object_counter_events (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                camera_id INTEGER,
                module_id TEXT NOT NULL,
                processed_at TEXT NOT NULL,
                object_type TEXT NOT NULL,
                count INTEGER NOT NULL,
                confidence_avg REAL NOT NULL,
                dwell_avg_ms REAL,
                width_avg REAL,
                height_avg REAL,
                zone TEXT,
                snapshot_path TEXT
            )",
            [],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_object_counter_processed_at ON object_counter_events(processed_at DESC)",
            [],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_object_counter_camera ON object_counter_events(camera_id)",
            [],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_object_counter_type ON object_counter_events(object_type)",
            [],
        )?;

        conn.execute(
            "CREATE TABLE IF NOT EXISTS object_counter_lines (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                camera_id INTEGER NOT NULL,
                name TEXT NOT NULL,
                start_x REAL NOT NULL,
                start_y REAL NOT NULL,
                end_x REAL NOT NULL,
                end_y REAL NOT NULL,
                direction TEXT NOT NULL,
                object_type TEXT,
                enabled INTEGER NOT NULL DEFAULT 1,
                created_at TEXT DEFAULT CURRENT_TIMESTAMP,
                updated_at TEXT DEFAULT CURRENT_TIMESTAMP
            )",
            [],
        )?;

        conn.execute(
            "CREATE TABLE IF NOT EXISTS object_counter_zones (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                camera_id INTEGER NOT NULL,
                name TEXT NOT NULL,
                polygon TEXT NOT NULL,
                zone_type TEXT NOT NULL,
                object_type TEXT,
                dwell_threshold_ms INTEGER,
                enabled INTEGER NOT NULL DEFAULT 1,
                created_at TEXT DEFAULT CURRENT_TIMESTAMP,
                updated_at TEXT DEFAULT CURRENT_TIMESTAMP
            )",
            [],
        )?;

        Ok(Self {
            conn: Mutex::new(conn),
        })
    }

    pub fn insert_event(
        &self,
        camera_id: Option<i32>,
        module_id: &str,
        processed_at: &str,
        object_type: &str,
        count: i32,
        confidence_avg: f32,
        dwell_avg_ms: Option<f32>,
        width_avg: Option<f32>,
        height_avg: Option<f32>,
        zone: Option<&str>,
        snapshot_path: Option<&str>,
    ) -> SqliteResult<i64> {
        let conn = self.conn.lock().unwrap();

        conn.execute(
            "INSERT INTO object_counter_events (
                camera_id, module_id, processed_at, object_type, count,
                confidence_avg, dwell_avg_ms, width_avg, height_avg, zone, snapshot_path
            ) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)",
            params![
                camera_id,
                module_id,
                processed_at,
                object_type,
                count,
                confidence_avg,
                dwell_avg_ms,
                width_avg,
                height_avg,
                zone,
                snapshot_path,
            ],
        )?;

        Ok(conn.last_insert_rowid())
    }

    pub fn list_events(
        &self,
        limit: i32,
        offset: i32,
        camera_filter: Option<i32>,
        object_types: &[String],
        from: Option<&str>,
        to: Option<&str>,
    ) -> SqliteResult<Vec<ObjectCounterEvent>> {
        let conn = self.conn.lock().unwrap();
        let mut query = String::from(
                "SELECT id, camera_id, module_id, processed_at, object_type, count,
                    confidence_avg, dwell_avg_ms, width_avg, height_avg, zone, snapshot_path
             FROM object_counter_events WHERE 1=1",
        );
        let mut params: Vec<Box<dyn rusqlite::ToSql>> = Vec::new();

        if let Some(camera) = camera_filter {
            query.push_str(" AND camera_id = ?");
            params.push(Box::new(camera));
        }

        if !object_types.is_empty() {
            let placeholders: Vec<String> = object_types
                .iter()
                .map(|_| "?".to_string())
                .collect();
            query.push_str(&format!(" AND object_type IN ({})", placeholders.join(",")));
            for ty in object_types {
                params.push(Box::new(ty.clone()));
            }
        }

        if let Some(from_ts) = from {
            query.push_str(" AND processed_at >= ?");
            params.push(Box::new(from_ts.to_string()));
        }

        if let Some(to_ts) = to {
            query.push_str(" AND processed_at <= ?");
            params.push(Box::new(to_ts.to_string()));
        }

        query.push_str(" ORDER BY processed_at DESC LIMIT ? OFFSET ?");
        params.push(Box::new(limit));
        params.push(Box::new(offset));

        let param_refs: Vec<&dyn rusqlite::ToSql> = params.iter().map(|p| p.as_ref()).collect();

        let mut stmt = conn.prepare(&query)?;
        let rows = stmt.query_map(param_refs.as_slice(), |row| {
            Ok(ObjectCounterEvent {
                id: row.get(0)?,
                camera_id: row.get(1).ok(),
                module_id: row.get(2)?,
                processed_at: row.get(3)?,
                object_type: row.get(4)?,
                count: row.get(5)?,
                confidence_avg: row.get(6)?,
                dwell_avg_ms: row.get(7).ok(),
                width_avg: row.get(8).ok(),
                height_avg: row.get(9).ok(),
                zone: row.get(10).ok(),
                snapshot_path: row.get(11).ok(),
            })
        })?;

        rows.collect()
    }

    pub fn list_aggregates(
        &self,
        bucket_minutes: i64,
        buckets: i64,
        camera_filter: Option<i32>,
        object_types: &[String],
    ) -> SqliteResult<Vec<ObjectCounterAggregate>> {
        let conn = self.conn.lock().unwrap();
        let end_time = Utc::now();
        let start_time = end_time - chrono::Duration::minutes(bucket_minutes * buckets);
        let seconds = bucket_minutes * 60;

        let mut query = String::from(
            "SELECT datetime((strftime('%s', processed_at) / ?) * ?, 'unixepoch') as bucket,
                    SUM(count) as total
             FROM object_counter_events WHERE processed_at >= ?",
        );

        let mut params: Vec<Box<dyn rusqlite::ToSql>> = vec![
            Box::new(seconds),
            Box::new(seconds),
            Box::new(start_time.to_rfc3339())
        ];

        if let Some(camera) = camera_filter {
            query.push_str(" AND camera_id = ?");
            params.push(Box::new(camera));
        }

        if !object_types.is_empty() {
            let placeholders: Vec<String> = object_types
                .iter()
                .map(|_| "?".to_string())
                .collect();
            query.push_str(&format!(" AND object_type IN ({})", placeholders.join(",")));
            for ty in object_types {
                params.push(Box::new(ty.clone()));
            }
        }

        query.push_str(" GROUP BY bucket ORDER BY bucket ASC LIMIT ?");
        params.push(Box::new(buckets));

        let param_refs: Vec<&dyn rusqlite::ToSql> = params.iter().map(|p| p.as_ref()).collect();
        let mut stmt = conn.prepare(&query)?;
        let rows = stmt.query_map(param_refs.as_slice(), |row| {
            Ok(ObjectCounterAggregate {
                bucket_start: row.get(0)?,
                total_count: row.get::<_, Option<i64>>(1)?.unwrap_or(0) as i32,
            })
        })?;

        rows.collect()
    }

    pub fn list_top_cameras(
        &self,
        limit: i32,
        minutes: i64,
        object_types: &[String],
    ) -> SqliteResult<Vec<ObjectCounterTopCamera>> {
        let conn = self.conn.lock().unwrap();
        let since = (Utc::now() - chrono::Duration::minutes(minutes)).to_rfc3339();

        let mut query = String::from(
            "SELECT camera_id, SUM(count) as total
             FROM object_counter_events
             WHERE processed_at >= ?",
        );

        let mut params: Vec<Box<dyn rusqlite::ToSql>> = vec![Box::new(since)];

        if !object_types.is_empty() {
            let placeholders: Vec<String> = object_types
                .iter()
                .map(|_| "?".to_string())
                .collect();
            query.push_str(&format!(" AND object_type IN ({})", placeholders.join(",")));
            for ty in object_types {
                params.push(Box::new(ty.clone()));
            }
        }

        query.push_str(" GROUP BY camera_id ORDER BY total DESC LIMIT ?");
        params.push(Box::new(limit));

        let param_refs: Vec<&dyn rusqlite::ToSql> = params.iter().map(|p| p.as_ref()).collect();
        let mut stmt = conn.prepare(&query)?;
        let rows = stmt.query_map(param_refs.as_slice(), |row| {
            Ok(ObjectCounterTopCamera {
                camera_id: row.get(0).ok(),
                total_count: row.get::<_, Option<i64>>(1)?.unwrap_or(0) as i32,
            })
        })?;

        rows.collect()
    }

    fn fetch_line(&self, conn: &Connection, id: i64) -> SqliteResult<ObjectCounterLine> {
        conn.query_row(
            "SELECT id, camera_id, name, start_x, start_y, end_x, end_y, direction, object_type, enabled
             FROM object_counter_lines WHERE id = ?",
            [id],
            |row| {
                Ok(ObjectCounterLine {
                    id: row.get(0)?,
                    camera_id: row.get(1)?,
                    name: row.get(2)?,
                    start_x: row.get(3)?,
                    start_y: row.get(4)?,
                    end_x: row.get(5)?,
                    end_y: row.get(6)?,
                    direction: row.get(7)?,
                    object_type: row.get(8).ok(),
                    enabled: row.get::<_, i64>(9)? != 0,
                })
            },
        )
    }

    pub fn list_lines(&self, camera_id: Option<i32>) -> SqliteResult<Vec<ObjectCounterLine>> {
        let conn = self.conn.lock().unwrap();
        let mut query = String::from(
            "SELECT id, camera_id, name, start_x, start_y, end_x, end_y, direction, object_type, enabled
             FROM object_counter_lines",
        );
        let mut params: Vec<Box<dyn rusqlite::ToSql>> = Vec::new();

        if let Some(id) = camera_id {
            query.push_str(" WHERE camera_id = ?");
            params.push(Box::new(id));
        }

        query.push_str(" ORDER BY camera_id, id");
        let param_refs: Vec<&dyn rusqlite::ToSql> = params.iter().map(|p| p.as_ref()).collect();

        let mut stmt = conn.prepare(&query)?;
        let rows = stmt.query_map(param_refs.as_slice(), |row| {
            Ok(ObjectCounterLine {
                id: row.get(0)?,
                camera_id: row.get(1)?,
                name: row.get(2)?,
                start_x: row.get(3)?,
                start_y: row.get(4)?,
                end_x: row.get(5)?,
                end_y: row.get(6)?,
                direction: row.get(7)?,
                object_type: row.get(8).ok(),
                enabled: row.get::<_, i64>(9)? != 0,
            })
        })?;

        rows.collect()
    }

    pub fn upsert_line(&self, payload: ObjectCounterLineInput) -> SqliteResult<ObjectCounterLine> {
        let conn = self.conn.lock().unwrap();
        if let Some(id) = payload.id {
            conn.execute(
                "UPDATE object_counter_lines SET
                    camera_id = ?,
                    name = ?,
                    start_x = ?,
                    start_y = ?,
                    end_x = ?,
                    end_y = ?,
                    direction = ?,
                    object_type = ?,
                    enabled = ?,
                    updated_at = CURRENT_TIMESTAMP
                 WHERE id = ?",
                params![
                    payload.camera_id,
                    payload.name,
                    payload.start_x,
                    payload.start_y,
                    payload.end_x,
                    payload.end_y,
                    payload.direction,
                    payload.object_type,
                    payload.enabled as i32,
                    id,
                ],
            )?;
            return self.fetch_line(&conn, id);
        }

        conn.execute(
            "INSERT INTO object_counter_lines (
                camera_id, name, start_x, start_y, end_x, end_y, direction, object_type, enabled
            ) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)",
            params![
                payload.camera_id,
                payload.name,
                payload.start_x,
                payload.start_y,
                payload.end_x,
                payload.end_y,
                payload.direction,
                payload.object_type,
                payload.enabled as i32,
            ],
        )?;

        let id = conn.last_insert_rowid();
        self.fetch_line(&conn, id)
    }

    pub fn delete_line(&self, id: i64) -> SqliteResult<()> {
        let conn = self.conn.lock().unwrap();
        conn.execute("DELETE FROM object_counter_lines WHERE id = ?", [id])?;
        Ok(())
    }

    fn fetch_zone(&self, conn: &Connection, id: i64) -> SqliteResult<ObjectCounterZone> {
        conn.query_row(
            "SELECT id, camera_id, name, polygon, zone_type, object_type, dwell_threshold_ms, enabled
             FROM object_counter_zones WHERE id = ?",
            [id],
            |row| {
                Ok(ObjectCounterZone {
                    id: row.get(0)?,
                    camera_id: row.get(1)?,
                    name: row.get(2)?,
                    polygon: row.get(3)?,
                    zone_type: row.get(4)?,
                    object_type: row.get(5).ok(),
                    dwell_threshold_ms: row.get(6).ok(),
                    enabled: row.get::<_, i64>(7)? != 0,
                })
            },
        )
    }

    pub fn list_zones(&self, camera_id: Option<i32>) -> SqliteResult<Vec<ObjectCounterZone>> {
        let conn = self.conn.lock().unwrap();
        let mut query = String::from(
            "SELECT id, camera_id, name, polygon, zone_type, object_type, dwell_threshold_ms, enabled
             FROM object_counter_zones",
        );
        let mut params: Vec<Box<dyn rusqlite::ToSql>> = Vec::new();

        if let Some(id) = camera_id {
            query.push_str(" WHERE camera_id = ?");
            params.push(Box::new(id));
        }

        query.push_str(" ORDER BY camera_id, id");
        let param_refs: Vec<&dyn rusqlite::ToSql> = params.iter().map(|p| p.as_ref()).collect();

        let mut stmt = conn.prepare(&query)?;
        let rows = stmt.query_map(param_refs.as_slice(), |row| {
            Ok(ObjectCounterZone {
                id: row.get(0)?,
                camera_id: row.get(1)?,
                name: row.get(2)?,
                polygon: row.get(3)?,
                zone_type: row.get(4)?,
                object_type: row.get(5).ok(),
                dwell_threshold_ms: row.get(6).ok(),
                enabled: row.get::<_, i64>(7)? != 0,
            })
        })?;

        rows.collect()
    }

    pub fn upsert_zone(&self, payload: ObjectCounterZoneInput) -> SqliteResult<ObjectCounterZone> {
        let conn = self.conn.lock().unwrap();
        if let Some(id) = payload.id {
            conn.execute(
                "UPDATE object_counter_zones SET
                    camera_id = ?,
                    name = ?,
                    polygon = ?,
                    zone_type = ?,
                    object_type = ?,
                    dwell_threshold_ms = ?,
                    enabled = ?,
                    updated_at = CURRENT_TIMESTAMP
                 WHERE id = ?",
                params![
                    payload.camera_id,
                    payload.name,
                    payload.polygon,
                    payload.zone_type,
                    payload.object_type,
                    payload.dwell_threshold_ms,
                    payload.enabled as i32,
                    id,
                ],
            )?;
            return self.fetch_zone(&conn, id);
        }

        conn.execute(
            "INSERT INTO object_counter_zones (
                camera_id, name, polygon, zone_type, object_type, dwell_threshold_ms, enabled
            ) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)",
            params![
                payload.camera_id,
                payload.name,
                payload.polygon,
                payload.zone_type,
                payload.object_type,
                payload.dwell_threshold_ms,
                payload.enabled as i32,
            ],
        )?;

        let id = conn.last_insert_rowid();
        self.fetch_zone(&conn, id)
    }

    pub fn delete_zone(&self, id: i64) -> SqliteResult<()> {
        let conn = self.conn.lock().unwrap();
        conn.execute("DELETE FROM object_counter_zones WHERE id = ?", [id])?;
        Ok(())
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PlateStatistics {
    pub total_records: i64,
    pub today_records: i64,
    pub unique_plates: i64,
}
