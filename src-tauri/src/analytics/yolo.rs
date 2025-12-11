use std::cmp::Ordering;
use std::convert::TryFrom;
use std::path::{Path, PathBuf};
use std::result::Result as StdResult;
use std::sync::Mutex;

use serde_json::Value;
use image::{imageops::FilterType, DynamicImage, GenericImageView};
use ndarray::{s, Array2, Array4, ArrayD, Ix2, IxDyn};
use once_cell::sync::OnceCell;
use ort::{
    environment::{self, Environment},
    execution_providers::{
        cpu::CPUExecutionProvider, ExecutionProvider, ExecutionProviderDispatch,
    },
    logging::LogLevel,
    session::builder::GraphOptimizationLevel,
    session::Session,
    value::{DynValue, Tensor, ValueType},
    Error as OrtError,
};

#[cfg(target_os = "windows")]
use ort::execution_providers::directml::DirectMLExecutionProvider;

use super::{AnalyticsEngine, BoundingBox, DetectionBox, ExecutionProviderPreference};

const YOLO_CONFIDENCE_THRESHOLD: f32 = 0.30;
const YOLO_NMS_THRESHOLD: f32 = 0.35;
const YOLO_MAX_DETECTIONS: usize = 300;

#[cfg(target_os = "windows")]
#[allow(dead_code)]
fn find_discrete_gpu_device_id() -> i32 {
    // Try device IDs 0-3 and pick the first one that works
    // DirectML enumerates adapters with discrete GPUs typically having lower IDs
    // In most systems: 0 = discrete GPU, 1 = integrated GPU
    // We'll try 0 as default, which should be the high-performance adapter
    0
}
pub(super) const FACE_CLASS_LABELS: &[&str] = &["Face"];
pub(super) const FACE_COLOR_PALETTE: &[&str] = &["#ff7f50"];

pub(super) const COCO_CLASS_LABELS: &[&str] = &[
    "person",
    "bicycle",
    "car",
    "motorcycle",
    "airplane",
    "bus",
    "train",
    "truck",
    "boat",
    "traffic light",
    "fire hydrant",
    "stop sign",
    "parking meter",
    "bench",
    "bird",
    "cat",
    "dog",
    "horse",
    "sheep",
    "cow",
    "elephant",
    "bear",
    "zebra",
    "giraffe",
    "backpack",
    "umbrella",
    "handbag",
    "tie",
    "suitcase",
    "frisbee",
    "skis",
    "snowboard",
    "sports ball",
    "kite",
    "baseball bat",
    "baseball glove",
    "skateboard",
    "surfboard",
    "tennis racket",
    "bottle",
    "wine glass",
    "cup",
    "fork",
    "knife",
    "spoon",
    "bowl",
    "banana",
    "apple",
    "sandwich",
    "orange",
    "broccoli",
    "carrot",
    "hot dog",
    "pizza",
    "donut",
    "cake",
    "chair",
    "couch",
    "potted plant",
    "bed",
    "dining table",
    "toilet",
    "tv",
    "laptop",
    "mouse",
    "remote",
    "keyboard",
    "cell phone",
    "microwave",
    "oven",
    "toaster",
    "sink",
    "refrigerator",
    "book",
    "clock",
    "vase",
    "scissors",
    "teddy bear",
    "hair drier",
    "toothbrush",
];

pub(super) const COCO_COLOR_PALETTE: &[&str] = &[
    "#2563eb", "#10b981", "#f59e0b", "#ef4444", "#8b5cf6", "#ec4899", "#14b8a6", "#6366f1",
    "#f97316", "#22c55e", "#a855f7", "#f43f5e", "#06b6d4", "#0ea5e9", "#84cc16", "#d946ef",
    "#9333ea", "#facc15", "#60a5fa", "#f87171",
];

#[derive(Clone, Copy)]
pub struct YoloDetectorOptions {
    pub model_file: &'static str,
    pub class_labels: &'static [&'static str],
    pub color_palette: &'static [&'static str],
    pub default_color: &'static str,
}

pub struct YoloDetector {
    session: Mutex<Session>,
    input_width: usize,
    input_height: usize,
    options: YoloDetectorOptions,
}

