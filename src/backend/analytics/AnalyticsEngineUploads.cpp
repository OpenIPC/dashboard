#include "AnalyticsEngine.h"
#include "AnalyticsUploadRetryPolicy.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRandomGenerator>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QtConcurrent>
#include <keychain.h>
QString AnalyticsEngine::analyticsSecretKey(const QString &name) const
{
    return QStringLiteral("analytics/upload/%1").arg(name);
}

QString AnalyticsEngine::readSecretFromKeychain(const QString &name) const
{
    QKeychain::ReadPasswordJob job(QStringLiteral("OpenIPC"));
    job.setKey(analyticsSecretKey(name));

    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    return job.textData();
}

void AnalyticsEngine::writeSecretToKeychain(const QString &name, const QString &value) const
{
    QKeychain::WritePasswordJob job(QStringLiteral("OpenIPC"));
    job.setKey(analyticsSecretKey(name));
    job.setTextData(value);

    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
}

void AnalyticsEngine::deleteSecretFromKeychain(const QString &name) const
{
    QKeychain::DeletePasswordJob job(QStringLiteral("OpenIPC"));
    job.setKey(analyticsSecretKey(name));

    QEventLoop loop;
    connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
}

static QString base64Url(const QByteArray &data)
{
    QByteArray b = data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    return QString::fromLatin1(b);
}

static QString makeCodeVerifier()
{
    QByteArray bytes(32, 0);
    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] = (char)QRandomGenerator::global()->bounded(0, 256);
    }
    return base64Url(bytes);
}

