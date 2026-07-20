#include "DashboardWebServer.h"

#include "CameraHealthController.h"
#include "CameraModel.h"
#include "LogModel.h"
#include "DashboardWebPreviewManager.h"
#include "DashboardWebRecordingManager.h"
#include "DashboardWebRtcManager.h"
#include "PathUtils.h"
#include "PtzController.h"
#include "RecordingFileCatalog.h"
#include "SystemController.h"
#include "UserManager.h"
#include "presentation/DashboardPresentation.h"
#include "analytics/AnalyticsEngine.h"

#include <QCryptographicHash>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QSet>
#include <QSysInfo>
#include <QTcpSocket>
#include <QUrl>
#include <QUuid>

#include <functional>
#include <memory>
#include <algorithm>
#include <utility>

namespace {

constexpr int kArchiveMaxItems = 500;
constexpr qint64 kFileChunkBytes = 256 * 1024;

struct WebCameraInput
{
    QString name;
    QString ip;
    QString hdUrl;
    QString sdUrl;
    QString login;
    QString password;
    int rtspPort = 554;
    int onvifPort = 80;
};

QByteArray archiveId(const QString &canonicalPath)
{
    return QCryptographicHash::hash(canonicalPath.toUtf8(), QCryptographicHash::Sha256).toHex();
}

bool isOnlineStatus(const QString &status)
{
    const QString normalized = status.trimmed().toLower();
    return normalized == QStringLiteral("online") || normalized == QStringLiteral("онлайн");
}

QVariantMap parseJsonObject(const QByteArray &body, bool *ok)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(body, &error);
    *ok = error.error == QJsonParseError::NoError && document.isObject();
    return *ok ? document.object().toVariantMap() : QVariantMap();
}

QString normalizedCameraHost(QString host)
{
    host = host.trimmed();
    if (host.startsWith(QLatin1Char('[')) && host.endsWith(QLatin1Char(']'))) {
        host = host.mid(1, host.size() - 2);
    }
    return host;
}

bool isValidCameraHost(const QString &host)
{
    const QString normalized = normalizedCameraHost(host);
    if (normalized.isEmpty() || normalized.size() > 253
        || normalized.contains(QRegularExpression(QStringLiteral("[\\s/@?#]")))) {
        return false;
    }
    QHostAddress address;
    if (address.setAddress(normalized)) return true;
    static const QRegularExpression hostName(
        QStringLiteral("^[A-Za-z0-9](?:[A-Za-z0-9_.-]{0,251}[A-Za-z0-9])?$"));
    return hostName.match(normalized).hasMatch();
}

QString defaultRtspPath(const QString &profile, bool subStream)
{
    if (profile == QStringLiteral("onvif")) {
        return subStream ? QStringLiteral("/Streaming/Channels/102")
                         : QStringLiteral("/Streaming/Channels/101");
    }
    return subStream ? QStringLiteral("/stream=1") : QStringLiteral("/stream=0");
}

bool isValidRtspPath(const QString &path)
{
    return path.startsWith(QLatin1Char('/')) && path.size() <= 1024
        && !path.contains(QLatin1Char('\r')) && !path.contains(QLatin1Char('\n'));
}

QString buildRtspUrl(const QString &host, int port, const QString &path)
{
    const QString normalized = normalizedCameraHost(host);
    const QString authority = normalized.contains(QLatin1Char(':'))
        ? QStringLiteral("[%1]").arg(normalized) : normalized;
    return QStringLiteral("rtsp://%1:%2%3").arg(authority).arg(port).arg(path);
}

QString rewriteRtspAuthority(const QString &source, const QString &host, int port,
                             const QString &fallbackPath)
{
    QUrl url(source);
    if (url.isValid() && url.scheme().compare(QStringLiteral("rtsp"), Qt::CaseInsensitive) == 0
        && !url.host().isEmpty()) {
        url.setHost(normalizedCameraHost(host));
        url.setPort(port);
        return url.toString();
    }
    return buildRtspUrl(host, port, fallbackPath);
}

bool parseWebCameraInput(const QVariantMap &input, const Camera *current,
                         WebCameraInput *result, QString *error)
{
    if (!result || !error) return false;
    const bool editing = current != nullptr;
    const QString profile = input.value(QStringLiteral("profile"), QStringLiteral("openipc"))
                                .toString().trimmed().toLower();
    if (profile != QStringLiteral("openipc") && profile != QStringLiteral("onvif")
        && profile != QStringLiteral("generic")) {
        *error = QStringLiteral("Unsupported camera profile");
        return false;
    }

    result->ip = normalizedCameraHost(input.value(
        QStringLiteral("ip"), editing ? current->ip : QString()).toString());
    result->name = input.value(QStringLiteral("name"),
                               editing ? current->name : result->ip).toString().trimmed();
    if (result->name.isEmpty()) result->name = result->ip;
    result->rtspPort = input.value(QStringLiteral("rtspPort"),
                                   editing ? current->port : 554).toInt();
    result->onvifPort = input.value(QStringLiteral("onvifPort"),
                                    editing ? current->onvifPort : 80).toInt();

    if (!isValidCameraHost(result->ip)) {
        *error = QStringLiteral("Invalid camera host or IP address");
        return false;
    }
    if (result->name.isEmpty() || result->name.size() > 128) {
        *error = QStringLiteral("Camera name must contain 1 to 128 characters");
        return false;
    }
    if (result->rtspPort < 1 || result->rtspPort > 65535
        || result->onvifPort < 1 || result->onvifPort > 65535) {
        *error = QStringLiteral("Camera ports must be between 1 and 65535");
        return false;
    }

    const QString hdPath = input.value(QStringLiteral("hdPath")).toString().trimmed();
    const QString sdPath = input.value(QStringLiteral("sdPath")).toString().trimmed();
    if ((!hdPath.isEmpty() && !isValidRtspPath(hdPath))
        || (!sdPath.isEmpty() && !isValidRtspPath(sdPath))) {
        *error = QStringLiteral("RTSP paths must start with / and contain at most 1024 characters");
        return false;
    }

    if (editing && hdPath.isEmpty()) {
        const QString source = !current->hdStreamUrl.isEmpty() ? current->hdStreamUrl
                                                               : current->streamUrl;
        result->hdUrl = rewriteRtspAuthority(source, result->ip, result->rtspPort,
                                             defaultRtspPath(profile, false));
    } else {
        result->hdUrl = buildRtspUrl(result->ip, result->rtspPort,
                                     hdPath.isEmpty() ? defaultRtspPath(profile, false) : hdPath);
    }
    if (editing && sdPath.isEmpty()) {
        result->sdUrl = rewriteRtspAuthority(current->sdStreamUrl, result->ip, result->rtspPort,
                                             defaultRtspPath(profile, true));
    } else {
        result->sdUrl = buildRtspUrl(result->ip, result->rtspPort,
                                     sdPath.isEmpty() ? defaultRtspPath(profile, true) : sdPath);
    }

    result->login = editing ? current->login : QString();
    result->password = editing ? current->password : QString();
    if (input.contains(QStringLiteral("login"))) {
        result->login = input.value(QStringLiteral("login")).toString().trimmed();
    }
    if (input.contains(QStringLiteral("password"))) {
        result->password = input.value(QStringLiteral("password")).toString();
    }
    if (result->login.size() > 128 || result->password.size() > 512) {
        *error = QStringLiteral("Camera credentials are too long");
        return false;
    }
    return true;
}

QString contentTypeForResource(const QString &path)
{
    if (path.endsWith(QStringLiteral(".css"))) return QStringLiteral("text/css; charset=utf-8");
    if (path.endsWith(QStringLiteral(".js"))) return QStringLiteral("application/javascript; charset=utf-8");
    if (path.endsWith(QStringLiteral(".svg"))) return QStringLiteral("image/svg+xml");
    return QStringLiteral("text/html; charset=utf-8");
}

QString redactedLogMessage(QString message)
{
    static const QRegularExpression bearer(
        QStringLiteral("(?i)\\bBearer\\s+[A-Za-z0-9._~+/-]{8,}"));
    static const QRegularExpression assignment(
        QStringLiteral("(?i)\\b(password|passwd|token|secret|authorization)\\s*[:=]\\s*[^\\s,;]+"));
    static const QRegularExpression windowsUserPath(
        QStringLiteral("(?i)[A-Z]:[\\\\/]Users[\\\\/][^\\\\/\\s]+"));
    static const QRegularExpression unixUserPath(
        QStringLiteral("/home/[^/\\s]+"));
    message.replace(bearer, QStringLiteral("Bearer [REDACTED]"));
    message.replace(assignment, QStringLiteral("\\1=[REDACTED]"));
    message.replace(windowsUserPath, QStringLiteral("%USERPROFILE%"));
    message.replace(unixUserPath, QStringLiteral("$HOME"));
    QUrl url(message);
    if (url.isValid() && !url.userInfo().isEmpty()) {
        url.setUserInfo(QString());
        message = url.toString(QUrl::FullyEncoded);
    }
    return message.left(8192);
}

} // namespace

DashboardHttpProtocol::Response DashboardWebServer::jsonResponse(int status, const QVariant &data,
                                                                 const QString &error) const
{
    QJsonObject root;
    root.insert(QStringLiteral("ok"), status >= 200 && status < 300);
    if (!error.isEmpty()) {
        root.insert(QStringLiteral("error"), error);
    } else {
        root.insert(QStringLiteral("data"), QJsonValue::fromVariant(data));
    }
    DashboardHttpProtocol::Response response;
    response.status = status;
    response.reason = DashboardHttpProtocol::statusReason(status);
    response.body = QJsonDocument(root).toJson(QJsonDocument::Compact);
    addSecurityHeaders(&response, true);
    return response;
}

