#include "ArchiveController.h"
#include "PathUtils.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMetaObject>
#include <QProcess>
#include <QCoreApplication>

#include <algorithm>

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
    // No SDK logout needed for local files, but invalidate pending async work.
    ++m_searchRequestId;
    if (m_isSearching) {
        m_isSearching = false;
        emit isSearchingChanged();
    }

    {
        QMutexLocker locker(&m_mutex);
        m_files.clear();
    }
    emit searchResultsChanged();
}

void ArchiveController::search(const QDateTime &startTime, const QDateTime &endTime, const QString &cameraIp, const QString &recordingsPath)
{
    const quint64 requestId = ++m_searchRequestId;
    if (!m_isSearching) {
        m_isSearching = true;
        emit isSearchingChanged();
    }

    {
        QMutexLocker locker(&m_mutex);
        m_files.clear();
    }
    emit searchResultsChanged();

    QPointer<ArchiveController> controller(this);
    (void)QtConcurrent::run([controller, requestId, startTime, endTime, cameraIp, recordingsPath]() {
        QList<RecordedFile> foundFiles;
        QString rootPath = PathUtils::localPathFromUserInput(recordingsPath);
        if (rootPath.isEmpty()) {
            rootPath = RecordingFileCatalog::defaultRecordingRoot();
        }
        rootPath = QDir::cleanPath(rootPath);

        QDir dir(rootPath);
        if (!dir.exists()) {
            qWarning() << "Recordings path does not exist:" << rootPath;
        } else {
            const QStringList filters{QStringLiteral("*.mp4"),
                                      QStringLiteral("*.mkv"),
                                      QStringLiteral("*.avi"),
                                      QStringLiteral("*.mov")};

            QDirIterator it(rootPath, filters, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                const QFileInfo fi(it.next());
                const auto file = RecordingFileCatalog::inspectFile(fi, cameraIp);
                if (!file.has_value()) {
                    continue;
                }

                const bool afterStart = !startTime.isValid() || file->startTime >= startTime;
                const bool beforeEnd = !endTime.isValid() || file->startTime <= endTime;
                if (afterStart && beforeEnd) {
                    foundFiles.append(*file);
                }
            }
        }

        std::sort(foundFiles.begin(), foundFiles.end(), [](const RecordedFile &a, const RecordedFile &b) {
            if (a.startTime == b.startTime) {
                return a.fileName < b.fileName;
            }
            return a.startTime > b.startTime;
        });

        const int fileCount = foundFiles.size();

        if (!controller) {
            return;
        }

        QMetaObject::invokeMethod(controller, [controller, requestId, foundFiles, fileCount]() {
            if (!controller || controller->m_searchRequestId != requestId) {
                return;
            }

            {
                QMutexLocker locker(&controller->m_mutex);
                controller->m_files = foundFiles;
            }

            controller->m_isSearching = false;
            emit controller->isSearchingChanged();
            emit controller->searchResultsChanged();
            emit controller->searchFinished(fileCount);
        }, Qt::QueuedConnection);
    });
}

void ArchiveController::download(int index, const QString &savePath)
{
    RecordedFile rf;
    {
        QMutexLocker locker(&m_mutex);
        if (index < 0 || index >= m_files.size()) {
            emit downloadError(index, "Invalid index");
            return;
        }
        rf = m_files[index];
    }

    QPointer<ArchiveController> controller(this);
    (void)QtConcurrent::run([controller, index, rf, savePath]() {
        const QString errorText = [savePath, rf]() -> QString {
            if (QFile::exists(savePath)) {
                QFile::remove(savePath);
            }

            if (QFile::copy(rf.filePath, savePath)) {
                return QString();
            }

            return QStringLiteral("Failed to copy file");
        }();

        if (!controller) {
            return;
        }

        QMetaObject::invokeMethod(controller, [controller, index, errorText]() {
            if (!controller) {
                return;
            }

            if (errorText.isEmpty()) {
                emit controller->downloadProgress(index, 100);
                emit controller->downloadFinished(index);
            } else {
                emit controller->downloadError(index, errorText);
            }
        }, Qt::QueuedConnection);
    });
}

void ArchiveController::exportVideo(const QString &inputFile, const QString &outputFile, qint64 startMs, qint64 endMs)
{
    QPointer<ArchiveController> controller(this);
    (void)QtConcurrent::run([controller, inputFile, outputFile, startMs, endMs]() {
        QString errorText;
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
            errorText = QStringLiteral("Failed to start ffmpeg. Ensure ffmpeg.exe is in the application folder.");
        } else if (!process.waitForFinished(-1)) {
            errorText = QStringLiteral("ffmpeg process failed or timed out.");
        } else if (process.exitCode() != 0) {
            errorText = QStringLiteral("ffmpeg returned error: ") + QString::fromUtf8(process.readAllStandardError());
        }

        if (!controller) {
            return;
        }

        QMetaObject::invokeMethod(controller, [controller, errorText]() {
            if (!controller) {
                return;
            }

            if (errorText.isEmpty()) {
                emit controller->exportFinished();
            } else {
                emit controller->exportError(errorText);
            }
        }, Qt::QueuedConnection);
    });
}

QVariantList ArchiveController::searchResults() const
{
    QVariantList list;
    QMutexLocker locker(&m_mutex);
    
    for (const auto &f : m_files) {
        list.append(RecordingFileCatalog::toVariantMap(f));
    }
    return list;
}


