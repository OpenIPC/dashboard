pub mod anpr_config;
mod license_plate;
mod tracker;
mod yolo;

use std::cmp::Ordering;
use std::collections::HashMap;
use std::fs;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::Arc;
use std::time::{Duration, Instant};
use std::str::FromStr;

use base64::engine::general_purpose::STANDARD as BASE64_STANDARD;
use base64::Engine;
use chrono::{DateTime, TimeZone, Utc};
use image::{
    self,
    codecs::jpeg::JpegEncoder,
    imageops::{self, FilterType},
    ColorType, DynamicImage, GenericImageView, RgbaImage,
};
use parking_lot::RwLock;
use regex::Regex;
use serde::{Deserialize, Serialize};
use serde_json::Value;
use sha2::Digest;
use tauri::{AppHandle, Emitter, Manager, State};
use thiserror::Error;
use tracker::FaceTracker;
use yolo::{
    YoloDetector, YoloDetectorOptions, COCO_CLASS_LABELS, COCO_COLOR_PALETTE, FACE_CLASS_LABELS,
    FACE_COLOR_PALETTE,
};

// ANPR config module - functions reserved for future use
#[allow(unused_imports)]
pub use anpr_config::{
    get_anpr_config, get_config_path as get_anpr_config_path,
    initialize_config as initialize_anpr_config, update_config as update_anpr_config, AnprConfig,
};

use crate::{
    crypto::{encrypt_snapshot_bytes, encrypt_snapshot_metadata},
    database::commands::ObjectCounterDatabaseState,
    settings_root_dir,
};

use flate2::read::GzDecoder;
use tar::Archive as TarArchive;
use zip::ZipArchive;

const MODULES_DIR: &str = "modules";
const MANIFEST_FILE: &str = "module.json";
const MAX_SNAPSHOTS_PER_FRAME: usize = 6;
const MIN_SNAPSHOT_EDGE: u32 = 40;
const MIN_HD_WIDTH: u32 = 640;
const MIN_HD_HEIGHT: u32 = 360;
const JPEG_QUALITY: u8 = 90;
pub(super) const FACE_DETECTOR_MODEL_FILE: &str = "yolov11n-face.onnx";
pub(super) const OBJECT_COUNTER_MODEL_FILE: &str = "yolo11s.onnx";
pub(super) const LICENSE_PLATE_DETECTOR_MODEL_FILE: &str = "anpr_yolov8.onnx";
pub(super) const LICENSE_PLATE_OCR_MODEL_FILE: &str = "anpr_crnn.onnx";
const SNAPSHOT_TARGET_ASPECT: f32 = 4.0 / 3.0;
const SNAPSHOT_EXPANSION_FACTOR_MIN: f32 = 1.05;
const SNAPSHOT_EXPANSION_FACTOR_MAX: f32 = 1.9;
const SNAPSHOT_VERTICAL_BIAS_RATIO: f32 = 0.2;
const SNAPSHOT_MIN_CROP_WIDTH: f32 = 160.0;
const SNAPSHOT_MIN_CROP_HEIGHT: f32 = SNAPSHOT_MIN_CROP_WIDTH / SNAPSHOT_TARGET_ASPECT;
const SNAPSHOT_MAX_FRAME_COVERAGE_RATIO: f32 = 0.7;
const SNAPSHOT_MAX_EXPANSION_MULTIPLIER: f32 = 1.9;
const SNAPSHOT_ASPECT_MIN: f32 = 0.4;
const SNAPSHOT_ASPECT_MAX: f32 = 2.2;
const SNAPSHOT_EDGE_MARGIN_HORIZONTAL_RATIO: f32 = 0.01;
const SNAPSHOT_EDGE_MARGIN_VERTICAL_RATIO: f32 = 0.0;
const SNAPSHOT_MIN_TEXTURE_VARIANCE: f32 = 120.0;
const LICENSE_PLATE_SNAPSHOT_MIN_EDGE: u32 = 24;
const LICENSE_PLATE_SNAPSHOT_HORIZONTAL_MARGIN_RATIO: f32 = 0.35;
const LICENSE_PLATE_SNAPSHOT_TOP_MARGIN_RATIO: f32 = 0.6;
const LICENSE_PLATE_SNAPSHOT_BOTTOM_MARGIN_RATIO: f32 = 0.25;
const LICENSE_PLATE_SNAPSHOT_MIN_TEXTURE_VARIANCE: f32 = 45.0;
const SNAPSHOT_LIST_DEFAULT_LIMIT: usize = 40;
const SNAPSHOT_LIST_MAX_LIMIT: usize = 500;

const ANALYTICS_PROGRESS_EVENT: &str = "analytics-module-progress";
const ANALYTICS_DETECTION_EVENT: &str = "analytics-detection";

#[allow(unused_macros)]
macro_rules! onnx_runtime_version {
    () => {
        "1.23.0"
    };
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(super) enum ExecutionProviderPreference {
    Auto,
    DirectML,
    Cpu,
}

impl ExecutionProviderPreference {
    fn from_str(value: &str) -> Self {
        match value.trim().to_ascii_lowercase().as_str() {
            "dml" | "directml" => ExecutionProviderPreference::DirectML,
            "cpu" => ExecutionProviderPreference::Cpu,
            _ => ExecutionProviderPreference::Auto,
        }
    }

    fn label(self) -> &'static str {
        match self {
            ExecutionProviderPreference::Auto => "auto",
            ExecutionProviderPreference::DirectML => "dml",
            ExecutionProviderPreference::Cpu => "cpu",
        }
    }
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq)]
#[serde(rename_all = "kebab-case")]
pub enum FaceSnapshotMode {
    Disabled,
    Standard,
    Anonymized,
    Encrypted,
}

impl Default for FaceSnapshotMode {
    fn default() -> Self {
        FaceSnapshotMode::Standard
    }
}

impl FromStr for FaceSnapshotMode {
    type Err = String;

    fn from_str(value: &str) -> std::result::Result<Self, Self::Err> {
        match value.trim().to_ascii_lowercase().as_str() {
            "disabled" => Ok(FaceSnapshotMode::Disabled),
            "standard" | "default" | "enabled" => Ok(FaceSnapshotMode::Standard),
            "anonymized" | "anonymised" => Ok(FaceSnapshotMode::Anonymized),
            "encrypted" => Ok(FaceSnapshotMode::Encrypted),
            other => Err(format!("unsupported faceSnapshotsMode '{other}'")),
        }
    }
}

fn current_execution_provider_preference() -> ExecutionProviderPreference {
    let settings_path = settings_root_dir().join("settings.json");
    if let Ok(content) = fs::read_to_string(settings_path) {
        if let Ok(value) = serde_json::from_str::<serde_json::Value>(&content) {
            if let Some(raw) = value
                .get("analytics_provider")
                .and_then(|entry| entry.as_str())
            {
                return ExecutionProviderPreference::from_str(raw);
            }
        }
    }

    ExecutionProviderPreference::Auto
}

#[cfg(any(target_os = "linux", target_os = "macos"))]
pub(super) fn onnx_runtime_version_str() -> &'static str {
    onnx_runtime_version!()
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq)]
#[serde(rename_all = "kebab-case")]
pub enum AnalyticsModuleId {
    #[serde(rename = "face-detector")]
    FaceDetector,
    #[serde(rename = "license-plate-detector")]
    LicensePlateDetector,
    #[serde(rename = "object-counter")]
    ObjectCounter,
}

impl AnalyticsModuleId {
    pub fn as_str(&self) -> &'static str {
        match self {
            AnalyticsModuleId::FaceDetector => "face-detector",
            AnalyticsModuleId::LicensePlateDetector => "license-plate-detector",
            AnalyticsModuleId::ObjectCounter => "object-counter",
        }
    }
}

#[derive(Debug, Serialize, Deserialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct ModuleStatus {
    pub id: String,
    pub name: String,
    pub version: String,
    pub description: String,
    pub enabled: bool,
    pub state: String,
    pub progress: Option<f32>,
    pub message: Option<String>,
    pub last_activated_at: Option<String>,
    pub last_error_at: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub config: Option<ModuleConfig>,
}

#[derive(Debug, Serialize, Clone)]
#[serde(rename_all = "camelCase")]
struct ModuleProgressPayload {
    module_id: String,
    progress: f32,
    stage: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone, Default)]
#[serde(rename_all = "camelCase")]
pub struct ModuleConfig {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub snapshots_dir: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub face_snapshots_mode: Option<FaceSnapshotMode>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub face_snapshot_key_configured: Option<bool>,
}

impl ModuleConfig {
    fn is_empty(&self) -> bool {
        self.snapshots_dir.is_none()
            && self.face_snapshots_mode.is_none()
            && self.face_snapshot_key_configured.is_none()
    }
}

#[derive(Debug, Serialize, Deserialize, Clone)]
#[serde(default, rename_all = "camelCase")]
struct ModuleManifest {
    id: String,
    version: String,
    enabled: bool,
    last_activated_at: Option<String>,
    last_error: Option<String>,
    last_error_at: Option<String>,
    snapshots_dir: Option<String>,
    #[serde(default)]
    face_snapshots_mode: Option<FaceSnapshotMode>,
    #[serde(default)]
    face_snapshot_key: Option<String>,
}

impl Default for ModuleManifest {
    fn default() -> Self {
        Self {
            id: String::new(),
            version: String::new(),
            enabled: false,
            last_activated_at: None,
            last_error: None,
            last_error_at: None,
            snapshots_dir: None,
            face_snapshots_mode: None,
            face_snapshot_key: None,
        }
    }
}

pub trait AnalyticsEngine: Send + Sync {
    fn process(
        &self,
        frame: &DynamicImage,
        options: &Value,
    ) -> std::result::Result<Vec<DetectionBox>, String>;
}

pub struct AnalyticsState {
    inner: Arc<RwLock<AnalyticsInner>>,
}

struct AnalyticsInner {
    app_handle: AppHandle,
    root_dir: PathBuf,
    modules: HashMap<String, ModuleEntry>,
    face_trackers: HashMap<String, FaceTracker>,
}

#[derive(Clone)]
struct ModuleEntry {
    descriptor: ModuleDescriptor,
    manifest: ModuleManifest,
    state: ModuleState,
    engine: Option<Arc<dyn AnalyticsEngine>>,
    module_dir: PathBuf,
    min_inference_interval: Duration,
    last_processed_at: Option<Instant>,
    last_processed_timestamp: Option<DateTime<Utc>>,
    last_detections: Vec<DetectionBox>,
    inference_in_progress: bool,
}

#[derive(Clone)]
enum ModuleState {
    Disabled,
    Loading {
        progress: f32,
        message: Option<String>,
    },
    Ready,
    Error {
        message: String,
    },
}

type ModuleBuilder = fn(&Path) -> std::result::Result<Arc<dyn AnalyticsEngine>, String>;

#[derive(Debug, Clone, Copy)]
struct ModuleDescriptor {
    id: AnalyticsModuleId,
    name: &'static str,
    version: &'static str,
    description: &'static str,
    resources: &'static [ModuleResourceSpec],
    builder: ModuleBuilder,
    min_inference_interval: Duration,
}

#[allow(dead_code)]
#[derive(Debug, Clone, Copy)]
enum ModuleResourceSpec {
    File(ModuleDownloadSpec),
    Archive(ModuleArchiveSpec),
}

#[derive(Debug, Clone, Copy)]
struct ModuleDownloadSpec {
    url: &'static str,
    file_name: &'static str,
    sha256: Option<&'static str>,
}

#[derive(Debug, Clone, Copy)]
struct ModuleArchiveSpec {
    url: &'static str,
    file_name: &'static str,
    sha256: Option<&'static str>,
    archive_type: ArchiveType,
    strip_prefix: Option<&'static str>,
    target_dir: &'static str,
    required_files: &'static [&'static str],
}

#[allow(dead_code)]
#[derive(Debug, Clone, Copy)]
enum ArchiveType {
    Zip,
    TarGz,
}

#[derive(Debug, Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct DetectionBox {
    pub id: String,
    pub label: String,
    pub confidence: f32,
    pub bounds: BoundingBox,
    pub color: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub track_id: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub previous_bounds: Option<BoundingBox>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub zone: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub dwell_ms: Option<u64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub first_seen_at: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub last_seen_at: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub event_type: Option<DetectionEventType>,
}

