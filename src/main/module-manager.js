const { app } = require('electron');
const path = require('path');
const fs = require('fs');
const fsPromises = fs.promises;
const axios = require('axios');
const os = require('os');
const crypto = require('crypto');
const { pipeline } = require('stream/promises');
const extractZip = require('extract-zip');
const tar = require('tar');

class ModuleManager {
    constructor(appAPI) {
        this.appAPI = appAPI;
        this.availableModules = [];
        this.loadedModules = new Map();
        this.eventListeners = new Map();
        this.moduleRoots = [];
        this.registry = { modules: [] };
        this.registrySource = null;
        this.userModulesDir = null;
        this.installPromises = new Map();
    }

    registerListener(eventName, callback) {
        if (!this.eventListeners.has(eventName)) {
            this.eventListeners.set(eventName, []);
        }
        this.eventListeners.get(eventName).push(callback);
    }

    getListeners(eventName) {
        return this.eventListeners.get(eventName) || [];
    }

    unregisterListener(eventName, callback) {
        if (!this.eventListeners.has(eventName)) {
            return;
        }
        const listeners = this.eventListeners.get(eventName);
        const index = listeners.indexOf(callback);
        if (index > -1) {
            listeners.splice(index, 1);
        }
    }

    async initialize(options = {}) {
        const builtInDir = options.builtInDir || path.join(app.getAppPath(), 'modules');
        const appDataPath = typeof (this.appAPI?.configManager?.getDataPath) === 'function'
            ? this.appAPI.configManager.getDataPath()
            : null;
        const computedUserDir = appDataPath
            ? path.join(appDataPath, 'modules')
            : path.join(app.getPath('userData'), 'modules');
        const userDir = options.userDir || computedUserDir;

        this.moduleRoots = [];
        if (await this.#dirExists(builtInDir)) {
            this.moduleRoots.push({ path: builtInDir, writable: false, origin: 'builtin' });
        }
        await fsPromises.mkdir(userDir, { recursive: true });
        this.moduleRoots.push({ path: userDir, writable: true, origin: 'user' });
        this.userModulesDir = userDir;

        console.log('[ModuleManager] Модульные директории:', this.moduleRoots.map(root => `${root.origin}:${root.path}`).join(', '));

        await this.#loadRegistry(options.registryPath, options.registryUrl);
        if (this.registrySource) {
            console.log('[ModuleManager] Реестр модулей загружен из', this.registrySource);
        }
    }

