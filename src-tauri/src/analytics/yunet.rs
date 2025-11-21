use std::cmp::Ordering;
use std::path::{Path, PathBuf};
use std::result::Result as StdResult;
use std::sync::Arc;

use image::{imageops::FilterType, DynamicImage, GenericImageView};
use ndarray::{Array2, Array4, ArrayView1, CowArray, IxDyn};
use once_cell::sync::OnceCell;
use ort::{
    environment::Environment, session::Session, value::Value, GraphOptimizationLevel, LoggingLevel,
    SessionBuilder,
};

#[cfg(any(target_os = "linux", target_os = "macos"))]
use super::onnx_runtime_version_str;
use super::{AnalyticsEngine, BoundingBox, DetectionBox};

const CONFIDENCE_THRESHOLD: f32 = 0.5;
const NMS_THRESHOLD: f32 = 0.3;
const TOP_K: usize = 5000;
const KEEP_TOP_K: usize = 750;
const MODEL_FILE: &str = super::FACE_DETECTOR_MODEL_FILE;

const MIN_SIZES: &[&[f32]] = &[
    &[10.0, 16.0, 24.0],
    &[32.0, 48.0],
    &[64.0, 96.0],
    &[128.0, 192.0, 256.0],
];

const STEPS: &[f32] = &[8.0, 16.0, 32.0, 64.0];
const VARIANCE: [f32; 2] = [0.1, 0.2];
const YUNET_STRIDES: [usize; 3] = [8, 16, 32];

static ORT_ENVIRONMENT: OnceCell<Arc<Environment>> = OnceCell::new();

#[derive(Clone, Copy)]
enum YuNetOutputLayout {
    Legacy {
        loc: usize,
        conf: usize,
        iou: usize,
    },
    Strided {
        bbox: [usize; 3],
        cls: [usize; 3],
        obj: [usize; 3],
    },
}

pub struct YuNetFaceDetector {
    session: Session,
    layout: YuNetOutputLayout,
    priors: Vec<[f32; 4]>,
    input_width: usize,
    input_height: usize,
}

impl YuNetFaceDetector {
    pub fn new(module_dir: &Path) -> StdResult<Self, String> {
        let model_path = module_dir.join(MODEL_FILE);
        if !model_path.exists() {
            return Err(format!(
                "Face detector model not found at {}",
                model_path.display()
            ));
        }

        println!(
            "YuNet init: module_dir={} model_path={}",
            module_dir.display(),
            model_path.display()
        );

        let environment = get_environment(module_dir)?;
        let session = SessionBuilder::new(&environment)
            .map_err(|err| format!("Failed to create ONNX session builder: {err}"))?
            .with_optimization_level(GraphOptimizationLevel::Level3)
            .map_err(|err| format!("Failed to configure ONNX optimization level: {err}"))?
            .with_model_from_file(model_path.as_os_str())
            .map_err(|err| format!("Failed to load YuNet model: {err}"))?;

        let (input_width, input_height) = resolve_input_shape(&session)?;
        println!(
            "YuNet init: resolved input_width={} input_height={}",
            input_width, input_height
        );

        let layout = resolve_output_layout(&session)?;
        match layout {
            YuNetOutputLayout::Legacy { .. } => {
                println!("YuNet init: using legacy loc/conf/iou outputs");
            }
            YuNetOutputLayout::Strided { .. } => {
                println!("YuNet init: using strided bbox/cls/obj outputs");
            }
        }

        let priors = generate_priors(input_width, input_height);
        println!("YuNet init: generated {} priors", priors.len());

        Ok(Self {
            session,
            layout,
            priors,
            input_width,
            input_height,
        })
    }
}

