#!/usr/bin/env node
const path = require('path');
const fs = require('fs');
const fsPromises = fs.promises;
const os = require('os');
const crypto = require('crypto');
const { pipeline } = require('stream/promises');
const { spawn } = require('child_process');
const axios = require('axios');
const extractZip = require('extract-zip');
const tar = require('tar');

const manifest = require('../modules/license-plate/runtime-manifest.json');

const repoRoot = path.resolve(__dirname, '..');
const runtimeBase = path.join(repoRoot, 'runtime');
const runtimeRoot = path.join(runtimeBase, 'license-plate');

function detectPlatformKey() {
  const arch = process.arch;
  return `${process.platform}-${arch}`;
}

function resolveReleaseBaseUrl() {
  const release = manifest.release || {};
  if (release.baseUrl) {
    return release.baseUrl.replace(/\/$/, '');
  }
  if (release.repo) {
    return `https://github.com/${release.repo}/releases/download`;
  }
  return null;
}

function normalizeArtifact(artifact) {
  if (!artifact) return null;
  if (artifact.url) return artifact;
  const fileName = artifact.fileName || artifact.name;
  const release = manifest.release || {};
  const tag = release.tag || manifest.version;
  const baseUrl = resolveReleaseBaseUrl();
  if (fileName && baseUrl && tag) {
    return { ...artifact, url: `${baseUrl}/${tag}/${fileName}` };
  }
  return artifact;
}

function getArtifactForPlatform() {
  const key = detectPlatformKey();
  if (manifest.artifacts && manifest.artifacts[key]) {
    return normalizeArtifact(manifest.artifacts[key]);
  }
  if (manifest.artifacts && manifest.artifacts[process.platform]) {
    return normalizeArtifact(manifest.artifacts[process.platform]);
  }
  return null;
}

function computeVersionTag(artifact) {
  if (!artifact) return null;
  const base = manifest.version || '0.0.0';
  if (artifact.version) {
    return `${base}:${artifact.version}`;
  }
  if (artifact.fileName) {
    return `${base}:${artifact.fileName}`;
  }
  return `${base}:${artifact.url || 'unknown'}`;
}

async function readRuntimeInfo(dir) {
  try {
    const raw = await fsPromises.readFile(path.join(dir, 'runtime-info.json'), 'utf-8');
    return JSON.parse(raw);
  } catch (err) {
    return null;
  }
}

async function downloadFile(url, destination) {
  console.log(`[runtime] Downloading ${url}`);
  const response = await axios.get(url, { responseType: 'stream', maxRedirects: 5 });
  await fsPromises.mkdir(path.dirname(destination), { recursive: true });
  await pipeline(response.data, fs.createWriteStream(destination));
}

async function verifySha(filePath, expected) {
  if (!expected) return true;
  return new Promise((resolve, reject) => {
    const hash = crypto.createHash('sha256');
    const stream = fs.createReadStream(filePath);
    stream.on('error', reject);
    stream.on('data', chunk => hash.update(chunk));
    stream.on('end', () => {
      const digest = hash.digest('hex');
      const ok = digest.toLowerCase() === expected.toLowerCase();
      if (!ok) {
        console.error(`[runtime] Checksum mismatch: expected ${expected}, got ${digest}`);
      }
      resolve(ok);
    });
  });
}

async function extractArchive(archivePath, targetDir, type) {
  await fsPromises.mkdir(targetDir, { recursive: true });
  if (!type || type === 'zip') {
    await extractZip(archivePath, { dir: targetDir });
    return;
  }
  if (type === 'tar.gz' || type === 'tgz' || type === 'tar.xz' || type === 'txz') {
    try {
      await tar.x({ file: archivePath, cwd: targetDir });
    } catch (err) {
      const message = err && err.message ? err.message : '';
      const code = err && err.code ? err.code : '';
      const needsFallback = /Unrecognized archive format/i.test(message) || code === 'TAR_BAD_ARCHIVE';
      if (!needsFallback) {
        throw err;
      }
      console.warn(`[runtime] Node tar extraction failed (${code || message.trim()}), trying system tar`);
      await extractWithSystemTar(archivePath, targetDir);
    }
    return;
  }
  throw new Error(`Unsupported archive type: ${type}`);
}

async function extractWithSystemTar(archivePath, targetDir) {
  const tarCommand = process.platform === 'win32' ? 'tar.exe' : 'tar';
  const args = ['-xf', archivePath, '-C', targetDir];
  await fsPromises.mkdir(targetDir, { recursive: true });
  await new Promise((resolve, reject) => {
    const child = spawn(tarCommand, args, { stdio: 'inherit' });
    child.on('error', (error) => {
      reject(error);
    });
    child.on('exit', (code) => {
      if (code === 0) {
        resolve();
      } else {
        reject(new Error(`System tar exited with code ${code}`));
      }
    });
  });
}