#[derive(Debug, Serialize, Clone)]
#[serde(rename_all = "kebab-case")]
pub enum DetectionEventType {
    Entered,
    Updated,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct BoundingBox {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

// Face snapshot roadmap (iteration 2):
// 1. Support disable/anonymize/encrypt modes for snapshots.
// 2. Add MobileFaceNet embeddings and persist with metadata.
// 3. Wire up frontend controls for selecting mode and key management.
const FACE_DETECTION_MIN_CONFIDENCE: f32 = 0.35;
const FACE_DETECTION_IOU_SUPPRESSION: f32 = 0.32;
const FACE_DETECTION_CENTER_DISTANCE_RATIO: f32 = 0.18;
const FACE_DETECTION_MIN_SIZE_SIMILARITY: f32 = 0.4;
const FACE_DETECTION_COVERAGE_THRESHOLD: f32 = 0.6;
const FACE_DETECTION_MAX_FRAME_COVERAGE_RATIO: f32 = 0.45;
const FACE_DETECTION_MAX_DETECTIONS: usize = 6;

pub(super) fn bounding_box_iou(a: &BoundingBox, b: &BoundingBox) -> f32 {
    let ax1 = a.x;
    let ay1 = a.y;
    let ax2 = a.x + a.width;
    let ay2 = a.y + a.height;

    let bx1 = b.x;
    let by1 = b.y;
    let bx2 = b.x + b.width;
    let by2 = b.y + b.height;

    let inter_left = ax1.max(bx1);
    let inter_top = ay1.max(by1);
    let inter_right = ax2.min(bx2);
    let inter_bottom = ay2.min(by2);

    let inter_width = (inter_right - inter_left).max(0.0);
    let inter_height = (inter_bottom - inter_top).max(0.0);
    if inter_width <= 0.0 || inter_height <= 0.0 {
        return 0.0;
    }

    let inter_area = inter_width * inter_height;
    let area_a = (ax2 - ax1).max(0.0) * (ay2 - ay1).max(0.0);
    let area_b = (bx2 - bx1).max(0.0) * (by2 - by1).max(0.0);
    let union = (area_a + area_b - inter_area).max(f32::EPSILON);

    inter_area / union
}

fn bounding_box_intersection_area(a: &BoundingBox, b: &BoundingBox) -> f32 {
    let ax1 = a.x;
    let ay1 = a.y;
    let ax2 = a.x + a.width;
    let ay2 = a.y + a.height;

    let bx1 = b.x;
    let by1 = b.y;
    let bx2 = b.x + b.width;
    let by2 = b.y + b.height;

    let inter_left = ax1.max(bx1);
    let inter_top = ay1.max(by1);
    let inter_right = ax2.min(bx2);
    let inter_bottom = ay2.min(by2);

    let inter_width = (inter_right - inter_left).max(0.0);
    let inter_height = (inter_bottom - inter_top).max(0.0);
    if inter_width <= 0.0 || inter_height <= 0.0 {
        return 0.0;
    }

    inter_width * inter_height
}

fn is_duplicate_face_detection(existing: &DetectionBox, candidate: &DetectionBox) -> bool {
    if existing.label != candidate.label {
        return false;
    }

    let iou = bounding_box_iou(&existing.bounds, &candidate.bounds);
    if iou >= FACE_DETECTION_IOU_SUPPRESSION {
        return true;
    }

    let existing_cx = existing.bounds.x + existing.bounds.width / 2.0;
    let existing_cy = existing.bounds.y + existing.bounds.height / 2.0;
    let candidate_cx = candidate.bounds.x + candidate.bounds.width / 2.0;
    let candidate_cy = candidate.bounds.y + candidate.bounds.height / 2.0;

    let dx = existing_cx - candidate_cx;
    let dy = existing_cy - candidate_cy;
    let distance = (dx * dx + dy * dy).sqrt();
    let max_span = existing
        .bounds
        .width
        .max(existing.bounds.height)
        .max(candidate.bounds.width)
        .max(candidate.bounds.height)
        .max(1.0);

    let normalized_distance = distance / max_span;
    if normalized_distance <= FACE_DETECTION_CENTER_DISTANCE_RATIO {
        let area_existing =
            (existing.bounds.width.max(0.0) * existing.bounds.height.max(0.0)).max(1.0);
        let area_candidate =
            (candidate.bounds.width.max(0.0) * candidate.bounds.height.max(0.0)).max(1.0);
        let size_similarity = area_existing.min(area_candidate) / area_existing.max(area_candidate);
        if size_similarity >= FACE_DETECTION_MIN_SIZE_SIMILARITY {
            return true;
        }
    }

    let intersection = bounding_box_intersection_area(&existing.bounds, &candidate.bounds);
    if intersection <= 0.0 {
        return false;
    }

    let area_existing = (existing.bounds.width.max(0.0) * existing.bounds.height.max(0.0)).max(1.0);
    let area_candidate =
        (candidate.bounds.width.max(0.0) * candidate.bounds.height.max(0.0)).max(1.0);
    let coverage_existing = intersection / area_existing;
    let coverage_candidate = intersection / area_candidate;

    coverage_existing >= FACE_DETECTION_COVERAGE_THRESHOLD
        || coverage_candidate >= FACE_DETECTION_COVERAGE_THRESHOLD
}

fn detection_weight(confidence: f32) -> f32 {
    confidence.max(0.001)
}

#[derive(Clone)]
struct FaceDetectionCluster {
    sum_weight: f32,
    sum_x1: f32,
    sum_y1: f32,
    sum_x2: f32,
    sum_y2: f32,
    best_detection: DetectionBox,
}

impl FaceDetectionCluster {
    fn new(detection: &DetectionBox) -> Self {
        let weight = detection_weight(detection.confidence);
        let x1 = detection.bounds.x;
        let y1 = detection.bounds.y;
        let x2 = detection.bounds.x + detection.bounds.width;
        let y2 = detection.bounds.y + detection.bounds.height;

        Self {
            sum_weight: weight,
            sum_x1: x1 * weight,
            sum_y1: y1 * weight,
            sum_x2: x2 * weight,
            sum_y2: y2 * weight,
            best_detection: detection.clone(),
        }
    }

    fn representative_detection(&self) -> DetectionBox {
        self.best_detection.clone()
    }

    fn update(&mut self, detection: &DetectionBox) {
        let weight = detection_weight(detection.confidence);
        let x1 = detection.bounds.x;
        let y1 = detection.bounds.y;
        let x2 = detection.bounds.x + detection.bounds.width;
        let y2 = detection.bounds.y + detection.bounds.height;

        self.sum_x1 += x1 * weight;
        self.sum_y1 += y1 * weight;
        self.sum_x2 += x2 * weight;
        self.sum_y2 += y2 * weight;
        self.sum_weight += weight;

        if detection.confidence > self.best_detection.confidence {
            self.best_detection = detection.clone();
        }
    }

    fn into_detection(self, _frame_width: u32, _frame_height: u32) -> DetectionBox {
        self.best_detection
    }
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct DetectionResponse {
    pub module_id: String,
    pub camera_id: Option<String>,
    pub detections: Vec<DetectionBox>,
    pub processed_at: String,
    pub frame_width: u32,
    pub frame_height: u32,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
struct SnapshotMetadata {
    module_id: String,
    camera_id: Option<String>,
    detection_id: String,
    image_file: String,
    captured_at: String,
    confidence: f32,
    bounds: BoundingBox,
    frame_width: u32,
    frame_height: u32,
}

#[derive(Debug, Serialize, Clone)]
#[serde(rename_all = "camelCase")]
pub struct SnapshotListItem {
    pub id: String,
    pub module_id: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub camera_id: Option<String>,
    pub detection_id: String,
    pub captured_at: String,
    pub confidence: f32,
    pub bounds: BoundingBox,
    pub frame_width: u32,
    pub frame_height: u32,
    pub image_file: String,
    pub metadata_file: String,
    pub folder_path: String,
    pub image_path: String,
    pub metadata_path: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub image_size: Option<u64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub metadata_size: Option<u64>,
    pub image_available: bool,
    pub encrypted: bool,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct SnapshotListResponse {
    pub total: usize,
    pub has_more: bool,
    pub items: Vec<SnapshotListItem>,
}

#[derive(Debug, Deserialize, Default)]
#[serde(rename_all = "camelCase")]
pub struct SnapshotListRequest {
    pub module_id: Option<String>,
    pub camera_id: Option<String>,
    pub limit: Option<usize>,
    pub offset: Option<usize>,
}

impl SnapshotListRequest {
    fn limit(&self) -> usize {
        let requested = self.limit.unwrap_or(SNAPSHOT_LIST_DEFAULT_LIMIT);
        requested.clamp(1, SNAPSHOT_LIST_MAX_LIMIT)
    }

    fn offset(&self) -> usize {
        self.offset.unwrap_or(0)
    }
}

#[derive(Error, Debug)]
pub enum AnalyticsError {
    #[error("module '{0}' not found")]
    ModuleNotFound(String),
    #[error("module '{0}' is not ready")]
    ModuleNotReady(String),
    #[error("module '{module_id}' processing failed: {message}")]
    ModuleProcessingFailed { module_id: String, message: String },
    #[error("module '{module_id}' initialization failed: {message}")]
    ModuleInitializationFailed { module_id: String, message: String },
    #[allow(dead_code)]
    #[error("module '{module_id}' resource preparation failed: {message}")]
    ResourcePreparationFailed { module_id: String, message: String },
    #[error("path resolution failed: {0}")]
    PathResolution(String),
    #[error("io error: {0}")]
    IoError(#[from] std::io::Error),
    #[error("serialization error: {0}")]
    SerdeError(#[from] serde_json::Error),
}

type Result<T> = std::result::Result<T, AnalyticsError>;

fn segments_intersect(
    p1: (f32, f32),
    p2: (f32, f32),
    q1: (f32, f32),
    q2: (f32, f32),
) -> bool {
    fn ccw(a: (f32, f32), b: (f32, f32), c: (f32, f32)) -> bool {
        (c.1 - a.1) * (b.0 - a.0) > (b.1 - a.1) * (c.0 - a.0)
    }
    ccw(p1, q1, q2) != ccw(p2, q1, q2) && ccw(p1, p2, q1) != ccw(p1, p2, q2)
}

fn point_in_polygon(point: (f32, f32), polygon: &[(f32, f32)]) -> bool {
    let mut inside = false;
    let mut j = polygon.len() - 1;
    for i in 0..polygon.len() {
        if (polygon[i].1 > point.1) != (polygon[j].1 > point.1)
            && (point.0
                < (polygon[j].0 - polygon[i].0) * (point.1 - polygon[i].1)
                    / (polygon[j].1 - polygon[i].1)
                    + polygon[i].0)
        {
            inside = !inside;
        }
        j = i;
    }
    inside
}

fn parse_polygon(polygon_str: &str) -> Vec<(f32, f32)> {
    if let Ok(points) = serde_json::from_str::<Vec<serde_json::Value>>(polygon_str) {
        points.iter().filter_map(|p| {
            if let (Some(x), Some(y)) = (p.get("x").and_then(|v| v.as_f64()), p.get("y").and_then(|v| v.as_f64())) {
                Some((x as f32, y as f32))
            } else {
                None
            }
        }).collect()
    } else {
        Vec::new()
    }
}

impl AnalyticsState {
    pub fn get_module_snapshots_dir(&self, module_id: &str) -> Option<PathBuf> {
        let inner = self.inner.read();
        inner.modules.get(module_id).map(|entry| resolve_snapshots_dir(entry))
    }

    fn deduplicate_face_detections(
        &self,
        mut detections: Vec<DetectionBox>,
        frame_width: u32,
        frame_height: u32,
    ) -> Vec<DetectionBox> {
        if detections.is_empty() || frame_width == 0 || frame_height == 0 {
            return detections;
        }

        detections.sort_by(|a, b| {
            b.confidence
                .partial_cmp(&a.confidence)
                .unwrap_or(std::cmp::Ordering::Equal)
        });

        let frame_area = (frame_width.max(1) as f32) * (frame_height.max(1) as f32);
        let mut clusters: Vec<FaceDetectionCluster> = Vec::new();

        for detection in detections.into_iter() {
            if detection.confidence < FACE_DETECTION_MIN_CONFIDENCE {
                continue;
            }

            if detection.bounds.width <= 0.0 || detection.bounds.height <= 0.0 {
                continue;
            }

            let area = detection.bounds.width * detection.bounds.height;
            let coverage_ratio = if frame_area > 0.0 {
                area / frame_area
            } else {
                0.0
            };

            if coverage_ratio > FACE_DETECTION_MAX_FRAME_COVERAGE_RATIO {
                continue;
            }

            let mut matched = false;
            for cluster in clusters.iter_mut() {
                let representative = cluster.representative_detection();
                if is_duplicate_face_detection(&representative, &detection) {
                    cluster.update(&detection);
                    matched = true;
                    break;
                }
            }

            if !matched && clusters.len() < FACE_DETECTION_MAX_DETECTIONS {
                clusters.push(FaceDetectionCluster::new(&detection));
            }
        }

        if clusters.is_empty() {
            return Vec::new();
        }

        let mut reduced: Vec<DetectionBox> = clusters
            .into_iter()
            .map(|cluster| cluster.into_detection(frame_width, frame_height))
            .collect();

        reduced.retain(|detection| {
            if should_skip_detection(detection, frame_width, frame_height) {
                println!(
                    "analytics face-detector: dropping degenerate detection id={} bounds=({:.1},{:.1},{:.1},{:.1})",
                    detection.id,
                    detection.bounds.x,
                    detection.bounds.y,
                    detection.bounds.width,
                    detection.bounds.height
                );
                false
            } else {
                true
            }
        });

        reduced.sort_by(|a, b| {
            b.confidence
                .partial_cmp(&a.confidence)
                .unwrap_or(std::cmp::Ordering::Equal)
        });

        if reduced.len() > FACE_DETECTION_MAX_DETECTIONS {
            reduced.truncate(FACE_DETECTION_MAX_DETECTIONS);
        }

        reduced
    }

    fn apply_face_tracking(
        &self,
        module_id: &str,
        detections: Vec<DetectionBox>,
        timestamp: DateTime<Utc>,
    ) -> Vec<DetectionBox> {
        let mut inner = self.inner.write();
        let tracker = inner
            .face_trackers
            .entry(module_id.to_string())
            .or_insert_with(FaceTracker::new);
        tracker.assign(detections, timestamp)
    }

    pub fn new(app_handle: &AppHandle) -> Result<Self> {
        let app_dir = app_handle
            .path()
            .app_data_dir()
            .map_err(|err| AnalyticsError::PathResolution(err.to_string()))?;
        let root_dir = app_dir.join(MODULES_DIR);
        fs::create_dir_all(&root_dir)?;

        let mut modules = HashMap::new();
        for descriptor in builtin_modules() {
            let module_dir = root_dir.join(descriptor.id.as_str());
            fs::create_dir_all(&module_dir)?;

            let manifest_path = module_dir.join(MANIFEST_FILE);
            let mut manifest = if manifest_path.exists() {
                let content = fs::read_to_string(&manifest_path)?;
                serde_json::from_str::<ModuleManifest>(&content).unwrap_or_default()
            } else {
                ModuleManifest::default()
            };

            manifest.id = descriptor.id.as_str().to_string();
            manifest.version = descriptor.version.to_string();

            let mut manifest_dirty = false;
            if descriptor.id == AnalyticsModuleId::FaceDetector {
                if let Some(ref snapshots_dir) = manifest.snapshots_dir {
                    let original = PathBuf::from(snapshots_dir);
                    let absolute = if original.is_absolute() {
                        original.clone()
                    } else {
                        module_dir.join(&original)
                    };
                    fs::create_dir_all(&absolute)?;
                    let canonical = absolute.canonicalize().unwrap_or(absolute);
                    if canonical.to_string_lossy() != snapshots_dir.as_str() {
                        manifest.snapshots_dir = Some(canonical.to_string_lossy().to_string());
                        manifest_dirty = true;
                    }
                } else {
                    let default_dir = default_snapshots_dir(&module_dir);
                    fs::create_dir_all(&default_dir)?;
                    let canonical = default_dir.canonicalize().unwrap_or(default_dir);
                    manifest.snapshots_dir = Some(canonical.to_string_lossy().to_string());
                    manifest_dirty = true;
                }

                if manifest.face_snapshots_mode.is_none() {
                    manifest.face_snapshots_mode = Some(FaceSnapshotMode::Standard);
                    manifest_dirty = true;
                }
            }

            let mut state = if let Some(ref message) = manifest.last_error {
                ModuleState::Error {
                    message: message.clone(),
                }
            } else if manifest.enabled {
                ModuleState::Ready
            } else {
                ModuleState::Disabled
            };

            let mut engine: Option<Arc<dyn AnalyticsEngine>> = None;
            if manifest.enabled && manifest.last_error.is_none() {
                match prepare_module_engine(&module_dir, descriptor) {
                    Ok(prepared_engine) => {
                        engine = Some(prepared_engine);
                        state = ModuleState::Ready;
                    }
                    Err(err) => {
                        let message = err.clone();
                        state = ModuleState::Error {
                            message: message.clone(),
                        };
                        manifest.last_error = Some(message.clone());
                        manifest.last_error_at = Some(Utc::now().to_rfc3339());
                        manifest_dirty = true;
                    }
                }
            }

            modules.insert(
                descriptor.id.as_str().to_string(),
                ModuleEntry {
                    descriptor,
                    manifest,
                    state,
                    engine,
                    module_dir: module_dir.clone(),
                    min_inference_interval: descriptor.min_inference_interval,
                    last_processed_at: None,
                    last_processed_timestamp: None,
                    last_detections: Vec::new(),
                    inference_in_progress: false,
                },
            );

            if manifest_dirty {
                if let Some(entry) = modules.get(descriptor.id.as_str()) {
                    let _ =
                        persist_manifest(&root_dir, entry.descriptor.id.as_str(), &entry.manifest);
                }
            }
        }

        Ok(Self {
            inner: Arc::new(RwLock::new(AnalyticsInner {
                app_handle: app_handle.clone(),
                root_dir,
                modules,
                face_trackers: HashMap::new(),
            })),
        })
    }

    fn emit_progress_event(&self, module_id: &str, progress: f32, stage: Option<String>) {
        let payload = ModuleProgressPayload {
            module_id: module_id.to_string(),
            progress,
            stage,
        };

        let app_handle = {
            let inner = self.inner.read();
            inner.app_handle.clone()
        };

        let _ = app_handle.emit(ANALYTICS_PROGRESS_EVENT, payload);
    }

    fn update_module_loading_progress(
        &self,
        module_id: &str,
        progress: f32,
        stage: Option<String>,
    ) {
        let clamped = progress.clamp(0.0, 1.0);
        let mut should_emit = false;
        {
            let mut inner = self.inner.write();
            if let Some(entry) = inner.modules.get_mut(module_id) {
                let stage_ref = stage.as_ref();
                let emit_needed = match &entry.state {
                    ModuleState::Loading {
                        progress: prev,
                        message,
                    } => {
                        (clamped - *prev).abs() >= 0.005
                            || message.as_ref().map(String::as_str) != stage_ref.map(|s| s.as_str())
                    }
                    _ => true,
                };

                if emit_needed {
                    entry.state = ModuleState::Loading {
                        progress: clamped,
                        message: stage.clone(),
                    };
                    entry.engine = None;
                    entry.inference_in_progress = false;
                    should_emit = true;
                }
            }
        }

        if should_emit {
            self.emit_progress_event(module_id, clamped, stage);
        }
    }

    fn prepare_module_engine_with_progress(
        &self,
        module_id: &str,
        module_dir: &Path,
        descriptor: ModuleDescriptor,
    ) -> std::result::Result<Arc<dyn AnalyticsEngine>, String> {
        self.ensure_module_resources_with_progress(module_id, module_dir, descriptor)?;
        (descriptor.builder)(module_dir)
    }

    fn ensure_module_resources_with_progress(
        &self,
        module_id: &str,
        module_dir: &Path,
        descriptor: ModuleDescriptor,
    ) -> std::result::Result<(), String> {
        let total = descriptor.resources.len().max(1);
        for (index, resource) in descriptor.resources.iter().enumerate() {
            let base = index as f32 / total as f32;
            let span = 1.0 / total as f32;
            let mut progress_adapter = |fraction: f32| {
                let overall = (base + fraction.clamp(0.0, 1.0) * span).min(0.999);
                self.update_module_loading_progress(module_id, overall, None);
            };

            ensure_module_resource_with_progress(
                module_dir,
                *resource,
                descriptor,
                &mut progress_adapter,
            )?;
            self.update_module_loading_progress(module_id, (base + span).min(1.0), None);
        }

        Ok(())
    }

    pub fn list_status(&self) -> Vec<ModuleStatus> {
        let inner = self.inner.read();
        inner.modules.values().map(module_status).collect()
    }

    pub fn list_snapshots(
        &self,
        module_filter: Option<&str>,
        camera_filter: Option<&str>,
        limit: usize,
        offset: usize,
    ) -> Result<SnapshotListResponse> {
        let module_filter = module_filter.and_then(|value| {
            let trimmed = value.trim();
            if trimmed.is_empty() {
                None
            } else {
                Some(trimmed.to_string())
            }
        });

        let camera_filter = camera_filter.and_then(|value| {
            let trimmed = value.trim();
            if trimmed.is_empty() {
                None
            } else {
                Some(trimmed.to_string())
            }
        });

        let module_contexts: Vec<(String, PathBuf)> = {
            let inner = self.inner.read();
            inner
                .modules
                .values()
                .map(|entry| {
                    (
                        entry.descriptor.id.as_str().to_string(),
                        resolve_snapshots_dir(entry),
                    )
                })
                .collect()
        };

        let mut records: Vec<SnapshotRecord> = Vec::new();
        for (module_id, dir) in module_contexts {
            if let Some(filter) = module_filter.as_deref() {
                if module_id != filter {
                    continue;
                }
            }

            collect_snapshots_from_dir(&dir, &module_id, camera_filter.as_deref(), &mut records);
        }

        records.sort_by(|a, b| b.sort_key.cmp(&a.sort_key));

        let total = records.len();
        let start = offset.min(total);
        let end = (start + limit).min(total);
        let items = records[start..end]
            .iter()
            .map(|record| record.item.clone())
            .collect();

        Ok(SnapshotListResponse {
            total,
            has_more: end < total,
            items,
        })
    }

    pub fn enable_module(&self, module_id: &str) -> Result<()> {
        let (module_dir, descriptor) = {
            let inner = self.inner.read();
            let entry = inner
                .modules
                .get(module_id)
                .ok_or_else(|| AnalyticsError::ModuleNotFound(module_id.to_string()))?;

            (entry.module_dir.clone(), entry.descriptor)
        };

        self.update_module_loading_progress(module_id, 0.0, None);

        let engine =
            match self.prepare_module_engine_with_progress(module_id, &module_dir, descriptor) {
                Ok(engine) => {
                    self.update_module_loading_progress(module_id, 1.0, None);
                    engine
                }
                Err(message) => {
                    self.set_module_error(module_id, message.clone())?;
                    return Err(AnalyticsError::ModuleInitializationFailed {
                        module_id: module_id.to_string(),
                        message,
                    });
                }
            };

        let mut inner = self.inner.write();
        let entry = inner
            .modules
            .get_mut(module_id)
            .ok_or_else(|| AnalyticsError::ModuleNotFound(module_id.to_string()))?;

        entry.state = ModuleState::Ready;
        entry.engine = Some(engine);
        entry.manifest.enabled = true;
        entry.manifest.last_activated_at = Some(Utc::now().to_rfc3339());
        entry.manifest.last_error = None;
        entry.manifest.last_error_at = None;
        entry.last_processed_at = None;
        entry.last_processed_timestamp = None;
        entry.last_detections.clear();
        entry.inference_in_progress = false;

        let module_key = entry.descriptor.id.as_str().to_string();
        let manifest = entry.manifest.clone();
        persist_manifest(&inner.root_dir, &module_key, &manifest)?;
        Ok(())
    }

    pub fn disable_module(&self, module_id: &str) -> Result<()> {
        let mut inner = self.inner.write();
        let (module_key, manifest) = {
            let entry = inner
                .modules
                .get_mut(module_id)
                .ok_or_else(|| AnalyticsError::ModuleNotFound(module_id.to_string()))?;

            entry.state = ModuleState::Disabled;
            entry.manifest.enabled = false;
            entry.engine = None;
            entry.inference_in_progress = false;

            (
                entry.descriptor.id.as_str().to_string(),
                entry.manifest.clone(),
            )
        };

        persist_manifest(&inner.root_dir, &module_key, &manifest)?;
        Ok(())
    }

    fn set_module_error(&self, module_id: &str, message: String) -> Result<()> {
        let mut inner = self.inner.write();
        let entry = inner
            .modules
            .get_mut(module_id)
            .ok_or_else(|| AnalyticsError::ModuleNotFound(module_id.to_string()))?;

        let occurred_at = Some(Utc::now().to_rfc3339());
        entry.state = ModuleState::Error {
            message: message.clone(),
        };
        entry.manifest.last_error = Some(message);
        entry.manifest.last_error_at = occurred_at;
        entry.engine = None;
        entry.inference_in_progress = false;

        let module_key = entry.descriptor.id.as_str().to_string();
        let manifest = entry.manifest.clone();
        persist_manifest(&inner.root_dir, &module_key, &manifest)?;
        Ok(())
    }

    fn clear_module_error(&self, module_id: &str) -> Result<()> {
        let (module_dir, descriptor, should_activate) = {
            let inner = self.inner.read();
            let entry = inner
                .modules
                .get(module_id)
                .ok_or_else(|| AnalyticsError::ModuleNotFound(module_id.to_string()))?;

            (
                entry.module_dir.clone(),
                entry.descriptor,
                entry.manifest.enabled,
            )
        };

        let engine = if should_activate {
            match prepare_module_engine(&module_dir, descriptor) {
                Ok(engine) => Some(engine),
                Err(message) => {
                    self.set_module_error(module_id, message.clone())?;
                    return Err(AnalyticsError::ModuleInitializationFailed {
                        module_id: module_id.to_string(),
                        message,
                    });
                }
            }
        } else {
            None
        };

        let mut inner = self.inner.write();
        let entry = inner
            .modules
            .get_mut(module_id)
            .ok_or_else(|| AnalyticsError::ModuleNotFound(module_id.to_string()))?;

        entry.manifest.last_error = None;
        entry.manifest.last_error_at = None;
        if entry.manifest.enabled {
            entry.state = ModuleState::Ready;
            entry.engine = engine;
            entry.inference_in_progress = false;
        } else {
            entry.state = ModuleState::Disabled;
            entry.engine = None;
            entry.inference_in_progress = false;
        }

        let module_key = entry.descriptor.id.as_str().to_string();
        let manifest = entry.manifest.clone();
        persist_manifest(&inner.root_dir, &module_key, &manifest)?;
        Ok(())
    }

    pub fn update_module_config(
        &self,
        module_id: &str,
        update: ModuleConfigUpdateRequest,
    ) -> Result<ModuleStatus> {
        let mut inner = self.inner.write();
        let entry = inner
            .modules
            .get_mut(module_id)
            .ok_or_else(|| AnalyticsError::ModuleNotFound(module_id.to_string()))?;

        let mut manifest_updated = false;

        if matches!(
            entry.descriptor.id,
            AnalyticsModuleId::FaceDetector | AnalyticsModuleId::LicensePlateDetector | AnalyticsModuleId::ObjectCounter
        ) {
            if let Some(target_dir) = &update.snapshots_dir {
                let resolved_path = target_dir
                    .as_ref()
                    .map(|value| value.trim())
                    .filter(|value| !value.is_empty())
                    .map(PathBuf::from)
                    .unwrap_or_else(|| default_snapshots_dir(&entry.module_dir));

                let absolute_path = if resolved_path.is_absolute() {
                    resolved_path
                } else {
                    entry.module_dir.join(resolved_path)
                };

                fs::create_dir_all(&absolute_path)?;

                let canonical = absolute_path
                    .canonicalize()
                    .unwrap_or_else(|_| absolute_path.clone());
                let canonical_str = canonical.to_string_lossy().to_string();

                if entry.manifest.snapshots_dir.as_deref() != Some(canonical_str.as_str()) {
                    entry.manifest.snapshots_dir = Some(canonical_str);
                    manifest_updated = true;
                }
            }
        }

        if entry.descriptor.id == AnalyticsModuleId::FaceDetector {
            if let Some(mode) = update.face_snapshots_mode {
                if entry.manifest.face_snapshots_mode != Some(mode) {
                    entry.manifest.face_snapshots_mode = Some(mode);
                    manifest_updated = true;
                }
            }

            if let Some(ref raw_key) = update.face_snapshot_key_hex {
                let normalized = normalize_face_snapshot_key(raw_key).map_err(|message| {
                    AnalyticsError::ModuleProcessingFailed {
                        module_id: module_id.to_string(),
                        message,
                    }
                })?;

                if entry
                    .manifest
                    .face_snapshot_key
                    .as_deref()
                    != Some(normalized.as_str())
                {
                    entry.manifest.face_snapshot_key = Some(normalized);
                    manifest_updated = true;
                }
            } else if update.reset_face_snapshot_key {
                if entry.manifest.face_snapshot_key.take().is_some() {
                    manifest_updated = true;
                }
            }
        }

        if !manifest_updated {
            return Ok(module_status(entry));
        }

        let module_key = entry.descriptor.id.as_str().to_string();
        let manifest = entry.manifest.clone();
        let status = module_status(entry);
        let root_dir = inner.root_dir.clone();
        drop(inner);

        persist_manifest(&root_dir, &module_key, &manifest)?;

        Ok(status)
    }

    pub fn process_frame(
        &self,
        module_id: &str,
        camera_id: Option<String>,
        frame_width: u32,
        frame_height: u32,
        frame_base64: &str,
        options: Option<Value>,
    ) -> Result<DetectionResponse> {
        let module_snapshot = {
            let inner = self.inner.read();
            inner
                .modules
                .get(module_id)
                .cloned()
                .ok_or_else(|| AnalyticsError::ModuleNotFound(module_id.to_string()))?
        };

        match &module_snapshot.state {
            ModuleState::Disabled => {
                return Err(AnalyticsError::ModuleNotReady(module_id.to_string()))
            }
            ModuleState::Error { message } => {
                return Err(AnalyticsError::ModuleNotReady(format!(
                    "{module_id} (error: {message})"
                )))
            }
            ModuleState::Loading { .. } => {
                return Err(AnalyticsError::ModuleNotReady(format!(
                    "{module_id} (loading resources)"
                )))
            }
            ModuleState::Ready => {}
        }

        if module_snapshot.manifest.last_error.is_some() {
            self.clear_module_error(module_id)?;
        }

        let now = Instant::now();
        let mut cached_response: Option<(Vec<DetectionBox>, Option<String>)> = None;
        let mut inference_started = false;
        let mut throttle_reason: Option<&'static str> = None;
        let mut throttle_elapsed: Option<Duration> = None;
        let mut state_error: Option<AnalyticsError> = None;

        let engine = {
            let mut inner = self.inner.write();
            match inner.modules.get_mut(module_id) {
                Some(entry) => {
                    match &entry.state {
                        ModuleState::Ready => {}
                        ModuleState::Disabled => {
                            state_error =
                                Some(AnalyticsError::ModuleNotReady(module_id.to_string()));
                        }
                        ModuleState::Error { message } => {
                            state_error = Some(AnalyticsError::ModuleNotReady(format!(
                                "{module_id} (error: {message})"
                            )));
                        }
                        ModuleState::Loading { .. } => {
                            state_error = Some(AnalyticsError::ModuleNotReady(format!(
                                "{module_id} (loading resources)"
                            )));
                        }
                    }

                    if state_error.is_some() {
                        None
                    } else if entry.inference_in_progress {
                        throttle_reason = Some("inflight");
                        cached_response = Some((
                            entry.last_detections.clone(),
                            entry
                                .last_processed_timestamp
                                .as_ref()
                                .map(|ts| ts.to_rfc3339()),
                        ));
                        entry.engine.clone()
                    } else {
                        let within_interval = entry
                            .last_processed_at
                            .map(|last| {
                                let elapsed = now.saturating_duration_since(last);
                                if elapsed < entry.min_inference_interval {
                                    throttle_reason = Some("interval");
                                    throttle_elapsed = Some(elapsed);
                                    true
                                } else {
                                    false
                                }
                            })
                            .unwrap_or(false);

                        if within_interval {
                            cached_response = Some((
                                entry.last_detections.clone(),
                                entry
                                    .last_processed_timestamp
                                    .as_ref()
                                    .map(|ts| ts.to_rfc3339()),
                            ));
                            entry.engine.clone()
                        } else {
                            entry.inference_in_progress = true;
                            inference_started = true;
                            entry.engine.clone()
                        }
                    }
                }
                None => {
                    state_error = Some(AnalyticsError::ModuleNotFound(module_id.to_string()));
                    None
                }
            }
        };

        if let Some(err) = state_error {
            if inference_started {
                let mut inner = self.inner.write();
                if let Some(entry) = inner.modules.get_mut(module_id) {
                    entry.inference_in_progress = false;
                }
            }
            return Err(err);
        }

        if let Some((detections, processed_at)) = cached_response {
            match throttle_reason {
                Some("inflight") => {
                    println!(
                        "analytics {module_id}: frame skipped (inference already in progress)"
                    );
                }
                Some("interval") => {
                    if let Some(elapsed) = throttle_elapsed {
                        println!(
                            "analytics {module_id}: frame throttled ({:.0} ms since last inference)",
                            elapsed.as_millis()
                        );
                    }
                }
                _ => {}
            }

            return Ok(DetectionResponse {
                module_id: module_id.to_string(),
                camera_id,
                detections,
                processed_at: processed_at.unwrap_or_else(|| Utc::now().to_rfc3339()),
                frame_width,
                frame_height,
            });
        }

        let engine = match engine {
            Some(engine) => engine,
            None => {
                if inference_started {
                    let mut inner = self.inner.write();
                    if let Some(entry) = inner.modules.get_mut(module_id) {
                        entry.inference_in_progress = false;
                    }
                }
                return Err(AnalyticsError::ModuleNotReady(format!(
                    "{module_id} (engine not initialized)"
                )));
            }
        };

        let base64_input = frame_base64.trim();
        let decoded = match BASE64_STANDARD.decode(base64_input.as_bytes()) {
            Ok(bytes) => bytes,
            Err(err) => {
                let message = format!("failed to decode frame: {err}");
                self.set_module_error(module_id, message.clone())?;
                return Err(AnalyticsError::ModuleProcessingFailed {
                    module_id: module_id.to_string(),
                    message,
                });
            }
        };

        let frame_image = match image::load_from_memory(&decoded) {
            Ok(img) => img,
            Err(err) => {
                let message = format!("invalid image data: {err}");
                self.set_module_error(module_id, message.clone())?;
                return Err(AnalyticsError::ModuleProcessingFailed {
                    module_id: module_id.to_string(),
                    message,
                });
            }
        };

        let options_value = options.unwrap_or(Value::Null);
        let detections = match engine.process(&frame_image, &options_value) {
            Ok(result) => result,
            Err(err) => {
                self.set_module_error(module_id, err.clone())?;
                return Err(AnalyticsError::ModuleProcessingFailed {
                    module_id: module_id.to_string(),
                    message: err,
                });
            }
        };

        println!(
            "analytics {module_id}: processed frame {}x{}, detections={}",
            frame_width,
            frame_height,
            detections.len()
        );

        let (actual_width, actual_height) = frame_image.dimensions();
        let effective_width = if frame_width == 0 {
            actual_width as f32
        } else {
            frame_width as f32
        };
        let effective_height = if frame_height == 0 {
            actual_height as f32
        } else {
            frame_height as f32
        };

        let normalized_detections: Vec<DetectionBox> = detections
            .into_iter()
            .map(|mut detection| {
                detection.bounds.x = detection.bounds.x.min(effective_width).max(0.0);
                detection.bounds.y = detection.bounds.y.min(effective_height).max(0.0);
                detection.bounds.width = detection.bounds.width.min(effective_width).max(0.0);
                detection.bounds.height = detection.bounds.height.min(effective_height).max(0.0);
                detection
            })
            .collect();

        let mut filtered_detections =
            if module_snapshot.descriptor.id == AnalyticsModuleId::FaceDetector {
                let deduped = self.deduplicate_face_detections(
                    normalized_detections,
                    actual_width,
                    actual_height,
                );
                println!(
                    "analytics {module_id}: face dedup reduced detections to {}",
                    deduped.len()
                );
                deduped
            } else {
                normalized_detections
            };

        let processed_timestamp = Utc::now();

        if module_snapshot.descriptor.id == AnalyticsModuleId::FaceDetector {
            filtered_detections = self.apply_face_tracking(
                module_id,
                filtered_detections,
                processed_timestamp,
            );
            let camera_ref = camera_id.as_deref();
            if let Err(err) = self.capture_face_snapshots(
                &module_snapshot,
                &frame_image,
                &filtered_detections,
                camera_ref,
            ) {
                println!(
                    "analytics {module_id}: failed to capture face snapshots: {}",
                    err
                );
            }
        } else if module_snapshot.descriptor.id == AnalyticsModuleId::ObjectCounter {
            filtered_detections = self.apply_face_tracking(
                module_id,
                filtered_detections,
                processed_timestamp,
            );

            let app_handle = {
                let inner = self.inner.read();
                inner.app_handle.clone()
            };
            filtered_detections = self.enrich_object_counter_detections(
                &app_handle,
                filtered_detections,
                effective_width as u32,
                effective_height as u32,
                camera_id.as_deref(),
            );
        } else if module_snapshot.descriptor.id == AnalyticsModuleId::LicensePlateDetector {
            let camera_ref = camera_id.as_deref();
            if let Err(err) = self.capture_license_plate_snapshots(
                &module_snapshot,
                &frame_image,
                &filtered_detections,
                camera_ref,
            ) {
                println!(
                    "analytics {module_id}: failed to capture license plate snapshots: {}",
                    err
                );
            }
        }

        let response = DetectionResponse {
            module_id: module_id.to_string(),
            camera_id: camera_id.clone(),
            detections: filtered_detections.clone(),
            processed_at: processed_timestamp.to_rfc3339(),
            frame_width: effective_width as u32,
            frame_height: effective_height as u32,
        };

        let app_handle = {
            let inner = self.inner.read();
            inner.app_handle.clone()
        };

        if module_snapshot.descriptor.id == AnalyticsModuleId::ObjectCounter {
            self.persist_object_counter_events(
                &app_handle,
                &response,
                &filtered_detections,
                &frame_image,
                &module_snapshot,
            );
        }

        // Emit detection event to frontend
        let _ = app_handle.emit(ANALYTICS_DETECTION_EVENT, &response);

        if inference_started {
            let mut inner = self.inner.write();
            if let Some(entry) = inner.modules.get_mut(module_id) {
                entry.last_detections = filtered_detections;
                entry.last_processed_at = Some(Instant::now());
                entry.last_processed_timestamp = Some(processed_timestamp);
                entry.inference_in_progress = false;
            }
        }

        Ok(response)
    }

    fn enrich_object_counter_detections(
        &self,
        app_handle: &AppHandle,
        detections: Vec<DetectionBox>,
        frame_width: u32,
        frame_height: u32,
        camera_id: Option<&str>,
    ) -> Vec<DetectionBox> {
        let Some(db_state) = app_handle.try_state::<ObjectCounterDatabaseState>() else {
            return detections;
        };

        let camera_numeric = parse_camera_id(camera_id);
        let camera_id_opt = if camera_numeric > 0 {
            Some(camera_numeric)
        } else {
            None
        };

        let zones = db_state.db.list_zones(camera_id_opt).unwrap_or_default();
        if zones.is_empty() {
            return detections;
        }

        detections.into_iter().map(|mut detection| {
            let center_x = detection.bounds.x + detection.bounds.width / 2.0;
            let center_y = detection.bounds.y + detection.bounds.height / 2.0;
            let center = (center_x, center_y);

            for zone in &zones {
                if !zone.enabled { continue; }
                let polygon_points = parse_polygon(&zone.polygon);
                let scaled_polygon: Vec<(f32, f32)> = polygon_points.iter().map(|p| {
                    (p.0 * frame_width as f32, p.1 * frame_height as f32)
                }).collect();

                if point_in_polygon(center, &scaled_polygon) {
                    detection.zone = Some(zone.name.clone());
                    break;
                }
            }
            detection
        }).collect()
    }

    fn persist_object_counter_events(
        &self,
        app_handle: &AppHandle,
        response: &DetectionResponse,
        detections: &[DetectionBox],
        frame: &DynamicImage,
        module: &ModuleEntry,
    ) {
        let Some(db_state) = app_handle.try_state::<ObjectCounterDatabaseState>() else {
            return;
        };

        if detections.is_empty() {
            return;
        }

        let camera_numeric = parse_camera_id(response.camera_id.as_deref());
        let camera_id_opt = if camera_numeric > 0 {
            Some(camera_numeric)
        } else {
            None
        };

        // Fetch lines and zones
        let lines = db_state.db.list_lines(camera_id_opt).unwrap_or_default();
        let zones = db_state.db.list_zones(camera_id_opt).unwrap_or_default();

        // Check for line crossings and zone entries
        for detection in detections {
            let center_x = detection.bounds.x + detection.bounds.width / 2.0;
            let center_y = detection.bounds.y + detection.bounds.height / 2.0;
            let center = (center_x, center_y);

            if let Some(prev_bounds) = &detection.previous_bounds {
                let prev_center_x = prev_bounds.x + prev_bounds.width / 2.0;
                let prev_center_y = prev_bounds.y + prev_bounds.height / 2.0;
                let prev_center = (prev_center_x, prev_center_y);
                
                // Skip if movement is too small (jitter)
                let dx = center.0 - prev_center.0;
                let dy = center.1 - prev_center.1;
                if dx * dx + dy * dy < 4.0 { // 2 pixels squared
                    continue;
                }

                let mut snapshot_path: Option<String> = None;

                // Line crossings
                for line in &lines {
                    if !line.enabled { continue; }
                    if let Some(obj_type) = &line.object_type {
                        if !obj_type.is_empty() && obj_type != &detection.label {
                            continue;
                        }
                    }

                    let line_start = (line.start_x as f32 * response.frame_width as f32, line.start_y as f32 * response.frame_height as f32);
                    let line_end = (line.end_x as f32 * response.frame_width as f32, line.end_y as f32 * response.frame_height as f32);

                    let intersect = segments_intersect(prev_center, center, line_start, line_end);
                    
                    if intersect {
                        println!("DEBUG: Line crossing detected! Line: {}, Object: {}", line.name, detection.label);
                        if snapshot_path.is_none() {
                             snapshot_path = self.save_object_snapshot(module, frame, detection, response.camera_id.as_deref());
                        }

                        // Check direction relative to line
                        // Line vector: L = B - A
                        // Movement vector: M = P2 - P1
                        // Cross product (2D) of L and M tells us if we are crossing from left to right or right to left relative to line
                        // But we need to know if we are crossing "Forward" (A->B right side) or "Backward"
                        // Let's use the cross product of Line vector and (P1 - A).
                        // CP1 = (B-A) x (P1-A)
                        // CP2 = (B-A) x (P2-A)
                        // If CP1 and CP2 have different signs, we crossed.
                        // If CP1 > 0 (Left) and CP2 < 0 (Right), we crossed Left->Right (Forward relative to normal pointing Right)
                        
                        let lx = line_end.0 - line_start.0;
                        let ly = line_end.1 - line_start.1;
                        
                        // Cross product (bx*ay - by*ax)
                        let cp1 = lx * (prev_center.1 - line_start.1) - ly * (prev_center.0 - line_start.0);
                        // let cp2 = lx * (center.1 - line_start.1) - ly * (center.0 - line_start.0);
                        
                        // If cp1 > 0, point was on the "Left" of the line (A->B).
                        // If cp1 < 0, point was on the "Right".
                        // "Forward" usually means crossing in the direction of the normal.
                        // Our normal is (-dy, dx) which is 90 deg counter-clockwise (Left).
                        // Wait, in overlay we defined Forward as 90 deg clockwise (Right).
                        // Let's stick to the overlay definition: Forward is "IN".
                        // If overlay draws arrow to the "Right" of A->B, then crossing from Left to Right is Forward.
                        
                        let direction = if cp1 > 0.0 { "forward" } else { "backward" };
                        
                        if line.direction != "both" && line.direction != "bidirectional" && line.direction != direction {
                             println!("DEBUG: Crossing ignored due to direction mismatch. Line: {}, Event: {}", line.direction, direction);
                             continue;
                        }

                        let direction_label = if direction == "forward" { "Entry" } else { "Exit" };
                        let event_desc = format!("{} ({})", line.name, direction_label);

                        let _ = db_state.db.insert_event(
                            camera_id_opt,
                            &response.module_id,
                            &response.processed_at,
                            &detection.label,
                            1,
                            detection.confidence,
                            detection.dwell_ms.map(|ms| ms as f32),
                            Some(detection.bounds.width),
                            Some(detection.bounds.height),
                            Some(&event_desc),
                            snapshot_path.as_deref(),
                        );
                    }
                }

                // Zone entries
                for zone in &zones {
                    if !zone.enabled { continue; }
                    if let Some(obj_type) = &zone.object_type {
                        if !obj_type.is_empty() && obj_type != &detection.label {
                            continue;
                        }
                    }

                    let polygon_points = parse_polygon(&zone.polygon);
                    let scaled_polygon: Vec<(f32, f32)> = polygon_points.iter().map(|p| {
                        (p.0 * response.frame_width as f32, p.1 * response.frame_height as f32)
                    }).collect();

                    let was_in = point_in_polygon(prev_center, &scaled_polygon);
                    let is_in = point_in_polygon(center, &scaled_polygon);

                    if !was_in && is_in {
                        println!("DEBUG: Zone entry detected! Zone: {}, Object: {}", zone.name, detection.label);
                        if snapshot_path.is_none() {
                             snapshot_path = self.save_object_snapshot(module, frame, detection, response.camera_id.as_deref());
                        }

                        let _ = db_state.db.insert_event(
                            camera_id_opt,
                            &response.module_id,
                            &response.processed_at,
                            &detection.label,
                            1,
                            detection.confidence,
                            detection.dwell_ms.map(|ms| ms as f32),
                            Some(detection.bounds.width),
                            Some(detection.bounds.height),
                            Some(&format!("Zone: {}", zone.name)),
                            snapshot_path.as_deref(),
                        );
                    }
                }
            }
        }
    }

    fn save_object_snapshot(
        &self,
        module: &ModuleEntry,
        frame: &DynamicImage,
        detection: &DetectionBox,
        camera_id: Option<&str>,
    ) -> Option<String> {
        let frame_rgba = frame.to_rgba8();
        let (frame_width, frame_height) = frame_rgba.dimensions();
        
        if detection.bounds.width <= 0.0 || detection.bounds.height <= 0.0 {
            return None;
        }

        let base_dir = resolve_snapshots_dir(module);
        let camera_segment = camera_id
            .filter(|value| !value.is_empty())
            .map(sanitize_path_segment)
            .unwrap_or_else(|| "unknown".to_string());
        
        let timestamp = Utc::now();
        let target_dir = base_dir
            .join(&camera_segment)
            .join(timestamp.format("%Y").to_string())
            .join(timestamp.format("%m").to_string())
            .join(timestamp.format("%d").to_string());
        
        if let Err(e) = fs::create_dir_all(&target_dir) {
            println!("Failed to create snapshot dir: {}", e);
            return None;
        }

        println!("DEBUG: Saving snapshot to {:?}", target_dir);

        // Expand bounds slightly (e.g. 10%)
        let expansion = 0.1;
        let expanded_bounds = BoundingBox {
            x: detection.bounds.x - detection.bounds.width * expansion / 2.0,
            y: detection.bounds.y - detection.bounds.height * expansion / 2.0,
            width: detection.bounds.width * (1.0 + expansion),
            height: detection.bounds.height * (1.0 + expansion),
        };

        // Ensure we have valid dimensions for cropping
        if frame_width == 0 || frame_height == 0 {
            return None;
        }

        let Some((x, y, width, height)) = compute_crop_rect(frame_width, frame_height, &expanded_bounds) else {
            println!("DEBUG: Failed to compute crop rect for snapshot");
            return None;
        };

        let crop = image::imageops::crop_imm(&frame_rgba, x, y, width, height).to_image();
        
        // Resize to standard width (320px) to ensure visibility for small objects
        // and consistency for large ones.
        let target_width = 320;
        let ratio = target_width as f32 / crop.width() as f32;
        let target_height = (crop.height() as f32 * ratio) as u32;
        
        let resized = image::imageops::resize(&crop, target_width, target_height, FilterType::Lanczos3);

        let filename = format!(
            "{}_{}_{}.jpg",
            timestamp.format("%H%M%S%f"),
            detection.label,
            detection.id
        );
        let path = target_dir.join(&filename);

        match resized.save(&path) {
            Ok(_) => {
                println!("DEBUG: Snapshot saved successfully: {:?}", path);
                // Return path relative to snapshots dir
                // Format: camera/YYYY/MM/DD/filename.jpg
                let relative_path = format!(
                    "{}/{}/{}/{}/{}",
                    camera_segment,
                    timestamp.format("%Y"),
                    timestamp.format("%m"),
                    timestamp.format("%d"),
                    filename
                );
                Some(relative_path)
            }
            Err(e) => {
                println!("Failed to save snapshot: {}", e);
                None
            }
        }
    }

    fn capture_face_snapshots(
        &self,
        module: &ModuleEntry,
        frame: &DynamicImage,
        detections: &[DetectionBox],
        camera_id: Option<&str>,
    ) -> Result<()> {
        if detections.is_empty() {
            return Ok(());
        }

        let snapshot_mode = module
            .manifest
            .face_snapshots_mode
            .unwrap_or(FaceSnapshotMode::Standard);
        if matches!(snapshot_mode, FaceSnapshotMode::Disabled) {
            return Ok(());
        }

        let encryption_key = if matches!(snapshot_mode, FaceSnapshotMode::Encrypted) {
            match module.manifest.face_snapshot_key.as_deref() {
                Some(key) => Some(key),
                None => {
                    println!(
                        "analytics {}: face snapshot encryption requested but faceSnapshotKey is not configured; skipping",
                        module.descriptor.id.as_str()
                    );
                    return Ok(());
                }
            }
        } else {
            None
        };

        let frame_rgba = frame.to_rgba8();
        let (frame_width, frame_height) = frame_rgba.dimensions();
        if frame_width < MIN_HD_WIDTH || frame_height < MIN_HD_HEIGHT {
            println!(
                "analytics {}: skipping snapshot capture for sub-HD frame {}x{}",
                module.descriptor.id.as_str(),
                frame_width,
                frame_height
            );
            return Ok(());
        }

        let base_dir = resolve_snapshots_dir(module);
        let camera_segment = camera_id
            .filter(|value| !value.is_empty())
            .map(sanitize_path_segment)
            .unwrap_or_else(|| "unknown".to_string());
        let camera_display = camera_id.unwrap_or("unknown");

        let timestamp = Utc::now();
        let target_dir = base_dir
            .join(&camera_segment)
            .join(timestamp.format("%Y").to_string())
            .join(timestamp.format("%m").to_string())
            .join(timestamp.format("%d").to_string());
        fs::create_dir_all(&target_dir)?;

        let mut ranked: Vec<(usize, &DetectionBox, f32)> = detections
            .iter()
            .enumerate()
            .map(|(idx, detection)| {
                let score = detection_priority(detection, frame_width, frame_height);
                (idx, detection, score)
            })
            .collect();
        ranked.sort_by(|a, b| b.2.partial_cmp(&a.2).unwrap_or(Ordering::Equal));

        if ranked.is_empty() {
            return Ok(());
        }

        let timestamp_str = timestamp.format("%Y%m%dT%H%M%S%.3fZ").to_string();

        let mut saved = 0usize;
        for (rank, (index, detection, priority)) in ranked.into_iter().enumerate() {
            if saved >= MAX_SNAPSHOTS_PER_FRAME {
                break;
            }
            if should_skip_detection(detection, frame_width, frame_height) {
                println!(
                    "analytics {}: skip detection idx={} due to shape/edge constraints (w={:.1}, h={:.1}, x={:.1}, y={:.1}, priority={:.3}, rank={})",
                    module.descriptor.id.as_str(),
                    index,
                    detection.bounds.width,
                    detection.bounds.height,
                    detection.bounds.x,
                    detection.bounds.y,
                    priority,
                    rank
                );
                continue;
            }
            let expanded_bounds =
                expand_detection_bounds(&detection.bounds, frame_width, frame_height);
            let Some((x, y, width, height)) =
                compute_crop_rect(frame_width, frame_height, &expanded_bounds)
            else {
                println!(
                    "analytics {}: skip snapshot idx={} due to invalid crop bounds ({:.1},{:.1},{:.1},{:.1}), expanded=({:.1},{:.1},{:.1},{:.1}), priority={:.3}, rank={})",
                    module.descriptor.id.as_str(),
                    index,
                    detection.bounds.x,
                    detection.bounds.y,
                    detection.bounds.width,
                    detection.bounds.height,
                    expanded_bounds.x,
                    expanded_bounds.y,
                    expanded_bounds.width,
                    expanded_bounds.height,
                    priority,
                    rank
                );
                continue;
            };
            if width < MIN_SNAPSHOT_EDGE || height < MIN_SNAPSHOT_EDGE {
                println!(
                    "analytics {}: skip snapshot idx={} due to small crop {}x{} (min={}), orig_bounds=({:.1},{:.1},{:.1},{:.1}), expanded=({:.1},{:.1},{:.1},{:.1}), priority={:.3}, rank={})",
                    module.descriptor.id.as_str(),
                    index,
                    width,
                    height,
                    MIN_SNAPSHOT_EDGE,
                    detection.bounds.x,
                    detection.bounds.y,
                    detection.bounds.width,
                    detection.bounds.height,
                    expanded_bounds.x,
                    expanded_bounds.y,
                    expanded_bounds.width,
                    expanded_bounds.height,
                    priority,
                    rank
                );
                continue;
            }

            let crop = imageops::crop_imm(&frame_rgba, x, y, width, height).to_image();

            let texture = measure_luma_variance(&crop);
            if texture < SNAPSHOT_MIN_TEXTURE_VARIANCE {
                println!(
                    "analytics {}: skip snapshot idx={} due to low texture variance {:.1} (threshold {:.1}), orig_bounds=({:.1},{:.1},{:.1},{:.1}), expanded=({:.1},{:.1},{:.1},{:.1}), priority={:.3}, rank={})",
                    module.descriptor.id.as_str(),
                    index,
                    texture,
                    SNAPSHOT_MIN_TEXTURE_VARIANCE,
                    detection.bounds.x,
                    detection.bounds.y,
                    detection.bounds.width,
                    detection.bounds.height,
                    expanded_bounds.x,
                    expanded_bounds.y,
                    expanded_bounds.width,
                    expanded_bounds.height,
                    priority,
                    rank
                );
                continue;
            }

            let prepared_image = prepare_snapshot_image(&crop);
            let processed_image = match snapshot_mode {
                FaceSnapshotMode::Anonymized => anonymize_snapshot_image(&prepared_image),
                _ => prepared_image.clone(),
            };

            let order = saved;
            let base_name = format!(
                "{}_{}_{}",
                timestamp_str,
                sanitize_path_segment(&detection.id),
                format!("{:03}", order)
            );
            let (image_filename, meta_filename) = match snapshot_mode {
                FaceSnapshotMode::Encrypted => (
                    format!("{base_name}.jpg.enc"),
                    format!("{base_name}.json.enc"),
                ),
                _ => (format!("{base_name}.jpg"), format!("{base_name}.json")),
            };
            let image_path = target_dir.join(&image_filename);
            let meta_path = target_dir.join(&meta_filename);

            let metadata = SnapshotMetadata {
                module_id: module.descriptor.id.as_str().to_string(),
                camera_id: camera_id.map(|value| value.to_string()),
                detection_id: detection.id.clone(),
                image_file: image_filename,
                captured_at: timestamp.to_rfc3339(),
                confidence: detection.confidence,
                bounds: detection.bounds.clone(),
                frame_width,
                frame_height,
            };

            match snapshot_mode {
                FaceSnapshotMode::Encrypted => {
                    if let Err(err) = save_encrypted_snapshot_files(
                        &image_path,
                        &meta_path,
                        &processed_image,
                        &metadata,
                        encryption_key.expect("encryption key checked above"),
                    ) {
                        println!(
                            "analytics {}: failed to save encrypted snapshot assets for {}: {}",
                            module.descriptor.id.as_str(),
                            image_path.display(),
                            err
                        );
                        continue;
                    }
                }
                _ => {
                    if let Err(err) = save_snapshot_image(&image_path, &processed_image) {
                        println!(
                            "analytics {}: failed to save snapshot image {}: {}",
                            module.descriptor.id.as_str(),
                            image_path.display(),
                            err
                        );
                        continue;
                    }

                    if let Err(err) = save_snapshot_metadata(&meta_path, &metadata) {
                        println!(
                            "analytics {}: failed to write snapshot metadata {}: {}",
                            module.descriptor.id.as_str(),
                            meta_path.display(),
                            err
                        );
                    }
                }
            }

            println!(
                "analytics {}: saved snapshot {} (output={}x{}, crop={}x{}, conf={:.2}, idx={}, order={}, camera={}, orig_bounds=({:.1},{:.1},{:.1},{:.1}), expanded=({:.1},{:.1},{:.1},{:.1}), priority={:.3}, texture_var={:.1}, rank={})",
                module.descriptor.id.as_str(),
                image_path.display(),
                processed_image.width(),
                processed_image.height(),
                width,
                height,
                detection.confidence,
                index,
                order,
                camera_display,
                detection.bounds.x,
                detection.bounds.y,
                detection.bounds.width,
                detection.bounds.height,
                expanded_bounds.x,
                expanded_bounds.y,
                expanded_bounds.width,
                expanded_bounds.height,
                priority,
                texture,
                rank
            );
            saved += 1;
        }

        Ok(())
    }

    fn capture_license_plate_snapshots(
        &self,
        module: &ModuleEntry,
        frame: &DynamicImage,
        detections: &[DetectionBox],
        camera_id: Option<&str>,
    ) -> Result<()> {
        if detections.is_empty() {
            return Ok(());
        }

        // Get app_handle for database access
        let app_handle = {
            let inner = self.inner.read();
            inner.app_handle.clone()
        };

        let frame_rgba = frame.to_rgba8();
        let (frame_width, frame_height) = frame_rgba.dimensions();
        if frame_width < MIN_HD_WIDTH || frame_height < MIN_HD_HEIGHT {
            println!(
                "analytics {}: skipping plate snapshots for sub-HD frame {}x{}",
                module.descriptor.id.as_str(),
                frame_width,
                frame_height
            );
            return Ok(());
        }

        let base_dir = resolve_snapshots_dir(module);
        let camera_segment = camera_id
            .filter(|value| !value.is_empty())
            .map(sanitize_path_segment)
            .unwrap_or_else(|| "unknown".to_string());
        let camera_display = camera_id.unwrap_or("unknown");

        let timestamp = Utc::now();
        let target_dir = base_dir
            .join(&camera_segment)
            .join(timestamp.format("%Y").to_string())
            .join(timestamp.format("%m").to_string())
            .join(timestamp.format("%d").to_string());
        fs::create_dir_all(&target_dir)?;

        let mut ranked: Vec<(usize, &DetectionBox, f32)> = detections
            .iter()
            .enumerate()
            .map(|(idx, detection)| {
                let score = detection_priority(detection, frame_width, frame_height);
                (idx, detection, score)
            })
            .collect();
        ranked.sort_by(|a, b| b.2.partial_cmp(&a.2).unwrap_or(Ordering::Equal));

        if ranked.is_empty() {
            return Ok(());
        }

        let timestamp_str = timestamp.format("%Y%m%dT%H%M%S%.3fZ").to_string();
        let mut saved = 0usize;

        for (rank, (index, detection, priority)) in ranked.into_iter().enumerate() {
            if saved >= MAX_SNAPSHOTS_PER_FRAME {
                break;
            }
            if detection.bounds.width <= 0.0 || detection.bounds.height <= 0.0 {
                println!(
                    "analytics {}: skip plate snapshot idx={} due to invalid bounds ({:.1},{:.1},{:.1},{:.1}), rank={}, priority={:.3}",
                    module.descriptor.id.as_str(),
                    index,
                    detection.bounds.x,
                    detection.bounds.y,
                    detection.bounds.width,
                    detection.bounds.height,
                    rank,
                    priority
                );
                continue;
            }

            let expanded_bounds =
                expand_license_plate_bounds(&detection.bounds, frame_width, frame_height);
            let Some((x, y, width, height)) =
                compute_crop_rect(frame_width, frame_height, &expanded_bounds)
            else {
                println!(
                    "analytics {}: skip plate snapshot idx={} due to crop failure, expanded=({:.1},{:.1},{:.1},{:.1}), rank={}, priority={:.3}",
                    module.descriptor.id.as_str(),
                    index,
                    expanded_bounds.x,
                    expanded_bounds.y,
                    expanded_bounds.width,
                    expanded_bounds.height,
                    rank,
                    priority
                );
                continue;
            };

            if width < LICENSE_PLATE_SNAPSHOT_MIN_EDGE || height < LICENSE_PLATE_SNAPSHOT_MIN_EDGE {
                println!(
                    "analytics {}: skip plate snapshot idx={} due to small crop {}x{} (min={}), rank={}, priority={:.3}",
                    module.descriptor.id.as_str(),
                    index,
                    width,
                    height,
                    LICENSE_PLATE_SNAPSHOT_MIN_EDGE,
                    rank,
                    priority
                );
                continue;
            }

            let crop = imageops::crop_imm(&frame_rgba, x, y, width, height).to_image();
            let texture = measure_luma_variance(&crop);
            if texture < LICENSE_PLATE_SNAPSHOT_MIN_TEXTURE_VARIANCE {
                println!(
                    "analytics {}: skip plate snapshot idx={} due to low texture variance {:.1} (<{:.1}), rank={}, priority={:.3}",
                    module.descriptor.id.as_str(),
                    index,
                    texture,
                    LICENSE_PLATE_SNAPSHOT_MIN_TEXTURE_VARIANCE,
                    rank,
                    priority
                );
                continue;
            }

            let prepared_image = prepare_snapshot_image(&crop);
            let order = saved;

            let label_segment = sanitize_path_segment(&detection.label);
            let detection_segment = sanitize_path_segment(&detection.id);
            let mut filename_parts = vec![timestamp_str.clone()];
            if !label_segment.is_empty() {
                filename_parts.push(label_segment);
            }
            if !detection_segment.is_empty() {
                filename_parts.push(detection_segment);
            }
            filename_parts.push(format!("{:03}", order));
            let base_name = filename_parts.join("_");

            // Save full vehicle frame
            let full_frame_filename = format!("{base_name}_full.jpg");
            let full_frame_path = target_dir.join(&full_frame_filename);
            if let Err(err) = save_snapshot_image(&full_frame_path, &frame_rgba) {
                println!(
                    "analytics {}: failed to save full vehicle frame {}: {}",
                    module.descriptor.id.as_str(),
                    full_frame_path.display(),
                    err
                );
            }

            // Save plate crop
            let image_filename = format!("{base_name}.jpg");
            let meta_filename = format!("{base_name}.json");
            let image_path = target_dir.join(&image_filename);
            let meta_path = target_dir.join(&meta_filename);

            if let Err(err) = save_snapshot_image(&image_path, &prepared_image) {
                println!(
                    "analytics {}: failed to save plate snapshot {}: {}",
                    module.descriptor.id.as_str(),
                    image_path.display(),
                    err
                );
                continue;
            }

            let metadata = SnapshotMetadata {
                module_id: module.descriptor.id.as_str().to_string(),
                camera_id: camera_id.map(|value| value.to_string()),
                detection_id: detection.id.clone(),
                image_file: image_filename,
                captured_at: timestamp.to_rfc3339(),
                confidence: detection.confidence,
                bounds: detection.bounds.clone(),
                frame_width,
                frame_height,
            };

            if let Err(err) = save_snapshot_metadata(&meta_path, &metadata) {
                println!(
                    "analytics {}: failed to write plate snapshot metadata {}: {}",
                    module.descriptor.id.as_str(),
                    meta_path.display(),
                    err
                );
            }

            // Insert record into database
            use crate::database::{commands::DatabaseState, PlateRecordInsert};
            if let Some(db_state) = app_handle.try_state::<DatabaseState>() {
                let plate_number = extract_plate_number(&detection.label);
                let camera_id_num = parse_camera_id(camera_id);

                let record = PlateRecordInsert {
                    camera_id: camera_id_num,
                    plate_number,
                    confidence: detection.confidence,
                    timestamp: timestamp.to_rfc3339(),
                    full_image_path: full_frame_path.to_string_lossy().to_string(),
                    plate_crop_path: image_path.to_string_lossy().to_string(),
                    vehicle_type: None,
                    direction: None,
                };

                if let Err(err) = db_state.db.insert_record(record) {
                    println!(
                        "analytics {}: failed to insert plate record into database: {}",
                        module.descriptor.id.as_str(),
                        err
                    );
                }
            } else {
                println!(
                    "analytics {}: database state not available for plate record",
                    module.descriptor.id.as_str()
                );
            }

            println!(
                "analytics {}: saved plate snapshot {} (label='{}', output={}x{}, crop={}x{}, conf={:.2}, idx={}, order={}, camera={}, expanded=({:.1},{:.1},{:.1},{:.1}), priority={:.3}, texture_var={:.1}, rank={})",
                module.descriptor.id.as_str(),
                image_path.display(),
                detection.label,
                prepared_image.width(),
                prepared_image.height(),
                width,
                height,
                detection.confidence,
                index,
                order,
                camera_display,
                expanded_bounds.x,
                expanded_bounds.y,
                expanded_bounds.width,
                expanded_bounds.height,
                priority,
                texture,
                rank
            );
            saved += 1;
        }

        Ok(())
    }
}

fn prepare_module_engine(
    module_dir: &Path,
    descriptor: ModuleDescriptor,
) -> std::result::Result<Arc<dyn AnalyticsEngine>, String> {
    for resource in descriptor.resources {
        let mut noop = |_fraction: f32| {};
        ensure_module_resource_with_progress(module_dir, *resource, descriptor, &mut noop)?;
    }

    (descriptor.builder)(module_dir)
}

#[allow(dead_code)]
fn ensure_module_resource(
    module_dir: &Path,
    resource: ModuleResourceSpec,
    descriptor: ModuleDescriptor,
) -> std::result::Result<(), String> {
    let mut noop = |_fraction: f32| {};
    ensure_module_resource_with_progress(module_dir, resource, descriptor, &mut noop)
}

fn ensure_module_resource_with_progress<F>(
    module_dir: &Path,
    resource: ModuleResourceSpec,
    descriptor: ModuleDescriptor,
    progress: &mut F,
) -> std::result::Result<(), String>
where
    F: FnMut(f32),
{
    match resource {
        ModuleResourceSpec::File(spec) => {
            ensure_file_resource_with_progress(module_dir, spec, descriptor, progress)
        }
        ModuleResourceSpec::Archive(spec) => {
            ensure_archive_resource_with_progress(module_dir, spec, descriptor, progress)
        }
    }
}

#[allow(dead_code)]
fn ensure_file_resource(
    module_dir: &Path,
    spec: ModuleDownloadSpec,
    descriptor: ModuleDescriptor,
) -> std::result::Result<(), String> {
    let mut noop = |_fraction: f32| {};
    ensure_file_resource_with_progress(module_dir, spec, descriptor, &mut noop)
}

fn ensure_file_resource_with_progress<F>(
    module_dir: &Path,
    spec: ModuleDownloadSpec,
    descriptor: ModuleDescriptor,
    progress: &mut F,
) -> std::result::Result<(), String>
where
    F: FnMut(f32),
{
    let target_path = module_dir.join(spec.file_name);

    if target_path.exists() {
        if let Some(expected) = spec.sha256 {
            if !verify_sha256(&target_path, expected)? {
                download_file_with_progress(
                    spec.url,
                    &target_path,
                    spec.sha256,
                    descriptor,
                    progress,
                )?;
            }
        }
        progress(1.0);
        return Ok(());
    }

    download_file_with_progress(spec.url, &target_path, spec.sha256, descriptor, progress)?;
    progress(1.0);
    Ok(())
}

#[allow(dead_code)]
fn ensure_archive_resource(
    module_dir: &Path,
    spec: ModuleArchiveSpec,
    descriptor: ModuleDescriptor,
) -> std::result::Result<(), String> {
    let mut noop = |_fraction: f32| {};
    ensure_archive_resource_with_progress(module_dir, spec, descriptor, &mut noop)
}

fn ensure_archive_resource_with_progress<F>(
    module_dir: &Path,
    spec: ModuleArchiveSpec,
    descriptor: ModuleDescriptor,
    progress: &mut F,
) -> std::result::Result<(), String>
where
    F: FnMut(f32),
{
    let all_present = spec
        .required_files
        .iter()
        .all(|rel| module_dir.join(rel).exists());
    if all_present {
        progress(1.0);
        return Ok(());
    }

    let archive_path = module_dir.join(spec.file_name);
    let should_download = if archive_path.exists() {
        if let Some(expected) = spec.sha256 {
            !verify_sha256(&archive_path, expected)?
        } else {
            false
        }
    } else {
        true
    };

    if should_download {
        let mut download_stage = |fraction: f32| {
            progress(0.9_f32 * fraction.clamp(0.0, 1.0));
        };
        download_file_with_progress(
            spec.url,
            &archive_path,
            spec.sha256,
            descriptor,
            &mut download_stage,
        )?;
    } else {
        progress(0.9);
    }

    extract_archive(module_dir, &archive_path, spec)?;
    progress(1.0);

    let all_present = spec
        .required_files
        .iter()
        .all(|rel| module_dir.join(rel).exists());
    if all_present {
        Ok(())
    } else {
        Err(format!(
            "Failed to extract required files for module {} from {}",
            descriptor.id.as_str(),
            spec.file_name
        ))
    }
}

fn download_file_with_progress<F>(
    url: &str,
    target_path: &Path,
    sha256: Option<&str>,
    descriptor: ModuleDescriptor,
    progress: &mut F,
) -> std::result::Result<(), String>
where
    F: FnMut(f32),
{
    let parent = target_path.parent().ok_or_else(|| {
        format!(
            "Invalid destination path for module {}",
            descriptor.id.as_str()
        )
    })?;
    fs::create_dir_all(parent)
        .map_err(|err| format!("Failed to prepare directory {}: {}", parent.display(), err))?;

    let mut response = reqwest::blocking::get(url)
        .map_err(|err| format!("Failed to download {}: {}", url, err))?;

    if !response.status().is_success() {
        return Err(format!(
            "Download failed for {} (status: {})",
            url,
            response.status()
        ));
    }

    let tmp_path = target_path.with_extension("download");
    let mut tmp_file = fs::File::create(&tmp_path).map_err(|err| {
        format!(
            "Failed to write temporary file {}: {}",
            tmp_path.display(),
            err
        )
    })?;

    let total_size = response.content_length();
    let mut downloaded: u64 = 0;
    let mut buffer = [0u8; 64 * 1024];
    let mut hasher = sha256.map(|_| sha2::Sha256::new());

    loop {
        let read_len = response
            .read(&mut buffer)
            .map_err(|err| format!("Failed to read download {}: {}", url, err))?;
        if read_len == 0 {
            break;
        }

        tmp_file
            .write_all(&buffer[..read_len])
            .map_err(|err| format!("Failed to write download {}: {}", url, err))?;

        if let Some(ref mut hasher) = hasher {
            hasher.update(&buffer[..read_len]);
        }

        downloaded += read_len as u64;
        if let Some(total) = total_size {
            if total > 0 {
                let fraction = (downloaded as f32 / total as f32).min(1.0);
                progress(fraction);
            }
        } else {
            let fraction = ((downloaded as f32) / 5_000_000.0).min(0.95);
            progress(fraction);
        }
    }

    tmp_file
        .sync_all()
        .map_err(|err| format!("Failed to finalize download {}: {}", url, err))?;

    if let Some(hasher) = hasher {
        let digest = hasher.finalize();
        let digest_hex = format!("{:x}", digest);
        if let Some(expected) = sha256 {
            if !digest_hex.eq_ignore_ascii_case(expected) {
                let _ = fs::remove_file(&tmp_path);
                return Err(format!(
                    "SHA256 mismatch for {} (expected {}, got {})",
                    url, expected, digest_hex
                ));
            }
        }
    }

    if target_path.exists() {
        fs::remove_file(target_path).map_err(|err| {
            format!(
                "Failed to replace existing file {}: {}",
                target_path.display(),
                err
            )
        })?;
    }

    fs::rename(&tmp_path, target_path).map_err(|err| {
        format!(
            "Failed to finalize download to {}: {}",
            target_path.display(),
            err
        )
    })?;

    progress(1.0);
    Ok(())
}

fn extract_archive(
    module_dir: &Path,
    archive_path: &Path,
    spec: ModuleArchiveSpec,
) -> std::result::Result<(), String> {
    match spec.archive_type {
        ArchiveType::Zip => extract_zip_archive(module_dir, archive_path, spec),
        ArchiveType::TarGz => extract_targz_archive(module_dir, archive_path, spec),
    }
}

fn extract_zip_archive(
    module_dir: &Path,
    archive_path: &Path,
    spec: ModuleArchiveSpec,
) -> std::result::Result<(), String> {
    let file = fs::File::open(archive_path)
        .map_err(|err| format!("Failed to open archive {}: {}", archive_path.display(), err))?;
    let mut archive = ZipArchive::new(file).map_err(|err| {
        format!(
            "Failed to read zip archive {}: {}",
            archive_path.display(),
            err
        )
    })?;
    let target_root = module_dir.join(spec.target_dir);

    for idx in 0..archive.len() {
        let mut entry = archive.by_index(idx).map_err(|err| {
            format!(
                "Failed to read zip entry {} in {}: {}",
                idx,
                archive_path.display(),
                err
            )
        })?;

        if entry.is_dir() {
            continue;
        }

        let Some(relative_path) = strip_archive_prefix(entry.name(), spec.strip_prefix) else {
            continue;
        };
        if relative_path.is_empty() {
            continue;
        }

        let target_path = target_root.join(&relative_path);
        if let Some(parent) = target_path.parent() {
            fs::create_dir_all(parent).map_err(|err| {
                format!("Failed to create directory {}: {}", parent.display(), err)
            })?;
        }

        let mut output = fs::File::create(&target_path)
            .map_err(|err| format!("Failed to create file {}: {}", target_path.display(), err))?;
        std::io::copy(&mut entry, &mut output).map_err(|err| {
            format!(
                "Failed to extract {} from {}: {}",
                relative_path,
                archive_path.display(),
                err
            )
        })?;
    }

    Ok(())
}

fn extract_targz_archive(
    module_dir: &Path,
    archive_path: &Path,
    spec: ModuleArchiveSpec,
) -> std::result::Result<(), String> {
    let file = fs::File::open(archive_path)
        .map_err(|err| format!("Failed to open archive {}: {}", archive_path.display(), err))?;
    let decoder = GzDecoder::new(file);
    let mut archive = TarArchive::new(decoder);
    let target_root = module_dir.join(spec.target_dir);

    let mut entries = archive.entries().map_err(|err| {
        format!(
            "Failed to iterate tar archive {}: {}",
            archive_path.display(),
            err
        )
    })?;

    while let Some(entry_result) = entries.next() {
        let mut entry = entry_result.map_err(|err| {
            format!(
                "Failed to read tar entry in {}: {}",
                archive_path.display(),
                err
            )
        })?;
        let entry_type = entry.header().entry_type();
        if entry_type.is_dir() || entry_type.is_gnu_longname() {
            continue;
        }
        if entry_type.is_symlink() || entry_type.is_hard_link() {
            continue;
        }

        let path = entry
            .path()
            .map_err(|err| format!("Invalid tar path in {}: {}", archive_path.display(), err))?;
        let path_str = path.to_string_lossy();
        let Some(relative_path) = strip_archive_prefix(&path_str, spec.strip_prefix) else {
            continue;
        };
        if relative_path.is_empty() {
            continue;
        }

        let target_path = target_root.join(&relative_path);
        if let Some(parent) = target_path.parent() {
            fs::create_dir_all(parent).map_err(|err| {
                format!("Failed to create directory {}: {}", parent.display(), err)
            })?;
        }

        entry.unpack(&target_path).map_err(|err| {
            format!(
                "Failed to extract {} from {}: {}",
                relative_path,
                archive_path.display(),
                err
            )
        })?;
    }

    Ok(())
}

fn strip_archive_prefix(path: &str, prefix: Option<&str>) -> Option<String> {
    let normalized = normalize_archive_path(path);
    if normalized.is_empty() {
        return None;
    }
    if normalized.split('/').any(|segment| segment == "..") {
        return None;
    }

    match prefix {
        Some(prefix) => {
            let mut normalized_prefix = normalize_archive_path(prefix);
            if normalized_prefix.is_empty() {
                return Some(normalized);
            }
            if !normalized_prefix.is_empty() && !normalized_prefix.ends_with('/') {
                normalized_prefix.push('/');
            }
            if !normalized_prefix.is_empty() && normalized.starts_with(&normalized_prefix) {
                let remainder = &normalized[normalized_prefix.len()..];
                if remainder.is_empty() || remainder.ends_with('/') {
                    None
                } else {
                    Some(remainder.to_string())
                }
            } else {
                None
            }
        }
        None => {
            if normalized.ends_with('/') {
                None
            } else {
                Some(normalized)
            }
        }
    }
}

fn normalize_archive_path(path: &str) -> String {
    path.trim_start_matches("./")
        .trim_start_matches('/')
        .replace('\\', "/")
}

fn verify_sha256(path: &Path, expected: &str) -> std::result::Result<bool, String> {
    let bytes = fs::read(path).map_err(|err| {
        format!(
            "Failed to read file {} for checksum: {}",
            path.display(),
            err
        )
    })?;
    let mut hasher = sha2::Sha256::new();
    hasher.update(&bytes);
    let digest = hasher.finalize();
    let digest_hex = format!("{:x}", digest);
    Ok(digest_hex.eq_ignore_ascii_case(expected))
}

fn module_status(entry: &ModuleEntry) -> ModuleStatus {
    let (state, progress, transient_message) = match &entry.state {
        ModuleState::Disabled => ("disabled".to_string(), None, None),
        ModuleState::Loading { progress, message } => (
            "loading".to_string(),
            Some((*progress).clamp(0.0, 1.0)),
            message.clone(),
        ),
        ModuleState::Ready => ("ready".to_string(), Some(1.0), None),
        ModuleState::Error { .. } => ("error".to_string(), None, None),
    };

    let message = transient_message.or_else(|| {
        entry
            .manifest
            .last_error
            .clone()
            .or_else(|| match &entry.state {
                ModuleState::Error { message } => Some(message.clone()),
                _ => None,
            })
    });

    let mut config = ModuleConfig {
        snapshots_dir: entry.manifest.snapshots_dir.clone(),
        face_snapshots_mode: None,
        face_snapshot_key_configured: None,
    };

    if entry.descriptor.id == AnalyticsModuleId::FaceDetector {
        config.face_snapshots_mode = Some(
            entry
                .manifest
                .face_snapshots_mode
                .unwrap_or_default(),
        );
        config.face_snapshot_key_configured = Some(entry.manifest.face_snapshot_key.is_some());
    }

    ModuleStatus {
        id: entry.descriptor.id.as_str().into(),
        name: entry.descriptor.name.into(),
        version: entry.descriptor.version.into(),
        description: entry.descriptor.description.into(),
        enabled: if matches!(entry.state, ModuleState::Loading { .. }) {
            true
        } else {
            entry.manifest.enabled
        },
        state,
        progress,
        message,
        last_activated_at: entry.manifest.last_activated_at.clone(),
        last_error_at: entry.manifest.last_error_at.clone(),
        config: if config.is_empty() {
            None
        } else {
            Some(config)
        },
    }
}

fn default_snapshots_dir(module_dir: &Path) -> PathBuf {
    module_dir.join("snapshots")
}

fn resolve_snapshots_dir(entry: &ModuleEntry) -> PathBuf {
    let resolved = entry
        .manifest
        .snapshots_dir
        .as_ref()
        .map(PathBuf::from)
        .unwrap_or_else(|| default_snapshots_dir(&entry.module_dir));

    if resolved.is_absolute() {
        resolved
    } else {
        entry.module_dir.join(resolved)
    }
}

fn sanitize_path_segment(value: &str) -> String {
    let mut sanitized = String::new();
    for ch in value.chars() {
        let replacement = match ch {
            'a'..='z' | 'A'..='Z' | '0'..='9' | '-' | '_' | '.' => Some(ch),
            ' ' | '\t' | '\n' | '\r' => Some('_'),
            _ => Some('_'),
        };
        if let Some(ch) = replacement {
            sanitized.push(ch);
        }
        if sanitized.len() >= 64 {
            break;
        }
    }

    if sanitized.is_empty() {
        "_".to_string()
    } else {
        sanitized
    }
}

fn normalize_face_snapshot_key(raw: &str) -> std::result::Result<String, String> {
    let trimmed = raw.trim();
    if trimmed.is_empty() {
        return Err("faceSnapshotKey must not be empty".to_string());
    }

    let candidate = trimmed.trim_start_matches("0x").to_ascii_lowercase();
    if candidate.len() != 64 {
        return Err("faceSnapshotKey must contain 64 hex characters (32 bytes)".to_string());
    }

    if !candidate.chars().all(|ch| ch.is_ascii_hexdigit()) {
        return Err("faceSnapshotKey must contain only hexadecimal characters".to_string());
    }

    Ok(candidate)
}

fn compute_crop_rect(
    frame_width: u32,
    frame_height: u32,
    bounds: &BoundingBox,
) -> Option<(u32, u32, u32, u32)> {
    if frame_width == 0 || frame_height == 0 {
        return None;
    }

    let max_x = frame_width.saturating_sub(1) as f32;
    let max_y = frame_height.saturating_sub(1) as f32;

    let mut x1 = bounds.x.floor().max(0.0);
    let mut y1 = bounds.y.floor().max(0.0);
    let mut x2 = (bounds.x + bounds.width).ceil().max(0.0);
    let mut y2 = (bounds.y + bounds.height).ceil().max(0.0);

    x1 = x1.min(max_x);
    y1 = y1.min(max_y);
    x2 = x2.min(frame_width as f32);
    y2 = y2.min(frame_height as f32);

    if x2 <= x1 || y2 <= y1 {
        return None;
    }

    let width = (x2 - x1).ceil().max(1.0) as u32;
    let height = (y2 - y1).ceil().max(1.0) as u32;

    Some((x1.floor() as u32, y1.floor() as u32, width, height))
}

fn expand_detection_bounds(
    bounds: &BoundingBox,
    frame_width: u32,
    frame_height: u32,
) -> BoundingBox {
    let frame_width_f = frame_width as f32;
    let frame_height_f = frame_height as f32;

    let expansion = dynamic_snapshot_expansion(bounds, frame_height_f);

    let base_width = bounds.width.max(1.0);
    let base_height = bounds.height.max(1.0);

    let mut scaled_width = (base_width * expansion).max(SNAPSHOT_MIN_CROP_WIDTH.min(frame_width_f));
    let mut scaled_height =
        (base_height * expansion).max(SNAPSHOT_MIN_CROP_HEIGHT.min(frame_height_f));

    if SNAPSHOT_MAX_EXPANSION_MULTIPLIER > 0.0 {
        let max_width = (base_width * SNAPSHOT_MAX_EXPANSION_MULTIPLIER).min(frame_width_f);
        let max_height = (base_height * SNAPSHOT_MAX_EXPANSION_MULTIPLIER).min(frame_height_f);
        if scaled_width > max_width {
            scaled_width = max_width.max(SNAPSHOT_MIN_CROP_WIDTH.min(frame_width_f));
        }
        if scaled_height > max_height {
            scaled_height = max_height.max(SNAPSHOT_MIN_CROP_HEIGHT.min(frame_height_f));
        }
    }

    let mut width = scaled_width.min(frame_width_f);
    let mut height = scaled_height.min(frame_height_f);

    if width / height < SNAPSHOT_TARGET_ASPECT {
        width = (height * SNAPSHOT_TARGET_ASPECT).min(frame_width_f);
    } else if width / height > SNAPSHOT_TARGET_ASPECT {
        height = (width / SNAPSHOT_TARGET_ASPECT).min(frame_height_f);
    }

    width = width
        .max(SNAPSHOT_MIN_CROP_WIDTH.min(frame_width_f))
        .min(frame_width_f);
    height = height
        .max(SNAPSHOT_MIN_CROP_HEIGHT.min(frame_height_f))
        .min(frame_height_f);

    if width / height < SNAPSHOT_TARGET_ASPECT {
        height = (width / SNAPSHOT_TARGET_ASPECT).min(frame_height_f);
    } else if width / height > SNAPSHOT_TARGET_ASPECT {
        width = (height * SNAPSHOT_TARGET_ASPECT).min(frame_width_f);
    }

    if SNAPSHOT_MAX_FRAME_COVERAGE_RATIO > 0.0 && SNAPSHOT_MAX_FRAME_COVERAGE_RATIO < 1.0 {
        let max_width = frame_width_f * SNAPSHOT_MAX_FRAME_COVERAGE_RATIO;
        let max_height = frame_height_f * SNAPSHOT_MAX_FRAME_COVERAGE_RATIO;
        let mut scale = 1.0f32;
        if width > max_width && max_width > 0.0 {
            scale = scale.min(max_width / width);
        }
        if height > max_height && max_height > 0.0 {
            scale = scale.min(max_height / height);
        }
        if scale < 1.0 {
            width = (width * scale)
                .max(SNAPSHOT_MIN_CROP_WIDTH.min(frame_width_f))
                .min(frame_width_f);
            height = (height * scale)
                .max(SNAPSHOT_MIN_CROP_HEIGHT.min(frame_height_f))
                .min(frame_height_f);
            if width / height < SNAPSHOT_TARGET_ASPECT {
                height = (width / SNAPSHOT_TARGET_ASPECT).min(frame_height_f);
            } else if width / height > SNAPSHOT_TARGET_ASPECT {
                width = (height * SNAPSHOT_TARGET_ASPECT).min(frame_width_f);
            }
        }
    }

    let mut x1 = bounds.x + bounds.width / 2.0 - width / 2.0;
    let mut y1 = bounds.y + bounds.height / 2.0 - height / 2.0;
    let mut x2 = x1 + width;
    let mut y2 = y1 + height;

    if x1 < 0.0 {
        x2 -= x1;
        x1 = 0.0;
    }
    if y1 < 0.0 {
        y2 -= y1;
        y1 = 0.0;
    }

    if x2 > frame_width as f32 {
        let diff = x2 - frame_width as f32;
        x1 -= diff;
        x2 = frame_width as f32;
        if x1 < 0.0 {
            x1 = 0.0;
        }
    }
    if y2 > frame_height as f32 {
        let diff = y2 - frame_height as f32;
        y1 -= diff;
        y2 = frame_height as f32;
        if y1 < 0.0 {
            y1 = 0.0;
        }
    }

    if bounds.height > 0.0 {
        let desired = (bounds.height * SNAPSHOT_VERTICAL_BIAS_RATIO).max(0.0);
        if desired > 0.0 {
            let available_up = y1.max(0.0);
            let shift_up = desired.min(available_up);
            if shift_up > 0.0 {
                y1 -= shift_up;
                y2 -= shift_up;
            } else {
                let available_down = (frame_height_f - y2).max(0.0);
                let shift_down = desired.min(available_down);
                if shift_down > 0.0 {
                    y1 += shift_down;
                    y2 += shift_down;
                }
            }
        }
    }

    BoundingBox {
        x: x1,
        y: y1,
        width: (x2 - x1).max(1.0),
        height: (y2 - y1).max(1.0),
    }
}

fn expand_license_plate_bounds(
    bounds: &BoundingBox,
    frame_width: u32,
    frame_height: u32,
) -> BoundingBox {
    let frame_width_f = frame_width as f32;
    let frame_height_f = frame_height as f32;

    let mut x1 = bounds.x - bounds.width * LICENSE_PLATE_SNAPSHOT_HORIZONTAL_MARGIN_RATIO;
    let mut x2 = bounds.x + bounds.width * (1.0 + LICENSE_PLATE_SNAPSHOT_HORIZONTAL_MARGIN_RATIO);
    let mut y1 = bounds.y - bounds.height * LICENSE_PLATE_SNAPSHOT_TOP_MARGIN_RATIO;
    let mut y2 = bounds.y + bounds.height * (1.0 + LICENSE_PLATE_SNAPSHOT_BOTTOM_MARGIN_RATIO);

    x1 = x1.max(0.0);
    y1 = y1.max(0.0);
    x2 = x2.min(frame_width_f);
    y2 = y2.min(frame_height_f);

    if x2 <= x1 || y2 <= y1 {
        return bounds.clone();
    }

    let width = (x2 - x1).max(bounds.width * 0.9).max(1.0);
    let height = (y2 - y1).max(bounds.height * 0.9).max(1.0);

    BoundingBox {
        x: x1,
        y: y1,
        width,
        height,
    }
}

fn dynamic_snapshot_expansion(bounds: &BoundingBox, frame_height: f32) -> f32 {
    if frame_height <= 0.0 {
        return SNAPSHOT_EXPANSION_FACTOR_MAX;
    }

    let relative_height = (bounds.height / frame_height).clamp(0.0, 1.0);
    let range = (SNAPSHOT_EXPANSION_FACTOR_MAX - SNAPSHOT_EXPANSION_FACTOR_MIN).max(0.0);
    let factor = SNAPSHOT_EXPANSION_FACTOR_MIN + (1.0 - relative_height).powf(1.2) * range;
    factor.clamp(SNAPSHOT_EXPANSION_FACTOR_MIN, SNAPSHOT_EXPANSION_FACTOR_MAX)
}

fn extract_plate_number(label: &str) -> String {
    // Extract plate number from detection label
    // Format: "Plate М666ММ777" or just "М666ММ777"
    let re = Regex::new(r"(?:Plate\s+)?([А-ЯA-Z0-9]+)").unwrap();
    if let Some(caps) = re.captures(label) {
        if let Some(plate) = caps.get(1) {
            return plate.as_str().to_string();
        }
    }
    label.to_string()
}

fn parse_camera_id(camera_id: Option<&str>) -> i32 {
    // Try to extract numeric camera ID from camera_id string
    // E.g., "camera_1" -> 1, "cam_5" -> 5
    camera_id
        .and_then(|id| {
            let re = Regex::new(r"(\d+)").unwrap();
            re.find(id).and_then(|m| m.as_str().parse::<i32>().ok())
        })
        .unwrap_or(0)
}

fn detection_priority(detection: &DetectionBox, frame_width: u32, frame_height: u32) -> f32 {
    let confidence = detection.confidence.clamp(0.0, 1.0);
    let area = (detection.bounds.width.max(1.0) * detection.bounds.height.max(1.0))
        / (frame_width.max(1) as f32 * frame_height.max(1) as f32);
    let area_score = area.sqrt().clamp(0.0, 1.0);
    let center_x = detection.bounds.x + detection.bounds.width / 2.0;
    let center_y = detection.bounds.y + detection.bounds.height / 2.0;
    let margin_x =
        (center_x.min(frame_width as f32 - center_x)).max(0.0) / frame_width.max(1) as f32;
    let margin_y =
        (center_y.min(frame_height as f32 - center_y)).max(0.0) / frame_height.max(1) as f32;
    let edge_bonus = margin_x.min(margin_y).clamp(0.0, 0.5) * 2.0;
    confidence * 0.6 + area_score * 0.3 + edge_bonus * 0.1
}

fn should_skip_detection(detection: &DetectionBox, frame_width: u32, frame_height: u32) -> bool {
    if detection.bounds.width <= 0.0 || detection.bounds.height <= 0.0 {
        return true;
    }

    let aspect = detection.bounds.width / detection.bounds.height.max(1.0);
    if aspect < SNAPSHOT_ASPECT_MIN || aspect > SNAPSHOT_ASPECT_MAX {
        return true;
    }

    if SNAPSHOT_EDGE_MARGIN_HORIZONTAL_RATIO > 0.0 {
        let margin_x = frame_width as f32 * SNAPSHOT_EDGE_MARGIN_HORIZONTAL_RATIO;
        if detection.bounds.x <= margin_x
            || detection.bounds.x + detection.bounds.width >= frame_width as f32 - margin_x
        {
            return true;
        }
    }

    if SNAPSHOT_EDGE_MARGIN_VERTICAL_RATIO > 0.0 {
        let margin_y = frame_height as f32 * SNAPSHOT_EDGE_MARGIN_VERTICAL_RATIO;
        if detection.bounds.y <= margin_y
            || detection.bounds.y + detection.bounds.height >= frame_height as f32 - margin_y
        {
            return true;
        }
    }

    false
}

fn measure_luma_variance(image: &RgbaImage) -> f32 {
    if image.width() == 0 || image.height() == 0 {
        return 0.0;
    }

    let mut sum = 0.0f64;
    let mut sum_sq = 0.0f64;
    let mut count = 0usize;

    for pixel in image.pixels() {
        let luma = 0.2126 * pixel[0] as f64 + 0.7152 * pixel[1] as f64 + 0.0722 * pixel[2] as f64;
        sum += luma;
        sum_sq += luma * luma;
        count += 1;
    }

    if count == 0 {
        return 0.0;
    }

    let mean = sum / count as f64;
    let variance = (sum_sq / count as f64) - mean * mean;
    variance.max(0.0) as f32
}

fn anonymize_snapshot_image(image: &RgbaImage) -> RgbaImage {
    imageops::blur(image, 12.0)
}

fn save_snapshot_image(path: &Path, image: &RgbaImage) -> std::result::Result<(), String> {
    let encoded = encode_snapshot_image(image)?;
    write_binary_file(path, &encoded)
}

fn prepare_snapshot_image(image: &RgbaImage) -> RgbaImage {
    // Return the original image without resizing to preserve quality and aspect ratio
    image.clone()
}

fn encode_snapshot_image(image: &RgbaImage) -> std::result::Result<Vec<u8>, String> {
    let mut buffer = Vec::new();
    {
        let mut encoder = JpegEncoder::new_with_quality(&mut buffer, JPEG_QUALITY);
        // Convert to RGB8 to ensure compatibility and remove alpha channel
        let rgb_image = DynamicImage::ImageRgba8(image.clone()).to_rgb8();
        encoder
            .encode(
                rgb_image.as_raw(),
                rgb_image.width(),
                rgb_image.height(),
                ColorType::Rgb8,
            )
            .map_err(|err| format!("failed to encode snapshot image: {err}"))?;
    }
    Ok(buffer)
}

fn write_binary_file(path: &Path, data: &[u8]) -> std::result::Result<(), String> {
    if let Some(parent) = path.parent() {
        if !parent.exists() {
            fs::create_dir_all(parent).map_err(|err| {
                format!(
                    "failed to prepare snapshot directory {}: {}",
                    parent.display(),
                    err
                )
            })?;
        }
    }

    fs::write(path, data)
        .map_err(|err| format!("failed to write snapshot file {}: {}", path.display(), err))
}

fn save_snapshot_metadata(
    path: &Path,
    metadata: &SnapshotMetadata,
) -> std::result::Result<(), String> {
    let bytes = snapshot_metadata_bytes(metadata)?;
    write_binary_file(path, &bytes)
}

fn snapshot_metadata_bytes(metadata: &SnapshotMetadata) -> std::result::Result<Vec<u8>, String> {
    serde_json::to_vec_pretty(metadata)
        .map_err(|err| format!("failed to serialize snapshot metadata: {}", err))
}

fn save_encrypted_snapshot_files(
    image_path: &Path,
    metadata_path: &Path,
    image: &RgbaImage,
    metadata: &SnapshotMetadata,
    key_hex: &str,
) -> std::result::Result<(), String> {
    let encoded_image = encode_snapshot_image(image)?;
    let encrypted_image = encrypt_snapshot_bytes(key_hex, &encoded_image)?;
    write_binary_file(image_path, &encrypted_image)?;

    let metadata_bytes = snapshot_metadata_bytes(metadata)?;
    let encrypted_metadata = encrypt_snapshot_metadata(key_hex, &metadata_bytes)?;
    write_binary_file(metadata_path, &encrypted_metadata)
}

#[derive(Debug)]
struct SnapshotRecord {
    sort_key: DateTime<Utc>,
    item: SnapshotListItem,
}

fn stringify_path_for_frontend(path: &Path) -> String {
    #[cfg(windows)]
    {
        return normalize_windows_path_for_frontend(path);
    }

    #[cfg(not(windows))]
    {
        return path.to_string_lossy().to_string();
    }
}

#[cfg(windows)]
fn normalize_windows_path_for_frontend(path: &Path) -> String {
    let raw = path.to_string_lossy().into_owned();

    if let Some(stripped) = raw.strip_prefix(r"\\?\\UNC\\") {
        return format!(r"\\\\{}", stripped);
    }
    if let Some(stripped) = raw.strip_prefix(r"\\?\\") {
        return stripped.to_string();
    }
    if let Some(stripped) = raw.strip_prefix(r"//?/UNC/") {
        return format!(r"//{}", stripped);
    }
    if let Some(stripped) = raw.strip_prefix(r"//?/") {
        return stripped.to_string();
    }

    raw
}

fn collect_snapshots_from_dir(
    root: &Path,
    module_id: &str,
    camera_filter: Option<&str>,
    records: &mut Vec<SnapshotRecord>,
) {
    if !root.exists() {
        return;
    }

    let mut pending: Vec<PathBuf> = vec![root.to_path_buf()];
    while let Some(current_dir) = pending.pop() {
        let entries = match fs::read_dir(&current_dir) {
            Ok(entries) => entries,
            Err(err) => {
                println!(
                    "analytics {}: failed to read snapshot directory {}: {}",
                    module_id,
                    current_dir.display(),
                    err
                );
                continue;
            }
        };

        for entry in entries {
            let entry = match entry {
                Ok(value) => value,
                Err(err) => {
                    println!("analytics {}: failed to read dir entry: {}", module_id, err);
                    continue;
                }
            };

            let path = entry.path();
            let file_type = match entry.file_type() {
                Ok(kind) => kind,
                Err(err) => {
                    println!(
                        "analytics {}: failed to resolve file type for {}: {}",
                        module_id,
                        entry.path().display(),
                        err
                    );
                    continue;
                }
            };

            if file_type.is_dir() {
                pending.push(path);
                continue;
            }

            let Some(ext) = path.extension().and_then(|ext| ext.to_str()) else {
                continue;
            };

            if !ext.eq_ignore_ascii_case("json") {
                continue;
            }

            let content = match fs::read_to_string(&path) {
                Ok(data) => data,
                Err(err) => {
                    println!(
                        "analytics {}: failed to read snapshot metadata {}: {}",
                        module_id,
                        path.display(),
                        err
                    );
                    continue;
                }
            };

            let metadata: SnapshotMetadata = match serde_json::from_str(&content) {
                Ok(value) => value,
                Err(err) => {
                    println!(
                        "analytics {}: failed to parse snapshot metadata {}: {}",
                        module_id,
                        path.display(),
                        err
                    );
                    continue;
                }
            };

            if let Some(filter) = camera_filter {
                match metadata.camera_id.as_deref() {
                    Some(camera_id) if camera_id == filter => {}
                    _ => continue,
                }
            }

            let folder_path = path.parent().unwrap_or(&current_dir).to_path_buf();
            let image_path = folder_path.join(&metadata.image_file);
            let image_available = image_path.exists();
            let image_size = fs::metadata(&image_path).ok().map(|meta| meta.len());
            let metadata_size = fs::metadata(&path).ok().map(|meta| meta.len());
            let sort_key = parse_captured_at_timestamp(&metadata.captured_at);
            let metadata_file = path
                .file_name()
                .and_then(|value| value.to_str())
                .unwrap_or_default()
                .to_string();
            let folder_path_str = stringify_path_for_frontend(&folder_path);
            let image_path_str = stringify_path_for_frontend(&image_path);
            let metadata_path_str = stringify_path_for_frontend(&path);
            let encrypted = metadata.image_file.ends_with(".enc");

            let item = SnapshotListItem {
                id: format!("{}::{}", module_id, metadata_path_str),
                module_id: module_id.to_string(),
                camera_id: metadata.camera_id.clone(),
                detection_id: metadata.detection_id.clone(),
                captured_at: metadata.captured_at.clone(),
                confidence: metadata.confidence,
                bounds: metadata.bounds.clone(),
                frame_width: metadata.frame_width,
                frame_height: metadata.frame_height,
                image_file: metadata.image_file.clone(),
                metadata_file,
                folder_path: folder_path_str,
                image_path: image_path_str,
                metadata_path: metadata_path_str,
                image_size,
                metadata_size,
                image_available,
                encrypted,
            };

            records.push(SnapshotRecord { sort_key, item });
        }
    }
}

fn parse_captured_at_timestamp(value: &str) -> DateTime<Utc> {
    DateTime::parse_from_rfc3339(value)
        .map(|dt| dt.with_timezone(&Utc))
        .unwrap_or_else(|_| Utc.timestamp_millis_opt(0).unwrap())
}

fn persist_manifest(root_dir: &Path, module_id: &str, manifest: &ModuleManifest) -> Result<()> {
    let module_dir = root_dir.join(module_id);
    fs::create_dir_all(&module_dir)?;
    let manifest_path = module_dir.join(MANIFEST_FILE);
    let content = serde_json::to_vec_pretty(manifest)?;
    fs::File::create(&manifest_path)?.write_all(&content)?;
    Ok(())
}

const FACE_DETECTOR_MODEL_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://raw.githubusercontent.com/Rinibr25/Face-Detector-Module-for-Dashboard-/main/yolov11n-face.onnx",
        file_name: FACE_DETECTOR_MODEL_FILE,
        sha256: None,
    },
);

const LICENSE_PLATE_DETECTOR_MODEL_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/Rinibr25/License-Plate-Detector-for-Dashboard/releases/download/v0.1.0/anpr_yolov8.onnx",
        file_name: LICENSE_PLATE_DETECTOR_MODEL_FILE,
        sha256: None,
    },
);

const LICENSE_PLATE_OCR_MODEL_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/Rinibr25/License-Plate-Detector-for-Dashboard/releases/download/v0.1.0/anpr_crnn.onnx",
        file_name: LICENSE_PLATE_OCR_MODEL_FILE,
        sha256: None,
    },
);

// Python PyTorch model for ANPR OCR (used with Python script)
const LICENSE_PLATE_PYTHON_MODEL_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/Rinibr25/License-Plate-Detector-for-Dashboard/releases/download/v0.1.0/crnn_ocr_model_best.pth",
        file_name: "crnn_ocr_model_best.pth",
        sha256: None,
    },
);

