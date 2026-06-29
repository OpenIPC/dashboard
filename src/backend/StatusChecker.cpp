#include "StatusChecker.h"
#include "CameraModel.h"
#include <QDebug>
#include <QUrl>

StatusChecker::StatusChecker(CameraModel *model, QObject *parent)
    : QObject(parent), m_model(model), m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &StatusChecker::checkAll);
}

void StatusChecker::start(int intervalMs)
{
    checkAll(); // Run immediately
    m_timer->start(intervalMs);
}

void StatusChecker::stop()
{
    m_timer->stop();
}

void StatusChecker::checkAll()
{
    if (!m_model) return;

    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto cam = m_model->getCamera(i);
        if (cam.ip.isEmpty()) continue;

        checkOne(cam.ip);
    }
}

void StatusChecker::checkOne(const QString &cameraIp)
{
    if (!m_model) return;

    const QString ip = cameraIp.trimmed();
    const int index = m_model->findIndexByIp(ip);
    if (index < 0) return;

    const auto cam = m_model->getCamera(index);
    if (cam.ip.isEmpty()) return;

    QString host = cam.ip;
    int port = cam.port > 0 ? cam.port : 554;

    const QStringList streamUrls = {
        cam.hdStreamUrl,
        cam.sdStreamUrl,
        cam.streamUrl
    };
    for (const QString &streamUrl : streamUrls) {
        const QUrl url(streamUrl);
        if (!url.isValid() || url.host().isEmpty()) {
            continue;
        }

        host = url.host();
        port = url.port(port > 0 ? port : 554);
        break;
    }

    checkCamera(cam.ip, host, port);
}

void StatusChecker::checkCamera(const QString &cameraIp, const QString &host, int port)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setProperty("cameraIp", cameraIp);
    
    connect(socket, &QTcpSocket::connected, this, &StatusChecker::onSocketConnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &StatusChecker::onSocketError);
    
    // Safety cleanup
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    socket->connectToHost(host, port);
    
    // Connection timeout - 3 seconds
    QTimer::singleShot(3000, socket, [socket, this]() {
        // If the socket was already connected and disconnected, it might be deleted. 
        // But QTimer with context object 'socket' won't fire if socket is deleted.
        // So this lambda only runs if we are still alive and likely stuck in ConnectingState.
        
        if (socket->state() != QAbstractSocket::ConnectedState) {
            // Treat as timeout/error
            QString ip = socket->property("cameraIp").toString();
            int idx = m_model->findIndexByIp(ip);
            if (idx >= 0) {
                 emit cameraStatusResolved(ip, "Offline");
                 emit cameraStatusDetailResolved(ip, QStringLiteral("RTSP-порт недоступен или не отвечает"));
            }
            socket->abort();
            socket->deleteLater();
        }
    });
}

void StatusChecker::onSocketConnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString ip = socket->property("cameraIp").toString();
    int index = m_model->findIndexByIp(ip);
    if (index >= 0) {
        emit cameraStatusResolved(ip, "Online");
        emit cameraStatusDetailResolved(ip, QString());
    }
    
    socket->disconnectFromHost();
    socket->deleteLater();
}

void StatusChecker::onSocketError(QAbstractSocket::SocketError)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString ip = socket->property("cameraIp").toString();
    int index = m_model->findIndexByIp(ip);
    // Suppress error if we are just closing
    // if (socket->state() == QAbstractSocket::UnconnectedState) return;

    if (index >= 0) {
        emit cameraStatusResolved(ip, "Offline");
        emit cameraStatusDetailResolved(ip, QStringLiteral("RTSP-порт недоступен"));
    }
    
    socket->abort();
    socket->deleteLater();
}

void StatusChecker::onTimeout()
{
    // Not used in this implementation (lambda used instead)
}
