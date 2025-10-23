// after-build.js
const { execSync } = require('child_process');
const path = require('path');

try {
    const scriptPath = path.join(__dirname, 'upx-compress.ps1');
    execSync(`powershell -ExecutionPolicy Bypass -File "${scriptPath}"`, { stdio: 'inherit' });
} catch (error) {
    console.warn('UPX compression failed:', error.message);
}