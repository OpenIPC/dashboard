#include "ArchiveController.h"
#include "PathUtils.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QPointer>
#include <QProcess>
#include <QCoreApplication>
#include <QSet>

#include <algorithm>
#include <utility>

namespace {

QString recordingsRootPath(const QString &recordingsPath)
{
    QString rootPath = PathUtils::localPathFromUserInput(recordingsPath);
    if (rootPath.isEmpty()) {
        rootPath = RecordingFileCatalog::defaultRecordingRoot();
    }
    return QDir::cleanPath(rootPath);
}

QStringList supportedRecordingFilters()
{
    return {QStringLiteral("*.mp4"),
            QStringLiteral("*.mkv"),
            QStringLiteral("*.avi"),
            QStringLiteral("*.mov")};
}

QList<RecordedFile> collectRecordings(const QString &rootPath, const QString &cameraIp = QString())
{
    QList<RecordedFile> files;
    QDir dir(rootPath);
    if (!dir.exists()) {
        return files;
    }

    QDirIterator it(rootPath, supportedRecordingFilters(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QFileInfo fi(it.next());
        const auto file = RecordingFileCatalog::inspectFile(fi, cameraIp);
        if (file.has_value()) {
            files.append(*file);
        }
    }

    return files;
}

QString formatBytes(qint64 bytes)
{
    double value = static_cast<double>(bytes);
    QStringList units{QStringLiteral("B"), QStringLiteral("KB"), QStringLiteral("MB"), QStringLiteral("GB"), QStringLiteral("TB")};
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < units.size() - 1) {
        value /= 1024.0;
        ++unitIndex;
    }
    return unitIndex == 0
        ? QStringLiteral("%1 %2").arg(bytes).arg(units.at(unitIndex))
        : QStringLiteral("%1 %2").arg(value, 0, 'f', 1).arg(units.at(unitIndex));
}

bool cleanupRootIsSafe(const QString &rootPath)
{
    const QFileInfo rootInfo(rootPath);
    if (!rootInfo.exists() || !rootInfo.isDir() || !rootInfo.isAbsolute()) {
        return false;
    }

    const QString canonical = QDir(rootPath).canonicalPath();
    if (canonical.isEmpty()) {
        return false;
    }

    const QDir dir(canonical);
    if (dir.isRoot()) {
        return false;
    }

    QDir parent(canonical);
    if (!parent.cdUp() || parent.path() == canonical) {
        return false;
    }

    return true;
}

bool fileIsInsideRoot(const QString &rootPath, const QString &filePath)
{
    const QString rootCanonical = QDir(rootPath).canonicalPath();
    const QString fileCanonical = QFileInfo(filePath).canonicalFilePath();
    if (rootCanonical.isEmpty() || fileCanonical.isEmpty()) {
        return false;
    }

    const QString normalizedRoot = QDir::cleanPath(rootCanonical) + QLatin1Char('/');
    const QString normalizedFile = QDir::cleanPath(fileCanonical);
    return normalizedFile.startsWith(normalizedRoot);
}

QString ffmpegProgram()
{
#ifdef Q_OS_WIN
    const QString bundled = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("ffmpeg.exe"));
#else
    const QString bundled = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("ffmpeg"));
#endif
    return QFile::exists(bundled) ? bundled : QStringLiteral("ffmpeg");
}

void appendLimited(QByteArray &target, const QByteArray &data, int maxBytes = 8000)
{
    target.append(data);
    if (target.size() > maxBytes) {
        target = target.right(maxBytes);
    }
}

QList<QByteArray> takeCompleteLines(QByteArray &buffer)
{
    QList<QByteArray> lines;
    int newlineIndex = buffer.indexOf('\n');
    while (newlineIndex >= 0) {
        QByteArray line = buffer.left(newlineIndex).trimmed();
        buffer.remove(0, newlineIndex + 1);
        if (!line.isEmpty()) {
            lines.append(line);
        }
        newlineIndex = buffer.indexOf('\n');
    }
    return lines;
}

