pub mod commands;

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

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PlateStatistics {
    pub total_records: i64,
    pub today_records: i64,
    pub unique_plates: i64,
}
