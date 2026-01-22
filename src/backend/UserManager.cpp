#include "UserManager.h"
#include <QDebug>

UserManager::UserManager(QObject *parent) : QObject(parent), m_isLoggedIn(false)
{
    loadUsers();
    if (m_users.isEmpty()) {
        // Create default admin user with full permissions
        addUser("admin", "admin", "admin", Perm_All);
    }

    // Check for remembered session
    QSettings settings;
    if (settings.value("auth/remember", false).toBool()) {
        QString username = settings.value("auth/username").toString();
        for (const auto &user : m_users) {
            if (user.username == username) {
                m_currentUser = user;
                m_isLoggedIn = true;
                break;
            }
        }
    }
}

QVariantMap UserManager::currentUser() const
{
    if (!m_isLoggedIn) return QVariantMap();
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
    // Refresh users from disk to ensure latest permissions are used
    loadUsers();
    QString hash = hashPassword(password);
    for (const auto &user : m_users) {
        if (user.username == username && user.passwordHash == hash) {
            m_currentUser = user;
            // Force admin to full permissions
            if (m_currentUser.username == "admin" || m_currentUser.role == "admin") {
                if ((m_currentUser.permissions & Perm_All) != Perm_All) {
                    m_currentUser.permissions = Perm_All;
                    for (int i = 0; i < m_users.size(); ++i) {
                        if (m_users[i].username == m_currentUser.username) {
                            m_users[i].permissions = Perm_All;
                            break;
                        }
                    }
                    saveUsers();
                }
            }
            m_isLoggedIn = true;
            qInfo() << "Logged in as" << m_currentUser.username << "permissions" << m_currentUser.permissions;

            QSettings settings;
            if (rememberMe) {
                settings.setValue("auth/remember", true);
                settings.setValue("auth/username", username);
            } else {
                settings.setValue("auth/remember", false);
                settings.remove("auth/username");
            }

            emit currentUserChanged();
            emit isLoggedInChanged();
            m_permissionsVersion++;
            emit permissionsVersionChanged();
            return true;
        }
    }
    return false;
}

void UserManager::logout()
{
    m_isLoggedIn = false;
    m_currentUser = User();

    QSettings settings;
    settings.setValue("auth/remember", false);
    settings.remove("auth/username");

    emit currentUserChanged();
    emit isLoggedInChanged();
    m_permissionsVersion++;
    emit permissionsVersionChanged();
}

bool UserManager::addUser(const QString &username, const QString &password, const QString &role, int permissions)
{
    for (const auto &user : m_users) {
        if (user.username == username) return false; // User already exists
    }

    User newUser;
    newUser.username = username;
    newUser.passwordHash = hashPassword(password);
    newUser.role = role;
    
    // If permissions not specified (<0), set defaults based on role
    if (permissions < 0) {
        if (role == "admin" || username == "admin") newUser.permissions = Perm_All;
        else newUser.permissions = Perm_LiveView | Perm_Playback | Perm_PTZ;
    } else {
        newUser.permissions = (role == "admin" || username == "admin") ? Perm_All : permissions;
    }
    
    m_users.append(newUser);
    saveUsers();
    emit usersChanged();
    m_permissionsVersion++;
    emit permissionsVersionChanged();
    return true;
}

bool UserManager::hasPermission(int permission) const
{
    if (!m_isLoggedIn) return false;
    if (m_currentUser.username == "admin" || m_currentUser.role == "admin") return true;
    if ((m_currentUser.permissions & Perm_All) == Perm_All) return true;
    return (m_currentUser.permissions & permission) == permission;
}

void UserManager::updateUserPermissions(const QString &username, int permissions)
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].username == username) {
            if (m_users[i].username == "admin" || m_users[i].role == "admin") {
                m_users[i].permissions = Perm_All;
            } else {
                m_users[i].permissions = permissions;
            }
            
            // If updating current user, refresh session
            if (m_isLoggedIn && m_currentUser.username == username) {
                m_currentUser.permissions = m_users[i].permissions;
                emit currentUserChanged();
            }
            
            saveUsers();
            emit usersChanged();
            m_permissionsVersion++;
            emit permissionsVersionChanged();
            return;
        }
    }
}

bool UserManager::deleteUser(const QString &username)
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].username == username) {
            // Prevent deleting the last admin
            if (m_users[i].role == "admin") {
                int adminCount = 0;
                for (const auto &u : m_users) {
                    if (u.role == "admin") adminCount++;
                }
                if (adminCount <= 1) return false;
            }
            
            m_users.removeAt(i);
            saveUsers();
            emit usersChanged();
            m_permissionsVersion++;
            emit permissionsVersionChanged();
            return true;
        }
    }
    return false;
}

bool UserManager::changePassword(const QString &username, const QString &oldPassword, const QString &newPassword)
{
    QString oldHash = hashPassword(oldPassword);
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].username == username) {
            // Verify old password
            if (m_users[i].passwordHash != oldHash) {
                return false;
            }

            m_users[i].passwordHash = hashPassword(newPassword);
            saveUsers();
            return true;
        }
    }
    return false;
}

bool UserManager::isAdmin() const
{
    return m_isLoggedIn && (m_currentUser.username == "admin" || m_currentUser.role == "admin" || (m_currentUser.permissions & Perm_All) == Perm_All);
}

void UserManager::loadUsers()
{
    QString path = usersFilePath();
    qInfo() << "UserManager users file:" << path;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open users file:" << path;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray array = doc.array();

    m_users.clear();
    bool changed = false;
    for (const auto &val : array) {
        User u = User::fromJson(val.toObject());
        if (u.username == "admin" || u.role == "admin") {
            if ((u.permissions & Perm_All) != Perm_All) {
                u.permissions = Perm_All;
                changed = true;
            }
        }
        m_users.append(u);
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
    QString path = usersFilePath();
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

    QJsonDocument doc(array);
    file.write(doc.toJson());

    // If we wrote to AppConfigLocation, remove stale AppDataLocation copy to avoid confusion
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/users.json";
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/users.json";
    if (path == configPath && QFile::exists(dataPath) && dataPath != configPath) {
        QFile::remove(dataPath);
    }
}

QString UserManager::hashPassword(const QString &password) const
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString UserManager::usersFilePath() const
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/users.json";
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/users.json";

    bool hasConfig = QFile::exists(configPath);
    bool hasData = QFile::exists(dataPath);

    if (hasData && !hasConfig) {
        QDir dir = QFileInfo(configPath).dir();
        if (!dir.exists()) dir.mkpath(".");
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
