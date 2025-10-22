const path = require('path');
const fs = require('fs');
const fsPromises = fs.promises;
const crypto = require('crypto');
const os = require('os');
const { pipeline } = require('stream/promises');
const { app } = require('electron');
const axios = require('axios');
const extractZip = require('extract-zip');
const tar = require('tar');

const manifest = require('./runtime-manifest.json');

const PROGRESS_CHANNEL = 'module-license-plate-runtime-progress';
const VERSION_FILENAME = 'runtime-info.json';
const RUNTIME_SUBDIR = path.join('runtime', 'license-plate');
const REPO_ROOT = path.join(__dirname, '..', '..');

let activePromise = null;
let cachedRuntimeInfo = null;

// Resolve files inside extracted runtimes that may add a single wrapper directory
function resolveWithinRuntime(runtimeRoot, relativePath) {
    const normalized = relativePath ? relativePath.replace(/^[/\\]+/, '') : '';
    const direct = normalized ? path.join(runtimeRoot, normalized) : runtimeRoot;
    if (fs.existsSync(direct)) {
        return direct;
    }
    try {
        const entries = fs.readdirSync(runtimeRoot, { withFileTypes: true });
        if (entries.length === 1 && entries[0].isDirectory()) {
            const altRoot = path.join(runtimeRoot, entries[0].name);
            if (!normalized) return altRoot;
            const alt = path.join(altRoot, normalized);
            if (fs.existsSync(alt)) return alt;
        }
    } catch (e) {
        // ignore and fall through
    }
    return direct;
}

function emitProgress(api, payload) {
    try {
        if (api && typeof api.sendToRenderer === 'function') {
            api.sendToRenderer(PROGRESS_CHANNEL, payload);
        }
    } catch (e) {
        console.warn('[LicensePlate Runtime] Failed to emit progress event', e);
    }
}

function getRuntimeRoot() {
    const base = app.getPath('userData');
    return path.join(base, RUNTIME_SUBDIR);
}

function getRuntimeBase() {
    return path.dirname(getRuntimeRoot());
}

function detectPlatformKey() {
    const arch = process.arch;
    return `${process.platform}-${arch}`;
}

function getArtifactForPlatform() {
    const key = detectPlatformKey();
    if (manifest.artifacts && manifest.artifacts[key]) {
        return manifest.artifacts[key];
    }
    // Fallbacks: try without arch suffix, then exact platform
    if (manifest.artifacts && manifest.artifacts[process.platform]) {
        return manifest.artifacts[process.platform];
    }
    return null;
}

function computeVersionTag(artifact) {
    if (!artifact) return null;
    const base = manifest.version || '0.0.0';
    if (artifact.version) {
        return `${base}:${artifact.version}`;
    }
    return `${base}:${artifact.url || 'unknown'}`;
}

async function readRuntimeInfo(runtimeRoot) {
    try {
        const raw = await fsPromises.readFile(path.join(runtimeRoot, VERSION_FILENAME), 'utf-8');
        return JSON.parse(raw);
    } catch (e) {
        return null;
    }
}

function getDevPythonExecutable() {
    try {
        const scriptsDir = process.platform === 'win32' ? 'Scripts' : 'bin';
        const pythonBinary = process.platform === 'win32' ? 'python.exe' : 'python3';

        const projectVenv = path.join(REPO_ROOT, 'venv', scriptsDir, pythonBinary);
        if (fs.existsSync(projectVenv)) return projectVenv;

        const legacyVenv = path.join(REPO_ROOT, '.analytics_venvs', 'dml', scriptsDir, pythonBinary);
        if (fs.existsSync(legacyVenv)) return legacyVenv;
    } catch (e) {
        // ignore and fall through
    }
    return null;
}

function getDevelopmentRuntimeInfo() {
    const pythonSrc = path.join(REPO_ROOT, 'python_src');
    const scriptEntry = path.join(pythonSrc, 'test_plate_yunet.py');
    if (!fs.existsSync(scriptEntry)) {
        return null;
    }
    const pythonExecutable = getDevPythonExecutable() || 'python';
    return {
        mode: 'development',
        runtimeRoot: pythonSrc,
        scriptRoot: pythonSrc,
        pythonPath: pythonExecutable
    };
}

async function downloadFile(url, destination, onProgress) {
    const response = await axios.get(url, { responseType: 'stream', maxRedirects: 5 });
    const total = parseInt(response.headers['content-length'] || '0', 10) || 0;
    let downloaded = 0;
    if (typeof onProgress === 'function' && total === 0) {
        onProgress({ stage: 'download', downloaded, total });
    }
    response.data.on('data', (chunk) => {
        downloaded += chunk.length;
        if (typeof onProgress === 'function') {
            onProgress({ stage: 'download', downloaded, total, progress: total ? downloaded / total : null });
        }
    });
    await pipeline(response.data, fs.createWriteStream(destination));
}

