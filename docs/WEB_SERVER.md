# OpenIPC Dashboard Web Server

The P6 web platform is an embedded companion server. It exposes the existing Dashboard state to a browser without creating a second configuration database or moving camera credentials into JavaScript.

## Start and configure

Open **Settings > Web** in the desktop application, enable the server and save the settings. The default endpoint is:

```text
http://127.0.0.1:8080
```

The same executable can run without loading the QML window:

```bash
appOpenIPC-Dashboard --server-only
```

Server-only mode uses the saved Web settings but starts the HTTP listener even when automatic startup is disabled. The initial administrator must be created once in the desktop UI before browser login is possible.

Only one Dashboard process can listen on a configured HTTP/WebSocket port pair. Do not run the desktop Web server and a separate `--server-only` instance on the same ports. If the settings page reports `The bound address is already in use`, close the previous Dashboard process or assign a different port pair, then apply and restart the server.

## Network model

- Remote access is disabled by default. The server binds to `127.0.0.1` even if another bind address remains in old settings.
- Enabling LAN access makes the configured bind address effective. Use `0.0.0.0` to listen on all IPv4 interfaces.
- Do not forward the plain HTTP port from a router to the Internet.
- For remote access, prefer WireGuard/Tailscale/another VPN or an HTTPS reverse proxy with strict network allowlists.
- Enable **Secure cookie** only when browser traffic reaches Dashboard through HTTPS. A browser correctly refuses to send that cookie over plain HTTP.

The server does not enable CORS. Browser API access is same-origin by design.

## Authentication and permissions

The web server uses the desktop user database and the same permission mask:

| API area | Required permission |
| --- | --- |
| Dashboard, cameras and health status | Live View |
| Start health checks; discover, add, edit and delete cameras | Settings |
| Archive list and playback | Playback |
| Archive/snapshot download | Export |
| Analytics modules and events | Analytics |
| PTZ commands | PTZ |
| Recording and monitor controls | Live View |
| Settings, logs, diagnostics and device administration | Settings |
| User and active-session administration | User Manage |

Session tokens contain 256 random bits. Only their SHA-256 digests are held in memory. Browser cookies are `HttpOnly`, `SameSite=Strict` and use a bounded, sliding expiration with a 24-hour absolute lifetime. Password, permission and user changes invalidate the affected user's web sessions without interrupting unrelated operators. Administrators can inspect opaque session IDs and revoke an individual session; raw tokens are never listed.

Five failed logins from one address within a minute block that address for five minutes. Authenticated mutations are limited to 60 requests per session/address per minute. State-changing cookie requests require `X-OpenIPC-CSRF: 1`; requests with an `Origin` header must match the server host. Confirmed device mutations and Majestic apply requests additionally require an `Idempotency-Key`.

## API v1

Public:

- `GET /api/v1/server`
- `POST /api/v1/auth/login`

Authenticated:

- `GET /api/v1/auth/session`
- `POST /api/v1/auth/logout`
- `GET /api/v1/presentation`
- `GET /api/v1/dashboard`
- `GET /api/v1/cameras`
- `GET /api/v1/discovery`
- `GET /api/v1/cameras/<index>/preview.mjpeg?quality=sd|hd` (continuous browser preview)
- `GET /api/v1/cameras/<index>/preview.jpg?quality=sd|hd` (single-frame fallback)
- `GET /api/v1/cameras/<index>/snapshot.jpg` (opaque browser download)
- `GET /api/v1/recordings/active`
- `POST /api/v1/recording`
- `GET /api/v1/health`
- `POST /api/v1/health/run`
- `POST /api/v1/cameras` (manual onboarding)
- `POST /api/v1/cameras/<index>/update`
- `POST /api/v1/cameras/<index>/delete`
- `POST /api/v1/discovery/start`
- `POST /api/v1/discovery/stop`
- `POST /api/v1/discovery/clear`
- `POST /api/v1/discovery/add`
- `GET /api/v1/analytics`
- `GET /api/v1/archive?camera=&limit=200`
- `GET /api/v1/archive/file/<id>`
- `POST /api/v1/ptz`
- `GET|POST /api/v1/settings`
- `GET /api/v1/users`; `POST /api/v1/users/create|permissions|delete|password`
- `POST /api/v1/sessions/revoke`
- `GET /api/v1/logs`; `POST /api/v1/logs/clear`
- `GET /api/v1/diagnostics`; `GET /api/v1/diagnostics/bundle`
- `GET /api/v1/configuration/export`; `POST /api/v1/configuration/import`
- `POST /api/v1/devices/operation`; `GET /api/v1/devices/operations/<id>`
- `POST /api/v1/devices/majestic/preview|apply`
- `POST /api/v1/camex/preview`

Every JSON response has one of these shapes:

```json
{"ok":true,"data":{}}
```

```json
{"ok":false,"error":"Authentication required"}
```

Browser login sets the `HttpOnly` cookie and does not expose its token to JavaScript. Non-browser API clients may request a Bearer session with `{"username":"...","password":"...","sessionMode":"bearer"}`; that response includes the token for `Authorization: Bearer <session-token>` and does not set a cookie. Archive playback supports `Range: bytes=...` and streams bounded chunks instead of loading a recording into RAM.

## Live updates and video