// Standalone ANPR OCR executable (Windows)
const LICENSE_PLATE_ANPR_EXE_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/Rinibr25/License-Plate-Detector-for-Dashboard/releases/download/v0.1.0/anpr_ocr.exe",
        file_name: "anpr_ocr.exe",
        sha256: None,
    },
);

const OBJECT_COUNTER_MODEL_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(
    ModuleDownloadSpec {
        url: "https://github.com/Rinibr25/Object-Counter-for-Dashboard/releases/download/v0.1.0/yolo11s.onnx",
        file_name: OBJECT_COUNTER_MODEL_FILE,
        sha256: None,
    },
);

#[cfg(target_os = "windows")]
const ONNX_RUNTIME_DLL_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(ModuleDownloadSpec {
    url: "https://github.com/Rinibr25/License-Plate-Detector-for-Dashboard/releases/download/v0.1.0/onnxruntime.dll",
    file_name: "runtime/onnxruntime.dll",
    sha256: Some("f5131591edac6b0a8090d0e329040a49319d7a689cb5b465235fbf7030fa8027"),
});
#[cfg(target_os = "windows")]
const ONNX_RUNTIME_SHARED_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::File(ModuleDownloadSpec {
    url: "https://github.com/Rinibr25/License-Plate-Detector-for-Dashboard/releases/download/v0.1.0/onnxruntime_providers_shared.dll",
    file_name: "runtime/onnxruntime_providers_shared.dll",
    sha256: Some("3b27e1417d12b73a6a34d80414c083e359e092d2f0ce572d7e67be8cdbe9e825"),
});
#[cfg(target_os = "windows")]
const FACE_DETECTOR_RESOURCES: &[ModuleResourceSpec] = &[
    FACE_DETECTOR_MODEL_RESOURCE,
    ONNX_RUNTIME_DLL_RESOURCE,
    ONNX_RUNTIME_SHARED_RESOURCE,
];
#[cfg(target_os = "windows")]
const OBJECT_COUNTER_RESOURCES: &[ModuleResourceSpec] = &[
    OBJECT_COUNTER_MODEL_RESOURCE,
    ONNX_RUNTIME_DLL_RESOURCE,
    ONNX_RUNTIME_SHARED_RESOURCE,
];
#[cfg(target_os = "windows")]
const LICENSE_PLATE_RESOURCES: &[ModuleResourceSpec] = &[
    LICENSE_PLATE_DETECTOR_MODEL_RESOURCE,
    LICENSE_PLATE_OCR_MODEL_RESOURCE,
    LICENSE_PLATE_ANPR_EXE_RESOURCE,
    LICENSE_PLATE_PYTHON_MODEL_RESOURCE,
    ONNX_RUNTIME_DLL_RESOURCE,
    ONNX_RUNTIME_SHARED_RESOURCE,
];

