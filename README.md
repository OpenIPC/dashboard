---

# Dashboard

![OpenIPC Dashboard Screenshot](build/screenshot.png) 

**Dashboard** is a cross-platform desktop application for easy camera management and monitoring based on the OpenIPC firmware.

The application was created using Electron and provides a single interface for viewing video streams, administering settings, working with the file system, and direct access to the camera's command line.

---

## 🚀 Main Features

*   **Multi-view:** View up to 64 video streams simultaneously in a customizable grid.
*   **Dual Stream Support:** Instantly switch between the main (HD) and secondary (SD) streams.
*   **Full-screen Mode:** Detailed full-screen viewing of a single camera with a double-click.
*   **Built-in SSH Client:** A full-fledged terminal for direct access to the camera's command line without third-party programs.
*   **File Manager (SCP):** A convenient two-panel manager for uploading firmware, downloading recordings, and managing files on the camera.
*   **Settings Editor:** A graphical interface for changing all parameters of the Majestic firmware (`majestic.yaml`), grouped by tabs.
*   **Monitoring:** Displays the status (online/offline) and SoC temperature of the camera in real time.
*   **Cross-platform:** Works on Windows, macOS, and Linux.

---

## 📦 Installation

Ready-to-use installation files for the latest version can be found on the **[Releases page](https://github.com/openipc/dashboard/releases)**.

#### Windows
1.  Download the `OpenIPC-Dashboard-Setup-x.x.x.exe` file.
2.  Run the installer and follow the instructions.

#### macOS
1.  Download the `OpenIPC-Dashboard-x.x.x.dmg` file.
2.  Open the `.dmg` file and drag the `OpenIPC Dashboard.app` into your Applications folder.

#### Linux
1.  Download the `OpenIPC-Dashboard-x.x.x.AppImage` file.
2.  Make the file executable:
    ```bash
    chmod +x OpenIPC-Dashboard-x.x.x.AppImage
    ```
3.  Launch the app:
    ```bash
    ./OpenIPC-Dashboard-x.x.x.AppImage --no-sandbox
    ```

---

## 🎨 White-Labeling & Branding

The application supports full customization (white-labeling), allowing you to adapt it for your company or clients.

### How It Works

On startup, the application looks for a `branding.json` file in the same directory as its executable file (`.exe`, `.AppImage`). If the file is found, the application will apply the custom settings. Otherwise, it will run with the default "DASHBOARD for OpenIPC" branding.

### Step-by-Step Guide

1.  **Locate the Application Folder:** Find the directory where the application's executable is located.
2.  **Create `branding.json`:** In that same directory, create a new text file named exactly `branding.json`.
3.  **Configure the File:** Open `branding.json` in a text editor and add your desired settings. See the parameters and examples below.
4.  **(Optional) Add Your Logo:** If you specified a `logoPath`, place the logo image file in the same directory.
5.  **Launch the Application:** Run the executable to see your changes.

### Configuration Parameters

| Parameter            | Type      | Description                                                                                                                              |
| -------------------- | --------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| `appName`            | `String`  | Replaces the default application name.                                                                                                   |
| `logoPath`           | `String`  | Relative path to your logo file (e.g., `custom_logo.png`). Recommended: 80x80px PNG with transparency.                                   |
| `features`           | `Object`  | An object to enable or disable specific UI features.                                                                                     |
| `showDonations`      | `Boolean` | If `false`, hides the "Support the Project" section in the "About" tab. Default is `true`.                                               |
| `showIssueReporting` | `Boolean` | If `false`, hides the "Report an Issue" button in the settings window. Default is `true`.                                                |
| `showAboutTab`       | `Boolean` | If `false`, completely hides the "About" tab in the settings window. Default is `true`.                                                  |

### Example `branding.json`

```json
{
  "appName": "VMS Pro",
  "logoPath": "vms_pro_logo.png",
  "features": {
    "showDonations": false,
    "showIssueReporting": false,
    "showAboutTab": true
  }
}
```
> **Note:** Only include the parameters you wish to override. Any omitted parameters will use their default values. The application must be restarted for changes to take effect.

---

## 🛠️ For Developers

### Technology Stack
*   [Electron](https://www.electronjs.org/)
*   [Node.js](https://nodejs.org/)
*   HTML, CSS, JavaScript (Vanilla JS)
*   [JSMpeg](https://jsmpeg.com/) for video decoding
*   [ssh2](https://github.com/mscdex/ssh2) for SSH and SCP

### Launching in Development Mode

1.  Clone the repository:
    ```bash
    git clone https://github.com/openipc/dashboard.git
    cd dashboard
    ```

2.  Install the dependencies:
    ```bash
    npm install
    ```

3.  Launch the app:
    ```bash
    npm start
    ```

### Building the App

To build the installation files for your current platform, run the command:

```bash
npm run dist
```

The finished files will appear in the `dist` folder.

### Analytics Runtimes

The heavy analytics components are no longer bundled with the Electron application.  Instead, each module downloads its runtime on demand using the manifests in `runtime/analytics/runtime-manifest.json` and `runtime/license-plate/runtime-manifest.json`.

- **Building/Publishing analytics runtimes** – use the external repository that hosts prebuilt archives (for example [`Rinibr25/Analytics-Runtime-for-Dashboard`](https://github.com/Rinibr25/Analytics-Runtime-for-Dashboard)).  After producing new archives, upload them to a release and record the file name, SHA-256 checksum, and size in the manifest.
- **Building/Publishing license-plate runtimes** – run `python python_src/build_license_plate_runtime.py --platform <platform_tag> --provider <cpu|dml>` on the target OS to produce an archive.  Upload the archive to the License Plate runtime release, then update the corresponding manifest entry.
- **Development workflow** – you can still execute the python sources directly (`python python_src/analytics.py --help`, `python python_src/test_plate_yunet.py`) without touching the packaged runtimes. The application will prefer local interpreters during development and fall back to downloaded archives in production.

When the manifests are updated and a new Dashboard build is published, end users automatically receive the new runtimes during module activation.

## CI / GitHub Actions

This repository includes a GitHub Actions workflow that builds both application variants (`lite` and `intellect`) on Ubuntu and Windows runners and performs UPX compression on produced binaries to reduce final artifact size.

Workflow file: `.github/workflows/ci-build-upx.yml`.

Local testing of UPX compression scripts:

Linux/macOS (bash):
```bash
chmod +x scripts/upx-compress.sh
./scripts/upx-compress.sh dist
```

Windows (PowerShell):
```powershell
pwsh -File .\scripts\upx-compress.ps1 -DistPath dist
```

Notes:
- The workflow installs UPX (`apt` on Ubuntu, `choco` on Windows) before running compression.
- The CI uploads compressed `dist/` as workflow artifacts. If you prefer the original uncompressed artifacts kept separately, adjust the workflow to copy them before compression.

## ✅ Release Checklist

- `npm ci` (or `npm install`) completes without warnings or audit failures.
- `npm run dist:lite` and `npm run dist:intellect` succeed locally.
- Runtime manifests (`runtime/**/runtime-manifest.json`) reference published archives with correct URLs, sizes, and SHA-256 digests.
- Smoke-test `npm start` against at least one camera profile, verifying live view, archive list, and terminal access.
- Update version metadata (`version-config.json` and release notes) and tag the repository.