DashboardHttpProtocol::Response DashboardWebServer::emptyResponse(int status) const
{
    DashboardHttpProtocol::Response response;
    response.status = status;
    response.reason = DashboardHttpProtocol::statusReason(status);
    response.body.clear();
    addSecurityHeaders(&response, true);
    return response;
}

void DashboardWebServer::addSecurityHeaders(DashboardHttpProtocol::Response *response, bool api) const
{
    if (!response) return;
    response->headers.insert("X-Content-Type-Options", "nosniff");
    response->headers.insert("X-Frame-Options", "DENY");
    response->headers.insert("Referrer-Policy", "no-referrer");
    response->headers.insert("Permissions-Policy", "camera=(), microphone=(), geolocation=()");
    response->headers.insert("Cross-Origin-Resource-Policy", "same-origin");
    if (api) {
        response->headers.insert("Cache-Control", "no-store");
    } else {
        response->headers.insert("Content-Security-Policy",
                                 "default-src 'self'; img-src 'self' http: https: data:; "
                                 "media-src 'self' http: https: blob:; connect-src 'self' ws: wss:; "
                                 "style-src 'self'; script-src 'self'; frame-ancestors 'none'");
    }
}

DashboardHttpProtocol::Response DashboardWebServer::routeStatic(
    const DashboardHttpProtocol::Request &request) const
{
    if (request.method != "GET" && request.method != "HEAD") {
        return jsonResponse(405, {}, tr("Only GET and HEAD are allowed"));
    }
    QString resourcePath;
    if (request.path == QStringLiteral("/") || request.path == QStringLiteral("/index.html")) {
        resourcePath = QStringLiteral(":/web/index.html");
    } else if (request.path == QStringLiteral("/app.js")) {
        resourcePath = QStringLiteral(":/web/app.js");
    } else if (request.path == QStringLiteral("/styles.css")) {
        resourcePath = QStringLiteral(":/web/styles.css");
    } else {
        return jsonResponse(404, {}, tr("Resource not found"));
    }
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return jsonResponse(500, {}, tr("Web resource is unavailable"));
    }
    DashboardHttpProtocol::Response response;
    response.contentType = contentTypeForResource(resourcePath).toUtf8();
    response.body = file.readAll();
    response.headers.insert("Cache-Control", "no-cache, must-revalidate");
    addSecurityHeaders(&response, false);
    return response;
}

DashboardWebSessionStore::Session DashboardWebServer::authenticate(
    const DashboardHttpProtocol::Request &request, QByteArray *rawToken, bool touch)
{
    QByteArray token = DashboardHttpProtocol::bearerToken(request);
    if (token.isEmpty()) {
        token = DashboardHttpProtocol::parseCookies(request.header("cookie")).value("openipc_session");
    }
    if (rawToken) *rawToken = token;
    return m_sessions.find(token, touch);
}

bool DashboardWebServer::requirePermission(const DashboardWebSessionStore::Session &session,
                                           int permission,
                                           DashboardHttpProtocol::Response *response) const
{
    if (!session.isValid()) {
        if (response) *response = jsonResponse(401, {}, tr("Authentication required"));
        return false;
    }
    if ((session.permissions & UserManager::Perm_All) == UserManager::Perm_All
        || (session.permissions & permission) == permission) {
        return true;
    }
    if (response) *response = jsonResponse(403, {}, tr("Permission denied"));
    return false;
}

bool DashboardWebServer::validateMutationRequest(const DashboardHttpProtocol::Request &request,
                                                 DashboardHttpProtocol::Response *response) const
{
    const QByteArray origin = request.header("origin");
    if (!origin.isEmpty()
        && !DashboardHttpProtocol::originMatchesHost(origin, request.header("host"))) {
        if (response) *response = jsonResponse(403, {}, tr("Origin check failed"));
        return false;
    }
    const bool bearer = !DashboardHttpProtocol::bearerToken(request).isEmpty();
    const bool login = request.path == QStringLiteral("/api/v1/auth/login");
    if (!bearer && !login && request.header("x-openipc-csrf") != "1") {
        if (response) *response = jsonResponse(403, {}, tr("CSRF check failed"));
        return false;
    }
    return true;
}

bool DashboardWebServer::mutationAllowed(const QByteArray &rawToken, const QString &peerAddress,
                                         qint64 nowMs)
{
    const QByteArray key = QCryptographicHash::hash(
        rawToken + '\0' + peerAddress.toUtf8(), QCryptographicHash::Sha256);
    QList<qint64> &attempts = m_mutationWindows[key];
    while (!attempts.isEmpty() && attempts.first() < nowMs - 60 * 1000) {
        attempts.removeFirst();
    }
    if (attempts.size() >= 60) return false;
    attempts.append(nowMs);
    if (m_mutationWindows.size() > 512) {
        for (auto it = m_mutationWindows.begin(); it != m_mutationWindows.end();) {
            if (it.value().isEmpty() || it.value().last() < nowMs - 60 * 1000) {
                it = m_mutationWindows.erase(it);
            } else {
                ++it;
            }
        }
    }
    return true;
}

void DashboardWebServer::audit(const DashboardWebSessionStore::Session &session,
                               const QString &action, const QString &target,
                               const QString &outcome)
{
    if (!m_systemController) return;
    m_systemController->addLog(
        QtInfoMsg,
        QStringLiteral("AUDIT web user=%1 action=%2 target=%3 outcome=%4")
            .arg(redactedLogMessage(session.username).left(64),
                 redactedLogMessage(action).left(80),
                 redactedLogMessage(target).left(160),
                 redactedLogMessage(outcome).left(80)));
}

bool DashboardWebServer::loginAllowed(const QString &peerAddress, qint64 nowMs)
{
    LoginWindow &window = m_loginWindows[peerAddress];
    if (window.blockedUntilMs > nowMs) return false;
    while (!window.attempts.isEmpty() && window.attempts.first() < nowMs - 60 * 1000) {
        window.attempts.removeFirst();
    }
    return window.attempts.size() < 5;
}

void DashboardWebServer::recordLoginFailure(const QString &peerAddress, qint64 nowMs)
{
    LoginWindow &window = m_loginWindows[peerAddress];
    window.attempts.append(nowMs);
    if (window.attempts.size() >= 5) window.blockedUntilMs = nowMs + 5 * 60 * 1000;
}

