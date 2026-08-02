# Architecture

## Runtime layers

- QML owns presentation and user intent. It does not persist secrets.
- `SystemController` coordinates camera models, state, discovery and device services.
- `GstPlayer` owns one playback pipeline per visible camera. Analytics consumes the same decoded frames instead of opening a second RTSP session.
- Each camera has at most one dedicated recorder. It remuxes the compressed camera stream to MP4 and finalizes it with EOS.
- `AnalyticsEngine` schedules inference, evidence capture and the SQLite analytics event store.

## Stream lifecycle

`GstPlayer` exposes `idle`, `connecting`, `streaming`, `reconnecting` and `authentication-error` states. Network failures use a 1, 2, 4, 8, 16, 30 second bounded backoff. Thirty stable frames reset the attempt counter. A frame watchdog detects pipelines that remain alive but stop producing video. HTTP/RTSP authentication errors do not create an infinite retry loop.

The grid prefers SD for normal multi-camera viewing, switches to HD for fullscreen or analytics, and falls back to SD when HD fails. Credentials are injected into an in-memory URL immediately before pipeline creation.

Manual and analytics-triggered recording share a single recorder state machine, so they cannot open two competing HD recording sessions. If an event owns the recorder, a manual request is queued until the event MP4 is finalized. If a manual recording is already active, analytics writes the event from its decoded pre/post-event frame buffer to the event's original path. Direct stream recordings retain camera audio; the buffered fallback is video-only.

## Persistence

Application state is stored in `state.sqlite3` using WAL-capable SQLite and explicit `schema_migrations`. Updates use a database transaction and a singleton `app_state` record. On first run after an upgrade, valid legacy `state.json` data is imported, credential fields are removed, and the plaintext legacy files are deleted after the database commit succeeds.

Analytics events use a separate indexed SQLite store so high-volume event history does not rewrite application state.

## Sites and fleet management

`FleetManager` owns the non-secret `Site → Area` topology, camera assignments and tags, saved
views, configuration baselines, inventory metadata and bounded batch history. Its JSON state is
stored inside the same transactional application-state record; camera credentials remain in the
operating-system credential store and are never part of fleet import/export.

The service resolves `site:<id>`, `area:<id>` and `tag:<value>` aliases for `UserManager`, so every
existing per-camera authorization check also understands fleet scopes. QML only submits intent:
the backend repeats permission, camera-scope, compatibility, maintenance-window and backup checks
before a batch device is dispatched. Configuration snapshots and diffs are recursively redacted.

Read operations use a bounded queue. Configuration mutation is limited to an explicit baseline,
requires a successful per-device backup, retains partial results and recovery guidance, and never
attempts automatic firmware rollback. Fleet administration is a native Qt workspace; the Web
companion remains focused on monitoring and archive workflows.

## Incident event foundation

`IncidentManager` is the P13 normalization boundary between operational event producers and future
incident workflows. Analytics, Health, Archive/Recording and Fleet Audit adapters submit source
payloads; the manager resolves camera/Site/Area scope, redacts nested secret-like fields, assigns a
schema-v1 identity and derives read-only five-minute correlation groups. The bounded event set is
persisted through the same transactional state document as the rest of the controller.

Lifecycle transitions and outbound notifications must consume this normalized contract rather than
coupling directly to source-specific payloads. See `docs/INCIDENT_EVENTS.md`.

## Extension points

- `MajesticClient` is a schema-driven OpenIPC device service. It discovers the live Majestic configuration/schema, flattens supported fields for QML, creates minimal nested patches, validates them against the camera schema, redacts sensitive diffs, and owns runtime commands, metrics and backup I/O. Older firmware without `config.schema.json` is exposed read-only rather than guessed.
- `NetworkDiscoveryService` combines official OpenIPC mDNS markers, ONVIF WS-Discovery and bounded Majestic/WebUI/RTSP probing. Candidates carry confidence and evidence and are merged with Dahua SDK results by IP.
- `CameraOnboardingParser` accepts `openipc://` and RTSP QR payloads without embedding credentials in persisted URLs.
- `ModelArtifactVerifier` is the trust boundary for downloaded inference artifacts.
- `ReconnectPolicy` is independent of GStreamer and unit tested.
