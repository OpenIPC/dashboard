use std::cmp::Ordering;
use std::path::{Path, PathBuf};
use std::process::Command;
use std::result::Result as StdResult;
use std::sync::Mutex;
use std::time::{Duration, Instant};

#[cfg(windows)]
use std::os::windows::process::CommandExt;

use image::imageops::{self, FilterType};
use image::{DynamicImage, GenericImageView, GrayImage, Luma};
use ort::{
    session::builder::GraphOptimizationLevel,
    session::Session,
    value::{Tensor, ValueType},
};
use serde::{Deserialize, Serialize};
use serde_json::{json, Value};

use super::anpr_config;
use super::yolo::{
    build_execution_provider_chain, ensure_ort_environment, YoloDetector, YoloDetectorOptions,
};
use super::{AnalyticsEngine, BoundingBox, DetectionBox, ExecutionProviderPreference};

const LICENSE_PLATE_MODEL_FILE: &str = "anpr_yolov8.onnx";
const LICENSE_PLATE_OCR_MODEL_FILE: &str = "anpr_crnn.onnx";
const PYTHON_OCR_SCRIPT: &str = "anpr_ocr.py";
const PYTHON_OCR_MODEL_FILE: &str = "crnn_ocr_model_best.pth";
const LICENSE_PLATE_LABELS: &[&str] = &["License Plate"];
const LICENSE_PLATE_COLORS: &[&str] = &["#facc15"];
const LICENSE_PLATE_DEFAULT_COLOR: &str = "#facc15";
const LICENSE_PLATE_FALLBACK_LABEL: &str = "License Plate";
// CRNN model alphabet (Latin): "0123456789ABCEHKMOPTXY"
// Will transliterate to Cyrillic after recognition
const PLATE_ALPHABET: &str = "0123456789ABCEHKMOPTXY";
const CRNN_INPUT_WIDTH: usize = 128;
const CRNN_INPUT_HEIGHT: usize = 32;
const RECOGNITION_CACHE_TTL: Duration = Duration::from_secs(5);
const RECOGNITION_CACHE_MAX_SIZE: usize = 32;
const RECOGNITION_IOU_THRESHOLD: f32 = 0.55;
const RECOGNITION_CENTER_DISTANCE_THRESHOLD: f32 = 0.18;
const SLOW_FRAME_LOG_THRESHOLD: Duration = Duration::from_millis(80);

pub struct LicensePlateEngine {
    detector: YoloDetector,
    recognizer: CrnnRecognizer,
    recognition_cache: Mutex<Vec<PlateCacheEntry>>,
    module_dir: PathBuf,
}

impl LicensePlateEngine {
    pub fn new(
        module_dir: &Path,
        provider_pref: ExecutionProviderPreference,
    ) -> StdResult<Self, String> {
        let options = YoloDetectorOptions {
            model_file: LICENSE_PLATE_MODEL_FILE,
            class_labels: LICENSE_PLATE_LABELS,
            color_palette: LICENSE_PLATE_COLORS,
            default_color: LICENSE_PLATE_DEFAULT_COLOR,
        };

        let detector = YoloDetector::new(module_dir, provider_pref, options)?;
        let recognizer = CrnnRecognizer::new(module_dir, provider_pref)?;

        Ok(Self {
            detector,
            recognizer,
            recognition_cache: Mutex::new(Vec::new()),
            module_dir: module_dir.to_path_buf(),
        })
    }

    fn lookup_cached_label(&self, bounds: &BoundingBox) -> Option<String> {
        let now = Instant::now();
        let mut cache = match self.recognition_cache.lock() {
            Ok(guard) => guard,
            Err(err) => {
                println!("license-plate: cache lock poisoned: {err}");
                return None;
            }
        };

        cache.retain(|entry| now.duration_since(entry.updated_at) <= RECOGNITION_CACHE_TTL);

        let mut best: Option<&PlateCacheEntry> = None;
        for entry in cache.iter() {
            let iou = box_iou(&entry.bounds, bounds);
            if iou >= RECOGNITION_IOU_THRESHOLD {
                best = Some(entry);
                break;
            }
            let distance = center_distance_ratio(&entry.bounds, bounds);
            if distance <= RECOGNITION_CENTER_DISTANCE_THRESHOLD {
                best = Some(entry);
                break;
            }
        }

        best.map(|entry| entry.label.clone())
    }

