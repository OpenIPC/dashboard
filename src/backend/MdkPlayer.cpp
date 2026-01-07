#include "MdkPlayer.h"
#include "analytics/AnalyticsEngine.h"
#include <QQuickWindow>
#include <QDebug>
#include <mdk/Global.h>
#include <mdk/RenderAPI.h>
#include <QUrl>
#include <QUrlQuery>
#include <limits>
#include <mutex>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <QMetaObject>
#include <QMetaMethod>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QPointer>

#include <QDateTime>

class MdkPlayerRenderer : public QQuickFramebufferObject::Renderer {
public:
    MdkPlayerRenderer(MdkPlayer *item) : m_item(item) {
        if (m_item && m_item->player()) {
            m_player = m_item->player();
            QPointer<MdkPlayer> guardedItem(item);
            m_player->setRenderCallback([guardedItem](void *) {
                if (guardedItem)
                    QMetaObject::invokeMethod(guardedItem, "update");
            });
        }
    }
    
    ~MdkPlayerRenderer() {
        if (m_player) {
             m_player->setRenderCallback(nullptr);
        }
    }

    void render() override {
        if (!m_player) return;
        
        // We need to ensure the window is exposed before rendering
        if (!m_item || !m_item->window() || !m_item->window()->isExposed()) return;

        int currentFbo = framebufferObject()->handle();
        
        // Update RenderAPI if FBO changed or not set
        if (!m_apiSet || m_renderApi.fbo != currentFbo) {
            m_renderApi.getProcAddress = [](const char* name, void* opaque) {
                QOpenGLContext* ctx = static_cast<QOpenGLContext*>(opaque);
                if (!ctx) ctx = QOpenGLContext::currentContext();
                if (!ctx) return (void*)nullptr;
                return (void*)ctx->getProcAddress(name);
            };
            m_renderApi.opaque = QOpenGLContext::currentContext();
            m_renderApi.fbo = currentFbo;
            m_player->setRenderAPI(&m_renderApi);
            m_apiSet = true;
            qInfo() << "MdkPlayerRenderer: RenderAPI set with FBO" << currentFbo;
        }

        double t = m_player->renderVideo();
        if (t < 0) {
             // qDebug() << "MdkPlayerRenderer: No frame rendered";
        }
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    void synchronize(QQuickFramebufferObject *item) override {
        m_item = static_cast<MdkPlayer*>(item);
        if (!m_item->player()) return;
        
        m_player = m_item->player(); // Update shared_ptr just in case
        
        QSize size = m_item->size().toSize() * m_item->window()->effectiveDevicePixelRatio();
        m_player->setVideoSurfaceSize(size.width(), size.height());
        m_player->setAspectRatio(m_item->fillMode());
        m_player->rotate(m_item->orientation());
        // Mirroring handled by QML transform for now, or we could use setVideoSurfaceSize(-w, h)
    }

private:
    MdkPlayer *m_item;
    std::shared_ptr<mdk::Player> m_player;
    mdk::GLRenderAPI m_renderApi;
    bool m_apiSet = false;
};

MdkPlayer::MdkPlayer(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    qInfo() << "MdkPlayer::MdkPlayer constructor start";
    setFlag(ItemHasContents, true);
    m_player = std::make_shared<mdk::Player>();
    qInfo() << "MdkPlayer::MdkPlayer m_player created";
    
    m_lastFpsTime = QDateTime::currentMSecsSinceEpoch();

    m_player->onMediaStatus([this](mdk::MediaStatus oldValue, mdk::MediaStatus newValue) {
        qInfo() << "MdkPlayer status changed from" << (int)oldValue << "to" << (int)newValue << "for" << m_url;
        emit mediaStatusChanged((int)newValue);
        return true;
    });
    
    // Default FPS counter
    m_player->onFrame<mdk::VideoFrame>([this](mdk::VideoFrame&, int track) {
         if (track >= 0) {
             m_frameCount++;
         }
         return 0;
    });

    m_infoTimer = new QTimer(this);
    m_infoTimer->setInterval(500); // Update every 500ms
    connect(m_infoTimer, &QTimer::timeout, this, &MdkPlayer::updateMediaInfo);

    qInfo() << "MdkPlayer::MdkPlayer constructor end";
}

MdkPlayer::~MdkPlayer()
{
    // Unregister callbacks to prevent access violation if MDK calls them during destruction
    m_player->onMediaStatus(nullptr);
    m_player->onEvent(nullptr);
    m_player->onFrame<mdk::VideoFrame>(nullptr);

    m_player->set(mdk::State::Stopped);
    m_player->setMedia(nullptr);
}

QQuickFramebufferObject::Renderer *MdkPlayer::createRenderer() const
{
    return new MdkPlayerRenderer(const_cast<MdkPlayer*>(this));
}

int MdkPlayer::mediaStatus() const
{
    return (int)m_player->mediaStatus();
}

QString MdkPlayer::url() const
{
    return m_url;
}

void MdkPlayer::setUrl(const QString &url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged();

    update();

    if (m_url.isEmpty()) {
        m_player->set(mdk::State::Stopped);
        m_player->setMedia(nullptr);
        setRunning(false);
    } else {
        m_player->setMedia(m_url.toUtf8().constData());
        
        // Buffer: 250ms to smooth out network jitter (fixes "jerky" video)
        if (m_bufferMode == 0) { // Realtime
            m_player->setBufferRange(0);
            m_player->setProperty("avformat.fflags", "nobuffer");
            m_player->setProperty("avformat.max_delay", "0");
        } else if (m_bufferMode == 2) { // Smooth
            m_player->setBufferRange(1000);
            m_player->setProperty("avformat.fflags", "");
            m_player->setProperty("avformat.max_delay", "1000000");
        } else { // Balanced (Default)
            m_player->setBufferRange(250);
            m_player->setProperty("avformat.fflags", "");
            m_player->setProperty("avformat.max_delay", "250000");
        }

        // Removed "nobuffer" to allow smooth playback
        // m_player->setProperty("avformat.fflags", "nobuffer");
        m_player->setProperty("avformat.flush_packets", "1");
        m_player->setProperty("avformat.discardcorrupt", "1");
        // Slightly increased probe size for better FPS detection
        m_player->setProperty("avformat.probesize", "200000");
        m_player->setProperty("avformat.analyzeduration", "200000");
        m_player->setProperty("avformat.rtsp_transport", m_rtspTransport.toUtf8().constData());
        m_player->setProperty("avformat.reorder_queue_size", "0");
        m_player->setProperty("avformat.buffer_size", "2048000");
        
        if (m_running) {
            m_player->set(mdk::State::Playing);
        } else {
            m_player->prepare();
        }
    }
}

bool MdkPlayer::isRunning() const
{
    return m_running;
}

void MdkPlayer::setRunning(bool running)
{
    if (m_running == running)
        return;

    m_running = running;
    emit runningChanged();

    bool isNetwork = m_url.startsWith("rtsp://", Qt::CaseInsensitive) || 
                     m_url.startsWith("http://", Qt::CaseInsensitive) || 
                     m_url.startsWith("https://", Qt::CaseInsensitive) || 
                     m_url.startsWith("udp://", Qt::CaseInsensitive);

    if (m_running && !m_url.isEmpty()) {
        if (m_infoTimer) m_infoTimer->start();
        if (isNetwork) {
            // Force reload media to ensure fresh stream for network sources
            m_player->setMedia(nullptr);
            m_player->setMedia(m_url.toUtf8().constData());

            // Buffer: 250ms to smooth out network jitter (fixes "jerky" video)
            if (m_bufferMode == 0) { // Realtime
                m_player->setBufferRange(0);
                m_player->setProperty("avformat.fflags", "nobuffer");
                m_player->setProperty("avformat.max_delay", "0");
            } else if (m_bufferMode == 2) { // Smooth
                m_player->setBufferRange(1000);
                m_player->setProperty("avformat.fflags", "");
                m_player->setProperty("avformat.max_delay", "1000000");
            } else { // Balanced (Default)
                m_player->setBufferRange(250);
                m_player->setProperty("avformat.fflags", "");
                m_player->setProperty("avformat.max_delay", "250000");
            }

            // Removed "nobuffer" to allow smooth playback
            // m_player->setProperty("avformat.fflags", "nobuffer");
            m_player->setProperty("avformat.flush_packets", "1");
            m_player->setProperty("avformat.discardcorrupt", "1");
            // Slightly increased probe size for better FPS detection
            m_player->setProperty("avformat.probesize", "200000");
            m_player->setProperty("avformat.analyzeduration", "200000");
            m_player->setProperty("avformat.rtsp_transport", m_rtspTransport.toUtf8().constData());
            m_player->setProperty("avformat.reorder_queue_size", "0");
            m_player->setProperty("avformat.buffer_size", "2048000");
        }

        m_player->set(mdk::State::Playing);
    } else {
        if (m_infoTimer) m_infoTimer->stop();
        if (isNetwork) {
            m_player->set(mdk::State::Stopped);
            m_player->setMedia(nullptr);
            update();
        } else {
            m_player->set(mdk::State::Paused);
        }
    }
}

void MdkPlayer::setFillMode(float mode)
{
    if (qFuzzyCompare(m_fillMode, mode))
        return;

    m_fillMode = mode;
    emit fillModeChanged();
    update();
}

void MdkPlayer::setOrientation(int orientation)
{
    if (m_orientation == orientation)
        return;

    m_orientation = orientation;
    emit orientationChanged();
    update();
}

void MdkPlayer::setMirror(bool mirror)
{
    if (m_mirror == mirror)
        return;

    m_mirror = mirror;
    emit mirrorChanged();
    update();
}

void MdkPlayer::setBufferMode(int mode)
{
    if (m_bufferMode == mode)
        return;

    m_bufferMode = mode;
    emit bufferModeChanged();
    
    // If running, we might need to restart to apply buffer changes effectively
    // or just rely on next start/url change.
    // Dynamic change is tricky for some properties, but we can try setting properties on the fly.
    // However, setBufferRange works dynamically. avformat options usually require re-opening.
    if (isRunning()) {
        // Soft restart or re-apply where possible
         m_player->setBufferRange(m_bufferMode == 0 ? 0 : (m_bufferMode == 2 ? 1000 : 250));
         // For deep avformat changes, full restart is safer to ensure consistency
         // But users might not like video flickering on setting change. 
         // Let's force restart if user changes this setting while playing.
         QString currentUrl = m_url;
         setRunning(false);
         setRunning(true);
    }
}

void MdkPlayer::setRtspTransport(const QString &transport)
{
    if (m_rtspTransport == transport)
        return;

    m_rtspTransport = transport;
    emit rtspTransportChanged();
    
    if (isRunning()) {
         setRunning(false);
         setRunning(true);
    }
}

qint64 MdkPlayer::duration() const
{
    return m_player ? m_player->mediaInfo().duration : 0;
}

qint64 MdkPlayer::position() const
{
    return m_player ? m_player->position() : 0;
}

void MdkPlayer::setPosition(qint64 ms)
{
    if (m_player) {
        m_player->seek(ms);
        emit positionChanged();
    }
}

void MdkPlayer::updateMediaInfo()
{
    if (!m_player) return;
    
    // Emit position/duration updates
    emit positionChanged();
    emit durationChanged();

    const auto& info = m_player->mediaInfo();
    if (info.video.empty()) return;
    
    const auto& v = info.video[0];
    QString codec = QString::fromStdString(v.codec.codec);
    QString res = QString("%1x%2").arg(v.codec.width).arg(v.codec.height);
    
    qint64 rawBitrate = v.codec.bit_rate > 0 ? v.codec.bit_rate : info.bit_rate;
    int bitrate = rawBitrate > 0 ? static_cast<int>((rawBitrate * 8) / 1000) : 0;
    
    double fps = m_videoFps;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 delta = now - m_lastFpsTime;
    
    if (delta >= 1000) {
        int count = m_frameCount.exchange(0);
        fps = (double)count * 1000.0 / delta;
        m_lastFpsTime = now;
    } else if (m_videoFps == 0.0) {
        fps = v.codec.frame_rate;
    }
    
    bool changed = false;
    if (m_videoCodec != codec) { m_videoCodec = codec; changed = true; }
    if (m_videoResolution != res) { m_videoResolution = res; changed = true; }
    if (m_videoBitrate != bitrate) { m_videoBitrate = bitrate; changed = true; }
    if (std::abs(m_videoFps - fps) > 0.1) { m_videoFps = fps; changed = true; }
    
    if (changed) {
        emit videoStatsChanged();
    }
}

void MdkPlayer::setVolume(double volume)
{
    if (qFuzzyCompare(m_volume, volume))
        return;

    m_volume = volume;
    m_player->setVolume(static_cast<float>(m_volume));
    emit volumeChanged();
}

void MdkPlayer::setMuted(bool muted)
{
    if (m_muted == muted)
        return;

    m_muted = muted;
    m_player->setMute(m_muted);
    emit mutedChanged();
}

void MdkPlayer::setHwDecoders(const QStringList &decoders)
{
    if (m_hwDecoders == decoders)
        return;

    m_hwDecoders = decoders;
    
    std::vector<std::string> stdDecoders;
    for (const QString &d : decoders) {
        stdDecoders.push_back(d.toStdString());
    }
    
    m_player->setDecoders(mdk::MediaType::Video, stdDecoders);
    emit hwDecodersChanged();
}

void MdkPlayer::setAnalyticsEngine(QObject* engine)
{
    if (m_analyticsEngineObj == engine)
        return;

    m_analyticsEngineObj = engine;
    m_analyticsEngine = qobject_cast<AnalyticsEngine*>(engine);
    
    qInfo() << "MdkPlayer::setAnalyticsEngine" << (m_analyticsEngine ? "enabled" : "disabled") << "for" << m_cameraId;

    if (m_analyticsEngine) {
        // Wrap analytics processing to avoid uncaught exceptions killing the app
        m_player->onFrame<mdk::VideoFrame>([this](mdk::VideoFrame& frame, int track) {
            if (track < 0) return 0;

            m_frameCount++;

            qint64 now = QDateTime::currentMSecsSinceEpoch();
            if (now - m_lastFrameTime <= 200) return 0;

            m_lastFrameTime = now;

            if (!m_analyticsEngine || m_cameraId.isEmpty()) return 0;

            // Check if any modules are actually enabled for this camera to avoid unnecessary processing
            if (!m_analyticsEngine->hasActiveModules(m_cameraId)) return 0;

            // Check if engine is busy before allocating memory for frame copy
            if (m_analyticsEngine->isBusy(m_cameraId)) return 0;

            try {
                QImage scaledImg;
                
                if (frame.format() != mdk::PixelFormat::BGRA) {
                    auto cpuFrame = frame.to(mdk::PixelFormat::BGRA);
                    if (cpuFrame && cpuFrame.bufferData(0)) {
                        QImage wrapper(cpuFrame.bufferData(0), cpuFrame.width(), cpuFrame.height(), cpuFrame.bytesPerLine(0), QImage::Format_ARGB32);
                        scaledImg = wrapper.scaledToWidth(640, Qt::FastTransformation);
                    }
                } else if (frame.bufferData(0)) {
                    QImage wrapper(frame.bufferData(0), frame.width(), frame.height(), frame.bytesPerLine(0), QImage::Format_ARGB32);
                    scaledImg = wrapper.scaledToWidth(640, Qt::FastTransformation);
                }

                if (!scaledImg.isNull()) {
                    m_analyticsEngine->processFrame(scaledImg, m_cameraId);
                }
            } catch (const std::exception &ex) {
                qCritical() << "Analytics processing failed for" << m_cameraId << "error:" << ex.what();
            } catch (...) {
                qCritical() << "Analytics processing failed with unknown error for" << m_cameraId;
            }

            return 0;
        });
    } else {
        m_player->onFrame<mdk::VideoFrame>([this](mdk::VideoFrame&, int track) {
             if (track >= 0) m_frameCount++;
             return 0;
        });
    }
    
    emit analyticsEngineChanged();
}

void MdkPlayer::setCameraId(const QString& id)
{
    if (m_cameraId == id)
        return;

    m_cameraId = id;
    emit cameraIdChanged();
}

void MdkPlayer::setPlaybackRate(double rate)
{
    if (qFuzzyCompare(m_playbackRate, rate))
        return;
    m_playbackRate = rate;
    if (m_player) {
        m_player->setPlaybackRate(rate);
    }
    emit playbackRateChanged();
}