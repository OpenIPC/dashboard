#pragma once

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QString>

class SystemController;

class DashboardWebPreviewManager final : public QObject
{
public:
    enum class FrameStatus {
        Ready,
        WarmingUp,
        InvalidCamera,
        MissingStream,
        Unavailable
    };

    struct FrameResult {
        FrameStatus status = FrameStatus::Unavailable;
        QByteArray jpeg;
    };

    explicit DashboardWebPreviewManager(SystemController *systemController,
                                        QObject *parent = nullptr);
    ~DashboardWebPreviewManager() override;

    FrameResult frame(int cameraIndex, const QString &quality);
    void stop();

private:
    struct StreamState;

    StreamState *ensureStream(int cameraIndex, const QString &quality,
                              FrameStatus *status);
    void removeStream(const QString &key);
    void cleanupIdleStreams();

    SystemController *m_systemController = nullptr;
    QHash<QString, StreamState *> m_streams;
};