When Qt WebSockets is available at build time, the server opens the configured WebSocket port and pushes debounced dashboard snapshots. The web UI falls back to five-second HTTP polling when WebSockets are unavailable or disconnected.

Live preview prefers WebRTC negotiated over the authenticated Dashboard WebSocket. Each occupied monitor cell owns an independent peer connection, so a failed camera does not interrupt the other cells. RTSP credentials remain inside the Dashboard process and are never sent to JavaScript.

For H.264 cameras GStreamer depayloads and reparses the camera stream, then packetizes it for WebRTC without decoding or re-encoding. This preserves the source frame rate and avoids unnecessary latency and CPU load. H.265 cannot be decoded by every browser, so Dashboard transcodes it to H.264 with a bounded low-latency profile: up to 15 FPS for multi-cell SD and 20 FPS for single-cell HD.

The authenticated MJPEG and single-JPEG endpoints remain as a compatibility fallback. The web client switches only the failed cell to MJPEG when WebRTC is unavailable, negotiation times out, or its peer connection fails. Idle fallback pipelines stop automatically after 12 seconds without browser frame requests.

WebRTC signaling messages use the existing WebSocket connection:

- browser to server: `webrtc-start`, `webrtc-answer`, `webrtc-ice`, `webrtc-stop`;
- server to browser: `webrtc-offer`, `webrtc-ice`, `webrtc-status`, `webrtc-error`.

Signaling is authenticated with the same session and Live View permission as the HTTP API. Dashboard currently exchanges host ICE candidates and is intended for localhost, LAN or VPN use. Internet traversal through arbitrary NAT requires a future STUN/TURN configuration layer.

Local archive files are the exception: they are served through the authenticated Dashboard HTTP endpoint because the browser cannot access the desktop filesystem directly.

## Web workspace

The browser UI follows the desktop operator workflow instead of presenting a separate card dashboard:

- persistent 1, 4 and 9 camera layouts with an explicit active cell;
- camera assignment from the device list and a clear action on every occupied cell;
- a collapsible actions area while the device list remains available;
- camera cards with status, IP address, RTSP port and temperature;
- permission-gated camera discovery, manual onboarding, editing and deletion;
- Health, Analytics and Archive workspaces displayed over the live layout;
- recording, snapshot, audio, fullscreen and continuous PTZ controls in every monitor cell;
- archive playback and permission-gated download;
- schema-driven Settings, Users/session administration and live log/diagnostics workspaces;
- Camex preview, Majestic read/diff/apply and OpenIPC device status, metrics, network, time and logs;
- explicitly confirmed sync-time, reboot and GitHub update actions with audit and replay protection;
- responsive desktop and mobile layouts with Russian and English localization.

Live preview uses WebRTC first and the authenticated Dashboard JPEG relay as fallback. While GStreamer connects, or if the source stream is unavailable, the cell remains usable and shows camera identity and connection metadata instead of a broken image.

Camera management uses the same `NetworkDiscoveryService`, `CameraModel` and transactional state as the desktop UI. Discovery results expose identification evidence and ports but never camera credentials. The edit form also receives no stored login, password or credential-bearing stream URL: blank credential fields preserve the existing backend values, while an explicit checkbox clears them. Manual onboarding sends RTSP paths separately and the server constructs the stored URL after validating the host and ports.

## Browser adaptations

- Configuration uses JSON download/upload instead of native file dialogs. The Web format includes only allowlisted settings, camera hosts/ports/paths and layouts; credentials are omitted and preserved for matching cameras during import.
- Snapshots, recordings, logs and diagnostic bundles use authenticated downloads with opaque identifiers instead of exposing local paths.
- Keychain access remains backend-only. Web shows configured state and write-only credential inputs, never stored secrets.
- Tray controls, native window chrome, printing and OS file associations are `Web N/A`. Browser fullscreen, responsive navigation and `Alt+1/2/3` layout shortcuts replace the transferable parts.
- A general SSH terminal is intentionally not exposed. Device administration is a bounded command API with RBAC, validation, confirmation, audit and redaction.

## Security headers and data boundaries

The server adds CSP, `X-Frame-Options: DENY`, `nosniff`, no-referrer, same-origin resource policy and no-store headers for API responses. Request headers are limited to 32 KiB and bodies to 1 MiB; chunked request bodies are rejected. Response header names and values are serialized through CR/LF-safe validation. Logs, diagnostics and device responses are bounded and recursively scrubbed. Login, user/settings changes, exports and dangerous device mutations generate `AUDIT web` events.

The API never returns:

- camera login names or passwords;
- credential-bearing RTSP URLs;
- user password hashes or salts;
- OAuth access/refresh tokens and client secrets;
- local model, evidence or archive filesystem paths.

Archive IDs are SHA-256 identifiers derived from canonical paths. Each playback request resolves the ID again below the configured recording root, preventing direct path input and traversal.

## Reverse proxy example

Terminate HTTPS at the proxy and forward to localhost only. A minimal conceptual Nginx location is:

```nginx
location / {
    proxy_pass http://127.0.0.1:8080;
    proxy_set_header Host $host;
    proxy_set_header X-Forwarded-Proto https;
}
```

WebSocket forwarding uses the separately configured WebSocket port. Restrict both upstream ports to localhost/firewall rules and apply normal proxy request limits. Dashboard currently validates browser `Origin` against the visible `Host`, so preserve the original host header.