struct LetterboxInfo {
    scale: f32,
    pad_x: f32,
    pad_y: f32,
    orig_width: u32,
    orig_height: u32,
}

#[derive(Clone)]
struct CandidateBox {
    x1: f32,
    y1: f32,
    x2: f32,
    y2: f32,
    score: f32,
    class_idx: usize,
}

static ORT_ENVIRONMENT: OnceCell<&'static Environment> = OnceCell::new();

impl YoloDetector {
    pub fn new(
        module_dir: &Path,
        provider_pref: ExecutionProviderPreference,
        options: YoloDetectorOptions,
    ) -> StdResult<Self, String> {
        let model_path = module_dir.join(options.model_file);
        if !model_path.exists() {
            return Err(format!(
                "YOLO detector model not found at {}",
                model_path.display()
            ));
        }

        println!(
            "YOLO init: module_dir={} model_path={}",
            module_dir.display(),
            model_path.display()
        );

        let _environment = get_environment(module_dir)?;
        let (execution_providers, provider_labels) = build_execution_provider_chain(provider_pref);
        println!(
            "YOLO init: provider preference='{}' execution chain={}",
            provider_pref.label(),
            describe_provider_chain(&provider_labels)
        );

        let builder = Session::builder()
            .map_err(|err| format!("Failed to create ONNX session builder: {err}"))?
            .with_optimization_level(GraphOptimizationLevel::Level3)
            .map_err(|err| format!("Failed to configure ONNX optimization level: {err}"))?;

        let builder = builder
            .with_execution_providers(execution_providers.as_slice())
            .map_err(|err| {
                format!(
                    "Failed to configure ONNX execution providers for preference '{}': {err}",
                    provider_pref.label()
                )
            })?;

        let session = builder
            .commit_from_file(&model_path)
            .map_err(|err| format!("Failed to load YOLO model: {err}"))?;

        let (input_width, input_height) = resolve_input_shape(&session)?;
        println!(
            "YOLO init: resolved input_width={} input_height={} classes={}",
            input_width,
            input_height,
            options.class_labels.len()
        );

        Ok(Self {
            session: Mutex::new(session),
            input_width,
            input_height,
            options,
        })
    }

    fn prepare_input(&self, frame: &DynamicImage) -> (Array4<f32>, LetterboxInfo) {
        let (orig_width, orig_height) = frame.dimensions();
        let input_width = self.input_width.max(1) as u32;
        let input_height = self.input_height.max(1) as u32;

        if orig_width == 0 || orig_height == 0 {
            return (
                Array4::zeros((1, 3, input_height as usize, input_width as usize)),
                LetterboxInfo {
                    scale: 1.0,
                    pad_x: 0.0,
                    pad_y: 0.0,
                    orig_width,
                    orig_height,
                },
            );
        }

        let scale = f32::min(
            input_width as f32 / orig_width as f32,
            input_height as f32 / orig_height as f32,
        );
        let resized_width = (orig_width as f32 * scale).round().max(1.0) as u32;
        let resized_height = (orig_height as f32 * scale).round().max(1.0) as u32;

        let pad_w = input_width as i32 - resized_width as i32;
        let pad_h = input_height as i32 - resized_height as i32;
        let pad_left = (pad_w / 2).max(0) as u32;
        let pad_top = (pad_h / 2).max(0) as u32;

        let resized = frame
            .resize_exact(resized_width, resized_height, FilterType::Triangle)
            .to_rgb8();

        let mut tensor = Array4::<f32>::zeros((1, 3, input_height as usize, input_width as usize));

        for y in 0..resized_height {
            let dst_y = y + pad_top;
            if dst_y >= input_height {
                continue;
            }
            for x in 0..resized_width {
                let dst_x = x + pad_left;
                if dst_x >= input_width {
                    continue;
                }
                let pixel = resized.get_pixel(x, y);
                let nx = dst_x as usize;
                let ny = dst_y as usize;
                tensor[[0, 0, ny, nx]] = pixel[0] as f32 / 255.0;
                tensor[[0, 1, ny, nx]] = pixel[1] as f32 / 255.0;
                tensor[[0, 2, ny, nx]] = pixel[2] as f32 / 255.0;
            }
        }

        (
            tensor,
            LetterboxInfo {
                scale: scale.max(f32::EPSILON),
                pad_x: pad_left as f32,
                pad_y: pad_top as f32,
                orig_width,
                orig_height,
            },
        )
    }

