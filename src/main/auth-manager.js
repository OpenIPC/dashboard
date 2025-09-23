// --- ФАЙЛ: src/main/auth-manager.js ---

const fsPromises = require('fs').promises;
const path = require('path');
const crypto = require('crypto');
const keytar = require('keytar');

// Используем функцию, чтобы избежать циклических зависимостей при require,
// так как config-manager тоже может понадобиться.
function getUsersPath() {
    const { getDataPath } = require('./config-manager');
    return path.join(getDataPath(), 'users.json');
}

const KEYTAR_SERVICE = 'OpenIPC-VMS';
const KEYTAR_ACCOUNT_AUTOLOGIN = 'autoLoginCredentials';
const KEYTAR_MASTER_ACCOUNT = 'master_encryption_key';

// credentials file stores encrypted blobs per camera id
function getCredentialsPath() {
    const { getDataPath } = require('./config-manager');
    return path.join(getDataPath(), 'credentials.json');
}

async function readCredentialsFile() {
    try {
        const p = getCredentialsPath();
        const data = await fsPromises.readFile(p, 'utf-8');
        return JSON.parse(data || '{}');
    } catch (e) {
        return {};
    }
}

async function writeCredentialsFile(obj) {
    try {
        const p = getCredentialsPath();
        await fsPromises.writeFile(p, JSON.stringify(obj, null, 2));
        try { require('fs').chmodSync(p, 0o600); } catch (e) { /* ignore on windows */ }
    } catch (e) {
        console.error('[AuthManager] Failed to write credentials file:', e);
    }
}

async function ensureMasterKey() {
    let key = await keytar.getPassword(KEYTAR_SERVICE, KEYTAR_MASTER_ACCOUNT);
    if (!key) {
        // generate 32 bytes key and store as base64
        const buf = crypto.randomBytes(32);
        key = buf.toString('base64');
        await keytar.setPassword(KEYTAR_SERVICE, KEYTAR_MASTER_ACCOUNT, key);
    }
    return Buffer.from(key, 'base64');
}

function aesGcmEncrypt(masterKeyBuf, plain) {
    const iv = crypto.randomBytes(12);
    const cipher = crypto.createCipheriv('aes-256-gcm', masterKeyBuf, iv);
    const encrypted = Buffer.concat([cipher.update(plain, 'utf8'), cipher.final()]);
    const tag = cipher.getAuthTag();
    return Buffer.concat([iv, tag, encrypted]).toString('base64');
}

function aesGcmDecrypt(masterKeyBuf, blob) {
    const data = Buffer.from(blob, 'base64');
    const iv = data.slice(0, 12);
    const tag = data.slice(12, 28);
    const encrypted = data.slice(28);
    const decipher = crypto.createDecipheriv('aes-256-gcm', masterKeyBuf, iv);
    decipher.setAuthTag(tag);
    const decrypted = Buffer.concat([decipher.update(encrypted), decipher.final()]);
    return decrypted.toString('utf8');
}

/**
 * Хеширует пароль с использованием соленого PBKDF2.
 * @param {string} password - Пароль для хеширования.
 * @returns {{salt: string, hash: string}} Объект с солью и хешем.
 */
function hashPassword(password) {
    const salt = crypto.randomBytes(16).toString('hex');
    const hash = crypto.pbkdf2Sync(password, salt, 1000, 64, 'sha512').toString('hex');
    return { salt, hash };
}

/**
 * Проверяет, совпадает ли пароль с хешем.
 * @param {string} password - Пароль для проверки.
 * @param {string} hash - Сохраненный хеш.
 * @param {string} salt - Сохраненная соль.
 * @returns {boolean} True, если пароль верный.
 */
function verifyPassword(password, hash, salt) {
    const hashToVerify = crypto.pbkdf2Sync(password, salt, 1000, 64, 'sha512').toString('hex');
    return hash === hashToVerify;
}

