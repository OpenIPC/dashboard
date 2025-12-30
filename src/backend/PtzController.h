#ifndef PTZCONTROLLER_H
#define PTZCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>

class PtzController : public QObject
{
    Q_OBJECT
public:
    explicit PtzController(QObject *parent = nullptr);

    // x, y, zoom are speeds between -1.0 and 1.0
    Q_INVOKABLE void move(const QString &ip, int port, const QString &username, const QString &password, float x, float y, float zoom);
    Q_INVOKABLE void stop(const QString &ip, int port, const QString &username, const QString &password);
    
    // speed between -1.0 and 1.0
    Q_INVOKABLE void focus(const QString &ip, int port, const QString &username, const QString &password, float speed);
    Q_INVOKABLE void stopFocus(const QString &ip, int port, const QString &username, const QString &password);

private:
    QNetworkAccessManager *m_nam;
    
    // Cache for discovered info
    struct DeviceInfo {
        QString ptzUrl;
        QString imagingUrl;
        QString profileToken;
        QString videoSourceToken;
        bool discovered = false;
    };
    QMap<QString, DeviceInfo> m_cache; // Key: ip:port

    void ensureDiscovered(const QString &ip, int port, const QString &username, const QString &password, std::function<void(bool)> callback);
    void probeDevice(const QString &ip, int port, const QString &username, const QString &password, int portIndex, int pathIndex, const QStringList &candidatePorts, const QStringList &candidatePaths, std::function<void(bool)> callback);
    void getCapabilities(const QString &url, const QString &username, const QString &password, std::function<void(QString ptzUrl, QString mediaUrl, QString imagingUrl)> callback);
    void getProfiles(const QString &mediaUrl, const QString &username, const QString &password, std::function<void(QString token, QString vsToken)> callback);

    void sendSoap12(const QString &url, const QString &username, const QString &password, const QString &action, const QString &bodyContent, std::function<void(QNetworkReply*)> onFinished);
    void sendSoap11(const QString &url, const QString &username, const QString &password, const QString &action, const QString &bodyContent, std::function<void(QNetworkReply*)> onFinished);
    
    QString createSoapEnvelope(const QString &username, const QString &password, const QString &body, bool isSoap12);
    QString generateNonce();
    QString getTimestamp();
    QString generatePasswordDigest(const QString &nonce, const QString &created, const QString &password);
};

#endif // PTZCONTROLLER_H
