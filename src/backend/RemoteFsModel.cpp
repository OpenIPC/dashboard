#include "RemoteFsModel.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>

RemoteFsModel::RemoteFsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_isLoading(false)
    , m_currentPath("/root") // Default start
{
}

int RemoteFsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_entries.count();
}

QVariant RemoteFsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.count())
        return QVariant();

    const RemoteFileEntry &entry = m_entries[index.row()];
    
    switch (role) {
    case NameRole: return entry.name;
    case SizeRole: return (qlonglong)entry.size; // Cast to avoid QML issues with uint64
    case IsDirRole: return entry.isDir;
    case DateRole: return entry.dateStr;
    case PermissionsRole: return entry.permissions;
    default: return QVariant();
    }
}

QHash<int, QByteArray> RemoteFsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "fileName";
    roles[SizeRole] = "fileSize";
    roles[IsDirRole] = "isDir";
    roles[DateRole] = "fileDate";
    roles[PermissionsRole] = "filePermissions";
    return roles;
}

QString RemoteFsModel::currentPath() const
{
    return m_currentPath;
}

bool RemoteFsModel::isLoading() const
{
    return m_isLoading;
}

void RemoteFsModel::connectAndList(const QString &ip, const QString &user, const QString &password)
{
    m_ip = ip;
    m_user = user;
    m_password = password;
    // Default to /root or /
    m_currentPath = "/"; 
    emit currentPathChanged();
    refresh();
}

void RemoteFsModel::refresh()
{
    if (m_ip.isEmpty()) return;
    listDirectory(m_currentPath);
}

void RemoteFsModel::navigateUp()
{
    if (m_currentPath == "/") return;
    
    QString newPath = m_currentPath;
    if (newPath.endsWith("/")) newPath.chop(1);
    
    int lastSlash = newPath.lastIndexOf('/');
    if (lastSlash != -1) {
        newPath = newPath.left(lastSlash);
    }
    if (newPath.isEmpty()) newPath = "/";
    
    m_currentPath = newPath;
    emit currentPathChanged();
    refresh();
}

void RemoteFsModel::navigate(const QString &folderName)
{
    QString newPath = m_currentPath;
    if (!newPath.endsWith("/")) newPath += "/";
    newPath += folderName;
    
    m_currentPath = newPath;
    emit currentPathChanged();
    refresh();
}

void RemoteFsModel::deleteItem(const QString &fileName)
{
    if (fileName.isEmpty() || fileName == "." || fileName == "..") return;
    
    QString fullPath = m_currentPath;
    if (!fullPath.endsWith("/")) fullPath += "/";
    fullPath += fileName;
    
    QString cmd = QString("rm -rf \"%1\"").arg(fullPath);
    runSshCommand(cmd, [this](const QString &out, const QString &err) {
        if (!err.isEmpty()) {
            emit errorOccurred("Deletion error: " + err);
        } else {
            refresh();
        }
    });
}

void RemoteFsModel::downloadFile(const QString &fileName, const QString &localDestPath)
{
    // Need to use scp
    // scp -r user@ip:remotePath localDest
    
    QString remotePath = m_currentPath;
    if (!remotePath.endsWith("/")) remotePath += "/";
    remotePath += fileName;
    
    QString program = "scp";
#ifdef Q_OS_WIN
    if (QStandardPaths::findExecutable("scp").isEmpty()) {
        emit errorOccurred("SCP not found.");
        return;
    }
#endif
    
    QStringList args;
    args << "-r" << "-o" << "StrictHostKeyChecking=no" << "-o" << "UserKnownHostsFile=/dev/null";
    
    // Add -tt ? scp doesn't like -tt usually, but it needs ASKPASS
    args << (m_user + "@" + m_ip + ":" + remotePath);
    args << localDestPath;
    
    m_isLoading = true;
    emit isLoadingChanged();

    // Spawn process with ASKPASS
    QProcess *process = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
#ifdef Q_OS_WIN
    // ASKPASS Setup (Duplicated from SshClient)
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString askPassBat = QDir::toNativeSeparators(tempPath + "/ssh_askpass_fs.bat");
    QFile batchFile(askPassBat);
    if (batchFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&batchFile);
        out << "@echo off\n";
        out << "echo " << m_password << "\n"; 
        batchFile.close();
    }
    env.insert("SSH_ASKPASS", askPassBat);
    env.insert("SSH_ASKPASS_REQUIRE", "force");
#endif
    if (!env.contains("DISPLAY")) env.insert("DISPLAY", "dummy:0");
    
    process->setProcessEnvironment(env);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [this, process, fileName](int exitCode, QProcess::ExitStatus status) {
        m_isLoading = false;
        emit isLoadingChanged();
        
        if (exitCode == 0) {
            emit fileDownloaded(fileName);
        } else {
            emit errorOccurred("Download failed: " + QString::fromUtf8(process->readAllStandardError()));
        }
        process->deleteLater();
    });
    
    process->start(program, args);
}