/**
 * Получает пароль для камеры из безопасного хранилища.
 * @param {string|number} cameraId - ID камеры.
 * @returns {Promise<string|null>} Пароль или null, если не найден.
 */
async function getPasswordForCamera(cameraId) {
    // Try credentials.json first (encrypted storage)
    try {
        const creds = await readCredentialsFile();
        if (creds && creds[cameraId]) {
            const master = await ensureMasterKey();
            try {
                const dec = aesGcmDecrypt(master, creds[cameraId]);
                return dec;
            } catch (e) {
                console.error('[AuthManager] Failed to decrypt camera password:', e);
                return null;
            }
        }
    } catch (e) { /* ignore */ }

    // Fallback: legacy keytar per-camera. If found, migrate into encrypted store.
    try {
        const legacy = await keytar.getPassword(KEYTAR_SERVICE, cameraId.toString());
        if (legacy) {
            // migrate
            await setPasswordForCamera(cameraId, legacy);
            try { await keytar.deletePassword(KEYTAR_SERVICE, cameraId.toString()); } catch (e) { /* ignore */ }
            return legacy;
        }
    } catch (e) { /* ignore */ }

    return null;
}

/**
 * Сохраняет пароль для камеры в зашифрованном виде в credentials.json
 */
async function setPasswordForCamera(cameraId, password) {
    try {
        const master = await ensureMasterKey();
        const blob = aesGcmEncrypt(master, password);
        const creds = await readCredentialsFile();
        creds[cameraId.toString()] = blob;
        await writeCredentialsFile(creds);
        return true;
    } catch (e) {
        console.error('[AuthManager] Failed to set encrypted password for camera:', e);
        return false;
    }
}

/**
 * Обрабатывает попытку входа пользователя.
 * @param {object} credentials - Данные для входа.
 * @returns {Promise<object>} Результат операции.
 */
async function handleLogin({ username, password, rememberMe }) {
    try {
        const data = await fsPromises.readFile(getUsersPath(), 'utf-8');
        const users = JSON.parse(data);
        const user = users.find(u => u.username === username);

        if (user && verifyPassword(password, user.hashedPassword, user.salt)) {
            if (rememberMe) {
                await keytar.setPassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN, JSON.stringify({ username, password }));
                console.log('[Login] Credentials saved for auto-login.');
            } else {
                await keytar.deletePassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN);
                console.log('[Login] Cleared any stored auto-login credentials.');
            }
            const userPayload = { 
                username: user.username, 
                role: user.role, 
                permissions: user.permissions || {} 
            };
            return { success: true, user: userPayload };
        }
        return { success: false, error: 'Invalid username or password' };
    } catch (e) {
        console.error('Login error:', e);
        return { success: false, error: 'Error reading user data' };
    }
}

/**
 * Пытается выполнить автоматический вход при запуске приложения.
 * @param {BrowserWindow} mainWindow - Главное окно для отправки результата.
 */
async function handleAutoLogin(mainWindow) {
    try {
        const credsJson = await keytar.getPassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN);
        if (credsJson) {
            const { username, password } = JSON.parse(credsJson);
            const loginResult = await handleLogin({ username, password, rememberMe: true });

            if (loginResult.success && mainWindow && !mainWindow.isDestroyed()) {
                console.log('[AutoLogin] Success.');
                mainWindow.webContents.send('auto-login-success', loginResult.user);
            } else if (!loginResult.success) {
                console.warn('[AutoLogin] Failed. Stored credentials may be outdated.');
                await clearAutoLoginCredentials();
            }
        }
    } catch (e) {
        console.error('[AutoLogin] Error:', e);
    }
}

/**
 * Очищает сохраненные учетные данные для автоматического входа.
 */
async function clearAutoLoginCredentials() {
    try {
        await keytar.deletePassword(KEYTAR_SERVICE, KEYTAR_ACCOUNT_AUTOLOGIN);
        console.log('[Logout] Cleared auto-login credentials.');
    } catch (e) {
        console.error('[Logout] Failed to clear credentials:', e);
    }
}

