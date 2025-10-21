module.exports = {
  "appId": "com.vavol.openipcdashboard.lite",
  "productName": "DASHBOARD for OpenIPC Lite",
  "directories": {
    "output": "dist"
  },
  "publish": [
    {
      "provider": "github",
      "owner": "OpenIPC",
      "repo": "dashboard"
    }
  ],
  "win": {
    "target": [
      "nsis",
      "portable"
    ],
    "icon": "assets/icon.png"
  },
  "linux": {
    "target": "AppImage",
    "artifactName": "${productName}-${version}.${ext}",
    "icon": "assets/icon.png"
  },
  "mac": {
    "icon": "assets/icon.png"
  },
  "nsis": {
    "artifactName": "${productName}-Setup-${version}.${ext}",
    "oneClick": false,
    "allowToChangeInstallationDirectory": true
  },
  "portable": {
    "artifactName": "${productName}-Portable-${version}.${ext}"
  },
  "files": [
    "**/*",
    "!modules/",
    "!extra/",
    "!python_src/",
    "!*.spec"
  ],
  "extraResources": [
    {
      "from": "mediamtx/",
      "to": "mediamtx"
    }
  ],
  "asarUnpack": [
    "**/node_modules/keytar/**"
  ],
  "afterAllArtifactBuild": () => {
    const { execSync } = require('child_process');
    const path = require('path');
    try {
      const scriptPath = path.join(__dirname, 'scripts', 'upx-compress.ps1');
      execSync(`powershell -ExecutionPolicy Bypass -File "${scriptPath}"`, { stdio: 'inherit' });
    } catch (error) {
      console.warn('UPX compression failed:', error.message);
    }
  },
  "generateUpdatesFilesForAllChannels": true
};