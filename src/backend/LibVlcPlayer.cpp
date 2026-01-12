#include "LibVlcPlayer.h"

#include <QDateTime>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QDir>
#include <QPainter>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QtDebug>
#include <vlc/vlc.h>
#include <cstring>

class LibVlcPlayerRenderer : public QQuickFramebufferObject::Renderer {
public:
    explicit LibVlcPlayerRenderer(LibVlcPlayer *item) : m_item(item) {}

    void render() override {
        if (!m_item) return;
        QImage frame = m_item->takeFrameCopy();
        if (frame.isNull()) return;

        QOpenGLFramebufferObject *fbo = framebufferObject();
        if (!fbo) return;

        fbo->bind();
        QOpenGLPaintDevice device(fbo->size());
        QPainter painter(&device);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        painter.save();
        int w = fbo->width();
        int h = fbo->height();
        painter.translate(w / 2.0, h / 2.0);
        painter.rotate(m_orientation);
        if (m_mirror) {
            painter.scale(-1.0, 1.0);
        }
        painter.drawImage(QRect(-w / 2, -h / 2, w, h), frame);
        painter.restore();

        painter.end();
        fbo->release();
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        return new QOpenGLFramebufferObject(size, format);
    }

    void synchronize(QQuickFramebufferObject *item) override {
        m_item = static_cast<LibVlcPlayer*>(item);
        if (m_item) {
            m_orientation = m_item->orientation();
            m_mirror = m_item->mirror();
        }
    }

private:
    LibVlcPlayer *m_item = nullptr;
    int m_orientation = 0;
    bool m_mirror = false;
};

// Callback for LibVLC logging
void vlcLogCallback(void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list args)
{
    Q_UNUSED(data);
    Q_UNUSED(ctx);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    
    QString msg = QString("[LibVLC] %1").arg(buffer);
    
    if (level == LIBVLC_ERROR) qCritical() << msg;
    else if (level == LIBVLC_WARNING) qWarning() << msg;
    else qInfo() << msg;
}

// Helper to safely apply adjustments without breaking HW acceleration chain
static void applyVlcAdjustments(libvlc_media_player_t *player, float brightness, float contrast, int hue, float saturation, float gamma)
{
    if (!player) return;
    
    bool hasAdjustments = (qAbs(brightness - 1.0f) > 0.01f || 
                           qAbs(contrast - 1.0f) > 0.01f || 
                           qAbs(saturation - 1.0f) > 0.01f || 
                           qAbs(gamma - 1.0f) > 0.01f || 
                           hue != 0);

    if (hasAdjustments) {
        libvlc_video_set_adjust_int(player, 0, 1); // Enable
        libvlc_video_set_adjust_float(player, 1, contrast);
        libvlc_video_set_adjust_float(player, 2, brightness);
        libvlc_video_set_adjust_int(player, 3, hue);
        libvlc_video_set_adjust_float(player, 4, saturation);
        libvlc_video_set_adjust_float(player, 5, gamma);
    } else {
        libvlc_video_set_adjust_int(player, 0, 0); // Disable
    }
}

LibVlcPlayer::LibVlcPlayer(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setFlag(ItemHasContents, true);
    
    // Build plugin path so codecs/demux are found when bundled
    QString pluginPath = QCoreApplication::applicationDirPath() + "/plugins";
    // VLC 3.x+ no longer supports --plugin-path, use environment variable instead
    if (!pluginPath.isEmpty()) {
        qputenv("VLC_PLUGIN_PATH", pluginPath.toUtf8());
    }

    std::vector<const char*> args = {
        "--no-xlib",
        "--no-video-title-show",
        "--network-caching=300",
        // "--clock-jitter=0", // Disabled to improve smoothness (jitter helps smooth out bursty packets)
        // "--clock-synchro=0",
    };
    // if (!pluginPath.isEmpty()) {
    //    args.push_back(m_pluginPathArg.constData());
    // }

    m_instance = libvlc_new(static_cast<int>(args.size()), args.data());
    if (m_instance) {
        libvlc_log_set(m_instance, vlcLogCallback, this);
    } else {
        qWarning() << "LibVLC: failed to create instance";
    }

    m_lastFpsTime = QDateTime::currentMSecsSinceEpoch();

    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(200);
    connect(m_statsTimer, &QTimer::timeout, this, &LibVlcPlayer::updateMediaInfo);
}

LibVlcPlayer::~LibVlcPlayer()
{
    stopPlayback();
    if (m_instance) {
        libvlc_release(m_instance);
        m_instance = nullptr;
    }
}