    fn run_inference(&self, input: Array4<f32>) -> StdResult<Array2<f32>, String> {
        let dims = input.shape();
        let shape = [dims[0], dims[1], dims[2], dims[3]];
        let tensor_data = input.into_raw_vec();
        let tensor = Tensor::from_array((shape, tensor_data))
            .map_err(|err| format!("Failed to build YOLO input tensor: {err}"))?;

        let output_value = {
            let mut session = self
                .session
                .lock()
                .map_err(|err| format!("Failed to acquire session lock: {err}"))?;

            let outputs = session
                .run(ort::inputs![tensor])
                .map_err(|err| format!("YOLO inference failed: {err}"))?;

            let maybe_value = outputs.into_iter().next().map(|(_, value)| value);

            maybe_value.ok_or_else(|| {
                "YOLO model produced no outputs. Ensure the model exports detection tensors."
                    .to_string()
            })?
        };

        extract_tensor(&output_value, "output")
    }

    fn postprocess(&self, predictions: Array2<f32>, meta: &LetterboxInfo, threshold: f32) -> Vec<DetectionBox> {
        if predictions.ncols() < 5 {
            println!(
                "YOLO postprocess: insufficient feature columns (expected >=5, got {})",
                predictions.ncols()
            );
            return Vec::new();
        }

        let mut candidates: Vec<CandidateBox> = Vec::new();

        for row in predictions.rows() {
            let (score, class_idx) = if predictions.ncols() > 4 {
                row.slice(s![4..])
                    .iter()
                    .enumerate()
                    .map(|(idx, raw)| (raw.clamp(0.0, 1.0), idx))
                    .max_by(|a, b| a.0.partial_cmp(&b.0).unwrap_or(Ordering::Equal))
                    .unwrap_or((0.0, 0usize))
            } else {
                (1.0f32, 0usize)
            };

            if !score.is_finite() || score < threshold {
                continue;
            }

            let cx = row[0];
            let cy = row[1];
            let width = row[2];
            let height = row[3];
            if width <= 0.0 || height <= 0.0 {
                continue;
            }

            let mut x1 = cx - width / 2.0;
            let mut y1 = cy - height / 2.0;
            let mut x2 = cx + width / 2.0;
            let mut y2 = cy + height / 2.0;

            x1 = (x1 - meta.pad_x) / meta.scale;
            y1 = (y1 - meta.pad_y) / meta.scale;
            x2 = (x2 - meta.pad_x) / meta.scale;
            y2 = (y2 - meta.pad_y) / meta.scale;

            x1 = x1.clamp(0.0, meta.orig_width as f32);
            y1 = y1.clamp(0.0, meta.orig_height as f32);
            x2 = x2.clamp(0.0, meta.orig_width as f32);
            y2 = y2.clamp(0.0, meta.orig_height as f32);

            if x2 <= x1 || y2 <= y1 {
                continue;
            }

            candidates.push(CandidateBox {
                x1,
                y1,
                x2,
                y2,
                score,
                class_idx,
            });
        }

        if candidates.is_empty() {
            return Vec::new();
        }

        let suppressed = non_max_suppression(candidates, YOLO_NMS_THRESHOLD);
        suppressed
            .into_iter()
            .enumerate()
            .map(|(idx, candidate)| DetectionBox {
                id: format!(
                    "{}-{}",
                    sanitize_label_for_id(self.label_for(candidate.class_idx)),
                    idx
                ),
                label: self.label_for(candidate.class_idx).to_string(),
                confidence: candidate.score.clamp(0.0, 1.0),
                bounds: BoundingBox {
                    x: candidate.x1,
                    y: candidate.y1,
                    width: candidate.x2 - candidate.x1,
                    height: candidate.y2 - candidate.y1,
                },
                color: self.color_for(candidate.class_idx).to_string(),
                track_id: None,
                previous_bounds: None,
                dwell_ms: None,
                first_seen_at: None,
                last_seen_at: None,
                event_type: None,
                zone: None,
            })
            .collect()
    }

