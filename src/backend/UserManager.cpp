#include "UserManager.h"
#include "AppPaths.h"

#include <QDebug>
#include <QEventLoop>
#include <QFileInfo>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <keychain.h>

namespace {
constexpr int kPasswordIterations = 120000;
constexpr int kPasswordKeyLength = 32;
constexpr int kPasswordSaltLength = 16;
constexpr char kPasswordAlgorithm[] = "pbkdf2-sha256";
constexpr char kLoginKeychainService[] = "OpenIPC.Dashboard";
constexpr char kTestSecretStoreEnv[] = "OPENIPC_TEST_SECRET_STORE";

QByteArray randomSalt(int length)
{
    QByteArray salt(length, Qt::Uninitialized);
    for (int i = 0; i < length; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    return salt;
}
}

UserManager::UserManager(QObject *parent)
    : QObject(parent)
    , m_isLoggedIn(false)
{
    loadUsers();

    QSettings settings;
    if (settings.value("auth/remember", false).toBool()) {
        const QString username = settings.value("auth/username").toString().trimmed();
        const QString legacyPassword = settings.value("auth/password").toString();
        if (!legacyPassword.isEmpty()) {
            if (!username.isEmpty() && writeLoginSecret(username, legacyPassword)) {
                qInfo() << "Migrated remembered login secret to the platform keychain";
            }
            settings.remove("auth/password");
            settings.sync();
        }

        const QString password = legacyPassword.isEmpty() ? readLoginSecret(username) : legacyPassword;
        for (const auto &user : m_users) {
            if (user.username == username && !password.isEmpty()) {
                m_rememberedUsername = username;
                m_rememberedPassword = password;
                break;
            }
        }

        if (m_rememberedUsername.isEmpty()) {
            settings.setValue("auth/remember", false);
            settings.remove("auth/username");
            settings.remove("auth/password");
            deleteLoginSecret(username);
            settings.sync();
        }
    }
}

QVariantMap UserManager::currentUser() const
{
    if (!m_isLoggedIn) {
        return QVariantMap();
    }

    return QVariantMap{
        {"username", m_currentUser.username},
        {"role", m_currentUser.role},
        {"permissions", m_currentUser.permissions}
    };
}

QVariantList UserManager::users() const
{
    QVariantList list;
    for (const auto &user : m_users) {
        list.append(QVariantMap{
            {"username", user.username},
            {"role", user.role},
            {"permissions", user.permissions}
        });
    }
    return list;
}

bool UserManager::isLoggedIn() const
{
    return m_isLoggedIn;
}

bool UserManager::login(const QString &username, const QString &password, bool rememberMe)
{
    loadUsers();
    const QString normalizedUsername = username.trimmed();

    for (int i = 0; i < m_users.size(); ++i) {
        User &storedUser = m_users[i];
        bool needsUpgrade = false;
        if (storedUser.username != normalizedUsername || !verifyPassword(storedUser, password, &needsUpgrade)) {
            continue;
        }

        bool changed = false;
        if ((storedUser.username == "admin" || storedUser.role == "admin")
            && (storedUser.permissions & Perm_All) != Perm_All) {
            storedUser.permissions = Perm_All;
            changed = true;
        }

        if (needsUpgrade) {
            setPassword(storedUser, password);
            changed = true;
        }

        if (changed) {
            saveUsers();
        }

        m_currentUser = storedUser;
        m_isLoggedIn = true;
        qInfo() << "Logged in as" << m_currentUser.username << "permissions" << m_currentUser.permissions;

        if (rememberMe) {
            setRememberedCredentials(normalizedUsername, password);
        } else {
            setRememberedCredentials(QString(), QString());
        }

        emit currentUserChanged();
        emit isLoggedInChanged();
        m_permissionsVersion++;
        emit permissionsVersionChanged();
        return true;
    }

    return false;
}

bool UserManager::authenticateForSession(const QString &username, const QString &password,
                                         QVariantMap *userInfo)
{
    const QString normalizedUsername = username.trimmed();
    if (normalizedUsername.isEmpty() || password.isEmpty()) {
        return false;
    }

    for (User &storedUser : m_users) {
        bool needsUpgrade = false;
        if (storedUser.username != normalizedUsername
            || !verifyPassword(storedUser, password, &needsUpgrade)) {
            continue;
        }

        bool changed = false;
        if ((storedUser.username == QStringLiteral("admin")
             || storedUser.role == QStringLiteral("admin"))
            && (storedUser.permissions & Perm_All) != Perm_All) {
            storedUser.permissions = Perm_All;
            changed = true;
        }
        if (needsUpgrade) {
            setPassword(storedUser, password);
            changed = true;
        }
        if (changed) {
            saveUsers();
        }
        if (userInfo) {
            *userInfo = {
                {QStringLiteral("username"), storedUser.username},
                {QStringLiteral("role"), storedUser.role},
                {QStringLiteral("permissions"), storedUser.permissions}
            };
        }
        return true;
    }
    return false;
}

bool UserManager::loginWithRememberedCredentials()
{
    if (m_rememberedUsername.isEmpty() || m_rememberedPassword.isEmpty()) {
        return false;
    }

    const QString username = m_rememberedUsername;
    const QString password = m_rememberedPassword;
    if (login(username, password, true)) {
        return true;
    }

    setRememberedCredentials(QString(), QString());
    return false;
}

void UserManager::logout()
{
    m_isLoggedIn = false;
    m_currentUser = User();

    emit currentUserChanged();
    emit isLoggedInChanged();
    m_permissionsVersion++;
    emit permissionsVersionChanged();
}

bool UserManager::setupInitialAdmin(const QString &username, const QString &password, bool rememberMe)
{
    loadUsers();
    const QString normalizedUsername = username.trimmed();
    if (!m_users.isEmpty() || normalizedUsername.isEmpty() || password.isEmpty()) {
        return false;
    }

    User adminUser;
    adminUser.username = normalizedUsername;
    adminUser.role = QStringLiteral("admin");
    adminUser.permissions = Perm_All;
    setPassword(adminUser, password);

    m_users.append(adminUser);
    saveUsers();
    emit usersChanged();

    m_currentUser = adminUser;
    m_isLoggedIn = true;
    if (rememberMe) {
        setRememberedCredentials(normalizedUsername, password);
    } else {
        setRememberedCredentials(QString(), QString());
    }

    emit currentUserChanged();
    emit isLoggedInChanged();
    m_permissionsVersion++;
    emit permissionsVersionChanged();
    emit userSecurityChanged(normalizedUsername);
    return true;
}

bool UserManager::addUser(const QString &username, const QString &password, const QString &role, int permissions)
{
    const QString normalizedUsername = username.trimmed();
    if (normalizedUsername.isEmpty() || password.isEmpty()) {
        return false;
    }

    for (const auto &user : m_users) {
        if (user.username == normalizedUsername) {
            return false;
        }
    }

    User newUser;
    newUser.username = normalizedUsername;
    newUser.role = role;
    setPassword(newUser, password);

    if (permissions < 0) {
        if (role == "admin" || normalizedUsername == "admin") {
            newUser.permissions = Perm_All;
        } else {
            newUser.permissions = Perm_LiveView | Perm_Playback | Perm_PTZ | Perm_Analytics;
        }
    } else {
        newUser.permissions = (role == "admin" || normalizedUsername == "admin") ? Perm_All : permissions;
    }

    m_users.append(newUser);
    saveUsers();
    emit usersChanged();
    m_permissionsVersion++;
    emit permissionsVersionChanged();
    emit userSecurityChanged(normalizedUsername);
    return true;
}

bool UserManager::hasPermission(int permission) const
{
    if (!m_isLoggedIn) {
        return false;
    }
    if (m_currentUser.username == "admin" || m_currentUser.role == "admin") {
        return true;
    }
    if ((m_currentUser.permissions & Perm_All) == Perm_All) {
        return true;
    }
    return (m_currentUser.permissions & permission) == permission;
}

void UserManager::updateUserPermissions(const QString &username, int permissions)
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].username != username) {
            continue;
        }

        if (m_users[i].username == "admin" || m_users[i].role == "admin") {
            m_users[i].permissions = Perm_All;
        } else {
            m_users[i].permissions = permissions;
        }

        if (m_isLoggedIn && m_currentUser.username == username) {
            m_currentUser.permissions = m_users[i].permissions;
            emit currentUserChanged();
        }

        saveUsers();
        emit usersChanged();
        m_permissionsVersion++;
        emit permissionsVersionChanged();
        emit userSecurityChanged(username);
        return;
    }
}

