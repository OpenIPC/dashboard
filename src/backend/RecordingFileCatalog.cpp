#include "RecordingFileCatalog.h"

#include "PathUtils.h"

#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

namespace {

QString normalizedForSearch(const QString &value)
{
    return value.trimmed().toLower();
}

QString sourceFromFileName(const QFileInfo &fileInfo)
{
    const QString haystack = (fileInfo.fileName() + QLatin1Char(' ') + fileInfo.absolutePath()).toLower();
    if (haystack.contains(QStringLiteral("event")) || haystack.contains(QStringLiteral("clip"))
        || haystack.contains(QStringLiteral("evidence"))) {
        return QStringLiteral("event");
    }
    if (haystack.contains(QStringLiteral("manual"))) {
        return QStringLiteral("manual");
    }
    return QStringLiteral("manual");
}

int typeFromSource(const QString &source)
{
    return source == QStringLiteral("event") ? 1 : 0;
}

QDateTime timestampFromFileName(const QString &fileName)
{
    static const QRegularExpression timestampRe(
        QStringLiteral("(\\d{4}-\\d{2}-\\d{2})[_-](\\d{2}-\\d{2}-\\d{2})(?:[-_](\\d{3}))?"));

    const QRegularExpressionMatch match = timestampRe.match(fileName);
    if (!match.hasMatch()) {
        return {};
    }

    const QString base = match.captured(1) + QLatin1Char('_') + match.captured(2);
    const QString ms = match.captured(3);
    return ms.isEmpty()
        ? QDateTime::fromString(base, QStringLiteral("yyyy-MM-dd_HH-mm-ss"))
        : QDateTime::fromString(base + QLatin1Char('-') + ms, QStringLiteral("yyyy-MM-dd_HH-mm-ss-zzz"));
}

qint64 durationFromTimestamps(const QDateTime &startTime, const QFileInfo &fileInfo)
{
    if (!startTime.isValid()) {
        return 0;
    }

    const QDateTime modified = fileInfo.lastModified();
    if (!modified.isValid() || modified <= startTime) {
        return 0;
    }

    const qint64 duration = startTime.msecsTo(modified);
    constexpr qint64 maxReasonableDurationMs = 24LL * 60LL * 60LL * 1000LL;
    return duration > 0 && duration <= maxReasonableDurationMs ? duration : 0;
}

} // namespace

namespace RecordingFileCatalog {

QString cameraToken(const QString &cameraId)
{
    QString token = cameraId.trimmed();
    static const QRegularExpression unsupported(QStringLiteral("[^A-Za-z0-9_-]+"));
    static const QRegularExpression repeated(QStringLiteral("_+"));

    token.replace(unsupported, QStringLiteral("_"));
    token.replace(repeated, QStringLiteral("_"));

    while (token.startsWith(QLatin1Char('_'))) {
        token.remove(0, 1);
    }
    while (token.endsWith(QLatin1Char('_'))) {
        token.chop(1);
    }

    return token.isEmpty() ? QStringLiteral("camera") : token;
}

QString defaultRecordingRoot()
{
    return QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + QStringLiteral("/OpenIPC");
}

QString buildRecordingPath(const QString &rootPath,
                           const QString &cameraId,
                           const QDateTime &timestamp,
                           const QString &source)
{
    QString root = PathUtils::localPathFromUserInput(rootPath);
    if (root.isEmpty()) {
        root = defaultRecordingRoot();
    }

    QDir().mkpath(root);

    const QDateTime safeTimestamp = timestamp.isValid() ? timestamp : QDateTime::currentDateTime();
    const QString safeSource = source.trimmed().isEmpty() ? QStringLiteral("manual") : cameraToken(source);
    const QString fileName = QStringLiteral("%1_%2_%3.mp4")
        .arg(cameraToken(cameraId),
             safeTimestamp.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss-zzz")),
             safeSource);

    return QDir(root).filePath(fileName);
}

bool isSupportedVideoFile(const QFileInfo &fileInfo)
{
    const QString suffix = fileInfo.suffix().toLower();
    return suffix == QStringLiteral("mp4")
        || suffix == QStringLiteral("mkv")
        || suffix == QStringLiteral("avi")
        || suffix == QStringLiteral("mov");
}

bool belongsToCamera(const QFileInfo &fileInfo, const QString &cameraId)
{
    const QString trimmedCamera = cameraId.trimmed();
    if (trimmedCamera.isEmpty()) {
        return true;
    }

    const QString raw = normalizedForSearch(trimmedCamera);
    const QString token = normalizedForSearch(cameraToken(trimmedCamera));
    const QString haystack = normalizedForSearch(fileInfo.fileName() + QLatin1Char(' ') + fileInfo.absolutePath());
    return haystack.contains(raw) || haystack.contains(token);
}

std::optional<RecordingFile> inspectFile(const QFileInfo &fileInfo, const QString &cameraId)
{
    if (!fileInfo.exists() || !fileInfo.isFile() || !isSupportedVideoFile(fileInfo)
        || !belongsToCamera(fileInfo, cameraId)) {
        return std::nullopt;
    }

    const QDateTime startTime = timestampFromFileName(fileInfo.fileName());
    if (!startTime.isValid()) {
        return std::nullopt;
    }

    const QString source = sourceFromFileName(fileInfo);
    const qint64 durationMs = durationFromTimestamps(startTime, fileInfo);
    const QString absolutePath = fileInfo.absoluteFilePath();

    RecordingFile file;
    file.valid = true;
    file.startTime = startTime;
    file.endTime = durationMs > 0 ? startTime.addMSecs(durationMs) : startTime;
    file.durationMs = durationMs;
    file.sizeBytes = fileInfo.size();
    file.type = typeFromSource(source);
    file.cameraId = cameraId.trimmed();
    file.cameraToken = cameraToken(cameraId);
    file.fileName = fileInfo.fileName();
    file.filePath = absolutePath;
    file.fileUrl = QUrl::fromLocalFile(absolutePath).toString();
    file.source = source;
    return file;
}

QVariantMap toVariantMap(const RecordingFile &file)
{
    QVariantMap map;
    map.insert(QStringLiteral("startTime"), file.startTime);
    map.insert(QStringLiteral("endTime"), file.endTime);
    map.insert(QStringLiteral("durationMs"), file.durationMs);
    map.insert(QStringLiteral("size"), file.sizeBytes);
    map.insert(QStringLiteral("sizeBytes"), file.sizeBytes);
    map.insert(QStringLiteral("channel"), file.channel);
    map.insert(QStringLiteral("type"), file.type);
    map.insert(QStringLiteral("cameraId"), file.cameraId);
    map.insert(QStringLiteral("cameraToken"), file.cameraToken);
    map.insert(QStringLiteral("fileName"), file.fileName);
    map.insert(QStringLiteral("filePath"), file.filePath);
    map.insert(QStringLiteral("fileUrl"), file.fileUrl);
    map.insert(QStringLiteral("source"), file.source);
    map.insert(QStringLiteral("dateKey"), file.startTime.date().toString(QStringLiteral("yyyy-MM-dd")));
    return map;
}

} // namespace RecordingFileCatalog