QQuickFramebufferObject::Renderer *LibVlcPlayer::createRenderer() const
{
    return new LibVlcPlayerRenderer(const_cast<LibVlcPlayer*>(this));
}

bool LibVlcPlayer::saveSnapshot(const QString &path)
{
    QImage result;
    {
        QMutexLocker locker(&m_frameMutex);
        if (m_frameBuffer.isEmpty() || m_frameSize.isEmpty()) {
            qWarning() << "LibVlcPlayer: Snapshot failed, buffer empty";
            return false;
        }

        // Create image from buffer (no copy yet)
        QImage wrapper(reinterpret_cast<const uchar*>(m_frameBuffer.constData()),
                       m_frameSize.width(), m_frameSize.height(),
                       QImage::Format_ARGB32);
        
        // Deep copy needed because buffer might change after unlock
        result = wrapper.copy();
    }
    
    if (result.isNull()) {
        qWarning() << "LibVlcPlayer: Snapshot failed, frame is null";
        return false;
    }
    
    // Ensure directory exists
    QFileInfo info(path);

    QDir dir = info.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    bool success = result.save(path);
    if (success) {
        qInfo() << "LibVlcPlayer: Snapshot saved to" << path;
    } else {
        qWarning() << "LibVlcPlayer: Failed to save snapshot to" << path;
    }
    return success;
}

void LibVlcPlayer::setUrl(const QString &url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged();

    if (m_running) {
        startPlayback();
    }
}

void LibVlcPlayer::setRecordingPath(const QString &path)
{
    if (m_recordingPath == path) return;
    m_recordingPath = path;
    emit recordingPathChanged();

    // Restart required to apply sout options
    if (m_running) startPlayback();
}

void LibVlcPlayer::setBackgroundMode(bool bg)
{
    if (m_backgroundMode == bg) return;
    m_backgroundMode = bg;
    emit backgroundModeChanged();
    if (m_running) startPlayback();
}

void LibVlcPlayer::setAnalyticsUrl(const QString &url)
{
    if (m_analyticsUrl == url)
        return;
    m_analyticsUrl = url;
    emit analyticsUrlChanged();
}

void LibVlcPlayer::setRunning(bool running)
{
    if (m_running == running)
        return;

    m_running = running;
    emit runningChanged();

    if (m_running) {
        if (m_statsTimer) m_statsTimer->start();
        startPlayback();
    } else {
        if (m_statsTimer) m_statsTimer->stop();
        stopPlayback();
    }
}

void LibVlcPlayer::setAudioNormalization(bool normalize)
{
    if (m_audioNormalization == normalize) return;
    m_audioNormalization = normalize;
    emit audioNormalizationChanged();
    
    if (m_running) {
        // Save current position
        qint64 pos = -1;
        if (m_player) {
            pos = libvlc_media_player_get_time(m_player);
        }

        startPlayback();

        // Restore position if valid (for archive playback)
        if (pos > 0 && m_player) {
            // We need to wait a tiny bit or just set it? 
            // set_time usually works after media is parsed, but for local files it might work immediately.
            // For RTSP streams pos is usually 0 or -1, so it won't affect live streams.
            libvlc_media_player_set_time(m_player, pos);
        }
    }
}

void LibVlcPlayer::setFillMode(float mode)
{
    if (qFuzzyCompare(m_fillMode, mode))
        return;
    m_fillMode = mode;
    emit fillModeChanged();
}

void LibVlcPlayer::setOrientation(int orientation)
{
    if (m_orientation == orientation)
        return;
    m_orientation = orientation;
    emit orientationChanged();
}

void LibVlcPlayer::setPlaybackRate(double rate)
{
    if (qFuzzyCompare(m_playbackRate, rate))
        return;
    m_playbackRate = rate;
    if (m_player)
        libvlc_media_player_set_rate(m_player, static_cast<float>(m_playbackRate));
    emit playbackRateChanged();
}

void LibVlcPlayer::setMirror(bool mirror)
{
    if (m_mirror == mirror)
        return;
    m_mirror = mirror;
    emit mirrorChanged();
}

void LibVlcPlayer::setBufferMode(int mode)
{
    if (m_bufferMode == mode)
        return;
    m_bufferMode = mode;
    emit bufferModeChanged();
    if (m_running) startPlayback();
}

void LibVlcPlayer::setRtspTransport(const QString &transport)
{
    if (m_rtspTransport == transport)
        return;
    m_rtspTransport = transport;
    emit rtspTransportChanged();
    if (m_running) startPlayback();
}