int progressFromFfmpegLine(const QByteArray &line, qint64 durationMs, int fallbackProgress)
{
    if (line == QByteArrayLiteral("progress=end")) {
        return 100;
    }

    if (!line.startsWith(QByteArrayLiteral("out_time_ms="))
        && !line.startsWith(QByteArrayLiteral("out_time_us="))) {
        return fallbackProgress;
    }

    const QList<QByteArray> parts = line.split('=');
    if (parts.size() != 2 || durationMs <= 0) {
        return fallbackProgress;
    }

    bool ok = false;
    qint64 raw = parts.at(1).toLongLong(&ok);
    if (!ok || raw <= 0) {
        return fallbackProgress;
    }

    const qint64 elapsedMs = raw > durationMs * 10 ? raw / 1000 : raw;
    return std::clamp(static_cast<int>((elapsedMs * 100) / durationMs), 0, 99);
}

} // namespace

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
        const QString rootPath = recordingsRootPath(recordingsPath);

        QDir dir(rootPath);
        if (!dir.exists()) {
            qWarning() << "Recordings path does not exist:" << rootPath;
        } else {
            QDirIterator it(rootPath, supportedRecordingFilters(), QDir::Files, QDirIterator::Subdirectories);
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
    if (m_isExporting) {
        const QString errorText = QStringLiteral("Export is already running.");
        m_exportErrorText = errorText;
        m_exportStatus = QStringLiteral("Экспорт уже выполняется");
        emit exportStateChanged();
        emit exportError(errorText);
        return;
    }

    const QString sourcePath = PathUtils::localPathFromUserInput(inputFile);
    const QString targetPath = PathUtils::localPathFromUserInput(outputFile);
    const qint64 durationMs = endMs - startMs;

    auto rejectExport = [this](const QString &errorText) {
        m_isExporting = false;
        m_exportProgress = 0;
        m_exportStatus = QStringLiteral("Ошибка экспорта");
        m_exportErrorText = errorText;
        emit exportStateChanged();
        emit exportError(errorText);
    };

    if (sourcePath.isEmpty() || !QFileInfo::exists(sourcePath)) {
        rejectExport(QStringLiteral("Source recording was not found."));
        return;
    }
    if (targetPath.isEmpty()) {
        rejectExport(QStringLiteral("Export destination is empty."));
        return;
    }
    if (durationMs <= 0) {
        rejectExport(QStringLiteral("Export range is empty."));
        return;
    }

    const QFileInfo targetInfo(targetPath);
    if (!targetInfo.absoluteDir().exists() && !targetInfo.absoluteDir().mkpath(QStringLiteral("."))) {
        rejectExport(QStringLiteral("Failed to create export folder."));
        return;
    }

    const quint64 requestId = ++m_exportRequestId;
    m_isExporting = true;
    m_exportProgress = 0;
    m_exportStatus = QStringLiteral("Экспорт запущен");
    m_exportErrorText.clear();
    m_exportOutputFile = targetPath;
    emit exportStateChanged();
    emit exportStarted(targetPath);
    emit exportProgress(0);

    QPointer<ArchiveController> controller(this);
    (void)QtConcurrent::run([controller, requestId, sourcePath, targetPath, startMs, durationMs]() {
        QString errorText;
        QByteArray stderrTail;
        int lastProgress = 0;

        auto publishProgress = [controller, requestId, &lastProgress](int progress) {
            progress = std::clamp(progress, 0, 100);
            if (progress <= lastProgress && progress != 100) {
                return;
            }
            if (!controller) {
                return;
            }
            lastProgress = progress;
            QMetaObject::invokeMethod(controller.data(), [controller, requestId, progress]() {
                if (!controller || controller->m_exportRequestId != requestId || !controller->m_isExporting) {
                    return;
                }
                controller->m_exportProgress = progress;
                controller->m_exportStatus = progress >= 100
                    ? QStringLiteral("Финализация экспорта")
                    : QStringLiteral("Экспорт %1%").arg(progress);
                emit controller->exportProgress(progress);
                emit controller->exportStateChanged();
            }, Qt::QueuedConnection);
        };

        QProcess process;
        process.setProgram(ffmpegProgram());

        const QString startStr = QString::number(startMs / 1000.0, 'f', 3);
        const QString durationStr = QString::number(durationMs / 1000.0, 'f', 3);
        
        QStringList args;
        args << QStringLiteral("-hide_banner")
             << QStringLiteral("-y")
             << QStringLiteral("-ss") << startStr
             << QStringLiteral("-i") << sourcePath
             << QStringLiteral("-t") << durationStr
             << QStringLiteral("-c") << QStringLiteral("copy")
             << QStringLiteral("-progress") << QStringLiteral("pipe:1")
             << QStringLiteral("-nostats")
             << targetPath;
        
        process.setArguments(args);
        process.start();
        
        if (!process.waitForStarted()) {
            errorText = QStringLiteral("Failed to start ffmpeg. Ensure ffmpeg.exe is in the application folder.");
        } else {
            QByteArray stdoutBuffer;
            while (!process.waitForFinished(250)) {
                appendLimited(stderrTail, process.readAllStandardError());
                stdoutBuffer.append(process.readAllStandardOutput());
                const QList<QByteArray> lines = takeCompleteLines(stdoutBuffer);
                for (const QByteArray &line : lines) {
                    publishProgress(progressFromFfmpegLine(line, durationMs, lastProgress));
                }
            }

            appendLimited(stderrTail, process.readAllStandardError());
            stdoutBuffer.append(process.readAllStandardOutput());
            const QList<QByteArray> lines = takeCompleteLines(stdoutBuffer);
            for (const QByteArray &line : lines) {
                publishProgress(progressFromFfmpegLine(line, durationMs, lastProgress));
            }

            if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
                errorText = QStringLiteral("ffmpeg returned error: ") + QString::fromUtf8(stderrTail).trimmed();
            }
        }

        if (!controller) {
            return;
        }

        QMetaObject::invokeMethod(controller, [controller, requestId, errorText, targetPath]() {
            if (!controller || controller->m_exportRequestId != requestId) {
                return;
            }

            if (errorText.isEmpty()) {
                controller->m_isExporting = false;
                controller->m_exportProgress = 100;
                controller->m_exportStatus = QStringLiteral("Экспорт завершен");
                controller->m_exportErrorText.clear();
                controller->m_exportOutputFile = targetPath;
                emit controller->exportProgress(100);
                emit controller->exportStateChanged();
                emit controller->exportFinished();
            } else {
                controller->m_isExporting = false;
                controller->m_exportStatus = QStringLiteral("Ошибка экспорта");
                controller->m_exportErrorText = errorText;
                emit controller->exportStateChanged();
                emit controller->exportError(errorText);
            }
        }, Qt::QueuedConnection);
    });
}

