#include "PtzController.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QUuid>
#include <QDebug>
#include <QRegularExpression>
#include <QNetworkProxy>

PtzController::PtzController(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    m_nam->setProxy(QNetworkProxy::NoProxy);
}

void PtzController::move(const QString &ip, int port, const QString &username, const QString &password, float x, float y, float zoom)
{
    qDebug() << "PtzController::move" << ip << port << x << y << zoom;
    ensureDiscovered(ip, port, username, password, [this, ip, port, username, password, x, y, zoom](bool success) {
        if (!success) {
            qWarning() << "PTZ Discovery failed, cannot move";
            return;
        }

        QString key = QString("%1:%2").arg(ip).arg(port);
        DeviceInfo &info = m_cache[key];

        QString body = QString(
            "<ContinuousMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
            "<ProfileToken>%1</ProfileToken>"
            "<Velocity>"
            "<PanTilt x=\"%2\" y=\"%3\" space=\"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace\" xmlns=\"http://www.onvif.org/ver10/schema\"/>"
            "<Zoom x=\"%4\" space=\"http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace\" xmlns=\"http://www.onvif.org/ver10/schema\"/>"
            "</Velocity>"
            "</ContinuousMove>"
        ).arg(info.profileToken)
         .arg(QLocale::c().toString(x, 'f', 1))
         .arg(QLocale::c().toString(y, 'f', 1))
         .arg(QLocale::c().toString(zoom, 'f', 1));

        sendSoap12(info.ptzUrl, username, password, "http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove", body, [this, info, username, password, body](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "SOAP 1.2 failed for ContinuousMove:" << reply->errorString();
                sendSoap11(info.ptzUrl, username, password, "http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove", body, [](QNetworkReply *r){ r->deleteLater(); });
            }
            reply->deleteLater();
        });
    });
}

void PtzController::stop(const QString &ip, int port, const QString &username, const QString &password)
{
    qDebug() << "PtzController::stop" << ip << port;
    ensureDiscovered(ip, port, username, password, [this, ip, port, username, password](bool success) {
        if (!success) {
            qWarning() << "PTZ Discovery failed, cannot stop";
            return;
        }

        QString key = QString("%1:%2").arg(ip).arg(port);
        DeviceInfo &info = m_cache[key];

        QString body = QString(
            "<Stop xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
            "<ProfileToken>%1</ProfileToken>"
            "<PanTilt>true</PanTilt>"
            "<Zoom>true</Zoom>"
            "</Stop>"
        ).arg(info.profileToken);

        sendSoap12(info.ptzUrl, username, password, "http://www.onvif.org/ver20/ptz/wsdl/Stop", body, [this, info, username, password, body](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "SOAP 1.2 failed for Stop:" << reply->errorString();
                sendSoap11(info.ptzUrl, username, password, "http://www.onvif.org/ver20/ptz/wsdl/Stop", body, [](QNetworkReply *r){ r->deleteLater(); });
            }
            reply->deleteLater();
        });
    });
}

void PtzController::focus(const QString &ip, int port, const QString &username, const QString &password, float speed)
{
    qDebug() << "PtzController::focus" << ip << port << speed;
    ensureDiscovered(ip, port, username, password, [this, ip, port, username, password, speed](bool success) {
        if (!success) {
            qWarning() << "PTZ Discovery failed, cannot focus";
            return;
        }

        QString key = QString("%1:%2").arg(ip).arg(port);
        DeviceInfo &info = m_cache[key];

        if (info.imagingUrl.isEmpty() || info.videoSourceToken.isEmpty()) {
            qWarning() << "Imaging URL or VideoSourceToken missing, cannot focus";
            return;
        }

        QString body = QString(
            "<Move xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\">"
            "<VideoSourceToken>%1</VideoSourceToken>"
            "<Focus>"
            "<Continuous xmlns=\"http://www.onvif.org/ver10/schema\">"
            "<Speed>%2</Speed>"
            "</Continuous>"
            "</Focus>"
            "</Move>"
        ).arg(info.videoSourceToken).arg(QLocale::c().toString(speed, 'f', 1));

        sendSoap12(info.imagingUrl, username, password, "http://www.onvif.org/ver20/imaging/wsdl/Move", body, [this, info, username, password, body](QNetworkReply *reply) {
            QByteArray response = reply->readAll();
            qDebug() << "Focus Move Response:" << response;
            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "SOAP 1.2 failed for Focus Move:" << reply->errorString();
                sendSoap11(info.imagingUrl, username, password, "http://www.onvif.org/ver20/imaging/wsdl/Move", body, [](QNetworkReply *r){ r->deleteLater(); });
            }
            reply->deleteLater();
        });
    });
}