void LibVlcPlayer::setBrightness(float val)
{
    if (qFuzzyCompare(m_brightness, val)) return;
    m_brightness = val;
    emit videoAdjustmentsChanged();
    applyVlcAdjustments(m_player, m_brightness, m_contrast, m_hue, m_saturation, m_gamma);
}

void LibVlcPlayer::setContrast(float val)
{
    if (qFuzzyCompare(m_contrast, val)) return;
    m_contrast = val;
    emit videoAdjustmentsChanged();
    applyVlcAdjustments(m_player, m_brightness, m_contrast, m_hue, m_saturation, m_gamma);
}

void LibVlcPlayer::setHue(int val)
{
    if (m_hue == val) return;
    m_hue = val;
    emit videoAdjustmentsChanged();
    applyVlcAdjustments(m_player, m_brightness, m_contrast, m_hue, m_saturation, m_gamma);
}

void LibVlcPlayer::setSaturation(float val)
{
    if (qFuzzyCompare(m_saturation, val)) return;
    m_saturation = val;
    emit videoAdjustmentsChanged();
    applyVlcAdjustments(m_player, m_brightness, m_contrast, m_hue, m_saturation, m_gamma);
}

void LibVlcPlayer::setGamma(float val)
{
    if (qFuzzyCompare(m_gamma, val)) return;
    m_gamma = val;
    emit videoAdjustmentsChanged();
    applyVlcAdjustments(m_player, m_brightness, m_contrast, m_hue, m_saturation, m_gamma);
}

void LibVlcPlayer::setHwDecoding(const QString &hw)
{
    if (m_hwDecoding == hw) return;
    m_hwDecoding = hw;
    emit hwDecodingChanged();
    if (m_running) startPlayback();
}

qint64 LibVlcPlayer::duration() const
{
    if (m_player) {
        return libvlc_media_player_get_length(m_player);
    }
    return 0;
}

qint64 LibVlcPlayer::position() const
{
    if (m_player) {
         return libvlc_media_player_get_time(m_player);
    }
    return 0;
}

void LibVlcPlayer::setPosition(qint64 pos)
{
    if (m_player) {
        libvlc_media_player_set_time(m_player, pos);
    }
}

void LibVlcPlayer::updateMediaInfo()
{
    if (m_player) {
        emit positionChanged();
        emit durationChanged();
    }
    
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 delta = now - m_lastFpsTime;
    if (delta >= 1000) {
        int count = m_frameCount.exchange(0);
        m_videoFps = (double)count * 1000.0 / (double)delta;
        m_lastFpsTime = now;
        emit videoStatsChanged();
    }
}

void LibVlcPlayer::setVolume(double volume)
{
    if (qFuzzyCompare(m_volume, volume))
        return;
    m_volume = volume;
    if (m_player)
        libvlc_audio_set_volume(m_player, static_cast<int>(m_volume * 100));
    emit volumeChanged();
}

void LibVlcPlayer::setMuted(bool muted)
{
    if (m_muted == muted)
        return;
    m_muted = muted;
    if (m_player)
        libvlc_audio_set_mute(m_player, m_muted);
    emit mutedChanged();
}

void LibVlcPlayer::setHwDecoders(const QStringList &decoders)
{
    // Placeholder: libVLC selects decoders internally; keep for API compatibility
    if (m_hwDecoders == decoders)
        return;
    m_hwDecoders = decoders;
    emit hwDecodersChanged();
}

void LibVlcPlayer::setAnalyticsEngine(QObject* engine)
{
    m_dummyAnalytics = engine;
    emit analyticsEngineChanged();
}

void LibVlcPlayer::setCameraId(const QString& id)
{
    if (m_cameraId == id)
        return;
    m_cameraId = id;
    emit cameraIdChanged();
}

QImage LibVlcPlayer::takeFrameCopy()
{
    if (!m_frameReady.exchange(false))
        return QImage();

    QMutexLocker locker(&m_frameMutex);
    if (m_frameBuffer.isEmpty() || m_frameSize.isEmpty())
        return QImage();

    QImage wrapped(reinterpret_cast<const uchar*>(m_frameBuffer.constData()),
                   m_frameSize.width(), m_frameSize.height(),
                   QImage::Format_ARGB32);
    return wrapped.copy();
}

void* LibVlcPlayer::lockFrame(void **planes)
{
    m_frameMutex.lock();
    const int bytes = m_frameSize.width() * m_frameSize.height() * 4;
    if (m_frameBuffer.size() != bytes) {
        m_frameBuffer.resize(bytes);
    }
    *planes = m_frameBuffer.data();
    return nullptr;
}