void ArchiveController::clearExportStatus()
{
    if (m_isExporting) {
        return;
    }

    m_exportProgress = 0;
    m_exportStatus.clear();
    m_exportErrorText.clear();
    m_exportOutputFile.clear();
    emit exportStateChanged();
}

QVariantMap ArchiveController::storageSummary(const QString &recordingsPath) const
{
    const QString rootPath = recordingsRootPath(recordingsPath);
    QVariantMap result;
    result.insert(QStringLiteral("rootPath"), rootPath);
    result.insert(QStringLiteral("exists"), QDir(rootPath).exists());
    result.insert(QStringLiteral("safe"), cleanupRootIsSafe(rootPath));

    qint64 totalBytes = 0;
    int manualCount = 0;
    int eventCount = 0;
    QDateTime oldest;
    QDateTime newest;

    const QList<RecordedFile> files = collectRecordings(rootPath);
    for (const RecordedFile &file : files) {
        totalBytes += file.sizeBytes;
        if (file.source == QStringLiteral("event")) {
            ++eventCount;
        } else {
            ++manualCount;
        }

        if (!oldest.isValid() || file.startTime < oldest) {
            oldest = file.startTime;
        }
        if (!newest.isValid() || file.startTime > newest) {
            newest = file.startTime;
        }
    }

    result.insert(QStringLiteral("fileCount"), files.size());
    result.insert(QStringLiteral("manualCount"), manualCount);
    result.insert(QStringLiteral("eventCount"), eventCount);
    result.insert(QStringLiteral("totalBytes"), totalBytes);
    result.insert(QStringLiteral("totalSizeText"), formatBytes(totalBytes));
    result.insert(QStringLiteral("oldest"), oldest);
    result.insert(QStringLiteral("newest"), newest);
    return result;
}