DashboardHttpProtocol::Response DashboardWebServer::routeApi(
    const DashboardHttpProtocol::Request &request, const QString &peerAddress,
    bool *streamingResponse, QTcpSocket *socket)
{
    if (request.method == "OPTIONS") {
        DashboardHttpProtocol::Response response = emptyResponse(204);
        response.headers.insert("Allow", "GET, POST, HEAD, OPTIONS");
        return response;
    }
    if (request.method == "POST") {
        DashboardHttpProtocol::Response validation;
        if (!validateMutationRequest(request, &validation)) return validation;
    }

    if (request.path == QStringLiteral("/api/v1/server") && request.method == "GET") {
        const QVariantMap data{
            {QStringLiteral("name"), QStringLiteral("OpenIPC Dashboard")},
            {QStringLiteral("apiVersion"), QStringLiteral("v1")},
            {QStringLiteral("hasUsers"), m_systemController->userManager()->hasUsers()},
            {QStringLiteral("webSocketsAvailable"), webSocketsAvailable()},
            {QStringLiteral("webSocketPort"), webSocketPort()},
            {QStringLiteral("webRtcAvailable"), m_webRtcManager
                 && m_webRtcManager->available()},
            {QStringLiteral("webRtcError"), m_webRtcManager
                 ? m_webRtcManager->availabilityError() : QString()}
        };
        return jsonResponse(200, data);
    }

    if (request.path == QStringLiteral("/api/v1/auth/login") && request.method == "POST") {
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        if (!loginAllowed(peerAddress, nowMs)) {
            DashboardHttpProtocol::Response response = jsonResponse(429, {}, tr("Too many login attempts"));
            response.headers.insert("Retry-After", "300");
            return response;
        }
        if (!m_systemController->userManager()->hasUsers()) {
            return jsonResponse(503, {}, tr("Create the initial administrator in the desktop application first"));
        }
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        QVariantMap user;
        if (!jsonOk || !m_systemController->userManager()->authenticateForSession(
                           input.value(QStringLiteral("username")).toString(),
                           input.value(QStringLiteral("password")).toString(), &user)) {
            recordLoginFailure(peerAddress, nowMs);
            return jsonResponse(401, {}, tr("Invalid username or password"));
        }
        m_loginWindows.remove(peerAddress);
        const QByteArray token = m_sessions.create(user);
        const bool bearerSession = input.value(QStringLiteral("sessionMode")).toString()
            .compare(QStringLiteral("bearer"), Qt::CaseInsensitive) == 0;
        QVariantMap sessionData = user;
        sessionData.insert(QStringLiteral("capabilities"),
                           m_systemController->presentation()->capabilityManifest(
                               user.value(QStringLiteral("permissions")).toInt(),
                               m_webRtcManager && m_webRtcManager->available(),
                               m_webRtcManager && m_webRtcManager->audioAvailable()));
        if (bearerSession) sessionData.insert(QStringLiteral("token"), QString::fromLatin1(token));
        DashboardHttpProtocol::Response response = jsonResponse(200, sessionData);
        if (!bearerSession) {
            QByteArray cookie = "openipc_session=" + token
                + "; Path=/; HttpOnly; SameSite=Strict; Max-Age="
                + QByteArray::number(m_sessionTimeoutMinutes * 60);
            if (m_secureCookies) cookie += "; Secure";
            response.headers.insert("Set-Cookie", cookie);
        }
        return response;
    }

    QByteArray rawToken;
    const auto session = authenticate(request, &rawToken);
    if (request.method == "POST" && session.isValid()
        && !mutationAllowed(rawToken, peerAddress, QDateTime::currentMSecsSinceEpoch())) {
        DashboardHttpProtocol::Response response = jsonResponse(
            429, {}, tr("Too many changes. Try again in a minute"));
        response.headers.insert("Retry-After", "60");
        return response;
    }
    if (request.path == QStringLiteral("/api/v1/auth/session") && request.method == "GET") {
        if (!session.isValid()) return jsonResponse(401, {}, tr("Authentication required"));
        QVariantMap sessionData = session.toVariantMap();
        sessionData.insert(QStringLiteral("capabilities"),
                           m_systemController->presentation()->capabilityManifest(
                               session.permissions, m_webRtcManager && m_webRtcManager->available(),
                               m_webRtcManager && m_webRtcManager->audioAvailable()));
        return jsonResponse(200, sessionData);
    }
    if (request.path == QStringLiteral("/api/v1/auth/logout") && request.method == "POST") {
        m_sessions.remove(rawToken);
        DashboardHttpProtocol::Response response = jsonResponse(200, QVariantMap{});
        response.headers.insert("Set-Cookie",
                                "openipc_session=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0");
        return response;
    }

    DashboardHttpProtocol::Response denied;
    if (request.path == QStringLiteral("/api/v1/presentation") && request.method == "GET") {
        if (!session.isValid()) return jsonResponse(401, {}, tr("Authentication required"));
        return jsonResponse(200, QVariantMap{
            {QStringLiteral("contractVersion"), 1},
            {QStringLiteral("capabilities"),
             m_systemController->presentation()->capabilityManifest(
                 session.permissions, m_webRtcManager && m_webRtcManager->available(),
                 m_webRtcManager && m_webRtcManager->audioAvailable())},
            {QStringLiteral("permissions"),
             m_systemController->presentation()->permissionCatalog()},
            {QStringLiteral("designTokens"),
             m_systemController->presentation()->designTokens()}
        });
    }
    if (request.path == QStringLiteral("/api/v1/settings") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        return jsonResponse(200, QVariantMap{
            {QStringLiteral("values"), m_systemController->presentation()->settingsView(
                 m_systemController->getAppSettings())},
            {QStringLiteral("schema"), m_systemController->presentation()->settingsSchema()}
        });
    }
    if (request.path == QStringLiteral("/api/v1/settings") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QVariantMap patch = input.value(QStringLiteral("settings"), input).toMap();
        QVariantMap normalized;
        QString settingsError;
        if (!jsonOk || !m_systemController->presentation()->normalizeSettingsPatch(
                           patch, &normalized, &settingsError)) {
            return jsonResponse(400, {}, settingsError.isEmpty()
                ? tr("Invalid settings patch") : settingsError);
        }
        m_systemController->saveAppSettings(normalized);
        audit(session, QStringLiteral("settings.update"),
              normalized.keys().join(QLatin1Char(',')));
        return jsonResponse(200, QVariantMap{
            {QStringLiteral("values"), m_systemController->presentation()->settingsView(
                 m_systemController->getAppSettings())}
        });
    }
    if (request.path == QStringLiteral("/api/v1/users") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_UserManage, &denied)) return denied;
        return jsonResponse(200, QVariantMap{
            {QStringLiteral("users"), m_systemController->userManager()->users()},
            {QStringLiteral("permissions"), m_systemController->presentation()->permissionCatalog()},
            {QStringLiteral("sessions"), m_sessions.sessions(rawToken)}
        });
    }
    if (request.path == QStringLiteral("/api/v1/users/create") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_UserManage, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString username = input.value(QStringLiteral("username")).toString().trimmed();
        const QString password = input.value(QStringLiteral("password")).toString();
        const QString role = input.value(QStringLiteral("role"), QStringLiteral("operator"))
                                 .toString().trimmed().toLower();
        int permissions = input.value(QStringLiteral("permissions"),
                                      UserManager::Perm_LiveView | UserManager::Perm_Playback
                                          | UserManager::Perm_PTZ | UserManager::Perm_Analytics).toInt();
        static const QRegularExpression usernamePattern(QStringLiteral("^[A-Za-z0-9_.@-]{1,64}$"));
        if (!jsonOk || !usernamePattern.match(username).hasMatch()
            || password.size() < 8 || password.size() > 256
            || (role != QStringLiteral("admin") && role != QStringLiteral("operator"))
            || permissions < 0 || permissions > 0x7f) {
            return jsonResponse(400, {}, tr("Invalid user data"));
        }
        if (role == QStringLiteral("admin")) permissions = UserManager::Perm_All;
        if (!m_systemController->userManager()->addUser(username, password, role, permissions)) {
            return jsonResponse(409, {}, tr("The user already exists or could not be created"));
        }
        audit(session, QStringLiteral("user.create"), username);
        return jsonResponse(201, QVariantMap{{QStringLiteral("username"), username}});
    }
    if (request.path == QStringLiteral("/api/v1/users/permissions") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_UserManage, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString username = input.value(QStringLiteral("username")).toString().trimmed();
        const int permissions = input.value(QStringLiteral("permissions"), -1).toInt();
        const QVariantList users = m_systemController->userManager()->users();
        const auto found = std::find_if(users.cbegin(), users.cend(), [&username](const QVariant &value) {
            return value.toMap().value(QStringLiteral("username")).toString() == username;
        });
        if (!jsonOk || found == users.cend() || permissions < 0 || permissions > 0x7f
            || found->toMap().value(QStringLiteral("role")).toString() == QStringLiteral("admin")) {
            return jsonResponse(400, {}, tr("Invalid permission update"));
        }
        m_systemController->userManager()->updateUserPermissions(username, permissions);
        audit(session, QStringLiteral("user.permissions"), username);
        return jsonResponse(200, QVariantMap{{QStringLiteral("username"), username},
                                             {QStringLiteral("permissions"), permissions}});
    }
    if (request.path == QStringLiteral("/api/v1/users/delete") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_UserManage, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString username = input.value(QStringLiteral("username")).toString().trimmed();
        if (!jsonOk || username.isEmpty() || username == session.username) {
            return jsonResponse(400, {}, tr("Invalid user deletion request"));
        }
        if (!m_systemController->userManager()->deleteUser(username)) {
            return jsonResponse(409, {}, tr("The last administrator cannot be deleted"));
        }
        m_sessions.removeForUser(username);
        audit(session, QStringLiteral("user.delete"), username);
        return jsonResponse(200, QVariantMap{{QStringLiteral("username"), username}});
    }
    if (request.path == QStringLiteral("/api/v1/users/password") && request.method == "POST") {
        if (!session.isValid()) return jsonResponse(401, {}, tr("Authentication required"));
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString username = input.value(QStringLiteral("username"), session.username).toString().trimmed();
        const QString oldPassword = input.value(QStringLiteral("oldPassword")).toString();
        const QString newPassword = input.value(QStringLiteral("newPassword")).toString();
        if (!jsonOk || username != session.username || newPassword.size() < 8
            || newPassword.size() > 256 || oldPassword.isEmpty()
            || !m_systemController->userManager()->changePassword(
                username, oldPassword, newPassword)) {
            return jsonResponse(400, {}, tr("The current password is invalid or the new password is not accepted"));
        }
        m_sessions.removeForUser(username);
        audit(session, QStringLiteral("password.change"), username);
        return jsonResponse(200, QVariantMap{{QStringLiteral("username"), username}});
    }
    if (request.path == QStringLiteral("/api/v1/sessions/revoke") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_UserManage, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString sessionId = input.value(QStringLiteral("id")).toString().trimmed().toLower();
        if (!jsonOk || !m_sessions.removeById(sessionId)) {
            return jsonResponse(404, {}, tr("Session not found"));
        }
        audit(session, QStringLiteral("session.revoke"), sessionId);
        return jsonResponse(200, QVariantMap{{QStringLiteral("id"), sessionId}});
    }
    if (request.path == QStringLiteral("/api/v1/logs") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool cursorOk = false;
        bool limitOk = false;
        const int cursor = request.query.queryItemValue(QStringLiteral("cursor")).toInt(&cursorOk);
        const int requestedLimit = request.query.queryItemValue(QStringLiteral("limit")).toInt(&limitOk);
        const QVariantMap logs = logsData(cursorOk ? qMax(0, cursor) : 0,
                                          limitOk ? qBound(1, requestedLimit, 200) : 100,
                                          request.query.queryItemValue(QStringLiteral("level")),
                                          request.query.queryItemValue(QStringLiteral("search")));
        if (request.query.queryItemValue(QStringLiteral("download")) == QStringLiteral("1")) {
            QByteArray body;
            const QVariantList items = logs.value(QStringLiteral("items")).toList();
            for (const QVariant &value : items) {
                const QVariantMap item = value.toMap();
                body += QStringLiteral("%1 [%2] %3\n")
                            .arg(item.value(QStringLiteral("timestamp")).toString(),
                                 item.value(QStringLiteral("level")).toString(),
                                 item.value(QStringLiteral("message")).toString()).toUtf8();
            }
            DashboardHttpProtocol::Response response;
            response.contentType = "text/plain; charset=utf-8";
            response.body = body;
            response.headers.insert("Content-Disposition", "attachment; filename=\"openipc-dashboard.log\"");
            addSecurityHeaders(&response, true);
            return response;
        }
        return jsonResponse(200, logs);
    }
    if (request.path == QStringLiteral("/api/v1/logs/clear") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        m_systemController->logModel()->clear();
        audit(session, QStringLiteral("logs.clear"), QStringLiteral("dashboard"));
        return jsonResponse(200, QVariantMap{});
    }
    if (request.path == QStringLiteral("/api/v1/diagnostics") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        return jsonResponse(200, diagnosticsData());
    }
    if (request.path == QStringLiteral("/api/v1/diagnostics/bundle") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        DashboardHttpProtocol::Response response;
        response.contentType = "application/json; charset=utf-8";
        response.body = QJsonDocument::fromVariant(QVariantMap{
            {QStringLiteral("generatedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
            {QStringLiteral("diagnostics"), diagnosticsData()},
            {QStringLiteral("logs"), logsData(0, 500, QString(), QString())}
        }).toJson(QJsonDocument::Indented);
        response.headers.insert("Content-Disposition",
                                "attachment; filename=\"openipc-diagnostics.json\"");
        addSecurityHeaders(&response, true);
        return response;
    }
    if (request.path == QStringLiteral("/api/v1/configuration/export")
        && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        QVariantList cameras;
        CameraModel *model = m_systemController->cameraModel();
        for (int i = 0; i < model->rowCount(); ++i) {
            const Camera camera = model->getCamera(i);
            auto streamPath = [](const QString &value) {
                const QUrl url(value);
                if (!url.isValid()) return QString();
                QString path = url.path(QUrl::FullyDecoded);
                if (path.isEmpty()) path = QStringLiteral("/");
                if (url.hasQuery()) path += QLatin1Char('?') + url.query(QUrl::FullyDecoded);
                return path;
            };
            cameras.append(QVariantMap{
                {QStringLiteral("name"), camera.name},
                {QStringLiteral("ip"), camera.ip},
                {QStringLiteral("rtspPort"), camera.port},
                {QStringLiteral("onvifPort"), camera.onvifPort},
                {QStringLiteral("hdPath"), streamPath(
                     camera.hdStreamUrl.isEmpty() ? camera.streamUrl : camera.hdStreamUrl)},
                {QStringLiteral("sdPath"), streamPath(camera.sdStreamUrl)},
                {QStringLiteral("group"), camera.group},
                {QStringLiteral("profile"), camera.isOpenIpc
                     ? QStringLiteral("openipc") : QStringLiteral("generic")}
            });
        }
        const QVariantMap configuration{
            {QStringLiteral("kind"), QStringLiteral("openipc-dashboard-web-config")},
            {QStringLiteral("formatVersion"), 1},
            {QStringLiteral("generatedAt"),
             QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
            {QStringLiteral("settings"), m_systemController->presentation()->settingsView(
                 m_systemController->getAppSettings())},
            {QStringLiteral("cameras"), cameras},
            {QStringLiteral("layoutTemplates"), m_systemController->layoutTemplates()}
        };
        DashboardHttpProtocol::Response response;
        response.contentType = "application/json; charset=utf-8";
        response.body = QJsonDocument::fromVariant(configuration).toJson(QJsonDocument::Indented);
        response.headers.insert("Content-Disposition",
                                "attachment; filename=\"openipc-dashboard-config.json\"");
        addSecurityHeaders(&response, true);
        audit(session, QStringLiteral("configuration.export"), QStringLiteral("web"));
        return response;
    }
    if (request.path == QStringLiteral("/api/v1/configuration/import")
        && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        QVariantMap configuration = parseJsonObject(request.body, &jsonOk);
        if (configuration.contains(QStringLiteral("configuration"))) {
            configuration = configuration.value(QStringLiteral("configuration")).toMap();
        }
        const QVariantList cameraInputs = configuration.value(QStringLiteral("cameras")).toList();
        if (!jsonOk
            || configuration.value(QStringLiteral("kind")).toString()
                != QStringLiteral("openipc-dashboard-web-config")
            || configuration.value(QStringLiteral("formatVersion")).toInt() != 1
            || cameraInputs.size() > 256) {
            return jsonResponse(400, {}, tr("Unsupported or invalid configuration file"));
        }
        QVariantMap normalizedSettings;
        QString settingsError;
        const QVariantMap settings = configuration.value(QStringLiteral("settings")).toMap();
        if (!m_systemController->presentation()->normalizeSettingsPatch(
                settings, &normalizedSettings, &settingsError)) {
            return jsonResponse(400, {}, settingsError);
        }
        struct ImportedCamera {
            int existingIndex = -1;
            WebCameraInput value;
            QString group;
        };
        QList<ImportedCamera> imported;
        QSet<QString> importedHosts;
        CameraModel *model = m_systemController->cameraModel();
        for (const QVariant &entry : cameraInputs) {
            QVariantMap input = entry.toMap();
            input.remove(QStringLiteral("login"));
            input.remove(QStringLiteral("password"));
            const QString host = normalizedCameraHost(input.value(QStringLiteral("ip")).toString());
            if (importedHosts.contains(host)) {
                return jsonResponse(400, {}, tr("Configuration contains duplicate camera hosts"));
            }
            importedHosts.insert(host);
            ImportedCamera camera;
            camera.existingIndex = model->findIndexByIp(host);
            const Camera current = camera.existingIndex >= 0
                ? model->getCamera(camera.existingIndex) : Camera{};
            QString inputError;
            if (!parseWebCameraInput(input,
                    camera.existingIndex >= 0 ? &current : nullptr,
                    &camera.value, &inputError)) {
                return jsonResponse(400, {}, inputError);
            }
            camera.group = input.value(QStringLiteral("group")).toString().trimmed().left(128);
            imported.append(camera);
        }
        if (!normalizedSettings.isEmpty()) m_systemController->saveAppSettings(normalizedSettings);
        int added = 0;
        int updated = 0;
        for (const ImportedCamera &camera : std::as_const(imported)) {
            if (camera.existingIndex >= 0) {
                m_systemController->updateCamera(
                    camera.existingIndex, camera.value.name, camera.value.ip,
                    camera.value.hdUrl, camera.value.rtspPort, camera.value.onvifPort,
                    camera.value.login, camera.value.password, camera.value.sdUrl);
                Camera saved = model->getCamera(camera.existingIndex);
                saved.group = camera.group;
                model->setCamera(camera.existingIndex, saved);
                ++updated;
            } else {
                const int before = model->rowCount();
                m_systemController->addManualCamera(
                    camera.value.name, camera.value.ip, camera.value.hdUrl,
                    camera.value.rtspPort, camera.value.onvifPort,
                    QString(), QString(), camera.value.sdUrl);
                if (model->rowCount() == before + 1) {
                    Camera saved = model->getCamera(before);
                    saved.group = camera.group;
                    model->setCamera(before, saved);
                    ++added;
                }
            }
        }
        if (configuration.value(QStringLiteral("layoutTemplates")).metaType().id()
            == QMetaType::QVariantList) {
            m_systemController->setLayoutTemplates(
                configuration.value(QStringLiteral("layoutTemplates")).toList().mid(0, 50));
        }
        audit(session, QStringLiteral("configuration.import"), QStringLiteral("web"),
              QStringLiteral("added:%1 updated:%2").arg(added).arg(updated));
        return jsonResponse(200, QVariantMap{
            {QStringLiteral("added"), added},
            {QStringLiteral("updated"), updated},
            {QStringLiteral("credentialsPreserved"), true}
        });
    }
    if (request.path == QStringLiteral("/api/v1/devices/operation") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const int cameraIndex = input.value(QStringLiteral("cameraIndex"), -1).toInt();
        const QString operation = input.value(QStringLiteral("operation")).toString().trimmed().toLower();
        static const QSet<QString> allowed{
            QStringLiteral("status"), QStringLiteral("majestic"), QStringLiteral("metrics"),
            QStringLiteral("network"), QStringLiteral("time"), QStringLiteral("logs"),
            QStringLiteral("update-info"), QStringLiteral("sync-time"),
            QStringLiteral("reboot"), QStringLiteral("github-update")
        };
        const bool mutation = operation == QStringLiteral("sync-time")
            || operation == QStringLiteral("reboot")
            || operation == QStringLiteral("github-update");
        const QString confirmation = input.value(QStringLiteral("confirm")).toString();
        if (mutation && confirmation != operation) {
            return jsonResponse(409, QVariantMap{
                {QStringLiteral("confirmationRequired"), operation}
            }, tr("Explicit confirmation is required"));
        }
        const QString idempotencyKey = request.header("idempotency-key").trimmed();
        const QString idempotencyScope = QStringLiteral("%1|%2|%3|%4")
            .arg(session.username, idempotencyKey).arg(cameraIndex).arg(operation);
        if (mutation) {
            static const QRegularExpression idempotencyPattern(
                QStringLiteral("^[A-Za-z0-9._:-]{8,80}$"));
            if (!idempotencyPattern.match(idempotencyKey).hasMatch()) {
                return jsonResponse(400, {}, tr("A valid Idempotency-Key is required"));
            }
            const QString previousId = m_idempotentDeviceOperations.value(idempotencyScope);
            if (!previousId.isEmpty()) {
                const QVariantMap previous = deviceOperation(previousId);
                if (!previous.isEmpty()) return jsonResponse(200, previous);
            }
        }
        QString operationError;
        const QString requestId = jsonOk && allowed.contains(operation)
            ? beginDeviceOperation(cameraIndex, operation, &operationError) : QString();
        if (requestId.isEmpty()) {
            return jsonResponse(400, {}, operationError.isEmpty()
                ? tr("Invalid device operation") : operationError);
        }
        if (mutation) {
            m_idempotentDeviceOperations.insert(idempotencyScope, requestId);
            while (m_idempotentDeviceOperations.size() > 200) {
                m_idempotentDeviceOperations.erase(m_idempotentDeviceOperations.begin());
            }
            audit(session, QStringLiteral("device.%1").arg(operation),
                  QStringLiteral("camera:%1").arg(cameraIndex), QStringLiteral("accepted"));
        }
        return jsonResponse(202, deviceOperation(requestId));
    }
    const QRegularExpressionMatch deviceOperationMatch = QRegularExpression(
        QStringLiteral("^/api/v1/devices/operations/([A-Za-z0-9_-]{8,80})$")).match(request.path);
    if (deviceOperationMatch.hasMatch() && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        const QVariantMap operation = deviceOperation(deviceOperationMatch.captured(1));
        return operation.isEmpty() ? jsonResponse(404, {}, tr("Device operation not found"))
                                   : jsonResponse(200, operation);
    }
    if (request.path == QStringLiteral("/api/v1/devices/majestic/preview")
        && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString operationId = input.value(QStringLiteral("operationId")).toString();
        const QVariantMap edited = input.value(QStringLiteral("edited")).toMap();
        const QVariantMap source = m_deviceOperations.value(operationId);
        const QVariantMap privateData = m_devicePrivateData.value(operationId);
        if (!jsonOk || operationId.isEmpty() || edited.isEmpty()
            || source.value(QStringLiteral("operation")).toString() != QStringLiteral("majestic")
            || source.value(QStringLiteral("status")).toString() != QStringLiteral("succeeded")
            || privateData.isEmpty()) {
            return jsonResponse(400, {}, tr("Load Majestic configuration before editing it"));
        }
        const QVariantMap original = privateData.value(QStringLiteral("config")).toMap();
        const QVariantMap patch = m_systemController->majesticClient()->buildPatch(original, edited);
        if (patch.isEmpty()) return jsonResponse(409, {}, tr("There are no configuration changes"));
        if (scrubSensitive(patch).toMap() != patch) {
            return jsonResponse(400, {}, tr("Secret fields cannot be changed from this editor"));
        }
        const QVariantList changes = m_systemController->majesticClient()->describeChanges(
            original, edited);
        const QString previewId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_deviceChangePreviews.insert(previewId, QVariantMap{
            {QStringLiteral("cameraIndex"), source.value(QStringLiteral("cameraIndex"))},
            {QStringLiteral("patch"), patch},
            {QStringLiteral("createdAtMs"), QDateTime::currentMSecsSinceEpoch()}
        });
        while (m_deviceChangePreviews.size() > 20) {
            m_deviceChangePreviews.erase(m_deviceChangePreviews.begin());
        }
        return jsonResponse(200, QVariantMap{
            {QStringLiteral("previewId"), previewId},
            {QStringLiteral("changes"), scrubSensitive(changes)},
            {QStringLiteral("expiresInSeconds"), 600}
        });
    }
    if (request.path == QStringLiteral("/api/v1/devices/majestic/apply")
        && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString previewId = input.value(QStringLiteral("previewId")).toString();
        const QString confirmation = input.value(QStringLiteral("confirm")).toString();
        const QString idempotencyKey = request.header("idempotency-key").trimmed();
        static const QRegularExpression idempotencyPattern(
            QStringLiteral("^[A-Za-z0-9._:-]{8,80}$"));
        const QString scope = QStringLiteral("%1|majestic|%2")
            .arg(session.username, idempotencyKey);
        const QString previousId = m_idempotentDeviceOperations.value(scope);
        if (!previousId.isEmpty()) {
            const QVariantMap previous = deviceOperation(previousId);
            if (!previous.isEmpty()) return jsonResponse(200, previous);
        }
        const QVariantMap preview = m_deviceChangePreviews.value(previewId);
        if (!jsonOk || previewId.isEmpty() || confirmation != previewId
            || !idempotencyPattern.match(idempotencyKey).hasMatch() || preview.isEmpty()
            || preview.value(QStringLiteral("createdAtMs")).toLongLong()
                < QDateTime::currentMSecsSinceEpoch() - 10 * 60 * 1000) {
            return jsonResponse(409, {}, tr("The confirmed change preview is missing or expired"));
        }
        const int cameraIndex = preview.value(QStringLiteral("cameraIndex"), -1).toInt();
        CameraModel *model = m_systemController->cameraModel();
        if (cameraIndex < 0 || cameraIndex >= model->rowCount()) {
            return jsonResponse(404, {}, tr("Camera not found"));
        }
        const Camera camera = model->getCamera(cameraIndex);
        const QString requestId = m_systemController->majesticClient()->applyConfiguration(
            camera.ip, camera.onvifPort > 0 ? camera.onvifPort : 80,
            camera.login, camera.password, preview.value(QStringLiteral("patch")).toMap());
        if (requestId.isEmpty()) return jsonResponse(500, {}, tr("Could not apply configuration"));
        m_deviceOperations.insert(requestId, QVariantMap{
            {QStringLiteral("id"), requestId},
            {QStringLiteral("cameraIndex"), cameraIndex},
            {QStringLiteral("cameraId"), camera.id},
            {QStringLiteral("operation"), QStringLiteral("majestic-apply")},
            {QStringLiteral("status"), QStringLiteral("pending")},
            {QStringLiteral("startedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}
        });
        m_deviceChangePreviews.remove(previewId);
        m_idempotentDeviceOperations.insert(scope, requestId);
        audit(session, QStringLiteral("device.majestic.apply"),
              QStringLiteral("camera:%1").arg(cameraIndex), QStringLiteral("accepted"));
        return jsonResponse(202, deviceOperation(requestId));
    }
    if (request.path == QStringLiteral("/api/v1/camex/preview") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString host = input.value(QStringLiteral("serverHost")).toString().trimmed();
        const QString clientId = input.value(QStringLiteral("clientId")).toString().trimmed();
        const int port = input.value(QStringLiteral("port"), 5800).toInt();
        if (!jsonOk || host.isEmpty() || host.size() > 253 || clientId.size() > 64
            || port < 1 || port > 65535
            || host.contains(QRegularExpression(QStringLiteral("[\\s/\\\\;|&`$]")))) {
            return jsonResponse(400, {}, tr("Invalid Camex preview settings"));
        }
        const QVariantMap settings{
            {QStringLiteral("serverHost"), host},
            {QStringLiteral("port"), port},
            {QStringLiteral("clientId"), clientId},
            {QStringLiteral("transport"), QStringLiteral("udp")},
            {QStringLiteral("encrypt"), false}
        };
        return jsonResponse(200, QVariantMap{
            {QStringLiteral("serverCommand"),
             m_systemController->camexController()->buildServerCommand(settings)},
            {QStringLiteral("clientCommand"),
             m_systemController->camexController()->buildClientCommand(settings)},
            {QStringLiteral("serverConfig"),
             m_systemController->camexController()->buildServerConfig(settings)}
        });
    }
    if (request.path == QStringLiteral("/api/v1/discovery") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        return jsonResponse(200, discoveryData());
    }
    if (request.path == QStringLiteral("/api/v1/discovery/start") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        if (m_systemController->networkDiscovery()->running()) {
            return jsonResponse(409, {}, tr("Camera discovery is already running"));
        }
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        if (!jsonOk) return jsonResponse(400, {}, tr("Invalid JSON body"));
        const QString interfaceId = input.value(QStringLiteral("interface")).toString().trimmed();
        const bool deepScan = input.value(QStringLiteral("deepScan"), false).toBool();
        m_systemController->scanNetwork(interfaceId, deepScan);
        return jsonResponse(202, discoveryData());
    }
    if (request.path == QStringLiteral("/api/v1/discovery/stop") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        m_systemController->stopNetworkScan();
        return jsonResponse(200, discoveryData());
    }
    if (request.path == QStringLiteral("/api/v1/discovery/clear") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        if (m_systemController->networkDiscovery()->running()) {
            return jsonResponse(409, {}, tr("Stop camera discovery before clearing results"));
        }
        m_systemController->clearDiscoveryResults();
        return jsonResponse(200, discoveryData());
    }
    if (request.path == QStringLiteral("/api/v1/discovery/add") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        QVariantList indexes = input.value(QStringLiteral("indexes")).toList();
        if (indexes.isEmpty() && input.contains(QStringLiteral("index"))) {
            indexes.append(input.value(QStringLiteral("index")));
        }
        if (!jsonOk || indexes.isEmpty() || indexes.size() > 100) {
            return jsonResponse(400, {}, tr("Invalid discovery selection"));
        }
        const QString login = input.value(QStringLiteral("login")).toString().trimmed();
        const QString password = input.value(QStringLiteral("password")).toString();
        const QString profileOption = input.value(QStringLiteral("profile"), QStringLiteral("auto"))
                                          .toString().trimmed().toLower();
        if (login.size() > 128 || password.size() > 512
            || (profileOption != QStringLiteral("auto") && profileOption != QStringLiteral("openipc")
                && profileOption != QStringLiteral("onvif") && profileOption != QStringLiteral("generic"))) {
            return jsonResponse(400, {}, tr("Invalid onboarding options"));
        }
        const QString profile = profileOption == QStringLiteral("auto") ? QString()
            : (profileOption == QStringLiteral("generic") ? QStringLiteral("rtsp") : profileOption);
        const int added = m_systemController->addDiscoveredCameras(indexes, login, password, profile);
        return jsonResponse(added > 0 ? 201 : 409,
                            QVariantMap{{QStringLiteral("added"), added},
                                        {QStringLiteral("discovery"), discoveryData()}},
                            added > 0 ? QString() : tr("No cameras were added"));
    }
    if (request.path == QStringLiteral("/api/v1/dashboard") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_LiveView, &denied)) return denied;
        return jsonResponse(200, dashboardData());
    }
    if (request.path == QStringLiteral("/api/v1/cameras") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_LiveView, &denied)) return denied;
        return jsonResponse(200, cameraData());
    }
    if (request.path == QStringLiteral("/api/v1/cameras") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        WebCameraInput camera;
        QString inputError;
        if (!jsonOk || !parseWebCameraInput(input, nullptr, &camera, &inputError)) {
            return jsonResponse(400, {}, inputError.isEmpty() ? tr("Invalid camera data") : inputError);
        }
        if (m_systemController->cameraModel()->contains(camera.ip)) {
            return jsonResponse(409, {}, tr("A camera with this host already exists"));
        }
        const int previousCount = m_systemController->cameraModel()->rowCount();
        m_systemController->addManualCamera(camera.name, camera.ip, camera.hdUrl,
                                            camera.rtspPort, camera.onvifPort,
                                            camera.login, camera.password, camera.sdUrl);
        if (m_systemController->cameraModel()->rowCount() != previousCount + 1) {
            return jsonResponse(409, {}, tr("The camera could not be added"));
        }
        return jsonResponse(201, cameraData().constLast());
    }
    const QRegularExpressionMatch updateCameraMatch = QRegularExpression(
        QStringLiteral("^/api/v1/cameras/(\\d+)/update$")).match(request.path);
    if (updateCameraMatch.hasMatch() && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        const int cameraIndex = updateCameraMatch.captured(1).toInt();
        CameraModel *model = m_systemController->cameraModel();
        if (cameraIndex < 0 || cameraIndex >= model->rowCount()) {
            return jsonResponse(404, {}, tr("Camera not found"));
        }
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const Camera current = model->getCamera(cameraIndex);
        if (!jsonOk || (input.contains(QStringLiteral("id"))
                        && input.value(QStringLiteral("id")).toString() != current.id)) {
            return jsonResponse(409, {}, tr("The camera list changed; refresh and try again"));
        }
        WebCameraInput camera;
        QString inputError;
        if (!parseWebCameraInput(input, &current, &camera, &inputError)) {
            return jsonResponse(400, {}, inputError);
        }
        const int duplicateIndex = model->findIndexByIp(camera.ip);
        if (duplicateIndex >= 0 && duplicateIndex != cameraIndex) {
            return jsonResponse(409, {}, tr("A camera with this host already exists"));
        }
        m_systemController->updateCamera(cameraIndex, camera.name, camera.ip, camera.hdUrl,
                                         camera.rtspPort, camera.onvifPort,
                                         camera.login, camera.password, camera.sdUrl);
        return jsonResponse(200, cameraData().value(cameraIndex));
    }
    const QRegularExpressionMatch deleteCameraMatch = QRegularExpression(
        QStringLiteral("^/api/v1/cameras/(\\d+)/delete$")).match(request.path);
    if (deleteCameraMatch.hasMatch() && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        const int cameraIndex = deleteCameraMatch.captured(1).toInt();
        CameraModel *model = m_systemController->cameraModel();
        if (cameraIndex < 0 || cameraIndex >= model->rowCount()) {
            return jsonResponse(404, {}, tr("Camera not found"));
        }
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const Camera camera = model->getCamera(cameraIndex);
        if (!jsonOk || (input.contains(QStringLiteral("id"))
                        && input.value(QStringLiteral("id")).toString() != camera.id)) {
            return jsonResponse(409, {}, tr("The camera list changed; refresh and try again"));
        }
        m_systemController->removeDevice(cameraIndex);
        return jsonResponse(200, QVariantMap{{QStringLiteral("id"), camera.id}});
    }
    const QRegularExpressionMatch previewMatch = QRegularExpression(
        QStringLiteral("^/api/v1/cameras/(\\d+)/preview\\.jpg$")).match(request.path);
    if (previewMatch.hasMatch() && (request.method == "GET" || request.method == "HEAD")) {
        if (!requirePermission(session, UserManager::Perm_LiveView, &denied)) return denied;
        const int cameraIndex = previewMatch.captured(1).toInt();
        const QString quality = request.query.queryItemValue(QStringLiteral("quality"));
        const auto result = m_previewManager->frame(cameraIndex, quality);
        if (result.status == DashboardWebPreviewManager::FrameStatus::Ready) {
            DashboardHttpProtocol::Response response;
            response.contentType = "image/jpeg";
            response.body = result.jpeg;
            response.headers.insert("Cache-Control", "no-store, max-age=0");
            addSecurityHeaders(&response, true);
            return response;
        }
        DashboardHttpProtocol::Response response;
        if (result.status == DashboardWebPreviewManager::FrameStatus::InvalidCamera) {
            response = jsonResponse(404, {}, tr("Camera not found"));
        } else if (result.status == DashboardWebPreviewManager::FrameStatus::MissingStream) {
            response = jsonResponse(409, {}, tr("Camera has no configured stream"));
        } else if (result.status == DashboardWebPreviewManager::FrameStatus::WarmingUp) {
            response = jsonResponse(503, {}, tr("Camera preview is warming up"));
            response.headers.insert("Retry-After", "1");
        } else {
            response = jsonResponse(502, {}, tr("Camera preview is unavailable"));
            response.headers.insert("Retry-After", "2");
        }
        return response;
    }
    const QRegularExpressionMatch previewStreamMatch = QRegularExpression(
        QStringLiteral("^/api/v1/cameras/(\\d+)/preview\\.mjpeg$")).match(request.path);
    if (previewStreamMatch.hasMatch() && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_LiveView, &denied)) return denied;
        const int cameraIndex = previewStreamMatch.captured(1).toInt();
        const QString quality = request.query.queryItemValue(QStringLiteral("quality"));
        DashboardHttpProtocol::Response streamError;
        if (sendPreviewStream(socket, cameraIndex, quality, &streamError)) {
            *streamingResponse = true;
            return {};
        }
        return streamError;
    }
    const QRegularExpressionMatch snapshotMatch = QRegularExpression(
        QStringLiteral("^/api/v1/cameras/(\\d+)/snapshot\\.jpg$")).match(request.path);
    if (snapshotMatch.hasMatch() && (request.method == "GET" || request.method == "HEAD")) {
        if (!requirePermission(session, UserManager::Perm_LiveView, &denied)) return denied;
        const int cameraIndex = snapshotMatch.captured(1).toInt();
        const auto result = m_previewManager->frame(cameraIndex, QStringLiteral("hd"));
        if (result.status != DashboardWebPreviewManager::FrameStatus::Ready
            || result.jpeg.isEmpty()) {
            DashboardHttpProtocol::Response response = jsonResponse(
                result.status == DashboardWebPreviewManager::FrameStatus::InvalidCamera ? 404 : 503,
                {}, result.status == DashboardWebPreviewManager::FrameStatus::InvalidCamera
                    ? tr("Camera not found") : tr("Snapshot is not ready; try again"));
            response.headers.insert("Retry-After", "1");
            return response;
        }
        DashboardHttpProtocol::Response response;
        response.contentType = "image/jpeg";
        response.body = request.method == "HEAD" ? QByteArray() : result.jpeg;
        response.headers.insert("Cache-Control", "no-store");
        response.headers.insert("Content-Disposition",
                                QByteArray("attachment; filename=\"openipc-snapshot-")
                                    + QByteArray::number(QDateTime::currentMSecsSinceEpoch())
                                    + ".jpg\"");
        addSecurityHeaders(&response, true);
        return response;
    }
    if (request.path == QStringLiteral("/api/v1/recordings/active")
        && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_LiveView, &denied)) return denied;
        return jsonResponse(200, m_recordingManager ? m_recordingManager->status() : QVariantList{});
    }
    if (request.path == QStringLiteral("/api/v1/recording") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_LiveView, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const int cameraIndex = input.value(QStringLiteral("cameraIndex"), -1).toInt();
        const QString action = input.value(QStringLiteral("action")).toString().trimmed().toLower();
        if (!jsonOk || !m_recordingManager || cameraIndex < 0
            || cameraIndex >= m_systemController->cameraModel()->rowCount()
            || (action != QStringLiteral("start") && action != QStringLiteral("stop")
                && action != QStringLiteral("toggle"))) {
            return jsonResponse(400, {}, tr("Invalid recording request"));
        }
        const bool shouldStart = action == QStringLiteral("start")
            || (action == QStringLiteral("toggle") && !m_recordingManager->isRecording(cameraIndex));
        QString operationError;
        const bool changed = shouldStart ? m_recordingManager->start(cameraIndex, &operationError)
                                         : m_recordingManager->stop(cameraIndex, &operationError);
        if (!changed) return jsonResponse(409, {}, operationError);
        return jsonResponse(shouldStart ? 202 : 200, QVariantMap{
            {QStringLiteral("cameraIndex"), cameraIndex},
            {QStringLiteral("recording"), shouldStart}
        });
    }
    if (request.path == QStringLiteral("/api/v1/health") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_LiveView, &denied)) return denied;
        return jsonResponse(200, healthData());
    }
    if (request.path == QStringLiteral("/api/v1/health/run") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_Settings, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const QString profile = jsonOk ? input.value(QStringLiteral("profile"), QStringLiteral("quick")).toString()
                                       : QStringLiteral("quick");
        if (!m_systemController->cameraHealthController()->runAll(profile)) {
            return jsonResponse(409, {}, tr("A health check is already running or no cameras are available"));
        }
        return jsonResponse(202, QVariantMap{{QStringLiteral("profile"), profile}});
    }
    if (request.path == QStringLiteral("/api/v1/analytics") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Analytics, &denied)) return denied;
        return jsonResponse(200, analyticsData());
    }
    if (request.path == QStringLiteral("/api/v1/archive") && request.method == "GET") {
        if (!requirePermission(session, UserManager::Perm_Playback, &denied)) return denied;
        const QString cameraId = request.query.queryItemValue(QStringLiteral("camera"));
        bool limitOk = false;
        const int requestedLimit = request.query.queryItemValue(QStringLiteral("limit")).toInt(&limitOk);
        const int limit = limitOk ? qBound(1, requestedLimit, kArchiveMaxItems) : 200;
        return jsonResponse(200, archiveItems(cameraId, limit));
    }
    if (request.path.startsWith(QStringLiteral("/api/v1/archive/file/"))
        && (request.method == "GET" || request.method == "HEAD")) {
        if (!requirePermission(session, UserManager::Perm_Playback, &denied)) return denied;
        if (request.query.queryItemValue(QStringLiteral("download")) == QStringLiteral("1")
            && !requirePermission(session, UserManager::Perm_Export, &denied)) return denied;
        const QString fileId = request.path.mid(QStringLiteral("/api/v1/archive/file/").size());
        DashboardHttpProtocol::Response fileError;
        if (sendArchiveFile(socket, request, fileId, &fileError)) {
            *streamingResponse = true;
            return {};
        }
        return fileError;
    }
    if (request.path == QStringLiteral("/api/v1/ptz") && request.method == "POST") {
        if (!requirePermission(session, UserManager::Perm_PTZ, &denied)) return denied;
        bool jsonOk = false;
        const QVariantMap input = parseJsonObject(request.body, &jsonOk);
        const int cameraIndex = input.value(QStringLiteral("cameraIndex"), -1).toInt();
        if (!jsonOk || cameraIndex < 0 || cameraIndex >= m_systemController->cameraModel()->rowCount()) {
            return jsonResponse(400, {}, tr("Invalid camera"));
        }
        const Camera camera = m_systemController->cameraModel()->getCamera(cameraIndex);
        const QString action = input.value(QStringLiteral("action")).toString();
        if (action == QStringLiteral("stop")) {
            m_systemController->ptzController()->stop(camera.ip, camera.onvifPort,
                                                      camera.login, camera.password);
        } else if (action == QStringLiteral("move")) {
            const float x = qBound(-1.0, input.value(QStringLiteral("x")).toDouble(), 1.0);
            const float y = qBound(-1.0, input.value(QStringLiteral("y")).toDouble(), 1.0);
            const float zoom = qBound(-1.0, input.value(QStringLiteral("zoom")).toDouble(), 1.0);
            m_systemController->ptzController()->move(camera.ip, camera.onvifPort,
                                                      camera.login, camera.password, x, y, zoom);
        } else {
            return jsonResponse(400, {}, tr("Unsupported PTZ action"));
        }
        return jsonResponse(202, QVariantMap{{QStringLiteral("action"), action}});
    }

    return jsonResponse(404, {}, tr("API endpoint not found"));
}