void PtzController::stopFocus(const QString &ip, int port, const QString &username, const QString &password)
{
    qDebug() << "PtzController::stopFocus" << ip << port;
    ensureDiscovered(ip, port, username, password, [this, ip, port, username, password](bool success) {
        if (!success) {
            return;
        }

        QString key = QString("%1:%2").arg(ip).arg(port);
        DeviceInfo &info = m_cache[key];

        if (info.imagingUrl.isEmpty() || info.videoSourceToken.isEmpty()) {
            return;
        }

        QString body = QString(
            "<Stop xmlns=\"http://www.onvif.org/ver20/imaging/wsdl\">"
            "<VideoSourceToken>%1</VideoSourceToken>"
            "</Stop>"
        ).arg(info.videoSourceToken);

        sendSoap12(info.imagingUrl, username, password, "http://www.onvif.org/ver20/imaging/wsdl/Stop", body, [this, info, username, password, body](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError) {
                sendSoap11(info.imagingUrl, username, password, "http://www.onvif.org/ver20/imaging/wsdl/Stop", body, [](QNetworkReply *r){ r->deleteLater(); });
            }
            reply->deleteLater();
        });
    });
}

void PtzController::ensureDiscovered(const QString &ip, int port, const QString &username, const QString &password, std::function<void(bool)> callback)
{
    QString key = QString("%1:%2").arg(ip).arg(port);
    if (m_cache.contains(key) && m_cache[key].discovered) {
        callback(true);
        return;
    }

    // Candidate ports from reference implementation
    QStringList candidatePorts;
    candidatePorts << QString::number(port); // Preferred
    candidatePorts << "8899" << "8999" << "8080" << "8000" << "85" << "8001" << "81";
    // Remove duplicates if port is already in list
    candidatePorts.removeDuplicates();

    QStringList candidatePaths;
    candidatePaths << "/onvif/device_service" << "/device_service" << "/onvif/device";

    probeDevice(ip, port, username, password, 0, 0, candidatePorts, candidatePaths, callback);
}

void PtzController::probeDevice(const QString &ip, int port, const QString &username, const QString &password, int portIndex, int pathIndex, const QStringList &candidatePorts, const QStringList &candidatePaths, std::function<void(bool)> callback)
{
    if (portIndex >= candidatePorts.size()) {
        qWarning() << "All candidate ports failed for ONVIF discovery on" << ip;
        callback(false);
        return;
    }

    if (pathIndex >= candidatePaths.size()) {
        // Try next port, reset path index
        probeDevice(ip, port, username, password, portIndex + 1, 0, candidatePorts, candidatePaths, callback);
        return;
    }

    QString currentPort = candidatePorts[portIndex];
    QString currentPath = candidatePaths[pathIndex];
    QString url = QString("http://%1:%2%3").arg(ip, currentPort, currentPath);
    
    qDebug() << "Probing ONVIF Device Service at" << url;

    getCapabilities(url, username, password, [this, ip, port, username, password, portIndex, pathIndex, candidatePorts, candidatePaths, callback](QString ptzUrl, QString mediaUrl, QString imagingUrl) {
        if (!ptzUrl.isEmpty()) {
            qDebug() << "Found PTZ URL:" << ptzUrl;
            if (!imagingUrl.isEmpty()) qDebug() << "Found Imaging URL:" << imagingUrl;
            
            // Now get Profile Token
            if (mediaUrl.isEmpty()) {
                 qWarning() << "PTZ URL found but Media URL missing. Cannot get profiles.";
            }

            getProfiles(mediaUrl, username, password, [this, ip, port, ptzUrl, imagingUrl, callback](QString token, QString vsToken) {
                if (!token.isEmpty()) {
                    qDebug() << "Found Profile Token:" << token << "VideoSourceToken:" << vsToken;
                    QString key = QString("%1:%2").arg(ip).arg(port);
                    m_cache[key] = {ptzUrl, imagingUrl, token, vsToken, true};
                    callback(true);
                } else {
                    qWarning() << "Failed to get Profile Token";
                    callback(false);
                }
            });

        } else {
            // Try next path on same port
            probeDevice(ip, port, username, password, portIndex, pathIndex + 1, candidatePorts, candidatePaths, callback);
        }
    });
}

