#include "DashboardHttpProtocol.h"

#include <QUrl>

namespace DashboardHttpProtocol {

namespace {

bool validHeaderName(const QByteArray &name)
{
    if (name.isEmpty()) return false;
    static const QByteArray separators("()<>@,;:\\\"/[]?={} \t");
    for (const char character : name) {
        const uchar value = static_cast<uchar>(character);
        if (value <= 32 || value >= 127 || separators.contains(character)) return false;
    }
    return true;
}

QByteArray safeHeaderValue(QByteArray value, const QByteArray &fallback = {})
{
    if (value.contains('\r') || value.contains('\n') || value.contains('\0')) return fallback;
    return value;
}

} // namespace

QByteArray Request::header(const QByteArray &name) const
{
    return headers.value(name.toLower());
}

QByteArray statusReason(int status)
{
    switch (status) {
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 204: return "No Content";
    case 206: return "Partial Content";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 409: return "Conflict";
    case 413: return "Payload Too Large";
    case 415: return "Unsupported Media Type";
    case 416: return "Range Not Satisfiable";
    case 429: return "Too Many Requests";
    case 500: return "Internal Server Error";
    case 503: return "Service Unavailable";
    default: return "Error";
    }
}

QByteArray Response::serialize(bool includeBody) const
{
    const QByteArray safeReason = safeHeaderValue(
        reason.isEmpty() ? statusReason(status) : reason, statusReason(status));
    const QByteArray safeContentType = safeHeaderValue(contentType, "application/octet-stream");
    QByteArray data = "HTTP/1.1 " + QByteArray::number(status) + ' '
        + safeReason + "\r\n";
    data += "Connection: close\r\n";
    data += "Content-Type: " + safeContentType + "\r\n";
    data += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
    for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
        if (!validHeaderName(it.key()) || it.value().contains('\r')
            || it.value().contains('\n') || it.value().contains('\0')) continue;
        data += it.key() + ": " + it.value() + "\r\n";
    }
    data += "\r\n";
    if (includeBody) {
        data += body;
    }
    return data;
}

ParseResult parseRequest(const QByteArray &buffer, qsizetype maxHeaderBytes, qsizetype maxBodyBytes)
{
    ParseResult result;
    const qsizetype headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0) {
        if (buffer.size() > maxHeaderBytes) {
            result.complete = true;
            result.status = 413;
            result.error = QStringLiteral("Request headers are too large");
        }
        return result;
    }
    result.complete = true;
    if (headerEnd > maxHeaderBytes) {
        result.status = 413;
        result.error = QStringLiteral("Request headers are too large");
        return result;
    }

    const QList<QByteArray> lines = buffer.left(headerEnd).split('\n');
    if (lines.isEmpty()) {
        result.error = QStringLiteral("Missing request line");
        return result;
    }
    const QList<QByteArray> requestLine = lines.first().trimmed().split(' ');
    if (requestLine.size() != 3 || !requestLine.at(2).startsWith("HTTP/1.")) {
        result.error = QStringLiteral("Invalid request line");
        return result;
    }
    result.request.method = requestLine.at(0).toUpper();
    if (result.request.method != "GET" && result.request.method != "POST"
        && result.request.method != "HEAD" && result.request.method != "OPTIONS") {
        result.status = 405;
        result.error = QStringLiteral("Unsupported HTTP method");
        return result;
    }

    const QUrl target = QUrl::fromEncoded(requestLine.at(1), QUrl::StrictMode);
    if (!target.isValid() || !target.isRelative() || !target.path().startsWith('/')) {
        result.error = QStringLiteral("Invalid request target");
        return result;
    }
    result.request.path = QUrl::fromPercentEncoding(target.path(QUrl::FullyEncoded).toUtf8());
    result.request.query = QUrlQuery(target);

    for (qsizetype index = 1; index < lines.size(); ++index) {
        const QByteArray line = lines.at(index).trimmed();
        if (line.isEmpty()) continue;
        const qsizetype separator = line.indexOf(':');
        if (separator <= 0) {
            result.error = QStringLiteral("Invalid header");
            return result;
        }
        const QByteArray name = line.left(separator).trimmed().toLower();
        const QByteArray value = line.mid(separator + 1).trimmed();
        if (!validHeaderName(name)) {
            result.error = QStringLiteral("Invalid header name");
            return result;
        }
        if (result.request.headers.contains(name)
            && (name == "content-length" || name == "transfer-encoding" || name == "host")) {
            result.error = QStringLiteral("Duplicate framing header");
            return result;
        }
        result.request.headers.insert(name, value);
    }

    if (!result.request.header("transfer-encoding").isEmpty()) {
        result.status = 415;
        result.error = QStringLiteral("Transfer-Encoding request bodies are not supported");
        return result;
    }
    bool lengthOk = true;
    const qint64 contentLength = result.request.header("content-length").isEmpty()
        ? 0 : result.request.header("content-length").toLongLong(&lengthOk);
    if (!lengthOk || contentLength < 0) {
        result.error = QStringLiteral("Invalid Content-Length");
        return result;
    }
    if (contentLength > maxBodyBytes) {
        result.status = 413;
        result.error = QStringLiteral("Request body is too large");
        return result;
    }
    const qsizetype bodyOffset = headerEnd + 4;
    if (buffer.size() - bodyOffset < contentLength) {
        result.complete = false;
        return result;
    }
    result.request.body = buffer.mid(bodyOffset, contentLength);
    result.valid = true;
    result.status = 200;
    return result;
}

