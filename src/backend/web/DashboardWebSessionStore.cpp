#include "DashboardWebSessionStore.h"

#include <QCryptographicHash>
#include <QRandomGenerator>

#include <cstring>
#include <algorithm>

QVariantMap DashboardWebSessionStore::Session::toVariantMap() const
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    return {
        {QStringLiteral("username"), username},
        {QStringLiteral("role"), role},
        {QStringLiteral("permissions"), permissions},
        {QStringLiteral("cameraScopes"), cameraScopes},
        {QStringLiteral("createdAt"), createdAt.toString(Qt::ISODate)},
        {QStringLiteral("expiresAt"), expiresAt.toString(Qt::ISODate)},
        {QStringLiteral("absoluteExpiresAt"), absoluteExpiresAt.toString(Qt::ISODate)},
        {QStringLiteral("lastSeenAt"), lastSeenAt.toString(Qt::ISODate)},
        {QStringLiteral("idleTtlSeconds"), qMax<qint64>(0, now.secsTo(expiresAt))},
        {QStringLiteral("absoluteTtlSeconds"), qMax<qint64>(0, now.secsTo(absoluteExpiresAt))},
        {QStringLiteral("peerAddress"), peerAddress},
        {QStringLiteral("origin"), origin},
        {QStringLiteral("userAgent"), userAgent}
    };
}

DashboardWebSessionStore::DashboardWebSessionStore(QObject *parent)
    : QObject(parent)
{
}

void DashboardWebSessionStore::setTimeoutMinutes(int minutes)
{
    m_timeoutMinutes = qBound(5, minutes, 24 * 60);
}

QByteArray DashboardWebSessionStore::create(const QVariantMap &user, const QVariantMap &context)
{
    QByteArray rawToken(32, Qt::Uninitialized);
    for (qsizetype index = 0; index < rawToken.size(); index += 8) {
        const quint64 random = QRandomGenerator::system()->generate64();
        const qsizetype copySize = qMin<qsizetype>(8, rawToken.size() - index);
        memcpy(rawToken.data() + index, &random, static_cast<size_t>(copySize));
    }
    rawToken = rawToken.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    const QDateTime now = QDateTime::currentDateTimeUtc();
    Session session;
    session.username = user.value(QStringLiteral("username")).toString();
    session.role = user.value(QStringLiteral("role")).toString();
    session.permissions = user.value(QStringLiteral("permissions")).toInt();
    session.cameraScopes = user.value(QStringLiteral("cameraScopes")).toStringList();
    session.peerAddress = context.value(QStringLiteral("peerAddress")).toString().left(64);
    session.origin = context.value(QStringLiteral("origin")).toString().left(512);
    session.userAgent = context.value(QStringLiteral("userAgent")).toString().left(512);
    session.createdAt = now;
    session.lastSeenAt = now;
    session.absoluteExpiresAt = now.addSecs(24 * 60 * 60);
    session.expiresAt = qMin(now.addSecs(m_timeoutMinutes * 60), session.absoluteExpiresAt);
    m_sessions.insert(digest(rawToken), session);
    emit countChanged();
    return rawToken;
}

DashboardWebSessionStore::Session DashboardWebSessionStore::find(const QByteArray &rawToken, bool touch)
{
    if (rawToken.isEmpty()) return {};
    const QByteArray tokenDigest = digest(rawToken);
    auto it = m_sessions.find(tokenDigest);
    if (it == m_sessions.end()) return {};
    const QDateTime now = QDateTime::currentDateTimeUtc();
    if (it->expiresAt <= now || it->absoluteExpiresAt <= now) {
        m_sessions.erase(it);
        emit countChanged();
        return {};
    }
    if (touch) {
        it->lastSeenAt = now;
        it->expiresAt = qMin(now.addSecs(m_timeoutMinutes * 60), it->absoluteExpiresAt);
    }
    return *it;
}

bool DashboardWebSessionStore::remove(const QByteArray &rawToken)
{
    const bool removed = m_sessions.remove(digest(rawToken)) > 0;
    if (removed) emit countChanged();
    return removed;
}

bool DashboardWebSessionStore::removeById(const QString &sessionId)
{
    const QByteArray digestBytes = QByteArray::fromHex(sessionId.toLatin1());
    if (digestBytes.size() != 32 || idForDigest(digestBytes) != sessionId.toLower()) return false;
    const bool removed = m_sessions.remove(digestBytes) > 0;
    if (removed) emit countChanged();
    return removed;
}

int DashboardWebSessionStore::removeForUser(const QString &username)
{
    int removed = 0;
    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        if (it->username == username) {
            it = m_sessions.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }
    if (removed > 0) emit countChanged();
    return removed;
}

QVariantList DashboardWebSessionStore::sessions(const QByteArray &currentToken) const
{
    QVariantList result;
    const QByteArray currentDigest = currentToken.isEmpty() ? QByteArray() : digest(currentToken);
    const QDateTime now = QDateTime::currentDateTimeUtc();
    for (auto it = m_sessions.constBegin(); it != m_sessions.constEnd(); ++it) {
        if (it->expiresAt <= now || it->absoluteExpiresAt <= now) continue;
        QVariantMap item = it->toVariantMap();
        item.insert(QStringLiteral("id"), idForDigest(it.key()));
        item.insert(QStringLiteral("current"), !currentDigest.isEmpty() && it.key() == currentDigest);
        result.append(item);
    }
    std::sort(result.begin(), result.end(), [](const QVariant &left, const QVariant &right) {
        return left.toMap().value(QStringLiteral("lastSeenAt")).toString()
            > right.toMap().value(QStringLiteral("lastSeenAt")).toString();
    });
    return result;
}

void DashboardWebSessionStore::clear()
{
    if (m_sessions.isEmpty()) return;
    m_sessions.clear();
    emit countChanged();
}

void DashboardWebSessionStore::cleanupExpired()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    int removed = 0;
    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        if (it->expiresAt <= now || it->absoluteExpiresAt <= now) {
            it = m_sessions.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }
    if (removed > 0) emit countChanged();
}

QByteArray DashboardWebSessionStore::digest(const QByteArray &rawToken)
{
    return QCryptographicHash::hash(rawToken, QCryptographicHash::Sha256);
}

QString DashboardWebSessionStore::idForDigest(const QByteArray &tokenDigest)
{
    return QString::fromLatin1(tokenDigest.toHex());
}
