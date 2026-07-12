#pragma once

#include <QJsonArray>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>

#include <memory>

class CameraModel;
class CameraHealthControllerPrivate;

class CameraHealthController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList profiles READ profiles CONSTANT)
    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)
    Q_PROPERTY(QVariantList currentResults READ currentResults NOTIFY currentResultsChanged)
    Q_PROPERTY(QVariantMap latestRun READ latestRun NOTIFY historyChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int completedProbes READ completedProbes NOTIFY progressChanged)
    Q_PROPERTY(int totalProbes READ totalProbes NOTIFY progressChanged)
    Q_PROPERTY(QString activeProfile READ activeProfile NOTIFY runningChanged)

public:
    explicit CameraHealthController(CameraModel *cameraModel, CameraModel *gridModel,
                                    QObject *parent = nullptr);
    ~CameraHealthController() override;

    QVariantList profiles() const;
    QVariantList history() const;
    QVariantList currentResults() const;
    QVariantMap latestRun() const;
    bool running() const;
    int completedProbes() const;
    int totalProbes() const;
    QString activeProfile() const;

    Q_INVOKABLE bool runAll(const QString &profileId);
    Q_INVOKABLE bool runCamera(const QString &cameraIp, const QString &profileId);
    Q_INVOKABLE bool refreshCameraTelemetry(const QString &cameraIp);
    Q_INVOKABLE void refreshAllTelemetry();
    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE QVariantMap runById(const QString &runId) const;
    Q_INVOKABLE QVariantMap resultForCamera(const QString &cameraIp) const;
    Q_INVOKABLE QString reportText(const QString &runId = QString()) const;

    QJsonArray historyJson() const;
    void restoreHistory(const QJsonArray &history);

signals:
    void historyChanged();
    void currentResultsChanged();
    void runningChanged();
    void progressChanged();
    void cameraResultUpdated(const QString &cameraIp);
    void telemetryUpdated(const QString &cameraIp);
    void runCompleted(const QString &runId);

private:
    std::unique_ptr<CameraHealthControllerPrivate> d;

    bool startRun(const QList<int> &cameraIndexes, const QString &profileId);
    void startPendingProbes();
    void startProbe(const QVariantMap &task);
    void startRtspProbe(const QVariantMap &task);
    void startHttpProbe(const QVariantMap &task);
    void setProbeRunning(const QVariantMap &task);
    void completeProbe(const QVariantMap &task, bool success, const QString &message,
                       int httpStatus, int elapsedMs, const QVariantMap &details = {});
    void finalizeRun();
};