void LibVlcPlayer::displayFrame()
{
    m_frameReady.store(true);
    m_frameCount++;
    emit frameReady();
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

void* LibVlcPlayer::lockCallback(void *opaque, void **planes)
{
    auto self = static_cast<LibVlcPlayer*>(opaque);
    return self ? self->lockFrame(planes) : nullptr;
}

void LibVlcPlayer::unlockCallback(void *opaque, void *picture, void *const *planes)
{
    Q_UNUSED(opaque);
    Q_UNUSED(picture);
    Q_UNUSED(planes);
    auto self = static_cast<LibVlcPlayer*>(opaque);
    if (self) self->m_frameMutex.unlock();
}

void LibVlcPlayer::displayCallback(void *opaque, void *picture)
{
    Q_UNUSED(picture);
    auto self = static_cast<LibVlcPlayer*>(opaque);
    if (self) self->displayFrame();
}

unsigned LibVlcPlayer::formatSetup(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
    auto self = static_cast<LibVlcPlayer*>(*opaque);
    if (!self) return 0;
    memcpy(chroma, "RV32", 4);
    if (*width == 0 || *height == 0) {
        *width = 1280;
        *height = 720;
    }
    *pitches = (*width) * 4;
    *lines = *height;

    {
        QMutexLocker locker(&self->m_frameMutex);
        self->m_frameSize = QSize(static_cast<int>(*width), static_cast<int>(*height));
        self->m_frameBuffer.resize(self->m_frameSize.width() * self->m_frameSize.height() * 4);
    }
    self->updateStatsOnFrame(static_cast<int>(*width), static_cast<int>(*height));
    return 1; // pixelsize (bytes per pixel)
}

void LibVlcPlayer::formatCleanup(void *opaque)
{
    Q_UNUSED(opaque);
}

void LibVlcPlayer::updateStatsOnFrame(int width, int height)
{
    QString res = QString::number(width) + "x" + QString::number(height);
    if (m_videoResolution != res) {
        m_videoResolution = res;
        emit videoStatsChanged();
    }
}

void LibVlcPlayer::ensurePlayer()
{
    if (!m_instance) return;

    if (!m_player) {
        m_player = libvlc_media_player_new(m_instance);
        libvlc_video_set_callbacks(m_player, &LibVlcPlayer::lockCallback, &LibVlcPlayer::unlockCallback, &LibVlcPlayer::displayCallback, this);
        libvlc_video_set_format_callbacks(m_player, &LibVlcPlayer::formatSetup, &LibVlcPlayer::formatCleanup);
    }
}

void LibVlcPlayer::applyMediaOptions(libvlc_media_t *media)
{
    if (!media) return;

    // Check if any video adjustments are active (non-default values)
    bool hasAdjustments = (qAbs(m_brightness - 1.0f) > 0.01f || 
                           qAbs(m_contrast - 1.0f) > 0.01f || 
                           qAbs(m_saturation - 1.0f) > 0.01f || 
                           qAbs(m_gamma - 1.0f) > 0.01f || 
                           m_hue != 0);

    // Determine if we should use Hardware Acceleration
    // Default to true unless explicitly disabled
    bool useHw = true;
    if (m_hwDecoding == "none" || m_hwDecoding == "off") {
        useHw = false;
    } 
    // Removed automatic downgrade. Prioritize Performance.

    if (useHw) {
        // Hardware decoding enabled
        if (m_hwDecoding.isEmpty() || m_hwDecoding == "auto") {
             // Use DXVA2 on Windows for better compatibility with callbacks (copy-back)
             #ifdef Q_OS_WIN
             libvlc_media_add_option(media, ":avcodec-hw=dxva2");
             #else
             libvlc_media_add_option(media, ":avcodec-hw=any");
             #endif
        } else {
             QString opt = QString(":avcodec-hw=%1").arg(m_hwDecoding);
             libvlc_media_add_option(media, opt.toUtf8().constData());
        }
        // NOTE: We do NOT add :video-filter=adjust here.
        // Adding it with D3D11/DXVA causes "swscale" recursion errors and stuttering.
    } else {
        // Software decoding - safe to use filters
        libvlc_media_add_option(media, ":avcodec-hw=none");
        libvlc_media_add_option(media, ":video-filter=adjust");
    }

    int cacheMs = 300;
    if (m_bufferMode == 0) cacheMs = 200; // Realtime: increased 100->200ms for stability
    else if (m_bufferMode == 2) cacheMs = 1000; // Smooth

    QString cacheOpt = QString(":network-caching=%1").arg(cacheMs);
    QString liveOpt = QString(":live-caching=%1").arg(cacheMs);
    libvlc_media_add_option(media, cacheOpt.toUtf8().constData());
    libvlc_media_add_option(media, liveOpt.toUtf8().constData());
    // libvlc_media_add_option(media, ":drop-late-frames"); // Disabled for smoothness
    // libvlc_media_add_option(media, ":skip-frames");

    if (m_rtspTransport.compare("tcp", Qt::CaseInsensitive) == 0) {
        libvlc_media_add_option(media, ":rtsp-tcp");
    } else if (m_rtspTransport.compare("udp", Qt::CaseInsensitive) == 0) {
        libvlc_media_add_option(media, ":rtsp-udp");
    }
    if (m_audioNormalization) {
        // Use 'normvol' to boost quiet sounds and limit loud ones (Normalization)
        // buff-size=10 : buffer for normalization (10 is usually strings of buffers, roughly 300ms)
        // level=2.0 : Max amplification factor (can be adjusted)
        libvlc_media_add_option(media, ":audio-filter=normvol");
        libvlc_media_add_option(media, ":norm-max-level=2.0"); 
    }
    if (!m_recordingPath.isEmpty()) {
        // Use sout chain to duplicate stream:
        // 1. To 'display' (for rendering)
        // 2. To 'std' (save to file, mp4 muxer)
        // Note: 'display' in sout context means we need to ensure it reaches our video callbacks.
        // LibVLC sout syntax: #duplicate{dst=display,dst=std{access=file,mux=mp4,dst='PATH'}}
        
        // Ensure path uses forward slashes (VLC requirement on Windows usually)
        QString safePath = m_recordingPath;
        safePath.replace("\\", "/");
        
        // Escaping single quotes if necessary (though unlikely in standard paths)
        safePath.replace("'", "\\'");

        QString sout;
        if (m_backgroundMode) {
             // In background mode, we ONLY save to file, we do NOT display.
             // This avoids decoding overhead for display.
             sout = QString(":sout=#std{access=file,mux=mp4,dst='%1'}").arg(safePath);
             qInfo() << "LibVLC: Enabling BACKGROUND recording to" << safePath;
        } else {
             // Standard mode: Display + Record
             sout = QString(":sout=#duplicate{dst=display,dst=std{access=file,mux=mp4,dst='%1'}}").arg(safePath);
             qInfo() << "LibVLC: Enabling recording to" << safePath;
        }
        libvlc_media_add_option(media, sout.toUtf8().constData());
        
        // When using sout, we often need to be careful with HW acceleration as mentioned in analysis,
        // but modern VLC (3+) handles duplicate with display quite well.
        // However, if the user sees black screen during recording, we might need to disable HW accel here.
        // For now, let's keep it as configured.
    }

    // Do NOT use :no-audio here, or we can't unmute later without restarting the stream.
    // We rely on libvlc_audio_set_mute() called in startPlayback instead.
    // if (m_muted)
    //    libvlc_media_add_option(media, ":no-audio");
}

void LibVlcPlayer::startPlayback()
{
    stopPlayback();

    if (!m_instance || m_url.isEmpty())
        return;

    qInfo() << "LibVLC: Starting playback for URL:" << m_url;

    ensurePlayer();
    if (!m_player) return;

    m_media = libvlc_media_new_location(m_instance, m_url.toUtf8().constData());
    applyMediaOptions(m_media);

    libvlc_media_player_set_media(m_player, m_media);
    libvlc_media_player_set_rate(m_player, static_cast<float>(m_playbackRate));
    libvlc_audio_set_volume(m_player, static_cast<int>(m_volume * 100));
    libvlc_audio_set_mute(m_player, m_muted);
    
    // Apply video adjustments
    applyVlcAdjustments(m_player, m_brightness, m_contrast, m_hue, m_saturation, m_gamma);

    libvlc_media_player_play(m_player);
}

void LibVlcPlayer::stopPlayback()
{
    if (m_player) {
        libvlc_media_player_stop(m_player);
    }
    if (m_media) {
        libvlc_media_release(m_media);
        m_media = nullptr;
    }
    {
        QMutexLocker locker(&m_frameMutex);
        m_frameBuffer.clear();
        m_frameSize = QSize();
    }
    m_frameReady.store(false);
}