    fn label_for(&self, class_idx: usize) -> &str {
        self.options
            .class_labels
            .get(class_idx)
            .copied()
            .unwrap_or("Object")
    }

    fn color_for(&self, class_idx: usize) -> &str {
        if self.options.color_palette.is_empty() {
            return self.options.default_color;
        }

        self.options
            .color_palette
            .get(class_idx % self.options.color_palette.len())
            .copied()
            .unwrap_or(self.options.default_color)
    }
}

impl AnalyticsEngine for YoloDetector {
    fn process(&self, frame: &DynamicImage, options: &Value) -> StdResult<Vec<DetectionBox>, String> {
        let (input_tensor, meta) = self.prepare_input(frame);
        let predictions = self.run_inference(input_tensor)?;
        
        let threshold = options
            .get("confidence_threshold")
            .and_then(|v| v.as_f64())
            .map(|v| v as f32)
            .unwrap_or(YOLO_CONFIDENCE_THRESHOLD);

        Ok(self.postprocess(predictions, &meta, threshold))
    }
}

fn sanitize_label_for_id(label: &str) -> String {
    let mut result = String::with_capacity(label.len());
    let mut last_dash = false;

    for ch in label.chars() {
        if ch.is_ascii_alphanumeric() {
            result.push(ch.to_ascii_lowercase());
            last_dash = false;
        } else if matches!(ch, ' ' | '-' | '_') {
            if !last_dash && !result.is_empty() {
                result.push('-');
                last_dash = true;
            }
        }
    }

    if result.is_empty() {
        "object".to_string()
    } else {
        if last_dash {
            result.pop();
        }
        result
    }
}

fn non_max_suppression(mut boxes: Vec<CandidateBox>, threshold: f32) -> Vec<CandidateBox> {
    boxes.sort_by(|a, b| b.score.partial_cmp(&a.score).unwrap_or(Ordering::Equal));
    let mut result: Vec<CandidateBox> = Vec::new();

    'outer: for candidate in boxes {
        for existing in &result {
            if existing.class_idx == candidate.class_idx && iou(&candidate, existing) >= threshold {
                continue 'outer;
            }
        }

        result.push(candidate);
        if result.len() >= YOLO_MAX_DETECTIONS {
            break;
        }
    }

    result
}

fn iou(a: &CandidateBox, b: &CandidateBox) -> f32 {
    let inter_left = a.x1.max(b.x1);
    let inter_top = a.y1.max(b.y1);
    let inter_right = a.x2.min(b.x2);
    let inter_bottom = a.y2.min(b.y2);

    let inter_width = (inter_right - inter_left).max(0.0);
    let inter_height = (inter_bottom - inter_top).max(0.0);
    if inter_width <= 0.0 || inter_height <= 0.0 {
        return 0.0;
    }

    let inter_area = inter_width * inter_height;
    let area_a = (a.x2 - a.x1).max(0.0) * (a.y2 - a.y1).max(0.0);
    let area_b = (b.x2 - b.x1).max(0.0) * (b.y2 - b.y1).max(0.0);
    let union = (area_a + area_b - inter_area).max(f32::EPSILON);
    inter_area / union
}