QVariantMap ArchiveController::cleanupRecordings(const QString &recordingsPath,
                                                 int keepDays,
                                                 qint64 maxBytes,
                                                 bool dryRun)
{
    const QString rootPath = recordingsRootPath(recordingsPath);
    QVariantMap result;
    result.insert(QStringLiteral("rootPath"), rootPath);
    result.insert(QStringLiteral("dryRun"), dryRun);
    result.insert(QStringLiteral("safe"), cleanupRootIsSafe(rootPath));

    if (!result.value(QStringLiteral("safe")).toBool()) {
        result.insert(QStringLiteral("error"), QStringLiteral("Recording folder is not safe for cleanup."));
        emit cleanupFinished(result);
        return result;
    }

    QList<RecordedFile> files = collectRecordings(rootPath);
    std::sort(files.begin(), files.end(), [](const RecordedFile &a, const RecordedFile &b) {
        if (a.startTime == b.startTime) {
            return a.filePath < b.filePath;
        }
        return a.startTime < b.startTime;
    });

    qint64 totalBytes = 0;
    for (const RecordedFile &file : std::as_const(files)) {
        totalBytes += file.sizeBytes;
    }

    QSet<QString> selectedPaths;
    const QDateTime cutoff = keepDays > 0 ? QDateTime::currentDateTime().addDays(-keepDays) : QDateTime();
    if (cutoff.isValid()) {
        for (const RecordedFile &file : std::as_const(files)) {
            if (file.startTime.isValid() && file.startTime < cutoff) {
                selectedPaths.insert(file.filePath);
            }
        }
    }

    qint64 remainingBytes = totalBytes;
    for (const RecordedFile &file : std::as_const(files)) {
        if (selectedPaths.contains(file.filePath)) {
            remainingBytes -= file.sizeBytes;
        }
    }

    if (maxBytes > 0 && remainingBytes > maxBytes) {
        for (const RecordedFile &file : std::as_const(files)) {
            if (selectedPaths.contains(file.filePath)) {
                continue;
            }
            selectedPaths.insert(file.filePath);
            remainingBytes -= file.sizeBytes;
            if (remainingBytes <= maxBytes) {
                break;
            }
        }
    }

    int wouldDeleteCount = 0;
    qint64 wouldDeleteBytes = 0;
    int deletedCount = 0;
    qint64 deletedBytes = 0;
    QStringList errors;

    for (const RecordedFile &file : std::as_const(files)) {
        if (!selectedPaths.contains(file.filePath)) {
            continue;
        }

        ++wouldDeleteCount;
        wouldDeleteBytes += file.sizeBytes;

        if (dryRun) {
            continue;
        }

        if (!fileIsInsideRoot(rootPath, file.filePath)) {
            errors.append(QStringLiteral("Skipped unsafe path: %1").arg(file.filePath));
            continue;
        }

        if (QFile::remove(file.filePath)) {
            ++deletedCount;
            deletedBytes += file.sizeBytes;
        } else {
            errors.append(QStringLiteral("Failed to delete: %1").arg(file.filePath));
        }
    }

    result.insert(QStringLiteral("fileCount"), files.size());
    result.insert(QStringLiteral("totalBytes"), totalBytes);
    result.insert(QStringLiteral("totalSizeText"), formatBytes(totalBytes));
    result.insert(QStringLiteral("wouldDeleteCount"), wouldDeleteCount);
    result.insert(QStringLiteral("wouldDeleteBytes"), wouldDeleteBytes);
    result.insert(QStringLiteral("wouldDeleteSizeText"), formatBytes(wouldDeleteBytes));
    result.insert(QStringLiteral("deletedCount"), deletedCount);
    result.insert(QStringLiteral("deletedBytes"), deletedBytes);
    result.insert(QStringLiteral("deletedSizeText"), formatBytes(deletedBytes));
    result.insert(QStringLiteral("remainingBytes"), dryRun ? totalBytes - wouldDeleteBytes : totalBytes - deletedBytes);
    result.insert(QStringLiteral("errors"), errors);
    emit cleanupFinished(result);
    return result;
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