#[cfg(target_os = "linux")]
const ONNX_RUNTIME_LINUX_ARCHIVE: &str =
    concat!("onnxruntime-linux-x64-", onnx_runtime_version!(), ".tgz");
#[cfg(target_os = "linux")]
const ONNX_RUNTIME_LINUX_URL: &str = concat!(
    "https://github.com/microsoft/onnxruntime/releases/download/v",
    onnx_runtime_version!(),
    "/onnxruntime-linux-x64-",
    onnx_runtime_version!(),
    ".tgz"
);
#[cfg(target_os = "linux")]
const ONNX_RUNTIME_LINUX_STRIP_PREFIX: &str =
    concat!("onnxruntime-linux-x64-", onnx_runtime_version!(), "/lib");
#[cfg(target_os = "linux")]
const FACE_DETECTOR_LINUX_REQUIRED: &str =
    concat!("runtime/libonnxruntime.so.", onnx_runtime_version!());
#[cfg(target_os = "linux")]
const FACE_DETECTOR_RUNTIME_REQUIRED: &[&str] =
    &[FACE_DETECTOR_LINUX_REQUIRED, ONNX_RUNTIME_LINUX_ARCHIVE];
#[cfg(target_os = "linux")]
const ONNX_RUNTIME_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::Archive(ModuleArchiveSpec {
    url: ONNX_RUNTIME_LINUX_URL,
    file_name: ONNX_RUNTIME_LINUX_ARCHIVE,
    sha256: None,
    archive_type: ArchiveType::TarGz,
    strip_prefix: Some(ONNX_RUNTIME_LINUX_STRIP_PREFIX),
    target_dir: "runtime",
    required_files: FACE_DETECTOR_RUNTIME_REQUIRED,
});
#[cfg(target_os = "linux")]
const FACE_DETECTOR_RESOURCES: &[ModuleResourceSpec] =
    &[FACE_DETECTOR_MODEL_RESOURCE, ONNX_RUNTIME_RESOURCE];
