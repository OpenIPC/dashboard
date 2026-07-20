#include "DashboardWebRecordingManager.h"

#include "CameraModel.h"
#include "SystemController.h"
#include "gst/GstPlayer.h"

#include <QDateTime>
#include <QFileInfo>
#include <QTimer>

struct DashboardWebRecordingManager::Entry
{
    GstPlayer *player = nullptr;
    QTimer *segmentTimer = nullptr;
    QString cameraIp;
    QString path;
    QDateTime startedAt;
};

DashboardWebRecordingManager::DashboardWebRecordingManager(SystemController *systemController,
                                                           QObject *parent)
    : QObject(parent)
    , m_systemController(systemController)
{
}

DashboardWebRecordingManager::~DashboardWebRecordingManager()
{
    stopAll();
}

QString DashboardWebRecordingManager::sourceUrl(int cameraIndex) const
{
    if (!m_systemController || cameraIndex < 0
        || cameraIndex >= m_systemController->cameraModel()->rowCount()) {
        return {};
    }
    const Camera camera = m_systemController->cameraModel()->getCamera(cameraIndex);
    const QString source = !camera.hdStreamUrl.trimmed().isEmpty() ? camera.hdStreamUrl
        : (!camera.streamUrl.trimmed().isEmpty() ? camera.streamUrl : camera.sdStreamUrl);
    return m_systemController->authenticatedStreamUrl(source, camera.ip);
}

bool DashboardWebRecordingManager::start(int cameraIndex, QString *error)
{
    if (m_recordings.contains(cameraIndex)) return true;
    if (!m_systemController || cameraIndex < 0
        || cameraIndex >= m_systemController->cameraModel()->rowCount()) {
        if (error) *error = tr("Camera not found");
        return false;
    }
    const Camera camera = m_systemController->cameraModel()->getCamera(cameraIndex);
    const QString url = sourceUrl(cameraIndex);
    if (url.trimmed().isEmpty()) {
        if (error) *error = tr("Camera has no configured stream");
        return false;
    }
    const QString path = m_systemController->generateRecordingPath(camera.ip);
    if (path.trimmed().isEmpty()) {
        if (error) *error = tr("Could not create a recording path");
        return false;
    }

    auto *entry = new Entry;
    entry->cameraIp = camera.ip;
    entry->path = path;
    entry->startedAt = QDateTime::currentDateTimeUtc();
    entry->player = new GstPlayer;
    entry->player->setCameraId(camera.ip);
    entry->player->setRtspTransport(
        m_systemController->getAppSettings().value(
            QStringLiteral("playerRtspTransport"), QStringLiteral("tcp")).toString());
    entry->player->setBufferMode(
        m_systemController->getAppSettings().value(QStringLiteral("playerBufferMode"), 1).toInt());
    connect(entry->player, &GstPlayer::errorOccurred, this,
            [this, cameraIndex](const QString &message) {
        Entry *active = m_recordings.value(cameraIndex, nullptr);
        if (!active) return;
        m_systemController->notifyRecordingError(active->cameraIp, active->path, message);
        emit recordingError(cameraIndex, message);
        // GstPlayer is the signal sender. Defer destruction until the signal
        // stack has unwound so the sender is never deleted from its own slot.
        QTimer::singleShot(0, this, [this, cameraIndex]() {
            destroyEntry(cameraIndex, true);
        });
    });

    entry->segmentTimer = new QTimer(this);
    entry->segmentTimer->setSingleShot(false);
    const int segmentMinutes = qBound(
        5, m_systemController->getAppSettings().value(
               QStringLiteral("recordingSegmentDuration"), 15).toInt(), 60);
    entry->segmentTimer->setInterval(segmentMinutes * 60 * 1000);
    connect(entry->segmentTimer, &QTimer::timeout, this,
            [this, cameraIndex]() { rotate(cameraIndex); });

    m_recordings.insert(cameraIndex, entry);
    entry->player->setUrl(url);
    entry->player->setRecordingPath(path);
    entry->player->setRunning(true);
    entry->segmentTimer->start();
    setCameraRecording(cameraIndex, true);
    m_systemController->notifyRecordingStarted(camera.ip, path, QStringLiteral("web"));
    emit recordingStateChanged(cameraIndex, true);
    return true;
}

bool DashboardWebRecordingManager::stop(int cameraIndex, QString *error)
{
    if (!m_recordings.contains(cameraIndex)) {
        if (error) *error = tr("The camera is not being recorded by Web");
        return false;
    }
    destroyEntry(cameraIndex, true);
    return true;
}

void DashboardWebRecordingManager::stopAll()
{
    const QList<int> indexes = m_recordings.keys();
    for (int cameraIndex : indexes) destroyEntry(cameraIndex, true);
}

bool DashboardWebRecordingManager::isRecording(int cameraIndex) const
{
    return m_recordings.contains(cameraIndex);
}

QVariantList DashboardWebRecordingManager::status() const
{
    QVariantList result;
    for (auto it = m_recordings.constBegin(); it != m_recordings.constEnd(); ++it) {
        const Entry *entry = it.value();
        result.append(QVariantMap{
            {QStringLiteral("cameraIndex"), it.key()},
            {QStringLiteral("cameraIp"), entry->cameraIp},
            {QStringLiteral("startedAt"), entry->startedAt.toString(Qt::ISODate)},
            {QStringLiteral("segmentStartedAt"),
             QFileInfo(entry->path).birthTime().toUTC().toString(Qt::ISODate)}
        });
    }
    return result;
}

void DashboardWebRecordingManager::setCameraRecording(int cameraIndex, bool recording)
{
    if (!m_systemController || cameraIndex < 0
        || cameraIndex >= m_systemController->cameraModel()->rowCount()) return;
    Camera camera = m_systemController->cameraModel()->getCamera(cameraIndex);
    if (camera.isRecording == recording) return;
    camera.isRecording = recording;
    m_systemController->cameraModel()->setCamera(cameraIndex, camera);
}

void DashboardWebRecordingManager::rotate(int cameraIndex)
{
    Entry *entry = m_recordings.value(cameraIndex, nullptr);
    if (!entry || !entry->player) return;
    const QString previousPath = entry->path;
    const QString nextPath = m_systemController->generateRecordingPath(entry->cameraIp);
    if (nextPath.isEmpty()) return;
    entry->player->setRunning(false);
    entry->player->setRecordingPath(QString());
    entry->path = nextPath;
    entry->player->setRecordingPath(nextPath);
    entry->player->setRunning(true);
    m_systemController->notifyRecordingSegment(entry->cameraIp, previousPath, nextPath);
}

void DashboardWebRecordingManager::destroyEntry(int cameraIndex, bool notifyStopped)
{
    Entry *entry = m_recordings.take(cameraIndex);
    if (!entry) return;
    if (entry->segmentTimer) {
        entry->segmentTimer->stop();
        entry->segmentTimer->deleteLater();
    }
    if (entry->player) {
        entry->player->setRunning(false);
        entry->player->setRecordingPath(QString());
        delete entry->player;
    }
    setCameraRecording(cameraIndex, false);
    if (notifyStopped) {
        m_systemController->notifyRecordingStopped(
            entry->cameraIp, entry->path, QStringLiteral("web"));
    }
    delete entry;
    emit recordingStateChanged(cameraIndex, false);
}
