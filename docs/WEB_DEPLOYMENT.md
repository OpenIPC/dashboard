# Secure Web deployment

OpenIPC Dashboard is a local-first service. Its HTTP listener must not be forwarded directly from a router to the public Internet. Choose one explicit deployment profile in **Settings > Web**.

| Profile | Bind policy | Intended use |
| --- | --- | --- |
| `localhost` | forced to `127.0.0.1` | desktop browser on the same computer |
| `lan` | configured address | trusted management VLAN/LAN |
| `vpn` | configured VPN address | WireGuard, Tailscale or equivalent private overlay |
| `reverse_proxy` | localhost by default; remote bind requires a separate opt-in | HTTPS termination at a trusted proxy |

## Trusted reverse proxy

The reverse-proxy profile requires:

- an external `https://` base URL;
- one or more exact proxy peer IP addresses;
- optionally, an explicit external `wss://` URL. Without one, Dashboard derives `wss://<external-host>/ws`.

Dashboard deliberately ignores `Forwarded`, `X-Forwarded-For`, `X-Forwarded-Host` and `X-Forwarded-Proto` for authentication and authorization. An external browser origin is accepted only when the TCP peer is in the explicit trusted-proxy list and the origin exactly matches the configured HTTPS base URL. This avoids trusting spoofed forwarding headers from an ordinary LAN client.

Example Nginx configuration:

```nginx
server {
    listen 443 ssl http2;
    server_name dashboard.example;

    location /ws {
        proxy_pass http://127.0.0.1:8081;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
    }

    location / {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        client_max_body_size 1m;
    }
}
```

Keep both upstream ports blocked by the host firewall. Configure `https://dashboard.example/`, `wss://dashboard.example/ws` and `127.0.0.1` in Dashboard. Secure cookies become mandatory for a valid reverse-proxy profile.

## Operations

Public, non-secret probes:

- `GET /api/v1/health/live` — process/listener liveness;
- `GET /api/v1/health/ready` — deployment policy and listener readiness;
- `GET /api/v1/server` — browser bootstrap capabilities.

Startup logs contain the effective profile, local/public URLs, WebSocket URL, TLS runtime availability and time-to-ready. Bind failures include the address, port and a conflict hint. The authenticated diagnostic bundle contains the same bounded server status.

For the first headless start, create a local password file readable only by the service account and
run the one-shot bootstrap together with server mode:

```bash
printf '%s\n' 'replace-with-a-strong-password' > /run/openipc-dashboard-admin
chmod 600 /run/openipc-dashboard-admin
OPENIPC_INITIAL_ADMIN_PASSWORD_FILE=/run/openipc-dashboard-admin \
  /opt/openipc-dashboard/AppRun --server-only --initialize-admin admin
rm /run/openipc-dashboard-admin
```

The password must be at least 12 characters. Bootstrap fails closed when any user already exists,
when the file is missing/oversized, or when `--initialize-admin` is used without `--server-only`.
The password is never accepted as a command-line value. Remove the file after the first successful
start, then start the ordinary service without `--initialize-admin`.

Set `OPENIPC_DATA_ROOT` to an absolute directory when a headless instance must use an isolated
runtime profile. Dashboard then keeps its `state.sqlite3`, `users.json`, QSettings, analytics
database, modules and logs below that root instead of reading the desktop user's profile. This is
recommended for systemd, scheduled tasks, containers and parallel staging environments:

```bash
export OPENIPC_DATA_ROOT=/var/lib/openipc-dashboard
```

`--data-root /var/lib/openipc-dashboard` is the equivalent command-line form used by the Windows
scheduled-task helper.

The directory must be writable only by the Dashboard service account. Omitting the variable keeps
the historical desktop locations and therefore preserves ordinary desktop upgrades.

The example [systemd unit](../packaging/systemd/openipc-dashboard.service) uses `--server-only`, a dedicated service account and a private home. On Windows, [the scheduled-task helper](../packaging/windows/install-headless-task.ps1) creates a headless task for the current account because the Dashboard executable is not a native Windows Service binary.

## Sessions and backup

Administrators can inspect opaque active-session IDs, origin/proxy peer, idle expiry and absolute expiry. Revocation requires a reason and creates an audit event. Raw session tokens are never listed.

Browser-safe configuration exports contain no camera passwords or secrets. Endpoint/profile settings are included for review but are preserved during browser import so a remote import cannot move or expose its own server. Restore endpoint/profile settings from the desktop UI after validating bind addresses and proxy peers.

## STUN/TURN design boundary

The current production profiles are localhost, trusted LAN, VPN and trusted HTTPS proxy. Public NAT traversal remains disabled. A future TURN implementation must store its password in the OS keychain, expose only `turnCredentialsConfigured`, pass credentials directly to `webrtcbin`, redact ICE URLs from diagnostics, and audit configuration changes. TURN must not be enabled until coturn interoperability, credential rotation and relay-only negative tests are part of the package gate.
