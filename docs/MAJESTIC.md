# Majestic integration

Dashboard treats Majestic as a discoverable device API, not as a fixed set of OpenIPC presets. This matters because the configuration surface varies by Majestic build, SoC, sensor and enabled firmware features.

## User flow

1. Add the camera with its HTTP/ONVIF port and credentials.
2. Right-click the camera in the device list or video grid.
3. Choose **OpenIPC / Majestic**.
4. Review the detected capabilities and edit settings under **All settings**.
5. Use **Review and apply** to inspect the minimal redacted diff before writing.

The settings page is generated from the schema returned by the camera. That includes current and future sections such as `video0`, `video1`, `jpeg`, `isp`, `audio`, `osd`, `motionDetect`, recording and outgoing/RTMP when the firmware advertises them. Unsupported fields are not invented.

## API coverage

| Operation | Majestic/OpenIPC endpoint | Dashboard behavior |
| --- | --- | --- |
| Read configuration | `GET /api/v1/config.json` | Preserves the complete JSON object, including unknown fields. |
| Discover settings | `GET /api/v1/config.schema.json` | Builds groups, field types, enum values, limits, defaults and live-image flags. |
| Apply changes | `POST /api/v1/config` | Sends a nested patch only after validation and diff confirmation; limited to 1 MiB. |
| Reset defaults | `GET /api/v1/reset?key=...` | Supports one or several schema paths and then reloads the effective config. |
| Live ISP adjustment | `POST /api/v1/image?...` | Debounced combined luminance/contrast/saturation/hue/mirror/flip preview. |
| Monitoring | `GET /metrics` | Displays the raw Prometheus payload and parses named values for status reporting. |
| JPEG snapshot | `GET /image.jpg?...` | Supports width, height, quality and grayscale query options; saves atomically. |
| Day/night hardware | `GET /night/{on,off,toggle,ircut,light}` | Explicit immediate actions. |
| Speaker PCM | `POST /play_audio` | Uploads a local file with a 32 MiB client-side safety limit. |
| Pipeline reload | `GET /cgi-bin/j/mj-apply.cgi` | Uses the current OpenIPC WebUI helper and keeps reload separate from saving. |

The contract follows the current [OpenIPC Majestic WebUI](https://github.com/OpenIPC/majestic-webui) and [OpenIPC Majestic documentation](https://github.com/OpenIPC/wiki/blob/master/en/majestic-streamer.md). Majestic itself is distributed separately in the [OpenIPC/majestic](https://github.com/OpenIPC/majestic) repository.

## Compatibility and safety

- A camera with `config.json` but no schema is shown in legacy read-only mode. Dashboard will not guess writable paths or rewrite `/etc/majestic.yaml` directly.
- Unknown keys remain in the original object. Normal form editing and raw JSON editing both produce a diff; removing a key in the raw editor does not silently delete it. Use the schema-backed reset action for defaults.
- Password/secret/token paths and `outgoing.server` are redacted in the confirmation dialog.
- HTTP failures retain the draft and include the status code/body where available. A 401 is surfaced as an authentication problem rather than retried indefinitely.
- Backups are written atomically and include a format/version marker, the effective config and its schema. Plain `config.json` files are also accepted for review. A backup can contain secrets.
- Structural encoder changes may interrupt RTSP briefly. Saving and pipeline reload are separate visible actions.

## Deliberate boundary

Majestic owns the media pipeline. Firmware upgrades, Linux networking, users, package management and device reboot belong to the wider OpenIPC operating system and are not disguised as Majestic settings. Dashboard already exposes SSH and the remote file manager for those administrative tasks; destructive firmware operations require their own audited workflow.