fn extract_tensor(value: &DynValue, name: &str) -> StdResult<Array2<f32>, String> {
    let (shape, data) = value
        .try_extract_tensor::<f32>()
        .map_err(|err| format!("Failed to extract output {name}: {err}"))?;

    let dims: Vec<usize> = shape
        .iter()
        .map(|dim| {
            usize::try_from(*dim)
                .map_err(|_| format!("Output {name} has unsupported dimension {dim}"))
        })
        .collect::<StdResult<_, _>>()?;

    let mut array = ArrayD::from_shape_vec(IxDyn(&dims), data.to_vec())
        .map_err(|err| format!("Output {name} could not be materialized: {err}"))?;
    let mut dims: Vec<usize> = array.shape().iter().copied().collect();
    println!("YOLO output {name} raw shape: {:?}", dims);

    if dims.is_empty() {
        return Err(format!("Output {name} has empty shape"));
    }

    let total_elements = array.len();
    if total_elements == 0 {
        return Err(format!("Output {name} contains no data"));
    }

    let features_axis = if dims.len() >= 2 {
        dims.iter()
            .enumerate()
            .rev()
            .find(|(_, &dim)| dim >= 5 && dim <= 512)
            .map(|(idx, _)| idx)
            .or_else(|| {
                dims.iter()
                    .enumerate()
                    .rev()
                    .find(|(_, &dim)| dim >= 5)
                    .map(|(idx, _)| idx)
            })
            .unwrap_or(dims.len() - 1)
    } else {
        dims.len() - 1
    };

    if dims.len() > 1 && features_axis != dims.len() - 1 {
        let mut axes: Vec<usize> = (0..dims.len()).collect();
        axes.swap(features_axis, dims.len() - 1);
        array = array.permuted_axes(axes);
        dims = array.shape().iter().copied().collect();
    }

    while dims.len() > 2 && dims.first() == Some(&1) {
        dims.remove(0);
    }

    let features = *dims
        .last()
        .ok_or_else(|| format!("Output {name} missing feature dimension"))?;
    if features < 5 {
        return Err(format!(
            "Output {name} feature dimension {} is too small to contain YOLO metadata",
            features
        ));
    }

    if total_elements % features != 0 {
        return Err(format!(
            "Output {name} element count {} not divisible by feature dimension {}",
            total_elements, features
        ));
    }

    let anchors = total_elements / features;
    if anchors == 0 {
        return Err(format!("Output {name} anchor count resolved to zero"));
    }

    let reshaped = array
        .into_shape((anchors, features))
        .map_err(|err| format!("Output {name} reshape failed: {err}"))?
        .into_dimensionality::<Ix2>()
        .map_err(|err| format!("Output {name} dimensionality conversion failed: {err}"))?;

    println!(
        "YOLO analytics: anchors={} features={} confidence_threshold={:.2}",
        anchors, features, YOLO_CONFIDENCE_THRESHOLD
    );
    Ok(reshaped)
}

pub(super) fn build_execution_provider_chain(
    preference: ExecutionProviderPreference,
) -> (Vec<ExecutionProviderDispatch>, Vec<&'static str>) {
    let mut providers: Vec<ExecutionProviderDispatch> = Vec::new();
    let mut labels: Vec<&'static str> = Vec::new();

    #[cfg(target_os = "windows")]
    {
        match preference {
            ExecutionProviderPreference::Auto | ExecutionProviderPreference::DirectML => {
                // Try multiple device IDs to find the best GPU
                // DirectML often lists: 0=discrete GPU, 1=integrated GPU, but not always
                let device_ids_to_try = vec![0, 1, 2];
                let mut directml_added = false;

                for device_id in device_ids_to_try {
                    let directml_provider =
                        DirectMLExecutionProvider::default().with_device_id(device_id);

                    match directml_provider.is_available() {
                        Ok(true) => {
                            providers.push(directml_provider.build().fail_silently());
                            labels.push("DirectML");
                            directml_added = true;
                            println!("✓ DirectML GPU acceleration is ENABLED");
                            println!("  → Using GPU device ID {} for analytics", device_id);
                            println!("  → If integrated GPU is used instead of discrete GPU,");
                            println!("     you can force discrete GPU in Windows Settings:");
                            println!("     Settings → System → Display → Graphics → Dashboard for OpenIPC → High Performance");
                            break;
                        }
                        Ok(false) => {
                            // Try next device
                            continue;
                        }
                        Err(_) => {
                            // Try next device
                            continue;
                        }
                    }
                }

                if !directml_added {
                    if matches!(preference, ExecutionProviderPreference::DirectML) {
                        println!(
                            "⚠ DirectML provider requested but unavailable; falling back to CPU"
                        );
                        println!(
                            "  → Check GPU drivers and ensure Windows 10+ with DirectX 12 support"
                        );
                    } else {
                        println!("ℹ DirectML provider not available in Auto mode; using CPU");
                    }
                }
            }
            ExecutionProviderPreference::Cpu => {
                println!("ℹ CPU-only mode selected (GPU acceleration disabled)");
            }
        }
    }

    #[cfg(not(target_os = "windows"))]
    {
        if matches!(preference, ExecutionProviderPreference::DirectML) {
            println!(
                "YOLO init: DirectML provider requested but unsupported on this platform; using CPU"
            );
        }
    }

    providers.push(CPUExecutionProvider::default().build().error_on_failure());
    labels.push("CPU");

    (providers, labels)
}

