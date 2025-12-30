#ifndef SSHCLIENT_H
#define SSHCLIENT_H

#include <QObject>
#include <QProcess>

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
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onStateChanged(QProcess::ProcessState newState);

private:
    QProcess *m_process;
    QString m_password;
    bool m_passwordSent;
    bool m_isConnected;
    QByteArray m_receiveBuffer;
};

#endif // SSHCLIENT_H