    fn store_cached_label(&self, bounds: &BoundingBox, label: &str) {
        if label.is_empty() || label == LICENSE_PLATE_FALLBACK_LABEL {
            return;
        }

        let now = Instant::now();
        let mut cache = match self.recognition_cache.lock() {
            Ok(guard) => guard,
            Err(err) => {
                println!("license-plate: cache lock poisoned: {err}");
                return;
            }
        };

        if let Some(entry) = cache.iter_mut().find(|entry| {
            box_iou(&entry.bounds, bounds) >= RECOGNITION_IOU_THRESHOLD
                || center_distance_ratio(&entry.bounds, bounds)
                    <= RECOGNITION_CENTER_DISTANCE_THRESHOLD
        }) {
            entry.bounds = bounds.clone();
            entry.label = label.to_string();
            entry.updated_at = now;
            return;
        }

        if cache.len() >= RECOGNITION_CACHE_MAX_SIZE {
            if let Some((index, _)) = cache
                .iter()
                .enumerate()
                .min_by_key(|(_, entry)| entry.updated_at)
            {
                cache.remove(index);
            }
        }

        cache.push(PlateCacheEntry {
            bounds: bounds.clone(),
            label: label.to_string(),
            updated_at: now,
        });
    }
}

impl AnalyticsEngine for LicensePlateEngine {
    fn process(&self, frame: &DynamicImage, options: &Value) -> StdResult<Vec<DetectionBox>, String> {
        let frame_start = Instant::now();

        // Get current ANPR config
        let config = anpr_config::get_config();

        let mut effective_options = options.clone();
        if effective_options.get("confidence_threshold").is_none() {
            if let Some(obj) = effective_options.as_object_mut() {
                obj.insert(
                    "confidence_threshold".to_string(),
                    json!(config.detection_confidence),
                );
            } else if effective_options.is_null() {
                effective_options = json!({
                    "confidence_threshold": config.detection_confidence
                });
            }
        }

        let mut detections = self.detector.process(frame, &effective_options)?;
        let (frame_width, frame_height) = frame.dimensions();

        for (_index, detection) in detections.iter_mut().enumerate() {
            if let Some(cached) = self.lookup_cached_label(&detection.bounds) {
                detection.label = cached;
                detection.color = LICENSE_PLATE_DEFAULT_COLOR.to_string();
                continue;
            }

            // IMPROVEMENT: Expand bbox using configurable factor
            // This minimizes distortion when resizing to 128×32 for OCR
            let expanded_bounds = expand_bbox_with_factor(
                &detection.bounds,
                frame_width,
                frame_height,
                config.crop_expansion_factor,
            );

            let roi = crop_region(frame, &expanded_bounds, frame_width, frame_height);
            let label = match roi {
                Some(region) => {
                    // Try CRNN OCR first
                    let (crnn_text, crnn_score) =
                        self.recognizer.recognize(&region).unwrap_or_default();

                    // Validate Russian plate format: X###XX### (letter-3digits-2letters-2or3digits)
                    let is_valid_format = validate_russian_plate_format(&crnn_text);

                    // Calculate confidence based on score and format validation
                    let crnn_confidence = if crnn_score > 0 && is_valid_format {
                        (crnn_score as f32 / 9.0).min(1.0)
                    } else if crnn_score > 0 {
                        // Invalid format penalty
                        (crnn_score as f32 / 9.0 * 0.5).min(1.0)
                    } else {
                        0.0
                    };

                    println!(
                        "  CRNN result: '{}' (score={}, valid={}, conf={:.1}%)",
                        crnn_text,
                        crnn_score,
                        is_valid_format,
                        crnn_confidence * 100.0
                    );

                    // Always try Python OCR if enabled for comparison
                    let text = if config.enable_python_ocr {
                        match self
                            .recognizer
                            .recognize_with_python(&region, &self.module_dir)
                        {
                            Ok(python_text) if !python_text.is_empty() => {
                                let python_valid = validate_russian_plate_format(&python_text);
                                println!(
                                    "  Python OCR result: '{}' (valid={})",
                                    python_text, python_valid
                                );

                                // Decision logic:
                                // 1. If Python valid and CRNN not valid -> Python
                                // 2. If both valid -> prefer longer match (more chars recognized)
                                // 3. If CRNN confidence > python_threshold and valid -> CRNN
                                // 4. Otherwise -> Python (has perspective correction)
                                if python_valid && !is_valid_format {
                                    println!("  → Using Python (CRNN format invalid)");
                                    python_text
                                } else if python_valid && is_valid_format {
                                    // Both valid - compare by length and confidence
                                    let python_len =
                                        python_text.chars().filter(|c| c.is_alphanumeric()).count();
                                    let crnn_len = crnn_score;

                                    if crnn_confidence > config.python_confidence_threshold
                                        && crnn_len >= python_len
                                    {
                                        println!(
                                            "  → Using CRNN (high confidence: {:.1}% > {:.1}%)",
                                            crnn_confidence * 100.0,
                                            config.python_confidence_threshold * 100.0
                                        );
                                        crnn_text
                                    } else {
                                        println!(
                                            "  → Using Python (better for angles/perspective)"
                                        );
                                        python_text
                                    }
                                } else if is_valid_format
                                    && crnn_confidence > config.crnn_confidence_threshold
                                {
                                    println!(
                                        "  → Using CRNN (valid format, confidence {:.1}% > {:.1}%)",
                                        crnn_confidence * 100.0,
                                        config.crnn_confidence_threshold * 100.0
                                    );
                                    crnn_text
                                } else {
                                    println!("  → Using Python (has perspective correction)");
                                    python_text
                                }
                            }
                            Ok(_) => {
                                println!("  Python OCR returned empty");
                                println!("  → Using CRNN result");
                                crnn_text
                            }
                            Err(err) => {
                                println!("  Python OCR error: {}", err);
                                println!("  → Using CRNN result");
                                crnn_text
                            }
                        }
                    } else {
                        crnn_text
                    };

                    if text.is_empty() {
                        LICENSE_PLATE_FALLBACK_LABEL.to_string()
                    } else {
                        format!("Plate {text}")
                    }
                }
                None => LICENSE_PLATE_FALLBACK_LABEL.to_string(),
            };

            detection.label = label;
            detection.color = LICENSE_PLATE_DEFAULT_COLOR.to_string();
            self.store_cached_label(&detection.bounds, &detection.label);
        }

        let elapsed = frame_start.elapsed();
        if elapsed > SLOW_FRAME_LOG_THRESHOLD {
            println!(
                "license-plate: processing frame took {:?} for {} detections",
                elapsed,
                detections.len()
            );
        }

        Ok(detections)
    }
}

