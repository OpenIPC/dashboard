#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QSettings>

class UserManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap currentUser READ currentUser NOTIFY currentUserChanged)
    Q_PROPERTY(QVariantList users READ users NOTIFY usersChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)

public:
    // Granular permissions (Dahua-style)
    enum Permission {
        Perm_None           = 0x00,
        Perm_LiveView       = 0x01, // Monitor
        Perm_Playback       = 0x02, // Playback
        Perm_PTZ            = 0x04, // PTZ Control
        Perm_Export         = 0x08, // Backup/Export
        Perm_Settings       = 0x10, // System Config
        Perm_UserManage     = 0x20, // Account Management
        Perm_All            = 0xFF  // Super Admin
    };
    Q_ENUM(Permission)

    explicit UserManager(QObject *parent = nullptr);

    QVariantMap currentUser() const;
    QVariantList users() const;
    bool isLoggedIn() const;

    Q_INVOKABLE bool login(const QString &username, const QString &password, bool rememberMe = false);
    Q_INVOKABLE void logout();
    Q_INVOKABLE bool addUser(const QString &username, const QString &password, const QString &role, int permissions = 0);
    Q_INVOKABLE bool deleteUser(const QString &username);
    Q_INVOKABLE bool changePassword(const QString &username, const QString &oldPassword, const QString &newPassword);
    Q_INVOKABLE bool isAdmin() const;
    
    // Permission checks
    Q_INVOKABLE bool hasPermission(int permission) const;
    Q_INVOKABLE void updateUserPermissions(const QString &username, int permissions);

signals:
    void currentUserChanged();
    void usersChanged();
    void isLoggedInChanged();

private:
    void loadUsers();
    void saveUsers();
    QString hashPassword(const QString &password) const;

    struct User {
        QString username;
        QString passwordHash;
        QString role;
        int permissions = 0;
        
        QJsonObject toJson() const {
            return QJsonObject{
                {"username", username},
                {"passwordHash", passwordHash},
                {"role", role},
                {"permissions", permissions}
            };
        }

        static User fromJson(const QJsonObject &json) {
            User u;
            u.username = json["username"].toString();
            u.passwordHash = json["passwordHash"].toString();
            u.role = json["role"].toString();
            // Backward compatibility: if no permissions stored, infer from role
            if (json.contains("permissions")) {
                u.permissions = json["permissions"].toInt();
            } else {
                if (u.role == "admin") u.permissions = Perm_All;
                else u.permissions = Perm_LiveView | Perm_Playback | Perm_PTZ;
            }
            return u;
        }
    };

    QList<User> m_users;
    User m_currentUser;
    bool m_isLoggedIn;
};

#endif // USERMANAGER_H
