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
    QString hash = hashPassword(password);
    for (const auto &user : m_users) {
        if (user.username == username && user.passwordHash == hash) {
            m_currentUser = user;
            m_isLoggedIn = true;

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
    
    // If permissions not specified (0), set defaults based on role
    if (permissions == 0) {
        if (role == "admin") newUser.permissions = Perm_All;
        else newUser.permissions = Perm_LiveView | Perm_Playback | Perm_PTZ;
    } else {
        newUser.permissions = permissions;
    }
    
    m_users.append(newUser);
    saveUsers();
    emit usersChanged();
    return true;
}

bool UserManager::hasPermission(int permission) const
{
    if (!m_isLoggedIn) return false;
    // Admin role always has full access regardless of bits (safety net)
    if (m_currentUser.role == "admin") return true;
    return (m_currentUser.permissions & permission) == permission;
}

void UserManager::updateUserPermissions(const QString &username, int permissions)
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].username == username) {
            m_users[i].permissions = permissions;
            
            // If updating current user, refresh session
            if (m_isLoggedIn && m_currentUser.username == username) {
                m_currentUser.permissions = permissions;
                emit currentUserChanged();
            }
            
            saveUsers();
            emit usersChanged();
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
    return m_isLoggedIn && m_currentUser.role == "admin";
}

void UserManager::loadUsers()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/users.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open users file:" << path;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray array = doc.array();

    m_users.clear();
    for (const auto &val : array) {
        m_users.append(User::fromJson(val.toObject()));
    }
    emit usersChanged();
}

void UserManager::saveUsers()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/users.json";
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
}

QString UserManager::hashPassword(const QString &password) const
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}