impl AnalyticsEngine for YuNetFaceDetector {
    fn process(&self, frame: &DynamicImage) -> StdResult<Vec<DetectionBox>, String> {
        let (orig_w, orig_h) = frame.dimensions();
        let resized = frame
            .resize_exact(
                self.input_width as u32,
                self.input_height as u32,
                FilterType::Triangle,
            )
            .to_rgb8();

        let mut tensor = Array4::<f32>::zeros((1, 3, self.input_height, self.input_width));
        for (x, y, pixel) in resized.enumerate_pixels() {
            let x = x as usize;
            let y = y as usize;
            tensor[[0, 0, y, x]] = pixel[0] as f32;
            tensor[[0, 1, y, x]] = pixel[1] as f32;
            tensor[[0, 2, y, x]] = pixel[2] as f32;
        }

        let input_array: CowArray<'_, f32, IxDyn> = CowArray::from(tensor.into_dyn());
        let input_value = Value::from_array(self.session.allocator(), &input_array)
            .map_err(|err| format!("Failed to build YuNet input tensor: {err}"))?;

        let output_values = self
            .session
            .run(vec![input_value])
            .map_err(|err| format!("YuNet inference failed: {err}"))?;

        let detections = match self.layout {
            YuNetOutputLayout::Legacy { loc, conf, iou } => {
                let loc_value = select_output(&output_values, loc, "loc")?;
                let conf_value = select_output(&output_values, conf, "conf")?;
                let iou_value = select_output(&output_values, iou, "iou")?;

                let loc = extract_tensor(loc_value, "loc")?;
                let conf = extract_tensor(conf_value, "conf")?;
                let iou = extract_tensor(iou_value, "iou")?;

                postprocess_legacy(
                    &self.priors,
                    &loc,
                    &conf,
                    &iou,
                    CONFIDENCE_THRESHOLD,
                    NMS_THRESHOLD,
                    TOP_K,
                    KEEP_TOP_K,
                    self.input_width as f32,
                    self.input_height as f32,
                )
            }
            YuNetOutputLayout::Strided {
                ref bbox,
                ref cls,
                ref obj,
            } => {
                let mut bbox_parts = Vec::with_capacity(bbox.len());
                let mut cls_parts = Vec::with_capacity(cls.len());
                let mut obj_parts = Vec::with_capacity(obj.len());

                for (stride_idx, ((&bbox_index, &cls_index), &obj_index)) in
                    bbox.iter().zip(cls.iter()).zip(obj.iter()).enumerate()
                {
                    let stride_suffix = YUNET_STRIDES
                        .get(stride_idx)
                        .map(|value| value.to_string())
                        .unwrap_or_else(|| format!("idx{}", stride_idx));
                    let bbox_label = format!("bbox_{}", stride_suffix);
                    let cls_label = format!("cls_{}", stride_suffix);
                    let obj_label = format!("obj_{}", stride_suffix);

                    let bbox_value =
                        select_output(&output_values, bbox_index, bbox_label.as_str())?;
                    let cls_value = select_output(&output_values, cls_index, cls_label.as_str())?;
                    let obj_value = select_output(&output_values, obj_index, obj_label.as_str())?;

                    bbox_parts.push(extract_tensor(bbox_value, bbox_label.as_str())?);
                    cls_parts.push(extract_tensor(cls_value, cls_label.as_str())?);
                    obj_parts.push(extract_tensor(obj_value, obj_label.as_str())?);
                }

                postprocess_strided(
                    &self.layout,
                    &bbox_parts,
                    &cls_parts,
                    &obj_parts,
                    CONFIDENCE_THRESHOLD,
                    NMS_THRESHOLD,
                    TOP_K,
                    KEEP_TOP_K,
                    self.input_width,
                    self.input_height,
                )
            }
        };

        let scale_x = orig_w as f32 / self.input_width as f32;
        let scale_y = orig_h as f32 / self.input_height as f32;

        Ok(detections
            .into_iter()
            .enumerate()
            .map(|(idx, det)| to_detection_box(idx, det, scale_x, scale_y))
            .collect())
    }
}

