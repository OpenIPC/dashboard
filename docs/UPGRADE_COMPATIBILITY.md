# Upgrade compatibility

## v0.2.8 qualification matrix

| Source | Target | State covered | Automated gate | Result |
| --- | --- | --- | --- | --- |
| v0.2.6.1 | v0.2.8 | cameras, groups, archive path, Web settings and credential references | `state_store_tests` previous-release fixture | Pass |
| v0.2.7 | v0.2.8 | layouts, archive path, LAN bind, HTTP/WebSocket ports and application settings | `state_store_tests` previous-release fixture | Pass |
| legacy user hash | v0.2.8 | administrator permissions and SHA-256 → PBKDF2-SHA256 migration | `user_manager_tests` | Pass |
| fresh profile | v0.2.8 | schema creation, first/repeated server start and one-shot headless administrator bootstrap | `state_store_tests`, server/browser smoke | Pass |

The state store opens the previous schema transactionally, preserves unknown forward-compatible
JSON fields and writes the current schema only after the new payload is durable. Camera passwords
remain in the OS credential store; state contains references, not secret values.

## Runtime and recovery budgets

The package gate records structured JSON for server startup, port-conflict rejection, shutdown and
restart. The release limits are:

- readiness endpoint: within 20 seconds (the measured value is included as `timeToReadyMs`);
- graceful test shutdown: within 5 seconds;
- port conflict: the second process must fail rather than silently select another port;
- recovery: the same profile and ports must become ready after the first process exits;
- QML startup: within the 45-second smoke timeout at 100%, 125%, 150% and 200% scaling.

Time-to-first-frame and reconnect-storm telemetry depend on a real camera/codec/GPU combination.
They remain part of the sanitized runtime report and real-camera qualification, not a synthetic
claim made by the package test.

## Rollback and backup

Before upgrading a production host, export browser-safe configuration and keep the existing
installer/AppImage. Exports intentionally omit passwords, OAuth secrets and session tokens. A
rollback must reuse the existing application-data and credential-store locations; do not delete
the SQLite database or `users.json` as a troubleshooting shortcut.

If startup fails, collect the diagnostic bundle and the `health/live` / `health/ready` responses.
The [runtime issue form](../.github/ISSUE_TEMPLATE/runtime-report.yml) lists the required sanitized
platform, Qt/GStreamer, GPU and reproduction data.