async function findRuntimeRoot(stagingDir) {
  const entries = await fsPromises.readdir(stagingDir, { withFileTypes: true });
  if (entries.length === 1 && entries[0].isDirectory()) {
    return path.join(stagingDir, entries[0].name);
  }
  return stagingDir;
}

async function copyDirectoryContents(srcDir, destDir) {
  const entries = await fsPromises.readdir(srcDir, { withFileTypes: true });
  await fsPromises.mkdir(destDir, { recursive: true });
  for (const entry of entries) {
    const from = path.join(srcDir, entry.name);
    const to = path.join(destDir, entry.name);
    await fsPromises.cp(from, to, { recursive: true });
  }
}

function resolveLocalArchive(artifact) {
  const candidates = [];
  const envPath = process.env.LICENSE_PLATE_RUNTIME_ARCHIVE || process.env.LP_RUNTIME_ARCHIVE;
  if (envPath) {
    candidates.push(envPath);
  }
  const fileName = artifact.fileName || '';
  if (fileName) {
    candidates.push(path.join(repoRoot, fileName));
    candidates.push(path.join(repoRoot, 'runtime', fileName));
    candidates.push(path.join(repoRoot, 'python_src', 'build', 'license_plate_runtime', 'dist', fileName));
  }
  for (const candidate of candidates) {
    if (candidate && fs.existsSync(candidate)) {
      return candidate;
    }
  }
  return null;
}

async function ensureLicensePlateRuntime() {
  const artifact = getArtifactForPlatform();
  if (!artifact || !artifact.url) {
    throw new Error('No runtime artifact defined for current platform');
  }

  await fsPromises.mkdir(runtimeBase, { recursive: true });
  const versionTag = computeVersionTag(artifact);
  const existingInfo = await readRuntimeInfo(runtimeRoot);
  if (existingInfo && existingInfo.version === versionTag) {
    console.log('[runtime] Bundled license-plate runtime already prepared');
    return runtimeRoot;
  }

  const stagingDir = path.join(runtimeBase, 'license-plate-staging');
  const tempDownload = path.join(os.tmpdir(), `license-plate-runtime-${Date.now()}`);
  let archivePath = resolveLocalArchive(artifact);
  let downloaded = false;

  try {
    if (!archivePath) {
      await downloadFile(artifact.url, tempDownload);
      archivePath = tempDownload;
      downloaded = true;
    } else {
      console.log('[runtime] Using local runtime archive', archivePath);
    }

    if (artifact.sha256 && archivePath) {
      const ok = await verifySha(archivePath, artifact.sha256);
      if (!ok) {
        throw new Error('Runtime archive checksum mismatch');
      }
    }

    await fsPromises.rm(stagingDir, { recursive: true, force: true });
    await fsPromises.mkdir(stagingDir, { recursive: true });
    await extractArchive(archivePath, stagingDir, artifact.archiveType);

    const extractedRoot = await findRuntimeRoot(stagingDir);
    await fsPromises.rm(runtimeRoot, { recursive: true, force: true });
    await fsPromises.mkdir(path.dirname(runtimeRoot), { recursive: true });
    if (extractedRoot !== runtimeRoot) {
      try {
        await fsPromises.rename(extractedRoot, runtimeRoot);
      } catch (err) {
        if (['EPERM', 'EACCES', 'EXDEV', 'EEXIST'].includes(err.code)) {
          console.warn(`[runtime] Rename failed with ${err.code}, falling back to copy`);
          await fsPromises.rm(runtimeRoot, { recursive: true, force: true });
          await copyDirectoryContents(extractedRoot, runtimeRoot);
        } else {
          throw err;
        }
      }
    }
    const infoPath = path.join(runtimeRoot, 'runtime-info.json');
    const infoPayload = {
      version: versionTag,
      manifestVersion: manifest.version || null,
      platform: detectPlatformKey(),
      preparedAt: new Date().toISOString(),
      source: downloaded ? 'download' : 'local',
      artifact: {
        fileName: artifact.fileName || null,
        url: artifact.url || null,
        sha256: artifact.sha256 || null,
        pythonExecutable: artifact.pythonExecutable || null,
        scriptRoot: artifact.scriptRoot || null
      }
    };
    await fsPromises.writeFile(infoPath, JSON.stringify(infoPayload, null, 2));
    const info = await readRuntimeInfo(runtimeRoot);
    if (!info) {
      throw new Error('Runtime prepared but runtime-info.json is missing');
    }
    console.log('[runtime] Prepared bundled license-plate runtime in', runtimeRoot);
    return runtimeRoot;
  } finally {
    await fsPromises.rm(stagingDir, { recursive: true, force: true }).catch(() => {});
    if (downloaded) {
      await fsPromises.unlink(tempDownload).catch(() => {});
    }
  }
}

if (require.main === module) {
  ensureLicensePlateRuntime().catch((err) => {
    console.error('[runtime] Failed to prepare bundled license-plate runtime:', err.message || err);
    process.exitCode = 1;
  });
}

module.exports = {
  ensureLicensePlateRuntime
};