void AnalyticsEngine::startOAuth(const QString &provider, const QString &clientId, const QString &clientSecret)
{
    if (clientId.trimmed().isEmpty()) {
        emit oauthError(provider, "client_id is empty");
        return;
    }

    cancelOAuth();

    m_oauthProvider = provider;
    m_oauthClientId = clientId;
    m_oauthClientSecret = clientSecret;
    m_oauthCodeVerifier = makeCodeVerifier();

    m_oauthServer = new QTcpServer(this);
    int preferredPort = (provider == "dropbox") ? 53682 : 0;
    if (preferredPort > 0) {
        if (!m_oauthServer->listen(QHostAddress::LocalHost, preferredPort)) {
            emit oauthError(provider, "Failed to start local callback server on port 53682");
            cancelOAuth();
            return;
        }
    } else {
        if (!m_oauthServer->listen(QHostAddress::LocalHost, 0)) {
            emit oauthError(provider, "Failed to start local callback server");
            cancelOAuth();
            return;
        }
    }

    m_oauthRedirectUri = QString("http://localhost:%1/").arg(m_oauthServer->serverPort());

    connect(m_oauthServer, &QTcpServer::newConnection, this, [this]() {
        QTcpSocket *socket = m_oauthServer->nextPendingConnection();
        if (!socket) return;
        socket->waitForReadyRead(3000);
        QByteArray request = socket->readAll();
        QString reqLine = QString::fromUtf8(request).split("\r\n").value(0);
        QString path = reqLine.section(' ', 1, 1);
        QUrl url("http://localhost" + path);
        QUrlQuery q(url);
        QString code = q.queryItemValue("code");
        QString error = q.queryItemValue("error");

        QByteArray body = "<html><body><h3>Authorization completed.</h3>You can close this window.</body></html>";
        QByteArray resp = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body;
        socket->write(resp);
        socket->flush();
        socket->disconnectFromHost();

        if (!error.isEmpty()) {
            emit oauthError(m_oauthProvider, error);
            cancelOAuth();
            return;
        }

        if (code.isEmpty()) {
            emit oauthError(m_oauthProvider, "No authorization code in callback");
            cancelOAuth();
            return;
        }

        // Exchange code for token
        QUrl tokenUrl;
        QUrlQuery bodyQuery;
        if (m_oauthProvider == "gdrive") {
            tokenUrl = QUrl("https://oauth2.googleapis.com/token");
            bodyQuery.addQueryItem("grant_type", "authorization_code");
            bodyQuery.addQueryItem("code", code);
            bodyQuery.addQueryItem("client_id", m_oauthClientId);
            bodyQuery.addQueryItem("redirect_uri", m_oauthRedirectUri);
            bodyQuery.addQueryItem("code_verifier", m_oauthCodeVerifier);
        } else if (m_oauthProvider == "onedrive") {
            tokenUrl = QUrl("https://login.microsoftonline.com/common/oauth2/v2.0/token");
            bodyQuery.addQueryItem("grant_type", "authorization_code");
            bodyQuery.addQueryItem("code", code);
            bodyQuery.addQueryItem("client_id", m_oauthClientId);
            bodyQuery.addQueryItem("redirect_uri", m_oauthRedirectUri);
            bodyQuery.addQueryItem("code_verifier", m_oauthCodeVerifier);
        } else if (m_oauthProvider == "dropbox") {
            tokenUrl = QUrl("https://api.dropbox.com/oauth2/token");
            bodyQuery.addQueryItem("grant_type", "authorization_code");
            bodyQuery.addQueryItem("code", code);
            bodyQuery.addQueryItem("client_id", m_oauthClientId);
            bodyQuery.addQueryItem("redirect_uri", m_oauthRedirectUri);
            bodyQuery.addQueryItem("code_verifier", m_oauthCodeVerifier);
        } else if (m_oauthProvider == "yadisk") {
            tokenUrl = QUrl("https://oauth.yandex.com/token");
            bodyQuery.addQueryItem("grant_type", "authorization_code");
            bodyQuery.addQueryItem("code", code);
            bodyQuery.addQueryItem("client_id", m_oauthClientId);
            bodyQuery.addQueryItem("client_secret", m_oauthClientSecret);
        } else {
            emit oauthError(m_oauthProvider, "Unsupported provider");
            cancelOAuth();
            return;
        }

        QNetworkRequest req(tokenUrl);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply *reply = m_networkManager->post(req, bodyQuery.query(QUrl::FullyEncoded).toUtf8());
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                emit oauthError(m_oauthProvider, reply->errorString());
                reply->deleteLater();
                cancelOAuth();
                return;
            }
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            QString accessToken = obj.value("access_token").toString();
            QString refreshToken = obj.value("refresh_token").toString();
            int expiresIn = obj.value("expires_in").toInt();
            if (accessToken.isEmpty()) {
                emit oauthError(m_oauthProvider, "No access_token in response");
            } else {
                emit oauthCompleted(m_oauthProvider, accessToken, refreshToken, expiresIn);
            }
            reply->deleteLater();
            cancelOAuth();
        });
    });

    QString scope;
    QUrl authUrl;
    QUrlQuery q;
    QString codeChallenge = base64Url(QCryptographicHash::hash(m_oauthCodeVerifier.toUtf8(), QCryptographicHash::Sha256));

    if (provider == "gdrive") {
        authUrl = QUrl("https://accounts.google.com/o/oauth2/v2/auth");
        scope = "https://www.googleapis.com/auth/drive.file";
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", m_oauthRedirectUri);
        q.addQueryItem("scope", scope);
        q.addQueryItem("access_type", "offline");
        q.addQueryItem("prompt", "consent");
        q.addQueryItem("code_challenge", codeChallenge);
        q.addQueryItem("code_challenge_method", "S256");
    } else if (provider == "onedrive") {
        authUrl = QUrl("https://login.microsoftonline.com/common/oauth2/v2.0/authorize");
        scope = "offline_access Files.ReadWrite";
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", m_oauthRedirectUri);
        q.addQueryItem("scope", scope);
        q.addQueryItem("response_mode", "query");
        q.addQueryItem("code_challenge", codeChallenge);
        q.addQueryItem("code_challenge_method", "S256");
    } else if (provider == "dropbox") {
        authUrl = QUrl("https://www.dropbox.com/oauth2/authorize");
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", m_oauthRedirectUri);
        q.addQueryItem("token_access_type", "offline");
        q.addQueryItem("code_challenge", codeChallenge);
        q.addQueryItem("code_challenge_method", "S256");
    } else if (provider == "yadisk") {
        authUrl = QUrl("https://oauth.yandex.com/authorize");
        q.addQueryItem("response_type", "code");
        q.addQueryItem("client_id", clientId);
        q.addQueryItem("redirect_uri", m_oauthRedirectUri);
        q.addQueryItem("force_confirm", "yes");
        q.addQueryItem("code_challenge", codeChallenge);
        q.addQueryItem("code_challenge_method", "S256");
    }

    if (!authUrl.isEmpty()) {
        authUrl.setQuery(q);
        emit oauthUrlReady(provider, authUrl.toString());
    } else {
        emit oauthError(provider, "Unsupported provider");
    }
}

