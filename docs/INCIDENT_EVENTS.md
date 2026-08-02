# Incident Event Foundation

P13.1 introduces one bounded, credential-safe event contract for Health, Analytics, Archive,
Recording and Fleet Audit sources. It is deliberately a foundation: incident lifecycle changes,
operator comments, notification routing and external delivery begin in later P13 packages.

## Normalized schema v1

Every event exposed by `IncidentManager` contains:

- `schemaVersion`, normalized event `id`, `sourceEventId` and `fingerprint`;
- `source`, `category`, `type` and `severity` (`info`, `warning`, `error`, `critical`);
- UTC `occurredAt`, `occurredAtMs` and `receivedAt` timestamps;
- optional `cameraId`, `cameraIp`, `siteId` and `areaId` scope;
- bounded `title`, `message`, `actor`, `evidence` and `attributes`;
- a `correlationKey` derived from camera or Site scope, category, type and a five-minute window.

The machine-readable contract is `docs/INCIDENT_EVENT_SCHEMA_V1.json`.

Source identifiers make ingestion idempotent. Events are retained newest-first with a hard limit of
5,000 records. Persistence uses the existing transactional `state.sqlite3` state document; no
additional plaintext credential store is created.

## Source adapters

- Analytics detections and rules retain their module metadata and snapshot/clip references.
- A completed Health run produces one normalized event per camera result.
- Archive export and Recording lifecycle/error callbacks produce operational events.
- Fleet audit actions carry actor, outcome and resolved Site/Area scope.

Correlation groups are read-only derived projections over normalized events. They are not yet
operator incidents and have no mutable lifecycle state.

## Security boundary

Nested keys containing password, credential, token, secret, PSK, API key, private key, authorization or cookie
material are replaced with `[redacted]`. Maps, lists, key names, strings and nesting depth are
bounded before persistence or presentation. Notification adapters must consume normalized events;
they must never read camera credentials directly.

## Next package

P13.2 builds the incident lifecycle (`new`, `acknowledged`, `investigating`, `resolved`,
`false positive`) and immutable activity timeline on top of schema v1 without changing source
adapters.
