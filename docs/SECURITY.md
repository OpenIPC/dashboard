# Security

## Secrets

- Remembered application passwords use QtKeychain and the native OS credential store.
- Camera passwords use a separate keychain namespace keyed by camera address.
- Persisted RTSP URLs are stripped of user information. Credentials are injected only into the in-memory URL passed to GStreamer.
- Legacy plaintext login and camera credentials are migrated and removed.

Ordinary application exports must not include passwords or OAuth client secrets. A Majestic backup is an explicit exception: it mirrors the camera's complete configuration and can contain RTMP stream keys or firmware fields such as plaintext network passwords. The UI warns before export; store these files like credentials. Diff previews redact password, secret, token and outgoing-server values.

Majestic commonly exposes HTTP Basic authentication over plain HTTP on the camera LAN. Dashboard does not downgrade an HTTPS endpoint, but it cannot add transport encryption to an HTTP-only firmware. Use a trusted management network, a VPN/tunnel, or an HTTPS reverse proxy for untrusted paths.

## Fleet operations

- `site:`, `area:` and `tag:` scopes are resolved in the backend and combined with the existing
  per-camera authorization check; filtering the Qt list is not considered authorization.
- Site topology import/export is administrator-only. Imports containing credential-like keys are
  rejected, and exports recursively strip password, token, secret, credential, PSK, API-key and
  private-key fields.
- Configuration snapshots, baselines, drift details and diagnostics use the same recursive
  redaction. Users without Settings permission cannot read baselines or configuration snapshots.
- A real baseline apply requires a compatible OpenIPC/Majestic device, an allowed camera scope,
  the maintenance window (or explicit override), and a successful per-device backup before write.
- Batch audit retains selection, outcome and recovery guidance but removes the local backup
  directory from persisted history. Automatic fleet firmware update/rollback remains disabled.

## Downloads

AI model URLs are pinned to a commit or release. Every artifact has an expected byte size and SHA-256 digest. Downloads go to a `.part` file, are verified, and are promoted with rollback protection. A failed verification never replaces an installed model.

TLS errors abort network operations; certificates are not silently accepted.

## Reporting

Do not include passwords, camera URLs with user information, access tokens, recordings or customer network layouts in a public issue. Provide sanitized logs and a minimal reproduction.

## Web deployment threat model

Protected assets are camera credentials, recordings, device-control authority, user accounts,
session tokens and the host filesystem. Relevant attackers are an unauthenticated LAN client, a
compromised browser origin, a malicious configuration import and a client spoofing reverse-proxy
headers.

Security boundaries:

- `localhost` is the default; LAN/VPN/reverse-proxy exposure requires an explicit profile;
- Dashboard never treats `Forwarded` or `X-Forwarded-*` as identity evidence;
- an external HTTPS Origin is accepted only from an exact configured proxy peer and exact external
  base URL;
- reverse-proxy deployment forces secure cookies; mutations still require the session, permission,
  same-origin/CSRF check, bounded input and audit;
- browser imports cannot mutate the listener, deployment profile or trusted proxy list;
- session administration exposes origin/peer and expiry metadata, never the raw token;
- the public liveness/readiness responses contain bounded operational state and no secrets.
- camera scope is applied to Qt views and enforced again on camera-specific HTTP, archive and
  WebSocket media/control paths; desktop/Web push-to-talk requires its own permission and bounded
  PCM transport;

Residual boundaries: plain HTTP on a trusted LAN does not provide confidentiality; use VPN or an
HTTPS reverse proxy. Dashboard has no public cloud relay and public NAT traversal/STUN/TURN remains
disabled until the credential, relay and interoperability design is qualified. See
[Secure Web deployment](WEB_DEPLOYMENT.md).