// --- CRUD операции для пользователей ---

async function getUsers() {
    try {
        const data = await fsPromises.readFile(getUsersPath(), 'utf-8');
        const users = JSON.parse(data);
        // Возвращаем только публичные данные, без хешей и солей
        return { 
            success: true, 
            users: users.map(u => ({ username: u.username, role: u.role, permissions: u.permissions || {} })) 
        };
    } catch (e) {
        return { success: false, error: e.message };
    }
}

async function addUser({ username, password, role }) {
    try {
        const data = await fsPromises.readFile(getUsersPath(), 'utf-8');
        const users = JSON.parse(data);
        if (users.some(u => u.username.toLowerCase() === username.toLowerCase())) {
            return { success: false, error: 'User with this name already exists.' };
        }
        const { salt, hash } = hashPassword(password);
        const newUser = { 
            username, 
            salt, 
            hashedPassword: hash, 
            role, 
            permissions: role === 'operator' ? {} : undefined 
        };
        users.push(newUser);
        await fsPromises.writeFile(getUsersPath(), JSON.stringify(users, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
}

async function updateUserPassword({ username, password }) {
    try {
        const data = await fsPromises.readFile(getUsersPath(), 'utf-8');
        let users = JSON.parse(data);
        const userIndex = users.findIndex(u => u.username === username);
        if (userIndex === -1) {
            return { success: false, error: 'User not found.' };
        }
        
        const { salt, hash } = hashPassword(password);
        users[userIndex].salt = salt;
        users[userIndex].hashedPassword = hash;
        await fsPromises.writeFile(getUsersPath(), JSON.stringify(users, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
}

async function updateUserRole({ username, role }) {
    try {
        const data = await fsPromises.readFile(getUsersPath(), 'utf-8');
        let users = JSON.parse(data);
        const userIndex = users.findIndex(u => u.username === username);
        if (userIndex === -1) {
            return { success: false, error: 'User not found.' };
        }

        // Защита от удаления последнего администратора
        if (users[userIndex].role === 'admin' && role !== 'admin') {
            const admins = users.filter(u => u.role === 'admin');
            if (admins.length <= 1) {
                return { success: false, error: 'Cannot change the role of the last administrator.' };
            }
        }
        
        users[userIndex].role = role;
        if (role === 'operator' && !users[userIndex].permissions) {
            users[userIndex].permissions = {};
        }
        if (role === 'admin') {
            delete users[userIndex].permissions;
        }

        await fsPromises.writeFile(getUsersPath(), JSON.stringify(users, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
}

async function updateUserPermissions({ username, permissions }) {
    try {
        const data = await fsPromises.readFile(getUsersPath(), 'utf-8');
        let users = JSON.parse(data);
        const userIndex = users.findIndex(u => u.username === username);
        if (userIndex === -1 || users[userIndex].role !== 'operator') {
            return { success: false, error: 'User not found or is not an operator.' };
        }
        users[userIndex].permissions = permissions;
        await fsPromises.writeFile(getUsersPath(), JSON.stringify(users, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
}

async function deleteUser({ username }) {
    try {
        const data = await fsPromises.readFile(getUsersPath(), 'utf-8');
        let users = JSON.parse(data);
        
        const admins = users.filter(u => u.role === 'admin');
        if (admins.length === 1 && admins[0].username === username) {
            return { success: false, error: 'Cannot delete the last administrator.' };
        }

        const filteredUsers = users.filter(u => u.username !== username);
        await fsPromises.writeFile(getUsersPath(), JSON.stringify(filteredUsers, null, 2));
        return { success: true };
    } catch (e) {
        return { success: false, error: e.message };
    }
}

module.exports = {
    hashPassword,
    verifyPassword,
    getPasswordForCamera,
    setPasswordForCamera,
    handleLogin,
    handleAutoLogin,
    clearAutoLoginCredentials,
    getUsers,
    addUser,
    updateUserPassword,
    updateUserRole,
    updateUserPermissions,
    deleteUser,
};