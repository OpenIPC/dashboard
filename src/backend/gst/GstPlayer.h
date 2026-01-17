#pragma once

#include <QQuickItem>
#include <QImage>
#include <QMutex>
#include <QTimer>
#include <atomic>

#include <gst/gst.h>

class GstPlayer : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(float scaleX READ scaleX NOTIFY scaleXChanged) // For weird aspect ratios if needed
    Q_PROPERTY(float scaleY READ scaleY NOTIFY scaleYChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QString rtspTransport READ rtspTransport WRITE setRtspTransport NOTIFY rtspTransportChanged)
    Q_PROPERTY(QString cameraId READ cameraId WRITE setCameraId NOTIFY cameraIdChanged)
    
    // Stubs for LibVlcPlayer compatibility
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QString hwDecoding READ hwDecoding WRITE setHwDecoding NOTIFY hwDecodingChanged)
    Q_PROPERTY(bool hwDecoders READ hwDecoders WRITE setHwDecoders NOTIFY hwDecodersChanged)
    Q_PROPERTY(float brightness READ brightness WRITE setBrightness NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(float contrast READ contrast WRITE setContrast NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(int hue READ hue WRITE setHue NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(float saturation READ saturation WRITE setSaturation NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(float gamma READ gamma WRITE setGamma NOTIFY videoAdjustmentsChanged)
    Q_PROPERTY(bool backgroundMode READ backgroundMode WRITE setBackgroundMode NOTIFY backgroundModeChanged)
    Q_PROPERTY(double playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)
    Q_PROPERTY(bool mirror READ mirror WRITE setMirror NOTIFY mirrorChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(int bufferMode READ bufferMode WRITE setBufferMode NOTIFY bufferModeChanged)
    Q_PROPERTY(bool audioNormalization READ audioNormalization WRITE setAudioNormalization NOTIFY audioNormalizationChanged)
    Q_PROPERTY(int fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    Q_PROPERTY(QString recordingPath READ recordingPath WRITE setRecordingPath NOTIFY recordingPathChanged)

    // Analytics/Snapshot support
    Q_PROPERTY(QObject* analyticsEngine READ analyticsEngine WRITE setAnalyticsEngine NOTIFY analyticsEngineChanged)
    Q_PROPERTY(QString analyticsUrl READ analyticsUrl WRITE setAnalyticsUrl NOTIFY analyticsUrlChanged)

    Q_PROPERTY(QString videoCodec READ videoCodec NOTIFY videoStatsChanged)
    Q_PROPERTY(int videoWidth READ videoWidth NOTIFY videoStatsChanged)
    Q_PROPERTY(int videoHeight READ videoHeight NOTIFY videoStatsChanged)
    Q_PROPERTY(int videoFps READ videoFps NOTIFY videoStatsChanged)
    Q_PROPERTY(int videoBitrate READ videoBitrate NOTIFY videoStatsChanged)

public:
    explicit GstPlayer(QQuickItem *parent = nullptr);
    ~GstPlayer() override;

    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

    QString url() const { return m_url; }
    void setUrl(const QString &url);
    
    // Stats accessors
    QString videoCodec() const { return m_videoCodec; }
    int videoWidth() const { return m_videoWidth; }
    int videoHeight() const { return m_videoHeight; }
    int videoFps() const { return m_videoFps; }
    int videoBitrate() const { return m_videoBitrate; }
    
    // Video Adjustments
    float brightness() const { return m_brightness; }
    void setBrightness(float value);

    float contrast() const { return m_contrast; }
    void setContrast(float value);

    int hue() const { return m_hue; }
    void setHue(int value);

    float saturation() const { return m_saturation; }
    void setSaturation(float value);
    
    // Geometry
    int orientation() const { return m_orientation; }
    void setOrientation(int angle);
    
    bool mirror() const { return m_mirror; }
    void setMirror(bool value);

    float gamma() const { return m_gamma; }
    void setGamma(float value);
    
    QString hwDecoding() const { return "auto"; }
    void setHwDecoding(const QString&) {}
    
    // Legacy support property for older configs
    bool hwDecoders() const { return true; }
    void setHwDecoders(bool) {}
    
    bool backgroundMode() const { return false; }
    void setBackgroundMode(bool) {}
    
    double playbackRate() const { return m_playbackRate; }
    void setPlaybackRate(double rate);


    // End of Video Adjustments / Geometry

    // (Resuming regular public methods)


    
    qint64 duration() const;
    qint64 position() const;
    void setPosition(qint64 pos);
    
    int bufferMode() const { return m_bufferMode; }
    void setBufferMode(int mode);

    bool audioNormalization() const { return false; }
    void setAudioNormalization(bool) {}

    int fillMode() const { return 1; }
    void setFillMode(int) {}

    QString recordingPath() const { return m_recordingPath; }
    void setRecordingPath(const QString &path) {
        if (m_recordingPath != path) {
            m_recordingPath = path;
            emit recordingPathChanged();
        }
    }

    bool isRunning() const { return m_running; }
    void setRunning(bool running);

    QString rtspTransport() const { return m_rtspTransport; }
    void setRtspTransport(const QString &transport); // "tcp", "udp", "http"

    double volume() const { return m_volume; }
    void setVolume(double volume);

    bool muted() const { return m_muted; }
    void setMuted(bool muted);

    Q_INVOKABLE bool saveSnapshot(const QString &path);
    Q_INVOKABLE void updateMediaInfo();

    // Frame Access for Analytics
    QImage takeFrameCopy();
    
    // Internal use for Renderer
    QImage getLastFrame();

    QString cameraId() const { return m_cameraId; }
    void setCameraId(const QString& id);

    QObject* analyticsEngine() const { return m_analyticsEngine; }
    void setAnalyticsEngine(QObject* engine);

    QString analyticsUrl() const { return m_analyticsUrl; }
    void setAnalyticsUrl(const QString &url) {
        if (m_analyticsUrl != url) {
            m_analyticsUrl = url;
            emit analyticsUrlChanged();
        }
    }

    float scaleX() const { return 1.0f; }
    float scaleY() const { return 1.0f; }

signals:
    void urlChanged();
    void runningChanged();
    void volumeChanged();
    void mutedChanged();
    void rtspTransportChanged();
    void frameReady(); 
    void errorOccurred(QString message);
    void cameraIdChanged();
    void analyticsEngineChanged();
    void analyticsUrlChanged();
    void scaleXChanged();
    void scaleYChanged();
    // Compatibility signals
    void orientationChanged();
    void hwDecodingChanged();
    void hwDecodersChanged();
    void videoAdjustmentsChanged();
    void backgroundModeChanged();
    void playbackRateChanged(); // existed
    void mirrorChanged();       // existed
    void durationChanged();
    void positionChanged();
    void bufferModeChanged();
    void audioNormalizationChanged();
    void fillModeChanged();
    void recordingPathChanged();
    // Compatibility stubs
    void videoStatsChanged();
    void mediaStatusChanged(int status);

private slots:
    void handleBusMessage(const QString& type, const QString& msg) {}

private:
    void startPipeline();
    void stopPipeline();
    void restartPipeline();
    void updateFlipMethod();
    GstElement* createVideoFilterBin();
    
    static GstFlowReturn onNewSample(GstElement *sink, GstPlayer *player);
    static void onBusMessage(GstBus *bus, GstMessage *msg, gpointer data);
    static void onSourcePadAdded(GstElement *element, GstPad *pad, gpointer user_data);
    static void onRecordingPadAdded(GstElement *element, GstPad *pad, gpointer user_data);
    static GstPadProbeReturn onSourcePadProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

    QString m_url;
    QString m_recordingPath;
    QString m_rtspTransport = "tcp";
    int m_bufferMode = 1;
    bool m_running = false;
    double m_volume = 1.0;
    bool m_muted = false;

    GstElement *m_pipeline = nullptr;
    GstElement *m_source = nullptr;
    GstElement *m_volumeElement = nullptr;
    
    // Filters
    GstElement *m_videoBalance = nullptr;
    GstElement *m_videoFlip = nullptr; 
    GstElement *m_videoGamma = nullptr;

    // Adjustments
    float m_brightness = 0.0f;
    float m_contrast = 1.0f;
    int m_hue = 0;
    float m_saturation = 1.0f;
    float m_gamma = 1.0f;
    int m_orientation = 0; 
    bool m_mirror = false;

    QMutex m_frameMutex;
    QImage m_currentFrame;
    std::atomic<bool> m_frameReady{false};

    QString m_cameraId;
    QObject* m_analyticsEngine = nullptr;
    QString m_analyticsUrl;
    

    friend class GstBusHelper;
    
public:
    GstElement* pipeline() const { return m_pipeline; }

private:
    // Stats storage
    QString m_videoCodec = "H264";
    int m_videoWidth = 0;
    int m_videoHeight = 0;
    int m_videoFps = 0;
    int m_videoBitrate = 0;

    // Stats calculation
    QTimer* m_statsTimer = nullptr;
    std::atomic<int> m_frameCountInst{0};
    std::atomic<long long> m_byteCountInst{0};
    
    qint64 m_lastDuration = 0;
    qint64 m_lastPosition = 0;

    // Recording helpers
    GstElement* m_muxer = nullptr; // Used for dynamic linking during recording
    void setupRecordingBin(GstElement* pipeline);

    double m_playbackRate = 1.0;

private slots:
    void updateStats();
};