/// Validate Russian license plate format
/// Valid formats: X###XX## or X###XX### (letter-digits-letters-digits)
/// Valid letters: А, В, Е, К, М, Н, О, Р, С, Т, У, Х (Cyrillic that look like Latin)
/// Also accepts Latin equivalents that are transliterated to Cyrillic
fn validate_russian_plate_format(text: &str) -> bool {
    if text.len() < 8 || text.len() > 9 {
        return false;
    }

    let chars: Vec<char> = text.chars().collect();
    let valid_cyrillic = ['А', 'В', 'Е', 'К', 'М', 'Н', 'О', 'Р', 'С', 'Т', 'У', 'Х'];
    let valid_latin = ['A', 'B', 'E', 'K', 'M', 'H', 'O', 'P', 'C', 'T', 'Y', 'X'];

    let is_valid_letter = |ch: char| valid_cyrillic.contains(&ch) || valid_latin.contains(&ch);

    // Check pattern: L DDD LL DDD or L DDD LL DD
    if chars.len() == 9 {
        // X###XX###
        is_valid_letter(chars[0])
            && chars[1].is_numeric()
            && chars[2].is_numeric()
            && chars[3].is_numeric()
            && is_valid_letter(chars[4])
            && is_valid_letter(chars[5])
            && chars[6].is_numeric()
            && chars[7].is_numeric()
            && chars[8].is_numeric()
    } else if chars.len() == 8 {
        // X###XX##
        is_valid_letter(chars[0])
            && chars[1].is_numeric()
            && chars[2].is_numeric()
            && chars[3].is_numeric()
            && is_valid_letter(chars[4])
            && is_valid_letter(chars[5])
            && chars[6].is_numeric()
            && chars[7].is_numeric()
    } else {
        false
    }
}

struct PlateCacheEntry {
    bounds: BoundingBox,
    label: String,
    updated_at: Instant,
}

/// JSON response from Python OCR script
#[derive(Debug, Deserialize, Serialize)]
struct PythonOcrResponse {
    success: Option<bool>,
    text: Option<String>,
    cyrillic: Option<String>,
    latin: Option<String>,
    error: Option<String>,
}

/// Transliterate Latin to Cyrillic for Russian plates
fn transliterate_to_cyrillic(text: &str) -> String {
    text.chars()
        .map(|ch| match ch {
            'A' => 'А',
            'B' => 'В',
            'E' => 'Е',
            'C' => 'С',
            'H' => 'Н',
            'K' => 'К',
            'M' => 'М',
            'O' => 'О',
            'P' => 'Р',
            'T' => 'Т',
            'X' => 'Х',
            'Y' => 'У',
            _ => ch,
        })
        .collect()
}

