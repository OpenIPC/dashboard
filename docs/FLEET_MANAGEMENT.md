# Sites and Fleet Management

P12 adds a native Qt workspace for organizing and maintaining larger camera deployments without
requiring an external cloud service. Open **Actions → Fleet** in the desktop application.

## Data model

- **Site** is a physical deployment or administrative boundary.
- **Area** belongs to one Site and describes a floor, entrance, production zone, or similar unit.
- Existing camera **Group** remains available alongside fleet topology.
- Camera **Tags** provide flexible cross-site labels such as `perimeter`, `critical`, or `indoor`.
- A camera can have one Site, one Area, multiple tags, and an `active`, `maintenance`, or `retired`
  lifecycle state.

Assignments, saved views, version baselines, redacted configuration snapshots and bounded batch
history are persisted in `state.sqlite3`. Credentials are not stored in fleet state.

## Inventory and access scope

Inventory combines camera identity, model, Site/Area/Group/Tags, firmware and Majestic versions,
capabilities, health, last-seen, maintenance and drift state. It can be filtered and exported as
CSV or JSON. Diagnostic export is JSON and contains no camera credentials.

User camera scopes accept direct camera identifiers and the aliases `site:<id>`, `area:<id>` and
`tag:<value>`. The backend applies the same authorization to inventory and every fleet operation;
the QML filter is not treated as a security boundary. Saved views are owned by their creator, with
administrator override.

## Configuration baselines and drift

An administrator can capture a Majestic baseline from an accessible OpenIPC camera. The stored
configuration and drift preview recursively remove password, token, secret, PSK, API-key and
private-key fields. Drift is a report only until an explicit batch operation is requested.

Before applying a baseline, Dashboard checks every selected camera for:

- camera scope and Settings permission;
- OpenIPC/Majestic compatibility;
- a valid baseline and Site assignment;
- the Site maintenance window, unless an explicit override is chosen;
- backup-before-change and a valid local backup directory for a real run.

Use **dry-run** first. A real run reads the current configuration again, writes a unique per-device
backup, applies only the nested difference, and records success, skip, cancellation, or failure for
each device. Cancellation stops new dispatches; an already in-flight request is allowed to finish
safely. Keep backups until the cameras have been verified manually.

Automatic fleet firmware upgrades and rollback are intentionally excluded until a real-camera
compatibility and recovery matrix exists.

## Moving topology between servers

Site-definition export contains Sites, Areas, assignments, saved views and sanitized baselines.
It does not contain passwords or tokens. Import first produces a conflict preview; definitions can
then be merged explicitly. Files containing credential-like fields are rejected rather than
silently trusted.