    async discoverModules() {
        this.availableModules = [];
        for (const root of this.moduleRoots) {
            if (!(await this.#dirExists(root.path))) {
                continue;
            }
            let entries;
            try {
                entries = await fsPromises.readdir(root.path, { withFileTypes: true });
            } catch (err) {
                console.error('[ModuleManager] Не удалось прочитать каталог модулей', root.path, err);
                continue;
            }
            for (const entry of entries) {
                if (!entry.isDirectory()) continue;
                const modulePath = path.join(root.path, entry.name);
                const manifestPath = path.join(modulePath, 'plugin.json');
                if (!(await this.#fileExists(manifestPath))) {
                    continue;
                }
                try {
                    const manifest = JSON.parse(await fsPromises.readFile(manifestPath, 'utf-8'));
                    manifest.id = entry.name;
                    manifest.path = modulePath;
                    manifest.source = root.origin;
                    manifest.installInfo = await this.#readModuleInfo(modulePath);
                    const existingIndex = this.availableModules.findIndex(m => m.id === manifest.id);
                    if (existingIndex !== -1) {
                        const existing = this.availableModules[existingIndex];
                        if (existing.source === 'builtin' && root.writable) {
                            this.availableModules[existingIndex] = manifest;
                        }
                        continue;
                    }
                    this.availableModules.push(manifest);
                } catch (err) {
                    console.error(`Ошибка чтения манифеста для модуля ${entry.name}:`, err);
                }
            }
        }
        console.log(`[ModuleManager] Найдено модулей: ${this.availableModules.length}`);
    }

    async loadEnabledModules(currentAppSettings) {
        const enabledModules = currentAppSettings.enabledModules || [];
        for (const moduleId of enabledModules) {
            let manifest = this.availableModules.find(m => m.id === moduleId);
            if (!manifest) {
                try {
                    const result = await this.installModule(moduleId);
                    if (result && result.installed) {
                        await this.discoverModules();
                        manifest = this.availableModules.find(m => m.id === moduleId);
                    }
                } catch (err) {
                    console.error(`[ModuleManager] Не удалось установить модуль ${moduleId}:`, err.message || err);
                    continue;
                }
            }
            if (manifest && !this.loadedModules.has(moduleId)) {
                this.loadModule(manifest);
            }
        }
    }

    loadModule(manifest) {
        try {
            const mainEntryPoint = manifest.entryPoints?.main;
            if (!mainEntryPoint) {
                return;
            }
            const modulePath = path.join(manifest.path, mainEntryPoint);
            const moduleCode = require(modulePath);
            if (typeof moduleCode.activate === 'function') {
                moduleCode.activate(this.appAPI);
                this.loadedModules.set(manifest.id, { manifest, main: moduleCode });
                console.log(`[ModuleManager] Модуль "${manifest.name}" успешно загружен (main).`);
            }
        } catch (e) {
            console.error(`Ошибка загрузки main-части модуля ${manifest.name}:`, e);
        }
    }

    async installModule(moduleId, { force = false } = {}) {
        if (!this.userModulesDir) {
            throw new Error('ModuleManager не инициализирован');
        }
        if (this.installPromises.has(moduleId)) {
            return this.installPromises.get(moduleId);
        }

        const installPromise = (async () => {
            const registryEntry = this.getRegistryEntry(moduleId);
            if (!registryEntry) {
                throw new Error(`Модуль ${moduleId} отсутствует в реестре`);
            }

            const artifact = this.#resolveArtifact(registryEntry);
            if (!artifact || !artifact.url) {
                throw new Error(`Не найден артефакт для модуля ${moduleId}`);
            }

            const targetDir = path.join(this.userModulesDir, moduleId);
            if (!force) {
                const installedInfo = await this.#readModuleInfo(targetDir);
                if (installedInfo && installedInfo.version === registryEntry.version) {
                    console.log(`[ModuleManager] Модуль ${moduleId} уже установлен (v${installedInfo.version}).`);
                    return { installed: false, reason: 'up-to-date', path: targetDir };
                }
            }

            await fsPromises.mkdir(path.dirname(targetDir), { recursive: true });
            const archiveExt = artifact.archiveType || this.#guessArchiveType(artifact.url);
            let tempDir;
            try {
                tempDir = await fsPromises.mkdtemp(path.join(os.tmpdir(), `dash-mod-${moduleId}-`));
                const archiveName = artifact.fileName || `module.${archiveExt}`;
                const archivePath = path.join(tempDir, archiveName);

                await this.#downloadFile(artifact.url, archivePath);
                if (artifact.sha256) {
                    const ok = await this.#verifySha256(archivePath, artifact.sha256);
                    if (!ok) {
                        throw new Error(`Контрольная сумма не совпала для ${moduleId}`);
                    }
                }

                const stagingDir = path.join(tempDir, 'staging');
                await fsPromises.mkdir(stagingDir, { recursive: true });
                await this.#extractArchive(archivePath, stagingDir, archiveExt);

                const extractedRoot = await this.#findSingleRoot(stagingDir);
                await fsPromises.rm(targetDir, { recursive: true, force: true });
                try {
                    await fsPromises.rename(extractedRoot, targetDir);
                } catch (err) {
                    if (['EXDEV', 'EACCES', 'EPERM', 'ENOTEMPTY'].includes(err.code)) {
                        await this.#copyDirectory(extractedRoot, targetDir);
                    } else {
                        throw err;
                    }
                }
            } finally {
                if (tempDir) {
                    await fsPromises.rm(tempDir, { recursive: true, force: true }).catch(() => {});
                }
            }

            const info = {
                id: registryEntry.id,
                version: registryEntry.version,
                installedAt: new Date().toISOString(),
                registry: {
                    version: this.registry.registryVersion || 1,
                    url: artifact.url,
                    platform: artifact.platform || this.#detectPlatformKey(),
                    archiveType: archiveExt
                }
            };
            await fsPromises.writeFile(path.join(targetDir, 'module-info.json'), JSON.stringify(info, null, 2));

            console.log(`[ModuleManager] Модуль ${moduleId} установлен по пути ${targetDir}`);

            return { installed: true, path: targetDir, info };
        })()
            .finally(() => {
                this.installPromises.delete(moduleId);
            });

        this.installPromises.set(moduleId, installPromise);
        return installPromise;
    }

    getRegistryEntry(moduleId) {
        return (this.registry.modules || []).find(mod => mod.id === moduleId);
    }

    async #loadRegistry(registryPath, registryUrl) {
        let payload = null;
        if (registryUrl) {
            try {
                const response = await axios.get(registryUrl, { timeout: 10000 });
                payload = response.data;
                this.registrySource = registryUrl;
            } catch (err) {
                console.error('[ModuleManager] Не удалось загрузить удаленный реестр модулей:', err.message || err);
            }
        }

        const candidatePaths = [
            registryPath,
            path.join(app.getAppPath(), 'module-registry.json'),
            path.join(__dirname, '..', '..', 'module-registry.json')
        ].filter(Boolean);

        if (!payload) {
            for (const candidate of candidatePaths) {
                try {
                    const raw = await fsPromises.readFile(candidate, 'utf-8');
                    payload = JSON.parse(raw);
                    this.registrySource = candidate;
                    break;
                } catch (err) {
                    // keep trying other candidates
                }
            }
        }

        if (!payload) {
            payload = { registryVersion: 1, modules: [] };
        }

        if (!Array.isArray(payload.modules)) {
            payload.modules = [];
        }

        this.registry = payload;
    }

    async #readModuleInfo(modulePath) {
        if (!(await this.#dirExists(modulePath))) {
            return null;
        }
        const infoPath = path.join(modulePath, 'module-info.json');
        if (!(await this.#fileExists(infoPath))) {
            return null;
        }
        try {
            return JSON.parse(await fsPromises.readFile(infoPath, 'utf-8'));
        } catch (err) {
            console.warn(`[ModuleManager] Некорректный module-info.json для ${modulePath}:`, err.message || err);
            return null;
        }
    }

    async #downloadFile(url, destination) {
        const response = await axios.get(url, { responseType: 'stream', maxRedirects: 5 });
        await fsPromises.mkdir(path.dirname(destination), { recursive: true });
        await pipeline(response.data, fs.createWriteStream(destination));
    }

    async #verifySha256(filePath, expected) {
        return new Promise((resolve, reject) => {
            const hash = crypto.createHash('sha256');
            const stream = fs.createReadStream(filePath);
            stream.on('error', reject);
            stream.on('data', chunk => hash.update(chunk));
            stream.on('end', () => {
                const digest = hash.digest('hex');
                resolve(digest.toLowerCase() === expected.toLowerCase());
            });
        });
    }