fn generate_priors(input_width: usize, input_height: usize) -> Vec<[f32; 4]> {
    let clamp_feature = |value: usize| if value == 0 { 1 } else { value };

    let feature_map_2th = [
        clamp_feature(((input_height + 1) / 2) / 2),
        clamp_feature(((input_width + 1) / 2) / 2),
    ];
    let feature_map_3th = [
        clamp_feature(feature_map_2th[0] / 2),
        clamp_feature(feature_map_2th[1] / 2),
    ];
    let feature_map_4th = [
        clamp_feature(feature_map_3th[0] / 2),
        clamp_feature(feature_map_3th[1] / 2),
    ];
    let feature_map_5th = [
        clamp_feature(feature_map_4th[0] / 2),
        clamp_feature(feature_map_4th[1] / 2),
    ];
    let feature_map_6th = [
        clamp_feature(feature_map_5th[0] / 2),
        clamp_feature(feature_map_5th[1] / 2),
    ];

    let feature_maps = [
        feature_map_3th,
        feature_map_4th,
        feature_map_5th,
        feature_map_6th,
    ];

    let width = input_width as f32;
    let height = input_height as f32;
    let mut priors = Vec::new();

    for (k, feature) in feature_maps.iter().enumerate() {
        let min_sizes = MIN_SIZES.get(k).copied().unwrap_or(&[]);
        if min_sizes.is_empty() {
            continue;
        }

        for i in 0..feature[0] {
            for j in 0..feature[1] {
                for &min_size in min_sizes.iter() {
                    let s_kx = min_size / width;
                    let s_ky = min_size / height;

                    let cx = (j as f32 + 0.5) * STEPS[k] / width;
                    let cy = (i as f32 + 0.5) * STEPS[k] / height;

                    priors.push([cx, cy, s_kx, s_ky]);
                }
            }
        }
    }

    priors
}

fn select_output<'a>(
    outputs: &'a [Value<'static>],
    index: usize,
    label: &str,
) -> StdResult<&'a Value<'static>, String> {
    outputs.get(index).ok_or_else(|| {
        format!(
            "ONNX output {label} missing at index {index}; available outputs: {}",
            outputs.len()
        )
    })
}

fn extract_tensor(value: &Value<'static>, name: &str) -> StdResult<Array2<f32>, String> {
    let owned = value
        .try_extract::<f32>()
        .map_err(|err| format!("Failed to extract output {name}: {err}"))?;

    let view = owned.view();
    let raw_shape: Vec<usize> = view.shape().iter().copied().collect();
    println!("YuNet output {name} raw shape: {:?}", raw_shape);

    if raw_shape.is_empty() {
        return Err(format!("Output {name} has empty shape"));
    }

    let features = *raw_shape
        .last()
        .ok_or_else(|| format!("Output {name} missing feature dimension"))?;
    if features == 0 {
        return Err(format!("Output {name} feature dimension is zero"));
    }

    let anchors = if raw_shape.len() == 1 {
        1
    } else {
        raw_shape[..raw_shape.len() - 1].iter().product()
    };

    if anchors == 0 {
        return Err(format!("Output {name} anchor count resolved to zero"));
    }

    let reshaped = view
        .to_owned()
        .into_shape((anchors, features))
        .map_err(|err| format!("Output {name} reshape failed: {err}"))?;

    println!("YuNet output {name}: anchors={anchors}, features={features}");
    Ok(reshaped)
}

