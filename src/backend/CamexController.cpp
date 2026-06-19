#include "CamexController.h"
#include "PathUtils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QElapsedTimer>

CamexController::CamexController(QObject *parent)
    : QObject(parent)
{
}

QString CamexController::normalizeClientId(const QString &value) const
{
    QString id = value.trimmed().toUpper();
    id.remove(QRegularExpression(QStringLiteral("[^A-Z0-9_-]")));
    return id.left(48);
}

QString CamexController::buildServerCommand(const QVariantMap &settings) const
{
    const QString binary = optionValue(settings, QStringLiteral("binary"), QStringLiteral("camex"));
    const QString configPath = optionValue(settings, QStringLiteral("configPath"), QStringLiteral("/etc/camex/camex.conf"));
    const QString bindIp = optionValue(settings, QStringLiteral("bindIp"), QStringLiteral("0.0.0.0"));
    const QString transport = optionValue(settings, QStringLiteral("transport"), QStringLiteral("udp")).toLower();
    const QString psk = optionValue(settings, QStringLiteral("psk"));
    const QString tunDev = optionValue(settings, QStringLiteral("tunDev"));
    const QString bindDev = optionValue(settings, QStringLiteral("bindDev"));
    const int port = optionInt(settings, QStringLiteral("port"), 5800);

    QStringList args;
    args << QStringLiteral("sudo") << shellQuote(binary)
         << QStringLiteral("--mode server")
         << QStringLiteral("--port %1").arg(port)
         << QStringLiteral("--bind-ip %1").arg(shellQuote(bindIp))
         << QStringLiteral("--config %1").arg(shellQuote(configPath))
         << QStringLiteral("--transport %1").arg(shellQuote(transport));

    if (optionBool(settings, QStringLiteral("encrypt"), !psk.isEmpty())) {
        args << QStringLiteral("--encrypt");
    }
    if (!psk.isEmpty()) {
        args << QStringLiteral("--psk %1").arg(shellQuote(psk));
    }
    if (!tunDev.isEmpty()) {
        args << QStringLiteral("--tun-dev %1").arg(shellQuote(tunDev));
    }
    if (!bindDev.isEmpty()) {
        args << QStringLiteral("--bind-dev %1").arg(shellQuote(bindDev));
    }

    return args.join(QLatin1Char(' '));
}

QString CamexController::buildClientCommand(const QVariantMap &settings) const
{
    const QString binary = optionValue(settings, QStringLiteral("binary"), QStringLiteral("camex"));
    const QString clientId = normalizeClientId(optionValue(settings, QStringLiteral("clientId"), QStringLiteral("OPENIPC-CAMERA")));
    const QString serverHost = optionValue(settings, QStringLiteral("serverHost"), QStringLiteral("vpn.example.org"));
    const QString transport = optionValue(settings, QStringLiteral("transport"), QStringLiteral("udp")).toLower();
    const QString psk = optionValue(settings, QStringLiteral("psk"));
    const QString localCidr = optionValue(settings, QStringLiteral("clientCidr"), QStringLiteral("10.0.0.2/24"));
    const QString gatewayIp = optionValue(settings, QStringLiteral("gatewayIp"), QStringLiteral("10.0.0.1"));
    const QString tunDev = optionValue(settings, QStringLiteral("tunDev"));
    const bool autoMode = optionBool(settings, QStringLiteral("autoMode"), true);
    const int port = optionInt(settings, QStringLiteral("port"), 5800);

    QStringList args;
    args << shellQuote(binary)
         << QStringLiteral("--mode client")
         << QStringLiteral("--server-host %1").arg(shellQuote(serverHost))
         << QStringLiteral("--port %1").arg(port)
         << QStringLiteral("--transport %1").arg(shellQuote(transport));

    if (autoMode) {
        args << QStringLiteral("--auto")
             << QStringLiteral("--name %1").arg(shellQuote(clientId));
    } else {
        args << QStringLiteral("--local-cidr %1").arg(shellQuote(localCidr))
             << QStringLiteral("--gateway-ip %1").arg(shellQuote(gatewayIp));
        const QStringList routes = routeList(settings.value(QStringLiteral("routeCidrs")));
        for (const QString &route : routes) {
            args << QStringLiteral("--route-cidr %1").arg(shellQuote(route));
        }
    }

    if (optionBool(settings, QStringLiteral("encrypt"), !psk.isEmpty())) {
        args << QStringLiteral("--encrypt");
    }
    if (!psk.isEmpty()) {
        args << QStringLiteral("--psk %1").arg(shellQuote(psk));
    }
    if (!tunDev.isEmpty()) {
        args << QStringLiteral("--tun-dev %1").arg(shellQuote(tunDev));
    }

    return args.join(QLatin1Char(' '));
}

