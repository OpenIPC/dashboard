#ifndef STATUSCHECKER_H
#define STATUSCHECKER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QTcpSocket>
#include <QAbstractSocket>

class CameraModel;

class StatusChecker : public QObject
{
    Q_OBJECT
public:
    explicit StatusChecker(CameraModel *model, QObject *parent = nullptr);
    void start(int intervalMs = 5000); // 5 seconds default
    void stop();
    void checkOne(const QString &cameraIp);

signals:
    void cameraStatusResolved(const QString &cameraIp, const QString &status);
    void cameraStatusDetailResolved(const QString &cameraIp, const QString &detail);

private slots:
    void checkAll();
    void onSocketConnected();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onTimeout();

private:
    CameraModel *m_model;
    QTimer *m_timer;
    void checkCamera(const QString &cameraIp, const QString &host, int port);
};

#endif // STATUSCHECKER_H
