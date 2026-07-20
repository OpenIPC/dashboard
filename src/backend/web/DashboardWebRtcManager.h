#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include <gst/gst.h>

class SystemController;

class DashboardWebRtcManager final : public QObject
{
    Q_OBJECT

public:
    explicit DashboardWebRtcManager(SystemController *systemController,
                                    QObject *parent = nullptr);
    ~DashboardWebRtcManager() override;

    bool available() const;
    bool audioAvailable() const { return m_audioAvailable; }
    QString availabilityError() const;
    int activePeers() const { return m_peers.size(); }

    bool startPeer(const QString &peerId, int cameraIndex, const QString &quality,
                   QString *error = nullptr);
    bool setAnswer(const QString &peerId, const QString &sdp, QString *error = nullptr);
    bool addIceCandidate(const QString &peerId, int mlineIndex,
                         const QString &candidate, QString *error = nullptr);
    void stopPeer(const QString &peerId);
    void stopAll();

signals:
    void messageReady(const QString &peerId, const QVariantMap &message);
    void peerStopped(const QString &peerId);

private:
    struct Peer;
    struct PromiseContext;

    static void onSourcePadAdded(GstElement *source, GstPad *pad, gpointer userData);
    static void onNegotiationNeeded(GstElement *webrtc, gpointer userData);
    static void onOfferCreated(GstPromise *promise, gpointer userData);
    static void onIceCandidate(GstElement *webrtc, guint mlineIndex,
                               gchar *candidate, gpointer userData);
    static void onBusMessage(GstBus *bus, GstMessage *message, gpointer userData);

    void postMessage(const QString &peerId, const QVariantMap &message);
    void postError(const QString &peerId, const QString &message);
    void scheduleStop(const QString &peerId);
    void removePeer(const QString &peerId, bool notify);

    SystemController *m_systemController = nullptr;
    QHash<QString, Peer *> m_peers;
    QString m_availabilityError;
    bool m_audioAvailable = false;
};