async function verifySha256(filePath, expected) {
    if (!expected) return true;
    return new Promise((resolve, reject) => {
        const hash = crypto.createHash('sha256');
        const stream = fs.createReadStream(filePath);
        stream.on('error', reject);
        stream.on('data', (chunk) => hash.update(chunk));
        stream.on('end', () => {
            const digest = hash.digest('hex');
            resolve(digest.toLowerCase() === expected.toLowerCase());
        });
    });
}

async function extractArchive(archivePath, targetDir, type) {
    if (type === 'zip' || type === undefined || type === null) {
        await extractZip(archivePath, { dir: targetDir });
        return;
    }
    if (type === 'tar.gz' || type === 'tgz' || type === 'tar.xz' || type === 'txz') {
        await tar.x({ file: archivePath, cwd: targetDir });
        return;
    }
    throw new Error(`Unsupported archive type: ${type}`);
}

async function prepareRuntimeFromDownload(api, artifact) {
    const runtimeBase = getRuntimeBase();
    const runtimeRoot = getRuntimeRoot();
    const stagingDir = path.join(runtimeBase, 'license-plate-staging');
    const downloadPath = path.join(os.tmpdir(), `license-plate-runtime-${Date.now()}`);

    await fsPromises.mkdir(runtimeBase, { recursive: true });
    emitProgress(api, { status: 'downloading', stage: 'start', url: artifact.url });
    await downloadFile(artifact.url, downloadPath, (p) => emitProgress(api, { status: 'downloading', ...p }));

    try {
        if (artifact.sha256) {
            emitProgress(api, { status: 'verifying' });
            const ok = await verifySha256(downloadPath, artifact.sha256);
            if (!ok) {
                throw new Error('Downloaded runtime checksum mismatch');
            }
        }

        emitProgress(api, { status: 'extracting' });
        await fsPromises.rm(stagingDir, { recursive: true, force: true });
        await fsPromises.mkdir(stagingDir, { recursive: true });
        await extractArchive(downloadPath, stagingDir, artifact.archiveType);

        await fsPromises.rm(runtimeRoot, { recursive: true, force: true });
        await fsPromises.mkdir(path.dirname(runtimeRoot), { recursive: true });
        await fsPromises.rename(stagingDir, runtimeRoot);

        const versionTag = computeVersionTag(artifact);
        const versionInfo = {
            version: versionTag,
            manifestVersion: manifest.version,
            artifact
        };
        await fsPromises.writeFile(path.join(runtimeRoot, VERSION_FILENAME), JSON.stringify(versionInfo, null, 2));

        return runtimeRoot;
    } finally {
        await fsPromises.unlink(downloadPath).catch(() => {});
        await fsPromises.rm(stagingDir, { recursive: true, force: true }).catch(() => {});
    }
}

