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
    Q_PROPERTY(bool hasUsers READ hasUsers NOTIFY usersChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
    Q_PROPERTY(int permissionsVersion READ permissionsVersion NOTIFY permissionsVersionChanged)
    Q_PROPERTY(int currentPermissions READ currentPermissions NOTIFY currentUserChanged)
    Q_PROPERTY(QString rememberedUsername READ rememberedUsername NOTIFY rememberedUsernameChanged)

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
        Perm_Analytics      = 0x40, // Analytics workspace
        Perm_All            = 0xFF  // Super Admin
    };
    Q_ENUM(Permission)

    explicit UserManager(QObject *parent = nullptr);

    QVariantMap currentUser() const;
    QVariantList users() const;
    bool hasUsers() const { return !m_users.isEmpty(); }
    bool isLoggedIn() const;
    int permissionsVersion() const { return m_permissionsVersion; }
    int currentPermissions() const { return m_currentUser.permissions; }
    QString rememberedUsername() const { return m_rememberedUsername; }

    Q_INVOKABLE bool login(const QString &username, const QString &password, bool rememberMe = false);
    Q_INVOKABLE void logout();
    Q_INVOKABLE bool setupInitialAdmin(const QString &username, const QString &password, bool rememberMe = false);
    Q_INVOKABLE bool addUser(const QString &username, const QString &password, const QString &role, int permissions = -1);
    Q_INVOKABLE bool deleteUser(const QString &username);
    Q_INVOKABLE bool changePassword(const QString &username, const QString &oldPassword, const QString &newPassword);
    Q_INVOKABLE bool isAdmin() const;
    
    // Permission checks
    Q_INVOKABLE bool hasPermission(int permission) const;
    Q_INVOKABLE bool canLiveView() const { return hasPermission(Perm_LiveView); }
    Q_INVOKABLE bool canPlayback() const { return hasPermission(Perm_Playback); }
    Q_INVOKABLE bool canPtz() const { return hasPermission(Perm_PTZ); }
    Q_INVOKABLE bool canExport() const { return hasPermission(Perm_Export); }
    Q_INVOKABLE bool canSettings() const { return hasPermission(Perm_Settings); }
    Q_INVOKABLE bool canUserManage() const { return hasPermission(Perm_UserManage); }
    Q_INVOKABLE bool canAnalytics() const { return hasPermission(Perm_Analytics); }
    Q_INVOKABLE void updateUserPermissions(const QString &username, int permissions);

signals:
    void currentUserChanged();
    void usersChanged();
    void isLoggedInChanged();
    void permissionsVersionChanged();
    void rememberedUsernameChanged();

private:
    struct User {
        QString username;
        QString passwordHash;
        QString passwordSalt;
        QString passwordAlgorithm;
        int passwordIterations = 0;
        QString role;
        int permissions = 0;
        
        QJsonObject toJson() const {
            QJsonObject json{
                {"username", username},
                {"passwordHash", passwordHash},
                {"role", role},
                {"permissions", permissions}
            };

            if (!passwordSalt.isEmpty()) {
                json["passwordSalt"] = passwordSalt;
            }
            if (!passwordAlgorithm.isEmpty()) {
                json["passwordAlgorithm"] = passwordAlgorithm;
            }
            if (passwordIterations > 0) {
                json["passwordIterations"] = passwordIterations;
            }

            return json;
        }

        static User fromJson(const QJsonObject &json) {
            User u;
            u.username = json["username"].toString();
            u.passwordHash = json["passwordHash"].toString();
            u.passwordSalt = json["passwordSalt"].toString();
            u.passwordAlgorithm = json["passwordAlgorithm"].toString();
            u.passwordIterations = json["passwordIterations"].toInt();
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

    void loadUsers();
    void saveUsers();
    QString usersFilePath() const;
    QString hashLegacyPassword(const QString &password) const;
    QByteArray derivePasswordKey(const QString &password, const QByteArray &salt, int iterations, int keyLength) const;
    void setPassword(User &user, const QString &password) const;
    bool verifyPassword(const User &user, const QString &password, bool *needsUpgrade = nullptr) const;
    void setRememberedUsername(const QString &username);

    QList<User> m_users;
    User m_currentUser;
    bool m_isLoggedIn;
    int m_permissionsVersion = 0;
    QString m_rememberedUsername;
};

#endif // USERMANAGER_H
