#ifndef RECORDINGFILECATALOG_H
#define RECORDINGFILECATALOG_H

#include <QDateTime>
#include <QFileInfo>
#include <QString>
#include <QVariantMap>

#include <optional>

namespace RecordingFileCatalog {

struct RecordingFile {
    QDateTime startTime;
    QDateTime endTime;
    qint64 durationMs = 0;
    qint64 sizeBytes = 0;
    int channel = 0;
    int type = 0;
    QString cameraId;
    QString cameraToken;
    QString fileName;
    QString filePath;
    QString fileUrl;
    QString source;
    bool valid = false;
};

QString cameraToken(const QString &cameraId);
QString defaultRecordingRoot();
QString buildRecordingPath(const QString &rootPath,
                           const QString &cameraId,
                           const QDateTime &timestamp = QDateTime::currentDateTime(),
                           const QString &source = QStringLiteral("manual"));
bool isSupportedVideoFile(const QFileInfo &fileInfo);
bool belongsToCamera(const QFileInfo &fileInfo, const QString &cameraId);
std::optional<RecordingFile> inspectFile(const QFileInfo &fileInfo,
                                         const QString &cameraId = QString());
QVariantMap toVariantMap(const RecordingFile &file);

}

#endif // RECORDINGFILECATALOG_H
