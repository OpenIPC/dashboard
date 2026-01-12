#pragma once

#include <QQuickFramebufferObject>
#include <QImage>
#include <QMutex>
#include <QAtomicInteger>
#include <atomic>
#include <QStringList>
#include <QTimer>

// Forward declaration of libvlc types
struct libvlc_instance_t;
struct libvlc_media_player_t;
struct libvlc_media_t;

class LibVlcPlayer : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(float fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QString videoCodec READ videoCodec NOTIFY videoStatsChanged)
    Q_PROPERTY(QString videoResolution READ videoResolution NOTIFY videoStatsChanged)
    Q_PROPERTY(int videoBitrate READ videoBitrate NOTIFY videoStatsChanged)
    Q_PROPERTY(double videoFps READ videoFps NOTIFY videoStatsChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QStringList hwDecoders READ hwDecoders WRITE setHwDecoders NOTIFY hwDecodersChanged)
    Q_PROPERTY(QObject* analyticsEngine READ analyticsEngine WRITE setAnalyticsEngine NOTIFY analyticsEngineChanged)
    Q_PROPERTY(QString cameraId READ cameraId WRITE setCameraId NOTIFY cameraIdChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool mirror READ mirror WRITE setMirror NOTIFY mirrorChanged)
    Q_PROPERTY(double playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)
    Q_PROPERTY(int bufferMode READ bufferMode WRITE setBufferMode NOTIFY bufferModeChanged)
    Q_PROPERTY(QString rtspTransport READ rtspTransport WRITE setRtspTransport NOTIFY rtspTransportChanged)
    Q_PROPERTY(QString analyticsUrl READ analyticsUrl WRITE setAnalyticsUrl NOTIFY analyticsUrlChanged)
    
    // Video Adjustments
    Q_PROPERTY(float brightness READ brightness WRITE setBrightness NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(float contrast READ contrast WRITE setContrast NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(int hue READ hue WRITE setHue NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(float saturation READ saturation WRITE setSaturation NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(float gamma READ gamma WRITE setGamma NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(QString hwDecoding READ hwDecoding WRITE setHwDecoding NOTIFY hwDecodingChanged)
    Q_PROPERTY(bool audioNormalization READ audioNormalization WRITE setAudioNormalization NOTIFY audioNormalizationChanged)
    Q_PROPERTY(bool backgroundMode READ backgroundMode WRITE setBackgroundMode NOTIFY backgroundModeChanged)

public:
    explicit LibVlcPlayer(QQuickItem *parent = nullptr);
    ~LibVlcPlayer() override;
    
    Q_INVOKABLE bool saveSnapshot(const QString &path);

    Renderer *createRenderer() const override;
    
    // Recording
    Q_PROPERTY(QString recordingPath READ recordingPath WRITE setRecordingPath NOTIFY recordingPathChanged)
    QString recordingPath() const { return m_recordingPath; }
    void setRecordingPath(const QString &path);

    bool audioNormalization() const { return m_audioNormalization; }
    void setAudioNormalization(bool normalize);
    
    bool backgroundMode() const { return m_backgroundMode; }
    void setBackgroundMode(bool bg);

    QString url() const { return m_url; }
    void setUrl(const QString &url);

    QString analyticsUrl() const { return m_analyticsUrl; }
    void setAnalyticsUrl(const QString &url);

    void setHwDecoding(const QString &hw);
    QString hwDecoding() const { return m_hwDecoding; }

    // Adjustments setters/getters
    float brightness() const { return m_brightness; }
    void setBrightness(float val);

    float contrast() const { return m_contrast; }
    void setContrast(float val);

    int hue() const { return m_hue; }
    void setHue(int val);

    float saturation() const { return m_saturation; }
    void setSaturation(float val);

    float gamma() const { return m_gamma; }
    void setGamma(float val);

    bool isRunning() const { return m_running; }
    void setRunning(bool running);

    float fillMode() const { return m_fillMode; }
    void setFillMode(float mode);

    int orientation() const { return m_orientation; }
    void setOrientation(int orientation);

    double playbackRate() const { return m_playbackRate; }
    void setPlaybackRate(double rate);

    bool mirror() const { return m_mirror; }
    void setMirror(bool mirror);

    int bufferMode() const { return m_bufferMode; }
    void setBufferMode(int mode);

    QString rtspTransport() const { return m_rtspTransport; }
    void setRtspTransport(const QString &transport);

    qint64 duration() const; // Not supported, returns 0 for live
    qint64 position() const; // Not supported, returns 0 for live
    void setPosition(qint64 ms);

    Q_INVOKABLE void updateMediaInfo();

    QString videoCodec() const { return m_videoCodec; }
    QString videoResolution() const { return m_videoResolution; }
    int videoBitrate() const { return m_videoBitrate; }
    double videoFps() const { return m_videoFps; }
    double volume() const { return m_volume; }
    bool muted() const { return m_muted; }
    QStringList hwDecoders() const { return m_hwDecoders; }

    void setVolume(double volume);
    void setMuted(bool muted);
    void setHwDecoders(const QStringList &decoders);

    QObject* analyticsEngine() const { return m_dummyAnalytics; }
    void setAnalyticsEngine(QObject* engine);

    QString cameraId() const { return m_cameraId; }
    void setCameraId(const QString& id);

    // Expose state for reconnection logic (stubbed)
    Q_INVOKABLE int mediaStatus() const { return m_running ? 1 : 0; }

    // Frame access for renderer
    QImage takeFrameCopy();

signals:
    void urlChanged();
    void analyticsUrlChanged();
    void runningChanged();
    void recordingPathChanged();
    void audioNormalizationChanged();
    void fillModeChanged();
    void orientationChanged();
    void videoStatsChanged();
    void volumeChanged();
    void mutedChanged();
    void hwDecodersChanged();
    void frameReady();
    void mediaStatusChanged(int status);
    void analyticsEngineChanged();
    void cameraIdChanged();
    void durationChanged();
    void positionChanged();
    void mirrorChanged();
    void playbackRateChanged();
    void bufferModeChanged();
    void rtspTransportChanged();
    void backgroundModeChanged();
    // New Signals
    void videoAdjustmentsChanged();
    void hwDecodingChanged();

private:
    static void* lockCallback(void *opaque, void **planes);
    static void unlockCallback(void *opaque, void *picture, void *const *planes);
    static void displayCallback(void *opaque, void *picture);
    static unsigned formatSetup(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
    static void formatCleanup(void *opaque);

    void* lockFrame(void **planes);
    void displayFrame();
    void ensurePlayer();
    void startPlayback();
    void stopPlayback();
    void applyMediaOptions(libvlc_media_t *media);
    void updateStatsOnFrame(int width, int height);

    libvlc_instance_t *m_instance = nullptr;
    libvlc_media_player_t *m_player = nullptr;
    libvlc_media_t *m_media = nullptr;

    QString m_url;
    QString m_recordingPath;
    QString m_analyticsUrl;
    bool m_running = false;
    bool m_audioNormalization = false;
    int m_orientation = 0;
    bool m_mirror = false;
    float m_fillMode = 1.0f;
    double m_playbackRate = 1.0;
    int m_bufferMode = 1; // 0: realtime, 1: balanced, 2: smooth
    QString m_rtspTransport = "tcp";
    
    // Adjustments
    float m_brightness = 1.0f;
    float m_contrast = 1.0f;
    int m_hue = 0;
    float m_saturation = 1.0f;
    float m_gamma = 1.0f;
    QString m_hwDecoding = "auto";

    QString m_videoCodec = "H264/HEVC";
    QString m_videoResolution;
    int m_videoBitrate = 0; // Unknown for live; keep 0
    double m_videoFps = 0.0;
    double m_volume = 1.0;
    bool m_muted = false;
    bool m_backgroundMode = false;
    QStringList m_hwDecoders;

    QObject* m_dummyAnalytics = nullptr;
    QString m_cameraId;

    QMutex m_frameMutex;
    QByteArray m_frameBuffer;
    QSize m_frameSize;
    std::atomic<bool> m_frameReady{false};
    std::atomic<int> m_frameCount{0};
    qint64 m_lastFpsTime = 0;

    QByteArray m_pluginPathArg;

    QTimer *m_statsTimer = nullptr;
};
