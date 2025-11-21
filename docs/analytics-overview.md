# Video Analytics Implementation Overview

## Current Capabilities
- `src-tauri/src/analytics.rs` provides module lifecycle management (enable/disable, manifest persistence) and a placeholder `process_frame` command.
- Built-in module descriptors: face-detector, license-plate-detector, object-counter. Status data travels via `analytics_list_modules`.
- Frontend settings (`src/components/SettingsModal.tsx`) already surface module metadata and toggles, but they operate on mock state only.
- The Analytics page (`src/components/Analytics.tsx`) renders static charts with hard-coded sample data.

## Gaps to Close
- Wire Tauri analytics commands into the React application (list modules, enable/disable, request detections).
- Persist module enablement in shared state (context) so that module status is reflected across UI.
- Implement ingestion of camera frames for analytics processing (decide on source: live video tiles, RTSP snapshots, or server-side captures).
- Replace placeholder analytics visuals with live data (detections, event counts, heatmaps, etc.).
- Surface detections in real-time UI components (overlays, notifications, archive tagging).
- Add robust error handling for module downloads, runtime failures, and resource constraints.

## Open Questions
- Where will the analytics runtime binaries live (local bundle vs. on-demand download)?
- What is the desired cadence for frame sampling and processing per module/camera?
- Should detections be stored in the existing archive system or in a dedicated store for analytics events?
- What minimum viable set of visualizations is required for the first release?

## Next Steps
1. Define frontend service wrappers around the available Tauri commands with strict typing.
2. Introduce shared state (context or tanstack query) to cache module statuses and expose operations to UI/components.
3. Prototype frame capture pipeline (e.g., capture single frame from active player, encode to Base64, call `analytics_process_frame`).
4. Replace mock Analytics page with widgets sourced from real detection/event data.
5. Extend camera tiles/overlays to reflect detection results in real time (highlight bounding boxes, counters, etc.).
6. Document operational workflows (module installation, fallback paths) for end users.