fn postprocess_legacy(
    priors: &[[f32; 4]],
    loc: &Array2<f32>,
    conf: &Array2<f32>,
    iou: &Array2<f32>,
    confidence_threshold: f32,
    nms_threshold: f32,
    top_k: usize,
    keep_top_k: usize,
    input_width: f32,
    input_height: f32,
) -> Vec<[f32; 5]> {
    let mut scores = Vec::with_capacity(conf.nrows());
    for idx in 0..conf.nrows() {
        let conf_slice = conf.row(idx);
        let cls_score = if conf_slice.len() > 1 {
            conf_slice[1]
        } else {
            conf_slice[0]
        };
        let iou_score = iou.row(idx)[0].clamp(0.0, 1.0);
        let score = (cls_score * iou_score).sqrt();
        scores.push(score);
    }

    let max_score = scores.iter().copied().fold(f32::MIN, f32::max).max(0.0);
    println!(
        "YuNet analytics: priors={}, candidates={}, max_score={:.3}, threshold={:.2}",
        priors.len(),
        scores.len(),
        max_score,
        confidence_threshold
    );

    let mut boxes = Vec::with_capacity(loc.nrows());
    let mut debug_logged = false;
    for (idx, prior) in priors.iter().enumerate() {
        if idx >= loc.nrows() {
            break;
        }
        let loc_row = loc.row(idx);
        if loc_row.len() < 14 {
            continue;
        }
        let decode_point = |offset: usize| -> (f32, f32) {
            let x = (prior[0] + loc_row[offset] * VARIANCE[0] * prior[2]) * input_width;
            let y = (prior[1] + loc_row[offset + 1] * VARIANCE[0] * prior[3]) * input_height;
            (x, y)
        };

        let mut corners = Vec::with_capacity(4);
        for &offset in &[4usize, 6, 10, 12] {
            if offset + 1 < loc_row.len() {
                corners.push(decode_point(offset));
            }
        }

        if corners.len() < 4 {
            continue;
        }

        let (mut min_x, mut min_y) = (f32::MAX, f32::MAX);
        let (mut max_x, mut max_y) = (f32::MIN, f32::MIN);
        for (x, y) in &corners {
            min_x = min_x.min(*x);
            min_y = min_y.min(*y);
            max_x = max_x.max(*x);
            max_y = max_y.max(*y);
        }

        let x1 = min_x;
        let y1 = min_y;
        let x2 = max_x;
        let y2 = max_y;

        let width = (x2 - x1).max(0.0);
        let height = (y2 - y1).max(0.0);

        boxes.push([x1, y1, x2, y2]);

        if !debug_logged {
            if width < 80.0 || height < 80.0 {
                if let Some(slice) = loc_row.as_slice() {
                    let preview: Vec<f32> = slice.iter().take(16).cloned().collect();
                    println!(
						"YuNet debug anchor idx={} prior=({:.4},{:.4},{:.4},{:.4}) loc0-15={:?} width={:.2} height={:.2}",
						idx,
						prior[0],
						prior[1],
						prior[2],
						prior[3],
						preview,
						width,
						height
					);
                    debug_logged = true;
                }
            }
        }
    }

    let mut indices: Vec<_> = (0..scores.len()).collect();
    indices.sort_by(|&a, &b| scores[b].partial_cmp(&scores[a]).unwrap_or(Ordering::Equal));
    if indices.len() > top_k {
        indices.truncate(top_k);
    }

    let keep = non_max_suppression(
        &boxes,
        &scores,
        &indices,
        nms_threshold,
        keep_top_k,
        confidence_threshold,
    );
    println!("YuNet analytics: kept {} boxes after NMS", keep.len());
    if keep.is_empty() {
        if let Some(&best_idx) = indices.first() {
            let best_score = scores.get(best_idx).copied().unwrap_or_default();
            println!(
                "YuNet analytics: top candidate idx={} score={:.3}",
                best_idx, best_score
            );
        }
    }
    keep.into_iter()
        .map(|idx| {
            [
                boxes[idx][0],
                boxes[idx][1],
                boxes[idx][2],
                boxes[idx][3],
                scores[idx],
            ]
        })
        .collect()
}

