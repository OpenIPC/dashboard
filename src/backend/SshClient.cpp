#include "SshClient.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QStandardPaths>
#include <libssh/libssh.h>

namespace {

QString knownHostsFilePath()
{
    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    const QString path = dir.filePath("known_hosts");
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::WriteOnly);
        file.close();
    }
    return path;
}

QString currentServerFingerprint(ssh_session session)
{
    ssh_key serverKey = nullptr;
    unsigned char *hash = nullptr;
    size_t hashLength = 0;

    QString fingerprint;
    if (ssh_get_server_publickey(session, &serverKey) == SSH_OK) {
        if (ssh_get_publickey_hash(serverKey, SSH_PUBLICKEY_HASH_SHA256, &hash, &hashLength) == SSH_OK && hash && hashLength > 0) {
            fingerprint = QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(hash), static_cast<int>(hashLength)).toBase64());
            ssh_clean_pubkey_hash(&hash);
        }
        ssh_key_free(serverKey);
    }

    return fingerprint;
}

bool verifyKnownHost(ssh_session session, QString &errorMessage, QString &infoMessage)
{
    const auto knownState = ssh_session_is_known_server(session);
    switch (knownState) {
    case SSH_KNOWN_HOSTS_OK:
        return true;
    case SSH_KNOWN_HOSTS_NOT_FOUND:
    case SSH_KNOWN_HOSTS_UNKNOWN:
        if (ssh_session_update_known_hosts(session) != SSH_OK) {
            errorMessage = QString("Failed to store SSH host key: %1").arg(ssh_get_error(session));
            return false;
        }
        infoMessage = QString("Trusted new SSH host key (SHA256:%1)").arg(currentServerFingerprint(session));
        return true;
    case SSH_KNOWN_HOSTS_CHANGED:
        errorMessage = QString("SSH host key mismatch detected (SHA256:%1). Connection refused.").arg(currentServerFingerprint(session));
        return false;
    case SSH_KNOWN_HOSTS_OTHER:
        errorMessage = QString("A different SSH host key type is already stored for this host (SHA256:%1). Connection refused.").arg(currentServerFingerprint(session));
        return false;
    case SSH_KNOWN_HOSTS_ERROR:
    default:
        errorMessage = QString("Unable to validate SSH host key: %1").arg(ssh_get_error(session));
        return false;
    }
}

} // namespace

SshWorker::SshWorker(QObject *parent)
    : QThread(parent), m_session(nullptr), m_channel(nullptr), m_abort(false), m_connectRequested(false), m_disconnectRequested(false)
{
}

SshWorker::~SshWorker()
{
    m_mutex.lock();
    m_abort = true;
    m_cond.wakeOne();
    m_mutex.unlock();
    wait();
}

void SshWorker::connectToHost(const QString &ip, const QString &user, const QString &password)
{
    QMutexLocker locker(&m_mutex);
    m_ip = ip;
    m_user = user;
    m_password = password;
    m_connectRequested = true;
    m_disconnectRequested = false;
    m_cond.wakeOne();
    if (!isRunning()) {
        start();
    }
}

void SshWorker::sendCommand(const QString &command)
{
    QMutexLocker locker(&m_mutex);
    m_commandQueue.append(command);
    m_cond.wakeOne();
}

void SshWorker::disconnectFromHost()
{
    QMutexLocker locker(&m_mutex);
    m_disconnectRequested = true;
    m_cond.wakeOne();
}

void SshWorker::clearSensitiveData()
{
    if (!m_password.isEmpty()) {
        m_password.fill(QChar(u'\0'));
        m_password.clear();
    }
}

void SshWorker::cleanup()
{
    if (m_channel) {
        ssh_channel_close(m_channel);
        ssh_channel_free(m_channel);
        m_channel = nullptr;
    }
    if (m_session) {
        ssh_disconnect(m_session);
        ssh_free(m_session);
        m_session = nullptr;
    }
}

