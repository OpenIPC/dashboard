#include "ArchiveController.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QCoreApplication>

ArchiveController::ArchiveController(QObject *parent) : QObject(parent)
{
}

ArchiveController::~ArchiveController()
{
    logout();
}

void ArchiveController::login(const QString &ip, int port, const QString &username, const QString &password)
{
    // Local archive search doesn't require SDK login
    Q_UNUSED(ip);
    Q_UNUSED(port);
    Q_UNUSED(username);
    Q_UNUSED(password);
    
    emit loginSuccess();
}

void ArchiveController::logout()
{
    // No SDK logout needed for local files
}

void ArchiveController::search(const QDateTime &startTime, const QDateTime &endTime, const QString &cameraIp, const QString &recordingsPath)
{
    if (m_isSearching) {
        qWarning() << "Already searching";
        return;
    }

    m_isSearching = true;
    emit isSearchingChanged();
    m_files.clear();
    emit searchResultsChanged();

    QtConcurrent::run([this, startTime, endTime, cameraIp, recordingsPath]() {
        int nFileCount = 0;
        
        QDir dir(recordingsPath);
        if (!dir.exists()) {
            qWarning() << "Recordings path does not exist:" << recordingsPath;
        } else {
            QString sanitizedIp = cameraIp;
            sanitizedIp.replace(".", "_");
            
            // Filter for .mp4 files
            QStringList filters;
            filters << "*.mp4";
            
            QDirIterator it(recordingsPath, filters, QDir::Files, QDirIterator::NoIteratorFlags);
            while (it.hasNext()) {
                QString filePath = it.next();
                QFileInfo fi(filePath);
                QString fileName = fi.fileName();
                
                // Check if file belongs to this camera
                if (!fileName.startsWith(sanitizedIp)) {
                    continue;
                }
                
                // Parse timestamp from filename: IP_yyyy-MM-dd_HH-mm-ss.mp4
                // Example: 192_168_1_10_2025-12-25_12-00-00.mp4
                
                if (fileName.length() < 24) continue; 
                
                // Extract date part: yyyy-MM-dd_HH-mm-ss (19 chars)
                // It is at the end, before .mp4 (4 chars)
                QString timeStr = fileName.right(23).left(19); 
                QDateTime fileTime = QDateTime::fromString(timeStr, "yyyy-MM-dd_HH-mm-ss");
                
                if (!fileTime.isValid()) {
                    continue;
                }
                
                if (fileTime >= startTime && fileTime <= endTime) {
                    RecordedFile rf;
                    rf.startTime = fileTime;
                    rf.endTime = fileTime.addSecs(fi.size() / 1024 / 1024 * 10); // Rough estimate or 0
                    rf.size = fi.size();
                    rf.fileName = fileName;
                    rf.filePath = filePath;
                    rf.type = 0;
                    
                    QMutexLocker locker(&m_mutex);
                    m_files.append(rf);
                    nFileCount++;
                }
            }
        }

        m_isSearching = false;
        emit isSearchingChanged();
        emit searchResultsChanged();
        emit searchFinished(nFileCount);
    });
}

void ArchiveController::download(int index, const QString &savePath)
{
    if (index < 0 || index >= m_files.size()) {
        emit downloadError(index, "Invalid index");
        return;
    }

    RecordedFile rf = m_files[index];
    
    QtConcurrent::run([this, index, rf, savePath]() {
        if (QFile::exists(savePath)) {
            QFile::remove(savePath);
        }
        
        if (QFile::copy(rf.filePath, savePath)) {
            emit downloadProgress(index, 100);
            emit downloadFinished(index);
        } else {
            emit downloadError(index, "Failed to copy file");
        }
    });
}



void ArchiveController::exportVideo(const QString &inputFile, const QString &outputFile, qint64 startMs, qint64 endMs)
{
    QtConcurrent::run([this, inputFile, outputFile, startMs, endMs]() {
        // Check for ffmpeg
        QString ffmpegPath = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
        if (!QFile::exists(ffmpegPath)) {
            // Fallback: try system path
            ffmpegPath = "ffmpeg";
        }
        
        // If we can't find ffmpeg easily, or if we want to be safe, we might just copy if range is full
        // But user wants trimming.
        
        QProcess process;
        process.setProgram(ffmpegPath);
        
        QString startStr = QString::number(startMs / 1000.0, 'f', 3);
        QString durationStr = QString::number((endMs - startMs) / 1000.0, 'f', 3);
        
        // ffmpeg -ss <start> -i <input> -t <duration> -c copy <output>
        QStringList args;
        args << "-y" << "-ss" << startStr << "-i" << inputFile << "-t" << durationStr << "-c" << "copy" << outputFile;
        
        process.setArguments(args);
        process.start();
        
        if (!process.waitForStarted()) {
             emit exportError("Failed to start ffmpeg. Ensure ffmpeg.exe is in the application folder.");
             return;
        }
        
        if (!process.waitForFinished(-1)) {
             emit exportError("ffmpeg process failed or timed out.");
             return;
        }
        
        if (process.exitCode() == 0) {
            emit exportFinished();
        } else {
            emit exportError("ffmpeg returned error: " + process.readAllStandardError());
        }
    });
}

QVariantList ArchiveController::searchResults() const
{
    QVariantList list;
    // QMutexLocker locker(&m_mutex); // Can't use locker in const method easily without mutable mutex
    // Assuming called from main thread after search finished
    
    for (const auto &f : m_files) {
        QVariantMap map;
        map["startTime"] = f.startTime;
        map["endTime"] = f.endTime;
        map["size"] = f.size;
        map["fileName"] = f.fileName;
        map["filePath"] = f.filePath;
        list.append(map);
    }
    return list;
}