#[cfg(target_os = "linux")]
const OBJECT_COUNTER_RESOURCES: &[ModuleResourceSpec] =
    &[OBJECT_COUNTER_MODEL_RESOURCE, ONNX_RUNTIME_RESOURCE];
#[cfg(target_os = "linux")]
const LICENSE_PLATE_RESOURCES: &[ModuleResourceSpec] = &[
    LICENSE_PLATE_DETECTOR_MODEL_RESOURCE,
    LICENSE_PLATE_OCR_MODEL_RESOURCE,
    ONNX_RUNTIME_RESOURCE,
];

#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const ONNX_RUNTIME_MAC_ARM_ARCHIVE: &str =
    concat!("onnxruntime-osx-arm64-", onnx_runtime_version!(), ".tgz");
#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const ONNX_RUNTIME_MAC_ARM_URL: &str = concat!(
    "https://github.com/microsoft/onnxruntime/releases/download/v",
    onnx_runtime_version!(),
    "/onnxruntime-osx-arm64-",
    onnx_runtime_version!(),
    ".tgz"
);
#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const ONNX_RUNTIME_MAC_ARM_STRIP_PREFIX: &str =
    concat!("onnxruntime-osx-arm64-", onnx_runtime_version!(), "/lib");
#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const FACE_DETECTOR_MAC_ARM_REQUIRED: &str =
    concat!("runtime/libonnxruntime.", onnx_runtime_version!(), ".dylib");