bool UserManager::deleteUser(const QString &username)
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].username != username) {
            continue;
        }

        if (m_users[i].role == "admin") {
            int adminCount = 0;
            for (const auto &user : m_users) {
                if (user.role == "admin") {
                    adminCount++;
                }
            }
            if (adminCount <= 1) {
                return false;
            }
        }

        m_users.removeAt(i);
        saveUsers();
        if (m_rememberedUsername == username) {
            setRememberedCredentials(QString(), QString());
        }
        emit usersChanged();
        m_permissionsVersion++;
        emit permissionsVersionChanged();
        emit userSecurityChanged(username);
        return true;
    }

    return false;
}

bool UserManager::changePassword(const QString &username, const QString &oldPassword, const QString &newPassword)
{
    if (newPassword.isEmpty()) {
        return false;
    }

    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].username != username) {
            continue;
        }

        if (!verifyPassword(m_users[i], oldPassword)) {
            return false;
        }

        setPassword(m_users[i], newPassword);
        saveUsers();
        if (m_rememberedUsername == username) {
            setRememberedCredentials(username, newPassword);
        }
        emit userSecurityChanged(username);
        return true;
    }

    return false;
}

bool UserManager::isAdmin() const
{
    return m_isLoggedIn
        && (m_currentUser.username == "admin"
            || m_currentUser.role == "admin"
            || (m_currentUser.permissions & Perm_All) == Perm_All);
}