fn postprocess_strided(
    layout: &YuNetOutputLayout,
    bbox_parts: &[Array2<f32>],
    cls_parts: &[Array2<f32>],
    obj_parts: &[Array2<f32>],
    confidence_threshold: f32,
    nms_threshold: f32,
    top_k: usize,
    keep_top_k: usize,
    input_width: usize,
    input_height: usize,
) -> Vec<[f32; 5]> {
    if bbox_parts.is_empty() || cls_parts.is_empty() || obj_parts.is_empty() {
        println!("YuNet analytics: strided layout missing outputs");
        return Vec::new();
    }

    let stride_values = match layout {
        YuNetOutputLayout::Strided { .. } => &YUNET_STRIDES,
        YuNetOutputLayout::Legacy { .. } => {
            println!("YuNet analytics: strided postprocess invoked with legacy layout");
            return Vec::new();
        }
    };

    let mut boxes: Vec<[f32; 4]> = Vec::new();
    let mut scores: Vec<f32> = Vec::new();
    let mut debug_logged = false;
    let mut raw_debug_count = 0usize;

    for stride_idx in 0..bbox_parts.len() {
        let bbox = &bbox_parts[stride_idx];
        let cls = match cls_parts.get(stride_idx) {
            Some(value) => value,
            None => continue,
        };
        let obj = match obj_parts.get(stride_idx) {
            Some(value) => value,
            None => continue,
        };

        let stride = stride_values.get(stride_idx).copied().unwrap_or(0);
        if stride == 0 {
            println!(
                "YuNet analytics: stride value missing for index {} (available={})",
                stride_idx,
                stride_values.len()
            );
            continue;
        }

        let grid_width = (input_width / stride).max(1);
        let grid_height = (input_height / stride).max(1);
        let expected_anchors = grid_width * grid_height;
        if expected_anchors != bbox.nrows() {
            println!(
                "YuNet analytics: stride {} anchor count {} mismatch grid {}x{}",
                stride,
                bbox.nrows(),
                grid_width,
                grid_height
            );
        }

        if bbox.nrows() != cls.nrows() || bbox.nrows() != obj.nrows() {
            let stride_label = stride_values
                .get(stride_idx)
                .map(|value| value.to_string())
                .unwrap_or_else(|| format!("idx{}", stride_idx));
            println!(
                "YuNet analytics: stride {} shape mismatch bbox={} cls={} obj={}",
                stride_label,
                bbox.nrows(),
                cls.nrows(),
                obj.nrows()
            );
            continue;
        }

        for anchor_idx in 0..bbox.nrows() {
            let bbox_row = bbox.row(anchor_idx);
            let cls_row = cls.row(anchor_idx);
            let obj_row = obj.row(anchor_idx);

            if cls_row.len() == 0 || obj_row.len() == 0 {
                continue;
            }

            let class_logit = if cls_row.len() > 1 {
                cls_row[1]
            } else {
                cls_row[0]
            };
            let obj_logit = obj_row[0];
            let class_prob = sigmoid(class_logit);
            let obj_prob = sigmoid(obj_logit);
            let score = (class_prob * obj_prob).sqrt();

            if !score.is_finite() || score <= 0.0 {
                continue;
            }

            if raw_debug_count < 5 {
                let stride_label = stride_values
                    .get(stride_idx)
                    .map(|value| value.to_string())
                    .unwrap_or_else(|| format!("idx{}", stride_idx));
                let bbox_raw: Vec<f32> = bbox_row
                    .as_slice()
                    .map(|slice| slice.to_vec())
                    .unwrap_or_else(|| bbox_row.iter().copied().collect());
                println!(
					"YuNet raw stride={} anchor={} bbox_row={:?} class_logit={:.3} obj_logit={:.3} class_prob={:.3} obj_prob={:.3}",
					stride_label,
					anchor_idx,
					bbox_raw,
					class_logit,
					obj_logit,
					class_prob,
					obj_prob
				);
                raw_debug_count += 1;
            }

            if let Some(decoded) = decode_bbox_strided(
                bbox_row,
                anchor_idx,
                stride,
                grid_width,
                input_width,
                input_height,
            ) {
                let width = (decoded[2] - decoded[0]).max(0.0);
                let height = (decoded[3] - decoded[1]).max(0.0);
                if width <= 0.0 || height <= 0.0 {
                    continue;
                }

                if !debug_logged && (width < 80.0 || height < 80.0) {
                    let stride_label = stride_values
                        .get(stride_idx)
                        .map(|value| value.to_string())
                        .unwrap_or_else(|| format!("idx{}", stride_idx));
                    println!(
						"YuNet debug stride={} anchor={} decoded_bbox={:?} class_prob={:.3} obj_prob={:.3} score={:.3}",
						stride_label,
						anchor_idx,
						decoded,
						class_prob,
						obj_prob,
						score
					);
                    debug_logged = true;
                }

                boxes.push(decoded);
                scores.push(score);
            }
        }
    }

    if boxes.is_empty() {
        println!("YuNet analytics: strided layout produced no valid boxes");
        return Vec::new();
    }

    let max_score = scores.iter().copied().fold(f32::MIN, f32::max).max(0.0);
    println!(
        "YuNet analytics: strided candidates={} max_score={:.3} threshold={:.2}",
        scores.len(),
        max_score,
        confidence_threshold
    );

    let mut indices: Vec<_> = (0..scores.len()).collect();
    indices.sort_by(|&a, &b| scores[b].partial_cmp(&scores[a]).unwrap_or(Ordering::Equal));
    if indices.len() > top_k {
        indices.truncate(top_k);
    }

    let keep = non_max_suppression(
        &boxes,
        &scores,
        &indices,
        nms_threshold,
        keep_top_k,
        confidence_threshold,
    );
    println!(
        "YuNet analytics: kept {} boxes after NMS (strided)",
        keep.len()
    );

    keep.into_iter()
        .map(|idx| {
            [
                boxes[idx][0],
                boxes[idx][1],
                boxes[idx][2],
                boxes[idx][3],
                scores[idx],
            ]
        })
        .collect()
}