void PtzController::getCapabilities(const QString &url, const QString &username, const QString &password, std::function<void(QString, QString, QString)> callback)
{
    QString body = 
        "<GetCapabilities xmlns=\"http://www.onvif.org/ver10/device/wsdl\">"
        "<Category>All</Category>"
        "</GetCapabilities>";

    sendSoap12(url, username, password, "http://www.onvif.org/ver10/device/wsdl/GetCapabilities", body, [this, url, username, password, body, callback](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString resp = reply->readAll();
            // Parse PTZ URL - handle both prefixed and non-prefixed tags
            QRegularExpression rePtz("(?is)(?:<[^:>]*:PTZ[^>]*>|<PTZ[^>]*>).*?(?:<[^:>]*:XAddr>|<XAddr>)([^<]+)(?:</[^:>]*:XAddr>|</XAddr>)");
            QRegularExpressionMatch matchPtz = rePtz.match(resp);
            QString ptzUrl = matchPtz.hasMatch() ? matchPtz.captured(1) : QString();

            // Parse Media URL
            QRegularExpression reMedia("(?is)(?:<[^:>]*:Media[^>]*>|<Media[^>]*>).*?(?:<[^:>]*:XAddr>|<XAddr>)([^<]+)(?:</[^:>]*:XAddr>|</XAddr>)");
            QRegularExpressionMatch matchMedia = reMedia.match(resp);
            QString mediaUrl = matchMedia.hasMatch() ? matchMedia.captured(1) : QString();

            // Parse Imaging URL
            QRegularExpression reImaging("(?is)(?:<[^:>]*:Imaging[^>]*>|<Imaging[^>]*>).*?(?:<[^:>]*:XAddr>|<XAddr>)([^<]+)(?:</[^:>]*:XAddr>|</XAddr>)");
            QRegularExpressionMatch matchImaging = reImaging.match(resp);
            QString imagingUrl = matchImaging.hasMatch() ? matchImaging.captured(1) : QString();

            // Fallback: If PTZ found but Media not, guess Media URL
            if (mediaUrl.isEmpty() && !ptzUrl.isEmpty()) {
                QString guess = ptzUrl;
                if (guess.contains("PTZ", Qt::CaseInsensitive)) {
                     guess.replace("PTZ", "Media", Qt::CaseInsensitive);
                     qWarning() << "Media URL missing, guessing from PTZ URL:" << guess;
                     mediaUrl = guess;
                }
            }

            if (ptzUrl.isEmpty() || mediaUrl.isEmpty()) {
                 qWarning() << "SOAP 1.2 Success but failed to parse URLs. PTZ:" << ptzUrl << "Media:" << mediaUrl << "Imaging:" << imagingUrl << "Response:" << resp;
            }

            callback(ptzUrl, mediaUrl, imagingUrl);
        } else {
            // Try SOAP 1.1
             sendSoap11(url, username, password, "http://www.onvif.org/ver10/device/wsdl/GetCapabilities", body, [callback](QNetworkReply *reply) {
                if (reply->error() == QNetworkReply::NoError) {
                    QString resp = reply->readAll();
                    QRegularExpression rePtz("(?is)(?:<[^:>]*:PTZ[^>]*>|<PTZ[^>]*>).*?(?:<[^:>]*:XAddr>|<XAddr>)([^<]+)(?:</[^:>]*:XAddr>|</XAddr>)");
                    QRegularExpressionMatch matchPtz = rePtz.match(resp);
                    QString ptzUrl = matchPtz.hasMatch() ? matchPtz.captured(1) : QString();

                    QRegularExpression reMedia("(?is)(?:<[^:>]*:Media[^>]*>|<Media[^>]*>).*?(?:<[^:>]*:XAddr>|<XAddr>)([^<]+)(?:</[^:>]*:XAddr>|</XAddr>)");
                    QRegularExpressionMatch matchMedia = reMedia.match(resp);
                    QString mediaUrl = matchMedia.hasMatch() ? matchMedia.captured(1) : QString();

                    QRegularExpression reImaging("(?is)(?:<[^:>]*:Imaging[^>]*>|<Imaging[^>]*>).*?(?:<[^:>]*:XAddr>|<XAddr>)([^<]+)(?:</[^:>]*:XAddr>|</XAddr>)");
                    QRegularExpressionMatch matchImaging = reImaging.match(resp);
                    QString imagingUrl = matchImaging.hasMatch() ? matchImaging.captured(1) : QString();

                    // Fallback: If PTZ found but Media not, guess Media URL
                    if (mediaUrl.isEmpty() && !ptzUrl.isEmpty()) {
                        QString guess = ptzUrl;
                        if (guess.contains("PTZ", Qt::CaseInsensitive)) {
                            guess.replace("PTZ", "Media", Qt::CaseInsensitive);
                            qWarning() << "Media URL missing, guessing from PTZ URL:" << guess;
                            mediaUrl = guess;
                        }
                    }

                    if (ptzUrl.isEmpty() || mediaUrl.isEmpty()) {
                         qWarning() << "SOAP 1.1 Success but failed to parse URLs. PTZ:" << ptzUrl << "Media:" << mediaUrl << "Imaging:" << imagingUrl << "Response:" << resp;
                    }

                    callback(ptzUrl, mediaUrl, imagingUrl);
                } else {
                    callback(QString(), QString(), QString());
                }
                reply->deleteLater();
             });
        }
        reply->deleteLater();
    });
}

