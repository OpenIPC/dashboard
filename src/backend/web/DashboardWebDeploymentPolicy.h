#pragma once

#include <QHostAddress>
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace DashboardWebDeploymentPolicy {

struct Config
{
    QString profile = QStringLiteral("localhost");
    QString configuredBindAddress = QStringLiteral("127.0.0.1");
    QString externalBaseUrl;
    QString externalWebSocketUrl;
    QStringList trustedProxyAddresses;
    bool allowRemote = false;
    bool secureCookies = false;
    bool valid = true;
    QString validationError;
};

Config fromSettings(const QVariantMap &settings);
bool isTrustedProxy(const Config &config, const QString &peerAddress);
bool originAllowed(const Config &config, const QByteArray &originHeader,
                   const QByteArray &hostHeader, const QString &peerAddress);
QString localHttpUrl(const QString &effectiveBindAddress, int port);
QString publicHttpUrl(const Config &config, const QString &effectiveBindAddress, int port);
QString publicWebSocketUrl(const Config &config, const QString &effectiveBindAddress,
                           int webSocketPort);
QVariantMap publicStatus(const Config &config);

} // namespace DashboardWebDeploymentPolicy