struct CrnnRecognizer {
    session: Mutex<Session>,
    alphabet: Vec<char>,
    num_classes: usize,
}

impl CrnnRecognizer {
    fn new(
        module_dir: &Path,
        provider_pref: ExecutionProviderPreference,
    ) -> StdResult<Self, String> {
        let model_path = module_dir.join(LICENSE_PLATE_OCR_MODEL_FILE);
        if !model_path.exists() {
            return Err(format!("CRNN model not found at {}", model_path.display()));
        }

        ensure_ort_environment(module_dir)?;
        let (providers, provider_labels) = build_execution_provider_chain(provider_pref);
        println!("license-plate OCR: Using CRNN with enhanced video preprocessing");
        println!("  Execution providers: {:?}", provider_labels);

        let session = Session::builder()
            .map_err(|err| format!("Failed to build CRNN session: {err}"))?
            .with_optimization_level(GraphOptimizationLevel::Level3)
            .map_err(|err| format!("Failed to set optimization level: {err}"))?
            .with_execution_providers(providers.as_slice())
            .map_err(|err| format!("Failed to configure execution providers: {err}"))?
            .commit_from_file(&model_path)
            .map_err(|err| format!("Failed to load CRNN model: {err}"))?;

        let num_classes = resolve_num_classes(&session)?;
        let mut alphabet: Vec<char> = PLATE_ALPHABET.chars().collect();
        alphabet.truncate(num_classes.saturating_sub(1));

        Ok(Self {
            session: Mutex::new(session),
            alphabet,
            num_classes,
        })
    }

    /// Recognize plate using Python subprocess (with perspective correction)
    fn recognize_with_python(
        &self,
        roi: &DynamicImage,
        module_dir: &Path,
    ) -> StdResult<String, String> {
        use std::fs;

        // Save crop to temp file
        let temp_dir = std::env::temp_dir();
        let temp_image = temp_dir.join(format!("plate_crop_{}.jpg", std::process::id()));

        roi.save(&temp_image)
            .map_err(|err| format!("Failed to save temp image: {err}"))?;

        // Find Python script - try multiple locations
        let possible_paths = vec![
            // Downloaded by module system: in module directory
            Some(module_dir.join(PYTHON_OCR_SCRIPT)),
            // Production: next to modules directory
            module_dir
                .parent()
                .and_then(|p| p.parent())
                .map(|p| p.join("python_src").join(PYTHON_OCR_SCRIPT)),
            // Development: in src-tauri
            std::env::current_dir().ok().map(|p| {
                p.join("src-tauri")
                    .join("python_src")
                    .join(PYTHON_OCR_SCRIPT)
            }),
            // Fallback: relative to executable
            std::env::current_exe().ok().and_then(|exe| {
                exe.parent()
                    .map(|p| p.join("python_src").join(PYTHON_OCR_SCRIPT))
            }),
        ];

        let script_path = possible_paths
            .into_iter()
            .flatten()
            .find(|p| p.exists())
            .ok_or_else(|| {
                let searched = vec![
                    module_dir.join(PYTHON_OCR_SCRIPT).display().to_string(),
                    module_dir
                        .parent()
                        .and_then(|p| p.parent())
                        .map(|p| {
                            p.join("python_src")
                                .join(PYTHON_OCR_SCRIPT)
                                .display()
                                .to_string()
                        })
                        .unwrap_or_default(),
                ];
                format!("Python OCR script not found. Searched: {:?}", searched)
            })?;

        // Find Python model - try multiple locations
        let possible_model_paths = vec![
            // Downloaded by module system: in module directory
            Some(module_dir.join(PYTHON_OCR_MODEL_FILE)),
            // Development: in anpr/ subdirectory next to the script
            script_path
                .parent()
                .map(|p| p.join("anpr").join(PYTHON_OCR_MODEL_FILE)),
        ];

        let model_path = possible_model_paths
            .into_iter()
            .flatten()
            .find(|p| p.exists())
            .ok_or_else(|| {
                format!(
                    "Python OCR model not found. Searched: {} and script_dir/anpr/",
                    module_dir.join(PYTHON_OCR_MODEL_FILE).display()
                )
            })?;

        // Execute Python subprocess
        let mut cmd = Command::new("python");
        cmd.arg(&script_path)
            .arg("--model")
            .arg(&model_path)
            .arg("--image")
            .arg(&temp_image)
            .arg("--json");

        // Hide console window on Windows
        #[cfg(windows)]
        {
            const CREATE_NO_WINDOW: u32 = 0x08000000;
            cmd.creation_flags(CREATE_NO_WINDOW);
        }

        let output = cmd
            .output()
            .map_err(|err| format!("Failed to execute Python OCR: {err}"))?;

        // Cleanup temp file
        let _ = fs::remove_file(&temp_image);

        if !output.status.success() {
            let stderr = String::from_utf8_lossy(&output.stderr);
            return Err(format!("Python OCR failed: {stderr}"));
        }

        // Parse JSON response
        let stdout = String::from_utf8_lossy(&output.stdout);
        let response: PythonOcrResponse = serde_json::from_str(&stdout)
            .map_err(|err| format!("Failed to parse Python response: {err}"))?;

        if let Some(error) = response.error {
            return Err(format!("Python OCR error: {error}"));
        }

        Ok(response.cyrillic.or(response.text).unwrap_or_default())
    }

