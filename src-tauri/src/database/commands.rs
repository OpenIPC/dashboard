// Tauri commands for plate database management

use crate::database::{PlateDatabase, PlateRecord, PlateStatistics};
use std::path::PathBuf;
use std::sync::Arc;
use tauri::State;

pub struct DatabaseState {
    pub db: Arc<PlateDatabase>,
}

#[tauri::command]
pub async fn read_plate_image(path: String) -> Result<Vec<u8>, String> {
    // Remove Windows long path prefix if present
    let clean_path = path.trim_start_matches(r"\\?\");

    std::fs::read(clean_path).map_err(|e| format!("Failed to read image {}: {}", clean_path, e))
}

#[tauri::command]
pub async fn get_plate_records(
    db: State<'_, DatabaseState>,
    limit: i32,
    offset: i32,
    plate_filter: Option<String>,
    camera_filter: Option<i32>,
    date_from: Option<String>,
    date_to: Option<String>,
) -> Result<Vec<PlateRecord>, String> {
    let plate_ref = plate_filter.as_deref();
    let date_from_ref = date_from.as_deref();
    let date_to_ref = date_to.as_deref();

    db.db
        .get_records(
            limit,
            offset,
            plate_ref,
            camera_filter,
            date_from_ref,
            date_to_ref,
        )
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn get_plate_record_by_id(
    db: State<'_, DatabaseState>,
    id: i64,
) -> Result<Option<PlateRecord>, String> {
    db.db
        .get_record_by_id(id)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn update_plate_notes(
    db: State<'_, DatabaseState>,
    id: i64,
    notes: String,
) -> Result<(), String> {
    db.db
        .update_notes(id, &notes)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn delete_plate_record(db: State<'_, DatabaseState>, id: i64) -> Result<(), String> {
    db.db
        .delete_record(id)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn get_plate_statistics(db: State<'_, DatabaseState>) -> Result<PlateStatistics, String> {
    db.db
        .get_statistics()
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn search_plate_history(
    db: State<'_, DatabaseState>,
    plate_number: String,
) -> Result<Vec<PlateRecord>, String> {
    db.db
        .search_plate_history(&plate_number)
        .map_err(|e| format!("Database error: {}", e))
}

pub fn init_plate_database(app_data_dir: &PathBuf) -> Result<Arc<PlateDatabase>, String> {
    let db_path = app_data_dir.join("plate_records.db");
    let db = PlateDatabase::new(&db_path)
        .map_err(|e| format!("Failed to initialize database: {}", e))?;
    Ok(Arc::new(db))
}