void RemoteFsModel::uploadFile(const QString &localPath)
{
    // Implementation for upload would be similar to download but args swapped
    // scp -r localPath user@ip:currentPath
    
    QString program = "scp";
#ifdef Q_OS_WIN
    if (QStandardPaths::findExecutable("scp").isEmpty()) {
        emit errorOccurred("SCP not found.");
        return;
    }
#endif

    QStringList args;
    args << "-r" << "-o" << "StrictHostKeyChecking=no" << "-o" << "UserKnownHostsFile=/dev/null";
    args << localPath << (m_user + "@" + m_ip + ":" + m_currentPath);
    
    m_isLoading = true;
    emit isLoadingChanged();

    QProcess *process = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
#ifdef Q_OS_WIN
    // Reuse the bat file if it exists, or create new
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString askPassBat = QDir::toNativeSeparators(tempPath + "/ssh_askpass_fs.bat");
    // Ensure it exists (re-write just in case)
    QFile batchFile(askPassBat);
    if (batchFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&batchFile);
        out << "@echo off\n";
        out << "echo " << m_password << "\n"; 
        batchFile.close();
    }
    env.insert("SSH_ASKPASS", askPassBat);
    env.insert("SSH_ASKPASS_REQUIRE", "force");
#endif
    if (!env.contains("DISPLAY")) env.insert("DISPLAY", "dummy:0");
    
    process->setProcessEnvironment(env);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [this, process](int exitCode, QProcess::ExitStatus status) {
        m_isLoading = false;
        emit isLoadingChanged();
        
        if (exitCode == 0) {
            emit fileUploaded(QFileInfo(process->arguments().at(process->arguments().indexOf("-r")+4)).fileName()); // Hacky path retrieval for signal
            refresh();
        } else {
            emit errorOccurred("Upload failed: " + QString::fromUtf8(process->readAllStandardError()));
        }
        process->deleteLater();
    });
    
    process->start(program, args);
}

void RemoteFsModel::runSshCommand(const QString &cmd, std::function<void(const QString &, const QString &)> callback)
{
    QString program = "ssh";
    QStringList args;
    // No -tt for file operations usually, to keep output clean, unless we really need it for auth? 
    // With ASKPASS we shouldn't need -tt for non-interactive commands.
    args << "-o" << "StrictHostKeyChecking=no" << "-o" << "UserKnownHostsFile=/dev/null";
    args << (m_user + "@" + m_ip);
    args << cmd;

    m_isLoading = true;
    emit isLoadingChanged();

    QProcess *process = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
#ifdef Q_OS_WIN
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString askPassBat = QDir::toNativeSeparators(tempPath + "/ssh_askpass_fs.bat");
    QFile batchFile(askPassBat);
    if (batchFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&batchFile);
        out << "@echo off\n";
        out << "echo " << m_password << "\n";
        batchFile.close();
    }
    env.insert("SSH_ASKPASS", askPassBat);
    env.insert("SSH_ASKPASS_REQUIRE", "force");
#endif
    if (!env.contains("DISPLAY")) env.insert("DISPLAY", "dummy:0");
    
    process->setProcessEnvironment(env);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [this, process, callback](int exitCode, QProcess::ExitStatus status) {
        m_isLoading = false;
        emit isLoadingChanged();
        
        QString output = QString::fromUtf8(process->readAllStandardOutput());
        QString error = QString::fromUtf8(process->readAllStandardError());
        
        callback(output, error);
        process->deleteLater();
    });
    
    process->start(program, args);
}

void RemoteFsModel::renameItem(const QString &oldName, const QString &newName)
{
    if (oldName.isEmpty() || newName.isEmpty()) return;
    
    QString fullOld = m_currentPath;
    if (!fullOld.endsWith("/")) fullOld += "/";
    fullOld += oldName;
    
    QString fullNew = m_currentPath;
    if (!fullNew.endsWith("/")) fullNew += "/";
    fullNew += newName;
    
    // mv "old" "new"
    QString cmd = QString("mv \"%1\" \"%2\"").arg(fullOld, fullNew);
    runSshCommand(cmd, [this](const QString &out, const QString &err) {
        if (!err.isEmpty()) {
            emit errorOccurred("Rename error: " + err);
        } else {
            refresh();
        }
    });
}

void RemoteFsModel::createDirectory(const QString &dirName)
{
    if (dirName.isEmpty()) return;
    
    QString fullPath = m_currentPath;
    if (!fullPath.endsWith("/")) fullPath += "/";
    fullPath += dirName;
    
    QString cmd = QString("mkdir -p \"%1\"").arg(fullPath);
    runSshCommand(cmd, [this](const QString &out, const QString &err) {
        if (!err.isEmpty()) {
            emit errorOccurred("Create dir error: " + err);
        } else {
            refresh();
        }
    });
}

