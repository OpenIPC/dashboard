#include "SshClient.h"
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QProcessEnvironment>
#include <QCoreApplication>

SshClient::SshClient(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_passwordSent(false)
    , m_isConnected(false)
{
    connect(m_process, &QProcess::readyReadStandardOutput, this, &SshClient::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &SshClient::onReadyReadStandardError);
    connect(m_process, &QProcess::stateChanged, this, &SshClient::onStateChanged);
}

SshClient::~SshClient()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished();
    }
}

bool SshClient::isConnected() const
{
    return m_isConnected;
}

void SshClient::connectToHost(const QString &ip, const QString &user, const QString &password)
{
    if (m_process->state() != QProcess::NotRunning) {
        disconnectFromHost();
    }

    m_password = password;
    m_passwordSent = false;
    m_receiveBuffer.clear();
    
    QString program = "ssh";
    QStringList args;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Determine if we should use ssh (OpenSSH) or plink
    // We prefer OpenSSH if available because it supports PTY allocation (-tt) reliably which gives us banners/prompts
    bool usePlink = true;
    
    // Check for ssh in path
    if (!QStandardPaths::findExecutable("ssh").isEmpty()) {
        usePlink = false;
        program = "ssh";
        
        // Create simple batch file helper for ASKPASS to avoid complex environment inheritance issues on Windows
        // and avoid the need for the main application to handle ASKPASS mode.
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString askPassBat = QDir::toNativeSeparators(tempPath + "/ssh_askpass_auth.bat");
        QFile batchFile(askPassBat);
        if (batchFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&batchFile);
            out << "@echo off\n";
            // Escape special batch characters in password if needed, but for now assume simple
            out << "echo " << password << "\n"; 
            batchFile.close();
        }

        env.insert("SSH_ASKPASS", askPassBat);
        env.insert("SSH_ASKPASS_REQUIRE", "force");
        // Dummy display required for ASKPASS to trigger
        if (!env.contains("DISPLAY")) {
            env.insert("DISPLAY", "dummy:0");
        }
        
        // -tt forces pseudo-tty allocation (important for interactive shell behavior)
        args << "-tt" << "-o" << "StrictHostKeyChecking=no" << "-o" << "UserKnownHostsFile=/dev/null" << (user + "@" + ip);
    }
#ifdef Q_OS_WIN
    // Fallback to plink ONLY if ssh is missing (rare on modern Windows) or if specifically desired
    else if (!QStandardPaths::findExecutable("plink").isEmpty()) {
        usePlink = true;
        program = "plink";
        // -t forces PTY allocation, -batch disables interactive prompts (we handle host keys manually if needed, or rely on -batch to fail them? No, better to be interactive)
        // Actually, for plink, -t is strictly pty allocation.
        args << "-ssh" << "-l" << user << "-pw" << password << "-t" << ip;
    }
#endif
    else {
         emit errorOccurred("No SSH client found (ssh or plink)");
         return;
    }

    m_process->setProcessEnvironment(env);
    
    qDebug() << "Starting SSH:" << program << args;
    m_process->start(program, args);
}

void SshClient::sendCommand(const QString &command)
{
    if (m_process->state() == QProcess::Running) {
        m_process->write(command.toUtf8());
        m_process->write("\n");
    }
}

void SshClient::disconnectFromHost()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
}

void SshClient::onReadyReadStandardOutput()
{
    QByteArray data = m_process->readAllStandardOutput();
    QString text = QString::fromUtf8(data);
    
    // Automatic handling of Plink host key prompt
    if (text.contains("Store key in cache?", Qt::CaseInsensitive) || 
        text.contains("Update cached key?", Qt::CaseInsensitive)) {
        m_process->write("y\n");
    }
    
    // Fallback for SSH password prompt if ASKPASS fails (OpenSSH sometimes prompts on TTY/stdout even with ASKPASS set)
    if (!m_passwordSent && !m_password.isEmpty()) {
        if (text.contains("password:", Qt::CaseInsensitive) || text.contains("passphrase", Qt::CaseInsensitive)) {
            m_process->write(m_password.toUtf8());
            m_process->write("\n");
            m_passwordSent = true;
        }
    }

    emit dataReceived(text);
}

void SshClient::onReadyReadStandardError()
{
    QByteArray data = m_process->readAllStandardError();
    QString text = QString::fromUtf8(data);
    
    // Fallback for SSH password prompt if it appears on stderr
    if (!m_passwordSent && !m_password.isEmpty()) {
        if (text.contains("password:", Qt::CaseInsensitive) || text.contains("passphrase", Qt::CaseInsensitive)) {
            m_process->write(m_password.toUtf8());
            m_process->write("\n");
            m_passwordSent = true;
        }
    }

    emit dataReceived(text);
}

void SshClient::onStateChanged(QProcess::ProcessState newState)
{
    bool connected = (newState == QProcess::Running);
    if (m_isConnected != connected) {
        m_isConnected = connected;
        emit connectedChanged();
    }
    
    if (newState == QProcess::NotRunning) {
        QString error = m_process->readAllStandardError();
        if (!error.isEmpty()) {
            emit errorOccurred(error);
        }
    }
}