fn decode_bbox_strided(
    row: ArrayView1<f32>,
    anchor_idx: usize,
    stride: usize,
    grid_width: usize,
    input_width: usize,
    input_height: usize,
) -> Option<[f32; 4]> {
    if row.len() < 4 || stride == 0 || grid_width == 0 {
        return None;
    }

    let input_width_f = input_width as f32;
    let input_height_f = input_height as f32;

    let raw: [f32; 4] = [row[0], row[1], row[2], row[3]];
    let looks_normalized = raw
        .iter()
        .all(|value| value.is_finite() && value.abs() <= 4.0);

    if looks_normalized {
        let mut x1 = raw[0] * input_width_f;
        let mut y1 = raw[1] * input_height_f;
        let mut x2 = raw[2] * input_width_f;
        let mut y2 = raw[3] * input_height_f;

        if !x1.is_finite() || !y1.is_finite() || !x2.is_finite() || !y2.is_finite() {
            return None;
        }

        if x2 < x1 {
            std::mem::swap(&mut x1, &mut x2);
        }
        if y2 < y1 {
            std::mem::swap(&mut y1, &mut y2);
        }

        x1 = x1.clamp(0.0, input_width_f);
        x2 = x2.clamp(0.0, input_width_f);
        y1 = y1.clamp(0.0, input_height_f);
        y2 = y2.clamp(0.0, input_height_f);

        if x2 <= x1 || y2 <= y1 {
            return None;
        }

        return Some([x1, y1, x2, y2]);
    }

    let stride_f = stride as f32;
    let col = (anchor_idx % grid_width) as f32;
    let row_idx = (anchor_idx / grid_width) as f32;

    let tx = row[0];
    let ty = row[1];
    let tw = row[2];
    let th = row[3];

    let cx = ((sigmoid(tx) * 2.0) - 0.5 + col) * stride_f;
    let cy = ((sigmoid(ty) * 2.0) - 0.5 + row_idx) * stride_f;
    let w = (sigmoid(tw) * 2.0).powi(2) * stride_f;
    let h = (sigmoid(th) * 2.0).powi(2) * stride_f;

    if !cx.is_finite() || !cy.is_finite() || !w.is_finite() || !h.is_finite() {
        return None;
    }
    if w <= 0.0 || h <= 0.0 {
        return None;
    }

    let half_w = w / 2.0;
    let half_h = h / 2.0;
    let mut x1 = cx - half_w;
    let mut y1 = cy - half_h;
    let mut x2 = cx + half_w;
    let mut y2 = cy + half_h;

    x1 = x1.clamp(0.0, input_width_f);
    x2 = x2.clamp(0.0, input_width_f);
    y1 = y1.clamp(0.0, input_height_f);
    y2 = y2.clamp(0.0, input_height_f);

    if x2 <= x1 || y2 <= y1 {
        return None;
    }

    Some([x1, y1, x2, y2])
}

fn decode_bbox_from_row(
    row: ArrayView1<f32>,
    input_width: f32,
    input_height: f32,
) -> Option<[f32; 4]> {
    if let Some(slice) = row.as_slice() {
        return decode_bbox_from_slice(slice, input_width, input_height);
    }

    let values: Vec<f32> = row.iter().copied().collect();
    decode_bbox_from_slice(&values, input_width, input_height)
}

fn decode_bbox_from_slice(values: &[f32], input_width: f32, input_height: f32) -> Option<[f32; 4]> {
    if values.len() >= 8 {
        return decode_bbox_from_points(values, input_width, input_height);
    }
    if values.len() >= 4 {
        return decode_bbox_from_four(values, input_width, input_height);
    }
    None
}

