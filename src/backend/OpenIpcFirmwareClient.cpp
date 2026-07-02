#include "OpenIpcFirmwareClient.h"

#include "PathUtils.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMetaObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSharedPointer>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
#include <QAbstractSocket>
#include <QWebSocket>
#include <QWebSocketProtocol>
#endif

#include <algorithm>

namespace {

constexpr int kTransferTimeoutMs = 15000;
constexpr qint64 kMaximumFirmwareArchiveBytes = 128LL * 1024LL * 1024LL;

QByteArray limitedBody(const QByteArray &body)
{
    constexpr qsizetype maximum = 4096;
    return body.size() <= maximum ? body : body.left(maximum) + QByteArrayLiteral("…");
}

QString htmlDecode(QString text)
{
    text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    text.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
    text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));

    static const QRegularExpression decimalEntity(QStringLiteral("&#(\\d+);"));
    QRegularExpressionMatchIterator it = decimalEntity.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        bool ok = false;
        const uint code = match.captured(1).toUInt(&ok);
        if (ok) {
            text.replace(match.captured(0), QString(QChar(code)));
        }
    }
    return text;
}

QString stripTags(QString html)
{
    html.replace(QRegularExpression(QStringLiteral("<script\\b[^>]*>.*?</script>"),
                                    QRegularExpression::DotMatchesEverythingOption
                                    | QRegularExpression::CaseInsensitiveOption),
                 QString());
    html.replace(QRegularExpression(QStringLiteral("<style\\b[^>]*>.*?</style>"),
                                    QRegularExpression::DotMatchesEverythingOption
                                    | QRegularExpression::CaseInsensitiveOption),
                 QString());
    html.replace(QRegularExpression(QStringLiteral("<br\\s*/?>"),
                                    QRegularExpression::CaseInsensitiveOption),
                 QStringLiteral("\n"));
    html.replace(QRegularExpression(QStringLiteral("</p>|</div>|</dt>|</dd>|</li>|</tr>"),
                                    QRegularExpression::CaseInsensitiveOption),
                 QStringLiteral("\n"));
    html.remove(QRegularExpression(QStringLiteral("<[^>]+>")));
    return htmlDecode(html).simplified();
}