#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const FACE_DETECTOR_RUNTIME_REQUIRED: &[&str] =
    &[FACE_DETECTOR_MAC_ARM_REQUIRED, ONNX_RUNTIME_MAC_ARM_ARCHIVE];
#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const ONNX_RUNTIME_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::Archive(ModuleArchiveSpec {
    url: ONNX_RUNTIME_MAC_ARM_URL,
    file_name: ONNX_RUNTIME_MAC_ARM_ARCHIVE,
    sha256: None,
    archive_type: ArchiveType::TarGz,
    strip_prefix: Some(ONNX_RUNTIME_MAC_ARM_STRIP_PREFIX),
    target_dir: "runtime",
    required_files: FACE_DETECTOR_RUNTIME_REQUIRED,
});
#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const FACE_DETECTOR_RESOURCES: &[ModuleResourceSpec] =
    &[FACE_DETECTOR_MODEL_RESOURCE, ONNX_RUNTIME_RESOURCE];
#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const OBJECT_COUNTER_RESOURCES: &[ModuleResourceSpec] =
    &[OBJECT_COUNTER_MODEL_RESOURCE, ONNX_RUNTIME_RESOURCE];
#[cfg(all(target_os = "macos", target_arch = "aarch64"))]
const LICENSE_PLATE_RESOURCES: &[ModuleResourceSpec] = &[
    LICENSE_PLATE_DETECTOR_MODEL_RESOURCE,
    LICENSE_PLATE_OCR_MODEL_RESOURCE,
    ONNX_RUNTIME_RESOURCE,
];