    fn recognize(&self, roi: &DynamicImage) -> StdResult<(String, usize), String> {
        // Try multiple preprocessing strategies and pick the longest result
        // (longer = more characters recognized = likely better)
        let strategies = vec![
            ("minimal", preprocess_minimal(roi)),
            ("video", preprocess_plate_for_video(roi)),
            ("print", preprocess_for_print(roi)),
            ("new_format", preprocess_new_format(roi)),
        ];

        let mut best_result = String::new();
        let mut best_score = 0;

        for (name, preprocessed) in strategies {
            let input = self.prepare_input_from_gray(&preprocessed);
            let tensor =
                Tensor::from_array(([1usize, 1, CRNN_INPUT_HEIGHT, CRNN_INPUT_WIDTH], input))
                    .map_err(|err| format!("Failed to build tensor: {err}"))?;

            let latin_text = {
                let mut session = self
                    .session
                    .lock()
                    .map_err(|_| "Failed to acquire session lock".to_string())?;

                let inference_outputs = session
                    .run(ort::inputs![tensor])
                    .map_err(|err| format!("CRNN inference failed: {err}"))?;

                let output_value = inference_outputs
                    .into_iter()
                    .next()
                    .map(|(_, value)| value)
                    .ok_or_else(|| "CRNN produced no outputs".to_string())?;

                decode_sequence(output_value, &self.alphabet, self.num_classes)?
            };

            let cyrillic_text = transliterate_to_cyrillic(&latin_text);

            // Score: length + alphanumeric ratio
            let alphanumeric_count = cyrillic_text
                .chars()
                .filter(|c| c.is_alphanumeric())
                .count();
            let score = alphanumeric_count;

            println!(
                "  OCR strategy '{}': '{}' (score={})",
                name, cyrillic_text, score
            );

            if score > best_score {
                best_score = score;
                best_result = cyrillic_text;
            }
        }

        Ok((best_result, best_score))
    }

    fn prepare_input_from_gray(&self, preprocessed: &GrayImage) -> Vec<f32> {
        let mut data = Vec::with_capacity(CRNN_INPUT_HEIGHT * CRNN_INPUT_WIDTH);
        for pixel in preprocessed.pixels() {
            let normalized = (pixel[0] as f32 / 255.0 - 0.5) / 0.5;
            data.push(normalized);
        }

        if data.len() != CRNN_INPUT_HEIGHT * CRNN_INPUT_WIDTH {
            data.resize(CRNN_INPUT_HEIGHT * CRNN_INPUT_WIDTH, 0.0);
        }

        data
    }
}

fn resolve_num_classes(session: &Session) -> StdResult<usize, String> {
    let output = session
        .outputs
        .first()
        .ok_or_else(|| "CRNN model exposes no outputs".to_string())?;

    match &output.output_type {
        ValueType::Tensor { shape, .. } => {
            let dims: Vec<usize> = shape
                .iter()
                .map(|dim| {
                    usize::try_from(*dim)
                        .map_err(|_| "Output dimension must be positive".to_string())
                })
                .collect::<StdResult<_, _>>()?;

            let classes = dims
                .last()
                .copied()
                .unwrap_or_else(|| PLATE_ALPHABET.len() + 1);
            Ok(classes)
        }
        _ => Err("CRNN output is not a tensor".to_string()),
    }
}

