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
    
    // Use SSH_ASKPASS with a compiled helper for reliability
    QDir appDir = QCoreApplication::applicationDirPath();
    QString askPassExe = appDir.filePath("askpass_helper.exe");
    QString passFilePath = appDir.filePath("ssh_pass.txt");
    
    // Compile the helper if it doesn't exist
    if (!QFile::exists(askPassExe)) {
        // Try to find g++ in the known path or system path
        QString compilerPath = "g++";
        // Check the workspace specific path first
        QString workspaceCompiler = "C:/OpenIPC-Dashboard-Cpp/6.4.2/mingw_64/bin/g++.exe";
        if (QFile::exists(workspaceCompiler)) {
            compilerPath = workspaceCompiler;
        }
        
        // Source file is expected to be in the same directory as the executable or we can write it temporarily
        // But we created it in src/backend/askpass_helper.cpp. 
        // Since we are running from build directory, we might need to locate it.
        // For simplicity, let's write the source here to a temp file.
        QString sourcePath = appDir.filePath("askpass_helper.cpp");
        QFile sourceFile(sourcePath);
        if (sourceFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            sourceFile.write("#include <iostream>\n");
            sourceFile.write("#include <fstream>\n");
            sourceFile.write("#include <string>\n");
            sourceFile.write("#include <cstdlib>\n");
            sourceFile.write("int main() {\n");
            sourceFile.write("    const char* passFileEnv = std::getenv(\"SSH_PASS_FILE\");\n");
            sourceFile.write("    if (!passFileEnv) return 1;\n");
            sourceFile.write("    std::ifstream f(passFileEnv, std::ios::binary);\n");
            sourceFile.write("    if (f) {\n");
            sourceFile.write("        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());\n");
            sourceFile.write("        std::cout << content;\n");
            sourceFile.write("    }\n");
            sourceFile.write("    return 0;\n");
            sourceFile.write("}\n");
            sourceFile.close();
            
            QProcess compile;
            compile.start(compilerPath, QStringList() << sourcePath << "-o" << askPassExe);
            compile.waitForFinished();
        }
    }

    // 1. Write password to a file (exact bytes, no newlines)
    QFile passFile(passFilePath);
    if (passFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        passFile.write(password.toUtf8());
        passFile.close();
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("SSH_ASKPASS", QDir::toNativeSeparators(askPassExe));
    env.insert("SSH_ASKPASS_REQUIRE", "force");
    env.insert("SSH_PASS_FILE", QDir::toNativeSeparators(passFilePath));
    
    // Force DISPLAY to dummy value to trigger ASKPASS if needed
    if (!env.contains("DISPLAY")) {
        env.insert("DISPLAY", "dummy:0");
    }
    m_process->setProcessEnvironment(env);

    QStringList args;
    // -tt forces pseudo-tty allocation
    // -o StrictHostKeyChecking=no avoids "yes/no" prompt for new hosts
    args << "-tt" << "-o" << "StrictHostKeyChecking=no" << "-o" << "UserKnownHostsFile=/dev/null" << (user + "@" + ip);
    
    qDebug() << "Starting SSH:" << "ssh" << args;
    m_process->start("ssh", args);
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
    m_receiveBuffer.append(data);
    QString text = QString::fromUtf8(data);
    
    // No manual password handling needed with ASKPASS
    
    emit dataReceived(text);
}

void SshClient::onReadyReadStandardError()
{
    QByteArray data = m_process->readAllStandardError();
    m_receiveBuffer.append(data);
    QString text = QString::fromUtf8(data);
    
    // No manual password handling needed with ASKPASS
    
    emit dataReceived(text);
}

void SshClient::onStateChanged(QProcess::ProcessState newState)
{
    if (newState == QProcess::Running) {
        m_isConnected = true;
        emit connectedChanged();
    } else if (newState == QProcess::NotRunning) {
        m_isConnected = false;
        emit connectedChanged();
        emit errorOccurred("Session ended");
    }
}
