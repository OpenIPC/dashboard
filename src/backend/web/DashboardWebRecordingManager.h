#pragma once

#include <QHash>
#include <QObject>
#include <QVariantList>

class GstPlayer;
class QTimer;
class SystemController;

class DashboardWebRecordingManager final : public QObject
{
    Q_OBJECT

public:
    explicit DashboardWebRecordingManager(SystemController *systemController,
                                          QObject *parent = nullptr);
    ~DashboardWebRecordingManager() override;

    bool start(int cameraIndex, QString *error = nullptr);
    bool stop(int cameraIndex, QString *error = nullptr);
    void stopAll();
    bool isRecording(int cameraIndex) const;
    int activeCount() const { return m_recordings.size(); }
    QVariantList status() const;

signals:
    void recordingStateChanged(int cameraIndex, bool recording);
    void recordingError(int cameraIndex, const QString &message);

private:
    struct Entry;

    QString sourceUrl(int cameraIndex) const;
    void setCameraRecording(int cameraIndex, bool recording);
    void rotate(int cameraIndex);
    void destroyEntry(int cameraIndex, bool notifyStopped);

    SystemController *m_systemController = nullptr;
    QHash<int, Entry *> m_recordings;
};
