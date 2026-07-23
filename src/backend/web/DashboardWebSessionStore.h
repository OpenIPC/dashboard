#pragma once

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class DashboardWebSessionStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    struct Session {
        QString username;
        QString role;
        int permissions = 0;
        QDateTime createdAt;
        QDateTime expiresAt;
        QDateTime absoluteExpiresAt;
        QDateTime lastSeenAt;
        QString peerAddress;
        QString origin;
        QString userAgent;

        bool isValid() const {
            const QDateTime now = QDateTime::currentDateTimeUtc();
            return !username.isEmpty() && expiresAt > now && absoluteExpiresAt > now;
        }
        QVariantMap toVariantMap() const;
    };

    explicit DashboardWebSessionStore(QObject *parent = nullptr);

    int count() const { return m_sessions.size(); }
    void setTimeoutMinutes(int minutes);
    QByteArray create(const QVariantMap &user, const QVariantMap &context = {});
    Session find(const QByteArray &rawToken, bool touch = true);
    bool remove(const QByteArray &rawToken);
    bool removeById(const QString &sessionId);
    int removeForUser(const QString &username);
    QVariantList sessions(const QByteArray &currentToken = {}) const;
    void clear();
    void cleanupExpired();

signals:
    void countChanged();

private:
    static QByteArray digest(const QByteArray &rawToken);
    static QString idForDigest(const QByteArray &tokenDigest);

    QHash<QByteArray, Session> m_sessions;
    int m_timeoutMinutes = 60;
};