async function ensureRuntimeReady(api, options = {}) {
    const { forceDownload = false, preferDownload = false } = options;

    if (!forceDownload && cachedRuntimeInfo && cachedRuntimeInfo.pythonPath && fs.existsSync(cachedRuntimeInfo.pythonPath)) {
        return cachedRuntimeInfo;
    }

    const devInfo = getDevelopmentRuntimeInfo();
    if (devInfo && devInfo.pythonPath && !forceDownload && !preferDownload) {
        cachedRuntimeInfo = { ...devInfo, version: 'dev' };
        emitProgress(api, { status: 'ready', mode: 'development', pythonPath: devInfo.pythonPath, version: 'dev' });
        return cachedRuntimeInfo;
    }

    if (activePromise) {
        return activePromise;
    }

    activePromise = (async () => {
        emitProgress(api, { status: 'checking' });
        const artifact = getArtifactForPlatform();
        if (!artifact || !artifact.url) {
            throw new Error('No runtime artifact defined for current platform');
        }
        const runtimeRoot = getRuntimeRoot();
        const versionTag = computeVersionTag(artifact);
        const runtimeInfo = await readRuntimeInfo(runtimeRoot);
        if (!forceDownload && runtimeInfo && runtimeInfo.version === versionTag) {
            const pythonPathCandidate = resolveWithinRuntime(runtimeRoot, artifact.pythonExecutable || 'python/python.exe');
            const scriptRootCandidate = resolveWithinRuntime(runtimeRoot, artifact.scriptRoot || '');
            if (fs.existsSync(pythonPathCandidate) && fs.existsSync(scriptRootCandidate)) {
                cachedRuntimeInfo = {
                    mode: 'downloaded',
                    runtimeRoot,
                    scriptRoot: scriptRootCandidate,
                    pythonPath: pythonPathCandidate,
                    version: runtimeInfo.version || versionTag
                };
                emitProgress(api, {
                    status: 'ready',
                    mode: 'cached',
                    pythonPath: cachedRuntimeInfo.pythonPath,
                    version: runtimeInfo.version || versionTag
                });
                return cachedRuntimeInfo;
            }
        }

        const preparedRoot = await prepareRuntimeFromDownload(api, artifact);
        const pythonPath = resolveWithinRuntime(preparedRoot, artifact.pythonExecutable || 'python/python.exe');
        const scriptRoot = resolveWithinRuntime(preparedRoot, artifact.scriptRoot || '');
        if (!fs.existsSync(pythonPath)) {
            throw new Error(`Runtime downloaded but python executable not found at ${pythonPath}`);
        }
        if (!fs.existsSync(scriptRoot)) {
            throw new Error(`Runtime downloaded but script root not found at ${scriptRoot}`);
        }
        cachedRuntimeInfo = {
            mode: 'downloaded',
            runtimeRoot: preparedRoot,
            scriptRoot,
            pythonPath,
            version: versionTag
        };
        emitProgress(api, { status: 'ready', mode: 'downloaded', pythonPath, version: versionTag });
        return cachedRuntimeInfo;
    })();

    try {
        const info = await activePromise;
        return info;
    } catch (err) {
        emitProgress(api, { status: 'error', message: err.message || String(err) });
        throw err;
    } finally {
        activePromise = null;
    }
}

function clearCachedRuntime() {
    cachedRuntimeInfo = null;
}

async function reinstallRuntime(api) {
    if (activePromise) {
        try {
            await activePromise;
        } catch (e) {
            // swallow error; reinstall will attempt a fresh download
        }
    }
    emitProgress(api, { status: 'resetting' });
    clearCachedRuntime();
    await fsPromises.rm(getRuntimeRoot(), { recursive: true, force: true });
    return ensureRuntimeReady(api, { forceDownload: true, preferDownload: true });
}

async function getRuntimeStatus() {
    const runtimeRoot = getRuntimeRoot();
    const artifact = getArtifactForPlatform();
    const storedInfo = await readRuntimeInfo(runtimeRoot);
    const devInfo = getDevelopmentRuntimeInfo();

    let runtime = cachedRuntimeInfo ? { ...cachedRuntimeInfo } : null;
    if (!runtime && artifact && storedInfo) {
        const pythonPath = resolveWithinRuntime(runtimeRoot, artifact.pythonExecutable || 'python/python.exe');
        const scriptRoot = resolveWithinRuntime(runtimeRoot, artifact.scriptRoot || '');
        if (fs.existsSync(pythonPath) && fs.existsSync(scriptRoot)) {
            runtime = {
                mode: 'downloaded',
                runtimeRoot,
                scriptRoot,
                pythonPath,
                version: storedInfo.version
            };
        }
    }

    if (!runtime && devInfo) {
        runtime = { ...devInfo, version: devInfo.version || 'dev' };
    }

    if (runtime && storedInfo && runtime.mode === 'downloaded' && !runtime.version) {
        runtime.version = storedInfo.version;
    }

    const pythonPath = runtime ? runtime.pythonPath : (artifact ? resolveWithinRuntime(runtimeRoot, artifact.pythonExecutable || 'python/python.exe') : null);
    const installed = pythonPath ? fs.existsSync(pythonPath) : false;

    return {
        manifestVersion: manifest.version,
        platformKey: detectPlatformKey(),
        artifact,
        runtimeRoot,
        runtime,
        storedInfo,
        installed,
        pythonPath,
        developmentAvailable: !!devInfo
    };
}

function resolvePythonScript(scriptName, runtimeInfo) {
    const info = runtimeInfo || cachedRuntimeInfo || getDevelopmentRuntimeInfo();
    if (!info || !info.scriptRoot) {
        return path.join(REPO_ROOT, 'python_src', scriptName);
    }
    return path.join(info.scriptRoot, scriptName);
}

module.exports = {
    ensureRuntimeReady,
    resolvePythonScript,
    getRuntimeRoot,
    getDevelopmentRuntimeInfo,
    reinstallRuntime,
    clearCachedRuntime,
    getRuntimeStatus
};
