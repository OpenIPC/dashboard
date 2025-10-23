#!/usr/bin/env node
const path = require('path');
const fs = require('fs');
const { spawnSync } = require('child_process');

const { ensureLicensePlateRuntime } = require('./prepare-license-plate-runtime');

const repoRoot = path.resolve(__dirname, '..');
const analyticsDir = path.join(repoRoot, 'extra', 'analytics');

function fileExists(filePath) {
  try {
    return fs.existsSync(filePath);
  } catch (err) {
    return false;
  }
}

function ensureAnalyticsExecutables() {
  const isWindows = process.platform === 'win32';
  const expected = [path.join(analyticsDir, isWindows ? 'analytics_cpu.exe' : 'analytics_cpu')];
  if (isWindows) {
    expected.push(path.join(analyticsDir, 'analytics_dml.exe'));
  }

  const allPresent = expected.every(fileExists);
  if (allPresent) {
    console.log('[analytics] Existing analytics executables detected, skipping rebuild');
    return;
  }

  const python = process.env.PYTHON || (isWindows ? 'python' : 'python3');
  console.log(`[analytics] Building analytics executables using ${python}`);
  const result = spawnSync(python, ['python_src/build_analytics.py'], {
    cwd: repoRoot,
    stdio: 'inherit'
  });
  if (result.status !== 0) {
    throw new Error('Failed to build analytics executables');
  }

  const postCheck = expected.every(fileExists);
  if (!postCheck) {
    throw new Error('Analytics executables are missing after build');
  }
}

async function main() {
  ensureAnalyticsExecutables();
  await ensureLicensePlateRuntime();
}

if (require.main === module) {
  main().catch((err) => {
    console.error('[analytics] Failed to prepare analytics assets:', err.message || err);
    process.exitCode = 1;
  });
}

module.exports = {
  ensureAnalyticsExecutables,
  prepareAnalyticsAssets: main
};
