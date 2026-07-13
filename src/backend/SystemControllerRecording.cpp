#include "SystemController.h"
#include "PathUtils.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <algorithm>

QVariantList SystemController::getRecordings(const QString &cameraIp, const QDate &date)
{
    QVariantList results;
    QString path = PathUtils::localPathFromUserInput(m_appSettings.value("recordingsPath").toString());
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/OpenIPC";
    }

    QDir dir(path);
    if (!dir.exists()) return results;

    QString dateStr1 = date.toString("yyyy-MM-dd");
    QString dateStr2 = date.toString("yyyyMMdd");

    // Recursive search for mp4 files
    QDirIterator it(path, QStringList() << "*.mp4" << "*.mkv" << "*.avi", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QString fileName = it.fileName();

        // Filter by date
        if (!fileName.contains(dateStr1) && !fileName.contains(dateStr2)) {
            continue;
        }

        // Filter by camera IP (if provided and not empty)
        // Note: IP in filename might be sanitized (e.g. 192.168.1.10 -> 192_168_1_10)
        if (!cameraIp.isEmpty()) {
            QString sanitizedIp = cameraIp;
            sanitizedIp.replace(".", "_");
            if (!fileName.contains(cameraIp) && !fileName.contains(sanitizedIp)) {
                // Try checking parent folder name too
                QString parent = QFileInfo(filePath).dir().dirName();
                if (!parent.contains(cameraIp) && !parent.contains(sanitizedIp)) {
                    continue;
                }
            }
        }

        QFileInfo fi(filePath);
        QVariantMap rec;
        rec["fileName"] = fileName;
        rec["filePath"] = "file:///" + filePath; // URL for QML
        rec["size"] = fi.size();
        rec["created"] = fi.birthTime();
        rec["modified"] = fi.lastModified();

        // Try to extract time from filename
        // Look for HH-mm-ss or HHmmss pattern
        // Simple heuristic: find 6 digits or XX-XX-XX
        // For now, just use modification time as a fallback for sorting

        results.append(rec);
    }

    // Sort by time (modification time)
    std::sort(results.begin(), results.end(), [](const QVariant &a, const QVariant &b) {
        return a.toMap()["modified"].toDateTime() < b.toMap()["modified"].toDateTime();
    });

    return results;
}

QList<int> SystemController::getRecordingDates(const QString &cameraIp, int year, int month)
{
    QList<int> days;
    QString path = PathUtils::localPathFromUserInput(m_appSettings.value("recordingsPath").toString());
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/OpenIPC";
    }

    QDir dir(path);
    if (!dir.exists()) return days;

    QString sanitizedIp = cameraIp;
    sanitizedIp.replace(".", "_");

    // Filter: IP_YYYY-MM-*
    QString pattern = QString("%1_%2-%3-*").arg(sanitizedIp).arg(year).arg(month, 2, 10, QChar('0'));
    QStringList filters;
    filters << pattern;

    QDirIterator it(path, filters, QDir::Files);
    while (it.hasNext()) {
        it.next();
        QString filename = it.fileName();
        // Extract day from filename: IP_YYYY-MM-DD_...
        // Split by underscore
        QStringList parts = filename.split("_");
        // Find the part that looks like a date YYYY-MM-DD
        for (const QString &part : parts) {
            if (part.count('-') == 2 && part.length() == 10) {
                QDate d = QDate::fromString(part, "yyyy-MM-dd");
                if (d.isValid() && d.year() == year && d.month() == month) {
                    if (!days.contains(d.day())) {
                        days.append(d.day());
                    }
                }
            }
        }
    }

    return days;
}

QString SystemController::generateRecordingPath(const QString &ip)
{
    QString path = PathUtils::localPathFromUserInput(m_appSettings.value("recordingsPath").toString());
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/OpenIPC";
    }
    QDir().mkpath(path);

    QString sanitizedIp = ip;
    sanitizedIp.replace(".", "_");
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");

    return QString("%1/%2_%3.mp4").arg(path, sanitizedIp, timestamp);
}

void SystemController::toggleRecording(int gridIndex)
{
    Q_UNUSED(gridIndex)
    // Legacy method kept for ABI compatibility if needed, but implementation removed
    // Logic moved to client-side (QML + player) to avoid ffmpeg dependency
    qWarning() << "SystemController::toggleRecording is deprecated. Use the client-side player recording API instead.";
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