void PtzController::getProfiles(const QString &mediaUrl, const QString &username, const QString &password, std::function<void(QString, QString)> callback)
{
    if (mediaUrl.isEmpty()) {
        callback(QString(), QString());
        return;
    }

    QString body = "<GetProfiles xmlns=\"http://www.onvif.org/ver10/media/wsdl\"/>";
    
    auto parseProfiles = [](const QString &resp) -> QPair<QString, QString> {
        // Find first Profile block
        // We look for the start of a Profile and capture until the end tag or start of next profile (heuristic)
        // Actually, let's just find the first Profile token, and then search for VideoSourceConfiguration/SourceToken nearby?
        // No, that's risky.
        // Let's try to match the whole Profile block.
        QRegularExpression reProfile("(?is)<(?:[^:>]*:)?Profiles[^>]*token=\"([^\"]+)\"(.*?)</(?:[^:>]*:)?Profiles>");
        QRegularExpressionMatch matchProfile = reProfile.match(resp);
        
        if (matchProfile.hasMatch()) {
            QString token = matchProfile.captured(1);
            QString content = matchProfile.captured(2);
            
            QRegularExpression reVs("(?is)<(?:[^:>]*:)?VideoSourceConfiguration.*?(?:<(?:[^:>]*:)?SourceToken>)([^<]+)</(?:[^:>]*:)?SourceToken>");
            QRegularExpressionMatch matchVs = reVs.match(content);
            QString vsToken = matchVs.hasMatch() ? matchVs.captured(1) : QString();
            
            return {token, vsToken};
        }
        
        // Fallback simple
        QRegularExpression re2("(?is)<Profiles[^>]*token=\"([^\"]+)\"");
        QRegularExpressionMatch match2 = re2.match(resp);
        return {match2.hasMatch() ? match2.captured(1) : QString(), QString()};
    };

    sendSoap12(mediaUrl, username, password, "http://www.onvif.org/ver10/media/wsdl/GetProfiles", body, [this, mediaUrl, username, password, body, callback, parseProfiles](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString resp = reply->readAll();
            auto result = parseProfiles(resp);
            callback(result.first, result.second);
        } else {
             sendSoap11(mediaUrl, username, password, "http://www.onvif.org/ver10/media/wsdl/GetProfiles", body, [callback, parseProfiles](QNetworkReply *reply) {
                if (reply->error() == QNetworkReply::NoError) {
                    QString resp = reply->readAll();
                    auto result = parseProfiles(resp);
                    callback(result.first, result.second);
                } else {
                    qWarning() << "GetProfiles failed (SOAP 1.1):" << reply->errorString();
                    callback(QString(), QString());
                }
                reply->deleteLater();
             });
        }
        reply->deleteLater();
    });
}