#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const ONNX_RUNTIME_MAC_X64_ARCHIVE: &str =
    concat!("onnxruntime-osx-x64-", onnx_runtime_version!(), ".tgz");
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const ONNX_RUNTIME_MAC_X64_URL: &str = concat!(
    "https://github.com/microsoft/onnxruntime/releases/download/v",
    onnx_runtime_version!(),
    "/onnxruntime-osx-x64-",
    onnx_runtime_version!(),
    ".tgz"
);
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const ONNX_RUNTIME_MAC_X64_STRIP_PREFIX: &str =
    concat!("onnxruntime-osx-x64-", onnx_runtime_version!(), "/lib");
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const FACE_DETECTOR_MAC_X64_REQUIRED: &str =
    concat!("runtime/libonnxruntime.", onnx_runtime_version!(), ".dylib");
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const FACE_DETECTOR_RUNTIME_REQUIRED: &[&str] =
    &[FACE_DETECTOR_MAC_X64_REQUIRED, ONNX_RUNTIME_MAC_X64_ARCHIVE];
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const ONNX_RUNTIME_RESOURCE: ModuleResourceSpec = ModuleResourceSpec::Archive(ModuleArchiveSpec {
    url: ONNX_RUNTIME_MAC_X64_URL,
    file_name: ONNX_RUNTIME_MAC_X64_ARCHIVE,
    sha256: None,
    archive_type: ArchiveType::TarGz,
    strip_prefix: Some(ONNX_RUNTIME_MAC_X64_STRIP_PREFIX),
    target_dir: "runtime",
    required_files: FACE_DETECTOR_RUNTIME_REQUIRED,
});
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const FACE_DETECTOR_RESOURCES: &[ModuleResourceSpec] =
    &[FACE_DETECTOR_MODEL_RESOURCE, ONNX_RUNTIME_RESOURCE];
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const LICENSE_PLATE_RESOURCES: &[ModuleResourceSpec] = &[
    LICENSE_PLATE_DETECTOR_MODEL_RESOURCE,
    LICENSE_PLATE_OCR_MODEL_RESOURCE,
    ONNX_RUNTIME_RESOURCE,
];
#[cfg(all(target_os = "macos", target_arch = "x86_64"))]
const OBJECT_COUNTER_RESOURCES: &[ModuleResourceSpec] =
    &[OBJECT_COUNTER_MODEL_RESOURCE, ONNX_RUNTIME_RESOURCE];

