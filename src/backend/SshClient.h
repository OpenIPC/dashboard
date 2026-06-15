#ifndef SSHCLIENT_H
#define SSHCLIENT_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>

struct ssh_session_struct;
typedef struct ssh_session_struct* ssh_session;
struct ssh_channel_struct;
typedef struct ssh_channel_struct* ssh_channel;

class SshWorker : public QThread
{
    Q_OBJECT
public:
    explicit SshWorker(QObject *parent = nullptr);
    ~SshWorker();

    void connectToHost(const QString &ip, const QString &user, const QString &password);
    void sendCommand(const QString &command);
    void disconnectFromHost();

signals:
    void connected();
    void disconnected();
    void dataReceived(const QString &data);
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    QString m_ip;
    QString m_user;
    QString m_password;
    
    ssh_session m_session;
    ssh_channel m_channel;
    
    QMutex m_mutex;
    QWaitCondition m_cond;
    QStringList m_commandQueue;
    bool m_abort;
    bool m_connectRequested;
    bool m_disconnectRequested;
    
    void cleanup();
    void clearSensitiveData();
};

class SshClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit SshClient(QObject *parent = nullptr);
    ~SshClient();

    bool isConnected() const;

    Q_INVOKABLE void connectToHost(const QString &ip, const QString &user, const QString &password);
    Q_INVOKABLE void sendCommand(const QString &command);
    Q_INVOKABLE void disconnectFromHost();

signals:
    void connectedChanged();
    void dataReceived(const QString &data);
    void errorOccurred(const QString &error);

private slots:
    void onWorkerConnected();
    void onWorkerDisconnected();
    void onWorkerDataReceived(const QString &data);
    void onWorkerErrorOccurred(const QString &error);

private:
    SshWorker *m_worker;
    bool m_isConnected;
};

#endif // SSHCLIENT_H
