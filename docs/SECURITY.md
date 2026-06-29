# Security

## Secrets

- Remembered application passwords use QtKeychain and the native OS credential store.
- Camera passwords use a separate keychain namespace keyed by camera address.
- Persisted RTSP URLs are stripped of user information. Credentials are injected only into the in-memory URL passed to GStreamer.
- Legacy plaintext login and camera credentials are migrated and removed.

Ordinary application exports must not include passwords or OAuth client secrets. A Majestic backup is an explicit exception: it mirrors the camera's complete configuration and can contain RTMP stream keys or firmware fields such as plaintext network passwords. The UI warns before export; store these files like credentials. Diff previews redact password, secret, token and outgoing-server values.

Majestic commonly exposes HTTP Basic authentication over plain HTTP on the camera LAN. Dashboard does not downgrade an HTTPS endpoint, but it cannot add transport encryption to an HTTP-only firmware. Use a trusted management network, a VPN/tunnel, or an HTTPS reverse proxy for untrusted paths.

## Downloads

AI model URLs are pinned to a commit or release. Every artifact has an expected byte size and SHA-256 digest. Downloads go to a `.part` file, are verified, and are promoted with rollback protection. A failed verification never replaces an installed model.

TLS errors abort network operations; certificates are not silently accepted.

## Reporting

Do not include passwords, camera URLs with user information, access tokens, recordings or customer network layouts in a public issue. Provide sanitized logs and a minimal reproduction.
