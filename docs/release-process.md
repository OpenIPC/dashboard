# Release Process for VMS Dashboard

This document outlines the complete process for creating and distributing releases of VMS Dashboard.

## Release Types

### Development Releases
- **Purpose**: Testing new features, bug fixes
- **Frequency**: As needed
- **Stability**: Beta quality
- **Distribution**: GitHub releases (pre-release)

### Stable Releases
- **Purpose**: Production-ready versions
- **Frequency**: Monthly or when major features are complete
- **Stability**: Production quality
- **Distribution**: GitHub releases, package managers

## Pre-Release Checklist

### Code Quality
- [ ] All tests pass
- [ ] Code builds without warnings
- [ ] ESLint checks pass
- [ ] TypeScript compilation successful
- [ ] No security vulnerabilities

### Documentation
- [ ] README.md updated
- [ ] CHANGELOG.md updated
- [ ] Version numbers updated
- [ ] Build guide current
- [ ] API documentation current

### Testing
- [ ] Manual testing on target platforms
- [ ] Camera connectivity tests
- [ ] go2rtc integration tests
- [ ] UI/UX testing complete
- [ ] Performance testing

## Version Management

### Semantic Versioning
VMS Dashboard follows [Semantic Versioning](https://semver.org/):
- **MAJOR.MINOR.PATCH** (e.g., 1.2.3)
- **Major**: Breaking changes
- **Minor**: New features (backward compatible)
- **Patch**: Bug fixes (backward compatible)

### Version Update Process
1. Update version in `package.json`
2. Update version in `src-tauri/tauri.conf.json`
3. Update version in `src-tauri/Cargo.toml`
4. Create changelog entry
5. Commit version changes

```bash
# Update versions
npm version patch  # or minor, major
# This updates package.json and creates git tag

# Update Tauri configs manually
# Edit src-tauri/tauri.conf.json - "version" field
# Edit src-tauri/Cargo.toml - version field

# Commit changes
git add .
git commit -m "chore: bump version to v1.2.3"
git tag v1.2.3
```

## Build Process

### Local Release Builds

#### 1. Prepare Environment
```bash
# Clean workspace
git clean -fd
npm install
npm run download-go2rtc
```

#### 2. Build All Platforms
```bash
# Windows build (on Windows machine)
npm run build-windows

# Linux build (on Linux machine) 
npm run build-linux

# macOS build (on macOS machine)
npm run build-macos
```

#### 3. Collect Artifacts
```bash
# Windows artifacts
src-tauri/target/release/bundle/msi/*.msi

# Linux artifacts  
src-tauri/target/release/bundle/deb/*.deb
src-tauri/target/release/bundle/appimage/*.AppImage

# macOS artifacts
src-tauri/target/release/bundle/dmg/*.dmg
```

### Automated Release (GitHub Actions)

#### Trigger Release Workflow
1. Create and push git tag:
```bash
git tag v1.2.3
git push origin v1.2.3
```

2. GitHub Actions automatically:
   - Builds for all platforms
   - Creates release artifacts
   - Publishes GitHub release
   - Uploads installers

#### Workflow Files
- `.github/workflows/release.yml` - Main release workflow
- `.github/workflows/build.yml` - Build verification
- `.github/workflows/prepare-release.yml` - Pre-release checks

## Release Distribution

### GitHub Releases

#### Create Release
1. Go to [GitHub Releases](https://github.com/OpenIPC/dashboard/releases)
2. Click "Create a new release"
3. Choose tag version (e.g., v1.2.3)
4. Set release title: "VMS Dashboard v1.2.3"
5. Add release notes (see template below)
6. Upload platform installers
7. Mark as pre-release if applicable
8. Publish release

#### Release Notes Template
```markdown
# VMS Dashboard v1.2.3

## 🚀 New Features
- Added camera group management
- Improved RTSP authentication handling
- New layout templates

## 🔧 Improvements  
- Better error handling for ONVIF discovery
- Performance optimizations for video streaming
- Updated go2rtc to latest version

## 🐛 Bug Fixes
- Fixed camera connection timeout issues
- Resolved layout switching problems
- Fixed memory leaks in video players

## 📦 Downloads
- **Windows**: VMS-Dashboard-v1.2.3-windows.msi
- **Linux**: VMS-Dashboard-v1.2.3-linux.deb / VMS-Dashboard-v1.2.3-linux.AppImage  
- **macOS**: VMS-Dashboard-v1.2.3-macos.dmg

## 🔧 Installation
See [Installation Guide](./docs/installation.md) for detailed instructions.

## 📋 System Requirements
- Windows 10+, Linux (Ubuntu 18.04+), macOS 10.15+
- 4GB RAM (8GB recommended)
- Network access for camera communication

## 🆘 Support
- [Documentation](https://github.com/OpenIPC/dashboard/wiki)
- [Issues](https://github.com/OpenIPC/dashboard/issues)
- [Discussions](https://github.com/OpenIPC/dashboard/discussions)
```

### Package Managers (Future)

#### Chocolatey (Windows)
```bash
# Future implementation
choco install vms-dashboard
```

#### Homebrew (macOS)
```bash
# Future implementation  
brew install --cask vms-dashboard
```

#### APT Repository (Linux)
```bash
# Future implementation
sudo apt install vms-dashboard
```

## Post-Release Tasks

### Immediate Tasks
- [ ] Verify downloads work correctly
- [ ] Test installation on clean systems
- [ ] Update project website/documentation
- [ ] Announce release on social media
- [ ] Update any demo videos/screenshots

### Monitoring
- [ ] Monitor GitHub issues for release-related bugs
- [ ] Track download statistics
- [ ] Collect user feedback
- [ ] Monitor performance metrics

## Hotfix Process

### Critical Bug Fixes
1. Create hotfix branch from release tag:
```bash
git checkout -b hotfix/v1.2.4 v1.2.3
```

2. Apply minimal fix
3. Update version to patch level
4. Test thoroughly
5. Merge to main and tag:
```bash
git checkout main
git merge hotfix/v1.2.4
git tag v1.2.4
git push origin main v1.2.4
```

## Release Schedule

### Planned Release Cycle
- **Major releases**: Quarterly (4x per year)
- **Minor releases**: Monthly
- **Patch releases**: As needed for critical bugs
- **Pre-releases**: Weekly during active development

### Release Timeline Example
```
January: v1.0.0 (Major - New year, major features)
February: v1.1.0 (Minor - Feature additions)  
March: v1.2.0 (Minor - Improvements)
April: v2.0.0 (Major - Breaking changes)
```

## Quality Gates

### Automated Checks
- [ ] All CI/CD tests pass
- [ ] Security scans clean
- [ ] Performance benchmarks within limits
- [ ] Cross-platform build successful

### Manual Verification
- [ ] Installation test on clean VMs
- [ ] Basic functionality verification  
- [ ] Camera connection testing
- [ ] User acceptance testing

## Communication

### Release Announcements
- GitHub release page
- Project README badges
- OpenIPC community channels  
- Developer mailing lists
- Social media posts

### Documentation Updates
- Update installation instructions
- Refresh screenshots/videos
- Update API documentation
- Verify external links

## Rollback Procedures

### Emergency Rollback
If critical issues are discovered:

1. **Immediate**: Mark release as problematic in GitHub
2. **Short-term**: Create hotfix release
3. **Communication**: Notify users of issues and timeline
4. **Long-term**: Investigate root cause and improve processes

```bash
# Remove problematic release (last resort)
git tag -d v1.2.3
git push origin :refs/tags/v1.2.3
```

## Tools and Scripts

### Useful Commands
```bash
# Check version consistency
grep -r "version" package.json src-tauri/

# Generate changelog
git log --oneline v1.2.2..v1.2.3

# Build size analysis
ls -lh src-tauri/target/release/bundle/

# Test installer
# Windows: msiexec /i installer.msi /L*V install.log  
# Linux: sudo dpkg -i package.deb
# macOS: hdiutil attach image.dmg
```

### Automation Scripts
- `tools/release-prepare.py` - Version updates and validation
- `tools/build-all.py` - Multi-platform build orchestration
- `tools/test-installers.py` - Automated installer testing

## Security Considerations

### Code Signing
- **Windows**: Authenticode signing (future)
- **macOS**: Apple Developer ID signing (future)  
- **Linux**: GPG signatures (future)

### Supply Chain Security
- Dependency vulnerability scanning
- go2rtc binary verification
- Build environment security
- Artifact integrity checks

## Metrics and Analytics

### Release Metrics
- Download counts per platform
- Installation success rates
- User retention post-release
- Bug report frequency

### Performance Metrics  
- Binary size trends
- Build time optimization
- Resource usage benchmarks
- Startup time measurements

---

**Next Review**: This document should be reviewed and updated with each major release to ensure it remains current and effective.