void AnalyticsEngine::cancelOAuth()
{
    if (m_oauthServer) {
        m_oauthServer->close();
        m_oauthServer->deleteLater();
        m_oauthServer = nullptr;
    }
    m_oauthProvider.clear();
    m_oauthClientId.clear();
    m_oauthClientSecret.clear();
    m_oauthRedirectUri.clear();
    m_oauthCodeVerifier.clear();
}

void AnalyticsEngine::enqueueUpload(const QString &filePath)
{
    if (filePath.isEmpty()) return;
    if (!m_uploadEnabled || m_uploadTarget.isEmpty()) return;

    bool startQueue = false;
    {
        QMutexLocker locker(&m_uploadMutex);
        m_uploadQueue.enqueue({filePath,
                               m_uploadProvider,
                               m_uploadTarget,
                               1,
                               AnalyticsUploadRetryPolicy::defaultMaximumAttempts()});
        m_uploadState = QStringLiteral("queued");
        m_uploadLastUpdatedMs = QDateTime::currentMSecsSinceEpoch();
        if (!m_uploadActive) {
            m_uploadActive = true;
            startQueue = true;
        }
    }

    emit uploadStatusChanged();
    emit analyticsTelemetryChanged();
    if (startQueue) {
        QMetaObject::invokeMethod(this, [this]() { processNextUpload(); }, Qt::QueuedConnection);
    }
}

QVariantMap AnalyticsEngine::uploadStatus() const
{
    QMutexLocker locker(&m_uploadMutex);
    return {
        {QStringLiteral("enabled"), m_uploadEnabled},
        {QStringLiteral("active"), m_uploadActive},
        {QStringLiteral("state"), m_uploadState},
        {QStringLiteral("queueDepth"), m_uploadQueue.size() + (m_uploadActive ? 1 : 0)},
        {QStringLiteral("filePath"), m_uploadCurrentFile},
        {QStringLiteral("provider"), m_uploadCurrentProvider},
        {QStringLiteral("attempt"), m_uploadCurrentAttempt},
        {QStringLiteral("lastError"), m_uploadLastError},
        {QStringLiteral("lastUpdatedMs"), m_uploadLastUpdatedMs},
        {QStringLiteral("completed"), static_cast<qulonglong>(m_uploadCompletedCount)},
        {QStringLiteral("failed"), static_cast<qulonglong>(m_uploadFailedCount)},
        {QStringLiteral("retried"), static_cast<qulonglong>(m_uploadRetryCount)}
    };
}