    async #extractArchive(archivePath, targetDir, archiveType) {
        const type = archiveType || 'zip';
        if (type === 'zip') {
            await extractZip(archivePath, { dir: targetDir });
            return;
        }
        if (['tar.gz', 'tgz', 'tar.xz', 'txz'].includes(type)) {
            await tar.x({ file: archivePath, cwd: targetDir });
            return;
        }
        throw new Error(`Неизвестный тип архива: ${type}`);
    }

    async #findSingleRoot(stagingDir) {
        const entries = await fsPromises.readdir(stagingDir, { withFileTypes: true });
        if (entries.length === 1 && entries[0].isDirectory()) {
            return path.join(stagingDir, entries[0].name);
        }
        return stagingDir;
    }

    async #copyDirectory(srcDir, destDir) {
        await fsPromises.mkdir(destDir, { recursive: true });
        if (typeof fsPromises.cp === 'function') {
            await fsPromises.cp(srcDir, destDir, { recursive: true });
            return;
        }
        const entries = await fsPromises.readdir(srcDir, { withFileTypes: true });
        for (const entry of entries) {
            const src = path.join(srcDir, entry.name);
            const dest = path.join(destDir, entry.name);
            if (entry.isDirectory()) {
                await this.#copyDirectory(src, dest);
            } else {
                await fsPromises.copyFile(src, dest);
            }
        }
    }

    #detectPlatformKey() {
        return `${process.platform}-${process.arch}`;
    }

    #resolveArtifact(entry) {
        const platformKey = this.#detectPlatformKey();
        const platforms = entry.platforms || {};
        const artifact = platforms[platformKey] || platforms.default || entry.artifact || null;
        if (artifact && !artifact.platform) {
            artifact.platform = platformKey;
        }
        return artifact;
    }

    #guessArchiveType(url) {
        if (typeof url !== 'string') {
            return 'zip';
        }
        const lowered = url.toLowerCase();
        if (lowered.endsWith('.tar.xz')) return 'tar.xz';
        if (lowered.endsWith('.txz')) return 'tar.xz';
        if (lowered.endsWith('.tar.gz')) return 'tar.gz';
        if (lowered.endsWith('.tgz')) return 'tar.gz';
        if (lowered.endsWith('.zip')) return 'zip';
        return 'zip';
    }

    async #dirExists(dirPath) {
        try {
            const stat = await fsPromises.stat(dirPath);
            return stat.isDirectory();
        } catch (err) {
            return false;
        }
    }

    async #fileExists(filePath) {
        try {
            const stat = await fsPromises.stat(filePath);
            return stat.isFile();
        } catch (err) {
            return false;
        }
    }
}

module.exports = { ModuleManager };