QVariantList DashboardWebServer::cameraData() const
{
    QVariantList cameras;
    const CameraModel *model = m_systemController->cameraModel();
    for (int index = 0; index < model->rowCount(); ++index) {
        const Camera camera = model->getCamera(index);
        const QVariantMap health = m_systemController->cameraHealthController()->resultForCamera(camera.ip);
        QVariantMap item = m_systemController->presentation()->cameraView(
            camera, index, scrubSensitive(health).toMap());
        cameras.append(item);
    }
    return cameras;
}

QVariantMap DashboardWebServer::discoveryData() const
{
    QVariantList cameras;
    const CameraModel *model = m_systemController->discoveryModel();
    for (int index = 0; index < model->rowCount(); ++index) {
        const Camera camera = model->getCamera(index);
        cameras.append(QVariantMap{
            {QStringLiteral("index"), index},
            {QStringLiteral("name"), camera.name},
            {QStringLiteral("ip"), camera.ip},
            {QStringLiteral("manufacturer"), camera.manufacturer},
            {QStringLiteral("methods"), camera.discoveryMethods},
            {QStringLiteral("evidence"), camera.discoveryEvidence},
            {QStringLiteral("confidence"), camera.discoveryConfidence},
            {QStringLiteral("openIpc"), camera.isOpenIpc},
            {QStringLiteral("rtspPort"), camera.port},
            {QStringLiteral("onvifPort"), camera.onvifPort},
            {QStringLiteral("profile"), camera.onboardingProfile},
            {QStringLiteral("validationStatus"), camera.validationStatus},
            {QStringLiteral("validationMessage"), camera.validationMessage},
            {QStringLiteral("alreadyAdded"), camera.alreadyAdded}
        });
    }

    NetworkDiscoveryService *discovery = m_systemController->networkDiscovery();
    return {
        {QStringLiteral("running"), discovery && discovery->running()},
        {QStringLiteral("progress"), discovery ? discovery->progress() : 0},
        {QStringLiteral("phase"), discovery ? discovery->phase() : QString()},
        {QStringLiteral("foundCount"), cameras.size()},
        {QStringLiteral("summary"), m_systemController->discoverySessionSummary()},
        {QStringLiteral("interfaces"), m_systemController->getNetworkInterfaces()},
        {QStringLiteral("cameras"), cameras}
    };
}