fn decode_sequence(
    value: ort::value::DynValue,
    alphabet: &[char],
    _num_classes: usize,
) -> StdResult<String, String> {
    let (shape, data) = value
        .try_extract_tensor::<f32>()
        .map_err(|err| format!("Output extraction failed: {err}"))?;

    let flat = data.to_vec();
    let dims: Vec<usize> = shape
        .iter()
        .map(|dim| usize::try_from(*dim).map_err(|_| "Dimension must be positive".to_string()))
        .collect::<StdResult<_, _>>()?;

    if dims.is_empty() {
        return Ok(String::new());
    }

    let (sequence_len, classes) = match dims.as_slice() {
        [seq, _batch, cls] => (*seq, *cls),
        [seq, cls] => (*seq, *cls),
        _ => return Err(format!("Unsupported output shape {:?}", dims)),
    };

    if classes == 0 || sequence_len == 0 {
        return Ok(String::new());
    }

    let mut result = String::new();
    let mut prev_index = 0usize;

    for t in 0..sequence_len {
        let start = t * classes;
        let end = start + classes;
        let slice = &flat[start..end];
        let (best_index, _) = slice
            .iter()
            .enumerate()
            .max_by(|a, b| a.1.partial_cmp(b.1).unwrap_or(Ordering::Equal))
            .unwrap_or((0, &0.0));

        if best_index != 0 && best_index != prev_index {
            let alpha_index = best_index - 1;
            if alpha_index < alphabet.len() {
                result.push(alphabet[alpha_index]);
            }
        }

        prev_index = best_index;
    }

    Ok(result)
}

/// Minimal preprocessing - just grayscale and resize
fn preprocess_minimal(roi: &DynamicImage) -> GrayImage {
    let gray = roi.to_luma8();
    resize_with_padding(&gray, CRNN_INPUT_WIDTH as u32, CRNN_INPUT_HEIGHT as u32)
}

/// Preprocessing optimized for printed/high-contrast plates
fn preprocess_for_print(roi: &DynamicImage) -> GrayImage {
    let gray = roi.to_luma8();

    // Light denoising
    let denoised = apply_gaussian_denoise(&gray, 0.5);

    // Moderate contrast
    let contrasted = stretch_contrast_aggressive(&denoised, 0.02);

    // Resize
    let resized = resize_with_padding(
        &contrasted,
        CRNN_INPUT_WIDTH as u32,
        CRNN_INPUT_HEIGHT as u32,
    );

    // Light sharpening
    imageops::unsharpen(&resized, 1.0, 1)
}

/// Enhanced preprocessing specifically for video surveillance footage
/// Handles: motion blur, compression artifacts, varying lighting, angles
fn preprocess_plate_for_video(roi: &DynamicImage) -> GrayImage {
    let gray = roi.to_luma8();

    // Step 1: Denoise while preserving edges (bilateral-like with Gaussian)
    let denoised = apply_gaussian_denoise(&gray, 1.0);

    // Step 2: CLAHE for adaptive contrast - handles varying lighting
    let clahe_applied = apply_clahe(&denoised, 2.0, 8);

    // Step 3: Aggressive contrast stretching for video compression artifacts
    let contrasted = stretch_contrast_aggressive(&clahe_applied, 0.05);

    // Step 4: Resize with high-quality filter
    let resized = resize_with_padding(
        &contrasted,
        CRNN_INPUT_WIDTH as u32,
        CRNN_INPUT_HEIGHT as u32,
    );

    // Step 5: Strong sharpening to counter motion blur
    let sharpened = imageops::unsharpen(&resized, 2.0, 3);

    sharpened
}

/// Preprocessing optimized for new-format Russian plates with wider characters
/// Uses adaptive thresholding (Otsu-like) to handle varying lighting
fn preprocess_new_format(roi: &DynamicImage) -> GrayImage {
    let gray = roi.to_luma8();

    // Step 1: Slight denoise
    let denoised = apply_gaussian_denoise(&gray, 0.8);

    // Step 2: CLAHE with smaller tiles for local contrast
    let clahe_applied = apply_clahe(&denoised, 3.0, 4);

    // Step 3: Adaptive binarization using Otsu's method
    let binarized = apply_otsu_threshold(&clahe_applied);

    // Step 4: Resize
    let resized = resize_with_padding(
        &binarized,
        CRNN_INPUT_WIDTH as u32,
        CRNN_INPUT_HEIGHT as u32,
    );

    // Step 5: Light smoothing to reduce binarization artifacts
    imageops::blur(&resized, 0.5)
}