void PtzController::sendSoap12(const QString &urlStr, const QString &username, const QString &password, const QString &action, const QString &bodyContent, std::function<void(QNetworkReply*)> onFinished)
{
    QUrl url(urlStr);
    QNetworkRequest request(url);
    // Rust implementation does NOT include action in Content-Type for SOAP 1.2
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    // request.setRawHeader("Connection", "close"); // Let QNAM handle connection (Keep-Alive)
    request.setRawHeader("User-Agent", "curl/8.14.1");
    request.setRawHeader("Accept", "*/*");
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    QString envelope = createSoapEnvelope(username, password, bodyContent, true);
    QByteArray data = envelope.toUtf8();

    QNetworkReply *reply = m_nam->post(request, data);
    
    // Handle SSL errors just in case, though we are likely on HTTP
    connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &errors) {
        qWarning() << "SSL verification failed:" << errors;
        reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this, [reply, onFinished, urlStr]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "SOAP 1.2 Error on" << urlStr << ":" << reply->errorString() << "HTTP:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->bytesAvailable() > 0) {
                 qDebug() << "Response Body:" << reply->readAll();
                 reply->open(QIODevice::ReadOnly);
                 reply->seek(0);
            }
        }
        onFinished(reply);
    });
}

void PtzController::sendSoap11(const QString &urlStr, const QString &username, const QString &password, const QString &action, const QString &bodyContent, std::function<void(QNetworkReply*)> onFinished)
{
    QUrl url(urlStr);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=utf-8");
    request.setRawHeader("SOAPAction", action.toUtf8());
    // request.setRawHeader("Connection", "close"); // Let QNAM handle connection
    request.setRawHeader("User-Agent", "curl/8.14.1");
    request.setRawHeader("Accept", "*/*");
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    QString envelope = createSoapEnvelope(username, password, bodyContent, false);
    QByteArray data = envelope.toUtf8();

    QNetworkReply *reply = m_nam->post(request, data);

    connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &errors) {
        qWarning() << "SSL verification failed:" << errors;
        reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this, [reply, onFinished, urlStr]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "SOAP 1.1 Error on" << urlStr << ":" << reply->errorString() << "HTTP:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->bytesAvailable() > 0) {
                 qDebug() << "Response Body:" << reply->readAll();
                 reply->open(QIODevice::ReadOnly);
                 reply->seek(0);
            }
        }
        onFinished(reply);
    });
}

QString PtzController::createSoapEnvelope(const QString &username, const QString &password, const QString &body, bool isSoap12)
{
    QString headerBlock;
    if (!username.isEmpty()) {
        QString nonce = generateNonce();
        QString created = getTimestamp();
        QString passwordDigest = generatePasswordDigest(nonce, created, password);
        
        headerBlock = QString(
            "<s:Header>\n"
            "<Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">\n"
            "<UsernameToken>\n"
            "<Username>%1</Username>\n"
            "<Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">%2</Password>\n"
            "<Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">%3</Nonce>\n"
            "<Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">%4</Created>\n"
            "</UsernameToken>\n"
            "</Security>\n"
            "</s:Header>\n"
        ).arg(username, passwordDigest, nonce, created);
    }

    QString envelope;
    if (isSoap12) {
        envelope = QString(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\">\n"
            "%1"
            "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n"
            "%2\n"
            "</s:Body>\n"
            "</s:Envelope>"
        ).arg(headerBlock, body);
    } else {
        envelope = QString(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
            "%1"
            "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n"
            "%2\n"
            "</s:Body>\n"
            "</s:Envelope>"
        ).arg(headerBlock, body);
    }

    return envelope;
}

QString PtzController::generateNonce()
{
    QByteArray nonce = QUuid::createUuid().toRfc4122();
    return QString::fromLatin1(nonce.toBase64());
}

QString PtzController::getTimestamp()
{
    // Match Rust's chrono::Utc::now().format("%Y-%m-%dT%H:%M:%SZ")
    // Rust SecondsFormat::Secs does NOT include milliseconds.
    return QDateTime::currentDateTimeUtc().toString("yyyy-MM-ddTHH:mm:ssZ");
}

QString PtzController::generatePasswordDigest(const QString &nonceBase64, const QString &created, const QString &password)
{
    // Digest = B64(SHA1(B64_DECODE(Nonce) + Created + Password))
    QByteArray nonce = QByteArray::fromBase64(nonceBase64.toLatin1());
    QByteArray timestamp = created.toUtf8();
    QByteArray pwd = password.toUtf8();

    QByteArray data = nonce + timestamp + pwd;
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha1);
    return QString::fromLatin1(hash.toBase64());
}