QVariantMap DashboardWebServer::dashboardData() const
{
    const QVariantList cameras = cameraData();
    int online = 0;
    int recording = 0;
    for (const QVariant &value : cameras) {
        const QVariantMap camera = value.toMap();
        if (isOnlineStatus(camera.value(QStringLiteral("status")).toString())) ++online;
        if (camera.value(QStringLiteral("recording")).toBool()) ++recording;
    }
    return {
        {QStringLiteral("generatedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
        {QStringLiteral("summary"), QVariantMap{
             {QStringLiteral("total"), cameras.size()},
             {QStringLiteral("online"), online},
             {QStringLiteral("offline"), cameras.size() - online},
             {QStringLiteral("recording"), recording}}},
        {QStringLiteral("cameras"), cameras},
        {QStringLiteral("health"), healthData()},
        {QStringLiteral("server"), status()}
    };
}

QVariantMap DashboardWebServer::healthData() const
{
    CameraHealthController *health = m_systemController->cameraHealthController();
    return {
        {QStringLiteral("running"), health->running()},
        {QStringLiteral("activeProfile"), health->activeProfile()},
        {QStringLiteral("completedProbes"), health->completedProbes()},
        {QStringLiteral("totalProbes"), health->totalProbes()},
        {QStringLiteral("latestRun"), scrubSensitive(health->latestRun())},
        {QStringLiteral("results"), scrubSensitive(health->currentResults())}
    };
}

QVariantMap DashboardWebServer::analyticsData() const
{
    AnalyticsEngine *analytics = m_systemController->analyticsEngine();
    return {
        {QStringLiteral("diagnostics"), scrubSensitive(analytics->analyticsDiagnostics())},
        {QStringLiteral("evidence"), scrubSensitive(analytics->analyticsEvidenceSummary())},
        {QStringLiteral("modules"), scrubSensitive(analytics->moduleInventory())},
        {QStringLiteral("events"), scrubSensitive(analytics->queryAnalyticsEvents(-1, {}, {}, 100))}
    };
}

QVariantMap DashboardWebServer::logsData(int cursor, int limit, const QString &level,
                                         const QString &search) const
{
    const LogModel *model = m_systemController->logModel();
    QVariantList items;
    const QString normalizedLevel = level.trimmed().toLower();
    const QString normalizedSearch = search.trimmed().left(128);
    int scanned = qMax(0, cursor);
    while (items.size() < qBound(1, limit, 500) && scanned < model->rowCount()) {
        const int row = model->rowCount() - 1 - scanned;
        ++scanned;
        const QModelIndex index = model->index(row, 0);
        const QString itemLevel = model->data(index, LogModel::LevelStringRole).toString();
        const QString message = redactedLogMessage(
            model->data(index, LogModel::MessageRole).toString());
        if (!normalizedLevel.isEmpty() && normalizedLevel != QStringLiteral("all")
            && itemLevel.toLower() != normalizedLevel) continue;
        if (!normalizedSearch.isEmpty()
            && !message.contains(normalizedSearch, Qt::CaseInsensitive)) continue;
        const QDateTime timestamp = model->data(index, LogModel::TimestampRole).toDateTime();
        items.append(QVariantMap{
            {QStringLiteral("id"), QStringLiteral("%1-%2").arg(timestamp.toMSecsSinceEpoch()).arg(row)},
            {QStringLiteral("timestamp"), timestamp.toUTC().toString(Qt::ISODateWithMs)},
            {QStringLiteral("level"), itemLevel},
            {QStringLiteral("message"), message}
        });
    }
    return {
        {QStringLiteral("items"), items},
        {QStringLiteral("cursor"), cursor},
        {QStringLiteral("nextCursor"), scanned < model->rowCount() ? scanned : -1},
        {QStringLiteral("total"), model->rowCount()}
    };
}

QVariantMap DashboardWebServer::diagnosticsData()
{
    return {
        {QStringLiteral("application"), QVariantMap{
             {QStringLiteral("name"), QCoreApplication::applicationName()},
             {QStringLiteral("version"), QCoreApplication::applicationVersion()},
             {QStringLiteral("qtVersion"), QString::fromLatin1(qVersion())}}},
        {QStringLiteral("system"), QVariantMap{
             {QStringLiteral("product"), QSysInfo::prettyProductName()},
             {QStringLiteral("architecture"), QSysInfo::currentCpuArchitecture()},
             {QStringLiteral("kernel"), QSysInfo::kernelType() + QLatin1Char(' ') + QSysInfo::kernelVersion()}}},
        {QStringLiteral("process"), QVariantMap{
             {QStringLiteral("cpuPercent"), m_systemController->processCpuPercent()},
             {QStringLiteral("memoryMiB"), m_systemController->processMemoryMB()}}},
        {QStringLiteral("server"), status()},
        {QStringLiteral("cameras"), QVariantMap{
             {QStringLiteral("total"), m_systemController->cameraModel()->rowCount()},
             {QStringLiteral("online"), m_systemController->onlineCameraCount()},
             {QStringLiteral("attention"), m_systemController->camerasNeedingAttentionCount()}}},
        {QStringLiteral("logs"), QVariantMap{
             {QStringLiteral("count"), m_systemController->logModel()->rowCount()}}},
        {QStringLiteral("storage"), m_systemController->presentation()->settingsView(
             m_systemController->getAppSettings())},
        {QStringLiteral("health"), healthData()}
    };
}

QVariant DashboardWebServer::scrubSensitive(const QVariant &value) const
{
    if (value.metaType().id() == QMetaType::QVariantMap) {
        QVariantMap clean;
        const QVariantMap map = value.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            QString key = it.key().toLower();
            key.remove(QLatin1Char('_'));
            key.remove(QLatin1Char('-'));
            key.remove(QLatin1Char('.'));
            const bool sensitive = key == QStringLiteral("login")
                || key == QStringLiteral("username")
                || key == QStringLiteral("path")
                || key.contains(QStringLiteral("password"))
                || key.contains(QStringLiteral("secret"))
                || key.contains(QStringLiteral("token"))
                || key.contains(QStringLiteral("salt"))
                || key.contains(QStringLiteral("filepath"))
                || key.contains(QStringLiteral("modelpath"))
                || key.contains(QStringLiteral("snapshotspath"))
                || key.contains(QStringLiteral("clipspath"))
                || key.contains(QStringLiteral("evidencepath"))
                || key.contains(QStringLiteral("recordingspath"));
            if (!sensitive) clean.insert(it.key(), scrubSensitive(it.value()));
        }
        return clean;
    }
    if (value.metaType().id() == QMetaType::QVariantList) {
        QVariantList clean;
        for (const QVariant &item : value.toList()) clean.append(scrubSensitive(item));
        return clean;
    }
    if (value.metaType().id() == QMetaType::QString) {
        const QString text = value.toString();
        QUrl url(text);
        if (url.isValid() && !url.scheme().isEmpty() && !url.userInfo().isEmpty()) {
            url.setUserInfo(QString());
            return url.toString(QUrl::FullyEncoded);
        }
    }
    return value;
}

QVariantList DashboardWebServer::archiveItems(const QString &cameraId, int limit) const
{
    QString root = PathUtils::localPathFromUserInput(
        m_systemController->getAppSettings().value(QStringLiteral("recordingsPath")).toString());
    if (root.isEmpty()) root = RecordingFileCatalog::defaultRecordingRoot();
    QVariantList items;
    QDirIterator iterator(root, QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (iterator.hasNext() && items.size() < limit) {
        const QFileInfo fileInfo(iterator.next());
        const auto recording = RecordingFileCatalog::inspectFile(fileInfo, cameraId);
        if (!recording.has_value()) continue;
        const QString canonical = fileInfo.canonicalFilePath();
        if (canonical.isEmpty()) continue;
        QVariantMap item = scrubSensitive(RecordingFileCatalog::toVariantMap(*recording)).toMap();
        item.remove(QStringLiteral("filePath"));
        item.remove(QStringLiteral("fileUrl"));
        item.insert(QStringLiteral("id"), QString::fromLatin1(archiveId(canonical)));
        item.insert(QStringLiteral("streamUrl"),
                    QStringLiteral("/api/v1/archive/file/%1").arg(item.value(QStringLiteral("id")).toString()));
        items.append(item);
    }
    std::sort(items.begin(), items.end(), [](const QVariant &left, const QVariant &right) {
        return left.toMap().value(QStringLiteral("startTime")).toDateTime()
            > right.toMap().value(QStringLiteral("startTime")).toDateTime();
    });
    return items;
}

QString DashboardWebServer::archiveFileForId(const QString &fileId) const
{
    static const QRegularExpression validId(QStringLiteral("^[a-f0-9]{64}$"));
    if (!validId.match(fileId).hasMatch()) return {};
    QString root = PathUtils::localPathFromUserInput(
        m_systemController->getAppSettings().value(QStringLiteral("recordingsPath")).toString());
    if (root.isEmpty()) root = RecordingFileCatalog::defaultRecordingRoot();
    const QString canonicalRoot = QFileInfo(root).canonicalFilePath();
    if (canonicalRoot.isEmpty()) return {};
    QDirIterator iterator(canonicalRoot, QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QFileInfo fileInfo(iterator.next());
        if (!RecordingFileCatalog::isSupportedVideoFile(fileInfo)) continue;
        const QString canonical = fileInfo.canonicalFilePath();
        const QString prefix = canonicalRoot.endsWith(QDir::separator())
            ? canonicalRoot : canonicalRoot + QDir::separator();
        if (!canonical.startsWith(prefix, Qt::CaseInsensitive)) continue;
        if (QString::fromLatin1(archiveId(canonical)) == fileId) return canonical;
    }
    return {};
}

bool DashboardWebServer::sendArchiveFile(QTcpSocket *socket,
                                         const DashboardHttpProtocol::Request &request,
                                         const QString &fileId,
                                         DashboardHttpProtocol::Response *errorResponse)
{
    const QString path = archiveFileForId(fileId);
    QFileInfo info(path);
    if (path.isEmpty() || !info.exists() || !info.isFile()) {
        if (errorResponse) *errorResponse = jsonResponse(404, {}, tr("Archive file not found"));
        return false;
    }
    qint64 start = 0;
    qint64 end = info.size() - 1;
    bool partial = false;
    const QByteArray range = request.header("range").trimmed();
    if (!range.isEmpty()) {
        static const QRegularExpression rangePattern(QStringLiteral("^bytes=(\\d+)-(\\d*)$"));
        const auto match = rangePattern.match(QString::fromLatin1(range));
        if (!match.hasMatch()) {
            if (errorResponse) *errorResponse = jsonResponse(416, {}, tr("Invalid byte range"));
            return false;
        }
        start = match.captured(1).toLongLong();
        if (!match.captured(2).isEmpty()) end = match.captured(2).toLongLong();
        if (start < 0 || end < start || start >= info.size()) {
            if (errorResponse) *errorResponse = jsonResponse(416, {}, tr("Byte range is outside the file"));
            return false;
        }
        end = qMin(end, info.size() - 1);
        partial = true;
    }

    QMimeDatabase mimeDatabase;
    const QByteArray mime = mimeDatabase.mimeTypeForFile(info).name().toUtf8();
    const qint64 length = end - start + 1;
    QByteArray header = "HTTP/1.1 " + QByteArray(partial ? "206 Partial Content" : "200 OK") + "\r\n";
    header += "Connection: close\r\nContent-Type: " + mime + "\r\n";
    header += "Content-Length: " + QByteArray::number(length) + "\r\n";
    header += "Accept-Ranges: bytes\r\nCache-Control: private, no-store\r\n";
    if (request.query.queryItemValue(QStringLiteral("download")) == QStringLiteral("1")) {
        QByteArray safeName = info.fileName().toUtf8();
        safeName.replace('"', '_');
        safeName.replace('\r', '_');
        safeName.replace('\n', '_');
        header += "Content-Disposition: attachment; filename=\"" + safeName + "\"\r\n";
    }
    header += "X-Content-Type-Options: nosniff\r\nCross-Origin-Resource-Policy: same-origin\r\n";
    if (partial) {
        header += "Content-Range: bytes " + QByteArray::number(start) + '-'
            + QByteArray::number(end) + '/' + QByteArray::number(info.size()) + "\r\n";
    }
    header += "\r\n";
    socket->write(header);
    if (request.method == "HEAD") {
        socket->disconnectFromHost();
        return true;
    }

    auto file = QSharedPointer<QFile>::create(path);
    if (!file->open(QIODevice::ReadOnly) || !file->seek(start)) {
        socket->disconnectFromHost();
        return true;
    }
    auto remaining = QSharedPointer<qint64>::create(length);
    auto pump = std::make_shared<std::function<void()>>();
    *pump = [socket, file, remaining]() {
        if (!socket || socket->state() == QAbstractSocket::UnconnectedState) return;
        while (*remaining > 0 && socket->bytesToWrite() < 512 * 1024) {
            const QByteArray chunk = file->read(qMin(kFileChunkBytes, *remaining));
            if (chunk.isEmpty()) {
                *remaining = 0;
                break;
            }
            *remaining -= chunk.size();
            socket->write(chunk);
        }
        if (*remaining <= 0 && socket->bytesToWrite() == 0) socket->disconnectFromHost();
    };
    connect(socket, &QTcpSocket::bytesWritten, socket, [pump](qint64) { (*pump)(); });
    (*pump)();
    return true;
}