fn decode_bbox_from_points(
    values: &[f32],
    input_width: f32,
    input_height: f32,
) -> Option<[f32; 4]> {
    let limit = values.len().min(16);
    if limit < 8 {
        return None;
    }

    let normalized = values[..limit]
        .iter()
        .all(|coord| coord.is_finite() && coord.abs() <= 2.0);

    let mut min_x = f32::MAX;
    let mut min_y = f32::MAX;
    let mut max_x = f32::MIN;
    let mut max_y = f32::MIN;

    for idx in (0..limit).step_by(2) {
        if idx + 1 >= limit {
            break;
        }
        let mut x = values[idx];
        let mut y = values[idx + 1];
        if normalized {
            x *= input_width;
            y *= input_height;
        }
        if !x.is_finite() || !y.is_finite() {
            continue;
        }
        min_x = min_x.min(x);
        min_y = min_y.min(y);
        max_x = max_x.max(x);
        max_y = max_y.max(y);
    }

    if !min_x.is_finite() || !min_y.is_finite() || !max_x.is_finite() || !max_y.is_finite() {
        return None;
    }

    if max_x <= min_x || max_y <= min_y {
        return None;
    }

    Some([min_x, min_y, max_x, max_y])
}

fn decode_bbox_from_four(values: &[f32], input_width: f32, input_height: f32) -> Option<[f32; 4]> {
    if values.len() < 4 {
        return None;
    }
    let slice = &values[..4];
    let normalized = slice
        .iter()
        .all(|coord| coord.is_finite() && coord.abs() <= 2.0);

    let (scale_x, scale_y) = if normalized {
        (input_width, input_height)
    } else {
        (1.0, 1.0)
    };

    let x1 = slice[0] * scale_x;
    let y1 = slice[1] * scale_y;
    let x2 = slice[2] * scale_x;
    let y2 = slice[3] * scale_y;

    if x2 > x1 && y2 > y1 {
        return Some([x1, y1, x2, y2]);
    }

    let cx = slice[0] * scale_x;
    let cy = slice[1] * scale_y;
    let w = slice[2].abs() * scale_x;
    let h = slice[3].abs() * scale_y;

    if w <= 0.0 || h <= 0.0 {
        return None;
    }

    let x1 = cx - w / 2.0;
    let y1 = cy - h / 2.0;
    let x2 = cx + w / 2.0;
    let y2 = cy + h / 2.0;

    if x2 > x1 && y2 > y1 {
        Some([x1, y1, x2, y2])
    } else {
        None
    }
}

#[inline]
fn sigmoid(x: f32) -> f32 {
    1.0 / (1.0 + (-x).exp())
}

fn non_max_suppression(
    boxes: &[[f32; 4]],
    scores: &[f32],
    order: &[usize],
    threshold: f32,
    keep_top_k: usize,
    confidence_threshold: f32,
) -> Vec<usize> {
    let mut keep = Vec::new();
    for &idx in order {
        if scores[idx] < confidence_threshold {
            continue;
        }

        let mut suppressed = false;
        for &kept in &keep {
            if iou(&boxes[idx], &boxes[kept]) > threshold {
                suppressed = true;
                break;
            }
        }

        if !suppressed {
            keep.push(idx);
            if keep.len() >= keep_top_k {
                break;
            }
        }
    }
    keep
}

fn iou(a: &[f32; 4], b: &[f32; 4]) -> f32 {
    let x1 = a[0].max(b[0]);
    let y1 = a[1].max(b[1]);
    let x2 = a[2].min(b[2]);
    let y2 = a[3].min(b[3]);

    let inter_w = (x2 - x1).max(0.0);
    let inter_h = (y2 - y1).max(0.0);
    let inter_area = inter_w * inter_h;

    if inter_area <= 0.0 {
        return 0.0;
    }

    let area_a = (a[2] - a[0]).max(0.0) * (a[3] - a[1]).max(0.0);
    let area_b = (b[2] - b[0]).max(0.0) * (b[3] - b[1]).max(0.0);
    inter_area / (area_a + area_b - inter_area).max(1e-6)
}

fn to_detection_box(index: usize, det: [f32; 5], scale_x: f32, scale_y: f32) -> DetectionBox {
    let x1 = det[0] * scale_x;
    let y1 = det[1] * scale_y;
    let x2 = det[2] * scale_x;
    let y2 = det[3] * scale_y;
    let width = (x2 - x1).max(0.0);
    let height = (y2 - y1).max(0.0);

    DetectionBox {
        id: format!("face-{}", index),
        label: "Face".to_string(),
        confidence: det[4].clamp(0.0, 1.0),
        bounds: BoundingBox {
            x: x1,
            y: y1,
            width,
            height,
        },
        color: "#ff7f50".to_string(),
    }
}