void UserManager::loadUsers()
{
    const QString path = usersFilePath();
    qInfo() << "UserManager users file:" << path;

    m_users.clear();

    QFile file(path);
    if (!file.exists()) {
        qInfo() << "Users file does not exist yet:" << path;
        emit usersChanged();
        m_permissionsVersion++;
        emit permissionsVersionChanged();
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open users file:" << path;
        emit usersChanged();
        m_permissionsVersion++;
        emit permissionsVersionChanged();
        return;
    }

    const QByteArray data = file.readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonArray array = doc.array();

    bool changed = false;
    for (const auto &value : array) {
        User user = User::fromJson(value.toObject());
        if ((user.username == "admin" || user.role == "admin") && (user.permissions & Perm_All) != Perm_All) {
            user.permissions = Perm_All;
            changed = true;
        }
        m_users.append(user);
    }

    if (changed) {
        saveUsers();
    }

    emit usersChanged();
    m_permissionsVersion++;
    emit permissionsVersionChanged();
}

void UserManager::saveUsers()
{
    const QString path = usersFilePath();
    QDir dir = QFileInfo(path).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not save users file:" << path;
        return;
    }

    QJsonArray array;
    for (const auto &user : m_users) {
        array.append(user.toJson());
    }

    const QJsonDocument doc(array);
    file.write(doc.toJson());

    const QString configPath = AppPaths::configDirectory() + "/users.json";
    const QString dataPath = AppPaths::dataDirectory() + "/users.json";
    if (path == configPath && QFile::exists(dataPath) && dataPath != configPath) {
        QFile::remove(dataPath);
    }
}