void AnalyticsEngine::processNextUpload()
{
    UploadTask task;
    {
        QMutexLocker locker(&m_uploadMutex);
        if (m_uploadQueue.isEmpty()) {
            m_uploadActive = false;
            locker.unlock();
            emit uploadStatusChanged();
            emit analyticsTelemetryChanged();
            return;
        }
        task = m_uploadQueue.dequeue();
        m_uploadState = QStringLiteral("uploading");
        m_uploadCurrentFile = task.filePath;
        m_uploadCurrentProvider = task.provider;
        m_uploadCurrentAttempt = task.attempt;
        m_uploadLastError.clear();
        m_uploadLastUpdatedMs = QDateTime::currentMSecsSinceEpoch();
    }
    emit uploadStatusChanged();
    emit analyticsTelemetryChanged();

    (void)QtConcurrent::run([this, task]() {
        auto parseParams = [](const QString &s) {
            QMap<QString, QString> params;
            const QStringList parts = s.split(";", Qt::SkipEmptyParts);
            for (const QString &part : parts) {
                const int idx = part.indexOf('=');
                if (idx > 0) {
                    QString key = part.left(idx).trimmed().toLower();
                    QString val = part.mid(idx + 1).trimmed();
                    params.insert(key, val);
                }
            }
            return params;
        };

        auto ftpUpload = [](const QString &filePath, const QString &target) {
            QUrl url(target);
            if (!url.isValid() || url.scheme().isEmpty()) {
                return false;
            }

            QString path = url.path();
            if (path.isEmpty() || path.endsWith("/")) {
                if (!path.endsWith("/")) path += "/";
                path += QFileInfo(filePath).fileName();
                url.setPath(path);
            }

            QStringList args;
            args << "-s" << "-S" << "--fail"
                 << "--connect-timeout" << "15"
                 << "--max-time" << "60"
                 << "--ftp-create-dirs" << "-T" << filePath << url.toString();

            QProcess proc;
            proc.start("curl", args);
            if (!proc.waitForStarted(5000)) {
                return false;
            }
            if (!proc.waitForFinished(65000)) {
                proc.kill();
                proc.waitForFinished(2000);
                return false;
            }
            return proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0;
        };

        auto waitForReply = [](QNetworkReply *reply, QByteArray *payload) {
            QEventLoop loop;
            QTimer timeout;
            timeout.setSingleShot(true);
            bool timedOut = false;
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            QObject::connect(&timeout, &QTimer::timeout, &loop, [&]() {
                timedOut = true;
                reply->abort();
            });
            timeout.start(45000);
            loop.exec();
            timeout.stop();
            if (payload) {
                *payload = reply->readAll();
            }
            const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const bool ok = !timedOut && reply->error() == QNetworkReply::NoError
                && (statusCode == 0 || statusCode < 400);
            reply->deleteLater();
            return ok;
        };

        auto httpUpload = [&](QNetworkReply *reply) {
            return waitForReply(reply, nullptr);
        };

        auto httpFetch = [&](QNetworkReply *reply, QByteArray &out) {
            return waitForReply(reply, &out);
        };

        bool ok = false;
        if (task.provider == "local") {
            QDir dir(task.target);
            if (!dir.exists()) dir.mkpath(".");
            const QString source = QFileInfo(task.filePath).absoluteFilePath();
            const QString dest = QFileInfo(dir.filePath(QFileInfo(task.filePath).fileName())).absoluteFilePath();
            const QString partial = dest + QStringLiteral(".part");
            if (source == dest) {
                ok = true;
            } else {
                QFile::remove(partial);
                if (QFile::copy(source, partial)) {
                    QFile::remove(dest);
                    ok = QFile::rename(partial, dest);
                    if (!ok) {
                        QFile::remove(partial);
                    }
                }
            }
        } else if (task.provider == "ftp") {
            ok = ftpUpload(task.filePath, task.target);
        } else if (task.provider == "gdrive") {
            QMap<QString, QString> params = parseParams(task.target);
            QString token = params.value("token");
            if (token.isEmpty()) token = params.value("access_token");
            if (token.isEmpty()) token = m_uploadAccessToken;
            QString folderId = params.value("folder");
            if (!token.isEmpty()) {
                QNetworkAccessManager mgr;
                QNetworkRequest req(QUrl("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart"));
                req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

                QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);

                QHttpPart metaPart;
                metaPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
                QJsonObject meta;
                meta.insert("name", QFileInfo(task.filePath).fileName());
                if (!folderId.isEmpty()) {
                    QJsonArray parents;
                    parents.append(folderId);
                    meta.insert("parents", parents);
                }
                metaPart.setBody(QJsonDocument(meta).toJson(QJsonDocument::Compact));

                QHttpPart filePart;
                filePart.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
                QFile *file = new QFile(task.filePath);
                if (file->open(QIODevice::ReadOnly)) {
                    filePart.setBodyDevice(file);
                    file->setParent(multiPart);
                    multiPart->append(metaPart);
                    multiPart->append(filePart);

                    QNetworkReply *reply = mgr.post(req, multiPart);
                    multiPart->setParent(reply);
                    ok = httpUpload(reply);
                } else {
                    delete file;
                    delete multiPart;
                }
            }
        } else if (task.provider == "onedrive") {
            QMap<QString, QString> params = parseParams(task.target);
            QString token = params.value("token");
            if (token.isEmpty()) token = params.value("access_token");
            if (token.isEmpty()) token = m_uploadAccessToken;
            QString folderPath = params.value("path");
            if (folderPath.isEmpty()) folderPath = params.value("folder");
            if (!token.isEmpty()) {
                QString fileName = QFileInfo(task.filePath).fileName();
                QString path = folderPath;
                if (!path.isEmpty() && !path.startsWith("/")) path.prepend("/");
                if (!path.isEmpty() && path.endsWith("/")) path.chop(1);
                QString fullPath = path.isEmpty() ? ("/" + fileName) : (path + "/" + fileName);
                QUrl url("https://graph.microsoft.com/v1.0/me/drive/root:" + fullPath + ":/content");

                QNetworkAccessManager mgr;
                QNetworkRequest req(url);
                req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

                QFile *file = new QFile(task.filePath);
                if (file->open(QIODevice::ReadOnly)) {
                    QNetworkReply *reply = mgr.put(req, file);
                    file->setParent(reply);
                    ok = httpUpload(reply);
                } else {
                    delete file;
                }
            }
        } else if (task.provider == "dropbox") {
            QMap<QString, QString> params = parseParams(task.target);
            QString token = params.value("token");
            if (token.isEmpty()) token = m_uploadAccessToken;
            QString path = params.value("path");
            if (path.isEmpty()) path = "/OpenIPC";
            if (!path.startsWith("/")) path.prepend("/");
            QString fullPath = path;
            if (!fullPath.endsWith("/")) fullPath += "/";
            fullPath += QFileInfo(task.filePath).fileName();
            if (!token.isEmpty()) {
                QNetworkAccessManager mgr;
                QNetworkRequest req(QUrl("https://content.dropboxapi.com/2/files/upload"));
                req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
                QJsonObject arg;
                arg.insert("path", fullPath);
                arg.insert("mode", "add");
                arg.insert("autorename", true);
                arg.insert("mute", false);
                req.setRawHeader("Dropbox-API-Arg", QJsonDocument(arg).toJson(QJsonDocument::Compact));
                req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

                QFile *file = new QFile(task.filePath);
                if (file->open(QIODevice::ReadOnly)) {
                    QNetworkReply *reply = mgr.post(req, file);
                    file->setParent(reply);
                    ok = httpUpload(reply);
                } else {
                    delete file;
                }
            }
        } else if (task.provider == "yadisk") {
            QMap<QString, QString> params = parseParams(task.target);
            QString token = params.value("token");
            if (token.isEmpty()) token = m_uploadAccessToken;
            QString path = params.value("path");
            if (path.isEmpty()) path = "/OpenIPC";
            if (!path.startsWith("/")) path.prepend("/");
            QString fullPath = path;
            if (!fullPath.endsWith("/")) fullPath += "/";
            fullPath += QFileInfo(task.filePath).fileName();
            if (!token.isEmpty()) {
                QNetworkAccessManager mgr;
                QUrlQuery q;
                q.addQueryItem("path", fullPath);
                q.addQueryItem("overwrite", "true");
                QUrl reqUrl("https://cloud-api.yandex.net/v1/disk/resources/upload");
                reqUrl.setQuery(q);
                QNetworkRequest req(reqUrl);
                req.setRawHeader("Authorization", ("OAuth " + token).toUtf8());
                QNetworkReply *reply = mgr.get(req);
                QByteArray payload;
                if (httpFetch(reply, payload)) {
                    QJsonDocument doc = QJsonDocument::fromJson(payload);
                    QJsonObject obj = doc.object();
                    QUrl uploadUrl(obj.value("href").toString());
                    if (uploadUrl.isValid()) {
                        QNetworkRequest putReq(uploadUrl);
                        QFile *file = new QFile(task.filePath);
                        if (file->open(QIODevice::ReadOnly)) {
                            QNetworkReply *putReply = mgr.put(putReq, file);
                            file->setParent(putReply);
                            ok = httpUpload(putReply);
                        } else {
                            delete file;
                        }
                    }
                }
            }
        } else {
            qWarning() << "Upload provider not implemented:" << task.provider;
        }

        QMetaObject::invokeMethod(this, [this, task, ok]() mutable {
            if (ok) {
                {
                    QMutexLocker locker(&m_uploadMutex);
                    m_uploadCompletedCount += 1;
                    m_uploadState = QStringLiteral("success");
                    m_uploadLastError.clear();
                    m_uploadLastUpdatedMs = QDateTime::currentMSecsSinceEpoch();
                }
                qInfo() << "Analytics evidence upload completed"
                        << QFileInfo(task.filePath).fileName() << task.provider;
                emit uploadStatusChanged();
                emit analyticsTelemetryChanged();
                processNextUpload();
                return;
            }

            const QString errorText = QStringLiteral("Upload to %1 failed.").arg(task.provider);
            if (AnalyticsUploadRetryPolicy::shouldRetry(task.attempt, true, task.maximumAttempts)) {
                const int delayMs = AnalyticsUploadRetryPolicy::retryDelayMs(task.attempt);
                UploadTask retryTask = task;
                retryTask.attempt += 1;
                {
                    QMutexLocker locker(&m_uploadMutex);
                    m_uploadRetryCount += 1;
                    m_uploadState = QStringLiteral("retry_wait");
                    m_uploadLastError = errorText;
                    m_uploadLastUpdatedMs = QDateTime::currentMSecsSinceEpoch();
                }
                qWarning() << "Analytics evidence upload retry scheduled"
                           << QFileInfo(task.filePath).fileName()
                           << "attempt" << retryTask.attempt << "in" << delayMs << "ms";
                emit uploadStatusChanged();
                emit analyticsTelemetryChanged();

                QTimer::singleShot(delayMs, this, [this, retryTask]() {
                    bool startQueue = false;
                    {
                        QMutexLocker locker(&m_uploadMutex);
                        m_uploadQueue.enqueue(retryTask);
                        if (!m_uploadActive) {
                            m_uploadActive = true;
                            startQueue = true;
                        }
                    }
                    emit uploadStatusChanged();
                    emit analyticsTelemetryChanged();
                    if (startQueue) {
                        processNextUpload();
                    }
                });
                processNextUpload();
                return;
            }

            {
                QMutexLocker locker(&m_uploadMutex);
                m_uploadFailedCount += 1;
                m_uploadState = QStringLiteral("failed");
                m_uploadLastError = errorText;
                m_uploadLastUpdatedMs = QDateTime::currentMSecsSinceEpoch();
            }
            qWarning() << "Analytics evidence upload failed"
                       << QFileInfo(task.filePath).fileName() << task.provider;
            emit uploadStatusChanged();
            emit analyticsTelemetryChanged();
            processNextUpload();
        }, Qt::QueuedConnection);
    });
}
