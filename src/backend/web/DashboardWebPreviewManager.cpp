#include "DashboardWebPreviewManager.h"

#include "CameraModel.h"
#include "SystemController.h"
#include "gst/GstPlayer.h"

#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QImage>
#include <QTimer>

namespace {

constexpr qint64 kFrameCacheMs = 55;
constexpr qint64 kIdleStreamTimeoutMs = 12 * 1000;
constexpr int kJpegQuality = 80;

QString normalizedQuality(const QString &quality)
{
    return quality.compare(QStringLiteral("hd"), Qt::CaseInsensitive) == 0
        ? QStringLiteral("hd") : QStringLiteral("sd");
}

QString previewSource(const Camera &camera, const QString &quality)
{
    if (quality == QStringLiteral("hd")) {
        if (!camera.hdStreamUrl.trimmed().isEmpty()) return camera.hdStreamUrl;
        if (!camera.streamUrl.trimmed().isEmpty()) return camera.streamUrl;
        return camera.sdStreamUrl;
    }
    if (!camera.sdStreamUrl.trimmed().isEmpty()) return camera.sdStreamUrl;
    if (!camera.streamUrl.trimmed().isEmpty()) return camera.streamUrl;
    return camera.hdStreamUrl;
}

} // namespace

struct DashboardWebPreviewManager::StreamState {
    QString key;
    QString identity;
    GstPlayer *player = nullptr;
    QByteArray encodedFrame;
    QString lastError;
    qint64 lastRequestedMs = 0;
    qint64 lastEncodedMs = 0;
};

DashboardWebPreviewManager::DashboardWebPreviewManager(SystemController *systemController,
                                                       QObject *parent)
    : QObject(parent)
    , m_systemController(systemController)
{
    auto *cleanupTimer = new QTimer(this);
    cleanupTimer->setInterval(3000);
    connect(cleanupTimer, &QTimer::timeout, this,
            [this]() { cleanupIdleStreams(); });
    cleanupTimer->start();
}

DashboardWebPreviewManager::~DashboardWebPreviewManager()
{
    stop();
}

DashboardWebPreviewManager::StreamState *DashboardWebPreviewManager::ensureStream(
    int cameraIndex, const QString &requestedQuality, FrameStatus *status)
{
    if (!m_systemController || cameraIndex < 0
        || cameraIndex >= m_systemController->cameraModel()->rowCount()) {
        if (status) *status = FrameStatus::InvalidCamera;
        return nullptr;
    }

    const Camera camera = m_systemController->cameraModel()->getCamera(cameraIndex);
    const QString quality = normalizedQuality(requestedQuality);
    const QString source = previewSource(camera, quality).trimmed();
    if (source.isEmpty()) {
        if (status) *status = FrameStatus::MissingStream;
        return nullptr;
    }

    const QString key = QStringLiteral("%1:%2").arg(cameraIndex).arg(quality);
    const QString authenticatedSource = m_systemController->authenticatedStreamUrl(source, camera.ip);
    const QString identity = camera.id + QLatin1Char('|') + camera.ip + QLatin1Char('|')
        + authenticatedSource;
    StreamState *stream = m_streams.value(key, nullptr);
    if (stream && stream->identity != identity) {
        removeStream(key);
        stream = nullptr;
    }
    if (stream) {
        if (status) *status = FrameStatus::WarmingUp;
        return stream;
    }

    stream = new StreamState;
    stream->key = key;
    stream->identity = identity;
    stream->lastRequestedMs = QDateTime::currentMSecsSinceEpoch();
    stream->player = new GstPlayer;
    stream->player->setParent(this);
    stream->player->setMuted(true);
    stream->player->setBufferMode(1);
    stream->player->setRtspTransport(QStringLiteral("tcp"));
    stream->player->setCameraId(camera.ip);
    stream->player->setUrl(authenticatedSource);
    GstPlayer *player = stream->player;
    connect(player, &GstPlayer::errorOccurred, this,
            [this, key, player](const QString &message) {
                StreamState *current = m_streams.value(key, nullptr);
                if (current && current->player == player) current->lastError = message;
            });
    stream->player->setRunning(true);
    m_streams.insert(key, stream);
    qInfo().noquote() << "Web preview relay started for" << camera.ip << quality;
    if (status) *status = FrameStatus::WarmingUp;
    return stream;
}

DashboardWebPreviewManager::FrameResult DashboardWebPreviewManager::frame(
    int cameraIndex, const QString &quality)
{
    FrameStatus initialStatus = FrameStatus::Unavailable;
    StreamState *stream = ensureStream(cameraIndex, quality, &initialStatus);
    if (!stream) return {initialStatus, {}};

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    stream->lastRequestedMs = now;
    if (!stream->encodedFrame.isEmpty() && now - stream->lastEncodedMs < kFrameCacheMs) {
        return {FrameStatus::Ready, stream->encodedFrame};
    }

    QImage image = stream->player->takeFrameCopy();
    if (image.isNull()) {
        const bool failed = !stream->lastError.isEmpty()
            || stream->player->connectionState() == QStringLiteral("authentication-error");
        return {failed ? FrameStatus::Unavailable : FrameStatus::WarmingUp, {}};
    }

    const bool hd = normalizedQuality(quality) == QStringLiteral("hd");
    const QSize maximumSize = hd ? QSize(1280, 720) : QSize(960, 540);
    if (image.width() > maximumSize.width() || image.height() > maximumSize.height()) {
        image = image.scaled(maximumSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QByteArray jpeg;
    QBuffer buffer(&jpeg);
    if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "JPEG", kJpegQuality)) {
        return {FrameStatus::Unavailable, {}};
    }
    stream->encodedFrame = jpeg;
    stream->lastEncodedMs = now;
    return {FrameStatus::Ready, stream->encodedFrame};
}

void DashboardWebPreviewManager::removeStream(const QString &key)
{
    StreamState *stream = m_streams.take(key);
    if (!stream) return;
    if (stream->player) {
        stream->player->setRunning(false);
        delete stream->player;
    }
    qInfo().noquote() << "Web preview relay stopped for" << key;
    delete stream;
}

void DashboardWebPreviewManager::cleanupIdleStreams()
{
    const qint64 threshold = QDateTime::currentMSecsSinceEpoch() - kIdleStreamTimeoutMs;
    const QStringList keys = m_streams.keys();
    for (const QString &key : keys) {
        const StreamState *stream = m_streams.value(key, nullptr);
        if (stream && stream->lastRequestedMs < threshold) removeStream(key);
    }
}

void DashboardWebPreviewManager::stop()
{
    const QStringList keys = m_streams.keys();
    for (const QString &key : keys) removeStream(key);
}