QString UserManager::hashLegacyPassword(const QString &password) const
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QByteArray UserManager::derivePasswordKey(const QString &password, const QByteArray &salt, int iterations, int keyLength) const
{
    if (iterations <= 0 || keyLength <= 0) {
        return QByteArray();
    }

    const QByteArray secret = password.toUtf8();
    const int hashLength = 32;
    const int blockCount = (keyLength + hashLength - 1) / hashLength;
    QByteArray derived;
    derived.reserve(blockCount * hashLength);

    for (int blockIndex = 1; blockIndex <= blockCount; ++blockIndex) {
        QByteArray blockSalt = salt;
        blockSalt.append(static_cast<char>((blockIndex >> 24) & 0xFF));
        blockSalt.append(static_cast<char>((blockIndex >> 16) & 0xFF));
        blockSalt.append(static_cast<char>((blockIndex >> 8) & 0xFF));
        blockSalt.append(static_cast<char>(blockIndex & 0xFF));

        QByteArray u = QMessageAuthenticationCode::hash(blockSalt, secret, QCryptographicHash::Sha256);
        QByteArray t = u;
        for (int iteration = 1; iteration < iterations; ++iteration) {
            u = QMessageAuthenticationCode::hash(u, secret, QCryptographicHash::Sha256);
            for (int byteIndex = 0; byteIndex < t.size(); ++byteIndex) {
                t[byteIndex] = static_cast<char>(t[byteIndex] ^ u[byteIndex]);
            }
        }

        derived.append(t);
    }

    derived.truncate(keyLength);
    return derived;
}

void UserManager::setPassword(User &user, const QString &password) const
{
    const QByteArray salt = randomSalt(kPasswordSaltLength);
    user.passwordSalt = QString::fromLatin1(salt.toHex());
    user.passwordIterations = kPasswordIterations;
    user.passwordAlgorithm = QString::fromLatin1(kPasswordAlgorithm);
    user.passwordHash = QString::fromLatin1(derivePasswordKey(password, salt, user.passwordIterations, kPasswordKeyLength).toHex());
}

bool UserManager::verifyPassword(const User &user, const QString &password, bool *needsUpgrade) const
{
    if (needsUpgrade) {
        *needsUpgrade = false;
    }

    if (user.passwordAlgorithm == QLatin1StringView(kPasswordAlgorithm) && !user.passwordSalt.isEmpty() && user.passwordIterations > 0) {
        const QByteArray salt = QByteArray::fromHex(user.passwordSalt.toLatin1());
        const QString derivedHash = QString::fromLatin1(
            derivePasswordKey(password, salt, user.passwordIterations, kPasswordKeyLength).toHex());
        return !derivedHash.isEmpty() && derivedHash == user.passwordHash;
    }

    const bool matchesLegacy = hashLegacyPassword(password) == user.passwordHash;
    if (matchesLegacy && needsUpgrade) {
        *needsUpgrade = true;
    }
    return matchesLegacy;
}

void UserManager::setRememberedCredentials(const QString &username, const QString &password)
{
    const QString normalizedUsername = username.trimmed();
    bool remember = !normalizedUsername.isEmpty() && !password.isEmpty();

    const QString previousUsername = m_rememberedUsername;
    if (remember && !writeLoginSecret(normalizedUsername, password)) {
        qWarning() << "Could not persist remembered login secret in the platform keychain";
        remember = false;
    }

    if (!previousUsername.isEmpty() && (!remember || previousUsername != normalizedUsername)) {
        deleteLoginSecret(previousUsername);
    }

    QSettings settings;
    // Never retain the legacy plaintext field. A failed secure save must not
    // silently downgrade credential storage back to QSettings.
    settings.remove("auth/password");
    if (!remember) {
        settings.setValue("auth/remember", false);
        settings.remove("auth/username");
    } else {
        settings.setValue("auth/remember", true);
        settings.setValue("auth/username", normalizedUsername);
    }
    settings.sync();

    const QString nextUsername = remember ? normalizedUsername : QString();
    const QString nextPassword = remember ? password : QString();
    const bool usernameChanged = m_rememberedUsername != nextUsername;
    const bool passwordChanged = m_rememberedPassword != nextPassword;

    if (!usernameChanged && !passwordChanged) {
        return;
    }

    m_rememberedUsername = nextUsername;
    m_rememberedPassword = nextPassword;
    if (usernameChanged) {
        emit rememberedUsernameChanged();
    }
    if (passwordChanged) {
        emit rememberedPasswordChanged();
    }
}

