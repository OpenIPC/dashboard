#include "DashboardWebDeploymentPolicy.h"

#include "DashboardHttpProtocol.h"

#include <QSet>
#include <QRegularExpression>
#include <QUrl>

namespace {

QString displayHost(const QString &address)
{
    return address == QStringLiteral("0.0.0.0") || address == QStringLiteral("::")
        ? QStringLiteral("127.0.0.1") : address;
}

QString normalizedProfile(const QVariantMap &settings)
{
    const QString requested = settings.value(QStringLiteral("webDeploymentProfile"))
                                  .toString().trimmed().toLower();
    static const QSet<QString> supported{
        QStringLiteral("localhost"), QStringLiteral("lan"), QStringLiteral("vpn"),
        QStringLiteral("reverse_proxy")
    };
    if (supported.contains(requested)) return requested;
    return settings.value(QStringLiteral("webServerAllowRemote"), false).toBool()
        ? QStringLiteral("lan") : QStringLiteral("localhost");
}

QUrl normalizedExternalUrl(const QString &value, const QStringList &schemes)
{
    QUrl url(value.trimmed());
    if (!url.isValid() || !schemes.contains(url.scheme().toLower()) || url.host().isEmpty()
        || !url.userInfo().isEmpty() || url.hasQuery() || url.hasFragment()) {
        return {};
    }
    QString path = url.path();
    if (path.isEmpty()) path = QStringLiteral("/");
    if (!path.startsWith(QLatin1Char('/'))) return {};
    url.setPath(path);
    return url;
}

QStringList trustedProxyList(const QVariant &value)
{
    QStringList result;
    const QStringList candidates = value.toString().split(
        QRegularExpression(QStringLiteral("[,;\\s]+")), Qt::SkipEmptyParts);
    for (const QString &candidate : candidates) {
        QHostAddress address;
        if (!address.setAddress(candidate.trimmed())) continue;
        const QString normalized = address.toString();
        if (!result.contains(normalized)) result.append(normalized);
    }
    return result;
}

bool sameOrigin(const QUrl &left, const QUrl &right)
{
    auto effectivePort = [](const QUrl &url) {
        if (url.port() > 0) return url.port();
        if (url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0
            || url.scheme().compare(QStringLiteral("wss"), Qt::CaseInsensitive) == 0) return 443;
        return 80;
    };
    return left.scheme().compare(right.scheme(), Qt::CaseInsensitive) == 0
        && left.host().compare(right.host(), Qt::CaseInsensitive) == 0
        && effectivePort(left) == effectivePort(right);
}

} // namespace

namespace DashboardWebDeploymentPolicy {

Config fromSettings(const QVariantMap &settings)
{
    Config config;
    config.profile = normalizedProfile(settings);
    config.configuredBindAddress = settings.value(
        QStringLiteral("webServerBindAddress"), QStringLiteral("127.0.0.1"))
                                       .toString().trimmed();
    if (config.configuredBindAddress.isEmpty()) {
        config.configuredBindAddress = QStringLiteral("127.0.0.1");
    }
    config.allowRemote = config.profile == QStringLiteral("lan")
        || config.profile == QStringLiteral("vpn")
        || (config.profile == QStringLiteral("reverse_proxy")
            && settings.value(QStringLiteral("webServerAllowRemote"), false).toBool());

    const QUrl externalBase = normalizedExternalUrl(
        settings.value(QStringLiteral("webExternalBaseUrl")).toString(),
        {QStringLiteral("https")});
    const QUrl externalWebSocket = normalizedExternalUrl(
        settings.value(QStringLiteral("webExternalWebSocketUrl")).toString(),
        {QStringLiteral("wss")});
    config.externalBaseUrl = externalBase.toString(QUrl::FullyEncoded);
    config.externalWebSocketUrl = externalWebSocket.toString(QUrl::FullyEncoded);
    config.trustedProxyAddresses = trustedProxyList(
        settings.value(QStringLiteral("webTrustedProxyAddresses")));
    config.secureCookies = settings.value(QStringLiteral("webSecureCookies"), false).toBool()
        || (config.profile == QStringLiteral("reverse_proxy") && !config.externalBaseUrl.isEmpty());

    if (config.profile == QStringLiteral("reverse_proxy")) {
        if (config.externalBaseUrl.isEmpty()) {
            config.valid = false;
            config.validationError = QStringLiteral(
                "Reverse proxy profile requires an HTTPS external base URL");
        } else if (config.trustedProxyAddresses.isEmpty()) {
            config.valid = false;
            config.validationError = QStringLiteral(
                "Reverse proxy profile requires at least one trusted proxy IP address");
        }
    }
    return config;
}

bool isTrustedProxy(const Config &config, const QString &peerAddress)
{
    QHostAddress peer;
    if (!peer.setAddress(peerAddress)) return false;
    return config.trustedProxyAddresses.contains(peer.toString());
}

bool originAllowed(const Config &config, const QByteArray &originHeader,
                   const QByteArray &hostHeader, const QString &peerAddress)
{
    if (originHeader.isEmpty()
        || DashboardHttpProtocol::originMatchesHost(originHeader, hostHeader)) {
        return true;
    }
    if (config.profile != QStringLiteral("reverse_proxy")
        || !isTrustedProxy(config, peerAddress) || config.externalBaseUrl.isEmpty()) {
        return false;
    }
    const QUrl origin(QString::fromUtf8(originHeader));
    const QUrl external(config.externalBaseUrl);
    return origin.isValid() && sameOrigin(origin, external);
}

QString localHttpUrl(const QString &effectiveBindAddress, int port)
{
    return QStringLiteral("http://%1:%2").arg(displayHost(effectiveBindAddress)).arg(port);
}

QString publicHttpUrl(const Config &config, const QString &effectiveBindAddress, int port)
{
    return config.profile == QStringLiteral("reverse_proxy") && !config.externalBaseUrl.isEmpty()
        ? config.externalBaseUrl : localHttpUrl(effectiveBindAddress, port);
}

QString publicWebSocketUrl(const Config &config, const QString &effectiveBindAddress,
                           int webSocketPort)
{
    if (config.profile == QStringLiteral("reverse_proxy")) {
        if (!config.externalWebSocketUrl.isEmpty()) return config.externalWebSocketUrl;
        if (!config.externalBaseUrl.isEmpty()) {
            QUrl url(config.externalBaseUrl);
            url.setScheme(QStringLiteral("wss"));
            url.setPath(QStringLiteral("/ws"));
            return url.toString(QUrl::FullyEncoded);
        }
    }
    return QStringLiteral("ws://%1:%2").arg(displayHost(effectiveBindAddress)).arg(webSocketPort);
}

QVariantMap publicStatus(const Config &config)
{
    return {
        {QStringLiteral("profile"), config.profile},
        {QStringLiteral("allowRemote"), config.allowRemote},
        {QStringLiteral("secureCookies"), config.secureCookies},
        {QStringLiteral("externalBaseUrlConfigured"), !config.externalBaseUrl.isEmpty()},
        {QStringLiteral("externalWebSocketUrlConfigured"), !config.externalWebSocketUrl.isEmpty()},
        {QStringLiteral("trustedProxyCount"), config.trustedProxyAddresses.size()},
        {QStringLiteral("valid"), config.valid},
        {QStringLiteral("validationError"), config.validationError}
    };
}

} // namespace DashboardWebDeploymentPolicy
