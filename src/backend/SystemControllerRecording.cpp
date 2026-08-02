#include "SystemController.h"
#include "PathUtils.h"
#include "RecordingFileCatalog.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>

#include <algorithm>

QVariantList SystemController::getRecordings(const QString &cameraIp, const QDate &date)
{
    QVariantList results;
    QString path = PathUtils::localPathFromUserInput(m_appSettings.value("recordingsPath").toString());
    if (path.isEmpty()) {
        path = RecordingFileCatalog::defaultRecordingRoot();
    }

    QDir dir(path);
    if (!dir.exists()) return results;

    const QStringList filters{QStringLiteral("*.mp4"),
                              QStringLiteral("*.mkv"),
                              QStringLiteral("*.avi"),
                              QStringLiteral("*.mov")};
    QDirIterator it(path, filters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QFileInfo fileInfo(it.next());
        const auto file = RecordingFileCatalog::inspectFile(fileInfo, cameraIp);
        if (!file.has_value() || file->startTime.date() != date) {
            continue;
        }

        QVariantMap rec = RecordingFileCatalog::toVariantMap(*file);
        rec.insert(QStringLiteral("created"), fileInfo.birthTime());
        rec.insert(QStringLiteral("modified"), fileInfo.lastModified());
        rec.insert(QStringLiteral("localPath"), file->filePath);
        rec.insert(QStringLiteral("filePath"), file->fileUrl);
        results.append(rec);
    }

    std::sort(results.begin(), results.end(), [](const QVariant &a, const QVariant &b) {
        return a.toMap().value(QStringLiteral("startTime")).toDateTime()
            < b.toMap().value(QStringLiteral("startTime")).toDateTime();
    });

    return results;
}

QList<int> SystemController::getRecordingDates(const QString &cameraIp, int year, int month)
{
    QList<int> days;
    QString path = PathUtils::localPathFromUserInput(m_appSettings.value("recordingsPath").toString());
    if (path.isEmpty()) {
        path = RecordingFileCatalog::defaultRecordingRoot();
    }

    QDir dir(path);
    if (!dir.exists()) return days;

    const QStringList filters{QStringLiteral("*.mp4"),
                              QStringLiteral("*.mkv"),
                              QStringLiteral("*.avi"),
                              QStringLiteral("*.mov")};
    QDirIterator it(path, filters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QFileInfo fileInfo(it.next());
        const auto file = RecordingFileCatalog::inspectFile(fileInfo, cameraIp);
        if (!file.has_value()) {
            continue;
        }

        const QDate date = file->startTime.date();
        if (date.isValid() && date.year() == year && date.month() == month && !days.contains(date.day())) {
            days.append(date.day());
        }
    }

    std::sort(days.begin(), days.end());
    return days;
}

QString SystemController::generateRecordingPath(const QString &ip)
{
    QString path = PathUtils::localPathFromUserInput(m_appSettings.value("recordingsPath").toString());
    if (path.isEmpty()) {
        path = RecordingFileCatalog::defaultRecordingRoot();
    }

    return RecordingFileCatalog::buildRecordingPath(path, ip, QDateTime::currentDateTime(), QStringLiteral("manual"));
}

void SystemController::toggleRecording(int gridIndex)
{
    Q_UNUSED(gridIndex)
    // Legacy method kept for ABI compatibility if needed, but implementation removed
    // Logic moved to client-side (QML + player) to avoid ffmpeg dependency
    qWarning() << "SystemController::toggleRecording is deprecated. Use the client-side player recording API instead.";
}

void SystemController::notifyRecordingStarted(const QString &cameraIp, const QString &path, const QString &source)
{
    addLog(QtInfoMsg,
           QStringLiteral("Recording started [%1]: %2 -> %3")
               .arg(source.trimmed().isEmpty() ? QStringLiteral("manual") : source.trimmed(),
                    cameraIp.trimmed().isEmpty() ? QStringLiteral("unknown camera") : cameraIp.trimmed(),
                    PathUtils::localPathFromUserInput(path)));
    m_incidentManager->ingestRecordingEvent(
        QStringLiteral("started"), cameraIp, PathUtils::localPathFromUserInput(path),
        QStringLiteral("Recording started"),
        {{QStringLiteral("owner"), source.trimmed().isEmpty() ? QStringLiteral("manual")
                                                               : source.trimmed()}});
}