void SshWorker::run()
{
    while (true) {
        m_mutex.lock();
        if (m_abort) {
            m_mutex.unlock();
            break;
        }
        
        bool doConnect = m_connectRequested;
        bool doDisconnect = m_disconnectRequested;
        QStringList cmds = m_commandQueue;
        m_commandQueue.clear();
        m_connectRequested = false;
        m_disconnectRequested = false;
        m_mutex.unlock();

        if (doDisconnect) {
            cleanup();
            clearSensitiveData();
            emit disconnected();
        }

        if (doConnect) {
            cleanup();
            
            m_session = ssh_new();
            if (m_session == nullptr) {
                emit errorOccurred("Failed to create SSH session");
                continue;
            }

            ssh_options_set(m_session, SSH_OPTIONS_HOST, m_ip.toStdString().c_str());
            ssh_options_set(m_session, SSH_OPTIONS_USER, m_user.toStdString().c_str());

            const QByteArray knownHostsPath = QDir::toNativeSeparators(::knownHostsFilePath()).toLocal8Bit();
            ssh_options_set(m_session, SSH_OPTIONS_KNOWNHOSTS, knownHostsPath.constData());

            int rc = ssh_connect(m_session);
            if (rc != SSH_OK) {
                emit errorOccurred(QString("Error connecting to host: %1").arg(ssh_get_error(m_session)));
                cleanup();
                clearSensitiveData();
                continue;
            }

            QString hostKeyError;
            QString hostKeyInfo;
            if (!verifyKnownHost(m_session, hostKeyError, hostKeyInfo)) {
                emit errorOccurred(hostKeyError);
                cleanup();
                clearSensitiveData();
                continue;
            }
            if (!hostKeyInfo.isEmpty()) {
                emit dataReceived(hostKeyInfo + "\n");
            }

            rc = ssh_userauth_password(m_session, nullptr, m_password.toStdString().c_str());
            if (rc != SSH_AUTH_SUCCESS) {
                emit errorOccurred(QString("Error authenticating with password: %1").arg(ssh_get_error(m_session)));
                cleanup();
                clearSensitiveData();
                continue;
            }

            clearSensitiveData();

            m_channel = ssh_channel_new(m_session);
            if (m_channel == nullptr) {
                emit errorOccurred("Error creating SSH channel");
                cleanup();
                continue;
            }

            rc = ssh_channel_open_session(m_channel);
            if (rc != SSH_OK) {
                emit errorOccurred("Error opening SSH channel");
                cleanup();
                continue;
            }

            rc = ssh_channel_request_pty(m_channel);
            if (rc != SSH_OK) {
                emit errorOccurred("Error requesting PTY");
                cleanup();
                continue;
            }

            rc = ssh_channel_request_shell(m_channel);
            if (rc != SSH_OK) {
                emit errorOccurred("Error requesting shell");
                cleanup();
                continue;
            }

            emit connected();
        }

        if (m_channel && ssh_channel_is_open(m_channel)) {
            for (const QString &cmd : cmds) {
                const QByteArray commandUtf8 = cmd.toUtf8();
                ssh_channel_write(m_channel, commandUtf8.constData(), static_cast<uint32_t>(commandUtf8.size()));
                if (!cmd.endsWith('\n') && cmd != "\x03") {
                    ssh_channel_write(m_channel, "\n", 1);
                }
            }

            char buffer[1024];
            int nbytes = ssh_channel_read_nonblocking(m_channel, buffer, sizeof(buffer), 0);
            if (nbytes > 0) {
                emit dataReceived(QString::fromUtf8(buffer, nbytes));
            } else if (nbytes < 0) {
                emit errorOccurred("Error reading from SSH channel");
                cleanup();
                emit disconnected();
            }
            
            nbytes = ssh_channel_read_nonblocking(m_channel, buffer, sizeof(buffer), 1);
            if (nbytes > 0) {
                emit dataReceived(QString::fromUtf8(buffer, nbytes));
            }
        }

        m_mutex.lock();
        if (!m_abort && !m_connectRequested && !m_disconnectRequested && m_commandQueue.isEmpty()) {
            if (m_channel && ssh_channel_is_open(m_channel)) {
                m_cond.wait(&m_mutex, 50);
            } else {
                m_cond.wait(&m_mutex);
            }
        }
        m_mutex.unlock();
    }
    cleanup();
    clearSensitiveData();
}

SshClient::SshClient(QObject *parent)
    : QObject(parent), m_worker(new SshWorker(this)), m_isConnected(false)
{
    connect(m_worker, &SshWorker::connected, this, &SshClient::onWorkerConnected);
    connect(m_worker, &SshWorker::disconnected, this, &SshClient::onWorkerDisconnected);
    connect(m_worker, &SshWorker::dataReceived, this, &SshClient::onWorkerDataReceived);
    connect(m_worker, &SshWorker::errorOccurred, this, &SshClient::onWorkerErrorOccurred);
}

SshClient::~SshClient()
{
    m_worker->disconnectFromHost();
    m_worker->wait();
}

bool SshClient::isConnected() const
{
    return m_isConnected;
}

void SshClient::connectToHost(const QString &ip, const QString &user, const QString &password)
{
    m_worker->connectToHost(ip, user, password);
}

void SshClient::sendCommand(const QString &command)
{
    m_worker->sendCommand(command);
}

void SshClient::disconnectFromHost()
{
    m_worker->disconnectFromHost();
}

void SshClient::onWorkerConnected()
{
    m_isConnected = true;
    emit connectedChanged();
}

void SshClient::onWorkerDisconnected()
{
    m_isConnected = false;
    emit connectedChanged();
}

void SshClient::onWorkerDataReceived(const QString &data)
{
    emit dataReceived(data);
}

void SshClient::onWorkerErrorOccurred(const QString &error)
{
    emit errorOccurred(error);
}