fn describe_provider_chain(labels: &[&str]) -> String {
    if labels.is_empty() {
        return "<none>".to_string();
    }

    labels.join(" -> ")
}

fn resolve_input_shape(session: &Session) -> StdResult<(usize, usize), String> {
    let input = session
        .inputs
        .first()
        .ok_or_else(|| "YOLO model exposes no inputs".to_string())?;

    match &input.input_type {
        ValueType::Tensor { shape, .. } => {
            let dims: Vec<usize> = shape
                .iter()
                .map(|dim| {
                    usize::try_from(*dim)
                        .map_err(|_| "YOLO model input dimensions must be positive".to_string())
                })
                .collect::<StdResult<_, _>>()?;

            if dims.len() < 4 {
                return Err("YOLO model input rank is lower than expected (need NCHW)".to_string());
            }

            let height = *dims
                .get(dims.len().saturating_sub(2))
                .ok_or_else(|| "YOLO input height index missing".to_string())?;
            let width = *dims
                .last()
                .ok_or_else(|| "YOLO input width index missing".to_string())?;

            Ok((width, height))
        }
        _ => Err("YOLO model input is not a tensor".to_string()),
    }
}

fn get_environment(module_dir: &Path) -> StdResult<&'static Environment, String> {
    ORT_ENVIRONMENT
        .get_or_try_init(|| {
            let runtime_path = configure_ort_runtime(module_dir)?;
            let _ = environment::init_from(&runtime_path)
                .with_name("vms-analytics-yolo")
                .commit()
                .map_err(|err: OrtError| err.to_string())?;

            let environment = environment::get_environment().map_err(|err| err.to_string())?;
            environment.set_log_level(LogLevel::Warning);
            Ok(environment)
        })
        .map(|env| *env)
}

pub(super) fn ensure_ort_environment(module_dir: &Path) -> StdResult<(), String> {
    let _ = get_environment(module_dir)?;
    Ok(())
}

fn configure_ort_runtime(module_dir: &Path) -> StdResult<String, String> {
    if let Ok(path) = std::env::var("ORT_DYLIB_PATH") {
        return Ok(path);
    }

    if let Some(path) = locate_runtime_library(module_dir) {
        std::env::set_var("ORT_DYLIB_PATH", &path);
        Ok(path)
    } else {
        Err("ONNX Runtime dynamic library not found. Set ORT_DYLIB_PATH or place the runtime alongside the model.".to_string())
    }
}

fn locate_runtime_library(module_dir: &Path) -> Option<String> {
    for candidate in candidate_runtime_paths(module_dir) {
        if candidate.exists() {
            return Some(candidate.display().to_string());
        }
    }
    None
}

fn candidate_runtime_paths(module_dir: &Path) -> Vec<PathBuf> {
    let mut paths = Vec::new();
    for runtime_file in runtime_library_candidates() {
        paths.push(module_dir.join(&runtime_file));
        paths.push(module_dir.join("runtime").join(&runtime_file));
        paths.push(module_dir.join("ort").join(&runtime_file));
    }
    paths
}

fn runtime_library_candidates() -> Vec<String> {
    #[cfg(target_os = "windows")]
    {
        vec!["onnxruntime.dll".to_string()]
    }

    #[cfg(target_os = "linux")]
    {
        vec![
            "libonnxruntime.so".to_string(),
            format!("libonnxruntime.so.{}", super::onnx_runtime_version_str()),
        ]
    }

    #[cfg(target_os = "macos")]
    {
        vec![
            "libonnxruntime.dylib".to_string(),
            format!("libonnxruntime.{}.dylib", super::onnx_runtime_version_str()),
        ]
    }

    #[cfg(not(any(target_os = "windows", target_os = "linux", target_os = "macos")))]
    {
        vec!["libonnxruntime.so".to_string()]
    }
}