fn apply_gaussian_denoise(image: &GrayImage, _sigma: f32) -> GrayImage {
    let (width, height) = image.dimensions();
    if width < 3 || height < 3 {
        return image.clone();
    }

    let mut result = GrayImage::new(width, height);
    let kernel = [
        [0.0625, 0.125, 0.0625],
        [0.125, 0.25, 0.125],
        [0.0625, 0.125, 0.0625],
    ];

    for y in 1..(height - 1) {
        for x in 1..(width - 1) {
            let mut sum = 0.0;
            for ky in 0..3_usize {
                for kx in 0..3_usize {
                    let px = image.get_pixel(x + kx as u32 - 1, y + ky as u32 - 1)[0] as f32;
                    sum += px * kernel[ky][kx];
                }
            }
            result.put_pixel(x, y, Luma([sum as u8]));
        }
    }

    // Copy borders
    for x in 0..width {
        result.put_pixel(x, 0, *image.get_pixel(x, 0));
        if height > 1 {
            result.put_pixel(x, height - 1, *image.get_pixel(x, height - 1));
        }
    }
    for y in 0..height {
        result.put_pixel(0, y, *image.get_pixel(0, y));
        if width > 1 {
            result.put_pixel(width - 1, y, *image.get_pixel(width - 1, y));
        }
    }

    result
}

fn apply_clahe(image: &GrayImage, _clip_limit: f32, tile_size: usize) -> GrayImage {
    // Simplified CLAHE approximation using histogram equalization per tile
    let (width, height) = image.dimensions();
    let mut result = image.clone();

    let tiles_x = (width as usize + tile_size - 1) / tile_size;
    let tiles_y = (height as usize + tile_size - 1) / tile_size;

    for ty in 0..tiles_y {
        for tx in 0..tiles_x {
            let x_start = (tx * tile_size) as u32;
            let y_start = (ty * tile_size) as u32;
            let x_end = ((tx + 1) * tile_size).min(width as usize) as u32;
            let y_end = ((ty + 1) * tile_size).min(height as usize) as u32;

            // Build histogram for this tile
            let mut hist = [0u32; 256];
            for y in y_start..y_end {
                for x in x_start..x_end {
                    hist[image.get_pixel(x, y)[0] as usize] += 1;
                }
            }

            // Build CDF
            let total_pixels = ((x_end - x_start) * (y_end - y_start)) as f32;
            let mut cdf = [0.0f32; 256];
            let mut sum = 0.0;
            for i in 0..256 {
                sum += hist[i] as f32 / total_pixels;
                cdf[i] = sum;
            }

            // Apply equalization to tile
            for y in y_start..y_end {
                for x in x_start..x_end {
                    let old_val = image.get_pixel(x, y)[0];
                    let new_val = (cdf[old_val as usize] * 255.0) as u8;
                    result.put_pixel(x, y, Luma([new_val]));
                }
            }
        }
    }

    result
}

/// Otsu's method for adaptive thresholding - converts to black/white
fn apply_otsu_threshold(image: &GrayImage) -> GrayImage {
    let (width, height) = image.dimensions();
    if width == 0 || height == 0 {
        return image.clone();
    }

    // Calculate histogram
    let mut hist = [0u32; 256];
    for pixel in image.pixels() {
        hist[pixel[0] as usize] += 1;
    }

    let total_pixels = (width * height) as f32;

    // Find optimal threshold using Otsu's method
    let mut sum = 0.0f32;
    for (i, &count) in hist.iter().enumerate() {
        sum += (i as f32) * (count as f32);
    }

    let mut sum_background = 0.0f32;
    let mut weight_background = 0.0f32;
    let mut max_variance = 0.0f32;
    let mut threshold = 0u8;

    for t in 0..256 {
        weight_background += hist[t] as f32;
        if weight_background == 0.0 {
            continue;
        }

        let weight_foreground = total_pixels - weight_background;
        if weight_foreground == 0.0 {
            break;
        }

        sum_background += (t as f32) * (hist[t] as f32);

        let mean_background = sum_background / weight_background;
        let mean_foreground = (sum - sum_background) / weight_foreground;

        let variance =
            weight_background * weight_foreground * (mean_background - mean_foreground).powi(2);

        if variance > max_variance {
            max_variance = variance;
            threshold = t as u8;
        }
    }

    // Apply threshold
    let mut result = GrayImage::new(width, height);
    for (x, y, pixel) in image.enumerate_pixels() {
        let value = if pixel[0] > threshold { 255 } else { 0 };
        result.put_pixel(x, y, Luma([value]));
    }

    result
}

