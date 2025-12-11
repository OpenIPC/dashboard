use std::collections::{HashMap, HashSet};

use chrono::{DateTime, Utc};
use rand::{distributions::Alphanumeric, Rng};

use super::{bounding_box_iou, BoundingBox, DetectionBox, DetectionEventType};

const DEFAULT_IOU_THRESHOLD: f32 = 0.32;
const DEFAULT_MAX_MISSED_FRAMES: u32 = 8;

#[derive(Debug, Clone, Copy)]
#[derive(Default)]
struct FaceTrackerConfig {
    iou_threshold: f32,
    max_missed_frames: u32,
}

#[derive(Debug, Clone)]
struct TrackState {
    id: String,
    bounds: BoundingBox,
    previous_bounds: Option<BoundingBox>,
    confidence: f32,
    first_seen: DateTime<Utc>,
    last_seen: DateTime<Utc>,
    missed_frames: u32,
    total_updates: u64,
}

impl TrackState {
    fn new(id: String, detection: &DetectionBox, timestamp: DateTime<Utc>) -> Self {
        Self {
            id,
            bounds: detection.bounds.clone(),
            previous_bounds: None,
            confidence: detection.confidence,
            first_seen: timestamp,
            last_seen: timestamp,
            missed_frames: 0,
            total_updates: 1,
        }
    }

    fn update(&mut self, detection: &DetectionBox, timestamp: DateTime<Utc>) {
        self.previous_bounds = Some(self.bounds.clone());
        self.bounds = detection.bounds.clone();
        self.confidence = detection.confidence;
        self.last_seen = timestamp;
        self.missed_frames = 0;
        self.total_updates = self.total_updates.saturating_add(1);
    }

    fn dwell_ms(&self) -> u64 {
        self.last_seen
            .signed_duration_since(self.first_seen)
            .num_milliseconds()
            .max(0) as u64
    }
}

#[derive(Default)]
pub struct FaceTracker {
    tracks: HashMap<String, TrackState>,
    config: FaceTrackerConfig,
}

impl FaceTracker {
    pub fn new() -> Self {
        Self {
            tracks: HashMap::new(),
            config: FaceTrackerConfig {
                iou_threshold: DEFAULT_IOU_THRESHOLD,
                max_missed_frames: DEFAULT_MAX_MISSED_FRAMES,
            },
        }
    }

    pub fn assign(
        &mut self,
        detections: Vec<DetectionBox>,
        timestamp: DateTime<Utc>,
    ) -> Vec<DetectionBox> {
        if detections.is_empty() {
            self.increment_missed_frames();
            self.retire_stale_tracks();
            return detections;
        }

        let mut assigned: HashSet<String> = HashSet::new();
        let mut enriched: Vec<DetectionBox> = Vec::with_capacity(detections.len());

        for mut detection in detections.into_iter() {
            let maybe_track = self.match_track(&detection, &assigned);
            let (track_id, event_type) = match maybe_track {
                Some(track_id) => {
                    if let Some(track) = self.tracks.get_mut(&track_id) {
                        let was_new = track.total_updates <= 1;
                        track.update(&detection, timestamp);
                        let event = if was_new {
                            DetectionEventType::Entered
                        } else {
                            DetectionEventType::Updated
                        };
                        (track_id, event)
                    } else {
                        let new_id = self.spawn_track(&detection, timestamp);
                        (new_id, DetectionEventType::Entered)
                    }
                }
                None => {
                    let new_id = self.spawn_track(&detection, timestamp);
                    (new_id, DetectionEventType::Entered)
                }
            };

            assigned.insert(track_id.clone());

            if let Some(track) = self.tracks.get(&track_id) {
                detection.track_id = Some(track.id.clone());
                detection.previous_bounds = track.previous_bounds.clone();
                detection.first_seen_at = Some(track.first_seen.to_rfc3339());
                detection.last_seen_at = Some(track.last_seen.to_rfc3339());
                detection.dwell_ms = Some(track.dwell_ms());
                detection.event_type = Some(event_type);
            }

            enriched.push(detection);
        }

        self.increment_missed_frames_for_unassigned(&assigned);
        self.retire_stale_tracks();

        enriched
    }

    fn match_track(&self, detection: &DetectionBox, assigned: &HashSet<String>) -> Option<String> {
        let mut best_id: Option<String> = None;
        let mut best_score = 0.0f32;

        // First pass: IoU matching
        for (track_id, track) in self.tracks.iter() {
            if assigned.contains(track_id) {
                continue;
            }

            let iou = bounding_box_iou(&track.bounds, &detection.bounds);
            if iou >= self.config.iou_threshold && iou > best_score {
                best_score = iou;
                best_id = Some(track_id.clone());
            }
        }

        if best_id.is_some() {
            return best_id;
        }

        // Second pass: Distance-based matching for fast moving objects
        // If IoU failed (likely due to high speed/low fps), check if the object is reasonably close
        // relative to its size.
        let mut min_dist_sq = f32::MAX;
        let cx = detection.bounds.x + detection.bounds.width / 2.0;
        let cy = detection.bounds.y + detection.bounds.height / 2.0;
        
        // Allow movement up to 3x the object's diagonal size
        let diag_sq = detection.bounds.width.powi(2) + detection.bounds.height.powi(2);
        let threshold_sq = diag_sq * 9.0; 

        for (track_id, track) in self.tracks.iter() {
            if assigned.contains(track_id) {
                continue;
            }

            // Only consider tracks that were seen recently (e.g. within last 2 frames)
            // to avoid jumping to old stale tracks
            if track.missed_frames > 2 {
                continue;
            }

            let tcx = track.bounds.x + track.bounds.width / 2.0;
            let tcy = track.bounds.y + track.bounds.height / 2.0;
            
            let dx = cx - tcx;
            let dy = cy - tcy;
            let dist_sq = dx*dx + dy*dy;

            if dist_sq < threshold_sq && dist_sq < min_dist_sq {
                min_dist_sq = dist_sq;
                best_id = Some(track_id.clone());
            }
        }

        best_id
    }

    fn spawn_track(&mut self, detection: &DetectionBox, timestamp: DateTime<Utc>) -> String {
        let id = self.generate_track_id();
        let state = TrackState::new(id.clone(), detection, timestamp);
        self.tracks.insert(id.clone(), state);
        id
    }

    fn generate_track_id(&self) -> String {
        let mut rng = rand::thread_rng();
        let suffix: String = (&mut rng)
            .sample_iter(&Alphanumeric)
            .take(6)
            .map(char::from)
            .collect();
        format!("track-{}", suffix.to_lowercase())
    }

    fn increment_missed_frames(&mut self) {
        for track in self.tracks.values_mut() {
            track.missed_frames = track.missed_frames.saturating_add(1);
        }
    }

    fn increment_missed_frames_for_unassigned(&mut self, assigned: &HashSet<String>) {
        for (track_id, track) in self.tracks.iter_mut() {
            if !assigned.contains(track_id) {
                track.missed_frames = track.missed_frames.saturating_add(1);
            }
        }
    }

    fn retire_stale_tracks(&mut self) {
        self.tracks
            .retain(|_, track| track.missed_frames <= self.config.max_missed_frames);
    }
}