#[cfg(all(
    target_os = "macos",
    not(any(target_arch = "aarch64", target_arch = "x86_64"))
))]
const FACE_DETECTOR_RESOURCES: &[ModuleResourceSpec] = &[FACE_DETECTOR_MODEL_RESOURCE];
#[cfg(all(
    target_os = "macos",
    not(any(target_arch = "aarch64", target_arch = "x86_64"))
))]
const OBJECT_COUNTER_RESOURCES: &[ModuleResourceSpec] = &[OBJECT_COUNTER_MODEL_RESOURCE];
#[cfg(all(
    target_os = "macos",
    not(any(target_arch = "aarch64", target_arch = "x86_64"))
))]
const LICENSE_PLATE_RESOURCES: &[ModuleResourceSpec] = &[
    LICENSE_PLATE_DETECTOR_MODEL_RESOURCE,
    LICENSE_PLATE_OCR_MODEL_RESOURCE,
];

#[cfg(not(any(target_os = "windows", target_os = "linux", target_os = "macos")))]
const FACE_DETECTOR_RESOURCES: &[ModuleResourceSpec] = &[FACE_DETECTOR_MODEL_RESOURCE];
#[cfg(not(any(target_os = "windows", target_os = "linux", target_os = "macos")))]
const OBJECT_COUNTER_RESOURCES: &[ModuleResourceSpec] = &[OBJECT_COUNTER_MODEL_RESOURCE];
#[cfg(not(any(target_os = "windows", target_os = "linux", target_os = "macos")))]
const LICENSE_PLATE_RESOURCES: &[ModuleResourceSpec] = &[
    LICENSE_PLATE_DETECTOR_MODEL_RESOURCE,
    LICENSE_PLATE_OCR_MODEL_RESOURCE,
];

fn builtin_modules() -> Vec<ModuleDescriptor> {
    vec![
        ModuleDescriptor {
            id: AnalyticsModuleId::FaceDetector,
            name: "Face Detector",
            version: "1.0.0",
            description: "Detect faces and draw bounding boxes over video streams.",
            resources: FACE_DETECTOR_RESOURCES,
            builder: face_detector_builder,
            min_inference_interval: Duration::from_millis(250),
        },
        ModuleDescriptor {
            id: AnalyticsModuleId::LicensePlateDetector,
            name: "License Plate Detector",
            version: "1.0.0",
            description: "Detect vehicle license plates and recognize their text.",
            resources: LICENSE_PLATE_RESOURCES,
            builder: license_plate_builder,
            min_inference_interval: Duration::from_millis(350),
        },
        ModuleDescriptor {
            id: AnalyticsModuleId::ObjectCounter,
            name: "Object Counter",
            version: "1.0.0",
            description: "Detect and count people, vehicles, and other common objects.",
            resources: OBJECT_COUNTER_RESOURCES,
            builder: object_counter_builder,
            min_inference_interval: Duration::from_millis(200),
        },
    ]
}

fn face_detector_builder(
    module_dir: &Path,
) -> std::result::Result<Arc<dyn AnalyticsEngine>, String> {
    let provider_pref = current_execution_provider_preference();
    println!(
        "analytics face-detector: provider preference '{}'",
        provider_pref.label()
    );
    let default_color = FACE_COLOR_PALETTE.first().copied().unwrap_or("#ff7f50");
    let options = YoloDetectorOptions {
        model_file: FACE_DETECTOR_MODEL_FILE,
        class_labels: FACE_CLASS_LABELS,
        color_palette: FACE_COLOR_PALETTE,
        default_color,
    };
    YoloDetector::new(module_dir, provider_pref, options)
        .map(|engine| Arc::new(engine) as Arc<dyn AnalyticsEngine>)
}

fn license_plate_builder(
    module_dir: &Path,
) -> std::result::Result<Arc<dyn AnalyticsEngine>, String> {
    let provider_pref = current_execution_provider_preference();
    println!(
        "analytics license-plate: provider preference '{}'",
        provider_pref.label()
    );
    license_plate::LicensePlateEngine::new(module_dir, provider_pref)
        .map(|engine| Arc::new(engine) as Arc<dyn AnalyticsEngine>)
}

fn object_counter_builder(
    module_dir: &Path,
) -> std::result::Result<Arc<dyn AnalyticsEngine>, String> {
    let provider_pref = current_execution_provider_preference();
    println!(
        "analytics object-counter: provider preference '{}'",
        provider_pref.label()
    );
    let default_color = COCO_COLOR_PALETTE.first().copied().unwrap_or("#2563eb");
    let options = YoloDetectorOptions {
        model_file: OBJECT_COUNTER_MODEL_FILE,
        class_labels: COCO_CLASS_LABELS,
        color_palette: COCO_COLOR_PALETTE,
        default_color,
    };
    YoloDetector::new(module_dir, provider_pref, options)
        .map(|engine| Arc::new(engine) as Arc<dyn AnalyticsEngine>)
}

#[allow(dead_code)]
fn not_implemented_builder(_: &Path) -> std::result::Result<Arc<dyn AnalyticsEngine>, String> {
    Err("analytics engine not yet implemented".to_string())
}

#[tauri::command]
pub fn analytics_list_modules(
    state: State<'_, AnalyticsState>,
) -> std::result::Result<Vec<ModuleStatus>, String> {
    Ok(state.list_status())
}

#[tauri::command]
pub fn analytics_list_snapshots(
    state: State<'_, AnalyticsState>,
    payload: Option<SnapshotListRequest>,
) -> std::result::Result<SnapshotListResponse, String> {
    let params = payload.unwrap_or_default();
    state
        .list_snapshots(
            params.module_id.as_deref(),
            params.camera_id.as_deref(),
            params.limit(),
            params.offset(),
        )
        .map_err(|err| err.to_string())
}

#[tauri::command]
pub fn analytics_enable_module(
    state: State<'_, AnalyticsState>,
    module_id: String,
) -> std::result::Result<(), String> {
    state
        .enable_module(&module_id)
        .map_err(|err| err.to_string())
}

#[tauri::command]
pub fn analytics_disable_module(
    state: State<'_, AnalyticsState>,
    module_id: String,
) -> std::result::Result<(), String> {
    state
        .disable_module(&module_id)
        .map_err(|err| err.to_string())
}

#[derive(Debug, Deserialize)]
pub struct UpdateModuleConfigPayload {
    module_id: String,
    #[serde(default)]
    snapshots_dir: Option<Option<String>>,
    #[serde(default)]
    face_snapshots_mode: Option<String>,
    #[serde(default)]
    face_snapshot_key_hex: Option<String>,
    #[serde(default)]
    reset_face_snapshot_key: Option<bool>,
}

#[derive(Debug, Default)]
pub struct ModuleConfigUpdateRequest {
    pub snapshots_dir: Option<Option<String>>,
    pub face_snapshots_mode: Option<FaceSnapshotMode>,
    pub face_snapshot_key_hex: Option<String>,
    pub reset_face_snapshot_key: bool,
}

#[tauri::command]
pub fn analytics_update_module_config(
    state: State<'_, AnalyticsState>,
    payload: UpdateModuleConfigPayload,
) -> std::result::Result<ModuleStatus, String> {
    let parsed_mode = payload
        .face_snapshots_mode
        .as_ref()
        .map(|raw| FaceSnapshotMode::from_str(raw))
        .transpose()
        .map_err(|err| err.to_string())?;

    let update = ModuleConfigUpdateRequest {
        snapshots_dir: payload.snapshots_dir,
        face_snapshots_mode: parsed_mode,
        face_snapshot_key_hex: payload.face_snapshot_key_hex,
        reset_face_snapshot_key: payload.reset_face_snapshot_key.unwrap_or(false),
    };

    state
        .update_module_config(&payload.module_id, update)
        .map_err(|err| err.to_string())
}

#[derive(Debug, Deserialize)]
pub struct ProcessFramePayload {
    module_id: String,
    camera_id: Option<String>,
    frame_base64: String,
    frame_width: u32,
    frame_height: u32,
    options: Option<Value>,
}

#[tauri::command]
pub fn analytics_process_frame(
    state: State<'_, AnalyticsState>,
    payload: ProcessFramePayload,
) -> std::result::Result<DetectionResponse, String> {
    state
        .process_frame(
            &payload.module_id,
            payload.camera_id,
            payload.frame_width,
            payload.frame_height,
            &payload.frame_base64,
            payload.options,
        )
        .map_err(|err| err.to_string())
}

pub fn prepare_analytics_manager(app: &AppHandle) -> Result<AnalyticsState> {
    let manager = AnalyticsState::new(app)?;
    {
        let statuses = manager.list_status();
        for status in statuses {
            if status.enabled && status.state != "ready" {
                let _ = manager.enable_module(&status.id);
            }
        }
    }
    Ok(manager)
}
