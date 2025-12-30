#ifndef MDKPLAYER_H
#define MDKPLAYER_H

#include <QQuickFramebufferObject>
#include <mdk/Player.h>
#include <memory>
#include <chrono>
#include <mutex>
#include <atomic>
#include <QTimer>

class AnalyticsEngine;

class MdkPlayer : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    // -1: KeepAspectRatioCrop, 0: IgnoreAspectRatio, >0: KeepAspectRatio (default 1)
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

public:
    explicit MdkPlayer(QQuickItem *parent = nullptr);
    ~MdkPlayer();

    Renderer *createRenderer() const override;

    QString url() const;
    void setUrl(const QString &url);

    bool isRunning() const;
    void setRunning(bool running);

    float fillMode() const { return m_fillMode; }
    void setFillMode(float mode);

    int orientation() const { return m_orientation; }
    void setOrientation(int orientation);

    double playbackRate() const { return m_playbackRate; }
    void setPlaybackRate(double rate);
    
    bool mirror() const { return m_mirror; }
    void setMirror(bool mirror);

    qint64 duration() const;
    qint64 position() const;
    void setPosition(qint64 ms);

    Q_INVOKABLE void updateMediaInfo();

    std::shared_ptr<mdk::Player> player() const { return m_player; }

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

    QObject* analyticsEngine() const { return m_analyticsEngineObj; }
    void setAnalyticsEngine(QObject* engine);

    QString cameraId() const { return m_cameraId; }
    void setCameraId(const QString& id);

    // Expose state for reconnection logic
    Q_INVOKABLE int mediaStatus() const;

signals:
    void urlChanged();
    void runningChanged();
    void fillModeChanged();
    void orientationChanged();
    void videoStatsChanged();
    void volumeChanged();
    void mutedChanged();
    void hwDecodersChanged();
    void frameReady(const QImage &image);
    void mediaStatusChanged(int status);
    void analyticsEngineChanged();
    void cameraIdChanged();
    void durationChanged();
    void positionChanged();
    void mirrorChanged();
    void playbackRateChanged();

private:
    std::shared_ptr<mdk::Player> m_player;
    QString m_url;
    bool m_running = false;
    int m_orientation = 0;
    bool m_mirror = false;
    float m_fillMode = 1.0f; // KeepAspectRatio
    double m_playbackRate = 1.0;
    
    QString m_videoCodec;
    QString m_videoResolution;
    int m_videoBitrate = 0;
    double m_videoFps = 0.0;
    double m_volume = 1.0;
    bool m_muted = false;
    QStringList m_hwDecoders;

    QObject* m_analyticsEngineObj = nullptr;
    AnalyticsEngine* m_analyticsEngine = nullptr;
    QString m_cameraId;

    // Analytics throttling
    qint64 m_lastFrameTime = 0;

    // FPS Calculation
    std::atomic<int> m_frameCount{0};
    qint64 m_lastFpsTime = 0;
    
    QTimer* m_infoTimer = nullptr;
};

#endif // MDKPLAYER_H