void SystemController::notifyRecordingStopped(const QString &cameraIp, const QString &path, const QString &source)
{
    addLog(QtInfoMsg,
           QStringLiteral("Recording stopped [%1]: %2 -> %3")
               .arg(source.trimmed().isEmpty() ? QStringLiteral("manual") : source.trimmed(),
                    cameraIp.trimmed().isEmpty() ? QStringLiteral("unknown camera") : cameraIp.trimmed(),
                    PathUtils::localPathFromUserInput(path)));
    m_incidentManager->ingestRecordingEvent(
        QStringLiteral("stopped"), cameraIp, PathUtils::localPathFromUserInput(path),
        QStringLiteral("Recording stopped"),
        {{QStringLiteral("owner"), source.trimmed().isEmpty() ? QStringLiteral("manual")
                                                               : source.trimmed()}});
}

void SystemController::notifyRecordingSegment(const QString &cameraIp, const QString &oldPath, const QString &newPath)
{
    addLog(QtInfoMsg,
           QStringLiteral("Recording segment rotated: %1, %2 -> %3")
               .arg(cameraIp.trimmed().isEmpty() ? QStringLiteral("unknown camera") : cameraIp.trimmed(),
                    PathUtils::localPathFromUserInput(oldPath),
                    PathUtils::localPathFromUserInput(newPath)));
    m_incidentManager->ingestRecordingEvent(
        QStringLiteral("segment-rotated"), cameraIp, PathUtils::localPathFromUserInput(newPath),
        QStringLiteral("Recording segment rotated"),
        {{QStringLiteral("previousPath"), PathUtils::localPathFromUserInput(oldPath)}});
}

void SystemController::notifyRecordingError(const QString &cameraIp, const QString &path, const QString &message)
{
    addLog(QtWarningMsg,
           QStringLiteral("Recording warning: %1, %2, %3")
               .arg(cameraIp.trimmed().isEmpty() ? QStringLiteral("unknown camera") : cameraIp.trimmed(),
                    PathUtils::localPathFromUserInput(path),
                    message.trimmed().isEmpty() ? QStringLiteral("unknown error") : message.trimmed()));
    m_incidentManager->ingestRecordingEvent(
        QStringLiteral("error"), cameraIp, PathUtils::localPathFromUserInput(path),
        message.trimmed().isEmpty() ? QStringLiteral("Unknown recording error") : message.trimmed());
}

void SystemController::exportRecording(const QString &inputFile, const QString &outputFile, int startMs, int endMs)
{
    QProcess *proc = new QProcess(this);

    // Convert ms to HH:MM:SS.mmm format
    auto formatTime = [](int ms) {
        int h = ms / 3600000;
        ms %= 3600000;
        int m = ms / 60000;
        ms %= 60000;
        int s = ms / 1000;
        ms %= 1000;
        return QString("%1:%2:%3.%4")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'))
            .arg(ms, 3, 10, QChar('0'));
    };

    QString startTime = formatTime(startMs);
    QString duration = formatTime(endMs - startMs);

    QStringList args;
    args << "-ss" << startTime
         << "-i" << inputFile
         << "-t" << duration
         << "-c" << "copy"
         << "-y" << outputFile;

    QString program = "ffmpeg";
#ifdef Q_OS_WIN
    if (QFile::exists(QCoreApplication::applicationDirPath() + "/ffmpeg.exe")) {
        program = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
    }
#endif

    qDebug() << "Exporting:" << program << args.join(" ");

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [proc](int exitCode, QProcess::ExitStatus) {
        qDebug() << "Export finished with code" << exitCode;
        proc->deleteLater();
    });

    proc->start(program, args);
}