QHash<QByteArray, QByteArray> parseCookies(const QByteArray &header)
{
    QHash<QByteArray, QByteArray> cookies;
    for (const QByteArray &part : header.split(';')) {
        const qsizetype separator = part.indexOf('=');
        if (separator <= 0) continue;
        cookies.insert(part.left(separator).trimmed(), part.mid(separator + 1).trimmed());
    }
    return cookies;
}

QByteArray bearerToken(const Request &request)
{
    const QByteArray authorization = request.header("authorization").trimmed();
    const QByteArray prefix = QByteArrayLiteral("Bearer ");
    if (authorization.size() <= prefix.size()
        || authorization.left(prefix.size()).compare(prefix, Qt::CaseInsensitive) != 0) {
        return {};
    }
    return authorization.mid(prefix.size()).trimmed();
}

bool originMatchesHost(const QByteArray &originHeader, const QByteArray &hostHeader)
{
    const QByteArray encodedOrigin = originHeader.trimmed();
    const QByteArray encodedHost = hostHeader.trimmed();
    if (encodedOrigin.isEmpty() || encodedHost.isEmpty()) return false;

    const QUrl origin = QUrl::fromEncoded(encodedOrigin, QUrl::StrictMode);
    const QString scheme = origin.scheme().toLower();
    if (!origin.isValid() || (scheme != QStringLiteral("http")
                              && scheme != QStringLiteral("https"))
        || origin.host().isEmpty() || !origin.userInfo().isEmpty()
        || !origin.query().isEmpty() || !origin.fragment().isEmpty()
        || (!origin.path().isEmpty() && origin.path() != QStringLiteral("/"))) {
        return false;
    }

    const QUrl host = QUrl::fromEncoded(scheme.toUtf8() + "://" + encodedHost,
                                        QUrl::StrictMode);
    if (!host.isValid() || host.host().isEmpty() || !host.userInfo().isEmpty()
        || (!host.path().isEmpty() && host.path() != QStringLiteral("/"))) {
        return false;
    }

    const int defaultPort = scheme == QStringLiteral("https") ? 443 : 80;
    return origin.host().compare(host.host(), Qt::CaseInsensitive) == 0
        && origin.port(defaultPort) == host.port(defaultPort);
}

} // namespace DashboardHttpProtocol
