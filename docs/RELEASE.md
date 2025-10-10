# Release Process

This document describes the automated release process for VMS Dashboard.

## Automated Builds

The project uses GitHub Actions for automated building and releasing:

### 1. Build and Test Workflow (`build.yml`)
- **Triggers**: Push to `main` or `develop` branches, Pull Requests to `main`
- **Platforms**: Windows, macOS (Intel & ARM), Ubuntu
- **Actions**:
  - Installs dependencies
  - Runs linting
  - Builds frontend
  - Tests Rust code
  - Builds Tauri application (without releasing)

### 2. Prepare Release Workflow (`prepare-release.yml`)
- **Trigger**: Manual workflow dispatch
- **Purpose**: Updates version numbers and creates git tag
- **Steps**:
  1. Updates `package.json` version
  2. Updates `src-tauri/Cargo.toml` version
  3. Updates `src-tauri/tauri.conf.json` version
  4. Commits changes
  5. Creates and pushes git tag

### 3. Release Workflow (`release.yml`)
- **Triggers**: 
  - Push of version tags (`v*.*.*`)
  - Manual workflow dispatch
- **Platforms**: Windows, macOS (Intel & ARM), Ubuntu
- **Actions**:
  - Creates GitHub release (draft)
  - Builds application for all platforms
  - Uploads binaries to release
  - Publishes release

## How to Create a Release

### Option 1: Automated Process (Recommended)

1. Go to **Actions** tab in GitHub repository
2. Select **"Prepare Release"** workflow
3. Click **"Run workflow"**
4. Enter the new version number (e.g., `1.0.0`)
5. Click **"Run workflow"**

This will:
- Update all version files
- Create a git tag
- Automatically trigger the release build

### Option 2: Manual Process

1. Update version in `package.json`:
   ```bash
   npm version 1.0.0 --no-git-tag-version
   ```

2. Update version in `src-tauri/Cargo.toml`:
   ```toml
   version = "1.0.0"
   ```

3. Update version in `src-tauri/tauri.conf.json`:
   ```json
   {
     "version": "1.0.0"
   }
   ```

4. Commit and tag:
   ```bash
   git add .
   git commit -m "chore: bump version to 1.0.0"
   git tag v1.0.0
   git push origin main
   git push origin v1.0.0
   ```

## Release Artifacts

Each release will include:

### Windows
- `.msi` installer
- `.exe` portable executable

### macOS
- `.dmg` disk image (Intel)
- `.dmg` disk image (Apple Silicon)
- `.app.tar.gz` application bundle

### Linux
- `.deb` package (Debian/Ubuntu)
- `.AppImage` portable application

## Version Numbering

The project follows [Semantic Versioning](https://semver.org/):
- **MAJOR.MINOR.PATCH** (e.g., 1.0.0)
- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

## Platform-Specific Notes

### Windows
- Requires Windows 10 or later
- Signed with self-signed certificate (users may see security warnings)

### macOS
- Supports macOS 10.15 (Catalina) or later
- Universal binary for Intel and Apple Silicon
- Not notarized (users may need to bypass Gatekeeper)

### Linux
- Requires modern Linux distribution with WebKit2GTK
- Tested on Ubuntu 20.04+, should work on most distributions

## Troubleshooting

### Build Failures
- Check GitHub Actions logs for detailed error messages
- Ensure all dependencies are properly specified
- Verify Rust and Node.js versions are compatible

### Release Issues
- Verify version numbers are consistent across all files
- Check that git tag follows `v*.*.*` format
- Ensure GitHub token has sufficient permissions

### Platform-Specific Issues
- **macOS**: Install Xcode command line tools
- **Linux**: Install required system dependencies
- **Windows**: Ensure Visual Studio Build Tools are available