QString CamexController::buildServerConfig(const QVariantMap &settings) const
{
    const QString clientId = normalizeClientId(optionValue(settings, QStringLiteral("clientId"), QStringLiteral("OPENIPC-CAMERA")));
    const QString bindIp = optionValue(settings, QStringLiteral("bindIp"), QStringLiteral("0.0.0.0"));
    const QString serverCidr = optionValue(settings, QStringLiteral("serverCidr"), QStringLiteral("10.0.0.1/24"));
    const QString clientCidr = optionValue(settings, QStringLiteral("clientCidr"), QStringLiteral("10.0.0.2/24"));
    const QString gatewayIp = optionValue(settings, QStringLiteral("gatewayIp"), QStringLiteral("10.0.0.1"));
    const QString psk = optionValue(settings, QStringLiteral("psk"));
    const QString clientPsk = optionValue(settings, QStringLiteral("clientPsk"));
    const QString tunDev = optionValue(settings, QStringLiteral("tunDev"));
    const QString bindDev = optionValue(settings, QStringLiteral("bindDev"));
    const int port = optionInt(settings, QStringLiteral("port"), 5800);
    const int mtu = optionInt(settings, QStringLiteral("mtu"), 1500);
    const bool encrypt = optionBool(settings, QStringLiteral("encrypt"), !psk.isEmpty() || !clientPsk.isEmpty());

    QStringList lines;
    lines << QStringLiteral("# /etc/camex/camex.conf")
          << QStringLiteral("# Server-side camex configuration generated by OpenIPC Dashboard.")
          << QStringLiteral("# Give every camera a unique [client ID] section and unique local_cidr.")
          << QStringLiteral("")
          << QStringLiteral("[server]")
          << QStringLiteral("port=%1").arg(port)
          << QStringLiteral("bind_ip=%1").arg(bindIp)
          << QStringLiteral("local_cidr=%1").arg(serverCidr)
          << QStringLiteral("mtu=%1").arg(mtu)
          << QStringLiteral("encrypt=%1").arg(encrypt ? QStringLiteral("yes") : QStringLiteral("no"));

    if (!psk.isEmpty()) {
        lines << QStringLiteral("psk=%1").arg(psk);
    }
    if (!tunDev.isEmpty()) {
        lines << QStringLiteral("tun_dev=%1").arg(tunDev);
    }
    if (!bindDev.isEmpty()) {
        lines << QStringLiteral("bind_dev=%1").arg(bindDev);
    }

    lines << QStringLiteral("")
          << QStringLiteral("[defaults]")
          << QStringLiteral("gateway_ip=%1").arg(gatewayIp)
          << QStringLiteral("mtu=%1").arg(mtu);

    const QStringList routes = routeList(settings.value(QStringLiteral("routeCidrs")));
    for (const QString &route : routes) {
        lines << QStringLiteral("route_cidr=%1").arg(route);
    }

    lines << QStringLiteral("")
          << QStringLiteral("[client %1]").arg(clientId)
          << QStringLiteral("local_cidr=%1").arg(clientCidr)
          << QStringLiteral("gateway_ip=%1").arg(gatewayIp);

    for (const QString &route : routes) {
        lines << QStringLiteral("route_cidr=%1").arg(route);
    }
    if (!clientPsk.isEmpty()) {
        lines << QStringLiteral("psk=%1").arg(clientPsk);
    }

    return lines.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

bool CamexController::saveTextFile(const QString &pathOrUrl, const QString &content) const
{
    const QString path = PathUtils::localPathFromUserInput(pathOrUrl);
    if (path.isEmpty()) {
        return false;
    }

    QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    file.write(content.toUtf8());
    file.close();
    return true;
}

QVariantMap CamexController::checkTcpPort(const QString &host, int port, int timeoutMs) const
{
    QVariantMap result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("message")] = QStringLiteral("TCP check was not started.");

    const QString trimmedHost = host.trimmed();
    if (trimmedHost.isEmpty() || port <= 0 || port > 65535) {
        result[QStringLiteral("message")] = QStringLiteral("Specify a valid host and port.");
        return result;
    }

    QElapsedTimer elapsed;
    elapsed.start();

    QTcpSocket socket;
    socket.connectToHost(trimmedHost, static_cast<quint16>(port));
    if (socket.waitForConnected(qBound(250, timeoutMs, 10000))) {
        socket.disconnectFromHost();
        result[QStringLiteral("ok")] = true;
        result[QStringLiteral("message")] = QStringLiteral("TCP port is reachable in %1 ms.").arg(elapsed.elapsed());
    } else {
        result[QStringLiteral("message")] = QStringLiteral("TCP port is not reachable: %1").arg(socket.errorString());
    }

    result[QStringLiteral("elapsedMs")] = static_cast<int>(elapsed.elapsed());
    return result;
}

QString CamexController::shellQuote(const QString &value)
{
    if (value.isEmpty()) {
        return QStringLiteral("''");
    }

    QString quoted = value;
    quoted.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QStringLiteral("'") + quoted + QStringLiteral("'");
}

QString CamexController::optionValue(const QVariantMap &settings, const QString &key, const QString &fallback)
{
    const QString value = settings.value(key).toString().trimmed();
    return value.isEmpty() ? fallback : value;
}

int CamexController::optionInt(const QVariantMap &settings, const QString &key, int fallback)
{
    bool ok = false;
    const int value = settings.value(key).toInt(&ok);
    return ok ? value : fallback;
}

bool CamexController::optionBool(const QVariantMap &settings, const QString &key, bool fallback)
{
    if (!settings.contains(key)) {
        return fallback;
    }
    return settings.value(key).toBool();
}

QStringList CamexController::routeList(const QVariant &value)
{
    QStringList routes;
    if (value.metaType().id() == QMetaType::QStringList) {
        routes = value.toStringList();
    } else if (value.canConvert<QVariantList>()) {
        const QVariantList list = value.toList();
        for (const QVariant &item : list) {
            routes.append(item.toString());
        }
    } else {
        routes = value.toString().split(QRegularExpression(QStringLiteral("[,\\n;\\s]+")), Qt::SkipEmptyParts);
    }

    for (QString &route : routes) {
        route = route.trimmed();
    }
    routes.removeAll(QString());
    routes.removeDuplicates();
    return routes;
}