fn resolve_input_shape(session: &Session) -> StdResult<(usize, usize), String> {
    let input = session
        .inputs
        .first()
        .ok_or_else(|| "YuNet model exposes no inputs".to_string())?;

    let dims: Vec<Option<usize>> = input.dimensions().collect();

    let height = dims
        .get(dims.len().saturating_sub(2))
        .and_then(|d| *d)
        .ok_or_else(|| "YuNet input height is dynamic or unknown".to_string())?;
    let width = dims
        .last()
        .and_then(|d| *d)
        .ok_or_else(|| "YuNet input width is dynamic or unknown".to_string())?;

    Ok((width, height))
}

fn resolve_output_layout(session: &Session) -> StdResult<YuNetOutputLayout, String> {
    let mut bbox = [usize::MAX; 3];
    let mut cls = [usize::MAX; 3];
    let mut obj = [usize::MAX; 3];
    let mut loc = usize::MAX;
    let mut conf = usize::MAX;
    let mut iou = usize::MAX;

    for (index, output) in session.outputs.iter().enumerate() {
        let name = if output.name.is_empty() {
            continue;
        } else {
            output.name.as_str()
        };

        let dims: Vec<String> = output
            .dimensions()
            .map(|dim| match dim {
                Some(value) => value.to_string(),
                None => "dyn".to_string(),
            })
            .collect();
        println!(
            "YuNet session output idx={} name='{}' dims={:?}",
            index, name, dims
        );

        match name {
            "bbox_8" => bbox[0] = index,
            "bbox_16" => bbox[1] = index,
            "bbox_32" => bbox[2] = index,
            "cls_8" => cls[0] = index,
            "cls_16" => cls[1] = index,
            "cls_32" => cls[2] = index,
            "obj_8" => obj[0] = index,
            "obj_16" => obj[1] = index,
            "obj_32" => obj[2] = index,
            "loc" | "loc_out" | "locations" => loc = index,
            "conf" | "conf_out" | "confidence" => conf = index,
            "iou" | "iou_out" => iou = index,
            _ => {}
        }
    }

    let strided_ready = bbox.iter().all(|&idx| idx != usize::MAX)
        && cls.iter().all(|&idx| idx != usize::MAX)
        && obj.iter().all(|&idx| idx != usize::MAX);
    if strided_ready {
        return Ok(YuNetOutputLayout::Strided { bbox, cls, obj });
    }

    if loc != usize::MAX && conf != usize::MAX && iou != usize::MAX {
        return Ok(YuNetOutputLayout::Legacy { loc, conf, iou });
    }

    Err("YuNet model outputs do not match expected layouts (legacy or strided)".to_string())
}

fn get_environment(module_dir: &Path) -> StdResult<Arc<Environment>, String> {
    ORT_ENVIRONMENT
        .get_or_try_init(|| {
            configure_ort_runtime(module_dir)?;
            Environment::builder()
                .with_name("vms-analytics")
                .with_log_level(LoggingLevel::Warning)
                .build()
                .map(|env| env.into_arc())
                .map_err(|err| err.to_string())
        })
        .cloned()
}

fn configure_ort_runtime(module_dir: &Path) -> StdResult<(), String> {
    if std::env::var("ORT_DYLIB_PATH").is_ok() {
        return Ok(());
    }

    if let Some(path) = locate_runtime_library(module_dir) {
        std::env::set_var("ORT_DYLIB_PATH", &path);
        Ok(())
    } else {
        Err("ONNX Runtime dynamic library not found. Set ORT_DYLIB_PATH or place the runtime alongside the model.".to_string())
    }
}

fn locate_runtime_library(module_dir: &Path) -> Option<String> {
    let candidates = candidate_runtime_paths(module_dir);
    for path in candidates {
        if path.exists() {
            return Some(path.display().to_string());
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
            format!("libonnxruntime.so.{}", onnx_runtime_version_str()),
        ]
    }

    #[cfg(target_os = "macos")]
    {
        vec![
            "libonnxruntime.dylib".to_string(),
            format!("libonnxruntime.{}.dylib", onnx_runtime_version_str()),
        ]
    }

    #[cfg(not(any(target_os = "windows", target_os = "linux", target_os = "macos")))]
    {
        vec!["libonnxruntime.so".to_string()]
    }
}