QString rawTagForField(const QString &html, const QString &tagName, const QString &fieldName)
{
    const QString escaped = QRegularExpression::escape(fieldName);
    const QRegularExpression rx(
        QStringLiteral("<%1\\b[^>]*(?:id|name)\\s*=\\s*['\"]%2['\"][^>]*>")
            .arg(tagName, escaped),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) return match.captured(0);

    const QRegularExpression rxReverse(
        QStringLiteral("<%1\\b[^>]*(?:id|name)\\s*=\\s*[^>]*>")
            .arg(tagName),
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = rxReverse.globalMatch(html);
    while (it.hasNext()) {
        const QString tag = it.next().captured(0);
        const QRegularExpression attrRx(
            QStringLiteral("(?:id|name)\\s*=\\s*['\"]%1['\"]")
                .arg(escaped),
            QRegularExpression::CaseInsensitiveOption);
        if (attrRx.match(tag).hasMatch()) return tag;
    }
    return {};
}

QString attributeValue(const QString &tag, const QString &attribute)
{
    const QRegularExpression rx(
        QStringLiteral("\\b%1\\s*=\\s*(['\"])(.*?)\\1")
            .arg(QRegularExpression::escape(attribute)),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = rx.match(tag);
    return match.hasMatch() ? htmlDecode(match.captured(2)) : QString();
}

QString inputValue(const QString &html, const QString &fieldName)
{
    const QString input = rawTagForField(html, QStringLiteral("input"), fieldName);
    if (!input.isEmpty()) return attributeValue(input, QStringLiteral("value"));

    const QString escaped = QRegularExpression::escape(fieldName);
    const QRegularExpression textareaRx(
        QStringLiteral("<textarea\\b[^>]*(?:id|name)\\s*=\\s*['\"]%1['\"][^>]*>(.*?)</textarea>")
            .arg(escaped),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch textarea = textareaRx.match(html);
    if (textarea.hasMatch()) return htmlDecode(textarea.captured(1).trimmed());
    return {};
}

QString selectedValue(const QString &html, const QString &fieldName)
{
    const QString escaped = QRegularExpression::escape(fieldName);
    const QRegularExpression selectRx(
        QStringLiteral("<select\\b[^>]*(?:id|name)\\s*=\\s*['\"]%1['\"][^>]*>(.*?)</select>")
            .arg(escaped),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch select = selectRx.match(html);
    if (!select.hasMatch()) return {};

    const QString body = select.captured(1);
    QRegularExpression optionRx(QStringLiteral("<option\\b([^>]*)>(.*?)</option>"),
                                QRegularExpression::CaseInsensitiveOption
                                | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = optionRx.globalMatch(body);
    QString firstValue;
    while (it.hasNext()) {
        const QRegularExpressionMatch option = it.next();
        const QString attrs = option.captured(1);
        const QString value = attributeValue(QStringLiteral("<option ") + attrs + QLatin1Char('>'),
                                             QStringLiteral("value"));
        const QString normalized = value.isEmpty() ? stripTags(option.captured(2)) : value;
        if (firstValue.isEmpty()) firstValue = normalized;
        if (attrs.contains(QRegularExpression(QStringLiteral("\\bselected\\b"),
                                              QRegularExpression::CaseInsensitiveOption))) {
            return normalized;
        }
    }
    return firstValue;
}

bool checkboxChecked(const QString &html, const QString &fieldName)
{
    const QRegularExpression inputRx(QStringLiteral("<input\\b[^>]*>"),
                                     QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression nameRx(
        QStringLiteral("(?:id|name)\\s*=\\s*['\"]%1['\"]")
            .arg(QRegularExpression::escape(fieldName)),
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = inputRx.globalMatch(html);
    while (it.hasNext()) {
        const QString input = it.next().captured(0);
        if (!nameRx.match(input).hasMatch()) continue;
        const QString type = attributeValue(input, QStringLiteral("type")).toLower();
        if (type != QStringLiteral("checkbox")) continue;
        return input.contains(QRegularExpression(QStringLiteral("\\bchecked\\b"),
                                                 QRegularExpression::CaseInsensitiveOption));
    }
    return false;
}

QString fieldValue(const QString &html, const QString &fieldName)
{
    const QString select = selectedValue(html, fieldName);
    if (!select.isEmpty()) return select;
    return inputValue(html, fieldName);
}

QString dlValue(const QString &html, const QString &label)
{
    const QString escaped = QRegularExpression::escape(label);
    const QRegularExpression rx(
        QStringLiteral("<dt[^>]*>\\s*%1\\s*</dt>\\s*<dd[^>]*>(.*?)</dd>").arg(escaped),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch match = rx.match(html);
    return match.hasMatch() ? stripTags(match.captured(1)) : QString();
}

QString firstWord(QString text)
{
    text = text.simplified();
    const int space = text.indexOf(QLatin1Char(' '));
    return space > 0 ? text.left(space) : text;
}

QString parenthesizedValue(const QString &text)
{
    const QRegularExpression rx(QStringLiteral("\\(([^)]+)\\)"));
    const QRegularExpressionMatch match = rx.match(text);
    return match.hasMatch() ? match.captured(1).simplified() : QString();
}

QString firmwareVariantFromInstalled(const QString &installed)
{
    const QString text = installed.simplified();
    const int dash = text.lastIndexOf(QLatin1Char('-'));
    if (dash < 0 || dash + 1 >= text.size()) return QString();
    return text.mid(dash + 1).split(QRegularExpression(QStringLiteral("\\s+"))).value(0).trimmed();
}

QString hiddenJsonBlock(const QString &html, const QString &elementId)
{
    const QRegularExpression rx(
        QStringLiteral("<script\\b[^>]*id\\s*=\\s*['\"]%1['\"][^>]*>(.*?)</script>")
            .arg(QRegularExpression::escape(elementId)),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch match = rx.match(html);
    return match.hasMatch() ? htmlDecode(match.captured(1).trimmed()) : QString();
}

QVariantMap parseMetricsText(const QString &text)
{
    QVariantMap metrics;
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (const QString &lineValue : lines) {
        const QString line = lineValue.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;
        const int separator = line.lastIndexOf(QRegularExpression(QStringLiteral("\\s+")));
        if (separator <= 0) continue;
        const QString key = line.left(separator).trimmed();
        const QString rawValue = line.mid(separator).trimmed();
        bool ok = false;
        const double number = rawValue.toDouble(&ok);
        metrics.insert(key, ok ? QVariant(number) : QVariant(rawValue));
    }
    return metrics;
}

QString boolString(const QVariant &value, bool defaultValue = false)
{
    if (!value.isValid()) return defaultValue ? QStringLiteral("true") : QStringLiteral("false");
    if (value.metaType().id() == QMetaType::Bool) {
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    const QString text = value.toString().trimmed().toLower();
    if (text == QStringLiteral("1") || text == QStringLiteral("true")
        || text == QStringLiteral("yes") || text == QStringLiteral("on")) {
        return QStringLiteral("true");
    }
    return QStringLiteral("false");
}

QString stringSetting(const QVariantMap &settings, const QString &key)
{
    return settings.value(key).toString().trimmed();
}

QVariantList ntpServersFromSettings(const QVariantMap &settings)
{
    const QVariant value = settings.value(QStringLiteral("servers"));
    if (value.canConvert<QVariantList>()) return value.toList();

    QVariantList servers;
    for (int i = 0; i < 4; ++i) {
        const QString server = settings.value(QStringLiteral("server_%1").arg(i)).toString();
        if (!server.isEmpty()) servers.append(server);
    }
    return servers;
}

QString contentDispositionFilename(QNetworkReply *reply)
{
    const QByteArray header = reply->rawHeader("Content-Disposition");
    const QRegularExpression rx(QStringLiteral("filename\\*?=(?:UTF-8''|['\"]?)([^'\";]+)"),
                                QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = rx.match(QString::fromUtf8(header));
    return match.hasMatch() ? QUrl::fromPercentEncoding(match.captured(1).toUtf8()) : QString();
}

} // namespace

OpenIpcFirmwareClient::OpenIpcFirmwareClient(QObject *parent)
    : QObject(parent)
{
}

OpenIpcFirmwareClient::~OpenIpcFirmwareClient()
{
#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
    stopLiveLogs();
    if (m_upgradeSocket) {
        m_upgradeSocket->close();
        m_upgradeSocket->deleteLater();
        m_upgradeSocket = nullptr;
    }
#endif
}

bool OpenIpcFirmwareClient::webSocketsAvailable() const
{
#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
    return true;
#else
    return false;
#endif
}

QString OpenIpcFirmwareClient::newRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QNetworkRequest OpenIpcFirmwareClient::makeRequest(const QString &host, int port,
                                                   const QString &path,
                                                   const QString &username,
                                                   const QString &password) const
{
    QUrl relative;
    const int queryIndex = path.indexOf(QLatin1Char('?'));
    if (queryIndex >= 0) {
        relative.setPath(path.left(queryIndex));
        relative.setQuery(path.mid(queryIndex + 1));
    } else {
        relative.setPath(path);
    }
    return makeRequest(host, port, relative, username, password);
}

QNetworkRequest OpenIpcFirmwareClient::makeRequest(const QString &host, int port,
                                                   const QUrl &relativeUrl,
                                                   const QString &username,
                                                   const QString &password) const
{
    const QString trimmedHost = host.trimmed();
    QUrl url(trimmedHost.contains(QStringLiteral("://"))
                 ? trimmedHost : QStringLiteral("http://") + trimmedHost);
    if (url.scheme().isEmpty()) url.setScheme(QStringLiteral("http"));
    if (port > 0) {
        url.setPort(port);
    } else if (url.port() < 0) {
        url.setPort(url.scheme() == QStringLiteral("https") ? 443 : 80);
    }
    url.setPath(relativeUrl.path());
    url.setQuery(relativeUrl.query());

    QNetworkRequest request(url);
    request.setTransferTimeout(kTransferTimeoutMs);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "OpenIPC-Dashboard/0.2");
    request.setRawHeader("Accept", "application/json, text/html, text/plain, */*");
    if (!username.isEmpty()) {
        request.setRawHeader("Authorization", "Basic "
            + (username + QLatin1Char(':') + password).toUtf8().toBase64());
    }
    return request;
}

QNetworkRequest OpenIpcFirmwareClient::makeWebSocketRequest(const QString &host, int port,
                                                            const QString &path,
                                                            const QString &username,
                                                            const QString &password) const
{
    QNetworkRequest request = makeRequest(host, port, path, username, password);
    QUrl url = request.url();
    url.setScheme(url.scheme() == QStringLiteral("https") ? QStringLiteral("wss") : QStringLiteral("ws"));
    request.setUrl(url);
    return request;
}

QVariantMap OpenIpcFirmwareClient::parseJsonObject(const QByteArray &json, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = parseError.error == QJsonParseError::NoError
                ? QStringLiteral("JSON document is not an object")
                : parseError.errorString();
        }
        return {};
    }
    if (error) error->clear();
    return document.object().toVariantMap();
}

QVariantList OpenIpcFirmwareClient::parseJsonArray(const QByteArray &json, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isArray()) {
        if (error) {
            *error = parseError.error == QJsonParseError::NoError
                ? QStringLiteral("JSON document is not an array")
                : parseError.errorString();
        }
        return {};
    }
    if (error) error->clear();
    return document.array().toVariantList();
}

QVariantMap OpenIpcFirmwareClient::parsePulseForTest(const QByteArray &json)
{
    return parseJsonObject(json);
}

QVariantMap OpenIpcFirmwareClient::metricsFromTextForTest(const QString &text)
{
    return parseMetricsText(text);
}

QVariantMap OpenIpcFirmwareClient::parseNetworkPageForTest(const QString &html)
{
    QVariantMap network;
    network.insert(QStringLiteral("hostname"), fieldValue(html, QStringLiteral("network_hostname")));
    network.insert(QStringLiteral("interface"), fieldValue(html, QStringLiteral("network_interface")));
    network.insert(QStringLiteral("dhcp"), checkboxChecked(html, QStringLiteral("network_dhcp")));
    network.insert(QStringLiteral("address"), fieldValue(html, QStringLiteral("network_address")));
    network.insert(QStringLiteral("netmask"), fieldValue(html, QStringLiteral("network_netmask")));
    network.insert(QStringLiteral("gateway"), fieldValue(html, QStringLiteral("network_gateway")));
    network.insert(QStringLiteral("nameserver"), fieldValue(html, QStringLiteral("network_nameserver")));
    network.insert(QStringLiteral("wlanSsid"), fieldValue(html, QStringLiteral("network_wlan_ssid")));
    network.insert(QStringLiteral("wlanPassword"), fieldValue(html, QStringLiteral("network_wlan_password")));
    network.insert(QStringLiteral("macAddress"), fieldValue(html, QStringLiteral("mac_address")));

    const QVariantMap current{
        {QStringLiteral("hostname"), dlValue(html, QStringLiteral("Hostname"))},
        {QStringLiteral("interface"), dlValue(html, QStringLiteral("Interface"))},
        {QStringLiteral("mode"), dlValue(html, QStringLiteral("Mode"))},
        {QStringLiteral("address"), dlValue(html, QStringLiteral("IP"))},
        {QStringLiteral("netmask"), dlValue(html, QStringLiteral("Netmask"))},
        {QStringLiteral("gateway"), dlValue(html, QStringLiteral("Gateway"))},
        {QStringLiteral("nameserver"), dlValue(html, QStringLiteral("DNS"))},
        {QStringLiteral("macAddress"), dlValue(html, QStringLiteral("MAC"))}
    };
    network.insert(QStringLiteral("current"), current);

    auto fillFromCurrent = [&network, &current](const QString &key) {
        if (network.value(key).toString().isEmpty()) {
            network.insert(key, current.value(key));
        }
    };
    fillFromCurrent(QStringLiteral("hostname"));
    fillFromCurrent(QStringLiteral("interface"));
    fillFromCurrent(QStringLiteral("address"));
    fillFromCurrent(QStringLiteral("netmask"));
    fillFromCurrent(QStringLiteral("gateway"));
    fillFromCurrent(QStringLiteral("nameserver"));
    fillFromCurrent(QStringLiteral("macAddress"));
    if (!network.contains(QStringLiteral("dhcp")) || !network.value(QStringLiteral("dhcp")).isValid()) {
        network.insert(QStringLiteral("dhcp"),
                       current.value(QStringLiteral("mode")).toString().compare(QStringLiteral("DHCP"),
                                                                                Qt::CaseInsensitive) == 0);
    }
    return network;
}

QVariantMap OpenIpcFirmwareClient::parseTimePageForTest(const QString &html)
{
    QVariantMap time;
    time.insert(QStringLiteral("zoneName"), fieldValue(html, QStringLiteral("tz_name")));
    time.insert(QStringLiteral("zoneData"), fieldValue(html, QStringLiteral("tz_data")));
    QVariantList servers;
    for (int i = 0; i < 4; ++i) {
        servers.append(fieldValue(html, QStringLiteral("server_%1").arg(i)));
    }
    time.insert(QStringLiteral("servers"), servers);
    time.insert(QStringLiteral("deviceTime"), dlValue(html, QStringLiteral("Device time")));
    time.insert(QStringLiteral("currentZoneName"), dlValue(html, QStringLiteral("Zone name")));
    time.insert(QStringLiteral("currentZoneData"), dlValue(html, QStringLiteral("POSIX string")));
    time.insert(QStringLiteral("ntpSummary"), dlValue(html, QStringLiteral("NTP servers")));
    if (time.value(QStringLiteral("zoneName")).toString().isEmpty()) {
        time.insert(QStringLiteral("zoneName"), time.value(QStringLiteral("currentZoneName")));
    }
    if (time.value(QStringLiteral("zoneData")).toString().isEmpty()) {
        time.insert(QStringLiteral("zoneData"), time.value(QStringLiteral("currentZoneData")));
    }
    return time;
}

QVariantMap OpenIpcFirmwareClient::parseUpdatePageForTest(const QString &html)
{
    const QString installed = dlValue(html, QStringLiteral("Installed"));
    const QString soc = dlValue(html, QStringLiteral("SoC"));
    const QString flash = dlValue(html, QStringLiteral("Flash"));
    const QString socFamily = parenthesizedValue(soc);
    QVariantMap info;
    info.insert(QStringLiteral("installed"), installed);
    info.insert(QStringLiteral("latest"), dlValue(html, QStringLiteral("Latest on GitHub")));
    info.insert(QStringLiteral("soc"), soc);
    info.insert(QStringLiteral("socName"), firstWord(soc));
    info.insert(QStringLiteral("socFamily"), socFamily.isEmpty() ? firstWord(soc) : socFamily);
    info.insert(QStringLiteral("flash"), flash);
    info.insert(QStringLiteral("flashType"), firstWord(flash).toLower());
    info.insert(QStringLiteral("variant"), firmwareVariantFromInstalled(installed));
    QString checksum = dlValue(html, QStringLiteral("SHA-256"));
    if (checksum.isEmpty()) checksum = dlValue(html, QStringLiteral("SHA256"));
    if (checksum.isEmpty()) checksum = dlValue(html, QStringLiteral("Checksum"));
    info.insert(QStringLiteral("checksum"), checksum);
    info.insert(QStringLiteral("sha256"), checksum);
    info.insert(QStringLiteral("signature"), dlValue(html, QStringLiteral("Signature")));
    info.insert(QStringLiteral("githubAvailable"),
                !html.contains(QRegularExpression(QStringLiteral("id=[\"']fw-install-github[\"'][^>]*disabled"),
                                                  QRegularExpression::CaseInsensitiveOption)));
    return info;
}

QVariantMap OpenIpcFirmwareClient::parseStatusPageForTest(const QString &html)
{
    QVariantMap status;
    QVariantMap device{
        {QStringLiteral("soc"), dlValue(html, QStringLiteral("SoC"))},
        {QStringLiteral("sensor"), dlValue(html, QStringLiteral("Sensor"))},
        {QStringLiteral("firmware"), dlValue(html, QStringLiteral("Firmware"))},
        {QStringLiteral("build"), dlValue(html, QStringLiteral("Build"))},
        {QStringLiteral("majestic"), dlValue(html, QStringLiteral("Majestic"))},
        {QStringLiteral("uboot"), dlValue(html, QStringLiteral("U-Boot"))}
    };
    QVariantMap network{
        {QStringLiteral("hostname"), dlValue(html, QStringLiteral("Host"))},
        {QStringLiteral("address"), dlValue(html, QStringLiteral("Address"))},
        {QStringLiteral("macAddress"), dlValue(html, QStringLiteral("MAC"))},
        {QStringLiteral("interface"), dlValue(html, QStringLiteral("Link"))},
        {QStringLiteral("gateway"), dlValue(html, QStringLiteral("Gateway"))}
    };
    QVariantMap storage{
        {QStringLiteral("flash"), dlValue(html, QStringLiteral("Flash"))},
        {QStringLiteral("overlayJson"), hiddenJsonBlock(html, QStringLiteral("overlay-data"))}
    };
    status.insert(QStringLiteral("device"), device);
    status.insert(QStringLiteral("network"), network);
    status.insert(QStringLiteral("storage"), storage);
    return status;
}

void OpenIpcFirmwareClient::getText(
    const QString &requestId, const QString &operation, const QString &host, int port,
    const QString &username, const QString &password, const QString &path,
    const std::function<void(const QByteArray &, QNetworkReply *)> &onSuccess)
{
    QNetworkReply *reply = m_networkManager.get(makeRequest(host, port, path, username, password));
    handleSimpleReply(reply, requestId, operation, onSuccess);
}

void OpenIpcFirmwareClient::postForm(
    const QString &requestId, const QString &operation, const QString &host, int port,
    const QString &username, const QString &password, const QString &path,
    const QVariantMap &fields,
    const std::function<void(const QByteArray &, QNetworkReply *)> &onSuccess)
{
    QUrlQuery form;
    for (auto it = fields.cbegin(); it != fields.cend(); ++it) {
        form.addQueryItem(it.key(), it.value().toString());
    }
    QNetworkRequest request = makeRequest(host, port, path, username, password);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Accept", "text/html, application/json, text/plain, */*");
    QNetworkReply *reply = m_networkManager.post(request, form.toString(QUrl::FullyEncoded).toUtf8());
    handleSimpleReply(reply, requestId, operation, onSuccess);
}

void OpenIpcFirmwareClient::handleSimpleReply(
    QNetworkReply *reply, const QString &requestId, const QString &operation,
    const std::function<void(const QByteArray &, QNetworkReply *)> &onSuccess)
{
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, requestId, operation, onSuccess]() {
        const QByteArray body = reply->readAll();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool okStatus = status == 0 || (status >= 200 && status < 400);
        if (reply->error() == QNetworkReply::NoError && okStatus) {
            if (onSuccess) onSuccess(body, reply);
            emit operationSucceeded(requestId, operation, QString::fromUtf8(limitedBody(body)).trimmed());
        } else {
            emitFailure(requestId, operation, reply, body);
        }
        reply->deleteLater();
    });
}

void OpenIpcFirmwareClient::emitFailure(const QString &requestId, const QString &operation,
                                        QNetworkReply *reply, const QByteArray &body)
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString message = reply->errorString();
    const QString response = stripTags(QString::fromUtf8(limitedBody(body))).trimmed();
    if (!response.isEmpty()) message += QStringLiteral(": ") + response;
    emit operationFailed(requestId, operation, message, status);
}

void OpenIpcFirmwareClient::emitFailureLater(const QString &requestId, const QString &operation,
                                             const QString &message, int httpStatus)
{
    QMetaObject::invokeMethod(this, [this, requestId, operation, message, httpStatus]() {
        emit operationFailed(requestId, operation, message, httpStatus);
    }, Qt::QueuedConnection);
}

QString OpenIpcFirmwareClient::loadStatus(const QString &host, int port,
                                          const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    struct PendingStatus {
        QVariantMap status;
        QStringList errors;
        int remaining = 4;
    };
    auto pending = QSharedPointer<PendingStatus>::create();
    auto finish = [this, requestId, pending]() {
        if (--pending->remaining > 0) return;
        if (!pending->status.isEmpty()) {
            emit statusLoaded(requestId, pending->status);
            emit operationSucceeded(requestId, QStringLiteral("firmware-status"),
                                    QStringLiteral("status loaded"));
        } else {
            emit operationFailed(requestId, QStringLiteral("firmware-status"),
                                 pending->errors.join(QStringLiteral("; ")), 0);
        }
    };

    auto attach = [this, &host, port, &username, &password, requestId, pending, finish](
                      const QString &path, const QString &operation,
                      const std::function<void(const QByteArray &)> &parse) {
        QNetworkReply *reply = m_networkManager.get(makeRequest(host, port, path, username, password));
        connect(reply, &QNetworkReply::finished, this, [reply, operation, parse, pending, finish]() {
            const QByteArray body = reply->readAll();
            if (reply->error() == QNetworkReply::NoError) {
                parse(body);
            } else {
                pending->errors.append(operation + QStringLiteral(": ") + reply->errorString());
            }
            reply->deleteLater();
            finish();
        });
    };

    attach(QStringLiteral("/cgi-bin/j/pulse.cgi"), QStringLiteral("pulse"),
           [pending](const QByteArray &body) {
        pending->status.insert(QStringLiteral("pulse"), OpenIpcFirmwareClient::parsePulseForTest(body));
    });
    attach(QStringLiteral("/metrics"), QStringLiteral("metrics"), [pending](const QByteArray &body) {
        const QString raw = QString::fromUtf8(body);
        pending->status.insert(QStringLiteral("metrics"), parseMetricsText(raw));
        pending->status.insert(QStringLiteral("metricsText"), raw);
    });
    attach(QStringLiteral("/api/v1/config.json"), QStringLiteral("config"), [pending](const QByteArray &body) {
        pending->status.insert(QStringLiteral("config"), OpenIpcFirmwareClient::parseJsonObject(body));
    });
    attach(QStringLiteral("/cgi-bin/status.cgi"), QStringLiteral("status-page"), [pending](const QByteArray &body) {
        pending->status.insert(QStringLiteral("statusPage"),
                               OpenIpcFirmwareClient::parseStatusPageForTest(QString::fromUtf8(body)));
    });
    return requestId;
}

QString OpenIpcFirmwareClient::loadNetwork(const QString &host, int port,
                                           const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    getText(requestId, QStringLiteral("network-load"), host, port, username, password,
            QStringLiteral("/cgi-bin/fw-network.cgi"),
            [this, requestId](const QByteArray &body, QNetworkReply *) {
        emit networkLoaded(requestId, parseNetworkPageForTest(QString::fromUtf8(body)));
    });
    return requestId;
}

QString OpenIpcFirmwareClient::saveNetwork(const QString &host, int port,
                                           const QString &username, const QString &password,
                                           const QVariantMap &settings)
{
    const QString requestId = newRequestId();
    const QString interfaceName = stringSetting(settings, QStringLiteral("interface"));
    const bool dhcp = boolString(settings.value(QStringLiteral("dhcp")), true) == QStringLiteral("true");
    if (interfaceName.isEmpty()) {
        emitFailureLater(requestId, QStringLiteral("network-save"),
                         QStringLiteral("Network interface cannot be empty"));
        return requestId;
    }
    if (!dhcp && (stringSetting(settings, QStringLiteral("address")).isEmpty()
                  || stringSetting(settings, QStringLiteral("netmask")).isEmpty())) {
        emitFailureLater(requestId, QStringLiteral("network-save"),
                         QStringLiteral("Static network mode requires IP address and netmask"));
        return requestId;
    }
    if (interfaceName == QStringLiteral("wlan0")
        && (stringSetting(settings, QStringLiteral("wlanSsid")).isEmpty()
            || stringSetting(settings, QStringLiteral("wlanPassword")).isEmpty())) {
        emitFailureLater(requestId, QStringLiteral("network-save"),
                         QStringLiteral("Wi-Fi mode requires SSID and password"));
        return requestId;
    }

    const QVariantMap form{
        {QStringLiteral("action"), QStringLiteral("update")},
        {QStringLiteral("network_address"), stringSetting(settings, QStringLiteral("address"))},
        {QStringLiteral("network_dhcp"), dhcp ? QStringLiteral("true") : QStringLiteral("false")},
        {QStringLiteral("network_gateway"), stringSetting(settings, QStringLiteral("gateway"))},
        {QStringLiteral("network_hostname"), stringSetting(settings, QStringLiteral("hostname"))},
        {QStringLiteral("network_nameserver"), stringSetting(settings, QStringLiteral("nameserver"))},
        {QStringLiteral("network_netmask"), stringSetting(settings, QStringLiteral("netmask"))},
        {QStringLiteral("network_interface"), interfaceName},
        {QStringLiteral("network_wlan_ssid"), stringSetting(settings, QStringLiteral("wlanSsid"))},
        {QStringLiteral("network_wlan_password"), stringSetting(settings, QStringLiteral("wlanPassword"))}
    };
    postForm(requestId, QStringLiteral("network-save"), host, port, username, password,
             QStringLiteral("/cgi-bin/fw-network.cgi"), form,
             [this, requestId, settings](const QByteArray &, QNetworkReply *) {
        emit networkSaved(requestId, settings);
    });
    return requestId;
}

QString OpenIpcFirmwareClient::resetNetwork(const QString &host, int port,
                                            const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    postForm(requestId, QStringLiteral("network-reset"), host, port, username, password,
             QStringLiteral("/cgi-bin/fw-network.cgi"),
             QVariantMap{{QStringLiteral("action"), QStringLiteral("reset")}},
             [this, requestId](const QByteArray &, QNetworkReply *) { emit networkReset(requestId); });
    return requestId;
}

QString OpenIpcFirmwareClient::changeMacAddress(const QString &host, int port,
                                                const QString &username, const QString &password,
                                                const QString &macAddress)
{
    const QString requestId = newRequestId();
    const QRegularExpression macRx(QStringLiteral("^([0-9a-fA-F]{2}[:-]){5}([0-9a-fA-F]{2})$"));
    if (!macRx.match(macAddress.trimmed()).hasMatch()) {
        emitFailureLater(requestId, QStringLiteral("network-mac"),
                         QStringLiteral("Invalid MAC address"));
        return requestId;
    }
    postForm(requestId, QStringLiteral("network-mac"), host, port, username, password,
             QStringLiteral("/cgi-bin/fw-network.cgi"),
             QVariantMap{{QStringLiteral("action"), QStringLiteral("changemac")},
                         {QStringLiteral("mac_address"), macAddress.trimmed()}},
             [this, requestId, macAddress](const QByteArray &, QNetworkReply *) {
        emit macAddressChanged(requestId, macAddress.trimmed());
    });
    return requestId;
}

QString OpenIpcFirmwareClient::scanWifi(const QString &host, int port,
                                        const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    getText(requestId, QStringLiteral("wifi-scan"), host, port, username, password,
            QStringLiteral("/cgi-bin/j/network.cgi?scan=1"),
            [this, requestId](const QByteArray &body, QNetworkReply *) {
        QString error;
        const QVariantMap payload = parseJsonObject(body, &error);
        emit wifiScanned(requestId, payload.value(QStringLiteral("networks")).toList(),
                         payload.value(QStringLiteral("error"), error).toString());
    });
    return requestId;
}

QString OpenIpcFirmwareClient::loadTime(const QString &host, int port,
                                        const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    getText(requestId, QStringLiteral("time-load"), host, port, username, password,
            QStringLiteral("/cgi-bin/fw-time.cgi"),
            [this, requestId](const QByteArray &body, QNetworkReply *) {
        emit timeLoaded(requestId, parseTimePageForTest(QString::fromUtf8(body)));
    });
    return requestId;
}

QString OpenIpcFirmwareClient::saveTime(const QString &host, int port,
                                        const QString &username, const QString &password,
                                        const QVariantMap &settings)
{
    const QString requestId = newRequestId();
    const QString zoneName = stringSetting(settings, QStringLiteral("zoneName"));
    const QString zoneData = stringSetting(settings, QStringLiteral("zoneData"));
    if (zoneName.isEmpty() || zoneData.isEmpty()) {
        emitFailureLater(requestId, QStringLiteral("time-save"),
                         QStringLiteral("Timezone name and POSIX string cannot be empty"));
        return requestId;
    }

    QVariantMap form{{QStringLiteral("action"), QStringLiteral("update")},
                     {QStringLiteral("tz_name"), zoneName},
                     {QStringLiteral("tz_data"), zoneData}};
    const QVariantList servers = ntpServersFromSettings(settings);
    for (int i = 0; i < 4; ++i) {
        form.insert(QStringLiteral("server_%1").arg(i),
                    i < servers.size() ? servers.at(i).toString().trimmed() : QString());
    }

    postForm(requestId, QStringLiteral("time-save"), host, port, username, password,
             QStringLiteral("/cgi-bin/fw-time.cgi"), form,
             [this, requestId, settings](const QByteArray &, QNetworkReply *) {
        emit timeSaved(requestId, settings);
    });
    return requestId;
}

QString OpenIpcFirmwareClient::syncTime(const QString &host, int port,
                                        const QString &username, const QString &password,
                                        bool setFromComputer)
{
    const QString requestId = newRequestId();
    QString path = QStringLiteral("/cgi-bin/j/time.cgi");
    if (setFromComputer) {
        path += QStringLiteral("?set=%1").arg(QDateTime::currentSecsSinceEpoch());
    }
    getText(requestId, QStringLiteral("time-sync"), host, port, username, password, path,
            [this, requestId](const QByteArray &body, QNetworkReply *) {
        emit timeSynced(requestId, parseJsonObject(body));
    });
    return requestId;
}

QString OpenIpcFirmwareClient::loadLogs(const QString &host, int port,
                                        const QString &username, const QString &password,
                                        const QString &source, int lines)
{
    const QString requestId = newRequestId();
    const QString normalized = source.trimmed().isEmpty()
        ? QStringLiteral("syslog") : source.trimmed().toLower();
    if (normalized == QStringLiteral("kernel") || normalized == QStringLiteral("dmesg")) {
        getText(requestId, QStringLiteral("logs-load"), host, port, username, password,
                QStringLiteral("/cgi-bin/j/dmesg.cgi"),
                [this, requestId, normalized](const QByteArray &body, QNetworkReply *) {
            emit logsLoaded(requestId, normalized, QString::fromUtf8(body).trimmed());
        });
        return requestId;
    }

    lines = std::clamp(lines, 20, 2000);
    QString command = QStringLiteral("logread -n %1").arg(lines);
    if (normalized == QStringLiteral("majestic")) {
        command += QStringLiteral(" | grep -i majestic");
    }
    QUrl relative;
    relative.setPath(QStringLiteral("/cgi-bin/j/run.cgi"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("web"), QString::fromUtf8(command.toUtf8().toBase64()));
    relative.setQuery(query);

    QNetworkReply *reply = m_networkManager.get(makeRequest(host, port, relative, username, password));
    handleSimpleReply(reply, requestId, QStringLiteral("logs-load"),
                      [this, requestId, normalized](const QByteArray &body, QNetworkReply *) {
        emit logsLoaded(requestId, normalized, stripTags(QString::fromUtf8(body)).trimmed());
    });
    return requestId;
}

QString OpenIpcFirmwareClient::setLogBufferSize(const QString &host, int port,
                                                const QString &username, const QString &password,
                                                int sizeKiB)
{
    const QString requestId = newRequestId();
    if (sizeKiB < 16 || sizeKiB > 4096) {
        emitFailureLater(requestId, QStringLiteral("logs-buffer"),
                         QStringLiteral("Log buffer must be between 16 and 4096 KiB"));
        return requestId;
    }
    postForm(requestId, QStringLiteral("logs-buffer"), host, port, username, password,
             QStringLiteral("/cgi-bin/info-logs.cgi"),
             QVariantMap{{QStringLiteral("syslog_size"), QString::number(sizeKiB)}},
             [this, requestId, sizeKiB](const QByteArray &, QNetworkReply *) {
        emit logBufferSizeChanged(requestId, sizeKiB);
    });
    return requestId;
}

QString OpenIpcFirmwareClient::saveFirmwareBackup(const QString &host, int port,
                                                  const QString &username, const QString &password,
                                                  const QString &destinationPath)
{
    const QString requestId = newRequestId();
    QString path = PathUtils::localPathFromUserInput(destinationPath);
    if (path.isEmpty()) {
        emitFailureLater(requestId, QStringLiteral("firmware-backup"),
                         QStringLiteral("Backup destination is empty"));
        return requestId;
    }

    QNetworkReply *reply = m_networkManager.get(makeRequest(
        host, port, QStringLiteral("/cgi-bin/ext-backuper.cgi?backup=create"),
        username, password));
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId, path]() mutable {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emitFailure(requestId, QStringLiteral("firmware-backup"), reply, body);
            reply->deleteLater();
            return;
        }
        QFileInfo targetInfo(path);
        if (targetInfo.isDir()) {
            QString filename = contentDispositionFilename(reply);
            if (filename.isEmpty()) {
                filename = QStringLiteral("openipc-backup-%1.tgz")
                    .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
            }
            path = targetInfo.absoluteFilePath() + QLatin1Char('/') + filename;
        }
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(body) != body.size() || !file.commit()) {
            emit operationFailed(requestId, QStringLiteral("firmware-backup"),
                                 file.errorString(), 0);
        } else {
            emit backupSaved(requestId, path);
            emit operationSucceeded(requestId, QStringLiteral("firmware-backup"), path);
        }
        reply->deleteLater();
    });
    return requestId;
}

QString OpenIpcFirmwareClient::reboot(const QString &host, int port,
                                      const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    getText(requestId, QStringLiteral("firmware-reboot"), host, port, username, password,
            QStringLiteral("/cgi-bin/fw-restart.cgi"),
            [this, requestId](const QByteArray &, QNetworkReply *) {
        emit rebootStarted(requestId);
    });
    return requestId;
}

QString OpenIpcFirmwareClient::loadUpdateInfo(const QString &host, int port,
                                              const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
    getText(requestId, QStringLiteral("update-info"), host, port, username, password,
            QStringLiteral("/cgi-bin/fw-update.cgi"),
            [this, requestId](const QByteArray &body, QNetworkReply *) {
        emit updateInfoLoaded(requestId, parseUpdatePageForTest(QString::fromUtf8(body)));
    });
    return requestId;
}

QString OpenIpcFirmwareClient::uploadFirmwareArchive(const QString &host, int port,
                                                     const QString &username, const QString &password,
                                                     const QString &archivePath)
{
    const QString requestId = newRequestId();
    QFile file(PathUtils::localPathFromUserInput(archivePath));
    const QFileInfo info(file.fileName());
    const QString suffix = info.fileName().toLower();
    if (!suffix.endsWith(QStringLiteral(".tgz")) && !suffix.endsWith(QStringLiteral(".gz"))) {
        emitFailureLater(requestId, QStringLiteral("firmware-upload"),
                         QStringLiteral("Firmware archive must be a .tgz or .gz file"));
        return requestId;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        emitFailureLater(requestId, QStringLiteral("firmware-upload"), file.errorString());
        return requestId;
    }
    if (file.size() <= 0 || file.size() > kMaximumFirmwareArchiveBytes) {
        emitFailureLater(requestId, QStringLiteral("firmware-upload"),
                         QStringLiteral("Firmware archive size is outside the safety limit"));
        return requestId;
    }

    QNetworkRequest request = makeRequest(host, port, QStringLiteral("/upload"), username, password);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/octet-stream"));
    request.setRawHeader("File-Location", "/tmp/firmware.tgz");
    QNetworkReply *reply = m_networkManager.post(request, file.readAll());
    handleSimpleReply(reply, requestId, QStringLiteral("firmware-upload"),
                      [this, requestId](const QByteArray &, QNetworkReply *) {
        emit firmwareUploaded(requestId, QStringLiteral("/tmp/firmware.tgz"));
    });
    return requestId;
}

QString OpenIpcFirmwareClient::startGithubUpdate(const QString &host, int port,
                                                 const QString &username, const QString &password,
                                                 bool kernel, bool rootfs, bool reset, bool force)
{
    return startFirmwareUpgrade(host, port, username, password,
                                QStringLiteral("github"), kernel, rootfs, reset, force);
}

QString OpenIpcFirmwareClient::startFirmwareUpgrade(const QString &host, int port,
                                                    const QString &username, const QString &password,
                                                    const QString &source,
                                                    bool kernel, bool rootfs, bool reset, bool force)
{
    const QString requestId = newRequestId();
    const QString trimmedSource = source.trimmed().isEmpty() ? QStringLiteral("github") : source.trimmed();
    if (!kernel && !rootfs) {
        emitFailureLater(requestId, QStringLiteral("firmware-update"),
                         QStringLiteral("Select kernel and/or rootfs"));
        return requestId;
    }
#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
    if (m_upgradeSocket) {
        m_upgradeSocket->close();
        m_upgradeSocket->deleteLater();
        m_upgradeSocket = nullptr;
    }

    m_upgradeSocket = new QWebSocket(QStringLiteral("OpenIPC-Dashboard firmware upgrade"),
                                     QWebSocketProtocol::VersionLatest, this);
    m_upgradeRequestId = requestId;
    m_upgradeSocketOpened = false;
    QWebSocket *socket = m_upgradeSocket;

    connect(socket, &QWebSocket::connected, this, [this, socket, requestId, trimmedSource,
                                                   kernel, rootfs, reset, force]() {
        if (socket != m_upgradeSocket || requestId != m_upgradeRequestId) return;
        m_upgradeSocketOpened = true;
        QJsonObject payload{
            {QStringLiteral("source"), trimmedSource},
            {QStringLiteral("kernel"), kernel},
            {QStringLiteral("rootfs"), rootfs},
            {QStringLiteral("reset"), reset},
            {QStringLiteral("force"), force}
        };
        socket->sendTextMessage(QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact)));
        emit updateStarted(requestId, trimmedSource);
        emit operationSucceeded(requestId, QStringLiteral("firmware-update-start"),
                                QStringLiteral("Firmware updater started"));
    });
    connect(socket, &QWebSocket::binaryMessageReceived, this,
            [this, requestId](const QByteArray &message) {
        if (requestId == m_upgradeRequestId) {
            emit firmwareUpgradeOutput(requestId, QString::fromUtf8(message));
        }
    });
    connect(socket, &QWebSocket::textMessageReceived, this,
            [this, requestId](const QString &message) {
        if (requestId == m_upgradeRequestId) emit firmwareUpgradeOutput(requestId, message);
    });
    connect(socket, &QWebSocket::disconnected, this, [this, socket, requestId]() {
        const bool opened = m_upgradeSocketOpened;
        if (socket == m_upgradeSocket) {
            m_upgradeSocket->deleteLater();
            m_upgradeSocket = nullptr;
            m_upgradeSocketOpened = false;
            m_upgradeRequestId.clear();
        }
        if (opened) {
            emit firmwareUpgradeRebooting(requestId);
            emit operationSucceeded(requestId, QStringLiteral("firmware-update"),
                                    QStringLiteral("Firmware flashing started; camera is rebooting"));
        }
    });
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onUpgradeSocketError()));

    socket->open(makeWebSocketRequest(host, port, QStringLiteral("/ws/upgrade"), username, password));
    return requestId;
#else
    Q_UNUSED(host)
    Q_UNUSED(port)
    Q_UNUSED(username)
    Q_UNUSED(password)
    Q_UNUSED(trimmedSource)
    Q_UNUSED(reset)
    Q_UNUSED(force)
    emitFailureLater(requestId, QStringLiteral("firmware-update"),
                     QStringLiteral("Native firmware flashing uses /ws/upgrade. Qt WebSockets is not bundled in this build yet; open the camera WebUI Update page for the final flash step."),
                     0);
    return requestId;
#endif
}

QString OpenIpcFirmwareClient::startLiveLogs(const QString &host, int port,
                                             const QString &username, const QString &password)
{
    const QString requestId = newRequestId();
#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
    if (m_liveLogsSocket) {
        m_liveLogsSocket->close();
        m_liveLogsSocket->deleteLater();
        m_liveLogsSocket = nullptr;
    }

    m_liveLogsSocket = new QWebSocket(QStringLiteral("OpenIPC-Dashboard live logs"),
                                      QWebSocketProtocol::VersionLatest, this);
    m_liveLogsRequestId = requestId;
    m_liveLogsSocketOpened = false;
    QWebSocket *socket = m_liveLogsSocket;

    connect(socket, &QWebSocket::connected, this, [this, socket, requestId]() {
        if (socket != m_liveLogsSocket || requestId != m_liveLogsRequestId) return;
        m_liveLogsSocketOpened = true;
        emit liveLogsStarted(requestId);
    });
    connect(socket, &QWebSocket::binaryMessageReceived, this,
            [this, requestId](const QByteArray &message) {
        if (requestId == m_liveLogsRequestId) emit liveLogChunk(requestId, QString::fromUtf8(message));
    });
    connect(socket, &QWebSocket::textMessageReceived, this,
            [this, requestId](const QString &message) {
        if (requestId == m_liveLogsRequestId) emit liveLogChunk(requestId, message);
    });
    connect(socket, &QWebSocket::disconnected, this, [this, socket, requestId]() {
        const bool opened = m_liveLogsSocketOpened;
        if (socket == m_liveLogsSocket) {
            m_liveLogsSocket->deleteLater();
            m_liveLogsSocket = nullptr;
            m_liveLogsSocketOpened = false;
            m_liveLogsRequestId.clear();
        }
        emit liveLogsStopped(requestId, opened ? QStringLiteral("closed") : QStringLiteral("not-opened"));
    });
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onLiveLogsSocketError()));

    socket->open(makeWebSocketRequest(host, port, QStringLiteral("/ws/logs"), username, password));
    return requestId;
#else
    Q_UNUSED(host)
    Q_UNUSED(port)
    Q_UNUSED(username)
    Q_UNUSED(password)
    emitFailureLater(requestId, QStringLiteral("logs-live"),
                     QStringLiteral("Qt WebSockets is not bundled in this build; falling back to HTTP log polling."),
                     0);
    return requestId;
#endif
}

void OpenIpcFirmwareClient::stopLiveLogs()
{
#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
    if (!m_liveLogsSocket) return;
    const QString requestId = m_liveLogsRequestId;
    QWebSocket *socket = m_liveLogsSocket;
    m_liveLogsSocket = nullptr;
    m_liveLogsSocketOpened = false;
    m_liveLogsRequestId.clear();
    socket->close();
    socket->deleteLater();
    if (!requestId.isEmpty()) emit liveLogsStopped(requestId, QStringLiteral("stopped"));
#endif
}

#if defined(OPENIPC_HAS_QT_WEBSOCKETS)
void OpenIpcFirmwareClient::onUpgradeSocketError()
{
    QWebSocket *socket = m_upgradeSocket;
    if (!socket || m_upgradeSocketOpened) {
        return;
    }

    const QString requestId = m_upgradeRequestId;
    const QString message = socket->errorString().isEmpty()
        ? QStringLiteral("Could not start the upgrade WebSocket")
        : socket->errorString();
    emit operationFailed(requestId, QStringLiteral("firmware-update"), message, 0);
    m_upgradeRequestId.clear();
}

void OpenIpcFirmwareClient::onLiveLogsSocketError()
{
    QWebSocket *socket = m_liveLogsSocket;
    if (!socket) {
        return;
    }

    const QString requestId = m_liveLogsRequestId;
    const QString message = socket->errorString().isEmpty()
        ? QStringLiteral("Live logs WebSocket error")
        : socket->errorString();
    emit operationFailed(requestId, QStringLiteral("logs-live"), message, 0);
    m_liveLogsRequestId.clear();
}
#endif