QString UserManager::loginSecretKey(const QString &username) const
{
    return QStringLiteral("login/") + username.trimmed();
}

QString UserManager::readLoginSecret(const QString &username) const
{
    if (username.trimmed().isEmpty()) {
        return {};
    }

    const QString key = loginSecretKey(username);
    if (qEnvironmentVariable(kTestSecretStoreEnv) == QLatin1String("settings")) {
        QSettings settings;
        return settings.value(QStringLiteral("test-secrets/") + key).toString();
    }

    QKeychain::ReadPasswordJob job(QString::fromLatin1(kLoginKeychainService));
    job.setKey(key);
    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error()) {
        if (job.error() != QKeychain::EntryNotFound) {
            qWarning() << "Could not read remembered login secret:" << job.errorString();
        }
        return {};
    }
    return job.textData();
}

bool UserManager::writeLoginSecret(const QString &username, const QString &password) const
{
    if (username.trimmed().isEmpty() || password.isEmpty()) {
        return false;
    }

    const QString key = loginSecretKey(username);
    if (qEnvironmentVariable(kTestSecretStoreEnv) == QLatin1String("settings")) {
        QSettings settings;
        settings.setValue(QStringLiteral("test-secrets/") + key, password);
        settings.sync();
        return settings.status() == QSettings::NoError;
    }

    QKeychain::WritePasswordJob job(QString::fromLatin1(kLoginKeychainService));
    job.setKey(key);
    job.setTextData(password);
    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
    if (job.error()) {
        qWarning() << "Could not write remembered login secret:" << job.errorString();
        return false;
    }
    return true;
}

void UserManager::deleteLoginSecret(const QString &username) const
{
    if (username.trimmed().isEmpty()) {
        return;
    }

    const QString key = loginSecretKey(username);
    if (qEnvironmentVariable(kTestSecretStoreEnv) == QLatin1String("settings")) {
        QSettings settings;
        settings.remove(QStringLiteral("test-secrets/") + key);
        settings.sync();
        return;
    }

    QKeychain::DeletePasswordJob job(QString::fromLatin1(kLoginKeychainService));
    job.setKey(key);
    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
    if (job.error() && job.error() != QKeychain::EntryNotFound) {
        qWarning() << "Could not delete remembered login secret:" << job.errorString();
    }
}

QString UserManager::usersFilePath() const
{
    const QString configPath = AppPaths::configDirectory() + "/users.json";
    const QString dataPath = AppPaths::dataDirectory() + "/users.json";

    const bool hasConfig = QFile::exists(configPath);
    const bool hasData = QFile::exists(dataPath);

    if (hasData && !hasConfig) {
        QDir dir = QFileInfo(configPath).dir();
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        QFile::copy(dataPath, configPath);
        QFile::remove(dataPath);
        return configPath;
    }

    if (hasConfig && hasData) {
        QFileInfo cfgInfo(configPath);
        QFileInfo dataInfo(dataPath);
        if (dataInfo.lastModified() > cfgInfo.lastModified()) {
            QFile::remove(configPath);
            QFile::copy(dataPath, configPath);
        }
        QFile::remove(dataPath);
        return configPath;
    }

    return configPath;
}
