// Tauri commands for plate database management

use crate::analytics::AnalyticsState;
use crate::database::{
    ObjectCounterAggregate,
    ObjectCounterDatabase,
    ObjectCounterEvent,
    ObjectCounterLine,
    ObjectCounterLineInput,
    ObjectCounterTopCamera,
    ObjectCounterZone,
    ObjectCounterZoneInput,
    PlateDatabase,
    PlateRecord,
    PlateStatistics,
};
use std::path::PathBuf;
use std::sync::Arc;
use tauri::State;

pub struct DatabaseState {
    pub db: Arc<PlateDatabase>,
}

pub struct ObjectCounterDatabaseState {
    pub db: Arc<ObjectCounterDatabase>,
}

#[tauri::command]
pub async fn read_plate_image(path: String) -> Result<Vec<u8>, String> {
    // Remove Windows long path prefix if present
    let clean_path = path.trim_start_matches(r"\\?\");

    std::fs::read(clean_path).map_err(|e| format!("Failed to read image {}: {}", clean_path, e))
}

#[tauri::command]
pub async fn read_object_image(
    state: State<'_, AnalyticsState>,
    path: String,
) -> Result<Vec<u8>, String> {
    // Remove Windows long path prefix if present
    let clean_path = path.trim_start_matches(r"\\?\");
    let path_buf = PathBuf::from(clean_path);

    // If path is absolute, read it directly
    if path_buf.is_absolute() {
        return std::fs::read(&path_buf)
            .map_err(|e| format!("Failed to read image {}: {}", clean_path, e));
    }

    // If relative, try to resolve against object-counter module
    if let Some(snapshots_dir) = state.get_module_snapshots_dir("object-counter") {
        let resolved_path = snapshots_dir.join(clean_path);
        return std::fs::read(&resolved_path)
            .map_err(|e| format!("Failed to read image {:?}: {}", resolved_path, e));
    }

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

pub fn init_object_counter_database(app_data_dir: &PathBuf) -> Result<Arc<ObjectCounterDatabase>, String> {
    let db_path = app_data_dir.join("object_counter.db");
    let db = ObjectCounterDatabase::new(&db_path)
        .map_err(|e| format!("Failed to initialize database: {}", e))?;
    Ok(Arc::new(db))
}

#[tauri::command]
pub async fn get_object_counter_events(
    db: State<'_, ObjectCounterDatabaseState>,
    limit: i32,
    offset: i32,
    camera_filter: Option<i32>,
    object_types: Option<Vec<String>>,
    date_from: Option<String>,
    date_to: Option<String>,
) -> Result<Vec<ObjectCounterEvent>, String> {
    let types = object_types.unwrap_or_default();
    db.db
        .list_events(
            limit,
            offset,
            camera_filter,
            &types,
            date_from.as_deref(),
            date_to.as_deref(),
        )
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn get_object_counter_aggregates(
    db: State<'_, ObjectCounterDatabaseState>,
    bucket_minutes: i64,
    buckets: i64,
    camera_filter: Option<i32>,
    object_types: Option<Vec<String>>,
) -> Result<Vec<ObjectCounterAggregate>, String> {
    let types = object_types.unwrap_or_default();
    db.db
        .list_aggregates(bucket_minutes, buckets, camera_filter, &types)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn get_object_counter_top_cameras(
    db: State<'_, ObjectCounterDatabaseState>,
    limit: i32,
    minutes: i64,
    object_types: Option<Vec<String>>,
) -> Result<Vec<ObjectCounterTopCamera>, String> {
    let types = object_types.unwrap_or_default();
    db.db
        .list_top_cameras(limit, minutes, &types)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn get_object_counter_lines(
    db: State<'_, ObjectCounterDatabaseState>,
    camera_id: Option<i32>,
) -> Result<Vec<ObjectCounterLine>, String> {
    db.db
        .list_lines(camera_id)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn upsert_object_counter_line(
    db: State<'_, ObjectCounterDatabaseState>,
    payload: ObjectCounterLineInput,
) -> Result<ObjectCounterLine, String> {
    db.db
        .upsert_line(payload)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn delete_object_counter_line(
    db: State<'_, ObjectCounterDatabaseState>,
    id: i64,
) -> Result<(), String> {
    db.db
        .delete_line(id)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn get_object_counter_zones(
    db: State<'_, ObjectCounterDatabaseState>,
    camera_id: Option<i32>,
) -> Result<Vec<ObjectCounterZone>, String> {
    db.db
        .list_zones(camera_id)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn upsert_object_counter_zone(
    db: State<'_, ObjectCounterDatabaseState>,
    payload: ObjectCounterZoneInput,
) -> Result<ObjectCounterZone, String> {
    db.db
        .upsert_zone(payload)
        .map_err(|e| format!("Database error: {}", e))
}

#[tauri::command]
pub async fn delete_object_counter_zone(
    db: State<'_, ObjectCounterDatabaseState>,
    id: i64,
) -> Result<(), String> {
    db.db
        .delete_zone(id)
        .map_err(|e| format!("Database error: {}", e))
}