void RemoteFsModel::listDirectory(const QString &path)
{
    // busybox ls -l
    // Try to get easy parsing if possible, but standard `ls -l` is safest assumption
    QString cmd = QString("ls -l \"%1\"").arg(path);
    
    runSshCommand(cmd, [this](const QString &out, const QString &err) {
        if (!out.isEmpty()) {
            parseLsOutput(out);
        } else if (!err.isEmpty()) {
            emit errorOccurred("List error: " + err);
        } else {
            // Empty dir
            beginResetModel();
            m_entries.clear();
            endResetModel();
        }
    });
}

void RemoteFsModel::parseLsOutput(const QString &output)
{
    beginResetModel();
    m_entries.clear();
    
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    // Regex for: drwxr-xr-x    2 root     root           220 Jan  1 00:00 filename
    // 1: permissions
    // ...
    // size (number before date)
    // date (Month Day Time/Year)
    // filename (rest)
    
    // Simplified regex
    // ^([d-])[rwx-]{9}\s+\d+\s+\w+\s+\w+\s+(\d+)\s+(.{12})\s+(.+)$
    
    // Note: Busybox ls output spacing varies.
    // Let's split by whitespace, but filename can have spaces.
    // We assume standard column counts.
    // 1: perms
    // 2: links
    // 3: owner
    // 4: group
    // 5: size
    // 6,7,8: date parts (Jan 01 12:34)
    // 9+: name
    
    for (const QString &line : lines) {
        if (line.startsWith("total")) continue;
        
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;
        
        QStringList parts = trimmed.split(QRegularExpression("\\s+"));
        if (parts.count() < 9) continue; // Malformed or unexpected
        
        QString perms = parts[0];
        bool isDir = perms.startsWith('d');
        
        QString sizeStr = parts[4];
        quint64 size = sizeStr.toULongLong();
        
        QString dateStr = parts[5] + " " + parts[6] + " " + parts[7];
        
        // Name is everything from index 8 onwards
        // Reconstruct name (handles spaces)
        // Find index of 8th space?
        // Better:
        int nameStart = 0;
        int spaceCount = 0;
        for (int i=0; i<line.length(); i++) {
            if (line[i].isSpace()) {
                if (i > 0 && !line[i-1].isSpace()) spaceCount++;
            }
            if (spaceCount == 8) {
                // Find next non-space
                int j = i;
                while (j < line.length() && line[j].isSpace()) j++;
                nameStart = j;
                break;
            }
        }
        
        QString name;
        if (nameStart > 0 && nameStart < line.length()) {
            name = line.mid(nameStart);
        } else {
            // Fallback
             name = parts.mid(8).join(" ");
        }

        if (name == "." || name == "..") continue;

        RemoteFileEntry entry;
        entry.name = name;
        entry.isDir = isDir;
        entry.size = size;
        entry.permissions = perms;
        entry.dateStr = dateStr;
        
        m_entries.append(entry);
    }
    
    // Sort: Dirs first, then name
    std::sort(m_entries.begin(), m_entries.end(), [](const RemoteFileEntry &a, const RemoteFileEntry &b) {
        if (a.isDir != b.isDir) return a.isDir > b.isDir;
        return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
    });
    
    endResetModel();
}

void RemoteFsModel::localDeleteItem(const QString &path)
{
    QString cleanPath = path;
    if (cleanPath.startsWith("file:///")) cleanPath.replace("file:///", "");
    else if (cleanPath.startsWith("file://")) cleanPath.replace("file://", "");
    
    QFileInfo fi(cleanPath);
    if (fi.isDir()) {
        QDir dir(cleanPath);
        dir.removeRecursively();
    } else {
        QFile::remove(cleanPath);
    }
}

void RemoteFsModel::localRenameItem(const QString &oldPath, const QString &newName)
{
    QString cleanOld = oldPath;
    if (cleanOld.startsWith("file:///")) cleanOld.replace("file:///", "");
    else if (cleanOld.startsWith("file://")) cleanOld.replace("file://", "");
    
    QFileInfo fi(cleanOld);
    QDir dir = fi.absoluteDir();
    QString newPath = dir.absoluteFilePath(newName);
    
    QFile::rename(cleanOld, newPath);
}

void RemoteFsModel::localCreateDirectory(const QString &parentPath, const QString &dirName)
{
    QString cleanParent = parentPath;
    if (cleanParent.startsWith("file:///")) cleanParent.replace("file:///", "");
    else if (cleanParent.startsWith("file://")) cleanParent.replace("file://", "");
    
    QDir dir(cleanParent);
    dir.mkdir(dirName);
}