fn stretch_contrast_aggressive(image: &GrayImage, clip_fraction: f32) -> GrayImage {
    if image.width() == 0 || image.height() == 0 {
        return image.clone();
    }

    let total_pixels = (image.width() as u32) * (image.height() as u32);
    if total_pixels == 0 {
        return image.clone();
    }

    let clip_count = (total_pixels as f32 * clip_fraction).round() as u32;
    let low_target = clip_count;
    let high_target = total_pixels.saturating_sub(clip_count);

    let mut histogram = [0u32; 256];
    for pixel in image.pixels() {
        histogram[pixel[0] as usize] += 1;
    }

    let mut cumulative = 0u32;
    let mut low_value = 0u8;
    for (idx, &count) in histogram.iter().enumerate() {
        cumulative += count;
        if cumulative >= low_target {
            low_value = idx as u8;
            break;
        }
    }

    cumulative = 0u32;
    let mut high_value = 255u8;
    for (idx, &count) in histogram.iter().enumerate() {
        cumulative += count;
        if cumulative >= high_target {
            high_value = idx as u8;
            break;
        }
    }

    if high_value <= low_value {
        return image.clone();
    }

    let mut output = image.clone();
    let scale = 255.0f32 / (high_value - low_value) as f32;
    for pixel in output.pixels_mut() {
        let val = pixel[0];
        let clamped = val.clamp(low_value, high_value);
        let stretched = ((clamped - low_value) as f32 * scale).clamp(0.0, 255.0);
        pixel[0] = stretched as u8;
    }

    output
}

fn resize_with_padding(image: &GrayImage, target_width: u32, target_height: u32) -> GrayImage {
    if target_width == 0 || target_height == 0 {
        return image.clone();
    }

    let (width, height) = image.dimensions();
    if width == 0 || height == 0 {
        return GrayImage::from_pixel(target_width, target_height, Luma([128]));
    }

    let scale = (target_width as f32 / width as f32).min(target_height as f32 / height as f32);
    let resized_width = (width as f32 * scale).round().max(1.0) as u32;
    let resized_height = (height as f32 * scale).round().max(1.0) as u32;

    let resized = imageops::resize(image, resized_width, resized_height, FilterType::Lanczos3);

    let mut canvas = GrayImage::from_pixel(target_width, target_height, Luma([128]));
    let offset_x = ((target_width - resized_width) / 2) as i64;
    let offset_y = ((target_height - resized_height) / 2) as i64;
    imageops::overlay(&mut canvas, &resized, offset_x, offset_y);

    canvas
}

/// Expand bbox by a configurable factor while maintaining aspect ratio
/// Factor > 1.0 will expand the bbox, e.g., 1.5 = 50% larger
fn expand_bbox_with_factor(
    bounds: &BoundingBox,
    frame_width: u32,
    frame_height: u32,
    expansion_factor: f32,
) -> BoundingBox {
    let current_w = bounds.width;
    let current_h = bounds.height;

    if current_w <= 0.0 || current_h <= 0.0 || expansion_factor <= 0.0 {
        return bounds.clone();
    }

    // Calculate center
    let center_x = bounds.x + current_w / 2.0;
    let center_y = bounds.y + current_h / 2.0;

    // Apply expansion factor
    let new_w = current_w * expansion_factor;
    let new_h = current_h * expansion_factor;

    // Calculate new bounds from center
    let new_x = (center_x - new_w / 2.0).max(0.0);
    let new_y = (center_y - new_h / 2.0).max(0.0);
    let new_x2 = (center_x + new_w / 2.0).min(frame_width as f32);
    let new_y2 = (center_y + new_h / 2.0).min(frame_height as f32);

    BoundingBox {
        x: new_x,
        y: new_y,
        width: new_x2 - new_x,
        height: new_y2 - new_y,
    }
}

fn crop_region(
    frame: &DynamicImage,
    bounds: &BoundingBox,
    frame_width: u32,
    frame_height: u32,
) -> Option<DynamicImage> {
    let x1 = bounds.x.max(0.0).floor();
    let y1 = bounds.y.max(0.0).floor();
    let x2 = (bounds.x + bounds.width).min(frame_width as f32).ceil();
    let y2 = (bounds.y + bounds.height).min(frame_height as f32).ceil();

    if x2 <= x1 || y2 <= y1 {
        return None;
    }

    let w = (x2 - x1).max(1.0) as u32;
    let h = (y2 - y1).max(1.0) as u32;
    let sx = x1 as u32;
    let sy = y1 as u32;

    Some(frame.crop_imm(sx, sy, w, h))
}

fn box_iou(a: &BoundingBox, b: &BoundingBox) -> f32 {
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

fn center_distance_ratio(a: &BoundingBox, b: &BoundingBox) -> f32 {
    let ax = a.x + a.width / 2.0;
    let ay = a.y + a.height / 2.0;
    let bx = b.x + b.width / 2.0;
    let by = b.y + b.height / 2.0;
    let dx = ax - bx;
    let dy = ay - by;
    let distance = (dx * dx + dy * dy).sqrt();
    let diag = ((a.width + b.width) * 0.5).hypot((a.height + b.height) * 0.5);
    if diag <= f32::EPSILON {
        return 1.0;
    }
    distance / diag
}
